# zlOS 64-bit user process ABI

**Status (2026-09-03): implemented as a bounded persistent preemptive multi-process
ABI.** The UEFI64 path has two process objects, each with its own PML4, PID,
generation-tagged internal identity, RX
code page, guarded RW/NX user stack, guarded two-page supervisor RW/NX
TSS-selected kernel stack, saved register
frame, handle table and 32-page anonymous-memory window. The inherited kernel map is supervisor-only; user
copies validate the complete range; faults kill only the offender. A fixed
two-slot kernel service selects exact lifecycle handles with bounded round robin
and runs one preemptible process turn per desktop or text work-loop call.
`/system/user.bin` can be loaded from zlfs with `userexec`; its bytes are not
linked into the kernel.

This is deliberately not advertised as POSIX. It is the smallest stable seam
that supports a real isolated file-backed program without importing pathname,
descriptor and virtual-memory policy the system does not yet have.

## Image contract

- path: `/system/user.bin` on a positively identified zlfs volume;
- format: 1..4096 raw x86-64 machine-code bytes;
- entry: byte zero;
- mapping: readable/executable, not writable;
- stack: one 4 KiB read/write, non-executable page with an unmapped lower guard;
- kernel entry stack: two private supervisor read/write, NX pages with an
  unmapped lower guard; TSS `rsp0` selects its virtual top;
- exit: syscall 3; falling through traps and kills only the process.

There are no relocations, dynamic libraries or shared pages yet. Add an ELF
loader only after a second real program needs it.

## Register contract

Invoke `int 0x80` with the syscall number in `RAX` and arguments in `RBX`,
`RCX`, `RDX`. The result is returned in `RAX`. Negative errno values are
returned in two's-complement form. All user pointers are validated across the
complete range before the kernel touches the first byte.

The admitted number set is ABI version 1 in `user_syscalls.json` and the kernel
consumes its generated header. The generator requires unique, ordered,
positive numbers below the sign bit and rejects an undeclared gap in the
current 1..25 range. Any unsigned value outside that set returns `-ENOSYS`;
the target gate covers zero, 26, the sign bit and all bits set.

| nr | operation | arguments | result |
|---:|---|---|---|
| 1 | write console byte | `RBX=byte` | 0 |
| 2 | get PID | none | PID |
| 3 | exit | `RBX=signed status` | does not return to user code |
| 4 | validate readable range | `RBX=ptr RCX=len` | 0 or `-1` |
| 5 | time | none | 100 Hz monotonic tick |
| 6 | yield | none | 0 after another runnable process may run |
| 7 | file open | `RBX=name RCX=len RDX=flags` | handle; flag bit 0 creates |
| 8 | file read | `RBX=handle RCX=dst RDX=capacity` | whole-file byte count |
| 9 | file write | `RBX=handle RCX=src RDX=len` | replaced byte count |
| 10 | file close | `RBX=handle` | 0 |
| 11 | file info | `RBX=slot RCX=name-dst RDX=capacity` | file size |
| 12 | file remove | `RBX=name RCX=len` | 0 |
| 13 | file rename | `RBX=handle RCX=name RDX=len` | 0 |
| 14 | filesystem sync | none | 0 after ordered writeback |
| 15 | IPC send | `RBX=PID RCX=src RDX=len` | bytes queued |
| 16 | IPC receive | `RBX=dst RCX=capacity` | bytes copied; `-EAGAIN` if empty |
| 17 | IPC sender | none | PID of the last received message |
| 18 | window open | `RBX=title RCX=len` | opaque owner-bound handle |
| 19 | window present | `RBX=handle RCX=text RDX=len` | 0 |
| 20 | input poll | `RBX=handle RCX=event-dst RDX=capacity` | 16 bytes; `-EAGAIN` if empty |
| 21 | window close | `RBX=handle` | 0 |
| 22 | anonymous reserve | `RBX=first-page RCX=page-count` | virtual base or negative errno |
| 23 | anonymous commit | `RBX=first-page RCX=page-count` | 0 or negative errno |
| 24 | anonymous release | `RBX=first-page RCX=page-count` | 0 or negative errno |
| 25 | bounded sleep | `RBX=ticks RCX=0 RDX=0` | 0 after the process becomes eligible at its deadline |

Sleep accepts 1 through `0x7fffffff` ticks (100 Hz), measured from the syscall's
current tick. Zero, wider or ambiguous delays and nonzero reserved arguments
return `-EINVAL` without yielding. Only a process currently dispatched by the
persistent service may sleep; diagnostic processes outside that service receive
`-EAGAIN`. The exact lifecycle handle owns the request. Its saved user register
frame resumes at the instruction after `int 0x80`; waiting consumes no dispatch
turns or charged run ticks. Deadline eligibility is wrap-safe and does not
promise immediate execution, real-time latency, suspend semantics or cancellation.

