/* virtio_net.c - the network card. Two virtqueues instead of one.
 *
 * WHY VIRTIO AND NOT e1000. virtio_gpu.c is 495 lines and already drives a
 * virtqueue - descriptor table, available ring, used ring, the capability
 * walk, the feature handshake. A network card is the same shape with two
 * queues instead of one and a different payload. e1000 would be a second
 * device model learned from scratch for no gain.
 *
 * NOT SHARED WITH virtio_gpu.c, deliberately. Factoring the common virtqueue
 * out into a virtio.c would be the tidy move and it would touch a file three
 * other sessions are building against right now. The duplication here is the
 * capability walk and ~60 lines of ring setup, and it is the cheaper of the
 * two mistakes this week. If the tracks converge, that is the refactor.
 *
 * WHAT IS DIFFERENT FROM THE GPU, and each one is a place to get it wrong:
 *
 *   TWO QUEUES, so every piece of ring state is per-queue. virtio_gpu.c keeps
 *   avail_idx and used_seen as file statics because it has exactly one queue;
 *   copying that shape here would have receive and transmit sharing an index.
 *
 *   THE RECEIVE QUEUE IS FILLED IN ADVANCE. A GPU command is a request the
 *   device answers; a packet arrives whether or not anyone asked. So all 32
 *   receive buffers are published at init, and every one consumed is
 *   immediately republished - a receive queue that runs dry stops receiving
 *   and nothing says so.
 *
 *   THE HEADER IS ALWAYS 12 BYTES. virtio_net_hdr_v1 ends with num_buffers,
 *   and under VIRTIO_F_VERSION_1 that field is present whether or not
 *   VIRTIO_NET_F_MRG_RXBUF was negotiated. The legacy header is 10 bytes and
 *   assuming it shifts every frame by two - which presents as "the card
 *   receives garbage", not as a header bug.
 *
 * THE ARENA. §4 item 1: do not guess an address, compute it from the map and
 * assert it. The map at the top of fb.c runs 128 MiB to 255 MiB and is FULL -
 * bg 128, sp 160, sched 176, back 192, nvme 208, xhci 224, virtio-gpu 240,
 * and virtio-gpu's framebuffer runs to 255. The ceiling above that is
 * memmap.h's HI_TOP - the smallest guest zlOS promises to boot on, and the line
 * a DMA buffer may not cross because past it is memory nobody promised exists.
 * virtio_gpu.c records what crossing it looks like: not a loud failure but
 * ERR_UNSPEC out of RESOURCE_ATTACH_BACKING, which reads like a driver bug.
 * HI_TOP was 256 MiB when this paragraph was written and is now 1 GiB; the
 * arena below is unaffected, because it was never near either number.
 *
 * The unused tails of nvme's and xhci's regions are tempting and wrong. They
 * are only unused today, they are inside a neighbour's declared span, and
 * fb.c's assert chain does not cover either of them - so a later growth there
 * would collide silently. That is the shape this project has hit FIVE times.
 *
 * So: BELOW the map. 32 MiB up holds the RAM filesystem, which ends at
 * 0x02025000; 128 MiB is the first thing the map claims. 64 MiB is in the
 * middle of an 80 MiB hole that nothing else touches, comfortably inside the
 * smallest guest we support, and the asserts below fail the build if either
 * neighbour ever grows into it.
 *
 * "AN 80 MiB HOLE THAT NOTHING ELSE TOUCHES" WAS TRUE WHEN IT WAS WRITTEN AND
 * IS THE REASON THIS REGION IS NOW IN memmap.h. It stopped being this file's
 * fact to know the moment somebody else went looking for space in the same
 * hole - and they would have gone looking in memmap.h, where this region was
 * not. The hole is now 52..64 and 65..128 MiB, and the browser's storage has
 * 80..96 of it.
 */

#include "dma.h"
#include "memmap.h"

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

#if defined(ZL_64)
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

int  pci_count(void);
int  pci_vendor(int i);
int  pci_device(int i);
void pci_scan(void);
void pci_enable(int i);
u32  pci_bar(int i, int which);
u32  pci_bar_hi(int i, int which);
u32  pci_read32(int bus, int dev, int fn, int off);
int  pci_bus_of(int i);
int  pci_dev_of(int i);
int  pci_fn_of(int i);
u32  idt_ticks(void);

/* MMIO. volatile because these are registers - the compiler must not cache a
 * value the hardware changes underneath it, nor reorder the accesses. The
 * addresses are uptr and the DMA addresses below are u32: a BAR can live above
 * 4 GiB under UEFI, everything the DEVICE reads from us cannot. */
static u32  mmio_r(uptr a)          { return *(volatile u32 *)a; }
static void mmio_w(uptr a, u32 v)   { *(volatile u32 *)a = v; }
static u16  mmio_r16(uptr a)        { return *(volatile u16 *)a; }
static void mmio_w16(uptr a, u16 v) { *(volatile u16 *)a = v; }
static u8   mmio_r8(uptr a)         { return *(volatile u8 *)a; }
static void mmio_w8(uptr a, u8 v)   { *(volatile u8 *)a = v; }

