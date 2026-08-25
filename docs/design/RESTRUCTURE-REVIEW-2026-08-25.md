# Repository Restructure Review - 2026-08-25

Status: reviewed and reconciled onto integrated `main` in the isolated
`codex/full-program` working tree. Phase 1 and its moved-path repairs are
implemented but remain uncommitted and unpushed. Later phases are decisions and
migration targets, not completed work.

## Review Method

Three read-only reviews examined the isolated `codex/full-restructure` worktree:

- architecture and ownership boundaries
- builds, automation, and moved-path integrity
- documentation, tests, archives, and evidence boundaries

No review agent edited files, compiled the project, booted QEMU, or touched the
shared dirty checkout.

The reviewed migration was subsequently three-way reconciled from its older
`85fcdf542b9c` base onto integrated `main` at `4b9883123682`. Current-main
behavior won every genuine rename conflict; the older worktree remains
preserved as an input record.

## Decision

The Phase 1 cleanup is worth keeping. It gives the language implementation,
zlOS sources, tests, tools, current docs, evidence, and archives distinct homes.
It is not the final architecture.

The strongest remaining problem is that `kernel/` is a zlOS product root, not
only a kernel. It currently owns applications, browser code, graphics, tests,
tools, and product documentation. Renaming or splitting that root is a real
build and source migration and must not be disguised as another cosmetic move.

## Intended Destination

```text
zlos/
|-- README.md
|-- build/
|-- boot/{multiboot,uefi,raw}/
|-- kernel/
|   |-- include/zlos/
|   |-- arch/x86/
|   |-- core/{init,console}/
|   |-- mm/
|   |-- process/
|   |-- syscall/
|   |-- bus/pci/
|   |-- drivers/{display,usb,hid,storage,network,rtc}/
|   |-- block/
|   |-- fs/zlfs/
|   |-- net/
|   |-- crypto/
|   `-- runtime/
|-- system/{compositor,desktop,shell}/
|-- apps/{registry,shared,browser,settings,terminal,games}/
|-- libraries/{ui,web,image}/
|-- tests/{unit,integration,system,visual,hardware,fixtures,support}/
|-- tools/{checks,images,generate,inspect,run}/
`-- docs/{status,architecture,subsystems,runbooks,plans,evidence,archive}/
```

Names in this tree are provisional. Ownership and dependency direction are the
decision; spelling is secondary.

## Phases

### Phase 1 - Repository Hygiene

Implemented in this working tree:

- language C sources moved under `src/` by frontend, runtime, backend, self-host,
  and tooling ownership
- zlOS implementation grouped under `kernel/src/`
- zlOS apps, boot files, tests, tools, docs, evidence, and archives separated
- root and subsystem orientation files added
- current, evidence, proposal, and archive meanings documented

This phase has static verification only. It has no fresh compiler, host-harness,
QEMU, graphical, or hardware proof.

### Phase 1.1 - Path Integrity

Implemented during this review:

- repaired CI references to moved boot gates and host tests
- made desktop comparison render the base worktree instead of HEAD's tree
- gave host harnesses temporary include discovery for nested source headers
- repaired the memory-map guard's source-copy and owner paths
- made the DMA exception check fail if its Intel source disappears
- repaired the RAM guard after its move made it scan its own prose and miss a
  Python QEMU command assembled through a variable
- expanded documentation path checking to nested source and untracked docs
- corrected generator working-directory documentation

### Phase 2 - Product And Build Boundaries

Not implemented:

- rename the zlOS product root from `kernel/` to `zlos/`
- separate kernel, system, apps, and reusable libraries
- replace duplicated build lists and source-directory-wide includes with one
  machine-readable build manifest and component-owned public headers
- make CI and local tooling consume that same manifest

Phase 2 must land atomically. A commit containing moves without their path
rewrites, or rewrites without their moves, is broken by construction.

### Phase 3 - Source Ownership

Not implemented:

- split the USB host controller from HID, storage, and CDC class behavior now
  concentrated in `xhci.c`
- reduce `kernel.zl` to composition and move platform, shell, desktop, and apps
  into owned modules
- move PCI and RTC out of architecture ownership
- separate block/cache policy from device drivers
- separate cryptography from networking
- split process, syscall, IPC, window, memory, and execution responsibilities

Large files are not split merely because they are large. Each split needs a
real public boundary, a dependency direction, and a runnable check.

### Phase 4 - Evidence Navigation

Not implemented:

- classify tests as unit, integration, system, visual, hardware, benchmark,
  support, or fixture without changing what any test proves
- add a test matrix mapping claims to commands and required environments
- finish current/reference/plan/evidence/archive subdivisions in both doc trees
- add receipts for retained binary evidence and mark historical prompts as such

## Open Risks

- `tools/hazard-scan.sh` still infers build configuration from shell scripts;
  dynamic include/source variables make that parser unreliable.
- `kernel/SOURCES` checks inclusion but does not yet reject every unclassified
  implementation file.
- C files still rely on basename includes and broad source-directory search
  paths. Current folders express intent more strongly than enforced boundaries.
- No fresh build, QEMU, graphical, or physical-hardware evidence exists for this
  restructure.

## Rules

- Do not create `done/` or `half-done/` source folders. Completion belongs in a
  status ledger backed by evidence.
- Do not treat an archive move as implementation evidence.
- Keep generated assets labeled and eventually place them under explicit
  `generated/` or `assets/` ownership.
- Keep the shared checkout untouched; continue risky migration work in the
  isolated worktree until a verified integration point exists.
