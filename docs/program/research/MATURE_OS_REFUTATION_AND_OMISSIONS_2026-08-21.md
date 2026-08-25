# Mature OS refutation and omissions pass — 2026-08-21

This is an independent adversarial pass over the eight repositories covered by
`MATURE_OS_AUDIT_2026-08-21.md`. It does not repeat the feature catalogue. It
tries to falsify the strongest security, build and test conclusions, records
material omissions, and states exactly what the parent synthesis must correct.

## Bottom line

The original audit's central evidence rule is correct: source, build
reachability, tests, configured CI and observed runtime are different claims.
This pass found six high-impact omissions and two important evidence
corrections:

1. **banan-os has a source-level privilege escalation in `setreuid`.** The
   unprivileged rejection predicates use `== saved-id` where they need `!=`.
   For the normal login path, root first drops all three IDs to the selected
   user; a later `setreuid(-1, 0)` is therefore accepted and installs effective
   UID 0.
2. **LemonOS corrupts credentials on fork.** `Process::Fork()` assigns the
   parent's effective GID into the child's effective UID and never copies the
   effective GID. With the normal root-group state, a child can become
   effective UID 0. Its login also calls a `setgid` stub that returns success,
   and `kill` has no ownership check.
3. **hhuOS has Ring-3 execution but not a meaningful protection policy.** Any
   userspace caller can request a writable mapping of arbitrary physical I/O,
   mount/unmount filesystems, kill arbitrary processes and shut down the
   machine. The syscall dispatcher and handlers directly dereference caller
   pointers.
4. **Brook's broad syscall/desktop surface is not a security boundary.** Its
   own durable syscall audit lists roughly 60 unvalidated user-pointer
   dereferences across roughly 40 syscalls. Signal delivery and process-control
   paths lack permission enforcement, and any process can attempt to grab all
   input, map scanout and map other windows' buffers.
5. **duckOS shared-memory authorization is bypassable.** Shared-memory IDs are
   sequential and globally looked up; `sys_shmallow` does not verify that the
   caller created or owns the object before granting permissions. The existing
   `kill lacks permission checks` wording is too broad, however: actual signal
   delivery branches do contain UID checks; signal 0 and a negative-process-
   group branch remain wrong.
6. **MollenOS has an invalid destructor call in its PE cache.** The duplicate
   module path passes a raw image buffer to `ModuleDelete(struct Module*)`,
   which then interprets file bytes as object fields and frees them.
7. **Serenity's CI is multiarchitecture build/boot CI, not
   multiarchitecture target-test CI.** The target runner explicitly skips all
   tests on AArch64 and RISC-V unless passed `--force`; CI does not pass it and
   the runner writes `Failed: 0`.
8. **Brook's freestanding tests are build targets, not registered test
   executions.** Its host-native suite is genuinely registered with CTest;
   the freestanding PMM/VMM/heap/VFS binaries are only created by CMake in the
   inspected graph.

None of these findings is a local runtime reproduction. They are direct source
paths with deterministic control-flow consequences. No repository was built or
booted in this pass.

## Scope, pins and license verification

`git rev-parse HEAD`, `git status --short`, root license inspection and
`git submodule status --recursive` were run against the local source shelf. All
eight inspected worktrees were clean.

| Repository | Inspected commit | Root / first-party license finding |
|---|---|---|
| IanNorris/Brook | `76c3155394d325855019271f2bc3e33c951a6b5e` | MIT (`LICENSE`) |
| Mathewnd/Astral | `465437a974480cbf9f2eb9c190caec81f22919e3` | MIT (`LICENSE`) |
| Bananymous/banan-os | `c915c064e8e42c4fa2e11754baf6c8dc6fb215ef` | BSD-2-Clause (`LICENSE`) |
| LemonOSProject/LemonOS | `15f607e8be86fcaf9c86a68c91915e820c9e935e` | BSD-2-Clause (`LICENSE`) |
| SerenityOS/serenity | `3d83e4509fd20d7438e1ae8470ffe668c136229c` | BSD-2-Clause (`LICENSE`) |
| Meulengracht/MollenOS | `f7d88d4d6f77528aaad8f7840b0b33f1d99add85` | root contains GPLv3 text; first-party source headers say GPL v3 or later |
| hhuOS/hhuOS | `39bf35c6b98bf8665690349579a698a3478be815` | root contains GPLv3 text; first-party source headers say GPL v3 or later |
| byteduck/duckOS | `3634e410c359c97e3b8d038fbffcdd07eddf34d8` | root contains GPLv3 text; first-party source uses `GPL-3.0-or-later` |

