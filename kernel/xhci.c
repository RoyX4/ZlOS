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

/* An MMIO address is not always 32 bits wide.
 *
 * UEFI firmware puts 64-bit BARs above 4 GiB as a matter of routine - OVMF
 * maps QEMU's xHCI at 0xC000000000, and Intel's PCH xHCI is a 64-bit BAR too.
 * The 32-bit BIOS build cannot reach those and says so honestly; the 64-bit
 * build, which is the one that boots real hardware, can. Everything the
 * CONTROLLER reads from us still lives below 4 GiB, so the DMA addresses in
 * the rings stay 32-bit and no descriptor format changes. */
#if defined(ZL_64)
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

int  pci_count(void);
int  pci_class(int i);
int  pci_subclass(int i);
int  pci_prog_if(int i);
int  pci_vendor(int i);
int  pci_device(int i);
void pci_scan(void);
void pci_enable(int i);
u32  pci_bar(int i, int which);
u32  pci_bar_hi(int i, int which);
int  pci_bar_is64(int i, int which);
u32  pci_bar_size(int i, int which);
u32  idt_ticks(void);           /* the 100 Hz PIT counter, for real timeouts */

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
static uptr xbase    = 0;       /* BAR0 - the whole register block          */
static uptr xop      = 0;       /* operational registers                     */
static uptr xrt      = 0;       /* runtime registers                         */
static uptr xdb      = 0;       /* doorbell array                            */
static int  xslots   = 0;
static int  xports   = 0;
static int  xver     = 0;
static int  xctxsize = 32;      /* 32 or 64 bytes, from HCCPARAMS1.CSZ      */

/* MMIO. volatile because these are registers - the compiler must not cache
 * a value the hardware changes underneath it, nor reorder the accesses. */
static u32  rd32(uptr addr)         { return *(volatile u32 *)addr; }
static void wr32(uptr addr, u32 v)  { *(volatile u32 *)addr = v; }
static u8   rd8 (uptr addr)         { return *(volatile u8  *)addr; }

/* A 64-bit register is written as two 32-bit halves; the low half must go
 * first, because several of these latch on the write to the high dword. */
static void wr64(uptr addr, u64 v)
{
    *(volatile u32 *)addr       = (u32)(v & 0xFFFFFFFFu);
    *(volatile u32 *)(addr + 4) = (u32)(v >> 32);
}

/* ---- stage 1: find the controller -------------------------------------- */
/* USB host controllers are PCI class 0x0C (serial bus), subclass 0x03 (USB),
 * and the programming interface says which kind: 0x30 is xHCI. That is how we
 * avoid grabbing an old UHCI/EHCI controller by mistake. */
int xhci_present(void);   /* defined just below xhci_find */

/* Bounded wait, in real milliseconds wherever possible.
 *
 * The old code here spun a fixed iteration count - twenty million uncached
 * MMIO reads per port. That is not a timeout, it is a number that happened to
 * be large, and on a port that never completes reset it presents as a hang.
 * The PIT ticks at 100 Hz once interrupts are on, so use it; but this driver
 * can also run before that, and a wait that depends on a timer which is not
 * ticking is also a hang. So: whichever bound trips first wins. */
static int wait_bit(uptr addr, u32 mask, int want_set, int ms)
{
    u32  t0    = idt_ticks();
    u32  ticks = (u32)(ms / 10) + 1;          /* 10 ms per tick at 100 Hz */
    long spins = (long)ms * 50000;            /* fallback if the PIT is dead */

    while (spins-- > 0) {
        u32 v = rd32(addr) & mask;
        if (want_set ? (v == mask) : (v == 0)) return 1;
        if (idt_ticks() - t0 >= ticks) return 0;
    }
    return 0;
}

static int  xscratch  = 0;      /* scratchpad buffers the controller wants   */
static u32  xpagesize = 4096;
static u32  xecp      = 0;      /* offset of the extended capability list    */
static int  xbar_high = 0;      /* BAR sits above 4 GiB                      */

int xhci_find(void)
{
    pci_scan();
    for (int i = 0; i < pci_count(); i++) {
        if (pci_class(i)    != 0x0C) continue;   /* serial bus controller */
        if (pci_subclass(i) != 0x03) continue;   /* USB                   */
        /* The programming interface is what separates xHCI (0x30) from the
         * older UHCI (0x00), OHCI (0x10) and EHCI (0x20) controllers. Without
         * this check we would happily take an EHCI controller's BAR and then
         * read a completely different register layout out of it - every
         * offset wrong, and no obvious symptom. */
        if (pci_prog_if(i)  != 0x30) continue;

        pci_enable(i);                 /* memory space + BUS MASTER: the
                                          controller DMAs, so mastering is
                                          not optional */

        /* BAR0 is a 64-bit BAR on essentially every real xHCI controller, and
         * UEFI firmware puts it above 4 GiB (OVMF: 0xC000000000). Read both
         * halves; a 32-bit build cannot reach a high one and must say so
         * rather than silently using a truncated address. */
        u32 lo = pci_bar(i, 0);
        u32 hi = pci_bar_hi(i, 0);
        if (hi != 0 && sizeof(uptr) < 8) { xbar_high = 1; continue; }
        xbase = ((uptr)hi << 32) | (uptr)lo;
        if (!xbase) continue;

        xhci_idx = i;

        u8  caplen = rd8(xbase + XCAP_CAPLENGTH);
        u32 hcs1   = rd32(xbase + XCAP_HCSPARAMS1);
        u32 hcs2   = rd32(xbase + XCAP_HCSPARAMS2);
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
        /* xECP is a dword offset from BAR0 to the extended capability list */
        xecp     = (hcc1 >> 16) & 0xFFFF;

        /* MaxScratchpadBufs is split across HCSPARAMS2: high 5 bits at [25:21],
         * low 5 bits at [31:27]. QEMU asks for none, Intel hardware asks for
         * several dozen - see xhci_alloc_scratchpad(). */
        xscratch = (int)((((hcs2 >> 21) & 0x1F) << 5) | ((hcs2 >> 27) & 0x1F));

        /* PAGESIZE is a bitmap: bit n set means 2^(n+12) is supported. The
         * scratchpad buffers must be exactly this size. */
        u32 ps = rd32(xop + XOP_PAGESIZE) & 0xFFFF;
        xpagesize = 4096;
        for (int b = 0; b < 16; b++)
            if (ps & (1u << b)) { xpagesize = 1u << (b + 12); break; }

        return i;
    }
    return -1;
}

int xhci_bar_high(void) { return xbar_high; }
int xhci_scratchpads(void) { return xscratch; }

/* ---- taking the controller away from the firmware ----------------------
 * On a UEFI machine the firmware's USB stack is still armed when we arrive,
 * and it services the controller through System Management Interrupts. If we
 * reset the controller while SMM still owns it, the firmware's handler keeps
 * reading and ACKNOWLEDGING the events we are trying to read - so we lose
 * events at random and the driver looks flaky in a way no amount of staring at
 * our own code explains.
 *
 * The handshake is defined for exactly this: walk the extended capability list
 * to the USB Legacy Support capability, set the OS-owned bit, wait for the
 * firmware to drop the BIOS-owned bit, then disable every SMI it had enabled.
 * This must happen BEFORE the host controller reset. */
#define ECAP_ID_LEGACY   1
#define LEGACY_BIOS_OWNED (1u << 16)
#define LEGACY_OS_OWNED   (1u << 24)

int xhci_take_from_firmware(void)
{
    if (!xhci_present() || !xecp) return 1;      /* nothing to hand over */

    uptr cap = xbase + (uptr)xecp * 4;
    for (int guard = 0; guard < 64; guard++) {   /* the list is short; never trust it to end */
        u32 v  = rd32(cap);
        u32 id = v & 0xFF;
        u32 nx = (v >> 8) & 0xFF;

        if (id == ECAP_ID_LEGACY) {
            /* claim it */
            wr32(cap, rd32(cap) | LEGACY_OS_OWNED);

            /* give the firmware up to a second to let go - it is doing real
             * work inside SMM, and a spin count is not a unit of time */
            wait_bit(cap, LEGACY_BIOS_OWNED, 0, 1000);

            /* Whether or not it cooperated, force the issue: clear the BIOS
             * bit, then disable every SMI source in USBLEGCTLSTS (the next
             * dword) and clear the three write-1-to-clear SMI status bits at
             * the top. The mask keeps the reserved fields and zeroes the
             * enables - the same one Linux uses in pci-quirks.c. */
            wr32(cap, (rd32(cap) & ~LEGACY_BIOS_OWNED) | LEGACY_OS_OWNED);
            wr32(cap + 4, (rd32(cap + 4) & 0x000E1FEEu) | 0xE0000000u);
            return 1;
        }

        if (!nx) break;                          /* end of the list */
        cap += (uptr)nx * 4;
    }
    return 1;
}

