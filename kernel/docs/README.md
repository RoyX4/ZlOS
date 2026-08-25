# Kernel Docs Map

This folder mixes prompts, run receipts, decisions, plans, and hardware notes.
Read the top banner of each file before treating it as current.

## Live Or Current

| File | Truth state |
|---|---|
| `reference/system/input-stack.md` | Current input model and probe map. |
| `reference/system/memory-model.md`, `reference/system/memory-map.md`, `reference/system/dma-sites.md` | Current fixed-address and memory-boundary references. |
| `reference/ui/widgets.md` | Measured 68-widget visual specification and inconsistency register. |
| `guides/thinkpad-first-boot.md` | Physical-boot runbook; does not by itself prove a successful boot happened. |
| `current-speed-and-quality-diagnosis.md` | Current speed diagnosis from measured traces, with hardware/QEMU boundaries called out. |
| `plans/performance-architecture-roadmap.md` | Current performance architecture plan. |

## Evidence And Run Receipts

These record work that happened. They are evidence, not a promise that adjacent
hardware-only gates closed. Dated receipts live in `evidence/`.

| File | Receipt |
|---|---|
| `evidence/browser-storage-run.md` | Browser storage/BSS work receipt. |
| `evidence/browser-render-run.md` | Browser rendering work receipt. |
| `evidence/app-registry-truth-2026-08-22.md` | Application-registry false-green repair and exact route receipt. |
| `browser-status.md` | Browser state and measured capability boundary. |
| `evidence/desktop-platform-run.md` | Platform-track run record. |
| `evidence/desktop-overnight-run.md` | Overnight compositor run record. |
| `evidence/desktop-feel.md` | Completed feel/control track with measured gates and remaining physical boundary. |
| `evidence/desktop-scale-and-effects.md` | Implemented scale/effects diagnosis and measured visual result. |
| `evidence/desktop-smp-bands.md` | Measured SMP band-rendering result and its explicit speed ceiling. |
| `evidence/desktop-v10-plan.md` | Completed ten-item v10 plan retained with its measured run and corrections. |
| `evidence/system-track.md` | Completed system-track implementation record and measured costs. |
| `evidence/physical-input-and-recorder-diagnosis-2026-08-23.md` | Physical ThinkPad input and persistent-recorder diagnosis with explicit limits. |
| `evidence/display/gen9-modeset-plan.json` | Audited Gen9 display research, resolved source conflicts, and retained hardware hazard register. |
| `evidence/mp00/benchmark-registry-2026-08-23.md` | Exact host benchmark receipt with explicit open regressions and native-target limits. |
| `evidence/mp00/test-inventory-2026-08-22.md` | Host-test classification and execution receipt retained from the MP-00 foundation run. |
| `evidence/performance-architecture-implementation-2026-08-22.md` | Performance architecture implementation batch. |
| `evidence/exercises/` | Retained exercise images, transcripts and physical/QEMU bundles; disposable new runs still go to ignored `kernel/exercise-out/`. |

## Guides

| File | Purpose |
|---|---|
| `guides/desktop-build-guide.md` | Plain-language desktop/compositor orientation. |
| `guides/thinkpad-first-boot.md` | Physical ThinkPad boot and evidence-capture runbook. |

## Architecture

| File | Purpose |
|---|---|
| `architecture/boot/boot-architecture-decision.md` | Boot ownership ADR and loader/kernel boundary. |
| `architecture/boot/typed-boot-handover.md` | Typed boot handover contract and recovery-policy boundary. |
| `architecture/boot/persistent-boot-observer.md` | Persistent boot diagnosis architecture. |
| `architecture/system/always-on-telemetry.md` | Always-on recorder and telemetry contract. |
| `architecture/system/automatic-system-audit.md` | Automatic system-audit architecture and proof boundaries. |
| `architecture/system/exec-kill-path.md` | Process termination and non-cooperative execution contract. |
| `architecture/system/user-process-abi.md` | 64-bit user process ABI. |

## Concepts

| File | Purpose |
|---|---|
| `concepts/what-is-a-bios.md` | Firmware ownership and hardware openness explainer. |
| `concepts/what-is-actually-impossible.md` | Audited feasibility wall map with corrected assumptions. |
| `concepts/beyond-the-kernel.md` | Language, browser, network, and C-to-zl boundary analysis. |

## Driver Notes

