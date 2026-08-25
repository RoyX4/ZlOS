# Focused repositories: adversarial refutation and coverage pass

Date: 2026-08-21
Source shelf: `/home/roy/Documents/repos/zl-starred-sources`
Scope: the 15 focused repositories, at the exact checked-out commits named below.

This is not another feature summary. It challenges the strongest claims, checks whether apparently complete features are wired into a real execution path, searches for trust-boundary failures, records licensing constraints, and identifies where the available evidence stops. “Has a source file named X” is not accepted as proof that X works.

## Executive corrections

The most important corrections are:

1. **vib-OS is not a protected multi-user Unix-like OS.** Its primary process implementation explicitly says programs run in kernel space without memory protection; boot uses that implementation, while a separate aspirational `task_struct`/syscall/fork implementation is not wired into boot. Ext4, APFS, sandboxing, syscalls, DNS and several storage/device paths exist as source but are not initialized or are stubs. Terminal Python/Nano execution and `ping` are simulated.
2. **AthenX authentication accepts every credential.** `security/user/user.c::validate_credentials` returns success before doing any lookup or hash comparison. Its documented syscall surface is also mostly absent from the actual switch.
3. **mission-control's VNC data plane has no lease authentication.** `/vnc/1` through `/vnc/99` proxy directly to enumerable no-password QEMU WebSockets. The lease token protects touch/release APIs, not the WebSocket itself. Launch failures also leak slots and overlays.
4. **Banana OS lets any caller of `SysLoadDLL` execute a filesystem path as ring-0 code, and the loader trusts malformed ELF offsets.** This is not a reusable security design.
5. **MaslOS-2's real build is stronger than source-only evidence, but its ELF loader accepts only a pointer, not a length, trusts all offsets/counts, and can loop forever when `DT_RELA` exists with `DT_RELAENT == 0`.** The generated ISO still produced no serial oracle in a 15-second headless QEMU probe.
6. **Hyper's AArch64 handover is implemented and tested.** Any claim that it is merely a TODO is stale/wrong. The remaining AArch64 TODO is an abstraction/cache-maintenance refinement, not an absent handover.
7. **The Unix history repository is now fully fetched and its headline scale is independently supported:** 849,161 commits, 8,037 merges, 196 local/remote refs counted by `git branch -a`, 39 tags, and a 1.98 GiB pack. It is a dataset, not an OS implementation or the reproducible builder.

## Verification method

For each repository I checked the current commit, root license material, feature entry points, boot/build integration, TODO/stub paths, tests/CI, and at least one of build, static validation, artifact inspection, or runtime probing where the local machine allowed it. Severity in this document means severity **if the code is treated as production/reference-quality**, not a judgment of the author's stated hobby or research goals.

Evidence labels:

- **B** — freshly built in this checkout.
- **T** — a test or executable smoke check ran successfully.
- **S** — source path and control flow inspected.
- **A** — tracked artifact inspected, but not freshly proven at runtime.
- **C** — documentation/CI claim only.
- **U** — not verified locally.

## Cross-repository high-risk findings

| Severity | Repository | Finding | Evidence |
|---|---|---|---|
| Critical | AthenX-2.0 | `validate_credentials()` immediately returns `0`, accepting all credentials | S: `security/user/user.c` |
| Critical | mission-control | Enumerable WebSocket VNC endpoints have neither lease-token middleware nor VNC password | S: `public/vnc/.htaccess`, `app/Services/VmLauncher.php` |
| Critical | Banana OS | Unprivileged syscall surface loads attacker-selected ELF as kernel code | S: `kernel/sys/loaddll.cpp`, `kernel/thr/elf2.cpp` |
| High | vib-OS | Booted programs execute at EL1/kernel privilege with direct KAPI pointers; “sandbox” and separate user-task system are not used | S: `kernel/core/process.c`, `kernel/core/main.c` |
| High | MaslOS-2 | ELF parser has no input length/bounds checks and can divide/step by zero on malformed relocation metadata | S: `kernel/elf/elf.cpp` |
| High | mission-control | Any exception after allocation leaves a live lease/occupied slot; overlay can leak | S: `app/Http/Controllers/TryController.php` |
| High | mYOS | ATA waits can hang forever and filesystem writes can corrupt adjacent memory via unbounded names | S: `src/disk.c`, `src/fs.c` |
| High | vib-OS | README “full” network/filesystem/runtime claims contradict uncalled or stub implementations | S/C: paths detailed below |
| Medium | snarkOS | `--rest-rps 0` is accepted by CLI and causes division by zero during REST construction | S: `cli/src/commands/start.rs`, `node/rest/src/lib.rs` |

