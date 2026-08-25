# Current zlOS driver and application baseline

Date: 2026-08-21

Scope: the active shared checkout at `/home/roy/Documents/repos/zl-linux`, read
against its current `kernel/HANDOFF.md`, `docs/EXECUTION-ROADMAP.md`, shared
`kernel/SOURCES`, app registries, host gates, and representative driver paths.

## Why this baseline exists

The external-repository inventory is useful only if it does not recommend
rebuilding what zlOS already has. This document separates four destination
states:

- **retain and harden:** a real zlOS mechanism already exists;
- **factor/migrate:** behavior exists but is coupled to the kernel or one
  device/application;
- **complete:** source is present but gated, partial, simulated, or unreachable;
- **add:** no substantive implementation was found in the current tree.

This was a read-only audit of a heavily dirty shared checkout. No zlOS file was
edited, no generated source was refreshed, and no build was run. Three narrow
read-only host checks were executed: the 53-app inventory, the seven earlier
game rules, and the eight newer game rules.

## Evidence vocabulary

- `[S]`: substantive source exists.
- `[R-build]`: included by the active source/build graph.
- `[R-image]`: present in the produced image or package graph.
- `[R-init]`: called by the normal boot/service initialization path.
- `[R-user-route]`: reachable through an actual user launch/dispatch route.
- `[T]`: a deterministic host or target test exists.
- `[V-host]`: the named host check passed during this pass.
- `[QEMU]`: an emulator scenario is recorded for the behavior.
- `[H-host-harness]`: a host tool exercised physical hardware outside native
  zlOS boot.
- `[H-native-boot]`: the native zlOS artifact exercised the physical behavior.
- `[P]`: partial, gated, unreachable, or only a narrow proof.
- `[F]`: a check or source path contradicts the broader claim.
- `[stub]`: a callable diagnostic or fixed/skeletal surface exists but the
  advertised provider/application behavior is not implemented.

Evidence is per behavior. A compiled driver file does not make every device
lifecycle path work, and a game-rules test does not prove that the game is
launchable from the desktop.

## Current physical/product spine — retain

The following are not donor opportunities; they are current assets that
external ideas must join without replacement:

1. Native BIOS, raw-sector, multiboot64, and UEFI build paths with one shared
   source inventory.
2. A measured compositor with workspaces, damage tracking, cached wallpaper,
   time-based animation, software/hardware cursor paths, frame telemetry, and
   a software rendering fallback.
3. Persistent `zlfs` named files on NVMe, a Files application, editor save
   path, and an independent raw USB `ZLLOG` journal.
4. The physical ThinkPad Intel Gen9.5 display/modeset investigation, including
   the write-combined framebuffer correction and retained host-harness evidence;
   native zlOS does not yet arm the physical write/modeset path.
5. Real QEMU networking through virtio-net, ARP/IPv4, DNS, TCP, TLS 1.3, HTTP,
   HTML/CSS, PNG, bounded JavaScript, and the browser.
6. xHCI keyboard, pointer, and Bulk-Only/SCSI mass-storage paths with bounded
   timeout/recovery and detailed persistent diagnostics.
7. A zl application catalogue substantially larger than the original desktop,
   backed by shared rendering/widget code and direct host rule tests for many
   games.

## Driver and platform inventory

### Build/platform and observability

| Current surface | Evidence | Current boundary | Destination action |
|---|---|---|---|
| One shared `SOURCES` inventory for four kernel targets | `[S][R-build][T]` | generated zl C, GDT/trampoline/entry and runtime remain intentionally target-specific | retain; require shipped-image inventory/hash receipt per target |
| Native UEFI witness and kernel chainload | `[S][R-build][R-init][H-native-boot]` | exact pre-kernel witness plus kernel image; not a general package boot manager | retain as boot oracle; add assertion payload matrix, not another loader |
| ZLLOG RAM/emergency rings and raw USB journal | `[S][R-build][R-init][T][H-native-boot]` | early/IRQ-safe evidence plus later durable checkpoints | retain permanently; later logging service imports rather than replaces it |
| PCI configuration enumeration | `[S][R-build][R-init]` | current devices are matched directly by in-kernel code | factor typed BDF/resource ownership and generated match inventory |
| PIC/APIC/IOAPIC, PIT/TSC and SMP | `[S][R-build][R-init][T][H-native-boot]` | current services app presents them as logical services, not isolated processes | harden lifecycle, CPU ownership, shootdown, calibration and failure receipts |