The repository-level license never automatically covers vendored libraries,
ports, fonts, firmware, game data or submodules. In particular:

- Astral pins Jinx at `1c40ceb62e09befc5172d1caf53e3e440a19f624`
  and nested uACPI at `022bc38ee675da1c63e6a6567e510cf58b3d2f7e`.
- LemonOS pins cxxshim, fmt, frigg and mlibc separately.
- MollenOS pins libacpi, libgracht, zstd, its image builder and VaFS tool
  separately; the GUI/userspace described in `BUILDING.md` is a separate
  `vali-userspace` source tree, not native audited content here.
- hhuOS contains many separately licensed emulator, game, graphics and FatFs
  submodules. Treating its application catalogue as one GPL-owned codebase
  would be false.
- Brook's `.gitmodules` declares only UEFI headers and FatFs, but the Git index
  also contains undeclared NetSurf-related gitlinks under
  `tools/netsurf-build/`. `git submodule status --recursive` stops with
  `fatal: no submodule mapping found in .gitmodules for path
  'tools/netsurf-build/buildsystem'`. The NetSurf runtime screenshots are claim
  evidence, not a reproducible source/bootstrap receipt from this checkout.

## IanNorris/Brook

### What held

The host-test architecture is real. `src/tests/host/CMakeLists.txt:33-43`
registers executables with CTest, including real TCP, lock, futex, descriptor,
loopback and scheduler-policy sources. Scheduler plugins are built as shared
libraries and exercised through `dlopen` (`:95-127`). The driver hazard ledger
and the broad kernel/build graph also remain valuable evidence.

### Refutations and omissions

- **High — the syscall surface is explicitly unsafe against hostile
  userspace.** `src/kernel/src/syscall_DOCS.md:198-212` records BRO-004
  (roughly 60 unvalidated user-pointer dereferences across roughly 40
  syscalls), BRO-082 (unchecked signal-frame access), weak null-only checks and
  direct dereferences. This should have been in the top-level security matrix,
  not only inferred from scattered TODOs.
- **High — process signalling has no credential check.** `sys_tgkill`,
  `sys_tkill` and `sys_kill` locate a target and call `ProcessSendSignal`
  without comparing real/effective IDs or privileges
  (`src/kernel/src/syscall.cpp:9885-9927`). Signal 0 returns success before even
  resolving a target, so it does not perform the permission/existence probe its
  comment claims.
- **High — compositor privilege is asserted in comments, not enforced.** Any
  process can attempt `brook_input_grab` (`syscall.cpp:10460-10474`), map the
  writable scanout buffer and flip it (`:10797-10819`), enumerate windows and
  map another window's VFB (`:10823-10844`). None of these entry points checks
  EUID, an authority token, service identity or ownership. This is input
  capture, display integrity and window-content confidentiality exposure.
- **High/medium — `read` validates a destination as readable, not writable.**
  `sys_read` writes into `bufAddr` after only calling `UserBufferReadable`
  (`syscall.cpp:1962-1974`). The comment says user pages are always writable,
  but the same file implements real `mprotect` remapping with writable flags
  around `:3912-4009`. A read into a read-only mapping can therefore enter a
  kernel write fault rather than cleanly returning `EFAULT`.
- **Evidence correction — only the host suite is registered.**
  `src/tests/CMakeLists.txt:13-39,41-109` creates freestanding `.elf` targets
  but never calls `enable_testing()` or `add_test()`. Do not label those
  binaries `[T-executed]`; they are test source/build reachability. The host
  suite is `[T-configured]`, with no inspected CI or local result.

