/* inputtest.c - drive the REAL input.c from Linux, with fake hardware.
 *
 * Same trick as fbbench does for fb.c: input.c talks to exactly four functions
 * outside itself - idt_scan, idt_ticks, xhci_key and the three idt_mouse_*
 * readers - so stubbing those turns the whole event stack into an ordinary
 * program that can be asserted against in milliseconds instead of a boot.
 *
 * This compiles the EXACT input.c that ships in the kernel. If it drifts from
 * what boots, that is a bug in this file, not a licence to patch a copy.
 *
 * It exists because desktop-TODO 0d - "push EV_MOUSE from the same pump" - is
 * the kind of change whose failures are all invisible from a screenshot: a
 * phantom event at boot, a flood of duplicates, a coalesce that swallows a
 * button, an event carrying the pointer's CURRENT position rather than the one
 * it had when the event was made. None of those look like anything.
 *
 * Build and run:  ./build.sh && ./inputtest
 */
#include <stdio.h>
#include <string.h>

/* ---- input.c's public surface ------------------------------------------ */
void input_poll(void);
int  input_next(void);
int  input_type(void);
int  input_code(void);
int  input_x(void);
int  input_y(void);
int  input_char(void);
int  input_key(void);
int  input_queued(void);

#define EV_NONE     0
#define EV_KEY_DOWN 1
#define EV_KEY_UP   2
#define EV_CHAR     3
#define EV_MOUSE    4

/* ---- the fake hardware -------------------------------------------------- */
static int fake_x = 400, fake_y = 300, fake_btn = 0;
static unsigned fake_ticks = 0;
static int scan_q[64], scan_head, scan_tail;

int idt_mouse_x(void)   { return fake_x; }
int idt_mouse_y(void)   { return fake_y; }
int idt_mouse_btn(void) { return fake_btn; }
unsigned int idt_ticks(void) { return fake_ticks; }
int xhci_key(void)      { return 0; }

int idt_scan(void)
{
    if (scan_head == scan_tail) return 0;
    int v = scan_q[scan_head];
    scan_head = (scan_head + 1) % 64;
    return v;
}

static void send_scan(int sc)
{
    scan_q[scan_tail] = sc;
    scan_tail = (scan_tail + 1) % 64;
}

/* COM1, the third source. -1 means "no byte", and it is ALSO what a machine
 * with no UART answers forever - which is the case this stub can express and a
 * real port cannot. */
static int ser_q[64], ser_head, ser_tail;

int ser_rx(void)
{
    if (ser_head == ser_tail) return -1;
    int v = ser_q[ser_head];
    ser_head = (ser_head + 1) % 64;
    return v;
}

static int fake_usb_ptr = 0, fake_ux = 0, fake_uy = 0, fake_ubtn = 0;

/* No USB pointer in the harness - which is a case worth being able to express,
 * because it is the PS/2 fallback the laptop's TrackPoint takes. */
int xhci_ptr_ready(void) { return fake_usb_ptr; }
int xhci_ptr_poll(void)  { return 0; }
int xhci_ptr_x(void)     { return fake_ux; }
int xhci_ptr_y(void)     { return fake_uy; }
int xhci_ptr_btn(void)   { return fake_ubtn; }

static void send_ser(const char *s)
{
    while (*s) {
        ser_q[ser_tail] = (unsigned char)*s++;
        ser_tail = (ser_tail + 1) % 64;
    }
}

/* ---- assertions --------------------------------------------------------- */
static int fails;

