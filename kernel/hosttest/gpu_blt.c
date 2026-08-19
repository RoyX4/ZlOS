/* gpu_blt.c - make the Intel blitter draw something, from Linux userspace,
 * on the real GPU, in about a second and with no reboot.
 *
 * WHY THIS EXISTS
 * ---------------
 * zlOS has never used a GPU for anything. `intel.c` is 5000+ lines of display
 * and modeset - GGTT, pipes, DPLL, EDID, AUX - and not one line that makes the
 * hardware DRAW. Every pixel the desktop has ever shown came out of a CPU loop
 * in fb.c; fb3d.c's header says "no GPU" in as many words.
 *
 * The first step toward changing that is not writing a ring buffer in the
 * kernel. It is finding out whether we can encode a blitter command the
 * hardware accepts AT ALL, because that is the part most likely to be wrong and
 * the part hardest to debug from inside a kernel with no debugger.
 *
 * So this splits the problem in two:
 *
 *   1. IS THE COMMAND RIGHT?   <- this file. i915 owns the ring and schedules
 *                                 our batch on the real BCS engine. If the
 *                                 rectangle appears, the command stream is
 *                                 correct on this silicon. Seconds per run.
 *   2. CAN WE OWN THE RING?    <- zlOS side, later, with a command stream that
 *                                 is already known-good.
 *
 * THIS DOES NOT DETACH i915 AND MUST NOT START. `modeset-run.sh` blanks the
 * screen and detaches the driver because it reprograms the DISPLAY, which is
 * i915's exclusively. The blitter is not the display: it is a DMA engine that
 * moves pixels between buffers. Submitting to it through the render node is the
 * ordinary, supported way for any userspace program to use it, runs alongside
 * the desktop, and cannot blank anything. It is also the only option that does
 * not interrupt whoever is using this laptop.
 *
 * NO PANEL POWER IS TOUCHED HERE, deliberately. intel.c's hazard list (T12's
 * 500 ms power-cycle delay, AUX into an unpowered panel) is about hardware this
 * file never addresses. Keep it that way: if a change here starts needing panel
 * registers, it belongs in a different file with a different review.
 *
 * BUILD/RUN
 *   cd kernel/hosttest && ./build.sh && ./gpu_blt
 *   ./gpu_blt --probe    what the GPU is and which engines it has (read-only)
 *   ./gpu_blt --blit     the actual fill, verified by reading the result back
 *   ./gpu_blt --bench    the same fill many times, against a CPU fill
 *
 * Everything here is raw ioctl on /dev/dri/renderD128. No libdrm, no Mesa - the
 * point is to learn the register/command level that zlOS will have to speak,
 * not to borrow a library zlOS cannot have.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <drm/drm.h>
#include <drm/i915_drm.h>

/* THE KERNEL'S OWN COMMAND EMITTER, compiled straight into this harness.
 * Not a copy - the same text. Whatever this file proves on silicon is proved
 * about the code zlOS will ship, and a later edit to gpu.c cannot drift away
 * from the encoding verified here. */
#include "../gpu.c"

/* ---- the command we are trying to get the hardware to run ----------------
 *
 * XY_COLOR_BLT: fill a rectangle in a destination surface with a constant
 * colour. Seven dwords on Gen8+, where the destination address is 64-bit.
 * This is the simplest thing the blitter can do and therefore the right first
 * one: no source surface, no shader, no state, one command.
 *
 *   DW0  opcode, and how many dwords follow
 *   DW1  BR13: raster op, colour depth, destination pitch IN BYTES
 *   DW2  top-left     y in 31:16, x in 15:0
 *   DW3  bottom-right y in 31:16, x in 15:0   (exclusive)
 *   DW4  destination address, low 32
 *   DW5  destination address, high 32
 *   DW6  the colour
 *
 * The client/opcode split: bits 31:29 select the BLT client (2), bits 28:22
 * the opcode (0x50). Bits 21 and 20 say to write alpha and RGB - without them
 * the blit is a no-op that completes successfully, which is a very confusing
 * way to fail. The low bits are (dword count - 2).
 */
