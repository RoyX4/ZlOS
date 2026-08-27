/* virtio_gpu.c - a real GPU driver we can actually prove.
 *
 * WHY THIS ONE
 * ------------
 * intel.c targets the silicon in the laptop and is written from Intel's own
 * documentation, but it has never executed: QEMU has no Intel GPU, so the only
 * way to run it is to boot the machine. That makes it the least testable code
 * in the project.
 *
 * virtio-gpu is the opposite. It is a real, specified GPU interface - the one
 * SerenityOS, Redox and Linux guests all drive - and QEMU implements it fully,
 * so every line here can be exercised on every build. What it teaches is not a
 * toy: virtqueues, descriptor chains, memory barriers, DMA scatter lists,
 * fenced command submission, resource lifetimes and damage-tracked flushing
 * are exactly the shape of amdgpu and i915. The difference between this and a
 * discrete GPU driver is the command opcodes and the shader ISA, not the
 * structure.
 *
 * THE VIRTIO MODEL
 * ----------------
 * A virtio device is not a set of registers you poke. It is a set of QUEUES in
 * OUR memory that the device reads:
 *
 *     descriptor table   an array of {addr, len, flags, next} - the buffers
 *     available ring     indices WE publish: "these descriptors are ready"
 *     used ring          indices the DEVICE publishes: "these are done"
 *
 * A GPU command is a chain of two descriptors - our request, then a buffer for
 * the device's reply - published on the available ring, announced by writing
 * to a notify register, and completed on the used ring.
 *
 * Where the registers live is itself discovered rather than fixed: virtio 1.0
 * puts them behind PCI vendor-specific capabilities, each saying which BAR and
 * what offset within it. So this walks the capability list first.
 *
 * Getting a picture on screen is then five commands:
 *     GET_DISPLAY_INFO      what resolution is the display?
 *     RESOURCE_CREATE_2D    make a framebuffer object on the host
 *     RESOURCE_ATTACH_BACKING  point it at OUR pages
 *     SET_SCANOUT           bind it to the monitor
 *     TRANSFER_TO_HOST_2D + RESOURCE_FLUSH   per frame
 */

#include "memmap.h"
#include "dma.h"

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

static u32  mmio_r(uptr a)         { return *(volatile u32 *)a; }
static void mmio_w(uptr a, u32 v)  { *(volatile u32 *)a = v; }
static u16  mmio_r16(uptr a)       { return *(volatile u16 *)a; }
static void mmio_w16(uptr a, u16 v){ *(volatile u16 *)a = v; }
static u8   mmio_r8(uptr a)        { return *(volatile u8 *)a; }
static void mmio_w8(uptr a, u8 v)  { *(volatile u8 *)a = v; }

/* ---- virtio PCI capability types --------------------------------------- */
#define VIRTIO_PCI_CAP_COMMON_CFG  1
#define VIRTIO_PCI_CAP_NOTIFY_CFG  2
#define VIRTIO_PCI_CAP_ISR_CFG     3
#define VIRTIO_PCI_CAP_DEVICE_CFG  4

/* ---- common configuration structure, offsets from its base ------------- */
#define CC_DEVICE_FEATURE_SEL  0x00
#define CC_DEVICE_FEATURE      0x04
#define CC_DRIVER_FEATURE_SEL  0x08
#define CC_DRIVER_FEATURE      0x0C
#define CC_MSIX_CONFIG         0x10
#define CC_NUM_QUEUES          0x12
#define CC_DEVICE_STATUS       0x14
#define CC_CONFIG_GEN          0x15
#define CC_QUEUE_SELECT        0x16
#define CC_QUEUE_SIZE          0x18
#define CC_QUEUE_MSIX_VECTOR   0x1A
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

/* ---- virtio-gpu commands ------------------------------------------------ */
#define CMD_GET_DISPLAY_INFO        0x0100
#define CMD_RESOURCE_CREATE_2D      0x0101
#define CMD_RESOURCE_UNREF          0x0102
#define CMD_SET_SCANOUT             0x0103
#define CMD_RESOURCE_FLUSH          0x0104
#define CMD_TRANSFER_TO_HOST_2D     0x0105
#define CMD_RESOURCE_ATTACH_BACKING 0x0106
#define RESP_OK_NODATA              0x1100
#define RESP_OK_DISPLAY_INFO        0x1101
#define FORMAT_B8G8R8X8             2

