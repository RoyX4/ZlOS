# Driver, service, and application deep dive for nine architecture OS repositories

Date: 2026-08-21

Source shelf: `/home/roy/Documents/repos/zl-starred-sources`

Purpose: identify the complete meaningful driver and first-party application/service surface of ChittiOS, NexiOS, RustOS, Zinnia, Skift, Fudge, TacOS, NyauxKC, and Cyjon/Fern-Night, then turn the best ideas into clean-room zlOS contracts.

This is a companion to:

- [`ARCHITECTURE_OS_AUDIT_2026-08-21.md`](ARCHITECTURE_OS_AUDIT_2026-08-21.md), the broad architecture pass;
- [`ARCHITECTURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md`](ARCHITECTURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md), the adversarial correction pass; and
- [`ADDITIONAL_EXECUTABLE_EVIDENCE_2026-08-21.md`](ADDITIONAL_EXECUTABLE_EVIDENCE_2026-08-21.md), the narrow local build/run pass.

Where the first audit and the refutation differ, the refutation is authoritative. This report incorporates those corrections rather than repeating the original overstatements.

## Executive result

The repositories have radically different meanings of “driver” and “app”:

- **ChittiOS** has by far the widest locally present product surface: 48 agent manifests, 33 manifest-declared WASM modules, a native browser/media/voice stack, multiple games, productivity tools, agent workflows, networking, filesystems, and broad virtual/physical driver source. Its breadth is real at source level, but many physical drivers are partial, several user-facing tools are privileged kernel agents, and lifecycle/security claims outrun the actual revocation and isolation paths.
- **NexiOS** has a small boot-reachable driver set and a surprisingly large shell/control surface. It boots AHCI/ATA fallback, VirtIO block/network, framebuffer/terminal, PS/2, timers, RTC, and a static IPv4/UDP stack. Its 61 shell command registrations are more mature than its driver registry: the registry records several boot devices as metadata with null load/unload callbacks, so `modprobe` is not a real dynamic driver loader for them.
- **RustOS** is a service operating system, not a large local driver collection. Its defining strategy is to put ordinary hardware support behind a pinned Linux Driver VM and expose authenticated, fail-closed block/network/display/input provider contracts. That gains Linux hardware breadth but makes the local Rust source much narrower than the product surface and creates a large recovery/attestation boundary.
- **Zinnia** has the cleanest locally owned modular driver set: separately compiled dynamic crates for NVMe, VirtIO block/network/GPU, Intel IGC Ethernet, xHCI, USB HID/storage, and ext2, above a conventional Unix kernel device layer. It intentionally has no first-party userspace distribution; framebuffer console, TTY/PTY, sockets, syscalls, and `/init` are platform facilities, not desktop apps.
- **Skift** has a small local microkernel and local Strata service layer. Most of the advertised UI/browser/app ecosystem is externally fetched through locked projects and is absent from this checkout. Its app ideas document is a wishlist, not an implementation inventory. The local Skift backend still returns unsupported for networking, PTYs, process spawning, several mappings, DNS, and sandboxing.
- **Fudge** has the broadest traditional hobby-OS driver catalogue after ChittiOS and a coherent userspace built around typed mailboxes: shell, filesystems, networking tools, a window manager and seven GUI utilities, a Game Boy emulator, demos, and PCX tools. Several headline driver files are only match/probe shells—AHCI, NVMe, and UHCI/OHCI/EHCI are not functioning controllers—and there is almost no automated verification.
- **TacOS** is the smallest genuinely connected desktop slice: framebuffer WM, graphical terminal, PTY, shell, editor, Doom, utilities, local IPC, and a USTAR/TempFS root image. It lacks persistent storage and networking, and its ELF/syscall defects make the user boundary unsafe despite real ring-3 processes.
- **NyauxKC** is a kernel rewrite checkpoint, not an app OS. It has no syscalls, ELF loader, or userland. Its boot-reachable device surface is framebuffer console, ACPI/uACPI, PCI ECAM enumeration, HPET/LAPIC/serial, Intel IOMMU pass-through table setup, and a VirtIO-GPU queue that reaches only display-info query.
- **Cyjon/Fern-Night** is one system split across two languages and repositories: the local pure-assembly Cyjon kernel consumes a pinned external Fern-Night C userspace image. The shipped image has a software WM, console, shell, task/memory/filesystem utilities, PS/2 input, RTC, serial, ACPI/APIC/SMP, an in-memory VFS image, shared libraries, IPC, and a garbage-collector daemon. It has no block/network/audio/USB/GPU driver stack. Two desktop menu actions point at removed binaries.

The best zlOS direction is not to aggregate filenames. It is to combine:

1. Zinnia-style narrow driver modules and explicit bus/device interfaces;
2. RustOS-style authenticated provider ownership and fail-closed handoff;
3. Fudge-style typed event protocols, but with process isolation and explicit replies;
4. Chitti-style capability manifests and broad user workflows, but outside ring 0;
5. NexiOS-style bounded pools/timeouts and adversarial state tests; and
6. a generated, testable product registry that proves every shipped app and driver is build-reachable, startable, stoppable, recoverable, and removable.

## Evidence model and audit boundary

Each item uses the strongest applicable labels:

- **`C` — claimed:** documentation, status table, screenshot, or checked-in log says it exists.
- **`S` — source:** implementation source was inspected.
- **`R` — reachable:** a normal build graph, boot path, manifest, registry, or default image reaches it statically.
- **`T` — covered:** an identified automated test, model, CI job, or explicit scenario covers the path; this does not mean it passed here.
- **`B` — built locally:** this audit family compiled the relevant target from the immutable checkout.
- **`Q` — boot/runtime verified locally:** the target reached and demonstrated the feature in a local VM/runtime.
- **`H` — physical hardware verified locally:** locally observed on physical hardware.

No driver or app in this report earns `Q` or `H`. Zinnia's x86_64 kernel and complete workspace driver set earn narrow `B` from the companion executable pass. NexiOS's kernel and ISO earn `B`, but its target self-tests did not produce a result. Checked-in CI, tests, screenshots, and project hardware tables remain `C`/`T`, never local runtime proof.

“External” has two distinct meanings:

- **external source dependency:** fetched or pinned source outside the repository, such as RustOS's Linux/Buildroot DVM or Skift's Karm/Hideo projects;
- **bundled external content:** a third-party asset shipped in the image, such as a WAD or game data, while the surrounding app code is local.

No source code was copied into zlOS. All recommendations below are behavior, contract, lifecycle, and test ideas for an independent implementation.

## Immutable source ledger

| System | Audited local commit | Local/external boundary relevant to this report |
|---|---|---|
| ChittiOS | `a282d7907c93f39fa1e2ce42741132276e02bb4f` | Main kernel, drivers, 48 agent packages, browser/media stacks local; some assets and nested platform source are third-party |
| NexiOS | `624b9e5f5209e51088754a5f3672c34b1c5df880` | Kernel, drivers, shell, and utilities local; checked-in generated ELF artifacts are not separate apps |
| RustOS | `73db2d5fec0e4dfa72c4c8ccf1cffc17e932861c` | Rust kernel/services local; Buildroot, Linux, standard drivers, and optional NVIDIA-open path external/pinned |
| Zinnia | `1c10cf45381092aa3bb0388b6b63af10859a0e22` | Kernel and driver crates local; uACPI is an initialized submodule; no first-party userspace distribution |
| Skift | `e3822171165cef5535e25602794750c4df1e3530` | Hjert and local Strata layer present; Karm/Hideo/Vaev/Luna/Games/assets fetched from locked external projects |
| Fudge | `5ffb39c7b608a3b117a724d11dc6725857b6c6de` | Kernel, modules, utilities, WM, demos, emulator local |
| TacOS | `c10e450debf628233f5b345e2232dc1fc5d28619` | Kernel, libraries, all listed apps, and WM local; Doom engine local with bundled Doom data |
| NyauxKC | `94879b5b2c5cb1f0ee7c9a52cc73273c574ed4c0` | Kernel only; Limine/OVMF and Rust dependencies fetched; no userland source |
| Cyjon | `b164a3a8d7b1cff0e853944a11078dd749104019` | Assembly kernel local; Fern-Night userland is initialized gitlink `00298942749a1d6a913f8b5dbc3d6483a4c57b96` |

## Cross-repository driver map

This table records meaningful implementation classes, not every helper file.

