# Architecture and OS audit of nine starred repositories

Date: 2026-08-21
Audit root: `/home/roy/Documents/repos/zl-starred-sources`
Purpose: extract architecture, mechanisms, strengths, gaps, and reusable ideas for zl, zlOS, and the planned agent system without copying implementation code.

> **Adversarial correction notice:** this is the first-pass dossier. The later
> [`ARCHITECTURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md`](ARCHITECTURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md)
> is authoritative wherever wording conflicts. In particular, do not treat
> Chitti built-ins as publisher-signed skills, NexiOS as a working derivation
> tree, Skift as a working transactional-transfer implementation, or Fudge as
> lacking per-task Ring-3 page tables. The normalized matrix and integration
> contracts already incorporate the corrected trust boundaries.

## Executive result

The nine repositories do not form one ladder from “small” to “large.” They are useful for different reasons:

- **ChittiOS** is the strongest direct reference for an agent-native execution model: a tool grammar, capability-gated executor, attenuated subagents, taint/citation checks, signed skills, and an audit chain. Its breadth is real at source level, but important security language is stronger than its current isolation: the orchestrator remains in ring 0, several “tenants” are special flat binaries rather than general processes, and the audit body is bounded and process-local.
- **NexiOS** is the strongest reference for explicit real-time invariants and deterministic test infrastructure: fixed pools, deadline accounting, CSpace derivation/revocation, state-rewind tests, and a very large in-kernel test corpus. “Hard real-time” remains an aspiration rather than a demonstrated end-to-end property while admission is advisory, I/O is often polled, SMP is unfinished, and waits remain unbounded.
- **RustOS** has the best evidence discipline: ownership and authority contracts, fail-closed provider boundaries, source-to-formal-model maps, mutation/conformance gates, and unusually candid failed-release evidence. It is also by far the heaviest architecture and cannot be treated as proof that all described services work end to end.
- **Zinnia** is the cleanest compact Unix-kernel reference here: broad syscall/VFS/network support and narrow dynamically linked driver crates. It is mostly kernel and drivers, not a complete desktop distribution, and CI only compiles x86_64.
- **Skift** demonstrates an object-capability kernel with userspace service strata and dependency-locked modular builds. The checked-out mirror omits most advertised framework source until external projects are fetched, networking and several host APIs are unimplemented, and the current domain free-slot calculation appears to break capability transfer.
- **Fudge** offers the most interesting minimal concurrency model: every task receives typed mailbox events, programs bind callbacks, and higher-level driver/filesystem policy lives in userspace. It has almost no automated verification and relies on fixed global pools, weak isolation, and many synchronous or polling paths.
- **TacOS** is the best small end-to-end Unix/desktop learning sample: Limine boot, SMP, fork/exec, local sockets, PTYs, a framebuffer window server, terminal, shell, and Doom are connected into one image. It openly calls itself a toy; source contains blocking spins, incomplete syscalls/drivers, and an apparent ELF write-permission inversion.
- **NyauxKC** is a rewrite-stage Rust kernel, useful mainly for its simple per-CPU run-queue and VFS object decomposition. Its own roadmap correctly marks syscalls, user threads, and ELF loading absent. Multiarchitecture build recipes outrun implemented architecture modules, and its PCI BAR helper appears to reverse `(bus, slot, function)`.
- **Cyjon** is an unusually readable pure-assembly, higher-half, SMP multitasking kernel paired with a separately built C userspace image from Fern-Night. It has no tests/CI, its top-level Makefile masks all build failures, and its custom no-commercial/no-derivatives license makes it a study-only source.

For zlOS, the best combined direction is **Zinnia's narrow driver seams + Fudge's event-driven user services + Skift/Chitti capability handles + NexiOS bounded allocation/deadline rules + RustOS evidence contracts**. Chitti's ring-0 agent approach should not be copied. Agents should be ordinary isolated zlOS processes whose tool calls cross one small, deterministic, capability-checking broker.

## Evidence rules and limitations

Every substantive item uses one or more of these labels:

- **`[C] Claimed`**: stated by a README, roadmap, status document, screenshot, or checked-in log.
- **`[S] Source`**: implementation source exists at the cited path/identifier.
- **`[R] Reachable`**: the implementation is connected to the repository's normal build, boot, configuration, or default service graph. This is a static reachability judgment, not proof that it works.
- **`[T] Covered`**: a test, formal model, CI job, or test scenario explicitly exercises the feature. It means the coverage artifact exists, not that this audit ran it successfully.
- **`[V] Locally verified`**: built or executed successfully during this audit.

No item in this first-pass report is labelled `[V]`. A later focused executable pass is recorded separately in [`ADDITIONAL_EXECUTABLE_EVIDENCE_2026-08-21.md`](ADDITIONAL_EXECUTABLE_EVIDENCE_2026-08-21.md): NexiOS kernel/ISO and Zinnia kernel/driver compilation earned narrow `[V-build]` evidence, while NexiOS target self-tests did not earn `[V-test]`. Checked-in test histories and CI workflows remain `[C][T]`, never automatically `[V]`.

“Source exists” is deliberately weaker than “works.” A driver file, syscall number, formal model, screenshot, or TODO-compatible interface is not runtime evidence. Exact line references are against the immutable commits below.

## Immutable snapshot and license ledger

| Repository | Audited commit | Commit date | License in checkout | Checkout caveat |
|---|---|---:|---|---|
| `chittios/chitti` | `a282d7907c93f39fa1e2ce42741132276e02bb4f` | 2026-08-19 | Apache-2.0 (`LICENSE`) | `third_party/m1n1` and nested artwork initialized at recorded gitlinks |
| `staycool1374-Ger/nexios` | `624b9e5f5209e51088754a5f3672c34b1c5df880` | 2026-08-16 | GPL-3.0-or-later (`LICENSE.txt`) | checked-in results, no local execution |
| `ierwarf/rustos` | `73db2d5fec0e4dfa72c4c8ccf1cffc17e932861c` | 2026-08-21 | MIT (`LICENSE`) | full gate is `cargo xtask check`, not workspace build |
| `zinnia-os/zinnia` | `1c10cf45381092aa3bb0388b6b63af10859a0e22` | 2026-08-08 | GPL-2.0-only (`LICENSE`) | `bindings/uacpi/uacpi` initialized at recorded gitlink |
| `skift-org/skift` | `e3822171165cef5535e25602794750c4df1e3530` | 2026-07-08 | LGPL-3.0-or-later (`license.txt`) | most Karm/Hideo/Vaev source is fetched externally |
| `jezze/fudge` | `5ffb39c7b608a3b117a724d11dc6725857b6c6de` | 2026-08-20 | MIT (`LICENCE:1-18`) | Travis-only build check; no current test gate |
| `UnmappedStack/TacOS` | `c10e450debf628233f5b345e2232dc1fc5d28619` | 2025-10-08 | MPL-2.0 (`LICENSE:1`) | no CI/test suite in checkout |
| `rayanmargham/NyauxKC` | `94879b5b2c5cb1f0ee7c9a52cc73273c574ed4c0` | 2026-08-07 | ISC-style text (`LICENSE:1-12`) | license copyright names mintsuki, not repo author |
| `CorruptedByCPU/Cyjon` | `b164a3a8d7b1cff0e853944a11078dd749104019` | 2024-08-16 | custom attribution, non-commercial, no-derivatives (`LICENSE:3-15`) | initialized Fern-Night `0029894`; build wrapper masks failure |

License consequence: ideas and clean-room designs may be learned from all nine, but code reuse would require separate legal review. Cyjon is especially unsuitable for reuse; NexiOS, Zinnia, and Skift are copyleft. This report recommends mechanisms, not code transfer.

