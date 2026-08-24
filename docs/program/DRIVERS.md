# Complete platform and driver registry

This registry normalizes the meaningful device breadth found across the 33
audited repositories. It is an implementation target list, not a claim that the
drivers exist in zlOS today.

States:

- `Preserve` — a substantive current zlOS path exists and must not regress.
- `Core` — required for the main x86-64 laptop/desktop/VM product.
- `Expand` — implement after the common contract for useful hardware breadth.
- `Profile` — required only when that product/architecture profile is selected.
- `Later` — retained destination with dependency-late scheduling.
- `Oracle` — software/simulator/reference path kept to verify hardware paths.

Every provider inherits DA-00 through DA-08O and MP-05: immutable descriptor,
required/optional capabilities, match/admit/reserve/start/abort/online/quiesce/
recover/offline/remove lifecycle, authenticated package identity, resource and
DMA authority, bounded waits, cancellation, reset, hot-unplug, peer-death,
telemetry, and host/QEMU/physical evidence appropriate to the claim.

A discovered PCI/USB ID is not a working provider. A shared controller family
uses one implementation plus a revision/quirk table only when the behavior is
actually shared.

## Firmware, architecture and platform providers

| ID | Target | State | Primary phase | Complete boundary |
|---|---|---|---|---|
| PLAT-001 | BIOS/raw-sector acquisition | Preserve | MP-02 | bounded layout, exact origin, recovery |
| PLAT-002 | GRUB/Multiboot acquisition | Preserve | MP-02 | protocol/modules/map/framebuffer validation |
| PLAT-003 | native x86-64 UEFI/GOP acquisition | Preserve | MP-02 | disciplined memory-map exit and image identity |
| PLAT-004 | PXE/TFTP/network acquisition | Later | MP-02 | authenticated payload, bound size/time/origin |
| PLAT-005 | ACPI table admission | Core | MP-02 | RSDP/XSDT/RSDT checks and immutable inventory |
| PLAT-006 | AML/uACPI execution service | Core | MP-05 | bounded methods, OS hooks, policy outside parser |
| PLAT-007 | SMBIOS inventory | Expand | MP-02 | checked strings/records/version fallback |
| PLAT-008 | FDT/device-tree admission | Profile | MP-19 | checked nodes/ranges/phandles and version behavior |
| PLAT-009 | x86 CPUID/MSR/PAT feature provider | Preserve | MP-02 | required/optional features before first use |
| PLAT-010 | x86 microcode update provider | Later | MP-19 | signed revision policy and per-CPU receipt |
| PLAT-011 | x86 topology/SMP provider | Preserve | MP-03 | staged online/offline, IPI/TLB/barrier proof |
| PLAT-012 | x86 PIC | Preserve | MP-05 | legacy fallback, ownership and masking |
| PLAT-013 | x86 LAPIC/x2APIC | Preserve | MP-05 | per-CPU timer/IPI/error lifecycle |
| PLAT-014 | x86 I/O APIC | Preserve | MP-05 | GSI/override/trigger/polarity ownership |
| PLAT-015 | MSI/MSI-X allocator | Core | MP-05 | vector ownership, affinity, mask and teardown |
| PLAT-016 | AArch64 PSCI/EL platform | Profile | MP-19 | EL transition, CPU online/offline and reset |
| PLAT-017 | ARM GICv2 | Profile | MP-19 | distributor/CPU interface and lifecycle |
| PLAT-018 | ARM GICv3/ITS | Profile | MP-19 | redistributors, LPIs/MSI and affinity |
| PLAT-019 | RISC-V SBI platform | Profile | MP-19 | versioned SBI calls and hart lifecycle |
| PLAT-020 | RISC-V PLIC | Profile | MP-19 | source/context priority, ownership, teardown |
| PLAT-021 | PIT | Preserve | MP-05 | calibration/fallback, bounded programming |
| PLAT-022 | HPET | Expand | MP-05 | checked table/counters, wrap and routing |
| PLAT-023 | TSC/deadline timer | Preserve | MP-03 | invariant calibration, drift and fallback |
| PLAT-024 | CMOS/RTC | Preserve | MP-05 | stable read, invalid time and wall-clock separation |
| PLAT-025 | ARM generic timer | Profile | MP-19 | monotonic deadline and suspend continuity |
| PLAT-026 | PL031/Goldfish RTC | Profile | MP-19 | validated wall clock for ARM/VM profiles |
| PLAT-027 | RISC-V timer | Profile | MP-19 | deadline interrupt and wrap behavior |
| PLAT-028 | 16550 UART | Preserve | MP-02 | early panic-safe polling then IRQ mode |
| PLAT-029 | ARM PL011 UART | Profile | MP-19 | early/normal console transition |
| PLAT-030 | Raspberry Pi mini-UART/mailbox | Profile | MP-19 | clock/mailbox bounds and platform identity |
| PLAT-031 | watchdog | Expand | MP-18 | arm/pet/disarm, crash receipt and reboot cause |
| PLAT-032 | hardware RNG | Core | MP-05 | health checks, entropy accounting, fallback |
| PLAT-033 | TPM 2.0 | Later | MP-19 | measured boot, sealed keys, quote and reset policy |
| PLAT-034 | secure-boot key provider | Later | MP-02 | trust roots, rotation, revocation and recovery |
| PLAT-035 | QEMU fw_cfg | Expand | MP-02 | bounded items, exact machine identity and no trust promotion |
| PLAT-036 | KVM paravirtual clock | Expand | MP-05 | versioned stable reads, migration and clock fallback |
| PLAT-037 | legacy APM | Later | MP-15 | compatibility-only power calls and failure reporting |
| PLAT-038 | Apple SMC | Profile | MP-19 | keys, sensors, power events and bounded platform access |

