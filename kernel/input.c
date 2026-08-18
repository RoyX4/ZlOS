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
extern int ser_rx(void);          /* one byte from COM1, or -1 (support.c) */
/* The USB pointer. A tablet reports an ABSOLUTE position, which is why it is
 * preferred over the PS/2 mouse's relative deltas whenever it is present. */
extern int xhci_ptr_ready(void);
extern int xhci_ptr_poll(void);
extern int xhci_ptr_x(void);
extern int xhci_ptr_y(void);
extern int xhci_ptr_btn(void);

/* ---- event model ------------------------------------------------------- */
#define EV_NONE      0
#define EV_KEY_DOWN  1
#define EV_KEY_UP    2
#define EV_CHAR      3
#define EV_MOUSE     4

/* Key codes and modifier bits. Shared with xhci.c, which produces the same
 * codes for USB HID - see keycodes.h for why they are not declared here. */
#include "keycodes.h"

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
static int ms_x, ms_y, ms_btn, ms_seen;

/* THERE ARE TWO POINTERS AND THIS READ THE WRONG ONE.
 *
 * zlOS drives both: xhci.c an absolute usb-tablet, idt.c a relative PS/2
 * mouse. The `mouse_x` builtin has preferred the tablet since it was written -
 * a tablet cannot drift, and on a UEFI laptop the PS/2 emulation dies with
 * ExitBootServices. This function read idt_mouse_x() and nothing else.
 *
 * While the shell owned the screen that was invisible, because the shell read
 * mouse_x() directly and got the right answer. The moment wm_frame() became
 * the top of the system, the compositor's ONLY source of pointer events was
 * this queue - so on any machine with a usb-tablet, which is what QEMU gives
 * and what try.sh attaches, no EV_MOUSE was ever pushed at all. No dragging,
 * no clicking, no dock, no menu. The whole pointer half of the desktop.
 *
 * No gate caught it and none could have: every gate in this repo drives zlOS
 * by TYPING, and a dock that does nothing photographs identically to one that
 * works. probe-dock.py exists because of this.
 *
 * One rule, one place: prefer the tablet, exactly as the builtin does. */
static void pump_mouse(void)
{
    int x, y, b;
    if (xhci_ptr_ready()) {
        xhci_ptr_poll();                 /* the report is pulled, not pushed */
        x = xhci_ptr_x(); y = xhci_ptr_y(); b = xhci_ptr_btn();
    } else {
        x = idt_mouse_x(); y = idt_mouse_y(); b = idt_mouse_btn();
    }
    if (!ms_seen) { ms_x = x; ms_y = y; ms_btn = b; ms_seen = 1; return; }
    if (x == ms_x && y == ms_y && b == ms_btn) return;
    ms_x = x; ms_y = y; ms_btn = b;
    evq_push(EV_MOUSE, (u32)b, mods, x, y);
}

/* The key a character came from. Printable keys ARE their unshifted ASCII by
 * the convention in keycodes.h, so this only has to name the four control
 * characters that have a key code, and fold shifted letters back to the
 * unshifted key - 'A' and 'a' are one key, and a consumer watching for
 * KEY_DOWN 'a' should see it whichever was typed. */
static u32 key_of_char(int c)
{
    switch (c) {
        case 27: return KEY_ESC;
        case  8: return KEY_BACKSPACE;
        case  9: return KEY_TAB;
        case 13: return KEY_ENTER;
    }
    if (c >= 'A' && c <= 'Z') return (u32)(c + 32);
    return (u32)c;
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

    /* USB HID.
     *
     * This used to push EV_CHAR and nothing else, which made the USB keyboard
     * a second-class source: every consumer that waits for a KEY rather than a
     * CHARACTER was deaf to it. The `=` demo in kernel.zl exits on EV_KEY_DOWN
     * with KEY_ESC, so with a USB keyboard ESC did nothing at all and the demo
     * could only end by timing out - and the sweep scored that "ok", because
     * from outside, timing out and exiting look identical.
     *
     * PS/2 emits the key first and the character second (line 230), so this
     * does the same. Codes at or above KEY_NONCHAR - the arrows, Home/End, the
     * function keys - have no character and emit only the key event. */
    for (int i = 0; i < 8; i++) {
        int c = xhci_key();
        if (!c) break;
        if (c >= KEY_NONCHAR) {
            evq_push(EV_KEY_DOWN, (u32)c, mods, 0, 0);
        } else {
            evq_push(EV_KEY_DOWN, key_of_char(c), mods, 0, 0);
            evq_push(EV_CHAR, (u32)c, mods, 0, 0);
        }
    }

    /* SERIAL, the third source.
     *
     * A terminal on the other end of COM1 is a keyboard as far as this queue is
     * concerned, and it has to be: once wm_frame() owns the screen, apps get
     * keys from here and from nowhere else, so a byte that only zl's old shell
     * loop could read is a byte the desktop can never see. verify.sh and every
     * probe-*.py drive this machine down that wire.
     *
     * EV_CHAR ONLY, DELIBERATELY - no EV_KEY_DOWN beside it, unlike PS/2 and
     * USB. A serial byte carries no press/release and no modifier state; it is
     * already the character. Synthesising a key event would also CHANGE an
     * existing behaviour: input_key() returns the key when one is queued, so a
     * serial ESC would start arriving as KEY_ESC (0x101) where the editor has
     * always seen 27. Pushing the character alone leaves every existing
     * consumer byte-for-byte where it was and adds the compositor as a new one.
     *
     * Bounded at 16 per poll for the same reason as the PS/2 drain: a fast
     * sender must not be able to hold this loop. ser_rx() answers -1 when the
     * machine has no UART, which is the ThinkPad - see support.c for why that
     * check is not optional. */
    for (int i = 0; i < 16; i++) {
        int c = ser_rx();
        if (c < 0) break;
        if (c == 0) continue;                    /* a NUL is not a keystroke */
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

int input_key_held(int code)
{
    for (int i = 0; i < 512; i++) {
        if (!key_down[i]) continue;
        u32 k = (i >= 0x100) ? sc_extended(i - 0x100) : sc_special(i);
        if (!k) k = to_char(i, 0);
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