#define BLT_CLIENT        (2u << 29)
#define XY_COLOR_BLT_OP   (0x50u << 22)
#define BLT_WRITE_ALPHA   (1u << 21)
#define BLT_WRITE_RGB     (1u << 20)
#define XY_COLOR_BLT_LEN  (7u - 2u)
#define XY_COLOR_BLT_DW0  (BLT_CLIENT | XY_COLOR_BLT_OP | \
                           BLT_WRITE_ALPHA | BLT_WRITE_RGB | XY_COLOR_BLT_LEN)

#define BR13_ROP_PATCOPY  (0xF0u << 16)   /* dst = pattern, i.e. a solid fill */
#define BR13_DEPTH_32BPP  (3u << 24)

#define MI_BATCH_BUFFER_END 0x05000000u

/* Where we ask the kernel to place our buffers in the GPU's address space.
 * Softpin (EXEC_OBJECT_PINNED) instead of relocations: relocations are the
 * legacy path, they are gone entirely on newer hardware, and pinning means the
 * address in the batch is one WE chose - which is exactly the discipline zlOS
 * will need when it owns the GGTT itself. Any page-aligned address inside the
 * ppGTT will do; these are picked to be obviously deliberate in a dump. */
#define DST_GPU_ADDR   0x00200000ull   /* 2 MiB;  the surface is 3 MiB, so it
                                        * runs to 5 MiB - the batch must clear
                                        * that, and the first version of this
                                        * file put it at 3 MiB, INSIDE the
                                        * destination. The kernel rejected the
                                        * whole submission with ENOSPC ("No
                                        * space left on device"), which reads
                                        * like the GPU is out of memory and is
                                        * really "your two pinned objects
                                        * overlap". Softpin means these
                                        * addresses are OURS to get right. */
#define BATCH_GPU_ADDR 0x08000000ull   /* 128 MiB, clear of a 33 MiB 4K surface */

/* The surface is allocated at the largest size any mode uses, once, and the
 * smaller tests fill a rectangle inside it. Same buffer, same pitch, same
 * cache behaviour for every measurement - otherwise the sweep compares
 * allocation luck rather than fill rate. */
#define W 3840
#define H 2160
#define BPP 4
#define PITCH (W * BPP)                /* 15360; BR13's pitch field is 16 bits,
                                        * so this is near the format's ceiling */
#define BATCH_BYTES 65536u

/* Catch that overlap at COMPILE time rather than as an ioctl errno. This is
 * the same discipline memmap.h imposes on the kernel's fixed high-RAM map, for
 * exactly the same reason: two hand-picked addresses whose ranges cross are
 * individually sensible and only wrong when you subtract them. */
_Static_assert(DST_GPU_ADDR + (unsigned long long)(W * BPP) * H <= BATCH_GPU_ADDR,
               "the batch buffer is pinned inside the destination surface");
#define FILL_COLOR 0x60D2EBu          /* the northstar accent, so a stray
                                       * frame on screen is recognisable */

static int drm_fd = -1;

static double now_s(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

/* ioctl that retries on EINTR/EAGAIN, which DRM returns routinely under load
 * and which is NOT an error - treating it as one produces intermittent
 * failures that look like hardware flakiness. */
static int drm_ioctl(unsigned long req, void *arg)
{
    int r;
    do { r = ioctl(drm_fd, req, arg); } while (r == -1 && (errno == EINTR || errno == EAGAIN));
    return r;
}

static int open_render_node(void)
{
    /* renderD128 is the RENDER node: compute and DMA engines, no modeset
     * capability at all. card0 could change the display; we deliberately do
     * not open it. If this fails with EACCES the user is not in `render` and
     * has no ACL - say so plainly rather than reporting "no GPU". */
    drm_fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    if (drm_fd < 0) {
        fprintf(stderr, "cannot open /dev/dri/renderD128: %s\n", strerror(errno));
        if (errno == EACCES)
            fprintf(stderr, "  (not in group `render` and no ACL grants you access)\n");
        return 0;
    }
    return 1;
}

static int getparam(int param, int *out)
{
    struct drm_i915_getparam gp;
    memset(&gp, 0, sizeof gp);
    gp.param = param;
    gp.value = out;
    return drm_ioctl(DRM_IOCTL_I915_GETPARAM, &gp) == 0;
}

static const char *engine_class_name(unsigned c)
{
    switch (c) {
    case I915_ENGINE_CLASS_RENDER:        return "RCS  render";
    case I915_ENGINE_CLASS_COPY:          return "BCS  copy/blitter";
    case I915_ENGINE_CLASS_VIDEO:         return "VCS  video decode";
    case I915_ENGINE_CLASS_VIDEO_ENHANCE: return "VECS video enhance";
    default:                              return "?    unknown";
    }
}

/* Returns 1 if a COPY-class engine exists - i.e. there is a blitter to talk
 * to at all. Everything after this depends on that being true. */
static int probe(int verbose)
{
    int chipset = 0, has_blt = 0;
    int have_copy = 0;

    if (!getparam(I915_PARAM_CHIPSET_ID, &chipset))
        fprintf(stderr, "  warn  CHIPSET_ID unavailable: %s\n", strerror(errno));
    getparam(I915_PARAM_HAS_BLT, &has_blt);

    if (verbose) {
        printf("  device        0x%04X%s\n", chipset,
               chipset == 0x9B41 ? "  (CometLake-U GT2, Gen9.5 - the ThinkPad panel)" : "");
        printf("  HAS_BLT       %d\n", has_blt);
    }

    /* Enumerate engines the honest way: ask, twice - once for the size, once
     * for the data. A hardcoded engine list is how you end up submitting to an
     * engine this silicon does not have. */
    struct drm_i915_query_item item;
    struct drm_i915_query q;
    memset(&item, 0, sizeof item);
    memset(&q, 0, sizeof q);
    item.query_id = DRM_I915_QUERY_ENGINE_INFO;
    q.num_items = 1;
    q.items_ptr = (unsigned long long)(uintptr_t)&item;

    if (drm_ioctl(DRM_IOCTL_I915_QUERY, &q) || item.length <= 0) {
        fprintf(stderr, "  FAIL  engine query: %s\n", strerror(errno));
        return 0;
    }
    void *buf = calloc(1, (size_t)item.length);
    if (!buf) return 0;
    item.data_ptr = (unsigned long long)(uintptr_t)buf;
    if (drm_ioctl(DRM_IOCTL_I915_QUERY, &q) || item.length <= 0) {
        fprintf(stderr, "  FAIL  engine query (data): %s\n", strerror(errno));
        free(buf);
        return 0;
    }

    struct drm_i915_query_engine_info *ei = buf;
    if (verbose) printf("  engines       %u\n", ei->num_engines);
    for (unsigned i = 0; i < ei->num_engines; i++) {
        unsigned cls = ei->engines[i].engine.engine_class;
        if (verbose)
            printf("    class %u instance %u   %s\n",
                   cls, ei->engines[i].engine.engine_instance, engine_class_name(cls));
        if (cls == I915_ENGINE_CLASS_COPY) have_copy = 1;
    }
    free(buf);

    if (verbose)
        printf("  %s\n", have_copy
               ? "ok    a COPY engine exists - there is a blitter to submit to"
               : "FAIL  no COPY engine on this device");
    return have_copy;
}

/* ---- buffer objects ------------------------------------------------------ */

static int bo_create(unsigned long long size, unsigned *handle)
{
    struct drm_i915_gem_create c;
    memset(&c, 0, sizeof c);
    c.size = size;
    if (drm_ioctl(DRM_IOCTL_I915_GEM_CREATE, &c)) {
        fprintf(stderr, "  FAIL  GEM_CREATE %llu bytes: %s\n", size, strerror(errno));
        return 0;
    }
    *handle = c.handle;
    return 1;
}

static void *bo_map(unsigned handle, size_t size)
{
    struct drm_i915_gem_mmap_offset mo;
    memset(&mo, 0, sizeof mo);
    mo.handle = handle;
    /* Write-back: this part has LLC, so the CPU and the GPU are coherent
     * through the cache and a plain read after the blit sees the result. On a
     * part without LLC this would have to be WC plus explicit domain flushes -
     * a difference worth knowing about before assuming zlOS can be this
     * casual. */
    mo.flags = I915_MMAP_OFFSET_WB;
    if (drm_ioctl(DRM_IOCTL_I915_GEM_MMAP_OFFSET, &mo)) {
        fprintf(stderr, "  FAIL  MMAP_OFFSET: %s\n", strerror(errno));
        return NULL;
    }
    void *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, (off_t)mo.offset);
    if (p == MAP_FAILED) {
        fprintf(stderr, "  FAIL  mmap: %s\n", strerror(errno));
        return NULL;
    }
    return p;
}

static void bo_close(unsigned handle)
{
    struct drm_gem_close c;
    memset(&c, 0, sizeof c);
    c.handle = handle;
    drm_ioctl(DRM_IOCTL_GEM_CLOSE, &c);
}

/* Make the CPU's view of the buffer current before we read it. GEM_WAIT says
 * the GPU is finished; SET_DOMAIN(CPU) is what guarantees we are not reading a
 * stale cache line. Skipping it is the classic way to get a test that passes
 * on one machine and fails on another. */
static void bo_to_cpu(unsigned handle)
{
    struct drm_i915_gem_set_domain sd;
    memset(&sd, 0, sizeof sd);
    sd.handle = handle;
    sd.read_domains = I915_GEM_DOMAIN_CPU;
    sd.write_domain = I915_GEM_DOMAIN_CPU;
    drm_ioctl(DRM_IOCTL_I915_GEM_SET_DOMAIN, &sd);
}

/* Build the batch: one XY_COLOR_BLT, then MI_BATCH_BUFFER_END. Returns the
 * number of BYTES written. */
static unsigned build_blit_batch(unsigned *b, int x1, int y1, int x2, int y2,
                                 unsigned pitch, unsigned long long dst_addr,
                                 unsigned color, int drop_rgb)
{
    struct gpu_batch batch;
    gpu_batch_init(&batch, b, BATCH_BYTES / 4);
    if (!gpu_fill_rect(&batch, dst_addr, pitch, x1, y1, x2, y2, color))
        fprintf(stderr, "  warn  gpu_fill_rect refused the rectangle\n");
    /* --negative clears BLT_WRITE_RGB AFTER the fact. gpu.c will not emit a
     * command it believes is broken, and should not - so the harness breaks it
     * here, outside the kernel code, which is the honest place for it. */
    if (drop_rgb && batch.at >= 1) b[0] &= ~GPU_BLT_WRITE_RGB;
    gpu_batch_end(&batch);
    return gpu_batch_bytes(&batch);
}

/* K back-to-back XY_COLOR_BLTs in ONE batch, then END.
 *
 * This is the variable the first benchmark got wrong. Measuring one blit per
 * submission measures the SUBMISSION, not the blitter - and zlOS, owning the
 * ring itself, will not pay an ioctl per rectangle. A compositor frame is tens
 * or hundreds of rectangles, so batching is the realistic shape and one-per-
 * submit is the pessimal one. */
static unsigned build_blit_batch_n(unsigned *b, int k, int x1, int y1, int x2, int y2,
                                   unsigned pitch, unsigned long long dst_addr,
                                   unsigned color)
{
    struct gpu_batch batch;
    gpu_batch_init(&batch, b, BATCH_BYTES / 4);
    for (int n = 0; n < k; n++)
        if (!gpu_fill_rect(&batch, dst_addr, pitch, x1, y1, x2, y2, color)) break;
    gpu_batch_end(&batch);
    return gpu_batch_bytes(&batch);
}

/* Submit and wait. Returns 1 on success. */
static int submit(unsigned dst_handle, unsigned batch_handle, unsigned batch_bytes)
{
    struct drm_i915_gem_exec_object2 obj[2];
    struct drm_i915_gem_execbuffer2 eb;
    memset(obj, 0, sizeof obj);
    memset(&eb, 0, sizeof eb);

    /* The destination is written, so it must be flagged EXEC_OBJECT_WRITE -
     * that is what makes the kernel serialise our readback against the blit
     * instead of letting them race. */
    obj[0].handle = dst_handle;
    obj[0].offset = DST_GPU_ADDR;
    obj[0].flags  = EXEC_OBJECT_PINNED | EXEC_OBJECT_SUPPORTS_48B_ADDRESS | EXEC_OBJECT_WRITE;

    obj[1].handle = batch_handle;
    obj[1].offset = BATCH_GPU_ADDR;
    obj[1].flags  = EXEC_OBJECT_PINNED | EXEC_OBJECT_SUPPORTS_48B_ADDRESS;

    eb.buffers_ptr = (unsigned long long)(uintptr_t)obj;
    eb.buffer_count = 2;
    eb.batch_len = batch_bytes;
    /* I915_EXEC_BLT picks the COPY engine. I915_EXEC_NO_RELOC promises we
     * pinned everything ourselves, which we did. */
    eb.flags = I915_EXEC_BLT | I915_EXEC_NO_RELOC;

    if (drm_ioctl(DRM_IOCTL_I915_GEM_EXECBUFFER2, &eb)) {
        fprintf(stderr, "  FAIL  EXECBUFFER2: %s\n", strerror(errno));
        return 0;
    }

    struct drm_i915_gem_wait w;
    memset(&w, 0, sizeof w);
    w.bo_handle = dst_handle;
    w.timeout_ns = 2000000000ll;         /* 2 s: generous, and bounded */
    if (drm_ioctl(DRM_IOCTL_I915_GEM_WAIT, &w)) {
        fprintf(stderr, "  FAIL  GEM_WAIT: %s\n", strerror(errno));
        return 0;
    }
    return 1;
}

/* ---- the actual test ----------------------------------------------------- */

/* A number recorded without its conditions is not a measurement. This box runs
 * several agent sessions at once and the load average has been seen at 14 while
 * a benchmark was running - which moves BOTH columns, and not by the same
 * amount. Print it next to the result so a figure copied out of this output
 * carries the caveat with it. */
static double loadavg1(void)
{
    FILE *f = fopen("/proc/loadavg", "r");
    double la = -1;
    if (f) { if (fscanf(f, "%lf", &la) != 1) la = -1; fclose(f); }
    return la;
}

static void warn_if_busy(void)
{
    double la = loadavg1();
    printf("  load average  %.2f%s\n", la,
           la > 2.0 ? "   <-- CONTENDED. Treat these numbers as indicative only;"
                      " re-run on a quiet box before quoting them." : "");
}

/* `drop_rgb` deliberately breaks the command the way the hardware permits it to
 * be broken silently: without BLT_WRITE_RGB the blit is dispatched, completes,
 * reports no error, and writes nothing. It is the exact failure this file's
 * verification exists to catch, so --negative runs it on purpose and FAILS if
 * the check says everything is fine. A verifier nobody has watched reject a
 * bad result is not a verifier. */
static int do_blit_ex(int bench, int drop_rgb);
static int do_blit(int bench) { return do_blit_ex(bench, 0); }

static int do_blit_ex(int bench, int drop_rgb)
{
    const size_t dst_size = (size_t)PITCH * H;
    unsigned dst_h = 0, batch_h = 0;
    unsigned *dst = NULL, *batch = NULL;
    int rc = 1;

    if (!bo_create(dst_size, &dst_h)) return 1;
    if (!bo_create(4096, &batch_h)) { bo_close(dst_h); return 1; }

    dst = bo_map(dst_h, dst_size);
    batch = bo_map(batch_h, 4096);
    if (!dst || !batch) goto out;

    /* Poison the destination first. If the blit does nothing, the readback
     * below sees the poison and says so - a test that starts from zeroed
     * memory and checks for zero cannot tell "filled with 0" from "never
     * ran", and this command has a documented way of completing successfully
     * while writing nothing (see BLT_WRITE_RGB). */
    const unsigned POISON = 0xDEADBEEFu;
    for (size_t i = 0; i < dst_size / 4; i++) dst[i] = POISON;

    const int bx1 = 100, by1 = 50, bx2 = 700, by2 = 500;
    unsigned batch_bytes = build_blit_batch(batch, bx1, by1, bx2, by2,
                                            PITCH, DST_GPU_ADDR, FILL_COLOR, drop_rgb);

    printf("  batch         %u bytes: DW0=0x%08X BR13=0x%08X\n",
           batch_bytes, batch[0], batch[1]);
    printf("  rect          (%d,%d)..(%d,%d) in a %dx%d 32bpp surface, pitch %d\n",
           bx1, by1, bx2, by2, W, H, PITCH);

    double t0 = now_s();
    if (!submit(dst_h, batch_h, batch_bytes)) { rc = 1; goto out; }
    double t1 = now_s();

    bo_to_cpu(dst_h);

    /* Verify by READING THE RESULT, not by the absence of a hang. Three
     * distinct checks, because each catches a different wrong outcome:
     * inside must be the colour, outside must still be poison, and the
     * boundary must be exactly where we asked. */
    size_t inside_ok = 0, inside_bad = 0, outside_clobbered = 0;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            unsigned px = dst[(size_t)y * W + x] & 0x00FFFFFFu;
            int in = (x >= bx1 && x < bx2 && y >= by1 && y < by2);
            if (in) { if (px == FILL_COLOR) inside_ok++; else inside_bad++; }
            else if (dst[(size_t)y * W + x] != POISON) outside_clobbered++;
        }
    }
    size_t want = (size_t)(bx2 - bx1) * (size_t)(by2 - by1);

    printf("  inside        %zu/%zu pixels are 0x%06X\n", inside_ok, want, FILL_COLOR);
    printf("  outside       %zu pixels clobbered (want 0)\n", outside_clobbered);
    printf("  submit+wait   %.3f ms\n", (t1 - t0) * 1e3);
    warn_if_busy();

    if (inside_ok == want && inside_bad == 0 && outside_clobbered == 0) {
        printf("  ok    THE BLITTER DREW IT. Command stream is correct on this silicon.\n");
        rc = 0;
    } else {
        printf("  FAIL  the blit did not produce the expected rectangle\n");
        if (inside_ok == 0 && outside_clobbered == 0)
            printf("        nothing was written at all - the batch ran but drew nothing\n");
        rc = 1;
    }

    if (rc == 0 && bench) {
        /* Compare against the CPU doing the identical fill. This is the number
         * the whole exercise is for, and it is deliberately measured on the
         * SAME buffer and the SAME rectangle. */
        const int N = 200;
        double g0 = now_s();
        for (int i = 0; i < N; i++) submit(dst_h, batch_h, batch_bytes);
        double g1 = now_s();

        bo_to_cpu(dst_h);
        double c0 = now_s();
        for (int i = 0; i < N; i++)
            for (int y = by1; y < by2; y++) {
                unsigned *row = dst + (size_t)y * W + bx1;
                for (int x = 0; x < bx2 - bx1; x++) row[x] = FILL_COLOR;
            }
        double c1 = now_s();

        double gpu_ms = (g1 - g0) * 1e3 / N, cpu_ms = (c1 - c0) * 1e3 / N;
        double mpix = (double)want / 1e6;
        printf("\n  %d iterations of a %.2f Mpixel fill:\n", N, mpix);
        printf("    blitter     %7.3f ms/fill   %8.1f Mpix/s\n", gpu_ms, mpix / (gpu_ms / 1e3));
        printf("    CPU         %7.3f ms/fill   %8.1f Mpix/s\n", cpu_ms, mpix / (cpu_ms / 1e3));
        if (gpu_ms < cpu_ms)
            printf("    blitter is %.2fx faster\n", cpu_ms / gpu_ms);
        else
            printf("    CPU is %.2fx faster - at THIS size, submission cost dominates\n",
                   gpu_ms / cpu_ms);
    }

