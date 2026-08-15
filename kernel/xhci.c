/* xhci.c - a USB 3 host controller driver, so zlOS has its own keyboard.
 *
 * WHY THIS EXISTS
 * ---------------
 * zlOS boots on a real ThinkPad and has no input. The i8042 "PS/2 controller"
 * that Linux reports is not real hardware on a machine like that - the
 * firmware synthesises it, and that emulation is a BOOT SERVICE. The moment we
 * call ExitBootServices and take the machine, it stops existing, and every key
 * press vanishes. The keyboard was never PS/2; it is USB, internally, behind
 * the xHCI controller.
 *
 * So this is the driver that gives zlOS real input on real hardware. It is
 * also, by some distance, the largest piece of hardware programming in the
 * project: a USB host controller is not a few ports you poke, it is a
 * command-driven engine that DMAs data structures you build in memory.
 *
 * THE SHAPE OF xHCI
 * -----------------
 * Three register blocks, all inside PCI BAR0, found by reading offsets out of
 * the first one:
 *     capability  at BAR0 + 0            - read-only description of the chip
 *     operational at BAR0 + CAPLENGTH    - run/stop, reset, the port array
 *     runtime     at BAR0 + RTSOFF       - interrupters and the event ring
 *     doorbells   at BAR0 + DBOFF        - one per device slot; ringing one
 *                                          tells the controller to look at a ring
 *
 * You do not "read a byte from the keyboard". You build a ring of Transfer
 * Request Blocks in memory, hand the controller its address, ring a doorbell,
 * and it DMAs the result back and posts a completion event on another ring.
 *
 * This file is built in stages so each one can be proven before the next:
 *     1. find the controller, map it, read its parameters   <- this stage
 *     2. reset and start it                                 <- this stage
 *     3. rings: command, event, DCBAA
 *     4. port reset and device enumeration
 *     5. HID boot protocol - actual keystrokes
 */

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

int  pci_count(void);
int  pci_class(int i);
int  pci_subclass(int i);
int  pci_vendor(int i);
int  pci_device(int i);
void pci_scan(void);
void pci_enable(int i);
u32  pci_bar(int i, int which);
u32  pci_bar_size(int i, int which);

/* ---- capability registers (BAR0 + 0) ----------------------------------- */
#define XCAP_CAPLENGTH   0x00   /* u8:  bytes to the operational block      */
#define XCAP_HCIVERSION  0x02   /* u16: 0x0100 = xHCI 1.0, 0x0110 = 1.1     */
#define XCAP_HCSPARAMS1  0x04   /* slots [7:0], interrupters [18:8], ports [31:24] */
#define XCAP_HCSPARAMS2  0x08
#define XCAP_HCCPARAMS1  0x10   /* AC64 bit0, CSZ bit2, xECP [31:16]        */
#define XCAP_DBOFF       0x14   /* doorbell array offset                     */
#define XCAP_RTSOFF      0x18   /* runtime register offset                   */

/* ---- operational registers (BAR0 + CAPLENGTH) -------------------------- */
#define XOP_USBCMD       0x00   /* RS bit0, HCRST bit1, INTE bit2           */
#define XOP_USBSTS       0x04   /* HCH bit0, CNR bit11 (controller not ready) */
#define XOP_PAGESIZE     0x08
#define XOP_DNCTRL       0x14
#define XOP_CRCR         0x18   /* command ring control (64-bit)            */
#define XOP_DCBAAP       0x30   /* device context base address array (64-bit) */
#define XOP_CONFIG       0x38   /* MaxSlotsEn in [7:0]                       */
#define XOP_PORTSC(p)    (0x400 + ((p) - 1) * 0x10)

#define USBCMD_RS        (1u << 0)
#define USBCMD_HCRST     (1u << 1)
#define USBCMD_INTE      (1u << 2)
#define USBSTS_HCH       (1u << 0)
#define USBSTS_CNR       (1u << 11)

