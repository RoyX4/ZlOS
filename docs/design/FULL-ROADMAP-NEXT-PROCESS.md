# First process-management implementation sequence

Status: implementation started, with host evidence and fresh native-UEFI parent/child QEMU proof; full closure pending. This bounded slice is inside `M-03.03`; it does not declare the full process subsystem complete.

[Current progress and proof boundaries](../evidence/process-spawn-wait-2026-09-08.md) · [Implemented ABI](userspace-spawn-wait-abi.md).
Checkmarks below identify bounded target steps passed again by combined image
`6ae68572…`, after the allocator and command-route repairs and reconciliation
with main `9c4cb509`.
The progress receipt records the exact host/QEMU split; allocation and release
refusal injection remain host evidence. The rebuilt host suite, native boot,
all four parent/child scenarios and external fault/exit/sleep probes pass.
Native/BIOS32 USB re-plug and BIOS32 double-fault capture also pass locally.
Earlier BIOS32 Run and 47-app lifecycle checks belong to `52dc8b9c…`. The full
combined-source boot/app matrix and hosted closure remain open at
`N-PROCESS.30`; passing the bounded local set does not close those larger gates.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Original audited contract](userspace-process-management-next.md)

**Entry:** the selected baseline and memory/entry-state comparison are accepted; `D-05`, `D-06` and `D-07` are resolved for this bounded slice. Preserve the existing two-slot/raw-image limit until a separately proved extension replaces it.

## N-PROCESS.01

- [ ] Capture the current implementation and receipt identities; reuse passing evidence where its source/input scope is unchanged.

Requires: `M-03.02`, `D-05`, `D-06`, `D-07`.

## N-PROCESS.02

- [ ] Read process_lifecycle, process_memory, scheduler_policy, user_process_service and the architecture entry/loader together; record who currently owns each state field.

Requires: `N-PROCESS.01`.

## N-PROCESS.03

- [ ] Write the spawn/wait ABI layouts and error/result enums, including eight-byte handle copyout and a versioned fixed-size wait result.

Requires: `N-PROCESS.02`.

## N-PROCESS.04

- [ ] Derive the parent identity from the active process; define the separate privileged kernel caller rather than accepting a user-supplied parent token.

Requires: `N-PROCESS.03`.

## N-PROCESS.05

- [ ] Capture a supervisor-only kernel address-space template at a point where no caller's user mappings can contaminate it.

Requires: `N-PROCESS.04`.

## N-PROCESS.06

- [ ] Separate image preparation from active process selection; preparation must not overwrite proc64_index, proc64, active CR3, TSS stack or saved return context.

Requires: `N-PROCESS.05`.

## N-PROCESS.07

- [ ] Represent all candidate frame, mapping, identity and scheduler ownership until publication or rollback.

Requires: `N-PROCESS.06`.

## N-PROCESS.08

- [ ] Prepare a child with only its own user mappings and the allowed protected kernel mappings; reject user-accessible kernel/device mappings.

Requires: `N-PROCESS.07`.

## N-PROCESS.09

- [ ] Keep the existing flat zlfs name and 1..4096-byte raw executable format for the first bounded slice; reject invalid names, lengths and unavailable images explicitly.

Requires: `N-PROCESS.08`.

## N-PROCESS.10

- [ ] Validate the complete writable output range before acquiring resources; preserve all eight handle bytes and leave the buffer unchanged on failure.

Requires: `N-PROCESS.09`.

## N-PROCESS.11

- [ ] Build a failing host test for every allocation and admission boundary, including the last possible failure before publication.

Requires: `N-PROCESS.10`.

## N-PROCESS.12

- [ ] Prove that successful and failed child preparation preserve the parent's address-space root, register/FP state, TSS stack and return path.

Requires: `N-PROCESS.11`.

## N-PROCESS.13

- [ ] Reserve the lifecycle identity and scheduler slot without replacing any live entry; return an explicit error when the two fixed slots are full.

Requires: `N-PROCESS.12`.

## N-PROCESS.14

- [ ] Choose and document the single publication point and how a copyout failure before/after it is handled; do not lose a live child or return an unowned handle.

Requires: `N-PROCESS.13`.

## N-PROCESS.15

- [ ] Implement the bounded spawn path using the existing lifecycle, memory, scheduler and file reader; do not add a second process registry.

Requires: `N-PROCESS.14`.

## N-PROCESS.16

- [ ] Add the syscall through the generated ABI owner and preserve unknown/negative/high-bit syscall rejection.

Requires: `N-PROCESS.15`.

## N-PROCESS.17

- [ ] Test handle generations 0x80000000 and 0xffffffff; neither may be confused with a signed error return.

Requires: `N-PROCESS.16`.

## N-PROCESS.18

- [ ] Define wait outcomes for a live child, normal exit and fault, including exact signed status and fault vector/error/address.

Requires: `N-PROCESS.17`.

## N-PROCESS.19

- [ ] Require the exact current parent's child handle; refuse a foreign child or stale generation without altering either process.

Requires: `N-PROCESS.18`.

## N-PROCESS.20

- [ ] Validate the complete wait-output range before consuming terminal custody; preserve the result if copyout or cleanup cannot finish.

Requires: `N-PROCESS.19`.

## N-PROCESS.21

- [ ] Implement wait/reap cleanup ordering so frames, mappings, scheduler membership and lifecycle identity return to their expected baselines.

Requires: `N-PROCESS.20`.

## N-PROCESS.22

- [ ] Define terminal-parent adoption as a privileged, preflighted transfer of direct children into kernel custody; preserve their handles and terminal records.

Requires: `N-PROCESS.21`.

## N-PROCESS.23

- [ ] Test parent death both before and after child termination, including a runnable adopted child and a terminal child awaiting administrative reap.

Requires: `N-PROCESS.22`.

## N-PROCESS.24

- [ ] Test slot reuse and stale handles after every exit/fault/reap path; a previous generation must never control its replacement.

Requires: `N-PROCESS.23`.

## N-PROCESS.25

- [ ] Create a disk-loaded parent and child fixture through the existing shipping filesystem tooling; bind both exact byte sequences to the scenario receipt.

Requires: `N-PROCESS.24`.

## N-PROCESS.26

- [x] Boot the current native-UEFI image in QEMU; parent starts child, survives its exit, obtains the exact status and reaps it.

Requires: `N-PROCESS.25`.

## N-PROCESS.27

- [x] Run a child that faults and attempts access to the parent's user page; parent must continue independently and report the fault accurately.

Requires: `N-PROCESS.26`.

## N-PROCESS.28

- [x] Retry wait after an invalid output pointer without losing the only termination record; then prove successful retry and final cleanup.

Requires: `N-PROCESS.27`.

## N-PROCESS.29

- [x] Exercise exhaustion, repeated spawn/wait cycles, parent death and failure injection while checking allocator/scheduler/lifecycle counts.

Requires: `N-PROCESS.28`.

## N-PROCESS.30

- [ ] Rerun affected host tests and existing external fault, normal-exit, sleep, boot and desktop regressions; use hosted/contained gates rather than unrestricted laptop fan-out.

Requires: `N-PROCESS.29`.

## N-PROCESS.31

- [ ] Update the source ABI documentation, owning generated tables/inventories and exact source-bound scenario receipts; retain the two-slot/raw-image/SMP/physical limits.

Requires: `N-PROCESS.30`.

## N-PROCESS.32

- [ ] Record an integration-ready review of the intended files and exact bounded evidence; publish only when authorized. Retain the two-slot/raw-image/SMP/physical limits and leave wider feature completion open.

Requires: `N-PROCESS.31`.