## Repository-by-repository refutation

### 1. `robotman2412/sapphire-gpu`

- **Snapshot:** `575efe030d2e`; CERN-OHL-P-2.0.
- **What survives refutation:** there are real SpinalHDL components for a serial command interface, status/debug/config registers, IRQ state, AHB3-to-DMA adaptation, SPI, framebuffer/texture reading and writing, ILI9341 scanout, pixel conversion, and eight simulation entry points. The required-versus-optional feature descriptor is a good negotiation pattern.
- **What does not:** README claims hardware rasterization, background scanout, many conversions, future shaders and compute. There is no rasterizer or shader core. `docs/cmd/drawing.md` and `docs/cmd/compute.md` are only 113 and 114 bytes. `hdl/sapphire/scanout/CrtScanout.scala` declares ports but no behavior. `SapphireCfg.descStruct` advertises `scanoutCount := 0` even though scanout-related components exist.
- **Serious integration gap:** `hdl/sapphire/dma/DmaMux.scala` declares `select` as a reset-zero register but never assigns it and does not connect setup/arbitration. Therefore no controller is selected after reset. Multiple selected bits would also drive the same target without one-hot enforcement.
- **Additional gap:** `hdl/sapphire/mem/Ahb3ToDma.scala` marks teardown correctness unverified and unreachable because it has no idle timeout; `TextureWriter` explicitly lacks 24-bpp support.
- **Evidence:** S. `sbt`/Verilator were unavailable, so no elaboration or simulation was rerun. This is a component library/prototype, not a complete GPU.
- **Clean-room constraint:** permissive open-hardware license still has notice/documentation obligations if code or hardware description is reused. Ideas can be restated independently; do not imply the interface is production-stable.

### 2. `UltraOS/Hyper`

- **Snapshot:** `d4f446550ebb`; MIT.
- **What survives refutation:** compact multi-firmware loader with BIOS i686, x86-64 UEFI, AArch64 UEFI, FAT12/16/32, ISO9660, MBR/GPT, PXE/TFTP, path/origin/UUID/PARTUUID addressing, fragmented-file handling and a custom Ultra handover protocol. The assertion kernel plus generated-image matrix is unusually strong evidence for bootloader work.
- **Fresh evidence:** `pytest --collect-only -q` found **75** parameterized cases. The main audit's BIOS run of **33 passed, 42 deselected** is consistent with this collection. The collection covers AArch64 UEFI normal and EL2/VHE paths, not just x86. Pytest emitted unknown-marker warnings because the custom marks are not registered, a hygiene issue rather than a false pass.
- **Correction:** AArch64 handover is real: `loader/arch/aarch64/handover.asm` and `handover_impl.c` build TTBR/TCR/MAIR/SCTLR state, handle EL1/EL2 and VHE, perform cache maintenance, then branch to the kernel. Tests include `test_normal_uefi_aarch64_*` and `test_uefi_aarch64_boot_el2`. The TODO at `handover_impl.c` is to add a proper abstraction for one cache-related operation, not “implement AArch64 handover.”
- **Actual gaps:** config chain-loading remains TODO in `loader/loader.c`; UEFI APM is unsupported; `loader/include/common/hardened_string.h` admits its known-length string path can still read out of bounds because it uses `strlen` rather than a bounded operation.
- **Evidence:** B/T/S from the main audit plus this independent test collection. This is the strongest boot-path reference in the focused set.

### 3. `ProvableHQ/snarkOS`

