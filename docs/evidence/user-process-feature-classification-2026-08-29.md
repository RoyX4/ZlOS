# Current user-process feature classification

Date: 2026-09-03

This receipt maps the bounded native UEFI64 Ring 3 implementation into twelve
kernel feature rows. It does not promote the fixed two-slot persistent service
into a general userspace process API or a stable cross-version ABI.

## Current implementation

The x86-64 path provides two fixed process objects with separate PML4 roots,
user stacks, guarded two-page supervisor-only TSS kernel stacks, saved frames and eight-entry file
handle tables. User code is read/execute, the user stack is read/write/NX, its
lower guard PTE is absent, and inherited kernel/device mappings are
supervisor-only. `copy_from_user` and `copy_to_user` admit the complete fixed
code/stack span or fully committed anonymous span before touching the first byte.

Each private process slot also has an absent lower kernel-stack PTE followed by
two supervisor read/write, NX pages. The TSS selects that private virtual top
before Ring 3 entry. Return assembly moves to the permanent kernel stack before
restoring the kernel CR3, and the diagnostic retains bounded high-water use for
both fixed stack slots.

Each slot acquires eight physical frames through the typed PMM: four page-table
levels, code, user stack and two kernel-stack pages. Acquisition is all-or-
nothing, replacement allocates the complete successor first, and teardown
preflights all frame owners. The final UEFI marker requires both slots to be
released and the exact pre-process PMM baseline restored.

The fixed-frame owner for each slot has a 16-page replacement quota. The
anonymous owner has a 32-page quota, equal to its complete typed window. The
final marker requires both fixed and both anonymous live totals to be zero,
quota values to remain exact, high-water to remain bounded, refusal totals to
remain zero and the PMM metadata-to-owner sum invariant to pass.

Each slot also owns a typed 32-page anonymous window. Reservation creates no
mapping or frame. Commitment allocates and zeroes the whole range before one
page-table transaction publishes it; OOM and injected transaction failures
restore the exact baseline. Release unmaps before owner-checked PMM reclaim.
The invariant accepts hardware accessed/dirty bits and refuses every other
unexpected address or permission change.

The syscall boundary uses `int 0x80` and `iretq`. ABI version 1 generates the
admitted numbers 1 through 24 for console, identity/lifecycle, bounded copy,
time, yield, whole-file storage, PID IPC, text-window and anonymous-memory
operations. Dispatch
bodies and argument contracts remain hand-written.

An exact generation-tagged lifecycle handle now names each live slot. Exit and
fault records remain distinct until observation and reap; stale generations are
rejected and an exhausted generation retires its slot. The bounded scheduler
uses that handle as its owner token. Its coordinator performs one preemptible
Ring 3 turn per kernel work-loop call and fail-stops if lifecycle and policy
state disagree.

## Current evidence

The exact current subject is the `build_identity` field in generated
`docs/program/FEATURE-STATUS.json` and the user-process receipt; this page does
not duplicate that volatile value.

`user-process-native-uefi64-qemu-2026-08-29.json` binds the exact current
`zlOS-usb.img`, verifier, `usermode.c`, `process_memory.c`, `pmm.c`, their
headers, the 177-check PMM test, 191-check process-memory test, 243-check
anonymous-memory test, `idt.c`,
`gdt64.c`, ABI document, boot log and receipt
generator. A fresh native UEFI64 QEMU run records:

| Assertion | Observed result |
|---|---|
| syscall lifecycle | Ring 3 enters with `iretq`, makes six `int 0x80` calls, exits and returns to a live kernel |
| unknown syscall admission | zero, 25, the sign bit and all bits set return `-ENOSYS` |
| anonymous lifecycle | Ring 3 reserves two pages, commits zero-filled frames, crosses the page boundary, releases both and has the released copy range refused |
| reserved-page containment | a reserved page stays non-present and faults its owner with vector 14/error `0x4`; sibling `V` exits |
| released-page containment | release removes the PTE and frame, then exact-address access faults with vector 14/error `0x4`; sibling `R` exits |
| privilege and copy boundary | `cli` gets vector 13, kernel/device accesses get vector 14, and a crossing pointer is refused before dereference |
| separate address spaces | two PML4/user-stack/kernel-stack contexts resume as `AB12` and exit independently |
| fault containment | one process gets vector 13 while its sibling emits `K` and exits |
| lower stack guard | a write to the absent guard PTE gets vector 14, error `0x6`, exact guard CR2; sibling `G` exits and the kernel continues |
| guarded TSS stacks | both two-page supervisor/NX stacks are selected and retain nonzero high-water below 8 KiB across syscall, preemption and fault paths |
| process memory accounting | two fixed owners retain 16-page quotas, two anonymous owners retain 32-page quotas, all live totals return to zero, high-water remains bounded, refusals remain zero and the metadata invariant passes |
| process-frame lifecycle | eight typed frames per slot, disjoint two-process ownership, failure-atomic acquisition and exact final PMM-baseline restoration |
| process identity | stale generation is refused, signed exit and exact fault custody remain distinct, and identity reap follows resource release |
| persistent process service | four kernel work calls produce `ST12`, retain exits 11/22, detach terminal scheduler owners and restore the PMM baseline |
| desktop command route | external `/system/user.bin` is created through Files, started as PID 1000, fault-contained, observed and reaped while the desktop remains live |

