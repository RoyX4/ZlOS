# Oracle harnesses for driver verification

**Written 2026-09-01.** Deep prior-art research (64 web searches) plus direct measurement of this
machine. The question: what verification harness would most cheaply de-risk the driver work that
H2 depends on? Three candidates survived; three were killed for having fake oracles.

Context: `docs/evidence/status-audits/` records that `intel.c`'s write paths have **never
executed** (3 of 18 guarded functions), with 86 verified defects including a panel-damage class.
`REMAINING-WORK.md` H2 is "the ThinkPad lights its own panel." Everything below is aimed there.

---

## Measured on this machine (not assumed)

- **VT-d is live and on by default.** `/sys/class/iommu/` → `dmar0`, `dmar1`. 18 IOMMU groups.
  No `intel_iommu=` in `/proc/cmdline`.
- **The DMAR table has exactly two RMRRs** (LENOVO TP-N2W, 39-bit host address width):
  - `00:14.0` (PCH xHCI) — `0x6e750000–0x6e76ffff`, 128 KiB
  - `00:02.0` (iGPU) — `0x79000000–0x7d7fffff`, 72 MiB
  - Both are plain PCI endpoints on bus 0, below 4 GiB — *not* behind bridges.
- **The Thunderbolt xHCI `2d:00.0` and the NVMe `03:00.0` have no RMRR at all.** IOMMU group 16
  holds only `2d:00.0` and its bridge `06:02.0`.

That last fact is load-bearing: the firmware-reserved-region hole that would normally sink an
IOMMU-as-oracle plan **does not apply to the devices we would target**.

Existing harnesses (paths verified): `kernel/tests/host/intel_probe.c` (1158 lines, compiles
`intel.c` as a Linux userspace program against the live GPU BAR), `kernel/tests/host/gpu-dev.sh`,
and `kernel/tools/checks/test-real-xhci.sh` (VFIO-passes `2d:00.0`).

---

## 1. Phase-keyed modeset conformance oracle — build first

Turn i915's `i915_reg_rw` tracepoint into a golden modeset transcript, run our driver on the same
silicon, and compare.

**Do not diff the raw register write sequence.** i915 does atomic modeset with a completely
different control structure; sequence alignment is ill-posed and produces false positives faster
than they can be triaged. Instead diff **phase-boundary `{offset → last value written}` maps** —
order-insensitive and insensitive to firmware state.

Then add the oracle that actually decides correctness: **pipe CRC**, computed in hardware over the
pixels actually scanned out, exposed at `crtc-N/crc/control`. Same machine, same pipe, same test
pattern, so IGT's "CRCs are opaque across platforms" caveat does not bite.

Feasibility notes: `i915_reg_rw` is a bare `TRACE_EVENT_CONDITION` with no CONFIG guard (only
*request* tracing sits behind `CONFIG_DRM_I915_LOW_LEVEL_TRACEPOINTS`), and on Gen9 there is no DSB
and no DMC wakelock, so CPU-issued display writes really do all funnel through it.

**Killer risk — state it in any writeup.** DMC firmware and PSR hardware tracking mutate display
state with no traced write, which is exactly the power-well / link-state class most likely to bite
us. A green diff will *not* mean a correct driver. Capture the reference with
`i915.enable_dc=0 i915.enable_psr=0` and say plainly that the comparison is against a degraded i915.

**First step (one afternoon):** patch i915 with six `trace_marker` writes at declared phase
boundaries, capture `i915_reg_rw` across one `drmModeSetCrtc` with `trace-cmd`, reduce to a
per-phase offset→value map. This immediately answers whether the `intel_ddi_set_clock` `ddi*3+3`
bug is real.

