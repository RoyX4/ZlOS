# Mature starred OS repository audit — 2026-08-21

> **Adversarial correction notice:** this is the first-pass dossier. The later
> [`MATURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md`](MATURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md)
> is authoritative wherever wording conflicts. It adds critical banan-os and
> LemonOS credential flaws, Brook/hhuOS/duckOS authority failures, MollenOS PE
> cache corruption, and corrects Serenity's non-x86 test lane. The normalized
> matrix and integration contracts incorporate those corrections.

## Exact audited revisions

| Repository | Audited checkout commit |
|---|---|
| `IanNorris/Brook` | `76c3155394d325855019271f2bc3e33c951a6b5e` |
| `Mathewnd/Astral` | `465437a974480cbf9f2eb9c190caec81f22919e3` |
| `Bananymous/banan-os` | `c915c064e8e42c4fa2e11754baf6c8dc6fb215ef` |
| `LemonOSProject/LemonOS` | `15f607e8be86fcaf9c86a68c91915e820c9e935e` |
| `SerenityOS/serenity` | `3d83e4509fd20d7438e1ae8470ffe668c136229c` |
| `Meulengracht/MollenOS` | `f7d88d4d6f77528aaad8f7840b0b33f1d99add85` |
| `hhuOS/hhuOS` | `39bf35c6b98bf8665690349579a698a3478be815` |
| `byteduck/duckOS` | `3634e410c359c97e3b8d038fbffcdd07eddf34d8` |

Astral also contains the populated `jinx` submodule at `1c40ceb62e09befc5172d1caf53e3e440a19f624` and nested uACPI at `022bc38ee675da1c63e6a6567e510cf58b3d2f7e`. hhuOS pins third-party application and rendering submodules; those are treated as dependencies, not hhuOS-native implementations. LemonOS and MollenOS likewise contain vendored/submodule components with their own licenses.

### Orientation and license inventory

| Repository | Orientation/build material read | Root license evidence |
|---|---|---|
| Brook | `README.md:1-284`, root/kernel/test CMake, `src/drivers/DOCS.md:1-57` | MIT, `README.md:282-284`, `LICENSE` |
| Astral | `README.md:1-40`, root/kernel Makefiles, Jinx recipes | MIT, `LICENSE` |
| banan-os | `README.md`, root/kernel/userspace CMake, `ports/README.md`, `script/build.sh` | BSD-2-Clause, `LICENSE:1-24` |
| LemonOS | `README.md`, `Documentation/Build/Building-Lemon-OS.md`, `Documentation/System/IPC/README.md`, CMake/CI | BSD-2-Clause, `LICENSE:1-25` |
| SerenityOS | `README.md:1-147`, `CONTRIBUTING.md`, `Documentation/README.md`, browser/test/bare-metal docs, CMake/CI | BSD-2-Clause, `README.md:145-147`, `LICENSE` |
| MollenOS | `README.md`, `BUILDING.md`, `kernel/README.md`, CMake/CI | GPLv3, `LICENSE` |
| hhuOS | `README.md`, root/subsystem CMake, CI | GPLv3-or-later, `LICENSE` and CI license bundle at `.github/workflows/build.yml:31-48` |
| duckOS | `README.md`, `INSTRUCTIONS.md`, root/kernel/library/service/app CMake, CI | GPLv3-or-later, `README.md:129-130`, `LICENSE.txt` |

## Method and evidence contract

This is a source audit of the downloaded checkouts, not a popularity comparison. For each repository I read its orientation/build documentation, root and subsystem build graphs, CI, central kernel and userspace implementations, tests, TODO/FIXME evidence, and license. I traced representative implementation paths through each discoverable subsystem. No downloaded repository was modified.

Evidence labels are deliberately cumulative:

- **[C] Claimed** — documentation says the feature exists.
- **[S] Source exists** — a substantive implementation was inspected, not merely a filename.
- **[B] Build-reachable** — the implementation is included by the active build/deployment graph.
- **[T] Test-covered** — a focused test source/target reaches the behavior. This does not mean CI runs it.
- **[CI] CI-gated** — CI actually invokes the relevant build/test path.
- **[L] Locally built/run** — reserved for a build or runtime check performed during this audit.

There are **no [L] labels in this report**: no local builds, boots, or tests were run. Consequently, `[C,S,B,T,CI]` is still not hardware/runtime proof. This distinction is the most important finding: several repositories contain compiled stubs, unreachable drivers, tests that cannot fail their process, or CI that only builds images.

## Executive result

No single repository is the template for zlOS. The strongest transferable pieces are:

This report puts the cross-project synthesis and zlOS integration plan before the detailed repository evidence so the actionable result is visible first. The eight per-repository source audits follow the plan.

| Need | Best reference | Why | Main warning |
|---|---|---|---|
| Whole-system breadth and verification structure | Brook, SerenityOS | Brook has unusually direct subsystem tests and driver audit notes; Serenity has the deepest host/target test architecture | Brook remains young; Serenity's scale is expensive to reproduce |
| Protocol-first service architecture | MollenOS, LemonOS | Generated contracts make IPC and provider boundaries explicit | Mollen isolation/lifecycle is incomplete; Lemon endpoint accounting is suspicious |
| Desktop/compositor design | SerenityOS, LemonOS, banan-os | Damage tracking, shared buffers, occlusion, separate servers | Do not copy full framebuffer blits or trust client-supplied dimensions |
| Hardware/storage breadth | Astral, banan-os, SerenityOS | NVMe/AHCI/xHCI/HDA/network implementations are build-reachable | Many timeout, hot-unplug, and recovery paths remain unfinished |
| Reproducible ports | banan-os | Pinned sources, checksums, dependency recursion, patch and build hashes | A recipe is not proof that a port runs now |
| Capability/security model | SerenityOS | `pledge`/`unveil` are monotonic and enforced in VFS/exec/fork | Browser architecture documentation is partly aspirational |
| Read-only application packaging | MollenOS | VaFS application images plus explicit writable data partition | Namespace/process ownership enforcement is mostly TODO |
| Educational clarity | hhuOS | Central boot composition is readable and explicit | 32-bit, no TCP, limited tests; explicitly not a daily-use OS |

The correct zlOS synthesis is: **Brook/Serenity verification discipline + Mollen/Lemon typed service contracts + Serenity monotonic capabilities + Lemon/banan shared-memory damage compositor + banan reproducible ports + an evidence ledger that refuses to call buildable code “working.”**

---

## Cross-repository feature/evidence matrix

This matrix records the highest evidence reached in this audit. “B” does not mean booted; “T” does not mean CI ran the test.

| Capability | Brook | Astral | banan | Lemon | Serenity | Mollen | hhuOS | duckOS |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 64-bit kernel | B | B | B | B | B | B | — (32-bit) | B (i686 primary) |
| SMP/preemption | B/T scheduler | B | B | B | B/T/CI | B | B | planned/partial |
| User processes + ELF | B | B | B | B | B/T/CI | PE loader B | B | B |
| VM/COW/SHM | B/T | B | B (anon COW absent) | B | B/T/CI | B/T/CI | B | B/T |
| Capability sandbox | partial syscall policy | POSIX credentials | weak login model | partial | pledge/unveil B/T | scopes incomplete | minimal | permission gaps |
| VFS | B/T | B | B | B | B/T/CI | userspace filed B | B | B/T |
| Writable ext2-family | ext2 B/T, ext4 module B | ext2 B | ext2 B | ext2 module B | ext2 B/T | no ext2 focus | FatFs/ISO | ext2 B |
| NVMe | B/driver | B | B | B | B | absent | absent | absent |
| AHCI/SATA | —/USB+VirtIO focus | B | B | B | B | B | B | source paths vary |
| xHCI/HID/MSC | B | B | B | compiled but disabled | B/T | source not build-reachable | absent | limited/other |
| TCP/IP | B/T | B | B/T (partial) | B | B/T/CI | Internet unsupported | UDP only | B/T, incomplete |
| HDA/audio mixer | B | B | B | HDA stub | B/T/CI | service split/limited | SoundBlaster | AC97 + Quack |
| Loadable modules | B/T scheduler | init-linked | B | B/configured | B | B/provider modules | static composition | mostly static |
| GUI compositor | B | X/ports | B/damage+SHM | B/damage+SHM | B/damage+server | external repo | no general desktop | Pond B |
| Typed/generated IPC | partial protocol stack | no central IDL | local protocols | IDL B | IPC compiler B | `.gr` contracts B | service calls | River typed RPC |
| Host-native tests | strong | none found | limited userspace tests | weak false-green | strongest | memory tests | libc app only | kernel unit registry |
| CI runtime/test gate | not established here | none | none | build only | yes, broad | yes, runner risk | build only | build only |
| License | MIT | MIT | BSD-2 | BSD-2 | BSD-2 | GPLv3 | GPLv3+ | GPLv3+ |