### Display, rendering, and graphics

| Current surface | Evidence | Exact status | Destination action |
|---|---|---|---|
| UEFI linear framebuffer/GOP | `[S][R-build][R-init][T][H-native-boot]` | physically proven fallback and current scanout path | never remove; keep as recovery oracle |
| VGA text/basic path | `[S][R-build][R-init]` | legacy console/fallback surface | retain only as low-risk diagnostic fallback |
| Bochs Graphics Adapter | `[S][R-build][R-init][T][QEMU]` | QEMU-oriented modesetting path | retain as emulator provider behind common display contract |
| virtio-gpu | `[S][R-build][R-init][T][QEMU][P]` | scanout/2D proof path; not the production hardware north star | factor as provider; test resource/scanout/reset/teardown independently |
| Intel Gen9.5 display | read/probe `[S][R-build][R-init][T]`; write/modeset `[S][R-build][T][H-host-harness][P]` | host harness lit the physical panel, but no native kernel caller arms `lt_armed`; zlOS cannot currently light the panel through this write path | retain as the physical north star; require native artifact/nonce/display-effect proof before `[H-native-boot]` |
| Intel blitter command generation | `[S][R-build][T][P]` | commands are compiled and host-pinned to hardware-known dwords | complete load-bearing submission only behind software differential oracle |
| Intel ring submission | `[S][R-build][T][P]` | arithmetic tested; MMIO arm defaults off and lacks production hardware proof | keep gated; add ownership, fence, hang/reset and repeated-start proof |
| Intel hardware cursor | `[S][R-build][T][P]` | image/mapping arithmetic exists; install arm is gated and unproved | prove map/enable/move/disable/recovery on exact hardware |
| Software framebuffer primitives | `[S][R-build][R-init][T][H-native-boot]` | real current renderer and correctness fallback | retain as conformance oracle for every accelerated path |
| Software 3D renderer | `[S][R-build][R-user-route]` | cube/mesh demonstrations and Renderer app; no general GPU API | retain behavior; move scene/mesh contracts out of individual apps |
| Fonts, AA text, icons and shared widget catalogue | `[S][R-build][R-user-route][T]` | current desktop/app visual foundation | retain; cache glyph/surface work and add accessibility/input semantics |

### Input and USB

| Current surface | Evidence | Exact status | Destination action |
|---|---|---|---|
| PS/2 keyboard/pointer fallback | `[S][R-build][R-init][T]` | legacy input route through central queue | retain fallback; bounded controller waits and exact decoder tests |
| xHCI controller | `[S][R-build][R-init][T][H-native-boot]` | physical evidence reaches controller plus BOT/SCSI storage behavior; it does not automatically promote every class driver | factor controller core from class policy; add interrupt-driven completion and hotplug lifecycle |
| USB HID keyboard | `[S][R-build][R-init][T][QEMU]` | boot-style keyboard/report flow and repeat control; no exact physical HID receipt found in the current handoff | add full descriptor/usage layouts, multiple keyboards, disconnect proof and a separate physical receipt |
| USB pointer | `[S][R-build][R-init][T][QEMU]` | report flow, acceleration/coasting and exact button/wheel logging; current recorded proof is QEMU/host-side | add generic HID parser, multiple pointers, per-device settings and a separate physical receipt |
| USB Bulk-Only mass storage | `[S][R-build][R-init][T][H-native-boot]` | bounded 4 KiB staging, READ/WRITE(10), sense, sync-cache, CSW validation and one reset recovery | place behind generic async block provider; add unplug/cancel/concurrency and larger transfers |
| I2C-HID touchpad | `[S][R-build][P][stub]` | transport-only path reads raw diagnostic bytes for five seconds; no x/y/button/multitouch decoder and no session input-event route | implement and prove descriptor decoding, normalized events, multitouch, suspend/resume and detach before calling it an input provider |
| USB hubs/classes beyond HID and MSC | absent/partial | no complete general class/provider catalogue established | add only through common USB topology/class contracts |

### Storage, files, time, and persistence