/* ---- state ------------------------------------------------------------- */
static int  xhci_idx = -1;
static u32  xbase    = 0;       /* BAR0 - the whole register block          */
static u32  xop      = 0;       /* operational registers                     */
static u32  xrt      = 0;       /* runtime registers                         */
static u32  xdb      = 0;       /* doorbell array                            */
static int  xslots   = 0;
static int  xports   = 0;
static int  xver     = 0;
static int  xctxsize = 32;      /* 32 or 64 bytes, from HCCPARAMS1.CSZ      */

/* MMIO. volatile because these are registers - the compiler must not cache
 * a value the hardware changes underneath it, nor reorder the accesses. */
static u32  rd32(u32 addr)          { return *(volatile u32 *)addr; }
static void wr32(u32 addr, u32 v)   { *(volatile u32 *)addr = v; }
static u8   rd8 (u32 addr)          { return *(volatile u8  *)addr; }

/* A 64-bit register is written as two 32-bit halves; the low half must go
 * first, because several of these latch on the write to the high dword. */
static void wr64(u32 addr, u64 v)
{
    *(volatile u32 *)addr       = (u32)(v & 0xFFFFFFFFu);
    *(volatile u32 *)(addr + 4) = (u32)(v >> 32);
}

/* ---- stage 1: find the controller -------------------------------------- */
/* USB host controllers are PCI class 0x0C (serial bus), subclass 0x03 (USB),
 * and the programming interface says which kind: 0x30 is xHCI. That is how we
 * avoid grabbing an old UHCI/EHCI controller by mistake. */
int xhci_find(void)
{
    pci_scan();
    for (int i = 0; i < pci_count(); i++) {
        if (pci_class(i) != 0x0C) continue;
        if (pci_subclass(i) != 0x03) continue;
        xhci_idx = i;
        pci_enable(i);                 /* memory space + BUS MASTER: the
                                          controller DMAs, so mastering is
                                          not optional */
        xbase = pci_bar(i, 0);
        if (!xbase) { xhci_idx = -1; continue; }

        u8  caplen = rd8(xbase + XCAP_CAPLENGTH);
        u32 hcs1   = rd32(xbase + XCAP_HCSPARAMS1);
        u32 hcc1   = rd32(xbase + XCAP_HCCPARAMS1);

        /* CAPLENGTH is byte 0 and HCIVERSION is bytes 2-3, so a 32-bit read
         * at offset 2 is MISALIGNED and returns the wrong halves. Read the
         * aligned dword at 0 and take its top half. */
        xver     = (int)(rd32(xbase) >> 16);
        xop      = xbase + caplen;
        xdb      = xbase + (rd32(xbase + XCAP_DBOFF) & ~0x3u);
        xrt      = xbase + (rd32(xbase + XCAP_RTSOFF) & ~0x1Fu);
        xslots   = (int)(hcs1 & 0xFF);
        xports   = (int)((hcs1 >> 24) & 0xFF);
        /* CSZ: with it set every context structure is 64 bytes instead of 32.
         * Getting this wrong means every context field lands at the wrong
         * offset and nothing works, in a way that is very hard to see. */
        xctxsize = (hcc1 & (1u << 2)) ? 64 : 32;
        return i;
    }
    return -1;
}

int xhci_present(void)  { return xhci_idx >= 0 && xbase != 0; }
int xhci_version(void)  { return xver; }
int xhci_slots(void)    { return xslots; }
int xhci_ports(void)    { return xports; }
int xhci_ctx_size(void) { return xctxsize; }
u32 xhci_mmio(void)     { return xbase; }
int xhci_pci_index(void){ return xhci_idx; }

/* ---- stage 2: reset and start ------------------------------------------ */
/* The firmware was driving this controller a moment ago. Taking it over means
 * stopping it, resetting it, and waiting for it to say it is ready - the
 * hardware needs real time for that and will lie about its registers if you
 * read them too early, so every wait is bounded and checked. */
static int wait_clear(u32 addr, u32 mask, int spins)
{
    while (spins--) { if (!(rd32(addr) & mask)) return 1; }
    return 0;
}