| Driver/device class | ChittiOS | NexiOS | RustOS | Zinnia | Skift | Fudge | TacOS | NyauxKC | Cyjon/Fern-Night |
|---|---|---|---|---|---|---|---|---|---|
| Firmware/boot | Limine x86, UEFI/direct ARM, m1n1 M2, FDT `[S][R]` | Limine x86; ARM/RISC-V scaffolds `[S][R partial]` | UEFI x86 ownership contracts `[S][R][T]` | x86_64, partial RISC-V `[S][R]` | x86 EFI/opstart; thin RISC-V HAL `[S][R]` | GRUB x86; older ARM/RISC-V paths `[S][R partial]` | Limine x86_64 `[S][R]` | Limine recipes for four, source for x86_64/RISC-V only `[S][R partial]` | Limine x86_64 `[S][R]` |
| ACPI/FDT | ACPI tables, AML subset, FDT `[S][R][T]` | ACPI/APIC/PCI support `[S][R][T]` | local ACPI/PCI HAL; Linux DVM ACPI ecosystem `[S][R][T/external]` | uACPI integration with stub OS hooks, MCFG; DT bus `[S][R partial]` | minimal platform/HAL `[S][R]` | ACPI, APIC/PCI platform modules `[S][R]` | ACPI/APIC `[S][R]` | uACPI + MCFG/DMAR `[S][R]` | ACPI RSDT/XSDT/MADT, LAPIC/I/O APIC; HPET path disabled `[S][R partial]` |
| PCI | ECAM, Apple PCIe partial `[S][R]` | enumeration, BAR, MSI/MSI-X `[S][R][T]` | local ownership/transport; Linux DVM PCI/VFIO `[S][R/external]` | PCI matching and driver binding `[S][R]` | local enumeration/service `[S][R]` | PCI module and device matching `[S][R]` | PCI enumeration `[S][R]` | ECAM enumeration, BAR map; tuple-order defect `[S][R]` | none |
| IOMMU/DMA | Apple DART partial; DMA helpers `[S][R partial]` | DMA layer; IOMMU capability roadmap `[S][R partial]` | L0 IOMMU/VFIO ownership/reset contracts `[S][R][T]` | DMA through driver APIs, no general IOMMU driver identified | no IOMMU identified | conventional DMA modules, no isolation contract | no IOMMU | Intel VT-d pass-through context tables only `[S][R partial]` | no IOMMU |
| Block/storage | virtio-blk, NVMe, AHCI/SATA, USB MSC, SDHCI partial `[S][R]` | AHCI, ATA PIO fallback, virtio-blk `[S][R][T]` | DVM block provider over ivshmem; Linux AHCI/NVMe/virtio/USB external `[S][R][T]` | NVMe, virtio-blk, USB storage `[S][R][B]` | bootfs only in local source | IDE/ATA/ATAPI, virtio-blk; AHCI/NVMe stubs `[S][R partial]` | broken/incomplete NVMe source; initramfs only `[S][R partial]` | no block device; RAMFS initramfs only | memory-backed VFS module only |
| USB | xHCI, HID, MSC, UVC partial, UAC partial `[S][R]` | none | Linux DVM external USB stack | xHCI, hub, HID, storage `[S][R][B]` | none local | UHCI/OHCI/EHCI match stubs `[S][R stub]` | none | none | none |
| Input | USB HID keyboard/mouse/touch, PS/2, PL050, virtio-input, partial I2C/BT HID `[S][R]` | x86 PS/2 keyboard `[S][R][T]` | DVM input transport and `inputd` `[S][R][T]` | PS/2, USB HID, evdev `[S][R][B]` | PS/2 keyboard/mouse via `strata-device`/`strata-input` `[S][R]` | PS/2 keyboard/mouse, generic keyboard/mouse modules `[S][R]` | PS/2 keyboard `[S][R]` | none | PS/2 keyboard/mouse `[S][R]` |
| Display/GPU | firmware FB, EDID, virtio-gpu, VMSVGA, Bochs VBE, multi-output; AGX compute partial `[S][R]` | framebuffer and terminal `[S][R][T]` | authenticated DVM display, GPU DVM, uiserver/Wayland; physical AMD path incomplete `[S][R][T]` | plain FB/DRM, virtio-gpu/virgl/EDID `[S][R][B]` | boot framebuffer locally; Hideo/Karm UI external | VGA/VBE/BGA/Cirrus/partial i915/video modules `[S][R]` | firmware framebuffer/software WM `[S][R]` | Limine FB/flanterm plus virtio-gpu display-info query only `[S][R partial]` | Limine framebuffer shared to one owner + software WM `[S][R]` |
| Network | virtio-net, Intel e1000/e1000e/igb/igc, partial r8169, USB ECM/RNDIS; TCP/IP/TLS `[S][R]` | virtio-net; Ethernet/ARP/IPv4/ICMP/UDP, static config `[S][R][T]` | DVM Ethernet provider; smoltcp TCP/IP and AF_UNIX `[S][R][T]` | IGC, virtio-net; Ethernet/ARP/IPv4/ICMP/TCP/UDP/raw/local `[S][R][B]` | unsupported in local target backend | RTL8139, virtio-net; ARP/IPv4/ICMP/TCP/UDP `[S][R]` | local Unix sockets only | none | none |
| Wireless | WPA2/802.11/CCMP; Broadcom and Intel association incomplete; BT partial `[S][R partial]` | none | external Linux DVM can supply standard stack; no locally proven product path | none | none | none | none | none | none |
| Audio | HDA, virtio-snd, AC97, SB16, partial USB audio `[S][R]` | PC speaker metadata/control | provider-oriented design, no broad local hardware driver | none identified | none local | generic audio + platform modules `[S][R partial]` | none | none | none |
| Camera | UVC partial/test-only `[S][T partial]` | none | possible only through external driver-domain expansion, not current local feature | none | none | none | none | none | none |
| Clock/interrupts | LAPIC/HPET/PIT/PIC/GICv3/ARM timer/RTC/SNTP `[S][R]` | APIC/PIT/timer/RTC/serial `[S][R][T]` | local timer/RTC/IRQ HAL + tested SMP contracts `[S][R][T]` | APIC/HPET/TSC/RTC/serial `[S][R]` | x86 COM/PIC/PIT/CMOS `[S][R]` | APIC/PIC/PIT/RTC/UART/SMP `[S][R]` | APIC/PIT/serial/SMP `[S][R]` | LAPIC/HPET/serial; RISC-V timer/interrupt path `[S][R]` | LAPIC/I/O APIC/RTC/serial/SMP; HPET disabled `[S][R partial]` |
| Power | ACPI S5, partial S3, PSCI, battery/EC partial `[S][R]` | reboot/shutdown hooks; no full PM stack | policy/service contracts, recovery focused; physical breadth incomplete | no complete PM stack identified | `strata-power` service `[S][R]` | reboot utility/platform reset | reboot; no suspend/power drivers | none | none |
| VM integration | VBox clipboard/HGCM, virtio-serial/vdagent, OSC52, 9P `[S][R]` | QEMU-oriented virtio paths | QEMU/KVM DVM transport is core design `[S][R][T]` | virtio set | boot VM focus | virtio block/net, BGA/VGA | QEMU framebuffer | QEMU virtio-gpu only | QEMU/Limine framebuffer |

## Cross-repository first-party product map

| Product class | ChittiOS | NexiOS | RustOS | Zinnia | Skift | Fudge | TacOS | NyauxKC | Cyjon/Fern-Night |
|---|---|---|---|---|---|---|---|---|---|
| Shell/terminal | shell agent, terminal UI, panes/tabs `[S][R]` | 61 shell registrations, terminal and user shell `[S][R]` | native shell + console/Wayland terminal surfaces `[S][R]` | kernel TTY/PTY/fbcon only | local `strata-shell`; fuller UI external | shell, console, wshell GUI `[S][R]` | shell + graphical PTY terminal `[S][R]` | none | shell + graphical console `[S][R external userland]` |
| Files/productivity | files, notes, writer, sheets, slides, calendar, contacts, todo, archive, diff, hex, PDF `[S][R]` | cat/ls/mkdir/rm/more/touch plus VFS daemon `[S][R]` | service foundations, no office suite | none | advertised/wishlist apps external or absent | FS utilities, ext2/MBR services, GUI file viewer `[S][R]` | cat/ls/editor `[S][R]` | none | cat/ls and read-only memory VFS `[S][R external userland]` |
| Browser/network apps | browser, download, git, reader, weather, research, maps, mail, SSH stub, MCP/store `[S][R]` | ping/ifconfig/lspci/dmesg plus no TCP apps | `netprobe`; no general browser app local | none | Vaev/browser external | DNS, IRC, netcat, netdump, web client/server/socket `[S][R]` | none | none | none |
| Media/creative | gallery, media, recorder, paint, synth, radio; native codecs/voice `[S][R]` | framebuffer demo only | uiserver graphics/Wayland; no consumer media suite | no apps | external UI/app ecosystem | PCX tools, demos, Game Boy audio/video `[S][R]` | Doom and QOI/text UI `[S][R]` | wallpaper packaged but not an app | WM wallpaper/cursor, no media apps |
| Games | Breakout, chess, Freedoom, 2048, Minesweeper, Snake, Tetris `[S][R]` | Mandelbrot/spinning-rectangle demo | none beyond qualification apps | none | external `Games` project | Game Boy emulator; donut/mandelbrot demos `[S][R]` | Doom `[S][R]` | none | none active; Taris/3D are under `removed/` |
| Administration | activity, console logs, disk, ops, settings, pass, onboard, sandbox lab `[S][R]` | top/dmesg/meminfo/tasks/jobs/module commands/selftest `[S][R]` | rootd/initd/runtimed/sessiond/devmgrd/procd plus evidence tools `[S][R]` | init/module/syscall platform only | Strata CM/device/fs/input/power | init, kill, log, reboot, hashes, dump/date/random `[S][R]` | init/info/fault and IPC probes `[S][R]` | boot assertions only | ps/free/uptime + `gc.d` daemon `[S][R external userland]` |
| Agent-native | orchestrator, subagents, librarian/researcher/ops personas, skills/MCP/model runtime `[S][R]` | none | agent-oriented engineering evidence, not an in-OS assistant | none | none | actor/event model, not AI agents | none | none | none |

## 1. ChittiOS

### Build and reachability boundary

The local kernel module graph (`kernel/src/lib.rs`, `kernel/src/main.rs`) directly includes architecture, device, filesystem, network, browser, codec, UI, agent, skill, and service layers. Agent packaging is manifest-driven under the repository's package/agent roots. The audit counted **48 manifests**, **33 manifest-declared WASM modules**, **30 packages with `package_ui: true`**, and **three autostart packages**. A manifest and a compiled module establish `S/R`, not successful interaction.

The dual-architecture kernel test workflow and many in-tree `mod tests` provide `T` for substantial portions. The E2E harness is separate from normal CI. `HARDWARE.md` is valuable because it explicitly distinguishes working, partial, test-only, and absent paths, but it is still project evidence, not local `H`.

### Driver and device inventory