- **Snapshot:** `a8e2e33f6e76`; Apache-2.0.
- **Coverage correction:** `cargo metadata --no-deps --format-version 1` reports **21 workspace packages**, not 20. They cover account/CLI/display, node roles, BFT, BFT events and storage services, TCP, router/messages, sync/locators/communication, CDN, consensus, REST, metrics and utilities.
- **What survives refutation:** role-specific nodes, authenticated peer handshakes, routing and peer limits, multiple synchronization channels, RocksDB-backed ledger/checkpoints, BFT/consensus, a large REST surface, operational metrics, devnets/upgrades/chaos scripts, and hundreds of Rust test functions. The role split and explicit startup/shutdown lifecycle are useful architectural references.
- **Production gaps are narrower than the word TODO suggests:** the `unimplemented!()` calls found are in BFT ledger-service mock/translucent/prover implementations (`node/bft/ledger-service/src/{mock,translucent,prover}.rs`), not proof that the main validator path is generally stubbed. Still, sync contains unresolved peer-banning/integrity/rate-limit work, BFT gateway per-peer rate limiting is TODO, and plugin reload is absent.
- **New correctness bug:** CLI `rest_rps` is a plain `u32` with default 10 and no nonzero validator (`cli/src/commands/start.rs`). `node/rest/src/lib.rs::build_routes` computes `1_000_000_000 / rest_rps`; `--rest-rps 0` therefore panics during startup.
- **Authentication boundary warning:** `auth_middleware` explicitly skips authentication whenever global `JWT_SECRET` was not initialized. The CLI normally initializes it, but `Rest::start` itself does not enforce the invariant; direct library construction or the deliberate `--nojwt` option leaves node address, DB backup and Slipstream plugin-management routes open. This should be an explicit capability/mode contract, not a global side effect.
- **Plugins:** loading a config-selected native plugin is authenticated but still full in-process code execution. This is appropriate only for trusted administrator-controlled artifacts; it is not a sandbox/plugin-security pattern for zlOS.
- **Evidence:** S plus successful metadata parsing. A full build/test was not attempted because the workspace pins a large Git revision of `snarkVM` and exceeds a proportionate local gate on this four-core machine.

### 4. `dspinellis/unix-history-repo`

- **Snapshot checked out:** `d0d404ce5048` on the PDP-7 development ref; full remote history fetched. Licensing is mixed and source-specific; root `LICENSE` points to Caldera and BSD conditions. Treat each historical subtree as carrying its own provenance/license.
- **Fresh measurements:** `git rev-list --all --count` = **849,161**; `git rev-list --all --merges --count` = **8,037**; `git branch -a` count = **196**; tags = **39**; object packs = **1.98 GiB**, 5,455,351 packed objects, no garbage. These support README's “about 850 thousand,” “more than eight thousand,” and “2 GB” claims. A raw name/email uniqueness pass found 3,000 author identities, but that is not directly comparable with README's curated “about 2700 individual contributors.”
- **Timeline evidence:** the checked-out synthesized PDP-7 commit is dated 1970-06-30; newest refs include `origin/main`/`Licenses` dated 2026-01-05 and FreeBSD 15 refs dated 2025. Tags cover Research PDP-7 through BSD/386BSD milestones.
- **Boundary:** this repository is generated output. The custom synthesis code and metadata live in `unix-history-make`; README explicitly says this repo is regenerated and is not the contribution/reproduction source. Therefore do not mine its build system as an OS strategy. Use it for archaeology, blame continuity and provenance tests.
- **Evidence:** S/A with full-history Git measurements; no attempt to build historical Unix releases.

### 5. `felipenlunkes/osdev-projects`

- **Snapshot:** `0736c963d92a`; BSD-3-Clause.
- **Correct classification:** curated README/index only. There is no OS source, test suite, schema, scraper, validator or CI workflow in the checkout.
- **Useful scope:** family and implementation-language taxonomies, links, activity, architecture, hosting and license badges. It is a discovery seed, never feature evidence for the linked projects.
- **Data defects verified:** duplicate OS67 rows at README lines 497 and 499; malformed LemonOS URL at line 560 contains an injected PowerNex Markdown link; activity means “commits more recent than 4 years,” a manual and quickly stale rule. Architecture/platform coverage is incomplete and badge-backed licenses can drift.
- **Evidence:** S. Any integration should first normalize the list into machine-readable data, resolve redirects, deduplicate and timestamp all observations.

### 6. `marceldobehere/MaslOS-2`

