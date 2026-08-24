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
static int dev_buttons, dev_sent_buttons;
static int dev_starved;              /* service intervals with nothing posted */

static void mouse_move(int dx, int dy) { dev_acc_x += dx; dev_acc_y += dy; }

static int clamp127(int v) { return v > 127 ? 127 : (v < -127 ? -127 : v); }

/* One service interval on the pointer endpoint. A real device NAKs when it has
 * nothing to say, so a still hand costs no transfer at all. */
static int usb_tick_mouse(void)
{
    if (!dev_acc_x && !dev_acc_y && dev_buttons == dev_sent_buttons)
        return 0;                                /* NAK - nothing changed */

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
    dev_sent_buttons = dev_buttons;
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
    ptr_edge_head = ptr_edge_tail = 0;

    kbd_slot = KBD_SLOT; kbd_dci = KBD_DCI; kbd_mps = 8;
    kbd_enq = 0; kbd_cyc = 1;
    kbd_ready = 1;
    kbd_mods = 0;
    keyq_head = keyq_tail = 0;
    kevq_head = kevq_tail = 0;
    memset(prev_keys, 0, sizeof prev_keys);

    ep_ptr = (struct fake_ep){ PTR_SLOT, PTR_DCI, 0, 1, 0 };
    ep_kbd = (struct fake_ep){ KBD_SLOT, KBD_DCI, 0, 1, 0 };

    dev_acc_x = dev_acc_y = dev_buttons = dev_sent_buttons = 0;
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
    ep0_first_config_done = 0;
    ep0_first_config_port = 0;
    memset(ep0_first_config_diag, 0, sizeof ep0_first_config_diag);
    ptr_port = 0;
    kbd_port = 0;
}

/* ==== the tests =========================================================== */

static void test_configure_endpoint_preserves_slot_context(void)
{
    printf("\nCONFIGURE ENDPOINT MUST PRESERVE THE LIVE SLOT CONTEXT\n\n");

    for (int size = 32; size <= 64; size += 32) {
        xctxsize = size;
        zero_mem(CTX_INPUT, 33u * (u32)xctxsize);
        zero_mem(CTX_DEVICE(7), 2048);
        ctx_set(CTX_DEVICE(7), 0, 0, 0x02ABCDEFu);
        ctx_set(CTX_DEVICE(7), 0, 1, 0x12345678u);
        ctx_set(CTX_DEVICE(7), 0, 2, 0x89ABCDEFu);
        ctx_set(CTX_DEVICE(7), 0, 3, 0x76543210u);

        copy_slot_context_for_endpoints(7, 4);

        char label[96];
        snprintf(label, sizeof label, "%d-byte context keeps slot DW1", size);
        okv(label, (int)ctx_get(CTX_INPUT, 1, 1), (int)0x12345678u);
        snprintf(label, sizeof label, "%d-byte context keeps TT/interrupter DW2", size);
        okv(label, (int)ctx_get(CTX_INPUT, 1, 2), (int)0x89ABCDEFu);
        snprintf(label, sizeof label, "%d-byte context keeps address/state DW3", size);
        okv(label, (int)ctx_get(CTX_INPUT, 1, 3), (int)0x76543210u);
        snprintf(label, sizeof label, "%d-byte context changes only Context Entries", size);
        okv(label, (int)ctx_get(CTX_INPUT, 1, 0),
            (int)((0x02ABCDEFu & 0x07FFFFFFu) | (4u << 27)));
    }
}

static void test_ecm_device_class_filter(void)
{
    printf("\nCDC-ECM DISCOVERY MUST NOT RESET KNOWN NON-NETWORK DEVICES\n\n");

    okv("class 00 composite/per-interface device remains probeable",
        xhci_ecm_device_class_candidate(0x00), 1);
    okv("class 02 communications device remains probeable",
        xhci_ecm_device_class_candidate(0x02), 1);
    okv("class EF interface-association device remains probeable",
        xhci_ecm_device_class_candidate(0xEF), 1);
    okv("class E0 Bluetooth device is rejected before re-enumeration",
        xhci_ecm_device_class_candidate(0xE0), 0);
    okv("class 08 mass-storage device is not an ECM candidate",
        xhci_ecm_device_class_candidate(0x08), 0);
    okv("class 03 HID device is not an ECM candidate",
        xhci_ecm_device_class_candidate(0x03), 0);
}