int xhci_reset(void)
{
    if (!xhci_present()) return 0;

    /* stop it first - resetting a running controller is undefined */
    u32 cmd = rd32(xop + XOP_USBCMD);
    wr32(xop + XOP_USBCMD, cmd & ~USBCMD_RS);
    if (!wait_clear(xop + XOP_USBSTS, 0, 1)) { }        /* fallthrough */
    for (int i = 0; i < 1000000; i++)
        if (rd32(xop + XOP_USBSTS) & USBSTS_HCH) break; /* halted */

    /* host controller reset */
    wr32(xop + XOP_USBCMD, USBCMD_HCRST);
    if (!wait_clear(xop + XOP_USBCMD, USBCMD_HCRST, 2000000)) return 0;

    /* CNR - "controller not ready" - stays set while it reinitialises. Every
     * register read before this clears is meaningless. */
    if (!wait_clear(xop + XOP_USBSTS, USBSTS_CNR, 2000000)) return 0;

    /* tell it how many device slots we intend to use */
    wr32(xop + XOP_CONFIG, (u32)xslots);
    return 1;
}

int xhci_halted(void)  { return (rd32(xop + XOP_USBSTS) & USBSTS_HCH) ? 1 : 0; }
u32 xhci_usbsts(void)  { return rd32(xop + XOP_USBSTS); }
u32 xhci_usbcmd(void)  { return rd32(xop + XOP_USBCMD); }

/* Port status, so we can see what is actually plugged in. Bit 0 is "a device
 * is connected", bit 1 "port enabled"; bits [13:10] are the speed. */
u32 xhci_portsc(int port)
{
    if (!xhci_present() || port < 1 || port > xports) return 0;
    return rd32(xop + XOP_PORTSC(port));
}

int xhci_port_connected(int port) { return xhci_portsc(port) & 1; }
int xhci_port_enabled(int port)   { return (xhci_portsc(port) >> 1) & 1; }
int xhci_port_speed(int port)     { return (xhci_portsc(port) >> 10) & 0xF; }

/* how many ports currently have something attached */
int xhci_devices_attached(void)
{
    int n = 0;
    for (int p = 1; p <= xports; p++)
        if (xhci_port_connected(p)) n++;
    return n;
}

/* ==== stage 3: the rings ==================================================
 * This is where xHCI stops being "poke a register" and becomes a real DMA
 * engine. Three structures live in OUR memory and the controller reads them
 * directly, so their addresses, alignment and layout all have to be exactly
 * right - a wrong alignment does not fail loudly, it just never works.
 *
 *   DCBAA        an array of pointers, one per device slot, telling the
 *                controller where each device's context lives
 *   command ring TRBs we write and the controller executes (enable a slot,
 *                address a device, configure an endpoint)
 *   event ring   TRBs the CONTROLLER writes and we read - completions,
 *                port changes, transfer results
 *
 * A ring is a fixed array of 16-byte TRBs ending in a Link TRB that points
 * back to the start. The CYCLE BIT is how both sides agree on which entries
 * are new: producer and consumer each keep a cycle state, and an entry
 * belongs to the consumer only when its cycle bit matches. It flips every
 * time the ring wraps. Get this wrong and you either replay stale events
 * forever or see none at all.
 *
 * The buffers sit at fixed high addresses for the same reason the rest of
 * zlOS's scratch does: there is no allocator, and this memory must be
 * physically contiguous and identity mapped, which it is on every path.
 */
#define XMEM_DCBAA   0x0E000000u   /* 224 MiB: slot pointer array           */
#define XMEM_CMDRING 0x0E001000u   /* the command ring                       */
#define XMEM_EVTRING 0x0E002000u   /* the event ring                         */
#define XMEM_ERST    0x0E003000u   /* event ring segment table               */
#define XMEM_CTX     0x0E004000u   /* device + input contexts                */
#define XMEM_XFER    0x0E010000u   /* per-endpoint transfer rings            */
#define XMEM_DATA    0x0E020000u   /* DMA buffers for descriptors/reports    */

#define RING_TRBS    64            /* entries per ring, incl. the link TRB   */
#define TRB_BYTES    16

