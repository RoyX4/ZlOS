# Starred repositories: master research plan and taxonomy

**Snapshot date:** 2026-08-21
**Source shelf:** `/home/roy/Documents/repos/zl-starred-sources`
**Scope:** all 33 repositories starred by `RoyX4` at the final live recheck, their current default-branch
source trees, correctly declared submodules, documentation, build/configuration,
tests, CI, features, implementation strategies, gaps and lessons.

The list began at 32; `Ellicode/protOS-kernel` became the 33rd star while the audit was running and was added at its exact then-current commit.

## Required outcome

This research is complete only when it produces all of the following:

1. A dossier for every repository at an exact commit.
2. A normalized feature matrix, so the same feature means the same thing across
   projects.
3. Implementation notes that explain **how** a feature works and point to source,
   rather than copying a README checkbox.
4. An evidence level for every material claim.
5. Best-in-set implementations and strategies, including their tradeoffs.
6. Missing, partial, broken, misleading and unverified areas.
7. Licensing and clean-room boundaries.
8. A no-code-copy architecture and phased plan for applying useful ideas to zl,
   zlOS and related projects.
9. An independent coverage/refutation pass before completion.

## Evidence ladder

Every material feature claim receives the strongest demonstrated level below:

| Code | Meaning |
|---|---|
| `C` | Claimed by documentation, comment or roadmap only. |
| `S` | Source exists and its main logic was inspected. |
| `R` | Reachable from the checked build/registration/dispatch path. |
| `T` | Covered by an identified automated test or deterministic gate. |
| `B` | Built successfully in this research at the recorded commit. |
| `Q` | Booted or exercised successfully in QEMU/another VM here. |
| `H` | Physical-hardware evidence was reproduced here. |

The levels are cumulative only when the evidence really supports the lower
levels. A test file that is never registered is not `T`; dead source is not `R`;
a screenshot is not `H`; a green build is not a boot.

## Feature taxonomy

### Project and proof

- purpose, architecture and design philosophy
- active/archived/rewrite status
- language, toolchain, target architectures and boot protocol
- exact commit, license, imported/vendor/submodule boundary
- build system, reproducibility, CI, release/image pipeline
- unit, host, integration, VM and hardware tests
- tracing, logging, panic/crash reporting, profiling, debugging and fuzzing
- known bugs, TODOs, stubs, unsafe fallbacks and false-green hazards

### Boot and hardware discovery

- BIOS, UEFI, Limine/GRUB/custom loader, multiboot and network boot
- kernel format, relocation, higher-half mapping, boot protocol and modules
- ACPI/uACPI/ACPICA, SMBIOS, device tree, PCI/PCIe and enumeration
- CPU features, architecture abstraction, x86/i686/x86-64/ARM/RISC-V
- interrupt controllers, timers, RTC, HPET, LAPIC/IOAPIC/GIC/PIC/PIT
- SMP bring-up, per-CPU state, IPIs, TLB shootdown and CPU hot paths

### Memory and execution

- physical allocator, virtual memory, kernel heap and slab/arena/buddy designs
- page tables, address spaces, guard pages, COW, shared memory and mmap
- user/kernel isolation, safe user-copy, faults, signals and process teardown
- threads/tasks/processes, context switch, scheduler class and load balancing
- priority, real-time/deadline behavior, affinity, preemption and starvation
- executable loaders (ELF/PE), dynamic linker, TLS and runtime/ABI
- syscalls, POSIX/Linux/custom ABI, libc and language runtime

### Authority, IPC and reliability

- handles, capabilities, permissions, users/groups, sandbox and trust model
- IPC: messages, channels, ports, pipes, shared memory, RPC and events
- synchronization: mutexes, rwlocks, futexes, semaphores, atomics and RCU
- cancellation, timeouts, bounded queues, backpressure, lifecycle and restart
- formal models, contracts, invariants, fault injection and adversarial tests

### Files and storage

- VFS object model, namespace, mounts, descriptors and file/page cache
- initramfs/ramfs/tmpfs/devfs/procfs/sysfs
- FAT/ext2/ext4/custom filesystems and persistence/crash behavior
- block layer, partitioning, GPT/MBR, buffering, DMA and request queues
- ATA/IDE/AHCI, NVMe, virtio-block, USB mass storage and optical media
- permissions, timestamps, directories, symlinks, mmap and durability