static void test_physical_imation_mass_storage_descriptor(void)
{
    printf("\nTHE PHYSICAL IMATION STICK DESCRIPTOR MUST MATCH BOT\n\n");
    static const unsigned char descriptor[32] = {
        9, DESC_CONFIG, 32, 0, 1, 1, 0, 0x80, 100,
        9, DESC_INTERFACE, 0, 0, 2, 0x08, 0x06, 0x50, 0,
        7, DESC_ENDPOINT, 0x81, 2, 0x00, 0x02, 0,
        7, DESC_ENDPOINT, 0x02, 2, 0x00, 0x02, 0
    };
    zero_mem(CFG_BUF, CFG_MAX);
    memcpy((void *)(uintptr_t)CFG_BUF, descriptor, sizeof descriptor);

    struct msc_config cfg;
    ok("0718:067d config matches SCSI transparent Bulk-Only",
       msc_parse_config_descriptor(sizeof descriptor, &cfg));
    okv("configuration value", cfg.cfgval, 1);
    okv("mass-storage interface", cfg.iface, 0);
    okv("bulk IN endpoint", cfg.in_ep, 0x81);
    okv("bulk OUT endpoint", cfg.out_ep, 0x02);
    okv("bulk IN max packet", cfg.in_mps, 512);
    okv("bulk OUT max packet", cfg.out_mps, 512);
}

static void test_cdc_ecm_descriptor_and_receive_identity(void)
{
    printf("\nCDC-ECM DESCRIPTORS AND RX COMPLETIONS ARE BOUNDED\n\n");
    static const unsigned char descriptor[] = {
        9, 2, 88, 0, 2, 1, 0, 0x80, 50,
        8, 11, 0, 2, 2, 6, 0, 0,
        9, 4, 0, 0, 1, 2, 6, 0, 0,
        5, 0x24, 0, 0x10, 0x01,
        5, 0x24, 6, 0, 1,
        13, 0x24, 0x0f, 4, 0, 0, 0, 0, 0xea, 0x05, 0, 0, 0,
        7, 5, 0x85, 3, 16, 0, 9,
        9, 4, 1, 0, 0, 0x0a, 0, 0, 0,
        9, 4, 1, 1, 2, 0x0a, 0, 0, 0,
        7, 5, 0x01, 2, 0, 2, 0,
        7, 5, 0x82, 2, 0, 2, 0
    };
    memcpy((void *)(uintptr_t)CFG_BUF, descriptor, sizeof descriptor);
    struct ecm_config cfg;
    ok("standards-based CDC-ECM composite descriptor matches",
       ecm_parse_config_descriptor((int)sizeof descriptor, &cfg));
    okv("communications interface", cfg.comm_iface, 0);
    okv("data interface", cfg.data_iface, 1);
    okv("active data alternate setting", cfg.data_alt, 1);
    okv("bulk OUT endpoint", cfg.out_ep, 0x01);
    okv("bulk IN endpoint", cfg.in_ep, 0x82);
    okv("Ethernet MAC string index", cfg.mac_index, 4);
    okv("bounded Ethernet segment size", cfg.max_segment, 1514);

    /* Completion identity comes from the TRB pointer, not from a rotating
     * guess. With several posted buffers, guessing replays or skips frames. */
    ecm_ready = 1;
    ecm_slot = 7; ecm_in_dci = 5;
    ecm_rx_qh = ecm_rx_qt = 0;
    for (int i = 0; i < RING_TRBS; i++) ecm_rx_map[i] = -1;
    ecm_rx_map[3] = 2;
    u32 status = (1u << 24) | (ECM_FRAME_MAX - 60u);
    u32 ctrl = (7u << 24) | (5u << 16);
    ok("the ECM dispatcher claims its exact slot/endpoint event",
       ecm_event(ECM_IN_RING(7) + 3u * TRB_BYTES, status, ctrl));
    okv("the completed TRB selects receive buffer two", ecm_rx_q[0], 2);
    okv("residual length becomes a 60-byte Ethernet frame", ecm_rx_len[2], 60);
    ecm_ready = 0;
}

