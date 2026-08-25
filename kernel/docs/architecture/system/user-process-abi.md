# zlOS 64-bit user process ABI

**Status (2026-08-22): implemented as a bounded preemptive multi-process
ABI.** The UEFI64 path has two process objects, each with its own PML4, PID, RX
code page, guarded RW/NX user stack, TSS-selected kernel stack, saved register
frame and handle table. The inherited kernel map is supervisor-only; user
copies validate the complete range; faults kill only the offender. `/system/user.bin`
can be loaded from zlfs with the `userexec` command; it is not linked into the
kernel.

This is deliberately not advertised as POSIX. It is the smallest stable seam
that supports a real isolated file-backed program without importing pathname,
descriptor and virtual-memory policy the system does not yet have.

## Image contract

- path: `/system/user.bin` on a positively identified zlfs volume;
- format: 1..4096 raw x86-64 machine-code bytes;
- entry: byte zero;
- mapping: readable/executable, not writable;
- stack: one 4 KiB read/write, non-executable page with an unmapped lower guard;
- exit: syscall 3; falling through traps and kills only the process.

There are no relocations, dynamic libraries or shared pages yet. Add an ELF
loader only after a second real program needs it.

## Register contract

Invoke `int 0x80` with the syscall number in `RAX` and arguments in `RBX`,
`RCX`, `RDX`. The result is returned in `RAX`. Negative errno values are
returned in two's-complement form. All user pointers are validated across the
complete range before the kernel touches the first byte.

| nr | operation | arguments | result |
|---:|---|---|---|
| 1 | write console byte | `RBX=byte` | 0 |
| 2 | get PID | none | PID |
| 3 | exit | none | does not return to user code |
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

File reads/writes are whole-file operations capped at 4096 bytes. New zlfs v2
volumes use copy-on-write data plus dual checksummed directory generations, so
this avoids inventing partial-write semantics the flat namespace does not
promise. Explicit syscall 14 is the durability boundary.

## Proof and remaining boundary

`verify-efi.sh` proves the normal entry/return path and hostile cases: `cli`
gets `#GP`, kernel/device reads or writes get `#PF`, a crossing pointer is
refused before dereference, the process dies alone, and the kernel continues.
The built-in user image also exercises time and yield. A second gate alternates
two separate CR3/kernel-stack contexts across yield, verifies the resumed
register/iret frames produce `AB12`, and then proves a process that executes
`cli` gets `#GP` while its sibling still runs and exits.

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

Still not complete process infrastructure: the two-slot test scheduler is not
a persistent desktop spawn/reap service; the window ABI does not yet expose
pixel buffers, resize/configure events or clipboard; and no zl interpreter runs
as a user process. Those are separate gates; this document does not call them
implemented.