| Current surface | Evidence | Exact status | Destination action |
|---|---|---|---|
| NVMe namespace/block path | `[S][R-build][R-init][T][H-native-boot]` | backs `zlfs`; interface is not yet shared with xHCI storage | factor one asynchronous block API with identify/capacity/read/write/flush/cancel |
| `zlfs` | `[S][R-build][R-init][R-user-route][T][H-native-boot]` | persistent named files, editor and Files integration; crash/disk-full/concurrency scope remains narrower than a general FS | retain format/data; add transactions, rename, directories/metadata and power-cut matrix |
| Numbered RAM/file compatibility paths | `[S][R-build][R-init]` | legacy compatibility, not destination storage API | migrate callers and remove only after zero-caller receipt |
| Raw ZLLOG journal | `[S][R-build][R-init][T][H-native-boot]` | independent of filesystem readiness and intentionally fixed-format | retain as evidence device, not user filesystem |
| RTC/CMOS and 100 Hz tick | `[S][R-build][R-init]` | wall-clock/system display support | separate wall/monotonic clocks, validation, timezone and modern deadline timer provider |
| Archive Manager | `[S][R-build][R-user-route]` app | currently a user-facing archive model/view, not proof of a hardened general archive service | move parsing into restricted process and test hostile archives |
| No AHCI/SATA provider in shared `SOURCES` | absent | external donors have AHCI but zlOS current physical/storage spine is NVMe/USB | add only after block-provider contract, with reset/flush/error gates |

### Networking and communications

| Current surface | Evidence | Exact status | Destination action |
|---|---|---|---|
| virtio-net NIC | `[S][R-build][R-init][T][QEMU]` | real QEMU route; Network app exposes `eth0` as virtio-net | factor NIC provider from protocol stack and prove reset/ring/carrier/removal |
| Ethernet/ARP/IPv4 | `[S][R-build][R-init][T][QEMU]` | connected QEMU protocol path | retain and build deterministic packet-loss/reorder/fragment/error tests |
| DNS | `[S][R-build][R-init][R-user-route][T][QEMU]` | browser-connected resolver path | move behind resolver service with cache/TTL/cancel/negative response semantics |
| TCP | `[S][R-build][R-init][R-user-route][T][QEMU]` | browser-connected implementation | add retransmission/window/OOO/close/reset/exhaustion simulator and socket API |
| TLS 1.3/X.509/roots/entropy/crypto | `[S][R-build][R-init][R-user-route][T][QEMU]` | connected browser security path | isolate trust policy, certificate/time failures and constant-time/secret lifecycle concerns |
| HTTP/HTML/CSS/layout/PNG/JS/browser | `[S][R-build][R-init][R-user-route][T][QEMU]` | real bounded browser product path | split network, parser/decoder and web-content processes after 64-bit process boundary |
| Loopback UI model | `[S][R-user-route]` | Network app presents `lo`; this does not itself prove a complete socket loopback provider | leave separate evidence state |
| Physical Intel I219 Ethernet | absent | exact ThinkPad device identified; no driver | add after NIC contract; require dongle/carrier-specific hardware receipts |
| Intel AX201 Wi-Fi | absent | exact ThinkPad device identified; no driver | later provider plus firmware/regulatory/authentication design; do not block wired networking |

### Audio, camera, Bluetooth, power, and other devices

| Feature | Current status | Destination action |
|---|---|---|
| Audio device/provider/mixer | no substantive current driver in shared `SOURCES` | add a bounded AudioServer contract first; then HDA/virtio/USB providers and deterministic mixer tests |
| Camera | USB identity may be observed during enumeration, but no camera class/media pipeline | add UVC only behind USB class, DMA/frame and privacy/consent contracts |
| Bluetooth | physical Intel Bluetooth identity is visible, but no usable Bluetooth stack | add much later; separate HCI transport, pairing/keys, profiles and radio policy |
| Battery/thermal/power management | no complete provider/service established | add read-only ACPI-backed telemetry first, then explicit power policy |
| Suspend/resume | no complete system lifecycle proof | require every driver to quiesce/rebind before claiming suspend |
| IOMMU/DMA isolation | no complete enforced per-device domain established | default-deny device domains are a later security gate; passthrough is not completion |

## Application inventory

### Measured source shape

The current app implementation is substantial:

- ten `apps_*.zl` modules plus the main `kernel.zl` contain 16,514 lines;
- the central registry claims 47 catalogue entries;
- the reference checker identifies 53 expected reference applications;
- eight additional original zlOS windows are outside that reference list;
- therefore the source contains **61 named application implementations**,
  plus the `All Applications` catalogue window itself.

