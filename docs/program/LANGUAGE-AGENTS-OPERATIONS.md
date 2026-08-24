# Language, developer platform, agents, compatibility and operations

These workstreams make the product sustainable. They are not optional polish
after the kernel and apps: they supply the ABI, build graph, packages, review,
release, recovery and eventual in-zlOS rebuild.

## zl language and compiler program — MP-16

Preserve the existing self-hosting fixpoint, native backend and language
semantics while adding system-grade output. “zl cannot express it” and “nobody
has implemented the compiler path yet” must remain separate diagnoses.

### Front end and language model

| ID | Deliverable | Complete result |
|---|---|---|
| ZLP-001 | module/import system | deterministic resolution, cycles, visibility and diagnostics |
| ZLP-002 | namespaces/scopes | exact shadowing, qualification, exports and errors |
| ZLP-003 | source locations/diagnostics | spans, notes, fix hints and stable machine output |
| ZLP-004 | error/result model | explicit propagation, cleanup and no silent false success |
| ZLP-005 | exact integers | signed/unsigned widths, conversions and ABI agreement |
| ZLP-006 | byte and byte-string types | binary-safe indexing/slicing and no text conflation |
| ZLP-007 | arithmetic modes | checked/wrapping/saturating operations and overflow proof |
| ZLP-008 | enums/tagged unions | stable layout, exhaustiveness and unknown-version behavior |
| ZLP-009 | generics/interfaces | bounded monomorphization or dictionaries with ABI rules |
| ZLP-010 | function values/closures | calling convention, captures, lifetime and code/data authority |
| ZLP-011 | compile-time evaluation | deterministic bounded execution and dependency tracking |
| ZLP-012 | attributes/annotations | versioned validated metadata for ABI/sections/generation |
| ZLP-013 | conditional compilation | target/feature profiles included in artifact identity |
| ZLP-014 | package/module documentation | generated API docs with source/provenance links |

### Systems data, pointers and ABI

| ID | Deliverable | Complete result |
|---|---|---|
| ZLP-020 | packed/aligned records | explicit size/alignment/offset assertions |
| ZLP-021 | bitfields | endian/width/range-defined hardware and protocol layout |
| ZLP-022 | typed raw pointers | address-space/const/nullable distinctions and explicit unsafe boundary |
| ZLP-023 | volatile MMIO access | width/alignment/ordering semantics and target lowering |
| ZLP-024 | port I/O intrinsics | x86 width and privilege rules |
| ZLP-025 | memory/compiler barriers | target-specific ordering with conformance tests |
| ZLP-026 | atomic operations | memory orders, widths, lock-free facts and fallbacks |
| ZLP-027 | calling-convention declarations | SysV, interrupt, firmware and generated zlIDL ABI |
| ZLP-028 | external symbol/assembly declarations | typed linkage, clobbers and diagnostics |
| ZLP-029 | TLS/per-thread data | ABI, initialization, teardown and module interaction |
| ZLP-030 | endian/serialization primitives | checked length-first binary protocol support |
| ZLP-031 | ownership/lifetime helpers | explicit resource cleanup patterns without hiding authority |

### Native output, linker and tools

| ID | Deliverable | Complete result |
|---|---|---|
| ZLP-040 | ELF64 executable output | correct headers/segments/permissions/relocations/entry |
| ZLP-041 | ELF64 relocatable output | sections, symbols, RELA, COMDAT/weak policy and deterministic bytes |
| ZLP-042 | shared-object/PIC output | GOT/PLT/TLS/relocations and versioned ABI |
| ZLP-043 | named/custom sections | boot requests, metadata, init/fini and linker retention |
| ZLP-044 | archive librarian | deterministic static archives and indexes |
| ZLP-045 | zl linker | checked layout, scripts, GC, symbols, diagnostics and maps |
| ZLP-046 | debug information | line/function/type data consumable by debugger/profiler |
| ZLP-047 | object/binary inspection tools | sections/symbols/relocations/dependencies/provenance |
| ZLP-048 | dependency/build metadata | exact source/generated/tool inputs and cache keys |
| ZLP-049 | incremental build/cache | correctness-first invalidation and reproducible fallback |
| ZLP-050 | zl formatter/linter | stable formatting and semantic diagnostics |
| ZLP-051 | language server | parse/type/navigation/completion over exact compiler semantics |
| ZLP-052 | package manager integration | locked modules, signatures, licenses and build receipts |
| ZLP-053 | zlIDL generator | schemas to zl/C bindings, limits and conformance tests |
| ZLP-054 | compiler hostile corpus | parser/type/codegen/link malformed and differential cases |
| ZLP-055 | fixpoint and bootstrap ledger | stage identity, semantic/hash comparison and rollback |