static void test_multi_port_diagnostics_do_not_overwrite(void)
{
    printf("\nEVERY USB PORT MUST KEEP ITS OWN FAILURE BOUNDARY\n\n");
    memset(msc_port_stage, 0, sizeof msc_port_stage);
    memset(msc_port_slot, 0, sizeof msc_port_slot);
    memset(msc_port_vid, 0, sizeof msc_port_vid);
    memset(msc_port_pid, 0, sizeof msc_port_pid);
    memset(msc_port_cc, -1, sizeof msc_port_cc);
    msc_init_stage = MSC_INIT_CONTROLLER;
    msc_init_port = 0;
    msc_init_slot = 0;
    msc_init_cc = -1;

    slot_vid[2] = 0x0718; slot_pid[2] = 0x067d;
    msc_note_init(MSC_INIT_CONFIG_HEAD, 4, 2, 4);
    slot_vid[4] = 0x8087; slot_pid[4] = 0x0026;
    msc_note_init(MSC_INIT_CONFIG_HEAD, 10, 4, 6);

    okv("equal-depth Bluetooth scan cannot replace the earlier summary port",
        xhci_msc_init_port(), 4);
    okv("Imation port retains its own completion", xhci_msc_port_cc(4), 4);
    okv("Imation identity remains attached to port 4", xhci_msc_port_vid(4), 0x0718);
    okv("Bluetooth port retains its separate completion", xhci_msc_port_cc(10), 6);
    okv("Bluetooth identity remains attached to port 10", xhci_msc_port_vid(10), 0x8087);

    enum_note(4, ENUM_DESCRIPTOR, 4);
    enum_note(10, ENUM_DESCRIPTOR, 1);
    okv("enumeration failure remains on Imation port", xhci_enum_cc(4), 4);
    okv("successful Bluetooth enumeration remains separate", xhci_enum_cc(10), 1);
}

static void test_ep0_transfer_is_published_atomically(void)
{
    printf("\nEP0 MUST NEVER EXPOSE A HALF-BUILT CONTROL TRANSFER\n\n");
    const int slot = 5;
    u32 ring = EP0_RING(slot);
    ring_init(ring);
    ep0_enqueue[slot] = 0;
    ep0_cycle[slot] = 1;

    u32 publish_cycle = 0;
    u32 setup = ep0_begin_unpublished(
        slot, 0x0009000001000680ULL, 8,
        (TRB_SETUP << 10) | (1u << 6) | (3u << 16), &publish_cycle);
    volatile u32 *setup_words = (volatile u32 *)(uintptr_t)setup;
    okv("setup starts with the opposite cycle and is invisible", setup_words[3] & 1u, 0);
    ep0_push(slot, dma_addr(XMEM_DATA), 9, (TRB_DATA << 10) | (1u << 16));
    ep0_push(slot, 0, 0, (TRB_STATUS << 10) | (1u << 5));
    okv("building data/status still leaves setup invisible", setup_words[3] & 1u, 0);
    ep0_publish(setup, publish_cycle);
    okv("one final cycle-bit commit publishes the complete transfer", setup_words[3] & 1u, 1);

    /* The same invariant must survive crossing the Link TRB. */
    ring_init(ring);
    ep0_enqueue[slot] = RING_TRBS - 2;
    ep0_cycle[slot] = 1;
    setup = ep0_begin_unpublished(
        slot, 0x0009000001000680ULL, 8,
        (TRB_SETUP << 10) | (1u << 6) | (3u << 16), &publish_cycle);
    setup_words = (volatile u32 *)(uintptr_t)setup;
    ep0_push(slot, dma_addr(XMEM_DATA), 9, (TRB_DATA << 10) | (1u << 16));
    ep0_push(slot, 0, 0, (TRB_STATUS << 10) | (1u << 5));
    okv("wrapped transfer remains hidden until its setup commits", setup_words[3] & 1u, 0);
    okv("wrapped producer continues with the toggled cycle", ep0_cycle[slot], 0);
    ep0_publish(setup, publish_cycle);
    okv("wrapped complete transfer publishes with the old cycle", setup_words[3] & 1u, 1);
}