That is source inventory, not 61 proven launchable applications. The current
registry has one missing named app and one blank substitute, described below.

### Original/base zlOS applications

| ID | Application | Main behaviors | Current evidence/boundary |
|---:|---|---|---|
| 0 | Terminal | shell, console command routing, app launch | `[S][R-build][R-user-route][H-native-boot]`; kernel-resident, not an isolated terminal process |
| 1 | System Monitor | runtime/frame/system counters | `[S][R-build][R-user-route][T]`; should consume typed telemetry service later |
| 2 | About | system/product information | `[S][R-build][R-user-route]` |
| 3 | Snake | interactive game | `[S][R-build][R-user-route]`; not covered by the two rule suites run here |
| 4 | Menu | desktop/start menu | `[S][R-build][R-user-route]`; shell surface, not ordinary app destination |
| 5 | Browser | DNS/TCP/TLS/HTML/CSS/PNG/JS | `[S][R-build][R-user-route][T][QEMU]`; trusted in-kernel product today, must split into processes |
| 6 | Settings | persistent visual/input/system preferences | `[S][R-build][R-user-route][T]` |
| 7 | Run | command/application launcher | `[S][R-build][R-user-route]`; exact authority and parsing must become process/service contract |
| 8 | Paint | pixel drawing | `[S][R-build][R-user-route]` |
| 9 | 3D | rotating software-rendered cube | `[S][R-build][R-user-route]` |
| 10 | zlOS animation | animated visual demo | `[S][R-build][R-user-route]` |
| 11 | Pointer | mouse/pointer visualizer | `[S][R-build][R-user-route]` |
| 12 | Text Editor | text editing and named-file save path | `[S][R-build][R-user-route][T][H-native-boot]`; retain behavior, migrate behind File service |
| 13 | Files | named-file listing/open/edit workflows | `[S][R-build][R-user-route][T][H-native-boot]`; retain behavior, migrate behind VFS/File service |

### Utilities

All ten have source/name/size/icon/catalogue routes:

1. Clipboard;
2. Colour Picker;
3. Regex Tester;
4. Base Converter;
5. Text Diff;
6. Checksum;
7. Unit Converter;
8. Sticky Notes;
9. Keyboard Tester;
10. Benchmark.

The important migration distinction is state and authority:

- Clipboard becomes a session-scoped service with MIME/type, ownership,
  history/privacy and explicit cross-app read/write grants.
- Sticky Notes, Text Diff, Checksum and editor-like utilities use File handles,
  never direct global buffers.
- Regex/text parsing should be bounded and fuzzed in user space.
- Benchmark reports exact workload/device/backend/evidence identity instead of
  one undifferentiated score.

### System and diagnostic applications

The registry contains:

- Clocks & Timers;
- System Info;
- Calculator;
- Kernel Log;
- Renderer;
- Hex Viewer;
- Framebuffer;
- Console (`tty1`);
- Font Atlas;
- Disk Usage;
- Services;
- Archive Manager;
- Image Viewer;
- Network.

Their architectural value is high, but many currently read kernel state
directly. The destination is not to delete them; it is to make them the first
clients of typed services:

- Kernel Log -> Logger/ZLLOG reader;
- Renderer/Framebuffer/Font Atlas -> Display/Render diagnostics;
- Hex Viewer/Archive/Image Viewer -> restricted File/Decoder services;
- Disk Usage -> File/Storage service;
- Services -> supervisor health/lifecycle stream;
- Network -> NIC/socket/config service;
- System Info -> read-only consolidated inventory provider.

The Services app currently names `xhci.service`, `nvme.service`,
`virtio-net.service`, `apic.service`, `smp.service`, `i915.service`,
`zlfs.mount`, `rtc.service`, `i2c-hid.service`, and `sched.service`. Those are
useful desired boundaries, but most remain in-kernel logical labels rather than
restartable isolated services. The UI must not be used as evidence that service
isolation already exists.

### Games

The named game surface is:

- Snake;
- Word Guess;
- Tic-Tac-Toe;
- Nim;
- Tower of Hanoi;
- Lights Out;
- Connect Four;
- Maze;
- Tetris;
- Pong;
- Breakout;
- Minesweeper;
- 2048;
- Conway's Life;
- Asteroids;
- Invaders;
- 15 Puzzle;
- Reversi;
- Simon;
- Sokoban;
- Flappy;
- Missile Command;
- Blackjack;
- Frogger.