## Comparative subsystem map

| Subsystem | Chitti | NexiOS | RustOS | Zinnia | Skift | Fudge | TacOS | NyauxKC | Cyjon |
|---|---|---|---|---|---|---|---|---|---|
| Boot/architectures | Limine x86_64, custom AArch64 path `[S][R][T]` | Limine x86_64; ARM/RISC-V preparation `[S][R partial]` | UEFI/BSP/AP contract, x86_64 focus `[S][R][T]` | x86_64 plus partial RISC-V `[S][R]` | EFI x86_64 `[S][R][T]` | x86 GRUB; ARM/RISC-V early `[S][R partial]` | Limine BIOS/UEFI x86_64 `[S][R]` | Limine recipes for four arches, source only x86_64/RISC-V `[S][R partial]` | Limine x86_64 `[S][R]` |
| Memory | PMM, per-task spaces, heap, user validation `[S][R][T]` | fixed PMM/MemPool, VMM clone/frame caps `[S][R][T]` | transactional mapping, W^X, shootdown contracts `[S][R][T]` | PMM/slab/VMM/fault/shootdown `[S][R]` | VMO/address-space objects `[S][R]` | fixed pools, no general kernel heap `[C][S][R]` | bitmap PMM, slab, per-process tables `[S][R]` | freelist PMM, slab, VMM `[S][R]` | bitmap PMM, four-level page maps `[S][R]` |
| Scheduler/processes | preemptive tasks; special ring-3 tenants `[S][R][T]` | O(1) ready queues, RMS/sporadic server/deadlines `[S][R][T]` | per-CPU ownership, priorities/donation/fairness `[S][R][T]` | tasks/processes/signals/timers `[S][R]` | global earliest-slice scan `[S][R]` | per-core round-robin task lists `[S][R]` | SMP round-robin, fork/exec/wait partial `[S][R]` | per-CPU FIFO kernel threads `[S][R]` | SMP shared scan, ELF tasks/threads `[S][R]` |
| IPC/capabilities | opaque cap slots, channels, attenuated subagents `[S][R][T]` | CSpace/CNodes, revoke, queued/sync IPC `[S][R][T]` | handles/endpoints/call-reply/shared regions `[S][R][T]` | Unix IPC/syscalls, not capability-based `[S][R]` | object capabilities + bounded channel transfer `[S][R]` | typed mailboxes and event callbacks `[S][R]` | AF_UNIX streams/ring buffers `[S][R]` | none beyond locks/queues `[C] absent` | TTL message array, streams, shared memory `[S][R]` |
| Security | caps, grammar, taint/citation, signatures, audit `[S][R][T]` | CSpace derivation/revoke, owner-tagged frames `[S][R][T]` | authority contracts, W^X, authenticated providers `[S][R][T]` | Rust + process isolation; conventional Unix model `[S][R]` | pledge + object caps `[S][R]` | resource IDs/mailboxes; weak address-space story `[S]` | ring 3 but little validation/permissions `[S]` | kernel-only; no user boundary `[C] absent` | ring 3 syscall table; no fine-grained policy `[S][R]` |
| Storage/VFS | hierarchical store, FAT/ext2/virtio/block paths `[S][R][T]` | VFS, initrd/devfs/procfs/tmpfs/FAT32/pipes `[S][R][T]` | vfsd/storaged, FAT and provider contracts `[S][R][T]` | VFS, tmpfs/devtmpfs/initramfs/ext2 driver `[S][R]` | bootfs + userspace FS service `[S][R]` | message FS protocol; read-only ext2 service `[S][R]` | USTAR root + TempFS, no persistent FS `[S][R]` | RAMFS + fragile USTAR importer `[S][R]` | memory-backed VFS image from Fern-Night `[S][R]` |
| Network | smoltcp TCP/IP, TLS, HTTP/WS/SSH/MCP `[S][R][T]` | ARP/IPv4/ICMP/UDP; no TCP `[S][R][T]` | netd AF_UNIX + smoltcp AF_INET `[S][R][T]` | Ethernet/ARP/IPv4/ICMP/TCP/UDP/raw/local `[S][R]` | API stubs only `[S]` | RTL8139/virtio net + simple protocol utilities/apps `[S][R]` | local sockets only `[S][R]` | none | none |
| Graphics/media | compositor/browser/audio/image/video/voice `[S][R][T]` | framebuffer/console only | Wayland UI server + GPU provider contracts `[S][R][T partial]` | DRM/evdev/virtio-GPU/plainfb/fbcon `[S][R]` | compositor/UI in external stack `[C][R external]` | BGA/VGA/i915/audio + userspace WM `[S][R partial]` | framebuffer WM, shared images, terminal, Doom `[S][R]` | flanterm + incomplete virtio-GPU query `[S][R]` | framebuffer service + Fern-Night WM `[S][R]` |
| Agent runtime | native defining feature `[S][R][T]` | none | AI-oriented development contracts, not an in-OS agent runtime | none | none | actor model, not LLM agents | none | none | none |
| Verification | dual-arch in-kernel CI + extensive local E2E scenarios `[T]` | 926 checked-in pass claim + broad in-kernel tests `[C][T]` | host/OS/formal/fuzz/model/conformance gates `[T]` | fmt/clippy/x86 compile `[T]` | host tests via fetched deps; formatting non-blocking `[T]` | Travis `make` only `[T]` | lint only; no tests | make test target but no test files; boot assertions `[T weak]` | none; build failure masked |

## Repository audits

### 1. ChittiOS — agent-native kernel with unusually broad vertical scope

#### Architecture and boot

- `[C][S][R]` The README describes “agents as processes” without POSIX, libc, or a general ELF ABI. The actual module graph in `kernel/src/lib.rs:44-158` wires architecture, PMM/VMM, scheduler, IPC, capability tables, Synapse, services, UI, network, media, browser, skills, and model runtime into one kernel.
- `[S][R][T]` CI builds and boots in-kernel unit suites for both x86_64 and AArch64 (`.github/workflows/ci.yml:39-121`). End-to-end boot scenarios are intentionally excluded from CI (`.github/workflows/ci.yml:3-7`) and live in `tests/e2e/run.py` with the scenario contract in `tests/e2e/README.md`.
- `[S][R]` Per-task address spaces share a kernel half and validate a private user range (`kernel/src/mm/space.rs:1-26`, `AddressSpace::new` and `validate_user_range` around `:42-100`). PMM, heap, and architecture-specific mapping are centralized under `kernel/src/mm/mod.rs`; its real spinlock disables interrupts (`:22-63`).
- `[S][R]` The scheduler provides kernel tasks and preemption (`kernel/src/sched/mod.rs`). IPC endpoints carry bounded `u64` words (`kernel/src/ipc/mod.rs`); `kernel/src/channel/mod.rs` layers stream/datagram channels and adopted TCP handles over capability checks.

#### Agent execution and security mechanisms