### Missed reusable idea

Brook's subsystem-local defect ledgers (`syscall_DOCS.md`, driver docs and
memory docs) are more valuable than the feature count. zlOS should adopt the
same durable hazard inventory, but automatically block promotion when a high
security issue is open.

## Mathewnd/Astral

### What held

The build graph genuinely reaches modern storage, USB, networking, audio,
filesystems and a broad Jinx ports tree. The audit correctly withheld test, CI
and runtime claims.

### Refutations and omissions

- **The init dependency system is ordering metadata, not a robust boot DAG.**
  Every initializer has type `void (*)(void)`
  (`kernel-src/include/kernel/init.h:11-17`), so failure cannot propagate.
  `init_run_routine` recursively enters dependencies and marks a node done only
  after returning (`kernel-src/sys/init.c:5-18`); it has no `visiting` state,
  cycle detection, timeout, rollback or teardown. A dependency cycle recurses
  until failure. The original recommendation to add failure policy was right,
  but the feature should not be summarized as already having a reliable DAG.
- **Positive omission — Astral has a centralized authorization hook model.**
  `kernel-src/include/kernel/auth.h` defines system, filesystem, credential,
  process and network scopes with allow/defer/deny listeners.
  `kernel-src/sys/auth.c` applies deny-first aggregation, and
  `kernel-src/sys/defaultauth.c:298-304` registers the default policy. This is a
  more transferable zlOS idea than several individual drivers, although it has
  no project-native test evidence in this snapshot.
- Existing audit findings on unchecked executable-file mapping permissions,
  GPT checksums, storage timeouts/reset, USB detach, HDA waits and signal
  semantics survive refutation. `kernel-src/mm/mm.c:382-443`, for example,
  explicitly admits unaligned-map breakage and missing executable mapping
  permission enforcement.

## Bananymous/banan-os

### What held

The broad source/build inventory, honest partial TCP status, SHM/damage GUI,
port recipes and absence of CI all held. The 16 userspace tests are compiled
dependencies only: `userspace/tests/CMakeLists.txt:1-27` contains no runner or
CTest registration.

### Refutations and omissions

- **Critical — unprivileged `setreuid` can install arbitrary IDs.** In
  `kernel/kernel/Process.cpp:3837-3849`, the rejection conditions end with
  `euid == suid` and `ruid == suid`. They therefore reject the saved ID in one
  narrow case and allow values that differ from all permitted IDs. The normal
  login child is created as root and then calls `setuid(user)`; root's setuid
  path copies that user ID into real, effective and saved IDs
  (`Process.cpp:3714-3728`). From that state, `setreuid(-1, 0)` passes the
  faulty condition and line 3860 assigns effective UID 0. LibC exposes the
  syscall at `userspace/libraries/LibC/unistd.cpp:843-846`.