| Class | Local implementation and reachability | Maturity/lifecycle boundary |
|---|---|---|
| Boot/platform | Limine x86 path; UEFI stub/direct ARM; Apple M2 through bundled m1n1; FDT and ACPI discovery `[S][R][T]` | Multiple entry paths multiply reset/SMP/MMU parity work; Apple path is specialized |
| Firmware | RSDP/XSDT/FADT/MADT/MCFG; AML subset for `_S5`, `_CRS`, `_STA`, battery and power methods; fw_cfg; partial Apple SMC `[S][R]` | AML is not a general interpreter; EC/battery and SMC are partial |
| CPU/memory | x86/ARM SMP, paging/MMU, PMM/heap, per-task address spaces, ring 3/EL0, OOM/fault isolation, AVX2/XSAVE and NEON paths `[S][R][T]` | Root orchestrator remains privileged; specialized flat-binary tenants are narrower than a general process ABI |
| Interrupt/time | LAPIC, HPET, PIT, PIC, GICv3, ARM generic timer, RTC; SNTP at network layer `[S][R][T]` | Cross-architecture timing and suspend continuity remain harder than source inventory implies |
| PCI/IOMMU/buses | PCIe ECAM, partial Apple PCIe and DART, virtio PCI/MMIO, DesignWare/LPSS I2C partial/test-only `[S][R]` | No broad hot-unplug/rebind state machine; I2C and Apple ownership paths are incomplete |
| Block | virtio-blk, NVMe, AHCI/SATA, USB mass-storage BOT/SCSI, partial SDHCI; GPT/MBR and encrypted volume layer `[S][R][T]` | Recovery, surprise removal, power-loss ordering, and hardware matrix evidence vary by controller |
| Filesystems | ext4 read/write, FAT12/16/32 read/write, exFAT read/write, NTFS read-only, 9P read/write, VFS/mount layer `[S][R][T]` | Breadth increases parser attack surface; persistent recovery and corruption evidence is not uniform |
| USB/input | xHCI control/bulk/iso/hotplug; USB HID keyboard/mouse/touchscreen; PS/2, PL050, virtio-input; partial I2C HID and Bluetooth HID; keyboard layouts `[S][R]` | Several HID paths are partial; Bluetooth pairing/security is incomplete |
| Display | firmware framebuffer, EDID, ramfb, virtio-gpu KMS, VMSVGA, Bochs VBE, multi-output `[S][R][T]` | No general accelerated i915/AMD graphics. VMSVGA and physical paths are not uniformly verified |
| Apple GPU experiment | AGX bring-up/compute dispatch source `[S partial]` | Not a desktop graphics path and not wired as a general inference backend |
| Wired network | virtio-net, e1000/e1000e, igb/igc, partial r8169, USB CDC-ECM; partial RNDIS; ASIX/Realtek identify-only `[S][R]` | Identify-only is not a driver. Several physical NIC paths are unverified |
| Network stack | smoltcp-backed Ethernet, DHCPv4, DNS, ICMP, TCP, UDP, loopback, IPv6 pieces; TLS 1.3 client, HTTP, WebSocket, HLS, MCP `[S][R][T]` | No TLS server; certificate revocation checking absent; bounded connection/retry policy needs one shared contract |
| Wi-Fi/Bluetooth | 802.11 parsing, WPA2-PSK, CCMP; partial Broadcom and Intel paths; partial Bluetooth HID `[S][R partial][T partial]` | Broadcom/Intel cannot complete ordinary scan/association; modern BT pairing is absent |
| Audio | HDA, virtio-snd, AC97, SB16, partial UAC 1.0 `[S][R]` | Device coverage and underrun/recovery behavior vary; USB audio partial |
| Camera | UVC parsing/transfer path `[S][T partial]` | Test-oriented/partial rather than a complete camera product |
| Power | ACPI S5, partial S3, PSCI suspend/poweroff, fixed/GPE power buttons, partial EC/battery, energy-bias hooks `[S][R]` | No complete p-state, thermal, brightness, or resume-device rebind system |
| VM integration | VirtualBox VMMDev/HGCM/clipboard, virtio-serial/vdagent clipboard, OSC52 clipboard, 9P sharing `[S][R]` | Multiple clipboard/sharing backends need common authorization and disconnect cleanup |

Explicitly absent product classes include printing and general cross-device synchronization/sharing. “No source found” should remain a product gap, not be hidden behind broad “hardware support” wording.

### Complete manifest-level app and agent inventory

All entries below are local manifest/source unless noted. “WASM” means the manifest points to a local module; it does not imply an unprivileged general-purpose process.

| # | Package | User-visible function | Reachability/status |
|---:|---|---|---|
| 1 | `activity` | task/activity/status panel and activity mutation tools | WASM; `S/R` |
| 2 | `archive` | pack/unpack archive bundles | WASM; `S/R` |
| 3 | `breakout` | Breakout game | WASM; `S/R` |
| 4 | `browser` | native browser agent using the in-kernel browser engine | native agent; `S/R` |
| 5 | `calc` | calculator | WASM; `S/R` |
| 6 | `calendar` | calendar and event management | WASM; `S/R` |
| 7 | `chess` | full-rule chess UI; model/persona can play black | WASM; `S/R` |
| 8 | `clock` | clock, stopwatch, timer | WASM; `S/R` |
| 9 | `console` | log and kernel-trace viewer | WASM; `S/R` |
| 10 | `contacts` | address book | WASM; `S/R` |
| 11 | `dict` | offline dictionary and custom definitions | WASM; `S/R` |
| 12 | `diff` | side-by-side text diff | WASM; `S/R` |
| 13 | `disk` | read-only disk inventory | native/manifest, no declared WASM; `S/R` |
| 14 | `doc` | bundled documentation browser/router | WASM; `S/R` |
| 15 | `download` | HTTP(S) download and save | native/manifest; autostart; `S/R` |
| 16 | `files` | Synapse filesystem browser and preview | WASM; `S/R` |
| 17 | `freedoom` | software-rendered Freedoom frame app | WASM plus bundled third-party WAD/content; `S/R` |
| 18 | `gallery` | image gallery | WASM; `S/R` |
| 19 | `game2048` | 2048 game | WASM; `S/R` |
| 20 | `git` | init/status/add/commit/log/branch/checkout plus smart-HTTP clone/push | WASM; `S/R` |
| 21 | `hex` | hex viewer | WASM; `S/R` |
| 22 | `librarian` | knowledge-base agent | agent/manifest; `S/R` |
| 23 | `mail` | unified inbox and draft-oriented persona | agent/manifest; `S/R` |
| 24 | `maps` | map/pin card UI | WASM; `S/R` |
| 25 | `media` | image/audio/video controls over native media stack | agent/manifest; `S/R` |
| 26 | `minesweeper` | Minesweeper | WASM; `S/R` |
| 27 | `notes` | durable Markdown notes | WASM; autostart; `S/R` |
| 28 | `onboard` | first-boot setup | agent/manifest; `S/R` |
| 29 | `ops` | read-mostly administrator copilot | agent/manifest; `S/R` |
| 30 | `paint` | canvas painting | WASM; `S/R` |
| 31 | `pass` | secrets-vault UI helper with human-unlock posture | agent/manifest; `S/R` |
| 32 | `pdf` | PDF digest/render/text workflow | WASM; broad filesystem read grant; `S/R` |
| 33 | `radio` | tone/radio toy | WASM; `S/R` |
| 34 | `reader` | feed/article reader | agent/manifest; `S/R` |
| 35 | `recorder` | voice-memo coach | agent/manifest; `S/R` |
| 36 | `researcher` | attenuated web research agent | agent/manifest; `S/R` |
| 37 | `sandbox-lab` | capability attenuation/sandbox demonstration | WASM; `S/R` |
| 38 | `settings` | display and preference settings | WASM; `S/R` |
| 39 | `sheets` | small 4×6 spreadsheet | WASM; `S/R` |
| 40 | `slides` | slide canvas | WASM; `S/R` |
| 41 | `snake` | Snake game | WASM; `S/R` |
| 42 | `ssh` | SSH version exchange/transport and shell agent | explicitly stub/incomplete; `S/R partial` |
| 43 | `store` | registry/package search helper | agent/manifest; `S/R` |
| 44 | `synth` | tone synthesizer | WASM; `S/R` |
| 45 | `tetris` | Tetris | WASM; `S/R` |
| 46 | `todo` | task list and plan mode | agent/manifest; autostart; `S/R` |
| 47 | `weather` | weather UI with HTTP data | WASM; `S/R` |
| 48 | `writer` | long-form writing environment | WASM; `S/R` |

### Native user-facing feature surface beyond manifests

The manifest table understates the native platform. Source under `kernel/src/browser`, `image`, `audio`, `video`, `sound`, `framebuffer`, `kms`, `net`, `synapse`, and the compositor/UI roots includes:

- command/search browser, shell panes and tabs, action panel, editor, clipboard, notifications, themes, screenshot and screencast;
- authentication/login, task activity/top, logs and kernel trace;
- HTML/CSS parsing and layout, flex/forms/events, canvas/SVG, URL/CORS/cache/storage, a JavaScript interpreter/bytecode path, and a limited WASM/page-worker path;
- PNG/JPEG, PDF, WAV/MP3/AAC, H.264/H.265/VP9, MP4/MKV/TS/HLS handling;
- STT/TTS, voice-activity detection, mel/G2P and voice pipeline pieces;
- message-channel integration, MCP, installable skills, local/remote model execution, orchestrator/subagent flows, audit, taint, consent, and citations.

These are product facilities, not 48 additional independent apps.

### Lifecycle, recovery, and security gaps

- Bundled/local skill verification uses a symmetric MAC while naming the primitive as Ed25519. That is not publisher-signature trust.
- Foreign package ingestion is unfinished. Install can ignore placement failure; uninstall can leave artifacts and tool registrations.
- `Orchestrator::kill` is trace-only rather than a real termination/reclamation path.
- Subagent authority is derived from a manifest snapshot rather than strictly from the caller's currently live handles. Revoked or changed authority can therefore diverge from the snapshot.
- The orchestrator and much of the product surface remain in a very large trusted kernel. Browser, codecs, model runtime, networking, package install, and device drivers share a failure domain.
- Many device paths have no common lifecycle model for probe, exclusive claim, start, degraded mode, reset, surprise removal, suspend, resume, revoke, and final resource destruction.
- The audit chain is bounded and process-local; it is useful tamper evidence, not permanent external attestation.

