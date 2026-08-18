/* nvme.c - a storage driver, so zlOS can keep something.
 *
 * Every filesystem in zlOS so far has lived in RAM: the editor writes files,
 * and they evaporate at power off. That is the last thing separating this from
 * an operating system you could actually use, and the missing piece is a block
 * device driver.
 *
 * NVMe is the right one to write. It is the interface the laptop's own disk
 * speaks (a Toshiba KXG6AZNV512G on the PCIe bus), QEMU emulates it faithfully,
 * and unlike AHCI it was designed this decade - the spec is clean, the queues
 * are the same shape as everything else modern, and there is no thirty years of
 * IDE compatibility crust to emulate.
 *
 * THE SHAPE OF NVMe
 * -----------------
 * If xhci.c and virtio_gpu.c felt similar, this will feel familiar too, and
 * that is the real lesson: every fast modern device is a queue in host memory
 * that the device DMAs from.
 *
 *     submission queue   64-byte commands WE write
 *     completion queue   16-byte results the DEVICE writes
 *     doorbell           a register per queue: "I have added entries"
 *
 * There are two pairs. The ADMIN queues exist from reset and are used to ask
 * the controller what it is and to create the others. The I/O queues are
 * created with admin commands and carry the actual reads and writes.
 *
 * How the device knows which completions are new is a PHASE TAG rather than
 * xHCI's cycle bit - the same trick under a different name: one bit in each
 * entry that flips every time the ring wraps.
 *
 * Data buffers are described by PRPs - Physical Region Pages. PRP1 is a
 * physical address; if the transfer crosses into a second page, PRP2 is
 * either the next page or a pointer to a list of them. Keeping every transfer
 * inside one 4 KiB page, as this driver does for now, means PRP2 is unused and
 * the whole complication disappears.
 */

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
int  pci_class(int i);
int  pci_subclass(int i);
int  pci_prog_if(int i);
int  pci_vendor(int i);
int  pci_device(int i);
void pci_scan(void);
void pci_enable(int i);
u32  pci_bar(int i, int which);
u32  pci_bar_hi(int i, int which);
u32  idt_ticks(void);

static u32  rd32(uptr a)          { return *(volatile u32 *)a; }
static void wr32(uptr a, u32 v)   { *(volatile u32 *)a = v; }
static u64  rd64(uptr a)
{
    /* two aligned 32-bit reads: a 64-bit MMIO read is not guaranteed atomic
     * on a 32-bit bus and the controller does not require it */
    return ((u64)rd32(a + 4) << 32) | (u64)rd32(a);
}
static void wr64(uptr a, u64 v)
{
    wr32(a, (u32)(v & 0xFFFFFFFFu));
    wr32(a + 4, (u32)(v >> 32));
}

/* ---- controller registers ---------------------------------------------- */
#define NVME_CAP    0x00      /* capabilities, 64-bit                       */
#define NVME_VS     0x08      /* version                                     */
#define NVME_INTMS  0x0C
#define NVME_INTMC  0x10
#define NVME_CC     0x14      /* controller configuration                    */
#define NVME_CSTS   0x1C      /* controller status                           */
#define NVME_AQA    0x24      /* admin queue attributes                      */
#define NVME_ASQ    0x28      /* admin submission queue base, 64-bit         */
#define NVME_ACQ    0x30      /* admin completion queue base, 64-bit         */

#define CC_EN       (1u << 0)
#define CSTS_RDY    (1u << 0)
#define CSTS_CFS    (1u << 1)   /* controller fatal status */

/* ---- admin and I/O opcodes --------------------------------------------- */
#define ADMIN_CREATE_SQ  0x01
#define ADMIN_CREATE_CQ  0x05
#define ADMIN_IDENTIFY   0x06
#define IO_WRITE         0x01
#define IO_READ          0x02
#define IO_FLUSH         0x00

/* ---- our memory. 208 MiB: the gap between the framebuffer back buffer
 * (which ends around 201 MiB) and the xHCI arena at 224 MiB. ------------- */
#define NMEM_ASQ     0x0D000000u   /* admin submission queue                */
#define NMEM_ACQ     0x0D001000u   /* admin completion queue                */
#define NMEM_IOSQ    0x0D002000u   /* I/O submission queue                  */
#define NMEM_IOCQ    0x0D003000u   /* I/O completion queue                  */
#define NMEM_IDENT   0x0D004000u   /* identify data lands here              */
#define NMEM_DATA    0x0D010000u   /* one page of block data                */
#define QDEPTH       64