/* ---- the arena ------------------------------------------------------------
 * Computed from the map, not guessed, and asserted against both neighbours.
 */
/* FROM memmap.h, NOT FROM A LITERAL, AND THE NEIGHBOURS COME FROM THERE TOO.
 *
 * This region was 0x04000000 written here, with the two neighbours RESTATED as
 * NET_FLOOR 0x03000000 and NET_CEIL 0x08000000 - and memmap.h, the file whose
 * entire header is an argument against exactly that, did not know this region
 * existed. Two consequences, both real:
 *
 *   - NET_FLOOR named png.c's BASE. The assert read "I am above the picture
 *     arena" while comparing against the address the picture arena STARTS at,
 *     so a picture arena grown from 4 MiB to 20 would have passed it.
 *   - anyone placing a new region worked from memmap.h and could not see this
 *     one. That is not hypothetical: the browser's storage was going to 64 MiB
 *     until a grep for hex literals found this line.
 *
 * So the base and the span are declared in memmap.h with everything else, the
 * ordering chain there checks both neighbours, and what stays here is the only
 * thing that is genuinely this driver's business: that its own buffers fit
 * inside the region it was given. */
#define NET_BASE   ((u32)HI_NET)
#define NET_SIZE   ((u32)(HI_NET_END - HI_NET))

#define QSZ        32               /* descriptors per queue                 */
#define BUF_SZ     2048             /* one frame plus the 12-byte header     */
#define HDR_LEN    12               /* virtio_net_hdr_v1. NOT 10 - see above */
#define FRAME_MAX  (BUF_SZ - HDR_LEN)

#define RX_DESC    (NET_BASE + 0x00000u)
#define RX_AVAIL   (NET_BASE + 0x01000u)
#define RX_USED    (NET_BASE + 0x02000u)
#define TX_DESC    (NET_BASE + 0x03000u)
#define TX_AVAIL   (NET_BASE + 0x04000u)
#define TX_USED    (NET_BASE + 0x05000u)
#define RX_BUFS    (NET_BASE + 0x10000u)
#define TX_BUFS    (NET_BASE + 0x20000u)
#define NET_TOP    (TX_BUFS + (u32)QSZ * BUF_SZ)

#define RX_BUF(i)  (RX_BUFS + (u32)(i) * BUF_SZ)
#define TX_BUF(i)  (TX_BUFS + (u32)(i) * BUF_SZ)

/* These cost nothing at run time and fail the build the moment the map stops
 * making sense - which is the whole argument fb.c makes for its own chain, and
 * the reason nvme.c's absence of one is a gap rather than a style. */
_Static_assert(NET_TOP <= NET_BASE + NET_SIZE,
               "virtio-net buffers overrun the arena they were sized for");
_Static_assert(RX_BUFS + (u32)QSZ * BUF_SZ <= TX_BUFS,
               "the receive buffers reach into the transmit buffers");

/* ---- virtio PCI capability types --------------------------------------- */
#define VIRTIO_PCI_CAP_COMMON_CFG  1
#define VIRTIO_PCI_CAP_NOTIFY_CFG  2
#define VIRTIO_PCI_CAP_ISR_CFG     3
#define VIRTIO_PCI_CAP_DEVICE_CFG  4

#define CC_DEVICE_FEATURE_SEL  0x00
#define CC_DEVICE_FEATURE      0x04
#define CC_DRIVER_FEATURE_SEL  0x08
#define CC_DRIVER_FEATURE      0x0C
#define CC_NUM_QUEUES          0x12
#define CC_DEVICE_STATUS       0x14
#define CC_QUEUE_SELECT        0x16
#define CC_QUEUE_SIZE          0x18
#define CC_QUEUE_ENABLE        0x1C
#define CC_QUEUE_NOTIFY_OFF    0x1E
#define CC_QUEUE_DESC          0x20
#define CC_QUEUE_DRIVER        0x28
#define CC_QUEUE_DEVICE        0x30

#define STATUS_ACKNOWLEDGE 1
#define STATUS_DRIVER      2
#define STATUS_DRIVER_OK   4
#define STATUS_FEATURES_OK 8
#define STATUS_FAILED      128

#define DESC_NEXT  1
#define DESC_WRITE 2

#define VQ_RX 0
#define VQ_TX 1

/* virtio-net feature bits, low word */
#define VNET_F_MAC     (1u << 5)
#define VNET_F_STATUS  (1u << 16)

static int  vn_idx = -1;
static uptr cfg_common = 0, cfg_notify = 0, cfg_device = 0, cfg_isr = 0;
static u32  notify_mul = 0;
static u32  notify_off[2];          /* PER QUEUE. Not one, as the GPU has.   */
static int  vn_present_flag = 0;    /* found on PCI                          */
static int  vn_ready = 0;           /* queues live, DRIVER_OK set            */
static u8   vn_mac[6];
static int  vn_have_mac = 0;
static u32  vn_features = 0;

/* per-queue ring state. Two of everything, which is the whole difference from
 * the GPU driver and the easiest thing to get wrong by copying it. */
