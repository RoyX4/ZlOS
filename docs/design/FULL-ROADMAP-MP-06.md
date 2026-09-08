# MP-06: Make storage, files and updates survive failure

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/src/fs/; kernel/src/drivers/storage/; kernel/tools/images/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-06` exports: Bounded block/VFS/file ownership and durable transactions for process and service consumers.

The handoff enables only its named subset. `CLOSE-06` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-06.01 — Preserve existing NVMe/xHCI/zlfs save/reopen paths while defining common asynchronous block requests

Preserve existing NVMe/xHCI/zlfs save/reopen paths while defining common asynchronous block requests.

**Requires:** `D-01`, `D-02`, `D-07`, `D-10`, `D-12`, `H-00`, `H-03`, `H-04`, `H-05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-06.02 — Specify queue ownership, cancellation, flush/barrier/discard, removal, geometry and write-order contracts

Specify queue ownership, cancellation, flush/barrier/discard, removal, geometry and write-order contracts.

**Requires:** `M-06.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-06.03 — Integrate cache/writeback with memory accounting and pressure

Integrate cache/writeback with memory accounting and pressure; define exactly which completion means durable storage.

**Requires:** `M-06.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-06.04 — Implement VFS names, directories, links, mounts, namespaces, permissions, handles and file-backed mapping joins

Implement VFS names, directories, links, mounts, namespaces, permissions, handles and file-backed mapping joins.

**Requires:** `M-06.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-06 — Bounded development handoff: Make storage, files and updates survive failure

Bounded block/VFS/file ownership and durable transactions for process and service consumers.

**Requires:** `M-06.04`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-06.05 — Deepen zlfs and implement each selected filesystem/import adapter in a restricted parser boundary

Deepen zlfs and implement each selected filesystem/import adapter in a restricted parser boundary.

**Requires:** `M-06.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-06.06 — Define on-disk version migration, corruption admission, repair, quotas, backups, snapshots and encrypted-volume recovery

Define on-disk version migration, corruption admission, repair, quotas, backups, snapshots and encrypted-volume recovery.

**Requires:** `M-06.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-06.07 — Test interruption at every commit boundary, disk full, torn/reordered writes, media removal and concurrent users

Test interruption at every commit boundary, disk full, torn/reordered writes, media removal and concurrent users.

**Requires:** `M-06.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-06.08 — Build signed package manifests and immutable dependency closure before install/update/remove transactions

Build signed package manifests and immutable dependency closure before install/update/remove transactions.

**Requires:** `M-06.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-06.09 — Join package retirement to live process/service/handle revocation

Join package retirement to live process/service/handle revocation; preserve a usable previous generation on every failure.

**Requires:** `M-06.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-06.10 — Keep destructive repair and physical write tests behind an exact-target operator procedure

Keep destructive repair and physical write tests behind an exact-target operator procedure; use disposable images for automatic tests.