static void test_descriptor_retry_recovers_halted_ep0(void)
{
    printf("\nA FLAKY DESCRIPTOR MUST RECOVER AND RETRY, NOT LOSE THE DEVICE\n\n");
    driver_reset();
    const int slot = 6;
    xhci_idx = 0;
    xslots = MAX_SLOTS - 1;

    ring_init(XMEM_CMDRING);
    cmd_enqueue = 0;
    cmd_cycle = 1;
    ring_init(EP0_RING(slot));
    ep0_enqueue[slot] = 0;
    ep0_cycle[slot] = 1;

    /* Attempt one fails on Setup. The driver must Reset Endpoint, replace its
     * dequeue ring, and then attempt the same descriptor again. Queue the
     * exact controller events in that causal order. */
    ctl_post_event(EP0_RING(slot), 4u << 24,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));
    ctl_post_event(XMEM_CMDRING, 1u << 24,
                   TRB_CMD_COMPLETION << 10);
    ctl_post_event(XMEM_CMDRING + TRB_BYTES, 1u << 24,
                   TRB_CMD_COMPLETION << 10);
    ctl_post_event(EP0_RING(slot) + 2u * TRB_BYTES, 1u << 24,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));

    int got = descriptor_in(slot, 0x02000680u, 0x00090000u, CFG_BUF, 9);
    ok("second descriptor attempt succeeds", got);
    okv("the retained attempt count proves the retry happened",
        ep0_last_attempts, 2);
    okv("halt recovery completed before retry", ep0_last_recovery, 1);
    okv("the successful Status event is retained", ep0_last_event_stage, 3);
    okv("software and hardware restart from one fresh three-TRB TD",
        (int)ep0_enqueue[slot], 3);
}

