/* gpu.c - the blitter command stream, as zlOS will emit it.
 *
 * WHAT THIS IS AND IS NOT
 * -----------------------
 * This file BUILDS BLITTER COMMANDS. It does not submit them, does not touch a
 * ring register, and does not touch MMIO at all. That separation is the whole
 * design, and it is deliberate:
 *
 *   - The command ENCODING is the part most likely to be wrong and the part
 *     hardest to debug inside a kernel with no debugger. A wrong dword is a
 *     hang with no diagnostic.
 *   - The command encoding is also the part that can be proven CHEAPLY, on the
 *     real GPU, without owning it - kernel/hosttest/gpu_blt.c submits these
 *     exact dwords through i915's render node and reads the pixels back.
 *
 * So `gpu_blt.c` uses THIS FILE to build its batch. The hardware test and the
 * kernel are not two implementations that agree by inspection; they are one
 * implementation, and the thing verified on silicon is the thing that ships.
 * `hosttest/gputest.c` then pins the bytes so a later edit cannot drift from
 * what was proven.
 *
 * WHAT IS STILL MISSING, stated plainly: submission. zlOS has no ring. Getting
 * the BCS ring running (RING_TAIL/HEAD/START/CTL at engine base 0x22000, and
 * the execlist-vs-legacy-ringbuffer question Gen8+ raises) needs the hardware
 * with i915 detached, which blanks the screen and is a separate piece of work.
 * `kernel/docs/gpu-blitter.md` has the order.
 *
 * FREESTANDING. No libc, no headers, integer only - the same rules as every
 * other .c in this directory, so the host harness and the kernel compile the
 * identical text.
 */

typedef unsigned int       gpu_u32;
typedef unsigned long long gpu_u64;

/* ---- XY_COLOR_BLT, the seven dwords ---------------------------------------
 *
 * Verified on 8086:9B41 (CometLake-U GT2, Gen9.5) on 2026-08-19: this exact
 * encoding filled 270000/270000 pixels of a rectangle and clobbered nothing
 * outside it. See kernel/docs/gpu-blitter.md.
 *
 *   DW0  client | opcode | write-alpha | write-rgb | (dwords - 2)
 *   DW1  BR13:  raster op | colour depth | destination pitch IN BYTES
 *   DW2  top-left      y in 31:16, x in 15:0
 *   DW3  bottom-right  y in 31:16, x in 15:0, EXCLUSIVE
 *   DW4  destination graphics address, low 32
 *   DW5  destination graphics address, high 32
 *   DW6  the colour
 *
 * BLT_WRITE_RGB is not optional decoration. Without it the hardware accepts
 * the batch, executes it, raises no error and writes NOTHING - measured, and
 * it is what hosttest/gpu_blt.c --negative plants on purpose to prove the
 * checker can still fail.
 */
#define GPU_BLT_CLIENT       (2u << 29)
#define GPU_XY_COLOR_BLT_OP  (0x50u << 22)
#define GPU_BLT_WRITE_ALPHA  (1u << 21)
#define GPU_BLT_WRITE_RGB    (1u << 20)
#define GPU_XY_COLOR_BLT_DW  7u                 /* total dwords in the command */

#define GPU_BR13_ROP_PATCOPY (0xF0u << 16)      /* dst = pattern: a solid fill */
#define GPU_BR13_DEPTH_32BPP (3u << 24)

#define GPU_MI_BATCH_BUFFER_END 0x05000000u

/* BR13 carries the pitch in a 16-BIT field, in bytes. A 32bpp surface wider
 * than 16383 pixels cannot be described at all. fb.c's back buffer is 3840
 * wide at most today (15360 bytes, inside the field), but a caller that grows
 * past it must be refused rather than silently truncated - a wrapped pitch
 * scribbles diagonally across memory and looks like anything but a pitch bug. */
#define GPU_PITCH_MAX 0xFFFFu

/* The rectangle coordinates are 16-bit too, and the hardware treats them as
 * unsigned. A negative left edge from a caller doing its own clipping would
 * become a huge positive one. */
#define GPU_COORD_MAX 0xFFFFu

/* A linear batch under construction. `at` is the next free dword; `cap` is how
 * many the buffer holds. Nothing here allocates - the caller owns the memory,
 * because in the kernel it has to be memory the GGTT maps and in the harness it
 * is a GEM buffer. */