That is 24 named games. The host rule evidence is stronger than a typical
hobby-OS app catalogue:

- `[V-host]` the earlier seven-game suite ran 118 checks with zero failures,
  covering Word Guess, Tic-Tac-Toe, Nim, Tower of Hanoi, Lights Out, Connect
  Four, and Maze;
- `[V-host]` the newer eight-game suite ran 126 checks with zero failures,
  covering Reversi, Sokoban, 15 Puzzle, Blackjack, Frogger, and Missile
  Command plus its other included rule paths;
- the output itself states eight newer games, but only the behavior families
  explicitly printed by the runner are credited here;
- these are direct shipping-source rule checks, not window/render/input-route
  proof.

Games are valuable application-runtime probes. Preserve them as deterministic
fixtures for event routing, timers, rendering, storage, random-seed replay,
audio, process isolation and resource quotas; do not put their state at fixed
global physical addresses in the destination process model.

## Newly discovered application reachability defect

The current 53-app checker is false green.

Observed output:

```text
26  Maze  game  34  exists NO  size yes  icon yes  dock/shell/boot
...
all 53 reference apps resolve to an id in the tree
```

The source cause is deterministic:

1. `APP_MAZE = 34`.
2. `reg_exists()` rejects every ID from 34 through 39 as “never allocated.”
3. No separate shell/dock/boot route to `APP_MAZE` was found.
4. The checker only adds an app to `missing` when its name cannot be resolved;
   it prints `exists NO` but does not fail for that condition.
5. Conversely, `REG_FIRST = 14`, `reg_exists(14) == 1`, but no app constant,
   name, constructor, draw, event, or tick implementation claims ID 14.
6. The catalogue therefore contains one blank/unopenable ID 14 tile while the
   implemented Maze is absent. The total tile count looks correct because the
   blank substitutes for the missing app.

Evidence ceiling:

- Maze rules: `[S][T][V-host]`;
- Maze application implementation: `[S][R-build][P]` with internal dispatch code,
  but no actual catalogue/shell/dock launch route;
- Maze desktop/catalogue launchability: `[F]` at the current source graph;
- `apps53.py` completeness verdict: `[F]` because its own `exists NO` does not
  affect exit status.

This report diagnoses the issue only. The shared dirty zlOS checkout was not
edited. The correct future gate must fail if any expected app lacks all of:
valid ID, `reg_exists`, exact name, constructor, draw/event/tick as applicable,
icon, size, and a demonstrated human launch route. It must also reject every
registered ID that resolves only to the generic `App`/no-window fallback.

## Application feature families still missing or incomplete

Compared with the complete external inventory, current high-value gaps likely
include:

- real process-backed application isolation and per-app address spaces;
- app lifecycle/supervision, crash UI, restart and state restoration;
- package installation, update, rollback, dependency and permissions UI;
- user/session/login/lock/elevation flows;
- email, calendar, contacts, messaging and notification history;
- PDF/document/office workflows beyond narrow viewers/editors;
- media library/player, recorder, camera and audio controls;
- accessibility service, screen reader, focus traversal, high contrast and
  input remapping;
- printing/scanning;
- developer toolchain applications, source build/package manager and debugger;
- safe extensions/plugins/skills;
- multiuser file ownership/sharing and removable-device workflows;
- suspend/power/battery/thermal controls;
- localization, timezone, keyboard layouts and international text shaping.

These remain hypotheses until checked against the three all-repository deep
dives. The final all-33 matrix will decide which external systems provide the
best behavioral references and which merely contain similarly named apps.

## Destination placement rule

The external inventory must map into this boundary:

```text
hardware/firmware
      |
small kernel mechanisms: IRQ, VM, handles, scheduler, usercopy
      |
restartable providers: Display, Input, Block, USB, NIC, Audio, Clock
      |
policy services: File, Network, Package, Session, Clipboard, Logger,
                 Window, Decoder, Accessibility, Update
      |
ordinary zl processes: existing 61 app behaviors + future apps/games/tools
```

No external driver or app should be imported wholesale. The useful unit is a
behavioral contract, failure case, lifecycle state, protocol shape, UI flow, or
deterministic test. Current zlOS data, hardware evidence, visual identity and
application behaviors remain the product spine.