- **Snapshot:** `154d42e53fc0`; AGPL-3.0.
- **Fresh build:** after the documented main audit worked around the first-run `objects/` directory assumption, the full build completed and produced a **65 MiB bootable ISO**, **60 MiB `external/programs.saf`**, and an x86-64 kernel-loader ELF. The checkout returned clean, meaning generated tracked artifacts matched the snapshot. This upgrades the repository from source-only to build evidence.
- **Runtime limit:** `timeout 15 qemu-system-x86_64 -m 512M -cdrom MaslOS2.iso -display none -serial stdio -no-reboot -no-shutdown` reached the timeout with no serial output. That proves QEMU accepted the image, not that the desktop, apps, scheduler or storage booted correctly.
- **What survives refutation:** a broad GUI/app ecosystem, scheduler and task structures, per-task page-table contexts, kernel/user stacks, ELF apps/modules, message queues/conversation IDs, stdio-over-IPC, AHCI/custom partition/filesystem/archive layers and audio. These are meaningful integration examples.
- **High-risk parser:** `kernel/elf/elf.cpp::LoadElf(uint8_t *data)` receives no buffer length. It trusts `e_phoff`, `e_phnum`, `p_offset`, `p_filesz`, `p_memsz`, dynamic offsets and relocation sizes. A malformed file can cause arbitrary out-of-bounds reads/writes. `elf_apply_relocations` increments `off += rela_ent`; if a dynamic section provides relocations but leaves `DT_RELAENT` zero, the kernel loops forever. It also does not enforce `p_filesz <= p_memsz`.
- **Scheduler limitations:** explicit no-SMP TODO; random `uint64_t` PIDs are not checked for collision; lock handling is hand-rolled and the interrupt scheduler may forcibly unlock an already-locked global task list. These patterns need replacement, not copying.
- **Clean-room constraint:** AGPL is strong network copyleft. The requested idea-only analysis is appropriate; no source copying into zlOS.

### 7. `alexdboxall/Banana-Operating-System`

- **Snapshot:** `f463540c6072`; CC BY-NC 4.0 plus third-party notices.
- **What survives refutation:** ambitious 32-bit graphical OS with real kernel/process/VFS/device/GUI/application/package/installer structure and many tracked ELF/ISO artifacts. Dynamic driver relocation and kernel symbol resolution are technically substantive.
- **Critical privilege failure:** syscall table entry 43 maps to `SysLoadDLL`. `kernel/sys/loaddll.cpp` checks only that the user pointer is non-null and the file exists, then passes that user-selected path to `KeLoadAndExecuteDriver`, which calls the module entry as kernel code. There is no privilege/capability check and no safe copy of the user pathname.
- **Malformed-module attack surface:** `kernel/thr/elf2.cpp::KeIsELFValid` checks only four magic bytes; class, byte order and machine checks are commented out. Every ELF offset/count/section/string/relocation is trusted without comparing against file length. Relocation writes are therefore attacker-influenced ring-0 writes.
- **Resource/correctness defects:** driver open/read errors leak allocations; the number of bytes read is ignored; module data and the allocated kernel VAS are not consistently freed on failures; unknown RELA triggers a kernel panic. The driver model is a good shape but this loader is unsafe.
- **Claim correction:** README mentions x86/x86-64, but build/tool names and tracked active artifacts are i386/ELF32. No x86-64 build path was found.
- **Clean-room constraint:** the NonCommercial restriction is incompatible with many downstream/commercial distributions and Creative Commons is a poor software-code license. Treat only as design inspiration and preserve third-party provenance in any comparative documentation.

### 8. `KingVentrix007/AthenX-2.0`

- **Snapshot:** `e6734419e24f`; GPL-3.0.
- **What survives refutation:** a large vertical hobby-OS tree with 32-bit boot/interrupt/memory/storage/display/audio/network/shell/TUI/user-program components and a tracked i386 kernel ELF.
- **Critical authentication bypass:** `security/user/user.c::validate_credentials` declares `User check_user;` and immediately executes `return 0;`. All username/password inputs are accepted. The unreachable code below also prints stored and provided password hashes to the console.
- **Syscall claim mismatch:** the long function comment in `arch/i386/sys_handler.c` claims dozens of syscalls. The actual switch implements only exit/fork messages, read/write/open/close/lseek and a default. `SYS_EXIT` and `SYS_FORK` print “coming soon.” `screen_ctrl()` is a stub. `userspace/libc/syscall.c::syscall_write` has an empty non-void body.
- **Trust-boundary failures:** `handle_print_system_call` passes a raw user pointer directly to `printf(message)`, enabling format-string reads/writes if `%n` is supported and crashing on invalid pointers. Other syscalls also directly dereference user pointers. `SYS_READ` treats `param1` first as a `parameters *`, then overwrites it with 1001 bytes regardless of caller-provided capacity.
- **Concurrency correction:** `kernel/scheduler/multitasking.c::init_multitasking` is empty. The current system is not a functioning multitasking/userspace-isolation reference.
- **Evidence:** S/A; dry-run build topology works, but the required custom i686 cross compiler was absent, so no fresh binary/runtime proof.