/* TRB types we use */
#define TRB_LINK              6
#define TRB_ENABLE_SLOT       9
#define TRB_ADDRESS_DEVICE   11
#define TRB_CONFIGURE_EP     12
#define TRB_NOOP_CMD         23
#define TRB_TRANSFER_EVENT   32
#define TRB_CMD_COMPLETION   33
#define TRB_PORT_STATUS      34

/* runtime (interrupter 0) registers, relative to the runtime block */
#define XRT_IMAN    0x20
#define XRT_IMOD    0x24
#define XRT_ERSTSZ  0x28
#define XRT_ERSTBA  0x30
#define XRT_ERDP    0x38

static u32 cmd_enqueue = 0;        /* our write index into the command ring */
static u32 cmd_cycle   = 1;        /* the cycle bit we produce with          */
static u32 evt_dequeue = 0;        /* our read index into the event ring     */
static u32 evt_cycle   = 1;        /* the cycle bit we expect to consume     */

static void zero_mem(u32 addr, u32 bytes)
{
    volatile u32 *p = (volatile u32 *)addr;
    for (u32 i = 0; i < bytes / 4; i++) p[i] = 0;
}

/* write one TRB: two dwords of parameter, one of status, one of control */
static void trb_write(u32 ring, u32 index, u64 param, u32 status, u32 control)
{
    volatile u32 *t = (volatile u32 *)(ring + index * TRB_BYTES);
    t[0] = (u32)(param & 0xFFFFFFFFu);
    t[1] = (u32)(param >> 32);
    t[2] = status;
    t[3] = control;                 /* written last - it carries the cycle bit
                                       that makes the whole TRB live */
}

int xhci_init_rings(void)
{
    if (!xhci_present()) return 0;

    /* ---- DCBAA: one 64-bit slot pointer per device, entry 0 reserved ---- */
    zero_mem(XMEM_DCBAA, 2048);
    wr64(xop + XOP_DCBAAP, (u64)XMEM_DCBAA);

    /* ---- command ring, with a Link TRB closing the loop ----------------- */
    zero_mem(XMEM_CMDRING, RING_TRBS * TRB_BYTES);
    /* the last entry points back to the first; TC=1 (bit 1) tells the
     * controller to flip its cycle state when it follows the link */
    trb_write(XMEM_CMDRING, RING_TRBS - 1, (u64)XMEM_CMDRING, 0,
              (TRB_LINK << 10) | (1u << 1) | cmd_cycle);
    cmd_enqueue = 0;
    cmd_cycle   = 1;
    /* CRCR also carries our initial cycle state in bit 0 */
    wr64(xop + XOP_CRCR, (u64)XMEM_CMDRING | 1u);

    /* ---- event ring: the segment, then the table describing it ---------- */
    zero_mem(XMEM_EVTRING, RING_TRBS * TRB_BYTES);
    zero_mem(XMEM_ERST, 64);
    volatile u32 *erst = (volatile u32 *)XMEM_ERST;
    erst[0] = XMEM_EVTRING;         /* segment base, low                     */
    erst[1] = 0;                    /* segment base, high                    */
    erst[2] = RING_TRBS;            /* how many TRBs in this segment         */
    erst[3] = 0;
    evt_dequeue = 0;
    evt_cycle   = 1;

    /* the dequeue pointer must be set BEFORE the table base, per the spec */
    wr64(xrt + XRT_ERDP, (u64)XMEM_EVTRING);
    wr32(xrt + XRT_ERSTSZ, 1);      /* one segment */
    wr64(xrt + XRT_ERSTBA, (u64)XMEM_ERST);

    /* ---- run ------------------------------------------------------------ */
    wr32(xop + XOP_USBCMD, rd32(xop + XOP_USBCMD) | USBCMD_RS);
    for (int i = 0; i < 1000000; i++)
        if (!(rd32(xop + XOP_USBSTS) & USBSTS_HCH)) return 1;   /* running */
    return 0;
}

int xhci_running(void) { return !(rd32(xop + XOP_USBSTS) & USBSTS_HCH); }

/* Ring the doorbell for a slot. Slot 0 target 0 is the COMMAND ring; any
 * other slot is that device's endpoint. This is the only way to tell the
 * controller "there is new work on a ring you own". */
