/* gpu_planes.c - what does the firmware/driver ACTUALLY put in the plane
 * registers? Read-only, safe with i915 loaded, no arguments needed.
 *
 * WHY. intel.c states its own method plainly: "Everything else in this driver
 * is verified against what firmware programmed for the same hardware." There
 * was a tool for the timing registers (modeset_test) and none for the PLANE
 * registers - so the plane constants in intel.c, and the cursor mode
 * gpucursor.c depends on, were the one part of the driver with no witness.
 *
 * This is that witness. It maps BAR0, reads pipe A's three plane blocks and the
 * cursor block, and decodes the fields. It writes nothing.
 *
 * WHAT IT ESTABLISHED on 8086:9B41, 2026-08-19, against a live 2560x1440
 * desktop - all of it in kernel/docs/drivers/display/gpu-driver.md:
 *
 *   CUR_CTL mode bits = 0x27, which is exactly intel.c's CUR_MODE_64_ARGB.
 *     That constant is what gpucursor.c hands the display engine, and it had
 *     never been checked against anything.
 *
 *   PLANE_CTL format bits 27:24 = 0x2 on the primary, not the 0x4 that
 *     intel.c's PLANE_CTL_FORMAT_XRGB8888 uses. NOT a contradiction: i915 is
 *     running this panel at XR30 (XRGB2101010, 30-bit colour), so 0x2 is that
 *     format and 0x4 remains 8888. Worth knowing before a takeover, because a
 *     zlOS surface is 8888 and inherits nothing.
 *
 *   Alpha bits 5:4 = 0 - alpha ignored, the plane is opaque. NOTHING on this
 *     system currently uses a blended plane, so the encoding for a BLENDED one
 *     is still unverified and must not be guessed at.
 *
 * Exits 77 (skip) without root or without an Intel BAR0, so gates/land-gate.sh
 * can glob it like everything else in this directory.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#define BAR0 "/sys/bus/pci/devices/0000:00:02.0/resource0"
#define MAP_BYTES (8u << 20)

/* pipe A, from intel.c's own PLANE_STRIDE_BASE / PLANE_REG */
#define PLANE_BASE(p) (0x70180u + (unsigned)((p) - 1) * 0x100u)
#define PL_CTL 0x00u
#define PL_STRIDE 0x08u
#define PL_POS 0x0Cu
#define PL_SIZE 0x10u
#define PL_SURF 0x1Cu
#define CUR_CTL_A  0x70080u
#define CUR_BASE_A 0x70084u
#define CUR_POS_A  0x70088u

static volatile unsigned char *bar;
static unsigned R(unsigned o) { return *(volatile unsigned *)(bar + o); }

static const char *fmt_name(unsigned f)
{
    switch (f) {
    case 0x0: return "YUV422";
    case 0x2: return "XRGB2101010 (XR30 - 30-bit)";
    case 0x4: return "XRGB8888   (what zlOS draws)";
    case 0x6: return "XRGB16161616F";
    case 0xE: return "RGB565";
    default:  return "?";
    }
}
static const char *tile_name(unsigned t)
{
    switch (t) {
    case 0: return "linear";
    case 1: return "X-tiled";
    case 4: return "Y-tiled";
    case 5: return "Yf-tiled";
    default: return "?";
    }
}
static const char *alpha_name(unsigned a)
{
    switch (a) {
    case 0: return "ignored (opaque)";
    case 1: return "reserved";
    case 2: return "pre-multiplied";
    case 3: return "hardware alpha";
    default: return "?";
    }
}

int main(void)
{
    if (geteuid() != 0) { fprintf(stderr, "gpu_planes needs root (maps BAR0) - skipping.\n"); return 77; }
    int fd = open(BAR0, O_RDWR | O_SYNC);
    if (fd < 0) { fprintf(stderr, "no Intel BAR0 here - skipping.\n"); return 77; }
    bar = mmap(NULL, MAP_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (bar == MAP_FAILED) { fprintf(stderr, "mmap BAR0: %s - skipping.\n", strerror(errno)); return 77; }

    printf("gpu_planes: pipe A, read-only, i915 still holding the display\n\n");

    int enabled = 0;
    for (int p = 1; p <= 3; p++) {
        unsigned b = PLANE_BASE(p);
        unsigned ctl = R(b + PL_CTL);
        printf("  plane %d  CTL=0x%08X%s\n", p, ctl, (ctl & 0x80000000u) ? "  ENABLED" : "");
        if (!(ctl & 0x80000000u)) { printf("           (all registers idle)\n"); continue; }
        enabled++;
        unsigned size = R(b + PL_SIZE);
        printf("           format   0x%X  %s\n", (ctl >> 24) & 0xF, fmt_name((ctl >> 24) & 0xF));
        printf("           tiling   0x%X  %s\n", (ctl >> 10) & 0x7, tile_name((ctl >> 10) & 0x7));
        printf("           alpha    0x%X  %s\n", (ctl >> 4) & 0x3, alpha_name((ctl >> 4) & 0x3));
        printf("           size     %ux%u   stride reg 0x%X   surf 0x%08X\n",
               (size & 0x1FFFu) + 1u, ((size >> 16) & 0x1FFFu) + 1u,
               R(b + PL_STRIDE), R(b + PL_SURF));
    }

    unsigned cc = R(CUR_CTL_A);
    printf("\n  cursor   CUR_CTL=0x%08X  mode 0x%02X  base 0x%08X  pos 0x%08X\n",
           cc, cc & 0x3Fu, R(CUR_BASE_A), R(CUR_POS_A));
    /* 0x27 is intel.c's CUR_MODE_64_ARGB and 0x22 its CUR_MODE_128_ARGB. This
     * is the check that matters: gpucursor.c hands one of those to the display
     * engine and nothing had ever confirmed the encoding. */
    if ((cc & 0x3Fu) == 0x27u)
        printf("           mode 0x27 == intel.c's CUR_MODE_64_ARGB - CONFIRMED\n");
    else if ((cc & 0x3Fu) == 0x22u)
        printf("           mode 0x22 == intel.c's CUR_MODE_128_ARGB - CONFIRMED\n");
    else
        printf("           mode does NOT match either constant in intel.c - check them\n");

    printf("\n  %d of 3 plane blocks enabled. Reading zero does not prove a plane is\n", enabled);
    printf("  ABSENT - only that nothing is using it. i915 exposes two universal\n");
    printf("  planes per pipe on this part (1A PRI, 2A OVL), while intel.c's\n");
    printf("  comment says three; that discrepancy is unresolved and is why\n");
    printf("  intel_plane_setup still accepts 1..3.\n");
    return 0;
}