- `[S][R][T]` The core pipeline is registry -> grammar-constrained decoding -> capability/taint gate -> deterministic executor -> audit (`kernel/src/synapse/mod.rs:1-20`). Tools are numeric primitives registered in `kernel/src/synapse/registry.rs:139-141`; the registry includes filesystem, subagent, channel, network, UI, and audio operations.
- `[S][R]` Capabilities are opaque indexes in a per-task table; there is no ambient global handle namespace (`kernel/src/cap/mod.rs:1-6`). `kernel/src/agent/orchestrator.rs:111-203` owns the orchestrator table, maps grants into taint sources, and can replay an already compiled plan deterministically.
- `[S][R][T]` Subagents receive an attenuated subset, run in their own capability-owning task, and have grants revoked on completion (`kernel/src/agent/subagent.rs:66-163`). The widening-refusal test is at `kernel/src/agent/subagent.rs:259-273`.
- `[S][R]` Taint tracking blocks exfiltration according to input provenance, but it is intentionally syntactic per value rather than whole-turn information-flow control (`kernel/src/security/taint.rs:93-107`). Citation gating is a separate check in `kernel/src/security/citation.rs`.
- `[S][R]` Skill installation verifies an ECDSA P-256 signature, obtains consent, intersects approved and requested grants, persists a ceiling, and assigns an `/agent/<id>/**` sandbox (`kernel/src/skills/install.rs:72-168`). This is the strongest install-time least-authority design in the set.
- `[S][R]` The audit log is a MAC-linked bounded append structure (`kernel/src/synapse/audit.rs:14-29,113-160,199-267`). It keeps at most 16,384 records and compacts the oldest half; only the head is persisted. The session key is not TPM-bound. “Permanent audit” should therefore be read as tamper-evident within this software boundary, not immutable external attestation.
- `[S][R]` Native services are deterministic loops supervised with bounded restart behavior (`kernel/src/service/mod.rs:1-12,47-102`). This is a reusable bridge between process-style isolation and small kernel-owned policy engines.

#### Files, drivers, network, graphics, and userspace

- `[S][R][T]` Source exists for hierarchical store/filesystem operations, FAT/ext2, block and virtio devices, PCI/ACPI, USB HID, framebuffer/virtio GPU, Ethernet, smoltcp, DNS/DHCP, TCP, TLS, HTTP, WebSocket, SSH, MCP, browser, image/PDF/media/video, audio, voice, compositor, terminal, and local/remote model execution. The subsystem index in `CLAUDE.md:1041-1180` and module list in `kernel/src/lib.rs:44-158` show build reachability; E2E scenario names in `tests/e2e/run.py` cover many of these paths.
- `[C]` `HARDWARE.md` usefully distinguishes tested from speculative hardware. I2C is partial/unverified (`HARDWARE.md:79-89`), SDHCI and several laptop paths are unverified (`:90-112,221-240`), VMSVGA MMIO is unverified (`:127-139`), client TLS lacks CRL/OCSP (`:140-153`), and TLS server support is absent (`:240`). Wi-Fi association, modern Bluetooth pairing, brightness control, and general GPU acceleration are explicitly missing.
- `[S][R]` General ring-3 execution is narrower than the architecture language suggests. `CLAUDE.md:43-114` establishes a ring-3 rule but explicitly leaves the orchestrator in ring 0. Image decoders use checked-in x86/AArch64 flat binaries rather than a general ELF loader (`kernel/src/synapse/tenant.rs:1299-1319`), and a shared userspace slot serializes invocations (`tenant.rs:2007-2058`).

#### Strengths, gaps, and lessons

Best: make an agent plan a typed object; constrain decoding to the tool grammar; check capability, scope, taint, and consent before execution; attenuate subagents; sign installable skills; audit every boundary crossing.
Gap: the trusted computing base is enormous, the root orchestrator contradicts a strong “agent process” reading, parser isolation is specialized, and audit durability/attestation is limited.
zlOS lesson: copy the **protocol shape**, not the placement. Put model/orchestrator/browser/media code in ordinary processes. Keep only opaque handles, typed messages, grant attenuation, revocation, and a deterministic broker near the kernel boundary.

### 2. NexiOS — deterministic RTOS engineering and CSpace work in progress

#### Architecture and execution

- `[C][S][R]` NexiOS is a freestanding C++20 monolithic RTOS for x86_64 with an `int 0x82` syscall ABI and current CSpace support (`README.md:25-49`). ARM64/RISC-V have source scaffolding but x86_64 is the production path.
- `[S][R][T]` `src/kernel/task/scheduler.cpp` uses ready queues plus a bitmap for O(1)-style selection (`enqueue_ready` near `:206`), tick/deadline accounting (`on_tick` near `:1049`), rate-monotonic selection (`rate_monotonic_schedule` near `:2462`), and rescheduling (`reschedule` near `:2555`). `src/kernel/task/sporadic_server.cpp:30-194` tracks execution budget and replenishments.
- `[S][R][T]` IPC supports queued send/receive, endpoint capabilities, synchronous send, and blocked senders (`src/kernel/ipc/ipc.cpp`, `send:165`, `recv:262`, cap paths `:277-322`, `send_sync:336`, blocked queues `:463-511`). Priority ordering for blocked senders remains a roadmap item.
- `[S][R][T]` CSpace is a real CNode/slot graph with generations, reference checks, copy/mint/grant, remove, and bounded iterative revoke (`src/kernel/cap/cap.cpp:37-305`). Untyped retyping is currently frame-only and exact-sized (`src/kernel/cap/untyped.cpp:42-143`); child untyped derivation, IRQ/MMIO/IOMMU capabilities, and userspace drivers are open.

#### Memory, VFS, drivers, networking, and userspace

- `[S][R][T]` PMM uses static owner-tagged page pools and contiguous allocation (`src/kernel/memory/pmm.cpp`); `src/kernel/memory/mempool.cpp:37-439` supplies fixed slabs so critical paths avoid general allocation. VMM has architecture variants, cloning/deep-copy, and cap-authorized frame maps (`src/kernel/memory/vmm.cpp`, `map_frame_cap` around `:1232`).
- `[S][R][T]` A dedicated kernel ELF-loader task accepts chunked, cancelable requests (`src/kernel/elf/elf_loader.cpp:94-578`). The `runelf` user hook described in `docs/specs/elf-loader.md:170` is future work, so “background loader exists” is stronger than “general user execution is complete.”
- `[S][R]` VFS source covers initrd, devfs, procfs, tmpfs, FAT32, and pipes (`src/kernel/vfs/`). `vfsd` is still a kernel service task, despite future userspace-service language.
- `[S][R][T]` Drivers include AHCI, ATA PIO, virtio block/net, PCI, DMA, serial, framebuffer, and timers. Networking implements Ethernet/ARP/IPv4/ICMP/UDP structures; TCP is absent and explicitly future work. USB, audio, and a desktop stack are absent.
- `[S]` ARM/RISC-V signal delivery still terminates rather than delivering a user signal (`src/kernel/kernel.cpp:1298-1311`), a concrete limit on multiarchitecture parity.

#### Verification and claim audit

- `[T]` Roughly 140 files under `src/kernel/test/` cover scheduler, IPC, caps, allocation, VFS, drivers, ELF, timing, and failure paths. ResourceTracker and snapshot/rewind infrastructure let tests restore global state instead of accumulating contamination.
- `[C][T]` `test-history.txt:333-335` records 2026-08-16 results of `926 passed, 0 failed`, plus release-all `85/0` and selftest `133/0`. This is checked-in evidence, not a result reproduced here. README's `881` badge is stale rather than stronger evidence.
- `[C]` `ROADMAP.md:10-28,66-127` still marks child-untyped objects, IRQ/MMIO/IOMMU capabilities, register IPC, external pager, supervision, shared memory, priority-ordered senders, timer wheel, enforced admission, SMP, and IRQ-driven block I/O open. Later microkernel and userspace-driver phases are plans, not current architecture.
- `[C]` `BUGS.md` and `ROADMAP.md:55-60` disagree about some ELF/PML4/driver races; the roadmap marks them resolved while the bug file still presents open H2 risks. Treat those claims as stale until one canonical hazard ledger replaces both.