The [hosted gate receipt](hosted-user-process-gate-2026-09-03.md) records the
exact GitHub Actions run, branch head, Ubuntu host inventory, four boot routes,
native UEFI result, command assertions and explicit physical-evidence ceiling.

One immediately prior QEMU attempt terminated in the emulator itself with
signal 139 after the persistent journal came online. The gate reported that run
as failed and wrote no user-process receipt. The clean retry above is the
admitted evidence; the host-emulator crash is not reclassified as a guest pass.

## Classification

The following rows move from `PLANNED_UNPROVED` to `PARTIAL_CURRENT`:

| Feature | Current bounded proof | Still open |
|---|---|---|
| KR-005 per-process address space | two unique roots, PMM-owned tables/code/stacks, current-build switching, replacement and reclamation | persistent lifecycle service, concurrent teardown, PID reuse and physical proof |
| KR-006 privilege boundary | CPL3/TSS entry, syscall return and exact privilege/page faults | SMEP/SMAP, architecture parity and physical proof |
| KR-007 safe user copy | overflow/full-span admission and crossing-pointer refusal | arbitrary mapping walks, demand/copyout fault recovery and hostile corpus |
| KR-008 NX/W^X | RX code, RW/NX stack, supervisor mappings and non-RWX shipped ELF segments | authorized transitions and system-wide dynamic-map audit |
| KR-009 guard pages | exact lower user-stack guard address gets non-present user-write `#PF`; offender dies while sibling and kernel continue | direct kernel-stack overflow injection, emergency-stack guards, general lifecycle and physical proof |
| KR-010 anonymous memory | bounded reserve/commit/release, zero fill, failure-atomic OOM/rollback, collision/ownership checks, exact native reserved/released faults and PMM reclaim | general virtual-area allocation, demand paging, file/shared mappings, concurrent teardown and physical proof |
| KR-015 memory accounting | exact PMM live/high-water/available/refusal totals and quotas for two fixed and two anonymous owners; target totals restore to zero | unified process/service/cache/pinned/DMA/surface/kernel attribution, pressure/reclaim, SMP and physical proof |
| KR-017 kernel stack management | two guarded PMM-owned supervisor/NX TSS stacks, safe CR3 return order, bounded high-water and reclamation | general per-thread service, direct overflow injection, guarded IST, SMP and physical proof |
| KR-031 fault containment | GP-faulted offender cannot stop its sibling or kernel | broad malformed-state/vector and persistent desktop recovery coverage |
| KR-037 stable userspace ABI | documented version-1 convention plus generated number admission and current Ring 3 lifecycle | signals, compatibility/deprecation tooling and generated argument/layout manifest |
| KR-027 wait/exit status | signed exit and exact fault custody, parent-only host observation/reap, bounded target observation and command reap | userspace wait API, target parent/child authority, concurrency and physical proof |
| KR-028 process handles | generation-tagged slot identity, stale refusal, exhaustion retirement and exact scheduler ownership | userspace opaque handles, delegated authority/revocation and physical proof |

KR-009 and KR-017 are only `PARTIAL_CURRENT`: the fixed lower user-stack guard
is directly fault-observed, while the kernel-stack guards are selected and
use-observed without a deliberate overflow fault. General thread lifecycle,
guarded IST, SMP nesting and physical hardware remain open. KR-010 is also
`PARTIAL_CURRENT`: the fixed anonymous window is real, while general virtual
memory services remain open. KR-036 is separately `PARTIAL_CURRENT`; see
`syscall-abi-feature-classification-2026-08-29.md` for its red/green receipt.
KR-015 is also bounded to physical frames owned by this diagnostic; its separate
classification record preserves the unimplemented accounting categories.

Exact current maturity counts remain in generated `program/FEATURE-STATUS.json`.

## Verification

```sh
ZLOS_SKIP_BUILD=1 kernel/tools/checks/verify-efi.sh
kernel/tools/probes/probe-user-process.py --no-build
python3 tools/gen_feature_status.py --write --selftest
python3 tools/gen_feature_status.py --check --selftest
python3 tools/validate_master_program.py --self-test
tools/doc-check.sh
```
