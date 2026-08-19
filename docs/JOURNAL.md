# Journal

What happened, and what the tree looked like when it happened. Appended
automatically by `tools/journal.sh` from the post-commit hook — newest first.

Git stores the diff; this stores the context around it: outstanding hazard
counts, whether the docs still agreed with the code, which files moved. That is
the part that makes a change legible later, and the part nobody writes down.

## 2026-08-20 — chore: untrack build outputs and machine-local state (38a9402)

`main` · 147 files changed, 18 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>147 file(s)</summary>

- `.cache/clangd/index/FILE.h.79AEFC497861DFC5.idx`
- `.cache/clangd/index/__FILE.h.3324F85E382E036A.idx`
- `.cache/clangd/index/__fpos64_t.h.23C43F442076149A.idx`
- `.cache/clangd/index/__fpos_t.h.ECFBE9EEDAAB6648.idx`
- `.cache/clangd/index/__locale_t.h.40EA38A2DCA58618.idx`
- `.cache/clangd/index/__mbstate_t.h.6C32CF66236653D7.idx`
- `.cache/clangd/index/__sigset_t.h.72131E8056C7E284.idx`
- `.cache/clangd/index/__sigval_t.h.666FFB429519E69C.idx`
- `.cache/clangd/index/__stdarg___gnuc_va_list.h.A74B6A78649ACF2D.idx`
- `.cache/clangd/index/__stdarg___va_copy.h.DEB43B9F037301EA.idx`
- `.cache/clangd/index/__stdarg_header_macro.h.C9C0096265808772.idx`
- `.cache/clangd/index/__stdarg_va_arg.h.CDB15C9657EEC720.idx`
- _…and 135 more_

</details>

## 2026-08-19 — docs: stop README describing an untracked build artifact (3f00366)

`main` · 1 file changed, 5 insertions(+), 5 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `README.md`

</details>

## 2026-08-19 — docs: rewrite README - add zlOS, fix stale build list and file counts (8890fdf)

`main` · 1 file changed, 198 insertions(+), 174 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `README.md`

</details>

## 2026-08-19 — feat(gpu): RENDER_SURFACE_STATE - the last render-engine blocker, out of Mesa (793763a)

`main` · 2 files changed, 205 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/gen_gpu_surface.py`
- `kernel/gpu_surface.inc`

</details>

## 2026-08-19 — feat(gpu): all three forcewake domains confirmed on silicon - RCS unblocked (85516b7)

`main` · 2 files changed, 57 insertions(+), 24 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/gpuring.c`
- `kernel/hosttest/gputest.c`

</details>

## 2026-08-19 — feat(gpu): the ring is engine-parameterised - RCS reachable, and refused until safe (a7259db)

`main` · 2 files changed, 100 insertions(+), 14 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/gpuring.c`
- `kernel/hosttest/gputest.c`

</details>

## 2026-08-19 — feat(gpu): read GPU-visible memory through the aperture - proven on the live desktop (1f42adf)

`main` · 3 files changed, 124 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/.gitignore`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/gpu_aperture.c`

</details>

## 2026-08-19 — docs(intel): "No" was the wrong word - split the bounded goal from the unbounded one (effdd14)

`main` · 2 files changed, 34 insertions(+), 6 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/docs/intel-graphics-stack.md`
- `kernel/docs/what-is-actually-impossible.md`

</details>

## 2026-08-19 — docs: record the GPU increment - what closed, what is open, two corrections (00fc205)

`main` · 2 files changed, 157 insertions(+), 94 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `.ultra/STATE.md`
- `TODO.md`

</details>

## 2026-08-19 — fix(gputest): the suite stopped BUILDING, and I claimed it passed anyway (0b28bb1)

`main` · 1 file changed, 7 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/hosttest/gputest.c`

</details>

## 2026-08-19 — docs(gpu): the fill threshold's number was right, its stated reason was not (ca3a6ad)

`main` · 1 file changed, 103 insertions(+), 6 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/gpuring.c`

</details>

## 2026-08-19 — fix(gpu): both GGTT callers now use the guarded range function (148c7a1)

`main` · 3 files changed, 25 insertions(+), 10 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/gpucursor.c`
- `kernel/gpuring.c`
- `kernel/intel.c`

</details>

## 2026-08-19 — docs(gpu): the sixth route to RENDER_SURFACE_STATE, closed by kernel config (8c43348)

`main` · 1 file changed, 30 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gen9-blend-pipeline.md`

</details>

## 2026-08-19 — feat(gpu): fold today's one-off hardware probes into --survey, and find RCS live (4ed13ba)