/* Why the last bring-up refused. `nvme_setup()` returning 0 used to be the
 * whole story, and the shell reported every one of these as "the controller
 * did not come ready" - which is true of exactly one of them. */
#define NVF_NONE       0
#define NVF_NO_DEV     1
#define NVF_NOT_READY  2
#define NVF_IDENT      3
#define NVF_BLOCKSIZE  4
#define NVF_QUEUES     5
#define NVF_RAM        6
static int nv_fault = NVF_NONE;

static int  nv_idx   = -1;
static uptr nv_base  = 0;
static u32  nv_dstrd = 0;         /* doorbell stride, from CAP              */
static int  nv_ready = 0;
static u16  asq_tail = 0, acq_head = 0, acq_phase = 1;
static u16  iosq_tail = 0, iocq_head = 0, iocq_phase = 1;
static u16  next_cid = 1;
static u32  nv_nsid  = 1;
static u64  nv_blocks = 0;
static u32  nv_blocksize = 512;
static u32  nv_maxlba_lo = 0;

/* A doorbell lives at 0x1000 + (2*qid + is_completion) * (4 << DSTRD).
 * DSTRD exists so a controller can spread doorbells across cache lines; on
 * everything common it is zero and the stride is four bytes. */
static uptr doorbell(int qid, int is_cq)
{
    return nv_base + 0x1000 + (uptr)(2 * qid + is_cq) * (uptr)(4u << nv_dstrd);
}

static void zero_mem(u32 addr, u32 bytes)
{
    volatile u32 *p = (volatile u32 *)(uptr)addr;
    for (u32 i = 0; i < bytes / 4; i++) p[i] = 0;
}

/* ---- finding it -------------------------------------------------------- */
int nvme_find(void)
{
    pci_scan();
    for (int i = 0; i < pci_count(); i++) {
        if (pci_class(i)    != 0x01) continue;   /* mass storage        */
        if (pci_subclass(i) != 0x08) continue;   /* non-volatile memory */
        if (pci_prog_if(i)  != 0x02) continue;   /* NVMe, not NVMHCI    */

        pci_enable(i);
        u32 lo = pci_bar(i, 0);
        u32 hi = pci_bar_hi(i, 0);
        if (hi && sizeof(uptr) < 8) continue;    /* above 4 GiB, unreachable */
        /* Split by #if, not by sizeof: on the 32-bit target `hi << 32` is a
         * shift wider than the type - undefined behaviour, and gcc warns. The
         * guard above has already established hi == 0 on that target, so the
         * shift was never NEEDED there, only compiled there. This is the same
         * truncated-address bug class that has cost this project five
         * debugging sessions; it may as well not be UB while we are here. */
#if defined(ZL_64)
        uptr b = ((uptr)hi << 32) | (uptr)lo;
#else
        uptr b = (uptr)lo;
#endif
        if (!b) continue;

        nv_idx  = i;
        nv_base = b;
        u64 cap = rd64(nv_base + NVME_CAP);
        nv_dstrd = (u32)((cap >> 32) & 0xF);
        return i;
    }
    return -1;
}

int nvme_present(void) { return nv_idx >= 0 && nv_base != 0; }
u32 nvme_version(void) { return nvme_present() ? rd32(nv_base + NVME_VS) : 0; }
u32 nvme_cap_lo(void)  { return nvme_present() ? rd32(nv_base + NVME_CAP) : 0; }
u32 nvme_mmio(void)    { return (u32)nv_base; }

/* wait for a status bit, bounded in real time and by a spin count */
static int wait_csts(u32 mask, int want_set, int ms)
{
    u32  t0 = idt_ticks();
    u32  ticks = (u32)(ms / 10) + 1;
    long spins = (long)ms * 50000;
    while (spins-- > 0) {
        u32 v = rd32(nv_base + NVME_CSTS);
        if (v & CSTS_CFS) return 0;                /* controller gave up */
        if (want_set ? (v & mask) : !(v & mask)) return 1;
        if (idt_ticks() - t0 >= ticks) return 0;
    }
    return 0;
}

/* ---- commands ----------------------------------------------------------
 * A submission entry is sixteen dwords. Only a few matter for what we do:
 *   dw0  opcode in [7:0], command id in [31:16]
 *   dw1  namespace id
 *   dw6/7  PRP1 - the physical address of the data buffer
 *   dw10..15  command specific
 */
