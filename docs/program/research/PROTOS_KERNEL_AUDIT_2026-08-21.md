# Ellicode/protOS-kernel source and mechanism audit

Date: 2026-08-21
Repository: `Ellicode/protOS-kernel`
Immutable revision: `daa97b48e7a213c659d34eed9f3c225568243b56` (`main`)
Local source: `/home/roy/Documents/repos/zl-starred-sources/Ellicode__protOS-kernel`
License: MIT (`LICENSE`), with an embedded MIT notice and attribution for the EvalynOS-derived freelist PMM
Scope measured: 200 tracked files and 14,065 lines across C, headers, and assembly

## Verdict

ProtOS is a compact, unusually broad teaching kernel. In roughly fourteen thousand C/header/assembly lines it attempts a Limine boot, a graphical console, paging, a heap, ring-3 programs, round-robin scheduling, an ELF loader, syscalls, a VFS, a TAR initramfs, devfs, PTYs, asynchronous I/O, IPC, shared memory, a libc-like user library, a user-space window manager, a terminal, and a shell. That breadth makes it a useful map of subsystem seams.

It is not currently a safe implementation donor. The documented build fails on the audited toolchain. A kernel-only target can be made to compile with audit-only command-line workarounds, but the source contains broken ELF validation, unchecked user pointers, an in-range null syscall handler, use-after-free defects, unbounded message/file parsing, and ambient cross-process memory sharing. Treat the structure and learning sequence as inspiration; treat the boundary implementations as explicit counterexamples.

The README itself is candid: this is a hobby and learning kernel, not a production claim. Its `TODO.md` also accurately leaves PCI integration, storage, FAT32, the UI library, argv redesign, and most desktop applications unfinished.

## Evidence vocabulary

- `[C]`: repository claim or checklist only.
- `[S]`: mechanism read in source at the pinned revision.
- `[B]`: compilation/build evidence produced during this audit.
- `[R]`: runtime evidence produced during this audit.
- `[G]`: gap, unsafe boundary, contradiction, or unreachable path established from source/build evidence.

No `[R]` claim is made. No QEMU boot was reached.

## Build challenge

### Documented build: failed

The README prescribes:

```text
cmake -S . -B build
cmake --build build
```

An out-of-tree equivalent failed immediately under GCC 15.3.0 because the kernel code model does not support position-independent compilation. `CMakeLists.txt` passes `-no-pie` as a compile option, but does not pass `-fno-pie`. `[B][G]`

### First workaround: progressed, then failed

Configuring with `-DCMAKE_C_FLAGS=-fno-pie` advanced to `src/userspace/process.c`, then C23 compilation failed because `aio_remove` is called without its declaration: `process.c` does not include `filesystems/aio.h`. `[B][G]`

Warnings on that path independently exposed:

- a malformed `"\\x7FELF"` hex escape in the ELF magic check;
- an unsigned process-read return compared with a negative value;
- an integer-to-pointer width warning in address-space destruction;
- user executables linked with writable-and-executable load segments. `[B][G]`

### Second workaround: kernel target compiled, whole build did not

An audit-only global forced include of the missing AIO header, plus `-fno-pie`, allowed `kernel.elf` to link. It caused duplicate VFS declarations while compiling the user library, so the complete build still failed. This is evidence that most kernel translation units compile; it is not a clean or documented build success. `[B]`

There is no repository CI configuration and no test/spec tree in the pinned source. `[S]`

## Boot and hardware architecture

### Present mechanisms

- Limine base revision 5 supplies memory maps, HHDM, kernel addresses, framebuffer information, modules, and boot-time kernel file data. `[S]`
- The target is x86-64 UEFI/OVMF, C23 plus NASM, with a higher-half kernel linker script. `[S]`
- GDT/TSS, IDT stubs, PIC remapping, PIT ticks, panic handling, and serial logging are included in the active CMake target. `[S]`
- PS/2 keyboard and mouse drivers publish input through IPC topics. `[S]`
- PCI configuration-space enumeration scans every bus and slot, but only function zero. `[S]`