static void test_first_device_probe_is_the_immediate_next_td(void)
{
    printf("\nTHE FIRST DEVICE REQUEST MUST BE COMPARED WITH ITS IMMEDIATE TWIN\n\n");
    driver_reset();
    const int slot = 6;
    const u32 ring = EP0_RING(slot);
    xhci_idx = 0;
    xslots = MAX_SLOTS - 1;
    xctxsize = 32;
    cur_port = 4;

    ring_init(XMEM_CMDRING);
    cmd_enqueue = 0;
    cmd_cycle = 1;
    ring_init(ring);
    ep0_enqueue[slot] = 0;
    ep0_cycle[slot] = 1;
    ep0_first_device_probe_done = 0;
    memset(ep0_first_device_probe, 0, sizeof ep0_first_device_probe);

    /* descriptor_in() consumes the successful Status event at index two,
     * then the shipping probe must issue the same Device request beginning at
     * index three. Model the physical second-Setup cc4 and its two recovery
     * command completions in that exact order. */
    ctx_set(CTX_DEVICE(slot), 1, 0, 2);
    ctx_set(CTX_DEVICE(slot), 1, 2, dma_addr(ring + 3u * TRB_BYTES) | 1u);
    ctx_set(CTX_DEVICE(slot), 1, 3, 0);
    ctl_post_event(ring + 2u * TRB_BYTES, 1u << 24,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));
    ctl_post_event(ring + 3u * TRB_BYTES, (4u << 24) | 8u,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));
    ctl_post_event(XMEM_CMDRING, 1u << 24, TRB_CMD_COMPLETION << 10);
    ctl_post_event(XMEM_CMDRING + TRB_BYTES, 1u << 24,
                   TRB_CMD_COMPLETION << 10);

    ok("the real Device descriptor remains a public success",
       descriptor_in(slot, 0x01000680u, 0x00120000u, XMEM_DATA, 18));
    okv("public completion is restored to the first success", ep0_last_cc, 1);
    okv("public event stage is restored to Status", ep0_last_event_stage, 3);
    okv("public attempt count is restored", ep0_last_attempts, 1);

    okv("first probe is the Device request",
        (int)ep0_first_device_probe[0][0], (int)0x01000680u);
    okv("second probe is the identical Device request",
        (int)ep0_first_device_probe[1][0], (int)0x01000680u);
    okv("neither probe was polluted by Config",
        (int)ep0_first_device_probe[1][1], (int)0x00120000u);
    okv("first event points at Status index two",
        (int)ep0_first_device_probe[0][12], (int)(ring + 2u * TRB_BYTES));
    okv("first event completed successfully",
        (int)(ep0_first_device_probe[0][14] >> 24), 1);
    okv("second event points at Setup index three",
        (int)ep0_first_device_probe[1][12], (int)(ring + 3u * TRB_BYTES));
    okv("second event retains cc4",
        (int)(ep0_first_device_probe[1][14] >> 24), 4);
    okv("second event retains all eight Setup bytes as residual",
        (int)(ep0_first_device_probe[1][14] & 0xFFFFFFu), 8);
    okv("first producer snapshot is after one complete TD",
        (int)(ep0_first_device_probe[0][19] & 0xFFu), 3);
    okv("second producer snapshot is after two complete TDs",
        (int)(ep0_first_device_probe[1][19] & 0xFFu), 6);
    okv("probe metadata identifies physical port four",
        (int)(ep0_first_device_probe[1][20] & 0xFFu), 4);
    okv("probe metadata identifies the same slot",
        (int)((ep0_first_device_probe[1][20] >> 8) & 0xFFu), slot);
    okv("probe metadata retains completed recovery",
        (int)((ep0_first_device_probe[1][20] >> 24) & 3u), 2);
    okv("recovery replaced the live producer at ring index zero",
        (int)ep0_enqueue[slot], 0);

    u32 retained = ep0_first_device_probe[0][12];
    ctl_post_event(ring + 2u * TRB_BYTES, 1u << 24,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));
    ok("a later Device request still succeeds",
       descriptor_in(slot, 0x01000680u, 0x00120000u, XMEM_DATA, 18));
    okv("the done latch prevents replacing the first pair",
        (int)ep0_first_device_probe[0][12], (int)retained);
}