/* ---- our memory. Same discipline as the USB driver: fixed physical
 * addresses, identity mapped on every boot path, comfortably clear of the
 * framebuffer back buffer and the xHCI arena. -------------------------- */
#define VMEM_DESC     ((unsigned int)HI_VGPU)   /* descriptor table          */
#define VMEM_AVAIL    (VMEM_DESC + 0x1000u)   /* available ring              */
#define VMEM_USED     (VMEM_DESC + 0x2000u)   /* used ring                   */
#define VMEM_CMD      (VMEM_DESC + 0x3000u)   /* command buffers we send     */
#define VMEM_RESP     (VMEM_DESC + 0x4000u)   /* replies the device writes   */
#define VMEM_SGLIST   (VMEM_DESC + 0x5000u)   /* scatter list for the fb     */
/* 241 MiB, and it stays there. It was placed here when HI_TOP was 256 MiB and
 * this was the last clear region below that line; HI_TOP is now 1 GiB, so the
 * address is no longer FORCED - but nothing above it is claimed either, and
 * moving a live DMA region buys nothing. Everything the GPU DMAs still has to
 * be inside memory we promised exists, which is what HI_TOP means: cross it and
 * RESOURCE_ATTACH_BACKING fails with ERR_UNSPEC, which reads like a driver bug
 * and is not one.
 *
 * VMEM_FB_MAX is 14 MiB and is NOT sized by the ceiling - it is sized by
 * 1920x1200x4 = 9.2 MiB, with room. The ThinkPad's 2560x1440x4 is 14.06 MiB and
 * does NOT fit; virtio_gpu_setup() refuses it out loud rather than truncating
 * (see the `bytes > VMEM_FB_MAX` return below). That refusal is unchanged by
 * this commit and is a real limit, not an artefact of the old ceiling. */
#define VMEM_FB       (VMEM_DESC + 0x100000u)
#define VMEM_FB_MAX   0x00E00000u   /* 14 MiB - enough for 1920x1200x4 */

/* HI_TOP, enforced rather than described. The paragraph above is the whole
 * reason this assert exists: crossing it does not fail loudly, it fails as
 * ERR_UNSPEC from RESOURCE_ATTACH_BACKING and reads like a driver bug.
 * virtio_gpu.c was in no assertion before - the top half of the map was held up
 * by prose only.
 *
 * This assert has MORE SLACK than it used to (0x0FF00000 against 1 GiB instead
 * of against 256 MiB) and that is worth saying plainly rather than leaving for
 * someone to notice: it is no longer the tight constraint on this region. The
 * tight one is VMEM_FB_MAX above, and it is checked at run time. */
_Static_assert((unsigned long)VMEM_FB + VMEM_FB_MAX <= HI_TOP,
               "virtio-gpu: the framebuffer crosses the guest RAM ceiling");
_Static_assert((unsigned long)VMEM_SGLIST < VMEM_FB,
               "virtio-gpu: the rings have grown into the framebuffer");
#define QSZ           64            /* descriptors per queue                */

static int  vg_idx = -1;
static uptr cfg_common = 0, cfg_notify = 0, cfg_device = 0, cfg_isr = 0;
static u32  notify_off_mul = 0;
static u32  vg_notify_off  = 0;
static int  vg_ready = 0;
static int  vg_w = 0, vg_h = 0;
static u16  avail_idx = 0;
static u16  used_seen = 0;
static u32  vg_scanouts = 0;

/* ---- finding the register blocks ---------------------------------------
 * virtio 1.0 does not fix its registers at an offset. Each block is described
 * by a vendor-specific PCI capability holding {bar, offset, length}, so the
 * capability list has to be walked before anything else can happen. */