## Best strategies, compressed

### Architecture

1. **Keep mechanism in kernel, policy in restartable services.** Mollen demonstrates the cleanest shape: kernel handles, VM, futex, IPC and scheduling; filed/deviced/processd/usbd/netd own policy. Serenity shows how this remains practical with many system services.
2. **Make interfaces generated contracts.** Lemon's IDL and Mollen's `.gr` provider schemas prevent client/server drift. A zl interface definition should generate zl client, server, validator, trace decoder, fuzz corpus skeleton and conformance test.
3. **Use monotonic authority.** Serenity's pledge/unveil model only removes authority and enforces it at shared kernel boundaries. This is safer than banan's username/sudo trust or duckOS's missing `kill` permission checks.
4. **Keep boot order declarative and observable.** Astral's init routine dependencies and banan/hhuOS's readable composition roots should become a machine-checkable boot DAG with timing and failure receipts.
5. **Treat read-only images as packaging, not automatic security.** Mollen proves immutable app bytes, but without mount scopes, process ownership and revocation it does not prove isolation.

### Hardware and reliability

1. Initial enumeration is only milestone one. Across Astral, Brook, banan and Serenity, the recurring missing work is **timeouts, reset, cancellation, interrupt teardown, unplug/replug, DMA validation and concurrency**.
2. Every driver should expose a lifecycle state machine: `detected → resources claimed → queues ready → online → quiescing → detached/failed`, with bounded transitions and idempotent cleanup.
3. Storage claims require integrity evidence: checksummed partition parsing, forced I/O errors, power-cut simulation, cache flush ordering and cold-boot persistence. A read/write demo is insufficient.
4. Network claims require deterministic packet/timer tests. Brook's host TCP test is the closest model; duckOS and Astral expose exactly why retransmission/window/OOO logic must be tested without wall-clock/QEMU nondeterminism.

### Desktop and applications

1. Use shared client buffers plus server-owned window metadata, damage rectangles, occlusion and buffer flipping (banan/Lemon/Serenity).
2. Validate dimensions, strides, formats, handles and damage regions at the compositor boundary. Client memory must never define unchecked allocation/copy sizes.
3. Centralize audio in a service with per-client queues and one mixer (Serenity/duckOS). Give each stream bounded buffering, disconnect cleanup, negotiated format/rate and underrun/overrun telemetry.
4. Put hostile parsers in restricted disposable processes (Serenity ImageDecoder/WebContent pattern), especially image/font/archive/network formats.

### Verification and truth

1. The feature ledger must distinguish: `documented`, `source-present`, `build-reachable`, `booted`, `runtime-tested`, `failure-gated`, `QEMU-verified`, `hardware-verified`, plus the exact commit/artifact receipt.
2. Tests must propagate failure. Lemon's always-zero tests and Mollen's last-command-wins runner are concrete false-green examples.
3. Compile real kernel logic into host harnesses where semantics permit (Brook/Serenity). Use sanitizers/fuzzers there; use QEMU/hardware only for the privileged seam.
4. CI must boot the produced image, not only compile/upload it. Assert a version/build receipt inside the booted guest so a stale image cannot pass.
5. Count no vendored test, port recipe or disabled source as native runtime coverage.

## Gaps shared by the field

These are opportunities for zlOS to be unusually strong rather than merely feature-complete:

- **Truthful completion semantics.** Every audited project has at least one place where documentation, source, active build, tests and runtime could be confused.
- **Driver teardown and recovery.** Hot-unplug, failed DMA, reset and interrupt ownership are less mature than discovery.
- **Security after desktop success.** Authentication, credential checks, safe usercopy and IPC validation are commonly behind GUI polish.
- **Full service lifecycle.** Typed messages exist before deadlines, cancellation, peer death, restart, handle revocation and capacity ownership are finished.
- **Persistent correctness under failure.** Filesystems often have basic read/write before cache consistency, flush ordering, crash recovery and corruption handling.
- **Multiarchitecture parity.** Secondary architectures commonly compile before reaching SMP, MMU protection parity or driver usefulness.
- **End-to-end CI.** Build-only pipelines dominate. Serenity is the strongest exception, while Brook is strongest at small host-side kernel tests.
- **Package/runtime proof.** Large ports trees make impressive lists but rarely provide current automated install-and-launch receipts.

## Specific integration plan for zl/zlOS

This is a whole-architecture plan, not a deliberately crippled MVP. The sequencing protects current zlOS hardware/storage work while building toward the complete system.

### 0. Establish the evidence system first

Create one machine-readable feature ledger in the active zlOS repository. Each record must contain owner, dependency, state label, source target, build target, test target, last artifact hash, QEMU receipt, hardware receipt, known hazards and license provenance. Generate human documentation from it. Reject impossible promotions—for example, `runtime-tested` without `build-reachable`, or `hardware-verified` without a matching artifact hash.

Adopt a standard receipt:

```text
feature, source_commit, toolchain_digest, artifact_digest,
machine_or_qemu_profile, action, expected, observed, exit_status,
start/end timestamps, logs/screenshots, reviewer
```

### 1. Make boot a dependency DAG

Replace implicit ordering with Astral-style registered dependencies, while keeping a generated hhuOS-like readable composition view. Every node declares prerequisites, timeout, retry policy, critical/optional status and teardown. Boot logs should emit begin/end/result receipts and identify the exact blocking node.

### 2. Define the kernel/service boundary

Keep in kernel: address spaces, threads/scheduler, handles/capabilities, interrupts, DMA/IOMMU policy, minimal VFS primitives needed for boot, IPC/futex, clocks and primitive device resource ownership.

Move behind restartable services over time: device matching/driver policy, full filesystem namespace, package/application lifecycle, network configuration, audio mixing, clipboard, desktop, format decoding and non-boot-critical drivers. Use Mollen as the boundary reference but close its missing ownership/revocation semantics.

### 3. Build zlIDL once, use it everywhere

Define typed interfaces for process, filesystem, device, storage, USB, network, audio, input, window, clipboard, package and session services. Generate zl client/server stubs, validators, trace format, protocol version checks and conformance tests.

Every method must declare:

- request/response bounds and handle rights;
- synchronous deadline or asynchronous completion/cancellation;
- backpressure/capacity owner;
- disconnect/restart behavior;
- idempotency and replay rules;
- capability required and authority returned.

This directly avoids Lemon's queue ambiguity and duckOS's incomplete message-size validation.

### 4. Add monotonic capabilities and real sessions

Implement Serenity-style authority reduction as a general handle/capability system, not string promises only. Programs start with explicit handles and can drop but not reacquire rights. Enforce at VFS, process-control, device, network and IPC boundaries. Exec receives a manifest-derived baseline; fork inherits the reduced set.

Before calling the desktop multi-user, require password/credential verification, session identity, credential-aware `kill`/debug/device operations, audited elevation and no username-only shortcut. banan and duckOS show the failure mode.

### 5. Harden existing hardware before broadening it

For PCI/BGA/Intel display/xHCI/NVMe already present in zlOS, add lifecycle state machines, ownership, bounded waits, reset, teardown and forced-error tests. Build deterministic simulators for rings/queues/descriptors and host-test them with sanitizer/fuzz variants. Then add AHCI/VirtIO/E1000/HDA using the same contracts, not one-off code shapes.

Promotion gate per driver:

1. parser/register unit tests;
2. queue/ring model test;
3. QEMU enumerate and I/O;
4. injected timeout/reset/error;
5. repeated init/teardown;
6. hardware receipt;
7. concurrency/unplug where hardware permits.

### 6. Make zlfs the durable system spine

Add integrity checks, explicit flush/barrier semantics, crash-consistent metadata updates, corrupt-image tests, disk-full behavior and power-cut simulation. Expose a userspace filesystem provider contract even if zlfs remains in kernel initially, so it can migrate without changing applications.

Package the base system and apps as content-addressed read-only images inspired by Mollen VaFS, with explicit writable user/data/cache/state mounts. Sign/verify manifests. Isolation still comes from capabilities/namespaces, not read-only bytes alone.

### 7. Build the desktop as cooperating services

Use WindowServer with server-owned metadata, SHM double/triple buffers, damage/occlusion, cursor and display timing. Add AudioServer with bounded per-client rings and negotiated formats; SessionServer for login/launch; ClipboardServer; NotificationServer; FileOperationServer; isolated Image/Font/Archive decoders.

Start with software composition but make the buffer/fence protocol GPU-ready. Do not couple apps to a particular framebuffer or compositor implementation. Reuse existing zlOS visual/apps work through the new protocol boundary instead of rewriting appearances first.

### 8. Complete POSIX/self-hosting deliberately

Track syscall/libc behavior as conformance items, including error semantics—not a syscall count. Use differential tests against Linux for pure userspace semantics, then run the same binaries/backends on zlOS. Add ELF dynamic linking, threads/signals/job control, PTY/TTY, sockets, mmap/COW and toolchain needs in dependency order.

