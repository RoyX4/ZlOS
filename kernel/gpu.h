/* gpu.h - the shared surface between the command builder and the ring.
 *
 * gpu.c builds commands into a struct gpu_batch; gpuring.c submits them. Both
 * need the type, and duplicating a struct definition between two files is the
 * kind of thing that stays correct right up until someone adds a field. One
 * declaration, here.
 *
 * Kept deliberately small: this is not "the GPU API", it is the two things that
 * genuinely cross the file boundary.
 */
#ifndef ZL_GPU_H
#define ZL_GPU_H

typedef unsigned int       gpu_u32;
typedef unsigned long long gpu_u64;

/* A linear batch under construction. Nothing here allocates - the caller owns
 * the memory, because in the kernel it has to be memory the GGTT maps and in
 * the harness it is a GEM buffer. */
struct gpu_batch {
    gpu_u32 *dw;
    unsigned cap;
    unsigned at;
    int      overflow;      /* sticky: set once, never silently cleared */
};

void     gpu_batch_init(struct gpu_batch *b, gpu_u32 *storage, unsigned cap_dwords);
int      gpu_batch_end(struct gpu_batch *b);
unsigned gpu_batch_bytes(const struct gpu_batch *b);
int      gpu_batch_overflowed(const struct gpu_batch *b);

int gpu_fill_rect(struct gpu_batch *b, gpu_u64 dst_gfx, gpu_u32 pitch_bytes,
                  int x1, int y1, int x2, int y2, gpu_u32 color);
int gpu_copy_rect(struct gpu_batch *b,
                  gpu_u64 dst_gfx, gpu_u32 dst_pitch, int dx1, int dy1, int dx2, int dy2,
                  gpu_u64 src_gfx, gpu_u32 src_pitch, int sx1, int sy1);

/* THE MMIO BASE, AND WHY IT IS DECLARED HERE RATHER THAN IN EACH FILE.
 *
 * intel.c owns BAR0. gpuring.c dereferences it for every ring register. That
 * makes it a third thing which genuinely crosses the boundary, so it belongs
 * next to the other two.
 *
 * It is here specifically because it was WRONG while each file declared it
 * privately. intel.c exported `u32 intel_mmio(void) { return (u32)mmio; }`
 * and gpuring.c declared `gr_u32 intel_mmio(void);` and then widened the
 * result back to a pointer - so the >4 GiB BAR that intel.c assembles with
 * two deliberate 16-bit shifts was truncated at the accessor and re-widened
 * to a different, unrelated physical address. No warning fires: both sides
 * agreed, and both were wrong together. C does not check a prototype against
 * a definition in another translation unit, so ONE declaration both files
 * include is the only thing that makes that class impossible.
 *
 * intel_mmio() itself stays, unchanged and u32, because freestanding/
 * runtime_kernel.c binds it as a zl builtin and zl numbers are doubles. It is
 * a DIAGNOSTIC. Never dereference it. */
#if defined(ZL_64) || defined(__x86_64__)
typedef unsigned long long gpu_uptr;
#else
typedef unsigned int       gpu_uptr;
#endif
_Static_assert(sizeof(gpu_uptr) == sizeof(void *),
               "gpu_uptr must be able to hold a pointer on this target");

gpu_uptr intel_mmio_ptr(void);      /* BAR0, full width. Dereference THIS. */
gpu_uptr intel_aperture_ptr(void);  /* BAR2 (GMADR), full width.           */

#endif /* ZL_GPU_H */
