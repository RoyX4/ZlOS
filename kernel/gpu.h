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

#endif /* ZL_GPU_H */