### Gaps and failure modes

- The system is legacy PIC/PIT/PS2 based. There is no active APIC, SMP, ACPI topology, MSI/MSI-X, PCI BAR driver, storage driver, networking, or USB stack. `[S][G]`
- `src/acpi.c` is absent from the active CMake source list; ACPI is not an integrated subsystem. `[S][G]`
- PCI class names are indexed directly by an unbounded 8-bit class code even though the initializer covers only the small defined class range. An unknown class can read beyond the array. `[S][G]`
- PCI enumeration ignores functions 1-7 and multifunction header semantics. `[S][G]`
- PS/2 controller waits have no timeout. On absent or nonresponsive hardware, boot can wait forever. `[S][G]`

## Memory architecture

### Present mechanisms

- A lazily filled, ticket-lock-protected physical-page freelist is derived from EvalynOS with source attribution and its MIT notice retained. `[S]`
- Four-level page tables map the kernel, HHDM, framebuffer, process image, fixed user heap, fixed user stack, and a fixed user framebuffer region. `[S]`
- Each process receives a new PML4 with upper-half kernel entries copied from the kernel table. `[S]`
- The kernel enables CR0.WP and provides virtual-to-physical translation and address-space teardown. `[S]`
- Page-table and PMM mutations have global ticket locks. `[S]`
- PAT support is used to request write-combining for the user framebuffer. `[S]`

### Gaps and failure modes

- Executable permission is not constrained: the page model exposes no NX policy in the inspected mappings. User programs are also linked with RWX load segments. `[S][B][G]`
- `vmm_map_range` leaks all pages allocated earlier in a range when a later allocation fails. `[S][G]`
- `vmm_share_range` installs writable mappings into a destination at the same caller-selected virtual addresses, with no ownership, capability, destination consent, collision, or rollback contract. `[S][G]`
- Shared pages have no reference counts. Address-space destruction skips only the fixed framebuffer and otherwise frees mapped physical pages, so shared ownership is not modeled safely. `[S][G]`
- The fixed per-process stack virtual range is reused for every user thread. Multiple user threads in one process collide. `[S][G]`
- The heap has a declared maximum, but `brk` does not enforce it or check collisions with other regions. `[S][G]`
- `getmemsz` accumulates 64-bit memory-map lengths in `int`, and it counts all map entry types rather than usable memory despite the variable name. `[S][G]`
- Physical allocation logs out-of-memory but then dereferences a null freelist node. `[S][G]`

## Processes and scheduling

### Present mechanisms

- ELF processes, per-process descriptor tables, current working directories, PIDs, parent-like blocking creation, and nonblocking creation are implemented. `[S]`
- A global round-robin thread list performs interrupt-frame switching and uses per-thread kernel stacks. `[S]`
- Threads can be put to sleep awaiting a process or IPC queue and awakened later. `[S]`
- User-space entry assembly and a fixed 16-by-64-byte argv ABI complete a small end-to-end process path. `[S]`

### Gaps and failure modes

- Scheduling is single-core and global. There is no SMP ownership, per-CPU run queue, priority, fairness accounting, time budget, affinity, or real-time policy. `[S][G]`
- `exit_thread` frees the thread and then dereferences `thread->process`, a direct use-after-free. `[S][G]`
- Creating a user thread leaks its allocated thread structure if stack mapping fails. `[S][G]`
- Several `create_process` failure paths leak the new address space, file buffer, process, or mapped pages. `[S][G]`
- The VFS read result is stored in `uint64_t` and compared with a negative/less-than-zero success boundary, so errors encoded as negative values are not detected. The compiler confirms the comparison is always false. `[S][B][G]`
- The program heap is mapped before the ELF is validated, increasing cleanup complexity and exposure. `[S][G]`
- The argv ABI is a hard fixed ceiling rather than length-delimited input. The repository itself lists this for replacement. `[C][S]`

