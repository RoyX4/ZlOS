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

#include "telemetry.h"

extern u32 idt_ticks(void);
extern u32 cpu_tsc_lo(void) __attribute__((weak));
extern int idt_scan(void);        /* raw PS/2 scancode from the IRQ buffer */
extern u32 idt_scan_tsc(void) __attribute__((weak));
extern u32 idt_mouse_take_tsc(void) __attribute__((weak));
extern int xhci_key(void);        /* a decoded character from USB HID      */
extern int idt_mouse_x(void);     /* the PS/2 pointer, published by IRQ12  */
extern int idt_mouse_y(void);
extern int idt_mouse_btn(void);
extern int ser_rx(void);          /* one byte from COM1, or -1 (support.c) */
/* The USB pointer. A tablet reports an ABSOLUTE position, which is why it is
 * preferred over the PS/2 mouse's relative deltas whenever it is present. */
extern int xhci_ptr_ready(void);
extern int xhci_ptr_abs(void);    /* 1 = tablet (a position), 0 = mouse (a delta) */
extern int xhci_ptr_take_wheel(void);/* read-and-clear wheel notches */
extern int xhci_ptr_take_dx(void);   /* read-and-clear raw relative motion */
extern int xhci_ptr_take_dy(void);
extern int xhci_poll(int max);    /* the ONE drainer of the USB event ring */
extern int xhci_ptr_x(void);
extern int xhci_ptr_y(void);
extern int xhci_ptr_btn(void);
/* Ordered USB button states. Optional so input.c's standalone host harnesses
 * do not need the xHCI implementation. */
extern int xhci_ptr_take_button(void) __attribute__((weak));
extern u32 xhci_ptr_take_tsc(void) __attribute__((weak));

/* The internal Synaptics pad is I2C-HID, not PS/2. These are optional so the
 * same input.c remains independently host-testable and QEMU does not need an
 * LPSS controller. The pad contributes relative deltas beside the TrackPoint;
 * it does not replace it. */
extern int i2c_hid_service(void) __attribute__((weak));
extern int i2c_hid_pointer_ready(void) __attribute__((weak));
extern int i2c_hid_ptr_take_dx(void) __attribute__((weak));
extern int i2c_hid_ptr_take_dy(void) __attribute__((weak));
extern int i2c_hid_ptr_take_wheel(void) __attribute__((weak));
extern int i2c_hid_ptr_buttons(void) __attribute__((weak));
extern int i2c_hid_ptr_take_button(void) __attribute__((weak));

extern int xhci_key_event(void);  /* a raw USB HID key event               */
extern u32 xhci_key_event_tsc(void) __attribute__((weak));
extern int xhci_kbd_mods(void);   /* live USB modifier bitmap              */
extern int idt_mouse_wheel(void);  /* read-and-clear notch accumulator */

/* Optional persistent-flight-recorder seam. Host harnesses deliberately link
 * input.c without the recorder, so these stay weak and every call is guarded.
 * The event hook sees LOGICAL queue events, not raw HID completions: mouse
 * motion has already coalesced to one position per poll by the time it gets
 * here. The batch hook is emitted only for an active poll, so an idle desktop
 * cannot fill the journal with 100 identical zero-work records per second. */
extern void zllog_input_batch(unsigned processed, unsigned depth,
                              unsigned drops)
    __attribute__((weak));
extern void zllog_input_event(unsigned type, unsigned code, unsigned depth)
    __attribute__((weak));
extern void zllog_pointer_event(unsigned x, unsigned y, unsigned buttons,
                                unsigned depth, unsigned source_tsc,
                                unsigned sequence) __attribute__((weak));

/* ---- event model ------------------------------------------------------- */
#define EV_NONE      0
#define EV_KEY_DOWN  1
#define EV_KEY_UP    2
#define EV_CHAR      3
#define EV_MOUSE     4
/* A wheel notch. Its own type rather than a bit on EV_MOUSE, because it is the
 * one pointer event with no button and no movement - folding it in would make
 * every existing EV_MOUSE handler have to learn that a "move" it did not ask
 * for might really be a scroll. code carries the SIGNED notch count; x,y are
 * where the pointer was, because scroll goes to what is under the pointer. */
