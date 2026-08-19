# GGTT span collision — the hardware cursor and the GPU self-test surface overlap by 3 of 4 pages

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366` · arithmetic re-derived by hand**

Found independently by **two** fleet driver agents (`gpu-ring` and `intel-ungated`),
both citing `kernel/gpuring.c:493`. Verified here. **Confirmed, and the graphics-address
half is a real overlap.**

---

## The two allocations

```c
/* kernel/gpucursor.c:58-61 */
#define GPU_CURSOR_DIM   64u
#define GPU_CURSOR_BYTES (GPU_CURSOR_DIM * GPU_CURSOR_DIM * 4u)   /* 16384 = 4 pages */
#define GPU_CURSOR_PHYS  ((gc_u64)HI_GPU + 4096u)                 /* after the ring */
#define GPU_CURSOR_GFX   0x04001000u                              /* ring gfx + one page */
```

```c
/* kernel/gpuring.c:488-493 */
#define GPU_ST_W     64u
#define GPU_ST_H     64u
#define GPU_ST_PITCH (GPU_ST_W * 4u)
#define GPU_ST_BYTES (GPU_ST_PITCH * GPU_ST_H)                    /* 16384 = 4 pages */
#define GPU_ST_PHYS  ((gr_u64)HI_GPU + 4096u + 16384u)            /* after ring + cursor */
#define GPU_ST_GFX   0x04002000u                                  /* ring gfx + 2 pages */
```

## The arithmetic

Both objects are 64 × 64 × 4 = **16,384 bytes = 4 pages**.

| | start | end | pages |
|---|---|---|---|
| cursor **GFX** | `0x04001000` | `0x04005000` | 4 |
| self-test **GFX** | `0x04002000` | `0x04006000` | 4 |

**Overlap: `0x04002000`–`0x04005000` — 3 of 4 pages.**

The physical side is fine, and that is the part that makes this easy to miss:

| | start | end |
|---|---|---|
| cursor **PHYS** | `HI_GPU + 4096` | `HI_GPU + 20480` |
| self-test **PHYS** | `HI_GPU + 20480` | `HI_GPU + 36864` |

`GPU_ST_PHYS` is written as `HI_GPU + 4096 + 16384`, which correctly clears the
cursor's full 16 KiB. **The physical layout accounts for the cursor's real size; the
graphics layout does not.**

## The defect is in one comment and one constant

`/* ring gfx + 2 pages */` is wrong. The cursor occupies **four** pages, so the
self-test surface must start at ring gfx + 5 pages:

```c
#define GPU_ST_GFX   0x04005000u    /* ring (1 page) + cursor (4 pages) */
```

The same off-by-three exists in the comment on `GPU_CURSOR_GFX` — `/* ring gfx + one
page */` describes its *start*, correctly, but nothing anywhere records that it *ends*
four pages later, which is the fact `GPU_ST_GFX` needed.

## Why both `_Static_assert`s pass

```c
/* gpucursor.c:63 */
_Static_assert(GPU_CURSOR_PHYS + GPU_CURSOR_BYTES <= (gc_u64)HI_BLUR, …);
/* gpuring.c:497 */
_Static_assert(GPU_ST_PHYS + GPU_ST_BYTES <= (gr_u64)HI_BLUR,
               "the self-test surface runs past HI_GPU into the blur arena");
```

Each asserts its own object fits **below `HI_BLUR`**. Neither asserts anything about
the *other* object, and neither mentions graphics addresses at all. Both are true and
both are irrelevant to this bug.

This is the identical failure the repo already documented for the physical memory map —
*"the three `_Static_assert`s check base ordering only, not span containment"* — now
recurring in **GGTT space**, where there is no `memmap.h` equivalent and no checker at
all.

## Consequence

`gpu_selftest()` writes its 64×64 pattern through GGTT entries that also back the
hardware cursor image. Whichever ran last wins for the shared 3 pages: the self-test
either corrupts the cursor, or validates against pixels the cursor wrote and reports a
result that says nothing about the ring.

It has not bitten yet for the reason everything in this subsystem has not bitten yet —
`gpu_ring_arm(1)` and `gpu_cursor_arm(1)` are both deliberately unwired
(`.ultra/STATE.md` §5). This is a bug waiting on the ignition, which is exactly the
change that is next on the board.

## The fix, and the check that should come with it

1. Set `GPU_ST_GFX` to `0x04005000`.
2. Add the assert that would have caught it, and watch it go red at `0x04002000` first:

```c
_Static_assert(GPU_ST_GFX >= GPU_CURSOR_GFX + GPU_CURSOR_BYTES ||
               GPU_CURSOR_GFX >= GPU_ST_GFX + GPU_ST_BYTES,
               "the self-test surface and the cursor image overlap in GGTT space");
```

3. Better: give GGTT space the same treatment `kernel/memmap.h` gave physical space —
   one header owning every graphics-address base **with its span**, so the next object
   added is checked rather than commented.