Best: named deadlines and WCET budgets, fixed allocation domains, explicit state transitions, capability derivation, bounded revoke, deterministic snapshots, and a large adversarial kernel test corpus.
Gap: “hard real-time” is not demonstrated across single-core tick scheduling, polled I/O, advisory admission, unbounded waits, and incomplete high-resolution timers. Current architecture is monolithic despite a microkernel roadmap.
zlOS lesson: adopt a **boundedness budget** for every kernel operation—allocation source, maximum queue length, maximum traversal depth, maximum IRQ-off time, timeout behavior—and leave a test that exhausts each bound.

### 3. RustOS — strongest contracts and formal-evidence discipline

#### Architecture

- `[S][R][T]` Boot validates BSP state, firmware inputs, higher-half layout, and AP handoff in `kernel/src/main.rs:1-18,74` and `kernel/executive/src/boot.rs:1-16`; `initialize_application_processors` is near `boot.rs:371` and service bootstrap near `:666`.
- `[C][S][R]` Ring 0 is intended to provide mechanisms while policy lives in rootd, syscalld, vfsd, loaderd, netd, devmgrd, storaged, inputd, procd, sessiond, runtimed, and uiserver. Service ownership and first-start order are recorded in `services/AGENTS.md`; `docs/architecture/microkernel-overview.md` makes a Linux Driver VM the device-compatibility domain and specifies fail-closed behavior when it is unavailable.
- `[S][R][T]` Address-space updates admit a complete span before committing, reject overflow and W+X, and preserve failure atomicity (`kernel/mm/src/memory/address_space.rs:1-16`).
- `[S][R][T]` The SMP scheduler records exact continuation ownership, per-CPU current slots, bounded priority donation, and System/User/Idle scheduling classes (`kernel/ps/src/multitask/scheduler.rs:1-24,159-169,233-288`). IPC provides bounded endpoints, call/reply custody, handle transfer, and shared regions (`kernel/ipc-runtime/src/ipc/mod.rs`); object identity and handles live under `kernel/object/`.
- `[S][R]` Linux syscall compatibility has a single source in `kernel/ps/src/user/linux.rs` and delegates policy to service brokers. Loaderd has separate ELF and PE loaders (`services/loaderd/src/elf.rs`, `pe_loader.rs`) with shared overlap/overflow/W+X admission rules.
- `[S][R][T]` Netd implements AF_UNIX and smoltcp-backed AF_INET endpoints with authenticated provider input and deadlines (`services/netd/src/main.rs`). Uiserver holds bounded per-client Wayland state (`services/uiserver/src/wayland.rs:1-16`) for compositor, output, seat, pointer, keyboard, shared-memory, surface, and xdg objects. GPU runtime deliberately does not silently convert provider failure into a fake CPU-success path (`services/uiserver/src/gpu_runtime.rs:1-18`).

#### Verification, diagnostics, and current failures

- `[T]` PR CI runs script hygiene, hook selftests, format, config validation, `cargo xtask check`, selected host tests, formal-impact mapping, Kani/Verus/TLA+ verification, and docs builds (`.github/workflows/rust.yml:51-160`). Nightly adds deterministic/randomized formal gates and preserves evidence (`.github/workflows/formal-nightly.yml:39-60`).
- `[T]` `formal/` contains a large model catalogue, source-conformance links, mutation tests, TLA+, Kani, Verus, Miri, fuzz, Apalache, and TLAPS tooling. `formal/COVERAGE.md:57-72` explicitly says models do not prove the implementation or certification. That honesty is a feature.
- `[C][T]` Commercial gate documentation records failures, not just wins. Parser/adversarial, multicore/SMP, physical-GPU, cold-boot, reset/revoke, standard-Wayland-client performance, and capture/recovery gates remain failed or incomplete (`docs/ai/commercial-quality-gates.md:61-123`; `formal/COVERAGE.md:178,317-343`). Physical scanout reached a real-frame path but showed corruption and failed revoke/recovery capture.
- `[C][R]` `Cargo.toml` intentionally divides default members because a naive workspace build unifies incompatible std/no_std features. `docs/guides/build-artifacts.md:93-114` says `cargo xtask check` is the supported whole-tree gate.

Best: authority maps, exact state ownership, terminal-reply custody, bounded waits, fail-closed providers, source/model conformance, mutation testing, and receipts that retain failed evidence.
Gap: very high architectural and process complexity; formal-model presence can look like implementation proof if `formal/COVERAGE.md` is not read; several product-defining physical/runtime gates currently fail.
Agent-system lesson: copy the **intent -> contract -> bounded execution -> deterministic check -> independent review -> evidence receipt** chain. Every receipt should say what was not tested and preserve failing artifacts.

### 4. Zinnia — compact modular Unix kernel and driver crates

#### Architecture and features

- `[C][S][R]` Zinnia describes itself as a modular 64-bit Unix-like Rust kernel with most POSIX calls plus epoll/timerfd and DRM/evdev. It explicitly ships kernel and drivers, not a full bootable distribution; a separate bootstrap project creates images (`README.md`).
- `[S][R]` `kernel/src/lib.rs:31-50` registers architecture, memory, task, process, scheduler, syscall, VFS, network, executable loading, module, device, DRM, evdev, TTY, and support modules. Kernel initialization mounts initramfs/ramdisks, opens `/init`, attaches TTY stdio, creates a process, and executes it.
- `[S][R]` Memory includes PMM, slab/virtual allocation, per-address-space mappings, fault handling, and TLB shootdown (`kernel/src/memory/`, especially `memory/virt/fault.rs` and `shootdown.rs`). Processes/tasks provide signals, interval timers, and scheduler integration (`kernel/src/process/`, `kernel/src/sched/`); ELF/shebang execution lives in `kernel/src/vfs/exec/`.
- `[S][R]` Syscall groups cover memory, modules, process, signals, sockets, system calls, and VFS (`kernel/src/syscall/`). VFS has tmpfs, devtmpfs, initramfs, anonymous objects, pipes, epoll, eventfd, signalfd, and timerfd (`kernel/src/vfs/`); TTY and PTY live in `kernel/src/device/tty/`.
- `[S][R]` Network source implements Ethernet, ARP, IPv4, ICMP, TCP, UDP, raw, local, loopback, and socket layers (`kernel/src/device/net/`).
- `[S][R]` Narrow driver crates implement NVMe, virtio block/network/GPU, ext2, Intel IGC, xHCI, USB HID, and USB storage (`drivers/`). Kernel interfaces expose device, DRM, evdev, plain framebuffer/fbcon, and TTY rather than embedding desktop policy.

#### Evidence and gaps

- `[T]` CI checks format/clippy and compiles one x86_64 target (`.github/workflows/ci.yml:10-44`). Other architectures are commented out at `:28-29`; there is no QEMU boot or runtime test job.
- `[S]` Architecture source is substantive for x86_64 and partial RISC-V. AArch64/LoongArch target descriptions do not equal implemented ports.
- `[S]` The supplied checkout lacks the uACPI submodule, so its normal recursive build is not locally ready. No broad automated test tree is present.
- Gaps: no complete desktop/userspace distribution, AHCI/audio/Wi-Fi, capability model, agent runtime, runtime CI, or broad adversarial tests.

Best: small typed interfaces, narrow driver crates, kernel-to-driver dependency direction, an init graph, and broad conventional Unix object coverage without pretending the kernel is a full OS product.
zlOS lesson: make each driver a separately compiled module with an explicit registration descriptor and minimal shared ABI. Generate the build/module inventory from those descriptors so a source file cannot silently exist outside the shipped image.

### 5. Skift — object capabilities and userspace services, with an external-source caveat

#### What is actually in this checkout