**Requires:** `M-06.09`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-FS-001](#f-fs-001) | feature | block-device contract |
| [F-FS-002](#f-fs-002) | feature | partition discovery |
| [F-FS-003](#f-fs-003) | feature | volume identity |
| [F-FS-004](#f-fs-004) | feature | asynchronous block I/O |
| [F-FS-005](#f-fs-005) | feature | block cache |
| [F-FS-006](#f-fs-006) | feature | VFS object model |
| [F-FS-007](#f-fs-007) | feature | path resolution |
| [F-FS-008](#f-fs-008) | feature | mount namespace |
| [F-FS-009](#f-fs-009) | feature | devfs |
| [F-FS-010](#f-fs-010) | feature | initramfs/archive filesystem |
| [F-FS-011](#f-fs-011) | feature | zlfs core |
| [F-FS-012](#f-fs-012) | feature | zlfs directories |
| [F-FS-013](#f-fs-013) | feature | zlfs links |
| [F-FS-014](#f-fs-014) | feature | atomic rename/replace |
| [F-FS-015](#f-fs-015) | feature | file growth/truncate |
| [F-FS-016](#f-fs-016) | feature | file permissions/ownership |
| [F-FS-017](#f-fs-017) | feature | timestamps/metadata |
| [F-FS-018](#f-fs-018) | feature | extended attributes |
| [F-FS-019](#f-fs-019) | feature | file locking |
| [F-FS-020](#f-fs-020) | feature | durability API |
| [F-FS-021](#f-fs-021) | feature | journal/transaction log |
| [F-FS-022](#f-fs-022) | feature | free-space accounting |
| [F-FS-023](#f-fs-023) | feature | filesystem checker |
| [F-FS-024](#f-fs-024) | feature | corrupt-media admission |
| [F-FS-025](#f-fs-025) | feature | removable-media lifecycle |
| [F-FS-026](#f-fs-026) | feature | encrypted volume |
| [F-FS-027](#f-fs-027) | feature | snapshot/versioning |
| [F-FS-028](#f-fs-028) | feature | backup/restore |
| [F-FS-029](#f-fs-029) | feature | trash/recovery |
| [F-FS-030](#f-fs-030) | feature | package format |
| [F-FS-031](#f-fs-031) | feature | atomic package install |
| [F-FS-032](#f-fs-032) | feature | atomic package uninstall |
| [F-FS-033](#f-fs-033) | feature | package dependency solver |
| [F-FS-034](#f-fs-034) | feature | immutable system generation |
| [F-FS-035](#f-fs-035) | feature | update service |
| [F-FS-036](#f-fs-036) | feature | firmware store/update |
| [F-FS-037](#f-fs-037) | feature | package cache/cleanup |
| [F-FS-038](#f-fs-038) | feature | file indexing/search |
| [F-FS-039](#f-fs-039) | feature | content type/association |
| [F-FS-040](#f-fs-040) | feature | file preview/thumbnail |
| [C-P2.1](#c-p2-1) | contract | common block provider |
| [C-P2.2](#c-p2-2) | contract | bounded async request/completion queue |
| [C-P2.3](#c-p2-3) | contract | page/block cache and writeback worker |
| [C-P2.4](#c-p2-4) | contract | zlfs format and crash transaction |
| [C-P2.5](#c-p2-5) | contract | named-file migration and ZLLOG continuity |
| [C-P5.2](#c-p5-2) | contract | File/VFS broker and zlfs provider |
| [C-P5.7](#c-p5-7) | contract | Package service skeleton |
| [C-P7.3](#c-p7-3) | contract | transactional package install/upgrade/remove |
| [C-DA-11](#c-da-11) | contract | asynchronous block provider |
| [C-DA-21](#c-da-21) | contract | file/VFS and restricted parser services |
| [C-DA-24](#c-da-24) | contract | transactional install, update and uninstall |
| [C-VX-17](#c-vx-17) | contract | Application manifest, install and update |
| [T-BLK-001](#t-blk-001) | target | common asynchronous block provider |
| [T-BLK-002](#t-blk-002) | target | NVMe PCI controller/namespaces |
| [T-BLK-003](#t-blk-003) | target | AHCI/SATA |
| [T-BLK-004](#t-blk-004) | target | legacy ATA/IDE PIO |
| [T-BLK-005](#t-blk-005) | target | ATA bus-master DMA |
| [T-BLK-006](#t-blk-006) | target | ATAPI optical |
| [T-BLK-007](#t-blk-007) | target | virtio-blk |
| [T-BLK-008](#t-blk-008) | target | SCSI command core |
| [T-BLK-009](#t-blk-009) | target | USB MSC BOT |
| [T-BLK-016](#t-blk-016) | target | RAM disk |
| [T-BLK-017](#t-blk-017) | target | loop block device |
| [T-BLK-018](#t-blk-018) | target | read-only optical/ISO media |
| [T-BLK-020](#t-blk-020) | target | encrypted volume mapper |
| [T-BLK-021](#t-blk-021) | target | GPT parser |
| [T-BLK-022](#t-blk-022) | target | MBR/extended parser |
| [T-BLK-024](#t-blk-024) | target | crash/power-cut block simulator |
| [T-USB-006](#t-usb-006) | target | USB mass-storage class |
| [T-FSP-001](#t-fsp-001) | target | zlfs v2+ |
| [T-FSP-002](#t-fsp-002) | target | FAT12/16/32 |
| [T-FSP-003](#t-fsp-003) | target | ext2 read/write |
| [T-FSP-004](#t-fsp-004) | target | ext4 selected feature set |
| [T-FSP-005](#t-fsp-005) | target | ISO9660 |
| [T-FSP-006](#t-fsp-006) | target | tmpfs/RAMFS |
| [T-FSP-007](#t-fsp-007) | target | devfs |
| [T-FSP-008](#t-fsp-008) | target | procfs/system-information view |
| [T-FSP-009](#t-fsp-009) | target | sysfs/device-information view |
| [T-FSP-017](#t-fsp-017) | target | exFAT |
| [T-SVC-030](#t-svc-030) | target | Block Broker |
| [T-SVC-031](#t-svc-031) | target | VFS/File Service |
| [T-SVC-032](#t-svc-032) | target | zlfs Provider |
| [T-SVC-033](#t-svc-033) | target | Removable Media Manager |
| [T-SVC-034](#t-svc-034) | target | Volume/Partition Manager |
| [T-SVC-035](#t-svc-035) | target | File Cache/Writeback Service |
| [T-SVC-036](#t-svc-036) | target | Backup/Snapshot Service |
| [T-SVC-037](#t-svc-037) | target | Filesystem Check/Repair Service |
| [T-SVC-041](#t-svc-041) | target | Package Manager |
| [T-SVC-042](#t-svc-042) | target | Repository/Update Service |

<a id="f-fs-001"></a>
## F-FS-001 — block-device contract

**Original requirement:** sector geometry, capacity, alignment, async requests, flush, discard, errors, detach

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** NVMe is the only target provider; requests are not typed or asynchronous; no discard, cancellation, detach or partial-completion contract; no current physical-device receipt.

### Execution steps

- [ ] **F-FS-001.01 — Reconcile existing block-device contract**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for block-device contract. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: sector geometry, capacity, alignment, async requests, flush, discard, errors, detach
- [ ] **F-FS-001.02 — Freeze the exact contract for block-device contract**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: sector geometry, capacity, alignment, async requests, flush, discard, errors, detach. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-001.03 — Implement/prove: sector geometry**
  - Action: For block-device contract, implement or reuse and verify this exact obligation: sector geometry. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sector geometry; retain observable state/resource expectations.
- [ ] **F-FS-001.04 — Implement/prove: capacity**
  - Action: For block-device contract, implement or reuse and verify this exact obligation: capacity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capacity; retain observable state/resource expectations.
- [ ] **F-FS-001.05 — Implement/prove: alignment**
  - Action: For block-device contract, implement or reuse and verify this exact obligation: alignment. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for alignment; retain observable state/resource expectations.
- [ ] **F-FS-001.06 — Implement/prove: async requests**
  - Action: For block-device contract, implement or reuse and verify this exact obligation: async requests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for async requests; retain observable state/resource expectations.
- [ ] **F-FS-001.07 — Implement/prove: flush**
  - Action: For block-device contract, implement or reuse and verify this exact obligation: flush. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flush; retain observable state/resource expectations.
- [ ] **F-FS-001.08 — Implement/prove: discard**
  - Action: For block-device contract, implement or reuse and verify this exact obligation: discard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-001.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discard; retain observable state/resource expectations.
- [ ] **F-FS-001.09 — Implement/prove: errors**
  - Action: For block-device contract, implement or reuse and verify this exact obligation: errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-001.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for errors; retain observable state/resource expectations.
- [ ] **F-FS-001.10 — Implement/prove: detach**
  - Action: For block-device contract, implement or reuse and verify this exact obligation: detach. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-001.09.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for detach; retain observable state/resource expectations.
- [ ] **F-FS-001.11 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to block-device contract: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-001.10.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-001.12 — Integrate into the real consumer and runtime route**
  - Action: Wire block-device contract into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-001.11.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-001.13 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for block-device contract as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-001.12.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-002"></a>
## F-FS-002 — partition discovery

**Original requirement:** MBR/GPT checksums/ranges/overlap/unknowns and exact boot-origin binding

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-002.01 — Reconcile existing partition discovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for partition discovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: MBR/GPT checksums/ranges/overlap/unknowns and exact boot-origin binding
- [ ] **F-FS-002.02 — Freeze the exact contract for partition discovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: MBR/GPT checksums/ranges/overlap/unknowns and exact boot-origin binding. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-002.03 — Implement/prove: MBR/GPT checksums/ranges/overlap/unknowns and exact boot-origin binding**
  - Action: For partition discovery, implement or reuse and verify this exact obligation: MBR/GPT checksums/ranges/overlap/unknowns and exact boot-origin binding. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for MBR/GPT checksums/ranges/overlap/unknowns and exact boot-origin binding; retain observable state/resource expectations.
- [ ] **F-FS-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to partition discovery: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire partition discovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for partition discovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-003"></a>
## F-FS-003 — volume identity

**Original requirement:** stable UUID/label/generation handles, not enumeration order

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-003.01 — Reconcile existing volume identity**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for volume identity. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stable UUID/label/generation handles, not enumeration order
- [ ] **F-FS-003.02 — Freeze the exact contract for volume identity**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stable UUID/label/generation handles, not enumeration order. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-003.03 — Implement/prove: stable UUID/label/generation handles**
  - Action: For volume identity, implement or reuse and verify this exact obligation: stable UUID/label/generation handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stable UUID/label/generation handles; retain observable state/resource expectations.
- [ ] **F-FS-003.04 — Implement/prove: not enumeration order**
  - Action: For volume identity, implement or reuse and verify this exact obligation: not enumeration order. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for not enumeration order; retain observable state/resource expectations.
- [ ] **F-FS-003.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to volume identity: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-003.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-003.06 — Integrate into the real consumer and runtime route**
  - Action: Wire volume identity into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-003.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-003.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for volume identity as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-003.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-004"></a>
## F-FS-004 — asynchronous block I/O

**Original requirement:** bounded queues, deadlines, cancellation, partial completion and retry policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-004.01 — Reconcile existing asynchronous block I/O**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for asynchronous block I/O. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded queues, deadlines, cancellation, partial completion and retry policy
- [ ] **F-FS-004.02 — Freeze the exact contract for asynchronous block I/O**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded queues, deadlines, cancellation, partial completion and retry policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-004.03 — Implement/prove: bounded queues**
  - Action: For asynchronous block I/O, implement or reuse and verify this exact obligation: bounded queues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded queues; retain observable state/resource expectations.
- [ ] **F-FS-004.04 — Implement/prove: deadlines**
  - Action: For asynchronous block I/O, implement or reuse and verify this exact obligation: deadlines. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deadlines; retain observable state/resource expectations.
- [ ] **F-FS-004.05 — Implement/prove: cancellation**
  - Action: For asynchronous block I/O, implement or reuse and verify this exact obligation: cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-004.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancellation; retain observable state/resource expectations.
- [ ] **F-FS-004.06 — Implement/prove: partial completion and retry policy**
  - Action: For asynchronous block I/O, implement or reuse and verify this exact obligation: partial completion and retry policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-004.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for partial completion and retry policy; retain observable state/resource expectations.
- [ ] **F-FS-004.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to asynchronous block I/O: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-004.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-004.08 — Integrate into the real consumer and runtime route**
  - Action: Wire asynchronous block I/O into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-004.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-004.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for asynchronous block I/O as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-004.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-005"></a>
## F-FS-005 — block cache

**Original requirement:** coherent ownership, dirty accounting, eviction, writeback and device-loss behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no concurrent-owner model; no memory-pressure integration; device loss and retry/reset behavior are absent; no current QEMU fault-injection or physical receipt.

### Execution steps

- [ ] **F-FS-005.01 — Reconcile existing block cache**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for block cache. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: coherent ownership, dirty accounting, eviction, writeback and device-loss behavior
- [ ] **F-FS-005.02 — Freeze the exact contract for block cache**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: coherent ownership, dirty accounting, eviction, writeback and device-loss behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-005.03 — Implement/prove: coherent ownership**
  - Action: For block cache, implement or reuse and verify this exact obligation: coherent ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for coherent ownership; retain observable state/resource expectations.
- [ ] **F-FS-005.04 — Implement/prove: dirty accounting**
  - Action: For block cache, implement or reuse and verify this exact obligation: dirty accounting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dirty accounting; retain observable state/resource expectations.
- [ ] **F-FS-005.05 — Implement/prove: eviction**
  - Action: For block cache, implement or reuse and verify this exact obligation: eviction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for eviction; retain observable state/resource expectations.
- [ ] **F-FS-005.06 — Implement/prove: writeback and device-loss behavior**
  - Action: For block cache, implement or reuse and verify this exact obligation: writeback and device-loss behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for writeback and device-loss behavior; retain observable state/resource expectations.
- [ ] **F-FS-005.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to block cache: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-005.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-005.08 — Integrate into the real consumer and runtime route**
  - Action: Wire block cache into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-005.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-005.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for block cache as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-005.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-006"></a>
## F-FS-006 — VFS object model

**Original requirement:** opaque file/dir/mount handles, types, rights, references and generations

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-006.01 — Reconcile existing VFS object model**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for VFS object model. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: opaque file/dir/mount handles, types, rights, references and generations
- [ ] **F-FS-006.02 — Freeze the exact contract for VFS object model**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: opaque file/dir/mount handles, types, rights, references and generations. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-006.03 — Implement/prove: opaque file/dir/mount handles**
  - Action: For VFS object model, implement or reuse and verify this exact obligation: opaque file/dir/mount handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for opaque file/dir/mount handles; retain observable state/resource expectations.
- [ ] **F-FS-006.04 — Implement/prove: types**
  - Action: For VFS object model, implement or reuse and verify this exact obligation: types. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for types; retain observable state/resource expectations.
- [ ] **F-FS-006.05 — Implement/prove: rights**
  - Action: For VFS object model, implement or reuse and verify this exact obligation: rights. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rights; retain observable state/resource expectations.
- [ ] **F-FS-006.06 — Implement/prove: references and generations**
  - Action: For VFS object model, implement or reuse and verify this exact obligation: references and generations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-006.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for references and generations; retain observable state/resource expectations.
- [ ] **F-FS-006.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to VFS object model: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-006.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-006.08 — Integrate into the real consumer and runtime route**
  - Action: Wire VFS object model into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-006.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-006.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for VFS object model as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-006.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-007"></a>
## F-FS-007 — path resolution

**Original requirement:** rooted namespace, `.`/`..`, links, traversal bounds, race-safe operations

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-007.01 — Reconcile existing path resolution**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for path resolution. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: rooted namespace, `.`/`..`, links, traversal bounds, race-safe operations
- [ ] **F-FS-007.02 — Freeze the exact contract for path resolution**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: rooted namespace, `.`/`..`, links, traversal bounds, race-safe operations. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-007.03 — Implement/prove: rooted namespace**
  - Action: For path resolution, implement or reuse and verify this exact obligation: rooted namespace. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rooted namespace; retain observable state/resource expectations.
- [ ] **F-FS-007.04 — Implement/prove: `.`/`..`**
  - Action: For path resolution, implement or reuse and verify this exact obligation: `.`/`..`. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for `.`/`..`; retain observable state/resource expectations.
- [ ] **F-FS-007.05 — Implement/prove: links**
  - Action: For path resolution, implement or reuse and verify this exact obligation: links. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for links; retain observable state/resource expectations.
- [ ] **F-FS-007.06 — Implement/prove: traversal bounds**
  - Action: For path resolution, implement or reuse and verify this exact obligation: traversal bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-007.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for traversal bounds; retain observable state/resource expectations.
- [ ] **F-FS-007.07 — Implement/prove: race-safe operations**
  - Action: For path resolution, implement or reuse and verify this exact obligation: race-safe operations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-007.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for race-safe operations; retain observable state/resource expectations.
- [ ] **F-FS-007.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to path resolution: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-007.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-007.09 — Integrate into the real consumer and runtime route**
  - Action: Wire path resolution into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-007.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-007.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for path resolution as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-007.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-008"></a>
## F-FS-008 — mount namespace

**Original requirement:** per-session/container view, authority, propagation, detach and busy semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-008.01 — Reconcile existing mount namespace**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for mount namespace. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-session/container view, authority, propagation, detach and busy semantics
- [ ] **F-FS-008.02 — Freeze the exact contract for mount namespace**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-session/container view, authority, propagation, detach and busy semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-008.03 — Implement/prove: per-session/container view**
  - Action: For mount namespace, implement or reuse and verify this exact obligation: per-session/container view. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-session/container view; retain observable state/resource expectations.
- [ ] **F-FS-008.04 — Implement/prove: authority**
  - Action: For mount namespace, implement or reuse and verify this exact obligation: authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authority; retain observable state/resource expectations.
- [ ] **F-FS-008.05 — Implement/prove: propagation**
  - Action: For mount namespace, implement or reuse and verify this exact obligation: propagation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for propagation; retain observable state/resource expectations.
- [ ] **F-FS-008.06 — Implement/prove: detach and busy semantics**
  - Action: For mount namespace, implement or reuse and verify this exact obligation: detach and busy semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-008.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for detach and busy semantics; retain observable state/resource expectations.
- [ ] **F-FS-008.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to mount namespace: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-008.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-008.08 — Integrate into the real consumer and runtime route**
  - Action: Wire mount namespace into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-008.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-008.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for mount namespace as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-008.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-009"></a>
## F-FS-009 — devfs

**Original requirement:** generated device nodes from live handles, permission and disappearance behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-009.01 — Reconcile existing devfs**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for devfs. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generated device nodes from live handles, permission and disappearance behavior
- [ ] **F-FS-009.02 — Freeze the exact contract for devfs**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generated device nodes from live handles, permission and disappearance behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-009.03 — Implement/prove: generated device nodes from live handles**
  - Action: For devfs, implement or reuse and verify this exact obligation: generated device nodes from live handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated device nodes from live handles; retain observable state/resource expectations.
- [ ] **F-FS-009.04 — Implement/prove: permission and disappearance behavior**
  - Action: For devfs, implement or reuse and verify this exact obligation: permission and disappearance behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permission and disappearance behavior; retain observable state/resource expectations.
- [ ] **F-FS-009.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to devfs: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-009.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-009.06 — Integrate into the real consumer and runtime route**
  - Action: Wire devfs into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-009.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-009.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for devfs as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-009.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-010"></a>
## F-FS-010 — initramfs/archive filesystem

**Original requirement:** read-only checked archive, provenance and hostile input corpus

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-010.01 — Reconcile existing initramfs/archive filesystem**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for initramfs/archive filesystem. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: read-only checked archive, provenance and hostile input corpus
- [ ] **F-FS-010.02 — Freeze the exact contract for initramfs/archive filesystem**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: read-only checked archive, provenance and hostile input corpus. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-010.03 — Implement/prove: read-only checked archive**
  - Action: For initramfs/archive filesystem, implement or reuse and verify this exact obligation: read-only checked archive. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for read-only checked archive; retain observable state/resource expectations.
- [ ] **F-FS-010.04 — Implement/prove: provenance and hostile input corpus**
  - Action: For initramfs/archive filesystem, implement or reuse and verify this exact obligation: provenance and hostile input corpus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance and hostile input corpus; retain observable state/resource expectations.
- [ ] **F-FS-010.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to initramfs/archive filesystem: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-010.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-010.06 — Integrate into the real consumer and runtime route**
  - Action: Wire initramfs/archive filesystem into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-010.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-010.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for initramfs/archive filesystem as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-010.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-011"></a>
## F-FS-011 — zlfs core

**Original requirement:** persistent create/read/write/list/edit path with exact on-disk version

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** flat 32-entry namespace; 23-byte ASCII names; contiguous runs and no free-space index; no permissions or directories; no current QEMU persistence or physical-media receipt.

### Execution steps

- [ ] **F-FS-011.01 — Reconcile existing zlfs core**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for zlfs core. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: persistent create/read/write/list/edit path with exact on-disk version
- [ ] **F-FS-011.02 — Freeze the exact contract for zlfs core**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: persistent create/read/write/list/edit path with exact on-disk version. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-011.03 — Implement/prove: persistent create/read/write/list/edit path with exact on-disk version**
  - Action: For zlfs core, implement or reuse and verify this exact obligation: persistent create/read/write/list/edit path with exact on-disk version. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for persistent create/read/write/list/edit path with exact on-disk version; retain observable state/resource expectations.
- [ ] **F-FS-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to zlfs core: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire zlfs core into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for zlfs core as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-012"></a>
## F-FS-012 — zlfs directories

**Original requirement:** nested directories, types, permissions, rename and iteration consistency

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-012.01 — Reconcile existing zlfs directories**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for zlfs directories. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: nested directories, types, permissions, rename and iteration consistency
- [ ] **F-FS-012.02 — Freeze the exact contract for zlfs directories**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: nested directories, types, permissions, rename and iteration consistency. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-012.03 — Implement/prove: nested directories**
  - Action: For zlfs directories, implement or reuse and verify this exact obligation: nested directories. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for nested directories; retain observable state/resource expectations.
- [ ] **F-FS-012.04 — Implement/prove: types**
  - Action: For zlfs directories, implement or reuse and verify this exact obligation: types. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for types; retain observable state/resource expectations.
- [ ] **F-FS-012.05 — Implement/prove: permissions**
  - Action: For zlfs directories, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-FS-012.06 — Implement/prove: rename and iteration consistency**
  - Action: For zlfs directories, implement or reuse and verify this exact obligation: rename and iteration consistency. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-012.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rename and iteration consistency; retain observable state/resource expectations.
- [ ] **F-FS-012.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to zlfs directories: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-012.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-012.08 — Integrate into the real consumer and runtime route**
  - Action: Wire zlfs directories into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-012.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-012.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for zlfs directories as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-012.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-013"></a>
## F-FS-013 — zlfs links

**Original requirement:** hard/symbolic link policy, cycles, permissions and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-013.01 — Reconcile existing zlfs links**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for zlfs links. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: hard/symbolic link policy, cycles, permissions and recovery
- [ ] **F-FS-013.02 — Freeze the exact contract for zlfs links**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: hard/symbolic link policy, cycles, permissions and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-013.03 — Implement/prove: hard/symbolic link policy**
  - Action: For zlfs links, implement or reuse and verify this exact obligation: hard/symbolic link policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hard/symbolic link policy; retain observable state/resource expectations.
- [ ] **F-FS-013.04 — Implement/prove: cycles**
  - Action: For zlfs links, implement or reuse and verify this exact obligation: cycles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cycles; retain observable state/resource expectations.
- [ ] **F-FS-013.05 — Implement/prove: permissions and recovery**
  - Action: For zlfs links, implement or reuse and verify this exact obligation: permissions and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions and recovery; retain observable state/resource expectations.
- [ ] **F-FS-013.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to zlfs links: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-013.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-013.07 — Integrate into the real consumer and runtime route**
  - Action: Wire zlfs links into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-013.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-013.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for zlfs links as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-013.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-014"></a>
## F-FS-014 — atomic rename/replace

**Original requirement:** crash-consistent namespace transaction and exact failure states

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-014.01 — Reconcile existing atomic rename/replace**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for atomic rename/replace. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: crash-consistent namespace transaction and exact failure states
- [ ] **F-FS-014.02 — Freeze the exact contract for atomic rename/replace**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: crash-consistent namespace transaction and exact failure states. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-014.03 — Implement/prove: crash-consistent namespace transaction and exact failure states**
  - Action: For atomic rename/replace, implement or reuse and verify this exact obligation: crash-consistent namespace transaction and exact failure states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for crash-consistent namespace transaction and exact failure states; retain observable state/resource expectations.
- [ ] **F-FS-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to atomic rename/replace: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire atomic rename/replace into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for atomic rename/replace as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-015"></a>
## F-FS-015 — file growth/truncate

**Original requirement:** checked allocation, sparse policy, zeroing, concurrent maps and rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no explicit truncate API; no sparse files or hole policy; no concurrent mappings; growth can require half the remaining space; no target or physical receipt.

### Execution steps

- [ ] **F-FS-015.01 — Reconcile existing file growth/truncate**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file growth/truncate. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked allocation, sparse policy, zeroing, concurrent maps and rollback
- [ ] **F-FS-015.02 — Freeze the exact contract for file growth/truncate**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked allocation, sparse policy, zeroing, concurrent maps and rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-015.03 — Implement/prove: checked allocation**
  - Action: For file growth/truncate, implement or reuse and verify this exact obligation: checked allocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked allocation; retain observable state/resource expectations.
- [ ] **F-FS-015.04 — Implement/prove: sparse policy**
  - Action: For file growth/truncate, implement or reuse and verify this exact obligation: sparse policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sparse policy; retain observable state/resource expectations.
- [ ] **F-FS-015.05 — Implement/prove: zeroing**
  - Action: For file growth/truncate, implement or reuse and verify this exact obligation: zeroing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-015.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for zeroing; retain observable state/resource expectations.
- [ ] **F-FS-015.06 — Implement/prove: concurrent maps and rollback**
  - Action: For file growth/truncate, implement or reuse and verify this exact obligation: concurrent maps and rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-015.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for concurrent maps and rollback; retain observable state/resource expectations.
- [ ] **F-FS-015.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file growth/truncate: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-015.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-015.08 — Integrate into the real consumer and runtime route**
  - Action: Wire file growth/truncate into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-015.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-015.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file growth/truncate as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-015.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-016"></a>
## F-FS-016 — file permissions/ownership

**Original requirement:** user/group/mode or ACL policy enforced on handles and transitions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-016.01 — Reconcile existing file permissions/ownership**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file permissions/ownership. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: user/group/mode or ACL policy enforced on handles and transitions
- [ ] **F-FS-016.02 — Freeze the exact contract for file permissions/ownership**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: user/group/mode or ACL policy enforced on handles and transitions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-016.03 — Implement/prove: user/group/mode or ACL policy enforced on handles and transitions**
  - Action: For file permissions/ownership, implement or reuse and verify this exact obligation: user/group/mode or ACL policy enforced on handles and transitions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user/group/mode or ACL policy enforced on handles and transitions; retain observable state/resource expectations.
- [ ] **F-FS-016.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file permissions/ownership: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-016.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-016.05 — Integrate into the real consumer and runtime route**
  - Action: Wire file permissions/ownership into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-016.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-016.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file permissions/ownership as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-016.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-017"></a>
## F-FS-017 — timestamps/metadata

**Original requirement:** monotonic/wall-clock semantics, nanosecond/unknown policy and durable update

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-017.01 — Reconcile existing timestamps/metadata**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for timestamps/metadata. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: monotonic/wall-clock semantics, nanosecond/unknown policy and durable update
- [ ] **F-FS-017.02 — Freeze the exact contract for timestamps/metadata**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: monotonic/wall-clock semantics, nanosecond/unknown policy and durable update. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-017.03 — Implement/prove: monotonic/wall-clock semantics**
  - Action: For timestamps/metadata, implement or reuse and verify this exact obligation: monotonic/wall-clock semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for monotonic/wall-clock semantics; retain observable state/resource expectations.
- [ ] **F-FS-017.04 — Implement/prove: nanosecond/unknown policy and durable update**
  - Action: For timestamps/metadata, implement or reuse and verify this exact obligation: nanosecond/unknown policy and durable update. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for nanosecond/unknown policy and durable update; retain observable state/resource expectations.
- [ ] **F-FS-017.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to timestamps/metadata: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-017.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-017.06 — Integrate into the real consumer and runtime route**
  - Action: Wire timestamps/metadata into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-017.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-017.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for timestamps/metadata as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-017.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-018"></a>
## F-FS-018 — extended attributes

**Original requirement:** namespaced bounded metadata with permission and copy/backup behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-018.01 — Reconcile existing extended attributes**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for extended attributes. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: namespaced bounded metadata with permission and copy/backup behavior
- [ ] **F-FS-018.02 — Freeze the exact contract for extended attributes**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: namespaced bounded metadata with permission and copy/backup behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-018.03 — Implement/prove: namespaced bounded metadata with permission and copy/backup behavior**
  - Action: For extended attributes, implement or reuse and verify this exact obligation: namespaced bounded metadata with permission and copy/backup behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for namespaced bounded metadata with permission and copy/backup behavior; retain observable state/resource expectations.
- [ ] **F-FS-018.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to extended attributes: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-018.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-018.05 — Integrate into the real consumer and runtime route**
  - Action: Wire extended attributes into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-018.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-018.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for extended attributes as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-018.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-019"></a>
## F-FS-019 — file locking

**Original requirement:** advisory/mandatory policy, range locks, owner death and deadlock behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-019.01 — Reconcile existing file locking**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file locking. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: advisory/mandatory policy, range locks, owner death and deadlock behavior
- [ ] **F-FS-019.02 — Freeze the exact contract for file locking**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: advisory/mandatory policy, range locks, owner death and deadlock behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-019.03 — Implement/prove: advisory/mandatory policy**
  - Action: For file locking, implement or reuse and verify this exact obligation: advisory/mandatory policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for advisory/mandatory policy; retain observable state/resource expectations.
- [ ] **F-FS-019.04 — Implement/prove: range locks**
  - Action: For file locking, implement or reuse and verify this exact obligation: range locks. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for range locks; retain observable state/resource expectations.
- [ ] **F-FS-019.05 — Implement/prove: owner death and deadlock behavior**
  - Action: For file locking, implement or reuse and verify this exact obligation: owner death and deadlock behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for owner death and deadlock behavior; retain observable state/resource expectations.
- [ ] **F-FS-019.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file locking: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-019.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-019.07 — Integrate into the real consumer and runtime route**
  - Action: Wire file locking into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-019.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-019.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file locking as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-019.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-020"></a>
## F-FS-020 — durability API

**Original requirement:** flush/fsync/barrier meaning from app through filesystem to device

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** NVMe command completion is the lowest stated boundary; no device power-loss cache contract; no per-file fsync or barriers; no current QEMU power-cut or physical durability receipt.

### Execution steps

- [ ] **F-FS-020.01 — Reconcile existing durability API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for durability API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: flush/fsync/barrier meaning from app through filesystem to device
- [ ] **F-FS-020.02 — Freeze the exact contract for durability API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: flush/fsync/barrier meaning from app through filesystem to device. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-020.03 — Implement/prove: flush/fsync/barrier meaning from app through filesystem to device**
  - Action: For durability API, implement or reuse and verify this exact obligation: flush/fsync/barrier meaning from app through filesystem to device. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flush/fsync/barrier meaning from app through filesystem to device; retain observable state/resource expectations.
- [ ] **F-FS-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to durability API: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire durability API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for durability API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-021"></a>
## F-FS-021 — journal/transaction log

**Original requirement:** checksummed replay, ordering, torn-write and power-cut matrix

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** transaction scope is one flat-directory mutation; there is no append journal or replay log; generation wrap policy is narrow; no multi-object transaction; no current target or physical power-cut receipt.

### Execution steps

- [ ] **F-FS-021.01 — Reconcile existing journal/transaction log**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for journal/transaction log. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checksummed replay, ordering, torn-write and power-cut matrix
- [ ] **F-FS-021.02 — Freeze the exact contract for journal/transaction log**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checksummed replay, ordering, torn-write and power-cut matrix. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-021.03 — Implement/prove: checksummed replay**
  - Action: For journal/transaction log, implement or reuse and verify this exact obligation: checksummed replay. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checksummed replay; retain observable state/resource expectations.
- [ ] **F-FS-021.04 — Implement/prove: ordering**
  - Action: For journal/transaction log, implement or reuse and verify this exact obligation: ordering. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ordering; retain observable state/resource expectations.
- [ ] **F-FS-021.05 — Implement/prove: torn-write and power-cut matrix**
  - Action: For journal/transaction log, implement or reuse and verify this exact obligation: torn-write and power-cut matrix. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for torn-write and power-cut matrix; retain observable state/resource expectations.
- [ ] **F-FS-021.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to journal/transaction log: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-021.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-021.07 — Integrate into the real consumer and runtime route**
  - Action: Wire journal/transaction log into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-021.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-021.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for journal/transaction log as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-021.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-022"></a>
## F-FS-022 — free-space accounting

**Original requirement:** reserved blocks, disk-full admission, quotas, reclamation and truthful UI

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-022.01 — Reconcile existing free-space accounting**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for free-space accounting. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: reserved blocks, disk-full admission, quotas, reclamation and truthful UI
- [ ] **F-FS-022.02 — Freeze the exact contract for free-space accounting**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: reserved blocks, disk-full admission, quotas, reclamation and truthful UI. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-022.03 — Implement/prove: reserved blocks**
  - Action: For free-space accounting, implement or reuse and verify this exact obligation: reserved blocks. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reserved blocks; retain observable state/resource expectations.
- [ ] **F-FS-022.04 — Implement/prove: disk-full admission**
  - Action: For free-space accounting, implement or reuse and verify this exact obligation: disk-full admission. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for disk-full admission; retain observable state/resource expectations.
- [ ] **F-FS-022.05 — Implement/prove: quotas**
  - Action: For free-space accounting, implement or reuse and verify this exact obligation: quotas. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quotas; retain observable state/resource expectations.
- [ ] **F-FS-022.06 — Implement/prove: reclamation and truthful UI**
  - Action: For free-space accounting, implement or reuse and verify this exact obligation: reclamation and truthful UI. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reclamation and truthful UI; retain observable state/resource expectations.
- [ ] **F-FS-022.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to free-space accounting: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-022.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-022.08 — Integrate into the real consumer and runtime route**
  - Action: Wire free-space accounting into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-022.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-022.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for free-space accounting as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-022.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-023"></a>
## F-FS-023 — filesystem checker

**Original requirement:** read-only diagnose first, repair plan, backup/restore and durable receipt

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-023.01 — Reconcile existing filesystem checker**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for filesystem checker. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: read-only diagnose first, repair plan, backup/restore and durable receipt
- [ ] **F-FS-023.02 — Freeze the exact contract for filesystem checker**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: read-only diagnose first, repair plan, backup/restore and durable receipt. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-023.03 — Implement/prove: read-only diagnose first**
  - Action: For filesystem checker, implement or reuse and verify this exact obligation: read-only diagnose first. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for read-only diagnose first; retain observable state/resource expectations.
- [ ] **F-FS-023.04 — Implement/prove: repair plan**
  - Action: For filesystem checker, implement or reuse and verify this exact obligation: repair plan. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for repair plan; retain observable state/resource expectations.
- [ ] **F-FS-023.05 — Implement/prove: backup/restore and durable receipt**
  - Action: For filesystem checker, implement or reuse and verify this exact obligation: backup/restore and durable receipt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for backup/restore and durable receipt; retain observable state/resource expectations.
- [ ] **F-FS-023.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to filesystem checker: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-023.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-023.07 — Integrate into the real consumer and runtime route**
  - Action: Wire filesystem checker into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-023.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-023.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for filesystem checker as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-023.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-024"></a>
## F-FS-024 — corrupt-media admission

**Original requirement:** length/checksum/range/cycle validation before mount and bounded refusal

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no fuzzed corpus receipt; no cycle/link graph exists to validate; mount limits are zlfs-specific; no current QEMU or physical corrupt-media receipt.

### Execution steps

- [ ] **F-FS-024.01 — Reconcile existing corrupt-media admission**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for corrupt-media admission. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: length/checksum/range/cycle validation before mount and bounded refusal
- [ ] **F-FS-024.02 — Freeze the exact contract for corrupt-media admission**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: length/checksum/range/cycle validation before mount and bounded refusal. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-024.03 — Implement/prove: length/checksum/range/cycle validation before mount and bounded refusal**
  - Action: For corrupt-media admission, implement or reuse and verify this exact obligation: length/checksum/range/cycle validation before mount and bounded refusal. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for length/checksum/range/cycle validation before mount and bounded refusal; retain observable state/resource expectations.
- [ ] **F-FS-024.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to corrupt-media admission: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-024.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-024.05 — Integrate into the real consumer and runtime route**
  - Action: Wire corrupt-media admission into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-024.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-024.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for corrupt-media admission as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-024.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-025"></a>
## F-FS-025 — removable-media lifecycle

**Original requirement:** consent, automount policy, safe eject, busy state, detach and stale handles

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-025.01 — Reconcile existing removable-media lifecycle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for removable-media lifecycle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: consent, automount policy, safe eject, busy state, detach and stale handles
- [ ] **F-FS-025.02 — Freeze the exact contract for removable-media lifecycle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: consent, automount policy, safe eject, busy state, detach and stale handles. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-025.03 — Implement/prove: consent**
  - Action: For removable-media lifecycle, implement or reuse and verify this exact obligation: consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for consent; retain observable state/resource expectations.
- [ ] **F-FS-025.04 — Implement/prove: automount policy**
  - Action: For removable-media lifecycle, implement or reuse and verify this exact obligation: automount policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for automount policy; retain observable state/resource expectations.
- [ ] **F-FS-025.05 — Implement/prove: safe eject**
  - Action: For removable-media lifecycle, implement or reuse and verify this exact obligation: safe eject. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe eject; retain observable state/resource expectations.
- [ ] **F-FS-025.06 — Implement/prove: busy state**
  - Action: For removable-media lifecycle, implement or reuse and verify this exact obligation: busy state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-025.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for busy state; retain observable state/resource expectations.
- [ ] **F-FS-025.07 — Implement/prove: detach and stale handles**
  - Action: For removable-media lifecycle, implement or reuse and verify this exact obligation: detach and stale handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-025.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for detach and stale handles; retain observable state/resource expectations.
- [ ] **F-FS-025.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to removable-media lifecycle: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-025.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-025.09 — Integrate into the real consumer and runtime route**
  - Action: Wire removable-media lifecycle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-025.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-025.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for removable-media lifecycle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-025.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-026"></a>
## F-FS-026 — encrypted volume

**Original requirement:** key lifecycle, metadata protection, unlock/recovery and power-loss behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-026.01 — Reconcile existing encrypted volume**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for encrypted volume. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: key lifecycle, metadata protection, unlock/recovery and power-loss behavior
- [ ] **F-FS-026.02 — Freeze the exact contract for encrypted volume**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: key lifecycle, metadata protection, unlock/recovery and power-loss behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-026.03 — Implement/prove: key lifecycle**
  - Action: For encrypted volume, implement or reuse and verify this exact obligation: key lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for key lifecycle; retain observable state/resource expectations.
- [ ] **F-FS-026.04 — Implement/prove: metadata protection**
  - Action: For encrypted volume, implement or reuse and verify this exact obligation: metadata protection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for metadata protection; retain observable state/resource expectations.
- [ ] **F-FS-026.05 — Implement/prove: unlock/recovery and power-loss behavior**
  - Action: For encrypted volume, implement or reuse and verify this exact obligation: unlock/recovery and power-loss behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unlock/recovery and power-loss behavior; retain observable state/resource expectations.
- [ ] **F-FS-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to encrypted volume: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire encrypted volume into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for encrypted volume as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-027"></a>
## F-FS-027 — snapshot/versioning

**Original requirement:** bounded snapshots, copy-on-write accounting, restore and retention

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-027.01 — Reconcile existing snapshot/versioning**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for snapshot/versioning. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded snapshots, copy-on-write accounting, restore and retention
- [ ] **F-FS-027.02 — Freeze the exact contract for snapshot/versioning**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded snapshots, copy-on-write accounting, restore and retention. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-027.03 — Implement/prove: bounded snapshots**
  - Action: For snapshot/versioning, implement or reuse and verify this exact obligation: bounded snapshots. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded snapshots; retain observable state/resource expectations.
- [ ] **F-FS-027.04 — Implement/prove: copy-on-write accounting**
  - Action: For snapshot/versioning, implement or reuse and verify this exact obligation: copy-on-write accounting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for copy-on-write accounting; retain observable state/resource expectations.
- [ ] **F-FS-027.05 — Implement/prove: restore and retention**
  - Action: For snapshot/versioning, implement or reuse and verify this exact obligation: restore and retention. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for restore and retention; retain observable state/resource expectations.
- [ ] **F-FS-027.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to snapshot/versioning: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-027.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-027.07 — Integrate into the real consumer and runtime route**
  - Action: Wire snapshot/versioning into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-027.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-027.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for snapshot/versioning as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-027.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-028"></a>
## F-FS-028 — backup/restore

**Original requirement:** manifest, incremental/full, verification, encryption, target confirmation and drill

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-028.01 — Reconcile existing backup/restore**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for backup/restore. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: manifest, incremental/full, verification, encryption, target confirmation and drill
- [ ] **F-FS-028.02 — Freeze the exact contract for backup/restore**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: manifest, incremental/full, verification, encryption, target confirmation and drill. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-028.03 — Implement/prove: manifest**
  - Action: For backup/restore, implement or reuse and verify this exact obligation: manifest. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for manifest; retain observable state/resource expectations.
- [ ] **F-FS-028.04 — Implement/prove: incremental/full**
  - Action: For backup/restore, implement or reuse and verify this exact obligation: incremental/full. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for incremental/full; retain observable state/resource expectations.
- [ ] **F-FS-028.05 — Implement/prove: verification**
  - Action: For backup/restore, implement or reuse and verify this exact obligation: verification. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for verification; retain observable state/resource expectations.
- [ ] **F-FS-028.06 — Implement/prove: encryption**
  - Action: For backup/restore, implement or reuse and verify this exact obligation: encryption. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-028.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for encryption; retain observable state/resource expectations.
- [ ] **F-FS-028.07 — Implement/prove: target confirmation and drill**
  - Action: For backup/restore, implement or reuse and verify this exact obligation: target confirmation and drill. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-028.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for target confirmation and drill; retain observable state/resource expectations.
- [ ] **F-FS-028.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to backup/restore: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-028.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-028.09 — Integrate into the real consumer and runtime route**
  - Action: Wire backup/restore into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-028.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-028.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for backup/restore as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-028.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-029"></a>
## F-FS-029 — trash/recovery

**Original requirement:** per-user reversible delete, cross-volume behavior, retention and secure removal

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-029.01 — Reconcile existing trash/recovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for trash/recovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-user reversible delete, cross-volume behavior, retention and secure removal
- [ ] **F-FS-029.02 — Freeze the exact contract for trash/recovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-user reversible delete, cross-volume behavior, retention and secure removal. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-029.03 — Implement/prove: per-user reversible delete**
  - Action: For trash/recovery, implement or reuse and verify this exact obligation: per-user reversible delete. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-user reversible delete; retain observable state/resource expectations.
- [ ] **F-FS-029.04 — Implement/prove: cross-volume behavior**
  - Action: For trash/recovery, implement or reuse and verify this exact obligation: cross-volume behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cross-volume behavior; retain observable state/resource expectations.
- [ ] **F-FS-029.05 — Implement/prove: retention and secure removal**
  - Action: For trash/recovery, implement or reuse and verify this exact obligation: retention and secure removal. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-029.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retention and secure removal; retain observable state/resource expectations.
- [ ] **F-FS-029.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to trash/recovery: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-029.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-029.07 — Integrate into the real consumer and runtime route**
  - Action: Wire trash/recovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-029.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-029.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for trash/recovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-029.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-030"></a>
## F-FS-030 — package format

**Original requirement:** signed manifest, complete file list, sizes, target ABI/arch, dependencies, conflicts, licenses

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-030.01 — Reconcile existing package format**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for package format. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed manifest, complete file list, sizes, target ABI/arch, dependencies, conflicts, licenses
- [ ] **F-FS-030.02 — Freeze the exact contract for package format**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed manifest, complete file list, sizes, target ABI/arch, dependencies, conflicts, licenses. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-030.03 — Implement/prove: signed manifest**
  - Action: For package format, implement or reuse and verify this exact obligation: signed manifest. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed manifest; retain observable state/resource expectations.
- [ ] **F-FS-030.04 — Implement/prove: complete file list**
  - Action: For package format, implement or reuse and verify this exact obligation: complete file list. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for complete file list; retain observable state/resource expectations.
- [ ] **F-FS-030.05 — Implement/prove: sizes**
  - Action: For package format, implement or reuse and verify this exact obligation: sizes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-030.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sizes; retain observable state/resource expectations.
- [ ] **F-FS-030.06 — Implement/prove: target ABI/arch**
  - Action: For package format, implement or reuse and verify this exact obligation: target ABI/arch. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-030.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for target ABI/arch; retain observable state/resource expectations.
- [ ] **F-FS-030.07 — Implement/prove: dependencies**
  - Action: For package format, implement or reuse and verify this exact obligation: dependencies. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-030.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dependencies; retain observable state/resource expectations.
- [ ] **F-FS-030.08 — Implement/prove: conflicts**
  - Action: For package format, implement or reuse and verify this exact obligation: conflicts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-030.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conflicts; retain observable state/resource expectations.
- [ ] **F-FS-030.09 — Implement/prove: licenses**
  - Action: For package format, implement or reuse and verify this exact obligation: licenses. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-030.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for licenses; retain observable state/resource expectations.
- [ ] **F-FS-030.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to package format: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-030.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-030.11 — Integrate into the real consumer and runtime route**
  - Action: Wire package format into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-030.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-030.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for package format as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-030.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-031"></a>
## F-FS-031 — atomic package install

**Original requirement:** stage, verify every object/script/grant, publish once, reboot recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-031.01 — Reconcile existing atomic package install**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for atomic package install. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stage, verify every object/script/grant, publish once, reboot recovery
- [ ] **F-FS-031.02 — Freeze the exact contract for atomic package install**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stage, verify every object/script/grant, publish once, reboot recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-031.03 — Implement/prove: stage**
  - Action: For atomic package install, implement or reuse and verify this exact obligation: stage. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stage; retain observable state/resource expectations.
- [ ] **F-FS-031.04 — Implement/prove: verify every object/script/grant**
  - Action: For atomic package install, implement or reuse and verify this exact obligation: verify every object/script/grant. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for verify every object/script/grant; retain observable state/resource expectations.
- [ ] **F-FS-031.05 — Implement/prove: publish once**
  - Action: For atomic package install, implement or reuse and verify this exact obligation: publish once. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-031.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for publish once; retain observable state/resource expectations.
- [ ] **F-FS-031.06 — Implement/prove: reboot recovery**
  - Action: For atomic package install, implement or reuse and verify this exact obligation: reboot recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-031.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reboot recovery; retain observable state/resource expectations.
- [ ] **F-FS-031.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to atomic package install: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-031.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-031.08 — Integrate into the real consumer and runtime route**
  - Action: Wire atomic package install into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-031.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-031.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for atomic package install as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-031.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-032"></a>
## F-FS-032 — atomic package uninstall

**Original requirement:** stop/revoke/deregister/unpublish before deletion with explicit user-data policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-032.01 — Reconcile existing atomic package uninstall**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for atomic package uninstall. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stop/revoke/deregister/unpublish before deletion with explicit user-data policy
- [ ] **F-FS-032.02 — Freeze the exact contract for atomic package uninstall**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stop/revoke/deregister/unpublish before deletion with explicit user-data policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-032.03 — Implement/prove: stop/revoke/deregister/unpublish before deletion with explicit user-data policy**
  - Action: For atomic package uninstall, implement or reuse and verify this exact obligation: stop/revoke/deregister/unpublish before deletion with explicit user-data policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stop/revoke/deregister/unpublish before deletion with explicit user-data policy; retain observable state/resource expectations.
- [ ] **F-FS-032.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to atomic package uninstall: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-032.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-032.05 — Integrate into the real consumer and runtime route**
  - Action: Wire atomic package uninstall into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-032.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-032.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for atomic package uninstall as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-032.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-033"></a>
## F-FS-033 — package dependency solver

**Original requirement:** versions, conflicts, alternatives, architecture and reproducible decision receipt

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-033.01 — Reconcile existing package dependency solver**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for package dependency solver. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: versions, conflicts, alternatives, architecture and reproducible decision receipt
- [ ] **F-FS-033.02 — Freeze the exact contract for package dependency solver**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: versions, conflicts, alternatives, architecture and reproducible decision receipt. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-033.03 — Implement/prove: versions**
  - Action: For package dependency solver, implement or reuse and verify this exact obligation: versions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versions; retain observable state/resource expectations.
- [ ] **F-FS-033.04 — Implement/prove: conflicts**
  - Action: For package dependency solver, implement or reuse and verify this exact obligation: conflicts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conflicts; retain observable state/resource expectations.
- [ ] **F-FS-033.05 — Implement/prove: alternatives**
  - Action: For package dependency solver, implement or reuse and verify this exact obligation: alternatives. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for alternatives; retain observable state/resource expectations.
- [ ] **F-FS-033.06 — Implement/prove: architecture and reproducible decision receipt**
  - Action: For package dependency solver, implement or reuse and verify this exact obligation: architecture and reproducible decision receipt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-033.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for architecture and reproducible decision receipt; retain observable state/resource expectations.
- [ ] **F-FS-033.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to package dependency solver: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-033.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-033.08 — Integrate into the real consumer and runtime route**
  - Action: Wire package dependency solver into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-033.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-033.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for package dependency solver as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-033.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-034"></a>
## F-FS-034 — immutable system generation

**Original requirement:** content-addressed base, writable user state, atomic switch and rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-034.01 — Reconcile existing immutable system generation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for immutable system generation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: content-addressed base, writable user state, atomic switch and rollback
- [ ] **F-FS-034.02 — Freeze the exact contract for immutable system generation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: content-addressed base, writable user state, atomic switch and rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-034.03 — Implement/prove: content-addressed base**
  - Action: For immutable system generation, implement or reuse and verify this exact obligation: content-addressed base. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for content-addressed base; retain observable state/resource expectations.
- [ ] **F-FS-034.04 — Implement/prove: writable user state**
  - Action: For immutable system generation, implement or reuse and verify this exact obligation: writable user state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-034.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for writable user state; retain observable state/resource expectations.
- [ ] **F-FS-034.05 — Implement/prove: atomic switch and rollback**
  - Action: For immutable system generation, implement or reuse and verify this exact obligation: atomic switch and rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-034.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atomic switch and rollback; retain observable state/resource expectations.
- [ ] **F-FS-034.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to immutable system generation: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-034.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-034.07 — Integrate into the real consumer and runtime route**
  - Action: Wire immutable system generation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-034.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-034.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for immutable system generation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-034.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-035"></a>
## F-FS-035 — update service

**Original requirement:** signed metadata, download resume, staging, health gate, rollback and notices

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-035.01 — Reconcile existing update service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for update service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed metadata, download resume, staging, health gate, rollback and notices
- [ ] **F-FS-035.02 — Freeze the exact contract for update service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed metadata, download resume, staging, health gate, rollback and notices. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-035.03 — Implement/prove: signed metadata**
  - Action: For update service, implement or reuse and verify this exact obligation: signed metadata. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed metadata; retain observable state/resource expectations.
- [ ] **F-FS-035.04 — Implement/prove: download resume**
  - Action: For update service, implement or reuse and verify this exact obligation: download resume. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-035.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for download resume; retain observable state/resource expectations.
- [ ] **F-FS-035.05 — Implement/prove: staging**
  - Action: For update service, implement or reuse and verify this exact obligation: staging. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-035.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for staging; retain observable state/resource expectations.
- [ ] **F-FS-035.06 — Implement/prove: health gate**
  - Action: For update service, implement or reuse and verify this exact obligation: health gate. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-035.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for health gate; retain observable state/resource expectations.
- [ ] **F-FS-035.07 — Implement/prove: rollback and notices**
  - Action: For update service, implement or reuse and verify this exact obligation: rollback and notices. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-035.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rollback and notices; retain observable state/resource expectations.
- [ ] **F-FS-035.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to update service: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-035.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-035.09 — Integrate into the real consumer and runtime route**
  - Action: Wire update service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-035.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-035.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for update service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-035.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-036"></a>
## F-FS-036 — firmware store/update

**Original requirement:** origin/license/device match/signature, rollback and failure-safe flash policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-036.01 — Reconcile existing firmware store/update**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for firmware store/update. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: origin/license/device match/signature, rollback and failure-safe flash policy
- [ ] **F-FS-036.02 — Freeze the exact contract for firmware store/update**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: origin/license/device match/signature, rollback and failure-safe flash policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-036.03 — Implement/prove: origin/license/device match/signature**
  - Action: For firmware store/update, implement or reuse and verify this exact obligation: origin/license/device match/signature. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for origin/license/device match/signature; retain observable state/resource expectations.
- [ ] **F-FS-036.04 — Implement/prove: rollback and failure-safe flash policy**
  - Action: For firmware store/update, implement or reuse and verify this exact obligation: rollback and failure-safe flash policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-036.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rollback and failure-safe flash policy; retain observable state/resource expectations.
- [ ] **F-FS-036.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to firmware store/update: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-036.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-036.06 — Integrate into the real consumer and runtime route**
  - Action: Wire firmware store/update into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-036.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-036.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for firmware store/update as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-036.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-037"></a>
## F-FS-037 — package cache/cleanup

**Original requirement:** byte budgets, pinning, generation reachability, safe concurrent cleanup

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-037.01 — Reconcile existing package cache/cleanup**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for package cache/cleanup. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: byte budgets, pinning, generation reachability, safe concurrent cleanup
- [ ] **F-FS-037.02 — Freeze the exact contract for package cache/cleanup**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: byte budgets, pinning, generation reachability, safe concurrent cleanup. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-037.03 — Implement/prove: byte budgets**
  - Action: For package cache/cleanup, implement or reuse and verify this exact obligation: byte budgets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for byte budgets; retain observable state/resource expectations.
- [ ] **F-FS-037.04 — Implement/prove: pinning**
  - Action: For package cache/cleanup, implement or reuse and verify this exact obligation: pinning. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-037.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pinning; retain observable state/resource expectations.
- [ ] **F-FS-037.05 — Implement/prove: generation reachability**
  - Action: For package cache/cleanup, implement or reuse and verify this exact obligation: generation reachability. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-037.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generation reachability; retain observable state/resource expectations.
- [ ] **F-FS-037.06 — Implement/prove: safe concurrent cleanup**
  - Action: For package cache/cleanup, implement or reuse and verify this exact obligation: safe concurrent cleanup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-037.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe concurrent cleanup; retain observable state/resource expectations.
- [ ] **F-FS-037.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to package cache/cleanup: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-037.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-037.08 — Integrate into the real consumer and runtime route**
  - Action: Wire package cache/cleanup into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-037.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-037.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for package cache/cleanup as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-037.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-038"></a>
## F-FS-038 — file indexing/search

**Original requirement:** per-user permissions, incremental updates, cancellation, privacy and rebuild

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-038.01 — Reconcile existing file indexing/search**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file indexing/search. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-user permissions, incremental updates, cancellation, privacy and rebuild
- [ ] **F-FS-038.02 — Freeze the exact contract for file indexing/search**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-user permissions, incremental updates, cancellation, privacy and rebuild. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-038.03 — Implement/prove: per-user permissions**
  - Action: For file indexing/search, implement or reuse and verify this exact obligation: per-user permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-user permissions; retain observable state/resource expectations.
- [ ] **F-FS-038.04 — Implement/prove: incremental updates**
  - Action: For file indexing/search, implement or reuse and verify this exact obligation: incremental updates. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-038.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for incremental updates; retain observable state/resource expectations.
- [ ] **F-FS-038.05 — Implement/prove: cancellation**
  - Action: For file indexing/search, implement or reuse and verify this exact obligation: cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-038.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancellation; retain observable state/resource expectations.
- [ ] **F-FS-038.06 — Implement/prove: privacy and rebuild**
  - Action: For file indexing/search, implement or reuse and verify this exact obligation: privacy and rebuild. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-038.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and rebuild; retain observable state/resource expectations.
- [ ] **F-FS-038.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file indexing/search: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-038.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-038.08 — Integrate into the real consumer and runtime route**
  - Action: Wire file indexing/search into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-038.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-038.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file indexing/search as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-038.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-039"></a>
## F-FS-039 — content type/association

**Original requirement:** MIME sniff/extension policy, defaults, open-with, provenance and safe preview

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-039.01 — Reconcile existing content type/association**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for content type/association. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: MIME sniff/extension policy, defaults, open-with, provenance and safe preview
- [ ] **F-FS-039.02 — Freeze the exact contract for content type/association**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: MIME sniff/extension policy, defaults, open-with, provenance and safe preview. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-039.03 — Implement/prove: MIME sniff/extension policy**
  - Action: For content type/association, implement or reuse and verify this exact obligation: MIME sniff/extension policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for MIME sniff/extension policy; retain observable state/resource expectations.
- [ ] **F-FS-039.04 — Implement/prove: defaults**
  - Action: For content type/association, implement or reuse and verify this exact obligation: defaults. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-039.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for defaults; retain observable state/resource expectations.
- [ ] **F-FS-039.05 — Implement/prove: open-with**
  - Action: For content type/association, implement or reuse and verify this exact obligation: open-with. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-039.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for open-with; retain observable state/resource expectations.
- [ ] **F-FS-039.06 — Implement/prove: provenance and safe preview**
  - Action: For content type/association, implement or reuse and verify this exact obligation: provenance and safe preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-039.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance and safe preview; retain observable state/resource expectations.
- [ ] **F-FS-039.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to content type/association: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-039.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-039.08 — Integrate into the real consumer and runtime route**
  - Action: Wire content type/association into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-039.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-039.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for content type/association as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-039.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fs-040"></a>
## F-FS-040 — file preview/thumbnail

**Original requirement:** restricted decoder, size/time quotas, cache invalidation and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FS-040.01 — Reconcile existing file preview/thumbnail**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file preview/thumbnail. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: restricted decoder, size/time quotas, cache invalidation and privacy
- [ ] **F-FS-040.02 — Freeze the exact contract for file preview/thumbnail**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: restricted decoder, size/time quotas, cache invalidation and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FS-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FS-040.03 — Implement/prove: restricted decoder**
  - Action: For file preview/thumbnail, implement or reuse and verify this exact obligation: restricted decoder. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for restricted decoder; retain observable state/resource expectations.
- [ ] **F-FS-040.04 — Implement/prove: size/time quotas**
  - Action: For file preview/thumbnail, implement or reuse and verify this exact obligation: size/time quotas. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-040.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for size/time quotas; retain observable state/resource expectations.
- [ ] **F-FS-040.05 — Implement/prove: cache invalidation and privacy**
  - Action: For file preview/thumbnail, implement or reuse and verify this exact obligation: cache invalidation and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FS-040.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cache invalidation and privacy; retain observable state/resource expectations.
- [ ] **F-FS-040.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file preview/thumbnail: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FS-040.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FS-040.07 — Integrate into the real consumer and runtime route**
  - Action: Wire file preview/thumbnail into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FS-040.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FS-040.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file preview/thumbnail as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FS-040.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p2-1"></a>
## C-P2.1 — common block provider

**Original requirement:** common block provider

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 135.

### Preserved original contract

- **Dependencies/current/provenance:** current NVMe/xHCI/ZLLOG paths; Zinnia driver seams, Astral storage questions; reject controller-specific filesystem calls.
- **I/O and state:** provider handle, LBA/count, bounded buffer and operation in; capacity/block-size/result/residue out; provider state follows service lifecycle plus `Online -> Quiescing -> Offline`.
- **Invariants/failure:** checked range arithmetic; exact buffer length; no request beyond capacity; timeout/cancel/reset explicit; unsupported trim returns typed error.
- **Deterministic proof:** fake provider for zero/end/overflow/short I/O, timeout, retry, reset, flush ordering and hot removal.
- **Target proof:** QEMU NVMe/USB where modelled; physical xHCI ZLLOG and NVMe named-file operations through same suite.
- **Receipt/removal:** provider/version/geometry and operation trace; adapters retain old direct functions; remove controller-specific upper calls after caller inventory is zero.

### Execution steps

- [ ] **C-P2.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P2.1.02 — Resolve this contract's exact dependencies**
  - Action: current NVMe/xHCI/ZLLOG paths; Zinnia driver seams, Astral storage questions; reject controller-specific filesystem calls. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P2.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P2.1.03 — I/O and state — common block provider**
  - Action: provider handle, LBA/count, bounded buffer and operation in; capacity/block-size/result/residue out; provider state follows service lifecycle plus `Online -> Quiescing -> Offline`.
  - Requires: C-P2.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P2.1.
- [ ] **C-P2.1.04 — Invariants/failure — common block provider**
  - Action: checked range arithmetic; exact buffer length; no request beyond capacity; timeout/cancel/reset explicit; unsupported trim returns typed error.
  - Requires: C-P2.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P2.1.
- [ ] **C-P2.1.05 — Deterministic proof — common block provider**
  - Action: fake provider for zero/end/overflow/short I/O, timeout, retry, reset, flush ordering and hot removal.
  - Requires: C-P2.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P2.1.
- [ ] **C-P2.1.06 — Target proof — common block provider**
  - Action: QEMU NVMe/USB where modelled; physical xHCI ZLLOG and NVMe named-file operations through same suite.
  - Requires: C-P2.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P2.1.
- [ ] **C-P2.1.07 — Receipt/removal — common block provider**
  - Action: provider/version/geometry and operation trace; adapters retain old direct functions; remove controller-specific upper calls after caller inventory is zero.
  - Requires: C-P2.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P2.1.
- [ ] **C-P2.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P2.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P2.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p2-2"></a>
## C-P2.2 — bounded async request/completion queue

**Original requirement:** bounded async request/completion queue

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 144.

### Preserved original contract

- **Dependencies/current/provenance:** P2.1 and scheduler clocks; NexiOS bounded queues, Brook host tests; reject unbounded waits and I/O in IRQ/frame context.
- **I/O and state:** admitted request with deadline/cancel token in; completion handle/result out; request machine uses universal states.
- **Invariants/failure:** fixed capacity/ownership; IRQ only records completion; cancellation is terminal/idempotent; full queue returns backpressure; late completions cannot touch reused requests.
- **Deterministic proof:** controlled completions, full/one-over, cancel races, reset while queued/running, duplicate/late completion, wraparound IDs.
- **Target proof:** QEMU save while dragging; physical journal shows no block work in input/paint/IRQ and bounded latency counters.
- **Receipt/removal:** queue high-water/latencies/outcomes; sync adapter rollback; remove direct synchronous UI writes when trace caller count is zero.

### Execution steps

- [ ] **C-P2.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P2.2.02 — Resolve this contract's exact dependencies**
  - Action: P2.1 and scheduler clocks; NexiOS bounded queues, Brook host tests; reject unbounded waits and I/O in IRQ/frame context. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P2.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P2.2.03 — I/O and state — bounded async request/completion queue**
  - Action: admitted request with deadline/cancel token in; completion handle/result out; request machine uses universal states.
  - Requires: C-P2.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P2.2.
- [ ] **C-P2.2.04 — Invariants/failure — bounded async request/completion queue**
  - Action: fixed capacity/ownership; IRQ only records completion; cancellation is terminal/idempotent; full queue returns backpressure; late completions cannot touch reused requests.
  - Requires: C-P2.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P2.2.
- [ ] **C-P2.2.05 — Deterministic proof — bounded async request/completion queue**
  - Action: controlled completions, full/one-over, cancel races, reset while queued/running, duplicate/late completion, wraparound IDs.
  - Requires: C-P2.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P2.2.
- [ ] **C-P2.2.06 — Target proof — bounded async request/completion queue**
  - Action: QEMU save while dragging; physical journal shows no block work in input/paint/IRQ and bounded latency counters.
  - Requires: C-P2.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P2.2.
- [ ] **C-P2.2.07 — Receipt/removal — bounded async request/completion queue**
  - Action: queue high-water/latencies/outcomes; sync adapter rollback; remove direct synchronous UI writes when trace caller count is zero.
  - Requires: C-P2.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P2.2.
- [ ] **C-P2.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P2.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P2.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p2-3"></a>
## C-P2.3 — page/block cache and writeback worker

**Original requirement:** page/block cache and writeback worker

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 153.

### Preserved original contract

- **Dependencies/current/provenance:** P2.2 and heap/accounting; Astral page-cache boundary; reject acknowledgment that implies durability before flush.
- **I/O and state:** read/write pages, dirty policy and explicit sync in; cache hit/data or completion/failure out; page `Absent -> Clean -> Dirty -> Writeback -> Clean|Error`.
- **Invariants/failure:** bounded bytes; pin/ref counts; dirty data not evicted; explicit Save waits for metadata durability; failed provider retains dirty/error state.
- **Deterministic proof:** hit/miss/eviction, dirty-pressure, concurrent reads/writes, writeback error/retry, forced sync, shutdown drain, no-frame-allocation assertion.
- **Target proof:** QEMU fault-injected storage during desktop workload; hardware queue/cache telemetry in ZLLOG.
- **Receipt/removal:** hit/miss/dirty/flush metrics and disk hash; disable cache uses P2.1 sync provider; no removal of safe sync fallback.

### Execution steps

- [ ] **C-P2.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P2.3.02 — Resolve this contract's exact dependencies**
  - Action: P2.2 and heap/accounting; Astral page-cache boundary; reject acknowledgment that implies durability before flush. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P2.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P2.3.03 — I/O and state — page/block cache and writeback worker**
  - Action: read/write pages, dirty policy and explicit sync in; cache hit/data or completion/failure out; page `Absent -> Clean -> Dirty -> Writeback -> Clean|Error`.
  - Requires: C-P2.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P2.3.
- [ ] **C-P2.3.04 — Invariants/failure — page/block cache and writeback worker**
  - Action: bounded bytes; pin/ref counts; dirty data not evicted; explicit Save waits for metadata durability; failed provider retains dirty/error state.
  - Requires: C-P2.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P2.3.
- [ ] **C-P2.3.05 — Deterministic proof — page/block cache and writeback worker**
  - Action: hit/miss/eviction, dirty-pressure, concurrent reads/writes, writeback error/retry, forced sync, shutdown drain, no-frame-allocation assertion.
  - Requires: C-P2.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P2.3.
- [ ] **C-P2.3.06 — Target proof — page/block cache and writeback worker**
  - Action: QEMU fault-injected storage during desktop workload; hardware queue/cache telemetry in ZLLOG.
  - Requires: C-P2.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P2.3.
- [ ] **C-P2.3.07 — Receipt/removal — page/block cache and writeback worker**
  - Action: hit/miss/dirty/flush metrics and disk hash; disable cache uses P2.1 sync provider; no removal of safe sync fallback.
  - Requires: C-P2.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P2.3.
- [ ] **C-P2.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P2.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P2.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p2-4"></a>
## C-P2.4 — zlfs format and crash transaction

**Original requirement:** zlfs format and crash transaction

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 162.

### Preserved original contract

- **Dependencies/current/provenance:** P2.1-P2.3 and existing zlfs/fstest/probe-files; Brook tests and Mollen immutable packaging; reject mYOS-style monolithic unversioned state.
- **I/O and state:** versioned volume and file transaction in; committed generation/file handle out; transaction `Open -> DataWritten -> DataFlushed -> MetadataWritten -> MetadataFlushed -> Published`.
- **Invariants/failure:** CRC/version/range checks; data before metadata; old generation remains valid until publish; disk-full never truncates existing file; names/lengths bounded.
- **Deterministic proof:** power cut after every write, corrupt super/metadata/data, full disk, rename/write race, stale handle, unsupported version, model/reference comparison.
- **Target proof:** separate QEMU cold boots reopen exact bytes on NVMe and USB-backed test media; physical safe target validates commit/recovery.
- **Receipt/removal:** pre/post volume digest, cut point and recovered generation; read-only mount rollback; old flat format removed only after migration/import and recovery proof.

### Execution steps

- [ ] **C-P2.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P2.4.02 — Resolve this contract's exact dependencies**
  - Action: P2.1-P2.3 and existing zlfs/fstest/probe-files; Brook tests and Mollen immutable packaging; reject mYOS-style monolithic unversioned state. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P2.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P2.4.03 — I/O and state — zlfs format and crash transaction**
  - Action: versioned volume and file transaction in; committed generation/file handle out; transaction `Open -> DataWritten -> DataFlushed -> MetadataWritten -> MetadataFlushed -> Published`.
  - Requires: C-P2.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P2.4.
- [ ] **C-P2.4.04 — Invariants/failure — zlfs format and crash transaction**
  - Action: CRC/version/range checks; data before metadata; old generation remains valid until publish; disk-full never truncates existing file; names/lengths bounded.
  - Requires: C-P2.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P2.4.
- [ ] **C-P2.4.05 — Deterministic proof — zlfs format and crash transaction**
  - Action: power cut after every write, corrupt super/metadata/data, full disk, rename/write race, stale handle, unsupported version, model/reference comparison.
  - Requires: C-P2.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P2.4.
- [ ] **C-P2.4.06 — Target proof — zlfs format and crash transaction**
  - Action: separate QEMU cold boots reopen exact bytes on NVMe and USB-backed test media; physical safe target validates commit/recovery.
  - Requires: C-P2.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P2.4.
- [ ] **C-P2.4.07 — Receipt/removal — zlfs format and crash transaction**
  - Action: pre/post volume digest, cut point and recovered generation; read-only mount rollback; old flat format removed only after migration/import and recovery proof.
  - Requires: C-P2.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P2.4.
- [ ] **C-P2.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P2.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P2.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p2-5"></a>
## C-P2.5 — named-file migration and ZLLOG continuity

**Original requirement:** named-file migration and ZLLOG continuity

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 171.

### Preserved original contract

- **Dependencies/current/provenance:** P2.4, current Files/editor/Settings/browser data and raw ZLLOG; unix-history provenance; reject dual sources of truth without migration state.
- **I/O and state:** old slot/private sector and destination path in; copied/verified marker out; `Unseen -> Copied -> Verified -> Switched -> Retired`.
- **Invariants/failure:** source not deleted before byte verification and reboot; repeated migration idempotent; ZLLOG writes remain independent of VFS readiness.
- **Deterministic proof:** interruption at each state, duplicate run, corrupt source/destination, rollback, zero-caller inventory for old API.
- **Target proof:** QEMU and physical cold reboot for Settings/editor/Files/browser data; early crash still appears in raw ZLLOG.
- **Receipt/removal:** per-item hashes and caller inventory; switch back before retirement; delete numbered slots/private paths only after verified migration and zero callers.

### Execution steps

- [ ] **C-P2.5.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P2.5.02 — Resolve this contract's exact dependencies**
  - Action: P2.4, current Files/editor/Settings/browser data and raw ZLLOG; unix-history provenance; reject dual sources of truth without migration state. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P2.5.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P2.5.03 — I/O and state — named-file migration and ZLLOG continuity**
  - Action: old slot/private sector and destination path in; copied/verified marker out; `Unseen -> Copied -> Verified -> Switched -> Retired`.
  - Requires: C-P2.5.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P2.5.
- [ ] **C-P2.5.04 — Invariants/failure — named-file migration and ZLLOG continuity**
  - Action: source not deleted before byte verification and reboot; repeated migration idempotent; ZLLOG writes remain independent of VFS readiness.
  - Requires: C-P2.5.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P2.5.
- [ ] **C-P2.5.05 — Deterministic proof — named-file migration and ZLLOG continuity**
  - Action: interruption at each state, duplicate run, corrupt source/destination, rollback, zero-caller inventory for old API.
  - Requires: C-P2.5.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P2.5.
- [ ] **C-P2.5.06 — Target proof — named-file migration and ZLLOG continuity**
  - Action: QEMU and physical cold reboot for Settings/editor/Files/browser data; early crash still appears in raw ZLLOG.
  - Requires: C-P2.5.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P2.5.
- [ ] **C-P2.5.07 — Receipt/removal — named-file migration and ZLLOG continuity**
  - Action: per-item hashes and caller inventory; switch back before retirement; delete numbered slots/private paths only after verified migration and zero callers.
  - Requires: C-P2.5.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P2.5.
- [ ] **C-P2.5.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P2.5. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P2.5.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p5-2"></a>
## C-P5.2 — File/VFS broker and zlfs provider

**Original requirement:** File/VFS broker and zlfs provider

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 282.

### Preserved original contract

- **Dependencies/current/provenance:** P2 zlfs, P4 handles/IDL/supervisor; Mollen filed and Serenity VFS caps; reject global ambient root.
- **I/O and state:** directory/file handle plus bounded path component and operation in; typed file/dir handle or result out; open handle lifecycle.
- **Invariants/failure:** resolution rooted at capability; checked UTF-8/byte policy; no traversal escape; provider restart yields explicit stale/reopen behavior; rename atomic contract.
- **Deterministic proof:** path fuzz, long/NUL/dot components, permissions, concurrent rename/open, provider crash, stale handles, capacity-aware readdir.
- **Target proof:** QEMU and physical Files/editor/browser flows through broker; corrupt provider cannot crash kernel.
- **Receipt/removal:** protocol/provider/artifact and workflow receipts; adapter to old builtins; remove direct FS app calls at zero inventory.

### Execution steps

- [ ] **C-P5.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P5.2.02 — Resolve this contract's exact dependencies**
  - Action: P2 zlfs, P4 handles/IDL/supervisor; Mollen filed and Serenity VFS caps; reject global ambient root. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P5.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P5.2.03 — I/O and state — File/VFS broker and zlfs provider**
  - Action: directory/file handle plus bounded path component and operation in; typed file/dir handle or result out; open handle lifecycle.
  - Requires: C-P5.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P5.2.
- [ ] **C-P5.2.04 — Invariants/failure — File/VFS broker and zlfs provider**
  - Action: resolution rooted at capability; checked UTF-8/byte policy; no traversal escape; provider restart yields explicit stale/reopen behavior; rename atomic contract.
  - Requires: C-P5.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P5.2.
- [ ] **C-P5.2.05 — Deterministic proof — File/VFS broker and zlfs provider**
  - Action: path fuzz, long/NUL/dot components, permissions, concurrent rename/open, provider crash, stale handles, capacity-aware readdir.
  - Requires: C-P5.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P5.2.
- [ ] **C-P5.2.06 — Target proof — File/VFS broker and zlfs provider**
  - Action: QEMU and physical Files/editor/browser flows through broker; corrupt provider cannot crash kernel.
  - Requires: C-P5.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P5.2.
- [ ] **C-P5.2.07 — Receipt/removal — File/VFS broker and zlfs provider**
  - Action: protocol/provider/artifact and workflow receipts; adapter to old builtins; remove direct FS app calls at zero inventory.
  - Requires: C-P5.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P5.2.
- [ ] **C-P5.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P5.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P5.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p5-7"></a>
## C-P5.7 — Package service skeleton

**Original requirement:** Package service skeleton

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 327.

### Preserved original contract

- **Dependencies/current/provenance:** P5.2, P4 session/caps, signatures; Mollen images, banan recipes; reject read-only bytes as isolation.
- **I/O and state:** signed package/manifest in; staged verified files, capability request and installed record out; package common state.
- **Invariants/failure:** algorithm/key ID/trust root/policy version/rotation/revocation/anti-rollback and digest/ABI/protocol/dependency checks before commit; staged files/tools/roles/services/grants/handles publish atomically; writable mounts explicit; install cannot grant undeclared rights; uninstall first stops processes, revokes caps and deregisters callbacks, then applies explicit user-data retention policy.
- **Deterministic proof:** local-integrity versus publisher-authenticity, mislabeled algorithm, bad signature/digest/dependency/capability, failure/power cut at every write/registry/persist step, duplicate/install conflict, exact pre/post residue comparison, uninstall with live process, rollback and provider crash.
- **Target proof:** QEMU installs and launches an assertion package under requested handles.
- **Receipt/removal:** manifest/content/transaction/app receipt; uninstall/rollback preserves prior active package; no old bundle removal yet.

### Execution steps

- [ ] **C-P5.7.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P5.7.02 — Resolve this contract's exact dependencies**
  - Action: P5.2, P4 session/caps, signatures; Mollen images, banan recipes; reject read-only bytes as isolation. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P5.7.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P5.7.03 — I/O and state — Package service skeleton**
  - Action: signed package/manifest in; staged verified files, capability request and installed record out; package common state.
  - Requires: C-P5.7.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P5.7.
- [ ] **C-P5.7.04 — Invariants/failure — Package service skeleton**
  - Action: algorithm/key ID/trust root/policy version/rotation/revocation/anti-rollback and digest/ABI/protocol/dependency checks before commit; staged files/tools/roles/services/grants/handles publish atomically; writable mounts explicit; install cannot grant undeclared rights; uninstall first stops processes, revokes caps and deregisters callbacks, then applies explicit user-data retention policy.
  - Requires: C-P5.7.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P5.7.
- [ ] **C-P5.7.05 — Deterministic proof — Package service skeleton**
  - Action: local-integrity versus publisher-authenticity, mislabeled algorithm, bad signature/digest/dependency/capability, failure/power cut at every write/registry/persist step, duplicate/install conflict, exact pre/post residue comparison, uninstall with live process, rollback and provider crash.
  - Requires: C-P5.7.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P5.7.
- [ ] **C-P5.7.06 — Target proof — Package service skeleton**
  - Action: QEMU installs and launches an assertion package under requested handles.
  - Requires: C-P5.7.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P5.7.
- [ ] **C-P5.7.07 — Receipt/removal — Package service skeleton**
  - Action: manifest/content/transaction/app receipt; uninstall/rollback preserves prior active package; no old bundle removal yet.
  - Requires: C-P5.7.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P5.7.
- [ ] **C-P5.7.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P5.7. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P5.7.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p7-3"></a>
## C-P7.3 — transactional package install/upgrade/remove

**Original requirement:** transactional package install/upgrade/remove

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 398.

### Preserved original contract

- **Dependencies/current/provenance:** P5.7, P7.1 and zlfs crash transactions; Mollen packages; reject mutation in place.
- **I/O and state:** signed version/dependencies/caps in; active immutable generation and writable roots out; package state machine.
- **Invariants/failure:** previous version remains active until publish; rollback after any failure/power cut; uninstall cannot delete user data without explicit policy.
- **Deterministic proof:** cut each transition, dependency cycles/conflicts, downgrade policy, active app during upgrade, rollback/uninstall leaks.
- **Target proof:** QEMU cold boots at every injected state; physical install/upgrade/rollback assertion app.
- **Receipt/removal:** transaction/generation/content/app smoke receipts; prior generation retained per policy; delete old generation only after health and rollback window.

### Execution steps

- [ ] **C-P7.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P7.3.02 — Resolve this contract's exact dependencies**
  - Action: P5.7, P7.1 and zlfs crash transactions; Mollen packages; reject mutation in place. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P7.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P7.3.03 — I/O and state — transactional package install/upgrade/remove**
  - Action: signed version/dependencies/caps in; active immutable generation and writable roots out; package state machine.
  - Requires: C-P7.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P7.3.
- [ ] **C-P7.3.04 — Invariants/failure — transactional package install/upgrade/remove**
  - Action: previous version remains active until publish; rollback after any failure/power cut; uninstall cannot delete user data without explicit policy.
  - Requires: C-P7.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P7.3.
- [ ] **C-P7.3.05 — Deterministic proof — transactional package install/upgrade/remove**
  - Action: cut each transition, dependency cycles/conflicts, downgrade policy, active app during upgrade, rollback/uninstall leaks.
  - Requires: C-P7.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P7.3.
- [ ] **C-P7.3.06 — Target proof — transactional package install/upgrade/remove**
  - Action: QEMU cold boots at every injected state; physical install/upgrade/rollback assertion app.
  - Requires: C-P7.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P7.3.
- [ ] **C-P7.3.07 — Receipt/removal — transactional package install/upgrade/remove**
  - Action: transaction/generation/content/app smoke receipts; prior generation retained per policy; delete old generation only after health and rollback window.
  - Requires: C-P7.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P7.3.
- [ ] **C-P7.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P7.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P7.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-11"></a>
## C-DA-11 — asynchronous block provider

**Original requirement:** asynchronous block provider

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 306.

### Preserved original contract

**Depends on:** DA-04 through DA-08.

**Deliver:** identity/capacity/block limits, `readv`, `writev`, `flush`, optional
discard, cancel, health and reset. First wrap NVMe and xHCI mass storage without
changing zlfs format.

**Invariants:** checked 64-bit LBA/span arithmetic; DMA direction/ownership explicit;
each buffer completes exactly once; stale completions are rejected; acknowledged
durability level is explicit; reset never silently replays a write.

**Proof:** zero/unaligned/out-of-range spans, queue exhaustion, timeout, controller
fatal/error bits, partial completion, cancel race, media removal, reset, flush
ordering, power cut and cold-read integrity.

### Execution steps

- [ ] **C-DA-11.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-11.02 — Resolve this contract's exact dependencies**
  - Action: DA-04 through DA-08. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-11.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-11.03 — Deliver — asynchronous block provider**
  - Action: identity/capacity/block limits, `readv`, `writev`, `flush`, optional discard, cancel, health and reset. First wrap NVMe and xHCI mass storage without changing zlfs format.
  - Requires: C-DA-11.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-11.
- [ ] **C-DA-11.04 — Invariants — asynchronous block provider**
  - Action: checked 64-bit LBA/span arithmetic; DMA direction/ownership explicit; each buffer completes exactly once; stale completions are rejected; acknowledged durability level is explicit; reset never silently replays a write.
  - Requires: C-DA-11.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-11.
- [ ] **C-DA-11.05 — Proof — asynchronous block provider**
  - Action: zero/unaligned/out-of-range spans, queue exhaustion, timeout, controller fatal/error bits, partial completion, cancel race, media removal, reset, flush ordering, power cut and cold-read integrity.
  - Requires: C-DA-11.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-11.
- [ ] **C-DA-11.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-11. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-11.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-21"></a>
## C-DA-21 — file/VFS and restricted parser services

**Original requirement:** file/VFS and restricted parser services

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 533.

### Preserved original contract

**Depends on:** DA-11, DA-18 and DA-19.

**Deliver:** directory/file handles, namespace and mount authority, file picker,
search/recent/trash policy, and restartable image/font/archive/document/media parser
workers receiving only bounded bytes and output handles.

**Invariants:** no ambient paths or mount power; parsers receive explicit lengths
and budgets; a parser crash returns a failed object; user data survives service
restart; file mutations are transactional where promised.

**Proof:** traversal, symlink/parent loops, duplicate/case names, corrupt size/
offset/count, archive bomb, parser OOM/timeout/crash, unauthorized mount, disk full,
concurrent rename/write and power cut.

### Execution steps

- [ ] **C-DA-21.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-21.02 — Resolve this contract's exact dependencies**
  - Action: DA-11, DA-18 and DA-19. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-21.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-21.03 — Deliver — file/VFS and restricted parser services**
  - Action: directory/file handles, namespace and mount authority, file picker, search/recent/trash policy, and restartable image/font/archive/document/media parser workers receiving only bounded bytes and output handles.
  - Requires: C-DA-21.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-21.
- [ ] **C-DA-21.04 — Invariants — file/VFS and restricted parser services**
  - Action: no ambient paths or mount power; parsers receive explicit lengths and budgets; a parser crash returns a failed object; user data survives service restart; file mutations are transactional where promised.
  - Requires: C-DA-21.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-21.
- [ ] **C-DA-21.05 — Proof — file/VFS and restricted parser services**
  - Action: traversal, symlink/parent loops, duplicate/case names, corrupt size/ offset/count, archive bomb, parser OOM/timeout/crash, unauthorized mount, disk full, concurrent rename/write and power cut.
  - Requires: C-DA-21.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-21.
- [ ] **C-DA-21.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-21. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-21.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-24"></a>
## C-DA-24 — transactional install, update and uninstall

**Original requirement:** transactional install, update and uninstall

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 627.

### Preserved original contract

**Depends on:** DA-21 and DA-23.

**Deliver:** `stage -> validate all bytes/metadata/tools/roles/grants -> atomically
publish`; update with data migration/rollback; uninstall atomically unpublishes,
stops processes, revokes handles and deregisters callbacks before byte deletion.

**Invariants:** local integrity hash is distinct from publisher signature and
runtime audit; trust root/key/algorithm/policy version/rotation/revocation/anti-
rollback are recorded; failure preserves files, registry, roles, grants and handles.
The complete dependency set publishes atomically. Uninstall has an explicit refuse,
cascade or leave-dependent-broken policy; it never silently strands dependents.

**Proof:** inject every file, registry, migration, persist and publication failure;
path traversal, duplicate module, partial archive, wrong size, live process,
revocation, rollback, dependency-set partial failure, dependent uninstall and
explicit retain/delete user-data choices.

### Execution steps

- [ ] **C-DA-24.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-24.02 — Resolve this contract's exact dependencies**
  - Action: DA-21 and DA-23. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-24.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-24.03 — Deliver — transactional install, update and uninstall**
  - Action: `stage -> validate all bytes/metadata/tools/roles/grants -> atomically publish`; update with data migration/rollback; uninstall atomically unpublishes, stops processes, revokes handles and deregisters callbacks before byte deletion.
  - Requires: C-DA-24.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-24.
- [ ] **C-DA-24.04 — Invariants — transactional install, update and uninstall**
  - Action: local integrity hash is distinct from publisher signature and runtime audit; trust root/key/algorithm/policy version/rotation/revocation/anti- rollback are recorded; failure preserves files, registry, roles, grants and handles. The complete dependency set publishes atomically. Uninstall has an explicit refuse, cascade or leave-dependent-broken policy; it never silently strands dependents.
  - Requires: C-DA-24.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-24.
- [ ] **C-DA-24.05 — Proof — transactional install, update and uninstall**
  - Action: inject every file, registry, migration, persist and publication failure; path traversal, duplicate module, partial archive, wrong size, live process, revocation, rollback, dependency-set partial failure, dependent uninstall and explicit retain/delete user-data choices.
  - Requires: C-DA-24.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-24.
- [ ] **C-DA-24.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-24. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-24.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-17"></a>
## C-VX-17 — Application manifest, install and update

**Original requirement:** Application manifest, install and update

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 349.

### Preserved original contract

**Depends on:** package/service contract, VX-05, VX-16.

**Deliver:** canonical signed manifest and atomic install/update/rollback/
uninstall including files, assets, entry points, services, permissions,
localizations, licenses and data policy.

**Invariants:** authenticate before trust; traversal/link/bomb/script input rejected;
unknown required capability rejects; publication is all-or-nothing; uninstall
revokes/stops/unregisters before delete.

**Proof:** failure at every stage, key revoke/rotation/rollback, wrong arch/ABI,
dependency conflict, crash/reboot recovery, live app during update and exact
pre/post object comparison.

### Execution steps

- [ ] **C-VX-17.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-17.02 — Resolve this contract's exact dependencies**
  - Action: package/service contract, VX-05, VX-16. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-17.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-17.03 — Deliver — Application manifest, install and update**
  - Action: canonical signed manifest and atomic install/update/rollback/ uninstall including files, assets, entry points, services, permissions, localizations, licenses and data policy.
  - Requires: C-VX-17.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-17.
- [ ] **C-VX-17.04 — Invariants — Application manifest, install and update**
  - Action: authenticate before trust; traversal/link/bomb/script input rejected; unknown required capability rejects; publication is all-or-nothing; uninstall revokes/stops/unregisters before delete.
  - Requires: C-VX-17.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-17.
- [ ] **C-VX-17.05 — Proof — Application manifest, install and update**
  - Action: failure at every stage, key revoke/rotation/rollback, wrong arch/ABI, dependency conflict, crash/reboot recovery, live app during update and exact pre/post object comparison.
  - Requires: C-VX-17.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-17.
- [ ] **C-VX-17.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-17. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-17.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-blk-001"></a>
## T-BLK-001 — common asynchronous block provider

**Original requirement:** queue/cancel/flush/discard/remove contract

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 106.

### Execution steps

- [ ] **T-BLK-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve common asynchronous block provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-001.02 — Specify the complete target boundary**
  - Action: common asynchronous block provider must supply: queue/cancel/flush/discard/remove contract. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse common asynchronous block provider through the shared platform contract, delivering every part of: queue/cancel/flush/discard/remove contract. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for common asynchronous block provider.
- [ ] **T-BLK-001.05 — Qualify and retain this target's own result**
  - Action: Bind common asynchronous block provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-002"></a>
## T-BLK-002 — NVMe PCI controller/namespaces

**Original requirement:** admin/I/O queues, PRP, reset, format geometry

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 107.

### Execution steps

- [ ] **T-BLK-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve NVMe PCI controller/namespaces to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-002.02 — Specify the complete target boundary**
  - Action: NVMe PCI controller/namespaces must supply: admin/I/O queues, PRP, reset, format geometry. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse NVMe PCI controller/namespaces through the shared platform contract, delivering every part of: admin/I/O queues, PRP, reset, format geometry. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for NVMe PCI controller/namespaces.
- [ ] **T-BLK-002.05 — Qualify and retain this target's own result**
  - Action: Bind NVMe PCI controller/namespaces to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-003"></a>
## T-BLK-003 — AHCI/SATA

**Original requirement:** ports, NCQ/DMA, ATAPI, reset and hotplug

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 108.

### Execution steps

- [ ] **T-BLK-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve AHCI/SATA to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-003.02 — Specify the complete target boundary**
  - Action: AHCI/SATA must supply: ports, NCQ/DMA, ATAPI, reset and hotplug. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse AHCI/SATA through the shared platform contract, delivering every part of: ports, NCQ/DMA, ATAPI, reset and hotplug. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for AHCI/SATA.
- [ ] **T-BLK-003.05 — Qualify and retain this target's own result**
  - Action: Bind AHCI/SATA to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-004"></a>
## T-BLK-004 — legacy ATA/IDE PIO

**Original requirement:** identify/LBA/errors/deadlines and fallback

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 109.

### Execution steps

- [ ] **T-BLK-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve legacy ATA/IDE PIO to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-004.02 — Specify the complete target boundary**
  - Action: legacy ATA/IDE PIO must supply: identify/LBA/errors/deadlines and fallback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse legacy ATA/IDE PIO through the shared platform contract, delivering every part of: identify/LBA/errors/deadlines and fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for legacy ATA/IDE PIO.
- [ ] **T-BLK-004.05 — Qualify and retain this target's own result**
  - Action: Bind legacy ATA/IDE PIO to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-005"></a>
## T-BLK-005 — ATA bus-master DMA

**Original requirement:** PRD bounds, cache coherency and reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 110.

### Execution steps

- [ ] **T-BLK-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ATA bus-master DMA to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-005.02 — Specify the complete target boundary**
  - Action: ATA bus-master DMA must supply: PRD bounds, cache coherency and reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse ATA bus-master DMA through the shared platform contract, delivering every part of: PRD bounds, cache coherency and reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ATA bus-master DMA.
- [ ] **T-BLK-005.05 — Qualify and retain this target's own result**
  - Action: Bind ATA bus-master DMA to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-006"></a>
## T-BLK-006 — ATAPI optical

**Original requirement:** packet commands, media change and read-only

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 111.

### Execution steps

- [ ] **T-BLK-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ATAPI optical to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-006.02 — Specify the complete target boundary**
  - Action: ATAPI optical must supply: packet commands, media change and read-only. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse ATAPI optical through the shared platform contract, delivering every part of: packet commands, media change and read-only. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-006.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ATAPI optical.
- [ ] **T-BLK-006.05 — Qualify and retain this target's own result**
  - Action: Bind ATAPI optical to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-007"></a>
## T-BLK-007 — virtio-blk

**Original requirement:** negotiated limits, multiqueue and reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 112.

### Execution steps

- [ ] **T-BLK-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio-blk to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-007.02 — Specify the complete target boundary**
  - Action: virtio-blk must supply: negotiated limits, multiqueue and reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio-blk through the shared platform contract, delivering every part of: negotiated limits, multiqueue and reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-007.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio-blk.
- [ ] **T-BLK-007.05 — Qualify and retain this target's own result**
  - Action: Bind virtio-blk to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-008"></a>
## T-BLK-008 — SCSI command core

**Original requirement:** CDB/sense/capacity/timeout/retry contract

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 113.

### Execution steps

- [ ] **T-BLK-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve SCSI command core to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-008.02 — Specify the complete target boundary**
  - Action: SCSI command core must supply: CDB/sense/capacity/timeout/retry contract. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse SCSI command core through the shared platform contract, delivering every part of: CDB/sense/capacity/timeout/retry contract. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-008.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for SCSI command core.
- [ ] **T-BLK-008.05 — Qualify and retain this target's own result**
  - Action: Bind SCSI command core to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-009"></a>
## T-BLK-009 — USB MSC BOT

**Original requirement:** CBW/CSW recovery, stalls and residue

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 114.

### Execution steps

- [ ] **T-BLK-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB MSC BOT to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-009.02 — Specify the complete target boundary**
  - Action: USB MSC BOT must supply: CBW/CSW recovery, stalls and residue. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB MSC BOT through the shared platform contract, delivering every part of: CBW/CSW recovery, stalls and residue. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-009.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB MSC BOT.
- [ ] **T-BLK-009.05 — Qualify and retain this target's own result**
  - Action: Bind USB MSC BOT to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-016"></a>
## T-BLK-016 — RAM disk

**Original requirement:** bounded memory ownership and snapshot

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 121.

### Execution steps

- [ ] **T-BLK-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve RAM disk to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-016.02 — Specify the complete target boundary**
  - Action: RAM disk must supply: bounded memory ownership and snapshot. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse RAM disk through the shared platform contract, delivering every part of: bounded memory ownership and snapshot. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-016.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for RAM disk.
- [ ] **T-BLK-016.05 — Qualify and retain this target's own result**
  - Action: Bind RAM disk to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-017"></a>
## T-BLK-017 — loop block device

**Original requirement:** file-backed cycle/resize/flush semantics

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 122.

### Execution steps

- [ ] **T-BLK-017.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve loop block device to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-017.02 — Specify the complete target boundary**
  - Action: loop block device must supply: file-backed cycle/resize/flush semantics. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-017.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-017.03 — Implement the exact target behavior**
  - Action: Implement or reuse loop block device through the shared platform contract, delivering every part of: file-backed cycle/resize/flush semantics. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-017.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-017.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-017.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for loop block device.
- [ ] **T-BLK-017.05 — Qualify and retain this target's own result**
  - Action: Bind loop block device to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-017.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-018"></a>
## T-BLK-018 — read-only optical/ISO media

**Original requirement:** media change and immutable semantics

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 123.

### Execution steps

- [ ] **T-BLK-018.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve read-only optical/ISO media to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-018.02 — Specify the complete target boundary**
  - Action: read-only optical/ISO media must supply: media change and immutable semantics. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-018.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-018.03 — Implement the exact target behavior**
  - Action: Implement or reuse read-only optical/ISO media through the shared platform contract, delivering every part of: media change and immutable semantics. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-018.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-018.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-018.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for read-only optical/ISO media.
- [ ] **T-BLK-018.05 — Qualify and retain this target's own result**
  - Action: Bind read-only optical/ISO media to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-018.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-020"></a>
## T-BLK-020 — encrypted volume mapper

**Original requirement:** key handles, sectors, integrity and revoke

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 125.

### Execution steps

- [ ] **T-BLK-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve encrypted volume mapper to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-020.02 — Specify the complete target boundary**
  - Action: encrypted volume mapper must supply: key handles, sectors, integrity and revoke. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse encrypted volume mapper through the shared platform contract, delivering every part of: key handles, sectors, integrity and revoke. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-020.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for encrypted volume mapper.
- [ ] **T-BLK-020.05 — Qualify and retain this target's own result**
  - Action: Bind encrypted volume mapper to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-021"></a>
## T-BLK-021 — GPT parser

**Original requirement:** primary/backup CRC, overflow and overlap checks

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 126.

### Execution steps

- [ ] **T-BLK-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve GPT parser to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-021.02 — Specify the complete target boundary**
  - Action: GPT parser must supply: primary/backup CRC, overflow and overlap checks. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse GPT parser through the shared platform contract, delivering every part of: primary/backup CRC, overflow and overlap checks. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-021.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for GPT parser.
- [ ] **T-BLK-021.05 — Qualify and retain this target's own result**
  - Action: Bind GPT parser to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-022"></a>
## T-BLK-022 — MBR/extended parser

**Original requirement:** bounded chain, overlap and cycle rejection

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 127.

### Execution steps

- [ ] **T-BLK-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve MBR/extended parser to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-022.02 — Specify the complete target boundary**
  - Action: MBR/extended parser must supply: bounded chain, overlap and cycle rejection. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse MBR/extended parser through the shared platform contract, delivering every part of: bounded chain, overlap and cycle rejection. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-022.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for MBR/extended parser.
- [ ] **T-BLK-022.05 — Qualify and retain this target's own result**
  - Action: Bind MBR/extended parser to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-024"></a>
## T-BLK-024 — crash/power-cut block simulator

**Original requirement:** reorder/drop/tear/fail every operation

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 129.

### Execution steps

- [ ] **T-BLK-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve crash/power-cut block simulator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-024.02 — Specify the complete target boundary**
  - Action: crash/power-cut block simulator must supply: reorder/drop/tear/fail every operation. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse crash/power-cut block simulator through the shared platform contract, delivering every part of: reorder/drop/tear/fail every operation. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-024.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for crash/power-cut block simulator.
- [ ] **T-BLK-024.05 — Qualify and retain this target's own result**
  - Action: Bind crash/power-cut block simulator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-006"></a>
## T-USB-006 — USB mass-storage class

**Original requirement:** BOT/UAS selection and SCSI binding

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 141.

### Execution steps

- [ ] **T-USB-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB mass-storage class to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-006.02 — Specify the complete target boundary**
  - Action: USB mass-storage class must supply: BOT/UAS selection and SCSI binding. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB mass-storage class through the shared platform contract, delivering every part of: BOT/UAS selection and SCSI binding. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-006.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB mass-storage class.
- [ ] **T-USB-006.05 — Qualify and retain this target's own result**
  - Action: Bind USB mass-storage class to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-001"></a>
## T-FSP-001 — zlfs v2+

**Original requirement:** directories/permissions/transactions/recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 277.

### Execution steps

- [ ] **T-FSP-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve zlfs v2+ to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-001.02 — Specify the complete target boundary**
  - Action: zlfs v2+ must supply: directories/permissions/transactions/recovery. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse zlfs v2+ through the shared platform contract, delivering every part of: directories/permissions/transactions/recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-001.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for zlfs v2+.
- [ ] **T-FSP-001.05 — Qualify and retain this target's own result**
  - Action: Bind zlfs v2+ to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-002"></a>
## T-FSP-002 — FAT12/16/32

**Original requirement:** boot/removable interoperability and corruption

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 278.

### Execution steps

- [ ] **T-FSP-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve FAT12/16/32 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-002.02 — Specify the complete target boundary**
  - Action: FAT12/16/32 must supply: boot/removable interoperability and corruption. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse FAT12/16/32 through the shared platform contract, delivering every part of: boot/removable interoperability and corruption. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-002.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for FAT12/16/32.
- [ ] **T-FSP-002.05 — Qualify and retain this target's own result**
  - Action: Bind FAT12/16/32 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-003"></a>
## T-FSP-003 — ext2 read/write

**Original requirement:** allocation/links/permissions/fsync/recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 279.

### Execution steps

- [ ] **T-FSP-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ext2 read/write to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-003.02 — Specify the complete target boundary**
  - Action: ext2 read/write must supply: allocation/links/permissions/fsync/recovery. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse ext2 read/write through the shared platform contract, delivering every part of: allocation/links/permissions/fsync/recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-003.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ext2 read/write.
- [ ] **T-FSP-003.05 — Qualify and retain this target's own result**
  - Action: Bind ext2 read/write to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-004"></a>
## T-FSP-004 — ext4 selected feature set

**Original requirement:** explicit compatible features and journal policy

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 280.

### Execution steps

- [ ] **T-FSP-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ext4 selected feature set to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-004.02 — Specify the complete target boundary**
  - Action: ext4 selected feature set must supply: explicit compatible features and journal policy. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse ext4 selected feature set through the shared platform contract, delivering every part of: explicit compatible features and journal policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-004.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ext4 selected feature set.
- [ ] **T-FSP-004.05 — Qualify and retain this target's own result**
  - Action: Bind ext4 selected feature set to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-005"></a>
## T-FSP-005 — ISO9660

**Original requirement:** read-only optical/boot media and malformed trees

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 281.

### Execution steps

- [ ] **T-FSP-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ISO9660 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-005.02 — Specify the complete target boundary**
  - Action: ISO9660 must supply: read-only optical/boot media and malformed trees. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse ISO9660 through the shared platform contract, delivering every part of: read-only optical/boot media and malformed trees. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-005.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ISO9660.
- [ ] **T-FSP-005.05 — Qualify and retain this target's own result**
  - Action: Bind ISO9660 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-006"></a>
## T-FSP-006 — tmpfs/RAMFS

**Original requirement:** quotas, ownership and memory pressure

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 282.

### Execution steps

- [ ] **T-FSP-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve tmpfs/RAMFS to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-006.02 — Specify the complete target boundary**
  - Action: tmpfs/RAMFS must supply: quotas, ownership and memory pressure. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse tmpfs/RAMFS through the shared platform contract, delivering every part of: quotas, ownership and memory pressure. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-006.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for tmpfs/RAMFS.
- [ ] **T-FSP-006.05 — Qualify and retain this target's own result**
  - Action: Bind tmpfs/RAMFS to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-007"></a>
## T-FSP-007 — devfs

**Original requirement:** handle-mediated devices and dynamic removal

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 283.

### Execution steps

- [ ] **T-FSP-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve devfs to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-007.02 — Specify the complete target boundary**
  - Action: devfs must supply: handle-mediated devices and dynamic removal. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse devfs through the shared platform contract, delivering every part of: handle-mediated devices and dynamic removal. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-007.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for devfs.
- [ ] **T-FSP-007.05 — Qualify and retain this target's own result**
  - Action: Bind devfs to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-008"></a>
## T-FSP-008 — procfs/system-information view

**Original requirement:** generated read-only facts and permissions

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 284.

### Execution steps

- [ ] **T-FSP-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve procfs/system-information view to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-008.02 — Specify the complete target boundary**
  - Action: procfs/system-information view must supply: generated read-only facts and permissions. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse procfs/system-information view through the shared platform contract, delivering every part of: generated read-only facts and permissions. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-008.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for procfs/system-information view.
- [ ] **T-FSP-008.05 — Qualify and retain this target's own result**
  - Action: Bind procfs/system-information view to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-009"></a>
## T-FSP-009 — sysfs/device-information view

**Original requirement:** generated attributes and controlled writes

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 285.

### Execution steps

- [ ] **T-FSP-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve sysfs/device-information view to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-009.02 — Specify the complete target boundary**
  - Action: sysfs/device-information view must supply: generated attributes and controlled writes. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse sysfs/device-information view through the shared platform contract, delivering every part of: generated attributes and controlled writes. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-009.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for sysfs/device-information view.
- [ ] **T-FSP-009.05 — Qualify and retain this target's own result**
  - Action: Bind sysfs/device-information view to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-017"></a>
## T-FSP-017 — exFAT

**Original requirement:** large removable media, allocation/checksum/corruption/recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 293.

### Execution steps

- [ ] **T-FSP-017.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve exFAT to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-017.02 — Specify the complete target boundary**
  - Action: exFAT must supply: large removable media, allocation/checksum/corruption/recovery. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-017.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-017.03 — Implement the exact target behavior**
  - Action: Implement or reuse exFAT through the shared platform contract, delivering every part of: large removable media, allocation/checksum/corruption/recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-017.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-017.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-017.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for exFAT.
- [ ] **T-FSP-017.05 — Qualify and retain this target's own result**
  - Action: Bind exFAT to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-017.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-030"></a>
## T-SVC-030 — Block Broker

**Original requirement:** current block cache -> asynchronous provider arbitration

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 49.

### Execution steps

- [ ] **T-SVC-030.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Block Broker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-030.02 — Specify the complete target boundary**
  - Action: Block Broker must supply: current block cache -> asynchronous provider arbitration. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-030.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-030.03 — Implement the exact target behavior**
  - Action: Implement or reuse Block Broker through the shared platform contract, delivering every part of: current block cache -> asynchronous provider arbitration. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-030.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-030.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-SVC-030.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Block Broker.
- [ ] **T-SVC-030.05 — Qualify and retain this target's own result**
  - Action: Bind Block Broker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-030.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-031"></a>
## T-SVC-031 — VFS/File Service

**Original requirement:** handles, namespaces, mounts, permissions and file operations

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 50.

### Execution steps

- [ ] **T-SVC-031.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve VFS/File Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-031.02 — Specify the complete target boundary**
  - Action: VFS/File Service must supply: handles, namespaces, mounts, permissions and file operations. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-031.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-031.03 — Implement the exact target behavior**
  - Action: Implement or reuse VFS/File Service through the shared platform contract, delivering every part of: handles, namespaces, mounts, permissions and file operations. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-031.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-031.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-SVC-031.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for VFS/File Service.
- [ ] **T-SVC-031.05 — Qualify and retain this target's own result**
  - Action: Bind VFS/File Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-031.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-032"></a>
## T-SVC-032 — zlfs Provider

**Original requirement:** current zlfs v2 -> full provider and recovery contract

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 51.

### Execution steps

- [ ] **T-SVC-032.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve zlfs Provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-032.02 — Specify the complete target boundary**
  - Action: zlfs Provider must supply: current zlfs v2 -> full provider and recovery contract. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-032.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-032.03 — Implement the exact target behavior**
  - Action: Implement or reuse zlfs Provider through the shared platform contract, delivering every part of: current zlfs v2 -> full provider and recovery contract. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-032.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-032.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-SVC-032.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for zlfs Provider.
- [ ] **T-SVC-032.05 — Qualify and retain this target's own result**
  - Action: Bind zlfs Provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-032.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-033"></a>
## T-SVC-033 — Removable Media Manager

**Original requirement:** consent, mount/eject, dirty state and device loss

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 52.

### Execution steps

- [ ] **T-SVC-033.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Removable Media Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-033.02 — Specify the complete target boundary**
  - Action: Removable Media Manager must supply: consent, mount/eject, dirty state and device loss. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-033.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-033.03 — Implement the exact target behavior**
  - Action: Implement or reuse Removable Media Manager through the shared platform contract, delivering every part of: consent, mount/eject, dirty state and device loss. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-033.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-033.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-SVC-033.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Removable Media Manager.
- [ ] **T-SVC-033.05 — Qualify and retain this target's own result**
  - Action: Bind Removable Media Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-033.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-034"></a>
## T-SVC-034 — Volume/Partition Manager

**Original requirement:** GPT/MBR, formats, encryption and ownership

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 53.

### Execution steps

- [ ] **T-SVC-034.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Volume/Partition Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-034.02 — Specify the complete target boundary**
  - Action: Volume/Partition Manager must supply: GPT/MBR, formats, encryption and ownership. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-034.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-034.03 — Implement the exact target behavior**
  - Action: Implement or reuse Volume/Partition Manager through the shared platform contract, delivering every part of: GPT/MBR, formats, encryption and ownership. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-034.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-034.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-SVC-034.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Volume/Partition Manager.
- [ ] **T-SVC-034.05 — Qualify and retain this target's own result**
  - Action: Bind Volume/Partition Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-034.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-035"></a>
## T-SVC-035 — File Cache/Writeback Service

**Original requirement:** bounded cache, pressure, flush and durability receipts

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 54.

### Execution steps

- [ ] **T-SVC-035.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve File Cache/Writeback Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-035.02 — Specify the complete target boundary**
  - Action: File Cache/Writeback Service must supply: bounded cache, pressure, flush and durability receipts. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-035.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-035.03 — Implement the exact target behavior**
  - Action: Implement or reuse File Cache/Writeback Service through the shared platform contract, delivering every part of: bounded cache, pressure, flush and durability receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-035.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-035.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-SVC-035.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for File Cache/Writeback Service.
- [ ] **T-SVC-035.05 — Qualify and retain this target's own result**
  - Action: Bind File Cache/Writeback Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-035.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-036"></a>
## T-SVC-036 — Backup/Snapshot Service

**Original requirement:** scheduled/manual backup, restore, retention and verification

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 55.

### Execution steps

- [ ] **T-SVC-036.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Backup/Snapshot Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-036.02 — Specify the complete target boundary**
  - Action: Backup/Snapshot Service must supply: scheduled/manual backup, restore, retention and verification. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-036.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-036.03 — Implement the exact target behavior**
  - Action: Implement or reuse Backup/Snapshot Service through the shared platform contract, delivering every part of: scheduled/manual backup, restore, retention and verification. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-036.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-036.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-SVC-036.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Backup/Snapshot Service.
- [ ] **T-SVC-036.05 — Qualify and retain this target's own result**
  - Action: Bind Backup/Snapshot Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-036.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-037"></a>
## T-SVC-037 — Filesystem Check/Repair Service

**Original requirement:** offline/online validation, logged repair and rollback

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 56.

### Execution steps

- [ ] **T-SVC-037.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Filesystem Check/Repair Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-037.02 — Specify the complete target boundary**
  - Action: Filesystem Check/Repair Service must supply: offline/online validation, logged repair and rollback. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-037.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-037.03 — Implement the exact target behavior**
  - Action: Implement or reuse Filesystem Check/Repair Service through the shared platform contract, delivering every part of: offline/online validation, logged repair and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-037.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-037.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-SVC-037.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Filesystem Check/Repair Service.
- [ ] **T-SVC-037.05 — Qualify and retain this target's own result**
  - Action: Bind Filesystem Check/Repair Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-037.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-041"></a>
## T-SVC-041 — Package Manager

**Original requirement:** signed transactional install/update/remove and live revoke

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 60.

### Execution steps

- [ ] **T-SVC-041.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Package Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-041.02 — Specify the complete target boundary**
  - Action: Package Manager must supply: signed transactional install/update/remove and live revoke. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-041.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-041.03 — Implement the exact target behavior**
  - Action: Implement or reuse Package Manager through the shared platform contract, delivering every part of: signed transactional install/update/remove and live revoke. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-041.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-041.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-SVC-041.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Package Manager.
- [ ] **T-SVC-041.05 — Qualify and retain this target's own result**
  - Action: Bind Package Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-041.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-042"></a>
## T-SVC-042 — Repository/Update Service

**Original requirement:** metadata trust, channels, download, staging and rollback

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 61.

### Execution steps

- [ ] **T-SVC-042.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Repository/Update Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-10, D-12, H-06.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-042.02 — Specify the complete target boundary**
  - Action: Repository/Update Service must supply: metadata trust, channels, download, staging and rollback. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-042.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-042.03 — Implement the exact target behavior**
  - Action: Implement or reuse Repository/Update Service through the shared platform contract, delivering every part of: metadata trust, channels, download, staging and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-042.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-042.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-SVC-042.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Repository/Update Service.
- [ ] **T-SVC-042.05 — Qualify and retain this target's own result**
  - Action: Bind Repository/Update Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-042.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