`main` · 1 file changed, 54 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/hosttest/gpu_ring.c`

</details>

## 2026-08-19 — docs: correct CLAUDE.md - four intel.c write paths are NOT behind lt_armed (b18194a)

`main` · 1 file changed, 21 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `CLAUDE.md`

</details>

## 2026-08-19 — fix(gpu): make the driver match the sequence that actually ran on silicon (9e350a2)

`main` · 1 file changed, 69 insertions(+), 16 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/gpuring.c`

</details>

## 2026-08-19 — fix(intel): the GGTT bounds check overflowed, and the pointer did not (eaa5492)

`main` · 1 file changed, 26 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/intel.c`

</details>

## 2026-08-19 — feat(gpu): PROVEN ON SILICON - zlOS can drive the Gen9 ring itself (ccb1415)

`main` · 1 file changed, 54 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gpu-driver.md`

</details>

## 2026-08-19 — feat(gpu): THE RING RUNS ON REAL SILICON - 16384/16384 pixels (90e182b)

`main` · 2 files changed, 37 insertions(+), 5 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/docs/gpu-driver.md`
- `kernel/hosttest/gpu_ring.c`

</details>

## 2026-08-19 — fix(gpu): the GGTT needs its own mapping - the harness was writing to its heap (ca3cb74)

`main` · 1 file changed, 87 insertions(+), 9 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/hosttest/gpu_ring.c`

</details>

## 2026-08-19 — feat(gate): check-himap.sh - the C side of the memory map finally has a checker (7491c11)

`main` · 3 files changed, 120 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `gates/land-gate.sh`
- `kernel/.gitignore`
- `kernel/check-himap.sh`

</details>

## 2026-08-19 — docs(gpu): the binding table is ONE dword - the gap is now a single struct (cad52e5)

`main` · 1 file changed, 32 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gen9-blend-pipeline.md`

</details>

## 2026-08-19 — feat(gpu): gpu_batch.inc - Mesa's Gen9 command stream, as dwords zlOS can emit (213d4da)

`main` · 3 files changed, 747 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/docs/gen9-blend-pipeline.md`
- `kernel/gen_gpu_batch.py`
- `kernel/gpu_batch.inc`

</details>

## 2026-08-19 — docs(gpu): reconcile the two shader byte counts - 48 vs 80, both right (ccdb729)

`main` · 1 file changed, 19 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gen9-blend-pipeline.md`

</details>

## 2026-08-19 — docs(gpu): link the two halves of the render-engine unblock (aab8957)

`main` · 1 file changed, 16 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gpu-driver.md`

</details>

## 2026-08-19 — docs(gpu): the Gen9 blended-draw pipeline, captured from the vendor driver (d991c4e)

`main` · 1 file changed, 261 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gen9-blend-pipeline.md`

</details>

## 2026-08-19 — feat(gpu): the Gen9 pixel shader, lifted out of Mesa - 80 bytes, colour patchable (a1b0172)

`main` · 2 files changed, 159 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/gen_gpu_shader.py`
- `kernel/gpu_shader.inc`

</details>

## 2026-08-19 — docs(gpu): where a Gen9 pixel shader comes from - the answer was the shader cache (5eeb6c0)

`main` · 7 files changed, 302 insertions(+), 10 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>7 file(s)</summary>

- `kernel/docs/gen9-shader-source.md`
- `kernel/docs/gpu-driver.md`
- `kernel/fb.c`
- `kernel/gpu.c`
- `kernel/gpu.h`
- `kernel/gpuring.c`
- `kernel/hosttest/gputest.c`

</details>

## 2026-08-19 — docs(gpu): SMP bands - two docs disagreed at the top of the driver's own order (8a6716e)

`main` · 2 files changed, 77 insertions(+), 5 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/docs/DECISIONS.md`
- `kernel/docs/gpu-driver.md`

</details>

## 2026-08-19 — docs(gpu): glGetProgramBinary works and gives the wrong thing (c4fd9be)

`main` · 1 file changed, 24 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gpu-driver.md`

</details>

## 2026-08-19 — feat(desktop): the four ranked NEXT-PROMPT items, and what a second model found in them (b9172ab)

`main` · 20 files changed, 1449 insertions(+), 101 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>20 file(s)</summary>

- `docs/STATE-OF-THE-PROJECT.md`
- `docs/shots/palette-after-one-palette.png`
- `docs/shots/palette-before-two-palettes.png`
- `freestanding/runtime_kernel.c`
- `kernel/docs/DECISIONS.md`
- `kernel/docs/NEXT-PROMPT.md`
- `kernel/docs/POINTER-PROMPT.md`
- `kernel/fb.c`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/intel_probe.c`
- `kernel/hosttest/palette.c`
- `kernel/hosttest/termwrap.c`
- _…and 8 more_

</details>

## 2026-08-19 — docs(gpu): milestone 3 done - and why the ignition is deliberately not wired (02ad522)

`main` · 1 file changed, 33 insertions(+), 8 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gpu-driver.md`