## Bus, enumeration, resource and DMA providers

| ID | Target | State | Primary phase | Complete boundary |
|---|---|---|---|---|
| BUS-001 | PCI legacy configuration | Preserve | MP-05 | bridges/multifunction/ranges and tuple correctness |
| BUS-002 | PCIe ECAM/MCFG | Core | MP-05 | checked segments/bus windows and mappings |
| BUS-003 | PCI bridge/resource allocator | Core | MP-05 | BAR sizing, 64-bit BARs, windows and rollback |
| BUS-004 | PCI hotplug/rebind | Later | MP-05 | surprise removal, generation and competition |
| BUS-005 | Intel VMD discovery | Expand | MP-19 | nested NVMe ownership and reset |
| BUS-006 | ISA/LPC platform bus | Expand | MP-05 | decoded resource ownership and conflicts |
| BUS-007 | ACPI-enumerated platform bus | Core | MP-05 | _HID/_CID/_CRS/_PRS/_STA admission |
| BUS-008 | FDT platform bus | Profile | MP-19 | ranges/interrupts/clocks/resets/dma-ranges |
| BUS-009 | virtio PCI transport | Preserve | MP-05 | feature negotiation, queues, reset and generation |
| BUS-010 | virtio MMIO transport | Profile | MP-19 | version/features/IRQ/queue lifecycle |
| BUS-011 | USB device bus/core | Core | MP-05 | address/config/interface/endpoint ownership |
| BUS-012 | USB hub/topology | Core | MP-05 | depth/power/change/removal and child teardown |
| BUS-013 | I2C core | Expand | MP-19 | controller arbitration, timeouts and ownership |
| BUS-014 | Intel LPSS/DesignWare I2C | Expand | MP-19 | ACPI resources, reset and transfer errors |
| BUS-015 | SPI core/controller | Profile | MP-19 | chip-select/mode/rate/bounds and arbitration |
| BUS-016 | GPIO core/controller | Profile | MP-19 | pin ownership, direction, IRQ, suspend |
| BUS-017 | embedded-controller bus | Core | MP-19 | ACPI EC deadlines, burst mode and event queue |
| BUS-018 | Apple PCIe/DART platform | Profile | MP-19 | m1n1 handover, resources and device isolation |
| BUS-019 | DMA buffer allocator | Core | MP-05 | width/alignment/coherency/pin/unpin/zero |
| BUS-020 | Intel VT-d/DMAR IOMMU | Core | MP-05 | default-deny per-device domains and fault log |
| BUS-021 | AMD-Vi IOMMU | Expand | MP-19 | per-device domains and invalidation |
| BUS-022 | ARM SMMU | Profile | MP-19 | streams/domains/invalidation/faults |
| BUS-023 | Apple DART IOMMU | Profile | MP-19 | stream ownership and translation teardown |
| BUS-024 | no-IOMMU fallback | Core | MP-05 | explicitly non-isolated bounce/pinned policy |
| BUS-025 | OpenCores I2C controller | Profile | MP-19 | controller IRQ/transfer/reset and FDT resources |
| BUS-026 | ISA Plug and Play | Later | MP-15 | isolation/resource discovery with conflict handling |
| BUS-027 | Intel 8237 ISA DMA | Later | MP-15 | channel/width/boundary ownership and bounce buffers |
| BUS-028 | VM86 BIOS-call bridge | Later | MP-15 | bounded legacy firmware call sandbox and register contract |