static void test_device_then_config_trace_survives_recovery(void)
{
    printf("\nTHE PHYSICAL DEVICE->CONFIG FAILURE MUST SURVIVE RING RESET\n\n");
    driver_reset();
    const int slot = 6;
    const u32 ring = EP0_RING(slot);
    xhci_idx = 0;
    xslots = MAX_SLOTS - 1;
    xctxsize = 32;

    ring_init(XMEM_CMDRING);
    cmd_enqueue = 0;
    cmd_cycle = 1;
    ring_init(ring);
    ep0_enqueue[slot] = 0;
    ep0_cycle[slot] = 1;

    ctl_post_event(ring + 2u * TRB_BYTES, 1u << 24,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));
    ok("Device descriptor succeeds before the physical boundary",
       xhci_control_in(slot, 0x01000680u, 0x00120000u, XMEM_DATA, 18));
    okv("Device TD leaves Config starting at ring index three",
        (int)ep0_enqueue[slot], 3);

    /* Model the Intel failure exactly: Config's Setup at index three gets cc4
     * and the output EP0 context is Halted at that same dequeue pointer. */
    ctx_set(CTX_DEVICE(slot), 1, 0, 2);
    ctx_set(CTX_DEVICE(slot), 1, 2, dma_addr(ring + 3u * TRB_BYTES) | 1u);
    ctx_set(CTX_DEVICE(slot), 1, 3, 0);
    ctl_post_event(ring + 3u * TRB_BYTES, 4u << 24,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));
    ctl_post_event(XMEM_CMDRING, 1u << 24, TRB_CMD_COMPLETION << 10);
    ctl_post_event(XMEM_CMDRING + TRB_BYTES, 1u << 24,
                   TRB_CMD_COMPLETION << 10);

    okv("Config Setup returns the injected USB Transaction Error",
        xhci_control_in(slot, 0x02000680u, 0x00090000u, CFG_BUF, 9), 0);
    okv("failure was matched to Config Setup at ring index three",
        (int)ep0_last_trace[12], (int)(ring + 3u * TRB_BYTES));
    okv("trace retains the exact Configuration request after ring_init",
        (int)ep0_last_trace[0], (int)0x02000680u);
    okv("trace retains wLength nine", (int)ep0_last_trace[1],
        (int)0x00090000u);
    okv("trace retains cc4 before recovery", (int)(ep0_last_trace[14] >> 24), 4);
    okv("trace retains pre-recovery Halted endpoint state",
        (int)(ep0_last_trace[16] & 7u), 2);
    okv("trace retains pre-recovery hardware dequeue",
        (int)(ep0_last_trace[17] & ~0xFu),
        (int)(dma_addr(ring + 3u * TRB_BYTES) & ~0xFu));
    okv("recovery replaced the live software ring at index zero",
        (int)ep0_enqueue[slot], 0);

    msc_note_init(MSC_INIT_CONFIG_HEAD, 4, slot, ep0_last_cc);
    u32 retained = xhci_msc_port_ep0_trace(4, 0);
    ep0_last_trace[0] = 0xDEADBEEFu;
    okv("port 4 keeps its trace when a later transfer overwrites globals",
        (int)xhci_msc_port_ep0_trace(4, 0), (int)retained);
}

static void test_first_config_attempt_survives_all_retries(void)
{
    printf("\nTHE FIRST CLEAN CONFIG FAILURE MUST OUTLIVE EVERY RECOVERY\n\n");
    driver_reset();
    const int slot = 6;
    const u32 ring = EP0_RING(slot);
    xhci_idx = 0;
    xslots = MAX_SLOTS - 1;
    xctxsize = 32;
    cur_port = 4;
    ring_init(XMEM_CMDRING);
    cmd_enqueue = 0;
    cmd_cycle = 1;
    ring_init(ring);
    ep0_enqueue[slot] = 0;
    ep0_cycle[slot] = 1;

    ctl_post_event(ring + 2u * TRB_BYTES, 1u << 24,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));
    ok("Device descriptor establishes the clean predecessor",
       xhci_control_in(slot, 0x01000680u, 0x00120000u, XMEM_DATA, 18));

    ctx_set(CTX_DEVICE(slot), 1, 0, 2);
    ctx_set(CTX_DEVICE(slot), 1, 2, dma_addr(ring + 3u * TRB_BYTES) | 1u);
    ctx_set(CTX_DEVICE(slot), 1, 3, 0);
    /* First Config starts at index 3. The two hard retries start at index 0
     * after their successful Reset Endpoint + Set TR Dequeue commands. */
    ctl_post_event(ring + 3u * TRB_BYTES, (4u << 24) | 8u,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));
    for (int attempt = 0; attempt < 3; attempt++) {
        ctl_post_event(XMEM_CMDRING + (u32)(attempt * 2) * TRB_BYTES,
                       1u << 24, TRB_CMD_COMPLETION << 10);
        ctl_post_event(XMEM_CMDRING + (u32)(attempt * 2 + 1) * TRB_BYTES,
                       1u << 24, TRB_CMD_COMPLETION << 10);
        if (attempt < 2)
            ctl_post_event(ring, (4u << 24) | 8u,
                           (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                           ((u32)slot << 24));
    }

    okv("all three same-slot Configuration attempts fail", descriptor_in(
        slot, 0x02000680u, 0x00090000u, CFG_BUF, 9), 0);
    okv("public trace is the final replacement-ring attempt",
        (int)ep0_last_trace[12], (int)ring);
    okv("v6 retained the first clean Config Setup at index three",
        (int)ep0_first_config_diag[12], (int)(ring + 3u * TRB_BYTES));
    okv("v6 retained first-attempt metadata",
        (int)(ep0_first_config_diag[19] >> 24), 1);
    okv("v6 retained pre-recovery cc4",
        (int)(ep0_first_config_diag[14] >> 24), 4);
    okv("v6 identifies Reset Endpoint recovery",
        (int)ep0_first_config_diag[24], 1);
    okv("Reset Endpoint command completed",
        (int)ep0_first_config_diag[25], 1);
    okv("Set TR Dequeue command completed",
        (int)ep0_first_config_diag[26], 1);

    ep0_note_config_reenumeration(4, 1, 4, 0);
    ep0_note_config_reenumeration(4, 2, 1, 1);
    okv("fresh old-scheme result is retained",
        (int)(ep0_first_config_diag[30] & 0xFFu), 4);
    okv("pre-address scheme result is retained",
        (int)((ep0_first_config_diag[30] >> 8) & 0xFFu), 1);
    okv("winning whole-enumeration scheme is retained",
        (int)((ep0_first_config_diag[30] >> 16) & 3u), 2);
}

