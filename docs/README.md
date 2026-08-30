# Documentation Map

This folder is organized by truth state. A document can be valuable and still
not be current.

## Read First

| File | Truth state |
|---|---|
| `PROJECT-STATUS.md` | Current cross-repo, commit, plan, and implementation boundary. |
| `EXECUTION-ROADMAP.md` | Current priority spine. Later implementation notes update it; they do not convert unverified hardware into proof. |
| `CODE-MAP.md` | Current file-layout map. |
| `REPOSITORY-STRUCTURE.md` | Layout rules and source-ownership boundaries. |
| `DIRECTORY-CAPSULE-INDEX.md` | Complete index of the eight local documentation surfaces in every tracked directory. |
| `design/RESTRUCTURE-REVIEW-2026-08-25.md` | Independent review of the published Phase 1 restructure and the still-unimplemented later destination. |
| `GUARDS-THAT-DID-NOT-GUARD.md` | Current warning list for false-green gates. |
| `WORKING-RULE.md` | Current integration rule and why it exists. |

## Current Program

`program/` holds the complete feature/contract/phase program and machine-readable
status files. Treat it as the broad destination, not a claim that every item is
done.

## Partial Or Mixed Work

These are useful, but not a done list:

| Path | How to read it |
|---|---|
| `design/` | Proposals, decisions, and gap studies. Trust each file's own `Status:` line over the directory name. |
| `fleet/` | Audit boards and verification logs. `VERIFIED-*` files are evidence; `CRITICAL-*` files are findings; board files are triage. |
| `kernel/docs/` | Kernel-specific current references and plans. Its dated receipts and historical prompts now have separate subfolders. |

## Evidence

| Path | What it is |
|---|---|
| `evidence/MERGE-EVIDENCE.md` | Round-1 merge evidence and measured conflict classes. |
| `evidence/MERGE-ROUND-2.md` | Round-2 worktree evidence and integration state at that time. |
| `evidence/integration/` | Completed worktree and all-branch integration receipts. |
| `evidence/integration/WHOLE-TOPOLOGY-CLOSURE-2026-08-30.md` | Current whole-home branch, worktree, standalone-clone, dirty-file and deletion closure. |
| `evidence/status-audits/` | Dated whole-project status audits; evidence-rich, but not the current queue. |
| `evidence/POINTER-EVIDENCE.md` | Pointer fix measurements; explicitly not full human/live-VM closure. |
| `evidence/COMPARE-BOREDOS.md` | Measured comparison against a peer hobby OS. |
| `evidence/REPOSITORY-STRUCTURE-STUDY-2026-08-25.md` | Read-only layout measurements across all 33 pinned repositories, with detailed inspection of the largest systems. |
| `evidence/STRUCTURE-CLEANUP-2026-08-25.md` | Measured cleanup receipt: relocations, static checks, held metadata/source boundaries, and explicit no-build/no-boot/no-push scope. |
| `evidence/DOCUMENTATION-COVERAGE-AUDIT-2026-08-25.md` | Static proof that the 906-feature/21-phase/174-contract program is self-contained and navigable, with stale runtime evidence kept explicit. |
| `evidence/visual-diffs/` | Curated before/after visual evidence images. |

## Archive

`archive/` is for documents that are still worth keeping but should not be used
as the current queue.

| Path | Meaning |
|---|---|
| `archive/superseded/` | Plans that already say they are superseded or replaced. |
| `archive/prompts/` | Old execution prompts and reconciliation prompts kept for history. |
| `archive/handoffs/` | Dated continuation notes whose work moved into newer status files. |
| `archive/backups/` | Backup copies, not active references. |

## Rule

Do not mark a thing done because it moved folders. A file is done only when its
own evidence says what passed, what was skipped, and what remains hardware-only
or unverified.
