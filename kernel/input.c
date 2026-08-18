/* input.c - a real input stack.
 *
 * What zlOS had before this was a lookup table: one scancode in, one character
 * out. That is enough to type a command letter and nothing more. It has no
 * concept of a key being held, no caps lock, no control or alt, no arrow keys,
 * no repeat, and no idea that the keyboard and the mouse are two sources of
 * one stream of events.
 *
 * What an operating system needs instead is an EVENT model:
 *
 *     every key press and release is an event, with the modifier state
 *     attached, and characters are something derived from events rather than
 *     the only thing that exists
 *
 * That distinction is what lets a program tell Ctrl+C from 'c', hold an arrow
 * key to repeat, know that shift is down while the mouse moves, or handle a
 * key that has no character at all. It is the difference between a shell that
 * reads letters and a system that can host a text editor.
 *
 * Two sources feed the same queue: the PS/2 controller (via the IRQ1 handler)
 * and USB HID (via the xHCI driver). Neither knows about the other, and
 * nothing downstream can tell which one a keystroke came from - which is
 * exactly right, because on this laptop the internal keyboard is PS/2 and an
 * external one is USB, and both should just work.
 */

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

extern u32 idt_ticks(void);
extern int idt_scan(void);        /* raw PS/2 scancode from the IRQ buffer */
extern int xhci_key_event(void);  /* a raw USB HID key event               */
extern int xhci_kbd_mods(void);   /* live USB modifier bitmap              */

/* ---- event model ------------------------------------------------------- */
#define EV_NONE      0
#define EV_KEY_DOWN  1
#define EV_KEY_UP    2
#define EV_CHAR      3
#define EV_MOUSE     4

/* Key codes. Printable keys use their unshifted ASCII so the common case is
 * trivial; everything else is above 0x100 where it cannot collide. */
#define KEY_ESC       0x101
#define KEY_BACKSPACE 0x102
#define KEY_TAB       0x103
#define KEY_ENTER     0x104
#define KEY_LEFT      0x110
#define KEY_RIGHT     0x111
#define KEY_UP        0x112
#define KEY_DOWN      0x113
#define KEY_HOME      0x114
#define KEY_END       0x115
#define KEY_PGUP      0x116
#define KEY_PGDN      0x117
#define KEY_INSERT    0x118
#define KEY_DELETE    0x119
#define KEY_F1        0x120        /* F1..F12 are KEY_F1 + n */

#define MOD_SHIFT   (1 << 0)
#define MOD_CTRL    (1 << 1)
#define MOD_ALT     (1 << 2)
#define MOD_CAPS    (1 << 3)
#define MOD_NUM     (1 << 4)
#define MOD_SUPER   (1 << 5)

struct event {
    u16 type;
    u16 mods;
    u32 code;      /* a KEY_* value, or the character for EV_CHAR */
    int x, y;      /* mouse only */
};

#define EVQ_SIZE 64
static struct event evq[EVQ_SIZE];
static int evq_head = 0, evq_tail = 0;

static int mods = 0;          /* PS/2 modifiers, plus the shared caps/num latch */
static int usb_mods = 0;      /* USB modifiers, which arrive as their own snapshot */

/* Which keys are currently held, for repeat and for "is shift down" queries.
 * Three regions, because three numberings reach this file and they collide:
 *     0x000..0x0FF   PS/2 set 1
 *     0x100..0x1FF   PS/2 set 1, 0xE0-prefixed
 *     0x200..0x2FF   USB HID usage IDs
 * Without the third region a USB 'a' (usage 0x04) and a PS/2 F9 (scancode
 * 0x04) would be the same slot, and releasing one would un-hold the other. */
static u8 key_down[768];
static u32 repeat_code = 0;
static u32 repeat_at   = 0;
static int repeat_mods = 0;

#define REPEAT_DELAY  50      /* ticks before the first repeat: 500 ms */
#define REPEAT_RATE    3      /* ticks between repeats: ~33/second     */

