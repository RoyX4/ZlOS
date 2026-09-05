<!-- Managed by tools/directory-docs.py. Edit only the LOCAL block. -->
<!-- BEGIN GENERATED: directory-docs -->
# kernel/docs/architecture/system

## Purpose

System-wide ABI, audit, memory, process, and service contracts.

## Classification

- **Class:** `documentation`
- **State:** current or status-qualified documentation
- **Evidence boundary:** Documents must state whether they are plans, current authority, research, or historical evidence.

## Start here

- Read the [project status](../../../../docs/PROJECT-STATUS.md) for current truth.
- Read the local [status](STATUS.md), [tasks](TODO.md), [rules](RULES.md),
  [values](VALUES.md), and [operating procedure](SOP.md).
- Editing agents must also read [AGENTS.md](AGENTS.md).

## Child directories

- None.

## Direct tracked contents

- `always-on-telemetry.md`
- `automatic-system-audit.md`
- `bounded-scheduler-policy.md`
- `exec-kill-path.md`
- `user-process-abi.md`

This inventory is local orientation, not a completion claim. See the [code map](../../../../docs/CODE-MAP.md)
for repository-wide ownership.
<!-- END GENERATED: directory-docs -->

<!-- BEGIN LOCAL: directory-docs -->
## Current contracts

- [Bounded scheduler policy](bounded-scheduler-policy.md) separates the
  host-proved selection/state contract from privileged switching and records
  its bounded persistent Ring-3 target integration.
<!-- END LOCAL: directory-docs -->