### 9. `ktauchathuranga/privilegeos`

- **Snapshot:** `1563a393fe6d`; wrapper scripts MIT, while Linux and BusyBox retain GPL licenses.
- **Correct classification:** Linux 6.15.3 + BusyBox 1.36.1 rescue/initramfs builder. It is not an independently implemented kernel.
- **What survives refutation:** broad x86-64 rescue kernel configuration, BusyBox command surface, hybrid BIOS/UEFI GRUB image creation, removable-media size/mount guards, initramfs setup, QEMU launch, operational logs and a companion restore script.
- **Supply-chain defect:** `build.sh` downloads versioned Linux and BusyBox tarballs over HTTPS but performs no checksum or signature verification, despite README advising users to verify the final image. A reproducible rescue image needs pinned digests for every input.
- **Security boundary:** passwordless root shells respawn on tty1-3 and serial (`build.sh` generated inittab), and `/etc/passwd` uses empty root password. That can be valid for offline physical recovery media but must never be repackaged as a general secure OS.
- **Do not integrate:** `scripts/getadmin.sh` replaces Windows `sethc.exe` with `cmd.exe` to obtain SYSTEM at the login screen. Even with a restore script and legitimate recovery uses, credential bypass is not an zlOS feature or design goal.
- **Evidence:** shell syntax passed in the main audit; no large sudo/network build was run. The claimed release image/kernel are not tracked in this snapshot, so source does not prove a particular distributed image.

### 10. `yashvi-soni-30/mYOS`

- **Snapshot:** `20bf154d8da5`; **no root license file found**, so copying is not authorized by the checkout.
- **What survives refutation:** tiny multiboot shell, VGA output, keyboard IRQ, ATA PIO and a four-entry persistent filesystem serialized starting at LBA 1. It is a useful minimum vertical persistence demo.
- **Filesystem corruption risks:** custom `strcpy` has no destination bound; names are 16 bytes, while shell input can be much longer. `fs_write`, `fs_cat` and `fs_rm` ignore `current_dir` and file type, so a same-name entry in another directory can be modified/deleted and directories can be treated as files. Duplicates are allowed. `fs_pwd` pushes into a fixed 16-entry stack without a depth check even though malformed on-disk parent links are trusted.
- **Disk hangs/data loss:** `disk_wait` waits forever on BSY, never waits for DRQ, never checks ERR/DF, has no timeout and does not issue cache flush after writes. One absent/wedged disk can hang the kernel; a failed write is reported as success.
- **Interrupt fragility:** all 256 IDT gates are present but point at address zero except keyboard IRQ 33. Any exception/other enabled interrupt jumps to zero. Keyboard declares an `extended` state but never consumes the `0xE0` prefix; arrow handling therefore aliases keypad scan codes.
- **Artifact inconsistency:** `Release/kernel.elf` is 32-bit i386 while `iso/boot/kernel.elf` is a different 64-bit x86-64 ELF. The current source is 32-bit. `Release/mYOS.img` is a sparse 10 MiB image. Tracked artifacts are not a single coherent release proof.
- **Evidence:** S/A; no build script/test suite exists in the snapshot.

### 11. `cppsever/MineBios`

- **Snapshot:** `ca9831fa763c`; Apache-2.0.
- **Fresh build:** NASM produced a 512-byte boot sector and 61,326-byte kernel; the Makefile created a valid 1.44 MiB FAT12 image and copied `kernel.bin`. The default `make` target then launches QEMU rather than stopping after the build, which is poor CI behavior. QEMU was terminated after the non-headless launch; gameplay was not observed.
- **What survives refutation:** BIOS drive probing/retries, FAT12 root scan and FAT-chain following, then a real-mode mode-13h block game with camera, collision and physics. It is a game/application with a competent small boot wrapper, not an OS services architecture.
- **Loader limitation:** data LBA uses hard-coded `cluster + 31`, correct only for the fixed 1.44 MiB BPB layout. It does not derive data start or multiply by sectors-per-cluster, so a changed BPB breaks loading.
- **Quality signal:** NASM emitted a very large warning stream, principally deprecated `$` hexadecimal syntax in generated map data and invalid register-size specifications. Warnings do not stop the build.
- **Evidence:** B/S, not runtime T.

### 12. `viralcode/vib-OS`