static void evq_push(int type, u32 code, int m, int x, int y)
{
    int next = (evq_tail + 1) % EVQ_SIZE;
    if (next == evq_head) return;          /* full: drop, never overwrite */
    evq[evq_tail].type = (u16)type;
    evq[evq_tail].mods = (u16)m;
    evq[evq_tail].code = code;
    evq[evq_tail].x = x;
    evq[evq_tail].y = y;
    evq_tail = next;
}

/* ---- scancode set 1, unshifted and shifted ----------------------------
 * The i8042 is programmed for translation, so what arrives is set 1: the
 * original PC/XT numbering, where a release is the press code with bit 7 set
 * and anything unusual is prefixed with 0xE0. */
static const char sc_plain[128] = {
    0,   0,  '1','2','3','4','5','6','7','8','9','0','-','=', 0,  0,
    'q','w','e','r','t','y','u','i','o','p','[',']', 0,  0,  'a','s',
    'd','f','g','h','j','k','l',';','\'','`', 0, '\\','z','x','c','v',
    'b','n','m',',','.','/', 0,  '*', 0,  ' ', 0,  0,  0,  0,  0,  0,
    0,   0,  0,  0,  0,  0,  0,  '7','8','9','-','4','5','6','+','1',
    '2','3','0','.', 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

static const char sc_shift[128] = {
    0,   0,  '!','@','#','$','%','^','&','*','(',')','_','+', 0,  0,
    'Q','W','E','R','T','Y','U','I','O','P','{','}', 0,  0,  'A','S',
    'D','F','G','H','J','K','L',':','"','~',  0, '|','Z','X','C','V',
    'B','N','M','<','>','?', 0,  '*', 0,  ' ', 0,  0,  0,  0,  0,  0,
    0,   0,  0,  0,  0,  0,  0,  '7','8','9','-','4','5','6','+','1',
    '2','3','0','.', 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

/* the non-printing keys, by scancode */
static u32 sc_special(int sc)
{
    switch (sc) {
        case 0x01: return KEY_ESC;
        case 0x0E: return KEY_BACKSPACE;
        case 0x0F: return KEY_TAB;
        case 0x1C: return KEY_ENTER;
        case 0x3B: case 0x3C: case 0x3D: case 0x3E:
        case 0x3F: case 0x40: case 0x41: case 0x42:
        case 0x43: case 0x44: return KEY_F1 + (u32)(sc - 0x3B);
        case 0x57: return KEY_F1 + 10;     /* F11 */
        case 0x58: return KEY_F1 + 11;     /* F12 */
    }
    return 0;
}

/* the 0xE0-prefixed set: the arrows and the navigation cluster */
static u32 sc_extended(int sc)
{
    switch (sc) {
        case 0x48: return KEY_UP;
        case 0x50: return KEY_DOWN;
        case 0x4B: return KEY_LEFT;
        case 0x4D: return KEY_RIGHT;
        case 0x47: return KEY_HOME;
        case 0x4F: return KEY_END;
        case 0x49: return KEY_PGUP;
        case 0x51: return KEY_PGDN;
        case 0x52: return KEY_INSERT;
        case 0x53: return KEY_DELETE;
        case 0x1C: return KEY_ENTER;       /* keypad enter */
        case 0x5B: return 0;               /* left super - handled as a mod */
    }
    return 0;
}

/* Turn a key code plus the current modifiers into a character, or 0 if this
 * key does not produce one.
 *
 * Caps lock is not shift. It affects only letters, and it COMBINES with shift
 * rather than being overridden by it - caps on plus shift held gives you a
 * lowercase letter, which is a detail almost every first attempt gets wrong. */
static u32 to_char(int sc, int m)
{
    if (sc < 0 || sc >= 128) return 0;

    int shifted = (m & MOD_SHIFT) ? 1 : 0;
    char c = shifted ? sc_shift[sc] : sc_plain[sc];
    if (!c) return 0;

    if (m & MOD_CAPS) {
        if (c >= 'a' && c <= 'z') c = (char)(c - 32);
        else if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
    }

    /* Control turns a letter into its control code: Ctrl+C is 3, and that is
     * what makes an interrupt key possible at all. */
    if (m & MOD_CTRL) {
        if (c >= 'a' && c <= 'z') return (u32)(c - 'a' + 1);
        if (c >= 'A' && c <= 'Z') return (u32)(c - 'A' + 1);
        return 0;
    }
    return (u32)(unsigned char)c;
}

/* ---- USB HID: a third numbering, folded onto the first two ---------------
 * HID usage IDs are neither ASCII nor PC scancodes. Rather than carry a second
 * character map - which is how the two keyboards would drift apart - the
 * printable keys are translated into the set-1 scancode they correspond to and
 * then handed to to_char() above. One keymap, one caps/ctrl/shift policy, both
 * keyboards. The keys that have no character get a KEY_* code directly. */
static u32 hid_to_key(int usage)
{
    switch (usage) {
        case 0x28: return KEY_ENTER;
        case 0x29: return KEY_ESC;
        case 0x2A: return KEY_BACKSPACE;
        case 0x2B: return KEY_TAB;
        case 0x49: return KEY_INSERT;
        case 0x4A: return KEY_HOME;
        case 0x4B: return KEY_PGUP;
        case 0x4C: return KEY_DELETE;
        case 0x4D: return KEY_END;
        case 0x4E: return KEY_PGDN;
        case 0x4F: return KEY_RIGHT;
        case 0x50: return KEY_LEFT;
        case 0x51: return KEY_DOWN;
        case 0x52: return KEY_UP;
        case 0x58: return KEY_ENTER;                     /* keypad enter */
    }
    if (usage >= 0x3A && usage <= 0x45)                  /* F1..F12 */
        return KEY_F1 + (u32)(usage - 0x3A);
    return 0;
}

static int hid_to_sc1(int usage)
{
    /* HID runs the alphabet in alphabetical order; set 1 runs it in the order
     * the keys sit on the board. Neither is derivable from the other. */
    static const u8 letters[26] = {
        0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25,
        0x26, 0x32, 0x31, 0x18, 0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F,
        0x11, 0x2D, 0x15, 0x2C
    };
    if (usage >= 0x04 && usage <= 0x1D) return letters[usage - 0x04];
    if (usage >= 0x1E && usage <= 0x27) return 0x02 + (usage - 0x1E);  /* 1..9,0 */

    switch (usage) {
        case 0x2C: return 0x39;        /* space */
        case 0x2D: return 0x0C;        /* -     */
        case 0x2E: return 0x0D;        /* =     */
        case 0x2F: return 0x1A;        /* [     */
        case 0x30: return 0x1B;        /* ]     */
        case 0x31: return 0x2B;        /* \     */
        case 0x33: return 0x27;        /* ;     */
        case 0x34: return 0x28;        /* '     */
        case 0x35: return 0x29;        /* `     */
        case 0x36: return 0x33;        /* ,     */
        case 0x37: return 0x34;        /* .     */
        case 0x38: return 0x35;        /* /     */
    }
    return 0;
}

static int hid_mods_to_mods(int h)
{
    int m = 0;
    if (h & 0x22) m |= MOD_SHIFT;      /* bit 1 left, bit 5 right */
    if (h & 0x11) m |= MOD_CTRL;
    if (h & 0x44) m |= MOD_ALT;
    if (h & 0x88) m |= MOD_SUPER;
    return m;
}

/* ---- feeding the queue from the PS/2 stream ---------------------------- */
static int ext_pending = 0;

static void handle_scancode(int sc)
{
    if (sc == 0xE0) { ext_pending = 1; return; }

    int release = (sc & 0x80) ? 1 : 0;
    int code    = sc & 0x7F;
    int ext     = ext_pending;
    ext_pending = 0;

    /* modifiers first - they change how everything else is interpreted */
    if (!ext) {
        if (code == 0x2A || code == 0x36) {          /* either shift */
            if (release) mods &= ~MOD_SHIFT; else mods |= MOD_SHIFT;
            return;
        }
        if (code == 0x1D) {                          /* left control */
            if (release) mods &= ~MOD_CTRL; else mods |= MOD_CTRL;
            return;
        }
        if (code == 0x38) {                          /* left alt */
            if (release) mods &= ~MOD_ALT; else mods |= MOD_ALT;
            return;
        }
        if (code == 0x3A && !release) { mods ^= MOD_CAPS; return; }  /* caps toggles */
        if (code == 0x45 && !release) { mods ^= MOD_NUM;  return; }
    } else {
        if (code == 0x1D) {                          /* right control */
            if (release) mods &= ~MOD_CTRL; else mods |= MOD_CTRL;
            return;
        }
        if (code == 0x38) {                          /* right alt */
            if (release) mods &= ~MOD_ALT; else mods |= MOD_ALT;
            return;
        }
        if (code == 0x5B || code == 0x5C) {          /* super */
            if (release) mods &= ~MOD_SUPER; else mods |= MOD_SUPER;
            return;
        }
    }

    u32 key = ext ? sc_extended(code) : sc_special(code);
    u32 ch  = ext ? 0 : to_char(code, mods);
    if (!key && ch) key = ch;                        /* printable: code IS the char */
    if (!key) return;

    int slot = ext ? (0x100 + code) : code;
    if (slot < 0 || slot >= 512) return;

    if (release) {
        key_down[slot] = 0;
        if (repeat_code == key) repeat_code = 0;     /* stop repeating it */
        evq_push(EV_KEY_UP, key, mods, 0, 0);
        return;
    }

    key_down[slot] = 1;
    evq_push(EV_KEY_DOWN, key, mods, 0, 0);
    if (ch) evq_push(EV_CHAR, ch, mods, 0, 0);

    /* arm auto-repeat on this key */
    repeat_code = key;
    repeat_mods = mods;
    repeat_at   = idt_ticks() + REPEAT_DELAY;
}

/* The USB counterpart of handle_scancode, and deliberately the same shape: the
 * two keyboards must produce identical events for the same key, or a bug will
 * appear on one of them and not the other.
 *
 * Caps and num lock are the exception - they are latched state shared by both
 * keyboards, so they live in the PS/2 `mods` word and both paths toggle it. */
static void handle_hid_event(int ev)
{
    int usage = ev & 0xFF;
    int press = (ev & 0x10000) ? 1 : 0;
    int m     = hid_mods_to_mods((ev >> 8) & 0xFF) | (mods & (MOD_CAPS | MOD_NUM));

    if (usage == 0x39) { if (press) mods ^= MOD_CAPS; return; }   /* caps lock */
    if (usage == 0x53) { if (press) mods ^= MOD_NUM;  return; }   /* num lock  */

    u32 key = hid_to_key(usage);
    u32 ch  = key ? 0 : to_char(hid_to_sc1(usage), m);
    if (!key && ch) key = ch;                        /* printable: code IS the char */
    if (!key) return;

    int slot = 0x200 + usage;
    if (slot < 0 || slot >= 768) return;

    if (!press) {
        key_down[slot] = 0;
        if (repeat_code == key) repeat_code = 0;
        evq_push(EV_KEY_UP, key, m, 0, 0);
        return;
    }

    key_down[slot] = 1;
    evq_push(EV_KEY_DOWN, key, m, 0, 0);
    if (ch) evq_push(EV_CHAR, ch, m, 0, 0);

    repeat_code = key;
    repeat_mods = m;
    repeat_at   = idt_ticks() + REPEAT_DELAY;
}

/* ---- the pump ----------------------------------------------------------
 * Called from the shell's idle loop. Drains both hardware sources into the
 * one queue and generates repeats. Everything above is bookkeeping; this is
 * where the timing lives. */
void input_poll(void)
{
    /* PS/2 */
    for (int i = 0; i < 16; i++) {
        int sc = idt_scan();
        if (!sc) break;
        handle_scancode(sc);
    }

    /* USB HID. It used to hand over decoded characters, which is why arrow
     * keys never reached an application from a USB keyboard: there is no
     * character for Up, so it decoded to 0 and 0 means "nothing typed". It
     * hands over raw HID events now and this file translates them. */
    for (int i = 0; i < 16; i++) {
        int ev = xhci_key_event();
        if (!ev) break;
        handle_hid_event(ev);
    }
    /* After the drain, not before: xhci_key_event() polls the controller, so
     * reading the bitmap first would report the state one report out of date. */
    usb_mods = hid_mods_to_mods(xhci_kbd_mods());

    /* auto-repeat: one key at a time, the most recently pressed */
    if (repeat_code) {
        u32 now = idt_ticks();
        if (now >= repeat_at) {
            evq_push(EV_KEY_DOWN, repeat_code, repeat_mods, 0, 0);
            if (repeat_code < 0x100)
                evq_push(EV_CHAR, repeat_code, repeat_mods, 0, 0);
            repeat_at = now + REPEAT_RATE;
        }
    }
}

/* ---- what callers use -------------------------------------------------- */
static struct event last;

int input_next(void)
{
    input_poll();
    if (evq_head == evq_tail) return 0;
    last = evq[evq_head];
    evq_head = (evq_head + 1) % EVQ_SIZE;
    return last.type;
}

int input_type(void)  { return last.type; }
int input_code(void)  { return (int)last.code; }
int input_mods(void)  { return last.mods; }
/* Either keyboard holding shift means shift is down - they are one logical
 * keyboard, and the caller has no business knowing which one you reached for. */
int input_shift(void) { return ((mods | usb_mods) & MOD_SHIFT) ? 1 : 0; }
int input_ctrl(void)  { return ((mods | usb_mods) & MOD_CTRL) ? 1 : 0; }
int input_alt(void)   { return ((mods | usb_mods) & MOD_ALT) ? 1 : 0; }
int input_caps(void)  { return (mods & MOD_CAPS) ? 1 : 0; }   /* a latch, not held */

int input_key_held(int code)
{
    for (int i = 0; i < 768; i++) {
        if (!key_down[i]) continue;

        u32 k;
        if (i >= 0x200) {
            k = hid_to_key(i - 0x200);
            if (!k) k = to_char(hid_to_sc1(i - 0x200), 0);
        } else {
            k = (i >= 0x100) ? sc_extended(i - 0x100) : sc_special(i);
            if (!k) k = to_char(i, 0);
        }
        if ((int)k == code) return 1;
    }
    return 0;
}

/* The simple path, for code that only wants characters: returns the next
 * character typed, or 0. Everything else in the event stream is skipped. */
int input_char(void)
{
    for (int guard = 0; guard < 64; guard++) {
        int t = input_next();
        if (!t) return 0;
        if (t == EV_CHAR) return (int)last.code;
    }
    return 0;
}

/* And the path a text editor wants: a character OR a navigation key, so it
 * can tell "left arrow" from any letter. Returns a KEY_* code above 0x100 for
 * the non-printing ones. */
int input_key(void)
{
    for (int guard = 0; guard < 64; guard++) {
        int t = input_next();
        if (!t) return 0;
        if (t == EV_CHAR) return (int)last.code;
        if (t == EV_KEY_DOWN && last.code >= 0x100) return (int)last.code;
    }
    return 0;
}

int input_queued(void)
{
    int n = evq_tail - evq_head;
    return n < 0 ? n + EVQ_SIZE : n;
}
