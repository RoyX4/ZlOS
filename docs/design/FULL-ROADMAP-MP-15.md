# MP-15: Make development tools and compatibility usable

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `src/; stdlib/; kernel/apps/; kernel/src/core/; kernel/src/fs/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-15` exports: A PTY/process shell and reproducible tool execution suitable for bringing the zl toolchain onto zlOS.

The handoff enables only its named subset. `CLOSE-15` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-15.01 — Implement terminal/PTY/shell process groups, foreground jobs, signals, EOF, cancellation and command errors

Implement terminal/PTY/shell process groups, foreground jobs, signals, EOF, cancellation and command errors.

**Requires:** `D-01`, `D-02`, `D-20`, `H-00`, `H-03`, `H-04`, `H-06`, `H-08`, `H-11`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-15.02 — Provide core file/process/network utilities over the native authority and filesystem contracts

Provide core file/process/network utilities over the native authority and filesystem contracts.

**Requires:** `M-15.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-15 — Bounded development handoff: Make development tools and compatibility usable

A PTY/process shell and reproducible tool execution suitable for bringing the zl toolchain onto zlOS.

**Requires:** `M-15.02`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-15.03 — Bring editor/IDE, debugger, profiler, build/test/package tools and documentation onto the same process platform

Bring editor/IDE, debugger, profiler, build/test/package tools and documentation onto the same process platform.

**Requires:** `M-15.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-15.04 — Choose exact reference Linux/Windows/POSIX programs and versions, record required APIs and define support claims per workload

Choose exact reference Linux/Windows/POSIX programs and versions, record required APIs and define support claims per workload.

**Requires:** `M-15.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-15.05 — Implement executable/dynamic-linking/library/foreign-ABI contracts in dependency order with bounded unsupported behavior

Implement executable/dynamic-linking/library/foreign-ABI contracts in dependency order with bounded unsupported behavior.

**Requires:** `M-15.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-15.06 — Create reproducible port recipes with source, patches, dependency/license closure and target workload tests

Create reproducible port recipes with source, patches, dependency/license closure and target workload tests.

**Requires:** `M-15.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-15.07 — Add VM/emulation/container profiles and virtio/9P/shared-folder/clipboard integration with explicit grants

Add VM/emulation/container profiles and virtio/9P/shared-folder/clipboard integration with explicit grants.

**Requires:** `M-15.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-15.08 — Test guest/foreign workload escape attempts, process death, shared-resource revocation and upgrade/rollback

Test guest/foreign workload escape attempts, process death, shared-resource revocation and upgrade/rollback.