</details>

## 2026-08-19 — feat(gpu): the compositor calls the driver - hardware cursor, sprite as fallback (ca4c364)

`main` · 2 files changed, 31 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/hosttest/hoststubs.c`
- `kernel/wm.c`

</details>

## 2026-08-19 — feat(gpu): gpu_planes - the plane registers finally have a witness (857e2fc)

`main` · 3 files changed, 181 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/docs/gpu-driver.md`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/gpu_planes.c`

</details>

## 2026-08-19 — fix(gate): gpu_ring returned 2 for 'not root' and would have reddened the gate (b245224)

`main` · 1 file changed, 14 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/hosttest/gpu_ring.c`

</details>

## 2026-08-19 — docs(gpu): the driver's state - what is proven, what is inert, what blocks (ff8d66c)

`main` · 1 file changed, 103 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gpu-driver.md`

</details>

## 2026-08-19 — feat(gpu): the hardware cursor image - the third wired-but-unfired path today (c27ee44)

`main` · 3 files changed, 323 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/SOURCES`
- `kernel/gpucursor.c`
- `kernel/hosttest/gputest.c`

</details>

## 2026-08-19 — feat(gpu): kernel/gpuring.c - zlOS has a ring, and a map region to put it in (f9f3c47)

`main` · 5 files changed, 333 insertions(+), 3 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>5 file(s)</summary>

- `kernel/SOURCES`
- `kernel/gpuring.c`
- `kernel/hosttest/gputest.c`
- `kernel/i2c_hid.c`
- `kernel/memmap.h`

</details>

## 2026-08-19 — docs(gpu): the decision brief - SMP first, then the shader question, then RCS (67168b8)

`main` · 2 files changed, 102 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/docs/gpu-blitter.md`
- `kernel/docs/gpu-next.md`

</details>

## 2026-08-19 — perf(gpu): the blitter was the wrong ENGINE - blends are 48x, fills are 1x (97ecf04)

`main` · 1 file changed, 60 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gpu-blitter.md`

</details>

## 2026-08-19 — docs: correct my own claim - one gate DOES run two cores (a8377a8)

`main` · 2 files changed, 9 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/GUARDS-THAT-DID-NOT-GUARD.md`
- `docs/SESSION-CONTINUE.md`

</details>

## 2026-08-19 — perf(gpu): the blitter does not beat fb.c - the CPU column was never fb.c (81f8e11)

`main` · 2 files changed, 105 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/docs/gpu-blitter.md`
- `kernel/hosttest/gpu_blt.c`

</details>

## 2026-08-19 — perf(gpu): the present path measured - and it refutes the last commit (e09dc3f)

`main` · 2 files changed, 199 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/docs/gpu-blitter.md`
- `kernel/hosttest/gpu_blt.c`

</details>

## 2026-08-19 — feat(gpu): the blitter COPIES - the present path, verified on silicon (f2c8bd3)

`main` · 3 files changed, 229 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/gpu.c`
- `kernel/hosttest/gpu_blt.c`
- `kernel/hosttest/gputest.c`

</details>

## 2026-08-19 — docs(gpu): how to run the ring experiment, and what each outcome means (2b53ec3)

`main` · 1 file changed, 64 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gpu-blitter.md`

</details>

## 2026-08-19 — feat(gpu): the ring experiment, ready to run - and its recovery is proven (f8124a3)

`main` · 4 files changed, 525 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>4 file(s)</summary>

- `kernel/.gitignore`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/gpu-ring-run.sh`
- `kernel/hosttest/gpu_ring.c`

</details>

## 2026-08-19 — docs: NEXT-PROMPT, and a correction I owe - term_draw already clips (55e0226)

`main` · 3 files changed, 190 insertions(+), 6 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/HANDOFF.md`
- `kernel/docs/NEXT-PROMPT.md`
- `kernel/docs/look-and-speed.md`

</details>

## 2026-08-19 — docs(gpu): the BCS register map, read off the running GPU (ca5ae1d)

`main` · 1 file changed, 52 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gpu-blitter.md`

</details>

## 2026-08-19 — feat(gpu): the kernel emits the blitter stream - gpu.c, verified on silicon (549d178)

`main` · 6 files changed, 443 insertions(+), 28 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>6 file(s)</summary>

- `kernel/SOURCES`
- `kernel/docs/gpu-blitter.md`
- `kernel/gpu.c`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/gpu_blt.c`
- `kernel/hosttest/gputest.c`

</details>

## 2026-08-19 — docs(gpu): re-run the blitter sweep quiet - the top of the range was load (4e93dbb)

`main` · 1 file changed, 32 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/gpu-blitter.md`

</details>

## 2026-08-19 — fix(gate): exit 77 means SKIP, and the harness loop was calling it a failure (e1eb44a)