struct gpu_batch {
    gpu_u32 *dw;
    unsigned cap;
    unsigned at;
    int      overflow;      /* sticky: set once, never silently cleared */
};

void gpu_batch_init(struct gpu_batch *b, gpu_u32 *storage, unsigned cap_dwords)
{
    b->dw = storage;
    b->cap = cap_dwords;
    b->at = 0;
    b->overflow = 0;
}

/* Every append goes through here so the bounds check exists exactly once.
 * Returns 0 and sets the sticky overflow flag rather than writing past the
 * end - a batch buffer overrun in the kernel is a GPU executing whatever
 * followed it in memory. */
static int gpu_push(struct gpu_batch *b, gpu_u32 v)
{
    if (b->overflow) return 0;
    if (b->at >= b->cap) { b->overflow = 1; return 0; }
    b->dw[b->at++] = v;
    return 1;
}

/* Add one solid-colour rectangle fill.
 *
 * `dst_gfx` is a GRAPHICS address - an offset the engine resolves through a
 * page table, NOT a physical address. In zlOS that means a page mapped with
 * intel_ggtt_map() first; in the harness it is the softpinned ppGTT address.
 * Getting that wrong is silent: the blit succeeds and writes somewhere else.
 *
 * x2/y2 are EXCLUSIVE, matching every other rectangle API in this tree
 * (fb_fill_px takes w/h; the conversion belongs at the call site, once).
 *
 * Returns 1 if the command was written whole, 0 if it was rejected or did not
 * fit. A partial command is never left behind: the bounds are checked before
 * the first push.
 */
int gpu_fill_rect(struct gpu_batch *b, gpu_u64 dst_gfx, gpu_u32 pitch_bytes,
                  int x1, int y1, int x2, int y2, gpu_u32 color)
{
    if (!b || b->overflow) return 0;
    if (pitch_bytes == 0 || pitch_bytes > GPU_PITCH_MAX) return 0;
    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0) return 0;
    if (x2 <= x1 || y2 <= y1) return 0;              /* empty or inverted */
    if (x2 > (int)GPU_COORD_MAX || y2 > (int)GPU_COORD_MAX) return 0;
    /* Check the whole command fits BEFORE emitting any of it. */
    if (b->at + GPU_XY_COLOR_BLT_DW > b->cap) { b->overflow = 1; return 0; }

    gpu_push(b, GPU_BLT_CLIENT | GPU_XY_COLOR_BLT_OP |
                GPU_BLT_WRITE_ALPHA | GPU_BLT_WRITE_RGB |
                (GPU_XY_COLOR_BLT_DW - 2u));
    gpu_push(b, GPU_BR13_ROP_PATCOPY | GPU_BR13_DEPTH_32BPP |
                (pitch_bytes & 0xFFFFu));
    gpu_push(b, ((gpu_u32)y1 << 16) | ((gpu_u32)x1 & 0xFFFFu));
    gpu_push(b, ((gpu_u32)y2 << 16) | ((gpu_u32)x2 & 0xFFFFu));
    gpu_push(b, (gpu_u32)(dst_gfx & 0xFFFFFFFFu));
    gpu_push(b, (gpu_u32)(dst_gfx >> 32));
    gpu_push(b, color);
    return !b->overflow;
}

/* Close the batch. The engine keeps parsing until it sees this, so a batch
 * submitted without it runs off into whatever follows.
 *
 * The length handed to the hardware must be a multiple of 8 bytes, so an odd
 * dword count is padded - with a SECOND end, not a NOOP, so a parser that
 * somehow arrives at the pad still stops. */
int gpu_batch_end(struct gpu_batch *b)
{
    if (!b || b->overflow) return 0;
    if (!gpu_push(b, GPU_MI_BATCH_BUFFER_END)) return 0;
    if (b->at & 1u) { if (!gpu_push(b, GPU_MI_BATCH_BUFFER_END)) return 0; }
    return 1;
}

/* Bytes to hand to the submission path. Valid only after gpu_batch_end. */
unsigned gpu_batch_bytes(const struct gpu_batch *b)
{
    return b->overflow ? 0u : b->at * 4u;
}

int gpu_batch_overflowed(const struct gpu_batch *b) { return b->overflow; }