static void sqe(u32 base, u16 slot, u8 opcode, u16 cid, u32 nsid, u64 prp1,
                u32 cdw10, u32 cdw11, u32 cdw12)
{
    volatile u32 *e = (volatile u32 *)(uptr)(base + (u32)slot * 64);
    for (int i = 0; i < 16; i++) e[i] = 0;
    e[0]  = (u32)opcode | ((u32)cid << 16);
    e[1]  = nsid;
    e[6]  = (u32)(prp1 & 0xFFFFFFFFu);
    e[7]  = (u32)(prp1 >> 32);
    e[10] = cdw10;
    e[11] = cdw11;
    e[12] = cdw12;
}

/* Wait for a completion whose phase tag matches ours, and check it is the
 * command we sent. Returns the status field, or -1 on timeout. */
static int cq_wait(u32 cq_base, u16 *head, u16 *phase, int qid, u16 want_cid)
{
    u32 t0 = idt_ticks();
    for (long spin = 0; spin < 500000000L; spin++) {
        volatile u32 *e = (volatile u32 *)(uptr)(cq_base + (u32)(*head) * 16);
        u32 dw3 = e[3];
        if (((dw3 >> 16) & 1) == (u32)(*phase)) {
            u16 cid    = (u16)(dw3 & 0xFFFF);
            int status = (int)((dw3 >> 17) & 0x7FF);

            *head = (u16)((*head + 1) % QDEPTH);
            if (*head == 0) *phase ^= 1;
            wr32(doorbell(qid, 1), (u32)(*head));

            if (cid != want_cid) continue;      /* someone else's - keep going */
            return status;
        }
        if (idt_ticks() - t0 > 300) break;      /* three seconds */
    }
    return -1;
}

static int admin_cmd(u8 opcode, u32 nsid, u64 prp1, u32 cdw10, u32 cdw11)
{
    u16 cid = next_cid++;
    sqe(NMEM_ASQ, asq_tail, opcode, cid, nsid, prp1, cdw10, cdw11, 0);
    asq_tail = (u16)((asq_tail + 1) % QDEPTH);
    wr32(doorbell(0, 0), (u32)asq_tail);
    return cq_wait(NMEM_ACQ, &acq_head, &acq_phase, 0, cid);
}

/* ---- bring-up ---------------------------------------------------------- */
int nvme_init(void)
{
    if (nv_ready) return 1;
    if (!nvme_present() && nvme_find() < 0) return 0;

    /* The controller may be running - the firmware booted from it. Take it
     * down before touching the queue registers; they are read-only while
     * enabled. */
    u32 cc = rd32(nv_base + NVME_CC);
    if (cc & CC_EN) {
        wr32(nv_base + NVME_CC, cc & ~CC_EN);
        if (!wait_csts(CSTS_RDY, 0, 2000)) return 0;
    }

    zero_mem(NMEM_ASQ, 4096);
    zero_mem(NMEM_ACQ, 4096);
    asq_tail = 0; acq_head = 0; acq_phase = 1;

    /* queue sizes are zero-based in AQA */
    wr32(nv_base + NVME_AQA, ((QDEPTH - 1) << 16) | (QDEPTH - 1));
    wr64(nv_base + NVME_ASQ, (u64)NMEM_ASQ);
    wr64(nv_base + NVME_ACQ, (u64)NMEM_ACQ);

    /* CC: 4 KiB pages (MPS=0), NVM command set (CSS=0), round-robin
     * arbitration, and the queue entry sizes as powers of two - 2^6 = 64 for
     * submission, 2^4 = 16 for completion. Those are fixed by the spec and
     * getting them wrong means the controller reads our commands at the wrong
     * stride. */
    u32 newcc = CC_EN | (6u << 16) | (4u << 20);
    wr32(nv_base + NVME_CC, newcc);
    if (!wait_csts(CSTS_RDY, 1, 2000)) return 0;

    nv_ready = 1;
    return 1;
}

/* Identify: CNS=1 asks about the controller, CNS=0 about a namespace. The
 * reply is 4096 bytes of structured data. */
static u8 model_str[40];
static u8 serial_str[20];

int nvme_identify_controller(void)
{
    if (!nv_ready) return 0;
    zero_mem(NMEM_IDENT, 4096);
    if (admin_cmd(ADMIN_IDENTIFY, 0, (u64)NMEM_IDENT, 1, 0) != 0) return 0;

    /* Copy the strings out NOW. Identify Namespace reuses this same buffer,
     * and reading the model afterwards returns namespace fields interpreted as
     * text - which looks like a driver bug and is really a lifetime bug. */
    for (int i = 0; i < 20; i++)
        serial_str[i] = *(volatile u8 *)(uptr)(NMEM_IDENT + 4 + (u32)i);
    for (int i = 0; i < 40; i++)
        model_str[i] = *(volatile u8 *)(uptr)(NMEM_IDENT + 24 + (u32)i);
    return 1;
}