- **Snapshot:** `73b64299bf6e`; README claims MIT, but the referenced root `LICENSE` file is missing. Treat code reuse as legally ambiguous until the author supplies it; vendored components also need individual license review.
- **Fresh build:** `make kernel` produced a 7,813,904-byte, static AArch64 ELF at entry `0x40000000`. QEMU AArch64 is not installed locally, so boot/runtime remains unverified.
- **Primary architecture truth:** the header comment in `kernel/core/process.c` says programs run in kernel space and call kernel functions directly with “No memory protection.” ARM64 contexts start as EL1h and pass a raw KAPI pointer. This is the implementation used by `kernel/core/main.c::start_init_process`.
- **Two incompatible process worlds:** boot initializes `sched_init()` and a second `process_init()`, but launches `/sbin/init` through the kernel-space `kernel/core/process.c` table. `kernel/sched/fork.c` and `kernel/syscall/syscall.c` define a separate `task_struct`/address-space/userspace model. `syscall_init()` has no caller in the tree. `do_fork`/`do_execve` have no production caller. These files are not proof of user isolation, fork or exec.
- **Process-table bug:** `process_create` reserves a slot and PID before validating/reading/loading the file. Every early failure leaves the slot in READY state and leaks capacity. It also allocates stacks/ELF regions without a coherent address-space owner because all programs share kernel space.
- **Filesystem claim failures:** boot mounts only RamFS. `ext4_mount` and `apfs_mount` have no caller. APFS file reads are an explicit TODO returning `-1`; it is read-only metadata probing, not APFS support. Ext4 lacks triple-indirect blocks and full unlink, and does not reject unsupported modern feature bits before interpreting classic block pointers. FAT32 has a TODO block-device read path.
- **Network claim failures:** DNS builds a query then returns `-1` without sending; socket connect marks a socket connected immediately without transport; TCP transmits to broadcast MAC with ARP lookup TODO; ICMP echo replies are TODO. The terminal's `ping` prints four simulated replies. This is not “full TCP/IP/DNS/internet.”
- **Runtime/language simulation:** terminal `python` only prints that MicroPython exists under nonexistent `vendor/micropython`; `run` scans source text for `print`, hard-codes `add`, and hard-codes Fibonacci output. Nano is handled the same way. `runtimes/python` and `runtimes/nodejs` are host-specific download/build scripts with no checksums and no demonstrated OS integration.
- **Security-claim failures:** the sandbox allocates an “isolated” stack but never switches to it and only installs a global fault-recovery pointer; there are no callers. It is neither per-CPU nor an isolation boundary. ASLR is a timer-seeded global LCG, exposes low seed bits in logs and randomizes addresses in one kernel-space loader; it is not cryptographic. `sys_mmap` uses global pointers, writes directly to the chosen virtual range, and `munmap` is a no-op.
- **Hardware/SMP claim failures:** main prints “Loading NVMe/USB” without invoking those drivers. NVMe read/write are stubs, USB mass-storage transport is stubbed and its xHCI call is commented, Bluetooth send is TODO. ARM SMP initializes only CPU0 and explicitly does not boot secondaries; DTB parsing and CPU discovery are TODO.
- **CI false green:** `.github/workflows/ci.yml` runs `timeout 10s make run || true`, discards any exit status/output, and the following “kernel panic” step only echoes success. `scripts/run-tests.sh` checks tool/file presence and ELF architecture, not behavior.
- **Positive residue:** a large GUI/media/demo codebase, a reproducible kernel compile on this host, useful subsystem naming, ELF validation helpers and real RamFS/VFS/graphics work. It should be mined at function granularity with every path re-proven, not accepted by README feature labels.

### 13. `whispem/asm.fm`

- **Snapshot:** `a5572911ba9c`; MIT.
- **Fresh executable proof:** `make -j4` was already up to date. All **25** binaries ran under a five-second timeout and emitted WAV files recognized as 44.1 kHz, 16-bit mono PCM. `soxi` parsed every file with durations from 1.0 to 4.0 seconds.
- **What survives refutation:** oscillator, sequencing, polyphony/saturation, ADSR, FM, vibrato/tremolo/PWM, filtering/auto-wah, bitcrush/distortion/ring modulation, delay/chorus/reverb, supersaw and tempo examples all produce structurally valid audio.
- **Boundary:** standalone Linux x86-64 programs write fixed, offline WAV buffers to stdout. They are not a real-time mixer, device driver, streaming API, scheduler, portable DSP library or test suite. Algorithms and constants are duplicated across examples.
- **Integration lesson:** use as small DSP golden vectors. Reimplement the math behind a common zl audio API and compare exact/offline output; do not adopt its one-program-per-effect structure as an OS audio architecture.