Milestones are a real zl user process, then service, then driver provider. A
freestanding object-file demo is necessary but does not prove a loadable,
isolated, restartable provider.

### C-free zlOS boundary

The long-term selected first-party zlOS profile is written in zl, with only the
minimum architecture entry/trampoline instructions retained as assembly or
emitted machine code. This is distinct from three weaker claims:

1. a self-hosting compiler exists;
2. zl source is transpiled to C and then compiled;
3. a zl kernel entry point links to C runtimes, drivers and services.

The current system has all three weaker properties: zl has a compiler fixpoint,
but the live zlOS routes still turn `kernel.zl` into generated C and compile the
freestanding runtime plus the shared kernel/driver source list with GCC or
Clang. Therefore zlOS is not currently C-free.

Removal is dependency ordered:

1. make zl able to express exact-width data, packed layouts, pointers, volatile
   MMIO, port I/O, barriers, atomics, interrupt/firmware ABIs, external assembly,
   explicit unsafe operations, allocation and failure cleanup;
2. emit native ELF/PE relocatable objects, sections, symbols, relocations,
   debug information and deterministic archives without a C intermediary;
3. replace the freestanding runtime with zl-owned memory/string/arithmetic,
   panic, allocation and synchronization primitives;
4. migrate pure algorithms and parsers, then services and ordinary drivers,
   then memory/interrupt/scheduler and boot-critical mechanisms, retaining
   differential C oracles until each replacement is stronger;
5. run the compiler, linker, archive, package and build tools as zl programs;
6. build a compiler fixpoint and the complete matching boot manager, kernel,
   services, drivers, SDK, apps and packages inside zlOS;
7. remove GCC/Clang and C source from the selected first-party release graph.

“C-free zlOS” does not claim that CPU microcode, device firmware, UEFI/BIOS or
foreign Windows/Linux applications were written in zl; those are external
inputs with their own identity, license and authority boundaries. Optional GRUB
acquisition may also contain external C while the owned native/raw boot route
remains sufficient to build and boot the C-free zlOS profile. A stricter image
containing no C-origin third-party library is possible only by rewriting or
excluding each such package.

## Developer and compatibility program — MP-15

| ID | Deliverable | Complete result |
|---|---|---|
| DEV-001 | PTY and terminal ABI | sessions, resize, signals, Unicode, peer death |
| DEV-002 | interactive shell | quoting, expansion, pipelines, redirection, jobs and scripts |
| DEV-003 | core utilities | file/text/process/system/network tools with consistent errors |
| DEV-004 | source editor/IDE | project, language, build/test/debug and recovery workflow |
| DEV-005 | debugger | scoped attach, symbols, stepping, break/watch and dumps |
| DEV-006 | profiler/tracer | CPU/memory/I/O/frame/event correlation and export |
| DEV-007 | test runner | registered/executed/skipped/failed parity and strict exit |
| DEV-008 | package/port SDK | hermetic recipe, patch, build, stage, runtime and update |
| DEV-009 | POSIX surface ledger | program-driven syscalls/libc/shell behavior and gaps |
| DEV-010 | dynamic linker | bounded ELF imports/relocations/search/cache/unload |
| DEV-011 | shared libraries | ABI/version/ownership/update and dependency receipts |
| DEV-012 | Linux ABI/userland compatibility | isolated selected syscall, ELF interpreter, dynamic-linker and userland/container workloads, not a blanket claim |
| DEV-013 | Windows/PE compatibility subsystem | length-first PE/DLL loader plus selected NT/Win32/registry/window/graphics/audio/network/COM/Direct3D contracts for explicit applications |
| DEV-014 | scripting runtimes | sandboxed selected interpreters with package boundaries |
| DEV-015 | SDL/media compatibility | input/audio/display/files through app capabilities |
| DEV-016 | X11/Wayland bridge experiments | explicit legacy compatibility, no compositor authority bypass |
| DEV-017 | Windows/Linux virtual machine manager | hardware-assisted guests, licensed images, devices, snapshots, networking and containment |
| DEV-018 | emulator framework | selected CPU ISA and device emulation with deterministic fixtures, translation-cache bounds and limits |
| DEV-019 | sandbox/container profiles | namespaces, handles, resources, network and teardown |
| DEV-020 | Windows/Linux guest integration | typed shared folder/clipboard/input/display/audio/network and optional seamless-window authority |

Compatibility is earned per workload. A recipe, compiled binary or Linux driver
VM does not become native zlOS functionality merely by being present.

### Windows application routes