Port recipes should follow banan's pinned/checksummed/patch-hashed design. A port becomes “available” only after install, launch/smoke and artifact receipts. Self-hosting means the compiler/build tools rebuild a matching artifact inside zlOS, not merely run from an imported image.

### 9. Create the verification pyramid

- **Host:** zlfs structures, allocators, protocol codecs, network state machines, scheduler policy, compositor geometry, format parsers; ASan/UBSan/TSan/fuzz as applicable.
- **Kernel unit image:** PMM/VMM/page faults, handles/capabilities, syscalls, IPC/futex and driver queue simulators.
- **QEMU boot:** exact image boots, services become ready, persistent save survives cold reboot, network/audio/input/display smoke tests, failures propagate to CI.
- **Adversarial QEMU:** low memory/disk, corrupt filesystem, packet loss/reorder, device reset/removal, service crash/restart, malformed IPC.
- **Hardware:** each real controller/display/storage/input path gets a versioned receipt and known limitations.
- **Independent review:** a separate checker consumes receipts and tries to refute completion before the ledger can promote state.

### 10. Sequence without losing the complete target

1. Evidence ledger + boot DAG + failure-propagating gates.
2. zlfs crash consistency and existing driver lifecycle hardening.
3. handles/capabilities + zlIDL + service supervision.
4. process/ELF/libc/PTY/signal completeness sufficient for serious userspace.
5. network stack plus deterministic tests; E1000/VirtIO and service split.
6. desktop/audio/session/parser services on typed IPC.
7. immutable package/app images and reproducible port receipts.
8. self-hosted zl toolchain and application ecosystem.
9. secondary architecture parity only through the same evidence gates.

The end state is not “a small hobby desktop.” It is a self-hosting zl-owned system whose claims are stronger than the reference projects because every feature is tied to source, build, runtime, failure and hardware evidence.

## License and code-use boundary

This report is for **ideas and architecture only**, as requested. Do not copy implementation code.

- MIT: Brook, Astral.
- BSD-2-Clause: banan-os, LemonOS, SerenityOS core.
- GPLv3/GPLv3-or-later: MollenOS, hhuOS, duckOS.
- Vendored ports/submodules keep their own licenses regardless of the repository's root license.

Even permissively licensed code should not enter zlOS accidentally. Preserve a source-provenance record for every imported specification, algorithm, asset and dependency. For GPL references, implement from public specifications and independently written design notes/tests, not by transliterating source.

## Audit limitations and weakest evidence

1. No checkout was locally built or booted; there is no `[L]` evidence.
2. CI configuration was inspected, but current remote run results were not fetched.
3. Very large projects, especially SerenityOS, were audited by central paths and complete build/subsystem inventories rather than line-by-line semantic proof of every application.
4. Port recipes and vendored dependencies were classified, not rebuilt.
5. Source-reviewed defect suspicions—especially Lemon's endpoint semaphore accounting—remain hypotheses until reproduced.

These limits do not weaken the main conclusion: source existence, build reachability, test presence and demonstrated runtime behavior must remain separate states in zlOS.

## Repository audit: Meulengracht/MollenOS

### Position and build truth

MollenOS/Vali 0.8.0 is an i386/amd64 OS (`README.md:9-27`, `VERSION`). The root graph only permits those architectures and exposes SMP, signals, drivers and EHCI options (`CMakeLists.txt:54-88`), then adds boot, protocols, runtime, kernel, services, modules, tools and apps (`:146-173`). The image is a VaFS initrd with disk/VMDK targets (`:188-209`). Full GUI/userspace is imported from a separate `vali-userspace` build (`BUILDING.md:63-75`), so it is not audited as native content here. **[C,S,B]**

Docker builds cover i386/amd64 (`.github/workflows/build.yml:10-36`); CodeQL performs real CMake builds (`.github/workflows/codeql-analysis.yml:20-54`); unit workflow configures test mode and runs the test script (`.github/workflows/test.yml:14-23`). QEMU explicitly uses `-net none` (`CMakeLists.txt:222-230`).

### Meaningful features and how they work

- **Mechanism/policy split [S,B].** The kernel contains machine/interrupt/handle/IPC/memory/synchronization/thread/ACPI/timer machinery (`kernel/CMakeLists.txt:41-53`), while deviced, filed, processd, served, sessiond, usbd and netd run as services (`services/CMakeLists.txt:28-36`). `syslogd` exists but is commented out (`:34`).
- **Kernel machine and scheduling [S,B].** `kernel/machine.c:94,163,177` initializes futex/platform interrupts/HPET. Scheduler and thread lifecycle are at `kernel/components/scheduler.c:532-539` and `kernel/threads/threads.c:204,239,342,416,477,790`.
- **VM/futex/IPC [S,B,T].** VM map/commit/clone live at `kernel/memory/ms_map.c:306,424,508`; shared memory in `ms_shm.c`. A hashed futex wait/wake design is at `kernel/sync/futex.c:84-146,243-410`. The syscall table exposes threads, futex, multi-message IPC and SHM (`kernel/api/entry.c:156-212`); stream IPC begins at `kernel/ipc.c:107`.
- **Protocol-first services [S,B].** `.gr` schemas define device, file, mount, socket, process, library, package, session, storage, syslog and USB interfaces (`protocols/CMakeLists.txt:1-72`), plus provider contracts for drivers, filesystems, input, storage and USB host/hub. libgracht generates C client/server stubs. The schema, not hand-written calls, becomes the interface source of truth.
- **Device manager [S,B].** `services/deviced/` matches PCI/bus devices through YAML descriptors and launches providers.
- **Userspace VFS [S,B].** filed handles open/read/write/seek/stat/link/move/mount, GPT/MBR and storage. Central path lookup/load/child/handle operations are at `services/filed/vfs/utils.c:194,226,257,429,667`.
- **Process loader [S,B].** processd, built as `phoenix` (`services/processd/CMakeLists.txt:25-41`), uses PE/COFF rather than ELF. `services/processd/pe/loader.c:54,117,178,241` loads/unloads images and resolves libraries/exports with imports, relocations, cache and verification support.
- **Package/application service [S,B].** served installs packages, mounts app images and exposes commands. `services/served/server/application.c:272-327` mounts VaFS read-only and creates symlinks.
- **USB/network policy services [S,B].** usbd separates USB policy from HCI modules. netd has a meaningful local-socket implementation (`services/netd/domains/local.c:143-345,560-724`) but Internet and Bluetooth domains return unsupported (`domains/internet.c:32-38`).
- **Read-only image model [C,S,B].** The VaFS module implements open/read and returns `ENOTSUPPORTED` for create/unlink/write (`modules/filesystems/valifs/main.c:65-71,195-257,319-330,427-498`). Root images are packed at `CMakeLists.txt:188-190`; applications mount VaFS with `MOUNT_FLAG_READ` (`application.c:272-327`). The GPT model still has a writable data partition (`cmake/models/vali-gpt.yaml:1-44`).
- **Drivers [S,B].** BIOS/UEFI boot (`boot/CMakeLists.txt:15-42`, `boot/uefi/CMakeLists.txt:240-247`), PS/2, HID, EHCI/OHCI/UHCI, USB hub/mass storage, AHCI, MFS and VaFS are active through `modules/CMakeLists.txt:22-27` and nested graphs.

### Reachability, tests and incomplete promises

- Ethernet/E1000 source exists under `modules/eth/` but root modules never add it. **[S], not [B].** xHCI source exists under `modules/serial/usb/xhci`, but `modules/serial/CMakeLists.txt:1-6` omits it. **[S], not [B].** No NVMe implementation was found; ARM directories are placeholders.
- Application namespaces are TODO/no-op (`services/served/server/application.c:229-234`); process tracking/kill is TODO/no-op (`:329-336`); logging, namespaces and ownership remain unfinished (`:497-520`). Thus immutable storage images are real, but strong sandboxed application lifecycle is not.
- Filed scopes are TODO/no-op (`services/filed/scope.c:119`, `vfs/vfs.c:217`); unmount/mount tracking is unfinished (`vfs/vfs.c:205`). served lacks an install security check (`services/served/server/install.c:151`).
- Test mode adds kernel tests but comments out services (`CMakeLists.txt:3-44`). Kernel memory tests are substantial (`kernel/memory/CMakeLists.txt:1-13`) and CI invokes the runner. filed has a test (`services/filed/vfs/utils_test.c`) but it is unreachable in root test mode.
- `testing/run-tests.sh:1-11` loops through `*_test` without `set -e` or accumulated failure status. An early failure followed by a passing test can yield a successful script. **[T,CI] with a false-green runner risk.**
- Snapshot age matters: this checkout's head is from 2023-05-19, so current-hardware relevance is the weakest in the set.