#define EV_WHEEL     5

/* Key codes and modifier bits. Shared with xhci.c, which produces the same
 * codes for USB HID - see keycodes.h for why they are not declared here. */
#include "keycodes.h"

struct event {
    u16 type;
    u16 mods;
    u32 code;      /* a KEY_* value, or the character for EV_CHAR */
    int x, y;      /* mouse only */
    u32 tsc;       /* logical enqueue time, for input-to-present latency */
    u32 seq;
};

#define EVQ_SIZE 64
static struct event evq[EVQ_SIZE];
static int evq_head = 0, evq_tail = 0;
static u32 evq_pushed = 0, evq_dropped = 0;
static u32 evq_sequence = 0;
static u32 evq_source_tsc;

static int evq_depth(void)
{
    int n = evq_tail - evq_head;
    return n < 0 ? n + EVQ_SIZE : n;
}

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

/* which keys are currently held, for repeat and for "is shift down" queries */
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
static u32 key_sent[768];

static u32 repeat_code = 0;
static int repeat_slot = -1;   /* the PHYSICAL key that armed it */
static u32 repeat_at   = 0;
static int repeat_mods = 0;

#define REPEAT_DELAY  50      /* ticks before the first repeat: 500 ms */
#define REPEAT_RATE    3      /* ticks between repeats: ~33/second     */

static void evq_push(int type, u32 code, int m, int x, int y)
{
    int next = (evq_tail + 1) % EVQ_SIZE;
    if (next == evq_head) {
        evq_dropped++;                     /* full: drop, never overwrite */
        zlt_count(ZLLOG_C_INPUT_DROP, 1);
        zlt_event(ZLLOG_SUB_INPUT, ZLLOG_EV_DROP, ZLLOG_ERROR,
                  (unsigned)type, code, (unsigned)evq_depth());
        return;
    }
    evq[evq_tail].type = (u16)type;
    evq[evq_tail].mods = (u16)m;
    evq[evq_tail].code = code;
    evq[evq_tail].x = x;
    evq[evq_tail].y = y;
    evq[evq_tail].tsc = evq_source_tsc ? evq_source_tsc :
                         (cpu_tsc_lo ? cpu_tsc_lo() : 0u);
    evq[evq_tail].seq = ++evq_sequence;
    evq_tail = next;
    evq_pushed++;
    if (type == EV_MOUSE && zllog_pointer_event)
    {
        int at = (evq_tail + EVQ_SIZE - 1) % EVQ_SIZE;
        zllog_pointer_event((unsigned)x, (unsigned)y, code,
                            (unsigned)evq_depth(), evq[at].tsc, evq[at].seq);
    }
    else if (zllog_input_event)
        zllog_input_event((unsigned)type, code, (unsigned)evq_depth());
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

/* SUPER, TAPPED. A modifier produces no event, so a shortcut bound to the
 * modifier alone has nothing to fire on. This latches on the press and is
 * cleared by any other key arriving while it is held, so Super+Tab stays a
 * plain modifier and only a clean press-release emits KEY_SUPER. */
static int super_alone = 0;

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
            if (release) {
                mods &= ~MOD_SUPER;
                if (super_alone) { evq_push(EV_KEY_DOWN, KEY_SUPER, mods, 0, 0); }
                super_alone = 0;
            } else {
                mods |= MOD_SUPER;
                super_alone = 1;
            }
            return;
        }
    }

    /* anything else arriving means Super is being used AS a modifier */
    super_alone = 0;

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
/* A touchpad reports finger displacement, not mouse velocity. It gets the
 * user's speed setting but not the mouse/TrackPoint acceleration curve. The
 * physical first run fed a polled legacy relative report into the shared
 * curve: one stale delta was both repeated and accelerated, which felt like a
 * cursor sliding on ice. Separate remainders also stop sub-pixel movement from
 * one device leaking into the other. */
static int touch_rem_x, touch_rem_y;