out:
    if (dst) munmap(dst, dst_size);
    if (batch) munmap(batch, 4096);
    if (dst_h) bo_close(dst_h);
    if (batch_h) bo_close(batch_h);
    return rc;
}

/* ---- the sweep: where, if anywhere, does the blitter win? ---------------- */

static int do_sweep(void)
{
    const size_t dst_size = (size_t)PITCH * H;
    unsigned dst_h = 0, batch_h = 0;
    unsigned *dst = NULL, *batch = NULL;
    int rc = 1;

    if (!bo_create(dst_size, &dst_h)) return 1;
    if (!bo_create(BATCH_BYTES, &batch_h)) { bo_close(dst_h); return 1; }
    dst = bo_map(dst_h, dst_size);
    batch = bo_map(batch_h, BATCH_BYTES);
    if (!dst || !batch) goto out;

    printf("  surface %dx%d 32bpp, pitch %d (%.1f MiB), one allocation for every row\n",
           W, H, PITCH, (double)dst_size / (1024 * 1024));
    warn_if_busy();
    printf("  K = blits per submission. zlOS owns its own ring, so the K=1 column\n");
    printf("  is the cost of the IOCTL, not of the blitter.\n\n");
    printf("    rect          Mpix   K   blitter ms   Mpix/s  |   CPU ms   Mpix/s  | winner\n");
    printf("    --------------------------------------------------------------------------\n");

    struct { int w, h; } sizes[] = {
        {  64,  64 }, { 256, 256 }, { 512, 512 },
        { 1024, 768 }, { 1920, 1200 }, { 3840, 2160 },
    };
    const int Ks[] = { 1, 64 };

    for (unsigned s = 0; s < sizeof sizes / sizeof sizes[0]; s++) {
        int rw = sizes[s].w, rh = sizes[s].h;
        double mpix = (double)rw * rh / 1e6;

        for (unsigned ki = 0; ki < sizeof Ks / sizeof Ks[0]; ki++) {
            int k = Ks[ki];
            /* keep the batch inside its buffer */
            if ((unsigned)(k * 7 + 2) * 4u > BATCH_BYTES) continue;

            unsigned bytes = build_blit_batch_n(batch, k, 0, 0, rw, rh,
                                                PITCH, DST_GPU_ADDR, FILL_COLOR);
            /* iterations chosen so every row runs for a similar wall time */
            int iters = mpix > 4.0 ? 20 : (mpix > 0.5 ? 60 : 200);

            submit(dst_h, batch_h, bytes);            /* warm up */
            double g0 = now_s();
            for (int i = 0; i < iters; i++) submit(dst_h, batch_h, bytes);
            double g1 = now_s();
            double gpu_ms = (g1 - g0) * 1e3 / iters / k;

            bo_to_cpu(dst_h);
            double c0 = now_s();
            for (int i = 0; i < iters * k; i++)
                for (int y = 0; y < rh; y++) {
                    unsigned *row = dst + (size_t)y * W;
                    for (int x = 0; x < rw; x++) row[x] = FILL_COLOR;
                }
            double c1 = now_s();
            double cpu_ms = (c1 - c0) * 1e3 / iters / k;

            const char *win = gpu_ms < cpu_ms ? "blitter" : "CPU";
            double ratio = gpu_ms < cpu_ms ? cpu_ms / gpu_ms : gpu_ms / cpu_ms;
            printf("    %4dx%-4d  %8.2f  %3d  %9.4f %8.0f  | %8.4f %8.0f  | %s %.2fx\n",
                   rw, rh, mpix, k, gpu_ms, mpix / (gpu_ms / 1e3),
                   cpu_ms, mpix / (cpu_ms / 1e3), win, ratio);
        }
    }
    printf("\n");
    rc = 0;
out:
    if (dst) munmap(dst, dst_size);
    if (batch) munmap(batch, BATCH_BYTES);
    if (dst_h) bo_close(dst_h);
    if (batch_h) bo_close(batch_h);
    return rc;
}