int xhci_present(void)  { return xhci_idx >= 0 && xbase != 0; }
int xhci_version(void)  { return xver; }
int xhci_slots(void)    { return xslots; }
int xhci_ports(void)    { return xports; }
int xhci_ctx_size(void) { return xctxsize; }
u32 xhci_mmio(void)     { return (u32)xbase; }
int xhci_pci_index(void){ return xhci_idx; }

/* ---- stage 2: reset and start ------------------------------------------ */
/* The firmware was driving this controller a moment ago. Taking it over means
 * stopping it, resetting it, and waiting for it to say it is ready - the
 * hardware needs real time for that and will lie about its registers if you
 * read them too early, so every wait is bounded and checked. */
int xhci_reset(void)
{
    if (!xhci_present()) return 0;

    /* Ownership BEFORE reset. Resetting a controller that SMM still owns is
     * the one ordering mistake that makes everything after it unreliable. */
    xhci_take_from_firmware();

    /* stop it first - resetting a running controller is undefined */
    wr32(xop + XOP_USBCMD, rd32(xop + XOP_USBCMD) & ~USBCMD_RS);
    wait_bit(xop + XOP_USBSTS, USBSTS_HCH, 1, 100);     /* halted */

    /* host controller reset */
    wr32(xop + XOP_USBCMD, USBCMD_HCRST);
    if (!wait_bit(xop + XOP_USBCMD, USBCMD_HCRST, 0, 1000)) return 0;

    /* CNR - "controller not ready" - stays set while it reinitialises. Every
     * register read before this clears is meaningless. */
    if (!wait_bit(xop + XOP_USBSTS, USBSTS_CNR, 0, 1000)) return 0;

    /* tell it how many device slots we intend to use */
    wr32(xop + XOP_CONFIG, (u32)xslots);
    return 1;
}

int xhci_halted(void)  { return xhci_present() ? ((rd32(xop + XOP_USBSTS) & USBSTS_HCH) ? 1 : 0) : 1; }
u32 xhci_usbsts(void)  { return xhci_present() ? rd32(xop + XOP_USBSTS) : 0; }
u32 xhci_usbcmd(void)  { return xhci_present() ? rd32(xop + XOP_USBCMD) : 0; }

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
#define XMEM_SCRATCH_ARR 0x0E030000u /* array of scratchpad buffer pointers  */
#define XMEM_SCRATCH 0x0E040000u   /* the scratchpad pages themselves        */
#define SCRATCH_MAX  64            /* entries the pointer array has room for */
#define SCRATCH_BYTES 0x00400000u  /* 4 MiB reserved for scratchpad pages    */

#define RING_TRBS    64            /* entries per ring, incl. the link TRB   */
#define TRB_BYTES    16

/* TRB types we use */
#define TRB_LINK              6
#define TRB_ENABLE_SLOT       9
#define TRB_ADDRESS_DEVICE   11
#define TRB_CONFIGURE_EP     12
#define TRB_EVALUATE_CTX     13
#define TRB_RESET_ENDPOINT   14
#define TRB_DISABLE_SLOT     10
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

/* Write a pattern to the top and bottom of our DMA arena and read it back.
 * Absent RAM either reads back as zeroes/ones or wraps to a lower address, so
 * two different patterns at two addresses catches both. This is cheap and it
 * turns an invisible failure into an honest one. */
int xhci_ram_ok(void)
{
    volatile u32 *lo = (volatile u32 *)XMEM_DCBAA;
    volatile u32 *hi = (volatile u32 *)(XMEM_DATA + 0x800);
    *lo = 0xA5A5F00Du;
    *hi = 0x5A5A0FF0u;
    if (*lo != 0xA5A5F00Du) return 0;
    if (*hi != 0x5A5A0FF0u) return 0;
    *lo = 0; *hi = 0;
    return 1;
}

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

    /* ---- does this memory actually exist? -------------------------------
     * Everything below hands the controller physical addresses at 224 MiB and
     * up. There is no allocator and no memory map, so if the machine has less
     * RAM than that, every write goes into the void and every read comes back
     * as garbage - and the failure looks exactly like a driver bug, which cost
     * real debugging time to learn. Prove the memory is there first. */
    if (!xhci_ram_ok()) return 0;

    /* ---- DCBAA: one 64-bit slot pointer per device ----------------------
     * Entry 0 is NOT spare. If the controller reports MaxScratchpadBufs, it
     * needs that many page-sized buffers of its own, and entry 0 must point at
     * an array of their addresses. QEMU asks for none, which is why leaving it
     * zero worked there; Intel's PCH controllers ask for several dozen, and
     * with a null pointer they DMA their internal state to physical address 0
     * or raise a host system error. This is the single biggest difference
     * between "works in the emulator" and "works on the laptop". */
    zero_mem(XMEM_DCBAA, 2048);
    if (xscratch > 0) {
        if (xscratch > SCRATCH_MAX) return 0;      /* more than we reserved */
        if ((u32)xscratch * xpagesize > SCRATCH_BYTES) return 0;
        volatile u32 *sp = (volatile u32 *)XMEM_SCRATCH_ARR;
        zero_mem(XMEM_SCRATCH_ARR, 1024);
        for (int i = 0; i < xscratch; i++) {
            u32 page = XMEM_SCRATCH + (u32)i * xpagesize;
            zero_mem(page, xpagesize);
            sp[i * 2]     = page;                   /* low half  */
            sp[i * 2 + 1] = 0;                      /* high half */
        }
        volatile u32 *dcbaa = (volatile u32 *)XMEM_DCBAA;
        dcbaa[0] = XMEM_SCRATCH_ARR;
        dcbaa[1] = 0;
    }
    wr64(xop + XOP_DCBAAP, (u64)XMEM_DCBAA);

    /* ---- command ring, with a Link TRB closing the loop ----------------- */
    zero_mem(XMEM_CMDRING, RING_TRBS * TRB_BYTES);
    /* the last entry points back to the first; TC=1 (bit 1) tells the
     * controller to flip its cycle state when it follows the link */
    cmd_enqueue = 0;
    cmd_cycle   = 1;              /* set BEFORE the link TRB is written, or a
                                     re-init lays it down with a stale cycle */
    trb_write(XMEM_CMDRING, RING_TRBS - 1, (u64)XMEM_CMDRING, 0,
              (TRB_LINK << 10) | (1u << 1) | cmd_cycle);
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

int xhci_running(void)
{
    /* Guarded: xop is 0 until the controller is found, and an unguarded read
     * here dereferences physical address 4. Worse, xhci_test_noop() is
     * shell-callable and would then ring a doorbell at address 0 - a silent
     * write into low physical memory on a flat identity-mapped kernel. */
    if (!xhci_present()) return 0;
    return !(rd32(xop + XOP_USBSTS) & USBSTS_HCH);
}

/* Ring the doorbell for a slot. Slot 0 target 0 is the COMMAND ring; any
 * other slot is that device's endpoint. This is the only way to tell the
 * controller "there is new work on a ring you own". */
static void doorbell(u32 slot, u32 target)
{
    wr32(xdb + slot * 4, target);
}

/* Put a command on the ring and ring the bell. Returns the ADDRESS of the TRB
 * we wrote - a Command Completion Event carries that address in its parameter
 * field, which is the only reliable way to know a completion is ours. */
