# External starred-repository research index

The published 33-repository clean-room research suite is tracked in this active
OS checkout:

```text
docs/program/research/
```

The pinned read-only source shelf used by that research is:

```text
/home/roy/Documents/repos/zl-starred-sources/
```

This file exists so every new AI/session working in `zl-linux` discovers the
research before proposing architecture, drivers, services, apps, browser work,
visual changes, performance work, packages, agents, or public-demo changes.

## Read in this order

1. [`README.md`](../../../docs/program/research/README.md) — suite map, evidence boundary, reading order, donors and corrections.
2. [`RESPONSIVENESS_RENDERING_AND_VISUAL_POLISH_DEEP_DIVE_2026-08-22.md`](../../../docs/program/research/RESPONSIVENESS_RENDERING_AND_VISUAL_POLISH_DEEP_DIVE_2026-08-22.md) — why polished systems stay responsive and what current zlOS measurements select next.
3. [`CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md`](../../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md) — 906 stable product feature atoms and the exact 61-current-app plus 24-game crosswalk.
4. [`ZLOS_CURRENT_DRIVER_AND_APP_BASELINE_2026-08-21.md`](../../../docs/program/research/ZLOS_CURRENT_DRIVER_AND_APP_BASELINE_2026-08-21.md) and [`ZLOS_CURRENT_VISUAL_WEB_AND_APP_EXPERIENCE_BASELINE_2026-08-21.md`](../../../docs/program/research/ZLOS_CURRENT_VISUAL_WEB_AND_APP_EXPERIENCE_BASELINE_2026-08-21.md) — current zlOS reality and proof ceilings.
5. [`CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md`](../../../docs/program/research/CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md), [`IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`](../../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), [`DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`](../../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), and [`VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md`](../../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md) — dependency order and proof obligations.
6. The three `*_REFUTATION_AND_*` reports before accepting any donor claim. Corrections override first-pass praise.

## Ownership and evidence rules

- `zl-linux` remains authoritative for current implementation and live evidence: `kernel/HANDOFF.md`, `docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`, source, tests, QEMU and physical journals.
- The tracked research suite is authoritative for the pinned 33-repository comparison, feature normalization, donor provenance, rejected patterns and clean-room destination inventory.
- Never copy donor source. Transfer contracts, ideas, failure shapes and tests.
- Never upgrade screenshot, claim, fetched/external code, source presence, build, QEMU or host-harness evidence into native runtime/hardware proof.
- Revalidate any current-zlOS statement against this checkout before implementation; the active tree can advance after the dated research snapshot.
- Preserve the shared dirty checkout. Read current orientation and status before editing, and do not revert unrelated concurrent work.

The research catalogue is intentionally broader than the active roadmap. It
defines the complete destination; `kernel/HANDOFF.md`, `docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`
and current dependency gates determine what is safe and useful to implement
next.