## Block and storage providers

| ID | Target | State | Primary phase | Complete boundary |
|---|---|---|---|---|
| BLK-001 | common asynchronous block provider | Preserve | MP-06 | queue/cancel/flush/discard/remove contract |
| BLK-002 | NVMe PCI controller/namespaces | Preserve | MP-06 | admin/I/O queues, PRP, reset, format geometry |
| BLK-003 | AHCI/SATA | Core | MP-06 | ports, NCQ/DMA, ATAPI, reset and hotplug |
| BLK-004 | legacy ATA/IDE PIO | Expand | MP-06 | identify/LBA/errors/deadlines and fallback |
| BLK-005 | ATA bus-master DMA | Expand | MP-06 | PRD bounds, cache coherency and reset |
| BLK-006 | ATAPI optical | Expand | MP-06 | packet commands, media change and read-only |
| BLK-007 | virtio-blk | Core | MP-06 | negotiated limits, multiqueue and reset |
| BLK-008 | SCSI command core | Core | MP-06 | CDB/sense/capacity/timeout/retry contract |
| BLK-009 | USB MSC BOT | Preserve | MP-06 | CBW/CSW recovery, stalls and residue |
| BLK-010 | USB MSC UAS | Expand | MP-09 | streams/tags/task management and fallback |
| BLK-011 | SDHCI | Expand | MP-19 | voltage/clock/tuning/ADMA/card removal |
| BLK-012 | SD memory card | Expand | MP-19 | identify/capacity/bus width/error recovery |
| BLK-013 | eMMC | Profile | MP-19 | partitions, reliable write and lifetime data |
| BLK-014 | Raspberry Pi SD/mailbox | Profile | MP-19 | platform DMA/mailbox and card lifecycle |
| BLK-015 | floppy controller | Later | MP-15 | legacy profile, media geometry and timeout |
| BLK-016 | RAM disk | Core | MP-06 | bounded memory ownership and snapshot |
| BLK-017 | loop block device | Expand | MP-06 | file-backed cycle/resize/flush semantics |
| BLK-018 | read-only optical/ISO media | Expand | MP-06 | media change and immutable semantics |
| BLK-019 | network block provider | Later | MP-19 | authenticated loss/reconnect/flush semantics |
| BLK-020 | encrypted volume mapper | Later | MP-06 | key handles, sectors, integrity and revoke |
| BLK-021 | GPT parser | Preserve | MP-06 | primary/backup CRC, overflow and overlap checks |
| BLK-022 | MBR/extended parser | Preserve | MP-06 | bounded chain, overlap and cycle rejection |
| BLK-023 | storage multipath/provider failover | Later | MP-19 | identity, ordering and no split brain |
| BLK-024 | crash/power-cut block simulator | Oracle | MP-06 | reorder/drop/tear/fail every operation |
| BLK-025 | Apple ANS/NVMe controller | Profile | MP-19 | platform queues, DMA, reset and honest stub rejection |

## USB controller and class providers

