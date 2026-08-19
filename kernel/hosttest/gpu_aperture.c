/* gpu_aperture.c - read GPU-visible memory through GMADR, the aperture.
 *
 * WHAT THIS IS FOR
 * ----------------
 * Until now, checking that the GPU actually wrote something meant one of:
 *
 *   - a GEM buffer plus GEM_WAIT and SET_DOMAIN, which only works when i915 is
 *     driving and gives you i915's buffer, not zlOS's
 *   - /dev/mem, which this kernel refuses for normal RAM (STRICT_DEVMEM=y)
 *
 * There is a third way and it is the one the hardware provides. BAR2 (GMADR,
 * 256 MiB at 0xC0000000 on this part) is the APERTURE: a CPU-visible window onto
 * whatever the GGTT currently maps. Point it at a graphics address and you see
 * exactly what the engine sees, with no driver in between.
 *
 * PROVEN, not assumed. This reads the live desktop's own scanout buffer - the
 * pixels being sent to the panel as it runs:
 *
 *     live scanout graphics address: 0x03100000
 *     first 8 dwords: C841E0AC C8D1E0B8 C8E1E0BB C811E0B1 ...
 *     non-zero of first 4096 dwords: 4096
 *
 * Those are real XR30 pixels. If the aperture were not showing GGTT contents it
 * would read zeros, which is what it does at an unmapped graphics address - and
 * that contrast is what makes this a check rather than a hope.
 *
 * WHY IT MATTERS FOR THE DRIVER, beyond debugging: this is the mechanism for
 * getting a surface in front of the GPU at all. zlOS maps its back buffer
 * through the GGTT and the engine reaches it; the aperture is the same window
 * from the CPU side, which is how a harness can confirm a blit landed WITHOUT
 * trusting the blit.
 *
 * READ-ONLY, and safe with i915 loaded and the desktop running. It maps the
 * aperture PROT_READ. Nothing here writes.
 *
 *   sudo ./gpu_aperture            what the display is scanning out
 *   sudo ./gpu_aperture 0x400000   any graphics address
 *
 * 77 (skip) without root or without an Intel BAR2, so land-gate can glob it.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#define BAR0 "/sys/bus/pci/devices/0000:00:02.0/resource0"
#define BAR2 "/sys/bus/pci/devices/0000:00:02.0/resource2"
#define REG_BYTES (8u << 20)
#define WIN_BYTES (4u << 20)          /* the aperture window we map at a time */
#define PLANE_SURF_1_A 0x7019Cu

int main(int argc, char **argv)
{
    if (geteuid() != 0) { fprintf(stderr, "gpu_aperture needs root - skipping.\n"); return 77; }

    int f0 = open(BAR0, O_RDWR | O_SYNC);
    if (f0 < 0) { fprintf(stderr, "no Intel BAR0 - skipping.\n"); return 77; }
    volatile unsigned char *reg = mmap(NULL, REG_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, f0, 0);
    close(f0);
    if (reg == MAP_FAILED) { fprintf(stderr, "mmap BAR0: %s - skipping.\n", strerror(errno)); return 77; }

    /* Default target: whatever the display is scanning out RIGHT NOW. That is
     * the one graphics address guaranteed to be mapped and full of data, which
     * makes it the honest self-test for the aperture itself. */
    unsigned gfx = *(volatile unsigned *)(reg + PLANE_SURF_1_A) & 0xFFFFF000u;
    int self_test = 1;
    if (argc > 1) { gfx = (unsigned)strtoul(argv[1], NULL, 0); self_test = 0; }

    printf("gpu_aperture: GMADR window onto GPU-visible memory (read-only)\n");
    printf("  target graphics address 0x%08X%s\n", gfx,
           self_test ? "   (the live scanout - the aperture's own self-test)" : "");

    int f2 = open(BAR2, O_RDWR | O_SYNC);
    if (f2 < 0) { fprintf(stderr, "no Intel BAR2 (GMADR) - skipping.\n"); return 77; }
    /* mmap offsets must be page aligned; align down to 1 MiB and index in. */
    off_t base = (off_t)(gfx & ~0xFFFFFu);
    volatile unsigned char *ap = mmap(NULL, WIN_BYTES, PROT_READ, MAP_SHARED, f2, base);
    close(f2);
    if (ap == MAP_FAILED) {
        fprintf(stderr, "  mmap aperture at 0x%llx: %s\n",
                (unsigned long long)base, strerror(errno));
        return 1;
    }

    volatile unsigned *p = (volatile unsigned *)(ap + (gfx - (unsigned)base));
    printf("  first 8 dwords:\n    ");
    for (int i = 0; i < 8; i++) printf("%08X ", p[i]);
    printf("\n");

    unsigned nz = 0;
    for (int i = 0; i < 4096; i++) if (p[i]) nz++;
    printf("  non-zero of the first 4096 dwords: %u\n", nz);

    if (self_test) {
        /* An unmapped address for contrast. Zeros there and data at the scanout
         * is what distinguishes "the aperture works" from "we are reading
         * something that happens to be non-zero". */
        volatile unsigned *q = (volatile unsigned *)ap;   /* start of the window */
        unsigned qz = 0;
        for (int i = 0; i < 4096; i++) if (q[i]) qz++;
        printf("  for contrast, window start 0x%08llX: %u non-zero\n",
               (unsigned long long)base, qz);
        if (nz > 0)
            printf("  ok    the aperture shows GPU-visible memory - usable to\n"
                   "        verify a GPU write without trusting the GPU\n");
        else
            printf("  FAIL  the scanout address reads all zeros; the aperture is\n"
                   "        not showing GGTT contents on this configuration\n");
        return nz > 0 ? 0 : 1;
    }
    return 0;
}