int nvme_identify_namespace(void)
{
    if (!nv_ready) return 0;
    zero_mem(NMEM_IDENT, 4096);
    if (admin_cmd(ADMIN_IDENTIFY, nv_nsid, (u64)NMEM_IDENT, 0, 0) != 0) return 0;

    volatile u32 *id = (volatile u32 *)(uptr)NMEM_IDENT;
    nv_blocks    = ((u64)id[1] << 32) | (u64)id[0];       /* NSZE, in blocks */
    nv_maxlba_lo = id[0];

    /* LBA Format 0 is at byte 128; its LBADS field (byte 130) is the block
     * size as a power of two. 9 means 512 bytes, 12 means 4096. */
    u32 lbaf0 = id[32];
    u32 lbads = (lbaf0 >> 16) & 0xFF;
    if (lbads >= 9 && lbads <= 16) nv_blocksize = 1u << lbads;

    /* Every command this driver issues carries PRP1 and no PRP2, so one
     * transfer is one 4 KiB page. A device with a larger logical block would
     * have the CONTROLLER DMA past the end of that page and into whatever the
     * next arena holds - silently, with a successful completion status. The
     * old code accepted LBADS up to 16 (64 KiB) and would have done exactly
     * that. Nothing common reports more than 4096, which is precisely why
     * this would have sat here undiscovered.
     *
     * Supporting it means a PRP list, which is a real feature and not one to
     * fake. Refusing is honest; corrupting the xHCI arena is not. */
    if (nv_blocksize > 4096) {
        nv_fault = NVF_BLOCKSIZE;
        return 0;
    }
    return 1;
}

/* The recurring bug class, five times so far: a DMA buffer that is not inside
 * guest RAM. The symptoms read as protocol bugs every time - a command that
 * completes with good status and no data. This arena sits at 208 MiB, so a
 * machine given less than that fails here rather than three layers up.
 *
 * Probe the LOWEST and HIGHEST addresses the driver actually uses, not just
 * the base: the failure mode is a boundary, and a base-only probe passes on a
 * machine whose RAM ends in the middle of the arena. */
int nvme_ram_ok(void)
{
    volatile u32 *lo = (volatile u32 *)(uptr)NMEM_ASQ;
    volatile u32 *hi = (volatile u32 *)(uptr)(NMEM_DATA + 4096 - 4);
    u32 save_lo = *lo, save_hi = *hi;
    *lo = 0xA5A5F00Du;
    *hi = 0x5A5A0FF0u;
    int good = (*lo == 0xA5A5F00Du) && (*hi == 0x5A5A0FF0u);
    *lo = save_lo; *hi = save_hi;
    return good;
}

int nvme_fault(void) { return nv_fault; }

/* Identify Controller puts the model string at byte 24, 40 bytes, space
 * padded. Serial is at byte 4 for 20 bytes. */
int nvme_model_byte(int i)  { return (i >= 0 && i < 40) ? (int)model_str[i]  : 0; }
int nvme_serial_byte(int i) { return (i >= 0 && i < 20) ? (int)serial_str[i] : 0; }

u32 nvme_blocks_lo(void)  { return (u32)(nv_blocks & 0xFFFFFFFFu); }
u32 nvme_blocks_hi(void)  { return (u32)(nv_blocks >> 32); }
u32 nvme_blocksize(void)  { return nv_blocksize; }
/* capacity in mebibytes, computed without a 64-bit divide in the hot path */
u32 nvme_capacity_mb(void)
{
    u64 bytes = nv_blocks * (u64)nv_blocksize;
    return (u32)(bytes >> 20);
}

/* ---- I/O queues --------------------------------------------------------
 * Created with admin commands. The completion queue must exist before the
 * submission queue that points at it, which is why the order matters. */
int nvme_create_io_queues(void)
{
    if (!nv_ready) return 0;
    zero_mem(NMEM_IOSQ, 4096);
    zero_mem(NMEM_IOCQ, 4096);
    iosq_tail = 0; iocq_head = 0; iocq_phase = 1;

    /* CREATE_CQ: cdw10 = qid | (size-1)<<16, cdw11 = PC | IEN.
     * PC (physically contiguous) is bit 0; interrupts stay off - we poll. */
    if (admin_cmd(ADMIN_CREATE_CQ, 0, (u64)NMEM_IOCQ,
                  1u | ((QDEPTH - 1) << 16), 1u) != 0) return 0;

    /* CREATE_SQ: cdw11 = PC | (completion queue id)<<16 */
    if (admin_cmd(ADMIN_CREATE_SQ, 0, (u64)NMEM_IOSQ,
                  1u | ((QDEPTH - 1) << 16), 1u | (1u << 16)) != 0) return 0;
    return 1;
}

