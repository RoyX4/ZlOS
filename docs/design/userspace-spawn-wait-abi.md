# Bounded userspace spawn/wait ABI

Published in draft PR #15 through `e373dcb`; current reconciliation includes
main `9c4cb509`. Corrected combined source `6ae68572…` passes 78 host targets
and all 11 focused boot/process checks, including four native-UEFI parent/child
scenarios, native/BIOS32 USB re-plug and BIOS32 double-fault capture. Complete
hosted closure and the combined app matrix remain pending. Earlier BIOS32 Run
and 47-app lifecycle proof belongs to `52dc8b9c…`.
See the dated execution receipt for exact identities and limits.
[Execution receipt](../evidence/process-spawn-wait-2026-09-08.md) ·
[Audited source contract](userspace-process-management-next.md) ·
[Roadmap sequence](FULL-ROADMAP-NEXT-PROCESS.md).

This resolves the initial process portions of `D-02`, `D-05`, `D-06` and
`D-07`. Their full-system obligations remain open. The producer is the existing
lifecycle/memory/scheduler service; the consumer is a raw userspace program
using the existing x86-64 interrupt entry. No credentials, capability transfer,
ELF loader, argument inheritance, shared mappings or second process registry
are introduced. The earlier flat-file execution authority remains the bound.

## Calls and results

The generated ABI owner remains
[`user_syscalls.json`](../../kernel/src/arch/x86/user_syscalls.json), version 1.
Call through `int 0x80`, with the number in `RAX` and arguments in
`RBX`, `RCX`, `RDX`. `RAX` returns zero or a signed negative error.
All pointers and handles are 64 bits.

| Number | Operation | RBX | RCX | RDX |
|---|---|---|---|---|
| 26 | SPAWN | Name pointer | Name length | Writable eight-byte handle output |
| 27 | WAIT | Exact child handle | Writable result pointer | Exactly 32 |

SPAWN accepts 1..23 name bytes without embedded NUL, using the existing flat
zlfs lookup; slashes do not introduce a new directory resolver. The stored
program must contain 1..4096 raw x86-64 bytes. It acquires an empty one of the
two fixed slots. It never replaces a live process. The kernel derives the
parent from the current process; the caller cannot supply kernel authority.

The output handle is `(generation << 32) | (slot + 1)`. Every successful
allocation advances the slot generation. Generation `0xffffffff` is valid;
after that identity is reaped the slot is retired, rather than wrapping.
Generations with the high bit set are written as eight bytes, never returned
in the signed-error register. Kernel command PIDs remain separate display IDs.

WAIT is nonblocking. A live child returns `-11`; the caller can yield and retry.
A successful call obtains the terminal record, releases that child and copies
the result. Reusing the consumed handle returns a stale-identity error.
[`user_process_abi.h`](../../kernel/src/arch/x86/user_process_abi.h) fixes the
little-endian layout with compile-time size and offset checks:

| Byte offset | Width | Meaning |
|---|---|---|
| 0 | u32 | Result version, 1 |
| 4 | u32 | Kind: 1 normal exit, 2 fault |
| 8 | i32 | Signed exit status; zero for a fault |
| 12 | u32 | Fault vector; zero for a normal exit |
| 16 | u32 | Fault error; zero for a normal exit |
| 20 | u32 | Reserved, always zero |
| 24 | u64 | Fault address; zero for a normal exit |

| Error | Meaning |
|---|---|
| -1 | WAIT caller does not own this child |
| -2 | Missing executable, or stale/consumed child identity |
| -5 | Filesystem, ownership, service, admission or cleanup failure |
| -11 | Child is still live |
| -12 | Insufficient physical frames for the child |
| -22 | Invalid name/image/range/handle shape/result size |
| -28 | No reusable empty process slot |
| -38 | Unknown syscall number |

The existing desktop `userexec` wrapper keeps its `-3` invalid-image diagnostic;
the new userspace call uses `-22`. Syscall admission still rejects zero,
unknown and high-bit syscall numbers. That rule does not reject high-bit
*process handles* in a valid WAIT argument.

## Ownership and publication

