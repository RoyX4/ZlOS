/* ggttmap.h - the fixed GRAPHICS address map, in ONE place, checked by the
 * compiler. The counterpart to memmap.h, for the other address space.
 *
 * WHY THIS EXISTS, AND WHY memmap.h WAS NOT ENOUGH
 * -----------------------------------------------
 * memmap.h fixed the physical side and it worked: every multi-megabyte buffer
 * has a base and a span the compiler checks. The GGTT is a SECOND address
 * space, mapped by intel_ggtt_map(gfx_page, phys_addr), and it had no
 * equivalent - the bases were literals in the file that used them, with the
 * arithmetic in a trailing comment.
 *
 * It was wrong, in exactly the way memmap.h's own header predicts. gpucursor.c
 * declared the cursor image at graphics 0x04001000 and described it as "ring
 * gfx + one page", which is its START. It is 64x64x4 = 16 KiB, so it occupies
 * FOUR pages and ends at 0x04005000. gpuring.c then put the ring self-test
 * surface at 0x04002000, commented "ring gfx + 2 pages" - three pages inside
 * the cursor.
 *
 * The physical layout of the same two objects was CORRECT: GPU_ST_PHYS is
 * HI_GPU + 4096 + 16384, which clears the cursor's full 16 KiB. So one author
 * did the span arithmetic in one space and not the other, and the two
 * _Static_asserts that existed only checked each object against HI_BLUR - true
 * of both, and irrelevant to whether they overlapped each other.
 *
 * Nothing had failed yet only because gpu_ring_arm(1) and gpu_cursor_arm(1) are
 * both deliberately unwired. It was a bug waiting on the ignition.
 *
 * THE RULE: a base without a span is a comment, not a map. Every entry here
 * carries both, and every pair is asserted disjoint below.
 */
#ifndef ZL_GGTTMAP_H
#define ZL_GGTTMAP_H

/* 64 MiB into the graphics space. Chosen to sit clear of whatever the firmware
 * and the loader already mapped low; nothing here is negotiable with hardware,
 * it is ours to pick. */
#define GGTT_RING_GFX    0x04000000u
#define GGTT_RING_SPAN         4096u   /* one page: the command ring itself   */

/* The hardware cursor image. 64x64 ARGB = 16384 bytes = FOUR pages, which is
 * the number the old "+ one page" comment did not say. */
#define GGTT_CURSOR_GFX  (GGTT_RING_GFX + GGTT_RING_SPAN)
#define GGTT_CURSOR_SPAN      16384u

/* The ring self-test surface. 64x64x4, same size, and it must start after the
 * cursor ENDS rather than after the cursor BEGINS. */
#define GGTT_ST_GFX      (GGTT_CURSOR_GFX + GGTT_CURSOR_SPAN)
#define GGTT_ST_SPAN          16384u

/* Every pair, disjoint. These are the checks whose absence let the cursor and
 * the self-test share three pages. Written as explicit pairs rather than a
 * chain so that adding a fourth object forces you to add its two lines - a
 * chain of `>=` would silently accept an object inserted anywhere. */
_Static_assert(GGTT_CURSOR_GFX >= GGTT_RING_GFX + GGTT_RING_SPAN,
               "the cursor image overlaps the command ring in GGTT space");
_Static_assert(GGTT_ST_GFX >= GGTT_CURSOR_GFX + GGTT_CURSOR_SPAN,
               "the self-test surface overlaps the cursor image in GGTT space");
_Static_assert(GGTT_ST_GFX >= GGTT_RING_GFX + GGTT_RING_SPAN,
               "the self-test surface overlaps the command ring in GGTT space");

/* Every base is a page boundary, because intel_ggtt_map() takes a page number
 * and silently discards anything below bit 12. */
_Static_assert((GGTT_RING_GFX   & 0xFFFu) == 0, "GGTT_RING_GFX is not page aligned");
_Static_assert((GGTT_CURSOR_GFX & 0xFFFu) == 0, "GGTT_CURSOR_GFX is not page aligned");
_Static_assert((GGTT_ST_GFX     & 0xFFFu) == 0, "GGTT_ST_GFX is not page aligned");

#endif /* ZL_GGTTMAP_H */