static void test_forget_port_disables_and_invalidates_cached_slot(void)
{
    printf("\nA WHOLE-DEVICE RETRY MUST NOT REUSE THE BROKEN SLOT\n\n");
    driver_reset();
    const int slot = 6;
    xhci_idx = 0;
    xslots = MAX_SLOTS - 1;
    ring_init(XMEM_CMDRING);
    cmd_enqueue = 0;
    cmd_cycle = 1;
    port_slot[4] = slot;
    slot_vid[slot] = 0x0718;
    slot_pid[slot] = 0x067d;
    slot_class[slot] = 0x08;
    ctl_post_event(XMEM_CMDRING, 1u << 24, TRB_CMD_COMPLETION << 10);

    ok("old slot is explicitly disabled", xhci_forget_port(4));
    okv("port cache is cleared before re-enumeration", port_slot[4], 0);
    okv("old slot vendor identity is cleared", slot_vid[slot], 0);
    okv("old slot product identity is cleared", slot_pid[slot], 0);
    okv("old slot device class is cleared", slot_class[slot], 0);
}

static void test_preaddress_uses_bsr_without_set_address_delay(void)
{
    printf("\nTHE COMPATIBILITY PREFLIGHT MUST STAY AT USB ADDRESS ZERO\n\n");
    driver_reset();
    const int slot = 6;
    xhci_idx = 0;
    xslots = MAX_SLOTS - 1;
    xctxsize = 32;
    ring_init(XMEM_CMDRING);
    cmd_enqueue = 0;
    cmd_cycle = 1;
    ctl_post_event(XMEM_CMDRING, 1u << 24, TRB_CMD_COMPLETION << 10);

    ok("BSR Address Device command completes",
       xhci_address_device_mode(slot, 4, 3, 1));
    volatile u32 *command = (volatile u32 *)(uintptr_t)XMEM_CMDRING;
    okv("Address Device command carries Block Set Address Request",
        (int)((command[3] >> 9) & 1u), 1);
    okv("preflight still targets the requested slot",
        (int)(command[3] >> 24), slot);
    okv("preflight records the physical root port", cur_port, 4);
}

