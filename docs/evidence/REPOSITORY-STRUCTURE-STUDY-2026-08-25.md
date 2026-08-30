# Structure Study: 33 Starred Repositories

Date: 2026-08-25
Status: study complete; resulting zlOS migration implemented with static checks
Source shelf: `/home/roy/Documents/repos/zl-starred-sources`
Snapshot authority: `SOURCE_SNAPSHOT_MANIFEST_2026-08-21.md` in the research shelf

## Scope And Method

All 33 pinned top-level checkouts were counted. Working-tree size excludes each
top-level `.git` directory; tracked-file counts come from Git. The 17 largest or
most structurally relevant systems were then inspected one and two levels deep.
Generated and ignored outputs were identified separately because disk size alone
is a poor proxy for repository architecture.

## All 33 Repositories

| Rank by tracked files | Repository | Tracked files | Root files | Root source files |
|---:|---|---:|---:|---:|
| 1 | SerenityOS | 18,702 | 18 | 0 |
| 2 | NexiOS | 10,493 | 43 | 0 |
| 3 | MollenOS | 6,554 | 12 | 0 |
| 4 | Banana Operating System | 4,793 | 20 | 0 |
| 5 | Chitti | 1,872 | 17 | 0 |
| 6 | Brook | 1,493 | 26 | 0 |
| 7 | banan-os | 1,311 | 9 | 0 |
| 8 | RustOS | 1,248 | 14 | 0 |
| 9 | hhuOS | 1,171 | 8 | 0 |
| 10 | Astral | 1,149 | 15 | 0 |
| 11 | duckOS | 1,053 | 7 | 0 |
| 12 | MaslOS-2 | 637 | 18 | 0 |
| 13 | vib-OS | 606 | 8 | 0 |
| 14 | LemonOS | 534 | 8 | 0 |
| 15 | AthenX 2.0 | 532 | 9 | 1 |
| 16 | Fudge | 500 | 8 | 0 |
| 17 | privilegeos | 433 | 6 | 0 |
| 18 | freestanding-c-hdrs-gnu | 378 | 4 | 0 |
| 19 | snarkOS | 331 | 12 | 1 |
| 20 | Zinnia | 265 | 8 | 0 |
| 21 | Skift | 260 | 7 | 0 |
| 22 | Hyper | 201 | 9 | 0 |
| 23 | ProtOS kernel | 200 | 6 | 0 |
| 24 | TacOS | 199 | 8 | 0 |
| 25 | mission-control | 145 | 15 | 0 |
| 26 | Cyjon | 83 | 10 | 0 |
| 27 | Unix History | 83 | 83 | 74 |
| 28 | sapphire-gpu | 60 | 7 | 0 |
| 29 | NyauxKC | 51 | 7 | 0 |
| 30 | asm.fm | 31 | 30 | 25 |
| 31 | mYOS | 20 | 2 | 0 |
| 32 | MineBios | 9 | 4 | 0 |
| 33 | osdev-projects | 4 | 4 | 0 |

Unix History and asm.fm are intentionally flat data/program collections, not
large maintainable operating-system source trees. Of the other 31 repositories,
29 have zero implementation files at the repository root.

## Largest-System Layouts

| Repository | Dominant organization |
|---|---|
| SerenityOS | Product domains at root: `Kernel`, `Userland`, `Tests`, `Ports`, `Toolchain`, `Documentation`, `Meta`; kernel split into 19 subsystems and userland into applications, libraries, services, shell, utilities, and developer tools. |
| NexiOS | Unified `src/` split into kernel, libc, programs, services, and initrd; separate userspace, tools, build metadata, audits, and docs. Its tracked generated docs/graph output is not a pattern to copy. |
| MollenOS | Explicit platform ownership: boot, kernel, runtime libraries, modules, protocols, services, apps, testing, tools, docs, and CMake infrastructure. |
| Banana OS | Product domains: kernel, drivers, firmware, libraries, applications, packages, installer, sysroot, and docs. Generated docs/sysroot/disassembly bulk weakens the otherwise clear split. |
| Chitti | Rust workspace with kernel, userspace, agents, tests, tools, targets, assets, docs, and third-party code separated. |
| Brook | Compact `src/` with bootloader, kernel, drivers, apps, shared interfaces, tests, and userspace; repository-level tools, scripts, data, docs, and vendor code remain outside source. |
| banan-os | Kernel, userspace, shared BAN library, ports, bootloader, toolchain, scripts, and tools. Headers live in component-owned include trees. |
| RustOS | Kernel, drivers, driver domains, services, libraries, apps, tests, formal models, tools, docs, boot, compatibility, configuration, and vendor code are explicit peers. |
| hhuOS | Almost all product code under `src/`, then application, device, filesystem, kernel, and library domains; build, disk-image input, media, and tools stay outside. |
| Astral | Kernel source, binary/source recipes, distribution files, service scripts, patches, host recipes, install hooks, and tools are separate build products. |
| duckOS | Kernel, libraries, programs, services, ports, base filesystem, toolchain, scripts, and docs. |
| MaslOS-2 | Kernel, loader, modules, programs, common library, and external dependencies are distinct, but checked/generated object and ISO output create avoidable bulk. |
| vib-OS | Kernel, drivers, boot, libc, user programs, userspace services, runtimes, scripts, tools, examples, screenshots, and vendor code. |
| LemonOS | Kernel, applications, services, core API and GUI libraries, ports, system programs, interface compiler, resources, docs, and scripts. |
| AthenX 2.0 | Many named domains exist, but 30 root directories plus large `old/`, `obj/`, `out/`, and `iso/` trees make navigation and source/output boundaries substantially weaker. |
| Zinnia | Small Rust workspace: kernel, drivers, bindings, and toolchain. Its measured 649 MiB working tree is almost entirely ignored `target/` output, not source. |
| Skift | Minimal root: `src/`, `doc/`, and `meta/`; source divides into kernel, libraries, and services. |