| ID | Target | State | Primary phase | Complete boundary |
|---|---|---|---|---|
| USB-001 | UHCI host | Expand | MP-09 | schedule/control/bulk/interrupt and teardown |
| USB-002 | OHCI host | Expand | MP-09 | descriptors, root hub and recovery |
| USB-003 | EHCI host | Expand | MP-09 | async/periodic schedules and companion handoff |
| USB-004 | xHCI host | Preserve | MP-05 | rings/interrupters/ports/cancel/reset/remove |
| USB-005 | USB HID class | Preserve | MP-07 | descriptors/reports/usages/multiple interfaces |
| USB-006 | USB mass-storage class | Preserve | MP-06 | BOT/UAS selection and SCSI binding |
| USB-007 | USB Audio class 1/2 | Expand | MP-09 | descriptors, rates, isochronous recovery |
| USB-008 | USB Video class | Expand | MP-09 | probe/commit, formats, isochronous frames |
| USB-009 | CDC-ACM serial | Expand | MP-19 | line state, endpoints, disconnect |
| USB-010 | CDC-ECM network | Expand | MP-08 | descriptors, link, frames and hotplug |
| USB-011 | RNDIS network | Later | MP-08 | bounded messages, compatibility and teardown |
| USB-012 | USB Bluetooth HCI | Later | MP-19 | transport ownership and radio service binding |
| USB-013 | USB printer class | Later | MP-14 | job/status/cancel and privacy |
| USB-014 | USB smart-card/security token | Later | MP-19 | APDU/token authority and removal |

## Input providers

| ID | Target | State | Primary phase | Complete boundary |
|---|---|---|---|---|
| INPUT-001 | PS/2 controller | Preserve | MP-07 | dual-channel detection, reset and errors |
| INPUT-002 | PS/2 keyboard | Preserve | MP-07 | scan sets, LEDs, layouts and repeats |
| INPUT-003 | PS/2 mouse | Preserve | MP-07 | packets, wheel/buttons, resync and acceleration |
| INPUT-004 | USB HID keyboard | Preserve | MP-07 | boot/report modes and rollover |
| INPUT-005 | USB HID mouse | Preserve | MP-07 | buttons/wheel/high-resolution motion |
| INPUT-006 | generic HID report parser | Core | MP-07 | bounded descriptors, usages and collections |
| INPUT-007 | USB touch/tablet/stylus | Expand | MP-19 | contacts/pressure/tilt/calibration |
| INPUT-008 | USB gamepad/joystick | Expand | MP-12 | axes/buttons/hats/deadzones/remap |
| INPUT-009 | I2C-HID transport and decoder | Core | MP-07 | HID descriptor, full reports, no diagnostic-only claim |
| INPUT-010 | virtio-input/tablet | Core | MP-07 | negotiated events, absolute/relative modes |
| INPUT-011 | ARM PL050 keyboard/mouse | Profile | MP-19 | controller lifecycle and input binding |
| INPUT-012 | VMware absolute mouse | Expand | MP-19 | guest coordinate/focus/disconnect |
| INPUT-013 | Bluetooth HID | Later | MP-19 | pairing, encryption, reconnect and revoke |
| INPUT-014 | switch/dwell/accessibility devices | Later | MP-10 | semantic actions, timing and remapping |

## Display and graphics providers