### Clean-room zlOS lesson

Keep the breadth and the typed manifests, but move every agent, browser, codec, productivity app, and game into ordinary isolated processes. A manifest should be compiled into four separate artifacts: requested capabilities, initial live handles, UI declarations, and lifecycle policy. Install must be transactional; uninstall must prove that registrations, files, secrets, processes, and handles are gone. A manifest is a ceiling, never evidence that the authority is currently live.

## 2. NexiOS

### Build and reachability boundary

The real boot graph is in `src/kernel/kernel.cpp`, not the nominal driver registry. Boot initializes the registry, framebuffer, terminal, AHCI with ATA fallback, VirtIO block, VirtIO network, timer, RTC, keyboard metadata, and one program-registry demo. Network configuration is a fixed QEMU-style address (`10.0.2.15/24`, gateway `10.0.2.2`).

`src/kernel/task/taskdefs.cpp` statically defines `init`, `vfsd`, `iocd`, a generic user app, and disabled shell/dmesg entries; shell is brought up separately. The build includes userspace source into the initrd. Checked-in `.elf` and `.d` files are generated artifacts, not extra features.

The local executable pass built the kernel and ISO (`B`). The active CI runs x86 self-tests (`T`), but corrected local runners never produced the target test summary. Do not upgrade the driver or app paths to `Q`.

### Drivers and device services

| Class | Implementation | Reachability and gaps |
|---|---|---|
| Framebuffer/terminal | framebuffer initialization, terminal service, framebuffer metadata driver `[S][R][T]` | firmware/boot framebuffer only; no accelerated graphics |
| Keyboard | x86 PS/2 keyboard `[S][R][T]` | no mouse/USB input |
| Time/interrupts | PIT/timer, RTC, APIC/IRQ, serial; PC-speaker registration `[S][R][T]` | broader ARM/RISC-V timer/IRQ files are preparation rather than parity |
| PCI/DMA | PCI enumeration/BAR, MSI/MSI-X, DMA helpers `[S][R][T]` | no IOMMU isolation or hotplug lifecycle |
| Storage | AHCI, ATA PIO fallback, VirtIO block, generic block layer, I/O control daemon `[S][R][T]` | timeout/reset behavior exists, but ordinary I/O is often bounded busy-polling rather than a complete interrupt/recovery path |
| Network | VirtIO PCI transport/net device; Ethernet, ARP, IPv4, ICMP, UDP `[S][R][T]` | no TCP, DHCP, DNS, or general sockets; boot uses static IP |
| Filesystems/devices | VFS, initrd, devfs, procfs, tmpfs, FAT32, pipes, `vfsd` `[S][R][T]` | persistence/journaling and removable-media lifecycle absent |
| Other architectures | aarch64/riscv early boot, IRQ, serial, RTC, timer/HAL source `[S partial]` | not the production/default driver graph |

The `DriverRegistry` is primarily a fixed metadata table. Registered boot devices commonly have null `init`/`exit` callbacks, while boot directly calls their real initialization. Therefore `modprobe`/`modlist` expose a driver vocabulary, but cannot dynamically start or stop those boot drivers. Unload frees a slot/record without compacting count, another sign that this is not yet a complete module lifecycle.

### Boot services and userspace programs

| Program/service | Function | Status |
|---|---|---|
| `init` | initial userspace process | local, image-reachable `S/R` |
| `vfsd` | VFS daemon, configured as a sporadic server with 2% budget | local `S/R/T` |
| `iocd` | block/I/O control daemon | local `S/R/T` |
| `sh` | interactive shell | local, separately spawned `S/R` |
| `cat` | file output | local `S/R` |
| `ls` | directory listing | local `S/R` |
| `mkdir` | create directory | local `S/R` |
| `rm` | remove path | local `S/R` |
| `more` | paged text viewer | local `S/R` |
| `top` | task/activity display | local `S/R` |
| `prime` | computation/demo utility | local `S/R` |
| `user-app` | generic example user application | local `S/R` |
| `idle` | idle task/program | local `S/R` |
| `fault-probe` | intentional fault probe | local test/diagnostic `S/R/T` |
| `heap-probe` | heap behavior probe | local diagnostic `S/R/T` |
| `kva-probe` | kernel virtual-address boundary probe | local diagnostic `S/R/T` |
| `stack-probe` | stack behavior probe | local diagnostic `S/R/T` |
| `demo` | Mandelbrot plus spinning rectangles | only kernel `ProgramRegistry` entry; `S/R` |
| `dmesg` | log utility | source exists; task-table entry disabled, shell path may expose logs `S/R partial` |

### Shell command surface

`src/services/shell.cpp` registers 61 names. Aliases are counted as registered names because they are user-visible entry points:

- core: `help`, `clear`, `echo`, `uptime`, `tasks`, `meminfo`, `reboot`, `version`, `exit`, `shutdown`, `logout`;
- execution/control: `run`, `runelf`, `load`, `cancel-load`, `jobs`, `wait`, `fg`, `bg`, `disown`, `sleep`, `selftest`;
- modules/programs: `modprobe`, `modlist`, `listprog`;
- environment/shell language: `cd`, `pwd`, `export`, `env`, `which`, `locate`, `alias`, `unalias`, `history`, `type`, `source`, `.`, `set`, `read`, `printf`, `test`, `[`, `shift`, `trap`, `ulimit`, `umask`, `times`;
- directory stack: `dirs`, `pushd`, `popd`;
- filesystem: `mkdir`, `rm`, `rmdir`, `ls`, `cat`, `touch`, `less`;
- diagnostics/network/hardware: `ifconfig`, `ping`, `dmesg`, `lspci`.

Registration proves the dispatch surface, not POSIX parity or successful target execution.

### Lifecycle, recovery, and security gaps

- The driver registry and boot graph are disconnected. A generated registry should own actual probe/start/stop callbacks, dependencies, resources, and state.
- AHCI/VirtIO operations use bounded waits, which is better than infinite spin, but lack a complete asynchronous queue, cancellation, device-reset, and request-replay contract.
- `ProgramRegistry` silently drops a program on capacity/allocation failure.
- CSpace COPY source authority is not checked. Revocation uses a fixed eight-entry traversal queue and can silently truncate a broader descendant graph. Ordinary derivation linkage is missing, so the model is not a complete derivation tree.
- Static network configuration and absence of TCP/DNS/DHCP make most network-facing apps impossible even though `ping`/`ifconfig` exist.
- The target self-test result remains unknown locally. Build success is not runtime driver proof.

### Clean-room zlOS lesson

Use NexiOS's bounded pools, explicit time budgets, and probe programs, but make one registry authoritative. Every driver descriptor should generate build inclusion, device matching, dependencies, live state, admin visibility, and stop/reset callbacks. Any fixed traversal or queue must return `INCOMPLETE` with a continuation token rather than silently claiming completion.

## 3. RustOS

### Build and external-driver boundary

RustOS's local source owns the microkernel mechanisms, service contracts, provider protocols, compositor, and compatibility servers. It intentionally delegates broad device support to a **Linux Driver VM (DVM)** built from pinned Buildroot 2026.05 and Linux 6.12.94 inputs. Standard Linux VirtIO, PCI, USB, NVMe, storage, and GPU drivers are external source and must not be counted as locally implemented RustOS drivers.

The product image and service graph are manifest/staging driven. Local services are build-reachable. Project CI/formal/fault-injection evidence provides extensive `T`, but the physical display/revoke/recovery evidence remains incomplete and no feature here earns local `Q/H`.

### Local hardware/provider implementation

| Boundary | Local source responsibility | External/provider responsibility | Current limit |
|---|---|---|---|
| Boot/HAL | UEFI boot, ACPI, PCI discovery/ownership, RTC, timers, IRQ/MMIO, fallback console/framebuffer `[S][R][T]` | firmware and platform devices | x86_64-focused product path |
| IOMMU/device ownership | L0 assigns devices, owns IOMMU/VFIO policy, reset/revoke ordering `[S][R][T]` | Linux VFIO/device drivers operate assigned hardware | reset and physical recovery are product-defining failed/incomplete gates |
| DVM control | authenticated vsock control channel and provider identity `[S][R][T]` | Linux agent/Buildroot appliance | fail-closed if provider missing; this is correct but creates a boot availability dependency |
| Block | fixed ivshmem request/completion transport and storage broker contracts `[S][R][T]` | Linux AHCI/NVMe/virtio/USB storage drivers | transport correctness does not prove every controller/filesystem |
| Network | fixed shared-memory Ethernet transport; local `netd`/smoltcp `[S][R][T]` | Linux NIC driver | provider deadlines/reset and packet validation remain vital |
| Input | RDI3 bounded input ring and `inputd` ownership `[S][R][T]` | Linux HID/input drivers | overflow, disconnect, and revoke need deterministic user-visible behavior |
| Display/GPU | display protocol, GPU runtime, uiserver, KMS/dmabuf contract, QEMU staged-copy and physical direct-buffer paths `[S][R][T]` | Linux DRM/KMS driver; optional NVIDIA-open external path | current physical release targets a narrow AMD device ID/path and has failed corruption/recovery gates |
| Native fallback | boot console/framebuffer only | — | no broad direct USB/PS2/GPU/network fallback; DVM absence is deliberately fatal for those services |

### First-party services

