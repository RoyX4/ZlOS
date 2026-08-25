# Kernel Docs Map

This folder mixes prompts, run receipts, decisions, plans, and hardware notes.
Read the top banner of each file before treating it as current.

## Live Or Current

| File | Truth state |
|---|---|
| `POINTER-PROMPT.md` | Live work per its audit banner. It is not history. |
| `input-stack.md` | Current input model and probe map. |
| `memory-model.md`, `memory-map.md`, `dma-sites.md` | Current fixed-address and memory-boundary references. |
| `thinkpad-first-boot.md` | Physical-boot runbook; does not by itself prove a successful boot happened. |
| `current-speed-and-quality-diagnosis.md` | Current speed diagnosis from measured traces, with hardware/QEMU boundaries called out. |
| `performance-architecture-roadmap.md` | Current performance architecture plan. |

## Evidence And Run Receipts

These record work that happened. They are evidence, not a promise that adjacent
hardware-only gates closed. Dated receipts live in `evidence/`.

| File | Receipt |
|---|---|
| `evidence/browser-storage-run.md` | Browser storage/BSS work receipt. |
| `evidence/browser-render-run.md` | Browser rendering work receipt. |
| `browser-status.md` | Browser state and measured capability boundary. |
| `evidence/desktop-platform-run.md` | Platform-track run record. |
| `evidence/desktop-overnight-run.md` | Overnight compositor run record. |
| `evidence/performance-architecture-implementation-2026-08-22.md` | Performance architecture implementation batch. |
| `evidence/exercises/` | Retained exercise images, transcripts and physical/QEMU bundles; disposable new runs still go to ignored `kernel/exercise-out/`. |

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

`archive/audits/` contains dated task/session audits. `archive/handoffs/`
contains recovery notes for work that now has a newer receipt or status map.
The recovered `archive/audits/display-state-2026-08-25.md` is a historical
read-only display audit from commit `85fcdf5` plus a dirty restructure; it is
durable context now, not current implementation proof.

## Planning And Research

Files such as `driver-build-order.md`, `display-roadmap.md`,
`gpu-driver.md`, `gpu-next.md`, `wireless-plan.md`, `desktop-*.md`,
`os-landscape.md`, and `why-mainstream-desktops-feel-fast.md` are planning or
research unless they carry a receipt section with measured evidence.

## Rule

Do not convert `mostly done` into `done`. Do not convert QEMU proof into hardware
proof. If a file does not say what command or physical run proved a claim, treat
the claim as unverified until rechecked.