### No-code-copy lessons

- **Best idea:** smallest policy surface in kernel; generated service/provider contracts; read-only app images with one explicit writable location.
- **Tradeoff:** moving VFS/process/device policy out of kernel creates restartable boundaries but makes lifecycle, handle revocation and namespace ownership mandatory. Mollen has not completed those pieces.
- **zlOS action:** use protocol schemas as source of truth and generate both sides plus conformance tests. Keep ELF unless PE compatibility is an explicit product goal. Do not claim isolation until ownership, revoke/kill/restart and mount scope enforcement are test-gated.

---

## Repository audit: hhuOS/hhuOS

### Position and build truth

hhuOS explicitly describes itself as an educational C++/assembly x86 OS and **not** a daily-use operating system (`README.md:13-17`). That is an honest scope boundary. CI builds Towboot, Limine, Limine-VDD and GRUB images (`.github/workflows/build.yml:19-29`) and packages third-party licenses (`:31-57`); it does not run tests. Kernel, device, filesystem and application CMake graphs enumerate their subcomponents (`cmake/kernel/CMakeLists.txt:24-34`, `cmake/device/CMakeLists.txt:24-37`, `cmake/filesystem/CMakeLists.txt:24-37`, `cmake/applications/CMakeLists.txt:34-79`). **[C,S,B,CI-build]**

### Meaningful features and how they work

- **Readable boot orchestration [S,B].** `src/GatesOfHell.cpp` is the central composition root. It initializes IDE/AHCI (`:631-632`), registers FAT/ISO (`:639-645`), mounts root (`:651-665`), mounts memory-backed `/device`, `/temp` and process FS (`:667-677`), adds null/zero/random/mount/log/status nodes (`:679-684`), exposes QEMU firmware/ACPI/SMBIOS (`:686-720`), configures PS/2 (`:762-779`), builds network services and NICs (`:781-799`), initializes sound (`:801-809`), applies a mount table (`:811-842`) and launches shell (`:844-845`).
- **Processes/scheduling [S,B].** `src/kernel/process/Scheduler.cpp:43,86,99,147,180,202` constructs/starts the scheduler, marks ready, kills, yields and switches. `BinaryLoader.cpp:40,43,99` loads and creates a user thread from an executable.
- **Virtual memory [S,B].** `src/kernel/memory/VirtualAddressSpace.cpp:74,96,139` translates, maps and unmaps pages. A duplicate-program problem remains at `:174`.
- **Filesystems/storage [S,B].** The OS has its own VFS plus FAT, ISO, memory and process virtual filesystems. IDE, AHCI, floppy and virtual disks are active; ISO mount begins at `src/filesystem/iso/IsoDriver.cpp:34`. FatFs is a pinned third-party submodule and must not be described as wholly native.
- **Networking [S,B].** `src/kernel/network/NetworkStack.cpp:31-54` composes Ethernet, ARP, IPv4, ICMP and UDP. `UdpModule.cpp:49,71,101,134,162` registers ports, reads, writes and calculates checksums. RTL8139, NE2000 and loopback are active. There is no TCP implementation in the tree.
- **Hardware/UI [S,B].** PCI/ISA, SMP, APIC/IOAPIC/PIC, VBE/linear framebuffer, PS/2, SoundBlaster/PC speaker, HPET/APIC/PIT/RTC, ACPI/APM/SMBIOS and BIOS services are present. Applications render through framebuffer/game libraries rather than a general desktop/window compositor.
- **Applications [S,B].** Shell/utilities, games/emulators and the Pulsar 2D/3D/audio engine are active. Doom/Quake ports and ClassiCube are documented (`README.md:148-161`), with ClassiCube networking explicitly unsupported. TinyGL/PortableGL and many games/emulators are pinned submodules, not native kernel features.

### Verification and limits

- The only project test application is `src/application/ctest`; its README explicitly calls it simple libc verification rather than a framework (`src/application/ctest/README.md:3-4`). Macros are defined at `ctest.c:33-74` and individual checks begin at `:101`. **[T]**, but no CI test step.
- Vendored Peanut-GB/PortableGL tests do not test hhuOS kernel behavior.
- The OS is effectively 32-bit/i386. Executable entry is cast through 32-bit state (`src/kernel/process/BinaryLoader.cpp:99`), and architecture/memory interfaces are x86-32 oriented.
- No TCP, desktop compositor, modern USB stack or focused kernel failure-path suite was found.
- LibC contains hard stubs (`src/lib/libc/stdlib.cpp:99`), timezone gaps (`time.cpp:77,92`) and collation gaps (`string.cpp:175`). Ethernet checksum work remains TODO (`src/kernel/network/ethernet/EthernetModule.cpp:49,83`).

### No-code-copy lessons

- **Best idea:** a single readable boot composition root that makes service/device order reviewable; an explicit educational scope statement.
- **Tradeoff:** central orchestration is easy to understand but grows into a large failure domain. zlOS should keep the readable dependency view while generating/executing smaller independently testable init nodes.
- **zlOS action:** use hhuOS for teaching/documentation patterns, not as the networking, security or 64-bit architecture donor.

---

## Repository audit: byteduck/duckOS

### Position and build truth

duckOS is an x86 GUI OS with WIP AArch64 (`README.md:2`) and is explicitly on hold (`README.md:10-12`). It claims a window manager/compositor, TCP/UDP/IP/UNIX networking, E1000, AC97, ports, dynamic linking, ptrace/profiler and software 3D (`README.md:22-31`). The root graph builds static/shared libraries, services, programs/apps, kernel and images (`CMakeLists.txt:25-96`). Kernel source lists cover core, i686 and AArch64 paths (`kernel/CMakeLists.txt:26-267`). CI builds an i686 release image and uploads it (`.github/workflows/build-os.yml:1-66`) but runs no tests. **[C,S,B,CI-build]**

### Meaningful features and how they work

- **Boot and filesystem composition [S,B].** `kernel/kmain.cpp:85` starts task management, probes ext2 (`:126`), mounts proc/socket/PTY filesystems (`:158-193`) and execs init (`:214-216`). `kernel/filesystem/VFS.cpp:44,62,134,192,213` owns root, resolution, open, create and unlink.
- **Virtual memory [S,B,T].** `kernel/memory/VMSpace.cpp:146,168,190,204,218,223,279,314,366,449` covers unmap, region lookup/reservation, page fault, address search, allocation and free. Kernel tests generate randomized VM region operations (`kernel/tests/TestMemory.cpp:9-25`).
- **Shared-memory local IPC [S,B].** socketfs passes shared “shmallow” memory (`kernel/filesystem/socketfs/SocketFSInode.cpp:140,167`) and queues packet headers/blockers (`:288,300`).
- **Networking [C,S,B].** E1000 discovery starts at `kernel/net/NetworkAdapter.cpp:33`; `NetworkManager.cpp:88,154,173` dispatches ARP, UDP and TCP. `Router.cpp:9-14,28,112,122,158,176` maintains locked route tables and waiters. TCP has global socket maps (`TCPSocket.cpp:11-13`) and connection/data logic.
- **Dynamic linker [S,B].** `libraries/libexec/dlfunc.cpp:10,19` exposes `dlopen` and relocation; `Object.cpp:283,313,331,355` copies/performs relocations and resolves symbols; `Loader.cpp:21,70` exports loader entry points.
- **Pond window service [S,B].** `services/pond/Client.h:47-61` defines typed window RPCs; `Server.cpp:56,64,71` exposes a River bus server/endpoint. Window/display sources manage compositor state and surfaces. A client-size validation TODO remains at `services/pond/Client.cpp:147`.
- **Quack audio service [S,B].** `services/quack/SoundServer.cpp:36,45` accepts bus clients; `libraries/libsound/Connection.cpp:27` connects; `services/quack/Client.cpp:25` allocates a shared circular audio queue.
- **River RPC [S,B].** River serializes service messages, but message/function size validation remains TODO (`libraries/libriver/Function.hpp:109`, `libraries/libriver/Message.hpp:104`).
- **Userspace [S,B].** Init, Pond, Quack and DHCP are separate services (`README.md:47-55`); desktop apps, CLI tools, GUI/media/graphics/network libraries and source-built ports are enumerated at `README.md:56-117` and corresponding CMake graphs.

### Verification and serious gaps

