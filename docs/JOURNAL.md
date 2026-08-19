# Journal

What happened, and what the tree looked like when it happened. Appended
automatically by `tools/journal.sh` from the post-commit hook — newest first.

Git stores the diff; this stores the context around it: outstanding hazard
counts, whether the docs still agreed with the code, which files moved. That is
the part that makes a change legible later, and the part nobody writes down.

## 2026-08-20 — feat(refrender): support.js shim renders the ds.html desktop reference (84a8195)

`claude/compassionate-curie-a0599c` · 4 files changed, 793 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>4 file(s)</summary>

- `kernel/refrender/.gitignore`
- `kernel/refrender/package.json`
- `kernel/refrender/render-ref.mjs`
- `kernel/refrender/support.js`

</details>

## 2026-08-20 — feat(oracle): region map generated from the mockup, not transcribed (a4ae44b)

`claude/compassionate-curie-a0599c` · 3 files changed, 1314 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/oracle/.gitignore`
- `kernel/oracle/gen-regions.py`
- `kernel/oracle/regions.json`

</details>

## 2026-08-20 — feat(design): ONE palette source, and it is the reference's lime-on-grey (506a6a9)

`claude/compassionate-curie-a0599c` · 8 files changed, 9282 insertions(+), 62 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>8 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`
- `docs/design/ds-reference.html`
- `kernel/design.h`
- `kernel/refrender/ds.html`
- `kernel/refrender/vendor/react-dom.js`
- `kernel/refrender/vendor/react.js`
- `kernel/ui.c`

</details>

## 2026-08-20 — merge: desktop/storage-recovered, and port every app onto the theme roles (57ee9e9)

`claude/compassionate-curie-a0599c` · 16 files changed, 677 insertions(+), 134 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>9 file(s)</summary>

- `freestanding/runtime_kernel.c`
- `kernel/HANDOFF.md`
- `kernel/apps_games1.zl`
- `kernel/apps_games2.zl`
- `kernel/apps_registry.zl`
- `kernel/apps_system.zl`
- `kernel/apps_utils.zl`
- `kernel/hosttest/wmshot.c`
- `kernel/kernel.zl`

</details>

## 2026-08-19 — feat(apps): 3 system apps, RNG seeding, and a proven interactive probe (94cd4c8)

`desktop/app-suite` · 8 files changed, 435 insertions(+), 10 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>8 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`
- `kernel/apps_games1.zl`
- `kernel/apps_games2.zl`
- `kernel/apps_registry.zl`
- `kernel/apps_system.zl`
- `kernel/apps_utils.zl`
- `kernel/probe-catalog.py`

</details>

## 2026-08-19 — feat(apps): app-suite registry, catalog window, 11 utilities, 6 games (24c06bd)

`desktop/app-suite` · 11 files changed, 1731 insertions(+), 22 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>11 file(s)</summary>

- `compile.c`
- `kernel/apps_common.zl`
- `kernel/apps_games1.zl`
- `kernel/apps_games2.zl`
- `kernel/apps_registry.zl`
- `kernel/apps_system.zl`
- `kernel/apps_utils.zl`
- `kernel/check-zl-calls.sh`
- `kernel/kernel.zl`
- `kernel/mkdisk.sh`
- `kernel/raw_boot.asm`

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