## ELF loading and symbolization

### Present mechanisms

- The loader recognizes x86-64 ELF headers, iterates program headers, maps segment pages, copies file data, zeros BSS tails, and supports a relocation base for `ET_DYN`. `[S]`
- Boot-supplied kernel ELF data is reused to load a symbol table for panic backtraces. `[S]`

### Critical validation gaps

`elf_read` is not a valid trust boundary:

- it returns invalid when `strncmp(...) == 0`, rejecting matching magic rather than mismatching magic;
- the literal `"\\x7FELF"` is itself malformed because the hex escape consumes following hex characters; GCC warns that it is out of range;
- it receives no file size;
- it validates neither ELF class/data/version nor header sizes;
- it trusts program/section table offsets, counts, and entry sizes;
- it validates no segment `offset + filesz`, `vaddr + memsz`, `filesz <= memsz`, address range, overflow, overlap, or page permissions. `[S][B][G]`

Both user process loading and kernel symbol loading use this parser. Malformed initramfs programs can drive out-of-bounds kernel reads and arbitrary mappings; malformed boot-time kernel-file metadata can drive the symbol loader similarly. `[S][G]`

`get_symbol` stores `&symbol`, where `symbol` is a loop-local copy, then reads it after the loop. That is a dangling stack pointer and undefined behavior in the panic path. `[S][G]`

## Syscall and authority boundary

### Present mechanisms

The syscall table covers process creation/exit, file I/O, directory reads, stat, cwd, heap growth, framebuffer access, IPC send/receive/dispatch/subscribe, memory sharing, cursor/console operations, memory statistics, and panic. `[S]`

### Critical boundary gaps

- Kernel handlers directly call `strlen`, `strcpy`, `memcpy`, VFS routines, and IPC routines on raw user-supplied pointers. There is no central `copy_from_user`, `copy_to_user`, string-length cap, canonical-address validation, mapping/protection validation, or fault recovery. `[S][G]`
- `SYS_UNSHARE` is present in the syscall enum but its handler entry is commented out. Its numeric ID remains in range, so dispatch obtains a null function pointer and calls it. `[S][G]`
- Any process can invoke kernel panic, clear the global console, set the cursor, fetch/map the physical framebuffer, and request process creation. There is no capability or privilege model. `[S][G]`
- `sys_share` lets a caller share a selected range to any PID; it is not scoped by a handle, grant, receiver consent, or revocation. The advertised `unshare` is absent. `[S][G]`
- Directory reads accept no entry capacity, while stat/cwd/framebuffer operations write directly to caller pointers. `[S][G]`
- Heap growth does not enforce the declared maximum or reserve/commit ownership. `[S][G]`

The reusable idea is the small, indexed syscall surface. The implementation lesson is that every entry must route through one checked user-memory and authority layer before subsystem code.

## IPC, events, and shared memory

### Present mechanisms

- Per-process queues support blocking and nonblocking receive. `[S]`
- Named messages carry sender PID, payload size, and copied payload. `[S]`
- Global named publish/subscribe topics distribute keyboard and mouse events and window-manager requests. `[S]`
- The user-space window manager uses messages for window creation/refresh and shares each window's framebuffer with its client. `[S]`

### Critical failure modes

- Queues and payload sizes are unbounded, enabling kernel-heap exhaustion. `[S][G]`
- Queue locks are represented in the data model but not used around append, unlink, wait, wake, subscription, or process-list traversal. IRQ and process contexts can race. `[S][G]`
- Receive has no destination capacity and copies `top_msg->size` bytes into a raw user buffer. `[S][G]`
- `ipc_consume` frees `meta->msg` before reading `meta->msg->data`, a direct use-after-free. `[S][G]`
- Sending to a nonexistent PID frees the message structure but leaks its separately allocated payload. `[S][G]`
- `strncpy(..., 255)` can leave the 255-byte topic/name arrays unterminated; later `strcpy` and string comparisons assume termination. `[S][G]`
- Topics are an ambient global namespace: any process can subscribe, publish, impersonate an expected topic, or send directly to a PID. `[S][G]`
- A user-controlled metadata object carries the kernel allocation pointer later consumed by `ipc_consume`; raw pointer provenance is not protected. `[S][G]`