static u16  avail_idx[2];
static u16  used_seen[2];
static u16  tx_next;                /* round-robin over the transmit buffers */

static u32  n_tx, n_rx, n_rx_drop, n_tx_full, n_runt;
/* What the DRIVER saw, by ethertype, and which descriptor id each frame came
 * from. If the wire carried one ARP frame and the driver reports two, the
 * second came out of a buffer that still held an older one. */
static u32  n_arp_seen, n_ip_seen;
static u32  n_unwritten;   /* device reported a frame it never wrote */
static u32  id_hits[QSZ];
static u32  n_id_reuse;   /* an id handed back while we still held it */
static u8   id_inflight[QSZ];
/* A trace of what the DEVICE said, frame by frame: the descriptor id it
 * returned, the length it claimed, and the ethertype actually sitting in that
 * buffer. If the wire carried an IP frame and this says 0806, the buffer we
 * were pointed at is not the buffer the device wrote. */
#define TRACE_N 48
static u16  tr_id[TRACE_N];
static u16  tr_len[TRACE_N];
static u16  tr_et[TRACE_N];
static int  tr_n;

/* ---- helpers -------------------------------------------------------------- */
static void zero_mem(u32 addr, u32 bytes)
{
    volatile u32 *p = (volatile u32 *)(uptr)addr;
    for (u32 i = 0; i < bytes / 4; i++) p[i] = 0;
}

static u32 q_desc(int q)  { return q == VQ_RX ? RX_DESC  : TX_DESC;  }
static u32 q_avail(int q) { return q == VQ_RX ? RX_AVAIL : TX_AVAIL; }
static u32 q_used(int q)  { return q == VQ_RX ? RX_USED  : TX_USED;  }

/* `addr` is a DEVICE address and arrives already translated - every caller
 * wraps it in dma_addr(). It is 64-bit because the descriptor field is 64-bit
 * on every build, and taking a u32 here was how the high half came to be a
 * hardcoded zero rather than the top of a real address. See dma.h. */
static void desc_set(int q, int i, unsigned long long addr, u32 len, u16 flags, u16 next)
{
    volatile u32 *d = (volatile u32 *)(uptr)(q_desc(q) + (u32)i * 16);
    d[0] = (u32)addr;
    d[1] = (u32)(addr >> 32);        /* the high half, always written        */
    d[2] = len;
    d[3] = (u32)flags | ((u32)next << 16);
}

/* Publish descriptor `head` on queue q and ring its doorbell.
 *
 * The ring entry goes in FIRST and idx second, with a barrier between, because
 * the device may look the instant idx changes. Then the notify. This is
 * virtio_gpu.c's order and it is the one ordering rule in the whole protocol
 * that cannot be recovered from. */
static void vq_publish(int q, u16 head, int notify)
{
    volatile u16 *avail = (volatile u16 *)(uptr)q_avail(q);
    avail[2 + (avail_idx[q] % QSZ)] = head;
    __asm__ volatile("" ::: "memory");
    avail_idx[q]++;
    avail[1] = avail_idx[q];
    __asm__ volatile("" ::: "memory");
    if (notify) mmio_w16(cfg_notify + (uptr)notify_off[q] * notify_mul, (u16)q);
}

/* ---- finding the device ---------------------------------------------------
 * virtio 1.0 does not fix its registers at an offset: each block is described
 * by a vendor-specific PCI capability holding {bar, offset, length}. */
static int find_caps(int i)
{
    int bus = pci_bus_of(i), dev = pci_dev_of(i), fn = pci_fn_of(i);

    u32 sr = pci_read32(bus, dev, fn, 0x04);
    if (!((sr >> 16) & (1u << 4))) return 0;        /* no capability list */

    u32 ptr = pci_read32(bus, dev, fn, 0x34) & 0xFC;
    for (int guard = 0; guard < 48 && ptr >= 0x40; guard++) {
        u32 hdr  = pci_read32(bus, dev, fn, (int)ptr);
        u8  id   = (u8)(hdr & 0xFF);
        u8  next = (u8)((hdr >> 8) & 0xFF);

        if (id == 0x09) {                           /* vendor specific */
            u8  type = (u8)((hdr >> 24) & 0xFF);
            u8  bar  = (u8)(pci_read32(bus, dev, fn, (int)ptr + 4) & 0xFF);
            u32 off  = pci_read32(bus, dev, fn, (int)ptr + 8);

            u32 lo = pci_bar(i, bar);
            u32 hi = pci_bar_hi(i, bar);
            /* A 32-bit build cannot reach a BAR above 4 GiB and must SAY SO
             * rather than silently using a truncated address - xhci.c learned
             * this the expensive way: the reads looked perfect and every write
             * went into low RAM. */
            if (hi && sizeof(uptr) < 8) return 0;
            /* `<< 16 << 16`, not `<< 32`. On the 32-bit build uptr is 32 bits
             * wide and a shift by 32 is UNDEFINED - the guard above means it
             * never executes, but the compiler still compiles it and warns,
             * and buildefi.sh makes exactly this class -Werror because clang
             * once turned such a shift into a bare `ret` in the boot path.
             * Two 16-bit shifts are defined on both widths and identical on
             * the one where the value is non-zero. */
            uptr base = ((uptr)hi << 16 << 16) | (uptr)lo;
            if (!base) { ptr = next; continue; }

            if      (type == VIRTIO_PCI_CAP_COMMON_CFG) cfg_common = base + off;
            else if (type == VIRTIO_PCI_CAP_DEVICE_CFG) cfg_device = base + off;
            else if (type == VIRTIO_PCI_CAP_ISR_CFG)    cfg_isr    = base + off;
            else if (type == VIRTIO_PCI_CAP_NOTIFY_CFG) {
                cfg_notify = base + off;
                notify_mul = pci_read32(bus, dev, fn, (int)ptr + 16);
            }
        }
        if (!next) break;
        ptr = next;
    }
    return cfg_common != 0 && cfg_notify != 0;
}