- `[C]` The README advertises Karm, KarmUI, Hideo, Hjert, CuteKit, and Vaev. This checkout has only 260 tracked top-level/project files and declares most framework source as external projects in `project.json`; exact revisions are pinned in `project.lock`. `skift.sh install` fetches them before the Linux CI build (`.github/workflows/checks-linux.yml:19-38`). Advertised browser/UI breadth is therefore real only after external dependency resolution, not all present in this repository.
- `[S][R]` Hjert implements domains, tasks, address spaces, VMOs, I/O ports, channels, IRQs, listeners, pipes, clocks, and jobs. The syscall object operations are enumerated in `src/kernel/hjert/core/syscalls.cpp:27-510`.
- `[S][R]` Tasks carry mode, context/stack, job, address space, capability domain, blocker, and a monotonically reducing pledge (`src/kernel/hjert/core/task.cpp:24-173`). Scheduling scans a global vector and chooses the earliest slice end (`src/kernel/hjert/core/sched.cpp:16-64`); it is simple and not SMP-scalable.
- `[S][R]` Domains are hierarchical slot arrays (`src/kernel/hjert/core/domain.cpp:15-107`). Channels preflight bounded byte and capability capacity, transfer reference-counted kernel objects, and roll back atomically (`src/kernel/hjert/core/channel.cpp:12-150`).
- `[S][R]` Userspace strata include device, filesystem, input, power, and shell/compositor services (`src/srvs/strata-*`). Device service enumerates ISA/PCI/PS2 (`src/srvs/strata-device/`); `src/libs/karm-sys/skift/bootfs.cpp` and `src/srvs/strata-fs/` supply storage policy.

#### Concrete defect and incomplete APIs

- **`[S] Apparent critical bug`**: `Domain::_availableUnlocked()` increments its counter when a slot is occupied (`src/kernel/hjert/core/domain.cpp:98-106`). Channel receive treats that value as free destination slots (`src/kernel/hjert/core/channel.cpp:105-116`). An empty domain therefore reports zero capacity and rejects any received capability; a full domain reports high capacity and can reach an invalid unwrap. The name, caller, and predicate all agree this is inverted. It was not runtime-reproduced here.
- `[S]` The Skift backend still returns unsupported/TODO for pipe creation, directory creation, spawn/PTY, generic mmap, memory flush, current time/uptime, memory/CPU/user info, sandboxing, DNS/bundles, and all TCP/UDP operations (`src/libs/karm-sys/skift/sys.cpp:67-139,153-170,203-238,264-282`). Networking is not a current feature of the tracked OS source.
- `[T]` Linux CI builds kernel, EFI, target userspace, host userspace, and fetched host tests. Its formatting step is explicitly non-blocking via `|| echo` (`.github/workflows/checks-linux.yml:40-41`). No in-tree Hjert test suite was found.

Best: capabilities are object references rather than integers with ambient meaning; channel cap transfer is designed as one atomic transaction; services sit above a small object syscall layer; external project revisions are locked.
Gap: dependency spread complicates auditability, current networking/system APIs are incomplete, the scheduler is global/linear, and the capability-transfer defect undermines a core path.
zlOS lesson: use atomic **preflight -> reserve -> move references -> commit/rollback** for handle transfer, but add empty/full/boundary property tests before relying on it.

### 6. Fudge — actor-style mailboxes with tiny kernel policy

#### Architecture and mechanisms

- `[C][S][R]` Fudge deliberately makes mailbox messages the universal coordination primitive. Every program has a channel, binds callbacks by event type, drops unhandled events, and may poll manually (`README.md:5-27`; `lib/abi/channel.c`).
- `[C][S][R]` The kernel avoids general dynamic allocation and uses preallocated core, node, mailbox, and task rows. `pool_setup` initializes fixed free lists and caller-provided mailbox backing memory (`src/kernel/pool.c:291-346`). This makes exhaustion behavior inspectable, although pool limits are global and static.
- `[S][R]` Tasks have a small explicit DEAD/NEW/ASSIGNED/RUNNING/BLOCKED/UNBLOCKED state machine (`src/kernel/task.c:33-127`). `kernel_schedule` returns the current task to its core list, handles kill/block signals, scans blocked tasks, and picks the next list item (`src/kernel/kernel.c:263-323`). It is round-robin-like, with per-core lists and no priorities/deadlines.
- `[S][R]` Nodes route events from source resources to linked mailbox endpoints under locks (`src/kernel/kernel.c:341-395`). `kernel_loadtask` resets a task, locates a registered binary format, assigns mailbox 0, and moves the task into the scheduler (`kernel.c:397-435`).
- `[S][R]` Loadable kernel modules provide small device-facing pieces. Default configuration wires APIC/PIC/PIT/RTC/PS2, PCI, IDE/AHCI/NVMe/ATA, BGA/VGA, RTL8139, virtio-network, UART, USB controllers, i915, audio/video, block and console modules (`config/base.slang`, `config/arch.slang`). File presence and configuration reachability do not prove hardware maturity.
- `[S][R]` Higher-level policy is in userspace: ELF loading/job pipelines (`src/utils/elfload.c`, `lib/abi/job.c`), filesystem messages (`lib/abi/fs.c`), ext2 server (`src/utils/ext2srv.c`), shell/window manager/widgets, IRC/netcat, Game Boy emulator, and utilities.
- `[S]` Ext2 list/read/walk operations are implemented, but the write callback is empty while still bound (`src/utils/ext2srv.c:462-496`). Treat ext2 as read-only.

#### Evidence and gaps

- `[T weak]` `.travis.yml:1-5` runs only `make` with GCC and Clang. `src/test/` contains manual test programs (disk, loop, timer, graphics), not an automated assertion suite.
- `[C]` x86 is the normal build. `README.arm.md` and `README.riscv.md` describe earlier/partial ports rather than parity.
- Gaps: no capability attenuation, per-process security policy, durable audit, modern network stack/TLS, automated tests, clear memory protection evidence, or bounded latency contracts. Many drivers are minimal/polled.

Best: define the message/event protocol once, make services reactive, keep hardware modules narrow, and let userspace assemble pipelines.
zlOS lesson: use a typed event envelope (`protocol`, `version`, `event`, `request_id`, `source_handle`, bounded payload) and generated dispatch tables. Unlike Fudge, isolate services in separate address spaces and return explicit “unsupported” responses instead of silently dropping unknown messages.

### 7. TacOS — connected toy Unix desktop with useful small-system seams

#### Boot, memory, processes, IPC, files, and UI

- `[C][S][R]` TacOS openly calls itself a hobby toy with known bugs while claiming VFS, scheduler, TempFS, devices, context switching, virtual/physical memory, and Doom (`README.md:1-10`). The normal Makefile builds bootloader, kernel, libc, libraries, userspace, USTAR initrd, and a Limine BIOS/UEFI ISO (`Makefile:1-61`).
- `[S][R]` Boot performs PMM, GDT/IDT/SSE, paging, ACPI/APIC/PIT, VFS/initrd/devices, memory regions, scheduler, IPC/syscalls/PTY, PCI, framebuffer/TTY/keyboard, `/usr/bin/init`, LAPIC timer, then AP startup (`kernel/src/entry.c:99-137`).
- `[S][R]` The scheduler allocates task objects from a slab and walks a circular list, skipping absent/waiting/already-running tasks; a spinlock coordinates CPU selection (`kernel/src/tasks/scheduler.c:10-66`). SMP and per-task page tables exist, but this is an unprioritized global round robin.
- `[S][R]` `fork` clones resources, cwd, child state, page maps/memory regions, kernel stack, and call frame (`kernel/src/tasks/fork.c:17-91`). ELF `execve` loads x86_64 PT_LOAD segments and constructs argv/envp (`kernel/src/tasks/exec.c:14-120`). Syscalls include process, files, memory mapping, timing, PTY, shared memory, and local socket calls (`kernel/src/tasks/syscalls.c:480-517`).
- `[S][R]` AF_UNIX/SOCK_STREAM sockets use two ring buffers and TempFS pathname anchors (`kernel/src/tasks/ipc.c:12-158`). Blocking `accept` is a busy loop (`ipc.c:44-55`), with no network protocols.
- `[S][R]` VFS mounts USTAR root and TempFS/device nodes (`kernel/src/fs/`). It has cwd and path walking but explicitly calls its design clunky (`kernel/src/fs/vfs.c:1-6`). TempFS remove-directory/remove-file are TODO (`kernel/src/fs/tempfs.c:194-200`).
- `[S][R]` Userspace init forks a shell (`userspace/init/main.c:52-65`). A framebuffer window server accepts local-socket commands, assigns shared-memory images, composites a double buffer, and forwards keyboard events (`userspace/wm/main.c:30-115,163-214,277-350`). GTerm connects the WM library to a PTY and shell (`userspace/gterm/main.c:31-110`); Doom is built into the initrd.