static int find_caps(int i)
{
    int bus = pci_bus_of(i), dev = pci_dev_of(i), fn = pci_fn_of(i);

    /* status register bit 4 says a capability list exists */
    u32 sr = pci_read32(bus, dev, fn, 0x04);
    if (!((sr >> 16) & (1u << 4))) return 0;

    u32 ptr = pci_read32(bus, dev, fn, 0x34) & 0xFC;
    for (int guard = 0; guard < 48 && ptr >= 0x40; guard++) {
        u32 hdr  = pci_read32(bus, dev, fn, (int)ptr);
        u8  id   = (u8)(hdr & 0xFF);
        u8  next = (u8)((hdr >> 8) & 0xFF);

        if (id == 0x09) {                   /* vendor specific = virtio */
            u8  type = (u8)((hdr >> 24) & 0xFF);
            u8  bar  = (u8)(pci_read32(bus, dev, fn, (int)ptr + 4) & 0xFF);
            u32 off  = pci_read32(bus, dev, fn, (int)ptr + 8);

            u32 lo = pci_bar(i, bar);
            u32 hi = pci_bar_hi(i, bar);
            if (hi && sizeof(uptr) < 8) return 0;   /* unreachable from 32-bit */
            /* `<< 16 << 16`, not `<< 32` - see virtio_net.c's note. UB on the
             * 32-bit build, where uptr is 32 bits, guard above notwithstanding. */
            uptr base = ((uptr)hi << 16 << 16) | (uptr)lo;
            if (!base) { ptr = next; continue; }

            if      (type == VIRTIO_PCI_CAP_COMMON_CFG) cfg_common = base + off;
            else if (type == VIRTIO_PCI_CAP_DEVICE_CFG) cfg_device = base + off;
            else if (type == VIRTIO_PCI_CAP_ISR_CFG)    cfg_isr    = base + off;
            else if (type == VIRTIO_PCI_CAP_NOTIFY_CFG) {
                cfg_notify     = base + off;
                notify_off_mul = pci_read32(bus, dev, fn, (int)ptr + 16);
            }
        }
        if (!next) break;
        ptr = next;
    }
    return cfg_common != 0 && cfg_notify != 0;
}

int virtio_gpu_find(void)
{
    pci_scan();
    for (int i = 0; i < pci_count(); i++) {
        if (pci_vendor(i) != 0x1AF4) continue;          /* Red Hat / virtio */
        u32 d = (u32)pci_device(i);
        /* 0x1050 is the modern virtio-gpu ID; 0x1010 the transitional one */
        if (d != 0x1050 && d != 0x1010) continue;
        pci_enable(i);
        if (!find_caps(i)) continue;
        vg_idx = i;
        return i;
    }
    return -1;
}

int virtio_gpu_present(void) { return vg_idx >= 0 && cfg_common != 0; }

static void zero_mem(u32 addr, u32 bytes)
{
    volatile u32 *p = (volatile u32 *)(uptr)addr;
    for (u32 i = 0; i < bytes / 4; i++) p[i] = 0;
}

/* ---- the split virtqueue ------------------------------------------------
 * Three arrays the device reads directly. The layout is fixed by the spec:
 *   desc[i]   = { u64 addr; u32 len; u16 flags; u16 next; }   16 bytes
 *   avail     = { u16 flags; u16 idx; u16 ring[QSZ]; }
 *   used      = { u16 flags; u16 idx; struct { u32 id; u32 len; } ring[QSZ]; }
 */
#define DESC_NEXT  1
#define DESC_WRITE 2

static void desc_set(int i, u64 addr, u32 len, u16 flags, u16 next)
{
    volatile u32 *d = (volatile u32 *)(uptr)(VMEM_DESC + (u32)i * 16);
    d[0] = (u32)(addr & 0xFFFFFFFFu);
    d[1] = (u32)(addr >> 32);
    d[2] = len;
    d[3] = (u32)flags | ((u32)next << 16);
}

static int setup_queue(int q)
{
    mmio_w16(cfg_common + CC_QUEUE_SELECT, (u16)q);
    u16 size = mmio_r16(cfg_common + CC_QUEUE_SIZE);
    if (size == 0) return 0;                 /* queue does not exist */
    if (size > QSZ) mmio_w16(cfg_common + CC_QUEUE_SIZE, QSZ);

    vg_notify_off = (u32)mmio_r16(cfg_common + CC_QUEUE_NOTIFY_OFF);

    zero_mem(VMEM_DESC, 4096);
    zero_mem(VMEM_AVAIL, 4096);
    zero_mem(VMEM_USED, 4096);

    /* 64-bit physical addresses, low half first. Through dma_addr(), which is
     * the identity function today and is the ONE place that changes when the
     * kernel stops being identity-mapped - see dma.h. The high halves are no
     * longer a hardcoded 0: they are the top 32 bits of the translated address,
     * which is the same 0 today and is not guaranteed to stay 0. */
    {
        unsigned long long d = dma_addr(VMEM_DESC);
        unsigned long long a = dma_addr(VMEM_AVAIL);
        unsigned long long u = dma_addr(VMEM_USED);
        mmio_w(cfg_common + CC_QUEUE_DESC   + 0, (u32)d);
        mmio_w(cfg_common + CC_QUEUE_DESC   + 4, (u32)(d >> 32));
        mmio_w(cfg_common + CC_QUEUE_DRIVER + 0, (u32)a);
        mmio_w(cfg_common + CC_QUEUE_DRIVER + 4, (u32)(a >> 32));
        mmio_w(cfg_common + CC_QUEUE_DEVICE + 0, (u32)u);
        mmio_w(cfg_common + CC_QUEUE_DEVICE + 4, (u32)(u >> 32));
    }
    mmio_w16(cfg_common + CC_QUEUE_ENABLE, 1);

    avail_idx = 0;
    used_seen = 0;
    return 1;
}