/* Read or write one block. Deliberately one block at a time and into a fixed
 * page: that keeps every transfer inside a single 4 KiB region, so PRP2 is
 * never needed and there is no scatter list to get wrong. */
static int io_one(u8 opcode, u32 lba_lo, u32 lba_hi)
{
    if (!nv_ready) return 0;
    u16 cid = next_cid++;
    sqe(NMEM_IOSQ, iosq_tail, opcode, cid, nv_nsid, (u64)NMEM_DATA,
        lba_lo, lba_hi, 0 /* NLB is zero-based: 0 means one block */);
    iosq_tail = (u16)((iosq_tail + 1) % QDEPTH);
    wr32(doorbell(1, 0), (u32)iosq_tail);
    return cq_wait(NMEM_IOCQ, &iocq_head, &iocq_phase, 1, cid) == 0;
}

int nvme_read_block(u32 lba_lo, u32 lba_hi)  { return io_one(IO_READ,  lba_lo, lba_hi); }
int nvme_write_block(u32 lba_lo, u32 lba_hi) { return io_one(IO_WRITE, lba_lo, lba_hi); }

/* ---- a block and an ARBITRARY address ----------------------------------
 * Everything above moves data to and from one fixed page, and the only way
 * out of it was nvme_data_byte/nvme_data_set - one byte per call. A
 * filesystem reading a 512-byte block through that seam costs 512 crossings
 * of the zl/C boundary per block, which is not a performance question so much
 * as a reason nobody would build a filesystem on top of it.
 *
 * The DMA still lands in NMEM_DATA and nowhere else. That is deliberate: it
 * keeps every transfer inside one 4 KiB page, so PRP2 stays unused and there
 * is no scatter list to get wrong. The caller's address is reached by a copy
 * on THIS side of the controller, where a mistake is a wrong byte rather than
 * a device writing into an arena that belongs to something else.
 */
int nvme_read_to(u32 dst, u32 lba_lo, u32 lba_hi)
{
    if (!nv_ready) return 0;
    if (!io_one(IO_READ, lba_lo, lba_hi)) return 0;
    volatile u8       *d = (volatile u8 *)(uptr)dst;
    volatile const u8 *s = (volatile const u8 *)(uptr)NMEM_DATA;
    for (u32 i = 0; i < nv_blocksize; i++) d[i] = s[i];
    return 1;
}

int nvme_write_from(u32 src, u32 lba_lo, u32 lba_hi)
{
    if (!nv_ready) return 0;
    volatile u8       *d = (volatile u8 *)(uptr)NMEM_DATA;
    volatile const u8 *s = (volatile const u8 *)(uptr)src;
    for (u32 i = 0; i < nv_blocksize; i++) d[i] = s[i];
    return io_one(IO_WRITE, lba_lo, lba_hi);
}

u32 nvme_data(void)          { return NMEM_DATA; }
int nvme_data_byte(int i)
{
    if (i < 0 || i >= 4096) return 0;
    return (int)*(volatile u8 *)(uptr)(NMEM_DATA + (u32)i);
}
void nvme_data_set(int i, int v)
{
    if (i < 0 || i >= 4096) return;
    *(volatile u8 *)(uptr)(NMEM_DATA + (u32)i) = (u8)v;
}

int nvme_ready(void) { return nv_ready; }

/* Bring the whole thing up: enable, identify, create I/O queues. */
int nvme_setup(void)
{
    nv_fault = NVF_NONE;
    if (!nvme_present() && nvme_find() < 0) { nv_fault = NVF_NO_DEV;    return 0; }
    if (!nvme_ram_ok())                     { nv_fault = NVF_RAM;       return 0; }
    if (!nvme_init())                       { nv_fault = NVF_NOT_READY; return 0; }
    if (!nvme_identify_controller())        { nv_fault = NVF_IDENT;     return 0; }
    /* identify_namespace sets NVF_BLOCKSIZE itself when that is the reason */
    if (!nvme_identify_namespace()) {
        if (nv_fault == NVF_NONE) nv_fault = NVF_IDENT;
        return 0;
    }
    if (!nvme_create_io_queues())           { nv_fault = NVF_QUEUES;    return 0; }
    return 1;
}