| Service | Owned function | Reachability/status |
|---|---|---|
| `rootd` | privilege root, restart bootstrap, authority root | local `S/R/T` |
| `initd` | initial service startup | local `S/R/T` |
| `runtimed` | service manager and launch broker | local `S/R/T` |
| `sessiond` | user/session lifecycle | local `S/R/T` |
| `syscalld` | Linux-compatible memory, clock, signal and syscall policy | local `S/R/T` |
| `vfsd` | mount/openat/filesystem policy, FAT service | local `S/R/T` |
| `loaderd` | ELF and Windows PE loading; PE import resolution into bundled `System32` compatibility libraries | local `S/R/T` |
| `devmgrd` | device namespace, provider and hotplug coordination | local `S/R/T` |
| `inputd` | input device/event broker | local `S/R/T` |
| `storaged` | block provider/storage ownership | local `S/R/T` |
| `netd` | AF_UNIX and smoltcp-backed AF_INET/TCP networking | local `S/R/T` |
| `procd` | process policy/lifecycle service | local `S/R/T` |
| `uiserver` | framebuffer owner, compositor, Wayland server, console/terminal windows, cursor/input and scene composition | local `S/R/T partial` |
| Linux DVM agent | translates authenticated provider protocols to Linux drivers | local glue/agent plus external Linux appliance `S/R/T` |

`uiserver` locally implements desktop chrome, an Aero-like translucent theme, `/run/user/1000/wayland-0`, standard registry/compositor/output/seat/pointer/keyboard/shared-memory/surface/frame-callback/damage and xdg-style object paths, console windows for Linux and PE workloads, cursor handling, terminal display, and GPU scene submission. This is a real first-party UI server, not a full consumer app suite.

### First-party apps, qualifications, and compatibility payload

| Package | Function | Status |
|---|---|---|
| `shell` | native shell | local `S/R` |
| `abifuzz` | no-display ABI fuzz/qualification app | local diagnostic `S/R/T` |
| `execsmoke` | executable/loader mode smoke tests | local diagnostic `S/R/T` |
| `ipcbench` | IPC benchmark | local diagnostic `S/R/T` |
| `netprobe` | administrative/no-display network probe | local diagnostic `S/R/T` |
| `smpqual` | SMP qualification app | local diagnostic `S/R/T` |
| `wayclick` | native Wayland pointer/click client | local qualification/UI app `S/R/T` |
| `userdemo2.exe` | Windows PE demonstration program | local compatibility payload `S/R/T` |
| Linux DVM appliance | privileged hardware-provider workload, not an end-user app | staged external OS image `R` |

Bundled PE compatibility libraries under the staged `System32` tree include `ntdll`, `kernel32`, `kernelbase`, `msvcrt`, `ucrtbase`, `vcruntime140`, and `vcruntime140_1`. The PE surface is broader than a single demo stub: imports and console-class execution are implemented, while the GDI bridge remains limited.

### Lifecycle, recovery, and security gaps

- The DVM is an excellent containment boundary only if device assignment, DMA isolation, authenticated transport, timeouts, reset, revoke, and teardown remain correct together. A bug in L0 ownership or shared-memory validation crosses many hardware classes.
- A fail-closed provider prevents fake success but can make core product functions unavailable. User-visible degraded mode and recovery ownership must be explicit.
- Physical GPU evidence records corruption and failed revoke/recovery capture. Source and formal models do not erase that result.
- The service count and evidence machinery are heavy. Boot ordering, mutual dependencies, restart storms, stale provider epochs, and terminal reply custody need one generated graph.
- PR formal coverage is not the same as the nightly formal matrix; optional KVM tracing is not a mandatory PR gate.

### Clean-room zlOS lesson

Adopt the provider contract, not necessarily a permanent Linux appliance. zlOS can start with a driver domain for difficult hardware while keeping native core drivers. Each provider message needs `provider_epoch`, `device_generation`, `request_id`, deadline, buffer provenance, and terminal status. On provider death, the broker must revoke DMA first, fail outstanding requests exactly once, reset/quarantine the device, and only then allow a new provider generation.

## 4. Zinnia

### Build and reachability boundary

Zinnia is a kernel plus driver workspace. Driver crates are compiled as dynamic libraries and included by workspace globs. The companion pass built the x86_64 release kernel and all driver crates (`B`). Runtime loading still depends on an external initramfs and `/init` calling the module insertion path in the required order.

There is no first-party shell, desktop, package manager, media app, game, browser, or service distribution in this repository. The kernel opens `/init`, supplies TTY stdio, and provides Unix facilities. Describing those as apps would be false.

### Dynamic driver crates

| Crate | Function | Evidence and limit |
|---|---|---|
| `drivers/block/nvme` | NVMe controller, namespaces, queues, PRP/request path | local `S/R/B`; no local runtime/hardware proof |
| `drivers/block/virtio_blk` | VirtIO block device | local `S/R/B` |
| `drivers/fs/ext2` | ext2 filesystem operations | local `S/R/B`; filesystem module, not a standalone app |
| `drivers/net/igc` | Intel I225/I226-class Ethernet | local `S/R/B` |
| `drivers/net/virtio_net` | VirtIO network interface | local `S/R/B` |
| `drivers/usb/xhci` | xHCI host controller | local `S/R/B`; worker-stop path has TODOs |
| USB class/core modules | hubs, HID, mass storage integration | local kernel/driver source `S/R/B` |
| `drivers/video/virtio_gpu` | VirtIO GPU, EDID, virgl/DRM resource/context/execbuffer paths | local `S/R/B`; some property/gamma/disable paths incomplete |
| common VirtIO transport | queue/device transport shared by block/net/GPU | local `S/R/B` |

### In-kernel device and platform classes

- ACPI/uACPI and MCFG, with several OS hooks still stubbed/TODO;
- PCI enumeration and driver matching; device-tree matching;
- ns16550 serial, x86 PS/2, input and evdev;
- block BIOs, GPT/partitions, RAM block devices;
- DRM core, plain framebuffer, modes/objects, framebuffer console;
- TTY, PTY, virtual terminals, kmsg, memory/device control nodes;
- Ethernet, ARP, IPv4, ICMP, TCP, UDP, raw/local/loopback sockets;
- RTC, HPET, TSC, APIC and architecture interrupt/time support;
- VFS/tmpfs/devtmpfs/initramfs, pipes, epoll, eventfd, signalfd, timerfd, process/signals and executable loading.

All are platform surface `S/R`; only the compiled workspace earns `B`.

### Lifecycle and recovery gaps

- The dynamic module loader does not yet provide a complete dependency graph, global symbol publication, initialization-array execution, or module removal. A successfully built driver may still be impossible to load safely in an arbitrary order.
- uACPI source is present via submodule, but key OS hooks are stubs/TODO. “ACPI library present” is not complete firmware/device lifecycle.
- Malformed ELF/module input can panic. Module loading is a trust boundary and needs fail-closed validation.
- VM range modifications are nontransactional; partial work can survive failure.
- USB worker shutdown and some serial/DRM operations remain unfinished.
- CI compiles x86_64 but does not boot or execute the driver matrix.

### Clean-room zlOS lesson

Copy the repository shape: one driver per narrow module and typed common buses. Improve the lifecycle: a module descriptor should declare ABI version, supported IDs, dependencies, required capabilities, init/fini, quiesce/reset, suspend/resume, and exported protocols. The loader must stage and validate the full dependency transaction before publishing a single device.

## 5. Skift

### Local versus fetched product boundary

The checked-out Skift repository has a small local kernel/service tree. `project.json` and `project.lock` fetch and pin Karm, Hideo, Ambolt, Vaerk, Luna, Games, assets, and C ecosystem projects. Their source is not in this snapshot. The README's local links to absent `src/apps`, `src/web`, and `src/libs/karm-ui` are stale.

Therefore:

- Hjert and the local Strata services below are local `S/R`;
- the Hideo desktop, Karm UI framework, Vaev browser/web stack, Luna, Games, and asset/product projects are external locked dependencies, not locally audited app implementations;
- `doc/ideas.md` entries are ideas only, not features.

### Local drivers and device services

| Component | Function | Status |
|---|---|---|
| Hjert x86_64 HAL | EFI/platform entry, COM serial, PIC/PIT, CPU/interrupt primitives | local `S/R` |
| Hjert RISC-V HAL | architecture definitions/scaffolding | local `S partial`; not feature parity |
| boot framebuffer | early/output framebuffer | local `S/R` |
| `strata-device` CMOS | RTC/CMOS access | local `S/R` |
| `strata-device` I/O | controlled x86 port-I/O service | local `S/R` |
| `strata-device` PCI | PCI enumeration/device namespace | local `S/R` |
| `strata-device` PS/2 | PS/2 keyboard/mouse device path | local `S/R` |
| `strata-input` | input event broker | local `S/R` |
| `strata-power` | power policy/service endpoint | local `S/R` |

No local block controller, USB host, NIC, audio, camera, or accelerated GPU driver was found in this checkout.

### Local services and user-facing pieces

| Component | Function | Status |
|---|---|---|
| `strata-cm` | component/service manager and runner | local `S/R` |
| `strata-device` | hardware discovery and device namespace | local `S/R` |
| `strata-fs` | filesystem service | local `S/R` |
| `strata-input` | input service | local `S/R` |
| `strata-power` | power service | local `S/R` |
| `strata-shell` | framebuffer/input shell environment | local `S/R` |
| `strata-protos` | shared service protocols | local `S/R` |
| bootfs backend | supplies boot files to userspace | local `S/R` |

The target `karm-sys/skift/sys.cpp` backend still reports unsupported/TODO for major app-enabling operations: pipes, directory creation, process spawn, PTYs, general memory mapping/flush, user/system information, sandboxing, DNS, bundles, and TCP/UDP networking. External UI source cannot make these local target APIs complete.

### Explicit non-features

The following `doc/ideas.md` concepts must not be listed as implemented: application manager, audio/video player, calendar, camera, canvas, chat, code editor, maps, notes, contacts, recorder, slides, task manager, writer, image editor, and terminal. Some may exist in fetched ecosystem projects at pinned revisions, but that was outside this local-source audit.

