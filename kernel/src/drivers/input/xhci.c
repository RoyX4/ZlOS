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

#include "memmap.h"
#include "dma.h"

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

#include "telemetry.h"

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
#define USBCMD_HSEE      (1u << 3)
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
static int  enable_slot_last_cc = -1;
/* ZLDIAG5 controller-lifecycle words: USBLEGSUP before/after,
 * USBLEGCTLSTS before/after, handoff flags, xECP, USBSTS before reset, and
 * USBSTS after reset/CNR completion. */
static u32 xhci_lifecycle[8] = {
    0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu,
    0, 0, 0xFFFFFFFFu, 0xFFFFFFFFu
};

/* MMIO. volatile because these are registers - the compiler must not cache
 * a value the hardware changes underneath it, nor reorder the accesses. */
static u32  rd32(uptr addr)         { return *(volatile u32 *)addr; }
static void wr32(uptr addr, u32 v)  { *(volatile u32 *)addr = v; }

/* Ownership bits are DMA protocol, not ordinary C state. Linux uses wmb()
 * before exposing the first TRB and dma_rmb() after observing an event cycle
 * bit. On x86 those are real fences: a compiler barrier alone does not order
 * weakly ordered/WC writes against a bus-mastering device. */
static void dma_write_barrier(void)
{
    __asm__ volatile("sfence" ::: "memory");
}

static void dma_read_barrier(void)
{
    __asm__ volatile("lfence" ::: "memory");
}
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

    u32 polls = 0, first = 0, last = 0;
    while (spins-- > 0) {
        u32 raw = rd32(addr);
        if (polls == 0) first = raw;
        u32 v = raw & mask;
        last = raw; polls++;
        if (want_set ? (v == mask) : (v == 0)) {
            zlt_count(ZLLOG_C_MMIO_POLL, polls);
            return 1;
        }
        if (idt_ticks() - t0 >= ticks) break;
    }
    zlt_count(ZLLOG_C_MMIO_POLL, polls);
    zlt_snapshot(ZLLOG_SUB_USB, ZLLOG_SNAP_XHCI_WAIT, 0,
                 (unsigned)addr, first);
    zlt_snapshot(ZLLOG_SUB_USB, ZLLOG_SNAP_XHCI_WAIT, 1,
                 last, mask | (want_set ? 0x80000000u : 0u));
    zlt_trigger(ZLLOG_SUB_USB, ZLLOG_EV_TIMEOUT, ZLLOG_ERROR,
                (unsigned)addr, last, mask | (want_set ? 0x80000000u : 0u));
    return 0;
}

static void delay_ms(int ms)
{
    u32 t0 = idt_ticks();
    u32 ticks = (u32)(ms / 10) + 1u;
    long spins = (long)ms * 50000L;
    while (spins-- > 0) {
        __asm__ volatile("pause");
        if (idt_ticks() - t0 >= ticks) break;
    }
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
        /* `<< 16 << 16`, not `<< 32` - see virtio_net.c's note. The guard above
         * means the shift never MATTERS on the 32-bit build, but it is still
         * compiled there, where uptr is 32 bits and a shift by 32 is UB. */
        xbase = ((uptr)hi << 16 << 16) | (uptr)lo;
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
            xhci_lifecycle[0] = v;
            xhci_lifecycle[2] = rd32(cap + 4);
            xhci_lifecycle[4] |= 1u;            /* legacy capability found */
            xhci_lifecycle[5] = xecp;
            /* claim it */
            wr32(cap, rd32(cap) | LEGACY_OS_OWNED);

            /* give the firmware up to a second to let go - it is doing real
             * work inside SMM, and a spin count is not a unit of time */
            int cooperative = wait_bit(cap, LEGACY_BIOS_OWNED, 0, 1000);
            if (cooperative) xhci_lifecycle[4] |= 2u;
            else xhci_lifecycle[4] |= 4u;       /* firmware had to be forced */

            /* Whether or not it cooperated, force the issue: clear the BIOS
             * bit, then disable every SMI source in USBLEGCTLSTS (the next
             * dword) and clear the three write-1-to-clear SMI status bits at
             * the top. The mask keeps the reserved fields and zeroes the
             * enables - the same one Linux uses in pci-quirks.c. */
            wr32(cap, (rd32(cap) & ~LEGACY_BIOS_OWNED) | LEGACY_OS_OWNED);
            wr32(cap + 4, (rd32(cap + 4) & 0x000E1FEEu) | 0xE0000000u);
            xhci_lifecycle[1] = rd32(cap);
            xhci_lifecycle[3] = rd32(cap + 4);
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

    /* Linux waits for CNR before touching operational state. Firmware usually
     * leaves it clear, but register accesses while it is set are undefined. */
    if (!wait_bit(xop + XOP_USBSTS, USBSTS_CNR, 0, 5000)) return 0;
    xhci_lifecycle[6] = rd32(xop + XOP_USBSTS);

    /* Ownership BEFORE reset. Resetting a controller that SMM still owns is
     * the one ordering mistake that makes everything after it unreliable. */
    xhci_take_from_firmware();

    /* stop it first - resetting a running controller is undefined */
    wr32(xop + XOP_USBCMD,
         rd32(xop + XOP_USBCMD) & ~(USBCMD_RS | USBCMD_INTE | USBCMD_HSEE));
    if (!wait_bit(xop + XOP_USBSTS, USBSTS_HCH, 1, 100)) return 0;

    /* host controller reset */
    wr32(xop + XOP_USBCMD, USBCMD_HCRST);
    /* Intel's xHCI reset erratum requires a real delay after asserting HCRST
     * before the first register read. Immediate polling is accepted by QEMU
     * but is not a safe hardware sequence. */
    delay_ms(1);
    if (!wait_bit(xop + XOP_USBCMD, USBCMD_HCRST, 0, 1000)) return 0;

    /* CNR - "controller not ready" - stays set while it reinitialises. Every
     * register read before this clears is meaningless. */
    if (!wait_bit(xop + XOP_USBSTS, USBSTS_CNR, 0, 1000)) return 0;
    xhci_lifecycle[7] = rd32(xop + XOP_USBSTS);

    /* tell it how many device slots we intend to use */
    wr32(xop + XOP_CONFIG, (u32)xslots);
    return 1;
}

int xhci_halted(void)  { return xhci_present() ? ((rd32(xop + XOP_USBSTS) & USBSTS_HCH) ? 1 : 0) : 1; }
u32 xhci_lifecycle_diag(int word)
{
    return word >= 0 && word < 8 ? xhci_lifecycle[word] : 0xFFFFFFFFu;
}
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
#define XMEM_DCBAA   ((unsigned int)HI_XHCI)   /* 224 MiB: slot pointer array */
#define XMEM_CMDRING (XMEM_DCBAA + 0x1000u)  /* the command ring             */
#define XMEM_EVTRING (XMEM_DCBAA + 0x2000u)  /* the event ring               */
#define XMEM_ERST    (XMEM_DCBAA + 0x3000u)  /* event ring segment table     */
#define XMEM_CTX     (XMEM_DCBAA + 0x4000u)  /* device + input contexts      */
#define XMEM_XFER    (XMEM_DCBAA + 0x10000u) /* per-endpoint transfer rings  */
#define XMEM_DATA    (XMEM_DCBAA + 0x20000u) /* DMA buffers for descriptors  */
#define XMEM_SCRATCH_ARR (XMEM_DCBAA + 0x30000u) /* scratchpad pointer array */
#define XMEM_SCRATCH (XMEM_DCBAA + 0x40000u) /* the scratchpad pages         */
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
#define TRB_STOP_ENDPOINT    15
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
    volatile u32 *p = (volatile u32 *)(uptr)addr;
    for (u32 i = 0; i < bytes / 4; i++) p[i] = 0;
}

/* write one TRB: two dwords of parameter, one of status, one of control */
static void trb_write(u32 ring, u32 index, u64 param, u32 status, u32 control)
{
    volatile u32 *t = (volatile u32 *)(uptr)(ring + index * TRB_BYTES);
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
            {   /* The controller dereferences these. dma.h. */
                unsigned long long pa = dma_addr(page);
                sp[i * 2]     = (u32)pa;            /* low half  */
                sp[i * 2 + 1] = (u32)(pa >> 32);    /* high half */
            }
        }
        volatile u32 *dcbaa = (volatile u32 *)XMEM_DCBAA;
        {   unsigned long long pa = dma_addr(XMEM_SCRATCH_ARR);
            dcbaa[0] = (u32)pa;
            dcbaa[1] = (u32)(pa >> 32);
        }
    }
    wr64(xop + XOP_DCBAAP, dma_addr(XMEM_DCBAA));

    /* ---- command ring, with a Link TRB closing the loop ----------------- */
    zero_mem(XMEM_CMDRING, RING_TRBS * TRB_BYTES);
    /* the last entry points back to the first; TC=1 (bit 1) tells the
     * controller to flip its cycle state when it follows the link */
    cmd_enqueue = 0;
    cmd_cycle   = 1;              /* set BEFORE the link TRB is written, or a
                                     re-init lays it down with a stale cycle */
    trb_write(XMEM_CMDRING, RING_TRBS - 1, dma_addr(XMEM_CMDRING), 0,
              (TRB_LINK << 10) | (1u << 1) | cmd_cycle);
    /* CRCR also carries our initial cycle state in bit 0 */
    wr64(xop + XOP_CRCR, dma_addr(XMEM_CMDRING) | 1u);

    /* ---- event ring: the segment, then the table describing it ---------- */
    zero_mem(XMEM_EVTRING, RING_TRBS * TRB_BYTES);
    zero_mem(XMEM_ERST, 64);
    volatile u32 *erst = (volatile u32 *)XMEM_ERST;
    {   unsigned long long pa = dma_addr(XMEM_EVTRING);
        erst[0] = (u32)pa;          /* segment base, low                     */
        erst[1] = (u32)(pa >> 32);  /* segment base, high                    */
    }
    erst[2] = RING_TRBS;            /* how many TRBs in this segment         */
    erst[3] = 0;
    evt_dequeue = 0;
    evt_cycle   = 1;

    /* the dequeue pointer must be set BEFORE the table base, per the spec */
    wr64(xrt + XRT_ERDP, dma_addr(XMEM_EVTRING));
    wr32(xrt + XRT_ERSTSZ, 1);      /* one segment */
    wr64(xrt + XRT_ERSTBA, dma_addr(XMEM_ERST));

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
    uptr reg = xdb + slot * 4;
    wr32(reg, target);
    /* Doorbells are write-only PCIe posted registers. Flush through a safe
     * readable register on the same BAR; reading the doorbell itself is not a
     * defined operation on strict controllers. */
    (void)rd32(xop + XOP_USBSTS);
}

/* Put a command on the ring and ring the bell. Returns the ADDRESS of the TRB
 * we wrote - a Command Completion Event carries that address in its parameter
 * field, which is the only reliable way to know a completion is ours. */
static u32 cmd_submit(u64 param, u32 status, u32 type, u32 extra)
{
    u32 at = cmd_enqueue;
    u32 trb_addr = XMEM_CMDRING + at * TRB_BYTES;
    zlt_count(ZLLOG_C_XHCI_COMMAND, 1);
    zlt_event(ZLLOG_SUB_USB, ZLLOG_EV_COMMAND_SUBMIT, ZLLOG_INFO,
              type, trb_addr, extra);
    trb_write(XMEM_CMDRING, at, param, status, (type << 10) | extra | cmd_cycle);
    cmd_enqueue++;
    if (cmd_enqueue >= RING_TRBS - 1) {     /* the link TRB is not usable */
        /* hand the link TRB to the controller with the current cycle, then
         * flip ours - that is what makes the wrap visible to it */
        trb_write(XMEM_CMDRING, RING_TRBS - 1, dma_addr(XMEM_CMDRING), 0,
                  (TRB_LINK << 10) | (1u << 1) | cmd_cycle);
        cmd_enqueue = 0;
        cmd_cycle ^= 1;
    }
    doorbell(0, 0);
    return trb_addr;
}

/* Wait for the controller to post an event whose cycle bit matches ours.
 * Returns the TRB type, or 0 if nothing arrived in time. */
static u32 event_last_words[4];

static int event_poll(u32 *out_param_lo, u32 *out_status, u32 *out_ctrl, int spins)
{
    u32 polls = 0;
    while (spins--) {
        polls++;
        volatile u32 *e = (volatile u32 *)(uptr)(XMEM_EVTRING + evt_dequeue * TRB_BYTES);
        u32 ctrl = e[3];
        if ((ctrl & 1u) != evt_cycle) continue;      /* not ours yet */
        /* The controller commits an Event TRB by writing its cycle bit. Do
         * not let the CPU satisfy the payload loads before that ownership
         * observation. */
        dma_read_barrier();
        event_last_words[0] = e[0];
        event_last_words[1] = e[1];
        event_last_words[2] = e[2];
        event_last_words[3] = ctrl;
        if (out_param_lo) *out_param_lo = e[0];
        if (out_status)   *out_status   = e[2];
        if (out_ctrl)     *out_ctrl     = ctrl;
        int type = (int)((ctrl >> 10) & 0x3F);

        evt_dequeue++;
        if (evt_dequeue >= RING_TRBS) { evt_dequeue = 0; evt_cycle ^= 1; }
        /* tell the controller how far we have consumed, and clear the
         * event handler busy bit (bit 3) while we are there */
        wr64(xrt + XRT_ERDP, dma_addr(XMEM_EVTRING + evt_dequeue * TRB_BYTES) | (1u << 3));
        zlt_count(ZLLOG_C_MMIO_POLL, polls);
        zlt_count(ZLLOG_C_XHCI_EVENT, 1);
        return type;
    }
    zlt_count(ZLLOG_C_MMIO_POLL, polls);
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
static void kbd_event(u32 param, u32 status, u32 ctrl);  /* stage 5, below */
static int ecm_event(u32 param, u32 status, u32 ctrl);   /* CDC-ECM, below */
static int ecm_ready;

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
        if (t == 0) {
            zlt_snapshot(ZLLOG_SUB_USB, ZLLOG_SNAP_XHCI_COMMAND, 0,
                         trb_addr, (unsigned)spins);
            zlt_snapshot(ZLLOG_SUB_USB, ZLLOG_SNAP_XHCI_COMMAND, 1,
                         ((unsigned)evt_dequeue << 1) | evt_cycle,
                         event_last_words[2]);
            zlt_trigger(ZLLOG_SUB_USB, ZLLOG_EV_TIMEOUT, ZLLOG_ERROR,
                        1u, trb_addr, (unsigned)spins);
            return 0;
        }
        if (t == TRB_TRANSFER_EVENT) { kbd_event(p, s, c); continue; }
        if (t != TRB_CMD_COMPLETION) continue;      /* port change etc */
        /* p is a DEVICE address - the controller reports the address of the
         * Command TRB it completed. trb_addr is a KERNEL address. Identity
         * today; dma_kaddr() is what keeps this comparison true the day it is
         * not, and without it every command here would time out. See dma.h. */
        if (dma_kaddr(p) != trb_addr) continue;     /* a stale completion */
        if (status) *status = s;
        if (ctrl)   *ctrl   = c;
        zlt_event(ZLLOG_SUB_USB, ZLLOG_EV_COMMAND_COMPLETE,
                  ((s >> 24) == 1u) ? ZLLOG_INFO : ZLLOG_ERROR,
                  trb_addr, s, c);
        return 1;
    }
    zlt_snapshot(ZLLOG_SUB_USB, ZLLOG_SNAP_XHCI_COMMAND, 0,
                 trb_addr, 32u);
    zlt_snapshot(ZLLOG_SUB_USB, ZLLOG_SNAP_XHCI_COMMAND, 1,
                 ((unsigned)evt_dequeue << 1) | evt_cycle,
                 event_last_words[2]);
    zlt_trigger(ZLLOG_SUB_USB, ZLLOG_EV_TIMEOUT, ZLLOG_ERROR,
                2u, trb_addr, 32u);
    return 0;
}

/* Wait for the completion of ONE SPECIFIC transfer, named by slot and endpoint.
 *
 * THE SAME TRAP cmd_wait ALREADY LEARNED, one layer down. This was
 * event_wait(want, ...) and every caller passed want == TRB_TRANSFER_EVENT, so
 * it returned the FIRST transfer event on the ring whoever it belonged to. It
 * carried the right lesson in a comment - "a keypress that lands while we are
 * waiting must NOT be thrown away: dropping it also drops our obligation to
 * post another buffer, and the keyboard goes silent forever" - and then made
 * that exact mistake, because the `t == want` test ran FIRST and matched the
 * keypress. The dispatch below it was unreachable for every real caller.
 *
 * What that costs: xhci_control_in() runs about forty times during enumeration
 * and bulk_xfer() runs on every USB-storage read, and try.sh attaches a
 * usb-storage device. A HID completion landing inside either window was
 * consumed as if it were the transfer's own - so the HID endpoint was never
 * requeued and went silent for good, AND the transfer read somebody else's
 * completion code. Move the mouse while reading the USB stick and the pointer
 * could die outright.
 *
 * A Transfer Event names its slot and endpoint in the control dword, so match
 * on those and hand everything else to the dispatcher that owns it. */
static u32 xfer_last_param;
static u32 xfer_last_event[4];

