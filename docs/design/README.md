# Design Documents

The repository-wide per-directory orientation and agent-policy system is defined
in [`directory-documentation-system.md`](directory-documentation-system.md). It
covers every tracked directory and is enforced by `tools/directory-docs.py`.

This folder contains proposals, decisions, inventories, gap studies, and visual
references. It is not an implementation-status folder.

- Trust each document's own `Status:` banner.
- `proposal`, `plan`, and `ready to build` mean not implemented.
- `decision` fixes an intended behavior; it does not prove every engine follows it.
- Inventory and gap files are dated analysis and should be rechecked before use.
- Superseded designs belong in `../archive/superseded/`.

Current delivery status lives in [`../PROJECT-STATUS.md`](../PROJECT-STATUS.md)
and [`../program/`](../program/).