`main` · 2 files changed, 38 insertions(+), 7 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `gates/land-gate.sh`
- `kernel/docs/gpu-blitter.md`

</details>

## 2026-08-19 — feat(gpu): the Intel blitter draws for zlOS - first GPU work in the project (1bc75e4)

`main` · 3 files changed, 864 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/docs/gpu-blitter.md`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/gpu_blt.c`

</details>

## 2026-08-19 — feat(desktop): settle the look against the northstar, and count what a frame misses (472b4ee)

`main` · 15 files changed, 1668 insertions(+), 68 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>15 file(s)</summary>

- `.gitignore`
- `docs/shots/after-merge-help.png`
- `docs/shots/before-merge-help.png`
- `freestanding/runtime_kernel.c`
- `kernel/HANDOFF.md`
- `kernel/docs/DECISIONS.md`
- `kernel/docs/POINTER-PROMPT.md`
- `kernel/docs/desktop-v10-plan.md`
- `kernel/docs/look-and-speed.md`
- `kernel/docs/visual-speed-northstar.md`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/walltest.c`
- _…and 3 more_

</details>

## 2026-08-19 — fix(efi): the -Werror truncation guard was inert - 33 casts behind it (79836d1)

`main` · 16 files changed, 671 insertions(+), 82 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>16 file(s)</summary>

- `CLAUDE.md`
- `docs/GUARDS-THAT-DID-NOT-GUARD.md`
- `docs/SESSION-CONTINUE.md`
- `docs/STATE-OF-THE-PROJECT.md`
- `freestanding/runtime_kernel.c`
- `kernel/browser.c`
- `kernel/buildefi.sh`
- `kernel/console.c`
- `kernel/fb.c`
- `kernel/http.c`
- `kernel/i2c_hid.c`
- `kernel/intel.c`
- _…and 4 more_

</details>

## 2026-08-19 — docs: SESSION-CONTINUE - the hand-off from the merge session (0a827dd)

`main` · 1 file changed, 101 insertions(+)

| | |
|---|---|
| EFI truncation sites | 11 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `docs/SESSION-CONTINUE.md`

</details>

## 2026-08-19 — docs: the look-and-speed brief, and version the northstar it measures against (cfed3b6)

`main` · 2 files changed, 701 insertions(+)

| | |
|---|---|
| EFI truncation sites | 11 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/design/zlOS-design-northstar.html`
- `kernel/docs/LOOK-AND-SPEED-PROMPT.md`

</details>

## 2026-08-19 — fix(check-memmap): discover the addresses instead of iterating a list (274b8f6)

`main` · 1 file changed, 33 insertions(+)

| | |
|---|---|
| EFI truncation sites | 11 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/check-memmap.sh`

</details>

## 2026-08-19 — fix: drop examples/Zaccoding.zl - 14 bytes of replacement characters (8ea46ed)

`main` · 1 file changed, 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 11 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `examples/Zaccoding.zl`

</details>

## 2026-08-19 — chore: regenerated journal/TODO (post-commit hook) (2c2092f)

`main` · 1 file changed, 17 insertions(+)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |
## 2026-08-19 — docs: the journal entry for the commit that added the journal (b3f5fd2)

`ci/gates-and-agent-brief` · 1 file changed, 2 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 34 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-19 — chore: regenerated journal/TODO (post-commit hook) (3e6497c)

`main` · 1 file changed, 18 insertions(+)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-19 — chore: the journal and TODO the post-commit hook regenerated (0a4bc8f)

`main` · 2 files changed, 24 insertions(+), 16 deletions(-)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`

</details>

## 2026-08-19 — Merge branch 'ci/gates-and-agent-brief' (18f34e3)

`main` · 29 files changed, 2733 insertions(+), 73 deletions(-)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `.gitignore`
- `kernel/hosttest/wmshot.c`

</details>


## 2026-08-19 — ci: write down what happens, keep the docs true, regenerate the TODO (cb17faf)

`ci/gates-and-agent-brief` · 9 files changed, 719 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 34 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>9 file(s)</summary>

- `.github/workflows/docs.yml`
- `TODO.md`
- `docs/JOURNAL.md`
- `tools/doc-check-ignore.txt`
- `tools/doc-check.sh`
- `tools/doc-claims.txt`
- `tools/install-hooks.sh`
- `tools/journal.sh`
- `tools/todo.sh`

</details>

## 2026-08-18 — ci: preflight.sh and a pre-push hook, since main cannot be protected (46f4bd2)

`ci/gates-and-agent-brief` · 2 files changed, 132 insertions(+)

| | |
|---|---|
| EFI truncation sites | 34 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>2 file(s)</summary>

- `tools/install-hooks.sh`
- `tools/preflight.sh`

</details>