static int xfer_wait_trbs(int slot, int dci, const u32 *trbs, int ntrbs,
                          u32 *status, u32 *ctrl, int spins)
{
    xfer_last_param = 0;
    /* Bounded well above the number of foreign events that can plausibly
     * interleave - the pointer alone can have PTR_NBUF in flight. */
    for (int i = 0; i < 64; i++) {
        u32 p = 0, s = 0, c = 0;
        int t = event_poll(&p, &s, &c, spins);
        if (t == 0) return 0;                   /* nothing arrived at all */
        if (t != TRB_TRANSFER_EVENT) continue;  /* port change etc - noise */

        int es = (int)((c >> 24) & 0xFF);
        int ee = (int)((c >> 16) & 0x1F);
        if (es == slot && ee == dci) {
            if (trbs && ntrbs > 0) {
                u32 event_trb = (u32)dma_kaddr(p);
                int ours = 0;
                for (int j = 0; j < ntrbs; j++)
                    if (event_trb == trbs[j]) { ours = 1; break; }
                if (!ours) continue;          /* stale EP0 completion */
            }
            xfer_last_param = (u32)dma_kaddr(p);
            for (int j = 0; j < 4; j++) xfer_last_event[j] = event_last_words[j];
            if (status) *status = s;
            if (ctrl)   *ctrl   = c;
            zlt_count(ZLLOG_C_XHCI_TRANSFER, 1);
            if ((s >> 24) != 1u && (s >> 24) != 13u)
                zlt_event(ZLLOG_SUB_USB, ZLLOG_EV_COMMAND_COMPLETE,
                          ZLLOG_ERROR,
                          (unsigned)slot | ((unsigned)dci << 16), s, p);
            return t;
        }
        /* Somebody else's, and it carries an obligation to re-arm them. */
        kbd_event(p, s, c);
    }
    return 0;
}

/* A spin count is not a USB timeout. Five million cached reads can finish in
 * only a few milliseconds on the ThinkPad, while a real control transfer is
 * allowed to take much longer. Poll in bounded chunks until either the PIT's
 * real-time deadline or a conservative no-timer fallback budget expires. */