static void doorbell(u32 slot, u32 target)
{
    wr32(xdb + slot * 4, target);
}

/* Put a command on the ring and ring the bell. */
static u32 cmd_submit(u64 param, u32 status, u32 type, u32 extra)
{
    u32 at = cmd_enqueue;
    trb_write(XMEM_CMDRING, at, param, status, (type << 10) | extra | cmd_cycle);
    cmd_enqueue++;
    if (cmd_enqueue >= RING_TRBS - 1) {     /* the link TRB is not usable */
        /* hand the link TRB to the controller with the current cycle, then
         * flip ours - that is what makes the wrap visible to it */
        trb_write(XMEM_CMDRING, RING_TRBS - 1, (u64)XMEM_CMDRING, 0,
                  (TRB_LINK << 10) | (1u << 1) | cmd_cycle);
        cmd_enqueue = 0;
        cmd_cycle ^= 1;
    }
    doorbell(0, 0);
    return at;
}

/* Wait for the controller to post an event whose cycle bit matches ours.
 * Returns the TRB type, or 0 if nothing arrived in time. */
static int event_poll(u32 *out_param_lo, u32 *out_status, u32 *out_ctrl, int spins)
{
    while (spins--) {
        volatile u32 *e = (volatile u32 *)(XMEM_EVTRING + evt_dequeue * TRB_BYTES);
        u32 ctrl = e[3];
        if ((ctrl & 1u) != evt_cycle) continue;      /* not ours yet */
        if (out_param_lo) *out_param_lo = e[0];
        if (out_status)   *out_status   = e[2];
        if (out_ctrl)     *out_ctrl     = ctrl;
        int type = (int)((ctrl >> 10) & 0x3F);

        evt_dequeue++;
        if (evt_dequeue >= RING_TRBS) { evt_dequeue = 0; evt_cycle ^= 1; }
        /* tell the controller how far we have consumed, and clear the
         * event handler busy bit (bit 3) while we are there */
        wr64(xrt + XRT_ERDP, (u64)(XMEM_EVTRING + evt_dequeue * TRB_BYTES) | (1u << 3));
        return type;
    }
    return 0;
}

/* Wait for a PARTICULAR kind of event, discarding the ones we are not waiting
 * for.
 *
 * This is not a nicety. The controller posts a Port Status Change Event every
 * time a port's state moves, and the moment anything is plugged in those
 * interleave freely with command completions - so the next event on the ring
 * is very often not yours. A driver that assumes otherwise works perfectly on
 * an empty bus and fails the instant a keyboard exists, which is exactly the
 * case we care about. */
static int event_wait(int want, u32 *param, u32 *status, u32 *ctrl, int spins)
{
    for (int i = 0; i < 32; i++) {
        int t = event_poll(param, status, ctrl, spins);
        if (t == 0)    return 0;        /* nothing arrived at all */
        if (t == want) return t;
        /* anything else - a port change, a stray transfer event - is not an
         * error, it is just not ours. Drop it and keep looking. */
    }
    return 0;
}

/* A no-op command is the cheapest possible end-to-end proof: we built a ring
 * the controller can read, it executed something from it, and it wrote a
 * completion back onto a ring we can read. If this returns 1, the whole DMA
 * path works. */
int xhci_test_noop(void)
{
    if (!xhci_running()) return 0;
    cmd_submit(0, 0, TRB_NOOP_CMD, 0);
    u32 status = 0, ctrl = 0;
    if (!event_wait(TRB_CMD_COMPLETION, 0, &status, &ctrl, 5000000)) return 0;
    /* completion code lives in the top byte of the status dword; 1 = success */
    return ((status >> 24) & 0xFF) == 1;
}

/* ==== stage 4: reset a port, address the device, read its identity =========
 * Everything above proved the controller works. This stage makes the DEVICE
 * work: bring the port up, ask the controller for a slot, build the data
 * structures that describe the device, and then run an actual USB control
 * transfer to read the descriptor that says what it is.
 *
 * PORTSC IS A TRAP. It mixes ordinary read/write bits with write-1-to-CLEAR
 * status bits, and PED (bit 1) is write-1-to-DISABLE. So the obvious
 * `reg |= PR; write(reg)` disables the port and silently eats every change
 * flag on the way past. Every write here goes through portsc_keep(), which
 * masks those bits out, and the bits we DO want to acknowledge are named
 * explicitly. Linux gets this wrong in bug reports more often than anything
 * else in xhci-hub.c.
 */