For zlOS, preserve the event-driven seam but replace this with bounded kernel-owned envelopes, opaque message handles, capacity-aware receive, authenticated endpoints, backpressure, quotas, and capability-scoped shared-memory grants.

## VFS, TAR initramfs, devfs, PTYs, and AIO

### Present mechanisms

- A tree of inodes/dentries provides lookup, path splitting, open/read/write/close/stat/readdir, cwd traversal, and per-process descriptor tables. `[S]`
- A USTAR initramfs creates the initial hierarchy and file contents. `[S]`
- Devfs exposes character-like devices, including PTYs. `[S]`
- AIO tracks descriptors flagged for asynchronous operation. `[S]`
- The shell, `ls`, `read`, terminal, and process loader exercise this surface. `[S]`

### Parser and size-contract gaps

- Path splitting copies a component into a fixed 256-byte buffer without enforcing that limit. `[S][G]`
- USTAR parsing verifies no archive length or header/data bounds and does not verify checksum, magic/version, octal overflow, null termination, type-specific invariants, links, or total offset overflow. `[S][G]`
- Archive paths are copied into fixed buffers with unsafe string operations. `[S][G]`
- Devfs creation does not consistently validate allocation success, duplicate names, parent type, or name length. `[S][G]`
- Readdir has no caller capacity. `[S][G]`
- TTY write ignores an explicit byte size and treats data as a null-terminated string; TTY read can copy the whole internal buffer to a caller without an output-capacity contract. `[S][G]`
- Circular buffers have no synchronization despite process/interrupt use. `[S][G]`
- Descriptor and process teardown do not establish complete resource ownership for AIO, queues, subscriptions, mapped frames, and shared buffers. `[S][G]`

The useful architectural idea is a small VFS + initramfs + devfs bootstrap sequence. The clean-room requirement is a single checked slice/string ABI and adversarial parser suite before adding more filesystems.

## Graphics and desktop path

### Present mechanisms

- The kernel has framebuffer console text, fonts, pixels, rectangles, images, and BMP support. `[S]`
- `sys_fetch_fb` maps the physical framebuffer at a fixed user virtual address and returns geometry. `[S]`
- `libproto` provides user-space allocation, I/O, process, event, graphics, font, BMP, and window-manager client helpers. `[S]`
- `protowm` is a user-space compositor/window manager with a root background, window stack, frames, shadows, title bars, cursor drawing, drag-to-move, focus-by-stack-order, keyboard forwarding, and terminal launch shortcut. `[S]`
- Window clients allocate their pixel buffers; the WM shares those frames with the client and composites them into the real framebuffer. `[S]`
- A graphical terminal combines a window surface, PTY, font drawing, and shell process. `[S]`

### Gaps and unsafe UI contracts

- Every process can map and write the physical framebuffer, so the WM is convention rather than exclusive compositor authority. `[S][G]`
- Window creation accepts dimensions, positions, names, and frame style from ambient IPC without validated bounds or sender authority. Negative/huge dimensions can overflow allocation and drawing arithmetic. `[S][G]`
- The WM receives into a fixed 512-byte buffer while generic IPC can deliver arbitrary payload size. `[S][G]`
- Keyboard forwarding assumes a nonempty window list and forwards `meta->size` rather than the fixed local event size. `[S][G]`
- Refresh requests identify windows only by integer ID and do not verify that the sender owns the window. `[S][G]`
- Window names may be unterminated after `strncpy`; windows have no close/minimize/resize/damage lifecycle; allocated window surfaces are not reclaimed. `[S][G]`
- Compositing draws directly into one shared framebuffer with no clipping proof, per-client isolation, damage protocol, vsync, double buffering, or display ownership. `[S][G]`