### Devices, network and media

- PS/2, USB UHCI/OHCI/EHCI/xHCI, HID, hubs and hotplug
- serial, UART, GPIO, sensors, power, battery and thermal paths
- NICs: e1000/e1000e/rtl8139/rtl8169/igc/virtio-net and physical evidence
- Ethernet, ARP, IPv4/IPv6, ICMP, DHCP, DNS, UDP, TCP and sockets
- routing, loopback, Unix sockets, TLS/HTTP and remote files/protocols
- audio controllers, mixer/server, codecs, playback and synchronization
- framebuffer, BGA/VBE/GOP, DRM/KMS, virtio-GPU and accelerated GPU work

### User experience and ecosystem

- terminal/TTY/PTY, shell, core utilities and init/service manager
- window manager/compositor, display protocol and input routing
- UI toolkit, layout, widgets, themes, accessibility and app lifecycle
- Files, editor, terminal, calculator, media, games and system applications
- graphics/image/font/3D/media libraries and browser engines
- networking applications, package manager, ports and external compatibility
- compiler/toolchain ports, self-hosting and building the OS inside itself
- installer, updater, disk/image/recovery and browser-hosted demos

### Language, agents and future hardware

- custom languages, compiler backend, object formats and freestanding support
- FFI, packed layout, inline assembly, linker sections and module imports
- agent orchestration, tool ABI, plan parsing, capabilities and audit receipts
- deterministic/stochastic boundary, sub-agent authority and persistence
- HDL/GPU command/interface/raster/scanout/compute strategies
- assembly-only/system-call-only teaching patterns and historical precedents

## Repository groups and ownership

### Mature/general-purpose OS track

- `IanNorris/Brook`
- `Mathewnd/Astral`
- `Bananymous/banan-os`
- `LemonOSProject/LemonOS`
- `SerenityOS/serenity`
- `Meulengracht/MollenOS`
- `hhuOS/hhuOS`
- `byteduck/duckOS`

### Architecture/experimental OS track

- `chittios/chitti`
- `staycool1374-Ger/nexios`
- `ierwarf/rustos`
- `zinnia-os/zinnia`
- `skift-org/skift`
- `jezze/fudge`
- `UnmappedStack/TacOS`
- `rayanmargham/NyauxKC`
- `CorruptedByCPU/Cyjon`

### Focused/small/supporting track

- `robotman2412/sapphire-gpu`
- `UltraOS/Hyper`
- `ProvableHQ/snarkOS`
- `dspinellis/unix-history-repo`
- `felipenlunkes/osdev-projects`
- `marceldobehere/MaslOS-2`
- `alexdboxall/Banana-Operating-System`
- `KingVentrix007/AthenX-2.0`
- `ktauchathuranga/privilegeos`
- `yashvi-soni-30/mYOS`
- `cppsever/MineBios`
- `viralcode/vib-OS`
- `whispem/asm.fm`
- `brainboxdotcc/mission-control`
- `osdev0/freestanding-c-hdrs-gnu`
- `Ellicode/protOS-kernel`

## Dossier template

Each repository dossier must contain:

1. exact commit/default branch/license/activity;
2. honest one-paragraph maturity call;
3. source/build/test topology;
4. complete taxonomy inventory with evidence codes;
5. implementation deep dives for every material subsystem;
6. named best ideas and why they work;
7. gaps, bugs, stubs, unproven claims and project hazards;
8. what to learn, what not to copy, and the relevant license boundary;
9. concrete zl/zlOS/agent/hardware implications;
10. exact source paths and identifiers or line references.

## Synthesis rules

- Prefer the smallest project that demonstrates the idea correctly, then use a
  larger project to check scale and edge cases.
- Separate language inability from work nobody has implemented yet.
- Do not turn another project's architecture into zlOS's roadmap automatically.
- Compare complete data/control paths, not isolated class names.
- Prefer measured tests and failure handling over feature count.
- A clean-room lesson describes behavior, invariants and interfaces; it does not
  reproduce implementation text.
- Keep zlOS sequencing evidence-driven: current load-bearing joins come before
  attractive later architecture.
