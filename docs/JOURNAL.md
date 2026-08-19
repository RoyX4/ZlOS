# Journal

What happened, and what the tree looked like when it happened. Appended
automatically by `tools/journal.sh` from the post-commit hook — newest first.

Git stores the diff; this stores the context around it: outstanding hazard
counts, whether the docs still agreed with the code, which files moved. That is
the part that makes a change legible later, and the part nobody writes down.

## 2026-08-19 — docs(fleet): seven more gates that cannot fail, and a live DCBAA being zeroed (284f3d3)

`fleet/audit-2026-08-19` · 3 files changed, 164 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-gates-that-cannot-fail.md`
- `docs/fleet/README.md`

</details>

## 2026-08-19 — docs(fleet): the APs have no IDT, and .ultra/STATE.md ranks turning them on first (cc17df5)

`fleet/audit-2026-08-19` · 3 files changed, 159 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-smp-bands-have-no-idt.md`
- `docs/fleet/README.md`

</details>

## 2026-08-19 — docs(fleet): the browser cluster - hover navigates, and 1032 lines of dead CSS (1cb1693)

`fleet/audit-2026-08-19` · 2 files changed, 165 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-browser-cluster.md`

</details>

## 2026-08-19 — docs(fleet): the CI truncation gate scans 6 of ~50 translation units (ea3d900)

`fleet/audit-2026-08-19` · 5 files changed, 175 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>5 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-ci-truncation-gate-is-blind.md`
- `docs/fleet/README.md`
- `docs/fleet/VERIFICATION-LOG.md`

</details>

## 2026-08-19 — docs(fleet): hand-verified findings from the 100-agent audit (50a5679)

`fleet/audit-2026-08-19` · 8 files changed, 1113 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>8 file(s)</summary>

- `docs/fleet/CRITICAL-browser-urlbar-keys.md`
- `docs/fleet/CRITICAL-gpuring-bar-truncation.md`
- `docs/fleet/GROUND-TRUTH-CORRECTIONS.md`
- `docs/fleet/PANEL-POWER-IS-REACHABLE.md`
- `docs/fleet/README.md`
- `docs/fleet/VERIFICATION-LOG.md`
- `docs/fleet/VERIFIED-WM-SNAP.md`
- `docs/fleet/VERIFIED-ggtt-span-collision.md`

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