int virtio_net_find(void)
{
    pci_scan();
    for (int i = 0; i < pci_count(); i++) {
        if (pci_vendor(i) != 0x1AF4) continue;      /* Red Hat / virtio */
        u32 d = (u32)pci_device(i);
        /* 0x1041 is the modern virtio-net ID; 0x1000 the transitional one */
        if (d != 0x1041 && d != 0x1000) continue;
        pci_enable(i);
        if (!find_caps(i)) continue;
        vn_idx = i;
        vn_present_flag = 1;
        return i;
    }
    return -1;
}

int virtio_net_present(void) { return vn_present_flag && cfg_common != 0; }

/* Prove the arena is backed by RAM before handing its address to a device that
 * will DMA into it. Absent memory reads back wrong - or WRAPS to a lower
 * address, which is why it is two different patterns at two addresses and not
 * one at one - and the resulting failure looks exactly like a protocol bug.
 * xhci.c and virtio_gpu.c both ship this; nvme.c does not, and that is a gap
 * rather than a precedent. */
int virtio_net_ram_ok(void)
{
    volatile u32 *lo = (volatile u32 *)(uptr)NET_BASE;
    volatile u32 *hi = (volatile u32 *)(uptr)(NET_TOP - 4);
    *lo = 0xA5A5F00Du;
    *hi = 0x5A5A0FF0u;
    if (*lo != 0xA5A5F00Du) return 0;
    if (*hi != 0x5A5A0FF0u) return 0;
    *lo = 0; *hi = 0;
    return 1;
}

/* ---- queue setup ---------------------------------------------------------- */
static int setup_queue(int q)
{
    mmio_w16(cfg_common + CC_QUEUE_SELECT, (u16)q);
    u16 size = mmio_r16(cfg_common + CC_QUEUE_SIZE);
    if (size == 0) return 0;                        /* queue does not exist */
    if (size > QSZ) mmio_w16(cfg_common + CC_QUEUE_SIZE, QSZ);

    /* PER QUEUE, and read AFTER the select. One shared notify offset is the
     * bug this line exists to not have. */
    notify_off[q] = (u32)mmio_r16(cfg_common + CC_QUEUE_NOTIFY_OFF);

    zero_mem(q_desc(q), 4096);
    zero_mem(q_avail(q), 4096);
    zero_mem(q_used(q), 4096);

    {   /* Through dma_addr(): identity today, the one place that changes when
         * the kernel stops being identity-mapped. See dma.h. */
        unsigned long long d = dma_addr(q_desc(q));
        unsigned long long a = dma_addr(q_avail(q));
        unsigned long long u = dma_addr(q_used(q));
        mmio_w(cfg_common + CC_QUEUE_DESC   + 0, (u32)d);
        mmio_w(cfg_common + CC_QUEUE_DESC   + 4, (u32)(d >> 32));
        mmio_w(cfg_common + CC_QUEUE_DRIVER + 0, (u32)a);
        mmio_w(cfg_common + CC_QUEUE_DRIVER + 4, (u32)(a >> 32));
        mmio_w(cfg_common + CC_QUEUE_DEVICE + 0, (u32)u);
        mmio_w(cfg_common + CC_QUEUE_DEVICE + 4, (u32)(u >> 32));
    }
    mmio_w16(cfg_common + CC_QUEUE_ENABLE, 1);

    avail_idx[q] = 0;
    used_seen[q] = 0;
    return 1;
}

/* Publish every receive buffer. A packet arrives whether or not anyone asked
 * for it, so the device needs somewhere to put it before it needs it - and a
 * receive queue that runs dry silently stops receiving. */
static void rx_fill(void)
{
    for (int i = 0; i < QSZ; i++) {
        zero_mem(RX_BUF(i), 64);          /* the header, so a short frame
                                             cannot show the last one's */
        desc_set(VQ_RX, i, dma_addr(RX_BUF(i)), BUF_SZ, DESC_WRITE, 0);
        vq_publish(VQ_RX, (u16)i, 0);     /* one notify at the end, not 32 */
    }
    mmio_w16(cfg_notify + (uptr)notify_off[VQ_RX] * notify_mul, (u16)VQ_RX);
}