static int xfer_wait_trbs_ms(int slot, int dci, const u32 *trbs, int ntrbs,
                             u32 *status, u32 *ctrl, int ms)
{
    u32 t0 = idt_ticks();
    u32 start_event = ((u32)evt_dequeue << 1) | evt_cycle;
    u32 ticks = (u32)(ms / 10) + 1u;
    long budget = (long)ms * 50000L;
    const int chunk = 50000;

    while (budget > 0) {
        if (xfer_wait_trbs(slot, dci, trbs, ntrbs,
                           status, ctrl, chunk)) return 1;
        budget -= chunk;
        if (idt_ticks() - t0 >= ticks) break;
    }
    zlt_snapshot(ZLLOG_SUB_USB, ZLLOG_SNAP_XHCI_TRANSFER, 0,
                 (unsigned)slot | ((unsigned)dci << 16),
                 (trbs && ntrbs > 0) ? trbs[0] : start_event);
    zlt_snapshot(ZLLOG_SUB_USB, ZLLOG_SNAP_XHCI_TRANSFER, 1,
                 ((unsigned)evt_dequeue << 1) | evt_cycle,
                 event_last_words[2]);
    zlt_trigger(ZLLOG_SUB_USB, ZLLOG_EV_TIMEOUT, ZLLOG_ERROR,
                3u, (unsigned)slot | ((unsigned)dci << 16), (unsigned)ms);
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
    enable_slot_last_cc = -1;
    if (!xhci_running()) return 0;
    u32 trb = cmd_submit(0, 0, TRB_ENABLE_SLOT, 0);
    u32 status = 0, ctrl = 0;
    if (!cmd_wait(trb, &status, &ctrl, 5000000)) {
        enable_slot_last_cc = 0;
        return 0;
    }
    enable_slot_last_cc = (int)((status >> 24) & 0xFF);
    if (enable_slot_last_cc != 1) return 0;
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

/* "with no diagnostic" is now false, which is the point. Each family of
 * per-slot allocations must stop before the next one starts. */
_Static_assert(CTX_DEVICE(MAX_SLOTS) <= XMEM_XFER,
               "xhci: device contexts have grown into the transfer rings");
_Static_assert(EP0_RING(MAX_SLOTS) <= INT_RING(0),
               "xhci: EP0 rings alias the interrupt rings (MAX_SLOTS too large)");
_Static_assert(INT_RING(MAX_SLOTS) <= XMEM_DATA,
               "xhci: interrupt rings have grown into the DMA buffers");

static void ctx_set(u32 base, int which, int dword, u32 val)
{
    *(volatile u32 *)(uptr)(base + (u32)which * (u32)xctxsize + (u32)dword * 4) = val;
}

static u32 ctx_get(u32 base, int which, int dword)
{
    return *(volatile u32 *)(uptr)(base + (u32)which * (u32)xctxsize + (u32)dword * 4);
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
static int ep0_last_cc = -1;
static int ep0_last_event_stage = -1; /* 1 setup, 2 data, 3 status */
static int ep0_last_attempts = 1;
static int ep0_last_recovery = -1;    /* -1 none, 0 failed, 1 completed */
/* ZLDIAG4 snapshots the exact failed control TD before endpoint recovery
 * destroys its ring.  The 22 explicit little-endian words are:
 *   0..3 Setup, 4..7 Data, 8..11 Status, 12..15 Transfer Event,
 *   16 output-EP0 DW0, 17 DW2, 18 DW3, 19 metadata,
 *   20 four one-byte probe completion codes, 21 first destination dword.
 * Metadata: enqueue[7:0], producer cycle bit8, valid bit9, data-present bit10,
 * event-valid bit11, context-valid bit12, timeout bit13, context size[23:16],
 * descriptor attempt[31:24]. */
#define EP0_TRACE_WORDS 22
static u32 ep0_last_trace[EP0_TRACE_WORDS];
static u32 ep0_first_device_probe[2][EP0_TRACE_WORDS];
static u32 ep0_first_device_slot_dw3[2];
static int ep0_first_device_probe_done;
/* ZLDIAG6 retains the first Configuration request before descriptor retries
 * and endpoint recovery can replace it.  Words 0..21 are the ordinary raw
 * EP0 trace.  The remaining explicit LE words are:
 *  22 metadata (port/slot/cc/recovery), 23 slot-context DW3,
 *  24 recovery kind (0 none, 1 Reset Endpoint, 2 Stop Endpoint),
 *  25 recovery command CC, 26 Set TR Dequeue CC,
 *  27..29 post-recovery EP0 DW0/DW2/DW3,
 *  30 whole-enumeration results (old-scheme cc, pre-address cc, winner),
 *  31 reserved. */
#define EP0_CONFIG_DIAG_WORDS 32
static u32 ep0_first_config_diag[EP0_CONFIG_DIAG_WORDS];
static int ep0_first_config_done;
static int ep0_first_config_port;
static int ep0_last_recovery_kind;
static int ep0_last_recovery_cmd_cc;
static int ep0_last_set_deq_cc;
static u32 ep0_last_post_ctx[3];
static int address_last_cc = -1;
static int fix_ep0_last_cc = -1;
static int cur_slot    = 0;
static int cur_port    = 0;
static int cur_speed   = 0;

/* Lay a fresh ring down: all zeroes, with a Link TRB in the last slot pointing
 * back at the start and TC set so the controller flips its cycle on the wrap. */
static void ring_init(u32 ring)
{
    zero_mem(ring, RING_TRBS * TRB_BYTES);
    trb_write(ring, RING_TRBS - 1, dma_addr(ring), 0,
              (TRB_LINK << 10) | (1u << 1) | 1u);
}

/* Build the input context and issue Address Device. After this the device has
 * a USB address and answers control transfers - it is a real, addressed device
 * on the bus. */
static int xhci_address_device_mode(int slot, int port, int speed, int bsr)
{
    address_last_cc = -1;
    if (!xhci_running() || slot <= 0 || slot > xslots) return 0;
    if (slot >= MAX_SLOTS) return 0;         /* more devices than we track */

    /* the device context the controller will own, and the DCBAA entry that
     * tells it where to find it */
    zero_mem(CTX_DEVICE(slot), 2048);
    volatile u32 *dcbaa = (volatile u32 *)XMEM_DCBAA;
    {   unsigned long long pa = dma_addr(CTX_DEVICE(slot));
        dcbaa[slot * 2]     = (u32)pa;
        dcbaa[slot * 2 + 1] = (u32)(pa >> 32);
    }

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
    ctx_set(CTX_INPUT, 2, 2, (u32)dma_addr(EP0_RING(slot)) | 1u);
    ctx_set(CTX_INPUT, 2, 3, (u32)(dma_addr(EP0_RING(slot)) >> 32));
    ctx_set(CTX_INPUT, 2, 4, 8);            /* average TRB length */

    u32 trb = cmd_submit(dma_addr(CTX_INPUT), 0, TRB_ADDRESS_DEVICE,
                         ((u32)slot << 24) | (bsr ? (1u << 9) : 0u));
    u32 status = 0, ctrl = 0;
    if (!cmd_wait(trb, &status, &ctrl, 5000000)) {
        address_last_cc = 0;
        return 0;
    }
    address_last_cc = (int)((status >> 24) & 0xFF);
    if (address_last_cc != 1) return 0;

    /* Address Device performs the USB SET_ADDRESS transaction on our behalf,
     * but xHCI deliberately leaves its recovery interval to software. Linux
     * waits 10 ms here before the first descriptor request; without it the
     * physical Intel controller can accept Device and then lose the following
     * Configuration Setup while the new address is still settling. */
    if (!bsr) delay_ms(10);

    cur_slot = slot; cur_port = port; cur_speed = speed;
    return 1;
}

int xhci_address_device(int slot, int port, int speed)
{
    return xhci_address_device_mode(slot, port, speed, 0);
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
#define CONTROL_TIMEOUT_MS 1000

static int reset_endpoint(int slot, int dci);   /* defined with the commands */
static int stop_endpoint_ring(int slot, int dci, u32 ring,
                              u32 *producer_enq, u32 *producer_cycle);

static u32 ep0_push(int slot, u64 param, u32 status, u32 control)
{
    u32 ring = EP0_RING(slot);
    u32 index = ep0_enqueue[slot];
    trb_write(ring, index, param, status, control | ep0_cycle[slot]);
    ep0_enqueue[slot]++;
    if (ep0_enqueue[slot] >= RING_TRBS - 1) {
        /* hand the link TRB over with the CURRENT cycle, then flip ours - that
         * is what makes the wrap visible to the controller */
        trb_write(ring, RING_TRBS - 1, dma_addr(ring), 0,
                  (TRB_LINK << 10) | (1u << 1) | ep0_cycle[slot]);
        ep0_enqueue[slot] = 0;
        ep0_cycle[slot] ^= 1;
    }
    return ring + index * TRB_BYTES;
}

/* An endpoint ring stays live after its first doorbell. Publishing the Setup
 * TRB with the producer cycle before its Data and Status TRBs exist lets a
 * fast controller consume a half-built control transfer. QEMU happened not
 * to race us; Intel did, and every device's second EP0 request completed with
 * USB Transaction Error. Build the first TRB with the OPPOSITE cycle, fill the
 * whole transfer, then flip only that ownership bit as the atomic commit. */
static u32 ep0_begin_unpublished(int slot, u64 param, u32 status, u32 control,
                                 u32 *publish_cycle)
{
    u32 ring = EP0_RING(slot);
    u32 index = ep0_enqueue[slot];
    u32 cycle = ep0_cycle[slot];
    trb_write(ring, index, param, status, control | (cycle ^ 1u));
    ep0_enqueue[slot]++;
    if (ep0_enqueue[slot] >= RING_TRBS - 1) {
        trb_write(ring, RING_TRBS - 1, dma_addr(ring), 0,
                  (TRB_LINK << 10) | (1u << 1) | cycle);
        ep0_enqueue[slot] = 0;
        ep0_cycle[slot] ^= 1;
    }
    *publish_cycle = cycle;
    return ring + index * TRB_BYTES;
}

static void ep0_publish(u32 trb_addr, u32 cycle)
{
    /* Match Linux's giveback_first_trb(): all payload TRBs must be globally
     * visible to the controller before the first cycle bit changes owner. */
    dma_write_barrier();
    volatile u32 *control = (volatile u32 *)(uptr)(trb_addr + 12u);
    *control = (*control & ~1u) | (cycle & 1u);
    dma_write_barrier();
}

static void ep0_trace_trb(int word, u32 trb_addr)
{
    if (!trb_addr) return;
    volatile u32 *trb = (volatile u32 *)(uptr)trb_addr;
    for (int i = 0; i < 4; i++) ep0_last_trace[word + i] = trb[i];
}

static void ep0_trace_finish(int slot, u32 buf, int len, int timeout,
                             int event_valid)
{
    dma_read_barrier();
    ep0_last_trace[16] = ctx_get(CTX_DEVICE(slot), 1, 0);
    ep0_last_trace[17] = ctx_get(CTX_DEVICE(slot), 1, 2);
    ep0_last_trace[18] = ctx_get(CTX_DEVICE(slot), 1, 3);
    ep0_last_trace[19] = (ep0_enqueue[slot] & 0xFFU) |
                         ((ep0_cycle[slot] & 1U) << 8) | (1U << 9) |
                         (len > 0 ? (1U << 10) : 0U) |
                         (event_valid ? (1U << 11) : 0U) | (1U << 12) |
                         (timeout ? (1U << 13) : 0U) |
                         (((u32)xctxsize & 0xFFU) << 16);
    if (len > 0) ep0_last_trace[21] = *(volatile u32 *)(uptr)buf;
}

int xhci_control_in(int slot, u32 setup_lo, u32 setup_hi, u32 buf, int len)
{
    ep0_last_cc = -1;
    ep0_last_event_stage = -1;
    ep0_last_attempts = 1;
    ep0_last_recovery = -1;
    ep0_last_recovery_kind = 0;
    ep0_last_recovery_cmd_cc = -1;
    ep0_last_set_deq_cc = -1;
    for (int i = 0; i < 3; i++) ep0_last_post_ctx[i] = 0;
    for (int i = 0; i < EP0_TRACE_WORDS; i++) ep0_last_trace[i] = 0;
    if (!xhci_running() || slot <= 0 || slot >= MAX_SLOTS) return 0;
    if (len > 0) zero_mem(buf, (u32)((len + 3) & ~3));

    /* Setup stage. IDT (immediate data) means the eight request bytes ARE the
     * parameter field, not a pointer to them. TRT=3 declares an IN data stage. */
    u32 publish_cycle = 0;
    u32 setup_trb = ep0_begin_unpublished(
        slot, ((u64)setup_hi << 32) | (u64)setup_lo, 8,
        (TRB_SETUP << 10) | (1u << 6) | (len > 0 ? (3u << 16) : 0u),
        &publish_cycle);

    /* Data stage, DIR=1 for IN. Match Linux's ISP bit: if a device returns a
     * genuinely short descriptor we may receive a Data-stage cc13 before the
     * Status event. The waiter below deliberately continues through that
     * intermediate event; only Status-stage success completes the request. */
    u32 transfer_trbs[3];
    int transfer_count = 0;
    transfer_trbs[transfer_count++] = setup_trb;
    if (len > 0)
        transfer_trbs[transfer_count++] = ep0_push(
            slot, dma_addr(buf), (u32)len,
            (TRB_DATA << 10) | (1u << 16) | (1u << 2));

    /* Status stage, in the OPPOSITE direction to the data, with IOC set so the
     * controller tells us the whole thing landed. */
    transfer_trbs[transfer_count++] = ep0_push(
        slot, 0, 0,
        (TRB_STATUS << 10) | (1u << 5) | (len > 0 ? 0u : (1u << 16)));

    ep0_publish(setup_trb, publish_cycle);
    ep0_trace_trb(0, setup_trb);
    if (len > 0) ep0_trace_trb(4, transfer_trbs[1]);
    ep0_trace_trb(8, transfer_trbs[transfer_count - 1]);
    doorbell((u32)slot, 1);           /* EP0 is doorbell target 1 */

    u32 status = 0, ctrl = 0;
    u32 status_trb = transfer_trbs[transfer_count - 1];
    int status_only = 0;
    for (;;) {
        /* EP0 of THIS slot, not "the next transfer event on the ring" - a HID
         * completion landing mid-enumeration used to be taken for this one. */
        const u32 *wanted = status_only ? &status_trb : transfer_trbs;
        int nwanted = status_only ? 1 : transfer_count;
        if (!xfer_wait_trbs_ms(slot, 1, wanted, nwanted,
                               &status, &ctrl, CONTROL_TIMEOUT_MS)) {
            ep0_last_cc = 0;                   /* bounded timeout */
            ep0_trace_finish(slot, buf, len, 1, 0);
            /* A timed-out endpoint may still be Running. The xHCI specification
             * requires Stop Endpoint before replacing its dequeue pointer; Reset
             * Endpoint is valid only after a transfer error has Halted it. */
            ep0_last_recovery = stop_endpoint_ring(
                slot, 1, EP0_RING(slot), &ep0_enqueue[slot], &ep0_cycle[slot]);
            return 0;
        }
        for (int i = 0; i < transfer_count; i++)
            if (transfer_trbs[i] == xfer_last_param) {
                if (i == 0) ep0_last_event_stage = 1;
                else if (len > 0 && i == 1) ep0_last_event_stage = 2;
                else ep0_last_event_stage = 3;
            }
        for (int i = 0; i < 4; i++) ep0_last_trace[12 + i] = xfer_last_event[i];
        int cc = (int)((status >> 24) & 0xFF);

        /* ISP can report a short Data stage before the IOC Status event. A
         * control request is not complete until Status succeeds. The same
         * rule rejects any other surprising intermediate success event. */
        if ((cc == 1 || cc == 13) && xfer_last_param != status_trb) {
            status_only = 1;
            continue;
        }

        ep0_trace_finish(slot, buf, len, 0, 1);
        ep0_last_cc = cc;
        if (cc == 1 && xfer_last_param == status_trb) return 1;

        /* Stall (6) or transfer error (4) leaves the endpoint halted. Clear it
         * here so the NEXT request works - a device is allowed to refuse an
         * optional request, and that must not be fatal. */
        if (cc == 6 || cc == 4)
            ep0_last_recovery = reset_endpoint(slot, 1);
        return 0;
    }
}

static u32 ep0_probe_meta(int slot, int cc, int recovery)
{
    u32 recovery_code = recovery < 0 ? 0U : (recovery ? 2U : 1U);
    return 0x80000000U | ((u32)cur_port & 0xFFU) |
           (((u32)slot & 0xFFU) << 8) | (((u32)cc & 0xFFU) << 16) |
           ((recovery_code & 3U) << 24);
}

static void ep0_capture_first_config(int slot)
{
    if (ep0_first_config_done) return;
    ep0_first_config_done = 1;
    ep0_first_config_port = cur_port;
    for (int i = 0; i < EP0_TRACE_WORDS; i++)
        ep0_first_config_diag[i] = ep0_last_trace[i];
    ep0_first_config_diag[22] = ep0_probe_meta(
        slot, ep0_last_cc, ep0_last_recovery);
    ep0_first_config_diag[23] = ctx_get(CTX_DEVICE(slot), 0, 3);
    ep0_first_config_diag[24] = (u32)ep0_last_recovery_kind;
    ep0_first_config_diag[25] = (u32)ep0_last_recovery_cmd_cc;
    ep0_first_config_diag[26] = (u32)ep0_last_set_deq_cc;
    ep0_first_config_diag[27] = ep0_last_post_ctx[0];
    ep0_first_config_diag[28] = ep0_last_post_ctx[1];
    ep0_first_config_diag[29] = ep0_last_post_ctx[2];
    ep0_first_config_diag[30] = 0x0000FFFFu; /* both clean retries unattempted */
}

static void ep0_note_config_reenumeration(int port, int scheme, int cc,
                                           int winner)
{
    if (!ep0_first_config_done || port != ep0_first_config_port) return;
    u32 value = ep0_first_config_diag[30];
    u32 shift = scheme == 2 ? 8u : 0u;
    value &= ~(0xFFu << shift);
    value |= ((u32)cc & 0xFFu) << shift;
    if (winner) {
        value &= ~(3u << 16);
        value |= ((u32)scheme & 3u) << 16;
    }
    ep0_first_config_diag[30] = value;
}

/* Capture the literal first successful Device request and issue one immediate
 * identical request before Configuration or any class scan can fail.  v4's
 * post-failure matrix proved recovery did not revive EP0, but could not tell
 * whether Config caused the first failure. This pair answers that in one boot.
 * Restore the first result so enumeration observes the request it made. */
static void ep0_probe_first_device(int slot, u32 setup_lo, u32 setup_hi,
                                   u32 buf, int len)
{
    if (ep0_first_device_probe_done || len <= 0 || len > 20) return;
    ep0_first_device_probe_done = 1;       /* latch before issuing the repeat */

    u8 saved_bytes[20];
    u32 saved_trace[EP0_TRACE_WORDS];
    for (int i = 0; i < len; i++) saved_bytes[i] = *(volatile u8 *)(uptr)(buf + (u32)i);
    for (int i = 0; i < EP0_TRACE_WORDS; i++) {
        saved_trace[i] = ep0_last_trace[i];
        ep0_first_device_probe[0][i] = ep0_last_trace[i];
    }
    int saved_cc = ep0_last_cc;
    int saved_stage = ep0_last_event_stage;
    int saved_attempts = ep0_last_attempts;
    int saved_recovery = ep0_last_recovery;
    ep0_first_device_probe[0][20] =
        ep0_probe_meta(slot, saved_cc, saved_recovery);
    ep0_first_device_slot_dw3[0] = ctx_get(CTX_DEVICE(slot), 0, 3);

    (void)xhci_control_in(slot, setup_lo, setup_hi, buf, len);
    ep0_last_trace[19] |= 1U << 24;
    for (int i = 0; i < EP0_TRACE_WORDS; i++)
        ep0_first_device_probe[1][i] = ep0_last_trace[i];
    ep0_first_device_probe[1][20] =
        ep0_probe_meta(slot, ep0_last_cc, ep0_last_recovery);
    ep0_first_device_slot_dw3[1] = ctx_get(CTX_DEVICE(slot), 0, 3);

    for (int i = 0; i < len; i++) *(volatile u8 *)(uptr)(buf + (u32)i) = saved_bytes[i];
    for (int i = 0; i < EP0_TRACE_WORDS; i++) ep0_last_trace[i] = saved_trace[i];
    ep0_last_cc = saved_cc;
    ep0_last_event_stage = saved_stage;
    ep0_last_attempts = saved_attempts;
    ep0_last_recovery = saved_recovery;
}

u32 xhci_ep0_first_device_probe(int which, int word)
{
    if (which < 0 || which > 1 || word < 0 || word >= EP0_TRACE_WORDS) return 0;
    return ep0_first_device_probe[which][word];
}

u32 xhci_ep0_first_device_slot_context(int which)
{
    return which >= 0 && which < 2 ? ep0_first_device_slot_dw3[which] : 0;
}

/* Linux's usb_get_descriptor() deliberately tries up to three times because
 * real devices occasionally NAK or otherwise fail a descriptor request even
 * though the same request succeeds immediately afterwards. QEMU never needs
 * that tolerance. Preserve the deepest recovery result across the successful
 * retry so the firmware diagnostic can prove that this path was exercised. */
static int descriptor_in(int slot, u32 setup_lo, u32 setup_hi, u32 buf, int len)
{
    int recovery = -1;
    for (int attempt = 1; attempt <= 3; attempt++) {
        int ok = xhci_control_in(slot, setup_lo, setup_hi, buf, len);
        ep0_last_trace[19] |= (u32)attempt << 24;
        if (!ep0_first_config_done && setup_lo == 0x02000680u)
            ep0_capture_first_config(slot);
        if (ok) {
            ep0_last_attempts = attempt;
            ep0_last_recovery = recovery;
            if (!ep0_first_device_probe_done && setup_lo == 0x01000680u)
                ep0_probe_first_device(slot, setup_lo, setup_hi, buf, len);
            return 1;
        }
        if (ep0_last_recovery >= 0) recovery = ep0_last_recovery;
        ep0_last_attempts = attempt;
        if (attempt < 3) delay_ms(10);
    }
    ep0_last_recovery = recovery;
    return 0;
}

u32 xhci_ep0_first_config_diag(int word)
{
    if (word < 0 || word >= EP0_CONFIG_DIAG_WORDS) return 0;
    return ep0_first_config_diag[word];
}

/* ---- the actual identity of the device ---------------------------------- */
/* GET_DESCRIPTOR(DEVICE): bmRequestType 0x80 (device-to-host, standard),
 * bRequest 6, wValue 0x0100 (descriptor type 1, index 0), wLength 18. */
int xhci_get_device_descriptor(int slot)
{
    return descriptor_in(slot, 0x01000680u, 0x00120000u, XMEM_DATA, 18);
}

int xhci_desc_byte(int i)
{
    if (i < 0 || i >= 18) return 0;     /* only the device descriptor is here */
    return (int)*(volatile u8 *)(uptr)(XMEM_DATA + (u32)i);
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
static int set_endpoint_ring_head(int slot, int dci, u32 ring,
                                  u32 *producer_enq, u32 *producer_cycle)
{
    /* The endpoint is Stopped now. Replace the ring only after the command
     * has quiesced all DMA from the old one, then point hardware and software
     * at the same fresh producer cycle. */
    ring_init(ring);
    *producer_enq = 0;
    *producer_cycle = 1;
    dma_write_barrier();

    u32 trb = cmd_submit(dma_addr(ring) | 1u, 0,
                         16 /* Set TR Dequeue Pointer */,
                         ((u32)slot << 24) | ((u32)dci << 16));
    u32 status = 0;
    int waited = cmd_wait(trb, &status, 0, 2000000);
    if (dci == 1) {
        ep0_last_set_deq_cc = waited ? (int)((status >> 24) & 0xFF) : 0;
        dma_read_barrier();
        ep0_last_post_ctx[0] = ctx_get(CTX_DEVICE(slot), 1, 0);
        ep0_last_post_ctx[1] = ctx_get(CTX_DEVICE(slot), 1, 2);
        ep0_last_post_ctx[2] = ctx_get(CTX_DEVICE(slot), 1, 3);
    }
    if (!waited) return 0;
    return ((status >> 24) & 0xFF) == 1;
}

static int reset_endpoint_ring(int slot, int dci, u32 ring,
                               u32 *producer_enq, u32 *producer_cycle)
{
    if (!xhci_running() || slot <= 0) return 0;
    if (dci == 1) ep0_last_recovery_kind = 1;
    u32 trb = cmd_submit(0, 0, TRB_RESET_ENDPOINT,
                         ((u32)slot << 24) | ((u32)dci << 16));
    u32 status = 0;
    if (!cmd_wait(trb, &status, 0, 2000000)) {
        if (dci == 1) ep0_last_recovery_cmd_cc = 0;
        return 0;
    }
    if (dci == 1) ep0_last_recovery_cmd_cc = (int)((status >> 24) & 0xFF);
    if (((status >> 24) & 0xFF) != 1) return 0;

    return set_endpoint_ring_head(slot, dci, ring,
                                  producer_enq, producer_cycle);
}

static int stop_endpoint_ring(int slot, int dci, u32 ring,
                              u32 *producer_enq, u32 *producer_cycle)
{
    if (!xhci_running() || slot <= 0) return 0;
    if (dci == 1) ep0_last_recovery_kind = 2;
    u32 trb = cmd_submit(0, 0, TRB_STOP_ENDPOINT,
                         ((u32)slot << 24) | ((u32)dci << 16));
    u32 status = 0;
    if (!cmd_wait(trb, &status, 0, 5000000)) {
        if (dci == 1) ep0_last_recovery_cmd_cc = 0;
        return 0;
    }
    if (dci == 1) ep0_last_recovery_cmd_cc = (int)((status >> 24) & 0xFF);
    if (((status >> 24) & 0xFF) != 1) return 0;
    return set_endpoint_ring_head(slot, dci, ring,
                                  producer_enq, producer_cycle);
}

static int reset_endpoint(int slot, int dci)
{
    if (slot <= 0 || slot >= MAX_SLOTS) return 0;
    return reset_endpoint_ring(slot, dci, EP0_RING(slot),
                               &ep0_enqueue[slot], &ep0_cycle[slot]);
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
    fix_ep0_last_cc = -1;
    if (speed >= 3) return 1;                 /* high speed and up are fixed */

    /* eight bytes is all we are allowed to assume we can read */
    if (!descriptor_in(slot, 0x01000680u, 0x00080000u, XMEM_DATA, 8)) {
        fix_ep0_last_cc = ep0_last_cc;
        return 0;
    }
    fix_ep0_last_cc = ep0_last_cc;
    int real_mps = (int)*(volatile u8 *)(XMEM_DATA + 7);
    if (real_mps <= 0) return 0;
    if (real_mps == ep0_mps(speed)) return 1;  /* already right */

    /* Evaluate Context edits a live device: A1 alone means "only EP0 changed" */
    zero_mem(CTX_INPUT, 33u * (u32)xctxsize);
    ctx_set(CTX_INPUT, 0, 1, (1u << 1));
    ctx_set(CTX_INPUT, 2, 1, (3u << 1) | (4u << 3) | ((u32)real_mps << 16));

    u32 trb = cmd_submit(dma_addr(CTX_INPUT), 0, TRB_EVALUATE_CTX, (u32)slot << 24);
    u32 status = 0;
    if (!cmd_wait(trb, &status, 0, 2000000)) {
        fix_ep0_last_cc = 0;
        return 0;
    }
    fix_ep0_last_cc = (int)((status >> 24) & 0xFF);
    return fix_ep0_last_cc == 1;
}

/* ---- one call that does the whole dance for a port ----------------------
 * Returns the slot number on success, 0 on failure. This is what the shell
 * calls: it is the difference between "a controller exists" and "zlOS knows
 * what is plugged into port 5". */
static int port_slot[32];       /* remembers what we already brought up */
static u16 slot_vid[MAX_SLOTS];
static u16 slot_pid[MAX_SLOTS];
static u8  slot_class[MAX_SLOTS];

/* Enumeration happens before any class driver knows whether a device is a
 * keyboard, camera, Bluetooth radio or the boot stick. Keep each root port's
 * own boundary so a later device cannot erase the failure that matters. */
enum {
    ENUM_IDLE = 0,
    ENUM_CONNECTED = 1,
    ENUM_PORT_READY = 2,
    ENUM_SLOT_ENABLED = 3,
    ENUM_ADDRESSED = 4,
    ENUM_EP0_READY = 5,
    ENUM_DESCRIPTOR = 6
};
static u8 enum_stage[32];
static signed char enum_cc[32];

static void enum_note(int port, int stage, int cc)
{
    if (port <= 0 || port >= 32) return;
    if (stage < enum_stage[port]) return;
    enum_stage[port] = (u8)stage;
    enum_cc[port] = (signed char)cc;
}

int xhci_enumerate(int port)
{
    if (!xhci_port_connected(port)) return 0;
    enum_note(port, ENUM_CONNECTED, -1);

    /* Enumeration is not idempotent at the hardware level - asking twice
     * allocates a second slot for the same physical device and leaks the
     * first. Remember the answer instead. */
    if (port > 0 && port < 32 && port_slot[port]) {
        cur_slot = port_slot[port];
        cur_port = port;
        cur_speed = xhci_port_speed(port);
        return port_slot[port];
    }

    /* USB3 ports self-enable; USB2 ports need the reset. Either way we need
     * PED set before the device will answer. */
    if (!xhci_port_enabled(port)) {
        if (!xhci_port_reset(port)) return 0;
    } else {
        xhci_port_reset(port);          /* harmless, and clears stale changes */
    }
    enum_note(port, ENUM_PORT_READY, -1);

    int speed = xhci_port_speed(port);
    int slot  = xhci_enable_slot();
    if (!slot) { enum_note(port, ENUM_SLOT_ENABLED, enable_slot_last_cc); return 0; }
    enum_note(port, ENUM_SLOT_ENABLED, enable_slot_last_cc);

    if (!xhci_address_device(slot, port, speed)) {
        enum_note(port, ENUM_ADDRESSED, address_last_cc);
        xhci_disable_slot(slot);
        return 0;
    }
    enum_note(port, ENUM_ADDRESSED, address_last_cc);
    if (!fix_ep0_packet_size(slot, speed)) {
        enum_note(port, ENUM_EP0_READY, fix_ep0_last_cc);
        xhci_disable_slot(slot);
        return 0;
    }
    enum_note(port, ENUM_EP0_READY, fix_ep0_last_cc);
    if (!xhci_get_device_descriptor(slot)) {
        enum_note(port, ENUM_DESCRIPTOR, ep0_last_cc);
        xhci_disable_slot(slot);
        return 0;
    }
    enum_note(port, ENUM_DESCRIPTOR, ep0_last_cc);

    /* XMEM_DATA is shared scratch and the next descriptor request overwrites
     * it. Keep identity with the slot so later class drivers do not report the
     * last device scanned as the device they are actually configuring. */
    slot_vid[slot] = (u16)xhci_desc_vendor();
    slot_pid[slot] = (u16)xhci_desc_product();
    slot_class[slot] = (u8)xhci_desc_byte(4); /* bDeviceClass */

    if (port > 0 && port < 32) port_slot[port] = slot;
    return slot;
}

/* A descriptor failure leaves a cached addressed slot behind. Reusing it is
 * not a retry: it reuses the same failed device state and the same EP0. Give
 * the finite slot back and force the next attempt through port reset, slot
 * allocation, Address Device and Device descriptor again. Callers must never
 * use this on a port already claimed by a live class driver. */
static int xhci_forget_port(int port)
{
    if (port <= 0 || port >= 32) return 0;
    int slot = port_slot[port];
    port_slot[port] = 0;
    if (slot > 0 && slot < MAX_SLOTS) {
        slot_vid[slot] = 0;
        slot_pid[slot] = 0;
        slot_class[slot] = 0;
        if (!xhci_disable_slot(slot)) return 0;
    }
    return 1;
}

/* Linux's default USB2 compatibility sequence first asks for a Device
 * descriptor at address zero (Address Device BSR=1), resets the physical
 * device, then performs ordinary addressing. Keep the pre-address slot
 * temporary so we never rewrite a live output context in place. */
static int xhci_enumerate_preaddress(int port)
{
    if (!xhci_port_connected(port)) return 0;
    if (!xhci_port_reset(port)) return 0;
    int speed = xhci_port_speed(port);
    int probe_slot = xhci_enable_slot();
    if (!probe_slot || probe_slot >= MAX_SLOTS) return 0;
    if (!xhci_address_device_mode(probe_slot, port, speed, 1)) {
        xhci_disable_slot(probe_slot);
        return 0;
    }
    int probed = descriptor_in(probe_slot, 0x01000680u, 0x00400000u,
                               XMEM_DATA, 64);
    int released = xhci_disable_slot(probe_slot);
    if (!probed || !released) return 0;
    /* xhci_enumerate() performs the second port reset before normal address. */
    return xhci_enumerate(port);
}

static int xhci_reenumerate_port(int port, int preaddress)
{
    if (!xhci_forget_port(port)) return 0;
    return preaddress ? xhci_enumerate_preaddress(port) : xhci_enumerate(port);
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
#define PROTOCOL_MOUSE   2

/* ---- the pointer ---------------------------------------------------------
 * A PS/2 mouse is RELATIVE: it reports "moved 3 right, 2 up" and the driver
 * accumulates. That is why a mouse in a window drifts out of step with the
 * host pointer - two separate positions, kept in sync only by luck, and the
 * guest cursor can never be made to point where the host one is. QEMU says so
 * plainly: query-mice reports "absolute": false.
 *
 * A USB tablet is ABSOLUTE. Every report carries the position itself, 0..32767
 * across each axis, so there is nothing to accumulate and nothing to drift.
 * The guest cursor lands exactly where the host cursor is, with no grab.
 *
 * Both shapes are handled here, because they are the same driver with a
 * different report layout:
 *   boot mouse (subclass 1, protocol 2): [buttons, dx, dy, wheel]  - relative
 *   tablet     (subclass 0, protocol 0): [buttons, xlo, xhi, ylo, yhi, wheel]
 * The tablet is not a BOOT device, so it is matched on being HID with an
 * interrupt IN endpoint and a 6-byte report rather than on the class triple. */
/* THE POINTER NEEDS A PIPELINE, AND THE KEYBOARD DOES NOT.
 *
 * One outstanding buffer is right for a keyboard: a key is an event, it happens
 * when a finger moves, and the guest reads it whenever it likes. It is WRONG
 * for a pointer, and this is what made the pointer jumpy after the merge.
 *
 * An interrupt endpoint only transfers when the driver has posted a buffer for
 * it. With exactly one outstanding, the sequence is: the controller completes
 * it, we notice at the next frame, we decode and post another. So the device
 * gets ONE service opportunity per frame no matter how often the bus offers
 * one - and every interval in between is a report the mouse had and could not
 * hand over. Measured in hosttest/xhcitest.c: 8 reports offered in a frame, 1
 * delivered, 7 service intervals starved.
 *
 * That is not merely latency. QEMU's usb-mouse (hw/input/hid.c) delivers at
 * most 127 counts per report and keeps the remainder, so a starved endpoint
 * turns a hand's motion into a queue that drains at 127 counts per FRAME. The
 * pointer then coasts after the hand stops, which is exactly what "jumpy,
 * laggy, unpredictable" describes.
 *
 * PTR_NBUF buffers, so the endpoint is always armed and the bus - not the
 * compositor's frame rate - decides how often the mouse is asked.
 *
 * EACH TRB MUST NAME ITS OWN BUFFER. Posting several TRBs that all point at
 * one buffer is worse than posting one: the controller fills the same bytes N
 * times and N-1 reports' worth of delta is overwritten before anything reads
 * it. The buffer is therefore derived from the TRB's own ring index, and the
 * completion event carries that TRB's address, so decode and post agree by
 * construction rather than by a counter that could drift. */
#define PTR_NBUF     8                       /* reports in flight at once      */
#define PTR_BUFSZ    64u                     /* what ptr_requeue clears        */
#define PTR_BUF0     (XMEM_DATA + 0x500)     /* first of PTR_NBUF buffers      */
#define PTR_BUF(i)   (PTR_BUF0 + ((u32)(i) % PTR_NBUF) * PTR_BUFSZ)
/* XMEM_DATA + 0x800 is xhci_ram_ok()'s upper probe address - stay under it. */
_Static_assert(PTR_BUF0 + PTR_NBUF * PTR_BUFSZ <= XMEM_DATA + 0x800,
               "the pointer report buffers overrun the xHCI DMA scratch");

/* THE RING MUST BE LONGER THAN THE PIPELINE, AND THIS COST A ROUND.
 *
 * The first version put the Link TRB at PTR_NBUF, making the ring exactly as
 * long as the buffer count. It worked for exactly PTR_NBUF reports and then
 * the endpoint went silent for good, which hosttest/xhcitest.c's liveness case
 * caught and a screenshot never would have.
 *
 * The cycle bit is the reason. A Link TRB is handed to the controller with the
 * producer's CURRENT cycle, and the controller follows it - toggling its own -
 * when it reaches it. With the pipeline as long as the ring, the producer laps
 * the consumer: we rewrite the Link with the NEXT cycle while the controller
 * is still one TRB short of reading it, so it arrives at a Link that no longer
 * matches, decides the ring is empty, and stops.
 *
 * PTR_RING_USE gives PTR_RING_USE - PTR_NBUF slots of slack between the two,
 * so the Link is always rewritten long before the controller gets there. It
 * must be a MULTIPLE of PTR_NBUF as well: the buffer is the TRB index modulo
 * the buffer count, and that is only collision-free across the wrap when the
 * ring length divides evenly by it. */
#define PTR_RING_USE 32                      /* usable TRBs; Link sits here    */
_Static_assert(PTR_RING_USE % PTR_NBUF == 0,
               "pointer ring length must divide by the buffer count, or two "
               "outstanding TRBs share a buffer across the wrap");
_Static_assert(PTR_RING_USE > PTR_NBUF,
               "the pointer ring must be longer than the pipeline, or the "
               "producer rewrites the Link TRB's cycle under the controller");
_Static_assert(PTR_RING_USE < RING_TRBS - 1, "the pointer ring needs a Link TRB");
#define PTR_ABS_MAX  32767                   /* HID logical maximum per axis   */

extern int console_pxw(void);
extern int console_pxh(void);

static int ptr_slot = 0, ptr_dci = 0, ptr_mps = 8, ptr_ready = 0;
static int ptr_port = 0;       /* a configured class driver owns this port */
static int ptr_abs  = 0;       /* 1 = absolute tablet, 0 = relative mouse     */
static u32 ptr_enq  = 0, ptr_cyc = 1;
static int ptr_x = 0, ptr_y = 0, ptr_btn = 0;
static int ptr_dx_acc = 0, ptr_dy_acc = 0;  /* raw relative motion, unclamped */
#define PTR_EDGE_N 16
static u8 ptr_edge_state[PTR_EDGE_N];
static u32 ptr_edge_head, ptr_edge_tail;
/* THE WHEEL, which this driver decoded in a comment and nowhere else. Both
 * report layouts documented above end in a wheel byte and both were dropped on
 * the floor, so idt_mouse_wheel() (PS/2) was the only source of a notch in the
 * whole kernel - and every probe here, and a real laptop, attach a USB
 * pointer. The consequence was not subtle: the app catalog is 47 tiles in a
 * window that shows twelve, and the wheel is its ONLY scroll control, so 35 of
 * them could not be reached by any pointer at all. */
static int ptr_wz_acc = 0;                  /* wheel notches, read-and-cleared */
static unsigned ptr_reports = 0;
static u32 ptr_batch_oldest_tsc;
static unsigned ptr_events  = 0;   /* EVERY dispatch, any cc */
static unsigned kbd_events  = 0;   /* keyboard dispatches, any cc */
static unsigned kbd_requeues= 0;   /* how many times it was re-armed */
static int      kbd_lastcc  = -1;
static int      ptr_lastcc  = -1;  /* the last completion code */

static int kbd_slot  = 0;      /* the device slot the keyboard lives in       */
static int kbd_port  = 0;      /* and the port, so nothing re-enumerates it   */
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
 *     bits  7:0   HID usage ID   (never 0 for a real key)
 *     bits 15:8   HID modifier bitmap from the same report
 *     bit  16     1 = press, 0 = release
 *     bit  17     modifier snapshot; usage and press are ignored
 *
 * Modifier snapshots are queued rather than exposed only as final state. A
 * Super press and release can both complete in one compositor frame; storing
 * only the last bitmap turns that real tap into 0 -> 0 and loses it forever.
 */
#define KEV(press, mods, usage) (((press) << 16) | ((mods) << 8) | (usage))
#define KEV_MOD(mods)           ((1 << 17) | ((mods) << 8))

static int kevq[32];
static u32 kevq_tsc[32], kevq_current_tsc, kevq_last_tsc;
static int kevq_head = 0, kevq_tail = 0;

static void kevq_push(int ev)
{
    int next = (kevq_tail + 1) & 31;
    if (next == kevq_head) return;
    kevq[kevq_tail] = ev;
    kevq_tsc[kevq_tail] = kevq_current_tsc;
    kevq_tail = next;
}

static int kevq_pop(void)
{
    if (kevq_head == kevq_tail) return 0;
    int ev = kevq[kevq_head];
    kevq_last_tsc = kevq_tsc[kevq_head];
    kevq_head = (kevq_head + 1) & 31;
    return ev;
}

static u32 xhci_tsc_lo(void)
{
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    (void)hi;
    return lo;
}

static u8 cfg_byte(int i)
{
    if (i < 0 || i >= CFG_MAX) return 0;
    return *(volatile u8 *)(uptr)(CFG_BUF + (u32)i);
}

/* GET_DESCRIPTOR(CONFIGURATION). The first nine bytes carry wTotalLength,
 * which is how much there really is - interfaces and endpoints follow the
 * configuration header in one contiguous block. */
static int get_config(int slot, int len)
{
    if (len > CFG_MAX) len = CFG_MAX;
    return descriptor_in(slot, 0x02000680u, ((u32)len << 16), CFG_BUF, len);
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

/* Configure Endpoint carries an INPUT slot context as well as the endpoint
 * contexts being added.  It is not a fresh slot: every dword must start from
 * the controller-owned OUTPUT slot context.  The old code copied only DW0/1
 * and silently zeroed DW2/3 (TT routing, interrupter target, device address
 * and slot state). QEMU ignores those zeros; real Intel xHCI is allowed to
 * reject the command with Parameter/Context State Error. */
static void copy_slot_context_for_endpoints(int slot, int top_dci)
{
    for (int dword = 0; dword < 4; dword++)
        ctx_set(CTX_INPUT, 1, dword,
                ctx_get(CTX_DEVICE(slot), 0, dword));
    u32 slot_dw0 = ctx_get(CTX_INPUT, 1, 0);
    slot_dw0 = (slot_dw0 & 0x07FFFFFFu) | ((u32)top_dci << 27);
    ctx_set(CTX_INPUT, 1, 0, slot_dw0);
}

/* Add the keyboard's interrupt IN endpoint to the device the controller
 * already knows about. Configure Endpoint is an incremental edit: the add
 * flags say which contexts in the input block are meaningful, and everything
 * else is left exactly as it is. */
static int configure_endpoint(int slot, int dci, int mps, int speed, int binterval)
{
    ring_init(INT_RING(slot));
    /* NOTE: this used to reset kbd_enq/kbd_cyc here, unconditionally, for any
     * slot. That was safe only while the keyboard was the sole caller. The
     * pointer calls this too, for ITS slot, and the reset then rewound the
     * KEYBOARD's producer index after the keyboard had already armed index 0:
     * its next requeue wrote index 0 again while the controller sat waiting at
     * index 1 with a cycle bit that would never match. One report, then dead
     * forever. Each device now resets the state it owns, next to its own
     * ring_init - the state is per-endpoint, so it does not belong here. */

    zero_mem(CTX_INPUT, 33u * (u32)xctxsize);
    /* A0 (the slot context, because Context Entries changes) plus this
     * endpoint. Bit N of the add flags means device context index N. */
    ctx_set(CTX_INPUT, 0, 1, (1u << 0) | (1u << dci));

    /* copy the slot context forward, but raise Context Entries to cover the
     * new endpoint - the controller uses it as "how many contexts are valid" */
    copy_slot_context_for_endpoints(slot, dci);

    /* the endpoint itself. Input context index is device context index + 1,
     * because index 0 of the input block is the input control context. */
    int ic = dci + 1;
    ctx_set(CTX_INPUT, ic, 0, (u32)interval_encode(speed, binterval) << 16);
    /* CErr=3, EP type 7 = Interrupt IN, and the packet size the descriptor
     * asked for */
    ctx_set(CTX_INPUT, ic, 1, (3u << 1) | (7u << 3) | ((u32)mps << 16));
    ctx_set(CTX_INPUT, ic, 2, (u32)dma_addr(INT_RING(slot)) | 1u);   /* dequeue ptr, DCS=1 */
    ctx_set(CTX_INPUT, ic, 3, (u32)(dma_addr(INT_RING(slot)) >> 32));
    /* average TRB length, and Max ESIT Payload in the high half - the most
     * this endpoint can move in one service interval */
    ctx_set(CTX_INPUT, ic, 4, (u32)mps | ((u32)mps << 16));

    u32 trb = cmd_submit(dma_addr(CTX_INPUT), 0, TRB_CONFIGURE_EP, (u32)slot << 24);
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
    trb_write(ring, kbd_enq, dma_addr(KBD_REPORT), (u32)kbd_mps,
              (TRB_NORMAL << 10) | (1u << 5) | kbd_cyc);   /* IOC */
    kbd_enq++;
    if (kbd_enq >= RING_TRBS - 1) {
        trb_write(ring, RING_TRBS - 1, dma_addr(ring), 0,
                  (TRB_LINK << 10) | (1u << 1) | kbd_cyc);
        kbd_enq = 0;
        kbd_cyc ^= 1;
    }
    doorbell((u32)kbd_slot, (u32)kbd_dci);
}

/* Post ONE buffer for the pointer, at the next free slot of its ring.
 *
 * Called PTR_NBUF times at bring-up to fill the pipeline, and once per
 * completion after that, so the number outstanding stays constant. */
static void ptr_requeue(void)
{
    u32 ring = INT_RING(ptr_slot);
    /* Clear the buffer before handing it back, for the same reason the
     * keyboard does: on a short packet the bytes the device did not send would
     * still hold an older report, and a stale delta is movement that never
     * happened. */
    zero_mem(PTR_BUF(ptr_enq), PTR_BUFSZ);
    trb_write(ring, ptr_enq, dma_addr(PTR_BUF(ptr_enq)), (u32)ptr_mps,
              (TRB_NORMAL << 10) | (1u << 5) | ptr_cyc);   /* IOC */
    ptr_enq++;
    if (ptr_enq >= PTR_RING_USE) {
        trb_write(ring, PTR_RING_USE, dma_addr(ring), 0,
                  (TRB_LINK << 10) | (1u << 1) | ptr_cyc);
        ptr_enq = 0;
        ptr_cyc ^= 1;
    }
    doorbell((u32)ptr_slot, (u32)ptr_dci);
}

/* Fill the pipeline. Called once at bring-up; after that each completion posts
 * its own replacement, so the outstanding count stays at PTR_NBUF.
 *
 * A function rather than a loop at the call site so that the depth the kernel
 * runs with and the depth hosttest/xhcitest.c arms are the same number by
 * construction - a harness that armed its own guess would pass while the
 * shipping driver starved. */
static void ptr_arm_all(void)
{
    for (int i = 0; i < PTR_NBUF; i++) ptr_requeue();
}

/* One report, from the buffer the completed TRB named. Absolute reports are
 * scaled from the HID 0..32767 range onto the live screen; relative ones are
 * accumulated and clamped like the PS/2 path. */
static void ptr_decode(u32 buf)
{
    volatile u8 *r = (volatile u8 *)(uptr)buf;
    int w = console_pxw(), h = console_pxh();
    if (w <= 0) w = 1920;
    if (h <= 0) h = 1200;

    int old_btn = ptr_btn;
    ptr_btn = (int)r[0] & 0x07;
    /* The wheel byte sits at the END of whichever layout this device uses:
     * index 3 on a 4-byte boot mouse, index 5 on a 6-byte tablet. It is
     * SIGNED, and it is a delta, so it accumulates the same way dx/dy do -
     * publishing it as a level would count a notch twice or not at all.
     * ptr_requeue() zeroes the whole buffer before handing it back, so a short
     * report reads 0 here rather than a stale notch. */
    {
        int wz = (int)r[ptr_abs ? 5 : 3];
        if (wz > 127) wz -= 256;
        ptr_wz_acc += wz;
    }
    if (ptr_abs) {
        int rx = (int)r[1] | ((int)r[2] << 8);
        int ry = (int)r[3] | ((int)r[4] << 8);
        /* Scale the HID 0..32767 range onto the screen. Multiplying by (w - 1)
         * rather than w is what makes the far edge reachable: at rx == 32767
         * this yields exactly w - 1, the last real column. Using w would need a
         * clamp to avoid landing one pixel off the end. */
        ptr_x = (int)(((long long)rx * (w - 1)) / PTR_ABS_MAX);
        ptr_y = (int)(((long long)ry * (h - 1)) / PTR_ABS_MAX);
    } else {
        int dx = (int)r[1], dy = (int)r[2];
        if (dx > 127) dx -= 256;              /* signed bytes */
        if (dy > 127) dy -= 256;
        /* THE RAW DELTA, PUBLISHED SEPARATELY AND UNCLAMPED.
         *
         * ptr_x/ptr_y below stay a 1:1 clamped position, because kernel.zl's
         * mouse_x() builtin reads them and has always meant "where the pointer
         * is, in pixels". But input.c's acceleration curve needs the DELTA,
         * and it cannot recover it by differencing two of those positions: the
         * clamp means consecutive samples stop differing the moment the
         * pointer is against an edge, so below 1x the accelerated pointer
         * could never reach that edge at all.
         *
         * Accumulated and read-and-cleared, which is the contract
         * idt_mouse_wheel() already uses for notches - a delta published as a
         * position is a delta that gets counted twice or not at all. */
        ptr_dx_acc += dx;
        ptr_dy_acc += dy;
        ptr_x += dx;
        ptr_y += dy;                          /* HID mice count Y downward */
    }
    if (ptr_x < 0) ptr_x = 0;
    if (ptr_y < 0) ptr_y = 0;
    if (ptr_x > w - 1) ptr_x = w - 1;
    if (ptr_y > h - 1) ptr_y = h - 1;
    if (ptr_btn != old_btn) {
        u32 next = (ptr_edge_head + 1u) % PTR_EDGE_N;
        if (next == ptr_edge_tail) {
            zlt_count(ZLLOG_C_INPUT_DROP, 1u);
            zlt_event(ZLLOG_SUB_INPUT, ZLLOG_EV_DROP, ZLLOG_ERROR,
                      2u /* USB button edge */, (unsigned)ptr_btn, PTR_EDGE_N);
        } else {
            ptr_edge_state[ptr_edge_head] = (u8)ptr_btn;
            ptr_edge_head = next;
        }
    }
    ptr_reports++;
}

/* ---- HID usage IDs to characters and keys -------------------------------
 * These are NOT ASCII and NOT PC scancodes - they are a third numbering, from
 * the HID Usage Tables. Usage 0x04 is 'a' and the alphabet runs contiguously
 * from there, which is why the letters and digits are computed rather than
 * tabulated; only the punctuation needs a table.
 *
 * Returns a character (< KEY_NONCHAR) for keys that have one, a KEY_* code for
 * keys that do not, and 0 for a usage this keyboard layout does not produce. */
#include "keycodes.h"
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

    /* Keys with no character. These returned 0 - indistinguishable from "no
     * key" - so the arrows the `=` demo tells you to try were dropped in the
     * driver and never reached the event queue at all. They come back as
     * KEY_* codes, which are >= KEY_NONCHAR and so cannot be mistaken for a
     * character by anything downstream. */
    switch (usage) {
        case 0x3A: return KEY_F1 + 0;   case 0x3B: return KEY_F1 + 1;
        case 0x3C: return KEY_F1 + 2;   case 0x3D: return KEY_F1 + 3;
        case 0x3E: return KEY_F1 + 4;   case 0x3F: return KEY_F1 + 5;
        case 0x40: return KEY_F1 + 6;   case 0x41: return KEY_F1 + 7;
        case 0x42: return KEY_F1 + 8;   case 0x43: return KEY_F1 + 9;
        case 0x44: return KEY_F1 + 10;  case 0x45: return KEY_F1 + 11;
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
        now[i] = *(volatile u8 *)(uptr)(KBD_REPORT + (u32)(i + 2));
        if (now[i] == 1) rollover = 1;
    }

    /* Publish and queue the modifier bitmap even when nothing else moved:
     * pressing shift ALONE sends a report with no usage IDs at all, so this is
     * the only moment anyone learns shift went down. The queue preserves both
     * edges when a tap's press and release complete in one input_poll(). */
    if (mods != (int)kbd_mods) kevq_push(KEV_MOD(mods));
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

        kbd_enq   = 0;              /* our ring, our producer state */
        kbd_cyc   = 1;
        kbd_slot  = slot;
        kbd_port  = port;
        kbd_iface = found_iface;
        kbd_dci   = dci;
        kbd_mps   = ep_mps;
        kbd_ready = 1;
        for (int i = 0; i < 6; i++) prev_keys[i] = 0;
        keyq_head  = keyq_tail  = 0;
        kevq_head  = kevq_tail  = 0;
        kevq_current_tsc = kevq_last_tsc = 0;
        kbd_mods   = 0;

        kbd_requeue();                        /* arm the first read */
        return slot;
    }
    return 0;
}

/* Find a pointing device and arm it. Same shape as xhci_kbd_init, matching a
 * different interface: any HID interface whose protocol is not KEYBOARD. A
 * tablet declares subclass 0 / protocol 0 because it is not a boot device, so
 * "is it HID with an interrupt IN endpoint and not a keyboard" is the test
 * that catches both a tablet and a boot mouse. */
int xhci_ptr_init(void)
{
    if (!xhci_running()) return 0;
    if (ptr_ready) return ptr_slot;

    for (int port = 1; port <= xports; port++) {
        if (!xhci_port_connected(port)) continue;

        /* Skip the keyboard's port BEFORE touching it. xhci_enumerate() resets
         * the port and re-addresses the device, so calling it on a device that
         * is already configured pulls a working keyboard out from under
         * itself - the slot check afterwards is far too late, the damage is
         * done by then.
         *
         * NOTE, for whoever extends this: the same hazard applies to any port
         * whose device is already in use, and only the keyboard is tracked.
         * The mass-storage port IS re-enumerated here. That is safe only
         * because ptr_ready short-circuits this function, so it runs once at
         * bring-up, before '/' has claimed storage. If pointer discovery is
         * ever re-run later, track claimed ports generally instead. */
        if (port == kbd_port) continue;

        int slot = xhci_enumerate(port);
        if (!slot || slot >= MAX_SLOTS) continue;
        if (slot == kbd_slot) continue;

        if (!get_config(slot, 9)) continue;
        int total = (int)cfg_byte(2) | ((int)cfg_byte(3) << 8);
        if (total < 9) continue;
        if (total > CFG_MAX) total = CFG_MAX;
        if (!get_config(slot, total)) continue;

        int cfgval = (int)cfg_byte(5);
        int found_iface = -1, is_boot_mouse = 0;
        int ep_addr = 0, ep_mps = 0, ep_int = 1;

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
                if (cls == CLASS_HID && prot != PROTOCOL_KEYBOARD) {
                    found_iface   = (int)cfg_byte(off + 2);
                    is_boot_mouse = (sub == SUBCLASS_BOOT && prot == PROTOCOL_MOUSE);
                    ep_addr = 0;
                } else if (found_iface >= 0 && ep_addr == 0) {
                    found_iface = -1;
                }
            } else if (dtype == DESC_ENDPOINT && dlen >= 7 &&
                       found_iface >= 0 && ep_addr == 0) {
                int addr = (int)cfg_byte(off + 2);
                int attr = (int)cfg_byte(off + 3);
                if ((addr & 0x80) && (attr & 0x03) == 3) {
                    ep_addr = addr;
                    ep_mps  = (int)cfg_byte(off + 4) |
                              (((int)cfg_byte(off + 5) & 0x07) << 8);
                    ep_int  = (int)cfg_byte(off + 6);
                }
            }
            off += dlen;
        }

        if (found_iface < 0 || ep_addr == 0) continue;

        if (!set_configuration(slot, cfgval)) continue;
        /* A boot mouse is asked for boot protocol so its report is the fixed
         * 4-byte layout. A tablet must NOT be: boot protocol would give us a
         * relative mouse report and throw away the absolute position, which is
         * the entire reason for using one. */
        if (is_boot_mouse) set_boot_protocol(slot, found_iface);
        set_idle(slot, found_iface);

        int dci = ((ep_addr & 0x0F) * 2) + 1;
        if (dci < 2 || dci > 31) continue;
        if (ep_mps <= 0 || ep_mps > 1024) ep_mps = 8;

        if (!configure_endpoint(slot, dci, ep_mps, xhci_port_speed(port), ep_int))
            continue;

        ptr_enq   = 0;              /* our ring, our producer state */
        ptr_cyc   = 1;
        ptr_edge_head = ptr_edge_tail = 0;
        ptr_batch_oldest_tsc = 0;
        ptr_btn = 0;
        ptr_dx_acc = ptr_dy_acc = ptr_wz_acc = 0;
        ptr_slot  = slot;
        ptr_port  = port;
        ptr_dci   = dci;
        ptr_mps   = ep_mps;
        /* the tablet's report is 6 bytes and carries a position; the boot
         * mouse's is 4 and carries deltas */
        ptr_abs   = !is_boot_mouse && ep_mps >= 6;
        ptr_ready = 1;
        ptr_x = console_pxw() / 2;
        ptr_y = console_pxh() / 2;
        /* Fill the pipeline, not just prime it. One buffer means the device is
         * only asked once per frame; PTR_NBUF means the bus sets the rate. */
        ptr_arm_all();
        return slot;
    }
    return 0;
}

int xhci_ptr_ready(void)   { return ptr_ready; }
int xhci_ptr_abs(void)     { return ptr_abs; }
/* Read-and-clear: how far a RELATIVE mouse has moved since anyone last asked.
 * Meaningless for a tablet, which reports a position; ptr_decode only ever
 * accumulates these on the relative path. */
int xhci_ptr_take_wheel(void) { int v = ptr_wz_acc; ptr_wz_acc = 0; return v; }
int xhci_ptr_take_dx(void) { int v = ptr_dx_acc; ptr_dx_acc = 0; return v; }
int xhci_ptr_take_dy(void) { int v = ptr_dy_acc; ptr_dy_acc = 0; return v; }
int xhci_ptr_x(void)       { return ptr_x; }
int xhci_ptr_y(void)       { return ptr_y; }
int xhci_ptr_btn(void)     { return ptr_btn; }
int xhci_ptr_take_button(void)
{
    if (ptr_edge_tail == ptr_edge_head) return -1;
    int state = ptr_edge_state[ptr_edge_tail];
    ptr_edge_tail = (ptr_edge_tail + 1u) % PTR_EDGE_N;
    return state;
}
u32 xhci_ptr_take_tsc(void)
{
    u32 tsc = ptr_batch_oldest_tsc;
    ptr_batch_oldest_tsc = 0;
    return tsc;
}
unsigned xhci_ptr_reports(void) { return ptr_reports; }
unsigned xhci_ptr_events(void)  { return ptr_events; }
int      xhci_ptr_lastcc(void)  { return ptr_lastcc; }
unsigned xhci_kbd_events(void)  { return kbd_events; }
unsigned xhci_kbd_requeues(void){ return kbd_requeues; }
int      xhci_kbd_lastcc(void)  { return kbd_lastcc; }
int xhci_ptr_slot(void)    { return ptr_slot; }
int xhci_ptr_ep(void)      { return ptr_dci; }

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
    if (kbd_ready && ptr_ready) return kbd_slot;

    if (!owned) {
        if (!xhci_present() && xhci_find() < 0) return 0;
        if (!xhci_reset())      return 0;     /* take it away from the firmware */
        if (!xhci_init_rings()) return 0;     /* and give it OUR data structures */
        owned = 1;
    }
    int k = kbd_ready ? kbd_slot : xhci_kbd_init();
    /* A pointer is optional - a machine with no tablet and no USB mouse is
     * still perfectly usable, so never let this fail the bring-up. */
    if (!ptr_ready) xhci_ptr_init();
    return k;
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
static void kbd_event(u32 param, u32 status, u32 ctrl)
{
    int slot = (int)((ctrl >> 24) & 0xFF);
    int epid = (int)((ctrl >> 16) & 0x1F);
    int cc   = (int)((status >> 24) & 0xFF);

    if (ecm_event(param, status, ctrl)) return;

    /* ONE event ring carries both HID devices, so whichever poll runs first
     * sees the other's completions too. Dispatch on slot+endpoint rather than
     * assuming; dropping a transfer event also drops the requeue, and that
     * endpoint then goes silent for good. */
    if (ptr_ready && slot == ptr_slot && epid == ptr_dci) {
        ptr_events++;              /* counted BEFORE any cc filter */
        ptr_lastcc = cc;
        if (cc == 1 || cc == 13) {
            if (!ptr_batch_oldest_tsc) ptr_batch_oldest_tsc = xhci_tsc_lo();
            /* WHICH buffer. A Transfer Event's parameter is the address of the
             * TRB that produced it, and that TRB's index is the buffer index -
             * so the report is read from the bytes this completion is actually
             * about, not from whichever buffer was filled most recently. With
             * PTR_NBUF in flight those are different buffers, and reading the
             * wrong one replays or skips a delta. */
            u32 ring = INT_RING(ptr_slot);
            /* param is the DEVICE address of the TRB that completed; ring is a
             * KERNEL address. Same reasoning as cmd_wait above - see dma.h. */
            u32 pk   = (u32)dma_kaddr(param);
            u32 idx  = (pk - ring) / TRB_BYTES;
            /* A completion pointing outside our ring is not ours to decode.
             * Requeue anyway - the endpoint's liveness must not depend on the
             * event making sense. */
            if (pk >= ring && idx < PTR_RING_USE) ptr_decode(PTR_BUF(idx));
        }
        ptr_requeue();
        return;
    }
    if (!kbd_ready) return;
    if (slot != kbd_slot || epid != kbd_dci) return;
    kbd_events++;
    kbd_lastcc = cc;
    if (cc == 1 || cc == 13) {
        kevq_current_tsc = xhci_tsc_lo();
        hid_decode();                         /* success or short packet */
    }
    kbd_requeue();
    kbd_requeues++;
}

/* ONE DRAINER FOR ONE RING.
 *
 * The merge left two, and they were byte-for-byte the same function:
 * xhci_kbd_poll() from the branch that owned the keyboard, xhci_ptr_poll()
 * from the branch that owned the pointer. Neither was wrong on its own. The
 * damage was that input.c called them at different rates - the pointer's once
 * per frame from pump_mouse(), the keyboard's in a loop of up to sixteen - and
 * they took events off the SAME ring. A pointer report drawn by the keyboard
 * loop was decoded after pump_mouse() had already sampled the position, so it
 * did not move the pointer until the next frame; and a keystroke sitting in
 * front of a pointer report meant pump_mouse() spent its single poll on the
 * keystroke and the pointer did not move at all that frame.
 *
 * So: one function owns the ring, it runs once per frame before anything reads
 * the decoded state, and it drains generously rather than exactly once. The
 * two names below survive because the zl builtins `usb_poll` and `mouse_x`
 * call them from outside input.c; they are now wrappers, not owners.
 *
 * `max` bounds the WHOLE loop, not the transfer events found, so a burst of
 * port-status changes cannot hold the caller either. */
int xhci_poll(int max)
{
    if (!ptr_ready && !kbd_ready && !ecm_ready) return 0;

    int got = 0;
    for (int i = 0; i < max; i++) {
        u32 param = 0, status = 0, ctrl = 0;
        int type = event_poll(&param, &status, &ctrl, 1);
        if (!type) break;                    /* the ring is empty - done */
        if (type != TRB_TRANSFER_EVENT) continue;   /* consumed, ignored */
        kbd_event(param, status, ctrl);
        got++;
    }
    return got;
}

int xhci_kbd_poll(void) { return xhci_poll(1); }
int xhci_ptr_poll(void) { return xhci_poll(PTR_NBUF + 2); }

/* One character, or 0 if nothing was typed.
 *
 * This is the older, narrower view, kept because the zl `usb_key` builtin
 * compares what it gets against 13 and 27. It reads its OWN queue rather than
 * the event queue, so the shell reading characters and the compositor reading
 * events cannot steal keystrokes from each other. */
int xhci_key(void)
{
    /* Drain until a KEY comes out, or the ring is empty.
     *
     * Polling once and returning whatever happens to be in the queue was fine
     * while the keyboard was the ONLY device on this controller. It is not any
     * more: the pointer shares this event ring, so a poll that pops a POINTER
     * completion leaves the key queue empty and returns 0 - and input.c reads
     * that as "no more keys" and stops draining (input.c:271). With a pointer
     * producing events steadily, real keystrokes queue up behind them and
     * never surface. The keyboard looks completely dead while the ring is in
     * fact busy, and it only recovers if something else drains far enough.
     * Bounded so a flood cannot livelock the caller. */
    for (int i = 0; i < 32; i++) {
        int c = keyq_pop();
        if (c) return c;
        if (!xhci_kbd_poll()) break;        /* ring is empty - nothing queued */
    }
    return keyq_pop();
}

/* One key event - press or release, with the usage ID and the modifiers of the
 * report it came in. This is what input.c reads, and the only view through
 * which a key with no character can reach an application.
 *
 * IT DOES NOT POLL. It used to, and that is what made it a second drainer of
 * the pointer's ring: input.c calls this in a loop, so a pointer report drawn
 * here was decoded after pump_mouse() had already read the position. The
 * caller drains once, at the top of input_poll(), and this reads what that
 * drain decoded. */
int xhci_key_event(void)
{
    return kevq_pop();
}
u32 xhci_key_event_tsc(void) { return kevq_last_tsc; }

/* The live modifier bitmap, for a shift that is held with nothing else. */
int xhci_kbd_mods(void)
{
    return (int)kbd_mods;
}

/* raw report bytes, for showing what the hardware actually sent */
int xhci_kbd_report(int i)
{
    if (i < 0 || i > 7) return 0;
    return (int)*(volatile u8 *)(uptr)(KBD_REPORT + (u32)i);
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
#define MSC_IN_RING(s)   (XMEM_DCBAA + 0x500000u + (u32)(s) * RING_STRIDE)
#define MSC_OUT_RING(s)  (XMEM_DCBAA + 0x508000u + (u32)(s) * RING_STRIDE)
#define MSC_CBW          (XMEM_DCBAA + 0x510000u)
#define MSC_CSW          (XMEM_DCBAA + 0x510200u)
#define MSC_DATA         (XMEM_DCBAA + 0x511000u)
#define MSC_DATA_MAX     4096u

/* The mass-storage buffers are the highest thing xhci.c touches, so this is
 * where the arena's ceiling gets checked. The per-slot rings are indexed by
 * MAX_SLOTS, which is exactly the aliasing hazard the EP0_RING comment warns
 * about above - raising MAX_SLOTS now fails the build instead of silently
 * overlapping the next ring set. */
_Static_assert(XMEM_SCRATCH + SCRATCH_BYTES <= MSC_IN_RING(0),
               "xhci: the scratchpad pages collide with the MSC rings");
_Static_assert(MSC_IN_RING(MAX_SLOTS)  <= MSC_OUT_RING(0),
               "xhci: MSC in-rings alias the out-rings (MAX_SLOTS too large)");
_Static_assert(MSC_OUT_RING(MAX_SLOTS) <= MSC_CBW,
               "xhci: MSC out-rings alias the command block wrapper");
_Static_assert((unsigned long)MSC_DATA + MSC_DATA_MAX <= HI_VGPU,
               "xhci: the DMA arena escapes into virtio-gpu's region");

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

/* Persistent-observer bring-up must distinguish "no stick" from "Intel
 * rejected the endpoint context". These are RAM-only and allocation-free.
 * The scalar fields are a compact first-on-tie summary; the per-port table
 * below is authoritative and prevents unrelated devices hiding the disk. */
enum {
    MSC_INIT_IDLE = 0,
    MSC_INIT_CONTROLLER = 1,
    MSC_INIT_PORT = 2,
    MSC_INIT_ENUMERATED = 3,
    MSC_INIT_CONFIG_HEAD = 4,
    MSC_INIT_CONFIG_FULL = 5,
    MSC_INIT_INTERFACE = 6,
    MSC_INIT_SET_CONFIG = 7,
    MSC_INIT_ENDPOINTS = 8,
    MSC_INIT_READY = 9
};
static int msc_init_stage;
static int msc_init_port;
static int msc_init_slot;
static int msc_init_cc = -1;
static int msc_init_vid;
static int msc_init_pid;
static u8 msc_port_stage[32];
static u8 msc_port_slot[32];
static signed char msc_port_cc[32];
static u16 msc_port_vid[32];
static u16 msc_port_pid[32];
static u8 msc_port_candidate[32];
static u8 msc_port_ep0_event[32];
static u8 msc_port_ep0_attempts[32];
static signed char msc_port_ep0_recovery[32];
static u32 msc_port_ep0_trace[32][EP0_TRACE_WORDS];

static void msc_note_init(int stage, int port, int slot, int cc)
{
    if (port > 0 && port < 32) {
        msc_port_stage[port] = (u8)stage;
        msc_port_slot[port] = (u8)slot;
        msc_port_cc[port] = (signed char)cc;
        if (slot > 0 && slot < MAX_SLOTS) {
            msc_port_vid[port] = slot_vid[slot];
            msc_port_pid[port] = slot_pid[slot];
        }
        if (cc >= 0 && ep0_last_event_stage >= 0)
            msc_port_ep0_event[port] = (u8)ep0_last_event_stage;
        if (cc >= 0) {
            msc_port_ep0_attempts[port] = (u8)ep0_last_attempts;
            msc_port_ep0_recovery[port] = (signed char)ep0_last_recovery;
            for (int i = 0; i < EP0_TRACE_WORDS; i++)
                msc_port_ep0_trace[port][i] = ep0_last_trace[i];
        }
    }
    /* Keep the deepest first port as the compact legacy summary. Equal-stage
     * attempts on Bluetooth/camera ports must not overwrite an earlier disk.
     * The v2 firmware record below retains every port, so this is only a
     * screen-sized summary rather than the sole source of truth. */
    if (stage < msc_init_stage ||
        (stage == msc_init_stage && port != msc_init_port)) return;
    msc_init_stage = stage;
    msc_init_port = port;
    msc_init_slot = slot;
    msc_init_cc = cc;
    if (slot > 0 && slot < MAX_SLOTS) {
        msc_init_vid = slot_vid[slot];
        msc_init_pid = slot_pid[slot];
    }
}

struct msc_config {
    int cfgval, iface;
    int in_ep, out_ep;
    int in_mps, out_mps;
};

/* Parse independently from transport so the exact physical Imation
 * descriptor can live in the host regression suite. Descriptor bytes are
 * still read from the driver's bounded CFG_BUF; no device-controlled pointer
 * ever enters the kernel. */
static int msc_parse_config_descriptor(int total, struct msc_config *out)
{
    if (!out || total < 9 || total > CFG_MAX) return 0;
    out->cfgval = (int)cfg_byte(5);
    out->iface = -1;
    out->in_ep = out->out_ep = 0;
    out->in_mps = out->out_mps = 0;

    int off = (int)cfg_byte(0);
    while (off + 1 < total) {
        int dlen  = (int)cfg_byte(off);
        int dtype = (int)cfg_byte(off + 1);
        if (dlen < 2 || off + dlen > total) break;

        if (dtype == DESC_INTERFACE && dlen >= 9) {
            int cls  = (int)cfg_byte(off + 5);
            int sub  = (int)cfg_byte(off + 6);
            int prot = (int)cfg_byte(off + 7);
            if (cls == 0x08 && sub == 0x06 && prot == 0x50) {
                out->iface = (int)cfg_byte(off + 2);
                out->in_ep = out->out_ep = 0;
            } else if (out->iface >= 0 && (!out->in_ep || !out->out_ep)) {
                out->iface = -1;
            }
        } else if (dtype == DESC_ENDPOINT && dlen >= 7 && out->iface >= 0) {
            int addr = (int)cfg_byte(off + 2);
            int attr = (int)cfg_byte(off + 3);
            int mps  = (int)cfg_byte(off + 4) |
                       (((int)cfg_byte(off + 5) & 0x07) << 8);
            if ((attr & 0x03) == 2) {
                if ((addr & 0x80) && !out->in_ep) {
                    out->in_ep = addr; out->in_mps = mps;
                }
                if (!(addr & 0x80) && !out->out_ep) {
                    out->out_ep = addr; out->out_mps = mps;
                }
            }
        }
        off += dlen;
    }
    return out->iface >= 0 && out->in_ep && out->out_ep;
}

/* The journal writer needs more than a yes/no when a stick rejects a write.
 * Keep the complete outcome of the last command in fixed state: xHCI's
 * completion code, the Bulk-Only CSW status/residue, and (when the target
 * reports command failure) the decoded REQUEST SENSE triplet.  No allocation,
 * and no pointer to MSC_DATA escapes -- the next command reuses that buffer. */
enum {
    MSC_RESULT_OK = 0,
    MSC_RESULT_BAD_ARGUMENT,
    MSC_RESULT_NOT_READY,
    MSC_RESULT_CBW_TRANSFER,
    MSC_RESULT_DATA_TRANSFER,
    MSC_RESULT_CSW_TRANSFER,
    MSC_RESULT_CSW_SIGNATURE,
    MSC_RESULT_CSW_TAG,
    MSC_RESULT_CSW_FAILED,
    MSC_RESULT_CSW_PHASE,
    MSC_RESULT_CAPACITY_UNSUPPORTED
};
static int msc_last_result = MSC_RESULT_NOT_READY;
static int msc_last_xhci_cc = 0;
static int msc_last_csw_status = -1;
static u32 msc_last_residue = 0;
static int msc_last_recovery = -1;       /* -1 not attempted, 0 failed, 1 ok */
static int msc_sense_valid = 0;
static int msc_sense_key = 0, msc_sense_asc = 0, msc_sense_ascq = 0;
static int msc_last_opcode = 0;

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

    copy_slot_context_for_endpoints(slot, top);

    int ic_in = msc_in_dci + 1;
    ctx_set(CTX_INPUT, ic_in, 0, 0);                  /* bulk has no interval */
    ctx_set(CTX_INPUT, ic_in, 1, (3u << 1) | ((u32)EPTYPE_BULK_IN << 3) | ((u32)msc_in_mps << 16));
    ctx_set(CTX_INPUT, ic_in, 2, (u32)dma_addr(MSC_IN_RING(slot)) | 1u);
    ctx_set(CTX_INPUT, ic_in, 3, (u32)(dma_addr(MSC_IN_RING(slot)) >> 32));
    ctx_set(CTX_INPUT, ic_in, 4, (u32)msc_in_mps);

    int ic_out = msc_out_dci + 1;
    ctx_set(CTX_INPUT, ic_out, 0, 0);
    ctx_set(CTX_INPUT, ic_out, 1, (3u << 1) | ((u32)EPTYPE_BULK_OUT << 3) | ((u32)msc_out_mps << 16));
    ctx_set(CTX_INPUT, ic_out, 2, (u32)dma_addr(MSC_OUT_RING(slot)) | 1u);
    ctx_set(CTX_INPUT, ic_out, 3, (u32)(dma_addr(MSC_OUT_RING(slot)) >> 32));
    ctx_set(CTX_INPUT, ic_out, 4, (u32)msc_out_mps);

    u32 trb = cmd_submit(dma_addr(CTX_INPUT), 0, TRB_CONFIGURE_EP, (u32)slot << 24);
    u32 status = 0;
    if (!cmd_wait(trb, &status, 0, 5000000)) {
        msc_init_cc = 0;
        return 0;
    }
    msc_init_cc = (int)((status >> 24) & 0xFF);
    return msc_init_cc == 1;
}

/* Bulk-Only Reset Recovery, exactly once and never as an unbounded retry:
 * class reset, clear both USB endpoint halts, then replace both xHCI transfer
 * rings and their dequeue pointers. reset_endpoint() cannot be used here --
 * it deliberately points EP0 at EP0_RING, while these endpoints own MSC rings. */
static int msc_reset_recovery(void)
{
    if (!msc_slot || !msc_in_dci || !msc_out_dci) return 0;
    int in_ep = 0x80 | ((msc_in_dci - 1) / 2);
    int out_ep = msc_out_dci / 2;
    int ok = xhci_control_in(msc_slot, 0x0000FF21u,
                             (u32)msc_iface, 0, 0); /* BOT class reset */
    ok &= xhci_control_in(msc_slot, 0x00000102u,
                          (u32)in_ep, 0, 0);        /* CLEAR_FEATURE(HALT) */
    ok &= xhci_control_in(msc_slot, 0x00000102u,
                          (u32)out_ep, 0, 0);
    ok &= reset_endpoint_ring(msc_slot, msc_in_dci, MSC_IN_RING(msc_slot),
                              &msc_in_enq, &msc_in_cyc);
    ok &= reset_endpoint_ring(msc_slot, msc_out_dci, MSC_OUT_RING(msc_slot),
                              &msc_out_enq, &msc_out_cyc);
    return ok ? 1 : 0;
}

/* One bulk transfer: a single Normal TRB with interrupt-on-completion. Bulk
 * endpoints have no schedule, so this is as simple as USB gets. */
static int bulk_xfer(int slot, int dci, u32 buf, u32 len, int is_in)
{
    u32 ring = is_in ? MSC_IN_RING(slot) : MSC_OUT_RING(slot);
    u32 *enq = is_in ? &msc_in_enq : &msc_out_enq;
    u32 *cyc = is_in ? &msc_in_cyc : &msc_out_cyc;

    u32 trb_addr = ring + *enq * TRB_BYTES;
    trb_write(ring, *enq, dma_addr(buf), len,
              (TRB_NORMAL << 10) | (1u << 5) | *cyc);
    (*enq)++;
    if (*enq >= RING_TRBS - 1) {
        trb_write(ring, RING_TRBS - 1, dma_addr(ring), 0,
                  (TRB_LINK << 10) | (1u << 1) | *cyc);
        *enq = 0;
        *cyc ^= 1;
    }
    doorbell((u32)slot, (u32)dci);

    u32 status = 0, ctrl = 0;
    /* This bulk endpoint, not whatever completes first: the pointer shares
     * this ring and a disk read must not be able to kill it. */
    if (!xfer_wait_trbs_ms(slot, dci, &trb_addr, 1,
                           &status, &ctrl, 2000)) {
        msc_last_xhci_cc = 0;
        return 0;
    }
    int cc = (int)((status >> 24) & 0xFF);
    msc_last_xhci_cc = cc;
    if (cc == 1 || cc == 13) return 1;
    if (cc == 6 || cc == 4) {
        /* The failed command may have stopped between CBW, data and CSW. A
         * single-endpoint reset cannot re-synchronise those three phases. */
        msc_last_recovery = msc_reset_recovery();
    }
    return 0;
}

/* Build a Command Block Wrapper. The signature and the tag are how the device
 * tells our commands apart from noise, and the tag comes back in the status
 * wrapper so a reply can be matched to its request. */
static u32 build_cbw(u32 data_len, int is_in, const u8 *cdb, int cdb_len)
{
    volatile u8 *w = (volatile u8 *)MSC_CBW;
    for (int i = 0; i < 31; i++) w[i] = 0;
    volatile u32 *d = (volatile u32 *)MSC_CBW;
    d[0] = 0x43425355u;               /* 'USBC'                       */
    u32 tag = msc_tag++;
    d[1] = tag;                       /* our tag                      */
    d[2] = data_len;
    w[12] = is_in ? 0x80 : 0x00;      /* direction                    */
    w[13] = 0;                        /* LUN 0                        */
    w[14] = (u8)cdb_len;
    for (int i = 0; i < cdb_len && i < 16; i++) w[15 + i] = cdb[i];
    return tag;
}

static void msc_clear_sense(void)
{
    msc_sense_valid = 0;
    msc_sense_key = msc_sense_asc = msc_sense_ascq = 0;
}

/* Decode fixed-format (70h/71h) and descriptor-format (72h/73h) sense. */
static int msc_parse_sense(u32 len)
{
    volatile u8 *d = (volatile u8 *)MSC_DATA;
    int response = len ? (d[0] & 0x7F) : 0;
    if ((response == 0x70 || response == 0x71) && len >= 14) {
        msc_sense_key = d[2] & 0x0F;
        msc_sense_asc = d[12];
        msc_sense_ascq = d[13];
    } else if ((response == 0x72 || response == 0x73) && len >= 4) {
        msc_sense_key = d[1] & 0x0F;
        msc_sense_asc = d[2];
        msc_sense_ascq = d[3];
    } else {
        return 0;
    }
    msc_sense_valid = 1;
    return 1;
}

static int msc_parse_csw(u32 expected_tag)
{
    volatile u32 *c = (volatile u32 *)MSC_CSW;
    if (c[0] != 0x53425355u) {                 /* 'USBS' */
        msc_last_result = MSC_RESULT_CSW_SIGNATURE;
        return 0;
    }
    if (c[1] != expected_tag) {
        msc_last_result = MSC_RESULT_CSW_TAG;
        return 0;
    }
    msc_last_residue = c[2];
    msc_last_csw_status = *(volatile u8 *)(MSC_CSW + 12);
    if (msc_last_csw_status == 0) {
        msc_last_result = MSC_RESULT_OK;
        return 1;
    }
    msc_last_result = (msc_last_csw_status == 1)
                    ? MSC_RESULT_CSW_FAILED : MSC_RESULT_CSW_PHASE;
    return 0;
}

static int msc_trace_done(int ok, u32 tag)
{
    zlt_event(ZLLOG_SUB_STORAGE, ZLLOG_EV_COMMAND_COMPLETE,
              ok ? ZLLOG_INFO : ZLLOG_ERROR,
              ((unsigned)msc_last_opcode << 24) | (tag & 0x00ffffffu),
              (unsigned)msc_last_result,
              ((unsigned)(msc_last_csw_status & 0xff) << 24) |
              (msc_last_residue & 0x00ffffffu));
    return ok;
}

/* Run one SCSI command through the transport, without automatic recovery. */
static int scsi_cmd_raw(const u8 *cdb, int cdb_len, u32 data_len, int is_in)
{
    msc_last_csw_status = -1;
    msc_last_residue = 0;
    msc_last_xhci_cc = 0;
    msc_last_recovery = -1;
    msc_last_opcode = (cdb && cdb_len > 0) ? cdb[0] : 0;

    /* Validate BEFORE sending a CBW. Sending a promise of a data phase and
     * only then discovering the staging buffer is too small desynchronises
     * the Bulk-Only state machine for the next command. */
    if (!cdb || cdb_len < 1 || cdb_len > 16 || data_len > MSC_DATA_MAX) {
        msc_last_result = MSC_RESULT_BAD_ARGUMENT;
        return 0;
    }
    if (!msc_slot) {
        msc_last_result = MSC_RESULT_NOT_READY;
        return 0;
    }
    u32 tag = build_cbw(data_len, is_in, cdb, cdb_len);
    zlt_event(ZLLOG_SUB_STORAGE, ZLLOG_EV_COMMAND_SUBMIT, ZLLOG_INFO,
              ((unsigned)cdb[0] << 24) | (tag & 0x00ffffffu),
              data_len, (unsigned)is_in);

    if (!bulk_xfer(msc_slot, msc_out_dci, MSC_CBW, 31, 0)) {
        msc_last_result = MSC_RESULT_CBW_TRANSFER;
        return msc_trace_done(0, tag);
    }

    if (data_len) {
        if (is_in) zero_mem(MSC_DATA, data_len);
        if (!bulk_xfer(msc_slot, is_in ? msc_in_dci : msc_out_dci,
                       MSC_DATA, data_len, is_in)) {
            msc_last_result = MSC_RESULT_DATA_TRANSFER;
            return msc_trace_done(0, tag);
        }
    }

    zero_mem(MSC_CSW, 16);
    if (!bulk_xfer(msc_slot, msc_in_dci, MSC_CSW, 13, 1)) {
        msc_last_result = MSC_RESULT_CSW_TRANSFER;
        return msc_trace_done(0, tag);
    }

    int ok = msc_parse_csw(tag);
    if (!ok && (msc_last_result == MSC_RESULT_CSW_SIGNATURE
             || msc_last_result == MSC_RESULT_CSW_TAG
             || msc_last_result == MSC_RESULT_CSW_PHASE))
        msc_last_recovery = msc_reset_recovery();
    return msc_trace_done(ok, tag);
}

static int msc_request_sense_raw(void)
{
    u8 cdb[6] = { 0x03, 0, 0, 0, 18, 0 };
    if (!scsi_cmd_raw(cdb, 6, 18, 1)) return 0;
    return msc_parse_sense(18);
}

/* Command failure is followed once by REQUEST SENSE. Preserve the failed
 * command's status while retaining the decoded sense from the recovery
 * command, so diagnostics describe the operation the caller actually made. */
static int scsi_cmd(const u8 *cdb, int cdb_len, u32 data_len, int is_in)
{
    msc_clear_sense();
    if (scsi_cmd_raw(cdb, cdb_len, data_len, is_in)) return 1;
    if (msc_last_result != MSC_RESULT_CSW_FAILED) return 0;

    int saved_result = msc_last_result;
    int saved_xhci = msc_last_xhci_cc;
    int saved_csw = msc_last_csw_status;
    int saved_opcode = msc_last_opcode;
    u32 saved_residue = msc_last_residue;
    (void)msc_request_sense_raw();
    msc_last_result = saved_result;
    msc_last_xhci_cc = saved_xhci;
    msc_last_csw_status = saved_csw;
    msc_last_residue = saved_residue;
    msc_last_opcode = saved_opcode;
    return 0;
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
    if (msc_last_residue) {
        msc_last_result = MSC_RESULT_DATA_TRANSFER;
        return 0;
    }
    /* both fields are BIG endian - SCSI predates the x86 monoculture */
    volatile u8 *d = (volatile u8 *)MSC_DATA;
    u32 last = ((u32)d[0] << 24) | ((u32)d[1] << 16) | ((u32)d[2] << 8) | d[3];
    u32 blen = ((u32)d[4] << 24) | ((u32)d[5] << 16) | ((u32)d[6] << 8) | d[7];
    /* READ CAPACITY(10)'s all-ones sentinel means the device needs the 16-byte
     * command and its block count cannot fit this driver's u32 API. Refuse
     * instead of wrapping last + 1 to zero. */
    if (last == 0xFFFFFFFFu || !blen || blen > MSC_DATA_MAX) {
        msc_blocks = 0;
        msc_last_result = MSC_RESULT_CAPACITY_UNSUPPORTED;
        return 0;
    }
    msc_blocks    = last + 1;
    msc_blocksize = blen;
    return 1;
}

static int msc_range_ok(u32 lba, u32 count)
{
    if (!msc_ready || !msc_slot || !msc_blocks || !msc_blocksize) return 0;
    if (!count || count > 0xFFFFu) return 0;
    if (msc_blocksize > MSC_DATA_MAX) return 0;
    if (count > MSC_DATA_MAX / msc_blocksize) return 0;
    /* Subtraction makes the bound overflow-proof even at UINT32_MAX. */
    if (lba >= msc_blocks || count > msc_blocks - lba) return 0;
    return 1;
}

static void msc_build_rw10(u8 *cdb, int opcode, u32 lba, u32 count)
{
    for (int i = 0; i < 10; i++) cdb[i] = 0;
    cdb[0] = (u8)opcode;
    cdb[2] = (u8)(lba >> 24); cdb[3] = (u8)(lba >> 16);
    cdb[4] = (u8)(lba >> 8);  cdb[5] = (u8)lba;
    cdb[7] = (u8)(count >> 8); cdb[8] = (u8)count;
}

static void msc_copy_from_data(void *dst, u32 bytes)
{
    u8 *out = (u8 *)dst;
    volatile u8 *in = (volatile u8 *)MSC_DATA;
    for (u32 i = 0; i < bytes; i++) out[i] = in[i];
}

static void msc_copy_to_data(const void *src, u32 bytes)
{
    const u8 *in = (const u8 *)src;
    volatile u8 *out = (volatile u8 *)MSC_DATA;
    for (u32 i = 0; i < bytes; i++) out[i] = in[i];
}

static int msc_rw10(int opcode, u32 lba, u32 count)
{
    if (!msc_range_ok(lba, count)) {
        msc_last_result = (!msc_ready || !msc_slot || !msc_blocks)
                        ? MSC_RESULT_NOT_READY : MSC_RESULT_BAD_ARGUMENT;
        return 0;
    }
    u8 cdb[10];
    msc_build_rw10(cdb, opcode, lba, count);
    if (!scsi_cmd(cdb, 10, msc_blocksize * count, opcode == 0x28)) return 0;
    /* A short block read/write is not success even if the target marked the
     * command passed. Never copy or commit a partially transferred sector. */
    if (msc_last_residue) {
        msc_last_result = MSC_RESULT_DATA_TRANSFER;
        return 0;
    }
    return 1;
}

/* Compatibility API: leave one block in MSC_DATA for msc_byte(). */
int xhci_msc_read_block(u32 lba)
{
    return msc_rw10(0x28, lba, 1);              /* READ(10) */
}

/* Bounded block-buffer APIs for native C drivers. One command can move at
 * most the fixed 4 KiB staging window; callers split larger I/O explicitly. */
int xhci_msc_read_blocks(u32 lba, void *dst, u32 count)
{
    if (!dst) {
        msc_last_result = MSC_RESULT_BAD_ARGUMENT;
        return 0;
    }
    if (!msc_rw10(0x28, lba, count)) return 0;   /* READ(10) */
    msc_copy_from_data(dst, msc_blocksize * count);
    return 1;
}

int xhci_msc_write_blocks(u32 lba, const void *src, u32 count)
{
    if (!src || !msc_range_ok(lba, count)) {
        msc_last_result = (!msc_ready || !msc_slot || !msc_blocks)
                        ? MSC_RESULT_NOT_READY : MSC_RESULT_BAD_ARGUMENT;
        return 0;
    }
    msc_copy_to_data(src, msc_blocksize * count);
    return msc_rw10(0x2A, lba, count);           /* WRITE(10) */
}

int xhci_msc_sync_cache(void)
{
    if (!msc_ready || !msc_slot || !msc_blocks) {
        msc_last_result = MSC_RESULT_NOT_READY;
        return 0;
    }
    u8 cdb[10] = { 0x35, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    return scsi_cmd(cdb, 10, 0, 0);             /* SYNCHRONIZE CACHE(10) */
}

int xhci_msc_request_sense(void)
{
    if (!msc_ready || !msc_slot) {
        msc_last_result = MSC_RESULT_NOT_READY;
        return 0;
    }
    msc_clear_sense();
    return msc_request_sense_raw();
}

int xhci_msc_byte(int i)
{
    if (i < 0 || i >= (int)MSC_DATA_MAX) return 0;
    return (int)*(volatile u8 *)(uptr)(MSC_DATA + (u32)i);
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
int xhci_msc_max_blocks_per_io(void)
{
    if (!msc_blocksize || msc_blocksize > MSC_DATA_MAX) return 0;
    return (int)(MSC_DATA_MAX / msc_blocksize);
}
int xhci_msc_last_result(void)     { return msc_last_result; }
int xhci_msc_last_xhci_cc(void)    { return msc_last_xhci_cc; }
int xhci_msc_last_csw_status(void) { return msc_last_csw_status; }
u32 xhci_msc_last_residue(void)    { return msc_last_residue; }
int xhci_msc_last_recovery(void)   { return msc_last_recovery; }
int xhci_msc_last_opcode(void)     { return msc_last_opcode; }
int xhci_msc_sense_valid(void)     { return msc_sense_valid; }
int xhci_msc_sense_key(void)       { return msc_sense_key; }
int xhci_msc_sense_asc(void)       { return msc_sense_asc; }
int xhci_msc_sense_ascq(void)      { return msc_sense_ascq; }
int xhci_msc_init_stage(void)      { return msc_init_stage; }
int xhci_msc_init_port(void)       { return msc_init_port; }
int xhci_msc_init_slot(void)       { return msc_init_slot; }
int xhci_msc_init_cc(void)         { return msc_init_cc; }
int xhci_msc_init_vid(void)        { return msc_init_vid; }
int xhci_msc_init_pid(void)        { return msc_init_pid; }
int xhci_enum_stage(int port)
{
    return port > 0 && port < 32 ? enum_stage[port] : 0;
}
int xhci_enum_cc(int port)
{
    return port > 0 && port < 32 ? enum_cc[port] : -1;
}
int xhci_msc_port_stage(int port)
{
    return port > 0 && port < 32 ? msc_port_stage[port] : 0;
}
int xhci_msc_port_slot(int port)
{
    return port > 0 && port < 32 ? msc_port_slot[port] : 0;
}
int xhci_msc_port_cc(int port)
{
    return port > 0 && port < 32 ? msc_port_cc[port] : -1;
}
int xhci_msc_port_vid(int port)
{
    return port > 0 && port < 32 ? msc_port_vid[port] : 0;
}
int xhci_msc_port_pid(int port)
{
    return port > 0 && port < 32 ? msc_port_pid[port] : 0;
}
int xhci_msc_port_candidate(int port)
{
    return port > 0 && port < 32 ? msc_port_candidate[port] : 0;
}
int xhci_msc_port_ep0_event(int port)
{
    return port > 0 && port < 32 ? msc_port_ep0_event[port] : 0;
}
int xhci_msc_port_ep0_attempts(int port)
{
    return port > 0 && port < 32 ? msc_port_ep0_attempts[port] : 0;
}
int xhci_msc_port_ep0_recovery(int port)
{
    return port > 0 && port < 32 ? msc_port_ep0_recovery[port] : -1;
}
u32 xhci_msc_port_ep0_trace(int port, int word)
{
    if (port <= 0 || port >= 32 || word < 0 || word >= EP0_TRACE_WORDS) return 0;
    return msc_port_ep0_trace[port][word];
}

/* Try the cached addressed device once, then perform at most two genuine
 * whole-device retries. The first uses the ordinary reset/address sequence;
 * the second adds Linux's address-zero Device-descriptor compatibility
 * preflight. Every retry gets a fresh slot and EP0 ring. */
static int msc_get_config_head(int port, int *slot_io)
{
    if (!slot_io || *slot_io <= 0) return 0;
    for (int scheme = 0; scheme <= 2; scheme++) {
        int slot = *slot_io;
        if (scheme > 0) {
            slot = xhci_reenumerate_port(port, scheme == 2);
            if (!slot || slot >= MAX_SLOTS) {
                ep0_note_config_reenumeration(port, scheme, 0xFF, 0);
                continue;
            }
            *slot_io = slot;
            msc_note_init(MSC_INIT_ENUMERATED, port, slot, -1);
        }

        msc_note_init(MSC_INIT_CONFIG_HEAD, port, slot, -1);
        int ok = get_config(slot, 9);
        msc_note_init(MSC_INIT_CONFIG_HEAD, port, slot, ep0_last_cc);
        if (scheme > 0)
            ep0_note_config_reenumeration(port, scheme, ep0_last_cc, ok);
        if (ok) return 1;
    }
    return 0;
}

/* Find a bulk-only mass storage device and bring it up. The interface triple
 * is class 8 (mass storage), subclass 6 (SCSI transparent), protocol 0x50
 * (bulk-only) - anything else is a different transport we do not speak. */
int xhci_msc_init(void)
{
    if (msc_ready) return msc_slot;
    msc_init_stage = MSC_INIT_CONTROLLER;
    msc_init_port = msc_init_slot = 0;
    msc_init_cc = -1;
    msc_init_vid = msc_init_pid = 0;
    for (int port = 0; port < 32; port++) {
        msc_port_stage[port] = 0;
        msc_port_slot[port] = 0;
        msc_port_cc[port] = -1;
        msc_port_vid[port] = 0;
        msc_port_pid[port] = 0;
        msc_port_candidate[port] = 0;
        msc_port_ep0_event[port] = 0;
        msc_port_ep0_attempts[port] = 0;
        msc_port_ep0_recovery[port] = -1;
        for (int i = 0; i < EP0_TRACE_WORDS; i++)
            msc_port_ep0_trace[port][i] = 0;
    }
    if (!xhci_running()) return 0;

    for (int port = 1; port <= xports; port++) {
        if (!xhci_port_connected(port)) continue;
        /* Never reset a configured HID device merely because storage is being
         * rescanned. Its class driver owns the live slot and endpoint rings. */
        if ((kbd_ready && port == kbd_port) || (ptr_ready && port == ptr_port))
            continue;
        msc_note_init(MSC_INIT_PORT, port, 0, -1);
        int slot = xhci_enumerate(port);
        if (!slot || slot >= MAX_SLOTS) continue;
        msc_note_init(MSC_INIT_ENUMERATED, port, slot, -1);

        if (!msc_get_config_head(port, &slot)) continue;
        int total = (int)cfg_byte(2) | ((int)cfg_byte(3) << 8);
        if (total < 9) continue;
        if (total > CFG_MAX) total = CFG_MAX;
        msc_note_init(MSC_INIT_CONFIG_FULL, port, slot, -1);
        if (!get_config(slot, total)) {
            msc_note_init(MSC_INIT_CONFIG_FULL, port, slot, ep0_last_cc);
            continue;
        }
        msc_note_init(MSC_INIT_CONFIG_FULL, port, slot, ep0_last_cc);

        struct msc_config cfg;
        if (!msc_parse_config_descriptor(total, &cfg)) continue;
        if (port > 0 && port < 32) msc_port_candidate[port] = 1;
        msc_note_init(MSC_INIT_INTERFACE, port, slot, ep0_last_cc);

        msc_in_dci  = ((cfg.in_ep  & 0x0F) * 2) + 1;
        msc_out_dci = ((cfg.out_ep & 0x0F) * 2);
        if (msc_in_dci < 2 || msc_in_dci > 31)  continue;
        if (msc_out_dci < 2 || msc_out_dci > 31) continue;
        if (cfg.in_mps  <= 0 || cfg.in_mps  > 1024) cfg.in_mps  = 512;
        if (cfg.out_mps <= 0 || cfg.out_mps > 1024) cfg.out_mps = 512;
        msc_in_mps = cfg.in_mps; msc_out_mps = cfg.out_mps;
        msc_slot = slot; msc_iface = cfg.iface;

        msc_note_init(MSC_INIT_SET_CONFIG, port, slot, -1);
        if (!set_configuration(slot, cfg.cfgval)) {
            msc_note_init(MSC_INIT_SET_CONFIG, port, slot, ep0_last_cc);
            msc_slot = 0;
            continue;
        }
        msc_note_init(MSC_INIT_SET_CONFIG, port, slot, ep0_last_cc);
        msc_note_init(MSC_INIT_ENDPOINTS, port, slot, -1);
        if (!configure_bulk(slot)) {
            msc_note_init(MSC_INIT_ENDPOINTS, port, slot, msc_init_cc);
            msc_slot = 0;
            continue;
        }
        msc_note_init(MSC_INIT_ENDPOINTS, port, slot, msc_init_cc);

        msc_ready = 1;
        msc_note_init(MSC_INIT_READY, port, slot, msc_init_cc);
        return slot;
    }
    return 0;
}

/* ==== CDC-ECM Ethernet ====================================================
 * A standards-based USB Ethernet/tethering device is the shortest physical
 * network path on machines whose built-in NIC is hidden behind a proprietary
 * dongle. CDC-ECM uses one communications interface for descriptors/control
 * and one alternate data interface carrying ordinary Ethernet frames over a
 * bulk IN/OUT pair. No RNDIS framing and no private device protocol is
 * accepted here.
 *
 * Four receive buffers stay posted. Transfer completions share xHCI's one
 * event ring with HID and mass storage, so ecm_event() is called by the common
 * dispatcher and queues the exact buffer named by the completed TRB. A buffer
 * is not re-posted until netdev_poll has copied it out; DMA can therefore never
 * overwrite a frame waiting in the software queue. */

#define ECM_IN_RING(s)  (XMEM_DCBAA + 0x600000u + (u32)(s) * RING_STRIDE)
#define ECM_OUT_RING(s) (XMEM_DCBAA + 0x608000u + (u32)(s) * RING_STRIDE)
#define ECM_RX_BUF0     (XMEM_DCBAA + 0x610000u)
#define ECM_RX_N        4
#define ECM_FRAME_MAX   2048u
#define ECM_RX_BUF(i)   (ECM_RX_BUF0 + (u32)(i) * ECM_FRAME_MAX)
#define ECM_TX_BUF      (ECM_RX_BUF0 + ECM_RX_N * ECM_FRAME_MAX)
#define ECM_MAC_DESC    (ECM_TX_BUF + ECM_FRAME_MAX)

_Static_assert(ECM_IN_RING(MAX_SLOTS) <= ECM_OUT_RING(0),
               "xhci: CDC-ECM IN rings alias OUT rings");
_Static_assert(ECM_OUT_RING(MAX_SLOTS) <= ECM_RX_BUF0,
               "xhci: CDC-ECM OUT rings alias frame buffers");
_Static_assert(ECM_MAC_DESC + 256u <= HI_VGPU,
               "xhci: CDC-ECM DMA buffers escape the USB arena");

struct ecm_config {
    int cfgval, comm_iface, data_iface, data_alt;
    int in_ep, out_ep, in_mps, out_mps;
    int mac_index, max_segment;
};

static int ecm_slot, ecm_port, ecm_comm_iface, ecm_data_iface;
static int ecm_in_dci, ecm_out_dci, ecm_in_mps, ecm_out_mps;
static u32 ecm_in_enq, ecm_in_cyc = 1, ecm_out_enq, ecm_out_cyc = 1;
static signed char ecm_rx_map[RING_TRBS];
static u16 ecm_rx_len[ECM_RX_N];
static u8 ecm_rx_q[8];
static int ecm_rx_qh, ecm_rx_qt;
static u8 ecm_mac_addr[6];
static int ecm_n_tx, ecm_n_rx, ecm_n_drop, ecm_n_full;
static int ecm_last_cc;
static int ecm_init_stage, ecm_config_index = -1;
static u32 ecm_parse_bits;
static u8 ecm_diag_config[128];
static int ecm_diag_len;

static void ecm_note(int stage)
{
    if (stage > ecm_init_stage) ecm_init_stage = stage;
}

static int ecm_parse_config_descriptor(int total, struct ecm_config *out)
{
    if (!out || total < 9 || total > CFG_MAX) return 0;
    out->cfgval = (int)cfg_byte(5);
    out->comm_iface = out->data_iface = -1;
    out->data_alt = -1;
    out->in_ep = out->out_ep = 0;
    out->in_mps = out->out_mps = 0;
    out->mac_index = 0;
    out->max_segment = 1514;

    int current_iface = -1, current_alt = 0, current_class = -1;
    int off = (int)cfg_byte(0);
    while (off + 1 < total) {
        int dlen = (int)cfg_byte(off);
        int type = (int)cfg_byte(off + 1);
        if (dlen < 2 || off + dlen > total) return 0;

        if (type == DESC_INTERFACE && dlen >= 9) {
            current_iface = (int)cfg_byte(off + 2);
            current_alt = (int)cfg_byte(off + 3);
            current_class = (int)cfg_byte(off + 5);
            int sub = (int)cfg_byte(off + 6);
            if (current_class == 0x02 && sub == 0x06 &&
                out->comm_iface < 0)
                out->comm_iface = current_iface;
            if (current_class == 0x0A && current_alt > 0) {
                out->data_iface = current_iface;
                out->data_alt = current_alt;
                out->in_ep = out->out_ep = 0;
            }
        } else if (type == 0x24 && dlen >= 4 &&
                   current_iface == out->comm_iface) {
            int subtype = (int)cfg_byte(off + 2);
            if (subtype == 0x0F && dlen >= 13) {
                out->mac_index = (int)cfg_byte(off + 3);
                out->max_segment = (int)cfg_byte(off + 8) |
                                   ((int)cfg_byte(off + 9) << 8);
            } else if (subtype == 0x06 && dlen >= 5) {
                /* Union descriptor names the slave data interface. Retain it
                 * even when its alternate setting appears later. */
                if (out->data_iface < 0) out->data_iface = (int)cfg_byte(off + 4);
            }
        } else if (type == DESC_ENDPOINT && dlen >= 7 &&
                   current_class == 0x0A && current_iface == out->data_iface &&
                   current_alt == out->data_alt) {
            int addr = (int)cfg_byte(off + 2);
            int attr = (int)cfg_byte(off + 3);
            int mps = (int)cfg_byte(off + 4) |
                      (((int)cfg_byte(off + 5) & 0x07) << 8);
            if ((attr & 3) == 2) {
                if ((addr & 0x80) && !out->in_ep) {
                    out->in_ep = addr; out->in_mps = mps;
                } else if (!(addr & 0x80) && !out->out_ep) {
                    out->out_ep = addr; out->out_mps = mps;
                }
            }
        }
        off += dlen;
    }
    ecm_parse_bits = (out->cfgval > 0 ? 1u : 0u) |
        (out->comm_iface >= 0 ? 2u : 0u) |
        (out->data_iface >= 0 ? 4u : 0u) |
        (out->data_alt > 0 ? 8u : 0u) |
        (out->in_ep ? 16u : 0u) | (out->out_ep ? 32u : 0u) |
        (out->mac_index > 0 ? 64u : 0u) |
        (out->max_segment >= 64 && out->max_segment <= (int)ECM_FRAME_MAX
             ? 128u : 0u);
    return out->cfgval > 0 && out->comm_iface >= 0 &&
           out->data_iface >= 0 && out->data_alt > 0 &&
           out->in_ep && out->out_ep && out->mac_index > 0 &&
           out->max_segment >= 64 && out->max_segment <= (int)ECM_FRAME_MAX;
}

static int ecm_hex(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int ecm_get_mac(int slot, int index)
{
    if (index <= 0 || index > 255) return 0;
    u32 base = 0x03000680u | ((u32)index << 16);
    if (!descriptor_in(slot, base, (64u << 16) | 0x0409u,
                       ECM_MAC_DESC, 64)) return 0;
    int len = *(volatile u8 *)(uptr)ECM_MAC_DESC;
    if (len < 26 || len > 64 ||
        *(volatile u8 *)(uptr)(ECM_MAC_DESC + 1) != 3) return 0;
    for (int i = 0; i < 6; i++) {
        int hi = ecm_hex(*(volatile u8 *)(uptr)(ECM_MAC_DESC + 2u + (u32)i * 4u));
        int lo = ecm_hex(*(volatile u8 *)(uptr)(ECM_MAC_DESC + 4u + (u32)i * 4u));
        if (hi < 0 || lo < 0) return 0;
        ecm_mac_addr[i] = (u8)((hi << 4) | lo);
    }
    return 1;
}

static int ecm_get_config(int slot, int index, int len)
{
    if (index < 0 || index > 255 || len <= 0 || len > CFG_MAX) return 0;
    return descriptor_in(slot, 0x02000680u | ((u32)index << 16),
                         (u32)len << 16, CFG_BUF, len);
}

static int ecm_configure_bulk(int slot)
{
    ring_init(ECM_IN_RING(slot));
    ring_init(ECM_OUT_RING(slot));
    ecm_in_enq = ecm_out_enq = 0;
    ecm_in_cyc = ecm_out_cyc = 1;
    for (int i = 0; i < RING_TRBS; i++) ecm_rx_map[i] = -1;

    int top = ecm_in_dci > ecm_out_dci ? ecm_in_dci : ecm_out_dci;
    zero_mem(CTX_INPUT, 33u * (u32)xctxsize);
    ctx_set(CTX_INPUT, 0, 1,
            (1u << 0) | (1u << ecm_in_dci) | (1u << ecm_out_dci));
    copy_slot_context_for_endpoints(slot, top);

    int ci = ecm_in_dci + 1;
    ctx_set(CTX_INPUT, ci, 0, 0);
    ctx_set(CTX_INPUT, ci, 1, (3u << 1) |
            ((u32)EPTYPE_BULK_IN << 3) | ((u32)ecm_in_mps << 16));
    ctx_set(CTX_INPUT, ci, 2, (u32)dma_addr(ECM_IN_RING(slot)) | 1u);
    ctx_set(CTX_INPUT, ci, 3, (u32)(dma_addr(ECM_IN_RING(slot)) >> 32));
    ctx_set(CTX_INPUT, ci, 4, (u32)ecm_in_mps);

    int co = ecm_out_dci + 1;
    ctx_set(CTX_INPUT, co, 0, 0);
    ctx_set(CTX_INPUT, co, 1, (3u << 1) |
            ((u32)EPTYPE_BULK_OUT << 3) | ((u32)ecm_out_mps << 16));
    ctx_set(CTX_INPUT, co, 2, (u32)dma_addr(ECM_OUT_RING(slot)) | 1u);
    ctx_set(CTX_INPUT, co, 3, (u32)(dma_addr(ECM_OUT_RING(slot)) >> 32));
    ctx_set(CTX_INPUT, co, 4, (u32)ecm_out_mps);

    u32 trb = cmd_submit(dma_addr(CTX_INPUT), 0, TRB_CONFIGURE_EP,
                         (u32)slot << 24);
    u32 status = 0;
    if (!cmd_wait(trb, &status, 0, 5000000)) return 0;
    ecm_last_cc = (int)((status >> 24) & 0xFF);
    return ecm_last_cc == 1;
}

static void ecm_rx_post(int bi)
{
    if (!ecm_ready || bi < 0 || bi >= ECM_RX_N) return;
    u32 ring = ECM_IN_RING(ecm_slot);
    u32 idx = ecm_in_enq;
    zero_mem(ECM_RX_BUF(bi), ECM_FRAME_MAX);
    ecm_rx_map[idx] = (signed char)bi;
    trb_write(ring, idx, dma_addr(ECM_RX_BUF(bi)), ECM_FRAME_MAX,
              (TRB_NORMAL << 10) | (1u << 5) | ecm_in_cyc);
    ecm_in_enq++;
    if (ecm_in_enq >= RING_TRBS - 1) {
        trb_write(ring, RING_TRBS - 1, dma_addr(ring), 0,
                  (TRB_LINK << 10) | (1u << 1) | ecm_in_cyc);
        ecm_in_enq = 0;
        ecm_in_cyc ^= 1;
    }
    doorbell((u32)ecm_slot, (u32)ecm_in_dci);
}

static int ecm_event(u32 param, u32 status, u32 ctrl)
{
    if (!ecm_ready) return 0;
    int slot = (int)((ctrl >> 24) & 0xFF);
    int dci = (int)((ctrl >> 16) & 0x1F);
    if (slot != ecm_slot) return 0;
    if (dci == ecm_out_dci) return 1; /* stale completion after a TX timeout */
    if (dci != ecm_in_dci) return 0;

    u32 pk = (u32)dma_kaddr(param);
    u32 ring = ECM_IN_RING(ecm_slot);
    if (pk < ring || pk >= ring + (RING_TRBS - 1) * TRB_BYTES) {
        ecm_n_drop++;
        return 1;
    }
    int idx = (int)((pk - ring) / TRB_BYTES);
    int bi = ecm_rx_map[idx];
    ecm_rx_map[idx] = -1;
    if (bi < 0 || bi >= ECM_RX_N) { ecm_n_drop++; return 1; }

    int cc = (int)((status >> 24) & 0xFF);
    int residual = (int)(status & 0x00FFFFFFu);
    int len = (int)ECM_FRAME_MAX - residual;
    int next = (ecm_rx_qt + 1) & 7;
    /* A frame whose length is an exact multiple of the endpoint max packet
     * may be followed by a zero-length packet. That terminates the USB
     * transfer; it is not a truncated Ethernet frame and must not poison the
     * drop counter. */
    if ((cc == 1 || cc == 13) && len == 0) {
        ecm_rx_post(bi);
        return 1;
    }
    if ((cc != 1 && cc != 13) || len < 14 || len > (int)ECM_FRAME_MAX ||
        next == ecm_rx_qh) {
        ecm_n_drop++;
        ecm_rx_post(bi);
        return 1;
    }
    ecm_rx_len[bi] = (u16)len;
    ecm_rx_q[ecm_rx_qt] = (u8)bi;
    ecm_rx_qt = next;
    return 1;
}

/* A class reported in the device descriptor is a cheap, authoritative
 * negative filter. CDC functions may be declared on the device itself (02),
 * behind an interface association (EF), or only in interface descriptors
 * (00). Everything else is definitely not CDC-ECM and must not be reset just
 * to rediscover that fact. This matters on the ThinkPad: its AX201 Bluetooth
 * device reports E0 on several companion root ports, and probing each of
 * those as Ethernet kept the graphical boot behind minutes of bounded USB
 * recovery. Unknown class zero remains probeable, so composite tethering
 * devices continue to work. */
int xhci_ecm_device_class_candidate(int cls)
{
    return cls == 0x00 || cls == 0x02 || cls == 0xEF;
}

int xhci_ecm_init(void)
{
    if (ecm_ready) return 1;
    ecm_init_stage = 1;
    ecm_config_index = -1;
    ecm_parse_bits = 0;
    ecm_diag_len = 0;
    if (!owned) (void)xhci_bringup();
    if (!xhci_running()) return 0;

    for (int port = 1; port <= xports; port++) {
        if (!xhci_port_connected(port)) continue;
        if ((kbd_ready && port == kbd_port) || (ptr_ready && port == ptr_port))
            continue;
        if (msc_ready && port == msc_init_port) continue;
        int known_slot = port > 0 && port < 32 ? port_slot[port] : 0;
        if (known_slot > 0 && known_slot < MAX_SLOTS &&
            !xhci_ecm_device_class_candidate(slot_class[known_slot]))
            continue;
        ecm_note(2);
        /* HID/storage discovery may already have asked this unclaimed device
         * for configuration zero. Re-enumerate only a port no live class
         * driver owns, then ask for the CDC configuration first. */
        int slot = xhci_reenumerate_port(port, 0);
        if (!slot || slot >= MAX_SLOTS) continue;
        ecm_note(3);
        struct ecm_config cfg;
        int matched = 0;
        /* Composite USB networking devices commonly expose RNDIS as their
         * first configuration and standards-based CDC-ECM as their second.
         * Configuration descriptor index is not bConfigurationValue: inspect
         * each bounded index and later select the value carried by the match. */
        static const u8 config_order[4] = { 1, 0, 2, 3 };
        for (int oi = 0; oi < 4 && !matched; oi++) {
            int ci = config_order[oi];
            if (!ecm_get_config(slot, ci, 9)) continue;
            ecm_note(4);
            int total = (int)cfg_byte(2) | ((int)cfg_byte(3) << 8);
            if (total < 9) continue;
            if (total > CFG_MAX) total = CFG_MAX;
            if (!ecm_get_config(slot, ci, total)) continue;
            ecm_note(5);
            ecm_diag_len = total < (int)sizeof ecm_diag_config
                         ? total : (int)sizeof ecm_diag_config;
            for (int di = 0; di < ecm_diag_len; di++)
                ecm_diag_config[di] = (u8)cfg_byte(di);
            matched = ecm_parse_config_descriptor(total, &cfg);
            if (matched) ecm_config_index = ci;
        }
        if (!matched) continue;
        ecm_note(6);
        ecm_slot = slot; ecm_port = port;
        ecm_comm_iface = cfg.comm_iface; ecm_data_iface = cfg.data_iface;
        ecm_in_dci = ((cfg.in_ep & 0x0F) * 2) + 1;
        ecm_out_dci = (cfg.out_ep & 0x0F) * 2;
        ecm_in_mps = cfg.in_mps; ecm_out_mps = cfg.out_mps;
        if (ecm_in_dci < 2 || ecm_in_dci > 31 ||
            ecm_out_dci < 2 || ecm_out_dci > 31 ||
            ecm_in_mps <= 0 || ecm_in_mps > 1024 ||
            ecm_out_mps <= 0 || ecm_out_mps > 1024 ||
            !ecm_get_mac(slot, cfg.mac_index)) {
            ecm_slot = 0;
            continue;
        }
        ecm_note(7);
        if (!set_configuration(slot, cfg.cfgval)) {
            ecm_last_cc = ep0_last_cc; ecm_slot = 0; continue;
        }
        ecm_note(8);
        if (!xhci_control_in(slot, 0x00000B01u | ((u32)cfg.data_alt << 16),
                             (u32)cfg.data_iface, 0, 0)) {
            ecm_last_cc = ep0_last_cc; ecm_slot = 0; continue;
        }
        ecm_note(9);
        if (!ecm_configure_bulk(slot)) {
            ecm_slot = 0; continue;
        }
        ecm_note(10);
        if (!xhci_control_in(slot, 0x000C4321u,
                             (u32)cfg.comm_iface, 0, 0)) {
            ecm_last_cc = ep0_last_cc;
            ecm_slot = 0;
            continue;
        }
        ecm_note(11);
        ecm_rx_qh = ecm_rx_qt = 0;
        ecm_n_tx = ecm_n_rx = ecm_n_drop = ecm_n_full = 0;
        ecm_ready = 1;
        ecm_note(12);
        for (int i = 0; i < ECM_RX_N; i++) ecm_rx_post(i);
        return 1;
    }
    return 0;
}

int xhci_ecm_send(const u8 *frame, int len)
{
    if (!ecm_ready || !frame || len < 14 || len > (int)ECM_FRAME_MAX) return 0;
    for (int i = 0; i < len; i++)
        *(volatile u8 *)(uptr)(ECM_TX_BUF + (u32)i) = frame[i];
    u32 ring = ECM_OUT_RING(ecm_slot);
    u32 idx = ecm_out_enq;
    u32 trb_addr = ring + idx * TRB_BYTES;
    trb_write(ring, idx, dma_addr(ECM_TX_BUF), (u32)len,
              (TRB_NORMAL << 10) | (1u << 5) | ecm_out_cyc);
    ecm_out_enq++;
    if (ecm_out_enq >= RING_TRBS - 1) {
        trb_write(ring, RING_TRBS - 1, dma_addr(ring), 0,
                  (TRB_LINK << 10) | (1u << 1) | ecm_out_cyc);
        ecm_out_enq = 0;
        ecm_out_cyc ^= 1;
    }
    doorbell((u32)ecm_slot, (u32)ecm_out_dci);
    u32 status = 0, ctrl = 0;
    if (!xfer_wait_trbs_ms(ecm_slot, ecm_out_dci, &trb_addr, 1,
                           &status, &ctrl, 250)) {
        ecm_n_full++;
        return 0;
    }
    ecm_last_cc = (int)((status >> 24) & 0xFF);
    if (ecm_last_cc != 1 && ecm_last_cc != 13) return 0;
    ecm_n_tx++;
    return 1;
}

int xhci_ecm_poll(u8 *out, int max)
{
    if (!ecm_ready || !out || max <= 0) return 0;
    if (ecm_rx_qh == ecm_rx_qt) (void)xhci_poll(16);
    if (ecm_rx_qh == ecm_rx_qt) return 0;
    int bi = ecm_rx_q[ecm_rx_qh];
    ecm_rx_qh = (ecm_rx_qh + 1) & 7;
    int n = ecm_rx_len[bi];
    int copy = n < max ? n : max;
    for (int i = 0; i < copy; i++)
        out[i] = *(volatile u8 *)(uptr)(ECM_RX_BUF(bi) + (u32)i);
    if (copy < n) ecm_n_drop++;
    else ecm_n_rx++;
    ecm_rx_post(bi);
    return copy;
}

int xhci_ecm_mac(int i) { return i >= 0 && i < 6 ? ecm_mac_addr[i] : 0; }
int xhci_ecm_link_up(void) { return ecm_ready; }
int xhci_ecm_ready(void) { return ecm_ready; }
int xhci_ecm_slot(void) { return ecm_slot; }
int xhci_ecm_port(void) { return ecm_port; }
int xhci_ecm_tx_count(void) { return ecm_n_tx; }
int xhci_ecm_rx_count(void) { return ecm_n_rx; }
int xhci_ecm_rx_drops(void) { return ecm_n_drop; }
int xhci_ecm_tx_full(void) { return ecm_n_full; }
int xhci_ecm_last_cc(void) { return ecm_last_cc; }
int xhci_ecm_init_stage(void) { return ecm_init_stage; }
int xhci_ecm_config_index(void) { return ecm_config_index; }
u32 xhci_ecm_parse_bits(void) { return ecm_parse_bits; }
int xhci_ecm_diag_len(void) { return ecm_diag_len; }
int xhci_ecm_diag_byte(int i)
{
    return i >= 0 && i < ecm_diag_len ? ecm_diag_config[i] : 0;
}