| ID | Target | State | Primary phase | Complete boundary |
|---|---|---|---|---|
| GPU-001 | software raster/compositor oracle | Preserve | MP-07 | pixel-correct bounded fallback and differential hash |
| GPU-002 | UEFI GOP framebuffer | Preserve | MP-07 | validated mode/pitch/format and handoff |
| GPU-003 | VBE framebuffer | Expand | MP-15 | legacy modes and safe mapping |
| GPU-004 | VGA text/planar fallback | Preserve | MP-07 | panic/legacy console ownership |
| GPU-005 | Bochs/BGA display | Preserve | MP-07 | mode validation, framebuffer and teardown |
| GPU-006 | QEMU ramfb/simplefb | Expand | MP-07 | immutable mode and safe scanout |
| GPU-007 | virtio-gpu 2D/KMS | Preserve | MP-07 | resources/backing/scanout/flush/fences/reset |
| GPU-008 | virtio-gpu virgl/3D | Later | MP-19 | contexts/execbuffer/capabilities/isolation |
| GPU-009 | VMware SVGA/VMSVGA | Expand | MP-19 | modes/FIFO/cursor/fence/reset |
| GPU-010 | Cirrus legacy display | Later | MP-15 | compatibility-only bounded modes |
| GPU-011 | Intel Gen9/9.5 display | Core | MP-07 | VBT/EDID/pipes/planes/panel/hotplug/recovery |
| GPU-012 | Intel blitter/render rings | Core | MP-07 | commands/fences/hang/reset/software agreement |
| GPU-013 | older Intel display generations | Expand | MP-19 | generation-specific register providers |
| GPU-014 | AMD display/GPU provider | Later | MP-19 | firmware/modesetting/queues/reset/isolation |
| GPU-015 | NVIDIA display/GPU provider | Later | MP-19 | firmware/modesetting/queues/reset/isolation |
| GPU-016 | Raspberry Pi framebuffer/display | Profile | MP-19 | mailbox/scanout/blanking and ownership |
| GPU-017 | 3dfx legacy accelerator | Later | MP-15 | compatibility/raster fixture |
| GPU-018 | hardware cursor contract | Core | MP-07 | ownership, clipping, atomic update and fallback |
| GPU-019 | display connector/EDID/DP/HDMI core | Core | MP-07 | bounded EDID, hotplug, link and mode policy |
| GPU-020 | color management/HDR provider | Later | MP-19 | profiles, transfer, precision and fallback |
| GPU-021 | hardware video-codec provider | Later | MP-19 | formats/buffers/fences/reset/software fallback |
| GPU-022 | FPGA/custom accelerator provider | Later | MP-19 | required/optional capabilities and simulation |
| GPU-023 | Apple AGX compute/display research provider | Profile | MP-19 | firmware/queues/memory/fences/reset; no desktop claim until wired |
| GPU-024 | SPI/ILI9341 display | Profile | MP-19 | panel init, pixel transfer, rotation, damage and recovery |
| GPU-025 | EGA legacy display | Later | MP-15 | compatibility modes and bounded planar access |

## Network and radio providers

| ID | Target | State | Primary phase | Complete boundary |
|---|---|---|---|---|
| NIC-001 | loopback | Core | MP-08 | full socket semantics and fault injection |
| NIC-002 | virtio-net | Preserve | MP-08 | queues/features/offloads/reset/multiqueue |
| NIC-003 | Intel 8254x e1000 | Preserve | MP-08 | RX/TX rings, link, reset and physical variants |
| NIC-004 | Intel e1000e/I219 | Core | MP-08 | NVM/PHY/carrier/reset and ThinkPad receipt |
| NIC-005 | Intel igb | Expand | MP-08 | multiqueue/MSI-X/offloads and reset |
| NIC-006 | Intel igc/I225/I226 | Expand | MP-08 | generation link/queue/timing behavior |
| NIC-007 | Realtek RTL8139 | Expand | MP-08 | legacy rings, overflow and reset |
| NIC-008 | Realtek RTL8168/8169 | Expand | MP-08 | descriptor revisions/PHY/offloads/reset |
| NIC-009 | NE2000 | Later | MP-15 | emulator/legacy compatibility |
| NIC-010 | Cadence GEM | Profile | MP-19 | platform DMA/PHY/link |
| NIC-011 | USB CDC-ECM adapter | Expand | MP-08 | class lifecycle and physical dongle matrix |
| NIC-012 | USB RNDIS adapter | Later | MP-08 | compatibility protocol and containment |
| NIC-013 | USB ASIX Ethernet | Expand | MP-08 | real driver beyond identify-only |
| NIC-014 | USB Realtek Ethernet | Expand | MP-08 | real driver beyond identify-only |
| NIC-015 | tap/tun virtual interface | Expand | MP-15 | packet authority, queues and teardown |
| NIC-016 | bridge/VLAN interface | Later | MP-18 | loop prevention, policy and lifecycle |
| RADIO-001 | Intel AX201-class Wi-Fi | Core | MP-19 | firmware, scan, association, WPA, regulatory |
| RADIO-002 | Broadcom Wi-Fi | Later | MP-19 | selected chipset/firmware and regulatory proof |
| RADIO-003 | generic 802.11 service/provider ABI | Core | MP-19 | scan/auth/roam/power/privacy boundary |
| RADIO-004 | Bluetooth HCI/core | Later | MP-19 | controller, keys, pairing and profiles |
| RADIO-005 | cellular modem | Later | MP-19 | SIM/eSIM, data/SMS/calls and carrier lifecycle |