static int scale_axis(int d, int gain, int *rem)
{
    int t = *rem + d * gain;          /* in units of SPD_UNIT*SPD_UNIT */
    int out = t / (SPD_UNIT * SPD_UNIT);
    *rem = t - out * (SPD_UNIT * SPD_UNIT);
    return out;
}

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
    /* "IS THERE A USB POINTER" AND "IS IT ABSOLUTE" ARE TWO QUESTIONS, and
     * this asked only the first. `tablet` was xhci_ptr_ready(), so the
     * absolute branch below was taken for every USB pointer there is. A
     * usb-tablet really is absolute, so it looked correct - and every probe in
     * this repo attaches a usb-tablet, so nothing ever disagreed. try.sh
     * attaches a usb-MOUSE, which is relative, and it went down the same
     * branch: every USB mouse ran at exactly 1:1 with no acceleration, and
     * Settings' pointer-speed slider moved a number that reached nothing.
     *
     * xhci_ptr_abs() is the question that was meant. */
    int usb    = xhci_ptr_ready();
    int tablet = usb && xhci_ptr_abs();
    int touch  = i2c_hid_pointer_ready && i2c_hid_pointer_ready();
    int tdx = touch && i2c_hid_ptr_take_dx ? i2c_hid_ptr_take_dx() : 0;
    int tdy = touch && i2c_hid_ptr_take_dy ? i2c_hid_ptr_take_dy() : 0;
    int tbtn = touch && i2c_hid_ptr_buttons ? i2c_hid_ptr_buttons() : 0;
    int b, dx, dy;

    /* NO POLL IN HERE. input_poll() has already drained the ring, once, for
     * both devices - see the note at the top of it. Polling again from inside
     * the mouse pump is what made this the first of two competing drainers,
     * and it capped the pointer at one report per frame. */
    if (usb && !tablet) {
        /* A relative USB mouse hands over a DELTA, read-and-cleared, rather
         * than a position to difference. It has to: xhci.c clamps its own
         * ptr_x to the screen, so two consecutive positions stop differing the
         * moment the pointer sits against an edge - and below 1x the
         * accelerated pointer would then never reach that edge at all. */
        b = xhci_ptr_btn();
        if (!ms_seen) {
            ms_x = xhci_ptr_x(); ms_y = xhci_ptr_y();
            ms_btn = b | tbtn; ms_seen = 1;
            px_x = ms_x; px_y = ms_y;
            ms_pub_x = px_x; ms_pub_y = px_y;
            (void)xhci_ptr_take_dx(); (void)xhci_ptr_take_dy();  /* discard */
            return;
        }
        dx = xhci_ptr_take_dx();
        dy = xhci_ptr_take_dy();
        ms_x += dx; ms_y += dy;
    } else {
        int x, y;
        if (tablet) { x = xhci_ptr_x();  y = xhci_ptr_y();  b = xhci_ptr_btn();  }
        else        { x = idt_mouse_x(); y = idt_mouse_y(); b = idt_mouse_btn(); }
        if (!ms_seen) {
            ms_x = x; ms_y = y; ms_btn = b | tbtn; ms_seen = 1;
            px_x = x; px_y = y;                 /* adopt, do not announce */
            ms_pub_x = px_x; ms_pub_y = px_y;   /* ...and seed what "announced"
                                                   means, or the NEXT poll
                                                   compares a real position
                                                   against 0 and fires the
                                                   phantom this adoption exists
                                                   to prevent */
            return;
        }
        dx = x - ms_x; dy = y - ms_y;
        ms_x = x; ms_y = y;
    }

    /* A TABLET IS ABSOLUTE, so it does not go through the accel curve: its
       report already IS where the pointer should be, and scaling a delta
       derived from two absolute samples would make the pointer lag the pen and
       drift away from it. Speed and acceleration are a mouse's problem - and
       a USB mouse is a mouse, which is the whole point of the split above. */
    if (tablet) {
        px_x = ms_x; px_y = ms_y;
    } else if (dx | dy | tdx | tdy) {
        /* ONE gain for both axes, from the magnitude of the move. Deriving it
         * per-axis would give a fast-horizontal, slow-vertical move two
         * different gains and bend the direction the hand actually moved.
         * max + min/2 approximates the hypotenuse to about 12% with no sqrt. */
        if (dx | dy) {
            int a = dx < 0 ? -dx : dx;
            int c = dy < 0 ? -dy : dy;
            int m = (a > c) ? (a + c / 2) : (c + a / 2);
            int gain = spd_pct * accel_pct(m);
            px_x += scale_axis(dx, gain, &rem_x);
            px_y += scale_axis(dy, gain, &rem_y);
        }
        if (tdx | tdy) {
            int touch_gain = spd_pct * SPD_UNIT; /* 1:1 at default speed */
            px_x += scale_axis(tdx, touch_gain, &touch_rem_x);
            px_y += scale_axis(tdy, touch_gain, &touch_rem_y);
        }

        if (px_x < 0) px_x = 0;
        if (px_y < 0) px_y = 0;
        if (px_x > bnd_w) px_x = bnd_w;
        if (px_y > bnd_h) px_y = bnd_h;
    }

    /* THE WHEEL, before the coalesce test - it is a separate event and must not
     * be swallowed by "the pointer did not move", which is the normal case
     * while scrolling: a hand on a wheel is a hand holding the mouse still. */
    /* BOTH sources, summed. idt_mouse_wheel() is the PS/2 one and was the only
     * one; xhci.c decodes the USB wheel byte now, and every probe in this repo
     * (and a real laptop) drives a USB pointer, so on those the wheel used to
     * be a control that existed and never fired. Read both unconditionally -
     * take_wheel() is read-and-clear, so skipping it when the PS/2 mouse
     * answered first would let notches pile up and arrive in a burst. */
    int wz = idt_mouse_wheel() + (usb ? xhci_ptr_take_wheel() : 0) +
             (touch && i2c_hid_ptr_take_wheel ? i2c_hid_ptr_take_wheel() : 0);
    if (wz) evq_push(EV_WHEEL, (u32)wz, mods, px_x, px_y);

    /* Motion is state and may coalesce. Button transitions are history and
     * may not: a press+release can both arrive during one slow frame. xHCI
     * retains the ordered masks per HID report so both edges survive here. */
    if (usb && xhci_ptr_take_button) {
        int edge;
        while ((edge = xhci_ptr_take_button()) >= 0) {
            ms_btn = edge | tbtn;
            ms_pub_x = px_x; ms_pub_y = px_y;
            evq_push(EV_MOUSE, (u32)ms_btn, mods, px_x, px_y);
        }
    }
    if (touch && i2c_hid_ptr_take_button) {
        int edge;
        while ((edge = i2c_hid_ptr_take_button()) >= 0) {
            ms_btn = b | edge;
            ms_pub_x = px_x; ms_pub_y = px_y;
            evq_push(EV_MOUSE, (u32)ms_btn, mods, px_x, px_y);
        }
    }

    b |= tbtn;

    /* Coalesce on the REPORTED position, not the raw one: below 1x a raw move
     * can scale to nothing at all, and an event that says the pointer is where
     * it already was is a lie the compositor would act on. */
    if (px_x == ms_pub_x && px_y == ms_pub_y && b == ms_btn) return;
    ms_pub_x = px_x; ms_pub_y = px_y; ms_btn = b;
    evq_push(EV_MOUSE, (u32)b, mods, px_x, px_y);
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
        /* Report what the key SENT when it went down, not what it would derive
         * now - the modifiers may have changed in between, which is what
         * key_sent exists for. handle_scancode does the same for PS/2; without
         * this the USB half of input_key_held() compares against 0 and every
         * held USB key reads as not held. */
        u32 sent = key_sent[slot] ? key_sent[slot] : key;
        key_down[slot] = 0;
        key_sent[slot] = 0;
        if (repeat_code == sent) repeat_code = 0;
        evq_push(EV_KEY_UP, sent, m, 0, 0);
        return;
    }

    key_down[slot] = 1;
    key_sent[slot] = key;
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
    /* Unsigned subtraction deliberately handles a years-long counter wrap.
     * `processed` below means logical events successfully queued this poll;
     * drops is the number refused because the queue was full. */
    u32 pushed_before = evq_pushed;
    u32 dropped_before = evq_dropped;

    /* THE USB EVENT RING, DRAINED ONCE, FIRST, BY ONE CALLER.
     *
     * Both HID devices post completions to a single xHCI event ring. This used
     * to be drained from two places at two different rates - pump_mouse()
     * once, the keyboard loop below up to sixteen times - and whichever ran
     * first took whatever happened to be at the front. That is why the pointer
     * was jumpy: pump_mouse() got at most one report per frame, and sometimes
     * a keystroke instead of one.
     *
     * Draining here, before pump_mouse() reads the pointer and before the
     * keyboard loop pops its queue, means both see everything that arrived
     * since the last frame. The bound is generous on purpose - a hand moving
     * fast produces a report per USB service interval, several per frame, and
     * the surplus must not be left on the ring until the next one. */
    xhci_poll(32);

    /* PS/2 */
    for (int i = 0; i < 16; i++) {
        int sc = idt_scan();
        if (!sc) break;
        evq_source_tsc = idt_scan_tsc ? idt_scan_tsc() : 0u;
        handle_scancode(sc);
        evq_source_tsc = 0;
    }

    /* Automatic internal-pad startup is delayed and state-driven inside the
     * driver. On QEMU/other hardware the weak/exact probe is a quick no-op. */
    if (i2c_hid_service) (void)i2c_hid_service();

    evq_source_tsc = xhci_ptr_take_tsc && xhci_ptr_ready()
        ? xhci_ptr_take_tsc()
        : (idt_mouse_take_tsc ? idt_mouse_take_tsc() : 0u);
    pump_mouse();
    evq_source_tsc = 0;

    /* USB HID. It used to hand over decoded characters, which is why arrow
     * keys never reached an application from a USB keyboard: there is no
     * character for Up, so it decoded to 0 and 0 means "nothing typed". It
     * hands over raw HID events now and this file translates them. */
    for (int i = 0; i < 16; i++) {
        int ev = xhci_key_event();
        if (!ev) break;
        evq_source_tsc = xhci_key_event_tsc ? xhci_key_event_tsc() : 0u;
        handle_hid_event(ev);
        evq_source_tsc = 0;
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
        evq_source_tsc = 0;
        evq_push(EV_CHAR, (u32)c, mods, 0, 0);
    }
    /* The modifier bitmap is whatever the last report decoded, and the drain
     * that decoded it ran at the top of this function - so by here it is
     * current. (It used to matter that this came AFTER the keyboard loop,
     * because xhci_key_event() did the polling. It no longer does.) */
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

    {
        u32 processed = evq_pushed - pushed_before;
        u32 drops = evq_dropped - dropped_before;
        if ((processed || drops) && zllog_input_batch)
            zllog_input_batch(processed, (unsigned)evq_depth(), drops);
        if (processed || drops) zlt_observe(ZLLOG_C_INPUT_QUEUE,
                                             (unsigned)evq_depth());
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
u32 input_event_tsc(void) { return last.tsc; }
u32 input_event_seq(void) { return last.seq; }
/* Either keyboard holding shift means shift is down - they are one logical
 * keyboard, and the caller has no business knowing which one you reached for. */
int input_shift(void) { return ((mods | usb_mods) & MOD_SHIFT) ? 1 : 0; }
int input_ctrl(void)  { return ((mods | usb_mods) & MOD_CTRL) ? 1 : 0; }
int input_alt(void)   { return ((mods | usb_mods) & MOD_ALT) ? 1 : 0; }
int input_caps(void)  { return (mods & MOD_CAPS) ? 1 : 0; }   /* a latch, not held */

/* Is this key held? Read what it REPORTED, not what its scancode would mean
 * now. The old version re-derived with mods = 0, so a key pressed while shift
 * was down could never be found: hold Shift+A and ask for 'A' and it derived
 * 'a' and answered no. Same root cause as the stuck repeat, and it is also
 * O(1) work per slot instead of two table lookups. */
int input_key_held(int code)
{
    for (int i = 0; i < 768; i++)
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
    return evq_depth();
}
