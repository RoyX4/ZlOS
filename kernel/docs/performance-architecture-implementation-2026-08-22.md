# Performance architecture implementation — 2026-08-22

This is the receipt for the implementation batch requested from
[`performance-architecture-roadmap.md`](performance-architecture-roadmap.md),
[`../HANDOFF.md`](../HANDOFF.md), and
[`../../docs/EXECUTION-ROADMAP.md`](../../docs/EXECUTION-ROADMAP.md).

## Status

The locally executable software/QEMU batch is implemented and green. The whole
system roadmap is **not physically complete**: this environment cannot boot the
ThinkPad, attach its Lenovo Ethernet dongle, power-cycle the Imation journal
stick, observe panel scanout/photon time, or prove Intel render/modeset recovery
on `8086:9B41`. Those are hardware acceptance gates, not work a VM can honestly
manufacture.

The direct-render software path remains the oracle and fallback throughout.

## What landed

| roadmap seam | implementation now in the tree | local proof |
|---|---|---|
| retained composition | Independent bounded client and shell/shadow surfaces, explicit validity, 48 MiB budget, direct-render allocation fallback, release on close/minimise/resize, and no allocations in composition. | `wmtest`: unchanged moves cause zero app/shell rebuilds; refusal and eviction remain pixel-correct. |
| damage and occlusion | Bounded disjoint visible regions, opaque-window subtraction, exact L/touching cases, counted full-damage fragmentation fallback, and separate old/new cursor damage. | `wmtest`, `wmtest_feel`, `fbbench`. |
| input and pacing | Queue drain immediately before commit, oldest input correlation, one newest pending visual frame, TSC deadline pacing, missed-deadline and queue telemetry. | `inputtest`, `wmtest`, `probe-frame.py`. |
| storage latency | One bounded 128-page/4 KiB block cache, dirty ordered writeback, explicit sync, queue/cache/dirty telemetry, and service only outside IRQ/input/frame context. NVMe setup is idempotent. | `blocktest`, `fstest`, BIOS/UEFI format/list/editor workflow. |
| persistent user data | Settings at `/system/settings`; editor at `/user/notes.txt`; graphical Files uses named zlfs files; browser history, bookmarks and saved pages persist; rename/remove/sync exist; the numbered RAM-file implementation is retired. New v2 volumes use copy-on-write file replacement and dual checksummed directory generations; v1 remains mountable. | `settingstest`, `browsertest`, post-editor `ls` in both QEMU boots, and `fstest` power cuts after writes 0 through 6 with cold recovery of exactly old or new bytes. |
| worker CPUs | Surface allocation happens before frames. APs load the BSP IDT, enable their own LAPIC, sleep with MONITOR/MWAIT, receive an IPI companion wake under VMMs, and render only disjoint bands behind a barrier. | `probe-smp.py`: 4/4 cores, 0 of 175,224 pixels differ from serial. |
| 64-bit isolation | Two 64-bit process objects with independent PML4s, user/TSS-kernel stacks, saved frames and handle tables; supervisor-only kernel/device mappings, RX user code, NX guarded stacks, full-range copy validation, conservative `int 0x80`/`iretq`, kill-on-fault, PIT preemption, bounded PID IPC, owner-bound text windows/input queues, and a raw `/system/user.bin` loader. | `verify-efi.sh`: cooperative `AB12`, timer-preempted non-yielding `PQ`, sibling survival after `#GP`, IPC `h1o2`, and a real WM open/present/input-`W`/close lifecycle plus hostile pointer/privilege refusals. |
| network | Generic `netdev` selection, virtio-net, Intel e1000/e1000e including `8086:0d4f`, and xHCI CDC-ECM drivers; DHCP, DNS, triple-duplicate-ACK fast retransmit, guarded HTTP/1.0 keep-alive reuse and the bounded TLS/browser stack. CDC-ECM owns dedicated bulk rings, four receive buffers, exact TRB-to-buffer completion identity, MAC-string parsing and alternate-interface activation. | virtio, e1000 and CDC-ECM QEMU gates each pass 18/18 with DHCP, 20/20 ICMP, DNS/NXDOMAIN and zero error counters; CDC runs alongside USB storage, keyboard and tablet. Host e1000 55/55, DHCP 35/35, net 152/152, TCP 126/126, HTTP 101/101; xHCI fake-controller suite zero failures. |
| browser usefulness | Persistent history and bookmarks, explicit page save slots, honest unsupported-feature copy, and bounded filesystem seams. | normal and sanitizer `browsertest`: 138/138. |
| GPU fallback | Gen9 blitter-backed compositor copy remains opt-in; the first three scenes compare software/GPU hashes and automatically disable the GPU path on disagreement or failure. | `gputest`: 123 checks; UEFI/GOP and software composition remain bootable. |
| observability | 256-sample frame/input rings, phase/damage/bytes/window/app counters, `perf` percentile dump, cache/surface/SMP telemetry and live System Monitor values. | QEMU probes and host tests; physical photon time remains outside the recorder. |

Two sequence defects found only by the complete boot exercise were also fixed:

- INIT/SIPI workers previously had no usable per-core IDT/LAPIC wake state and
  could hang the BSP at the second band barrier. The AP startup and sleep/wake
  contract is now complete for both 32- and 64-bit trampolines.