#define PORTSC_CCS   (1u << 0)
#define PORTSC_PED   (1u << 1)
#define PORTSC_PR    (1u << 4)
#define PORTSC_PP    (1u << 9)
#define PORTSC_CSC   (1u << 17)
#define PORTSC_PRC   (1u << 21)
#define PORTSC_RW1C  0x00FE0000u        /* CSC..CEC, all write-1-to-clear */

static u32 portsc_keep(int port)
{
    return xhci_portsc(port) & ~(PORTSC_RW1C | PORTSC_PED | PORTSC_PR);
}

/* Reset a port and wait for it to come back enabled.
 *
 * A USB2 device is NOT usable when it merely reports "connected" - the port
 * has to be reset before the device will answer to address 0. (USB3 ports do
 * their own link training and come up enabled by themselves, which is why the
 * enabled check below is the real test rather than the reset itself.) */
int xhci_port_reset(int port)
{
    if (!xhci_present() || port < 1 || port > xports) return 0;
    u32 reg = xop + XOP_PORTSC(port);
    if (!(rd32(reg) & PORTSC_CCS)) return 0;      /* nothing plugged in */

    wr32(reg, portsc_keep(port) | PORTSC_PP | PORTSC_PR);

    /* the hardware drives the reset for milliseconds; PR clears and PRC sets
     * when it is done. Bounded so a dead port cannot wedge the kernel. */
    for (int i = 0; i < 20000000; i++) {
        u32 s = rd32(reg);
        if (!(s & PORTSC_PR) && (s & PORTSC_PRC)) break;
    }
    /* acknowledge exactly the two change bits we care about */
    wr32(reg, portsc_keep(port) | PORTSC_PRC | PORTSC_CSC);

    /* USB 2.0 §7.1.7.5 gives a device 10 ms of "reset recovery" before it has
     * to answer anything. There is no timer in this driver, so this is a
     * bounded read of a register the hardware owns - it cannot be optimised
     * away and it costs real bus cycles. */
    for (int i = 0; i < 2000000; i++) (void)rd32(reg);

    return (rd32(reg) & PORTSC_PED) ? 1 : 0;
}

/* Ask the controller to allocate a device slot. The slot number comes back in
 * the top byte of the completion event's control dword. */
int xhci_enable_slot(void)
{
    if (!xhci_running()) return 0;
    cmd_submit(0, 0, TRB_ENABLE_SLOT, 0);
    u32 status = 0, ctrl = 0;
    if (!event_wait(TRB_CMD_COMPLETION, 0, &status, &ctrl, 5000000)) return 0;
    if (((status >> 24) & 0xFF) != 1) return 0;
    return (int)((ctrl >> 24) & 0xFF);
}

/* ---- contexts -----------------------------------------------------------
 * A "context" is a block of dwords describing either the device as a whole
 * (slot context) or one endpoint. The INPUT context is what we hand the
 * controller to say "make it look like this"; the DEVICE context is what the
 * controller maintains and we can read back.
 *
 * Their size is 32 OR 64 bytes depending on HCCPARAMS1.CSZ, which is why
 * every field goes through ctx_set() rather than a struct. Hardcode 32 on a
 * 64-byte controller and every field lands in the wrong place - and it fails
 * silently, which is the worst kind. */
#define CTX_INPUT   XMEM_CTX
#define CTX_DEVICE  (XMEM_CTX + 0x1000)
#define EP0_RING    XMEM_XFER

static void ctx_set(u32 base, int which, int dword, u32 val)
{
    *(volatile u32 *)(base + (u32)which * (u32)xctxsize + (u32)dword * 4) = val;
}

static u32 ctx_get(u32 base, int which, int dword)
{
    return *(volatile u32 *)(base + (u32)which * (u32)xctxsize + (u32)dword * 4);
}