### 14. `brainboxdotcc/mission-control`

- **Snapshot:** `a6b7ec4838ef`; `composer.json` declares MIT, but no root LICENSE file is present in the checkout.
- **Fresh validation:** PHP 8.4 linted **95** PHP files successfully. `vendor/` is absent, so PHPUnit/Larastan/asset gates were not rerun. CI definition does install dependencies, build assets, run coverage tests and static analysis.
- **What survives refutation:** fixed slot pool, cache locks, lease/token hashing, hard/idle deadlines, qcow2 overlays, structured QEMU argument arrays, logs, touch/release APIs, reaper/kill/status commands and good feature-test coverage. It is a useful public-demo control-plane starting point.
- **Critical unauthenticated VNC:** QEMU is started with `-vnc host:display,websocket=host:port` and no password/SASL. `public/vnc/.htaccess` proxies `/vnc/1`…`/vnc/99` straight to enumerable localhost WebSocket ports. No route middleware or lease token is involved. Anyone who can reach the site can attempt another user's active slot directly. `/session/{lease}` itself also checks no session token, though UUID secrecy is irrelevant because the WebSocket needs only a slot number.
- **Launch rollback failure:** `TryController::start` allocates and marks a slot in use, stores its token, then creates an overlay and launches QEMU. Its `finally` only releases the start lock. Any exception after allocation leaves the lease/slot occupied; failures after overlay creation leak the overlay and possibly QEMU.
- **Allocator atomicity:** cache locking serializes cooperating app instances, but lease save and slot save are not one DB transaction or row lock. A cache backend outage/misconfiguration can split state.
- **Termination defects:** manual release sends SIGKILL first, waits “to exit cleanly,” then sends SIGTERM if the PID still exists—the order is reversed. Neither release nor the trait verifies process identity, so PID reuse can kill an unrelated process. Kill/unlink failures are suppressed and the slot is freed anyway, allowing an orphan VM to share a recycled display/WebSocket port.
- **Configuration defects:** `--enable-kvm` is always added even if `accel` is not KVM; non-AHCI controller values leave the base disk unattached; backing format is hard-coded `raw`; Apache mapping stops at slot 99 while the CLI allows larger counts. Extra arguments are administrator-controlled rather than attacker-controlled, so the incomplete denylist is a deployment hazard, not direct remote command injection; Symfony Process prevents shell interpretation.
- **Missing containment:** no cgroup CPU/RAM/process limits, seccomp/AppArmor profile, network egress policy, per-VM identity, or daemon supervisor. User-mode NAT is enabled by default. A public zlOS demo must add these before exposure.

### 15. `osdev0/freestanding-c-hdrs-gnu`

- **Snapshot:** `cdd9b3cc1180`; GCC GPL-3.0 with GCC Runtime Library Exception as applicable. These are compiler-generated/runtime headers, not a standalone permissive libc.
- **What survives refutation:** generated GCC 16.2.0 header sets for i686, x86-64, AArch64, RISC-V 64, LoongArch64 and m68k; architecture intrinsics plus core freestanding headers; primary GCC/binutils downloads pinned by BLAKE2 digests; a six-architecture generation workflow; targeted Clang compatibility patches.
- **Fresh evidence inherited and source-confirmed:** Clang cross-target smoke compile passed for `stddef/stdint/stdarg/limits` on all six targets and native GCC atomics passed. Clang atomics failed across all six because GCC's `stdatomic.h` invokes `__atomic_load` on `_Atomic` pointers in a way Clang rejects. The patch only adjusts `stddef`, `stdint` and some limit macros, not atomics.
- **CI weakness:** workflow proves the toolchain/header generation completes, then each matrix job directly commits and pushes generated output. It does not compile a header-smoke matrix with GCC and Clang. Concurrent push retry logic handles races but produces unreviewed generated commits and can hide compatibility regressions.
- **Scope correction:** importing this repository does not provide hosted libc functions, syscalls, POSIX headers, ABI startup, linker scripts or a C++ standard library. It supplies the compiler-owned portion of a freestanding toolchain.
- **Supply-chain note:** primary tarballs are pinned, but GCC's `contrib/download_prerequisites` performs additional dependency acquisition during generation. Preserve/cache and attest every transitive input for a reproducible zl toolchain.