## Audio, camera, sensor, power and peripheral providers

| ID | Target | State | Primary phase | Complete boundary |
|---|---|---|---|---|
| MEDIA-001 | Intel/PCI HDA controller | Core | MP-09 | codecs/widgets/routes/DMA/IRQ/reset |
| MEDIA-002 | AC97 | Expand | MP-09 | codecs/mixer/DMA/underrun |
| MEDIA-003 | virtio-snd | Core | MP-09 | negotiation/streams/queues/reset |
| MEDIA-004 | Sound Blaster 16 | Later | MP-15 | DSP/DMA/IRQ legacy compatibility |
| MEDIA-005 | PC speaker | Preserve | MP-09 | bounded tone service and no global blocking |
| MEDIA-006 | USB Audio | Expand | MP-09 | UAC formats/clock/isochronous recovery |
| MEDIA-007 | microphone/capture provider | Core | MP-09 | privacy indicator, permission and revoke |
| MEDIA-008 | UVC camera | Expand | MP-09 | formats/frame buffers/timestamps/unplug |
| PWR-001 | ACPI power button/reboot/shutdown | Core | MP-19 | policy capability and final state receipt |
| PWR-002 | ACPI battery/AC adapter | Core | MP-19 | units/rates/invalid data/event lifecycle |
| PWR-003 | ACPI embedded controller | Core | MP-19 | bounded transactions and event handling |
| PWR-004 | thermal zones/fans | Core | MP-19 | trip points, failsafe and policy service |
| PWR-005 | display/backlight | Core | MP-19 | safe ranges, restore and user policy |
| PWR-006 | suspend/resume/wake | Later | MP-19 | ordered quiesce, device state and recovery |
| PWR-007 | Raspberry Pi watchdog/fan/PWM/clocks | Profile | MP-19 | mailbox/GPIO lifetime and failsafe |
| SENSOR-001 | ambient light | Later | MP-19 | units/rate/calibration/privacy |
| SENSOR-002 | accelerometer/gyroscope | Later | MP-19 | units/rate/orientation/permission |
| SENSOR-003 | GPS/location | Later | MP-19 | permission, accuracy, retention and revoke |
| SENSOR-004 | generic sensor fusion service | Later | MP-19 | timestamped streams and provenance |
| PERIPH-001 | printer provider | Later | MP-14 | capabilities/status/job/cancel |
| PERIPH-002 | scanner provider | Later | MP-14 | capabilities/preview/capture/privacy |
| PERIPH-003 | keyboard backlight/LED | Expand | MP-19 | ownership, ranges and suspend restore |
| PERIPH-004 | parallel port | Later | MP-15 | ownership, status/timeout and printer compatibility |

## Virtual-machine and guest-integration providers