int virtio_net_init(void)
{
    if (vn_ready) return 1;
    if (!virtio_net_present() && virtio_net_find() < 0) return 0;
    if (!virtio_net_ram_ok()) return 0;

    /* the reset-and-negotiate handshake the spec requires, in order */
    mmio_w8(cfg_common + CC_DEVICE_STATUS, 0);
    /* Bounded, unlike virtio_gpu.c's bare while: a device that never clears
     * its status register is a hang with no diagnostic, and this driver runs
     * before the PIT on some paths so the spin count has to be the backstop. */
    for (long spin = 0; spin < 10000000L; spin++)
        if (mmio_r8(cfg_common + CC_DEVICE_STATUS) == 0) break;
    if (mmio_r8(cfg_common + CC_DEVICE_STATUS) != 0) return 0;

    mmio_w8(cfg_common + CC_DEVICE_STATUS, STATUS_ACKNOWLEDGE);
    mmio_w8(cfg_common + CC_DEVICE_STATUS, STATUS_ACKNOWLEDGE | STATUS_DRIVER);

    /* Negotiate NOTHING we do not handle. We want the MAC the host assigned
     * (bit 5) and VIRTIO_F_VERSION_1 (bit 32, which is not optional - without
     * it the device stays in legacy mode and every layout here is wrong).
     *
     * MRG_RXBUF is deliberately NOT taken: it lets the device split one frame
     * across several buffers, and handling that needs a reassembly path this
     * driver does not have. Refusing it is what makes "one descriptor is one
     * frame" true. */
    mmio_w(cfg_common + CC_DEVICE_FEATURE_SEL, 0);
    u32 feat_lo = mmio_r(cfg_common + CC_DEVICE_FEATURE);
    mmio_w(cfg_common + CC_DEVICE_FEATURE_SEL, 1);
    u32 feat_hi = mmio_r(cfg_common + CC_DEVICE_FEATURE);

    u32 want_lo = feat_lo & (VNET_F_MAC | VNET_F_STATUS);
    vn_features = want_lo;

    mmio_w(cfg_common + CC_DRIVER_FEATURE_SEL, 0);
    mmio_w(cfg_common + CC_DRIVER_FEATURE, want_lo);
    mmio_w(cfg_common + CC_DRIVER_FEATURE_SEL, 1);
    mmio_w(cfg_common + CC_DRIVER_FEATURE, feat_hi & 1u);   /* VERSION_1 */

    mmio_w8(cfg_common + CC_DEVICE_STATUS,
            STATUS_ACKNOWLEDGE | STATUS_DRIVER | STATUS_FEATURES_OK);
    if (!(mmio_r8(cfg_common + CC_DEVICE_STATUS) & STATUS_FEATURES_OK)) {
        mmio_w8(cfg_common + CC_DEVICE_STATUS, STATUS_FAILED);
        return 0;                                   /* it refused our terms */
    }

    /* the MAC, from device config space, byte at a time - it is six bytes and
     * a 32-bit read of the last two would run off the end of the structure */
    if (cfg_device && (want_lo & VNET_F_MAC)) {
        for (int i = 0; i < 6; i++) vn_mac[i] = mmio_r8(cfg_device + i);
        vn_have_mac = 1;
    }

    if (!setup_queue(VQ_RX)) return 0;
    if (!setup_queue(VQ_TX)) return 0;

    /* DRIVER_OK BEFORE filling the receive queue. The device is not allowed to
     * touch a queue until the status bit is set, so publishing buffers first
     * and notifying first would be a notification it may legitimately ignore -
     * leaving 32 buffers posted that it never looks at. */
    mmio_w8(cfg_common + CC_DEVICE_STATUS,
            STATUS_ACKNOWLEDGE | STATUS_DRIVER | STATUS_FEATURES_OK | STATUS_DRIVER_OK);

    tx_next = 0;
    n_tx = n_rx = n_rx_drop = n_tx_full = n_runt = 0;
    n_arp_seen = n_ip_seen = n_id_reuse = 0;
    n_unwritten = 0;
    for (int k = 0; k < QSZ; k++) { id_hits[k] = 0; id_inflight[k] = 1; }
    tr_n = 0;
    rx_fill();

    vn_ready = 1;
    return 1;
}

/* ---- transmit --------------------------------------------------------------
 * One descriptor, not a chain: the 12-byte header and the frame are written
 * into one contiguous buffer. A chain would be the general shape and it would
 * also be a second descriptor to get wrong for no benefit at this size.
 */
