# Driver and application audit taxonomy

Date: 2026-08-21

Purpose: prevent the deeper 33-repository pass from collapsing thousands of
files into “has drivers” or “has apps.” Every material item is classified by
behavior, placement, reachability, lifecycle, trust boundary, and proof.

## Evidence ladder

Each driver, service, application, utility, or game receives the strongest
evidence actually established for that item:

```text
claim -> source -> build-reachable -> image/registry-reachable
      -> launched/probed -> scenario-tested -> recovery-tested -> hardware
```

Additional flags:

- `stub`: function/API exists but returns fake success, `ENOSYS`, zero, or
  hardcoded data;
- `disabled`: implementation is intentionally not registered or compiled;
- `external`: source/product comes from a separately fetched tree, port, image,
  submodule, or userspace repository;
- `simulated`: UI/demo behavior does not cross the claimed real boundary;
- `false-green`: the outer gate returns success while required behavior failed,
  was skipped, was absent, or only printed a failure;
- `unsafe-boundary`: untrusted lengths, pointers, IDs, DMA, formats, or device
  state can cross into privileged code without complete admission;
- `lifecycle-incomplete`: probe/start exists without bounded stop/reset/remove,
  cancellation, cleanup, or restart.

No repository-wide grade overwrites item-level evidence.

## Driver inventory taxonomy

### Platform and firmware

- BIOS, UEFI, Multiboot, Limine and custom handover;
- ACPI/uACPI/ACPICA, AML, EC, power and battery;
- device tree/FDT;
- SMBIOS and firmware inventory;
- boot modules/initramfs/archive admission;
- CPU discovery, microcode, topology, per-CPU state and SMP;
- PIC, APIC/IOAPIC, MSI/MSI-X, GIC, PLIC and interrupt ownership;
- PIT, HPET, APIC timer, TSC, RTC and wall/monotonic clocks;
- IOMMU, DMA domains and firmware-reserved memory.

### Bus and device discovery

- PCI/PCIe config, ECAM, bridges, multifunction and resource sizing;
- ISA/LPC, ACPI-enumerated devices and platform devices;
- USB topology, hubs, address/configuration and class binding;
- I2C, SPI, GPIO, UART/serial and embedded-controller buses;
- virtio transport and feature negotiation;
- hotplug, detach and provider competition.

### Storage and block devices

- ATA PIO/DMA, IDE and AHCI/SATA;
- NVMe controller/namespace/admin/I/O queues;
- virtio-blk;
- USB mass storage, SCSI commands, BOT/UAS;
- SD/eMMC/SDHCI;
- floppy/optical/ramdisk/loop/network block;
- partition discovery: GPT/MBR;
- identify, capacity, block size, read, write, flush, trim/discard;
- async completion, cancellation, queue/ring exhaustion;
- timeout, reset, error decoding, partial transfer and device removal.

Filesystem code is recorded separately from the block driver so an ext2 file
does not prove its storage transport and an NVMe probe does not prove durable
files.

### USB and human input

- UHCI/OHCI/EHCI/xHCI controller cores;
- command/event/transfer rings and interrupters;
- root ports, hubs and topology;
- control transfer admission and endpoint lifecycle;
- HID boot keyboard/mouse;
- generic HID report parser, layouts/usages and multiple devices;
- touchpad/touchscreen/tablet/multitouch;
- gamepad/joystick;
- USB storage, audio, video/camera, serial, Bluetooth transport;
- suspend/resume, unplug/cancel and class teardown.

### Display and graphics

- firmware framebuffer/GOP/VBE;
- VGA/BGA/simple framebuffer;
- virtio-gpu;
- Intel/AMD/NVIDIA display discovery and modesetting;
- connector/EDID/VBT/hotplug, clocks, pipes, planes and panel power;
- framebuffer mapping/cache type, scanout ownership and page flip;
- hardware cursor;
- 2D blitter, command rings, fences and hang/reset;
- 3D/raster/shader/texture/compute;
- DRM/KMS-like user ABI or compositor provider;
- software renderer/fallback and differential scene oracle.

### Networking and radio

- virtio-net, E1000/e1000e/I219, RTL8139/8169 and other NICs;
- link/carrier, MAC, MTU, RX/TX rings, checksum/offload and reset;
- Ethernet, ARP, IPv4/IPv6, ICMP, UDP, TCP and local sockets;
- DHCP, DNS, TLS and HTTP are protocol/service evidence, not NIC evidence;
- Wi-Fi transport/firmware/scan/association/WPA/regulatory policy;
- Bluetooth HCI, pairing/key storage and profiles;
- loopback/tap/tun/bridge/routing/firewall;
- packet loss/reorder/duplicate/delay and exhaustion tests.

### Audio and media devices

