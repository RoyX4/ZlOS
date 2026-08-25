# The drivers — what 25 agents found

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366`**

Wave 2: 10 lenses on the Intel GPU and display, 15 on every other driver plus new-driver
design.

**Items marked ✓ were re-derived by hand.** Everything else is a lead.

---

## The single most important item

**[The render engine is not blocked.](UNBLOCKED-render-engine.md) ✓**
`RENDER_SURFACE_STATE` landed at `793763a`, three commits before `HEAD`, as
`kernel/src/drivers/display/assets/gpu_surface.inc`. `.ultra/STATE.md` and four other docs still call it the
blocker and say it needs a PRM this machine does not have. The 48×/96× win is available
and the project is routed away from it.

---

## Before arming anything on hardware

These three are in the ignition path and all are ✓ verified.

### ✓ 1. `intel_mmio()` truncates a 64-bit BAR
[Full write-up.](CRITICAL-gpuring-bar-truncation.md) `intel.c:435` returns `u32`;
`gpuring.c:249,255` widen it back. `intel.c` builds the >4 GiB address carefully at
`:423` and the accessor discards half of it. `hosttest/gputest.c:33` stubs it to `0`, so
116 checks cannot see it. **Blocks `gpu_ring_arm(1)` and the `G` command.**

### ✓ 2. Cursor and GPU self-test overlap 3 of 4 GGTT pages
[Full write-up.](VERIFIED-ggtt-span-collision.md) Physical layouts are asserted and do
not overlap; **graphics** addresses are not asserted and do. Found independently by two
lenses. Fires on first arming.

### ✓ 3. The display driver's failure reporting is inverted
[Full write-up.](CRITICAL-display-failure-reporting-inverted.md) A black panel reports
success (`intel.c:2555`); a backlight step failing reports total modeset failure
(`intel.c:3882`).

---

## The safety boundary that does not exist

**[`lt_armed` is armed by a keypress.](PANEL-POWER-IS-REACHABLE.md) ✓**
`kernel.zl:1489` `P` → `panel_up` → `intel.c:4323`. Five doc sites in three files say
nothing outside `hosttest/` arms it. The T12 and AUX discipline on that path is sound —
the code is careful. The documentation is not.

Related, agent-reported and **not** hand-verified: `CLAUDE.md`'s ungated-write table
lists four ungated MMIO writers where a function-by-function scan reportedly finds at
least eleven, naming `intel_backlight_set` (`:995`), `intel_flip` (`:1029`),
`intel_ggtt_map` (`:564`) and `intel_ggtt_map_range`.

---

## SMP bands — re-rank before doing

**[The APs have no IDT.](CRITICAL-smp-bands-have-no-idt.md) ✓** Any fault in a band job
triple-faults the laptop; the barrier at `smp.c:177` is unbounded. The 1.78× is real,
but the honest cost is 1.78× **plus** an AP IDT **plus** a bounded barrier.

Also ✓: `smp_go()` is **not** "reachable solely from the old text shell" — `term.c:190`
and `:204` map the typed words `smp` and `cores`, `kernel.zl:1874` calls it, and
`probe-smp.py:143` literally types `"smp\n"` into the compositor. Six documents say
otherwise.

---

## What the briefs got wrong, corrected by agents

This wave's briefs were built from `kernel/docs/driver-build-order.md`, which is
excellent and **systematically stale**. Every correction below came from an agent
reading code instead of accepting the brief.

| brief said | actual |
|---|---|
| `acpi_find_table()` is `static` — "highest leverage-per-line in the survey" | ✓ already external since `06b1a0f`. The *useful* remaining fact: it has external linkage, **zero external callers, and no prototype in any of the 12 headers** |
| `write_msr()` does not exist anywhere | ✓ exists at `cpu.c:65`. What's missing is everything around it: **zero call sites**, no `"memory"` clobber, no `cpu_has_msr()` guard |
| `pci.c` has no capability walk or D3→D0 | ✓ `pci_find_cap`, `pci_power_on`, `pci_power_state` all exist and `pci_enable` wakes before enabling. **Correct part:** `pci.c:118` is still a flat `bus 0..3` with no bridge recursion — measured, 8 of 26 functions on this laptop are invisible, **including a second xHCI** |
| `event_wait()` matches on TYPE only, `xhci.c:604` | `event_wait` **does not exist** — replaced by `xfer_wait` in `f334a3a`, which *does* filter slot+dci at `:640-642`. Five hits repo-wide, all comments or docs |
| `reset_endpoint` at `:940`, bulk call at `:1848` | substance **correct**, both line numbers stale — actually `:978` and `:2127`. The defect is live: `:989` hardcodes `EP0_RING(slot)` regardless of `dci` |
| blur/HID physical collision is live | ✓ fixed by `kernel/src/arch/x86/memmap.h`; `HI_HID` is *below* the blur arena |
| forcewake lives in `intel.c`; three domains implemented | `intel.c` has **zero** forcewake code. Two domains in `gpuring.c` (RENDER, BLITTER) — and that is a design choice, not a defect ([why](VERIFICATION-LOG.md)) |
| `gpu_planes.c` is the plane driver | that file exists only as a **host witness** under `hosttest/`; the driver is `intel.c:4702-4804`. And "nothing uses a blended plane" is contradicted by the tool's own output — `CUR_CTL` reads `0x04000027`, bit 5 = ARGB. **The cursor plane is a hardware-blended plane** |

`driver-build-order.md` needs the audit banner the other twenty-one planning documents
carry. Details in [`GROUND-TRUTH-CORRECTIONS.md`](GROUND-TRUTH-CORRECTIONS.md).

---

## Confirmed live defects in other drivers

| # | file:line | defect |
|---|---|---|
| 1 | `xhci.c:989` | `reset_endpoint` points **any** endpoint at EP0's ring and wipes EP0's producer state; reachable with a bulk DCI from `:2127` |
| 2 | `xhci.c:411` ✓ | `xhci_ram_ok()` zeroes the live DCBAA scratchpad pointer, and the zl builtin `usb_ram` reaches it any time ([here](CRITICAL-gates-that-cannot-fail.md)) |
| 3 | `xhci.c:1911` | HID interrupt endpoints have **no halt recovery** — one stall kills keyboard or pointer until reboot |
| 4 | `xhci.c:2167` | the CSW tag is written, documented as the reply matcher, and never checked |
| 5 | `nvme.c:99` | `IO_FLUSH` is defined and `grep` finds exactly one occurrence — the definition. The drive is in write-back mode |
| 6 | `pci.c:118` | flat `bus 0..3` scan; a second xHCI is invisible |
| 7 | `i2c_hid.c` | 16 of 18 exported functions reachable **only** from the `?` diagnostic key; nothing in input/wm/desktop calls any of it, so *"even a perfect driver moves no cursor"* |

## New-driver design — where the real blocker is

The `https-scope` and `new-drivers` lenses converge on the same answer, and it is not
what the roadmap says:

> **Entropy is the blocker, and it is a policy decision.** `cpu.c:144` — RDRAND is
> detected and reachable. `crypto.c` is 543 lines of vector-tested SHA/HMAC/AES/PBKDF2
> with no fuel.

And: **HTTPS is not unbuilt.** It reportedly exists on `desktop/browser-next`, in that
branch's `SOURCES`, reachable from this repo's own object store. Costing it as new work
would be wrong. *(Agent-reported; not verified here.)*

The two genuinely new, fully host-verifiable drivers remain `entropy.c`
(RDSEED → HMAC-DRBG-SHA256 on the existing `hmac_sha256`, testable against CAVP vectors
with no hardware) and `gpt.c` (CRC32 poly `0xEDB88320`, golden values already measured).

> **CORRECTION 2026-08-19, verified against the object store — and it is this
> board's own headline pattern happening inside this board's own recommendation.**
> `entropy.c` is **not** unwritten. It exists on `desktop/browser-next`, 160 lines,
> and unlike `crypto.c` it is **in that branch's `SOURCES` (line 85)** alongside
> `crypto.c`, `rsa.c`, `ecdsa.c`, `x509.c` and `tls.c` — so it is linked, not
> stranded.
>
> ```
> $ git show desktop/browser-next:kernel/src/net/entropy.c | wc -l
> 160
> $ git show desktop/browser-next:kernel/SOURCES | grep -n entropy
> 85:entropy.c
> ```
>
> This is the second instance of `UNBLOCKED-render-engine.md`'s exact finding —
> work that landed while the roadmap still costs it as unstarted. The lesson
> generalises: **before costing anything here as new, check every ref, not just
> `main`.** `STATE-OF-THE-PROJECT.md` §7.1 learned this once already, for
> `crypto.c` in `refs/wip`.
>
> **The technical recommendation survives, narrowed.** What exists is *not* the
> DRBG this board proposes: `entropy.c:81-83` returns `RND_HW` on a raw
> `cpu_rdrand32()`, with a TSC/RTC jitter hash as `RND_WEAK` fallback and
> `RND_NONE` when nothing varies. There is no HMAC-DRBG and no CAVP vector test.
> So the work is an **upgrade over 160 existing lines**, not a greenfield driver —
> re-cost it from 160, not from 0. What the existing file already gets right, and
> what a rewrite must keep: it reports its own quality tier and requires the
> caller to refuse on `RND_NONE`.
>
> `gpt.c` was re-checked and is genuinely absent from every ref. That half stands.

---

## Suggested order

1. Fix `intel_mmio()`. Everything on the GPU waits behind it.
2. Fix the GGTT overlap and add the graphics-space assert.
3. Correct `.ultra/STATE.md` on `RENDER_SURFACE_STATE` — then **write the surface-state
   builder**, host-tested against the captured instance, before touching hardware.
4. Fix `intel.c:2555` and the `MS_STEP_SOFT` verdict. Both are one line and both are
   about being able to *believe* the next hardware run.
5. Bound the SMP barrier and give the APs an IDT — *then* consider bands.
6. `xhci.c:989` `reset_endpoint`, and `xhci_ram_ok`'s DCBAA write.