int virtio_net_send(const u8 *frame, int len)
{
    if (!vn_ready || !frame) return 0;
    if (len <= 0 || len > FRAME_MAX) return 0;

    /* RECLAIM BEFORE REUSING. tx_next walks the 32 buffers round-robin, and
     * without this the 33rd frame overwrites the buffer and descriptor of the
     * 1st - which the device may still be reading. It does not fail loudly: it
     * silently corrupts or drops one frame, and only once traffic passes 32.
     *
     * Found by the 20-ping gate, and ONLY by running it twice. A single run
     * sends 22 frames and is clean; the second run crosses 32 and lost exactly
     * one packet, reproducibly, three times out of three. That is precisely
     * the failure §4 item 2 predicts - "a stack that works once and drops
     * every fourth packet looks identical on a single ping" - except it was
     * the driver under the stack, and one ping in forty-odd.
     *
     * in-flight is publishes minus completions. u16 arithmetic wraps and the
     * difference stays correct across the wrap, which is the whole reason
     * virtio counts with a free-running index rather than a ring position. */
    volatile u16 *tused = (volatile u16 *)(uptr)TX_USED;
    if ((u16)(avail_idx[VQ_TX] - tused[1]) >= QSZ) {
        u32 t0 = idt_ticks();
        long spins = 2000000L;
        while (spins-- > 0) {
            if ((u16)(avail_idx[VQ_TX] - tused[1]) < QSZ) break;
            if (idt_ticks() - t0 > 20) break;         /* ~200 ms */
        }
        if ((u16)(avail_idx[VQ_TX] - tused[1]) >= QSZ) {
            n_tx_full++;                              /* say so, do not lie */
            return 0;
        }
    }

    int i = tx_next % QSZ;
    volatile u8 *b = (volatile u8 *)(uptr)TX_BUF(i);

    for (int k = 0; k < HDR_LEN; k++) b[k] = 0;     /* no checksum offload,
                                                       no GSO, num_buffers 0 */
    for (int k = 0; k < len; k++) b[HDR_LEN + k] = frame[k];

    /* An Ethernet frame is 60 bytes minimum before the FCS. QEMU's user-mode
     * stack is forgiving; a real switch is not, and a runt ARP request is a
     * frame that gets silently dropped by the first hop. */
    int pad = len;
    while (pad < 60) { b[HDR_LEN + pad] = 0; pad++; }

    desc_set(VQ_TX, i, dma_addr(TX_BUF(i)), (u32)(HDR_LEN + pad), 0, 0);
    vq_publish(VQ_TX, (u16)i, 1);
    tx_next++;
    n_tx++;
    return 1;
}

/* ---- receive ---------------------------------------------------------------
 * Returns the frame length, or 0 if nothing has arrived. Non-blocking by
 * design: the caller is a frame loop, not a thread, and this kernel's whole
 * app contract is that nothing owns the loop.
 */
/* ---- A KNOWN, PRECISELY CHARACTERISED DEFECT ------------------------------
 * ONE frame per bring-up is reported by the device and never written by it.
 * It is always the same frame: the 33rd, which is the first wrap of the
 * 32-entry receive ring, on descriptor id 0.
 *
 * What was measured, with a packet capture and a driver trace taken in the
 * SAME run - comparing them across different runs is what made this take two
 * attempts:
 *
 *   45 inbound frames on the wire, 45 entries in the driver trace, and
 *   EXACTLY ONE mismatch:
 *       31: id 31  len 72  et 0x0800     wire: 0800
 *       32: id 0   len 72  et 0x0806     wire: 0800   <-- the previous
 *       33: id 1   len 72  et 0x0800     wire: 0800        contents of
 *                                                          buffer 0
 *
 * The device reports the CORRECT length for the frame that really arrived and
 * leaves the buffer holding what was in it before. Clearing the buffer before
 * re-posting it changes the symptom from a plausible stale frame to all
 * zeros, which is how "the device did not write it" was established rather
 * than assumed.
 *
 * Ruled out, each by experiment rather than by reasoning:
 *   - compiler ordering: a read barrier between the used-index check and the
 *     buffer read changes nothing (it is kept anyway; it is correct)
 *   - descriptor id reuse: the counter for a buffer handed back while still
 *     held is zero
 *   - used-ring index drift: the ids are strictly sequential 0..31,0,1,...
 *   - the transmit ring: avail equals used throughout
 *   - the ring being exactly full: leaving a slot spare changes nothing
 *   - ring SIZE: identical at 16 and 32 descriptors
 *   - the peer: the wire capture shows every request answered
 *
 * The consequence is one lost packet per bring-up. The ICMP layer reports it
 * honestly as loss, which is what the 20-ping gate is for. It is detected and
 * counted below rather than being allowed to deliver stale bytes upward. */