**Requires:** `M-15.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-15.09 — Keep native zlOS and compatibility semantics distinct in manifests and user-facing availability reports

Keep native zlOS and compatibility semantics distinct in manifests and user-facing availability reports.

**Requires:** `M-15.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-DE-001](#f-de-001) | feature | TTY/virtual console |
| [F-DE-002](#f-de-002) | feature | PTY |
| [F-DE-003](#f-de-003) | feature | Terminal application |
| [F-DE-004](#f-de-004) | feature | interactive shell |
| [F-DE-005](#f-de-005) | feature | pipelines/redirection |
| [F-DE-006](#f-de-006) | feature | shell job control |
| [F-DE-007](#f-de-007) | feature | shell scripting |
| [F-DE-008](#f-de-008) | feature | core file utilities |
| [F-DE-009](#f-de-009) | feature | text utilities |
| [F-DE-010](#f-de-010) | feature | process utilities |
| [F-DE-011](#f-de-011) | feature | storage utilities |
| [F-DE-012](#f-de-012) | feature | network utilities |
| [F-DE-013](#f-de-013) | feature | archive/hash utilities |
| [F-DE-014](#f-de-014) | feature | system administration CLI |
| [F-DE-015](#f-de-015) | feature | help/man pages |
| [F-DE-016](#f-de-016) | feature | command completion/history |
| [F-DE-017](#f-de-017) | feature | environment/config |
| [F-DE-018](#f-de-018) | feature | zl interpreter |
| [F-DE-019](#f-de-019) | feature | zl compiler |
| [F-DE-020](#f-de-020) | feature | assembler/linker integration |
| [F-DE-021](#f-de-021) | feature | package/build tool |
| [F-DE-022](#f-de-022) | feature | source debugger |
| [F-DE-023](#f-de-023) | feature | profiler |
| [F-DE-024](#f-de-024) | feature | tracing tools |
| [F-DE-025](#f-de-025) | feature | test runner |
| [F-DE-026](#f-de-026) | feature | fuzz runner |
| [F-DE-027](#f-de-027) | feature | benchmark runner |
| [F-DE-028](#f-de-028) | feature | emulator/QEMU launcher |
| [F-DE-029](#f-de-029) | feature | SDK/sysroot |
| [F-DE-030](#f-de-030) | feature | API documentation |
| [F-DE-031](#f-de-031) | feature | component/app template |
| [F-DE-032](#f-de-032) | feature | IDE/workbench |
| [F-DE-033](#f-de-033) | feature | version control client |
| [F-DE-034](#f-de-034) | feature | self-hosted build |
| [F-DE-035](#f-de-035) | feature | developer diagnostics mode |
| [F-DE-036](#f-de-036) | feature | crash symbolizer |
| [F-DE-037](#f-de-037) | feature | binary inspectors |
| [F-DE-038](#f-de-038) | feature | visual UI inspector |
| [F-DE-039](#f-de-039) | feature | browser developer tools |
| [F-DE-040](#f-de-040) | feature | documentation browser |
| [F-CP-001](#f-cp-001) | feature | POSIX compatibility ledger |
| [F-CP-002](#f-cp-002) | feature | libc |
| [F-CP-003](#f-cp-003) | feature | C/C++ application ABI |
| [F-CP-004](#f-cp-004) | feature | ELF dynamic loader |
| [F-CP-005](#f-cp-005) | feature | shared libraries |
| [F-CP-006](#f-cp-006) | feature | Windows/PE compatibility subsystem |
| [F-CP-007](#f-cp-007) | feature | Linux ABI and userland compatibility |
| [F-CP-008](#f-cp-008) | feature | Wayland-like client protocol |
| [F-CP-009](#f-cp-009) | feature | SDL-like app layer |
| [F-CP-010](#f-cp-010) | feature | terminal/ANSI compatibility |
| [F-CP-011](#f-cp-011) | feature | third-party port framework |
| [F-CP-012](#f-cp-012) | feature | port catalogue |
| [F-CP-013](#f-cp-013) | feature | DOOM/game ports |
| [F-CP-014](#f-cp-014) | feature | Unix utilities ports |
| [F-CP-015](#f-cp-015) | feature | language runtime ports |
| [F-CP-016](#f-cp-016) | feature | Windows/Linux virtual machine monitor |
| [F-CP-017](#f-cp-017) | feature | Windows/Linux guest integration |
| [F-CP-018](#f-cp-018) | feature | CPU and device emulation |
| [F-CP-019](#f-cp-019) | feature | Game Boy/retro emulation |
| [F-CP-020](#f-cp-020) | feature | compatibility evidence UI |
| [C-P7.4](#c-p7-4) | contract | reproducible port recipe and availability gate |
| [C-P7.5](#c-p7-5) | contract | program-driven POSIX/Linux compatibility |
| [T-PLAT-037](#t-plat-037) | target | legacy APM |
| [T-BUS-026](#t-bus-026) | target | ISA Plug and Play |
| [T-BUS-027](#t-bus-027) | target | Intel 8237 ISA DMA |
| [T-BUS-028](#t-bus-028) | target | VM86 BIOS-call bridge |
| [T-BLK-015](#t-blk-015) | target | floppy controller |
| [T-GPU-003](#t-gpu-003) | target | VBE framebuffer |
| [T-GPU-010](#t-gpu-010) | target | Cirrus legacy display |
| [T-GPU-017](#t-gpu-017) | target | 3dfx legacy accelerator |
| [T-GPU-025](#t-gpu-025) | target | EGA legacy display |
| [T-NIC-009](#t-nic-009) | target | NE2000 |
| [T-NIC-015](#t-nic-015) | target | tap/tun virtual interface |
| [T-MEDIA-004](#t-media-004) | target | Sound Blaster 16 |
| [T-PERIPH-004](#t-periph-004) | target | parallel port |
| [T-VM-004](#t-vm-004) | target | virtio serial |
| [T-VM-005](#t-vm-005) | target | virtio 9P/shared folder |
| [T-VM-006](#t-vm-006) | target | qemu-guest/vdagent integration |
| [T-VM-007](#t-vm-007) | target | VMware guest integration |
| [T-VM-008](#t-vm-008) | target | VirtualBox VMMDev/HGCM |
| [T-VM-009](#t-vm-009) | target | OSC52 clipboard bridge |
| [T-VM-012](#t-vm-012) | target | virtio console/ports |
| [T-FSP-010](#t-fsp-010) | target | devpts/PTY filesystem |
| [T-FSP-012](#t-fsp-012) | target | 9P filesystem |
| [T-FSP-013](#t-fsp-013) | target | FUSE-like userspace provider |
| [T-FSP-015](#t-fsp-015) | target | Mollen MFS compatibility/import |
| [T-FSP-016](#t-fsp-016) | target | ValiFS/VaFS immutable image |
| [T-FSP-018](#t-fsp-018) | target | NTFS read-only importer |
| [T-FSP-019](#t-fsp-019) | target | eventfs |
| [T-FSP-020](#t-fsp-020) | target | pipe/FIFO filesystem |
| [T-FSP-021](#t-fsp-021) | target | socket filesystem |
| [T-FSP-022](#t-fsp-022) | target | legacy custom-FS import adapters |
| [T-SVC-060](#t-svc-060) | target | SSH/Remote Shell Service |
| [T-SVC-120](#t-svc-120) | target | PTY/Terminal Service |
| [T-SVC-121](#t-svc-121) | target | Shell/Command Service |
| [T-SVC-122](#t-svc-122) | target | Debug Service |
| [T-SVC-123](#t-svc-123) | target | Profiler/Trace Service |
| [T-SVC-125](#t-svc-125) | target | Package Build/Port Service |
| [T-SVC-126](#t-svc-126) | target | POSIX/Linux Compatibility Service |
| [T-SVC-127](#t-svc-127) | target | Dynamic Linker/Loader Service |
| [T-SVC-128](#t-svc-128) | target | Virtual Machine Manager |
| [T-SVC-129](#t-svc-129) | target | Container/Sandbox Manager |
| [T-APP-071](#t-app-071) | target | SSH Client |
| [T-APP-080](#t-app-080) | target | Source Editor/IDE |
| [T-APP-082](#t-app-082) | target | Debugger |
| [T-APP-083](#t-app-083) | target | Profiler |
| [T-APP-084](#t-app-084) | target | Log/Trace Viewer |
| [T-APP-085](#t-app-085) | target | Package/Port Development |
| [T-APP-087](#t-app-087) | target | Disk/Filesystem Inspector |
| [T-APP-088](#t-app-088) | target | Network Packet Inspector |
| [T-APP-089](#t-app-089) | target | API/Protocol Workbench |
| [T-DEV-001](#t-dev-001) | target | PTY and terminal ABI |
| [T-DEV-002](#t-dev-002) | target | interactive shell |
| [T-DEV-003](#t-dev-003) | target | core utilities |
| [T-DEV-004](#t-dev-004) | target | source editor/IDE |
| [T-DEV-005](#t-dev-005) | target | debugger |
| [T-DEV-006](#t-dev-006) | target | profiler/tracer |
| [T-DEV-007](#t-dev-007) | target | test runner |
| [T-DEV-008](#t-dev-008) | target | package/port SDK |
| [T-DEV-009](#t-dev-009) | target | POSIX surface ledger |
| [T-DEV-010](#t-dev-010) | target | dynamic linker |
| [T-DEV-011](#t-dev-011) | target | shared libraries |
| [T-DEV-012](#t-dev-012) | target | Linux ABI compatibility |
| [T-DEV-013](#t-dev-013) | target | PE/Windows import quarantine |
| [T-DEV-014](#t-dev-014) | target | scripting runtimes |
| [T-DEV-015](#t-dev-015) | target | SDL/media compatibility |
| [T-DEV-016](#t-dev-016) | target | X11/Wayland bridge experiments |
| [T-DEV-017](#t-dev-017) | target | virtual machine manager |
| [T-DEV-018](#t-dev-018) | target | emulator framework |
| [T-DEV-019](#t-dev-019) | target | sandbox/container profiles |
| [T-DEV-020](#t-dev-020) | target | guest integration |

<a id="f-de-001"></a>
## F-DE-001 — TTY/virtual console

**Original requirement:** byte-counted I/O, resize, sessions, permissions and stable low-level fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-001.01 — Reconcile existing TTY/virtual console**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for TTY/virtual console. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: byte-counted I/O, resize, sessions, permissions and stable low-level fallback
- [ ] **F-DE-001.02 — Freeze the exact contract for TTY/virtual console**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: byte-counted I/O, resize, sessions, permissions and stable low-level fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-001.03 — Implement/prove: byte-counted I/O**
  - Action: For TTY/virtual console, implement or reuse and verify this exact obligation: byte-counted I/O. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for byte-counted I/O; retain observable state/resource expectations.
- [ ] **F-DE-001.04 — Implement/prove: resize**
  - Action: For TTY/virtual console, implement or reuse and verify this exact obligation: resize. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resize; retain observable state/resource expectations.
- [ ] **F-DE-001.05 — Implement/prove: sessions**
  - Action: For TTY/virtual console, implement or reuse and verify this exact obligation: sessions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sessions; retain observable state/resource expectations.
- [ ] **F-DE-001.06 — Implement/prove: permissions and stable low-level fallback**
  - Action: For TTY/virtual console, implement or reuse and verify this exact obligation: permissions and stable low-level fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions and stable low-level fallback; retain observable state/resource expectations.
- [ ] **F-DE-001.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to TTY/virtual console: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-001.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-001.08 — Integrate into the real consumer and runtime route**
  - Action: Wire TTY/virtual console into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-001.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-001.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for TTY/virtual console as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-001.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-002"></a>
## F-DE-002 — PTY

**Original requirement:** bounded duplex master/slave, poll/close/hangup, controlling session and stress tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-002.01 — Reconcile existing PTY**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for PTY. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded duplex master/slave, poll/close/hangup, controlling session and stress tests
- [ ] **F-DE-002.02 — Freeze the exact contract for PTY**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded duplex master/slave, poll/close/hangup, controlling session and stress tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-002.03 — Implement/prove: bounded duplex master/slave**
  - Action: For PTY, implement or reuse and verify this exact obligation: bounded duplex master/slave. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded duplex master/slave; retain observable state/resource expectations.
- [ ] **F-DE-002.04 — Implement/prove: poll/close/hangup**
  - Action: For PTY, implement or reuse and verify this exact obligation: poll/close/hangup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for poll/close/hangup; retain observable state/resource expectations.
- [ ] **F-DE-002.05 — Implement/prove: controlling session and stress tests**
  - Action: For PTY, implement or reuse and verify this exact obligation: controlling session and stress tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-002.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for controlling session and stress tests; retain observable state/resource expectations.
- [ ] **F-DE-002.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to PTY: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-002.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-002.07 — Integrate into the real consumer and runtime route**
  - Action: Wire PTY into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-002.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-002.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for PTY as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-002.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-003"></a>
## F-DE-003 — Terminal application

**Original requirement:** PTY client, Unicode text, scrollback, selection, resize, profiles and child lifecycle

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-003.01 — Reconcile existing Terminal application**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Terminal application. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: PTY client, Unicode text, scrollback, selection, resize, profiles and child lifecycle
- [ ] **F-DE-003.02 — Freeze the exact contract for Terminal application**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: PTY client, Unicode text, scrollback, selection, resize, profiles and child lifecycle. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-003.03 — Implement/prove: PTY client**
  - Action: For Terminal application, implement or reuse and verify this exact obligation: PTY client. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for PTY client; retain observable state/resource expectations.
- [ ] **F-DE-003.04 — Implement/prove: Unicode text**
  - Action: For Terminal application, implement or reuse and verify this exact obligation: Unicode text. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for Unicode text; retain observable state/resource expectations.
- [ ] **F-DE-003.05 — Implement/prove: scrollback**
  - Action: For Terminal application, implement or reuse and verify this exact obligation: scrollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scrollback; retain observable state/resource expectations.
- [ ] **F-DE-003.06 — Implement/prove: selection**
  - Action: For Terminal application, implement or reuse and verify this exact obligation: selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-003.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for selection; retain observable state/resource expectations.
- [ ] **F-DE-003.07 — Implement/prove: resize**
  - Action: For Terminal application, implement or reuse and verify this exact obligation: resize. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-003.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resize; retain observable state/resource expectations.
- [ ] **F-DE-003.08 — Implement/prove: profiles and child lifecycle**
  - Action: For Terminal application, implement or reuse and verify this exact obligation: profiles and child lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-003.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for profiles and child lifecycle; retain observable state/resource expectations.
- [ ] **F-DE-003.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Terminal application: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-003.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-003.10 — Integrate into the real consumer and runtime route**
  - Action: Wire Terminal application into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-003.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-003.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Terminal application as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-003.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-004"></a>
## F-DE-004 — interactive shell

**Original requirement:** tokenizer/parser, quoting, expansion, variables, status, errors and job control

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-004.01 — Reconcile existing interactive shell**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for interactive shell. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: tokenizer/parser, quoting, expansion, variables, status, errors and job control
- [ ] **F-DE-004.02 — Freeze the exact contract for interactive shell**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: tokenizer/parser, quoting, expansion, variables, status, errors and job control. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-004.03 — Implement/prove: tokenizer/parser**
  - Action: For interactive shell, implement or reuse and verify this exact obligation: tokenizer/parser. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tokenizer/parser; retain observable state/resource expectations.
- [ ] **F-DE-004.04 — Implement/prove: quoting**
  - Action: For interactive shell, implement or reuse and verify this exact obligation: quoting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quoting; retain observable state/resource expectations.
- [ ] **F-DE-004.05 — Implement/prove: expansion**
  - Action: For interactive shell, implement or reuse and verify this exact obligation: expansion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-004.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for expansion; retain observable state/resource expectations.
- [ ] **F-DE-004.06 — Implement/prove: variables**
  - Action: For interactive shell, implement or reuse and verify this exact obligation: variables. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-004.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for variables; retain observable state/resource expectations.
- [ ] **F-DE-004.07 — Implement/prove: status**
  - Action: For interactive shell, implement or reuse and verify this exact obligation: status. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-004.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for status; retain observable state/resource expectations.
- [ ] **F-DE-004.08 — Implement/prove: errors and job control**
  - Action: For interactive shell, implement or reuse and verify this exact obligation: errors and job control. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-004.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for errors and job control; retain observable state/resource expectations.
- [ ] **F-DE-004.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to interactive shell: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-004.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-004.10 — Integrate into the real consumer and runtime route**
  - Action: Wire interactive shell into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-004.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-004.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for interactive shell as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-004.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-005"></a>
## F-DE-005 — pipelines/redirection

**Original requirement:** handle-based stdin/out/err, files, append, errors and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-005.01 — Reconcile existing pipelines/redirection**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for pipelines/redirection. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: handle-based stdin/out/err, files, append, errors and cancellation
- [ ] **F-DE-005.02 — Freeze the exact contract for pipelines/redirection**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: handle-based stdin/out/err, files, append, errors and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-005.03 — Implement/prove: handle-based stdin/out/err**
  - Action: For pipelines/redirection, implement or reuse and verify this exact obligation: handle-based stdin/out/err. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handle-based stdin/out/err; retain observable state/resource expectations.
- [ ] **F-DE-005.04 — Implement/prove: files**
  - Action: For pipelines/redirection, implement or reuse and verify this exact obligation: files. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for files; retain observable state/resource expectations.
- [ ] **F-DE-005.05 — Implement/prove: append**
  - Action: For pipelines/redirection, implement or reuse and verify this exact obligation: append. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for append; retain observable state/resource expectations.
- [ ] **F-DE-005.06 — Implement/prove: errors and cancellation**
  - Action: For pipelines/redirection, implement or reuse and verify this exact obligation: errors and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for errors and cancellation; retain observable state/resource expectations.
- [ ] **F-DE-005.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to pipelines/redirection: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-005.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-005.08 — Integrate into the real consumer and runtime route**
  - Action: Wire pipelines/redirection into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-005.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-005.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for pipelines/redirection as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-005.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-006"></a>
## F-DE-006 — shell job control

**Original requirement:** foreground/background, groups, signals, wait, terminal ownership and notifications

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-006.01 — Reconcile existing shell job control**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for shell job control. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: foreground/background, groups, signals, wait, terminal ownership and notifications
- [ ] **F-DE-006.02 — Freeze the exact contract for shell job control**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: foreground/background, groups, signals, wait, terminal ownership and notifications. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-006.03 — Implement/prove: foreground/background**
  - Action: For shell job control, implement or reuse and verify this exact obligation: foreground/background. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for foreground/background; retain observable state/resource expectations.
- [ ] **F-DE-006.04 — Implement/prove: groups**
  - Action: For shell job control, implement or reuse and verify this exact obligation: groups. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for groups; retain observable state/resource expectations.
- [ ] **F-DE-006.05 — Implement/prove: signals**
  - Action: For shell job control, implement or reuse and verify this exact obligation: signals. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signals; retain observable state/resource expectations.
- [ ] **F-DE-006.06 — Implement/prove: wait**
  - Action: For shell job control, implement or reuse and verify this exact obligation: wait. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-006.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for wait; retain observable state/resource expectations.
- [ ] **F-DE-006.07 — Implement/prove: terminal ownership and notifications**
  - Action: For shell job control, implement or reuse and verify this exact obligation: terminal ownership and notifications. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-006.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for terminal ownership and notifications; retain observable state/resource expectations.
- [ ] **F-DE-006.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to shell job control: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-006.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-006.09 — Integrate into the real consumer and runtime route**
  - Action: Wire shell job control into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-006.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-006.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for shell job control as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-006.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-007"></a>
## F-DE-007 — shell scripting

**Original requirement:** defined syntax, conditionals/loops/functions/errors/modules and deterministic tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-007.01 — Reconcile existing shell scripting**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for shell scripting. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: defined syntax, conditionals/loops/functions/errors/modules and deterministic tests
- [ ] **F-DE-007.02 — Freeze the exact contract for shell scripting**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: defined syntax, conditionals/loops/functions/errors/modules and deterministic tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-007.03 — Implement/prove: defined syntax**
  - Action: For shell scripting, implement or reuse and verify this exact obligation: defined syntax. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for defined syntax; retain observable state/resource expectations.
- [ ] **F-DE-007.04 — Implement/prove: conditionals/loops/functions/errors/modules and deterministic tests**
  - Action: For shell scripting, implement or reuse and verify this exact obligation: conditionals/loops/functions/errors/modules and deterministic tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conditionals/loops/functions/errors/modules and deterministic tests; retain observable state/resource expectations.
- [ ] **F-DE-007.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to shell scripting: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-007.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-007.06 — Integrate into the real consumer and runtime route**
  - Action: Wire shell scripting into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-007.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-007.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for shell scripting as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-007.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-008"></a>
## F-DE-008 — core file utilities

**Original requirement:** truthful `ls/cat/cp/mv/rm/mkdir/touch/find` semantics with permissions/errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-008.01 — Reconcile existing core file utilities**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for core file utilities. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: truthful `ls/cat/cp/mv/rm/mkdir/touch/find` semantics with permissions/errors
- [ ] **F-DE-008.02 — Freeze the exact contract for core file utilities**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: truthful `ls/cat/cp/mv/rm/mkdir/touch/find` semantics with permissions/errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-008.03 — Implement/prove: truthful `ls/cat/cp/mv/rm/mkdir/touch/find` semantics with permissions/errors**
  - Action: For core file utilities, implement or reuse and verify this exact obligation: truthful `ls/cat/cp/mv/rm/mkdir/touch/find` semantics with permissions/errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for truthful `ls/cat/cp/mv/rm/mkdir/touch/find` semantics with permissions/errors; retain observable state/resource expectations.
- [ ] **F-DE-008.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to core file utilities: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-008.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-008.05 — Integrate into the real consumer and runtime route**
  - Action: Wire core file utilities into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-008.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-008.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for core file utilities as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-008.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-009"></a>
## F-DE-009 — text utilities

**Original requirement:** grep/sort/head/tail/wc/diff/encoding with streaming bounds

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-009.01 — Reconcile existing text utilities**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for text utilities. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: grep/sort/head/tail/wc/diff/encoding with streaming bounds
- [ ] **F-DE-009.02 — Freeze the exact contract for text utilities**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: grep/sort/head/tail/wc/diff/encoding with streaming bounds. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-009.03 — Implement/prove: grep/sort/head/tail/wc/diff/encoding with streaming bounds**
  - Action: For text utilities, implement or reuse and verify this exact obligation: grep/sort/head/tail/wc/diff/encoding with streaming bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grep/sort/head/tail/wc/diff/encoding with streaming bounds; retain observable state/resource expectations.
- [ ] **F-DE-009.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to text utilities: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-009.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-009.05 — Integrate into the real consumer and runtime route**
  - Action: Wire text utilities into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-009.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-009.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for text utilities as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-009.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-010"></a>
## F-DE-010 — process utilities

**Original requirement:** ps/top/kill/time/uptime with real service data and authorization

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-010.01 — Reconcile existing process utilities**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for process utilities. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: ps/top/kill/time/uptime with real service data and authorization
- [ ] **F-DE-010.02 — Freeze the exact contract for process utilities**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: ps/top/kill/time/uptime with real service data and authorization. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-010.03 — Implement/prove: ps/top/kill/time/uptime with real service data and authorization**
  - Action: For process utilities, implement or reuse and verify this exact obligation: ps/top/kill/time/uptime with real service data and authorization. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ps/top/kill/time/uptime with real service data and authorization; retain observable state/resource expectations.
- [ ] **F-DE-010.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to process utilities: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-010.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-010.05 — Integrate into the real consumer and runtime route**
  - Action: Wire process utilities into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-010.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-010.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for process utilities as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-010.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-011"></a>
## F-DE-011 — storage utilities

**Original requirement:** mount/df/du/fsck/format with explicit authority and destructive confirmation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-011.01 — Reconcile existing storage utilities**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for storage utilities. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: mount/df/du/fsck/format with explicit authority and destructive confirmation
- [ ] **F-DE-011.02 — Freeze the exact contract for storage utilities**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: mount/df/du/fsck/format with explicit authority and destructive confirmation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-011.03 — Implement/prove: mount/df/du/fsck/format with explicit authority and destructive confirmation**
  - Action: For storage utilities, implement or reuse and verify this exact obligation: mount/df/du/fsck/format with explicit authority and destructive confirmation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mount/df/du/fsck/format with explicit authority and destructive confirmation; retain observable state/resource expectations.
- [ ] **F-DE-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to storage utilities: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire storage utilities into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for storage utilities as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-012"></a>
## F-DE-012 — network utilities

**Original requirement:** ip/route/ping/dns/socket/curl-like tools using real services, never simulation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-012.01 — Reconcile existing network utilities**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for network utilities. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: ip/route/ping/dns/socket/curl-like tools using real services, never simulation
- [ ] **F-DE-012.02 — Freeze the exact contract for network utilities**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: ip/route/ping/dns/socket/curl-like tools using real services, never simulation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-012.03 — Implement/prove: ip/route/ping/dns/socket/curl-like tools using real services**
  - Action: For network utilities, implement or reuse and verify this exact obligation: ip/route/ping/dns/socket/curl-like tools using real services. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ip/route/ping/dns/socket/curl-like tools using real services; retain observable state/resource expectations.
- [ ] **F-DE-012.04 — Implement/prove: never simulation**
  - Action: For network utilities, implement or reuse and verify this exact obligation: never simulation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for never simulation; retain observable state/resource expectations.
- [ ] **F-DE-012.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to network utilities: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-012.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-012.06 — Integrate into the real consumer and runtime route**
  - Action: Wire network utilities into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-012.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-012.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for network utilities as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-012.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-013"></a>
## F-DE-013 — archive/hash utilities

**Original requirement:** bounded parsers, streaming, verification and clear algorithms

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-013.01 — Reconcile existing archive/hash utilities**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for archive/hash utilities. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded parsers, streaming, verification and clear algorithms
- [ ] **F-DE-013.02 — Freeze the exact contract for archive/hash utilities**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded parsers, streaming, verification and clear algorithms. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-013.03 — Implement/prove: bounded parsers**
  - Action: For archive/hash utilities, implement or reuse and verify this exact obligation: bounded parsers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded parsers; retain observable state/resource expectations.
- [ ] **F-DE-013.04 — Implement/prove: streaming**
  - Action: For archive/hash utilities, implement or reuse and verify this exact obligation: streaming. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for streaming; retain observable state/resource expectations.
- [ ] **F-DE-013.05 — Implement/prove: verification and clear algorithms**
  - Action: For archive/hash utilities, implement or reuse and verify this exact obligation: verification and clear algorithms. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for verification and clear algorithms; retain observable state/resource expectations.
- [ ] **F-DE-013.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to archive/hash utilities: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-013.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-013.07 — Integrate into the real consumer and runtime route**
  - Action: Wire archive/hash utilities into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-013.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-013.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for archive/hash utilities as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-013.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-014"></a>
## F-DE-014 — system administration CLI

**Original requirement:** service/user/package/update/device/power operations behind explicit handles

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-014.01 — Reconcile existing system administration CLI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for system administration CLI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: service/user/package/update/device/power operations behind explicit handles
- [ ] **F-DE-014.02 — Freeze the exact contract for system administration CLI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: service/user/package/update/device/power operations behind explicit handles. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-014.03 — Implement/prove: service/user/package/update/device/power operations behind explicit handles**
  - Action: For system administration CLI, implement or reuse and verify this exact obligation: service/user/package/update/device/power operations behind explicit handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for service/user/package/update/device/power operations behind explicit handles; retain observable state/resource expectations.
- [ ] **F-DE-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to system administration CLI: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire system administration CLI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for system administration CLI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-015"></a>
## F-DE-015 — help/man pages

**Original requirement:** version-matched searchable usage, examples, exit status and links

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-015.01 — Reconcile existing help/man pages**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for help/man pages. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: version-matched searchable usage, examples, exit status and links
- [ ] **F-DE-015.02 — Freeze the exact contract for help/man pages**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: version-matched searchable usage, examples, exit status and links. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-015.03 — Implement/prove: version-matched searchable usage**
  - Action: For help/man pages, implement or reuse and verify this exact obligation: version-matched searchable usage. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for version-matched searchable usage; retain observable state/resource expectations.
- [ ] **F-DE-015.04 — Implement/prove: examples**
  - Action: For help/man pages, implement or reuse and verify this exact obligation: examples. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for examples; retain observable state/resource expectations.
- [ ] **F-DE-015.05 — Implement/prove: exit status and links**
  - Action: For help/man pages, implement or reuse and verify this exact obligation: exit status and links. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-015.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exit status and links; retain observable state/resource expectations.
- [ ] **F-DE-015.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to help/man pages: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-015.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-015.07 — Integrate into the real consumer and runtime route**
  - Action: Wire help/man pages into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-015.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-015.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for help/man pages as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-015.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-016"></a>
## F-DE-016 — command completion/history

**Original requirement:** shell-aware suggestions, per-user persistence, sensitive filtering and clearing

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-016.01 — Reconcile existing command completion/history**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for command completion/history. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: shell-aware suggestions, per-user persistence, sensitive filtering and clearing
- [ ] **F-DE-016.02 — Freeze the exact contract for command completion/history**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: shell-aware suggestions, per-user persistence, sensitive filtering and clearing. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-016.03 — Implement/prove: shell-aware suggestions**
  - Action: For command completion/history, implement or reuse and verify this exact obligation: shell-aware suggestions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shell-aware suggestions; retain observable state/resource expectations.
- [ ] **F-DE-016.04 — Implement/prove: per-user persistence**
  - Action: For command completion/history, implement or reuse and verify this exact obligation: per-user persistence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-user persistence; retain observable state/resource expectations.
- [ ] **F-DE-016.05 — Implement/prove: sensitive filtering and clearing**
  - Action: For command completion/history, implement or reuse and verify this exact obligation: sensitive filtering and clearing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sensitive filtering and clearing; retain observable state/resource expectations.
- [ ] **F-DE-016.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to command completion/history: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-016.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-016.07 — Integrate into the real consumer and runtime route**
  - Action: Wire command completion/history into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-016.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-016.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for command completion/history as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-016.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-017"></a>
## F-DE-017 — environment/config

**Original requirement:** scoped variables, startup files, precedence, validation and provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-017.01 — Reconcile existing environment/config**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for environment/config. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: scoped variables, startup files, precedence, validation and provenance
- [ ] **F-DE-017.02 — Freeze the exact contract for environment/config**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: scoped variables, startup files, precedence, validation and provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-017.03 — Implement/prove: scoped variables**
  - Action: For environment/config, implement or reuse and verify this exact obligation: scoped variables. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scoped variables; retain observable state/resource expectations.
- [ ] **F-DE-017.04 — Implement/prove: startup files**
  - Action: For environment/config, implement or reuse and verify this exact obligation: startup files. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for startup files; retain observable state/resource expectations.
- [ ] **F-DE-017.05 — Implement/prove: precedence**
  - Action: For environment/config, implement or reuse and verify this exact obligation: precedence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for precedence; retain observable state/resource expectations.
- [ ] **F-DE-017.06 — Implement/prove: validation and provenance**
  - Action: For environment/config, implement or reuse and verify this exact obligation: validation and provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-017.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validation and provenance; retain observable state/resource expectations.
- [ ] **F-DE-017.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to environment/config: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-017.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-017.08 — Integrate into the real consumer and runtime route**
  - Action: Wire environment/config into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-017.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-017.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for environment/config as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-017.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-018"></a>
## F-DE-018 — zl interpreter

**Original requirement:** language semantics, diagnostics, module resolution and sandboxed execution modes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-018.01 — Reconcile existing zl interpreter**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for zl interpreter. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: language semantics, diagnostics, module resolution and sandboxed execution modes
- [ ] **F-DE-018.02 — Freeze the exact contract for zl interpreter**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: language semantics, diagnostics, module resolution and sandboxed execution modes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-018.03 — Implement/prove: language semantics**
  - Action: For zl interpreter, implement or reuse and verify this exact obligation: language semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for language semantics; retain observable state/resource expectations.
- [ ] **F-DE-018.04 — Implement/prove: diagnostics**
  - Action: For zl interpreter, implement or reuse and verify this exact obligation: diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for diagnostics; retain observable state/resource expectations.
- [ ] **F-DE-018.05 — Implement/prove: module resolution and sandboxed execution modes**
  - Action: For zl interpreter, implement or reuse and verify this exact obligation: module resolution and sandboxed execution modes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for module resolution and sandboxed execution modes; retain observable state/resource expectations.
- [ ] **F-DE-018.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to zl interpreter: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-018.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-018.07 — Integrate into the real consumer and runtime route**
  - Action: Wire zl interpreter into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-018.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-018.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for zl interpreter as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-018.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-019"></a>
## F-DE-019 — zl compiler

**Original requirement:** reproducible native/object outputs, diagnostics, optimization and target manifests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-019.01 — Reconcile existing zl compiler**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for zl compiler. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: reproducible native/object outputs, diagnostics, optimization and target manifests
- [ ] **F-DE-019.02 — Freeze the exact contract for zl compiler**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: reproducible native/object outputs, diagnostics, optimization and target manifests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-019.03 — Implement/prove: reproducible native/object outputs**
  - Action: For zl compiler, implement or reuse and verify this exact obligation: reproducible native/object outputs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reproducible native/object outputs; retain observable state/resource expectations.
- [ ] **F-DE-019.04 — Implement/prove: diagnostics**
  - Action: For zl compiler, implement or reuse and verify this exact obligation: diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for diagnostics; retain observable state/resource expectations.
- [ ] **F-DE-019.05 — Implement/prove: optimization and target manifests**
  - Action: For zl compiler, implement or reuse and verify this exact obligation: optimization and target manifests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for optimization and target manifests; retain observable state/resource expectations.
- [ ] **F-DE-019.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to zl compiler: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-019.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-019.07 — Integrate into the real consumer and runtime route**
  - Action: Wire zl compiler into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-019.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-019.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for zl compiler as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-019.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-020"></a>
## F-DE-020 — assembler/linker integration

**Original requirement:** named sections/symbols/relocations/layout, map output and checked errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-020.01 — Reconcile existing assembler/linker integration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for assembler/linker integration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: named sections/symbols/relocations/layout, map output and checked errors
- [ ] **F-DE-020.02 — Freeze the exact contract for assembler/linker integration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: named sections/symbols/relocations/layout, map output and checked errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-020.03 — Implement/prove: named sections/symbols/relocations/layout**
  - Action: For assembler/linker integration, implement or reuse and verify this exact obligation: named sections/symbols/relocations/layout. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for named sections/symbols/relocations/layout; retain observable state/resource expectations.
- [ ] **F-DE-020.04 — Implement/prove: map output and checked errors**
  - Action: For assembler/linker integration, implement or reuse and verify this exact obligation: map output and checked errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for map output and checked errors; retain observable state/resource expectations.
- [ ] **F-DE-020.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to assembler/linker integration: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-020.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-020.06 — Integrate into the real consumer and runtime route**
  - Action: Wire assembler/linker integration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-020.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-020.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for assembler/linker integration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-020.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-021"></a>
## F-DE-021 — package/build tool

**Original requirement:** manifests, dependency graph, cache, targets, tests, provenance and strict failures

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-021.01 — Reconcile existing package/build tool**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for package/build tool. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: manifests, dependency graph, cache, targets, tests, provenance and strict failures
- [ ] **F-DE-021.02 — Freeze the exact contract for package/build tool**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: manifests, dependency graph, cache, targets, tests, provenance and strict failures. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-021.03 — Implement/prove: manifests**
  - Action: For package/build tool, implement or reuse and verify this exact obligation: manifests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for manifests; retain observable state/resource expectations.
- [ ] **F-DE-021.04 — Implement/prove: dependency graph**
  - Action: For package/build tool, implement or reuse and verify this exact obligation: dependency graph. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dependency graph; retain observable state/resource expectations.
- [ ] **F-DE-021.05 — Implement/prove: cache**
  - Action: For package/build tool, implement or reuse and verify this exact obligation: cache. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cache; retain observable state/resource expectations.
- [ ] **F-DE-021.06 — Implement/prove: targets**
  - Action: For package/build tool, implement or reuse and verify this exact obligation: targets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-021.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for targets; retain observable state/resource expectations.
- [ ] **F-DE-021.07 — Implement/prove: tests**
  - Action: For package/build tool, implement or reuse and verify this exact obligation: tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-021.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tests; retain observable state/resource expectations.
- [ ] **F-DE-021.08 — Implement/prove: provenance and strict failures**
  - Action: For package/build tool, implement or reuse and verify this exact obligation: provenance and strict failures. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-021.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance and strict failures; retain observable state/resource expectations.
- [ ] **F-DE-021.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to package/build tool: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-021.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-021.10 — Integrate into the real consumer and runtime route**
  - Action: Wire package/build tool into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-021.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-021.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for package/build tool as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-021.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-022"></a>
## F-DE-022 — source debugger

**Original requirement:** processes/threads/breakpoints/registers/memory/stack/source with debug authority

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-022.01 — Reconcile existing source debugger**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for source debugger. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: processes/threads/breakpoints/registers/memory/stack/source with debug authority
- [ ] **F-DE-022.02 — Freeze the exact contract for source debugger**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: processes/threads/breakpoints/registers/memory/stack/source with debug authority. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-022.03 — Implement/prove: processes/threads/breakpoints/registers/memory/stack/source with debug authority**
  - Action: For source debugger, implement or reuse and verify this exact obligation: processes/threads/breakpoints/registers/memory/stack/source with debug authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for processes/threads/breakpoints/registers/memory/stack/source with debug authority; retain observable state/resource expectations.
- [ ] **F-DE-022.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to source debugger: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-022.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-022.05 — Integrate into the real consumer and runtime route**
  - Action: Wire source debugger into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-022.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-022.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for source debugger as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-022.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-023"></a>
## F-DE-023 — profiler

**Original requirement:** CPU/allocation/I/O/frame samples, symbols, overhead and export

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-023.01 — Reconcile existing profiler**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for profiler. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: CPU/allocation/I/O/frame samples, symbols, overhead and export
- [ ] **F-DE-023.02 — Freeze the exact contract for profiler**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: CPU/allocation/I/O/frame samples, symbols, overhead and export. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-023.03 — Implement/prove: CPU/allocation/I/O/frame samples**
  - Action: For profiler, implement or reuse and verify this exact obligation: CPU/allocation/I/O/frame samples. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for CPU/allocation/I/O/frame samples; retain observable state/resource expectations.
- [ ] **F-DE-023.04 — Implement/prove: symbols**
  - Action: For profiler, implement or reuse and verify this exact obligation: symbols. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for symbols; retain observable state/resource expectations.
- [ ] **F-DE-023.05 — Implement/prove: overhead and export**
  - Action: For profiler, implement or reuse and verify this exact obligation: overhead and export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for overhead and export; retain observable state/resource expectations.
- [ ] **F-DE-023.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to profiler: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-023.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-023.07 — Integrate into the real consumer and runtime route**
  - Action: Wire profiler into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-023.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-023.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for profiler as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-023.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-024"></a>
## F-DE-024 — tracing tools

**Original requirement:** typed events, filters/correlation, live/file views, privacy and drop evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-024.01 — Reconcile existing tracing tools**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for tracing tools. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: typed events, filters/correlation, live/file views, privacy and drop evidence
- [ ] **F-DE-024.02 — Freeze the exact contract for tracing tools**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: typed events, filters/correlation, live/file views, privacy and drop evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-024.03 — Implement/prove: typed events**
  - Action: For tracing tools, implement or reuse and verify this exact obligation: typed events. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typed events; retain observable state/resource expectations.
- [ ] **F-DE-024.04 — Implement/prove: filters/correlation**
  - Action: For tracing tools, implement or reuse and verify this exact obligation: filters/correlation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for filters/correlation; retain observable state/resource expectations.
- [ ] **F-DE-024.05 — Implement/prove: live/file views**
  - Action: For tracing tools, implement or reuse and verify this exact obligation: live/file views. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-024.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for live/file views; retain observable state/resource expectations.
- [ ] **F-DE-024.06 — Implement/prove: privacy and drop evidence**
  - Action: For tracing tools, implement or reuse and verify this exact obligation: privacy and drop evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-024.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and drop evidence; retain observable state/resource expectations.
- [ ] **F-DE-024.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to tracing tools: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-024.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-024.08 — Integrate into the real consumer and runtime route**
  - Action: Wire tracing tools into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-024.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-024.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for tracing tools as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-024.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-025"></a>
## F-DE-025 — test runner

**Original requirement:** discovery/filter/shard/timeout/skip/failure/artifact and target/host lanes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-025.01 — Reconcile existing test runner**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for test runner. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: discovery/filter/shard/timeout/skip/failure/artifact and target/host lanes
- [ ] **F-DE-025.02 — Freeze the exact contract for test runner**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: discovery/filter/shard/timeout/skip/failure/artifact and target/host lanes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-025.03 — Implement/prove: discovery/filter/shard/timeout/skip/failure/artifact and target/host lanes**
  - Action: For test runner, implement or reuse and verify this exact obligation: discovery/filter/shard/timeout/skip/failure/artifact and target/host lanes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discovery/filter/shard/timeout/skip/failure/artifact and target/host lanes; retain observable state/resource expectations.
- [ ] **F-DE-025.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to test runner: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-025.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-025.05 — Integrate into the real consumer and runtime route**
  - Action: Wire test runner into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-025.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-025.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for test runner as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-025.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-026"></a>
## F-DE-026 — fuzz runner

**Original requirement:** production entrypoints, corpus, minimization, sanitizer and durable failures

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-026.01 — Reconcile existing fuzz runner**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for fuzz runner. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: production entrypoints, corpus, minimization, sanitizer and durable failures
- [ ] **F-DE-026.02 — Freeze the exact contract for fuzz runner**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: production entrypoints, corpus, minimization, sanitizer and durable failures. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-026.03 — Implement/prove: production entrypoints**
  - Action: For fuzz runner, implement or reuse and verify this exact obligation: production entrypoints. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for production entrypoints; retain observable state/resource expectations.
- [ ] **F-DE-026.04 — Implement/prove: corpus**
  - Action: For fuzz runner, implement or reuse and verify this exact obligation: corpus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for corpus; retain observable state/resource expectations.
- [ ] **F-DE-026.05 — Implement/prove: minimization**
  - Action: For fuzz runner, implement or reuse and verify this exact obligation: minimization. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for minimization; retain observable state/resource expectations.
- [ ] **F-DE-026.06 — Implement/prove: sanitizer and durable failures**
  - Action: For fuzz runner, implement or reuse and verify this exact obligation: sanitizer and durable failures. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-026.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sanitizer and durable failures; retain observable state/resource expectations.
- [ ] **F-DE-026.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to fuzz runner: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-026.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-026.08 — Integrate into the real consumer and runtime route**
  - Action: Wire fuzz runner into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-026.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-026.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for fuzz runner as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-026.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-027"></a>
## F-DE-027 — benchmark runner

**Original requirement:** warmup/samples/clock/statistics/baseline/regression and environment identity

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-027.01 — Reconcile existing benchmark runner**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for benchmark runner. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: warmup/samples/clock/statistics/baseline/regression and environment identity
- [ ] **F-DE-027.02 — Freeze the exact contract for benchmark runner**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: warmup/samples/clock/statistics/baseline/regression and environment identity. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-027.03 — Implement/prove: warmup/samples/clock/statistics/baseline/regression and environment identity**
  - Action: For benchmark runner, implement or reuse and verify this exact obligation: warmup/samples/clock/statistics/baseline/regression and environment identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for warmup/samples/clock/statistics/baseline/regression and environment identity; retain observable state/resource expectations.
- [ ] **F-DE-027.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to benchmark runner: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-027.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-027.05 — Integrate into the real consumer and runtime route**
  - Action: Wire benchmark runner into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-027.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-027.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for benchmark runner as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-027.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-028"></a>
## F-DE-028 — emulator/QEMU launcher

**Original requirement:** exact images/config/device matrix, assertions, timeout and artifact capture

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-028.01 — Reconcile existing emulator/QEMU launcher**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for emulator/QEMU launcher. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact images/config/device matrix, assertions, timeout and artifact capture
- [ ] **F-DE-028.02 — Freeze the exact contract for emulator/QEMU launcher**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact images/config/device matrix, assertions, timeout and artifact capture. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-028.03 — Implement/prove: exact images/config/device matrix**
  - Action: For emulator/QEMU launcher, implement or reuse and verify this exact obligation: exact images/config/device matrix. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact images/config/device matrix; retain observable state/resource expectations.
- [ ] **F-DE-028.04 — Implement/prove: assertions**
  - Action: For emulator/QEMU launcher, implement or reuse and verify this exact obligation: assertions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for assertions; retain observable state/resource expectations.
- [ ] **F-DE-028.05 — Implement/prove: timeout and artifact capture**
  - Action: For emulator/QEMU launcher, implement or reuse and verify this exact obligation: timeout and artifact capture. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeout and artifact capture; retain observable state/resource expectations.
- [ ] **F-DE-028.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to emulator/QEMU launcher: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-028.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-028.07 — Integrate into the real consumer and runtime route**
  - Action: Wire emulator/QEMU launcher into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-028.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-028.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for emulator/QEMU launcher as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-028.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-029"></a>
## F-DE-029 — SDK/sysroot

**Original requirement:** headers/libraries/tools/ABI metadata/examples and exact consumer matrix

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-029.01 — Reconcile existing SDK/sysroot**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for SDK/sysroot. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: headers/libraries/tools/ABI metadata/examples and exact consumer matrix
- [ ] **F-DE-029.02 — Freeze the exact contract for SDK/sysroot**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: headers/libraries/tools/ABI metadata/examples and exact consumer matrix. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-029.03 — Implement/prove: headers/libraries/tools/ABI metadata/examples and exact consumer matrix**
  - Action: For SDK/sysroot, implement or reuse and verify this exact obligation: headers/libraries/tools/ABI metadata/examples and exact consumer matrix. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for headers/libraries/tools/ABI metadata/examples and exact consumer matrix; retain observable state/resource expectations.
- [ ] **F-DE-029.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to SDK/sysroot: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-029.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-029.05 — Integrate into the real consumer and runtime route**
  - Action: Wire SDK/sysroot into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-029.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-029.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for SDK/sysroot as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-029.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-030"></a>
## F-DE-030 — API documentation

**Original requirement:** generated signatures/contracts/examples/errors/version and source links

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-030.01 — Reconcile existing API documentation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for API documentation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generated signatures/contracts/examples/errors/version and source links
- [ ] **F-DE-030.02 — Freeze the exact contract for API documentation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generated signatures/contracts/examples/errors/version and source links. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-030.03 — Implement/prove: generated signatures/contracts/examples/errors/version and source links**
  - Action: For API documentation, implement or reuse and verify this exact obligation: generated signatures/contracts/examples/errors/version and source links. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated signatures/contracts/examples/errors/version and source links; retain observable state/resource expectations.
- [ ] **F-DE-030.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to API documentation: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-030.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-030.05 — Integrate into the real consumer and runtime route**
  - Action: Wire API documentation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-030.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-030.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for API documentation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-030.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-031"></a>
## F-DE-031 — component/app template

**Original requirement:** minimal correct manifest/process/AppKit/tests/assets/provenance scaffold

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-031.01 — Reconcile existing component/app template**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for component/app template. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: minimal correct manifest/process/AppKit/tests/assets/provenance scaffold
- [ ] **F-DE-031.02 — Freeze the exact contract for component/app template**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: minimal correct manifest/process/AppKit/tests/assets/provenance scaffold. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-031.03 — Implement/prove: minimal correct manifest/process/AppKit/tests/assets/provenance scaffold**
  - Action: For component/app template, implement or reuse and verify this exact obligation: minimal correct manifest/process/AppKit/tests/assets/provenance scaffold. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for minimal correct manifest/process/AppKit/tests/assets/provenance scaffold; retain observable state/resource expectations.
- [ ] **F-DE-031.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to component/app template: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-031.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-031.05 — Integrate into the real consumer and runtime route**
  - Action: Wire component/app template into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-031.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-031.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for component/app template as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-031.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-032"></a>
## F-DE-032 — IDE/workbench

**Original requirement:** project/editor/build/test/debug/profile/docs/package workflows

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-032.01 — Reconcile existing IDE/workbench**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for IDE/workbench. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: project/editor/build/test/debug/profile/docs/package workflows
- [ ] **F-DE-032.02 — Freeze the exact contract for IDE/workbench**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: project/editor/build/test/debug/profile/docs/package workflows. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-032.03 — Implement/prove: project/editor/build/test/debug/profile/docs/package workflows**
  - Action: For IDE/workbench, implement or reuse and verify this exact obligation: project/editor/build/test/debug/profile/docs/package workflows. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for project/editor/build/test/debug/profile/docs/package workflows; retain observable state/resource expectations.
- [ ] **F-DE-032.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to IDE/workbench: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-032.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-032.05 — Integrate into the real consumer and runtime route**
  - Action: Wire IDE/workbench into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-032.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-032.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for IDE/workbench as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-032.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-033"></a>
## F-DE-033 — version control client

**Original requirement:** repository/status/diff/commit/branch/network auth with secrets isolation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-033.01 — Reconcile existing version control client**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for version control client. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: repository/status/diff/commit/branch/network auth with secrets isolation
- [ ] **F-DE-033.02 — Freeze the exact contract for version control client**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: repository/status/diff/commit/branch/network auth with secrets isolation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-033.03 — Implement/prove: repository/status/diff/commit/branch/network auth with secrets isolation**
  - Action: For version control client, implement or reuse and verify this exact obligation: repository/status/diff/commit/branch/network auth with secrets isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for repository/status/diff/commit/branch/network auth with secrets isolation; retain observable state/resource expectations.
- [ ] **F-DE-033.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to version control client: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-033.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-033.05 — Integrate into the real consumer and runtime route**
  - Action: Wire version control client into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-033.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-033.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for version control client as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-033.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-034"></a>
## F-DE-034 — self-hosted build

**Original requirement:** matching compiler/system/packages rebuilt inside zlOS from declared inputs

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-034.01 — Reconcile existing self-hosted build**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for self-hosted build. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: matching compiler/system/packages rebuilt inside zlOS from declared inputs
- [ ] **F-DE-034.02 — Freeze the exact contract for self-hosted build**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: matching compiler/system/packages rebuilt inside zlOS from declared inputs. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-034.03 — Implement/prove: matching compiler/system/packages rebuilt inside zlOS from declared inputs**
  - Action: For self-hosted build, implement or reuse and verify this exact obligation: matching compiler/system/packages rebuilt inside zlOS from declared inputs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for matching compiler/system/packages rebuilt inside zlOS from declared inputs; retain observable state/resource expectations.
- [ ] **F-DE-034.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to self-hosted build: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-034.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-034.05 — Integrate into the real consumer and runtime route**
  - Action: Wire self-hosted build into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-034.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-034.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for self-hosted build as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-034.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-035"></a>
## F-DE-035 — developer diagnostics mode

**Original requirement:** extra checks/traces without weakening production authority or changing semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-035.01 — Reconcile existing developer diagnostics mode**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for developer diagnostics mode. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: extra checks/traces without weakening production authority or changing semantics
- [ ] **F-DE-035.02 — Freeze the exact contract for developer diagnostics mode**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: extra checks/traces without weakening production authority or changing semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-035.03 — Implement/prove: extra checks/traces without weakening production authority or changing semantics**
  - Action: For developer diagnostics mode, implement or reuse and verify this exact obligation: extra checks/traces without weakening production authority or changing semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for extra checks/traces without weakening production authority or changing semantics; retain observable state/resource expectations.
- [ ] **F-DE-035.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to developer diagnostics mode: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-035.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-035.05 — Integrate into the real consumer and runtime route**
  - Action: Wire developer diagnostics mode into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-035.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-035.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for developer diagnostics mode as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-035.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-036"></a>
## F-DE-036 — crash symbolizer

**Original requirement:** checked build-ID symbol sources, safe malformed-object handling and source mapping

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-036.01 — Reconcile existing crash symbolizer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for crash symbolizer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked build-ID symbol sources, safe malformed-object handling and source mapping
- [ ] **F-DE-036.02 — Freeze the exact contract for crash symbolizer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked build-ID symbol sources, safe malformed-object handling and source mapping. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-036.03 — Implement/prove: checked build-ID symbol sources**
  - Action: For crash symbolizer, implement or reuse and verify this exact obligation: checked build-ID symbol sources. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked build-ID symbol sources; retain observable state/resource expectations.
- [ ] **F-DE-036.04 — Implement/prove: safe malformed-object handling and source mapping**
  - Action: For crash symbolizer, implement or reuse and verify this exact obligation: safe malformed-object handling and source mapping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-036.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe malformed-object handling and source mapping; retain observable state/resource expectations.
- [ ] **F-DE-036.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to crash symbolizer: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-036.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-036.06 — Integrate into the real consumer and runtime route**
  - Action: Wire crash symbolizer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-036.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-036.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for crash symbolizer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-036.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-037"></a>
## F-DE-037 — binary inspectors

**Original requirement:** ELF/PE/archive/fs/protocol structure with length-first parsing and read-only defaults

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-037.01 — Reconcile existing binary inspectors**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for binary inspectors. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: ELF/PE/archive/fs/protocol structure with length-first parsing and read-only defaults
- [ ] **F-DE-037.02 — Freeze the exact contract for binary inspectors**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: ELF/PE/archive/fs/protocol structure with length-first parsing and read-only defaults. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-037.03 — Implement/prove: ELF/PE/archive/fs/protocol structure with length-first parsing and read-only defaults**
  - Action: For binary inspectors, implement or reuse and verify this exact obligation: ELF/PE/archive/fs/protocol structure with length-first parsing and read-only defaults. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ELF/PE/archive/fs/protocol structure with length-first parsing and read-only defaults; retain observable state/resource expectations.
- [ ] **F-DE-037.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to binary inspectors: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-037.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-037.05 — Integrate into the real consumer and runtime route**
  - Action: Wire binary inspectors into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-037.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-037.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for binary inspectors as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-037.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-038"></a>
## F-DE-038 — visual UI inspector

**Original requirement:** widget/semantic/layout/tokens/damage/performance inspection with debug grant

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-038.01 — Reconcile existing visual UI inspector**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for visual UI inspector. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: widget/semantic/layout/tokens/damage/performance inspection with debug grant
- [ ] **F-DE-038.02 — Freeze the exact contract for visual UI inspector**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: widget/semantic/layout/tokens/damage/performance inspection with debug grant. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-038.03 — Implement/prove: widget/semantic/layout/tokens/damage/performance inspection with debug grant**
  - Action: For visual UI inspector, implement or reuse and verify this exact obligation: widget/semantic/layout/tokens/damage/performance inspection with debug grant. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for widget/semantic/layout/tokens/damage/performance inspection with debug grant; retain observable state/resource expectations.
- [ ] **F-DE-038.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to visual UI inspector: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-038.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-038.05 — Integrate into the real consumer and runtime route**
  - Action: Wire visual UI inspector into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-038.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-038.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for visual UI inspector as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-038.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-039"></a>
## F-DE-039 — browser developer tools

**Original requirement:** console/network/DOM/style/layout/performance/storage under content isolation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-039.01 — Reconcile existing browser developer tools**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for browser developer tools. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: console/network/DOM/style/layout/performance/storage under content isolation
- [ ] **F-DE-039.02 — Freeze the exact contract for browser developer tools**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: console/network/DOM/style/layout/performance/storage under content isolation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-039.03 — Implement/prove: console/network/DOM/style/layout/performance/storage under content isolation**
  - Action: For browser developer tools, implement or reuse and verify this exact obligation: console/network/DOM/style/layout/performance/storage under content isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for console/network/DOM/style/layout/performance/storage under content isolation; retain observable state/resource expectations.
- [ ] **F-DE-039.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to browser developer tools: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-039.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-039.05 — Integrate into the real consumer and runtime route**
  - Action: Wire browser developer tools into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-039.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-039.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for browser developer tools as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-039.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-de-040"></a>
## F-DE-040 — documentation browser

**Original requirement:** offline version-matched docs, search, examples, provenance and links

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DE-040.01 — Reconcile existing documentation browser**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for documentation browser. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: offline version-matched docs, search, examples, provenance and links
- [ ] **F-DE-040.02 — Freeze the exact contract for documentation browser**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: offline version-matched docs, search, examples, provenance and links. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DE-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DE-040.03 — Implement/prove: offline version-matched docs**
  - Action: For documentation browser, implement or reuse and verify this exact obligation: offline version-matched docs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for offline version-matched docs; retain observable state/resource expectations.
- [ ] **F-DE-040.04 — Implement/prove: search**
  - Action: For documentation browser, implement or reuse and verify this exact obligation: search. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-040.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for search; retain observable state/resource expectations.
- [ ] **F-DE-040.05 — Implement/prove: examples**
  - Action: For documentation browser, implement or reuse and verify this exact obligation: examples. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-040.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for examples; retain observable state/resource expectations.
- [ ] **F-DE-040.06 — Implement/prove: provenance and links**
  - Action: For documentation browser, implement or reuse and verify this exact obligation: provenance and links. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DE-040.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance and links; retain observable state/resource expectations.
- [ ] **F-DE-040.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to documentation browser: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DE-040.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DE-040.08 — Integrate into the real consumer and runtime route**
  - Action: Wire documentation browser into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DE-040.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DE-040.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for documentation browser as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DE-040.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-001"></a>
## F-CP-001 — POSIX compatibility ledger

**Original requirement:** per-function semantic tests driven by selected real programs

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-001.01 — Reconcile existing POSIX compatibility ledger**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for POSIX compatibility ledger. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-function semantic tests driven by selected real programs
- [ ] **F-CP-001.02 — Freeze the exact contract for POSIX compatibility ledger**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-function semantic tests driven by selected real programs. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-001.03 — Implement/prove: per-function semantic tests driven by selected real programs**
  - Action: For POSIX compatibility ledger, implement or reuse and verify this exact obligation: per-function semantic tests driven by selected real programs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-function semantic tests driven by selected real programs; retain observable state/resource expectations.
- [ ] **F-CP-001.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to POSIX compatibility ledger: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-001.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-001.05 — Integrate into the real consumer and runtime route**
  - Action: Wire POSIX compatibility ledger into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-001.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-001.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for POSIX compatibility ledger as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-001.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-002"></a>
## F-CP-002 — libc

**Original requirement:** headers/runtime/threads/time/files/network/locale with conformance and ABI

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-002.01 — Reconcile existing libc**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for libc. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: headers/runtime/threads/time/files/network/locale with conformance and ABI
- [ ] **F-CP-002.02 — Freeze the exact contract for libc**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: headers/runtime/threads/time/files/network/locale with conformance and ABI. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-002.03 — Implement/prove: headers/runtime/threads/time/files/network/locale with conformance and ABI**
  - Action: For libc, implement or reuse and verify this exact obligation: headers/runtime/threads/time/files/network/locale with conformance and ABI. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for headers/runtime/threads/time/files/network/locale with conformance and ABI; retain observable state/resource expectations.
- [ ] **F-CP-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to libc: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire libc into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for libc as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-003"></a>
## F-CP-003 — C/C++ application ABI

**Original requirement:** startup, TLS, exceptions where selected, dynamic/static linking and ownership

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-003.01 — Reconcile existing C/C++ application ABI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for C/C++ application ABI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: startup, TLS, exceptions where selected, dynamic/static linking and ownership
- [ ] **F-CP-003.02 — Freeze the exact contract for C/C++ application ABI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: startup, TLS, exceptions where selected, dynamic/static linking and ownership. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-003.03 — Implement/prove: startup**
  - Action: For C/C++ application ABI, implement or reuse and verify this exact obligation: startup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for startup; retain observable state/resource expectations.
- [ ] **F-CP-003.04 — Implement/prove: TLS**
  - Action: For C/C++ application ABI, implement or reuse and verify this exact obligation: TLS. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for TLS; retain observable state/resource expectations.
- [ ] **F-CP-003.05 — Implement/prove: exceptions where selected**
  - Action: For C/C++ application ABI, implement or reuse and verify this exact obligation: exceptions where selected. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exceptions where selected; retain observable state/resource expectations.
- [ ] **F-CP-003.06 — Implement/prove: dynamic/static linking and ownership**
  - Action: For C/C++ application ABI, implement or reuse and verify this exact obligation: dynamic/static linking and ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-003.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dynamic/static linking and ownership; retain observable state/resource expectations.
- [ ] **F-CP-003.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to C/C++ application ABI: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-003.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-003.08 — Integrate into the real consumer and runtime route**
  - Action: Wire C/C++ application ABI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-003.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-003.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for C/C++ application ABI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-003.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-004"></a>
## F-CP-004 — ELF dynamic loader

**Original requirement:** length-first segments/relocations/dependencies/symbols/TLS/W^X and atomic caches

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-004.01 — Reconcile existing ELF dynamic loader**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for ELF dynamic loader. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: length-first segments/relocations/dependencies/symbols/TLS/W^X and atomic caches
- [ ] **F-CP-004.02 — Freeze the exact contract for ELF dynamic loader**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: length-first segments/relocations/dependencies/symbols/TLS/W^X and atomic caches. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-004.03 — Implement/prove: length-first segments/relocations/dependencies/symbols/TLS/W^X and atomic caches**
  - Action: For ELF dynamic loader, implement or reuse and verify this exact obligation: length-first segments/relocations/dependencies/symbols/TLS/W^X and atomic caches. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for length-first segments/relocations/dependencies/symbols/TLS/W^X and atomic caches; retain observable state/resource expectations.
- [ ] **F-CP-004.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to ELF dynamic loader: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-004.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-004.05 — Integrate into the real consumer and runtime route**
  - Action: Wire ELF dynamic loader into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-004.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-004.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for ELF dynamic loader as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-004.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-005"></a>
## F-CP-005 — shared libraries

**Original requirement:** ABI/version/soname/dependency/refcount/unload policy and reproducible packages

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-005.01 — Reconcile existing shared libraries**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for shared libraries. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: ABI/version/soname/dependency/refcount/unload policy and reproducible packages
- [ ] **F-CP-005.02 — Freeze the exact contract for shared libraries**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: ABI/version/soname/dependency/refcount/unload policy and reproducible packages. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-005.03 — Implement/prove: ABI/version/soname/dependency/refcount/unload policy and reproducible packages**
  - Action: For shared libraries, implement or reuse and verify this exact obligation: ABI/version/soname/dependency/refcount/unload policy and reproducible packages. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ABI/version/soname/dependency/refcount/unload policy and reproducible packages; retain observable state/resource expectations.
- [ ] **F-CP-005.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to shared libraries: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-005.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-005.05 — Integrate into the real consumer and runtime route**
  - Action: Wire shared libraries into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-005.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-005.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for shared libraries as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-005.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-006"></a>
## F-CP-006 — Windows/PE compatibility subsystem

**Original requirement:** quarantined PE/DLL loader plus program-driven NT/Win32, files, registry, windows, graphics, input, audio, network, COM and selected Direct3D translation after native process contracts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-006.01 — Reconcile existing Windows/PE compatibility subsystem**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Windows/PE compatibility subsystem. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: quarantined PE/DLL loader plus program-driven NT/Win32, files, registry, windows, graphics, input, audio, network, COM and selected Direct3D translation after native process contracts
- [ ] **F-CP-006.02 — Freeze the exact contract for Windows/PE compatibility subsystem**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: quarantined PE/DLL loader plus program-driven NT/Win32, files, registry, windows, graphics, input, audio, network, COM and selected Direct3D translation after native process contracts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-006.03 — Implement/prove: quarantined PE/DLL loader plus program-driven NT/Win32**
  - Action: For Windows/PE compatibility subsystem, implement or reuse and verify this exact obligation: quarantined PE/DLL loader plus program-driven NT/Win32. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quarantined PE/DLL loader plus program-driven NT/Win32; retain observable state/resource expectations.
- [ ] **F-CP-006.04 — Implement/prove: files**
  - Action: For Windows/PE compatibility subsystem, implement or reuse and verify this exact obligation: files. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for files; retain observable state/resource expectations.
- [ ] **F-CP-006.05 — Implement/prove: registry**
  - Action: For Windows/PE compatibility subsystem, implement or reuse and verify this exact obligation: registry. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for registry; retain observable state/resource expectations.
- [ ] **F-CP-006.06 — Implement/prove: windows**
  - Action: For Windows/PE compatibility subsystem, implement or reuse and verify this exact obligation: windows. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-006.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for windows; retain observable state/resource expectations.
- [ ] **F-CP-006.07 — Implement/prove: graphics**
  - Action: For Windows/PE compatibility subsystem, implement or reuse and verify this exact obligation: graphics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-006.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for graphics; retain observable state/resource expectations.
- [ ] **F-CP-006.08 — Implement/prove: input**
  - Action: For Windows/PE compatibility subsystem, implement or reuse and verify this exact obligation: input. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-006.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input; retain observable state/resource expectations.
- [ ] **F-CP-006.09 — Implement/prove: audio**
  - Action: For Windows/PE compatibility subsystem, implement or reuse and verify this exact obligation: audio. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-006.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for audio; retain observable state/resource expectations.
- [ ] **F-CP-006.10 — Implement/prove: network**
  - Action: For Windows/PE compatibility subsystem, implement or reuse and verify this exact obligation: network. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-006.09.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for network; retain observable state/resource expectations.
- [ ] **F-CP-006.11 — Implement/prove: COM and selected Direct3D translation after native process contracts**
  - Action: For Windows/PE compatibility subsystem, implement or reuse and verify this exact obligation: COM and selected Direct3D translation after native process contracts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-006.10.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for COM and selected Direct3D translation after native process contracts; retain observable state/resource expectations.
- [ ] **F-CP-006.12 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Windows/PE compatibility subsystem: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-006.11.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-006.13 — Integrate into the real consumer and runtime route**
  - Action: Wire Windows/PE compatibility subsystem into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-006.12.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-006.14 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Windows/PE compatibility subsystem as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-006.13.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-007"></a>
## F-CP-007 — Linux ABI and userland compatibility

**Original requirement:** isolated program-driven Linux syscalls, ELF interpreter/dynamic linker, signals/futex, `/proc`/`/dev`, files/network and selected userland/container workloads after native process contracts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-007.01 — Reconcile existing Linux ABI and userland compatibility**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Linux ABI and userland compatibility. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated program-driven Linux syscalls, ELF interpreter/dynamic linker, signals/futex, `/proc`/`/dev`, files/network and selected userland/container workloads after native process contracts
- [ ] **F-CP-007.02 — Freeze the exact contract for Linux ABI and userland compatibility**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated program-driven Linux syscalls, ELF interpreter/dynamic linker, signals/futex, `/proc`/`/dev`, files/network and selected userland/container workloads after native process contracts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-007.03 — Implement/prove: isolated program-driven Linux syscalls**
  - Action: For Linux ABI and userland compatibility, implement or reuse and verify this exact obligation: isolated program-driven Linux syscalls. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated program-driven Linux syscalls; retain observable state/resource expectations.
- [ ] **F-CP-007.04 — Implement/prove: ELF interpreter/dynamic linker**
  - Action: For Linux ABI and userland compatibility, implement or reuse and verify this exact obligation: ELF interpreter/dynamic linker. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ELF interpreter/dynamic linker; retain observable state/resource expectations.
- [ ] **F-CP-007.05 — Implement/prove: signals/futex**
  - Action: For Linux ABI and userland compatibility, implement or reuse and verify this exact obligation: signals/futex. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signals/futex; retain observable state/resource expectations.
- [ ] **F-CP-007.06 — Implement/prove: `/proc`/`/dev`**
  - Action: For Linux ABI and userland compatibility, implement or reuse and verify this exact obligation: `/proc`/`/dev`. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-007.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for `/proc`/`/dev`; retain observable state/resource expectations.
- [ ] **F-CP-007.07 — Implement/prove: files/network and selected userland/container workloads after native process contracts**
  - Action: For Linux ABI and userland compatibility, implement or reuse and verify this exact obligation: files/network and selected userland/container workloads after native process contracts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-007.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for files/network and selected userland/container workloads after native process contracts; retain observable state/resource expectations.
- [ ] **F-CP-007.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Linux ABI and userland compatibility: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-007.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-007.09 — Integrate into the real consumer and runtime route**
  - Action: Wire Linux ABI and userland compatibility into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-007.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-007.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Linux ABI and userland compatibility as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-007.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-008"></a>
## F-CP-008 — Wayland-like client protocol

**Original requirement:** versioned bounded surface/input/output protocol and compatibility tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-008.01 — Reconcile existing Wayland-like client protocol**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Wayland-like client protocol. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: versioned bounded surface/input/output protocol and compatibility tests
- [ ] **F-CP-008.02 — Freeze the exact contract for Wayland-like client protocol**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: versioned bounded surface/input/output protocol and compatibility tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-008.03 — Implement/prove: versioned bounded surface/input/output protocol and compatibility tests**
  - Action: For Wayland-like client protocol, implement or reuse and verify this exact obligation: versioned bounded surface/input/output protocol and compatibility tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioned bounded surface/input/output protocol and compatibility tests; retain observable state/resource expectations.
- [ ] **F-CP-008.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Wayland-like client protocol: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-008.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-008.05 — Integrate into the real consumer and runtime route**
  - Action: Wire Wayland-like client protocol into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-008.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-008.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Wayland-like client protocol as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-008.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-009"></a>
## F-CP-009 — SDL-like app layer

**Original requirement:** graphics/input/audio/files/time/thread subset for selected ports

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-009.01 — Reconcile existing SDL-like app layer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for SDL-like app layer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: graphics/input/audio/files/time/thread subset for selected ports
- [ ] **F-CP-009.02 — Freeze the exact contract for SDL-like app layer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: graphics/input/audio/files/time/thread subset for selected ports. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-009.03 — Implement/prove: graphics/input/audio/files/time/thread subset for selected ports**
  - Action: For SDL-like app layer, implement or reuse and verify this exact obligation: graphics/input/audio/files/time/thread subset for selected ports. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for graphics/input/audio/files/time/thread subset for selected ports; retain observable state/resource expectations.
- [ ] **F-CP-009.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to SDL-like app layer: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-009.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-009.05 — Integrate into the real consumer and runtime route**
  - Action: Wire SDL-like app layer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-009.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-009.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for SDL-like app layer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-009.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-010"></a>
## F-CP-010 — terminal/ANSI compatibility

**Original requirement:** explicit escape/Unicode/resize/input table and hostile sequence bounds

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-010.01 — Reconcile existing terminal/ANSI compatibility**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for terminal/ANSI compatibility. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit escape/Unicode/resize/input table and hostile sequence bounds
- [ ] **F-CP-010.02 — Freeze the exact contract for terminal/ANSI compatibility**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit escape/Unicode/resize/input table and hostile sequence bounds. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-010.03 — Implement/prove: explicit escape/Unicode/resize/input table and hostile sequence bounds**
  - Action: For terminal/ANSI compatibility, implement or reuse and verify this exact obligation: explicit escape/Unicode/resize/input table and hostile sequence bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit escape/Unicode/resize/input table and hostile sequence bounds; retain observable state/resource expectations.
- [ ] **F-CP-010.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to terminal/ANSI compatibility: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-010.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-010.05 — Integrate into the real consumer and runtime route**
  - Action: Wire terminal/ANSI compatibility into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-010.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-010.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for terminal/ANSI compatibility as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-010.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-011"></a>
## F-CP-011 — third-party port framework

**Original requirement:** pinned recipe, patches, licenses, dependencies, sandbox, build and scenario proof

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-011.01 — Reconcile existing third-party port framework**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for third-party port framework. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: pinned recipe, patches, licenses, dependencies, sandbox, build and scenario proof
- [ ] **F-CP-011.02 — Freeze the exact contract for third-party port framework**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: pinned recipe, patches, licenses, dependencies, sandbox, build and scenario proof. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-011.03 — Implement/prove: pinned recipe**
  - Action: For third-party port framework, implement or reuse and verify this exact obligation: pinned recipe. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pinned recipe; retain observable state/resource expectations.
- [ ] **F-CP-011.04 — Implement/prove: patches**
  - Action: For third-party port framework, implement or reuse and verify this exact obligation: patches. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for patches; retain observable state/resource expectations.
- [ ] **F-CP-011.05 — Implement/prove: licenses**
  - Action: For third-party port framework, implement or reuse and verify this exact obligation: licenses. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for licenses; retain observable state/resource expectations.
- [ ] **F-CP-011.06 — Implement/prove: dependencies**
  - Action: For third-party port framework, implement or reuse and verify this exact obligation: dependencies. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-011.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dependencies; retain observable state/resource expectations.
- [ ] **F-CP-011.07 — Implement/prove: sandbox**
  - Action: For third-party port framework, implement or reuse and verify this exact obligation: sandbox. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-011.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sandbox; retain observable state/resource expectations.
- [ ] **F-CP-011.08 — Implement/prove: build and scenario proof**
  - Action: For third-party port framework, implement or reuse and verify this exact obligation: build and scenario proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-011.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for build and scenario proof; retain observable state/resource expectations.
- [ ] **F-CP-011.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to third-party port framework: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-011.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-011.10 — Integrate into the real consumer and runtime route**
  - Action: Wire third-party port framework into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-011.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-011.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for third-party port framework as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-011.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-012"></a>
## F-CP-012 — port catalogue

**Original requirement:** installed vs available, origin, license, version, capabilities and test status

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-012.01 — Reconcile existing port catalogue**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for port catalogue. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: installed vs available, origin, license, version, capabilities and test status
- [ ] **F-CP-012.02 — Freeze the exact contract for port catalogue**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: installed vs available, origin, license, version, capabilities and test status. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-012.03 — Implement/prove: installed vs available**
  - Action: For port catalogue, implement or reuse and verify this exact obligation: installed vs available. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for installed vs available; retain observable state/resource expectations.
- [ ] **F-CP-012.04 — Implement/prove: origin**
  - Action: For port catalogue, implement or reuse and verify this exact obligation: origin. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for origin; retain observable state/resource expectations.
- [ ] **F-CP-012.05 — Implement/prove: license**
  - Action: For port catalogue, implement or reuse and verify this exact obligation: license. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for license; retain observable state/resource expectations.
- [ ] **F-CP-012.06 — Implement/prove: version**
  - Action: For port catalogue, implement or reuse and verify this exact obligation: version. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-012.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for version; retain observable state/resource expectations.
- [ ] **F-CP-012.07 — Implement/prove: capabilities and test status**
  - Action: For port catalogue, implement or reuse and verify this exact obligation: capabilities and test status. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-012.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capabilities and test status; retain observable state/resource expectations.
- [ ] **F-CP-012.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to port catalogue: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-012.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-012.09 — Integrate into the real consumer and runtime route**
  - Action: Wire port catalogue into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-012.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-012.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for port catalogue as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-012.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-013"></a>
## F-CP-013 — DOOM/game ports

**Original requirement:** integration workload with exact runtime/audio/input/file evidence, not subsystem proof

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-013.01 — Reconcile existing DOOM/game ports**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for DOOM/game ports. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: integration workload with exact runtime/audio/input/file evidence, not subsystem proof
- [ ] **F-CP-013.02 — Freeze the exact contract for DOOM/game ports**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: integration workload with exact runtime/audio/input/file evidence, not subsystem proof. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-013.03 — Implement/prove: integration workload with exact runtime/audio/input/file evidence**
  - Action: For DOOM/game ports, implement or reuse and verify this exact obligation: integration workload with exact runtime/audio/input/file evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for integration workload with exact runtime/audio/input/file evidence; retain observable state/resource expectations.
- [ ] **F-CP-013.04 — Implement/prove: not subsystem proof**
  - Action: For DOOM/game ports, implement or reuse and verify this exact obligation: not subsystem proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for not subsystem proof; retain observable state/resource expectations.
- [ ] **F-CP-013.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to DOOM/game ports: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-013.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-013.06 — Integrate into the real consumer and runtime route**
  - Action: Wire DOOM/game ports into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-013.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-013.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for DOOM/game ports as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-013.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-014"></a>
## F-CP-014 — Unix utilities ports

**Original requirement:** selected real workflows under native ownership/security semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-014.01 — Reconcile existing Unix utilities ports**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Unix utilities ports. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: selected real workflows under native ownership/security semantics
- [ ] **F-CP-014.02 — Freeze the exact contract for Unix utilities ports**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: selected real workflows under native ownership/security semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-014.03 — Implement/prove: selected real workflows under native ownership/security semantics**
  - Action: For Unix utilities ports, implement or reuse and verify this exact obligation: selected real workflows under native ownership/security semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for selected real workflows under native ownership/security semantics; retain observable state/resource expectations.
- [ ] **F-CP-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Unix utilities ports: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire Unix utilities ports into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Unix utilities ports as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-015"></a>
## F-CP-015 — language runtime ports

**Original requirement:** runtime-specific sandbox/resource/FFI/file/network tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-015.01 — Reconcile existing language runtime ports**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for language runtime ports. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: runtime-specific sandbox/resource/FFI/file/network tests
- [ ] **F-CP-015.02 — Freeze the exact contract for language runtime ports**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: runtime-specific sandbox/resource/FFI/file/network tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-015.03 — Implement/prove: runtime-specific sandbox/resource/FFI/file/network tests**
  - Action: For language runtime ports, implement or reuse and verify this exact obligation: runtime-specific sandbox/resource/FFI/file/network tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for runtime-specific sandbox/resource/FFI/file/network tests; retain observable state/resource expectations.
- [ ] **F-CP-015.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to language runtime ports: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-015.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-015.05 — Integrate into the real consumer and runtime route**
  - Action: Wire language runtime ports into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-015.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-015.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for language runtime ports as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-015.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-016"></a>
## F-CP-016 — Windows/Linux virtual machine monitor

**Original requirement:** hardware-assisted guest CPU/memory/device isolation, lifecycle, licensed images, console, networking, snapshots and limits

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-016.01 — Reconcile existing Windows/Linux virtual machine monitor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Windows/Linux virtual machine monitor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: hardware-assisted guest CPU/memory/device isolation, lifecycle, licensed images, console, networking, snapshots and limits
- [ ] **F-CP-016.02 — Freeze the exact contract for Windows/Linux virtual machine monitor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: hardware-assisted guest CPU/memory/device isolation, lifecycle, licensed images, console, networking, snapshots and limits. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-016.03 — Implement/prove: hardware-assisted guest CPU/memory/device isolation**
  - Action: For Windows/Linux virtual machine monitor, implement or reuse and verify this exact obligation: hardware-assisted guest CPU/memory/device isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hardware-assisted guest CPU/memory/device isolation; retain observable state/resource expectations.
- [ ] **F-CP-016.04 — Implement/prove: lifecycle**
  - Action: For Windows/Linux virtual machine monitor, implement or reuse and verify this exact obligation: lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lifecycle; retain observable state/resource expectations.
- [ ] **F-CP-016.05 — Implement/prove: licensed images**
  - Action: For Windows/Linux virtual machine monitor, implement or reuse and verify this exact obligation: licensed images. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for licensed images; retain observable state/resource expectations.
- [ ] **F-CP-016.06 — Implement/prove: console**
  - Action: For Windows/Linux virtual machine monitor, implement or reuse and verify this exact obligation: console. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-016.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for console; retain observable state/resource expectations.
- [ ] **F-CP-016.07 — Implement/prove: networking**
  - Action: For Windows/Linux virtual machine monitor, implement or reuse and verify this exact obligation: networking. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-016.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for networking; retain observable state/resource expectations.
- [ ] **F-CP-016.08 — Implement/prove: snapshots and limits**
  - Action: For Windows/Linux virtual machine monitor, implement or reuse and verify this exact obligation: snapshots and limits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-016.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for snapshots and limits; retain observable state/resource expectations.
- [ ] **F-CP-016.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Windows/Linux virtual machine monitor: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-016.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-016.10 — Integrate into the real consumer and runtime route**
  - Action: Wire Windows/Linux virtual machine monitor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-016.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-016.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Windows/Linux virtual machine monitor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-016.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-017"></a>
## F-CP-017 — Windows/Linux guest integration

**Original requirement:** time/input/display/audio/network/clipboard/files and optional seamless windows with explicit consent and protocol versions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-017.01 — Reconcile existing Windows/Linux guest integration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Windows/Linux guest integration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: time/input/display/audio/network/clipboard/files and optional seamless windows with explicit consent and protocol versions
- [ ] **F-CP-017.02 — Freeze the exact contract for Windows/Linux guest integration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: time/input/display/audio/network/clipboard/files and optional seamless windows with explicit consent and protocol versions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-017.03 — Implement/prove: time/input/display/audio/network/clipboard/files and optional seamless windows with explicit consent and protocol versions**
  - Action: For Windows/Linux guest integration, implement or reuse and verify this exact obligation: time/input/display/audio/network/clipboard/files and optional seamless windows with explicit consent and protocol versions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for time/input/display/audio/network/clipboard/files and optional seamless windows with explicit consent and protocol versions; retain observable state/resource expectations.
- [ ] **F-CP-017.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Windows/Linux guest integration: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-017.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-017.05 — Integrate into the real consumer and runtime route**
  - Action: Wire Windows/Linux guest integration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-017.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-017.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Windows/Linux guest integration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-017.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-018"></a>
## F-CP-018 — CPU and device emulation

**Original requirement:** deterministic selected ISA plus virtio/legacy device models, translation caches, quotas and fault injection

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-018.01 — Reconcile existing CPU and device emulation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for CPU and device emulation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic selected ISA plus virtio/legacy device models, translation caches, quotas and fault injection
- [ ] **F-CP-018.02 — Freeze the exact contract for CPU and device emulation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic selected ISA plus virtio/legacy device models, translation caches, quotas and fault injection. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-018.03 — Implement/prove: deterministic selected ISA plus virtio/legacy device models**
  - Action: For CPU and device emulation, implement or reuse and verify this exact obligation: deterministic selected ISA plus virtio/legacy device models. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic selected ISA plus virtio/legacy device models; retain observable state/resource expectations.
- [ ] **F-CP-018.04 — Implement/prove: translation caches**
  - Action: For CPU and device emulation, implement or reuse and verify this exact obligation: translation caches. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for translation caches; retain observable state/resource expectations.
- [ ] **F-CP-018.05 — Implement/prove: quotas and fault injection**
  - Action: For CPU and device emulation, implement or reuse and verify this exact obligation: quotas and fault injection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quotas and fault injection; retain observable state/resource expectations.
- [ ] **F-CP-018.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to CPU and device emulation: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-018.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-018.07 — Integrate into the real consumer and runtime route**
  - Action: Wire CPU and device emulation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-018.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-018.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for CPU and device emulation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-018.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-019"></a>
## F-CP-019 — Game Boy/retro emulation

**Original requirement:** legal ROM boundary, CPU/video/audio/input/save correctness and quotas

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-019.01 — Reconcile existing Game Boy/retro emulation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Game Boy/retro emulation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: legal ROM boundary, CPU/video/audio/input/save correctness and quotas
- [ ] **F-CP-019.02 — Freeze the exact contract for Game Boy/retro emulation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: legal ROM boundary, CPU/video/audio/input/save correctness and quotas. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-019.03 — Implement/prove: legal ROM boundary**
  - Action: For Game Boy/retro emulation, implement or reuse and verify this exact obligation: legal ROM boundary. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for legal ROM boundary; retain observable state/resource expectations.
- [ ] **F-CP-019.04 — Implement/prove: CPU/video/audio/input/save correctness and quotas**
  - Action: For Game Boy/retro emulation, implement or reuse and verify this exact obligation: CPU/video/audio/input/save correctness and quotas. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for CPU/video/audio/input/save correctness and quotas; retain observable state/resource expectations.
- [ ] **F-CP-019.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Game Boy/retro emulation: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-019.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-019.06 — Integrate into the real consumer and runtime route**
  - Action: Wire Game Boy/retro emulation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-019.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-019.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Game Boy/retro emulation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-019.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-cp-020"></a>
## F-CP-020 — compatibility evidence UI

**Original requirement:** shows exact supported programs/APIs/tests and failures instead of broad labels

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-CP-020.01 — Reconcile existing compatibility evidence UI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for compatibility evidence UI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: shows exact supported programs/APIs/tests and failures instead of broad labels
- [ ] **F-CP-020.02 — Freeze the exact contract for compatibility evidence UI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: shows exact supported programs/APIs/tests and failures instead of broad labels. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-CP-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-CP-020.03 — Implement/prove: shows exact supported programs/APIs/tests and failures instead of broad labels**
  - Action: For compatibility evidence UI, implement or reuse and verify this exact obligation: shows exact supported programs/APIs/tests and failures instead of broad labels. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-CP-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shows exact supported programs/APIs/tests and failures instead of broad labels; retain observable state/resource expectations.
- [ ] **F-CP-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to compatibility evidence UI: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-CP-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-CP-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire compatibility evidence UI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-CP-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-CP-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for compatibility evidence UI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-CP-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p7-4"></a>
## C-P7.4 — reproducible port recipe and availability gate

**Original requirement:** reproducible port recipe and availability gate

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 407.

### Preserved original contract

- **Dependencies/current/provenance:** P7.3 and toolchain; banan recipes/freestanding header provenance; reject recipe/build as runtime availability.
- **I/O and state:** pinned URL/commit/archive digest, patches, dependency/toolchain digests in; content-addressed package out.
- **Invariants/failure:** no mutable/unverified fetch; patch hash recorded; network optional after source cache; host contamination detected.
- **Deterministic proof:** rebuild comparison, wrong digest, changed patch/dependency, offline cache, install and launch smoke.
- **Target proof:** QEMU app launch/behavior; hardware only when driver/media interaction matters.
- **Receipt/removal:** source/build/package/install/launch chain; failed port remains unlisted; old artifact retained until replacement passes.

### Execution steps

- [ ] **C-P7.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P7.4.02 — Resolve this contract's exact dependencies**
  - Action: P7.3 and toolchain; banan recipes/freestanding header provenance; reject recipe/build as runtime availability. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P7.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P7.4.03 — I/O and state — reproducible port recipe and availability gate**
  - Action: pinned URL/commit/archive digest, patches, dependency/toolchain digests in; content-addressed package out.
  - Requires: C-P7.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P7.4.
- [ ] **C-P7.4.04 — Invariants/failure — reproducible port recipe and availability gate**
  - Action: no mutable/unverified fetch; patch hash recorded; network optional after source cache; host contamination detected.
  - Requires: C-P7.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P7.4.
- [ ] **C-P7.4.05 — Deterministic proof — reproducible port recipe and availability gate**
  - Action: rebuild comparison, wrong digest, changed patch/dependency, offline cache, install and launch smoke.
  - Requires: C-P7.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P7.4.
- [ ] **C-P7.4.06 — Target proof — reproducible port recipe and availability gate**
  - Action: QEMU app launch/behavior; hardware only when driver/media interaction matters.
  - Requires: C-P7.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P7.4.
- [ ] **C-P7.4.07 — Receipt/removal — reproducible port recipe and availability gate**
  - Action: source/build/package/install/launch chain; failed port remains unlisted; old artifact retained until replacement passes.
  - Requires: C-P7.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P7.4.
- [ ] **C-P7.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P7.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P7.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p7-5"></a>
## C-P7.5 — program-driven POSIX/Linux compatibility

**Original requirement:** program-driven POSIX/Linux compatibility

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 416.

### Preserved original contract

- **Dependencies/current/provenance:** P7.1 and selected target program; Brook/Astral/Zinnia; reject syscall-count roadmaps.
- **I/O and state:** program plus observed ABI requirements in; conformance items/shims out.
- **Invariants/failure:** native zlOS semantics stay explicit; error behavior tested; unknown call returns `ENOSYS`; no broad ambient authority introduced.
- **Deterministic proof:** differential pure userspace behavior, syscall/error corpus, selected program regression.
- **Target proof:** QEMU launch and workflow of chosen port; hardware only if program requires it.
- **Receipt/removal:** requirement-to-test-to-program receipt; shims remain isolated; no native API removal.

### Execution steps

- [ ] **C-P7.5.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P7.5.02 — Resolve this contract's exact dependencies**
  - Action: P7.1 and selected target program; Brook/Astral/Zinnia; reject syscall-count roadmaps. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P7.5.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P7.5.03 — I/O and state — program-driven POSIX/Linux compatibility**
  - Action: program plus observed ABI requirements in; conformance items/shims out.
  - Requires: C-P7.5.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P7.5.
- [ ] **C-P7.5.04 — Invariants/failure — program-driven POSIX/Linux compatibility**
  - Action: native zlOS semantics stay explicit; error behavior tested; unknown call returns `ENOSYS`; no broad ambient authority introduced.
  - Requires: C-P7.5.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P7.5.
- [ ] **C-P7.5.05 — Deterministic proof — program-driven POSIX/Linux compatibility**
  - Action: differential pure userspace behavior, syscall/error corpus, selected program regression.
  - Requires: C-P7.5.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P7.5.
- [ ] **C-P7.5.06 — Target proof — program-driven POSIX/Linux compatibility**
  - Action: QEMU launch and workflow of chosen port; hardware only if program requires it.
  - Requires: C-P7.5.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P7.5.
- [ ] **C-P7.5.07 — Receipt/removal — program-driven POSIX/Linux compatibility**
  - Action: requirement-to-test-to-program receipt; shims remain isolated; no native API removal.
  - Requires: C-P7.5.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P7.5.
- [ ] **C-P7.5.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P7.5. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P7.5.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-plat-037"></a>
## T-PLAT-037 — legacy APM

**Original requirement:** compatibility-only power calls and failure reporting

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 66.

### Execution steps

- [ ] **T-PLAT-037.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve legacy APM to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-037.02 — Specify the complete target boundary**
  - Action: legacy APM must supply: compatibility-only power calls and failure reporting. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-037.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-037.03 — Implement the exact target behavior**
  - Action: Implement or reuse legacy APM through the shared platform contract, delivering every part of: compatibility-only power calls and failure reporting. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-037.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-037.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-037.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for legacy APM.
- [ ] **T-PLAT-037.05 — Qualify and retain this target's own result**
  - Action: Bind legacy APM to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-037.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-026"></a>
## T-BUS-026 — ISA Plug and Play

**Original requirement:** isolation/resource discovery with conflict handling

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 98.

### Execution steps

- [ ] **T-BUS-026.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ISA Plug and Play to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-026.02 — Specify the complete target boundary**
  - Action: ISA Plug and Play must supply: isolation/resource discovery with conflict handling. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-026.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-026.03 — Implement the exact target behavior**
  - Action: Implement or reuse ISA Plug and Play through the shared platform contract, delivering every part of: isolation/resource discovery with conflict handling. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-026.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-026.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-026.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ISA Plug and Play.
- [ ] **T-BUS-026.05 — Qualify and retain this target's own result**
  - Action: Bind ISA Plug and Play to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-026.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-027"></a>
## T-BUS-027 — Intel 8237 ISA DMA

**Original requirement:** channel/width/boundary ownership and bounce buffers

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 99.

### Execution steps

- [ ] **T-BUS-027.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel 8237 ISA DMA to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-027.02 — Specify the complete target boundary**
  - Action: Intel 8237 ISA DMA must supply: channel/width/boundary ownership and bounce buffers. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-027.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-027.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel 8237 ISA DMA through the shared platform contract, delivering every part of: channel/width/boundary ownership and bounce buffers. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-027.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-027.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-027.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel 8237 ISA DMA.
- [ ] **T-BUS-027.05 — Qualify and retain this target's own result**
  - Action: Bind Intel 8237 ISA DMA to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-027.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-028"></a>
## T-BUS-028 — VM86 BIOS-call bridge

**Original requirement:** bounded legacy firmware call sandbox and register contract

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 100.

### Execution steps

- [ ] **T-BUS-028.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve VM86 BIOS-call bridge to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-028.02 — Specify the complete target boundary**
  - Action: VM86 BIOS-call bridge must supply: bounded legacy firmware call sandbox and register contract. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-028.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-028.03 — Implement the exact target behavior**
  - Action: Implement or reuse VM86 BIOS-call bridge through the shared platform contract, delivering every part of: bounded legacy firmware call sandbox and register contract. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-028.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-028.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-028.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for VM86 BIOS-call bridge.
- [ ] **T-BUS-028.05 — Qualify and retain this target's own result**
  - Action: Bind VM86 BIOS-call bridge to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-028.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-015"></a>
## T-BLK-015 — floppy controller

**Original requirement:** legacy profile, media geometry and timeout

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 120.

### Execution steps

- [ ] **T-BLK-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve floppy controller to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-015.02 — Specify the complete target boundary**
  - Action: floppy controller must supply: legacy profile, media geometry and timeout. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse floppy controller through the shared platform contract, delivering every part of: legacy profile, media geometry and timeout. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-015.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for floppy controller.
- [ ] **T-BLK-015.05 — Qualify and retain this target's own result**
  - Action: Bind floppy controller to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-003"></a>
## T-GPU-003 — VBE framebuffer

**Original requirement:** legacy modes and safe mapping

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 176.

### Execution steps

- [ ] **T-GPU-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve VBE framebuffer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-003.02 — Specify the complete target boundary**
  - Action: VBE framebuffer must supply: legacy modes and safe mapping. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse VBE framebuffer through the shared platform contract, delivering every part of: legacy modes and safe mapping. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for VBE framebuffer.
- [ ] **T-GPU-003.05 — Qualify and retain this target's own result**
  - Action: Bind VBE framebuffer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-010"></a>
## T-GPU-010 — Cirrus legacy display

**Original requirement:** compatibility-only bounded modes

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 183.

### Execution steps

- [ ] **T-GPU-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Cirrus legacy display to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-010.02 — Specify the complete target boundary**
  - Action: Cirrus legacy display must supply: compatibility-only bounded modes. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse Cirrus legacy display through the shared platform contract, delivering every part of: compatibility-only bounded modes. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-010.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Cirrus legacy display.
- [ ] **T-GPU-010.05 — Qualify and retain this target's own result**
  - Action: Bind Cirrus legacy display to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-017"></a>
## T-GPU-017 — 3dfx legacy accelerator

**Original requirement:** compatibility/raster fixture

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 190.

### Execution steps

- [ ] **T-GPU-017.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve 3dfx legacy accelerator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-017.02 — Specify the complete target boundary**
  - Action: 3dfx legacy accelerator must supply: compatibility/raster fixture. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-017.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-017.03 — Implement the exact target behavior**
  - Action: Implement or reuse 3dfx legacy accelerator through the shared platform contract, delivering every part of: compatibility/raster fixture. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-017.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-017.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-017.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for 3dfx legacy accelerator.
- [ ] **T-GPU-017.05 — Qualify and retain this target's own result**
  - Action: Bind 3dfx legacy accelerator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-017.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-025"></a>
## T-GPU-025 — EGA legacy display

**Original requirement:** compatibility modes and bounded planar access

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 198.

### Execution steps

- [ ] **T-GPU-025.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve EGA legacy display to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-025.02 — Specify the complete target boundary**
  - Action: EGA legacy display must supply: compatibility modes and bounded planar access. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-025.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-025.03 — Implement the exact target behavior**
  - Action: Implement or reuse EGA legacy display through the shared platform contract, delivering every part of: compatibility modes and bounded planar access. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-025.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-025.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-025.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for EGA legacy display.
- [ ] **T-GPU-025.05 — Qualify and retain this target's own result**
  - Action: Bind EGA legacy display to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-025.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-009"></a>
## T-NIC-009 — NE2000

**Original requirement:** emulator/legacy compatibility

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 212.

### Execution steps

- [ ] **T-NIC-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve NE2000 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-009.02 — Specify the complete target boundary**
  - Action: NE2000 must supply: emulator/legacy compatibility. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse NE2000 through the shared platform contract, delivering every part of: emulator/legacy compatibility. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-009.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for NE2000.
- [ ] **T-NIC-009.05 — Qualify and retain this target's own result**
  - Action: Bind NE2000 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-015"></a>
## T-NIC-015 — tap/tun virtual interface

**Original requirement:** packet authority, queues and teardown

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 218.

### Execution steps

- [ ] **T-NIC-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve tap/tun virtual interface to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-015.02 — Specify the complete target boundary**
  - Action: tap/tun virtual interface must supply: packet authority, queues and teardown. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse tap/tun virtual interface through the shared platform contract, delivering every part of: packet authority, queues and teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-015.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for tap/tun virtual interface.
- [ ] **T-NIC-015.05 — Qualify and retain this target's own result**
  - Action: Bind tap/tun virtual interface to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-media-004"></a>
## T-MEDIA-004 — Sound Blaster 16

**Original requirement:** DSP/DMA/IRQ legacy compatibility

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 233.

### Execution steps

- [ ] **T-MEDIA-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Sound Blaster 16 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-MEDIA-004.02 — Specify the complete target boundary**
  - Action: Sound Blaster 16 must supply: DSP/DMA/IRQ legacy compatibility. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-MEDIA-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-MEDIA-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse Sound Blaster 16 through the shared platform contract, delivering every part of: DSP/DMA/IRQ legacy compatibility. Do not fork a duplicate subsystem for this row.
  - Requires: T-MEDIA-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-MEDIA-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-MEDIA-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Sound Blaster 16.
- [ ] **T-MEDIA-004.05 — Qualify and retain this target's own result**
  - Action: Bind Sound Blaster 16 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-MEDIA-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-periph-004"></a>
## T-PERIPH-004 — parallel port

**Original requirement:** ownership, status/timeout and printer compatibility

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 252.

### Execution steps

- [ ] **T-PERIPH-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve parallel port to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PERIPH-004.02 — Specify the complete target boundary**
  - Action: parallel port must supply: ownership, status/timeout and printer compatibility. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PERIPH-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PERIPH-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse parallel port through the shared platform contract, delivering every part of: ownership, status/timeout and printer compatibility. Do not fork a duplicate subsystem for this row.
  - Requires: T-PERIPH-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PERIPH-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PERIPH-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for parallel port.
- [ ] **T-PERIPH-004.05 — Qualify and retain this target's own result**
  - Action: Bind parallel port to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PERIPH-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-004"></a>
## T-VM-004 — virtio serial

**Original requirement:** bounded ports, identity and disconnect

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 261.

### Execution steps

- [ ] **T-VM-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio serial to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-004.02 — Specify the complete target boundary**
  - Action: virtio serial must supply: bounded ports, identity and disconnect. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio serial through the shared platform contract, delivering every part of: bounded ports, identity and disconnect. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-004.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio serial.
- [ ] **T-VM-004.05 — Qualify and retain this target's own result**
  - Action: Bind virtio serial to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-005"></a>
## T-VM-005 — virtio 9P/shared folder

**Original requirement:** path/permission/cache/disconnect authority

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 262.

### Execution steps

- [ ] **T-VM-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio 9P/shared folder to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-005.02 — Specify the complete target boundary**
  - Action: virtio 9P/shared folder must supply: path/permission/cache/disconnect authority. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio 9P/shared folder through the shared platform contract, delivering every part of: path/permission/cache/disconnect authority. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-005.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio 9P/shared folder.
- [ ] **T-VM-005.05 — Qualify and retain this target's own result**
  - Action: Bind virtio 9P/shared folder to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-006"></a>
## T-VM-006 — qemu-guest/vdagent integration

**Original requirement:** authenticated clipboard/display/input seams

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 263.

### Execution steps

- [ ] **T-VM-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve qemu-guest/vdagent integration to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-006.02 — Specify the complete target boundary**
  - Action: qemu-guest/vdagent integration must supply: authenticated clipboard/display/input seams. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse qemu-guest/vdagent integration through the shared platform contract, delivering every part of: authenticated clipboard/display/input seams. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-006.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for qemu-guest/vdagent integration.
- [ ] **T-VM-006.05 — Qualify and retain this target's own result**
  - Action: Bind qemu-guest/vdagent integration to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-007"></a>
## T-VM-007 — VMware guest integration

**Original requirement:** clock/mouse/SVGA/shared state authority

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 264.

### Execution steps

- [ ] **T-VM-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve VMware guest integration to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-007.02 — Specify the complete target boundary**
  - Action: VMware guest integration must supply: clock/mouse/SVGA/shared state authority. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse VMware guest integration through the shared platform contract, delivering every part of: clock/mouse/SVGA/shared state authority. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-007.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for VMware guest integration.
- [ ] **T-VM-007.05 — Qualify and retain this target's own result**
  - Action: Bind VMware guest integration to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-008"></a>
## T-VM-008 — VirtualBox VMMDev/HGCM

**Original requirement:** versioned messages, shared folders/clipboard

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 265.

### Execution steps

- [ ] **T-VM-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve VirtualBox VMMDev/HGCM to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-008.02 — Specify the complete target boundary**
  - Action: VirtualBox VMMDev/HGCM must supply: versioned messages, shared folders/clipboard. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse VirtualBox VMMDev/HGCM through the shared platform contract, delivering every part of: versioned messages, shared folders/clipboard. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-008.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for VirtualBox VMMDev/HGCM.
- [ ] **T-VM-008.05 — Qualify and retain this target's own result**
  - Action: Bind VirtualBox VMMDev/HGCM to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-009"></a>
## T-VM-009 — OSC52 clipboard bridge

**Original requirement:** explicit user consent and bounded payload

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 266.

### Execution steps

- [ ] **T-VM-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve OSC52 clipboard bridge to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-009.02 — Specify the complete target boundary**
  - Action: OSC52 clipboard bridge must supply: explicit user consent and bounded payload. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse OSC52 clipboard bridge through the shared platform contract, delivering every part of: explicit user consent and bounded payload. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-009.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for OSC52 clipboard bridge.
- [ ] **T-VM-009.05 — Qualify and retain this target's own result**
  - Action: Bind OSC52 clipboard bridge to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-012"></a>
## T-VM-012 — virtio console/ports

**Original requirement:** negotiated ports, bounded streams and disconnect cleanup

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 269.

### Execution steps

- [ ] **T-VM-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio console/ports to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-012.02 — Specify the complete target boundary**
  - Action: virtio console/ports must supply: negotiated ports, bounded streams and disconnect cleanup. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio console/ports through the shared platform contract, delivering every part of: negotiated ports, bounded streams and disconnect cleanup. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-012.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio console/ports.
- [ ] **T-VM-012.05 — Qualify and retain this target's own result**
  - Action: Bind virtio console/ports to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-010"></a>
## T-FSP-010 — devpts/PTY filesystem

**Original requirement:** session ownership and peer cleanup

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 286.

### Execution steps

- [ ] **T-FSP-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve devpts/PTY filesystem to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-010.02 — Specify the complete target boundary**
  - Action: devpts/PTY filesystem must supply: session ownership and peer cleanup. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse devpts/PTY filesystem through the shared platform contract, delivering every part of: session ownership and peer cleanup. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-010.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for devpts/PTY filesystem.
- [ ] **T-FSP-010.05 — Qualify and retain this target's own result**
  - Action: Bind devpts/PTY filesystem to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-012"></a>
## T-FSP-012 — 9P filesystem

**Original requirement:** remote identity/cache/disconnect semantics

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 288.

### Execution steps

- [ ] **T-FSP-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve 9P filesystem to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-012.02 — Specify the complete target boundary**
  - Action: 9P filesystem must supply: remote identity/cache/disconnect semantics. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse 9P filesystem through the shared platform contract, delivering every part of: remote identity/cache/disconnect semantics. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-012.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for 9P filesystem.
- [ ] **T-FSP-012.05 — Qualify and retain this target's own result**
  - Action: Bind 9P filesystem to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-013"></a>
## T-FSP-013 — FUSE-like userspace provider

**Original requirement:** bounded IPC, mount authority and peer death

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 289.

### Execution steps

- [ ] **T-FSP-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve FUSE-like userspace provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-013.02 — Specify the complete target boundary**
  - Action: FUSE-like userspace provider must supply: bounded IPC, mount authority and peer death. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse FUSE-like userspace provider through the shared platform contract, delivering every part of: bounded IPC, mount authority and peer death. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-013.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for FUSE-like userspace provider.
- [ ] **T-FSP-013.05 — Qualify and retain this target's own result**
  - Action: Bind FUSE-like userspace provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-015"></a>
## T-FSP-015 — Mollen MFS compatibility/import

**Original requirement:** isolated parser, no native trust promotion

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 291.

### Execution steps

- [ ] **T-FSP-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Mollen MFS compatibility/import to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-015.02 — Specify the complete target boundary**
  - Action: Mollen MFS compatibility/import must supply: isolated parser, no native trust promotion. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse Mollen MFS compatibility/import through the shared platform contract, delivering every part of: isolated parser, no native trust promotion. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-015.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Mollen MFS compatibility/import.
- [ ] **T-FSP-015.05 — Qualify and retain this target's own result**
  - Action: Bind Mollen MFS compatibility/import to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-016"></a>
## T-FSP-016 — ValiFS/VaFS immutable image

**Original requirement:** checked compressed image, signatures and process isolation separate

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 292.

### Execution steps

- [ ] **T-FSP-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ValiFS/VaFS immutable image to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-016.02 — Specify the complete target boundary**
  - Action: ValiFS/VaFS immutable image must supply: checked compressed image, signatures and process isolation separate. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse ValiFS/VaFS immutable image through the shared platform contract, delivering every part of: checked compressed image, signatures and process isolation separate. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-016.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ValiFS/VaFS immutable image.
- [ ] **T-FSP-016.05 — Qualify and retain this target's own result**
  - Action: Bind ValiFS/VaFS immutable image to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-018"></a>
## T-FSP-018 — NTFS read-only importer

**Original requirement:** explicit supported features, isolated parser and no writes

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 294.

### Execution steps

- [ ] **T-FSP-018.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve NTFS read-only importer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-018.02 — Specify the complete target boundary**
  - Action: NTFS read-only importer must supply: explicit supported features, isolated parser and no writes. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-018.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-018.03 — Implement the exact target behavior**
  - Action: Implement or reuse NTFS read-only importer through the shared platform contract, delivering every part of: explicit supported features, isolated parser and no writes. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-018.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-018.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-018.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for NTFS read-only importer.
- [ ] **T-FSP-018.05 — Qualify and retain this target's own result**
  - Action: Bind NTFS read-only importer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-018.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-019"></a>
## T-FSP-019 — eventfs

**Original requirement:** bounded event objects, poll semantics and peer cleanup

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 295.

### Execution steps

- [ ] **T-FSP-019.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve eventfs to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-019.02 — Specify the complete target boundary**
  - Action: eventfs must supply: bounded event objects, poll semantics and peer cleanup. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-019.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-019.03 — Implement the exact target behavior**
  - Action: Implement or reuse eventfs through the shared platform contract, delivering every part of: bounded event objects, poll semantics and peer cleanup. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-019.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-019.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-019.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for eventfs.
- [ ] **T-FSP-019.05 — Qualify and retain this target's own result**
  - Action: Bind eventfs to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-019.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-020"></a>
## T-FSP-020 — pipe/FIFO filesystem

**Original requirement:** ownership, capacity, atomic writes and peer death

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 296.

### Execution steps

- [ ] **T-FSP-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve pipe/FIFO filesystem to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-020.02 — Specify the complete target boundary**
  - Action: pipe/FIFO filesystem must supply: ownership, capacity, atomic writes and peer death. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse pipe/FIFO filesystem through the shared platform contract, delivering every part of: ownership, capacity, atomic writes and peer death. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-020.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for pipe/FIFO filesystem.
- [ ] **T-FSP-020.05 — Qualify and retain this target's own result**
  - Action: Bind pipe/FIFO filesystem to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-021"></a>
## T-FSP-021 — socket filesystem

**Original requirement:** namespace, permissions, lifecycle and stale endpoint cleanup

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 297.

### Execution steps

- [ ] **T-FSP-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve socket filesystem to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-021.02 — Specify the complete target boundary**
  - Action: socket filesystem must supply: namespace, permissions, lifecycle and stale endpoint cleanup. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse socket filesystem through the shared platform contract, delivering every part of: namespace, permissions, lifecycle and stale endpoint cleanup. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-021.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for socket filesystem.
- [ ] **T-FSP-021.05 — Qualify and retain this target's own result**
  - Action: Bind socket filesystem to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-022"></a>
## T-FSP-022 — legacy custom-FS import adapters

**Original requirement:** MRAFS, SAF, SSFS and `abc` parsed in quarantine with explicit format identity

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 298.

### Execution steps

- [ ] **T-FSP-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve legacy custom-FS import adapters to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-022.02 — Specify the complete target boundary**
  - Action: legacy custom-FS import adapters must supply: MRAFS, SAF, SSFS and `abc` parsed in quarantine with explicit format identity. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse legacy custom-FS import adapters through the shared platform contract, delivering every part of: MRAFS, SAF, SSFS and `abc` parsed in quarantine with explicit format identity. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-022.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for legacy custom-FS import adapters.
- [ ] **T-FSP-022.05 — Qualify and retain this target's own result**
  - Action: Bind legacy custom-FS import adapters to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-060"></a>
## T-SVC-060 — SSH/Remote Shell Service

**Original requirement:** authenticated PTY, keys, limits and audit

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 80.

### Execution steps

- [ ] **T-SVC-060.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve SSH/Remote Shell Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-060.02 — Specify the complete target boundary**
  - Action: SSH/Remote Shell Service must supply: authenticated PTY, keys, limits and audit. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-060.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-060.03 — Implement the exact target behavior**
  - Action: Implement or reuse SSH/Remote Shell Service through the shared platform contract, delivering every part of: authenticated PTY, keys, limits and audit. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-060.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-060.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-SVC-060.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for SSH/Remote Shell Service.
- [ ] **T-SVC-060.05 — Qualify and retain this target's own result**
  - Action: Bind SSH/Remote Shell Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-060.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-120"></a>
## T-SVC-120 — PTY/Terminal Service

**Original requirement:** sessions, job control, resize, signals and peer cleanup

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 147.

### Execution steps

- [ ] **T-SVC-120.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PTY/Terminal Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-120.02 — Specify the complete target boundary**
  - Action: PTY/Terminal Service must supply: sessions, job control, resize, signals and peer cleanup. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-120.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-120.03 — Implement the exact target behavior**
  - Action: Implement or reuse PTY/Terminal Service through the shared platform contract, delivering every part of: sessions, job control, resize, signals and peer cleanup. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-120.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-120.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-SVC-120.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PTY/Terminal Service.
- [ ] **T-SVC-120.05 — Qualify and retain this target's own result**
  - Action: Bind PTY/Terminal Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-120.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-121"></a>
## T-SVC-121 — Shell/Command Service

**Original requirement:** parsing, pipelines, redirection, jobs and scripts

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 148.

### Execution steps

- [ ] **T-SVC-121.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Shell/Command Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-121.02 — Specify the complete target boundary**
  - Action: Shell/Command Service must supply: parsing, pipelines, redirection, jobs and scripts. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-121.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-121.03 — Implement the exact target behavior**
  - Action: Implement or reuse Shell/Command Service through the shared platform contract, delivering every part of: parsing, pipelines, redirection, jobs and scripts. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-121.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-121.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-SVC-121.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Shell/Command Service.
- [ ] **T-SVC-121.05 — Qualify and retain this target's own result**
  - Action: Bind Shell/Command Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-121.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-122"></a>
## T-SVC-122 — Debug Service

**Original requirement:** process-scoped attach, symbols, breakpoints and audit

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 149.

### Execution steps

- [ ] **T-SVC-122.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Debug Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-122.02 — Specify the complete target boundary**
  - Action: Debug Service must supply: process-scoped attach, symbols, breakpoints and audit. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-122.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-122.03 — Implement the exact target behavior**
  - Action: Implement or reuse Debug Service through the shared platform contract, delivering every part of: process-scoped attach, symbols, breakpoints and audit. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-122.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-122.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-SVC-122.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Debug Service.
- [ ] **T-SVC-122.05 — Qualify and retain this target's own result**
  - Action: Bind Debug Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-122.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-123"></a>
## T-SVC-123 — Profiler/Trace Service

**Original requirement:** bounded sampling/events, symbols and export

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 150.

### Execution steps

- [ ] **T-SVC-123.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Profiler/Trace Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-123.02 — Specify the complete target boundary**
  - Action: Profiler/Trace Service must supply: bounded sampling/events, symbols and export. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-123.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-123.03 — Implement the exact target behavior**
  - Action: Implement or reuse Profiler/Trace Service through the shared platform contract, delivering every part of: bounded sampling/events, symbols and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-123.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-123.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-SVC-123.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Profiler/Trace Service.
- [ ] **T-SVC-123.05 — Qualify and retain this target's own result**
  - Action: Bind Profiler/Trace Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-123.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-125"></a>
## T-SVC-125 — Package Build/Port Service

**Original requirement:** locked recipes, patches, staging and runtime gates

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 152.

### Execution steps

- [ ] **T-SVC-125.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Package Build/Port Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-125.02 — Specify the complete target boundary**
  - Action: Package Build/Port Service must supply: locked recipes, patches, staging and runtime gates. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-125.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-125.03 — Implement the exact target behavior**
  - Action: Implement or reuse Package Build/Port Service through the shared platform contract, delivering every part of: locked recipes, patches, staging and runtime gates. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-125.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-125.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-SVC-125.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Package Build/Port Service.
- [ ] **T-SVC-125.05 — Qualify and retain this target's own result**
  - Action: Bind Package Build/Port Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-125.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-126"></a>
## T-SVC-126 — POSIX/Linux Compatibility Service

**Original requirement:** program-driven ABI and isolation

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 153.

### Execution steps

- [ ] **T-SVC-126.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve POSIX/Linux Compatibility Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-126.02 — Specify the complete target boundary**
  - Action: POSIX/Linux Compatibility Service must supply: program-driven ABI and isolation. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-126.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-126.03 — Implement the exact target behavior**
  - Action: Implement or reuse POSIX/Linux Compatibility Service through the shared platform contract, delivering every part of: program-driven ABI and isolation. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-126.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-126.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-SVC-126.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for POSIX/Linux Compatibility Service.
- [ ] **T-SVC-126.05 — Qualify and retain this target's own result**
  - Action: Bind POSIX/Linux Compatibility Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-126.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-127"></a>
## T-SVC-127 — Dynamic Linker/Loader Service

**Original requirement:** checked ELF/shared objects/relocations and cache ownership

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 154.

### Execution steps

- [ ] **T-SVC-127.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Dynamic Linker/Loader Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-127.02 — Specify the complete target boundary**
  - Action: Dynamic Linker/Loader Service must supply: checked ELF/shared objects/relocations and cache ownership. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-127.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-127.03 — Implement the exact target behavior**
  - Action: Implement or reuse Dynamic Linker/Loader Service through the shared platform contract, delivering every part of: checked ELF/shared objects/relocations and cache ownership. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-127.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-127.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-SVC-127.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Dynamic Linker/Loader Service.
- [ ] **T-SVC-127.05 — Qualify and retain this target's own result**
  - Action: Bind Dynamic Linker/Loader Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-127.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-128"></a>
## T-SVC-128 — Virtual Machine Manager

**Original requirement:** VM lifecycle, devices, images, snapshots and containment

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 155.

### Execution steps

- [ ] **T-SVC-128.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Virtual Machine Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-128.02 — Specify the complete target boundary**
  - Action: Virtual Machine Manager must supply: VM lifecycle, devices, images, snapshots and containment. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-128.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-128.03 — Implement the exact target behavior**
  - Action: Implement or reuse Virtual Machine Manager through the shared platform contract, delivering every part of: VM lifecycle, devices, images, snapshots and containment. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-128.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-128.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-SVC-128.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Virtual Machine Manager.
- [ ] **T-SVC-128.05 — Qualify and retain this target's own result**
  - Action: Bind Virtual Machine Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-128.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-129"></a>
## T-SVC-129 — Container/Sandbox Manager

**Original requirement:** namespaces, capabilities, resources and teardown

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 156.

### Execution steps

- [ ] **T-SVC-129.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Container/Sandbox Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-129.02 — Specify the complete target boundary**
  - Action: Container/Sandbox Manager must supply: namespaces, capabilities, resources and teardown. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-129.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-129.03 — Implement the exact target behavior**
  - Action: Implement or reuse Container/Sandbox Manager through the shared platform contract, delivering every part of: namespaces, capabilities, resources and teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-129.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-129.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-SVC-129.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Container/Sandbox Manager.
- [ ] **T-SVC-129.05 — Qualify and retain this target's own result**
  - Action: Bind Container/Sandbox Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-129.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-071"></a>
## T-APP-071 — SSH Client

**Original requirement:** host-key verification, credentials, PTY and sessions

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 203.

### Execution steps

- [ ] **T-APP-071.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve SSH Client to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-071.02 — Specify the complete target boundary**
  - Action: SSH Client must supply: host-key verification, credentials, PTY and sessions. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-071.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-071.03 — Implement the exact target behavior**
  - Action: Implement or reuse SSH Client through the shared platform contract, delivering every part of: host-key verification, credentials, PTY and sessions. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-071.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-071.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-APP-071.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for SSH Client.
- [ ] **T-APP-071.05 — Qualify and retain this target's own result**
  - Action: Bind SSH Client to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-071.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-080"></a>
## T-APP-080 — Source Editor/IDE

**Original requirement:** projects, language services, build/test/debug and recovery

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 209.

### Execution steps

- [ ] **T-APP-080.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Source Editor/IDE to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-080.02 — Specify the complete target boundary**
  - Action: Source Editor/IDE must supply: projects, language services, build/test/debug and recovery. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-080.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-080.03 — Implement the exact target behavior**
  - Action: Implement or reuse Source Editor/IDE through the shared platform contract, delivering every part of: projects, language services, build/test/debug and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-080.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-080.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-APP-080.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Source Editor/IDE.
- [ ] **T-APP-080.05 — Qualify and retain this target's own result**
  - Action: Bind Source Editor/IDE to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-080.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-082"></a>
## T-APP-082 — Debugger

**Original requirement:** scoped attach, breakpoints, memory/registers and symbols

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 211.

### Execution steps

- [ ] **T-APP-082.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Debugger to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-082.02 — Specify the complete target boundary**
  - Action: Debugger must supply: scoped attach, breakpoints, memory/registers and symbols. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-082.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-082.03 — Implement the exact target behavior**
  - Action: Implement or reuse Debugger through the shared platform contract, delivering every part of: scoped attach, breakpoints, memory/registers and symbols. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-082.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-082.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-APP-082.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Debugger.
- [ ] **T-APP-082.05 — Qualify and retain this target's own result**
  - Action: Bind Debugger to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-082.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-083"></a>
## T-APP-083 — Profiler

**Original requirement:** CPU/memory/I/O/frame traces, comparisons and export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 212.

### Execution steps

- [ ] **T-APP-083.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Profiler to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-083.02 — Specify the complete target boundary**
  - Action: Profiler must supply: CPU/memory/I/O/frame traces, comparisons and export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-083.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-083.03 — Implement the exact target behavior**
  - Action: Implement or reuse Profiler through the shared platform contract, delivering every part of: CPU/memory/I/O/frame traces, comparisons and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-083.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-083.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-APP-083.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Profiler.
- [ ] **T-APP-083.05 — Qualify and retain this target's own result**
  - Action: Bind Profiler to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-083.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-084"></a>
## T-APP-084 — Log/Trace Viewer

**Original requirement:** queries, correlations, redaction and saved views

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 213.

### Execution steps

- [ ] **T-APP-084.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Log/Trace Viewer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-084.02 — Specify the complete target boundary**
  - Action: Log/Trace Viewer must supply: queries, correlations, redaction and saved views. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-084.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-084.03 — Implement the exact target behavior**
  - Action: Implement or reuse Log/Trace Viewer through the shared platform contract, delivering every part of: queries, correlations, redaction and saved views. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-084.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-084.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-APP-084.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Log/Trace Viewer.
- [ ] **T-APP-084.05 — Qualify and retain this target's own result**
  - Action: Bind Log/Trace Viewer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-084.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-085"></a>
## T-APP-085 — Package/Port Development

**Original requirement:** recipe, patch, build, test, stage and provenance

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 214.

### Execution steps

- [ ] **T-APP-085.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Package/Port Development to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-085.02 — Specify the complete target boundary**
  - Action: Package/Port Development must supply: recipe, patch, build, test, stage and provenance. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-085.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-085.03 — Implement the exact target behavior**
  - Action: Implement or reuse Package/Port Development through the shared platform contract, delivering every part of: recipe, patch, build, test, stage and provenance. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-085.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-085.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-APP-085.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Package/Port Development.
- [ ] **T-APP-085.05 — Qualify and retain this target's own result**
  - Action: Bind Package/Port Development to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-085.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-087"></a>
## T-APP-087 — Disk/Filesystem Inspector

**Original requirement:** volumes, blocks, fs structures and safe repair staging

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 216.

### Execution steps

- [ ] **T-APP-087.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Disk/Filesystem Inspector to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-087.02 — Specify the complete target boundary**
  - Action: Disk/Filesystem Inspector must supply: volumes, blocks, fs structures and safe repair staging. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-087.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-087.03 — Implement the exact target behavior**
  - Action: Implement or reuse Disk/Filesystem Inspector through the shared platform contract, delivering every part of: volumes, blocks, fs structures and safe repair staging. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-087.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-087.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-APP-087.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Disk/Filesystem Inspector.
- [ ] **T-APP-087.05 — Qualify and retain this target's own result**
  - Action: Bind Disk/Filesystem Inspector to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-087.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-088"></a>
## T-APP-088 — Network Packet Inspector

**Original requirement:** scoped capture, decode, filters and export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 217.

### Execution steps

- [ ] **T-APP-088.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Network Packet Inspector to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-088.02 — Specify the complete target boundary**
  - Action: Network Packet Inspector must supply: scoped capture, decode, filters and export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-088.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-088.03 — Implement the exact target behavior**
  - Action: Implement or reuse Network Packet Inspector through the shared platform contract, delivering every part of: scoped capture, decode, filters and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-088.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-088.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-APP-088.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Network Packet Inspector.
- [ ] **T-APP-088.05 — Qualify and retain this target's own result**
  - Action: Bind Network Packet Inspector to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-088.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-089"></a>
## T-APP-089 — API/Protocol Workbench

**Original requirement:** typed request/response, schemas, timing and hostile cases

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 218.

### Execution steps

- [ ] **T-APP-089.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve API/Protocol Workbench to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-089.02 — Specify the complete target boundary**
  - Action: API/Protocol Workbench must supply: typed request/response, schemas, timing and hostile cases. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-089.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-089.03 — Implement the exact target behavior**
  - Action: Implement or reuse API/Protocol Workbench through the shared platform contract, delivering every part of: typed request/response, schemas, timing and hostile cases. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-089.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-089.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-APP-089.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for API/Protocol Workbench.
- [ ] **T-APP-089.05 — Qualify and retain this target's own result**
  - Action: Bind API/Protocol Workbench to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-089.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-001"></a>
## T-DEV-001 — PTY and terminal ABI

**Original requirement:** sessions, resize, signals, Unicode, peer death

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 78.

### Execution steps

- [ ] **T-DEV-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PTY and terminal ABI to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-001.02 — Specify the complete target boundary**
  - Action: PTY and terminal ABI must supply: sessions, resize, signals, Unicode, peer death. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse PTY and terminal ABI through the shared platform contract, delivering every part of: sessions, resize, signals, Unicode, peer death. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-001.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PTY and terminal ABI.
- [ ] **T-DEV-001.05 — Qualify and retain this target's own result**
  - Action: Bind PTY and terminal ABI to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-002"></a>
## T-DEV-002 — interactive shell

**Original requirement:** quoting, expansion, pipelines, redirection, jobs and scripts

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 79.

### Execution steps

- [ ] **T-DEV-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve interactive shell to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-002.02 — Specify the complete target boundary**
  - Action: interactive shell must supply: quoting, expansion, pipelines, redirection, jobs and scripts. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse interactive shell through the shared platform contract, delivering every part of: quoting, expansion, pipelines, redirection, jobs and scripts. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-002.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for interactive shell.
- [ ] **T-DEV-002.05 — Qualify and retain this target's own result**
  - Action: Bind interactive shell to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-003"></a>
## T-DEV-003 — core utilities

**Original requirement:** file/text/process/system/network tools with consistent errors

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 80.

### Execution steps

- [ ] **T-DEV-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve core utilities to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-003.02 — Specify the complete target boundary**
  - Action: core utilities must supply: file/text/process/system/network tools with consistent errors. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse core utilities through the shared platform contract, delivering every part of: file/text/process/system/network tools with consistent errors. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-003.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for core utilities.
- [ ] **T-DEV-003.05 — Qualify and retain this target's own result**
  - Action: Bind core utilities to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-004"></a>
## T-DEV-004 — source editor/IDE

**Original requirement:** project, language, build/test/debug and recovery workflow

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 81.

### Execution steps

- [ ] **T-DEV-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve source editor/IDE to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-004.02 — Specify the complete target boundary**
  - Action: source editor/IDE must supply: project, language, build/test/debug and recovery workflow. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse source editor/IDE through the shared platform contract, delivering every part of: project, language, build/test/debug and recovery workflow. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-004.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for source editor/IDE.
- [ ] **T-DEV-004.05 — Qualify and retain this target's own result**
  - Action: Bind source editor/IDE to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-005"></a>
## T-DEV-005 — debugger

**Original requirement:** scoped attach, symbols, stepping, break/watch and dumps

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 82.

### Execution steps

- [ ] **T-DEV-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve debugger to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-005.02 — Specify the complete target boundary**
  - Action: debugger must supply: scoped attach, symbols, stepping, break/watch and dumps. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse debugger through the shared platform contract, delivering every part of: scoped attach, symbols, stepping, break/watch and dumps. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-005.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for debugger.
- [ ] **T-DEV-005.05 — Qualify and retain this target's own result**
  - Action: Bind debugger to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-006"></a>
## T-DEV-006 — profiler/tracer

**Original requirement:** CPU/memory/I/O/frame/event correlation and export

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 83.

### Execution steps

- [ ] **T-DEV-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve profiler/tracer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-006.02 — Specify the complete target boundary**
  - Action: profiler/tracer must supply: CPU/memory/I/O/frame/event correlation and export. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse profiler/tracer through the shared platform contract, delivering every part of: CPU/memory/I/O/frame/event correlation and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-006.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for profiler/tracer.
- [ ] **T-DEV-006.05 — Qualify and retain this target's own result**
  - Action: Bind profiler/tracer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-007"></a>
## T-DEV-007 — test runner

**Original requirement:** registered/executed/skipped/failed parity and strict exit

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 84.

### Execution steps

- [ ] **T-DEV-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve test runner to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-007.02 — Specify the complete target boundary**
  - Action: test runner must supply: registered/executed/skipped/failed parity and strict exit. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse test runner through the shared platform contract, delivering every part of: registered/executed/skipped/failed parity and strict exit. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-007.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for test runner.
- [ ] **T-DEV-007.05 — Qualify and retain this target's own result**
  - Action: Bind test runner to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-008"></a>
## T-DEV-008 — package/port SDK

**Original requirement:** hermetic recipe, patch, build, stage, runtime and update

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 85.

### Execution steps

- [ ] **T-DEV-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve package/port SDK to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-008.02 — Specify the complete target boundary**
  - Action: package/port SDK must supply: hermetic recipe, patch, build, stage, runtime and update. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse package/port SDK through the shared platform contract, delivering every part of: hermetic recipe, patch, build, stage, runtime and update. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-008.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for package/port SDK.
- [ ] **T-DEV-008.05 — Qualify and retain this target's own result**
  - Action: Bind package/port SDK to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-009"></a>
## T-DEV-009 — POSIX surface ledger

**Original requirement:** program-driven syscalls/libc/shell behavior and gaps

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 86.

### Execution steps

- [ ] **T-DEV-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve POSIX surface ledger to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-009.02 — Specify the complete target boundary**
  - Action: POSIX surface ledger must supply: program-driven syscalls/libc/shell behavior and gaps. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse POSIX surface ledger through the shared platform contract, delivering every part of: program-driven syscalls/libc/shell behavior and gaps. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-009.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for POSIX surface ledger.
- [ ] **T-DEV-009.05 — Qualify and retain this target's own result**
  - Action: Bind POSIX surface ledger to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-010"></a>
## T-DEV-010 — dynamic linker

**Original requirement:** bounded ELF imports/relocations/search/cache/unload

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 87.

### Execution steps

- [ ] **T-DEV-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve dynamic linker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-010.02 — Specify the complete target boundary**
  - Action: dynamic linker must supply: bounded ELF imports/relocations/search/cache/unload. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse dynamic linker through the shared platform contract, delivering every part of: bounded ELF imports/relocations/search/cache/unload. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-010.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for dynamic linker.
- [ ] **T-DEV-010.05 — Qualify and retain this target's own result**
  - Action: Bind dynamic linker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-011"></a>
## T-DEV-011 — shared libraries

**Original requirement:** ABI/version/ownership/update and dependency receipts

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 88.

### Execution steps

- [ ] **T-DEV-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve shared libraries to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-011.02 — Specify the complete target boundary**
  - Action: shared libraries must supply: ABI/version/ownership/update and dependency receipts. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse shared libraries through the shared platform contract, delivering every part of: ABI/version/ownership/update and dependency receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-011.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for shared libraries.
- [ ] **T-DEV-011.05 — Qualify and retain this target's own result**
  - Action: Bind shared libraries to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-012"></a>
## T-DEV-012 — Linux ABI compatibility

**Original requirement:** isolated selected syscall/program workload, not blanket claim

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 89.

### Execution steps

- [ ] **T-DEV-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Linux ABI compatibility to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-012.02 — Specify the complete target boundary**
  - Action: Linux ABI compatibility must supply: isolated selected syscall/program workload, not blanket claim. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse Linux ABI compatibility through the shared platform contract, delivering every part of: isolated selected syscall/program workload, not blanket claim. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-012.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Linux ABI compatibility.
- [ ] **T-DEV-012.05 — Qualify and retain this target's own result**
  - Action: Bind Linux ABI compatibility to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-013"></a>
## T-DEV-013 — PE/Windows import quarantine

**Original requirement:** length-first loader/cache for explicit compatibility apps only

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 90.

### Execution steps

- [ ] **T-DEV-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PE/Windows import quarantine to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-013.02 — Specify the complete target boundary**
  - Action: PE/Windows import quarantine must supply: length-first loader/cache for explicit compatibility apps only. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse PE/Windows import quarantine through the shared platform contract, delivering every part of: length-first loader/cache for explicit compatibility apps only. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-013.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PE/Windows import quarantine.
- [ ] **T-DEV-013.05 — Qualify and retain this target's own result**
  - Action: Bind PE/Windows import quarantine to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-014"></a>
## T-DEV-014 — scripting runtimes

**Original requirement:** sandboxed selected interpreters with package boundaries

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 91.

### Execution steps

- [ ] **T-DEV-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve scripting runtimes to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-014.02 — Specify the complete target boundary**
  - Action: scripting runtimes must supply: sandboxed selected interpreters with package boundaries. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse scripting runtimes through the shared platform contract, delivering every part of: sandboxed selected interpreters with package boundaries. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-014.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for scripting runtimes.
- [ ] **T-DEV-014.05 — Qualify and retain this target's own result**
  - Action: Bind scripting runtimes to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-015"></a>
## T-DEV-015 — SDL/media compatibility

**Original requirement:** input/audio/display/files through app capabilities

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 92.

### Execution steps

- [ ] **T-DEV-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve SDL/media compatibility to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-015.02 — Specify the complete target boundary**
  - Action: SDL/media compatibility must supply: input/audio/display/files through app capabilities. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse SDL/media compatibility through the shared platform contract, delivering every part of: input/audio/display/files through app capabilities. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-015.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for SDL/media compatibility.
- [ ] **T-DEV-015.05 — Qualify and retain this target's own result**
  - Action: Bind SDL/media compatibility to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-016"></a>
## T-DEV-016 — X11/Wayland bridge experiments

**Original requirement:** explicit legacy compatibility, no compositor authority bypass

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 93.

### Execution steps

- [ ] **T-DEV-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve X11/Wayland bridge experiments to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-016.02 — Specify the complete target boundary**
  - Action: X11/Wayland bridge experiments must supply: explicit legacy compatibility, no compositor authority bypass. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse X11/Wayland bridge experiments through the shared platform contract, delivering every part of: explicit legacy compatibility, no compositor authority bypass. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-016.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for X11/Wayland bridge experiments.
- [ ] **T-DEV-016.05 — Qualify and retain this target's own result**
  - Action: Bind X11/Wayland bridge experiments to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-017"></a>
## T-DEV-017 — virtual machine manager

**Original requirement:** machines, images, devices, snapshots and containment

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 94.

### Execution steps

- [ ] **T-DEV-017.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtual machine manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-017.02 — Specify the complete target boundary**
  - Action: virtual machine manager must supply: machines, images, devices, snapshots and containment. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-017.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-017.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtual machine manager through the shared platform contract, delivering every part of: machines, images, devices, snapshots and containment. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-017.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-017.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-017.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtual machine manager.
- [ ] **T-DEV-017.05 — Qualify and retain this target's own result**
  - Action: Bind virtual machine manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-017.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-018"></a>
## T-DEV-018 — emulator framework

**Original requirement:** CPU/device emulation with deterministic fixtures and limits

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 95.

### Execution steps

- [ ] **T-DEV-018.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve emulator framework to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-018.02 — Specify the complete target boundary**
  - Action: emulator framework must supply: CPU/device emulation with deterministic fixtures and limits. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-018.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-018.03 — Implement the exact target behavior**
  - Action: Implement or reuse emulator framework through the shared platform contract, delivering every part of: CPU/device emulation with deterministic fixtures and limits. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-018.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-018.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-018.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for emulator framework.
- [ ] **T-DEV-018.05 — Qualify and retain this target's own result**
  - Action: Bind emulator framework to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-018.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-019"></a>
## T-DEV-019 — sandbox/container profiles

**Original requirement:** namespaces, handles, resources, network and teardown

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 96.

### Execution steps

- [ ] **T-DEV-019.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve sandbox/container profiles to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-019.02 — Specify the complete target boundary**
  - Action: sandbox/container profiles must supply: namespaces, handles, resources, network and teardown. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-019.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-019.03 — Implement the exact target behavior**
  - Action: Implement or reuse sandbox/container profiles through the shared platform contract, delivering every part of: namespaces, handles, resources, network and teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-019.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-019.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-019.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for sandbox/container profiles.
- [ ] **T-DEV-019.05 — Qualify and retain this target's own result**
  - Action: Bind sandbox/container profiles to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-019.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-dev-020"></a>
## T-DEV-020 — guest integration

**Original requirement:** typed shared folder/clipboard/input/display authority

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 97.

### Execution steps

- [ ] **T-DEV-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve guest integration to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-20, H-15.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-DEV-020.02 — Specify the complete target boundary**
  - Action: guest integration must supply: typed shared folder/clipboard/input/display authority. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-DEV-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-DEV-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse guest integration through the shared platform contract, delivering every part of: typed shared folder/clipboard/input/display authority. Do not fork a duplicate subsystem for this row.
  - Requires: T-DEV-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-DEV-020.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: partial/invalid stdin; PTY EOF/hangup; foreground/background job signals; failed child; compiler diagnostic mismatch; ungranted debug; cancelled build; stale source/debug identity.
  - Requires: T-DEV-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for guest integration.
- [ ] **T-DEV-020.05 — Qualify and retain this target's own result**
  - Action: Bind guest integration to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-DEV-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
