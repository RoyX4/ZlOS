<!-- Managed by tools/directory-docs.py. Edit only the LOCAL block. -->
<!-- BEGIN GENERATED: directory-docs -->
# Standard operating procedure: `docs/archive/superseded/`

1. Read the local `README.md`, `STATUS.md`, `TODO.md`, and `RULES.md`.
2. Confirm current authority in the [project status](../../PROJECT-STATUS.md) and ownership in the [code map](../../CODE-MAP.md).
3. Inspect `git status` and preserve unrelated or concurrent work.
4. Define the smallest outcome and the evidence lane it requires.
5. Make the change within this directory's `archive` boundary.
6. Run the smallest relevant deterministic check and prove it can observe the
   changed behavior where practical.
7. Update local tasks or status only when the evidence supports the wording.
8. Report passed, failed, skipped, QEMU-only, and physical-only evidence
   separately.

For capsule maintenance, follow the [directory documentation plan](../../design/directory-documentation-system.md) and run
`python3 tools/directory-docs.py --check` from the repository root.
<!-- END GENERATED: directory-docs -->

<!-- BEGIN LOCAL: directory-docs -->
<!-- Immutable class: add current work or claims at the owning source or in a superseding receipt. -->
<!-- END LOCAL: directory-docs -->