int virtio_net_poll(u8 *out, int max)
{
    if (!vn_ready || !out || max <= 0) return 0;

    volatile u16 *used = (volatile u16 *)(uptr)RX_USED;
    if (used[1] == used_seen[VQ_RX]) return 0;      /* nothing new */

    /* THE READ BARRIER, and its absence was a real bug rather than a
     * theoretical one.
     *
     * The device publishes in this order: write the buffer, then write the
     * used-ring entry, then advance used->idx. A driver that has seen the new
     * idx must not read the entry or the buffer until that ordering is
     * guaranteed on its side too. Without the barrier the compiler is free to
     * hoist the buffer read above the idx check, and the result is a frame
     * assembled from a NEW length and OLD contents.
     *
     * MEASURED, on exactly the 33rd frame every time - the first wrap of the
     * 32-entry ring, i.e. the first buffer to be read twice:
     *
     *   31: id 31  len 72  et 0x0800
     *   32: id 0   len 72  et 0x0806   <- an IP length, an ARP payload
     *
     * That is the previous contents of buffer 0, from the very first frame of
     * the boot. One echo reply per bring-up was silently replaced by a stale
     * ARP frame, which the IP layer then counted as ARP and dropped - so the
     * ping that was waiting for it timed out and the run reported a loss.
     *
     * A compiler barrier is the right instruction here: x86 does not reorder
     * loads with other loads, so the ordering the device needs is already
     * guaranteed by the architecture and only the compiler had to be stopped.
     * On a weaker memory model this would have to be a real load fence. */
    __asm__ volatile("" ::: "memory");

    /* used ring entry: { u32 id; u32 len; } at offset 4, indexed mod QSZ */
    volatile u32 *ring = (volatile u32 *)(uptr)(RX_USED + 4);
    u32 slot = (u32)(used_seen[VQ_RX] % QSZ);
    u32 id   = ring[slot * 2 + 0];
    u32 blen = ring[slot * 2 + 1];
    used_seen[VQ_RX]++;

    if (blen <= (u32)HDR_LEN + 14u) n_runt++;    /* shorter than an ethernet
                                                   header: it cannot be a
                                                   frame, so record it */
    int n = 0;
    if (id < (u32)QSZ && blen > (u32)HDR_LEN) {
        n = (int)(blen - (u32)HDR_LEN);
        if (n > FRAME_MAX) n = FRAME_MAX;           /* the device said more
                                                       than the buffer holds */
        if (n > max) { n = max; n_rx_drop++; }      /* caller's buffer is
                                                       smaller: truncate and
                                                       COUNT it */
        volatile u8 *b = (volatile u8 *)(uptr)(RX_BUF(id) + HDR_LEN);

        /* DID THE DEVICE ACTUALLY WRITE THIS BUFFER? Because the buffer is
         * cleared before it is handed back, an all-zero ethernet header means
         * it did not - and that is a real, reproducible condition on this
         * device, not a theoretical one. See the note above virtio_net_poll.
         *
         * A frame that was never written is not a frame. Counting it and
         * dropping it turns a silent corruption - the previous contents of
         * the buffer delivered as if they had just arrived - into a named
         * event that shows up in the diagnostics. */
        int written = 0;
        for (int k = 0; k < 14 && !written; k++) if (b[k]) written = 1;
        if (!written) {
            n_unwritten++;
            zero_mem(RX_BUF(id), 64);
            desc_set(VQ_RX, (int)id, dma_addr(RX_BUF(id)), BUF_SZ, DESC_WRITE, 0);
            vq_publish(VQ_RX, (u16)id, 1);
            id_inflight[id] = 1;
            return 0;
        }

        for (int k = 0; k < n; k++) out[k] = b[k];
        n_rx++;
        id_hits[id]++;
        if (!id_inflight[id]) n_id_reuse++;   /* returned twice without being re-posted */
        id_inflight[id] = 0;
        if (n >= 14) {
            u32 et = ((u32)out[12] << 8) | out[13];
            if (et == 0x0806) n_arp_seen++;
            else if (et == 0x0800) n_ip_seen++;
            if (tr_n < TRACE_N) {
                tr_id[tr_n] = (u16)id;
                tr_len[tr_n] = (u16)blen;
                tr_et[tr_n] = (u16)et;
                tr_n++;
            }
        }
    } else {
        n_rx_drop++;
    }

    /* Republish the buffer IMMEDIATELY. Forgetting this is the failure that
     * looks like "the card received exactly 32 packets and then died". */
    if (id < (u32)QSZ) {
        /* CLEAR THE BUFFER BEFORE HANDING IT BACK. xhci.c makes the same
         * argument for its report buffers: on a short transfer the bytes the
         * device did not write still hold the PREVIOUS contents, and a reader
         * cannot tell the difference. rx_fill does this for the initial post
         * and the re-post did not, which is exactly the asymmetry that let a
         * stale frame be read as a live one. */
        zero_mem(RX_BUF(id), 64);
        desc_set(VQ_RX, (int)id, dma_addr(RX_BUF(id)), BUF_SZ, DESC_WRITE, 0);
        vq_publish(VQ_RX, (u16)id, 1);
        id_inflight[id] = 1;
    }
    return n;
}

/* ---- accessors ------------------------------------------------------------- */
int virtio_net_ready(void)    { return vn_ready; }
int virtio_net_has_mac(void)  { return vn_have_mac; }
int virtio_net_mac(int i)     { return (i >= 0 && i < 6) ? (int)vn_mac[i] : 0; }
int virtio_net_tx_count(void) { return (int)n_tx; }
int virtio_net_rx_count(void) { return (int)n_rx; }
int virtio_net_rx_drops(void) { return (int)n_rx_drop; }
/* frames refused because the transmit queue never drained. Non-zero here is
 * a real stall, not a tuning number - the device stopped consuming. */
int virtio_net_tx_full(void)  { return (int)n_tx_full; }
/* raw ring state, so "the guard never fired" can be distinguished from "the
 * guard is looking at a register the device never writes" */