The host service checks cover tick wrap, maximum delay, sibling progress,
unchanged idle output, exact runtime accounting and stale identity rejection.
The added native-UEFI QEMU oracle exercises the Ring-3 syscall and resume path
using injected scheduler timestamps. It must observe the `LSW` trace, independent
exit statuses 33 and 44, no dispatch before the deadline and exact frame
reclamation. This is not a wall-clock wake-latency or physical-hardware claim.

Each process has exactly 32 anonymous page slots beginning at PTE 6. Reserve
changes only the typed virtual state and consumes no physical frame. Commit
allocates and zeroes the complete requested frame set before atomically
publishing writable user/NX PTEs; any OOM or page-table transaction failure
restores the exact allocator and mapping baseline while retaining the
reservation. Release atomically removes every committed PTE before returning
its owner-checked frames to the PMM. Reserved and released pages are not valid
user-copy ranges and remain non-present to Ring 3.

The page-table invariant admits x86's hardware-managed accessed and dirty bits
without weakening the physical-address, permission or owner checks. A rollback
whose mapping state cannot be proved is marked broken and retains its frames;
it is never silently reclaimed.

File reads/writes are whole-file operations capped at 4096 bytes. New zlfs v2
volumes use copy-on-write data plus dual checksummed directory generations, so
this avoids inventing partial-write semantics the flat namespace does not
promise. Explicit syscall 14 is the durability boundary.

## Process identity and termination custody

Each live process has an internal 64-bit handle containing its bounded slot and
a nonzero generation. Reusing a reaped slot increments the generation. A slot
whose generation is exhausted is permanently retired instead of wrapping, so an
old handle cannot name a replacement. PID remains the human-facing label and
current IPC selector, but PID lookup resolves through the lifecycle table and
accepts only the exact runnable handle stored by that process object.

Runnable, exited and faulted are distinct lifecycle states. A normal exit keeps
the signed status supplied in `RBX`; a fault keeps the exact vector, error code
and address. Resource teardown happens before identity reap. The table also
refuses to reap a parent while a live child still names that exact parent
generation. The host lifecycle test covers capacity, duplicate PID refusal,
parent-only observation and reap, exact exit/fault records, generation
exhaustion, stale-handle rejection and slot reuse.

The current native-UEFI QEMU receipt executes generation reuse, preserves exit
status `-7`, keeps GP fault `(vector 13, error 0, address 0)` distinct from
sibling exit status `7`, and requires final identity reclamation after resource
teardown. The persistent service uses those exact handles as scheduler owners
and fail-stops if lifecycle and policy state disagree. This is QEMU functional
evidence, not physical-hardware proof. There is not yet a userspace
process-handle syscall, a general spawn/wait/cancellation ABI, persistent
parent/child authority or concurrent PID-reuse test.

## Proof and remaining boundary

`verify-efi.sh` proves the normal entry/return path and hostile cases: `cli`
gets `#GP`, kernel/device reads or writes get `#PF`, a crossing pointer is
refused before dereference, the process dies alone, and the kernel continues.
It also executes unknown syscall IDs 0, 25, `2^63` and `2^64-1` from Ring 3 and
requires `-ENOSYS` for all four.
The built-in user image also exercises time and yield. A second gate alternates
two separate CR3/kernel-stack contexts across yield, verifies the resumed
register/iret frames produce `AB12`, and then proves a process that executes
`cli` gets `#GP` while its sibling still runs and exits.

The lower user-stack guard is runtime-observed, not only source-declared. A
dedicated process writes to the middle of the absent guard PTE; the exception
path must retain vector 14, page-fault error `0x6` (non-present user write) and
the exact CR2 address. That offender is removed while sibling `G` exits and the
kernel continues.

Each fixed process also maps a private two-page supervisor-only NX kernel stack
above an absent guard PTE. Native UEFI verifies both TSS `rsp0` selections and
nonzero high-water below 8 KiB after syscall, timer-preemption and fault paths.
The abort path first moves `RSP` to the permanent kernel stack and only then
restores the kernel CR3, so it never unmaps the active process stack. This is
use and headroom evidence, not a deliberate kernel-stack overflow fault; IST1,
general per-thread allocation/reclamation and SMP nesting remain open.