### Lifecycle, recovery, and security gaps

- Capability receive is currently broken at a core boundary: `Domain::_availableUnlocked()` counts occupied slots while channel receive treats it as free capacity. Empty domains reject transferred caps and full domains can pass an invalid capacity check.
- `Space::map` can retain partial mappings on failure; mapping transactions are not atomic.
- The local app platform lacks process/network/PTY/sandbox primitives needed by the advertised product layer.
- Multiple external repositories increase update, provenance, compatibility, and audit complexity despite lockfile pinning.
- Device ownership is cleanly service-oriented but too small to demonstrate removal, reset, suspend, or provider restart.

### Clean-room zlOS lesson

Keep the object-capability shape and project lock discipline. Do not make app availability depend on stale documentation. Generate a local bill of materials that says, for every product feature, which repository/commit supplies source, which target APIs it needs, and which image contains it. Capability transfer must use a tested `preflight -> reserve -> attach -> commit/rollback` transaction with empty, full, boundary, duplicate, and receiver-death cases.

## 6. Fudge

### Build and reachability boundary

`src/rules.mk` includes the kernel, architecture modules, utilities, window manager, GUI utilities, PCX tools, demos, tests, and Game Boy emulator. Per-directory `rules.mk` files make top-level programs/modules build-reachable. Travis runs `make`, but the `src/test` binaries are demonstrations/manual probes rather than an automated assertion suite.

Fudge's architectural center is not POSIX. It routes typed messages through per-task mailboxes and lets programs bind event callbacks. Userspace assembles device, filesystem, job, window, and network pipelines above small kernel modules.

### Driver/module inventory

| Class | Modules | Reality |
|---|---|---|
| Generic interfaces | `audio`, `block`, `clock`, `console`, `ethernet`, `info`, `keyboard`, `log`, `mouse`, `timer`, `video`, base bus/driver | local `S/R`; protocol/interface modules |
| Platform/CPU | ACPI, APIC, CPUID, FPU, GDB stub, I/O, MSR, PAT, PCI, PIC, PIT, platform, RTC, SMP, UART, 64-bit-mode/system modules | local `S/R` |
| PS/2 | PS/2 bus, keyboard, mouse | local `S/R` |
| Storage | IDE, ATA, ATAPI, VirtIO block | local substantive `S/R` |
| Storage stubs | AHCI and NVMe | PCI match/BAR logging with empty setup and no device enumeration; `S/R stub` |
| Display | BGA, Cirrus, VGA, VBE, video interface | local `S/R` |
| Intel graphics | narrow i915 path for `8086:27AE`, fixed mode, vblank waits | partial; empty IRQ and unimplemented config/colormap operations `S/R partial` |
| Network | RTL8139 and VirtIO network | local `S/R` |
| USB host stubs | UHCI, OHCI, EHCI | match plus empty init/reset/attach/detach; `S/R stub` |
| Audio | generic audio plus architecture device modules/configuration | local `S/R partial`; not a modern broad hardware matrix |

File presence is especially misleading here: AHCI, NVMe, UHCI, OHCI, and EHCI are not functioning drivers. They are useful scaffolds for enumeration and module shape only.

### Services and command-line utilities

| Program | Function | Status |
|---|---|---|
| `init` | system startup and pipeline assembly | local `S/R` |
| `shell` | command shell | local `S/R` |
| `cd`, `pwd`, `ls`, `touch`, `rm` | filesystem navigation/mutation | local `S/R` |
| `echo`, `grep`, `wc`, `help`, `hello`, `motd` | ordinary text/help tools | local `S/R` |
| `date`, `timestamp`, `random` | clock/timestamp/random utilities | local `S/R` |
| `crc`, `md5`, `sha1` | checksum/hash tools | local `S/R` |
| `dump` | data/memory dump utility | local `S/R` |
| `kill`, `reboot`, `log` | process/system/log administration | local `S/R` |
| `elfload`, `elfunload` | user-facing ELF loading/unloading pipeline | local `S/R` |
| `ext2srv` | ext2 service | local `S/R`; read-only in practice because write callback is empty |
| `mbrsrv` | MBR partition service | local `S/R` |
| `dns` | DNS client utility | local `S/R` |
| `irc` | IRC client | local `S/R` |
| `netcat` | network stream tool | local `S/R` |
| `netdump` | packet/network inspection | local `S/R` |
| `socket` | socket utility/test | local `S/R` |
| `webc` | web client | local `S/R` |
| `webs` | web server | local `S/R` |
| `slang` | configuration/scripting-related utility | local `S/R` |

The supporting libraries cover channel/event ABI, filesystem/job/option handling, Base64, CPIO, ELF, 9P, PCF fonts, regex, tar, URL, UTF-8, CRC/MD5/SHA1, PCX, math, sockets, ARP/DNS/Ethernet/ICMP/IPv4/TCP/UDP, and a TCC-related library. An IPv6 header does not constitute a full IPv6 stack.

### GUI, media, games, and diagnostics

| Product | Function | Status |
|---|---|---|
| `wm` | userspace window manager with widgets, interface parser, rendering, text, blit, and colormap support | local `S/R` |
| `wabout` | graphical about dialog/app | local `S/R` |
| `wcalc` | graphical calculator | local `S/R` |
| `wfile` | graphical file viewer/manager | local `S/R` |
| `wrun` | graphical command/program runner | local `S/R` |
| `wsettings` | graphical settings | local `S/R` |
| `wshell` | graphical shell/terminal | local `S/R` |
| `wtest` | GUI/widget test app | local diagnostic `S/R` |
| `gameboy` | Game Boy emulator with CPU, video, and audio | local `S/R` |
| demo | software 3D/plasma-style demo | local `S/R` |
| PCX tools | PCX information, image data, and colormap utilities | local `S/R` |
| `disk`, `disk2` | disk test programs | local manual diagnostics `S/R` |
| `donut`, `mandelbrot` | visual/math demos | local `S/R` |
| `loop`, `timer` | scheduler/timer probes | local manual diagnostics `S/R` |

### Lifecycle, recovery, and security gaps

- Ring-3 tasks and per-task mappings are real; the earlier claim that Fudge lacked them was wrong. The security problem is weak authorization and resource identity, not total absence of address-space separation.
- ELF loading checks little beyond magic and performs unsafe relocation/address work.
- Receiving an oversized mailbox event destroys it and returns the same kind of loss/backpressure signal. A caller cannot distinguish “retry later” from “message irreversibly discarded.”
- Unknown events can be silently dropped by the callback model.
- Fixed pools make bounds visible but have weak failure propagation and no per-principal quotas/capability attenuation.
- Many drivers poll synchronously and lack cancel/reset/remove/suspend state.
- USB and modern storage stubs should be excluded from product claims until they execute I/O.
- There is no meaningful automated integration/driver test gate beyond compilation.

### Clean-room zlOS lesson

Adopt typed event envelopes and reactive services. Fix the semantics: every receive returns one of `DELIVERED`, `BUFFER_TOO_SMALL(size retained)`, `EMPTY`, `PEER_CLOSED`, or `MESSAGE_DROPPED(reason)`. No unsupported event is silently lost across a trust boundary. Run filesystem, network, WM, and emulator services in isolated processes with capability handles, quotas, and restart policy.

## 7. TacOS

### Build and image boundary

The root build recursively includes every userspace application directory and library, then packages a USTAR root image with home/usr/media content. The Limine dependency tracks a moving branch rather than a fixed release. No CI or automated test suite was found.

The default image is a genuine vertical slice: kernel, initramfs VFS, ring-3 ELF processes, PTYs, local sockets, software window manager, graphical terminal, shell, editor, utilities, and Doom.

### Driver/device inventory

| Class | Implementation | Status |
|---|---|---|
| Boot/CPU | Limine BIOS/UEFI x86_64, GDT/IDT, SMP | local `S/R` |
| Firmware/interrupts | ACPI, APIC, PIT | local `S/R` |
| Console/debug | serial, TTY, PTY, flanterm/text paths | local `S/R` |
| Input | PS/2 keyboard | local `S/R`; no mouse device, WM cursor is keyboard-driven |
| PCI | PCI enumeration | local `S/R` |
| Storage | NVMe source exists but is broken/incomplete | `S/R partial`; not a product storage path |
| Filesystems | USTAR initramfs root, TempFS, device nodes, VFS | local `S/R`; no persistent filesystem |
| IPC | AF_UNIX/local sockets and ring buffers | local `S/R` |
| Graphics | boot framebuffer, software WM, per-app shared-memory images, QOI/text rendering | local `S/R` |
| Network/audio/USB/power | no implementation found | absent |

### Applications and user-visible features

| App | Function | Status |
|---|---|---|
| `init` | opens `/dev/stdin0` and `/dev/tty0`, sets `PATH`, forks the shell | local/default `S/R` |
| `shell` | simple shell; `exit` and `cd` built-ins; fork/exec/wait; `-c` support | local `S/R` |
| `cat` | output file contents | local `S/R` |
| `ls` | directory listing | local `S/R` |
| `echo` | output arguments/text | local `S/R` |
| `clear` | clear terminal | local `S/R` |
| `dim` | modal terminal text editor with normal/insert/command-style interaction | local `S/R`; small Vim-like editor, not full Vim |
| `wm` | software window manager; create/title/flip protocol, move/focus windows and keyboard cursor | local/default GUI `S/R` |
| `gterm` | graphical terminal, PTY child shell, limited key mapping | local `S/R` |
| `info` | graphical OS information/logo window | local `S/R` |
| `Doom` | doomgeneric game port | local engine `S/R`, bundled Doom WAD/content is external |
| `helloworld` | sample program | local `S/R` |
| `ipctest` | local IPC test | local manual diagnostic `S/R` |
| `kbqueuetest` | keyboard queue test | local manual diagnostic `S/R` |
| `segfault` | deliberate user fault probe | local manual diagnostic `S/R` |