static void ok(const char *what, int cond)
{
    printf("  %-52s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}

/* drain the queue, counting what came out */
struct drained { int n, mouse, chars, keydown; int last_x, last_y, last_btn; };

static struct drained drain(void)
{
    struct drained d;
    memset(&d, 0, sizeof d);
    for (int guard = 0; guard < 256; guard++) {
        int t = input_next();
        if (!t) break;
        d.n++;
        if (t == EV_MOUSE) {
            d.mouse++;
            d.last_x = input_x();
            d.last_y = input_y();
            d.last_btn = input_code();
        } else if (t == EV_CHAR)     d.chars++;
        else if (t == EV_KEY_DOWN)   d.keydown++;
    }
    return d;
}

int main(void)
{
    printf("inputtest - the shipping input.c, against fake hardware\n\n");

    /* 1. The first poll must ADOPT the pointer, not announce it. idt.c starts
     *    the pointer at 400,300; if that arrives as an event, every boot
     *    begins with a phantom mouse move - including the text-mode gate path
     *    where there is no pointer at all. */
    struct drained d = drain();
    ok("first poll emits no phantom mouse event", d.mouse == 0);

    /* 2. Nothing moved: nothing to say. */
    d = drain();
    ok("an unchanged pointer produces no events", d.mouse == 0);

    /* 3. A move produces exactly one event, carrying the new position. */
    fake_x = 640; fake_y = 480;
    d = drain();
    ok("a move produces exactly one EV_MOUSE", d.mouse == 1);
    ok("...carrying the new position", d.last_x == 640 && d.last_y == 480);

    /* 4. Many moves between two drains COALESCE. A position is state, not an
     *    increment: replaying the intermediate ones would tell a window it had
     *    been dragged through places the hand had already left. */
    for (int i = 0; i < 20; i++) { fake_x++; fake_y++; input_poll(); }
    d = drain();
    ok("20 moves in one drain do not flood the queue", d.mouse <= 20 && d.mouse > 0);
    ok("...and the last one wins", d.last_x == 660 && d.last_y == 500);

    /* 5. A button change is an event even when the pointer has not moved -
     *    a click without a wobble is the normal case, not an edge case. */
    fake_btn = 1;
    d = drain();
    ok("a button press with no movement still reports", d.mouse == 1);
    ok("...with the button mask in the code field", d.last_btn == 1);
    fake_btn = 0;
    d = drain();
    ok("the release reports too", d.mouse == 1 && d.last_btn == 0);

    /* 6. The character path must be untouched by all of this. 0x1E/0x9E is
     *    'a' down and up in scancode set 1. This is the actual gate in
     *    desktop-TODO 0d: the demos that read characters still work. */
    fake_x += 7;                      /* mouse traffic in the same drain */
    send_scan(0x1E);
    send_scan(0x9E);
    int c = input_char();
    ok("input_char still returns a typed character past mouse traffic", c == 'a');

    /* 7. And a navigation key, which takes the other branch. 0xE0 0x4B is
     *    left-arrow; KEY_LEFT is 0x110. */
    fake_x += 3;
    send_scan(0xE0);
    send_scan(0x4B);
    int k = input_key();
    ok("input_key still returns a navigation key past mouse traffic", k == 0x110);

    /* 8. A pointer parked against the edge - the clamp holds it at a constant
     *    value - must not generate an event per poll forever. */
    fake_x = 0; fake_y = 0;
    drain();
    for (int i = 0; i < 50; i++) input_poll();
    d = drain();
    ok("a parked pointer is silent, not a per-poll event", d.mouse == 0);

    /* 9. SERIAL, the third source. This is not a nicety: once wm_frame() owns
     *    the screen, apps read keys from this queue and from nowhere else, so
     *    a serial byte that never enters it is a byte the desktop can never
     *    see - and verify.sh and every probe-*.py drive this machine by typing
     *    down that wire. */
    drain();
    send_ser("help\r");
    d = drain();
    ok("serial bytes arrive as characters", d.chars == 5);

    /* ...in the order they were sent. A queue exists to preserve that, and it
     *    is the reason zl's shell no longer reads COM1 itself: two readers of
     *    one FIFO hand out a newly-arrived byte before a queued one. */
    drain();
    send_ser("abc");
    ok("...in order", input_char() == 'a' && input_char() == 'b'
                       && input_char() == 'c');

    /* 10. NO key event beside the character, unlike PS/2 and USB. A serial
     *     byte has no press, no release and no modifier state - it is already
     *     the character. Synthesising a key would silently change an existing
     *     behaviour: input_key() prefers a queued key, so a serial ESC would
     *     start arriving as KEY_ESC (0x101) where the editor has always seen
     *     27, and that is exactly the sort of change nothing would notice. */
    drain();
    send_ser("\033");
    d = drain();
    ok("serial emits no key event beside the character", d.keydown == 0);
    drain();
    send_ser("\033");
    ok("...so input_key() still sees a serial ESC as 27", input_key() == 27);

    /* 11. An absent UART must be silent. There is no serial port on the
     *     ThinkPad; an undecoded port floats high, so the naive "is LSR bit 0
     *     set" is true forever and RBR reads 0xFF forever. That would be an
     *     endless stream of phantom keystrokes on the one machine that cannot
     *     be debugged over serial. support.c's ser_rx() answers -1 there, and
     *     an empty queue here is that same answer. */
    drain();
    for (int i = 0; i < 50; i++) input_poll();
    d = drain();
    ok("a silent UART produces no events at all", d.chars == 0 && d.n == 0);

    /* 12. THE USB POINTER MUST WIN. There are two pointers - an absolute
     *     usb-tablet through xhci.c and a relative PS/2 mouse through idt.c -
     *     and mouse_x() has preferred the tablet since it was written. This
     *     pump read the PS/2 one and nothing else, which was invisible while
     *     the shell owned the screen and total the moment wm_frame() did: the
     *     compositor's only source of pointer events is this queue, so on any
     *     machine with a tablet attached - which is what QEMU gives - NO mouse
     *     event was ever pushed. No dragging, no clicking, no dock, no menu.
     *
     *     Every gate in this repo types, so none of them could see it. This
     *     assertion is the one that can. */
    drain();
    fake_usb_ptr = 1;
    fake_ux = 1000; fake_uy = 700; fake_ubtn = 0;
    d = drain();
    /* A source switch REPORTS, unlike the very first poll, and that is
     * correct rather than a lapse: the pointer really is somewhere else now,
     * and swallowing it would leave every window believing the old position
     * until the hand happened to move again. */
    ok("plugging in a USB pointer reports the new position",
       d.mouse == 1 && d.last_x == 1000 && d.last_y == 700);
    fake_ux = 1100;
    d = drain();
    ok("the USB pointer produces events", d.mouse == 1);
    ok("...and its position is the one reported",
       d.last_x == 1100 && d.last_y == 700);

    /* ...and the PS/2 mouse must NOT be able to talk over it. Both moving at
     * once has to resolve to one answer, or the pointer jumps between them. */
    fake_x = 5; fake_y = 5;
    fake_ux = 1200;
    d = drain();
    ok("the PS/2 mouse cannot talk over it", d.last_x == 1200 && d.last_y == 700);

    /* and unplugging it falls back, which is the laptop's TrackPoint */
    fake_usb_ptr = 0;
    fake_x = 300; fake_y = 200;
    d = drain();
    ok("without a USB pointer, PS/2 is used", d.last_x == 300 && d.last_y == 200);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