#### Concrete defects and gaps

- **`[S] Apparent high-severity bug`**: ELF segment permission logic adds `KERNEL_PFLAG_WRITE` when the ELF segment is *not* writable (`kernel/src/tasks/exec.c:109-118`). Unless the flag name has inverted hardware semantics elsewhere, read-only code/data become writable while requested writable segments may not. No runtime reproduction was performed.
- `[S]` NVMe warns not to use it as a learning resource, calls itself incomplete/broken/given up, supports only the admin queue, and sends only an identify-like test (`kernel/src/drivers/nvme.c:3-8,117-157,189-192`). It is not a storage feature.
- `[S]` `wait`, process groups/signals, link/stat, clocks/times, TempFS deletion/truncate, and much libc remain TODO (`kernel/src/tasks/syscalls.c:76-283,454`; `libc/src/`). User pointers and fd indexes frequently lack systematic boundary validation.
- `[T weak]` `make lint` formats source in place and checks one signature convention (`Makefile:66-73`); there is no CI or automated runtime suite. `userspace/ipctest` and `segfault` are manual probes.

Best: the whole user-visible path is small enough to trace from boot to init to shell to WM to PTY; local sockets plus shared images are a clear compositor seam.
Gap: weak syscall validation/security, polling, unfinished lifecycle/filesystems/drivers, no network, and no automated gates.
zlOS lesson: preserve this end-to-end simplicity as a smoke image—boot, start init, open terminal, create a file, restart, and verify persistence—but back it with deterministic serial assertions and negative syscall tests.

### 8. NyauxKC — rewrite-stage kernel with honest roadmap gaps

#### Architecture and current reachability

- `[C]` The README says this is an in-progress Rust rewrite. Its checklist marks PMM, slab, x86_64/RISC-V page tables, VMM, VFS/RAMFS complete; USTAR partial; syscalls, user-thread scheduling, ELF, Xorg, and Wayland absent (`README.md:6-27`). That is a substantially accurate boundary.
- `[S][R]` Limine provides HHDM, RSDP, command line, modules, framebuffer, paging-mode, and SMP responses (`kernel/src/main.rs:29-90`; `kernel/src/arch/mod.rs:20-33`). Boot initializes PMM, BSP architecture, VMM, flanterm, uACPI, optional Intel IOMMU setup, CPU-local state, timers, APs, and the scheduler (`kernel/src/main.rs:99-193`).
- `[S][R]` Scheduler threads own separate normal/interrupt stacks and a timeslice. `sched_yield` pushes the old thread to a per-CPU FIFO, chooses the next or idle thread, arms a timer, and context switches (`kernel/src/scheduler/mod.rs:16-139`). Only a kernel thread is created; no processes or user mode exist.
- `[S][R]` VFS decomposes `vfs`, `vnode`, `dentry`, `vfsops`, and `vops`, with mount/unmount swapping vnode operations (`kernel/src/vfs/mod.rs:12-115`). RAMFS supports directory lookup, create/mkdir, byte-vector read/write (`kernel/src/vfs/ramfs.rs:12-108`), and boot imports a Limine USTAR module (`kernel/src/vfs/ustar.rs:41-147`).
- `[S][R partial]` PCI ECAM enumeration and a virtio-GPU queue/feature-negotiation/display-info path exist (`kernel/src/pci/mod.rs:369-462`; `kernel/src/virtio/mod.rs:393-625`). The driver stops after querying modes and does not expose display surfaces.
- `[S]` Intel IOMMU parsing and root/context table setup exist, but actual context page tables are explicitly TODO (`kernel/src/arch/x86_64/intel/iommu/mod.rs:159`).

#### Concrete defects and gaps

- **`[S] Apparent high-severity bug`**: PCI locations are stored as `(bus, slot, function)` (`kernel/src/pci/mod.rs:383-409`) and read helpers accept that order (`:14-60`), but `pci_map_bar` calls them as `(location.2, location.1, location.0)` (`:65-75`). Nonzero buses/functions therefore map the wrong configuration address; even the common bus-0/function-0 case hides the error.
- `[S]` USTAR parsing can slice `ptr[..1024]` without first proving 1024 bytes remain, rejects empty files with `panic!("?")`, ignores checksum/link semantics, and uses unchecked unwraps (`kernel/src/vfs/ustar.rs:65-77,89-147`). It is not a robust trust-boundary parser.
- `[S]` Thread stacks are mapped writable and executable (`kernel/src/scheduler/mod.rs:26-32`), so W^X is absent even before user mode.
- `[R partial]` The root Makefile offers x86_64, AArch64, RISC-V, and LoongArch images (`GNUmakefile:34-143`), but `kernel/src/arch/mod.rs:16-19` only defines RISC-V and x86_64 modules. Build recipes are not port evidence.
- `[T weak]` `kernel/GNUmakefile:37-39` has a Cargo test target, but no test files were found. Boot-time slab/VMM assertions in `kernel/src/main.rs:146-179` are the only direct self-checks.

Best: honest scope, clear CPU-local queue decomposition, and small VFS trait boundaries.
Gap: no user boundary, syscalls, executable loader, IPC, network, storage, mature graphics, security, or durable tests; multiple parser/PCI correctness issues.
zlOS lesson: treat every firmware, PCI, archive, and executable parser as hostile input. Require checked length arithmetic, property/fuzz tests, and a typed `PciLocation` struct so tuple-field reversal is impossible.

### 9. Cyjon — pure-assembly SMP kernel plus Fern-Night userspace image

#### Architecture and features