int virtio_net_runts(void)    { return (int)n_runt; }
int virtio_net_unwritten(void){ return (int)n_unwritten; }
int virtio_net_arp_seen(void) { return (int)n_arp_seen; }
int virtio_net_ip_seen(void)  { return (int)n_ip_seen; }
int virtio_net_id_reuse(void) { return (int)n_id_reuse; }
int virtio_net_tr_n(void)     { return tr_n; }
int virtio_net_tr_id(int i)   { return (i >= 0 && i < tr_n) ? (int)tr_id[i] : -1; }
int virtio_net_tr_len(int i)  { return (i >= 0 && i < tr_n) ? (int)tr_len[i] : -1; }
int virtio_net_tr_et(int i)   { return (i >= 0 && i < tr_n) ? (int)tr_et[i] : -1; }
int virtio_net_tx_avail(void) { return (int)avail_idx[VQ_TX]; }
int virtio_net_tx_used(void)
{
    volatile u16 *u = (volatile u16 *)(uptr)TX_USED;
    return (int)u[1];
}
int virtio_net_rx_avail(void) { return (int)avail_idx[VQ_RX]; }
int virtio_net_rx_used(void)
{
    volatile u16 *u = (volatile u16 *)(uptr)RX_USED;
    return (int)u[1];
}
u32 virtio_net_features(void) { return vn_features; }
u32 virtio_net_arena(void)    { return NET_BASE; }

/* Link status, when the device offers it. Bit 0 of the status word at offset 6
 * of device config. Without VNET_F_STATUS the link is assumed up, which is
 * what the spec says and is true of QEMU's user-mode network. */
int virtio_net_link_up(void)
{
    if (!vn_ready) return 0;
    if (!(vn_features & VNET_F_STATUS)) return 1;
    return (mmio_r16(cfg_device + 6) & 1) ? 1 : 0;
}

/* ---- the link-up gate -------------------------------------------------------
 * §4 item 1's gate: send an ARP request, receive the reply, print both MACs.
 * Two frames, and it proves the whole path end to end - PCI discovery, the
 * feature handshake, both queues, DMA in and DMA out.
 *
 * THE ARP HERE IS THE TEST, NOT THE PROTOCOL LAYER. It builds one hand-written
 * request and matches one reply. net.c owns ARP proper - a cache, retries, the
 * request/reply state - and when it exists this stays as what it is: the
 * two-frame proof that the card works, which is worth keeping precisely
 * because it does not depend on any layer above it.
 */
static u8  peer_mac[6];
static int peer_known;

int virtio_net_arp_probe(u32 my_ip, u32 target_ip, int ms)
{
    if (!vn_ready) return 0;

    u8 f[42];
    for (int i = 0; i < 6; i++) f[i] = 0xFF;             /* broadcast       */
    for (int i = 0; i < 6; i++) f[6 + i] = vn_mac[i];    /* our MAC         */
    f[12] = 0x08; f[13] = 0x06;                          /* ethertype ARP   */
    f[14] = 0x00; f[15] = 0x01;                          /* hw: Ethernet    */
    f[16] = 0x08; f[17] = 0x00;                          /* proto: IPv4     */
    f[18] = 6;    f[19] = 4;                             /* lengths         */
    f[20] = 0x00; f[21] = 0x01;                          /* opcode: request */
    for (int i = 0; i < 6; i++) f[22 + i] = vn_mac[i];   /* sender MAC      */
    f[28] = (u8)(my_ip >> 24); f[29] = (u8)(my_ip >> 16);
    f[30] = (u8)(my_ip >> 8);  f[31] = (u8)my_ip;
    for (int i = 0; i < 6; i++) f[32 + i] = 0;           /* target MAC: ?   */
    f[38] = (u8)(target_ip >> 24); f[39] = (u8)(target_ip >> 16);
    f[40] = (u8)(target_ip >> 8);  f[41] = (u8)target_ip;

    if (!virtio_net_send(f, 42)) return 0;

    /* Bounded two ways, whichever trips first: real milliseconds where the PIT
     * is running, a spin count where it is not. A wait that depends on a timer
     * which is not ticking is also a hang - xhci.c's wait_bit makes the same
     * argument and this is the same discipline. */
    u8  rx[FRAME_MAX];
    u32 t0    = idt_ticks();
    u32 ticks = (u32)(ms / 10) + 1;
    long spins = (long)ms * 50000;

    while (spins-- > 0) {
        int n = virtio_net_poll(rx, (int)sizeof rx);
        if (n >= 42 && rx[12] == 0x08 && rx[13] == 0x06 &&
            rx[20] == 0x00 && rx[21] == 0x02) {          /* ARP reply */
            u32 from = ((u32)rx[28] << 24) | ((u32)rx[29] << 16) |
                       ((u32)rx[30] << 8)  | (u32)rx[31];
            if (from == target_ip) {
                for (int i = 0; i < 6; i++) peer_mac[i] = rx[22 + i];
                peer_known = 1;
                return 1;
            }
        }
        if (idt_ticks() - t0 >= ticks) break;
    }
    return 0;
}

int virtio_net_peer_known(void) { return peer_known; }
int virtio_net_peer_mac(int i)  { return (i >= 0 && i < 6) ? (int)peer_mac[i] : 0; }