/* Send one command and wait for the reply.
 *
 * Two descriptors chained: ours (device-readable) then the response buffer
 * (device-writable, hence DESC_WRITE). Publishing means writing the head index
 * into the available ring and then bumping its idx - in that order, because
 * the device may look the instant idx changes. */
static int vq_send(u32 cmd_len, u32 resp_len)
{
    int head = 0;
    desc_set(0, dma_addr(VMEM_CMD),  cmd_len,  DESC_NEXT, 1);
    desc_set(1, dma_addr(VMEM_RESP), resp_len, DESC_WRITE, 0);

    volatile u16 *avail = (volatile u16 *)(uptr)VMEM_AVAIL;
    avail[2 + (avail_idx % QSZ)] = (u16)head;
    __asm__ volatile("" ::: "memory");       /* publish the ring entry first */
    avail_idx++;
    avail[1] = avail_idx;
    __asm__ volatile("" ::: "memory");

    /* the notify register's address is computed, not fixed */
    mmio_w16(cfg_notify + (uptr)vg_notify_off * notify_off_mul, 0);

    /* wait for the used ring to advance. Bounded in real time where the PIT
     * is running, and by a spin count where it is not. */
    volatile u16 *used = (volatile u16 *)(uptr)VMEM_USED;
    u32 t0 = idt_ticks();
    for (long spin = 0; spin < 200000000L; spin++) {
        if (used[1] != used_seen) { used_seen = used[1]; return 1; }
        if (idt_ticks() - t0 > 200) break;   /* ~2 seconds */
    }
    return 0;
}

/* every request begins with this 24-byte header */
static void hdr(u32 type)
{
    volatile u32 *c = (volatile u32 *)(uptr)VMEM_CMD;
    for (int i = 0; i < 16; i++) c[i] = 0;
    c[0] = type;        /* type          */
    c[1] = 0;           /* flags         */
    c[2] = 0; c[3] = 0; /* fence_id      */
    c[4] = 0;           /* ctx_id        */
    c[5] = 0;           /* ring_idx+pad  */
}

static u32 resp_type(void) { return *(volatile u32 *)(uptr)VMEM_RESP; }

int virtio_gpu_init(void)
{
    if (vg_ready) return 1;
    if (!virtio_gpu_present() && virtio_gpu_find() < 0) return 0;

    /* the reset-and-negotiate handshake the spec requires, in order */
    mmio_w8(cfg_common + CC_DEVICE_STATUS, 0);          /* reset */
    while (mmio_r8(cfg_common + CC_DEVICE_STATUS) != 0) { }
    mmio_w8(cfg_common + CC_DEVICE_STATUS, STATUS_ACKNOWLEDGE);
    mmio_w8(cfg_common + CC_DEVICE_STATUS, STATUS_ACKNOWLEDGE | STATUS_DRIVER);

    /* We want none of the optional features - no virgl, no blob resources -
     * but VIRTIO_F_VERSION_1 (bit 32) is not optional: without acknowledging
     * it the device stays in legacy mode and the layout above is wrong. */
    mmio_w(cfg_common + CC_DEVICE_FEATURE_SEL, 1);
    u32 feat_hi = mmio_r(cfg_common + CC_DEVICE_FEATURE);
    mmio_w(cfg_common + CC_DRIVER_FEATURE_SEL, 0);
    mmio_w(cfg_common + CC_DRIVER_FEATURE, 0);
    mmio_w(cfg_common + CC_DRIVER_FEATURE_SEL, 1);
    mmio_w(cfg_common + CC_DRIVER_FEATURE, feat_hi & 1u);   /* VERSION_1 only */

    mmio_w8(cfg_common + CC_DEVICE_STATUS,
            STATUS_ACKNOWLEDGE | STATUS_DRIVER | STATUS_FEATURES_OK);
    if (!(mmio_r8(cfg_common + CC_DEVICE_STATUS) & STATUS_FEATURES_OK)) {
        mmio_w8(cfg_common + CC_DEVICE_STATUS, STATUS_FAILED);
        return 0;                                   /* it refused our terms */
    }

    if (cfg_device) vg_scanouts = mmio_r(cfg_device + 8);   /* num_scanouts */

    if (!setup_queue(0)) return 0;                  /* queue 0 = controlq */

    mmio_w8(cfg_common + CC_DEVICE_STATUS,
            STATUS_ACKNOWLEDGE | STATUS_DRIVER | STATUS_FEATURES_OK | STATUS_DRIVER_OK);

    vg_ready = 1;
    return 1;
}