## Missing-major-feature check

The adversarial pass did not find a previously missed production-ready subsystem that changes the focused-set architecture. It did find several **real but easy-to-undervalue** assets:

- Hyper's AArch64 EL1/EL2/VHE handover and 75-case cross-firmware test matrix.
- snarkOS's 21-package role/lifecycle organization and authenticated, versioned network operations.
- MaslOS-2's successful full artifact build and broad stdio-over-IPC/app composition.
- asm.fm's 25 executable DSP golden vectors.
- Unix history's fully measurable provenance dataset and continuous timeline.
- Banana OS's package/installer/driver breadth, despite unsafe privilege boundaries.
- mission-control's lease/reaper/control-plane mechanics, once separated from its unauthenticated data plane.

## Clean-room integration rules for the parent synthesis

1. **Ideas, contracts and tests can transfer; code does not.** This is mandatory for AGPL MaslOS-2, GPL AthenX, CC BY-NC Banana OS, mixed historical Unix, missing-license mYOS/vib-OS, and any vendored subtree.
2. **Require a callable path from boot to feature.** A file, type, TODO-complete-looking implementation or successful compile is not a feature unless initialization, error handling and a behavioral oracle are present.
3. **Treat parser inputs as hostile.** Every ELF, filesystem, network packet, module config and disk structure needs a buffer length, checked arithmetic, feature-bit rejection and failure cleanup before any reuse of these ideas.
4. **Separate control-plane authentication from data-plane authentication.** mission-control proves that token-protected lifecycle APIs do not protect the actual VNC session.
5. **No feature badge without a proof level.** Record C/S/B/T/Q/H evidence separately so vib-OS-style CI cannot turn a timeout into “boot success.”
6. **Prefer Hyper-style parameterized executable matrices and asm.fm-style golden outputs.** Avoid README-driven inventories and presence-only scripts.
7. **Use snarkOS-style explicit roles and lifecycle contracts, but never global optional authentication or in-process untrusted native plugins.**
8. **For toolchain inputs, generate from pinned upstream and test consumers.** freestanding-c-hdrs-gnu's generation model is sound; its absent cross-compiler smoke matrix is the part zl must add.

## Exact commands and result ledger

Commands below were run against the source shelf during this pass:

```text
Hyper:       pytest --collect-only -q
             => 75 tests collected; unknown custom-marker warnings

snarkOS:     cargo metadata --no-deps --format-version 1
             => 21 workspace packages

Unix:        git rev-list --all --count
             => 849161
             git rev-list --all --merges --count
             => 8037
             git branch -a --format='%(refname:short)' | wc -l
             => 196
             git tag | wc -l
             => 39
             git count-objects -vH
             => 5,455,351 objects in two packs; 1.98 GiB; no garbage

MaslOS-2:    full make completed after first-run directory workaround
             => bootable 65 MiB ISO, 60 MiB SAF, x86-64 loader ELF
             timeout 15 qemu-system-x86_64 ... -serial stdio
             => timeout 124, no serial oracle

MineBios:    make -j4
             => 512-byte boot sector, 61,326-byte kernel, valid 1.44 MiB FAT12 image;
                default target launched QEMU and was manually terminated

vib-OS:      make kernel
             => 7,813,904-byte AArch64 static ELF, entry 0x40000000
             qemu-system-aarch64
             => unavailable locally; runtime not tested

asm.fm:      run every bin/*, capture stdout, inspect with file and soxi
             => 25/25 valid 44.1 kHz 16-bit mono PCM WAVs, 1.0-4.0 seconds

mission-control:
             find app config routes database tests -name '*.php' | php -l
             => 95/95 syntax-clean; vendor absent, so PHPUnit not run
```

## Weakest links

- Runtime/graphical behavior remains unproven for MaslOS-2, MineBios, mYOS, Banana OS, AthenX and vib-OS in this pass.
- snarkOS's full dependency graph and test suite were not compiled locally.
- Sapphire simulations were not rerun because the Scala/Verilator toolchain was absent.
- Several repositories are shallow snapshots; this document evaluates the checked-out code, not every historical branch or release.
- License labels in README/package metadata are not substitutes for a present license file; vib-OS, mYOS and mission-control need author clarification before code reuse.