- Kernel tests use a registry/runner (`kernel/tests/KernelTest.cpp:20-41`) and cover randomized VM regions, map containers and smart pointers (`kernel/tests/TestMemory.cpp:9-25`, `kernel/tests/TestMap.cpp:28`, `kernel/tests/TestArc.cpp:26`). **[T]** Root CMake has a QEMU tests target (`CMakeLists.txt:78-96`), but CI does not invoke it.
- `SafePointer` has TODO failure semantics in core usercopy paths (`kernel/memory/SafePointer.h:26,37,48,57`). This is a security/correctness blocker, not polish.
- `kill` lacks a permission check (`kernel/syscall/kill.cpp:8`); `SIG_IGN` is unimplemented (`kernel/syscall/sigaction.cpp:27`). `sendmsg` supports only one iovec and no control messages (`kernel/syscall/socket.cpp:64,73,84,93`).
- TCP lacks robust timeout, retransmission and window scaling (`kernel/net/TCPSocket.cpp:128,199,418,435`).
- Filesystem page-cache synchronization is absent (`kernel/memory/InodeVMObject.h:33`); ext2 lacks complete symlink/triple-indirect/time support (`kernel/filesystem/ext2/Ext2Inode.cpp:138,618,802`, `Ext2Filesystem.cpp:148`).
- LibC contains many pthread/fcntl/unistd/dirent/utime/stdio stubs, including `libraries/libc/fcntl.c:44`, `unistd.c:304`, `dirent.c:97`, `utime.c:7`, `stdio.c:425`.
- AArch64 is only stage-one/WIP; SMP is a future item (`README.md:33-45`). The future list also admits missing IPC redesign, filesystem cache, tests/docs, self-hosting, stability and a planned microkernel transition.

### No-code-copy lessons

- **Best ideas:** small service bus for window/audio services, SHM circular audio queue, dynamic linker separation, honest “on hold/future” documentation.
- **Do not inherit:** RPC without strict length/handle validation, unchecked user pointers, global TCP maps, or a planned architecture transition without a compatibility boundary.
- **zlOS action:** validate every generated message and user pointer at one boundary; add credential checks to process control; make network retransmit/timer behavior deterministic in a host simulator before relying on live QEMU tests.

---

## Repository audit: Bananymous/banan-os

### Position and build truth

banan-os is a hobby UNIX-like C++ OS for x86_64 and i686 (`README.md:6-8`). The root graph includes bootloader, kernel, the BAN support library and userspace (`CMakeLists.txt:29-60`); architecture selection is explicit (`kernel/CMakeLists.txt:135-161`). Its kernel graph enumerates ACPI, audio, devices, filesystems, interrupts, memory, networking, processes, storage, terminal and USB (`kernel/CMakeLists.txt:1-127`). Freestanding flags and the linked kernel are at `kernel/CMakeLists.txt:206-243`; UBSAN is intentionally disabled at `:129-133`. **[C,S,B]**

The build/image runner supports QEMU and Bochs (`script/build.sh:1-135`). There is no CI. `.pre-commit-config.yaml:1-8` only enforces whitespace/end-of-file hygiene.

### Meaningful features and how they work

- **Boot composition [S,B].** `kernel/kernel/kernel.cpp:120-272` sequences memory/page tables, ACPI, interrupts, timers, devfs, SMP, procfs, shared memory, RNG, scheduler, PCI/USB/device discovery, networking, VFS, TTY, driver images and finally `/usr/bin/init`.
- **ACPI/AML [S,B].** `kernel/kernel/ACPI/ACPI.cpp:201,313,600,784,1116` covers initialization, AML table loading, embedded controller and namespace devices; battery and EC drivers begin at `ACPI/Battery.cpp:97` and `EmbeddedController.cpp:31`.
- **SMP and interrupts [S,B].** APIC creation/multiprocessor initialization is at `kernel/kernel/APIC.cpp:150,250`; processor creation, SMP initialization and inter-CPU message handling are at `Processor.cpp:56,174,442`.
- **Process and VM [C,S,B].** Fork and exec are implemented at `kernel/kernel/Process.cpp:807,911`; address-space, file-backed, memory-backed and shared VM objects are separate. The README accurately distinguishes file-mapping COW from missing anonymous COW (`README.md:19-20`).
- **Filesystems [C,S,B].** VFS mount handling is at `kernel/kernel/FS/VirtualFileSystem.cpp:167-206`; ext2, FAT, devfs, procfs and temporary/RAM filesystems are active (`kernel/CMakeLists.txt:28-42`).
- **Storage [C,S,B].** NVMe initializes controller/admin/I/O queues (`kernel/kernel/Storage/NVMe/Controller.cpp:19-39,219-248`); AHCI executes commands and read/write paths (`Storage/ATA/AHCI/Device.cpp:58-267`); storage devices discover partitions (`Storage/StorageDevice.cpp:146`).
- **Networking [C,S,B,T].** E1000/E1000E, RTL8169-family, ARP, ICMP, IPv4, UDP, TCP and UNIX sockets are active (`kernel/CMakeLists.txt:65-77`). IPv4 receive dispatch is at `Networking/IPv4Layer.cpp:241`. The README labels TCP partial/buggy and SSL absent (`README.md:37-48`) instead of overstating maturity.
- **USB [C,S,B].** xHCI, HID, mass storage and hubs are active (`kernel/CMakeLists.txt:113-124`). `kernel/kernel/USB/XHCI/Controller.cpp:72-554` covers controller setup, ports, interrupter, scratchpads, devices, commands and IRQs. EHCI/OHCI/UHCI and VirtIO are explicitly absent (`README.md:31-35`).
- **Input, terminal and graphics [S,B].** Framebuffer, serial, text terminal, PTY/TTY and PS/2 keyboard/mouse are active (`kernel/CMakeLists.txt:45-50,100-106`); PS/2 initialization is at `Input/PS2/Controller.cpp:224,271`.
- **Audio [S,B].** AC97 and HDA are both in the kernel source graph (`kernel/CMakeLists.txt:10-14`).
- **Loadable drivers [S,B].** `kernel/kernel/Banos.cpp:60-195` validates ELF, loads/relocates sections and resolves kernel symbols; initial driver images load at `kernel/kernel/kernel.cpp:267`. Unload is missing (`Banos.cpp:42`) and only x86-64 relocation kinds are handled (`:170-187`).
- **Userspace [S,B].** All 62 program directories are pulled into the build (`userspace/programs/CMakeLists.txt:1-73`), including servers, desktop apps, shell/utilities, networking and driver tools. Audio, C, Clipboard, DEFLATE, ELF, Font, GUI, Image, Input and QR libraries are active (`userspace/libraries/CMakeLists.txt:1-16`).
- **Damage-driven GUI [S,B].** WindowServer establishes framebuffer/fonts and creates/resizes/invalidates SHM-backed windows (`userspace/programs/WindowServer/WindowServer.cpp:18,86-146`); LibGUI maps buffers and submits invalidations (`userspace/libraries/LibGUI/Window.cpp:24,157`). This is a real server/compositor boundary, not application-local framebuffer drawing.
- **Dynamic linking [S,B].** `userspace/programs/DynamicLoader/main.cpp:23-220` implements `_start`, ELF checks, lazy-binding trampoline, loaded-object metadata and relocation.
- **Service boot [S,B].** Init starts DHCP, resolver, audio and clipboard services (`userspace/programs/init/main.cpp:20-55`).
- **Reproducible ports [S,B].** The ports framework records dependency recursion, pinned Git commits, archive SHA-256, patches and source/build hashes (`ports/README.md:1-12`, `ports/install.sh:143-258`).

### Verification and security truth

Sixteen userspace test targets cover fork, framebuffer, globals, joystick, mmap/SHM, mouse, popen, pthread, setjmp, sorting, TCP/TLS/UDP/UNIX sockets and windows (`userspace/tests/CMakeLists.txt:1-27`). **[T]** There is no automated runner or CI gate, so they prove test sources/build reachability, not execution.

The ports tree includes X11/GTK/Mesa, SDL, LLVM/GCC, Python, Git, curl/OpenSSL/OpenSSH and games. These are recipes, not current runtime evidence.

Critical gaps:

- Init has no password authentication: the first session silently selects a user; later sessions request only a username before `setgid`/`setuid` (`userspace/programs/init/main.cpp:85-128`).
- `sudo` simply requests uid/gid 0 (`userspace/programs/sudo/main.cpp:11-27`). Actual exploitability depends on unverified install permissions and syscall policy, but the source model is not acceptable for zlOS.
- LibC includes hard stubs/assertions such as `fdatasync`, `lockf` and `crypt` (`userspace/libraries/LibC/unistd.cpp:653,700,715`).
- Driver unload is absent and ELF machine/type validation has TODOs (`kernel/kernel/Banos.cpp:42,74-78`). AHCI has a transfer limitation (`Storage/ATA/AHCI/Device.cpp:333`); xHCI streams/SuperSpeed and hub races remain unfinished.

### No-code-copy lessons

- **Best ideas:** candid feature maturity list, reproducible/pinned port recipes, explicit boot dependency order, SHM/damage GUI, small dynamic loader.
- **Do not inherit:** username-only login or implicit privilege elevation. Security state must precede desktop polish.
- **zlOS action:** make the feature ledger mechanically compare docs to build graph and tests. Model ports as `(source digest, patches, dependency digests, build receipt, runtime receipt)`.

