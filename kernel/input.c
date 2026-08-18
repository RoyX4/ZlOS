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
extern int xhci_key(void);        /* a decoded character from USB HID      */
extern int idt_mouse_x(void);     /* the PS/2 pointer, published by IRQ12  */
extern int idt_mouse_y(void);
extern int idt_mouse_btn(void);

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

static int mods = 0;

/* which keys are currently held, for repeat and for "is shift down" queries */
static u8 key_down[512];

/* WHAT EACH HELD KEY REPORTED WHEN IT WENT DOWN.
 *
 * A key code is derived through to_char(code, mods) and therefore DEPENDS ON
 * THE MODIFIERS AT THAT MOMENT. Re-deriving it on release asks a different
 * question - "what would this scancode mean now" - and the answer is different
 * the instant a modifier changes between press and release, which is something
 * a human does constantly: shift pressed late while typing a capital, ctrl
 * released before the letter of a shortcut, caps lock bumped.
 *
 * Every consumer of a key's identity now reads this instead of re-deriving:
 * the release event, the repeat disarm, and input_key_held. */
static u32 key_sent[512];

static u32 repeat_code = 0;
static int repeat_slot = -1;   /* the PHYSICAL key that armed it */
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
        /* Report what went DOWN. `key` here was re-derived from whatever
         * modifiers are held NOW, which is a different key the moment shift,
         * ctrl or caps changed since the press. */
        u32 sent = key_sent[slot] ? key_sent[slot] : key;
        key_sent[slot] = 0;
        /* ...and disarm by the PHYSICAL key, for the same reason. Comparing
         * repeat_code against the re-derived `key` silently fails to match
         * after any modifier change, and the queue then fills with that
         * character forever - the machine has to be reset. */
        if (repeat_slot == slot) { repeat_slot = -1; repeat_code = 0; }
        evq_push(EV_KEY_UP, sent, mods, 0, 0);
        return;
    }

    key_down[slot] = 1;
    key_sent[slot] = key;
    evq_push(EV_KEY_DOWN, key, mods, 0, 0);
    if (ch) evq_push(EV_CHAR, ch, mods, 0, 0);

    /* arm auto-repeat on this key */
    repeat_code = key;
    repeat_slot = slot;
    repeat_mods = mods;
    repeat_at   = idt_ticks() + REPEAT_DELAY;
}

/* ---- the mouse ---------------------------------------------------------
 * EV_MOUSE has been declared since this file was written and NOTHING EVER
 * PUSHED ONE. The keyboard produced proper events while the pointer stayed a
 * pair of globals that callers polled - so "the keyboard and the mouse are two
 * sources of one stream of events", which is this file's opening claim, was
 * only half true. A compositor cannot route what is not in the queue.
 *
 * A position is STATE, not an increment, so many moves between two polls
 * coalesce into one event carrying the latest position. That is correct rather
 * than lossy: replaying intermediate positions would tell a window it had been
 * dragged through places the hand had already left.
 *
 * A BUTTON is not state, and this is the seam. idt.c keeps only the current
 * mask, so a press and release that both land between two polls is already
 * gone before this code runs - no amount of work here recovers it. In practice
 * the pump runs every frame and a human click is 50-100 ms, so it does not
 * bite; the fix, when it is needed, is a latch in the ISR (idt.c) recording
 * "pressed since last read", not anything in this file.
 *
 * The first poll ADOPTS the pointer instead of announcing it. Otherwise the
 * initial 400,300 would arrive as a phantom move on every boot, including the
 * text-mode gate path where there is no pointer at all. */
static int ms_x, ms_y, ms_btn, ms_seen;   /* the last RAW position seen       */
static int px_x, px_y;                    /* the ACCELERATED pointer position */
static int ms_pub_x, ms_pub_y;            /* ...and the last one ANNOUNCED    */