- PC speaker/SoundBlaster/AC97/HDA;
- `virtio-snd` and USB Audio;
- codec/controller discovery, DMA periods, formats and rates;
- mixer, volume/mute, per-client streams and exclusive/shared policy;
- underrun/overrun, hotplug, suspend and recovery;
- microphone/recording and privacy indication;
- UVC camera and frame pipeline;
- media decode is a parser/application concern unless hardware acceleration is
  actually wired.

### Other devices

- keyboard LEDs/backlight and laptop brightness;
- battery/AC/thermal/fan/sensors;
- power button, reboot/shutdown, sleep and wake;
- RNG/TPM/secure boot/key storage;
- printer/scanner;
- network boot and remote console;
- watchdog;
- virtualization/guest agents;
- GPU/FPGA/HDL peripherals.

## Filesystem and storage-service inventory taxonomy

Block transport and filesystem behavior are recorded separately. For every
filesystem or storage service, inventory:

- VFS object model, namespace and per-process/session roots;
- initramfs/archive admission, devfs, procfs/sysfs-like views and pseudo-files;
- partition-to-filesystem binding, mount/unmount and removable-media ownership;
- page/file/directory cache, dirty-state tracking and writeback;
- file, directory, symlink, hard-link, rename and traversal semantics;
- identity, permissions, ownership, ACL/capability and mount authority;
- sparse files, large offsets, allocation, truncation and disk-full behavior;
- flush/fsync/barrier and the exact point at which durability is promised;
- journaling, copy-on-write, checksums, transactions and rollback;
- corrupt-media/header/size/offset/count admission before mutation;
- concurrent open/read/write/rename/unlink and process/service death;
- crash/power-loss recovery, fsck/repair, backup/restore and format migration;
- device removal, late completion, cache invalidation and remount behavior.

An archive parser, ext2 source file, mounted initramfs or successful NVMe read
cannot prove durable filesystem mutation. Each layer keeps its own evidence.

## Driver lifecycle contract

Every real driver is evaluated against:

```text
Absent -> Discovered -> Matched -> Admitted -> ResourcesReserved -> Starting
Starting -> Online -> Quiescing -> Offline -> Removed

Matched|Admitted|ResourcesReserved|Starting -> Aborting -> Discovered|Absent
Online|Quiescing|Recovering -> Failed
Failed -> Recovering -> Online|Offline|Quarantined
Offline|Quarantined -> Rebinding|Removed
```

Required questions:

1. How is exact hardware matched, including unknown revisions?
2. Which BAR, port, IRQ, DMA pages, firmware object and dependency handles are
   reserved, and who owns them?
3. Are all descriptor/table/firmware lengths admitted before use?
4. Does initialization publish `Online` only after every required step?
5. Does every wait have a monotonic deadline and typed timeout result?
6. What happens to in-flight operations on timeout, cancel, peer death, reset,
   unplug, process death and system shutdown?
7. Can stop/reset/remove run twice safely?
8. Are IRQs quiesced before queues/mappings are freed?
9. Does DMA stop before pages/domains are revoked?
10. Can a failed provider leave the safe fallback usable?
11. Which simulator/QEMU/hardware receipt proves each transition?
12. If admission or startup fails, does `abort(plan)` release every reserved
    BAR, port, IRQ, DMA page/domain, firmware object, dependency and handle?
13. Does dependency loss invalidate generations, fail outstanding requests once,
    and converge to degraded, offline or quarantined state without leaked claims?

Probe or enumeration without this lifecycle is `partial`.

## Service inventory taxonomy

Every service is inventoried independently from the driver or app that consumes
it. Required families are:

- Driver Supervisor and Device Manager;
- File/VFS, namespace, mount and file-operation portal;
- Window/compositor, display broker and surface manager;
- Session, login/authentication, input/focus/grab and user-environment policy;
- typed IPC/event broker and shared-object manager;
- package, dependency, install/update/rollback and launch service;
- NIC/socket, route, DHCP, resolver, TLS/request and network-policy services;
- AudioServer/mixer plus isolated image/font/archive/PDF/audio/video decoders;
- logger, telemetry, metrics, crash collection and symbol service;
- clipboard, notifications, file picker and other user-consent portals;
- accessibility, localization, keyboard layout, theme and settings/config;
- wall/monotonic clock, sensor and power-policy services;
- rescue, repair, backup/restore and provenance/evidence registry;
- public-demo lease, authenticated remote display gateway and cleanup reaper;
- agent orchestrator, deterministic tool broker, consent and receipt reviewer.

For each service record:

```text
stable role and protocol version
authority and exposure profile
dependencies, startup order and readiness condition
authenticated endpoints and per-client quotas
request IDs, deadlines, cancellation and exactly-once terminal results
health, metrics, degraded state and dependency-loss behavior
crash budget, backoff, quarantine and restart/reconnect policy
checkpoint/restore and data migration
reverse shutdown deadline and peer/handle cleanup
management authorization and audit/redaction policy
```

A daemon name, manifest, socket or autostart entry proves registration only. It
does not prove readiness, complete mediation, restart or safe teardown.

## Application inventory taxonomy

### Desktop/session shell

- login, authentication, lock, logout and switching;
- compositor/window manager, workspaces, panels, dock, launcher and menus;
- notifications, clipboard, drag/drop and file picker;
- settings/control center;
- accessibility, keyboard navigation, screen reader, magnifier, contrast and
  input remapping;
- localization, text input, fonts, layouts, timezone and regional formats.

### Core productivity and files

- terminal/shell and process/job control;
- file manager, search, archive, removable media and trash;
- text/code editor and document viewers;
- calculator, notes, diff, checksum, conversion and clipboard tools;
- office/document/spreadsheet/presentation/PDF workflows;
- print/scan.

### Internet and communications

- browser and its process split;
- download/upload and persisted history/bookmarks;
- email, calendar, contacts and messaging;
- SSH/remote shell, remote desktop/VNC and file transfer;
- network configuration, diagnostics and firewall UI.

### Media and creation

- image viewer/editor/paint;
- audio player/library/mixer/recorder;
- video player/library/editor;
- camera;
- font/image/PDF/archive/media decoders;
- 2D/3D renderer/demo/CAD/model viewer;
- asset metadata, thumbnails and indexing.

### System and development tools

- system monitor, process manager, logs, crash reports and profiler;
- disk/partition/filesystem usage and repair;
- service/driver/device manager;
- package manager, software centre, installer, updater and rollback;
- compiler, assembler, linker, debugger, IDE, source control and build tools;
- ABI/hex/framebuffer/font/input/network diagnostics;
- rescue/recovery tools and backup/restore.

### Games and demonstrations

- puzzles/board/card;
- arcade/action;
- simulations and cellular automata;
- 3D/graphics demonstrations;
- emulators and ports;
- deterministic rules, input/timer behavior, save/replay, audio and resource
  quotas are recorded separately from merely opening a window.

### Agent and automation applications

- typed plan/editor;
- tool broker and consent UI;
- subagent/role/resource monitor;
- skill/plugin/package manager;
- provenance/audit viewer;
- remote gateway and public demo control plane.

## Application completeness contract

For every claimed application:

1. **Identity:** unique stable ID, exact display name, category, version and
   package/source identity.
2. **Reachability:** a real user route—launcher, menu, shell, file association,
   boot/session action, or explicit protocol invocation.
3. **Construction:** resource admission and window/process creation either
   succeeds completely or reports a visible typed refusal.
4. **Behavior:** draw/output, input/events, timers/background work and file or
   network operations are wired to the claimed behavior.
5. **State:** state ownership, persistence, restore, migration and corruption
   behavior are explicit.
6. **Authority:** only declared handles for files, network, clipboard, devices,
   other processes and session data.
7. **Failure:** parser/service/app crash remains process-local; retries and
   unsaved data behavior are visible.
8. **Accessibility:** focus, keyboard operation, semantic names, contrast,
   scaling and reduced motion are tested where visual UI exists.
9. **Proof:** direct deterministic logic tests plus QEMU/graphical route and,
   when device-dependent, hardware evidence.
10. **Removal:** uninstall/upgrade revokes authority, stops processes and
    removes registrations without deleting retained user data by accident.

An application name, screenshot, source directory, package recipe, registry
row, or blank shell is insufficient.

## User-facing behavior dimensions

The deeper pass records not only app names but:

- window model: single/multiwindow, tabs, modal dialogs, fullscreen;
- input: mouse, keyboard, touch, gamepad, shortcuts, drag/drop;
- state: ephemeral, RAM, files, database, settings, cloud/remote;
- output: text, 2D, 3D, audio, video, print, network;
- integration: file types, clipboard, notifications, URL/protocol handlers;
- lifecycle: start, ready, background, suspend, crash, restart, close;
- privacy/security: data read, external sinks, secrets, device access;
- performance: bounded working set, frame/deadline behavior, background load;
- evidence: logic, route, screenshot/hash, scenario, restart and hardware.

## Clean-room extraction rule

For every external feature, retain only:

- public behavior and user workflow;
- interface/state-machine shape;
- failure/recovery case;
- test idea and expected observable result;
- placement/authority lesson;
- provenance and license boundary.

Do not copy implementation source. Similarity in filenames, class names, UI
layout, assets, icons, data, game content, firmware or third-party ports is not
authorized by this research.