---

## Repository audit: LemonOSProject/LemonOS

### Position and build truth

LemonOS is a UNIX-like 64-bit C++ OS (`README.md:5-9`); x86_64 is the only accepted kernel architecture (`Kernel/CMakeLists.txt:12,24-35`). Its active graph covers kernel, services, apps and libraries (`Kernel/CMakeLists.txt:52-157` and root/application/system CMake files). xbstrap/container/toolchain/image instructions are documented at `Documentation/Build/Building-Lemon-OS.md:8-87`. CI builds toolchain, base, system and disk artifacts (`.github/workflows/ci.yml:16-136`) but does not boot or execute OS tests. **[C,S,B,CI-build]**

### Meaningful features and how they work

- **Boot composition [S,B].** `Kernel/src/Kernel.cpp:45-145` initializes storage/USB, service filesystem, network/audio, configured modules, mounts, optional tests, PTY and init ELF; `:160-235` covers VFS/devices/splash/PS2/syscalls/scheduler.
- **x86-64 machine layer [S,B].** ACPI, APIC, CPU, ELF, GDT, IDT, PCI, PS/2, paging, PMM, SMP, scheduler, syscall, timers and TSS are enumerated at `Kernel/CMakeLists.txt:117-150`.
- **VM/process [S,B].** `Kernel/src/MM/AddressSpace.cpp:111-176` handles VMO mapping, anonymous memory and fork; process and syscall implementations are active.
- **Filesystems [S,B].** VFS, volume management, tmp/tar/pipe and FAT32 are in-kernel; ext2 is a deployed module. Ext2 implements mount, inode/block/directory operations and create/read/write paths (`Kernel/Modules/Ext2/Main.cpp:51,431-545,896,1207-1446`). FAT32 is explicitly read-only (`Kernel/src/Fs/Fat32.cpp:164-166`).
- **Storage [S,B].** AHCI, ATA, NVMe and GPT sources are active (`Kernel/CMakeLists.txt:101-109`).
- **Networking [S,B].** Adapters/interfaces, IP, UDP, TCP and local sockets are active. TCP accept/connect/listen/recv/send are implemented at `Kernel/src/Net/TCP.cpp:592-739`; local socket pair/connect/accept at `Kernel/src/Net/Socket.cpp:171-237`.
- **Modular drivers [S,B].** Ext2, PCAudio, E1000 and a test module are built (`Kernel/Modules/CMakeLists.txt:1-17`); ext2/PCAudio/E1000 are deployed by `Kernel/modules.cfg:1-3`.
- **Typed IPC [C,S,B].** The design is Service → Interface → paired message endpoints (`Documentation/System/IPC/README.md:3-14`), with sync/async IDL semantics (`:20-45`). Kernel queues and calls live in `Kernel/src/Objects/Message.cpp:6-178`; registry and endpoint pairing in `Service.cpp:16-99` and `Interface.cpp:28-69`; client poll integration in `LibLemon/src/IPC/interface.cpp:25-108`. `InterfaceCompiler/main.cpp:392-537` generates synchronous/asynchronous client/server plumbing.
- **Compositor [S,B].** LemonWM's run loop starts at `System/LemonWM/WM.cpp:14`. Windows are SHM double-buffered (`System/LemonWM/Window.cpp:189-199`, client mapping at `LibGUI/Window.cpp:40,85`). `System/LemonWM/Compositor.cpp:12-255` tracks damage, clips occluded regions, composes cursor/wallpaper and then blits the final framebuffer. The damage design is good; the final full-screen copy is a performance tradeoff.

### Tests, disabled features and defects

- The kernel test module runs only when `HAL::runTests` is enabled (`Kernel/src/Kernel.cpp:93-98`). String tests log warnings but return success (`Kernel/Modules/TestModule/StringTest.cpp:5-32`); the threading test body is commented and returns success (`ThreadingTest.cpp:12-34`).
- The userspace test app is built and installed (`Applications/CMakeLists.txt:80-82,118,132-155`) and contains pipe/terminal/audio/syscall tests (`Applications/Tests/Main.cpp:13-44`), but main prints failures and always returns 0.
- CI does not run either suite. Thus `[T]` exists, but failure is not gated and no test earns `[CI]`.
- xHCI is source/build-reachable, but `Initialize` returns before enumeration and the functional body is commented (`Kernel/src/USB/XHCI.cpp:21-35`). **[S,B] but disabled**, accurately labeled WIP at `README.md:40-42`.
- HDA is built/deployed, but audio channel/sample writing returns `ENOSYS` and volume is a no-op (`Kernel/Modules/PCAudio/HDAudio.cpp:10-16,42-46`). **[S,B] but stubbed.**
- `MessageEndpoint::Write` waits on the sender's availability semaphore then queues into the peer (`Kernel/src/Objects/Message.cpp:146-165`), while `Read` signals the receiver's own semaphore (`:61`). If endpoints own distinct semaphores, sustained one-way traffic can permanently exhaust sender capacity. This is a strong source-reviewed suspicion, not a reproduced bug.
- Synchronous calls need timeouts (`Message.cpp:70-109`); interface connect busy-yields (`Interface.cpp:65-67`); peer lifetime races need hardening.

### No-code-copy lessons

- **Best ideas:** IDL-generated typed IPC, explicit service registry, deploy-time module list, SHM/damage/occlusion compositor.
- **Do not inherit:** ambiguous queue-capacity ownership, unbounded calls, busy-wait lifecycle, tests whose process cannot fail, or compiled stubs counted as features.
- **zlOS action:** define each generated endpoint with capacity owner, backpressure, cancellation/deadline, disconnect semantics and handle-transfer rules. Make generated conformance tests fail the build.

---

## Repository audit: SerenityOS/serenity

### Position and build truth

SerenityOS supports x86-64, AArch64 and RISC-V (`README.md:3`) and claims security sandboxing, services, POSIX-like APIs/filesystems, networking, extensive tools/libraries/apps and 300+ ports (`README.md:27-45`). The root CMake graph pins exact GCC/LLVM expectations (`CMakeLists.txt:20-39`), generates code (`:63-64`), and defines QEMU/UEFI/GRUB/Limine/extlinux/RPi run and image targets (`:66-123`). Kernel reachability includes xHCI (`Kernel/CMakeLists.txt:35-39`), HDA (`:58-62`), AHCI/NVMe (`:118-127`), ext2/FAT (`:149-154`), MM (`:260`), E1000/Realtek (`:289-290`), pledge/unveil (`:343,375`) and process/scheduler (`:394,397`). **[C,S,B]**

### Whole-system feature map

- **Kernel/platform [S,B].** Multiarch boot plus I2C, PCI, SerialIO, EHCI/UHCI/xHCI/VirtIO; AHCI/NVMe/SD/USB/VirtIO storage; Bochs/Intel/VMware/VirtIO graphics; AC97/HDA; PS/2 and USB input; DevLoopFS/DevPtsFS/ext2/FAT/FUSE/ISO9660/Plan9/proc/RAM/sys filesystems; Cadence/IP/Intel/Realtek/VirtIO networking; process, signals, scheduler, namespaces/custody, promises/veil and profiling.
- **Userspace services [S,B].** Audio, clipboard, config, crash, DHCP, file operations, filesystem access, image decoding, launch/login/lookup, network, notifications, request brokering, SQL, SSH, system/session management, taskbar, WebContent/WebDriver/WebServer/WebWorker and WindowServer are separate processes.
- **Libraries [S,B].** AK and LibC sit below Core/GUI/Gfx, Web/JS/Wasm, TLS/HTTP/DNS/IMAP/SSH, PDF/media/image/audio, GL/SoftGPU/GPU, SQL/regex/Unicode/XML and numerous format libraries.
- **Applications [S,B].** Browser, file manager, IDE/editor/terminal/settings, media players/editors, mail/maps/PDF, PixelPaint, spreadsheet, utilities, games and demos are part of the repository graph. Existence/build reachability is not equivalent to every app passing an end-to-end test.

### How the strongest mechanisms work

