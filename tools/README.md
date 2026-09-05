# Repository Tools

Maintenance, validation, evidence, and status helpers live here. Language-aware
C tooling belongs in `src/tools/`; this directory is for repository-level
scripts and their checked data files.

Tools must fail clearly when prerequisites are absent and must distinguish a
skipped check from a passed check.

The [allocator feature-join repair](../docs/evidence/allocator-feature-join-repair-2026-09-05.md)
records the current scheduler continuation's full-gate blocker and focused
regression command, `python3 tools/test_feature_status_pmm.py`.

## Directory documentation

`directory-docs.py` creates and verifies the eight-part documentation capsule in
the repository root and every tracked directory. Run it after adding or moving a
directory:

```bash
python3 tools/directory-docs.py
python3 tools/directory-docs.py --check
```

The generated blocks are tool-owned. Verified local knowledge belongs only in
each file's preserved `BEGIN LOCAL` block. The complete design and truth rules
are in
[`docs/design/directory-documentation-system.md`](../docs/design/directory-documentation-system.md).