The WM protocol allocates shared image memory and handles create/title/flip operations. Its own comments acknowledge weak send/error handling (“hope for the best”). That is a useful prototype, not a robust compositor IPC contract.

### Lifecycle, recovery, and security gaps

- Negative syscall numbers can index before the syscall table.
- ELF validation is incomplete; writable-segment permission logic appears inverted; `filesz > memsz` can overflow/copy past allocation; global loader state is race-prone.
- Raw/shared WM buffers and IPC lack strong object/capability ownership and peer-death cleanup.
- Persistent storage is absent, so the editor cannot demonstrate durable file recovery.
- No networking, audio, USB, package/service manager, or real device hotplug exists.
- Processes can fault, but exception/cleanup behavior and resource reclamation are not comprehensively tested.
- Moving bootloader input makes reproduction less deterministic.

### Clean-room zlOS lesson

TacOS proves the value of a small end-to-end desktop milestone: input -> WM -> PTY terminal -> shell -> app. Reimplement that slice with a versioned compositor protocol, checked handles instead of raw shared addresses, per-window damage buffers, explicit peer-close, and a validated transactional ELF loader. Add persistence before calling the editor a product feature.

## 8. NyauxKC

### Build and architecture boundary

The top-level makefile offers x86_64, aarch64, riscv64, and loongarch64 image/run recipes. The Rust source tree implements only x86_64 and RISC-V architecture modules. AArch64/LoongArch linker and QEMU recipes are not implemented ports.

The README accurately marks syscalls, user threads, and ELF loading absent. There is no userspace source, shell, app, service, or program registry. The image packages a wallpaper and `meow.tar`, but assets alone are not apps.

### Boot-reachable device/platform inventory

| Class | Implementation | Reality |
|---|---|---|
| Boot/display console | Limine framebuffer and flanterm | local `S/R`; first visible output only |
| Memory | freelist PMM, slab, VMM and boot self-assertions | local `S/R`; kernel-only |
| Architecture | x86_64 GDT/IDT/TSS/page tables/LAPIC; RISC-V interrupts/page tables | local `S/R partial` |
| Time/debug | x86 HPET/LAPIC timer and serial | local `S/R` |
| Firmware | uACPI initialization and table helpers | local binding/integration `S/R` |
| PCI | ACPI MCFG ECAM enumeration and BAR mapping | local `S/R`, but `pci_map_bar` reverses the tuple's bus/function positions when reading BARs |
| Intel IOMMU | DMAR parse, root/context tables, global translation enable | local `S/R partial`; every context uses pass-through, no translated device isolation |
| VirtIO GPU | modern PCI capability parse, feature negotiation, queue setup, blocking control request | local `S/R partial`; reaches `GET_DISPLAY_INFO` and prints response only |
| VFS | RAMFS and USTAR import from first Limine module | local `S/R partial`; no syscall/user consumer |
| Scheduler | per-CPU kernel thread queue/context switch | local `S/R`; runs one kernel thread then halts/spins |

### Why the VirtIO-GPU path is not a graphics driver yet

`init_virtiogpu()` locates device `1af4:1050`, enables bus mastering/MMIO, walks modern VirtIO capabilities, negotiates version 1, creates queue 0, sets `DRIVER_OK`, submits `GET_DISPLAY_INFO`, and prints the returned modes. It does not create a resource, attach backing, set a scanout, transfer/flush pixels, process interrupts, expose a display object, or recover from reset. It is a useful transport bring-up probe.

### VFS/USTAR failure boundary

The USTAR importer:

- trusts the first module or hits a `todo!` for other selection;
- uses panic/unwrap-heavy parsing and insufficient bounds checks;
- rejects empty files by panic;
- ignores most entry types;
- writes file data starting at the archive header rather than the payload after the 512-byte header; and
- can slice beyond malformed/truncated input.

Thus RAMFS operations exist, but an imported file does not reliably contain its advertised payload.

### Lifecycle, recovery, and security gaps

- No user boundary means no application security model yet.
- `pci_map_bar` reverses the enumerator tuple's bus/function positions and can target the wrong configuration function when reading a BAR.
- IOMMU “enabled” means pass-through tables, not DMA isolation.
- VirtIO polling has no timeout/reset/recovery and panic-heavy negotiation.
- USTAR is an untrusted parser placed in kernel boot with panic and payload-offset defects.
- Scheduler stacks are executable/writable and thread termination panics.
- Four-architecture build recipes overstate two implemented architecture source paths.

### Clean-room zlOS lesson

Treat bring-up milestones as explicit levels: `DISCOVERED`, `NEGOTIATED`, `QUEUE_LIVE`, `FIRST_COMMAND`, `DISPLAY_OBJECT`, `FIRST_FRAME`, `RECOVERY_TESTED`, `HARDWARE_TESTED`. Never call level four a complete GPU driver. Parse initramfs into a validated staging tree outside the live VFS transaction and fuzz it before boot adoption.

## 9. Cyjon and Fern-Night

### Two-repository product boundary

Cyjon's local kernel is pure x86_64 assembly. Its build script initializes and builds the pinned `fern-night` submodule, then copies `fern-night/build/system.vfs` beside the assembly kernel. Fern-Night contains a C implementation of the compatible kernel plus the userspace, shared libraries, desktop, and VFS packer.

This distinction matters:

- the assembly kernel and its PS/2/RTC/serial/ACPI/APIC/SMP mechanisms are local Cyjon source;
- the shipped application source is external pinned Fern-Night source at gitlink `0029894`;
- the top-level `Makefile` invokes `./make || true`, masking any build failure;
- the Fern-Night `Makefile` has the same masking wrapper, although its inner `make` script often exits on individual compilation failures;
- no CI or test suite exists.

### Kernel/device inventory

| Class | Implementation | Status |
|---|---|---|
| Boot/graphics | Limine x86_64 and mandatory 32-bit framebuffer; serial failover output | local kernel `S/R`; halts forever if expected framebuffer missing |
| CPU/memory | higher-half four-level paging, task memory, ring-3 services/processes, SMP task scanning | local `S/R` |
| Firmware/interrupts | ACPI tables/MADT, LAPIC, I/O APIC, GDT/IDT, SMP AP startup | local `S/R`; HPET discovery/init is commented out |
| Time | RTC and LAPIC-derived uptime/scheduling | local `S/R` |
| Input | PS/2 controller, keyboard, mouse | local `S/R` |
| Debug | COM serial driver/logging | local `S/R` |
| Storage | Limine module registered as memory-backed custom VFS | local kernel interface plus external Fern image `S/R`; no disk controller |
| Framebuffer service | maps framebuffer to a single owning process and exposes metadata | local `S/R`; no GPU/display driver |
| IPC/process | fixed task table, streams, bounded IPC message array, memory sharing, ELF/shared-library loader, threads | local `S/R` |
| Missing | PCI, block controllers, USB, networking, audio, camera, power management, persistent filesystem | absent |

The syscall table bounds check is correct; the earlier suspicion of an index error was refuted. The actual hazards are raw user pointers passed into kernel services, weak validation, simple ELF identification, and exception paths that halt indefinitely.

### Shipped Fern-Night daemon and applications

The inner build compiles every top-level `daemon/*.c` and `software/*.c`, shared libraries, cursor/wallpaper, and home files into `system.vfs`. Directories under `removed/` are not built.

| Component | Function | Reachability/status |
|---|---|---|
| `gc.d` | kernel-mode/ring-0-style daemon that recursively closes child tasks and reclaims closed tasks, streams, address spaces, and page tables | external pinned source, image-reachable `S/R` |
| `wm` | default init from Limine `cmdline: wm`; software compositor/window manager | external pinned source, default `S/R` |
| `console` | graphical terminal window, child shell/program, stream parser and input forwarding | external pinned source `S/R` |
| `shell` | prompt, `clear`/`exit`, foreground/background execution, Ctrl-C-style prompt cancellation, IPC forwarding | external pinned source `S/R` |
| `cat` | file display with binary bytes replaced by dots | external pinned source `S/R` |
| `ls` | directory listing with hidden/long modes and colors | external pinned source `S/R` |
| `free` | total/used/free memory display and bar | external pinned source `S/R` |
| `ps` | PID, task memory, and command listing | external pinned source `S/R` |
| `uptime` | day/hour/minute/second uptime display | external pinned source `S/R` |

### Desktop and library features

The default `wm` owns the framebuffer and provides:

- wallpaper and cursor rendering from bundled TGA assets;
- software-composited window objects, z-order/focus, movement, close/minimize controls, dirty zones and framebuffer synchronization;
- taskbar window list and active-window indicator;
- live clock;
- start/menu panel; and
- JSON-defined interfaces plus shared image/font/color/rendering libraries.

The menu exposes **Console**, **Debug**, and **Task Manager**. Only Console is reachable as shipped. `debug` and `top` live under `removed/software` and are not compiled by the active build, so those two menu actions launch missing programs.

Shared libraries built into the image provide color, ELF, fonts, integer/math, software rendering, strings, terminal, JSON, syscall wrappers, random, image/TGA handling, interface widgets, tar/VFS helpers, threading, time, storage, streams, keyboard/mouse and framebuffer access. These are platform libraries, not separate apps.

### Lifecycle, recovery, and security gaps