- `[C][S][R]` Cyjon is a readability-first x86_64 assembly kernel (`README.md:9-28`). `kernel/init.asm:9-98` includes every kernel/library module into one NASM translation unit; boot initializes Limine, environment, binary memory map, ACPI, paging, GDT/IDT, streams, tasks, IPC, RTC, PS/2, storage, libraries, daemons, init, and SMP (`kernel/init.asm:107-184`).
- `[S][R]` Memory uses a bit-per-page availability map and first-fit contiguous scanning under a semaphore (`kernel/memory.asm:5-155`), cleans pages on release (`:157-220`), and supports mapping shared memory into another task (`:222-267`). Page code walks four-level tables, allocates/maps/removes/clones/merges them (`kernel/page.asm`; `kernel_page_address:11`, `kernel_page_alloc:94`, `kernel_page_clang:163`).
- `[S][R]` The APIC timer scheduler saves GPR/FPU state, uses LAPIC ID to find the current task, scans a shared fixed task array for active, awake, non-executing work, marks it CPU-owned, switches CR3, and restores state (`kernel/task.asm:9-204`). A source comment documents a current AP initialization race and a bypass (`task.asm:53-63`).
- `[S][R]` The syscall dispatcher bounds-checks an index and calls a service table before `sysret` (`kernel/syscall.asm:10-60`). Services cover exit, framebuffer, memory alloc/release/share, PID/status, mouse, storage read, exec, IPC send/receive, serial, sleep, uptime, task creation, memory info, and thread creation (`kernel/service.asm:10-38`).
- `[S][R]` IPC is a fixed global array of messages with source, target, type, payload, and TTL (`kernel/service.asm`, `kernel_service_ipc_send:273`, `receive:354`). Streams are bounded ring-like task resources (`kernel/stream.asm`).
- `[S][R]` ELF execution loads from a memory-backed VFS image, validates ELF type, maps load segments, resolves shared-library relocations, creates task/stack/page maps, and attaches streams (`kernel/exec.asm`, `kernel_exec:13`, `configure:155`, `link:440`, `load:564`).
- `[S][R]` The top-level build first builds the Fern-Night submodule and copies its `system.vfs` beside the assembly kernel (`make:20-24,41-54`). Fern-Night supplies C daemons, shared libraries, shell, console, ps/ls/cat/uptime, framebuffer WM, taskbar/menu/clock, JSON/config/image/font utilities, and the VFS image. That is how Cyjon obtains userspace; it is not implemented in the parent repository's assembly sources.
- `[S][R]` Drivers are limited to serial, RTC, PS/2 keyboard/mouse, ACPI/IOAPIC/LAPIC, Limine framebuffer, and memory-backed storage. There is no block-disk, network, audio, USB, or GPU-acceleration stack.

#### Evidence, claims, and gaps

- **`[S] Build-trust defect`**: top-level `Makefile:5-6` invokes `./make || true`, so a failed assembler, linker, dependency, or ISO command still yields a successful `make` exit. The script itself contains proper `|| exit 1` for kernel assembly/linking (`make:41-43`), but the wrapper discards that status. No green build invocation is trustworthy until this is removed.
- `[T absent]` No tests or CI files exist. There are serial/log and QEMU/Bochs scripts, but no assertions or exit protocol.
- `[C][S]` README says changes in Cyjon and Fern-Night are reflected in each other (`README.md:1-5`), while the build consumes a pinned Fern-Night submodule (`.gitmodules:4-6`). Compatibility is a maintenance claim, not mechanically enforced equivalence.
- `[C][S]` README names v0.2165 (`README.md:9`) while source declares revision 2167 (`kernel/config.inc:5-9`), another stale orientation signal.
- `[C]` The custom license prohibits commercial use and distributed derivatives (`LICENSE:3-15`). This is not an OSI open-source license and rules out code adoption.

Best: architecture is explicit at instruction level; initialization order, fixed structures, IPC TTL, page ownership, and context switch can be followed without framework indirection.
Gap: no verification, coarse syscall/security policy, fixed global limits, spinlocks/polling, sparse drivers, source-doc drift, masked failures, and restrictive license.
zlOS lesson: retain readable initialization and state layouts, but generate ABI layouts and boot-step receipts from higher-level definitions. Never let a wrapper neutralize a lower gate's exit status.

## Cross-repository findings that matter most

### Source-level correctness findings requiring upstream confirmation

1. **Skift capability receive likely cannot work correctly**: occupied-slot count is used as destination free capacity (`domain.cpp:98-106` -> `channel.cpp:105-116`).
2. **TacOS ELF W permission appears inverted**: not-writable segments receive the write bit (`exec.c:109-118`).
3. **NyauxKC PCI BAR addressing appears tuple-reversed**: stored `(bus, slot, function)` becomes `(function, slot, bus)` (`pci/mod.rs:65-75,383-409`).
4. **Cyjon builds can be false-green**: `./make || true` masks every internal failure (`Makefile:5-6`).
5. **Chitti's “permanent” audit interpretation is too strong**: bounded body compacts and only head is persisted (`synapse/audit.rs:113-160,199-267`).
6. **TacOS NVMe is source presence only**: its own header says broken/given up and there is no I/O queue (`drivers/nvme.c:3-8,117-157`).
7. **NexiOS bug and roadmap ledgers conflict**: resolved race claims in `ROADMAP.md:55-60` coexist with open H2 descriptions in `BUGS.md`.

These are inspection findings, not `[V]` reproductions. They should become minimal isolated tests before any upstream report or local architectural dependency.

### Claim patterns to reject in future zlOS research

- “Has a driver” when only a file or PCI probe exists.
- “Supports an architecture” when only a target JSON, linker script, or QEMU recipe exists.
- “Microkernel” when policy services still execute as kernel tasks.
- “Agent as process” when the privileged orchestrator is ring 0 and general process loading is absent.
- “Formally verified” when a model exists but implementation conformance or physical gates fail.
- “Hard real-time” without measured worst-case paths, enforced admission, interrupt latency, and bounded I/O waits.
- “Tests pass” when the wrapper masks exit status or only a checked-in log says so.
- “Desktop/browser included” when source is fetched from unexamined external repositories.

## Best strategies, compressed

| Strategy | Best reference | Clean-room rule for zl/zlOS/agents |
|---|---|---|
| Typed, deterministic tool execution | Chitti | Decode only registered grammar; compile a plan; validate all handles/scopes; execute from plan; record result digest |
| Authority attenuation and revocation | Chitti + NexiOS + Skift | Per-process opaque handle table; child grant must be subset; derivation tree supports bounded revoke; atomic transfer rollback |
| Information provenance | Chitti | Attach provenance labels to values and artifacts; block external sinks unless source policy permits; do not claim full IFC |
| Bounded kernel behavior | NexiOS | Every queue/pool/traversal/wait declares a maximum and exhaustion result; test exact boundary and one beyond |
| Evidence and contracts | RustOS | Machine-readable owner/authority/state contract; deterministic check; independent review; receipt includes failures and untested scope |
| Driver modularity | Zinnia | One driver crate/module, narrow ABI, declarative match table, independent compile, explicit shipped-image inventory |
| Reactive services | Fudge | Typed/versioned messages and generated callback dispatch; separate address spaces; explicit unsupported/error response |
| Capability objects and transfer | Skift | Handles reference kernel objects, not global IDs; preflight source/destination capacity; commit all or rollback all |
| End-to-end product smoke | TacOS | Boot to init/shell/WM/files test in one small image with deterministic serial milestones and negative checks |
| Typed hardware identity | NyauxKC finding | Use named `Bus`, `Device`, `Function`, `Bar` fields; never transport hardware coordinates as same-typed tuples |
| Readable boot state | Cyjon | Explicit ordered boot graph and compact state layouts, but preserve failure codes and auto-check prerequisites |

## Integrated architecture plan for zlOS and the agent system

This is the full target architecture, staged by dependency rather than a deliberately reduced final design.

### Layer 0 — evidence contract before features

1. Define evidence states in machine-readable metadata: `claimed`, `source`, `built`, `booted`, `scenario_passed`, `hardware_passed`, `formally_modeled`, `implementation_conformant`.
2. Every module and feature names an owner, authority, initialization prerequisite, failure policy, build target, test, and evidence receipt.
3. Build scripts must use strict exit propagation. Produce an inventory proving every shipped module was compiled and linked. Add a “source exists but not shipped” report.
4. Receipts include commit, configuration, commands, exit codes, artifact hashes, serial/QEMU logs, screenshots where visual, failures, skipped gates, and the weakest link.

This prevents the most common failure seen across all nine repositories: vocabulary outrunning reachable, tested behavior.

### Layer 1 — kernel mechanisms and typed object ABI