- **Page faults become explicit VM or signal outcomes [S,B].** `Kernel/Arch/PageFault.cpp:17` enters the handler; `:59-137` maps VM responses to recovery, SIGBUS/SIGSEGV, coredump or kernel crash.
- **Monotonic capabilities [S,B,T].** `Kernel/Syscalls/pledge.cpp:62` forbids adding promises and `:83` installs them. `Kernel/Syscalls/unveil.cpp:29,44,78,84` updates a path tree, prevents permission expansion, exposes the syscall and locks the policy. VFS traverses and enforces the unveil tree at `Kernel/FileSystem/VirtualFileSystem.cpp:927-1013`. Exec carries/reduces promises and veil state (`Kernel/Syscalls/execve.cpp:544-560,644`); fork copies it (`fork.cpp:38-64`). This is much stronger than checking permissions only in individual apps.
- **Multi-process browser containment [C,S,B].** The browser assigns a WebContent process per tab, separates requests and image decoding, and applies pledge/unveil plus separate users (`Documentation/Browser/ProcessArchitecture.md:3-29`). SystemServer sockets spawn fresh services (`:31-38`), and typed IPC connects OutOfProcessWebView/WebContent/PageHost (`:40-50`). The document explicitly says parts are aspirational (`:3`), so individual claims still need source/runtime proof.
- **Request brokering [S,B].** `Userland/Services/RequestServer/ConnectionFromClient.cpp:53` establishes typed IPC; `:176` transfers file descriptors; `:219` handles new clients; `:341,355` chooses protocols/WebSocket. IPC encoding/decoding and descriptor transport live in `Userland/Libraries/LibIPC/Encoder.h`, `Decoder.h` and `Message.cpp:89-104`.
- **Web workers [S,B].** `Userland/Services/WebWorker/DedicatedWorkerHost.cpp:36,52,95,145,162,224,232` constructs the worker realm, fetches scripts, completes loading, connects message ports and handles classic/module workers.
- **Compositor [S,B].** WindowServer uses a 60 Hz display/compose timer (`Userland/Services/WindowServer/Compositor.cpp:49-70`), wraps front/back/temp buffers per screen (`:92-131`) and invalidates/occlusion-composes from `:133-180` onward. This centralizes damage, cursor, wallpaper, animation and buffer flipping.
- **Audio server [S,B].** A dedicated mixer thread starts at `Userland/Services/AudioServer/Mixer.cpp:23-37`; clients get queues (`:39-50`); the loop waits for work, mixes/resamples with per-stream/master volume, clips and writes a hardware buffer (`:53-120`).

### Verification quality

The test graph spans major libraries, kernel and utilities (`Tests/CMakeLists.txt:1-42`). Host tests run via CTest; ASan/UBSan variants are documented (`Documentation/RunningTests.md:24-59`). Target tests and CI execution are described at `:67-94`; boot self-test mode at `:98-120`. CI builds x86 with GCC/Clang, AArch64, RISC-V and all-debug configurations, then runs Lagom Linux/macOS fuzz and non-fuzz variants (`.github/workflows/ci.yml:1-62`). **[T,CI]** This is the best verification architecture in the set, though this audit did not inspect a current CI result or run it locally.

### Limits and truth boundaries

- No binary package manager and no stable ABI (`Documentation/FAQ.md:31-33`). Patent-covered codecs are excluded (`FAQ.md:47`).
- Bare-metal support is constrained to listed SATA/NVMe/USB paths, xHCI/UHCI, E1000/RTL8168 and AC97/HDA; there is no Wi-Fi or real GPU acceleration (`Documentation/BareMetalInstallation.md:5-14`). Console-only mode is documented at `:56`.
- RISC-V SMP is unsupported with TODO processor/page-directory operations (`Kernel/Arch/riscv64/Processor.h:36,48,57,96,183`; `PageDirectory.h:39,42,45`). RISC-V MMU code maps too broadly RWX (`MMU.cpp:149`). AArch64 MSI remains TODO (`Kernel/Arch/aarch64/PCIMSI.h:20-38`).
- Exec stack layout is not fully ABI-compliant (`Kernel/Syscalls/execve.cpp:168`). USB mass storage handles only the first LUN and limited UAS (`Kernel/Bus/USB/Drivers/MassStorage/MassStorageDriver.cpp:66,97-98`). VFS has unveil error-leak TODOs (`VirtualFileSystem.cpp:535,1029`).
- README's “no extra dependencies/built from scratch” phrasing (`README.md:43`) is marketing shorthand: builds require pinned host toolchains/Lagom, and ports are third-party software. Core OS ownership is extensive; the literal statement should not become a zlOS evidence standard.

### No-code-copy lessons

- **Best ideas:** monotonic, kernel-enforced process capabilities; typed service IPC; hostile-format decoding in disposable restricted processes; host and target tests; multiarch CI; centralized GUI/audio services.
- **Tradeoff:** Serenity's breadth depends on a very large custom library/application surface. Copy the boundaries and evidence gates, not the scope all at once.
- **zlOS action:** introduce capability reduction at exec, inherit it across fork, enforce it at the shared VFS/network/device boundaries, and isolate image/font/archive parsers. Build host libraries from the same source where possible, then boot-test the ABI-specific seam.

---

## Repository audit: IanNorris/Brook

### Position and architecture

Brook describes itself as a C++ x86-64 UEFI OS with SMP, a compositing Wayland desktop and roughly 40% of the Linux syscall surface (`README.md:21-25`). The claim is backed by a coherent build graph: bootloader and kernel are separate cross-toolchain external projects (`CMakeLists.txt:13-35`), while the kernel target includes the scheduler, processes, memory, VFS, ext2, networking/TCP, compositor, VirtIO block, SMP, profiler and watchdog (`src/kernel/CMakeLists.txt:148-215`). **[C,S,B]**

### Meaningful features and how they work

- **UEFI boot and handoff [S,B].** The loader locates ACPI (`src/bootloader/src/acpi.cpp:27`, `AcpiInit`), initializes GOP (`graphics.cpp:10`, `GraphicsInit`), reads files (`fs.cpp:14`, `ReadFile`), validates/loads ELF segments (`elf_loader.cpp:40-50`) and installs page tables (`paging.cpp:169`, `LoadCR3`). `main.cpp:26` makes GOP availability a hard boot requirement.
- **Processes, ELF and Linux-like syscalls [C,S,B].** `src/kernel/src/elf_loader.cpp:26,297` implements normal and fixed-address ELF loading. `src/kernel/src/syscall.cpp:12477-12535` constructs the protected syscall table and leaves unknown calls on an unimplemented default. Fork preserves the saved register frame at `syscall.cpp:4548-4575`. The implementation is broad, but gaps include missing descriptor lookup checks (`:9346,9378`), no timeout in one wait path (`:12455`) and deliberately absent inotify (`:12535`).
- **Virtual filesystems and writable storage [C,S,B,T].** `src/kernel/src/vfs.cpp:42` registers filesystems, `:307` initializes VFS, `:315` mounts, and `:471-598` implements read/write/unlink/mkdir/rename/symlink dispatch. Ext2, FAT and ramdisk sources are linked. The VFS test links the real VFS, FAT glue, device registry and memory managers (`src/tests/CMakeLists.txt:71-108`); host ext2 tests use a real image (`src/tests/host/CMakeLists.txt:121-128`). Ext2 writes still omit triply-indirect blocks (`src/kernel/src/ext2_vfs.cpp:836`).
- **Storage [C,S,B].** VirtIO block supports scatter/gather reads and writes (`src/kernel/src/virtio_blk.cpp:686,860`), ordinary block I/O (`:992,1232`) and multi-device discovery (`:1532`, `InitAll`). USB mass storage is integrated into xHCI through bulk transfers and block operations (`src/drivers/xhci/xhci_mod.cpp:2254,2347,2518,2574,2646`).
- **Networking [C,S,B,T].** `src/kernel/src/net.cpp:184,404,499,607` covers ARP, IPv4, UDP and receive dispatch; `:631,643` initializes and registers interfaces; `:801,988,1065` handles DHCP/static/discovery; `:1387` creates sockets. TCP has sequencing and out-of-order handling beginning around `:1941`. `test_tcp` links the real TCP source and host lock/futex/fd/loopback tests exist (`src/tests/host/CMakeLists.txt:47-89`). A fixed global socket pool remains a known limit (`net.cpp:93`).
- **SMP and pluggable scheduling [C,S,B,T].** The kernel includes SMP and scheduler policy sources; RR and MLFQ are loadable scheduler modules. Host tests build both as shared objects and load them with `dlopen` (`src/tests/host/CMakeLists.txt:91-119`). The MLFQ debug dump is absent (`src/modules/sched_mlfq.cpp:199`).
- **Loadable hardware modules [C,S,B].** Modules are ELF objects with kernel symbol imports and relocations (`src/drivers/DOCS.md:5-7`). Active declarations include xHCI (`src/drivers/xhci/xhci_mod.cpp:3270`), Intel HDA (`src/drivers/intel_hda/intel_hda_mod.cpp:1011`), VirtIO GPU (`src/drivers/virtio_gpu/virtio_gpu_mod.cpp:1263`), VirtIO net (`src/drivers/virtio_net/virtio_net_mod.cpp:738`), RNG (`src/drivers/virtio_rng/virtio_rng_mod.cpp:439`), input (`src/drivers/virtio_input/virtio_input_mod.cpp:768`), Bochs (`src/drivers/bochs_display/bochs_display_mod.cpp:218`), PS/2 keyboard/mouse (`src/drivers/ps2_kbd/kbd_mod.cpp:44`, `src/drivers/ps2_mouse/mouse_mod.cpp:30`), ext4 (`src/drivers/ext4/ext4_vfs_mod.cpp:742`) and VirtIO block (`src/drivers/virtio_blk/virtio_blk_mod.cpp:80`).
- **xHCI breadth [S,B].** The driver addresses/configures devices (`xhci_mod.cpp:1081,1317`), issues control transfers (`:1189`), initializes/polls hubs (`:1456,1570`), parses HID reports (`:1935,2122`), executes bulk/MSC commands (`:2254,2347`) and enumerates ports (`:2720,2797`). TODOs include slot cleanup/capability locking (`:1627,1919`).
- **Audio [C,S,B].** Intel HDA implements command transport (`intel_hda_mod.cpp:285`), reset (`:338`), playback (`:724`) and init (`:867`); the kernel audio layer registers drivers and runs a mixer thread (`src/kernel/src/audio.cpp:49,182,245`).
- **Desktop/compositor and system tooling [C,S,B].** The repository claims a compositing Wayland stack, terminal/file/editor/system apps and hot-reloadable UI (`README.md:94-109`). The compositor and app sources are in the active kernel/userspace graph. This audit confirms source/build reachability, not protocol conformance or runtime polish.
- **Diagnostics [C,S,B].** Panic handling captures registers and stack traces (`src/kernel/src/panic.cpp:24,105`); profiler lifecycle is implemented (`src/kernel/src/profiler.cpp:647,663,689`). The README's panic QR facility is described at `README.md:65-68`.