Prior art (none of it is this): **SymDrive** (OSDI'12) compares driver implementations by I/O trace
but under symbolic execution with no device present; **i915 GVT `mmio_diff`** diffs vGPU vs host
MMIO as a *state snapshot*, not a sequence; **envytools `demmio`** decodes but does not align.

## 2. IOMMU as verdict — fold in alongside, zero new kernel code

Run our drivers with VT-d enforcing and treat every DMAR fault as a verdict: a fault is a
DMA-outside-mapped-RAM or use-after-unmap bug. Keep a map/unmap ledger that must balance at free.

This targets a bug class our own notes record hitting **five times** (a DMA buffer placed outside
guest RAM, or an address truncated to 32 bits).

Why this is not `dma-debug`: `dma-debug` is **purely software bookkeeping and never observes a byte
of device DMA**. A device that DMAs 8 bytes past a correctly-mapped buffer is invisible to it. The
in-flight `IOMMU_DEBUG_PAGEALLOC` sanitizer is a page_ext refcount, self-described as racy and
best-effort. Linux structurally cannot do fault-as-verdict — there a DMAR fault is a third-party
production incident, so the only affordable response is rate-limited logging. We wrote every
driver, so a fault is always our bug and halting is correct.

**First step, no new code:** boot zlOS as a VFIO guest through the existing
`kernel/tools/checks/test-real-xhci.sh` path. The *host's* IOMMU is already enforcing on `2d:00.0`;
any DMA outside the VFIO-mapped guest RAM raises a host DMAR fault naming source-id, direction and
faulting address. Writing our own VT-d driver (DMAR parse, root/context tables, second-level PTEs,
queued invalidation, fault ISR — six subsystems, comparable to the NVMe driver) is the *second*
increment, not the first.

**Killer risk:** enabling translation with an incomplete root table bricks boot — NVMe and display
both stop, no console. Stage the enable one device at a time with everything else in passthrough.
**And do the mutation test:** plant a known off-by-one and confirm the IOMMU catches it, or we have
built an oracle never proven to detect anything.

## 3. Device-model fidelity oracle — the one to write up publicly

Run the *same kernel image* under `qemu-system-x86_64 -device qemu-xhci` and on the metal, and diff
the transcripts. Every modern QEMU device fuzzer (Morphuzz, ViDeZZo, V-Shuttle, MundoFuzz,
HyperPill, Truman) uses crash/sanitizer oracles — **a device model that is stable but behaviourally
wrong is invisible to all of them.** That is exactly the bug class that ambushed the xHCI work when
it left QEMU. EmuFuzzer/PokeEMU established the real-hardware fidelity oracle for CPUs and nobody
carried it to devices; VDTest (ICSE'16) touched it once on toy devices and found the physical-device
oracle nearly tripled fault detection (64 vs 23).

Drop the VFIO framing — same binary both sides avoids IOMMU-group fights.

**The raw transcript is not the oracle.** Two real-hardware runs will not match each other (event
coalescing, PORTSC/link-training timing, NAK/retry counts, IMOD). Project onto the deterministic
subset: completion codes, slot/endpoint state machine as read back from the output device context,
delivered descriptor bytes, residual length, PORTSC change bits as a *set*.

**The one runnable check this rests on:** prove the differ catches our four known
real-hardware-only bugs before trusting it on unknowns. Every projection that kills nondeterminism
also risks projecting away the bug.

---

## Killed, with the reason

- **sched_ext runtime-conservation checker — the oracle is a tautology.** Idle time is attributed to
  `swapper/N`, which appears in `sched_switch` like any task, so sum-over-tasks = wall × CPUs *by
  construction of a correct timeline parser*. No scheduler behaviour, correct or buggy, can violate
  it. Separately, work-conservation is not a correctness property — `scx_pair`, `scx_layered`, EAS
  packing and core scheduling deliberately violate it. Upstream RV scheduler monitors are all
  control-flow automata with zero conservation properties, and `SCX_EXIT_ERROR_STALL` already ejects
  starving schedulers at a 30 s bound.
- **io_uring linearizability checker — vacuous.** `io_uring_enter(2)` states completions for
  unlinked ops are explicitly unordered, so *any* observed CQE order is a valid linearization; a
  kernel deliberately shuffling the CQ ring would return "linearizable" 100% of the time. What
  remains is two happens-before edge families checkable by an O(n) scan.
- **MMU/paging differential — done, and by something stronger.** `kvm-unit-tests` `x86/access.c`
  combinatorially varies pte/pde p/rw/user/a/d/nx/reserved, PSE, PKRU, CR0.WP, CR4.SMEP/PKE, EFER.NX
  across read/write/fetch/user and asserts on error code *and* resulting A/D bits — against a
  software reference model, which is strictly stronger than a three-way diff. Also "the real CPU is
  truth" collapses: A-bit setting is architecturally speculative (SDM 4.8) and TLB/paging-structure
  caching is explicitly implementation-defined.
- **Suspend/resume register round-trip** — real but narrow; the security subset was published in
  2015 (Bulygin/chipsec) and the rest is a labeling project.

## Weakest link in this document

The measurements in "Measured on this machine" and the kill-facts (RV monitor list, scx watchdog,
`x86/access.c` dimensions, `dma-debug` being bookkeeping-only, `i915_reg_rw`'s lack of a CONFIG
gate, GVT `mmio_diff` prior art) were reproduced directly. **Not** independently verified: the
CrashMonkey permuted-replay claim and the VDTest evaluation numbers (paywalled). Treat those two as
cited-but-unchecked.