1. Keep the zlOS kernel responsible for scheduling, address spaces, interrupts, page/object allocation, typed IPC, wait/poll, and handle tables—not filesystems, agent planning, browsers, or media policy.
2. Use per-process opaque handles with rights (`read`, `write`, `map`, `signal`, `transfer`, `derive`, `admin`) and object type tags. No ambient lookup by global integer.
3. Implement derivation and attenuation: child rights must be a subset; revocation walks a bounded derivation tree. Set an explicit maximum depth/count and return deterministic exhaustion errors.
4. IPC message = bounded bytes + bounded transferred handles + request/reply ID + protocol/version. Transfer is transactional with preflight and rollback.
5. Every syscall validates complete user spans before mutation, uses checked arithmetic, and commits atomically. Default W^X; executable mappings never writable.
6. Scheduler starts with fair per-CPU queues and explicit continuation ownership. Add priority/deadline classes only with donation, admission, timer, and IRQ latency tests—not only fields.

### Layer 2 — driver and service decomposition

1. Refactor each device into a narrow module descriptor: IDs, dependencies, resources, DMA/IOMMU requirements, probe/start/stop/recover operations, and diagnostic counters.
2. Put protocol-heavy and parser-heavy work in userspace services: filesystem, network, USB class, image/PDF/media, compositor, shell, and agent runtime.
3. Driver manager owns PCI/MMIO/IRQ/DMA handles and grants attenuated subsets. A driver cannot enumerate or map arbitrary hardware after startup.
4. Use typed hardware identities, not tuples. Parse firmware/config/archive inputs with fuzzed libraries and length-first admission.
5. Supervise each service with bounded restart and a clear state machine: `Absent -> Starting -> Ready -> Degraded -> Recovering -> Failed`. Never silently substitute a fake success path.

### Layer 3 — VFS, persistence, network, and desktop services

1. VFS broker resolves paths to file handles; filesystem services implement zlfs, initramfs, FAT/ext2 compatibility. Path resolution is capability-rooted rather than global-root ambient authority.
2. Preserve the existing zlfs/persistence work as the native store. Add crash-consistency receipts: save, power-cut/reboot, replay, verify; corrupt metadata; full disk; stale handle; concurrent rename/write.
3. Network service owns NIC queues and offers socket handles. Stage Ethernet/ARP/IPv4/ICMP/UDP, then TCP/DNS/DHCP/TLS with loopback and packet-fault scenarios. “Driver loaded” and “HTTP fetched” are separate evidence states.
4. Compositor owns scanout/input handles. Clients get shared surface handles and event channels. Window server never trusts client sizes/strides; damage regions and input routing are bounded.
5. Make a small reference image that boots through init to Files, terminal, compositor, and persistence scenarios. Keep it as a product smoke gate even after the system grows.

### Layer 4 — agent runtime as ordinary userspace

1. Agent supervisor is an unprivileged process. Model backends, prompt/context store, planners, browser, shell, media decoders, and connectors are separate services/processes.
2. Tool registry is a signed/versioned schema: input grammar, output schema, required rights, taint sources/sinks, cost/time/memory limits, idempotency, and rollback contract.
3. Planner emits a typed immutable plan. A small deterministic executor validates plan shape, current handles, user consent, policy, taint/provenance, and budgets immediately before each action.
4. Subagents are child processes with new handle tables. The grant operation is set intersection; widening fails. On completion/timeout/crash, revoke derived handles and retain artifacts/receipt.
5. Skills/plugins are packages with signed manifests and code/data separation. Install approval creates a maximum grant ceiling; each run can request less. Filesystem access is rooted at explicit directory handles.
6. Provenance tracks external/untrusted content through tool outputs and artifacts. Initially call it “provenance/taint policy,” not complete information-flow security. Add explicit declassification requiring user/policy authority.
7. Audit events form a hash/MAC chain, but durable integrity needs an append-only zlfs journal plus periodic externally anchored heads. State retention/compaction honestly; do not call a bounded RAM body permanent.
8. Remote connectors never receive kernel capabilities directly. A connector broker translates scoped application tokens into narrow operations and logs request/result digests without harvesting credentials or private data.

### Layer 5 — verification matrix

For every layer, require all applicable columns rather than one global green check:

| Gate | Required proof |
|---|---|
| Build | fresh build, strict exit, module/link inventory, reproducible config |
| Unit/property | empty/full/one-over bounds, malformed input, stale handles, revoke trees, permission matrix |
| Differential | interpreter/runtime/backend and protocol encoder/decoder agreement where mirrored |
| Sanitizer/fuzz | host parsers and algorithms under UBSan/ASan/fuzz; corpus and crash artifacts retained |
| QEMU | cold boot, SMP, process isolation, IPC, service crash/restart, persistence, network faults |
| Visual | screenshot/image probes for compositor/apps; not inferred from source |
| Hardware | named device/firmware, exact cold/warm/reset/recovery paths, raw logs |
| Formal | model property plus source-conformance check; model-only labelled separately |
| Agent adversarial | prompt injection, confused deputy, grant widening, taint laundering, replay, timeout, partial failure |
| Independent review | second agent/reviewer tries to refute claim against receipt and source |

## Recommended implementation order

1. **Repair truth infrastructure**: strict gates, evidence states, module inventory, durable receipt format.
2. **Finish typed process/handle/IPC foundation**: user spans, W^X, transactional handle transfer, revoke/attenuate, per-CPU ownership.
3. **Move parser/policy risk out of kernel**: driver manager, VFS, network, compositor, decoder services with supervision.
4. **Close existing zlOS product paths**: persistent zlfs + Files + compositor/reference apps with cold-boot and graphical proof.
5. **Add agent executor boundary**: registry/schema, immutable plan, deterministic checks, audit, ordinary-process subagents.
6. **Add provenance, skill signatures, connector broker, and external audit anchoring**.
7. **Expand drivers/network/media only behind per-feature evidence matrices**.
8. **Introduce formal models selectively** for handle transfer/revoke, scheduler ownership, zlfs commit/recovery, and agent action state machines; require implementation conformance tests.

## Final disposition by repository

| Repository | Use as primary reference for | Do not inherit |
|---|---|---|
| Chitti | grammar-gated tools, attenuation, signed skills, provenance, audit events | ring-0 orchestrator, giant kernel TCB, “permanent” audit wording |
| NexiOS | bounded pools, deadlines, CSpace revoke, rewind testing | hard-RT claim without end-to-end timing proof, roadmap-as-current-state |
| RustOS | contracts, authority, fail-closed providers, evidence receipts, formal mapping | process/architecture complexity before zlOS foundations are stable |
| Zinnia | driver modules, Unix object breadth, init graph | compile-only CI and kernel-only product assumptions |
| Skift | object capabilities, transactional handle transfer, service strata | current domain capacity bug, sprawling fetched-source audit boundary |
| Fudge | typed event-driven services and small hardware modules | weak isolation, silent drop semantics, absent automated tests |
| TacOS | small end-to-end init/shell/WM/PTY smoke path | syscall validation gaps, busy waits, permission and driver bugs |
| NyauxKC | simple per-CPU queue and VFS object vocabulary | tuple-typed hardware identity, unsafe USTAR parser, build-recipe port claims |
| Cyjon | readable boot/state/context mechanics | code reuse, masked failures, fixed global policy, restrictive license |

The strongest synthesis is not “put every feature in one kernel.” It is to combine **small typed mechanisms, separately owned services, least-authority handles, explicit bounds, and receipts that can survive an adversarial review**. That architecture gives zlOS room to absorb the best user-visible features from these projects without absorbing their trust boundaries or their evidence mistakes.