The strong idea for zlOS is user-space desktop policy connected through IPC and shared surfaces. The safe design needs compositor-exclusive scanout, opaque surface/window handles, ownership checks, validated geometry, damage rectangles, bounded events, focus policy, and revocable mappings.

## User applications and libraries

Implemented or substantially present:

- `corgi`: prompt, builtins/external program launching, command splitting; `[S]`
- `protofetch`: system/version/memory display; `[S]`
- `protowm`: compositor/window manager; `[S]`
- `terminal`: graphical terminal plus PTY-backed shell; `[S]`
- `ls`, `read`, and `panic`: small utilities; `[S]`
- `edit`: present as a target but listed unfinished by the README/TODO. `[C][S]`

The library surface is helpful as an OS vertical-slice checklist, but it duplicates kernel/user VFS types and has no ABI versioning, generated syscall contract, conformance tests, or hostile-input tests. `[S][G]`

## Best ideas worth adapting without copying code

1. **Vertical learning slices.** Each feature reaches from kernel mechanism through a small user wrapper to an application. This makes missing seams obvious.
2. **User-space window management.** Desktop policy is not hard-coded into the kernel, even in a small OS.
3. **Shared per-window pixel surfaces.** The shape is useful once replaced by handle-based, ownership-checked grants.
4. **Named event topics.** Human-readable topics make early system composition easy to inspect; production zlOS should compile names to authenticated endpoints or stable IDs.
5. **VFS + TAR + devfs bootstrap.** This is a compact route from boot modules to processes and device-backed terminal I/O.
6. **Candid project accounting.** README and TODO do not pretend that enumeration equals device support or that a stub app is finished.
7. **Source attribution.** The derived allocator retains a visible source-level license notice and attribution.

## Negative patterns zlOS should turn into invariant tests

1. A valid ELF magic must be accepted and every invalid byte/class/size combination rejected.
2. Every file and archive offset calculation must be checked against an explicit byte length with overflow-safe arithmetic.
3. Every syscall table slot must be either a valid handler or a defined `ENOSYS` handler; never null.
4. Kernel code must never dereference a user pointer directly.
5. IPC receive must take a capacity and return required/actual size without overflow.
6. Kernel allocations referenced by users must be opaque handles, never exposed pointers.
7. Free-order tests must catch use-after-free in IPC consumption and thread exit.
8. Shared mappings require owner, grantee, permissions, reference count, collision checks, and revocation.
9. Only the compositor may own scanout; clients receive bounded surfaces.
10. Build gates must execute the documented clean build under position-independent-by-default host toolchains.
11. Every blocking hardware wait needs a timeout and diagnostic.
12. Unknown PCI class/function layouts must remain bounded and enumerable.

## Clean-room role in the 33-repository corpus

ProtOS is a **breadth and seam donor**, not a correctness donor.

- Use its end-to-end subsystem ordering to verify that zlOS architecture reaches real applications.
- Use its user-space WM, PTY terminal, named IPC, shared surface, initramfs, and devfs relationships as questions to answer independently.
- Do not copy its code, constants, ABI, parser logic, pointer model, memory-sharing model, or authority decisions.
- Add the concrete failures above to zlOS design reviews and adversarial gates before equivalent zlOS features are called complete.

## Evidence boundary

Verified in this audit:

- pinned commit and license;
- tracked-file and source-line counts;
- active CMake source graph;
- source mechanisms listed above;
- documented build failure;
- `-fno-pie` progress and AIO declaration failure;
- audit-only workaround compilation of `kernel.elf`;
- absence of a repository test/CI tree at this revision.

Not verified:

- QEMU boot;
- ring-3 execution;
- process switching at runtime;
- filesystem or PTY behavior at runtime;
- IPC delivery at runtime;
- graphical WM/terminal behavior;
- behavior on physical hardware;
- full clean build success.

The weakest link is runtime: this audit establishes rich source reachability and concrete build/source failures, but no booted system evidence.