The Windows program has four explicit routes. Open-source software should be
ported and rebuilt against the zlOS SDK. Selected unmodified applications use
the quarantined Windows compatibility subsystem: PE/COFF and DLL loading,
imports/relocations/TLS/CRT/module ownership, then application-driven NT/Win32
contracts such as `ntdll`, `kernel32`, `user32`, `gdi32`, registry, COM, audio,
network and selected Direct3D translation. Applications requiring genuine
Windows behavior use a contained Windows VM with applicable guest licensing.
Different-ISA or historical workloads additionally consume the emulator.

A PE loader by itself is never called Windows compatibility. Each supported
application names its required API surface, test workflow, resource profile and
known failures. Compatibility code runs outside ring 0 and reaches zlOS files,
windows, input, audio, networking and GPU services only through ordinary
capabilities and portals.

### Linux application routes

Linux software has the same four routes. Source-available programs should be
ported natively through POSIX/libc/SDL and package recipes. Selected unmodified
ELF programs use the Linux compatibility subsystem: ELF interpreter and dynamic
linker, program-driven syscall translation, signals, futex, threads, files,
networking and the required `/proc`/`/dev` view. Larger distributions, kernel
modules or workloads that require real Linux run inside a contained Linux VM.
Different-ISA workloads additionally consume the emulator.

Linux compatibility is not permission compatibility. A translated Linux call
cannot bypass zlOS handles, namespaces, resource limits, portals or network
policy. Containers share the zlOS kernel and therefore require a proved syscall
surface; VMs run a guest kernel and remain the broader-compatibility fallback.

## Capability-native agent program — MP-17

Agents are ordinary unprivileged processes. Models, planners and orchestrators
never run in ring 0. The kernel knows handles and effects, not prompts.

| ID | Deliverable | Complete result |
|---|---|---|
| AGT-001 | typed intent/plan schema | versioned goals, steps, dependencies, limits and expected evidence |
| AGT-002 | deterministic tool broker | declared inputs/effects, authority checks, idempotence and receipts |
| AGT-003 | attenuated subagents | live parent-handle derivation, budgets and no authority resurrection |
| AGT-004 | cancellation and parent-death | task dead, every handle stale, no orphan root-cap process |
| AGT-005 | resource budgets | CPU/memory/time/I/O/network/tool/subtask ceilings and exhaustion result |
| AGT-006 | approval policy | risk-classified effects, explicit user decision and expiring grants |
| AGT-007 | secret mediation | scoped non-readable use where possible, redaction and zeroing |
| AGT-008 | provenance/taint graph | input/source/effect/derivation and privacy propagation |
| AGT-009 | durable audit receipts | actor/authority/input/output/effect with anchoring and drop evidence |
| AGT-010 | signed skill/plugin format | asymmetric publisher identity, trust roots, rotation/revocation |
| AGT-011 | transactional skill install | stage/verify/admit/publish; exact rollback at every failure |
| AGT-012 | complete uninstall/revoke | stop tasks, revoke handles, deregister tools/roles, retain data by policy |
| AGT-013 | tool and role registry | generated admitted capabilities and current health |
| AGT-014 | scheduled automation | user-owned triggers, deadlines, missed-run policy and history |
| AGT-015 | desktop semantic automation | accessibility/action APIs, no pixel-only ambient control |
| AGT-016 | filesystem/workspace automation | file portals, transaction scopes and rollback |
| AGT-017 | network/API automation | origin/endpoint grants, quotas and credential mediation |
| AGT-018 | interactive progress/control UI | plan, tools, grants, children, evidence, cancel and recovery |
| AGT-019 | independent completion reviewer | locked spec, separate evidence access and reject outcome |
| AGT-020 | evaluation/regression harness | golden tasks, adversarial cases, longitudinal reliability |
| AGT-021 | offline/local model provider seam | model identity, capability, resource and privacy contract |
| AGT-022 | remote model provider seam | endpoint/auth/data policy, cancellation and retention contract |
| AGT-023 | human handoff/continuation | durable state, unresolved decisions, weakest evidence and replay |
| AGT-024 | agent recovery/checkpoint | safe restart without replaying already committed effects |

Required hostile proofs include revoke-then-spawn, simultaneous revoke/spawn,
kill root orchestrator, nth-tool failure, partial install/uninstall, forged
receipt, secret in output/log, budget exhaustion, tool timeout, network loss,
reviewer rejection and a polished false-completion response.

## Operations, public demo and release program — MP-18