| File | Purpose |
|---|---|
| `drivers/display/gpu-driver.md` | Intel GPU driver state, proof, and remaining boundary. |
| `drivers/display/gpu-blitter.md` | Blitter bring-up order and measured silicon result. |
| `drivers/display/gen9-shader-source.md` | Gen9 shader provenance and captured program. |
| `drivers/display/gen9-blend-pipeline.md` | Fixed blend-pipeline packet recipe. |
| `drivers/network/ax201-wifi.md` | AX201 implementation and evidence boundary. |

## Archived Prompt Files

These old execution briefs live in `archive/prompts/`. Their unresolved work was
carried into newer status and roadmap documents; the old task lists are history.

| File | Banner state |
|---|---|
| `archive/prompts/BROWSER-PROMPT.md` | Mostly done; use for costing/hazards, not as the queue. |
| `archive/prompts/OVERNIGHT-PROMPT.md` | Mostly done; remaining items were moved to current status docs. |
| `archive/prompts/PLATFORM-PROMPT.md` | Mostly done; some outcomes landed in different shape. |
| `archive/prompts/EXEC-PROMPT.md` | Partly executed; open work is tracked elsewhere. |
| `archive/prompts/FEEL-PROMPT.md` | Partly executed; open work is tracked elsewhere. |
| `archive/prompts/SYSTEM-PROMPT.md` | Partly executed; open work is tracked elsewhere. |
| `archive/prompts/LOOK-AND-SPEED-PROMPT.md` | Older brief; current intent is in `visual-speed-northstar.md`. |
| `archive/prompts/NEXT-PROMPT.md` | Superseded dated ranking. |
| `archive/prompts/POINTER-PROMPT.md` | Implemented pointer diagnosis; manual feel acceptance remains elsewhere. |
| `archive/superseded/feature-catalogue-2026-08-17.md` | Stale pre-program feature/status survey; replaced by `docs/program/`. |
| `archive/superseded/desktop-plan.md` | Audited inversion plan: useful decisions, superseded task/status body. |
| `archive/superseded/desktop-TODO.md` | Audited 30-of-34-complete task list; never use as the current queue. |
| `archive/superseded/desktop-wiring.md` | Historical copy-paste wiring handoff; current boot already calls the compositor. |
| `archive/superseded/desktop-northstar-feasibility.md` | Retained layer analysis with explicitly superseded percentage estimates. |
| `archive/superseded/display-roadmap.md` | Useful display design with a contradicted status table; never use as the current queue. |

`archive/audits/` contains dated task/session audits. `archive/handoffs/`
contains recovery notes for work that now has a newer receipt or status map.
The recovered `archive/audits/display-state-2026-08-25.md` is a historical
read-only display audit from commit `85fcdf5` plus a dirty restructure; it is
durable context now, not current implementation proof.
`archive/audits/ds-clone-status-2026-08-20.md` is the preserved false-green
snapshot that the later application-registry receipt supersedes.

## Planning And Research

Current plans are grouped under `plans/`. The first pair is:

| File | Purpose |
|---|---|
| `plans/performance-architecture-roadmap.md` | Current physical-desktop performance architecture and test contract. |
| `plans/retained-window-surfaces.md` | Immediate retained-surface implementation contract. |
| `plans/driver-build-order.md` | Dated dependency-first driver sequencing; current queue is authoritative. |
| `plans/ds-clone-plan.md` | Dated visual-convergence method; wave status must be rechecked. |
| `plans/gpu-next.md` | Measured GPU direction and engine decision. |
| `plans/wireless-plan.md` | Audited, mostly-open WiFi and Bluetooth implementation plan. |

Plan status is stated inside each file; placement under `plans/` does not imply
that every listed item is open.

Comparative and source-heavy studies live in `research/`:

| File | Subject |
|---|---|
| `research/desktop-prior-art.md` | Graphics approaches used by other hand-built operating systems. |
| `research/intel-graphics-stack.md` | Intel graphics stack and zlOS boundary analysis. |
| `research/os-landscape.md` | Hobby-OS landscape and 3D strategy survey. |
| `research/starred-repository-research.md` | Index into the 32-repository external research corpus. |
| `research/why-mainstream-desktops-feel-fast.md` | Source-backed desktop latency and rendering study. |

## Rule

Do not convert `mostly done` into `done`. Do not convert QEMU proof into hardware
proof. If a file does not say what command or physical run proved a claim, treat
the claim as unverified until rechecked.