- Both public `Makefile` entry points mask failure with `|| true`; a green shell exit cannot prove the image was rebuilt.
- Fern-Night is a pinned external gitlink. Cyjon's app functionality disappears if that dependency is absent or incompatible.
- ELF identification is essentially magic-level before trusting many offsets/sizes; loader validation is not safe for hostile binaries.
- Syscalls accept raw user pointers with little range/copy validation.
- Exceptions print/log and halt forever rather than terminating only the faulting process and reclaiming its resources.
- The framebuffer is globally assigned to one process; owner crash/restart/handoff semantics are weak.
- `gc.d` reclaims tasks in a tight infinite scan and is itself highly privileged. Its recursive child close has no explicit bounded work or epoch protocol.
- The VFS image is read-only/memory-backed for practical product purposes; there is no persistent disk, crash recovery, permissions model, network, or package lifecycle.
- Desktop menu/build drift already produces two dead actions.

### Clean-room zlOS lesson

The strongest idea is the bilingual compatibility experiment: keep equivalent kernel/service contracts understandable across implementations. For zlOS, generate the syscall/protocol ABI from one schema and test multiple implementations against the same conformance vectors. Never mask build failure, and generate menus from the built app registry so an absent binary cannot remain launchable.

## Cross-cutting architecture synthesis for zlOS

### 1. One lifecycle state machine for every driver

The dominant cross-repository gap is not missing device classes. It is missing lifecycle semantics. zlOS should make this state machine part of the driver ABI:

```text
ABSENT
  -> DISCOVERED
  -> MATCHED
  -> CLAIMED
  -> STARTING
  -> READY
  -> DEGRADED
  -> RECOVERING
  -> QUIESCING
  -> STOPPED

Any nonterminal state may enter FAILED.
READY/DEGRADED/FAILED may enter REMOVED on surprise removal.
CLAIMED through REMOVED retain a monotonically increasing device generation.
```

Required transitions:

| Transition | Required proof/behavior |
|---|---|
| `DISCOVERED -> MATCHED` | immutable hardware identity and match reason |
| `MATCHED -> CLAIMED` | exclusive claim, resource reservation, capability grant, rollback on failure |
| `CLAIMED -> STARTING` | DMA isolation active before bus mastering; interrupts masked until queues valid |
| `STARTING -> READY` | bounded health probe and published protocol only after full initialization |
| `READY -> DEGRADED` | reason, lost functionality, user-visible effect, retry policy |
| `READY/DEGRADED -> RECOVERING` | stop new work, settle each outstanding request once, reset under a new epoch |
| `* -> REMOVED` | invalidate handles by generation, revoke DMA/IRQ/MMIO, complete requests with `DEVICE_GONE` |
| `QUIESCING -> STOPPED` | no callbacks, DMA, mapped user buffers, IRQs, child workers, or published nodes remain |

Chitti supplies breadth, Zinnia supplies module seams, RustOS supplies provider ownership, and NexiOS supplies boundedness. None alone supplies this complete product contract.

### 2. Separate implementation ownership from product claims

Every shipped feature record should contain:

| Field | Meaning |
|---|---|
| `implementation_origin` | `local`, `submodule`, `fetched_locked`, `system_dependency`, or `bundled_content` |
| `source_revision` | immutable repository and commit/content digest |
| `build_target` | exact artifact that compiles it |
| `image_target` | exact product image/package containing it |
| `registration` | device IDs, service name, app ID, protocols |
| `default_reachability` | boot, autostart, launchable, diagnostic-only, disabled, or removed |
| `evidence` | `C/S/R/T/B/Q/H` with timestamp and target |
| `limitations` | machine-readable stub/partial/unsafe/degraded markers |

This prevents Skift's fetched product layer, RustOS's external Linux drivers, Cyjon's submodule userland, and bundled game data from being reported as the same kind of local source.

### 3. Generate the app registry from build outputs

Dead menu entries in Fern-Night, wishlist apps in Skift, disabled tasks in NexiOS, and manifests with incomplete install/uninstall in Chitti all expose the same failure: multiple registries drift.

zlOS should have one generated app descriptor per built artifact:

```text
app id + version
executable digest
protocol/ABI version
required and optional capabilities
file/data dependencies and licenses
UI actions, file associations, background jobs
health/readiness protocol
stop deadline and restart policy
data migration/uninstall cleanup plan
test and evidence receipts
```

Menus, search, autostart, package install, permissions UI, and uninstall must all consume this registry. No built artifact means no launch action. No successful transactional placement means no registry commit.

### 4. Put apps and rich parsers outside the kernel

Chitti proves how much product surface one system can implement; it also demonstrates why browser, media, PDF, archive, skill/package, network, and model parsing should not share ring 0. Zinnia's narrow kernel/device interfaces and Fudge's message-driven services provide the better placement model.

Minimum isolation groups:

- hardware providers/drivers;
- filesystem and archive parsers;
- image/audio/video/PDF codecs;
- browser/web content;
- model/orchestrator and tool broker;
- secrets/authentication;
- ordinary apps and games.

Each receives only typed shared buffers and opaque handles. A parser crash becomes one failed request and a service restart, not a kernel panic.

### 5. Make request completion unambiguous

Across the surveyed systems, busy polls, destroyed oversized messages, silent drops, missing program registrations, and provider failure can all blur terminal state. Every asynchronous zlOS request must complete exactly once with one of:

```text
OK(result)
UNSUPPORTED(detail)
INVALID_INPUT(detail)
PERMISSION_DENIED(required_capability)
RESOURCE_EXHAUSTED(retryable, minimum_required)
TIMEOUT(stage)
CANCELLED(by)
DEVICE_GONE(generation)
PROVIDER_DIED(epoch)
PARTIAL(progress, continuation)
INTERNAL_ERROR(receipt_id)
```

Retryability is explicit. A queue-full result never destroys the request. A truncated traversal never returns ordinary success. Provider/device epochs prevent late completion from an old instance.

### 6. Driver test matrix

Every device class needs more than a happy-path boot:

| Test family | Cases |
|---|---|
| Discovery | absent device, duplicate match, malformed capability list, multifunction/bridge topology, alternate BAR layout |
| Resource admission | OOM at every allocation, IRQ unavailable, DMA map failure, IOMMU absent, partial MMIO map rollback |
| I/O | zero length, maximum length, unaligned/scatter input, queue full, completion reordering, cancellation |
| Fault/recovery | timeout, controller reset, provider crash, device fatal status, stale completion, repeated reset |
| Removal | surprise unplug during idle and I/O, owner death, module unload, replug with new generation |
| Power | suspend with outstanding work, resume/rebind, clock discontinuity, power loss during write |
| Security | hostile descriptor lengths, raw pointer rejection, DMA outside grant, forged handle/generation, parser fuzz |
| Evidence | source, build, VM boot, emulated I/O, fault injection, and named physical device tracked separately |

### 7. App/service test matrix

| Test family | Cases |
|---|---|
| Registry | artifact missing, dependency missing, stale menu action, disabled/removed app, digest mismatch |
| Launch | capability denied, executable malformed, service unavailable, first-run migration failure |
| Runtime | peer crash, oversized message, queue exhaustion, cancellation, background/foreground handoff |
| Persistence | atomic save, crash mid-save, low disk, corrupt file, incompatible schema, uninstall/reinstall |
| UI | keyboard-only, focus, scaling, small framebuffer, localization, screen reader/contrast contract |
| Security | hostile document/media/network input, clipboard/secrets boundary, capability revocation while active |
| Lifecycle | readiness timeout, graceful stop deadline, forced kill, restart backoff, crash loop quarantine |

### 8. Clean-room integration order for zlOS

This is an architecture order, not a reduced product ambition:

1. **Registry and contracts first:** generated driver/app/service descriptors, evidence fields, device/provider epochs, exact terminal results.
2. **Core buses:** PCI/ACPI/DT, DMA/IOMMU, IRQ, timer, input, framebuffer, block, networking interfaces with the common lifecycle.
3. **Reference vertical slice:** VirtIO block/network/GPU/input in QEMU, plus one physical storage, input, NIC, and display path, all with reset/removal fault injection.
4. **Process/service foundation:** isolated device providers, filesystem, compositor, terminal, network, codec, browser, and tool broker processes.
5. **User product surface:** files, editor, settings, activity, package manager, browser/media and agent workflows generated from the app registry.
6. **Compatibility breadth:** optional Linux driver domain for hardware not yet native, behind the same provider protocol and evidence ledger.
7. **Physical qualification:** named hardware matrix with exact firmware, topology, boot mode, failure/recovery tests, and retained receipts.

## Final judgment

No repository has “all the drivers and all the apps” in one reliable, locally proven package.

- ChittiOS has the widest source breadth and product imagination.
- RustOS has the strongest ownership/recovery/evidence discipline.
- Zinnia has the cleanest modular driver source boundary.
- Fudge has the most reusable event-driven service/app decomposition.
- NexiOS has the strongest bounded/test-oriented kernel habits.
- TacOS has the clearest compact desktop vertical slice.
- Skift has a strong object-capability direction but a misleading local/external product boundary.
- NyauxKC is useful as an honest bring-up-stage example, not a product reference.
- Cyjon/Fern-Night is valuable for readable cross-language architecture and a tiny desktop, while demonstrating why build errors and registry drift must never be masked.

The central zlOS opportunity is to make breadth **provable**. A feature is not “done” because a file, manifest, menu entry, dependency, or screenshot exists. It is done when the immutable source is built into the intended image, registered once, started with least authority, survives failure/recovery tests, stops cleanly, and carries a receipt stating exactly which VM and physical targets demonstrated it.

## Audit validation receipt

- The report was derived read-only from the nine immutable source checkouts and the initialized Fern-Night submodule. No audited source file was edited.
- Tracked-file status was clean and `git diff --check` returned no finding in all nine checkouts after the audit. The nested Fern-Night checkout was checked separately and was also clean.
- This report is the only file written by this deep-dive task.
- No new build, VM boot, or physical-hardware run was performed for this report. The only `B` evidence is inherited from the separately recorded executable-evidence pass; no `Q` or `H` evidence is claimed.
- The report's local Markdown links were resolved against this directory, it ends with a newline, and the final file passed trailing-whitespace and whitespace-error checks.
