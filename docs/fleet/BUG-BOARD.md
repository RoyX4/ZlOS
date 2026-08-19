# The bug hunt — 10 class sweeps, each finding adversarially refuted

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366`**

Wave 3 differed from the other three. It swept by **bug class across the whole tree**
rather than by file, and every candidate then went to an independent agent whose job was
to **refute** it, instructed to default to *refuted* when uncertain.

The ten classes: LLP64 truncation · DMA/physical bounds · gates that cannot fail ·
orphaned code · buffer overruns · integer overflow · silent failures · concurrency ·
wedged state machines · trust boundaries.

---

## The refutation stage earned its cost three times over

**It killed my own errors.** The refuters read the fleet documents already committed in
this directory and broke two of them. Both are corrected in place, with the wrong version
named rather than overwritten:

- `VERIFIED-ggtt-span-collision.md` claimed the self-test could "validate against pixels
  the cursor wrote." **False** — `gpuring.c:526` verifies via a physical address that does
  not alias the cursor. The self-test can fail spuriously and can **never pass falsely**.
- `CRITICAL-ci-truncation-gate-is-blind.md` implied the truncation class is unprotected in
  the 44 unscanned files. **Too strong** — `buildefi.sh:40-44` applies four `-Werror=`
  flags to every TU, so the *compiler* catches pointer↔integer casts even where the
  *gate* never counts them.

**It caught re-discoveries.** Three candidates were flagged as duplicates of findings
already committed here — the pipeline noticing itself.

**It downgraded severities honestly.** `idt.c:244`'s 20-byte `interrupt_frame` was
proposed as medium; the refuter measured that with the argument discarded the generated
code is *byte-identical* between the 20- and 40-byte structs, making it genuinely latent.

Nothing about the process is impressive if it only ever confirms. It didn't.

---

## What survived refutation

Full write-ups linked; all of these are ✓ hand-verified as well.

| class | finding |
|---|---|
| fake-gates | [`land-gate.sh:137` skips non-executable boot gates in silence](CRITICAL-gates-that-cannot-fail.md) — `verify-efi.sh` is in that list |
| fake-gates | [`run_tests.sh:192` drops the whole kernel-boot section with no output](CRITICAL-gates-that-cannot-fail.md) when QEMU is absent |
| fake-gates | [`hazard-scan.sh:86` scans 6 of ~50 TUs](CRITICAL-ci-truncation-gate-is-blind.md) — found independently by two class sweeps |
| dma-bounds | [`xhci_ram_ok()` zeroes the live DCBAA scratchpad pointer](CRITICAL-gates-that-cannot-fail.md), reachable from the zl builtin `usb_ram` |
| dma-bounds | [cursor / self-test overlap 3 of 4 GGTT pages](VERIFIED-ggtt-span-collision.md) |
| llp64 | [`intel_mmio()` truncates a 64-bit BAR](CRITICAL-gpuring-bar-truncation.md) — and neither the gate nor the four `-Werror=` flags can see this shape |
| llp64 | [`http.c:278` round-trips the body pointer through a `u32` accessor](VERIFICATION-LOG.md), and `browser.c:426` — which is C — widens it back |
| silent-failure | [link training reports success on a black panel](CRITICAL-display-failure-reporting-inverted.md) |
| silent-failure | [`MS_STEP_SOFT` failures fail the whole modeset](CRITICAL-display-failure-reporting-inverted.md) |
| buffer-overrun | [`build_request` writes 9 bytes past `req[512]`](CRITICAL-http-request-overflow.md) |
| orphans | [the CSS engine is in no build and has no caller](CRITICAL-browser-cluster.md) |
| concurrency | [the APs run with no IDT](CRITICAL-smp-bands-have-no-idt.md) |

### Survived, and not yet written up elsewhere

| file:line | finding |
|---|---|
| `kernel/wguard.sh:45` | the guard for the LLP64 class never reads the flag line it claims to guard. The refuter's amendment: dropping `-Werror=pointer-to-int-cast` **is** detected, so "cannot detect *any* single flag" overstates it — but the rest holds |
| `tools/hazard-scan.sh:62` | checks 1 and 2 report through `warn()`, which never sets `fail`; `fail=1` appears only at `:125` and `:133`. Advisory checks that look like gates |
| `tools/engine-parity.sh:141` | `if [ "$got" = "__BUILDFAIL__" ]; then mark="build!"` with no `fail=1` — a total engine build failure is not a failure |
| `kernel/check-himap.sh:107` | `grep -oP '0x0[0-9A-Fa-f]{7}'` requires a literal `0` after `0x`, making it a **spelling** filter rather than a value filter. Latent, not live |
| `kernel/memmap.h:73` | **new** — `AP_STACK_SPAN` (272 KiB) is derived from the *length* of `cpu_apic_ids[]`, but both trampolines address the stacks by a different rule. The refuter's correction: the load-bearing evidence is a line the original claim never cited |
| `kernel/settings.c:483` | `settings_load` has no caller — the Settings app writes to NVMe on every gesture and nothing ever reads it back. Settings do not persist |
| `kernel/gpucursor.c:149` | `gpu_cursor_install` has no caller, so `cursor_live` can never become 1 and `wm.c:1620,1627` permanently take the false branch |
| `kernel/fb.c:2380` | `px * 4u` wraps in 32 bits, so the ceiling test passes and the allocation proceeds |
| `kernel/fb.c:2569` | `slot_capture` computes `w * h` with no clamp — signed overflow on all builds |
| `kernel/input.c:118` | four input-path ring drops with no counter, one of which loses data permanently |
| `kernel/pci.c:315` | `pci_enable()` discards `pci_power_on()`'s result — reintroducing the exact D3hot failure `06b1a0f` was written to prevent |
| `kernel/xhci.c:262` | the firmware-ownership handshake measures whether SMM released the controller and throws the answer away |

---

## The structural finding

From the `silent-failure` sweep, and it is the most useful sentence the wave produced:

> The tree has **two populations.** `net.c`, `arena.c`, `wm.c`, `html.c`, `layout.c`,
> `notify.c`, `virtio_net.c` and `tcp.c` follow the doctrine written at
> `kernel/net.c:58-64` and are clean. **The display driver, the PCI power path, the xHCI
> firmware handoff and the whole input event path do not.**

The doctrine — *"EVERY DROP PATH GETS A COUNTER"* — was written after one silent drop
cost six wrong hypotheses. It is correct, well-argued, and already followed by half the
codebase. This is not "add error handling everywhere"; it is four subsystems and a rule
they already have.

The same shape appears in the gate findings: `docs/GUARDS-THAT-DID-NOT-GUARD.md` exists,
is excellent, and lists five. The sweep found seven more of exactly the kinds it
describes. **The document is right and incomplete**, which is a much better problem than
being wrong.

---

## Method note, for reuse

Two things made this wave produce more than the other three per agent:

1. **Sweeping by class, not by file.** A file-scoped agent finds an instance; a
   class-scoped agent finds the instance *and its siblings*, which is what makes "fix the
   shared function once" possible. Every finding carried a `siblings` field.
2. **Refuters defaulting to refuted.** An unproven bug costs more than a missed one here,
   because acting on it means editing kernel code that currently works. Three candidates
   died, two of my own documents were corrected, and several severities came down. That
   is the stage doing its job.