- The original audit's `sudo exploitability depends on permissions/syscall
  policy` caveat is no longer sufficient. Even if the sudo binary is not
  setuid, the `setreuid` syscall itself supplies a privilege-escalation path for
  a normally logged-in user.
- The passwordless init remains independently unsafe
  (`userspace/programs/init/main.cpp:85-115`). This is an authentication flaw;
  the `setreuid` predicate is an authorization flaw. They must not be collapsed
  into one finding.

## LemonOSProject/LemonOS

### What held

Typed IPC, the service/interface split, compositor damage tracking, compiled
but disabled xHCI, stubbed HDA and false-green tests all held. The endpoint
capacity defect is stronger after rereading: each endpoint owns a distinct
semaphore, `Write` consumes the sender's semaphore and queues to the peer,
while `Read` returns capacity to the receiver's semaphore
(`Kernel/src/Objects/Message.cpp:50-61,111-165`). One-way traffic deterministically
depletes the sender's credits; runtime scheduling details affect when it hangs,
not whether the accounting is mismatched.

### Refutations and omissions

- **Critical — fork copies the wrong credential field.**
  `Kernel/src/Objects/Process.cpp:573-583` first assigns `newProcess->euid =
  euid`, then overwrites it with `newProcess->euid = egid`. It never copies
  `newProcess->egid`. A parent with effective GID 0 produces a child with
  effective UID 0 regardless of its user UID.
- **High — login's group drop is a success-returning stub.** The GUI login does
  call `setuid(user.uid); setgid(user.gid);` but ignores both results
  (`System/Login/main.cpp:32-50`). Lemon's mlibc implements `sys_setgid` and
  `sys_setegid` as log-and-return-0 stubs
  (`mlibc/sysdeps/lemon/generic/lemon.cpp:129-136`). The session can retain root
  group credentials while believing the drop succeeded.
- **High — arbitrary process signalling.** `SysKill` resolves any PID and
  signals its main thread without UID, session or capability checks
  (`Kernel/src/Arch/x86_64/Syscalls.cpp:2324-2335`).
- **Authentication weakness.** Passwords are a single unsalted SHA-256 digest,
  and the failure path prints both stored and entered digests
  (`System/Login/main.cpp:35-42`). This is not an acceptable password storage or
  logging design.
- These findings lower Lemon's security assessment below `partial`: the typed
  IPC mechanism is a good boundary format, but credentials and process control
  do not preserve that boundary.

## SerenityOS/serenity

### What held

The kernel-enforced monotonic pledge/unveil mechanism survived source
challenge. The VFS enforcement and fork/exec propagation cited by the audit are
real. Host Lagom tests are genuinely run under ASan and UBSan with
failure-halting options (`.github/workflows/lagom-template.yml:88-162`). x86-64
target tests are booted in QEMU and the workflow checks a result file. This
remains the strongest verification architecture in the set.

### Refutations and omissions

- **Evidence correction — target tests are skipped on AArch64 and RISC-V.**
  The CI matrix contains normal AArch64 and RISC-V jobs
  (`.github/workflows/ci.yml:15-34`) and labels their common step `Run On-Target
  Tests`. But `Base/home/anon/Tests/run-tests-and-shutdown.sh:7-19` sets
  `fail_count=0` without invoking `run-tests` on either architecture unless
  `--force` is present. The SystemServer/CI invocation supplies no `--force`.
  The common workflow then accepts the generated `Failed: 0`
  (`.github/workflows/serenity-template.yml:165-188`). Therefore:

  - x86-64: build + QEMU boot + target tests + result check;
  - AArch64/RISC-V: build + QEMU boot/shutdown + result-file plumbing, but no
    target test suite;
  - all architectures: this static audit still has no current remote-run or
    local runtime result.

- **Positive omission — post-test filesystem integrity is explicitly gated.**
  The target workflow runs `e2fsck -f -y _disk_image` before reading results
  (`serenity-template.yml:176-188`). zlOS should copy the idea, not the command:
  test the same persistent image after workload and cold restart, and fail on
  any unexpected repair.
- The browser architecture document remains partly aspirational, ports remain
  third-party, and the lack of a stable ABI/package manager remains correctly
  stated. No evidence was found that refutes the core pledge/unveil claim.

## Meulengracht/MollenOS

### What held

The service split, generated Gracht protocols, read-only VaFS application
images and incomplete scopes/lifecycle findings all held. Internet networking,
E1000 build reachability, xHCI build reachability and native GUI scope must
remain negative/partial exactly as the audit states.

### Refutations and omissions

- **High — PE cache destroys the wrong object.** In
  `services/processd/pe/cache.c:245-287`, `moduleBuffer` is the raw loaded image
  and `module` is the `struct Module*` returned by `ModuleNew`. When
  `__AddModule` reports an existing hash, line 284 calls
  `ModuleDelete(moduleBuffer)`. `ModuleDelete` immediately treats its argument
  as a `struct Module`, destroys hashtables and frees member pointers
  (`services/processd/pe/module.c:67-81`). Reaching this duplicate-module path
  can therefore interpret PE bytes as pointers and corrupt memory. The
  `ModuleNew == NULL` path also leaks `moduleBuffer`.
- **CI test truth remains false-green.** `testing/run-tests.sh:3-11` executes all
  discovered `*_test` files but neither stops nor accumulates status. GitHub CI
  invokes that script directly (`.github/workflows/test.yml:17-23`). Only
  kernel tests are added in unit mode; services are commented out
  (`CMakeLists.txt:39-43`). Report this as `[T-source/configured,
  CI-configured-with-false-green-runner]`, never as a passing test suite.
- Root GPL wording needs precision: the root is a GPLv3 license text while
  first-party file headers generally grant GPL v3 or later. Dependencies keep
  their own licenses.

## hhuOS/hhuOS

### What held

The audit correctly described hhuOS as educational, 32-bit, Ring-3 capable,
UDP-only and unsuitable as a security/networking donor. User threads really do
enter Ring 3 through an IRET frame (`src/kernel/process/Thread.cpp:160-191`).
That privilege transition is source-real, but it does not by itself establish
a safe syscall boundary.

### Refutations and omissions

- **Critical — arbitrary physical-memory/MMIO mapping from userspace.** The
  public `MAP_IO` syscall accepts a caller-chosen physical address and page
  count, directly dereferences a caller result pointer and calls
  `mapIO(..., false)` with no credential/device-ownership check
  (`src/kernel/service/MemoryService.cpp:68-80`). The implementation maps those
  frames writable and user-accessible into the current address space
  (`:354-371`). A Ring-3 process can therefore request access to device or
  physical-memory ranges.
- **Critical — the syscall ABI trusts caller pointers.** The central interrupt
  handler obtains a raw `va_list` and `bool*` from registers and writes through
  the result pointer without validation
  (`src/kernel/interrupt/InterruptDescriptorTable.cpp:372-385`). Individual
  handlers likewise dereference pointers directly; this is a kernel integrity
  boundary failure, not just missing POSIX hardening.
- **High — privileged policy is absent.** Any caller can invoke mount/unmount
  (`src/kernel/service/FilesystemService.cpp:38-60`), kill an arbitrary process
  by ID (`ProcessService.cpp:178-192`) and request shutdown/reboot
  (`PowerManagementService.cpp:33-49`). No UID/GID/capability model was found in
  `Process` or these dispatch paths.
- Reword `user processes` as `separate Ring-3 address spaces and threads, with
  no audited multiuser/privileged-operation policy`. That distinction is
  essential for zlOS planning.

## byteduck/duckOS

### What held

Pond, Quack, River, dynamic linking, E1000 and the kernel unit registry remain
source/build-reachable. CI builds but does not run the QEMU test target. The
SafePointer failure semantics and River message-size omissions remain serious.

### Refutations and omissions

- **Critical — shared-memory permission grant lacks ownership.**
  `kernel/syscall/mem.cpp:94-118` looks up any global shared object by caller-
  supplied ID and grants permissions to any valid PID; its own TODO admits that
  callers may share regions they did not create. IDs start at 1 and increment
  (`kernel/memory/AnonymousVMObject.cpp:7-9`). `get_shared` performs only the
  lookup (`:66-74`), while `share` records the target PID as owner on first
  publication (`:77-87`). A process that guesses/enumerates an ID can grant
  itself read/write access to another process's shared buffer. This directly
  undermines Pond/Quack SHM confidentiality and integrity.
- **High — SafePointer failure continues into invalid access.**
  `Process::check_ptr` sends SIGSEGV but returns void; after a missing-region
  result it still calls `region.value()`
  (`kernel/tasking/Process.cpp:333-343`). SafePointer then executes the caller's
  dereference/memcpy after checking (`kernel/memory/SafePointer.h:188-200`). The
  audit's `TODO failure semantics` wording should explicitly state potential
  kernel assertion/fault or continued invalid access.
- **High — off-by-one page fault check.**
  `AnonymousVMObject::try_fault_in_page` rejects `page > size` rather than
  `page >= size`, then indexes `m_physical_pages[page]`
  (`AnonymousVMObject.cpp:107-117`). A fault exactly one page past the object can
  index out of bounds.
- **Correction — `kill` is not wholly permissionless.** Despite the stale TODO
  at `kernel/syscall/kill.cpp:8`, the PID and process-group delivery branches
  compare UIDs (`:15-47`). Real defects remain: signal 0 returns success without
  target or authorization checks (`:9-10`), and the `pid < -1` branch signals
  the caller itself after the group loop (`:31-39`). Replace the blanket audit
  statement with this exact behavior.
- River still deserializes function parameters while its size check is
  commented out (`libraries/libriver/Function.hpp:108-123`), and Pond accepts
  unbounded requested window sizes (`services/pond/Client.cpp:146-156`).

## Cross-repository integration constraints

1. **No implementation code should be copied.** The project request is for
   concepts and strategies. This is especially important for GPL-v3-or-later
   MollenOS, hhuOS and duckOS, but provenance is required for permissive code as
   well.
2. **Do not copy security APIs without their enforcement topology.** Serenity's
   pledge/unveil works because the kernel enforces monotonic state at shared VFS
   and exec/fork boundaries. Brook's `privileged compositor` comments and
   hhuOS's Ring-3 transition show that naming a boundary does not enforce it.
3. **Generated/typed IPC is necessary but insufficient.** Lemon and duckOS show
   that a schema does not supply queue-credit ownership, length validation,
   cancellation, credentials, handle ownership or disconnect semantics.
4. **Port catalogues are not native OS features.** Preserve source digest,
   dependency licenses, patches, build receipt, target launch receipt and data/
   asset licenses separately.
5. **A CI job name is not evidence.** Inspect the leaf runner and its exit
   propagation. Mollen's runner can return the last test's status; Serenity's
   non-x86 runner deliberately returns zero without testing; Lemon's tests
   return zero after reporting failures.

## Corrections the parent synthesis must propagate

The final feature matrix, executive summary and zlOS plan must make these exact
changes:

1. Add banan-os `setreuid(-1, 0)` privilege escalation as **critical source
   evidence**. Remove the claim that sudo exploitability is merely unknown
   pending install permissions; the syscall bug is an independent escalation.
2. Add LemonOS fork credential corruption, `setgid` success stub, unchecked
   credential-drop results and permissionless `SysKill`. Lower Lemon's security
   posture; keep typed IPC/compositor as architectural ideas only.
3. Add Brook's BRO-004/BRO-082 usercopy backlog, permissionless signal delivery
   and unauthenticated compositor-control syscalls to its principal gaps.
   Reclassify Brook freestanding tests from executed tests to buildable test
   binaries; retain host CTest as configured test evidence.
4. Rephrase Astral init as **declarative ordering without cycle/failure/
   teardown semantics**. Add its centralized authorization listener framework
   as a missed positive feature, with source-only evidence.
5. Rephrase Serenity as **multiarch build and QEMU boot CI; x86-64 target-test
   CI only**. Add its post-test filesystem consistency gate as a positive idea.
6. Add MollenOS's invalid `ModuleDelete(moduleBuffer)` duplicate-cache path and
   preserve the false-green CI warning. Do not count service tests as reachable
   in unit mode.
7. Rephrase hhuOS as Ring-3 process isolation without privileged-operation or
   safe-user-pointer enforcement. Explicitly record arbitrary physical I/O
   mapping, mount, kill and shutdown access.
8. Replace duckOS `kill lacks permission check` with the precise branch-level
   finding. Add shared-memory ownership bypass, SafePointer continuation and
   the anonymous-VM off-by-one defect.
9. State license scope precisely: root MIT/BSD/GPL findings do not cover ports,
   submodules or assets; MollenOS/hhuOS/duckOS first-party headers are GPL v3 or
   later even where the root file is simply the GPLv3 license text.
10. Keep every one of the above as `[S]` until a minimal reproducer or boot test
    earns `[L]`. None was locally executed in this pass.