/* ---- pointer speed and acceleration ------------------------------------
 * There was none of this at all. idt.c's IRQ12 handler integrated raw 1:1
 * deltas into a position and everything downstream read that position, so
 * crossing a 2560-wide screen took a physical hand sweep and slow precise
 * movement was exactly as coarse as fast movement. That is the whole of
 * "mouse feel" and none of it existed.
 *
 * IT LIVES HERE AND NOT IN THE ISR, AND THAT IS NOT A STYLE CHOICE. idt.c is
 * built -mgeneral-regs-only so a handler never touches SSE; putting a gain
 * calculation in the IRQ12 path - or calling out from it to code that uses
 * SSE - would corrupt whatever the interrupted code had in XMM. Every zl
 * number is a double, so the interrupted code is usually the interpreter
 * itself. This exact mistake killed the 64-bit boot once already. Everything
 * below is integer, so it could not touch SSE even if it were inlined
 * somewhere it should not be, and the gate checks that by disassembly.
 *
 * WORKING FROM A POSITION, NOT A DELTA. The ISR has already integrated the
 * PS/2 deltas by the time this runs, so the raw delta is recovered here as the
 * difference between consecutive raw positions. That is deliberate: the
 * alternative is publishing a delta accumulator from idt.c, and idt.c has
 * uncommitted changes in another session's worktree right now. This needs no
 * idt.c change at all.
 *
 * The consequence is that a "delta" here is one POLL's worth of movement -
 * roughly one frame - so it is a velocity, which is exactly the right input
 * for an acceleration curve. It also means the curve is frame-rate dependent:
 * under load a frame is longer, the per-frame delta is larger, and the pointer
 * accelerates more. Fixing that needs a clock finer than idt_ticks()' 100 Hz,
 * so it is noted rather than faked.
 */
#define SPD_MIN     25        /* 0.25x - percent, 100 is 1:1 */
#define SPD_MAX    400        /* 4x                          */
#define SPD_UNIT   100

/* The curve, in two segments, as FEEL-PROMPT asks - not a spline.
 * Below the threshold nothing is scaled at all, which is what makes slow
 * precise movement precise; above it the gain climbs linearly to a ceiling. */
#define ACC_THRESH   4        /* deltas at or under this are never accelerated */
#define ACC_GAIN    12        /* extra percent per unit of delta past it       */
#define ACC_MAX    300        /* the ceiling, in percent                       */

static int spd_pct  = SPD_UNIT;
static int accel_on = 1;

/* The clamp. Defaults match idt.c's own 2000x1500 so that at 1x with no
 * acceleration this stage is exactly the identity and the reported position is
 * byte-identical to what the ISR published. fb_setup pushes the real screen
 * size, next to where it pushes the same thing to idt.c. */
static int bnd_w = 2000, bnd_h = 1500;

void input_set_bounds(int w, int h)
{
    if (w > 0) bnd_w = w - 1;
    if (h > 0) bnd_h = h - 1;
}

void input_set_speed(int pct)
{
    if (pct < SPD_MIN) pct = SPD_MIN;
    if (pct > SPD_MAX) pct = SPD_MAX;
    spd_pct = pct;
}

int  input_speed(void)       { return spd_pct; }
void input_set_accel(int on) { accel_on = on ? 1 : 0; }
int  input_accel(void)       { return accel_on; }

/* Where the pointer IS, after scaling.
 *
 * Once acceleration exists this is a genuinely different number from
 * idt_mouse_x(), which is the raw ISR position and is now only an input to
 * this file rather than the answer. Anything asking "where is the pointer"
 * must come here; kernel.zl's mouse_x() builtin still reads the raw one and is
 * therefore unaccelerated - see docs/desktop-feel.md. */
int input_ptr_x(void) { return px_x; }
int input_ptr_y(void) { return px_y; }

/* The gain for a move of magnitude m, in percent. */
static int accel_pct(int m)
{
    if (!accel_on || m <= ACC_THRESH) return SPD_UNIT;
    int g = SPD_UNIT + (m - ACC_THRESH) * ACC_GAIN;
    return g > ACC_MAX ? ACC_MAX : g;
}