| ID | Deliverable | Complete result |
|---|---|---|
| OPS-001 | hermetic release build | empty tree, locked toolchain/dependencies and strict exit |
| OPS-002 | artifact identity/SBOM | source/config/tool/package/license/signature and boot origin |
| OPS-003 | signing and trust policy | offline roots, online roles, rotation, revocation and recovery |
| OPS-004 | release channels | stable/beta/developer/recovery with compatibility and promotion |
| OPS-005 | transactional updater | download/verify/stage/switch/reboot/health/rollback |
| OPS-006 | boot generations | current/previous/recovery identity and atomic selection |
| OPS-007 | migration framework | versioned data/config/package migrations and rollback |
| OPS-008 | release notes | generated changes, compatibility, defects and recovery steps |
| OPS-009 | CI build matrix | x86 lanes plus honest architecture build/boot/test distinctions |
| OPS-010 | host deterministic matrix | unit/property/fuzz/sanitizer/failure/mutation tests |
| OPS-011 | QEMU system matrix | BIOS/UEFI/media/device/workflow/assertion kernels |
| OPS-012 | native hardware matrix | exact device/firmware/topology and workload receipts |
| OPS-013 | performance promotion | budgets, distributions, machine/backend and regression action |
| OPS-014 | security promotion | threat assets, enforcement, hostile proof and residual risk |
| OPS-015 | accessibility promotion | semantic/keyboard/provider/scale/contrast/motion/locales |
| OPS-016 | visual promotion | regional goldens, layout/text metrics, animation traces |
| OPS-017 | crash/telemetry pipeline | consent, redaction, symbol identity, retention and alerting |
| OPS-018 | support bundles | user preview, scrub, reproduce and correlation ID |
| OPS-019 | backup/disaster recovery drill | verified backup, destructive simulation, restore and audit |
| OPS-020 | rescue image | separately signed, read-only default, explicit target and repair log |
| OPS-021 | public demo allocator | atomic slot/port/disk/resource lease and rollback |
| OPS-022 | QEMU launcher | allowlisted arguments, immutable base, per-lease overlay and limits |
| OPS-023 | VNC/WebSocket authentication | unenumerable per-lease data plane and expiration |
| OPS-024 | demo containment | CPU/RAM/PID/disk/egress/time/privacy policy |
| OPS-025 | demo reaper | prove process identity/death before resource reuse, zero orphans |
| OPS-026 | provenance/feature viewer | live maturity, evidence, licenses, permissions and health |
| OPS-027 | deprecation/support policy | lifecycle, compatibility window, migrations and archives |
| OPS-028 | independent release review | reviewer can reject; no self-certified completion |

## Future architecture and product profiles — MP-19

| ID | Profile | Complete result |
|---|---|---|
| FUT-001 | x86-64 laptop | battery/thermal/suspend, Intel display/audio/network, Wi-Fi/BT, touchpad |
| FUT-002 | x86-64 desktop/workstation | discrete GPU, multimonitor, wired/audio/USB and power |
| FUT-003 | QEMU development VM | complete virtio profile and deterministic gates |
| FUT-004 | headless/server | serial/network management, storage, no-display boot and updates |
| FUT-005 | AArch64 generic VM | UEFI/FDT, EL/MMU/GIC/timer/SMP/process/services |
| FUT-006 | Raspberry Pi | firmware/mailbox/GPIO/SD/display/input/network/power |
| FUT-007 | Apple Silicon research | m1n1/FDT/PCIe/DART/display/input/storage platform contracts |
| FUT-008 | RISC-V64 generic VM/board | SBI/Sv39/PLIC/timer/SMP/process/services |
| FUT-009 | tablet/touch product | touch/stylus/rotation/keyboard/a11y/power/camera |
| FUT-010 | secure/managed profile | TPM/measured boot/policy/update/attestation and recovery |
| FUT-011 | heterogeneous compute | CPU/GPU topology, memory/coherency, scheduling and fallback |
| FUT-012 | custom FPGA accelerator | capability descriptors, DMA/isolation, simulation and reset |

Secondary architecture completion requires its own target workload and service
tests. Multiarchitecture build success and QEMU boot/shutdown never inherit
x86-64 target-test or physical-hardware evidence.

## In-zlOS self-hosting promotion — MP-20

1. Run compiler, linker, archive, package and build tools as isolated zlOS
   processes with file/process/resource capabilities.
2. Import the exact locked source and dependency graph.
3. Build the compiler and verify its fixpoint inside zlOS.
4. Build boot manager, kernel, services, drivers, SDK, apps and packages.
5. assemble an identified current/previous/recovery image generation;
6. boot that exact generation through the full assertion matrix;
7. pass hostile, crash/power-cut, performance, a11y, visual, security, update/
   rollback and selected physical gates;
8. run an independent reviewer against the frozen manifest and receipts.

Only then does “zlOS builds itself” mean the matching distributable system, not
merely that one compiler binary reproduced another binary.
