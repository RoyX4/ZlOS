/* xhcitest.c - the xHCI EVENT RING, and the input stack on top of it, against
 * a controller made of memory.
 *
 * WHY THIS EXISTS
 * ---------------
 * The pointer was visibly broken after the eleven-track merge - jumpy, laggy,
 * unpredictable under the hand - and every headless probe in this repo passed
 * while it was. They all pass because they move the pointer in ONE LARGE JUMP
 * and assert on where it landed, which is precisely the motion pattern that
 * hides a drain-rate bug: one report carries the whole move, so one drain per
 * frame is enough. A hand does not move like that. A hand produces a report
 * every service interval for as long as it is moving, and the question that
 * matters is how many of those the driver takes off the ring per frame.
 *
 * That question cannot be asked of input.c alone. inputtest.c stubs
 * xhci_ptr_poll() as `return 0` and xhci_ptr_x() as "whatever the test set",
 * so the event ring - the thing that is actually wrong - is not in the picture
 * at all. It has to be asked of the REAL xhci.c, and that means being the
 * controller.
 *
 * SO THIS HARNESS IS THE HARDWARE
 * -------------------------------
 * xhci.c reaches outside itself for exactly eleven functions (eight PCI, two
 * console size, one tick), and every data structure it shares with the
 * controller is plain memory at a fixed address. So the whole driver compiles
 * on Linux unmodified - the same trick fbbench plays on fb.c - and this file
 * plays the other side of the DMA:
 *
 *     ctl_service_ep()   walks the endpoint's TRANSFER ring the way silicon
 *                        does, honouring the cycle bit and the Link TRB,
 *                        writes a report into the buffer THAT TRB NAMES, and
 *                        posts a Transfer Event on the event ring
 *     ctl_post_event()   produces on the event ring with its own cycle state
 *
 * Reading the buffer pointer out of the TRB rather than hardcoding PTR_REPORT
 * is the whole point: it means the harness stays honest if the driver is
 * changed to post several buffers, and it is what makes "all the TRBs point at
 * one buffer" show up as a wrong answer instead of a comment nobody reads.
 *
 * The device is QEMU's usb-mouse, modelled from hw/input/hid.c rather than
 * guessed at. Two properties matter and both are load-bearing here:
 *
 *   1. it ACCUMULATES undelivered motion and never drops it - hid_pointer_sync
 *      merges a new event into the previous unread one when the buttons have
 *      not changed, and when its queue is full it returns without clearing the
 *      pending slot, so the deltas keep piling into it
 *   2. it delivers at most 127 counts per report and SUBTRACTS what it sent
 *      (hid_pointer_poll: `dx = int_clamp(e->xdx, -127, 127); e->xdx -= dx;`),
 *      keeping the remainder for the next one
 *
 * Together those mean nothing is ever lost on the device side, and therefore
 * that the guest's maximum pointer speed is exactly
 *
 *          127 counts  x  (reports the driver takes per second)
 *
 * which is a number this file can measure.
 *
 * Build and run:  ./build.sh && ./xhcitest
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>

/* ---- the eleven functions xhci.c reaches outside itself ------------------
 * There is no PCI here: the harness assigns the register block directly, so
 * xhci_find() is never called and these exist only to let the file link. */
void pci_scan(void)            { }
int  pci_count(void)           { return 0; }
int  pci_class(int i)          { (void)i; return 0; }
int  pci_subclass(int i)       { (void)i; return 0; }
int  pci_prog_if(int i)        { (void)i; return 0; }
void pci_enable(int i)         { (void)i; }
unsigned int pci_bar(int i, int w)    { (void)i; (void)w; return 0; }
unsigned int pci_bar_hi(int i, int w) { (void)i; (void)w; return 0; }

/* A screen far larger than any case needs. ptr_decode() clamps to the console
 * size and input.c clamps to the bounds fb.c pushes; a case that runs into
 * either measures the clamp instead of the thing it is asking about. The
 * clamps have their own coverage in inputtest_feel.c. */
