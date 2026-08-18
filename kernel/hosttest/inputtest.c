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
void input_set_speed(int pct);
int  input_speed(void);
void input_set_accel(int on);
int  input_accel(void);
void input_set_bounds(int w, int h);
int  input_ptr_x(void);
int  input_ptr_y(void);

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

    /* Tests 1-8 are about QUEUE PLUMBING - phantom events, coalescing, the
     * character path - and every one of them predates pointer acceleration.
     * They assert exact positions, which only means anything at a gain of
     * exactly 1. Acceleration now ships ON by default (it is the feature; a
     * defect that persists until someone finds a toggle is not fixed), so
     * pin the gain to the identity here and let the acceleration section
     * below turn it on deliberately.
     *
     * This is not weakening them. It is the difference between "does the
     * queue behave" and "does the curve behave", which are two questions, and
     * running the plumbing tests under a nonlinear transform would only make
     * them fail for a reason they were never asking about. */
    input_set_speed(100);
    input_set_accel(0);

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

    /* ---- pointer speed and acceleration ---------------------------------
     * There was none of this at all: raw 1:1 deltas, so crossing a 2560-wide
     * screen took a physical hand sweep and slow precise movement was exactly
     * as coarse as fast movement.
     *
     * Every one of these failures is invisible from a screenshot, and most are
     * invisible from using it too - "the pointer feels slightly wrong" is not
     * a bug report anyone can act on. So: numbers.
     */
    input_set_bounds(4000, 3000);          /* out of the way of the clamp tests */

    /* MEASURE AGAINST THE ACCELERATED POSITION, NOT THE RAW ONE.
     *
     * input.c integrates SCALED deltas, so once any gain other than 1 has been
     * applied its pointer carries a permanent offset from the ISR's raw
     * position - and that is correct, not a bug: the alternative is the
     * pointer teleporting whenever the speed setting changes. It does mean a
     * test that compares d.last_x against fake_x is measuring the offset, not
     * the gain. Every check below takes a before/after of input_ptr_x(). */
    #define MOVED(axis, by) ({                                        \
        int _b = input_ptr_##axis(); fake_##axis += (by);             \
        input_poll(); drain(); input_ptr_##axis() - _b; })

    /* THE IDENTITY. At 1x with the curve off, this stage must reproduce the
     * ISR's own deltas exactly - not approximately. Anything else means the
     * default install has quietly changed how the mouse behaves. Both signs,
     * and zero, because a remainder bug shows up as asymmetric drift. */
    input_set_speed(100);
    input_set_accel(0);
    fake_x = 500; fake_y = 400; drain();
    int worst = 0;
    for (int i = 0; i < 200; i++) {
        int step = (i % 7) - 3;            /* -3..3, both signs, including 0 */
        int got = MOVED(x, step);
        int err = got - step;
        if (err < 0) err = -err;
        if (err > worst) worst = err;
    }
    ok("at 1x with accel off every delta is reproduced exactly", worst == 0);

    /* SMALL DELTAS ARE NEVER TOUCHED BY THE CURVE. This is the entire reason
     * it has two segments: a pointer you cannot place on a 1px window border
     * is worse than a slow one. Checked at every speed, because it is the
     * SPEED multiplier that would break it if the threshold were tested
     * against the scaled delta instead of the raw one. */
    input_set_accel(1);
    int small_ok = 1;
    for (int pct = 25; pct <= 400; pct += 25) {
        input_set_speed(pct);
        drain();
        /* ten 1-unit steps: the curve must contribute nothing, so the total is
         * the speed multiplier alone. One unit of slack for the remainder
         * carried in from the previous setting. */
        int b = input_ptr_x();
        for (int i = 0; i < 10; i++) { fake_x += 1; input_poll(); }
        drain();
        int got = input_ptr_x() - b, want = 10 * pct / 100;
        if (got < want - 1 || got > want + 1) small_ok = 0;
    }
    ok("a 1-unit delta is never accelerated, at any speed", small_ok);

    /* A LARGE DELTA AT 2x MOVES TWICE AS FAR. The headline property, and the
     * one a user would actually notice. Curve off, so this measures the speed
     * multiplier alone rather than speed times an accidental gain. */
    input_set_accel(0);
    input_set_speed(100);
    drain();
    int at1x = MOVED(x, 100);
    input_set_speed(200);
    drain();
    int at2x = MOVED(x, 100);
    ok("a 100-unit delta moves 100 at 1x", at1x == 100);
    ok("...and exactly 200 at 2x", at2x == 200);

    /* SUB-UNIT MOVEMENT IS CARRIED, NOT DISCARDED. At 50% a 1-unit delta
     * truncates to zero, so without a remainder the pointer would simply never
     * respond to slow movement - and slow movement is the case the whole
     * two-segment curve exists to protect. */
    input_set_speed(50);
    drain();
    int b50 = input_ptr_x();
    for (int i = 0; i < 10; i++) { fake_x += 1; input_poll(); }
    drain();
    ok("at 0.5x ten 1-unit steps move 5, not 0 - the remainder is carried",
       input_ptr_x() - b50 == 5);

    /* ...and symmetrically downward, or the pointer drifts one way over time.
     * C truncates toward zero, which is what makes this hold for both signs;
     * a shift would floor instead and the pointer would creep left forever. */
    for (int i = 0; i < 10; i++) { fake_x -= 1; input_poll(); }
    drain();
    ok("...and back to exactly where it started going the other way",
       input_ptr_x() == b50);

    /* THE CURVE ACTUALLY BENDS. A "two-segment curve" whose second segment is
     * flat is a speed multiplier with extra steps. */
    input_set_speed(100);
    input_set_accel(1);
    drain();
    int slow3  = MOVED(x, 3);
    int fast60 = MOVED(x, 60);
    int fast600 = MOVED(x, 600);
    ok("a 3-unit move is unaccelerated", slow3 == 3);
    ok("...a 60-unit move travels further than 60", fast60 > 60);
    /* THE CAP EXISTS - asserted without naming its value. Mirroring input.c's
     * ACC_MAX here would be a constant in two places that can drift, and a
     * gate agreeing with a stale copy of the thing it checks is bug class 6.
     * Instead: past the ceiling the gain no longer depends on the delta, so a
     * 10x bigger move travels exactly 10x further. An uncapped linear curve
     * would make it 10x further TIMES a 10x gain. */
    ok("...and the curve SATURATES rather than running away",
       fast600 == fast60 * 10);

    /* Turning it off must actually turn it off. */
    input_set_accel(0);
    drain();
    ok("accel off returns the same 60 units", MOVED(x, 60) == 60);

    /* THE POINTER STILL CANNOT LEAVE THE SCREEN. Acceleration multiplies the
     * distance travelled, so a clamp adequate at 1:1 is not automatically
     * adequate at 4x - and off the right-hand edge the pointer is unreachable
     * and the machine looks hung. Checked on the reported position AND on the
     * internal one, because clamping only on the way out would let the
     * internal position run away and then take hundreds of events to come
     * back - the pointer would "stick" to the edge. */
    input_set_bounds(1280, 800);
    input_set_speed(400);
    input_set_accel(1);
    int escaped = 0, stuck = 0;
    fake_x = 100; fake_y = 100; drain();
    for (int i = 0; i < 400; i++) {
        fake_x += 40; fake_y += 40;         /* hard right-and-down, fast */
        input_poll();
        d = drain();
        if (input_ptr_x() < 0 || input_ptr_x() > 1279 ||
            input_ptr_y() < 0 || input_ptr_y() > 799) escaped = 1;
        if (d.mouse && (d.last_x < 0 || d.last_x > 1279 ||
                        d.last_y < 0 || d.last_y > 799)) escaped = 1;
    }
    /* one move back off the edge must move it immediately, not after the
     * internal position unwinds some invisible overshoot */
    fake_x -= 40; fake_y -= 40;
    input_poll(); drain();
    if (input_ptr_x() > 1279 - 40) stuck = 1;
    for (int i = 0; i < 400; i++) {
        fake_x -= 40; fake_y -= 40;         /* ...and hard back */
        input_poll();
        d = drain();
        if (input_ptr_x() < 0 || input_ptr_x() > 1279 ||
            input_ptr_y() < 0 || input_ptr_y() > 799) escaped = 1;
    }
    ok("at 4x with accel the pointer never leaves the screen", !escaped);
    ok("...and comes straight back off the edge, not after an overshoot",
       !stuck);

    /* leave the module the way the kernel boots it */
    input_set_speed(100);
    input_set_accel(1);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