- a second `nvme_setup()` tried to create live queue IDs again and failed with
  fault 5. A ready controller now makes discovery idempotent.

The typed desktop routes were reconciled with the current app model: `ls` is a
textual zlfs listing, `files` opens the graphical manager, `format`/`mkfs` is the
only explicit destructive initialisation route, and closing the editor restores
the window that launched it before the post-save command is accepted.

## Fresh evidence from the final source

All results below were rerun from the integrated shared checkout at
`/home/roy/Documents/repos/zl-linux`. The BIOS and UEFI exercises rebuilt their
boot media first; `--no-build` is valid only when the corresponding ISO/disk
image is already known to contain the current kernel.

| gate | result |
|---|---|
| native BIOS/GRUB application exercise | **32/32** workflows |
| native 64-bit UEFI/GOP application exercise | **32/32** workflows |
| EFI stage-0/kernel/hostile Ring-3/cache/fallback gate | **green** |
| real-kernel SMP band oracle | **4/4 cores**, **0/175,224** differing pixels |
| virtio network | **18/18**, DHCP, 20/20 ping, DNS, zero driver errors |
| e1000 network | **18/18**, DHCP, 20/20 ping, DNS, zero driver errors |
| xHCI CDC-ECM network | **18/18**, DHCP, 20/20 ping, DNS, zero driver errors; USB storage, keyboard and tablet attached concurrently |
| hostile HTML/network/TCP/HTTP fuzz | **402,427** checks, zero failures |
| browser sanitizer | **138/138** |
| retained compositor host gate | zero failures, including no frame-time allocation |
| final 32-bit build | no undefined symbols |
| final 64-bit build | no undefined symbols |

The last KVM frame run before sandbox access to `/dev/kvm` disappeared measured
an ordinary 1920×1200 dock-hover p95 of 4.932 ms and drag p95 of 9.351 ms, both
under the 16.667 ms frame budget. Its drag input-to-CPU-present p95 was still
31.511 ms, so the physical latency definition of success is not earned. A later
TCG-only run was deliberately retained as an environment warning: emulation was
about 10× over budget and is not performance evidence.

## Gates that still require external hardware

| required acceptance | why it is still open |
|---|---|
| Repeat the scripted stacked-window interaction on the ThinkPad with `late=0`, `lost=0`, p95 route below 1 ms and no visible frame above 16.67 ms. | QEMU cannot measure the laptop panel, USB devices or photon output. |
| Expanded ZLLOG image: automatic mount, power-cut recovery and extraction on the Imation stick. | Requires flashing and physically interrupting power. |
| Intel I219 `8086:0d4f` carrier, DHCP/DNS/TCP through the Lenovo dongle. | The laptop needs the proprietary physical Ethernet dongle for carrier. |
| CDC-ECM MAC, carrier, DHCP/DNS/TCP through a physical USB adapter or phone tether, including unplug/replug. | QEMU proves class descriptors, xHCI rings and frames, not a particular cable/device or hotplug path. |
| Gen9 scene agreement, fence/hang recovery and a measured timing win on `8086:9B41`. | QEMU has no Comet Lake render engine oracle. |
| Second Intel modeset, teardown, PSR/hotplug/external-output behaviour and HDA audio. | These registers and panel links exist only on the target. |
| Expanded xHCI observer and USB mass-storage write/flush on the real devices. | QEMU/fake-device proof cannot certify the physical controller and stick. |

## Software work deliberately not misreported as finished

These are later roadmap depth, not hidden local failures:

- The bounded Ring-3 gate is timer-preemptive, but a persistent desktop
  spawn/reap service, pixel/configure window depth and the user-mode zl
  interpreter are not implemented;
- TCP has ordering, timeout and triple-duplicate-ACK retransmission,
  congestion/zero-window and hostile cases. Plain HTTP/1.0 reuses an idle,
  fully acknowledged same-peer connection only when the response explicitly
  grants keep-alive. CDC-ECM is QEMU-gated but not yet proven on a physical
  phone/adapter; AX201 is a separate driver not present here;
- Intel acceleration currently offloads compositor copy/blit. Blend-heavy
  render-engine composition and mandatory hardware use are not implemented;
- directories, ownership/permissions, crash-page import and a general
  file-backed logger remain post-process/storage-depth work.

## Reproduce

From `kernel/`:

```sh
cd hosttest
./build.sh
./wmtest && ./blocktest && ./fstest && ./browsertest
ASAN_OPTIONS=detect_leaks=0 ./browsertest_san
./fuzz 3000 1
cd ..
./build.sh
./build64.sh
./verify-efi.sh
python3 exercise.py --no-build
python3 exercise.py --uefi --no-build
python3 probe-smp.py --no-build
python3 probe-net.py --no-build --driver virtio
python3 probe-net.py --no-build --driver e1000
python3 probe-net.py --no-build --driver cdc
```

Omit `--no-build` after any source change that has not already gone through
`mkiso.sh` (BIOS) or `mkusb.sh` (UEFI). A kernel-only build does not refresh the
boot media used by the exercise.

Run `probe-frame.py` under KVM for a VM performance comparison. Only the
ThinkPad journal can close the physical definition of success.