static int fake_pxw = 60000, fake_pxh = 60000;
#define HOME_X 1000
#define HOME_Y 1000
int console_pxw(void) { return fake_pxw; }
int console_pxh(void) { return fake_pxh; }

static unsigned fake_ticks = 0;
unsigned int idt_ticks(void) { return fake_ticks; }

/* THE DRIVER ITSELF, included rather than linked.
 *
 * Its state - ptr_ready, ptr_slot, evt_cycle, the enqueue indices - is static,
 * and it is set by an enumeration sequence this harness has no intention of
 * faking: forty control transfers against a controller that does not exist
 * would be testing the harness, not the driver. Including the source puts that
 * state in reach so the test can start from "a mouse is enumerated" and go
 * straight to the thing under test, which is the ring. The shipping file is
 * not modified by a character and not #ifdef'd. */
/* paging.c's two translators, stubbed as the identity.
 *
 * xhci.c now routes every device-visible address through dma_addr()/dma_kaddr()
 * (dma.h), which call these. In the kernel they are a range check against the
 * one virtual window paging.c maps over the heap - and no xHCI buffer is in it,
 * so they return the identity for every address this file will ever see.
 * Identity here is therefore not a simplification, it is the same answer the
 * kernel gives.
 *
 * That makes this harness the ONLY place the dma_addr conversion is exercised
 * by something that actually walks the rings: it plays the controller, follows
 * the cycle bit and the Link TRB, and reads the buffer each TRB names. If the
 * conversion had broken a TRB address, the fake controller would read the wrong
 * memory and the pointer reports would stop decoding.
 */
unsigned long long vmm_phys(unsigned long long virt) { return virt; }
unsigned long long vmm_virt(unsigned long long phys) { return phys; }

#include "../xhci.c"

/* ---- the rest of input.c's world ---------------------------------------- */
int idt_scan(void)        { return 0; }
int idt_mouse_x(void)     { return 0; }
int idt_mouse_y(void)     { return 0; }
int idt_mouse_btn(void)   { return 0; }
int idt_mouse_wheel(void) { return 0; }
int ser_rx(void)          { return -1; }   /* -1 is "no UART", 0 is a NUL byte */

void input_poll(void);
int  input_next(void);
int  input_type(void);
int  input_code(void);
int  input_x(void);
int  input_y(void);
int  input_ptr_x(void);
int  input_ptr_y(void);
void input_set_bounds(int w, int h);
void input_set_speed(int pct);
void input_set_accel(int on);
int  input_speed(void);
int  input_accel(void);

#define EV_KEY_DOWN 1
#define EV_KEY_UP   2
#define EV_CHAR     3
#define EV_MOUSE    4

/* ---- assertions --------------------------------------------------------- */
static int fails;