The first kernel-side image creation captures a supervisor-only root while
userspace is not running. Image construction uses that retained root, never
the active parent's root. The current kernel root layout must remain stable;
dynamic replacement of the kernel root or adding new top-level kernel mappings
needs a separate template refresh/lifetime protocol before it is supported.

Each child owns eight physical frames: four page-table frames, code, user
stack and two kernel-stack frames. Its user branch contains RX code, a lower
guard, an RW/NX user stack, another guard, and supervisor RW/NX kernel stacks.
Anonymous entries begin absent. Parent anonymous mappings are not inherited.
Existing owner accounts allow two image sets during kernel-side replacement;
userspace SPAWN uses only an empty slot. Per-process anonymous ownership stays
in the existing separate account with its existing 32-page bound.

| State | Owner and rule |
|---|---|
| Candidate image frames and page tables | Local candidate owns all acquired frames until publication or rollback |
| Lifecycle and scheduler slots | Preflight private copies; publish the admitted child only after every fallible preparation step |
| Parent identity, mappings, file handles, inbox, saved frame | Existing parent process; construction leaves them intact |
| Selected process, active CR3, TSS stack and return context | Current single-CPU execution path; only explicit selection/entry changes them |
| Per-process FP image | Reset only the new child's image; active user/kernel FP scratch remains unchanged |
| Exit or fault record | Lifecycle owner retains it until successful owned reap; failed cleanup retains identity and record |

SPAWN validates the full writable eight-byte output range and copies the name
before file reading or allocation. It prepares the lifecycle and scheduler
candidate, acquires private frames, initializes anonymous metadata, then
publishes the child architecture state and admitted slots together. The final
output store follows that publication and cannot fail under this profile.

That last property is deliberately bounded: the syscall interrupt gate masks
interrupts, one CPU owns this service, these operations do not reschedule into
another userspace operation, and neither operation changes the parent's
validated output mappings. The copy is a direct byte store, with no second
fallible validation after custody has transferred. SMP, concurrent unmapping,
nested scheduling or recoverable hardware copy faults require a different
transaction/pinning protocol. They are not claimed here.

| Failure boundary | Required retained state |
|---|---|
| Name, output, file size, missing file or table capacity | No child published; output untouched |
| Lifecycle/scheduler admission | Original slots and parent unchanged |
| Any of eight frame allocation positions | Candidate frames released; original slots, parent and output unchanged; next PID restored |
| Nonempty anonymous custody in an apparently empty slot | Refuse construction without overwriting that custody |
| Internal rollback with corrupt frame ownership | Retain custody and halt with a diagnostic; never silently discard the owner |
| WAIT output or ownership validation | Child identity and termination record untouched |
| WAIT cleanup refusal | Identity, termination record and output retained for retry; scheduler detachment may already have succeeded |
| Successful WAIT | Child resources reclaimed, identity consumed, exact typed result copied |

Allocation/refusal telemetry is allowed to advance; “unchanged accounting”
means resource ownership and live allocation totals, not erased observations.

## Parent death

The privileged lifecycle adoption operation validates the whole table and the
exact terminal parent before changing any child references. A live parent,
stale generation or corrupt table is refused. Direct children transfer to
kernel custody; their identities and records do not change. Grandchildren keep
their direct parent's custody until that parent terminates.

| Ordering | Outcome |
|---|---|
| Parent dies while child is live | Adopted child remains schedulable; administrative reap returns pending until it terminates |
| Child terminates before parent | Parent can WAIT normally; if parent dies first, the retained child result transfers to kernel custody |
| Terminal parent is reaped | Adoption closes child references before the parent identity can be released/reused |
| Adopted child later terminates | Existing kernel administrative reap releases it |

Production service reconciliation adopts children after the parent returns
from its terminating userspace step. Slot release also enforces adoption before
identity release. Host tests cover both cleanup orders and terminal retry.
The two target orphan modes require child administrative reap before releasing
the parent identity, so delayed adoption during parent cleanup cannot satisfy
the oracle. `userps` provides the existing physical-frame total and allocator
invariant before admission and after all reaps. Actual results and exact image
identities live in the execution receipt. Automatic orphan reap remains open.