int main(int argc, char **argv)
{
    int want_probe = 0, want_blit = 0, want_bench = 0, want_sweep = 0, want_negative = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--probe")) want_probe = 1;
        else if (!strcmp(argv[i], "--blit")) want_blit = 1;
        else if (!strcmp(argv[i], "--bench")) { want_blit = 1; want_bench = 1; }
        else if (!strcmp(argv[i], "--sweep")) want_sweep = 1;
        else if (!strcmp(argv[i], "--negative")) want_negative = 1;
        else { fprintf(stderr, "usage: %s [--probe|--blit|--bench|--sweep|--negative]\n", argv[0]); return 2; }
    }
    if (!want_probe && !want_blit && !want_sweep && !want_negative) { want_probe = 1; want_blit = 1; }

    if (!open_render_node()) return 77;   /* 77 = skip, not fail: no GPU here */

    int rc = 0;
    printf("gpu_blt: the Intel blitter, through the render node, alongside i915\n\n");

    printf("probe:\n");
    int have_copy = probe(1);
    if (!have_copy) { close(drm_fd); return 1; }

    if (want_blit) {
        printf("\nblit:\n");
        rc = do_blit(want_bench);
    }
    if (want_sweep && rc == 0) {
        printf("\nsweep:\n");
        rc = do_sweep();
    }
    if (want_negative) {
        /* THE CHECK MUST BE ABLE TO FAIL. Clear BLT_WRITE_RGB and the hardware
         * accepts the batch, runs it, reports no error and writes nothing -
         * the one failure mode this file's verification exists to catch. If
         * do_blit_ex still says "ok" here, the verification is decorative and
         * every green run above meant nothing. */
        printf("\nnegative control (BLT_WRITE_RGB cleared - the blit MUST NOT fill):\n");
        int neg = do_blit_ex(0, 1);
        if (neg != 0) {
            printf("  ok    the check REJECTED a blit that wrote nothing - it can fail\n");
        } else {
            printf("  FAIL  the check passed a blit that should have written nothing.\n");
            printf("        The verification is not verifying. Do not trust --blit.\n");
            rc = 1;
        }
    }

    close(drm_fd);
    return rc;
}