/* Ask the host what the display looks like. The reply is an array of 16
 * scanouts, each { rect{x,y,w,h}, enabled, flags }; we take the first enabled
 * one, which is what a single-monitor guest wants. */
int virtio_gpu_display_info(void)
{
    if (!vg_ready) return 0;
    hdr(CMD_GET_DISPLAY_INFO);
    if (!vq_send(24, 24 + 16 * 24)) return 0;
    if (resp_type() != RESP_OK_DISPLAY_INFO) return 0;

    volatile u32 *r = (volatile u32 *)(uptr)(VMEM_RESP + 24);
    for (int i = 0; i < 16; i++) {
        u32 w  = r[i * 6 + 2];
        u32 h  = r[i * 6 + 3];
        u32 en = r[i * 6 + 4];
        if (en && w && h && w <= 0x7fffffffu && h <= 0x7fffffffu) {
            u64 bytes = (u64)w * (u64)h * 4u;
            if (bytes > VMEM_FB_MAX) continue;
            vg_w = (int)w; vg_h = (int)h; return 1;
        }
    }
    return 0;
}

int virtio_gpu_width(void)    { return vg_w; }
int virtio_gpu_height(void)   { return vg_h; }
int virtio_gpu_scanouts(void) { return (int)vg_scanouts; }
u32 virtio_gpu_fb(void)       { return VMEM_FB; }

/* ---- putting a framebuffer on the screen ------------------------------- */
int virtio_gpu_create_2d(u32 id, u32 w, u32 h)
{
    hdr(CMD_RESOURCE_CREATE_2D);
    volatile u32 *c = (volatile u32 *)(uptr)VMEM_CMD;
    c[6] = id;
    c[7] = FORMAT_B8G8R8X8;
    c[8] = w;
    c[9] = h;
    if (!vq_send(24 + 16, 24)) return 0;
    return resp_type() == RESP_OK_NODATA;
}

/* Hand the host the guest-physical pages behind the resource. One entry is
 * enough here because our framebuffer is one physically contiguous block -
 * a real allocator would produce a scatter list of many. */
int virtio_gpu_attach_backing(u32 id, u32 bytes)
{
    hdr(CMD_RESOURCE_ATTACH_BACKING);
    volatile u32 *c = (volatile u32 *)(uptr)VMEM_CMD;
    c[6] = id;
    c[7] = 1;                       /* nr_entries */
    {   /* THE framebuffer address the GPU DMAs from - the single most
         * important address in this file to get right, and the one whose
         * failure mode (ERR_UNSPEC) reads like a driver bug. */
        unsigned long long fb = dma_addr(VMEM_FB);
        c[8] = (u32)fb;             /* addr low   */
        c[9] = (u32)(fb >> 32);     /* addr high  */
    }
    c[10] = bytes;
    c[11] = 0;                      /* padding    */
    if (!vq_send(24 + 8 + 16, 24)) return 0;
    return resp_type() == RESP_OK_NODATA;
}

int virtio_gpu_set_scanout(u32 id, u32 w, u32 h)
{
    hdr(CMD_SET_SCANOUT);
    volatile u32 *c = (volatile u32 *)(uptr)VMEM_CMD;
    c[6] = 0; c[7] = 0; c[8] = w; c[9] = h;   /* rect */
    c[10] = 0;                                 /* scanout_id */
    c[11] = id;                                /* resource_id */
    if (!vq_send(24 + 24, 24)) return 0;
    return resp_type() == RESP_OK_NODATA;
}

/* The host does not see our writes automatically - the resource lives on its
 * side. TRANSFER copies our pixels into it, FLUSH tells it to put them on the
 * screen. Both take a rectangle, which is how damage tracking works: send the
 * box that changed, not the whole screen. */