/* SUB-UNIT MOVEMENT MUST NOT BE THROWN AWAY. At 50% a delta of 1 scales to 0
 * under integer division, so the pointer would simply never respond to slow
 * one-unit movement - the precise case the curve exists to protect. The
 * remainder is carried instead.
 *
 * Truncation toward zero is symmetric in C, so a negative delta accumulates
 * the same way a positive one does and the pointer does not drift. */
static int rem_x, rem_y;

static int scale_axis(int d, int gain, int *rem)
{
    int t = *rem + d * gain;          /* in units of SPD_UNIT*SPD_UNIT */
    int out = t / (SPD_UNIT * SPD_UNIT);
    *rem = t - out * (SPD_UNIT * SPD_UNIT);
    return out;
}

static void pump_mouse(void)
{
    int x = idt_mouse_x(), y = idt_mouse_y(), b = idt_mouse_btn();
    if (!ms_seen) {
        ms_x = x; ms_y = y; ms_btn = b; ms_seen = 1;
        px_x = x; px_y = y;                 /* adopt, do not announce */
        ms_pub_x = px_x; ms_pub_y = px_y;   /* ...and seed what "announced"
                                               means, or the NEXT poll compares
                                               a real position against 0 and
                                               fires the phantom this adoption
                                               exists to prevent */
        return;
    }

    int dx = x - ms_x, dy = y - ms_y;
    ms_x = x; ms_y = y;

    if (dx | dy) {
        /* ONE gain for both axes, from the magnitude of the move. Deriving it
         * per-axis would give a fast-horizontal, slow-vertical move two
         * different gains and bend the direction the hand actually moved.
         * max + min/2 approximates the hypotenuse to about 12% with no sqrt. */
        int a = dx < 0 ? -dx : dx;
        int c = dy < 0 ? -dy : dy;
        int m = (a > c) ? (a + c / 2) : (c + a / 2);
        int gain = spd_pct * accel_pct(m);

        px_x += scale_axis(dx, gain, &rem_x);
        px_y += scale_axis(dy, gain, &rem_y);

        if (px_x < 0) px_x = 0;
        if (px_y < 0) px_y = 0;
        if (px_x > bnd_w) px_x = bnd_w;
        if (px_y > bnd_h) px_y = bnd_h;
    }

    /* Coalesce on the REPORTED position, not the raw one: below 1x a raw move
     * can scale to nothing at all, and an event that says the pointer is where
     * it already was is a lie the compositor would act on. */
    if (px_x == ms_pub_x && px_y == ms_pub_y && b == ms_btn) return;
    ms_pub_x = px_x; ms_pub_y = px_y; ms_btn = b;
    evq_push(EV_MOUSE, (u32)b, mods, px_x, px_y);
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

    pump_mouse();

    /* USB HID, which already hands us decoded characters */
    for (int i = 0; i < 8; i++) {
        int c = xhci_key();
        if (!c) break;
        evq_push(EV_CHAR, (u32)c, mods, 0, 0);
    }

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
/* For EV_MOUSE: where the pointer was when the event was made, and which
 * buttons were down (input_code carries the mask). Reading idt_mouse_x()
 * instead would give wherever the pointer is NOW, which is a different and
 * usually wrong question once there is a queue between the two. */
int input_x(void)     { return last.x; }
int input_y(void)     { return last.y; }
int input_mods(void)  { return last.mods; }
int input_shift(void) { return (mods & MOD_SHIFT) ? 1 : 0; }
int input_ctrl(void)  { return (mods & MOD_CTRL) ? 1 : 0; }
int input_alt(void)   { return (mods & MOD_ALT) ? 1 : 0; }
int input_caps(void)  { return (mods & MOD_CAPS) ? 1 : 0; }

/* Is this key held? Read what it REPORTED, not what its scancode would mean
 * now. The old version re-derived with mods = 0, so a key pressed while shift
 * was down could never be found: hold Shift+A and ask for 'A' and it derived
 * 'a' and answered no. Same root cause as the stuck repeat, and it is also
 * O(1) work per slot instead of two table lookups. */
int input_key_held(int code)
{
    for (int i = 0; i < 512; i++)
        if (key_down[i] && (int)key_sent[i] == code) return 1;
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