static u32 cmd_submit(u64 param, u32 status, u32 type, u32 extra)
{
    u32 at = cmd_enqueue;
    u32 trb_addr = XMEM_CMDRING + at * TRB_BYTES;
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
    return trb_addr;
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
static void kbd_event(u32 status, u32 ctrl);    /* stage 5, defined below */

/* Wait for the completion of ONE SPECIFIC command.
 *
 * Matching on "the next command completion" is a trap. If an earlier command
 * timed out, its completion arrives later and the next waiter consumes it -
 * reporting success for a command whose result it never saw, and staying one
 * event out of step forever. A Command Completion Event carries the address of
 * the Command TRB that produced it, so match on that and nothing else. */
static int cmd_wait(u32 trb_addr, u32 *status, u32 *ctrl, int spins)
{
    for (int i = 0; i < 32; i++) {
        u32 p = 0, s = 0, c = 0;
        int t = event_poll(&p, &s, &c, spins);
        if (t == 0) return 0;
        if (t == TRB_TRANSFER_EVENT) { kbd_event(s, c); continue; }
        if (t != TRB_CMD_COMPLETION) continue;      /* port change etc */
        if (p != trb_addr) continue;                /* a stale completion */
        if (status) *status = s;
        if (ctrl)   *ctrl   = c;
        return 1;
    }
    return 0;
}

static int event_wait(int want, u32 *param, u32 *status, u32 *ctrl, int spins)
{
    for (int i = 0; i < 32; i++) {
        u32 p = 0, s = 0, c = 0;
        int t = event_poll(&p, &s, &c, spins);
        if (t == 0) return 0;                   /* nothing arrived at all */
        if (t == want) {
            if (param)  *param  = p;
            if (status) *status = s;
            if (ctrl)   *ctrl   = c;
            return t;
        }
        /* A keypress that lands while we are waiting on a command must NOT be
         * thrown away: dropping it also drops our obligation to post another
         * buffer, and the keyboard goes silent forever. Hand it to the
         * keyboard code and carry on waiting. Port-change events genuinely are
         * noise here and are discarded. */
        if (t == TRB_TRANSFER_EVENT) kbd_event(s, c);
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
    u32 trb = cmd_submit(0, 0, TRB_NOOP_CMD, 0);
    u32 status = 0, ctrl = 0;
    if (!cmd_wait(trb, &status, &ctrl, 5000000)) return 0;
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

/* Build a PORTSC value safe to write back.
 *
 * A blacklist here is a standing hazard: it preserves every bit nobody thought
 * about, including LWS (16) and WPR (31), which are write-triggers. Keep only
 * the fields that are genuinely read/write state - port power, link state,
 * indicators and wake enables - and let everything else write as zero. */
#define PORTSC_WRITE_KEEP  ( (0xFu << 5)   /* PLS  */ \
                           | (1u   << 9)   /* PP   */ \
                           | (0x3u << 14)  /* PIC  */ \
                           | (0x7u << 25)) /* WCE/WDE/WOE */
static u32 portsc_keep(int port)
{
    return xhci_portsc(port) & PORTSC_WRITE_KEEP;
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
    /* uptr, NOT u32. On a UEFI machine xop lives above 4 GiB, and truncating
     * it here sent every port write into low RAM instead of the controller -
     * reads through xhci_portsc() looked perfect while the port never moved. */
    uptr reg = xop + XOP_PORTSC(port);
    if (!(rd32(reg) & PORTSC_CCS)) return 0;      /* nothing plugged in */

    wr32(reg, portsc_keep(port) | PORTSC_PP | PORTSC_PR);

    /* The hardware drives the reset for tens of milliseconds. 500 ms is far
     * past any real device and still a bound the kernel cannot hang on. */
    wait_bit(reg, PORTSC_PR, 0, 500);

    /* Acknowledge EVERY pending change bit, not just the two we came for.
     * A change bit only raises an event on its 0->1 edge, so one left set is
     * one condition we are never told about again on that port. */
    wr32(reg, portsc_keep(port) | PORTSC_RW1C);

    /* USB 2.0 s7.1.7.5: a device gets 10 ms of reset recovery before it has to
     * answer anything. Ask for 20 and let the PIT measure it. */
    wait_bit(reg, 0xFFFFFFFFu, 1, 20);   /* never satisfied - a pure delay */

    return (rd32(reg) & PORTSC_PED) ? 1 : 0;
}

/* Ask the controller to allocate a device slot. The slot number comes back in
 * the top byte of the completion event's control dword. */
int xhci_enable_slot(void)
{
    if (!xhci_running()) return 0;
    u32 trb = cmd_submit(0, 0, TRB_ENABLE_SLOT, 0);
    u32 status = 0, ctrl = 0;
    if (!cmd_wait(trb, &status, &ctrl, 5000000)) return 0;
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
#define MAX_SLOTS   16                       /* how many devices we track      */
#define CTX_INPUT   XMEM_CTX                 /* scratch: rebuilt per command   */
#define CTX_DEVICE(s) (XMEM_CTX + 0x1000 + (u32)(s) * 0x800)

/* Every endpoint needs its OWN ring. The controller stores a dequeue pointer
 * per endpoint inside that endpoint's context, so two endpoints sharing one
 * ring will chase each other's pointer and corrupt both. One ring per slot for
 * EP0, one per slot for the interrupt endpoint. */
#define RING_STRIDE    0x400                 /* RING_TRBS * TRB_BYTES = 1024   */
#define EP0_RING(s)    (XMEM_XFER + (u32)(s) * RING_STRIDE)
/* 32 KiB apart, not 16: EP0_RING(16) would otherwise land exactly on
 * INT_RING(0) and raising MAX_SLOTS would alias them with no diagnostic. */
#define INT_RING(s)    (XMEM_XFER + 0x8000 + (u32)(s) * RING_STRIDE)

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

static u32 ep0_enqueue[MAX_SLOTS];
static u32 ep0_cycle[MAX_SLOTS];
static int cur_slot    = 0;
static int cur_port    = 0;
static int cur_speed   = 0;

/* Lay a fresh ring down: all zeroes, with a Link TRB in the last slot pointing
 * back at the start and TC set so the controller flips its cycle on the wrap. */
static void ring_init(u32 ring)
{
    zero_mem(ring, RING_TRBS * TRB_BYTES);
    trb_write(ring, RING_TRBS - 1, (u64)ring, 0,
              (TRB_LINK << 10) | (1u << 1) | 1u);
}

/* Build the input context and issue Address Device. After this the device has
 * a USB address and answers control transfers - it is a real, addressed device
 * on the bus. */
int xhci_address_device(int slot, int port, int speed)
{
    if (!xhci_running() || slot <= 0 || slot > xslots) return 0;
    if (slot >= MAX_SLOTS) return 0;         /* more devices than we track */

    /* the device context the controller will own, and the DCBAA entry that
     * tells it where to find it */
    zero_mem(CTX_DEVICE(slot), 2048);
    volatile u32 *dcbaa = (volatile u32 *)XMEM_DCBAA;
    dcbaa[slot * 2]     = CTX_DEVICE(slot);
    dcbaa[slot * 2 + 1] = 0;

    /* EP0's own transfer ring - one per slot, never shared */
    ring_init(EP0_RING(slot));
    ep0_enqueue[slot] = 0;
    ep0_cycle[slot]   = 1;

    zero_mem(CTX_INPUT, 33u * (u32)xctxsize);
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
    ctx_set(CTX_INPUT, 2, 2, EP0_RING(slot) | 1u);
    ctx_set(CTX_INPUT, 2, 3, 0);
    ctx_set(CTX_INPUT, 2, 4, 8);            /* average TRB length */

    u32 trb = cmd_submit((u64)CTX_INPUT, 0, TRB_ADDRESS_DEVICE, (u32)slot << 24);
    u32 status = 0, ctrl = 0;
    if (!cmd_wait(trb, &status, &ctrl, 5000000)) return 0;
    if (((status >> 24) & 0xFF) != 1) return 0;

    cur_slot = slot; cur_port = port; cur_speed = speed;
    return 1;
}

/* the USB address the controller assigned, read back out of ITS context */
int xhci_device_address(void)
{
    if (!cur_slot) return 0;
    return (int)(ctx_get(CTX_DEVICE(cur_slot), 0, 3) & 0xFF);
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

static int reset_endpoint(int slot, int dci);   /* defined with the commands */

static void ep0_push(int slot, u64 param, u32 status, u32 control)
{
    u32 ring = EP0_RING(slot);
    trb_write(ring, ep0_enqueue[slot], param, status, control | ep0_cycle[slot]);
    ep0_enqueue[slot]++;
    if (ep0_enqueue[slot] >= RING_TRBS - 1) {
        /* hand the link TRB over with the CURRENT cycle, then flip ours - that
         * is what makes the wrap visible to the controller */
        trb_write(ring, RING_TRBS - 1, (u64)ring, 0,
                  (TRB_LINK << 10) | (1u << 1) | ep0_cycle[slot]);
        ep0_enqueue[slot] = 0;
        ep0_cycle[slot] ^= 1;
    }
}

int xhci_control_in(int slot, u32 setup_lo, u32 setup_hi, u32 buf, int len)
{
    if (!xhci_running() || slot <= 0 || slot >= MAX_SLOTS) return 0;
    if (len > 0) zero_mem(buf, (u32)((len + 3) & ~3));

    /* Setup stage. IDT (immediate data) means the eight request bytes ARE the
     * parameter field, not a pointer to them. TRT=3 declares an IN data stage. */
    ep0_push(slot, ((u64)setup_hi << 32) | (u64)setup_lo, 8,
             (TRB_SETUP << 10) | (1u << 6) | (len > 0 ? (3u << 16) : 0u));

    /* Data stage, DIR=1 for IN. Deliberately WITHOUT interrupt-on-short-packet:
     * one interrupt for the whole transfer means one event to match, and a
     * short descriptor is not an error we need to hear about separately. */
    if (len > 0)
        ep0_push(slot, (u64)buf, (u32)len, (TRB_DATA << 10) | (1u << 16));

    /* Status stage, in the OPPOSITE direction to the data, with IOC set so the
     * controller tells us the whole thing landed. */
    ep0_push(slot, 0, 0, (TRB_STATUS << 10) | (1u << 5) | (len > 0 ? 0u : (1u << 16)));

    doorbell((u32)slot, 1);           /* EP0 is doorbell target 1 */

    u32 status = 0, ctrl = 0;
    if (!event_wait(TRB_TRANSFER_EVENT, 0, &status, &ctrl, 5000000)) return 0;
    int cc = (int)((status >> 24) & 0xFF);
    if (cc == 1 || cc == 13) return 1;      /* success, or a short packet */

    /* Stall (6) or transfer error (4) leaves the endpoint halted. Clear it
     * here so the NEXT request works - a device is allowed to refuse an
     * optional request, and that must not be fatal. */
    if (cc == 6 || cc == 4) reset_endpoint(slot, 1);
    return 0;
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
    if (i < 0 || i >= 18) return 0;     /* only the device descriptor is here */
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

/* ---- M2: hand a slot back ----------------------------------------------
 * Enable Slot consumes one of the controller's finite slots. Every failed
 * enumeration that does not give it back leaks one, and after xslots failures
 * the bus is dead until a full reset. */
int xhci_disable_slot(int slot)
{
    if (!xhci_running() || slot <= 0) return 0;
    u32 trb = cmd_submit(0, 0, TRB_DISABLE_SLOT, (u32)slot << 24);
    u32 status = 0;
    if (!cmd_wait(trb, &status, 0, 2000000)) return 0;
    return ((status >> 24) & 0xFF) == 1;
}

/* ---- M3: recover an endpoint the controller has halted ------------------
 * A STALL or a transfer error puts the endpoint in the Halted state, where it
 * ignores every doorbell. Without this, one refused optional request - and
 * SET_IDLE is explicitly optional - takes the control endpoint down for good,
 * and every later transfer burns its whole timeout before failing. */
static int reset_endpoint(int slot, int dci)
{
    if (!xhci_running() || slot <= 0) return 0;
    u32 trb = cmd_submit(0, 0, TRB_RESET_ENDPOINT,
                         ((u32)slot << 24) | ((u32)dci << 16));
    u32 status = 0;
    if (!cmd_wait(trb, &status, 0, 2000000)) return 0;
    if (((status >> 24) & 0xFF) != 1) return 0;

    /* The endpoint restarts at whatever we tell it, so point it back at the
     * head of its ring and re-sync our own producer state with it. */
    u32 ring = EP0_RING(slot);
    ring_init(ring);
    ep0_enqueue[slot] = 0;
    ep0_cycle[slot]   = 1;

    u32 trb2 = cmd_submit((u64)(ring | 1u), 0, 16 /* Set TR Dequeue Ptr */,
                          ((u32)slot << 24) | ((u32)dci << 16));
    u32 st2 = 0;
    if (!cmd_wait(trb2, &st2, 0, 2000000)) return 0;
    return ((st2 >> 24) & 0xFF) == 1;
}

/* ---- H4: correct EP0's max packet size for low and full speed -----------
 * The packet size EP0 uses is fixed by the link speed, and at low/full speed
 * the spec says to start at 8 and ask the device. Most full-speed HID devices
 * actually want 64. If we leave the context saying 8 and then request all 18
 * descriptor bytes, the device answers with a packet larger than the
 * controller was told to expect and the transfer dies with a babble error -
 * so the device simply never enumerates.
 *
 * QEMU's usb-kbd genuinely uses 8, which is exactly why this was invisible in
 * emulation and would have failed on real hardware. */
static int fix_ep0_packet_size(int slot, int speed)
{
    if (speed >= 3) return 1;                 /* high speed and up are fixed */

    /* eight bytes is all we are allowed to assume we can read */
    if (!xhci_control_in(slot, 0x01000680u, 0x00080000u, XMEM_DATA, 8)) return 0;
    int real_mps = (int)*(volatile u8 *)(XMEM_DATA + 7);
    if (real_mps <= 0) return 0;
    if (real_mps == ep0_mps(speed)) return 1;  /* already right */

    /* Evaluate Context edits a live device: A1 alone means "only EP0 changed" */
    zero_mem(CTX_INPUT, 33u * (u32)xctxsize);
    ctx_set(CTX_INPUT, 0, 1, (1u << 1));
    ctx_set(CTX_INPUT, 2, 1, (3u << 1) | (4u << 3) | ((u32)real_mps << 16));

    u32 trb = cmd_submit((u64)CTX_INPUT, 0, TRB_EVALUATE_CTX, (u32)slot << 24);
    u32 status = 0;
    if (!cmd_wait(trb, &status, 0, 2000000)) return 0;
    return ((status >> 24) & 0xFF) == 1;
}

/* ---- one call that does the whole dance for a port ----------------------
 * Returns the slot number on success, 0 on failure. This is what the shell
 * calls: it is the difference between "a controller exists" and "zlOS knows
 * what is plugged into port 5". */
static int port_slot[32];       /* remembers what we already brought up */

int xhci_enumerate(int port)
{
    if (!xhci_port_connected(port)) return 0;

    /* Enumeration is not idempotent at the hardware level - asking twice
     * allocates a second slot for the same physical device and leaks the
     * first. Remember the answer instead. */
    if (port > 0 && port < 32 && port_slot[port]) return port_slot[port];

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

    if (!xhci_address_device(slot, port, speed)) { xhci_disable_slot(slot); return 0; }
    if (!fix_ep0_packet_size(slot, speed))       { xhci_disable_slot(slot); return 0; }
    if (!xhci_get_device_descriptor(slot))       { xhci_disable_slot(slot); return 0; }

    if (port > 0 && port < 32) port_slot[port] = slot;
    return slot;
}


int xhci_cur_slot(void)  { return cur_slot;  }
int xhci_cur_speed(void) { return cur_speed; }

/* ==== stage 5: the HID boot protocol - actual keystrokes ==================
 * Everything so far has been about the controller and the bus. This stage is
 * about the KEYBOARD: find the interface that speaks HID, tell it to use the
 * boot protocol, open its interrupt endpoint, and read the eight-byte reports
 * it sends every time a key moves.
 *
 * The boot protocol exists precisely for this situation. A general HID device
 * describes its own report format in a report descriptor that you then have to
 * parse - a genuinely large job. But every keyboard is required to also
 * support a fixed 8-byte layout so that a BIOS can use it before any of that
 * machinery exists:
 *
 *     byte 0   modifier bitmap (ctrl/shift/alt/gui, left and right)
 *     byte 1   reserved
 *     bytes 2-7  up to six HID usage IDs, for the keys currently held down
 *
 * It is a list of what is DOWN, not a stream of events. Working out that a key
 * was newly pressed means diffing against the previous report - which is also
 * what stops one held key repeating forever.
 */
#define CFG_BUF      (XMEM_DATA + 0x100)     /* configuration descriptor       */
#define KBD_REPORT   (XMEM_DATA + 0x400)     /* the 8 bytes from the keyboard  */
#define CFG_MAX      256

/* USB descriptor types and the class triple that identifies a boot keyboard */
#define DESC_CONFIG      2
#define DESC_INTERFACE   4
#define DESC_ENDPOINT    5
#define CLASS_HID        3
#define SUBCLASS_BOOT    1
#define PROTOCOL_KEYBOARD 1

static int kbd_slot  = 0;      /* the device slot the keyboard lives in       */
static int kbd_iface = 0;      /* which interface of it is the keyboard       */
static int kbd_dci   = 0;      /* device context index of its IN endpoint     */
static int kbd_mps   = 8;      /* max packet size of that endpoint            */
static int kbd_ready = 0;
static u32 kbd_enq   = 0;
static u32 kbd_cyc   = 1;

static u8  prev_keys[6];
static u8  kbd_mods = 0;       /* modifier bitmap from the most recent report */
static int keyq[32];
static int keyq_head = 0, keyq_tail = 0;

static void keyq_push(int ch)
{
    int next = (keyq_tail + 1) & 31;
    if (next == keyq_head) return;          /* full - drop, never overwrite */
    keyq[keyq_tail] = ch;
    keyq_tail = next;
}

static int keyq_pop(void)
{
    if (keyq_head == keyq_tail) return 0;
    int ch = keyq[keyq_head];
    keyq_head = (keyq_head + 1) & 31;
    return ch;
}

/* ---- the EVENT queue, which is the one input.c reads --------------------
 * A character queue cannot carry an arrow key. There is no character for Up,
 * so the only honest answer hid_to_ascii() can give is 0, and 0 already means
 * "nothing was typed" - so the key vanishes with no error anywhere. That is
 * exactly the bug this queue exists to fix: it carries the HID usage ID, which
 * every key has, and lets input.c decide what it means.
 *
 * Translation deliberately does NOT happen here. Which keymap, whether caps
 * lock is on, whether ctrl folds a letter into a control code - that is policy,
 * input.c already owns all of it for PS/2, and duplicating it here is how the
 * two keyboards drift apart until "it works on the laptop but not the external
 * one". The transport reports what the hardware said and stops there.
 *
 *     bits  7:0   HID usage ID   (never 0 for a real key, so 0 = queue empty)
 *     bits 15:8   HID modifier bitmap from the same report
 *     bit  16     1 = press, 0 = release
 */
#define KEV(press, mods, usage) (((press) << 16) | ((mods) << 8) | (usage))

static int kevq[32];
static int kevq_head = 0, kevq_tail = 0;

static void kevq_push(int ev)
{
    int next = (kevq_tail + 1) & 31;
    if (next == kevq_head) return;
    kevq[kevq_tail] = ev;
    kevq_tail = next;
}

static int kevq_pop(void)
{
    if (kevq_head == kevq_tail) return 0;
    int ev = kevq[kevq_head];
    kevq_head = (kevq_head + 1) & 31;
    return ev;
}

static u8 cfg_byte(int i)
{
    if (i < 0 || i >= CFG_MAX) return 0;
    return *(volatile u8 *)(CFG_BUF + (u32)i);
}

/* GET_DESCRIPTOR(CONFIGURATION). The first nine bytes carry wTotalLength,
 * which is how much there really is - interfaces and endpoints follow the
 * configuration header in one contiguous block. */
static int get_config(int slot, int len)
{
    if (len > CFG_MAX) len = CFG_MAX;
    return xhci_control_in(slot, 0x02000680u, ((u32)len << 16), CFG_BUF, len);
}

/* SET_CONFIGURATION: host-to-device, standard, no data stage. Until this runs
 * the device is "addressed" but not "configured" and its endpoints are dead. */
static int set_configuration(int slot, int cfgval)
{
    return xhci_control_in(slot, 0x00000900u | ((u32)cfgval << 16), 0, 0, 0);
}

/* SET_PROTOCOL(boot): class request to the interface. wValue 0 = boot,
 * 1 = report. This is what guarantees the fixed 8-byte layout. */
static int set_boot_protocol(int slot, int iface)
{
    return xhci_control_in(slot, 0x00000B21u, (u32)iface, 0, 0);
}

/* SET_IDLE(0): stop the keyboard re-sending an unchanged report on a timer.
 * Without it a held key floods the interrupt endpoint. Not fatal if the device
 * refuses it, so the result is deliberately ignored by the caller. */
static int set_idle(int slot, int iface)
{
    return xhci_control_in(slot, 0x00000A21u, (u32)iface, 0, 0);
}

/* xHCI states a polling interval as 125us * 2^Interval. USB states it
 * differently depending on speed - microframes for high speed, whole
 * milliseconds for low and full speed - so the conversion is not the same. */
static int interval_encode(int speed, int binterval)
{
    if (binterval < 1) binterval = 1;
    if (speed >= 3) {                       /* high speed and above */
        int iv = binterval - 1;
        if (iv > 15) iv = 15;
        return iv;
    }
    /* low/full speed: bInterval counts FRAMES, and a frame is eight 125us
     * microframes, so scale up and take the log */
    int micro = binterval * 8;
    int iv = 0;
    while (iv < 15 && (1 << (iv + 1)) <= micro) iv++;
    return iv;
}

/* Add the keyboard's interrupt IN endpoint to the device the controller
 * already knows about. Configure Endpoint is an incremental edit: the add
 * flags say which contexts in the input block are meaningful, and everything
 * else is left exactly as it is. */
static int configure_endpoint(int slot, int dci, int mps, int speed, int binterval)
{
    ring_init(INT_RING(slot));
    kbd_enq = 0;
    kbd_cyc = 1;

    zero_mem(CTX_INPUT, 33u * (u32)xctxsize);
    /* A0 (the slot context, because Context Entries changes) plus this
     * endpoint. Bit N of the add flags means device context index N. */
    ctx_set(CTX_INPUT, 0, 1, (1u << 0) | (1u << dci));

    /* copy the slot context forward, but raise Context Entries to cover the
     * new endpoint - the controller uses it as "how many contexts are valid" */
    u32 slot_dw0 = ctx_get(CTX_DEVICE(slot), 0, 0);
    slot_dw0 = (slot_dw0 & 0x07FFFFFFu) | ((u32)dci << 27);
    ctx_set(CTX_INPUT, 1, 0, slot_dw0);
    ctx_set(CTX_INPUT, 1, 1, ctx_get(CTX_DEVICE(slot), 0, 1));

    /* the endpoint itself. Input context index is device context index + 1,
     * because index 0 of the input block is the input control context. */
    int ic = dci + 1;
    ctx_set(CTX_INPUT, ic, 0, (u32)interval_encode(speed, binterval) << 16);
    /* CErr=3, EP type 7 = Interrupt IN, and the packet size the descriptor
     * asked for */
    ctx_set(CTX_INPUT, ic, 1, (3u << 1) | (7u << 3) | ((u32)mps << 16));
    ctx_set(CTX_INPUT, ic, 2, INT_RING(slot) | 1u);   /* dequeue ptr, DCS=1 */
    ctx_set(CTX_INPUT, ic, 3, 0);
    /* average TRB length, and Max ESIT Payload in the high half - the most
     * this endpoint can move in one service interval */
    ctx_set(CTX_INPUT, ic, 4, (u32)mps | ((u32)mps << 16));

    u32 trb = cmd_submit((u64)CTX_INPUT, 0, TRB_CONFIGURE_EP, (u32)slot << 24);
    u32 status = 0, ctrl = 0;
    if (!cmd_wait(trb, &status, &ctrl, 5000000)) return 0;
    return ((status >> 24) & 0xFF) == 1;
}

/* Hand the controller one buffer to fill with the next report. An interrupt
 * endpoint has no "read" - you post a Normal TRB and the controller completes
 * it whenever the device has something to say. One outstanding at a time is
 * enough for a keyboard and keeps the ring easy to reason about. */
static void kbd_requeue(void)
{
    /* Clear the buffer before handing it back. On a short packet the bytes the
     * device did not send would otherwise still hold the previous report, and
     * hid_decode would see keys that are not held. */
    zero_mem(KBD_REPORT, 64);
    u32 ring = INT_RING(kbd_slot);
    trb_write(ring, kbd_enq, (u64)KBD_REPORT, (u32)kbd_mps,
              (TRB_NORMAL << 10) | (1u << 5) | kbd_cyc);   /* IOC */
    kbd_enq++;
    if (kbd_enq >= RING_TRBS - 1) {
        trb_write(ring, RING_TRBS - 1, (u64)ring, 0,
                  (TRB_LINK << 10) | (1u << 1) | kbd_cyc);
        kbd_enq = 0;
        kbd_cyc ^= 1;
    }
    doorbell((u32)kbd_slot, (u32)kbd_dci);
}

/* ---- HID usage IDs to characters ---------------------------------------
 * These are NOT ASCII and NOT PC scancodes - they are a third numbering, from
 * the HID Usage Tables. Usage 0x04 is 'a' and the alphabet runs contiguously
 * from there, which is why the letters and digits are computed rather than
 * tabulated; only the punctuation needs a table. */
static int hid_to_ascii(int usage, int mods)
{
    int shift = (mods & 0x22) ? 1 : 0;      /* bit 1 = left shift, bit 5 = right */

    if (usage >= 0x04 && usage <= 0x1D) {           /* a..z */
        int c = 'a' + (usage - 0x04);
        return shift ? (c - 32) : c;
    }
    if (usage >= 0x1E && usage <= 0x26) {           /* 1..9 */
        static const char sym[] = "!@#$%^&*(";
        return shift ? (int)sym[usage - 0x1E] : ('1' + (usage - 0x1E));
    }
    if (usage == 0x27) return shift ? ')' : '0';

    switch (usage) {
        case 0x28: return 13;                       /* Enter     */
        case 0x29: return 27;                       /* Escape    */
        case 0x2A: return 8;                        /* Backspace */
        case 0x2B: return 9;                        /* Tab       */
        case 0x2C: return ' ';
        case 0x2D: return shift ? '_'  : '-';
        case 0x2E: return shift ? '+'  : '=';
        case 0x2F: return shift ? '{'  : '[';
        case 0x30: return shift ? '}'  : ']';
        case 0x31: return shift ? '|'  : '\\';
        case 0x33: return shift ? ':'  : ';';
        case 0x34: return shift ? '"'  : '\'';
        case 0x35: return shift ? '~'  : '`';
        case 0x36: return shift ? '<'  : ',';
        case 0x37: return shift ? '>'  : '.';
        case 0x38: return shift ? '?'  : '/';
    }
    return 0;
}

/* Turn one report into characters. A key counts as pressed only if it was NOT
 * in the previous report - the report is a snapshot of what is held down, so
 * without the diff a key held for a tenth of a second types itself fifty
 * times. */
static void hid_decode(void)
{
    int mods = (int)*(volatile u8 *)(KBD_REPORT + 0);
    u8  now[6];
    int rollover = 0;

    for (int i = 0; i < 6; i++) {
        now[i] = *(volatile u8 *)(KBD_REPORT + (u32)(i + 2));
        if (now[i] == 1) rollover = 1;
    }

    /* Publish the modifier bitmap even when nothing else in the report moved:
     * pressing shift ALONE sends a report with no usage IDs at all, so this is
     * the only moment anyone learns shift went down. */
    kbd_mods = (u8)mods;

    /* Do not record a rollover report as the new key state, and do not decode
     * one. If we recorded it, the next ordinary report would find prev_keys
     * full of 0x01 and re-emit every key still being held. */
    if (rollover) return;

    /* presses: in the new report and not in the old */
    for (int i = 0; i < 6; i++) {
        int usage = now[i];
        if (usage <= 3) continue;           /* 0 = empty, 1-3 = rollover errors */

        int held = 0;
        for (int j = 0; j < 6; j++)
            if ((int)prev_keys[j] == usage) { held = 1; break; }
        if (held) continue;

        int ch = hid_to_ascii(usage, mods);
        if (ch) keyq_push(ch);              /* the legacy character view */
        kevq_push(KEV(1, mods, usage));
    }

    /* releases: in the old report and not in the new. These never existed
     * before, and without them input.c can never clear its held-key state or
     * stop auto-repeating - a key would repeat until a different one was
     * pressed. */
    for (int i = 0; i < 6; i++) {
        int usage = prev_keys[i];
        if (usage <= 3) continue;

        int still = 0;
        for (int j = 0; j < 6; j++)
            if ((int)now[j] == usage) { still = 1; break; }
        if (still) continue;

        kevq_push(KEV(0, mods, usage));
    }

    for (int i = 0; i < 6; i++) prev_keys[i] = now[i];
}

/* ---- bringing a keyboard up --------------------------------------------
 * Walk the configuration descriptor looking for an interface that declares
 * itself a boot-protocol keyboard, then take the interrupt IN endpoint that
 * follows it. Descriptors are a flat byte stream of {length, type, ...}
 * records, so this walks by length and never trusts a length of zero. */
int xhci_kbd_init(void)
{
    if (!xhci_running()) return 0;
    /* Re-entry would call ring_init() on the interrupt ring while the
     * controller still holds a dequeue pointer into it and has an outstanding
     * TRB - a producer/consumer desync on a live ring, reachable from the
     * shell. Once a keyboard is up, keep it. */
    if (kbd_ready) return kbd_slot;

    for (int port = 1; port <= xports; port++) {
        if (!xhci_port_connected(port)) continue;

        int slot = xhci_enumerate(port);
        if (!slot || slot >= MAX_SLOTS) continue;

        if (!get_config(slot, 9)) continue;
        int total = (int)cfg_byte(2) | ((int)cfg_byte(3) << 8);
        if (total < 9) continue;
        if (total > CFG_MAX) total = CFG_MAX;
        if (!get_config(slot, total)) continue;

        int cfgval = (int)cfg_byte(5);          /* bConfigurationValue */

        int found_iface = -1;
        int ep_addr = 0, ep_mps = 0, ep_int = 1;

        int off = (int)cfg_byte(0);             /* skip the config header */
        while (off + 1 < total) {
            int dlen  = (int)cfg_byte(off);
            int dtype = (int)cfg_byte(off + 1);
            if (dlen < 2) break;                /* malformed - refuse to spin */
            /* A descriptor that claims to extend past the end of what the
             * device sent is not a descriptor. Without this the reads below
             * pick up bytes left over from the PREVIOUS port's device. */
            if (off + dlen > total) break;

            if (dtype == DESC_INTERFACE && dlen >= 9) {
                int cls  = (int)cfg_byte(off + 5);
                int sub  = (int)cfg_byte(off + 6);
                int prot = (int)cfg_byte(off + 7);
                if (cls == CLASS_HID && sub == SUBCLASS_BOOT &&
                    prot == PROTOCOL_KEYBOARD) {
                    found_iface = (int)cfg_byte(off + 2);
                    ep_addr = 0;                /* take the next endpoint */
                } else if (found_iface >= 0 && ep_addr == 0) {
                    found_iface = -1;           /* a different interface began */
                }
            } else if (dtype == DESC_ENDPOINT && dlen >= 7 &&
                       found_iface >= 0 && ep_addr == 0) {
                int addr = (int)cfg_byte(off + 2);
                int attr = (int)cfg_byte(off + 3);
                if ((addr & 0x80) && (attr & 0x03) == 3) {   /* interrupt IN */
                    ep_addr = addr;
                    ep_mps  = (int)cfg_byte(off + 4) |
                              (((int)cfg_byte(off + 5) & 0x07) << 8);
                    ep_int  = (int)cfg_byte(off + 6);
                }
            }
            off += dlen;
        }

        if (found_iface < 0 || ep_addr == 0) continue;   /* not a keyboard */

        if (!set_configuration(slot, cfgval)) continue;
        set_boot_protocol(slot, found_iface);
        set_idle(slot, found_iface);          /* best effort - some refuse it */

        /* device context index for an IN endpoint: number * 2 + 1 */
        int dci = ((ep_addr & 0x0F) * 2) + 1;
        /* dci 1 IS endpoint 0. A device claiming bEndpointAddress 0x80 would
         * make us write an interrupt endpoint over EP0's context. The
         * controller would reject it, but we should not be asking. */
        if (dci < 2 || dci > 31) continue;
        if (ep_mps <= 0 || ep_mps > 1024) ep_mps = 8;

        if (!configure_endpoint(slot, dci, ep_mps, xhci_port_speed(port), ep_int))
            continue;

        kbd_slot  = slot;
        kbd_iface = found_iface;
        kbd_dci   = dci;
        kbd_mps   = ep_mps;
        kbd_ready = 1;
        for (int i = 0; i < 6; i++) prev_keys[i] = 0;
        keyq_head  = keyq_tail  = 0;
        kevq_head  = kevq_tail  = 0;
        kbd_mods   = 0;

        kbd_requeue();                        /* arm the first read */
        return slot;
    }
    return 0;
}

/* Bring the whole stack up, once, in the right order.
 *
 * The subtlety that cost a debugging round: the FIRMWARE leaves the controller
 * running. SeaBIOS drives USB keyboards itself, so on entry USBSTS says
 * "running" and a naive "is it already going?" check skips straight past reset
 * - leaving us pointed at the firmware's rings, in the firmware's memory,
 * which it is still using. "Running" is not the same as "ours". We track
 * ownership explicitly instead of inferring it. */
static int owned = 0;

int xhci_bringup(void)
{
    if (kbd_ready) return kbd_slot;

    if (!owned) {
        if (!xhci_present() && xhci_find() < 0) return 0;
        if (!xhci_reset())      return 0;     /* take it away from the firmware */
        if (!xhci_init_rings()) return 0;     /* and give it OUR data structures */
        owned = 1;
    }
    return xhci_kbd_init();
}

int xhci_owned(void)     { return owned; }
int xhci_kbd_ready(void) { return kbd_ready; }
int xhci_kbd_slot(void)  { return kbd_slot;  }
int xhci_kbd_ep(void)    { return kbd_dci;   }

/* Non-blocking: look at the event ring exactly once. If our endpoint completed
 * a transfer, decode the report and immediately post another buffer. Anything
 * else on the ring is consumed and ignored so it cannot block us forever. */
/* One completed interrupt transfer: decode it, then immediately give the
 * controller another buffer. The requeue is not optional - it is the only
 * thing keeping the keyboard alive, so it happens even when the completion
 * code was an error. */
static void kbd_event(u32 status, u32 ctrl)
{
    if (!kbd_ready) return;

    int slot = (int)((ctrl >> 24) & 0xFF);
    int epid = (int)((ctrl >> 16) & 0x1F);
    if (slot != kbd_slot || epid != kbd_dci) return;

    int cc = (int)((status >> 24) & 0xFF);
    if (cc == 1 || cc == 13) hid_decode();    /* success or short packet */

    kbd_requeue();
}

int xhci_kbd_poll(void)
{
    if (!kbd_ready) return 0;

    u32 status = 0, ctrl = 0;
    int type = event_poll(0, &status, &ctrl, 1);
    if (type != TRB_TRANSFER_EVENT) return 0;

    kbd_event(status, ctrl);
    return 1;
}

/* One character, or 0 if nothing was typed.
 *
 * This is the older, narrower view, kept because the zl `usb_key` builtin
 * compares what it gets against 13 and 27. It reads its OWN queue rather than
 * the event queue, so the shell reading characters and the compositor reading
 * events cannot steal keystrokes from each other. */
int xhci_key(void)
{
    xhci_kbd_poll();
    return keyq_pop();
}

/* One key event - press or release, with the usage ID and the modifiers of the
 * report it came in. This is what input.c reads, and the only view through
 * which a key with no character can reach an application. */
int xhci_key_event(void)
{
    xhci_kbd_poll();
    return kevq_pop();
}

/* The live modifier bitmap, for a shift that is held with nothing else. */
int xhci_kbd_mods(void)
{
    return (int)kbd_mods;
}

/* raw report bytes, for showing what the hardware actually sent */
int xhci_kbd_report(int i)
{
    if (i < 0 || i > 7) return 0;
    return (int)*(volatile u8 *)(KBD_REPORT + (u32)i);
}

/* ==== USB mass storage: Bulk-Only Transport ==============================
 * A USB stick is not a block device the way NVMe is. It is a SCSI target at
 * the end of a USB pipe, and the transport that carries SCSI over USB is
 * deliberately minimal - three steps, no interrupts, no queueing:
 *
 *   CBW   a 31-byte Command Block Wrapper on the bulk OUT endpoint, carrying
 *         a SCSI command inside it
 *   data  however many bytes the command moves, on bulk IN or bulk OUT
 *   CSW   a 13-byte Command Status Wrapper on bulk IN, saying whether it
 *         worked
 *
 * The SCSI commands themselves are the same ones a 1986 hard disk spoke:
 * INQUIRY to ask what it is, READ CAPACITY to ask how big, READ(10) to fetch
 * blocks. That is not an accident - it is why a USB stick works on anything.
 *
 * Everything below reuses the ring and context machinery the keyboard already
 * proved. The only genuinely new piece is bulk endpoints, which differ from
 * the interrupt endpoint only in their type field and in having no polling
 * interval.
 */
#define MSC_IN_RING(s)   (0x0E500000u + (u32)(s) * RING_STRIDE)
#define MSC_OUT_RING(s)  (0x0E508000u + (u32)(s) * RING_STRIDE)
#define MSC_CBW          0x0E510000u
#define MSC_CSW          0x0E510200u
#define MSC_DATA         0x0E511000u
#define MSC_DATA_MAX     4096u

#define EPTYPE_BULK_OUT  2
#define EPTYPE_BULK_IN   6

static int msc_slot = 0, msc_iface = 0;
static int msc_in_dci = 0, msc_out_dci = 0;
static int msc_in_mps = 512, msc_out_mps = 512;
static u32 msc_in_enq = 0, msc_in_cyc = 1;
static u32 msc_out_enq = 0, msc_out_cyc = 1;
static u32 msc_tag = 1;
static int msc_ready = 0;
static u32 msc_blocks = 0, msc_blocksize = 512;

/* Add both bulk endpoints to a configured device in one Configure Endpoint.
 * Doing it as one command matters: each one changes Context Entries, and two
 * separate commands would have the second overwrite the first's idea of how
 * many contexts are valid. */
static int configure_bulk(int slot)
{
    ring_init(MSC_IN_RING(slot));
    ring_init(MSC_OUT_RING(slot));
    msc_in_enq = 0; msc_in_cyc = 1;
    msc_out_enq = 0; msc_out_cyc = 1;

    int top = (msc_in_dci > msc_out_dci) ? msc_in_dci : msc_out_dci;

    zero_mem(CTX_INPUT, 33u * (u32)xctxsize);
    ctx_set(CTX_INPUT, 0, 1, (1u << 0) | (1u << msc_in_dci) | (1u << msc_out_dci));

    u32 slot_dw0 = ctx_get(CTX_DEVICE(slot), 0, 0);
    slot_dw0 = (slot_dw0 & 0x07FFFFFFu) | ((u32)top << 27);
    ctx_set(CTX_INPUT, 1, 0, slot_dw0);
    ctx_set(CTX_INPUT, 1, 1, ctx_get(CTX_DEVICE(slot), 0, 1));

    int ic_in = msc_in_dci + 1;
    ctx_set(CTX_INPUT, ic_in, 0, 0);                  /* bulk has no interval */
    ctx_set(CTX_INPUT, ic_in, 1, (3u << 1) | ((u32)EPTYPE_BULK_IN << 3) | ((u32)msc_in_mps << 16));
    ctx_set(CTX_INPUT, ic_in, 2, MSC_IN_RING(slot) | 1u);
    ctx_set(CTX_INPUT, ic_in, 3, 0);
    ctx_set(CTX_INPUT, ic_in, 4, (u32)msc_in_mps);

    int ic_out = msc_out_dci + 1;
    ctx_set(CTX_INPUT, ic_out, 0, 0);
    ctx_set(CTX_INPUT, ic_out, 1, (3u << 1) | ((u32)EPTYPE_BULK_OUT << 3) | ((u32)msc_out_mps << 16));
    ctx_set(CTX_INPUT, ic_out, 2, MSC_OUT_RING(slot) | 1u);
    ctx_set(CTX_INPUT, ic_out, 3, 0);
    ctx_set(CTX_INPUT, ic_out, 4, (u32)msc_out_mps);

    u32 trb = cmd_submit((u64)CTX_INPUT, 0, TRB_CONFIGURE_EP, (u32)slot << 24);
    u32 status = 0;
    if (!cmd_wait(trb, &status, 0, 5000000)) return 0;
    return ((status >> 24) & 0xFF) == 1;
}

/* One bulk transfer: a single Normal TRB with interrupt-on-completion. Bulk
 * endpoints have no schedule, so this is as simple as USB gets. */
static int bulk_xfer(int slot, int dci, u32 buf, u32 len, int is_in)
{
    u32 ring = is_in ? MSC_IN_RING(slot) : MSC_OUT_RING(slot);
    u32 *enq = is_in ? &msc_in_enq : &msc_out_enq;
    u32 *cyc = is_in ? &msc_in_cyc : &msc_out_cyc;

    trb_write(ring, *enq, (u64)buf, len, (TRB_NORMAL << 10) | (1u << 5) | *cyc);
    (*enq)++;
    if (*enq >= RING_TRBS - 1) {
        trb_write(ring, RING_TRBS - 1, (u64)ring, 0,
                  (TRB_LINK << 10) | (1u << 1) | *cyc);
        *enq = 0;
        *cyc ^= 1;
    }
    doorbell((u32)slot, (u32)dci);

    u32 status = 0, ctrl = 0;
    if (!event_wait(TRB_TRANSFER_EVENT, 0, &status, &ctrl, 20000000)) return 0;
    int cc = (int)((status >> 24) & 0xFF);
    if (cc == 1 || cc == 13) return 1;
    if (cc == 6 || cc == 4) reset_endpoint(slot, dci);   /* stall - recover */
    return 0;
}

/* Build a Command Block Wrapper. The signature and the tag are how the device
 * tells our commands apart from noise, and the tag comes back in the status
 * wrapper so a reply can be matched to its request. */
static void build_cbw(u32 data_len, int is_in, const u8 *cdb, int cdb_len)
{
    volatile u8 *w = (volatile u8 *)MSC_CBW;
    for (int i = 0; i < 31; i++) w[i] = 0;
    volatile u32 *d = (volatile u32 *)MSC_CBW;
    d[0] = 0x43425355u;               /* 'USBC'                       */
    d[1] = msc_tag++;                 /* our tag                      */
    d[2] = data_len;
    w[12] = is_in ? 0x80 : 0x00;      /* direction                    */
    w[13] = 0;                        /* LUN 0                        */
    w[14] = (u8)cdb_len;
    for (int i = 0; i < cdb_len && i < 16; i++) w[15 + i] = cdb[i];
}

/* Run one SCSI command through the transport. */
static int scsi_cmd(const u8 *cdb, int cdb_len, u32 data_len, int is_in)
{
    if (!msc_slot) return 0;
    build_cbw(data_len, is_in, cdb, cdb_len);

    if (!bulk_xfer(msc_slot, msc_out_dci, MSC_CBW, 31, 0)) return 0;

    if (data_len) {
        if (data_len > MSC_DATA_MAX) return 0;
        if (is_in) zero_mem(MSC_DATA, data_len);
        if (!bulk_xfer(msc_slot, is_in ? msc_in_dci : msc_out_dci,
                       MSC_DATA, data_len, is_in)) return 0;
    }

    zero_mem(MSC_CSW, 16);
    if (!bulk_xfer(msc_slot, msc_in_dci, MSC_CSW, 13, 1)) return 0;

    volatile u32 *c = (volatile u32 *)MSC_CSW;
    if (c[0] != 0x53425355u) return 0;         /* 'USBS' */
    return (*(volatile u8 *)(MSC_CSW + 12)) == 0;   /* status 0 = good */
}

int xhci_msc_inquiry(void)
{
    u8 cdb[6] = { 0x12, 0, 0, 0, 36, 0 };      /* INQUIRY, 36 bytes */
    return scsi_cmd(cdb, 6, 36, 1);
}

int xhci_msc_read_capacity(void)
{
    u8 cdb[10] = { 0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    if (!scsi_cmd(cdb, 10, 8, 1)) return 0;
    /* both fields are BIG endian - SCSI predates the x86 monoculture */
    volatile u8 *d = (volatile u8 *)MSC_DATA;
    u32 last = ((u32)d[0] << 24) | ((u32)d[1] << 16) | ((u32)d[2] << 8) | d[3];
    u32 blen = ((u32)d[4] << 24) | ((u32)d[5] << 16) | ((u32)d[6] << 8) | d[7];
    msc_blocks    = last + 1;
    msc_blocksize = blen ? blen : 512;
    return 1;
}

int xhci_msc_read_block(u32 lba)
{
    u8 cdb[10];
    for (int i = 0; i < 10; i++) cdb[i] = 0;
    cdb[0] = 0x28;                              /* READ(10) */
    cdb[2] = (u8)(lba >> 24); cdb[3] = (u8)(lba >> 16);
    cdb[4] = (u8)(lba >> 8);  cdb[5] = (u8)lba;
    cdb[7] = 0; cdb[8] = 1;                     /* one block */
    return scsi_cmd(cdb, 10, msc_blocksize, 1);
}

int xhci_msc_byte(int i)
{
    if (i < 0 || i >= (int)MSC_DATA_MAX) return 0;
    return (int)*(volatile u8 *)(MSC_DATA + (u32)i);
}

u32 xhci_msc_blocks(void)    { return msc_blocks; }
u32 xhci_msc_blocksize(void) { return msc_blocksize; }
u32 xhci_msc_capacity_mb(void)
{
    u64 bytes = (u64)msc_blocks * (u64)msc_blocksize;
    return (u32)(bytes >> 20);
}
int xhci_msc_ready(void) { return msc_ready; }
int xhci_msc_slot(void)  { return msc_slot; }

/* Find a bulk-only mass storage device and bring it up. The interface triple
 * is class 8 (mass storage), subclass 6 (SCSI transparent), protocol 0x50
 * (bulk-only) - anything else is a different transport we do not speak. */
int xhci_msc_init(void)
{
    if (msc_ready) return msc_slot;
    if (!xhci_running()) return 0;

    for (int port = 1; port <= xports; port++) {
        if (!xhci_port_connected(port)) continue;
        int slot = xhci_enumerate(port);
        if (!slot || slot >= MAX_SLOTS) continue;

        if (!get_config(slot, 9)) continue;
        int total = (int)cfg_byte(2) | ((int)cfg_byte(3) << 8);
        if (total < 9) continue;
        if (total > CFG_MAX) total = CFG_MAX;
        if (!get_config(slot, total)) continue;

        int cfgval = (int)cfg_byte(5);
        int iface = -1, in_ep = 0, out_ep = 0, in_mps = 0, out_mps = 0;

        int off = (int)cfg_byte(0);
        while (off + 1 < total) {
            int dlen  = (int)cfg_byte(off);
            int dtype = (int)cfg_byte(off + 1);
            if (dlen < 2) break;
            if (off + dlen > total) break;

            if (dtype == DESC_INTERFACE && dlen >= 9) {
                int cls  = (int)cfg_byte(off + 5);
                int sub  = (int)cfg_byte(off + 6);
                int prot = (int)cfg_byte(off + 7);
                if (cls == 0x08 && sub == 0x06 && prot == 0x50) {
                    iface = (int)cfg_byte(off + 2);
                    in_ep = out_ep = 0;
                } else if (iface >= 0 && (!in_ep || !out_ep)) {
                    iface = -1;                 /* a different interface began */
                }
            } else if (dtype == DESC_ENDPOINT && dlen >= 7 && iface >= 0) {
                int addr = (int)cfg_byte(off + 2);
                int attr = (int)cfg_byte(off + 3);
                int mps  = (int)cfg_byte(off + 4) |
                           (((int)cfg_byte(off + 5) & 0x07) << 8);
                if ((attr & 0x03) == 2) {       /* bulk */
                    if ((addr & 0x80) && !in_ep)  { in_ep  = addr; in_mps  = mps; }
                    if (!(addr & 0x80) && !out_ep){ out_ep = addr; out_mps = mps; }
                }
            }
            off += dlen;
        }

        if (iface < 0 || !in_ep || !out_ep) continue;

        msc_in_dci  = ((in_ep  & 0x0F) * 2) + 1;
        msc_out_dci = ((out_ep & 0x0F) * 2);
        if (msc_in_dci < 2 || msc_in_dci > 31)  continue;
        if (msc_out_dci < 2 || msc_out_dci > 31) continue;
        if (in_mps  <= 0 || in_mps  > 1024) in_mps  = 512;
        if (out_mps <= 0 || out_mps > 1024) out_mps = 512;
        msc_in_mps = in_mps; msc_out_mps = out_mps;
        msc_slot = slot; msc_iface = iface;

        if (!set_configuration(slot, cfgval)) { msc_slot = 0; continue; }
        if (!configure_bulk(slot))            { msc_slot = 0; continue; }

        msc_ready = 1;
        return slot;
    }
    return 0;
}