### Verification quality

The strongest Brook idea is not a driver; it is testing kernel logic on the host. Freestanding tests cover memory map, PMM, VMM, heap, strings, devices and VFS (`src/tests/CMakeLists.txt:41-108`). Host-native tests cover TCP, mutexes/RW locks/semaphores, cleanup, futex pools, descriptor tables, loopback and schedulers, with optional ThreadSanitizer (`src/tests/host/CMakeLists.txt:11-31,47-119`). **[T]** No result is marked `[CI]` or `[L]` here.

The repository also contains an unusually candid driver audit: `src/drivers/DOCS.md:39-57` records missing IRQ teardown, HDA locking/barrier issues, a VirtIO-net race, RNG synchronization and BAR-validation problems. This is a model for zlOS: make driver hazards durable and local to the subsystem.

### Gaps and no-code-copy lessons

- **Best ideas:** host harnesses around real kernel sources; dynamic scheduler modules tested through their load boundary; explicit driver hazard notes; integrated panic/profiler evidence.
- **Tradeoff:** linking kernel implementation into host shims is fast and sanitizer-friendly, but ABI/interrupt/MMU behavior still needs boot/hardware tests.
- **Do not inherit:** global fixed pools, incomplete teardown, or optimistic driver claims. Require timeouts, cancellation, hot-unplug cleanup and interrupt ownership before marking hardware “complete.”
- **zlOS action:** create a host-testable layer for zlfs metadata, network parsing, queues, allocators and compositor damage math, then retain QEMU/real-hardware gates for device semantics.

---

## Repository audit: Mathewnd/Astral

### Position and build truth

Astral is a 64-bit C x86-64 OS (`README.md:3`) claiming a preemptible SMP kernel, POSIX userspace, TCP/UDP/DHCP, ext2/FAT/tmpfs/devfs, NVMe/VirtIO/AHCI, VirtIO-net/RTL8169, xHCI/HID/hubs, uACPI power, multiuser operation, xbps and HDA/sndiod (`README.md:7-20`). The root Makefile builds firmware images and distro initrds (`Makefile:35-102`), disks (`:121-131`) and QEMU configurations (`:152-183`). The kernel Makefile compiles all C/assembly sources except uACPI tests with freestanding GNU17 flags (`kernel-src/Makefile:1-40`). **[C,S,B]**

### Meaningful features and how they work

- **Declarative boot initialization [S,B].** Rather than one monolithic init function, subsystem sources register ordered init routines. For example VFS registers after its dependency token (`kernel-src/fs/vfs.c:64-76`), devfs after VFS (`fs/devfs.c:503-519`) and NVMe after ACPI (`io/block/nvme.c:901-911`). This makes boot ordering visible to tooling.
- **Virtual memory [S,B].** `kernel-src/mm/mm.c:382,496,637,653` maps, unmaps, switches address spaces and initializes the manager. Page allocation (`mm/page.c:245`), slab initialization (`mm/slab.c:494,501`) and page cache initialization (`mm/cache.c:488`) are separate layers.
- **VFS and filesystems [S,B].** `kernel-src/fs/vfs.c:64,90,104` initializes, registers and mounts filesystems; `:399,518,624,636` implements vectored and scalar writes/reads. The active tree includes initrd, tmpfs, devfs, eventfs, ext2, FAT, pipefs and socketfs.
- **Processes, scheduling and ELF [S,B].** `kernel-src/include/kernel/scheduler.h:40-44` exposes scheduler init/queue/wakeup; `sys/scheduler/scheduler.c` supplies the implementation. ELF loading is declared at `include/kernel/elf.h:91` and implemented in `sys/elf.c`, including argument/environment/aux-vector layout around `sys/elf.c:270-284`.
- **Storage [C,S,B].** NVMe creates queue pairs, chooses queues, prepares PRPs and performs I/O (`io/block/nvme.c:376-673`), validates controller features and initializes namespaces (`:715-901`). AHCI allocates command slots and uses interrupts/DPCs (`io/block/ahci.c:172-278`), identifies ports (`:428-463`) and performs BIOS handoff/controller setup (`:481-541`). VirtIO block and GPT are also active.
- **Networking [C,S,B].** The public layer initializes ARP, IPv4, UDP, TCP, devices and loopback (`include/kernel/net.h:77-82`), sends UDP/IPv4 (`:88-89`) and registers adapters (`:94`). Sources include local sockets, DHCP, RTL8169 and VirtIO-net.
- **USB/input [C,S,B].** xHCI core, hub and HID layers are active. The HID parser and hub code implement report parsing and topology, though detach/hotplug cleanup remains incomplete.
- **Audio [C,S,B].** HDA implements stream start (`io/audio/hda.c:339`), stream allocation (`:606`), codec/path discovery (`:953,1007`); the abstraction registers devices and streams (`io/audio/audio.c:387,414`).
- **ACPI/power [C,S,B].** uACPI is integrated through `io/acpi/acpi.c:13,29,66` for early init, normal init and reboot, plus glue for host services.
- **Distro/userspace breadth [C,S,B].** The Jinx recipes cover a broad ported environment including X, Wine, GCC, bash, Quake and vim. These are build recipes and patches; this audit does not upgrade them to runtime proof.

### Verification and gaps

There is no project-native test suite or root CI in the inspected snapshot. Imported uACPI tests are upstream dependency tests, not Astral coverage. Therefore Astral has strong `[S,B]` breadth but no audited `[T]`, `[CI]` or `[L]` evidence.

Important source-level gaps:

- VM mappings can break on unaligned inputs and lack executable-file write-permission enforcement (`mm/mm.c:386,437`); device-fork semantics and TLB IPI scope remain TODO (`:593,626`).
- Signals have incomplete process-wide default actions, masks, IPIs, stop/parent locking and restartable syscalls (`sys/signal.c:288,323,352,356,405-406,531,554`).
- The page cache scans whole vnode lists and can lose error propagation (`mm/cache.c:349,483`).
- VFS rename needs prefix validation (`fs/vfs.c:785`); FAT preallocation has corruption/OOM TODOs (`fs/fat/fatfs.c:835,889`).
- AHCI lacks robust reset/timeout/NCQ handling (`io/block/ahci.c:578,631,638,648`); NVMe has unfinished interrupt cleanup (`io/block/nvme.c:840`); GPT checksum validation is TODO (`io/block/block.c:213`).
- USB detach/hotplug cleanup is unfinished (`io/usb/class/hub.c:401`, `io/usb/core/hub.c:563-571`); the HID parser has long-item, push/pop and 64-bit limitations (`io/usb/class/hid_parser.c:431-433,790`).
- HDA contains a wait-forever/overrun risk and hard-coded output path assumptions (`io/audio/hda.c:786,952-954`). TCP still needs segment buffering (`io/net/tcp.c:1258`).

### No-code-copy lessons

- **Best ideas:** ordered init registrations; broad modern storage/USB/audio coverage; a port graph that can approach self-hosting.
- **Tradeoff:** breadth arrived ahead of test and failure-path maturity. The dangerous work is not initial enumeration; it is timeout, reset, cancellation, detach and integrity validation.
- **zlOS action:** borrow the ordered init dependency concept, but make every init node declare failure policy, timeout and teardown. Put disk integrity checks and USB unplug/replug tests ahead of expanding the port catalogue.

---
