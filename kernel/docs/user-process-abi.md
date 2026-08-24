# zlOS 64-bit user process ABI

**Status (2026-08-21): implemented as a bounded first ABI.** The UEFI64 path
has a per-process PML4, a supervisor-only inherited kernel map, one RX code
page, a guarded RW/NX stack, validated user copies, kill-on-fault, and an
`int 0x80` entry on a dedicated TSS kernel stack. `/system/user.bin` can be
loaded from zlfs with the `userexec` command; it is not linked into the kernel.

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
| 6 | yield | none | 0 |
| 7 | file open | `RBX=name RCX=len RDX=flags` | handle; flag bit 0 creates |
| 8 | file read | `RBX=handle RCX=dst RDX=capacity` | whole-file byte count |
| 9 | file write | `RBX=handle RCX=src RDX=len` | replaced byte count |
| 10 | file close | `RBX=handle` | 0 |
| 11 | file info | `RBX=slot RCX=name-dst RDX=capacity` | file size |
| 12 | file remove | `RBX=name RCX=len` | 0 |
| 13 | file rename | `RBX=handle RCX=name RDX=len` | 0 |
| 14 | filesystem sync | none | 0 after ordered writeback |

File reads/writes are whole-file operations capped at 4096 bytes. zlfs already
provides data-before-metadata replacement, so this avoids inventing partial
write semantics that its current flat on-disk format cannot safely promise.
Explicit syscall 14 is the durability boundary.

## Proof and remaining boundary

`verify-efi.sh` proves the normal entry/return path and hostile cases: `cli`
gets `#GP`, kernel/device reads or writes get `#PF`, a crossing pointer is
refused before dereference, the process dies alone, and the kernel continues.
The built-in user image also exercises time and yield.

Still not complete process infrastructure: there is one process slot, no
preemptive process scheduler/CR3 switch between multiple user processes, no
window/input ABI, and no zl interpreter running as a user process. Those are
separate gates; this document does not call them implemented.