The four page-table levels, code page, user stack and both kernel-stack pages
are eight PMM-owned frames per fixed process slot. Host tests force allocation
to fail at every short-pool boundary and require byte-for-byte ownership and
accounting rollback. Native UEFI allocates two disjoint sets, repeatedly
replaces them across the process probes, then releases both and requires the
exact pre-process PMM baseline. Final teardown must also leave every lifecycle
slot empty while retaining its generation history.

The persistent service then admits two exact lifecycle handles. Four separate
kernel work-loop calls produce round-robin trace `ST12`: each process writes one
byte and yields on its first turn, then writes one byte and exits with statuses
11 and 22 on its second. The gate observes both terminal records, detaches both
scheduler owners, reaps both lifecycle identities and restores the exact PMM
baseline. The coordinator host test also injects runner failure, policy
corruption, lifecycle-policy drift, stale generations, mid-turn reap and counter
saturation. This is bounded kernel-owned persistence, not a userspace
spawn/wait/process-handle ABI.

Each fixed process owner has a 16-page quota because replacement acquires a
complete eight-page successor before releasing the predecessor. Each anonymous
owner has a 32-page quota, equal to its complete typed window. The allocator
tracks exact live pages, high-water, quota-aware availability and refusals per
owner. The target gate requires all four owner totals to return to zero, both
quota classes to remain exact, high-water to stay bounded, refusal totals to
remain zero and the metadata-to-owner sum invariant to pass. This is process
physical-frame accounting only, not unified accounting for services, caches,
pinned memory, DMA, surfaces or every kernel allocation.

Anonymous-memory host tests cover reserved/committed state, exact zero fill,
deterministic zeroed reuse, every short-pool OOM point, overlapping and hidden
PTE collisions, every injected write/flush rollback point, foreign ownership,
legal hardware accessed/dirty bits, mixed release and holey teardown. Native
UEFI then runs the actual ABI from Ring 3: it commits two zero pages, writes and
reads both, validates a cross-page user range, releases them, and confirms the
released range is refused. Separate processes read a reserved page and a
released page; each gets vector 14 with non-present user-read error `0x4` at
the exact anonymous base while siblings `V` and `R` still exit.

The PIT path uses the same complete saved frame. The timer ISR records the
interrupted Ring-3 frame, returns to the kernel scheduler, changes CR3 and the
TSS `rsp0`, and later resumes the process at the exact interrupted instruction.
The EFI gate runs two infinite-loop images that never issue yield or exit and
observes both `P` and `Q`, proving timer-driven progress rather than cooperative
switching.

IPC is nonblocking and bounded: each process owns four 64-byte queue slots.
Send validates the complete source range, rejects unknown/non-runnable PIDs and
returns `-ENOSPC` when full. Receive validates the destination before removing
the oldest message and exposes its sender through syscall 17. The EFI gate
exchanges `hi`/`ok` across separate PML4s and verifies sender IDs as `h1o2`.

The first window ABI is intentionally text-first rather than a shared raw
framebuffer. Two opaque windows are available system-wide; each carries a
256-byte presented text payload and eight fixed 16-byte input records
(`type`, `code`, `x`, `y`). The WM recognizes reserved user-app IDs, renders
through the normal retained-client path, routes keyboard/pointer events into
the owner queue, and closes every owned window on exit or fault. The EFI gate
opens a real WM window, presents `Ring3 window`, injects/polls key `W` through
the same bounded queue, closes it and proves no owner leak remains.

The desktop command route is also QEMU-observed rather than inferred. Its probe
creates a four-byte external `/system/user.bin` through Files and the disk-backed
editor, confirms the file through `ls`, starts PID 1000 with `userexec`, observes
the intentionally invalid image as a contained fault with `userps`, reaps slot 1
and then observes an empty table. That fixture proves external file loading,
command dispatch, fault custody and reap; it deliberately does not claim a
successful application workload. The
[GitHub-hosted gate](../../../../docs/evidence/hosted-user-process-gate-2026-09-03.md)
repeated this exact route under Ubuntu QEMU TCG and retained its six-assertion
receipt separately from physical-hardware evidence.

Still not complete process infrastructure: the anonymous window is fixed and
has no virtual-area allocator, demand-fault commit, file mapping, shared memory
or concurrent teardown protocol. The persistent scheduler remains fixed at two
slots and has no userspace process-management or cancellation ABI. The window ABI does not yet expose
pixel buffers, resize/configure events or clipboard; and no zl interpreter runs
as a user process. Memory accounting is not yet unified beyond these PMM-owned
fixed and anonymous frames. Those are separate gates; this document does not
call them implemented.