static void ok(const char *what, int cond)
{
    printf("  %-58s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}

static void okv(const char *what, int got, int want)
{
    int cond = (got == want);
    printf("  %-58s %s", what, cond ? "ok" : "FAIL");
    if (!cond) printf("   (got %d, wanted %d)", got, want);
    printf("\n");
    if (!cond) fails++;
}

/* ==== the fake controller ================================================= */

#define PTR_SLOT   1
#define PTR_DCI    3          /* endpoint 1 IN */
#define KBD_SLOT   2
#define KBD_DCI    3

/* the controller's producer state for the EVENT ring - the mirror of the
 * driver's evt_dequeue/evt_cycle */
static u32 ctl_evt_enq = 0, ctl_evt_cyc = 1;

static void ctl_post_event(u64 param, u32 status, u32 control)
{
    trb_write(XMEM_EVTRING, ctl_evt_enq, param, status, control | ctl_evt_cyc);
    ctl_evt_enq++;
    /* An event ring has no Link TRB - its extent comes from the ERST - so the
     * producer wraps at the segment size and flips its cycle, exactly as
     * event_poll() expects to see. */
    if (ctl_evt_enq >= RING_TRBS) { ctl_evt_enq = 0; ctl_evt_cyc ^= 1; }
}

/* one endpoint, from the controller's side */
struct fake_ep {
    int slot, dci;
    u32 deq;          /* its dequeue index into INT_RING(slot)     */
    u32 cyc;          /* the cycle bit it expects to consume       */
    int completed;    /* how many transfers it has finished, ever  */
};

static struct fake_ep ep_ptr = { PTR_SLOT, PTR_DCI, 0, 1, 0 };
static struct fake_ep ep_kbd = { KBD_SLOT, KBD_DCI, 0, 1, 0 };

/* Deliver ONE report on this endpoint, if the driver has posted a buffer for
 * it. Returns 1 if a transfer completed, 0 if the endpoint is starved - which
 * is the case that matters, because a starved endpoint is a report the device
 * had and the guest never got.
 *
 * This is the hardware contract in nine lines: a Normal TRB whose cycle bit
 * matches is work; a Link TRB is a jump; a cycle mismatch means the ring is
 * empty and the controller stops. */
static int ctl_service_ep(struct fake_ep *e, const unsigned char *report, int len)
{
    u32 ring = INT_RING((u32)e->slot);

    for (int hop = 0; hop < 2; hop++) {          /* at most one Link follow */
        volatile u32 *t = (volatile u32 *)(ring + e->deq * TRB_BYTES);
        u32 ctrl = t[3];
        if ((ctrl & 1u) != e->cyc) return 0;     /* no TRB posted - starved */

        int type = (int)((ctrl >> 10) & 0x3F);
        if (type == TRB_LINK) {
            e->deq = 0;
            if (ctrl & (1u << 1)) e->cyc ^= 1;   /* toggle cycle */
            continue;
        }

        /* A Normal TRB names the buffer to fill. Read it from the TRB - never
         * assume PTR_REPORT - so that a driver posting several buffers is
         * modelled correctly instead of flattered. */
        u32 buf = t[0];
        u32 cap = t[2] & 0x1FFFFu;
        int n = (len < (int)cap) ? len : (int)cap;
        for (int i = 0; i < n; i++)
            *(volatile unsigned char *)((uptr)buf + (u32)i) = report[i];

        u32 trb_addr = ring + e->deq * TRB_BYTES;
        u32 residual = (u32)((int)cap - n);
        int cc = (n == (int)cap) ? 1 : 13;       /* success, or short packet */
        ctl_post_event((u64)trb_addr,
                       residual | ((u32)cc << 24),
                       ((u32)TRB_TRANSFER_EVENT << 10)
                         | ((u32)e->dci << 16)
                         | ((u32)e->slot << 24));

        e->deq++;
        if (e->deq >= RING_TRBS) e->deq = 0;
        e->completed++;
        return 1;
    }
    return 0;
}

/* ==== the fake device: QEMU's usb-mouse =================================== */

static int dev_acc_x, dev_acc_y;     /* undelivered motion, as hid.c keeps it */
static int dev_buttons;
static int dev_starved;              /* service intervals with nothing posted */

static void mouse_move(int dx, int dy) { dev_acc_x += dx; dev_acc_y += dy; }

static int clamp127(int v) { return v > 127 ? 127 : (v < -127 ? -127 : v); }

/* One service interval on the pointer endpoint. A real device NAKs when it has
 * nothing to say, so a still hand costs no transfer at all. */
static int usb_tick_mouse(void)
{
    if (!dev_acc_x && !dev_acc_y) return 0;      /* NAK - nothing to report */

    int dx = clamp127(dev_acc_x), dy = clamp127(dev_acc_y);
    unsigned char rep[4];
    rep[0] = (unsigned char)dev_buttons;
    rep[1] = (unsigned char)(signed char)dx;
    rep[2] = (unsigned char)(signed char)dy;
    rep[3] = 0;

    if (!ctl_service_ep(&ep_ptr, rep, 4)) { dev_starved++; return 0; }

    /* only subtract what actually went out on the wire */
    dev_acc_x -= dx;
    dev_acc_y -= dy;
    return 1;
}

/* One service interval on the keyboard endpoint, carrying a boot report. */
static int usb_tick_kbd(const unsigned char rep[8])
{
    return ctl_service_ep(&ep_kbd, rep, 8);
}

/* ==== bringing the driver up without a bus ================================ */

static void arena_map(void)
{
    /* The DMA arena, at the address memmap.h gives xhci.c. Same trick as
     * fbbench: MAP_FIXED_NOREPLACE so a collision is an error rather than a
     * silent relocation of somebody else's memory. */
    void *p = mmap((void *)(uintptr_t)HI_XHCI, (size_t)(HI_VGPU - HI_XHCI),
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p == MAP_FAILED || (uintptr_t)p != (uintptr_t)HI_XHCI) {
        printf("FATAL: cannot map the xHCI arena at 0x%08lX\n",
               (unsigned long)HI_XHCI);
        exit(2);
    }

    /* A register block. Nothing here reads a meaningful value back - the
     * driver only WRITES to it on the paths under test (the ERDP update in
     * event_poll, the doorbell in requeue) - so plain memory is the whole of
     * the model. */
    void *bar = mmap(NULL, 0x10000, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (bar == MAP_FAILED) { printf("FATAL: no BAR\n"); exit(2); }
    xbase = (uptr)(uintptr_t)bar;
    xop   = xbase + 0x100;
    xrt   = xbase + 0x600;
    xdb   = xbase + 0x800;
}

/* Start from "a mouse and a keyboard are enumerated", which is the state the
 * bring-up sequence leaves behind, and set nothing else. */
static void driver_reset(void)
{
    memset((void *)(uintptr_t)HI_XHCI, 0, (size_t)(HI_VGPU - HI_XHCI));

    evt_dequeue = 0; evt_cycle = 1;
    ctl_evt_enq = 0; ctl_evt_cyc = 1;

    ring_init(INT_RING(PTR_SLOT));
    ring_init(INT_RING(KBD_SLOT));

    ptr_slot = PTR_SLOT; ptr_dci = PTR_DCI; ptr_mps = 4;
    ptr_abs = 0;                         /* try.sh attaches usb-mouse */
    ptr_enq = 0; ptr_cyc = 1;
    ptr_ready = 1;
    ptr_x = HOME_X; ptr_y = HOME_Y;
    ptr_btn = 0; ptr_reports = 0; ptr_events = 0;

    kbd_slot = KBD_SLOT; kbd_dci = KBD_DCI; kbd_mps = 8;
    kbd_enq = 0; kbd_cyc = 1;
    kbd_ready = 1;
    kbd_mods = 0;
    keyq_head = keyq_tail = 0;
    kevq_head = kevq_tail = 0;
    memset(prev_keys, 0, sizeof prev_keys);

    ep_ptr = (struct fake_ep){ PTR_SLOT, PTR_DCI, 0, 1, 0 };
    ep_kbd = (struct fake_ep){ KBD_SLOT, KBD_DCI, 0, 1, 0 };

    dev_acc_x = dev_acc_y = dev_buttons = 0;
    dev_starved = 0;

    /* Arm each endpoint exactly the way bring-up does. ptr_arm_all() is the
     * driver's own function, deliberately: if this harness armed a depth of
     * its own choosing it would be testing the harness. */
    ptr_arm_all();
    kbd_requeue();

    /* Drain anything input.c is holding from a previous case, and let its
     * pointer adopt the position rather than announce it. */
    input_set_bounds(fake_pxw, fake_pxh);
    input_poll();
    for (int i = 0; i < 256 && input_next(); i++) { }

    /* AND PUT input.c's POINTER BACK WHERE IT STARTED.
     *
     * It has no reset entry point - ms_seen latches on the first poll and the
     * accelerated position is a static that nothing clears - so without this
     * each case begins wherever the last one finished. That is not merely
     * untidy: the cases below move the pointer thousands of pixels, it ends up
     * pinned against the clamp, and every later assertion then measures zero
     * travel and "passes" or "fails" for a reason that has nothing to do with
     * the code under test. Walked home through the REAL path, at 1:1, so the
     * homing itself cannot hide a scaling bug. */
    int spd = input_speed(), acc = input_accel();
    input_set_accel(0);
    input_set_speed(100);
    for (int i = 0; i < 400; i++) {
        int ex = HOME_X - input_ptr_x(), ey = HOME_Y - input_ptr_y();
        if (!ex && !ey) break;
        mouse_move(ex, ey);
        for (int k = 0; k < PTR_NBUF; k++) usb_tick_mouse();
        input_poll();
    }
    for (int i = 0; i < 256 && input_next(); i++) { }
    (void)spd; (void)acc;
    /* EVERY CASE STARTS AT 1:1. The transport cases below assert how many
     * reports get through, and the accel curve would fold a second variable
     * into every one of those numbers - a 40 px report is over ACC_THRESH, so
     * it arrives tripled. test_relative_is_relative() sets its own. */
    input_set_speed(100);
    input_set_accel(0);

    /* ...and the device must not still be holding motion left over from the
     * walk home. It clamps at 127 counts a report and keeps the remainder, so
     * arriving at HOME does not mean its accumulator is empty - and whatever
     * is left would be delivered into the next case as travel nobody asked
     * for. This was worth three wrong numbers before it was noticed. */
    dev_acc_x = dev_acc_y = 0;
    dev_starved = 0;
    ptr_reports = 0;
    ep_ptr.completed = 0;
}

/* ==== the tests =========================================================== */

/* How much motion reaches the pointer in ONE frame, when the hand produced
 * `intervals` service intervals' worth of it since the last frame. */
static int one_frame_travel(int intervals, int px_per_interval)
{
    int before = input_ptr_x();
    for (int i = 0; i < intervals; i++) {
        mouse_move(px_per_interval, 0);
        usb_tick_mouse();
    }
    input_poll();
    return input_ptr_x() - before;
}

static void test_drain_rate(void)
{
    printf("\nMANY SMALL MOVES IN ONE FRAME - does the pointer track the sum?\n");
    printf("  (a hand moving steadily makes one report per service interval;\n");
    printf("   a frame is longer than an interval, so several are waiting)\n\n");

    driver_reset();
    /* Eight intervals of 40 px. Nothing here is near the device's 127-count
     * ceiling, so every count is deliverable and the ONLY thing that can lose
     * them is the driver. */
    int got = one_frame_travel(PTR_NBUF, 40);
    okv("8 reports of 40 px in one frame -> pointer moves 320 px",
        got, 40 * PTR_NBUF);
    okv("...and the device was never starved of a buffer", dev_starved, 0);

    driver_reset();
    got = one_frame_travel(4, 20);
    okv("4 reports of 20 px in one frame -> pointer moves 80 px", got, 80);

    /* The residual case: what the device could not deliver must not vanish. It
     * is still owed, and the next frame must collect it. */
    driver_reset();
    mouse_move(600, 0);                  /* one flick, five reports' worth */
    for (int i = 0; i < 8; i++) usb_tick_mouse();
    input_poll();
    int after_one = input_ptr_x() - HOME_X;
    for (int f = 0; f < 8; f++) {        /* eight more frames to settle */
        for (int i = 0; i < 8; i++) usb_tick_mouse();
        input_poll();
    }
    okv("a 600 px flick arrives in full within 9 frames",
        input_ptr_x() - HOME_X, 600);
    ok("...and most of it arrives in the FIRST frame", after_one >= 500);
}

static void test_ring_ownership(void)
{
    printf("\nONE RING, TWO ENDPOINTS - can either starve the other?\n\n");

    /* The keyboard's event is posted FIRST, so a pointer poll that takes
     * whatever is on the front of the ring takes the keystroke instead. */
    driver_reset();
    unsigned char down_a[8] = { 0, 0, 0x04, 0, 0, 0, 0, 0 };
    usb_tick_kbd(down_a);
    mouse_move(30, 0);
    usb_tick_mouse();

    input_poll();
    int moved = input_ptr_x() - HOME_X;
    int saw_a = 0;
    for (int i = 0; i < 64; i++) {
        int t = input_next();
        if (!t) break;
        if (t == EV_CHAR && input_code() == 'a') saw_a = 1;
    }
    okv("keystroke first on the ring: the pointer still moves 30 px", moved, 30);
    ok("...and the keystroke is not eaten either", saw_a);

    /* ...and the other order. */
    driver_reset();
    mouse_move(30, 0);
    usb_tick_mouse();
    usb_tick_kbd(down_a);

    input_poll();
    moved = input_ptr_x() - HOME_X;
    saw_a = 0;
    for (int i = 0; i < 64; i++) {
        int t = input_next();
        if (!t) break;
        if (t == EV_CHAR && input_code() == 'a') saw_a = 1;
    }
    okv("pointer first on the ring: it still moves 30 px", moved, 30);
    ok("...and the keystroke arrives in the same frame", saw_a);
}

static void test_endpoint_liveness(void)
{
    printf("\nLIVENESS - every completed transfer must be re-armed\n\n");

    driver_reset();
    for (int f = 0; f < 200; f++) {
        mouse_move(3, 0);
        usb_tick_mouse();
        input_poll();
    }
    okv("200 frames of steady movement -> 200 reports decoded",
        (int)ptr_reports, 200);
    okv("...with the device never starved", dev_starved, 0);

    /* The keyboard is the fragile one: dropping a transfer event also drops
     * its requeue, and the endpoint then goes silent for good. */
    driver_reset();
    unsigned char down[8] = { 0, 0, 0x04, 0, 0, 0, 0, 0 };
    unsigned char up[8]   = { 0, 0, 0,    0, 0, 0, 0, 0 };
    int chars = 0;
    for (int f = 0; f < 100; f++) {
        usb_tick_kbd((f & 1) ? up : down);
        mouse_move(2, 0);                 /* pointer traffic on the same ring */
        usb_tick_mouse();
        input_poll();
        for (int i = 0; i < 64; i++) {
            int t = input_next();
            if (!t) break;
            if (t == EV_CHAR && input_code() == 'a') chars++;
        }
    }
    okv("50 keypresses survive 100 frames of pointer traffic", chars, 50);
    okv("...and the pointer got all 100 of its reports", (int)ptr_reports, 100);
}

/* A usb-mouse is RELATIVE, and everything downstream has to agree about that.
 * pump_mouse() used to decide "absolute" from "a USB pointer exists", so a
 * relative mouse took the tablet branch: 1:1, no acceleration, and the speed
 * setting reaching nothing at all. Every probe in this repo attaches a
 * usb-tablet, so nothing disagreed. try.sh attaches a usb-mouse. */
static void test_relative_is_relative(void)
{
    printf("\nA MOUSE IS NOT A TABLET - does the speed setting reach it?\n\n");

    ok("the enumerated device reports itself relative", !xhci_ptr_abs());

    /* 1x with no acceleration must be the identity, or nothing below it can be
     * trusted: this is the case that has to stay byte-identical. */
    driver_reset();
    input_set_accel(0);
    input_set_speed(100);
    okv("1x, no accel: 8 reports of 20 px -> 160 px",
        one_frame_travel(PTR_NBUF, 20), 160);

    driver_reset();
    input_set_accel(0);
    input_set_speed(200);
    okv("2x, no accel: the same motion travels twice as far",
        one_frame_travel(PTR_NBUF, 20), 320);

    driver_reset();
    input_set_accel(0);
    input_set_speed(50);
    okv("0.5x: half as far, and the sub-unit remainder is carried",
        one_frame_travel(PTR_NBUF, 20), 80);

    /* Slow precise movement is the case the curve exists to protect: at or
     * below the threshold it must not be scaled at all.
     *
     * THE CURVE'S INPUT IS ONE FRAME'S DISPLACEMENT, NOT ONE REPORT'S - it is
     * a velocity, which is what input.c's own note says it must be. So "slow"
     * here means a slow FRAME: one report of 3 px, not eight of them. Eight
     * 2 px reports is a 16 px frame and is correctly accelerated, and getting
     * that backwards is how a test ends up demanding that fast movement be
     * treated as slow.
     *
     * Worth noting what the drain fix did to this: while only one report per
     * frame got through, the curve saw a FRACTION of the hand's real velocity
     * and under-accelerated accordingly. Feeding it every report is not just
     * more travel, it is the first time the input to the curve has been the
     * number the curve is documented to take. */
    driver_reset();
    input_set_accel(1);
    input_set_speed(100);
    int slow = one_frame_travel(1, 3);          /* a 3 px frame - under ACC_THRESH */
    okv("accel on: a frame slower than the threshold stays 1:1", slow, 3);

    driver_reset();
    input_set_accel(1);
    input_set_speed(100);
    int mid = one_frame_travel(PTR_NBUF, 2);    /* a 16 px frame - over it */
    ok("accel on: eight slow reports are ONE fast frame, and scale",
       mid > 16);

    driver_reset();
    input_set_accel(1);
    input_set_speed(100);
    int fast = one_frame_travel(PTR_NBUF, 40);  /* well over it */
    ok("accel on: fast movement is amplified", fast > 40 * PTR_NBUF);

    /* ...and back to the default, so ordering cannot leak into other cases. */
    input_set_accel(1);
    input_set_speed(100);
}

/* A control or bulk transfer waits on the SAME event ring the HID devices post
 * to. It used to take the first transfer event it saw, whoever it belonged to
 * - so a pointer report landing during enumeration or a USB-stick read was
 * consumed as that transfer's completion and never requeued, and the endpoint
 * died. try.sh attaches a usb-storage device, so this is not hypothetical. */
static void test_transfer_wait_is_addressed(void)
{
    printf("\nA DISK READ MUST NOT KILL THE POINTER\n\n");

    driver_reset();

    /* A pointer report is sitting on the ring... */
    mouse_move(25, 0);
    ok("a pointer report is waiting on the ring", usb_tick_mouse() == 1);

    /* ...when something waits for a transfer on a DIFFERENT endpoint. Nothing
     * will ever complete for slot 7, so this drains the ring and gives up -
     * which is the whole question: what did it do with what it drained? */
    u32 st = 0, ct = 0;
    int got = xfer_wait(7, 5, &st, &ct, 1);
    okv("it does not mistake the pointer's completion for its own", got, 0);

    /* The pointer's report must have been decoded, not swallowed... */
    okv("...the pointer's report was decoded anyway", (int)ptr_reports, 1);

    /* ...and, the part that actually kills an endpoint, re-armed. */
    mouse_move(25, 0);
    ok("...and its endpoint was re-armed, so it still works",
       usb_tick_mouse() == 1);

    input_poll();
    /* Two reports decoded is the claim - that neither was swallowed and the
     * endpoint kept running. Deliberately NOT an assertion on travel:
     * scale_axis carries a sub-unit remainder across calls by design, and it
     * is a static in input.c that no case can reset, so an exact-pixel
     * expectation here would be off by one for reasons that have nothing to do
     * with the event ring. The accel arithmetic has its own cases above. */
    okv("both reports were decoded, neither swallowed", (int)ptr_reports, 2);
    ok("...and the pointer actually moved", input_ptr_x() > HOME_X + 45);
}

/* The number the whole bug reduces to. Not an assertion - a measurement,
 * printed so a change to it is visible in the gate log. */
static void report_ceiling(void)
{
    driver_reset();
    /* Offer far more than any frame could take, then see what one frame took. */
    for (int i = 0; i < 64; i++) { mouse_move(100, 0); usb_tick_mouse(); }
    int before = input_ptr_x();
    input_poll();
    int per_frame = input_ptr_x() - before;

    printf("\n  reports the driver takes per frame: %d\n", ep_ptr.completed);
    printf("  pointer travel available in one frame: %d px\n", per_frame);
    printf("  therefore at 100 Hz (the PIT gate on wm_frame) the pointer\n");
    printf("  cannot exceed %d px/s, and a brisk hand is 3000-10000 px/s.\n",
           per_frame * 100);
}

int main(void)
{
    printf("xhcitest - the shipping xhci.c and input.c, against a fake controller\n");

    arena_map();

    test_drain_rate();
    test_ring_ownership();
    test_endpoint_liveness();
    test_relative_is_relative();
    test_transfer_wait_is_addressed();
    report_ceiling();

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