| ID | Target | State | Primary phase | Complete boundary |
|---|---|---|---|---|
| VM-001 | QEMU/KVM machine profile | Preserve | MP-01 | exact machine/device identity and gates |
| VM-002 | virtio RNG | Expand | MP-05 | entropy health/accounting and reset |
| VM-003 | virtio balloon | Later | MP-19 | pressure/accounting and reclaim safety |
| VM-004 | virtio serial | Expand | MP-15 | bounded ports, identity and disconnect |
| VM-005 | virtio 9P/shared folder | Later | MP-15 | path/permission/cache/disconnect authority |
| VM-006 | qemu-guest/vdagent integration | Later | MP-15 | authenticated clipboard/display/input seams |
| VM-007 | VMware guest integration | Later | MP-15 | clock/mouse/SVGA/shared state authority |
| VM-008 | VirtualBox VMMDev/HGCM | Later | MP-15 | versioned messages, shared folders/clipboard |
| VM-009 | OSC52 clipboard bridge | Later | MP-15 | explicit user consent and bounded payload |
| VM-010 | Linux Driver VM transport | Later | MP-19 | attested provider, IOMMU, epochs and recovery |
| VM-011 | headless/server hardware profile | Later | MP-19 | serial/network management and no-display boot |
| VM-012 | virtio console/ports | Expand | MP-15 | negotiated ports, bounded streams and disconnect cleanup |

## Filesystem providers attached above block/VFS

These are not block drivers and cannot inherit block proof automatically.

| ID | Target | State | Primary phase | Complete boundary |
|---|---|---|---|---|
| FSP-001 | zlfs v2+ | Preserve | MP-06 | directories/permissions/transactions/recovery |
| FSP-002 | FAT12/16/32 | Core | MP-06 | boot/removable interoperability and corruption |
| FSP-003 | ext2 read/write | Expand | MP-06 | allocation/links/permissions/fsync/recovery |
| FSP-004 | ext4 selected feature set | Later | MP-06 | explicit compatible features and journal policy |
| FSP-005 | ISO9660 | Core | MP-06 | read-only optical/boot media and malformed trees |
| FSP-006 | tmpfs/RAMFS | Core | MP-06 | quotas, ownership and memory pressure |
| FSP-007 | devfs | Core | MP-06 | handle-mediated devices and dynamic removal |
| FSP-008 | procfs/system-information view | Expand | MP-06 | generated read-only facts and permissions |
| FSP-009 | sysfs/device-information view | Expand | MP-06 | generated attributes and controlled writes |
| FSP-010 | devpts/PTY filesystem | Core | MP-15 | session ownership and peer cleanup |
| FSP-011 | initramfs/USTAR/TAR | Core | MP-02 | length-first archive admission and path safety |
| FSP-012 | 9P filesystem | Later | MP-15 | remote identity/cache/disconnect semantics |
| FSP-013 | FUSE-like userspace provider | Later | MP-15 | bounded IPC, mount authority and peer death |
| FSP-014 | network filesystem | Later | MP-19 | authentication/offline/cache/conflict policy |
| FSP-015 | Mollen MFS compatibility/import | Later | MP-15 | isolated parser, no native trust promotion |
| FSP-016 | ValiFS/VaFS immutable image | Later | MP-15 | checked compressed image, signatures and process isolation separate |
| FSP-017 | exFAT | Expand | MP-06 | large removable media, allocation/checksum/corruption/recovery |
| FSP-018 | NTFS read-only importer | Later | MP-15 | explicit supported features, isolated parser and no writes |
| FSP-019 | eventfs | Later | MP-15 | bounded event objects, poll semantics and peer cleanup |
| FSP-020 | pipe/FIFO filesystem | Core | MP-15 | ownership, capacity, atomic writes and peer death |
| FSP-021 | socket filesystem | Core | MP-15 | namespace, permissions, lifecycle and stale endpoint cleanup |
| FSP-022 | legacy custom-FS import adapters | Later | MP-15 | MRAFS, SAF, SSFS and `abc` parsed in quarantine with explicit format identity |

## Provider priority and proof rule

The initial physical product profile is x86-64 ThinkPad plus QEMU. That makes
Intel Gen9.5 display, I219, xHCI HID/MSC, NVMe, PS/2, ACPI, HDA, GOP/BGA,
virtio-gpu/net/block and e1000 the first provider set. Expansion targets remain
in the registry and become actionable after their common service contract is
stable.

Each provider has separate evidence fields for source, build, shipped image,
init, user route, deterministic test, QEMU, host harness and native hardware.
Controller hardware evidence does not propagate to class drivers; for example,
physical xHCI mass-storage proof does not prove a physical USB keyboard or UVC
camera.