## Kernel Fan-Out

The large systems do not generally leave their kernel implementation flat.

| Repository | Kernel files | Immediate kernel-root files | Immediate subsystem directories |
|---|---:|---:|---:|
| SerenityOS | 1,207 | 15 | 19 |
| NexiOS | 403 | 11 | 18 |
| MollenOS | 188 | 11 | 10 |
| Banana OS | 510 | 17 | 15 |
| Chitti | 542 | 44 | 38 |
| Brook | 256 | 2 | 3 |
| banan-os | 320 | 3 | 5 |
| RustOS | 264 | 5 | 11 |
| hhuOS | 125 | 0 | 8 |
| Astral | 469 | 3 | 7 |
| duckOS | 408 | 26 | 15 |
| MaslOS-2 | 124 | 4 | 15 |
| vib-OS | 189 | 4 | 18 |
| LemonOS | 203 | 5 | 3 |
| Zinnia | 196 | 8 | 12 |
| Skift | 72 | 0 | 5 |
| **zlOS before migration** | **374** | **199** | **4** |

The pre-migration zlOS root's 199 immediate files contained 126 code-like files
and 66 shell/Python scripts or probes. This was the largest clear structural
deviation from the good large-project patterns.

After migration, `kernel/` has 10 immediate files, 0 implementation files, and
6 ownership directories: `apps`, `boot`, `docs`, `src`, `tests`, and `tools`.

## Patterns Worth Copying

1. Keep the repository root as a control plane: orientation, build entry points,
   policy, configuration, and major product domains only.
2. Organize by ownership and runtime role, not by maturity. `done/` and
   `half-done/` become dishonest as evidence changes.
3. Separate kernel, user programs, services, libraries, drivers, tests, tools,
   docs, assets, ports/packages, boot code, and generated output.
4. Subdivide kernels by subsystem. Architecture, memory, tasking, filesystems,
   networking, devices, security, and syscalls recur across the strongest trees.
5. Keep component headers with their owner or in an explicit public include
   boundary; do not depend on one accidental global include directory.
6. Keep tests parallel to ownership. SerenityOS's broad `Tests/`, RustOS's
   contract/formal split, and Brook's host plus bare-metal split are useful
   models for different evidence levels.
7. Keep generated outputs outside source and ignored. Zinnia's `target/`,
   NexiOS's build/debug/graph output, MaslOS's ISO/objects, and AthenX's
   old/out/iso trees show why disk-size rankings can mislead.

## Implemented zlOS Direction

The comparison produced this layout, implemented on 2026-08-25:

```text
kernel/
|-- README.md, SOURCES, build and primary verification entry points
|-- boot/               BIOS, UEFI, assembly entry, linker layouts
|-- src/
|   |-- arch/x86/       CPU, GDT, IDT, APIC, SMP, paging primitives
|   |-- core/           scheduler, processes, IPC, heap, logging
|   |-- drivers/        display, input, storage, network, platform buses
|   |-- fs/             VFS and concrete filesystems
|   |-- net/            protocol stack and network services
|   |-- graphics/       framebuffer, compositor, windows, widgets, fonts
|   |-- web/            HTML, CSS, layout, JavaScript, HTTP/TLS
|   `-- runtime/        zl execution bridge and kernel runtime support
|-- apps/               zl application and game sources
|-- tests/
|   |-- host/           host-native subsystem tests
|   |-- oracle/         zlOS visual capture and scoring
|   |-- refrender/      independent reference renderer
|   `-- fixtures/       stable test inputs
|-- tools/
|   |-- checks/         static and boot verification scripts
|   |-- images/         ISO, disk, and ESP builders
|   |-- probes/         runtime/QEMU observation scripts
|   |-- generators/     icons, fonts, shaders, and embedded assets
|   `-- run/            convenience VM runners
`-- docs/               current, evidence, and archive
```

`kernel/SOURCES` remains the shared shipped-source manifest. Build scripts,
host harnesses, probes, oracles, generators, gates, and direct includes were
rewired to the new ownership paths in the same migration. Static path and syntax
checks were run; compilation, QEMU, and hardware behavior were deliberately not
claimed by this structure-only pass.
