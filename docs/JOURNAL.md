# Journal

What happened, and what the tree looked like when it happened. Appended
automatically by `tools/journal.sh` from the post-commit hook — newest first.

Git stores the diff; this stores the context around it: outstanding hazard
counts, whether the docs still agreed with the code, which files moved. That is
the part that makes a change legible later, and the part nobody writes down.

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