static void test_ep0_short_data_waits_for_status(void)
{
    printf("\nA SHORT DATA EVENT IS NOT THE END OF A CONTROL REQUEST\n\n");
    driver_reset();
    const int slot = 6;
    const u32 ring = EP0_RING(slot);
    xhci_idx = 0;
    xslots = MAX_SLOTS - 1;
    xctxsize = 32;
    ring_init(ring);
    ep0_enqueue[slot] = 0;
    ep0_cycle[slot] = 1;

    /* ISP makes a genuinely short IN stage observable as cc13 on Data. The
     * Status IOC still follows and is the only event that completes control. */
    ctl_post_event(ring + TRB_BYTES, (13u << 24) | 9u,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));
    ctl_post_event(ring + 2u * TRB_BYTES, 1u << 24,
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) |
                   ((u32)slot << 24));

    ok("control succeeds only after the queued Status event",
       xhci_control_in(slot, 0x01000680u, 0x00120000u, XMEM_DATA, 18));
    okv("the retained event is Status, not the short Data event",
        ep0_last_event_stage, 3);
    okv("the retained completion is final success", ep0_last_cc, 1);
    okv("the retained event pointer is the Status TRB",
        (int)ep0_last_trace[12], (int)(ring + 2u * TRB_BYTES));
    okv("the IN Data TRB requests interrupt on short packet",
        (int)((ep0_last_trace[7] >> 2) & 1u), 1);
}

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

static void test_button_edges_survive_one_slow_frame(void)
{
    printf("\nBUTTON EDGES ARE HISTORY, NOT FINAL POINTER STATE\n\n");
    driver_reset();

    /* Both reports arrive before input_poll(), exactly what happens when the
     * compositor stalls longer than a short click. Final state is released;
     * retaining only ptr_btn would erase the click completely. */
    dev_buttons = 1;
    ok("button-down report reaches the controller", usb_tick_mouse() == 1);
    dev_buttons = 0;
    ok("button-up report reaches the controller", usb_tick_mouse() == 1);
    input_poll();

    int states[4], n = 0;
    while (n < 4) {
        int type = input_next();
        if (!type) break;
        if (type == EV_MOUSE) states[n++] = (int)input_code();
    }
    okv("press and release both survive one input poll", n, 2);
    okv("first retained edge is button down", n > 0 ? states[0] : -1, 1);
    okv("second retained edge is button up", n > 1 ? states[1] : -1, 0);
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
    int got = xfer_wait_trbs(7, 5, 0, 0, &st, &ct, 1);
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

static void test_ep0_wait_rejects_stale_same_endpoint_event(void)
{
    printf("\nA LATE EP0 EVENT MUST NOT COMPLETE THE NEXT REQUEST\n\n");
    driver_reset();
    const int slot = 7;
    u32 current[3] = { EP0_RING(slot), EP0_RING(slot) + 16u, EP0_RING(slot) + 32u };

    ctl_post_event(EP0_RING(slot) + 48u, (1u << 24),
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) | ((u32)slot << 24));
    ctl_post_event(current[2], (1u << 24),
                   (TRB_TRANSFER_EVENT << 10) | (1u << 16) | ((u32)slot << 24));

    u32 status = 0, ctrl = 0;
    int got = xfer_wait_trbs(slot, 1, current, 3, &status, &ctrl, 1);
    okv("same-slot EP0 completion from the previous TD is ignored", got,
        TRB_TRANSFER_EVENT);
    okv("the current status TRB is the completion retained",
        (int)xfer_last_param, (int)current[2]);
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

    test_configure_endpoint_preserves_slot_context();
    test_ecm_device_class_filter();
    test_physical_imation_mass_storage_descriptor();
    test_cdc_ecm_descriptor_and_receive_identity();
    test_multi_port_diagnostics_do_not_overwrite();
    test_ep0_transfer_is_published_atomically();
    test_first_device_probe_is_the_immediate_next_td();
    test_descriptor_retry_recovers_halted_ep0();
    test_device_then_config_trace_survives_recovery();
    test_first_config_attempt_survives_all_retries();
    test_forget_port_disables_and_invalidates_cached_slot();
    test_preaddress_uses_bsr_without_set_address_delay();
    test_ep0_short_data_waits_for_status();
    test_button_edges_survive_one_slow_frame();
    test_drain_rate();
    test_ring_ownership();
    test_endpoint_liveness();
    test_relative_is_relative();
    test_transfer_wait_is_addressed();
    test_ep0_wait_rejects_stale_same_endpoint_event();
    report_ceiling();

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