/* EP0's maximum packet size is fixed by the link speed, and the device cannot
 * tell us until we can already talk to it - so the spec defines a starting
 * value per speed. Low and Full speed start at 8 and are corrected after the
 * first 8 bytes of the descriptor come back. */
static int ep0_mps(int speed)
{
    if (speed == 4) return 512;   /* SuperSpeed */
    if (speed == 3) return 64;    /* High speed */
    return 8;                     /* Low / Full  */
}

static u32 ep0_enqueue = 0;
static u32 ep0_cycle   = 1;
static int cur_slot    = 0;
static int cur_port    = 0;
static int cur_speed   = 0;

/* Build the input context and issue Address Device. After this the device has
 * a USB address and answers control transfers - it is a real, addressed device
 * on the bus. */
int xhci_address_device(int slot, int port, int speed)
{
    if (!xhci_running() || slot <= 0 || slot > xslots) return 0;

    /* the device context the controller will own, and the DCBAA entry that
     * tells it where to find it */
    zero_mem(CTX_DEVICE, 2048);
    volatile u32 *dcbaa = (volatile u32 *)XMEM_DCBAA;
    dcbaa[slot * 2]     = CTX_DEVICE;
    dcbaa[slot * 2 + 1] = 0;

    /* EP0's own transfer ring, closed with a link TRB like every other ring */
    zero_mem(EP0_RING, RING_TRBS * TRB_BYTES);
    trb_write(EP0_RING, RING_TRBS - 1, (u64)EP0_RING, 0,
              (TRB_LINK << 10) | (1u << 1) | 1u);
    ep0_enqueue = 0;
    ep0_cycle   = 1;

    zero_mem(CTX_INPUT, 2048);
    /* input control context: A0 = configure the slot, A1 = configure EP0 */
    ctx_set(CTX_INPUT, 0, 1, 0x3);

    /* slot context: route string 0 (we are on a root port), the link speed,
     * and "context entries = 1" meaning EP0 is the last one defined */
    ctx_set(CTX_INPUT, 1, 0, ((u32)speed << 20) | (1u << 27));
    ctx_set(CTX_INPUT, 1, 1, ((u32)port << 16));

    /* EP0 context: CErr=3 (retry three times), EP type 4 = control
     * bidirectional, and the max packet size for this speed */
    ctx_set(CTX_INPUT, 2, 1, (3u << 1) | (4u << 3) | ((u32)ep0_mps(speed) << 16));
    /* TR dequeue pointer, with DCS=1 to match the ring's initial cycle */
    ctx_set(CTX_INPUT, 2, 2, EP0_RING | 1u);
    ctx_set(CTX_INPUT, 2, 3, 0);
    ctx_set(CTX_INPUT, 2, 4, 8);            /* average TRB length */

    cmd_submit((u64)CTX_INPUT, 0, TRB_ADDRESS_DEVICE, (u32)slot << 24);
    u32 status = 0, ctrl = 0;
    if (!event_wait(TRB_CMD_COMPLETION, 0, &status, &ctrl, 5000000)) return 0;
    if (((status >> 24) & 0xFF) != 1) return 0;

    cur_slot = slot; cur_port = port; cur_speed = speed;
    return 1;
}

/* the USB address the controller assigned, read back out of ITS context */
int xhci_device_address(void)
{
    if (!cur_slot) return 0;
    return (int)(ctx_get(CTX_DEVICE, 0, 3) & 0xFF);
}

/* ---- control transfers ---------------------------------------------------
 * A USB control transfer is three TRBs on the endpoint's ring: the 8-byte
 * request, an optional data buffer, and a zero-length handshake in the
 * OPPOSITE direction to the data. Ring the doorbell and the controller runs
 * all three, then posts one transfer event. */
#define TRB_NORMAL  1
#define TRB_SETUP   2
#define TRB_DATA    3
#define TRB_STATUS  4