int virtio_gpu_flush(int x, int y, int w, int h)
{
    if (!vg_ready || !vg_w) return 0;

    hdr(CMD_TRANSFER_TO_HOST_2D);
    volatile u32 *c = (volatile u32 *)(uptr)VMEM_CMD;
    c[6] = (u32)x; c[7] = (u32)y; c[8] = (u32)w; c[9] = (u32)h;
    c[10] = (u32)(y * vg_w + x) * 4;    /* offset into the resource, low  */
    c[11] = 0;                          /*                          high  */
    c[12] = 1;                          /* resource_id */
    c[13] = 0;
    if (!vq_send(24 + 32, 24)) return 0;
    if (resp_type() != RESP_OK_NODATA) return 0;

    hdr(CMD_RESOURCE_FLUSH);
    c[6] = (u32)x; c[7] = (u32)y; c[8] = (u32)w; c[9] = (u32)h;
    c[10] = 1;                          /* resource_id */
    c[11] = 0;
    if (!vq_send(24 + 24, 24)) return 0;
    return resp_type() == RESP_OK_NODATA;
}

/* Bring the display up end to end and return the framebuffer address the rest
 * of zlOS should draw into. Zero means it did not work. */
u32 virtio_gpu_setup(void)
{
    if (!virtio_gpu_init())        return 0;
    if (!virtio_gpu_display_info()) return 0;
    if (vg_w <= 0 || vg_h <= 0)    return 0;

    u64 bytes64 = (u64)(u32)vg_w * (u64)(u32)vg_h * 4u;
    if (bytes64 > VMEM_FB_MAX) return 0;    /* wider than our arena - say so */
    u32 bytes = (u32)bytes64;
    if (!virtio_gpu_create_2d(1, (u32)vg_w, (u32)vg_h))     return 0;
    if (!virtio_gpu_attach_backing(1, bytes))               return 0;
    if (!virtio_gpu_set_scanout(1, (u32)vg_w, (u32)vg_h))   return 0;

    zero_mem(VMEM_FB, bytes);
    virtio_gpu_flush(0, 0, vg_w, vg_h);
    return VMEM_FB;
}

/* A full-resolution test pattern, written straight into the pages the GPU
 * scans out of. Done in C rather than zl because a million pixels through an
 * interpreted loop is slow enough to look broken - and the point here is the
 * GPU path, not the language. */
int virtio_gpu_testpattern(void)
{
    if (!vg_ready || vg_w <= 0 || vg_h <= 0) return 0;
    volatile u32 *fb = (volatile u32 *)(uptr)VMEM_FB;

    for (int y = 0; y < vg_h; y++) {
        for (int x = 0; x < vg_w; x++) {
            u32 r = (u32)(x * 255 / vg_w);
            u32 g = (u32)(y * 255 / vg_h);
            u32 b = (u32)(255 - (r + g) / 2);
            /* a grid on top, so scanline stride errors are obvious rather than
             * subtle - a wrong pitch shears these lines visibly */
            if ((x % 64) == 0 || (y % 64) == 0) { r = 255; g = 255; b = 255; }
            fb[y * vg_w + x] = (r << 16) | (g << 8) | b;
        }
    }
    return virtio_gpu_flush(0, 0, vg_w, vg_h);
}

int virtio_gpu_ready(void) { return vg_ready; }
u32 virtio_gpu_fb_max(void) { return VMEM_FB_MAX; }

/* Prove the framebuffer arena is backed by RAM before handing its address to a
 * device that will DMA into it. Absent memory reads back wrong, and the
 * resulting failure looks exactly like a protocol bug. */
int virtio_gpu_ram_ok(void)
{
    volatile u32 *lo = (volatile u32 *)(uptr)VMEM_FB;
    volatile u32 *hi = (volatile u32 *)(uptr)(VMEM_FB + VMEM_FB_MAX - 4);
    *lo = 0xC0FFEE01u; *hi = 0x1EEFF0C0u & 0xFFFFFFFFu;
    if (*lo != 0xC0FFEE01u) return 0;
    if (*hi != (0x1EEFF0C0u & 0xFFFFFFFFu)) return 0;
    *lo = 0; *hi = 0;
    return 1;
}

/* what the device said last - 0x1100 is success, 0x1200..0x1205 are the
 * error codes, and 0 means the queue never completed at all */
u32 virtio_gpu_last_resp(void) { return resp_type(); }