static void ep0_push(u64 param, u32 status, u32 control)
{
    trb_write(EP0_RING, ep0_enqueue, param, status, control | ep0_cycle);
    ep0_enqueue++;
    if (ep0_enqueue >= RING_TRBS - 1) {
        trb_write(EP0_RING, RING_TRBS - 1, (u64)EP0_RING, 0,
                  (TRB_LINK << 10) | (1u << 1) | ep0_cycle);
        ep0_enqueue = 0;
        ep0_cycle ^= 1;
    }
}

int xhci_control_in(int slot, u32 setup_lo, u32 setup_hi, u32 buf, int len)
{
    if (!xhci_running() || slot <= 0) return 0;
    if (len > 0) zero_mem(buf, (u32)((len + 3) & ~3));

    /* Setup stage. IDT (immediate data) means the eight request bytes ARE the
     * parameter field, not a pointer to them. TRT=3 declares an IN data stage. */
    ep0_push(((u64)setup_hi << 32) | (u64)setup_lo, 8,
             (TRB_SETUP << 10) | (1u << 6) | (len > 0 ? (3u << 16) : 0u));

    /* Data stage, DIR=1 for IN. Deliberately WITHOUT interrupt-on-short-packet:
     * one interrupt for the whole transfer means one event to match, and a
     * short descriptor is not an error we need to hear about separately. */
    if (len > 0)
        ep0_push((u64)buf, (u32)len, (TRB_DATA << 10) | (1u << 16));

    /* Status stage, in the OPPOSITE direction to the data, with IOC set so the
     * controller tells us the whole thing landed. */
    ep0_push(0, 0, (TRB_STATUS << 10) | (1u << 5) | (len > 0 ? 0u : (1u << 16)));

    doorbell((u32)slot, 1);           /* EP0 is doorbell target 1 */

    u32 status = 0, ctrl = 0;
    if (!event_wait(TRB_TRANSFER_EVENT, 0, &status, &ctrl, 5000000)) return 0;
    int cc = (int)((status >> 24) & 0xFF);
    return (cc == 1 || cc == 13) ? 1 : 0;   /* success, or a short packet */
}

/* ---- the actual identity of the device ---------------------------------- */
/* GET_DESCRIPTOR(DEVICE): bmRequestType 0x80 (device-to-host, standard),
 * bRequest 6, wValue 0x0100 (descriptor type 1, index 0), wLength 18. */
int xhci_get_device_descriptor(int slot)
{
    return xhci_control_in(slot, 0x01000680u, 0x00120000u, XMEM_DATA, 18);
}

int xhci_desc_byte(int i)
{
    if (i < 0 || i > 255) return 0;
    return (int)*(volatile u8 *)(XMEM_DATA + (u32)i);
}

static int desc16(int off)
{
    return xhci_desc_byte(off) | (xhci_desc_byte(off + 1) << 8);
}

int xhci_desc_vendor(void)   { return desc16(8);  }
int xhci_desc_product(void)  { return desc16(10); }
int xhci_desc_usbver(void)   { return desc16(2);  }
int xhci_desc_class(void)    { return xhci_desc_byte(4); }
int xhci_desc_mps0(void)     { return xhci_desc_byte(7); }
int xhci_desc_len(void)      { return xhci_desc_byte(0); }

/* ---- one call that does the whole dance for a port ----------------------
 * Returns the slot number on success, 0 on failure. This is what the shell
 * calls: it is the difference between "a controller exists" and "zlOS knows
 * what is plugged into port 5". */
int xhci_enumerate(int port)
{
    if (!xhci_port_connected(port)) return 0;

    /* USB3 ports self-enable; USB2 ports need the reset. Either way we need
     * PED set before the device will answer. */
    if (!xhci_port_enabled(port)) {
        if (!xhci_port_reset(port)) return 0;
    } else {
        xhci_port_reset(port);          /* harmless, and clears stale changes */
    }

    int speed = xhci_port_speed(port);
    int slot  = xhci_enable_slot();
    if (!slot) return 0;

    if (!xhci_address_device(slot, port, speed)) return 0;
    if (!xhci_get_device_descriptor(slot))       return 0;
    return slot;
}

int xhci_cur_slot(void)  { return cur_slot;  }
int xhci_cur_speed(void) { return cur_speed; }
