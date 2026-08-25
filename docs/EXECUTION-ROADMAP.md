# zlOS execution roadmap — fast, observable, persistent, protected

**Written 2026-08-20 against `main` at `b8a00ec`.** This is the current priority
order. It supersedes rankings in `ROAD-TO-TEN.md`, `NEXT-PROMPT.md`, and the open
items in `STATE-OF-THE-PROJECT.md` where those documents predate merge round 2.
Those files remain evidence and history; they are not the execution queue.

**Implementation update 2026-08-22:** the complete locally executable batch is
recorded in
[`kernel/docs/evidence/performance-architecture-implementation-2026-08-22.md`](../kernel/docs/evidence/performance-architecture-implementation-2026-08-22.md).
Retained composition, bounded regions, deadline/input work, async block cache,
named-file migration, preemptive multi-process hostile 64-bit Ring 3, generic virtio/e1000
networking with DHCP, browser persistence and opt-in GPU-copy fallback are now
implemented and gated. BIOS and native UEFI application exercises are 32/32.
ThinkPad/dongle/stick/panel gates remain open and are not converted into VM
claims below.

## The objective

Make zlOS feel immediate, explain every hardware boot after the fact, keep user
data across reboots, run ordinary code outside the kernel, and then deepen the
real drivers and applications without losing the safe software paths.

The dependency spine is:

```text
bare-metal baseline
        |
        v
persistent boot journal -----> trustworthy hardware experiments
        |                                  |
        v                                  v
measured latency work              Intel + real NIC drivers
        |                                  |
        v                                  v
block layer + durable files ------> network + browser
        |
        v
64-bit Ring 3 + address spaces ---> user applications
        |
        v
polish, app depth, broader hardware
```

This is deliberately serial at the joins. Performance work without a hardware
baseline is guessing. Intel write-path work without a persistent log is blind.
User processes without a durable file API strand every program in memory.

## What already exists — do not rebuild it

Measured from the current tree:

- The compositor is the boot state and a fresh native host benchmark records a
  3.454 ms dragged frame at 1920x1200 (`wmbench`, best of 7 on this CPU). The
  panel's 16.67 ms refresh interval is a hard departure deadline, not the CPU
  work target. Normal input routing and lightweight visual feedback should be
  sub-millisecond, with larger repaints retaining several milliseconds of
  margin. The existing outer contract is still `late = 0` and
  `lost = 0`. This has not been proved on the ThinkPad.
- The frame loop already sleeps with `sti; hlt`; damage tracking, cached
  wallpaper, time-based animation, a hardware-cursor path and frame counters
  already exist. Do not replace them with a second compositor.
- `heap.c` is a real 64 MiB allocator with free, reuse and coalescing. The
  program arena remains separate on purpose.
- `zlfs` is a persistent named-file filesystem over NVMe. The Files app and
  editor save-through path are in `kernel.zl`. Persistence exists; it is not yet
  the universal storage contract.
- Ring 3, a TSS and an `int 0x80` syscall round trip are proved on the 32-bit
  build. There is no per-process address space, and the real UEFI/ThinkPad
  64-bit path deliberately refuses Ring 3.
- The QEMU network path is real: virtio-net -> ARP/IP -> DNS/TCP/TLS -> browser.
  The browser includes TLS 1.3, X.509 validation, HTML/CSS, PNG and bounded JS.
- The ThinkPad has Intel I219-family Ethernet (`8086:0d4f`) and AX201 Wi-Fi
  (`8086:02f0`). zlOS drives neither. A physical ZLLOG run proved the legacy
  8254x reset path freezes inside I219 initialization, so that exact device is
  now inventoried and quarantined before MMIO instead of blocking the desktop.
  The I219 also has no usable carrier without the Lenovo Ethernet dongle on
  this laptop.
- The Intel Gen9.5 display and render work is unusually deep. Modesetting and
  ring primitives have host/hardware evidence, but the full zlOS hardware path
  is not yet load-bearing and must keep the UEFI GOP fallback.
- xHCI USB mass storage now has bounded caller-buffer `READ(10)`/`WRITE(10)`,
  `REQUEST SENSE`, `SYNCHRONIZE CACHE(10)`, CSW tag/status/residue checks and
  one bounded Bulk-Only reset-recovery path. The command and range logic is
  host-tested; a physical USB stick has not yet proved the write path.
- The default 132 MiB UEFI image has a 62 MiB `zlOS EFI` partition and a
  64 MiB raw `ZLLOG` partition; `mkusb.sh --log-mb` can select 128, 256 or
  512 MiB for longer retention. The host tool initializes, validates, reads and
  exports the journal without mounting it. The shipping kernel recorder and
  its torn-write/containment behaviour are proved against a file-backed fake
  USB device. An earlier physical v6 build proved automatic ThinkPad mount and
  power-cut recovery; the expanded current telemetry build still needs that
  physical rerun.

## Phase 0 — first bare-metal evidence

The current two-partition USB image passes `verify-efi.sh`. Flash it, boot the
ThinkPad through UEFI, and record the result before choosing an optimization.

Capture:

- a photo/video from firmware selection through the desktop;
- the on-screen framebuffer mode and frame/peak/late/lost counters;
- which input devices work;
- the result of the read-only PCI, xHCI, NVMe and Intel probes;
- any last visible boot stage if it fails.

This is the baseline. A QEMU-only regression and a ThinkPad-only regression are
different bugs and must never share one vague report called "slow".

## Phase 1 — speed is the primary product priority

The current desktop-specific diagnosis and first corrective pass are in
[`kernel/docs/current-speed-and-quality-diagnosis.md`](../kernel/docs/current-speed-and-quality-diagnosis.md).
The newest physical evidence and the detailed architecture/order for this phase
are in [`kernel/docs/plans/performance-architecture-roadmap.md`](../kernel/docs/plans/performance-architecture-roadmap.md).

The first instrumentation slice now exists. A 4,096-record RAM ring starts
before storage, retains boot text through `system ready`, redacts printable key
identity, keeps exact button/wheel/drop boundaries, samples ordinary pointer
activity, and records the first late-frame history plus representative late
frames, peaks, severe frames and one healthy painted frame in sixty. Each
sampled frame separates input,
app tick, compositor, vblank wait and present time in microseconds. The ring is
checkpointed from normal context at most once per ten seconds after the exact
`ZLLOG` target is mounted, never from an interrupt handler.

The current frame extension carries the oldest pending input sequence from its
driver boundary through the next painted CPU present, plus queue depth, damage
pixels, presented bytes and missed deadlines. It does not yet observe physical
scanout/photon time. Phase 1 remains the product priority; the journal is its
measurement infrastructure, not a detour.

The first complete ThinkPad trace has now isolated the main physical bottleneck:
the 2560x1440 scanout was effectively uncacheable and copied at about 111.59
MB/s, making a full-frame present roughly 131-137 ms. The current tree retypes
the existing identity mapping to write-combining after IDT setup and before AP
startup, and fixes software-cursor damage that could inflate two tiny patches
into a screen-spanning copy. Host regressions and the UEFI QEMU gate are green;
the latter proves the live reported cache transition. The fresh physical boot
now proves about 7,089 MB/s, 63.5 times the old slope, and a 2.070 ms median for
seven full-screen presents. Present is no longer the late-frame owner. The next
measured target is compositor/drawing work: it reached 104.628 ms and was the
largest phase in 247 of 366 retained late frames.

### 1.1 Measure perceived latency end to end

Timestamp these points with the calibrated TSC:

```text
device event -> input queue -> app dispatch -> damage -> frame start
             -> frame end -> present complete
```

Add one repeatable interaction probe covering window drag, terminal scroll,
browser scroll, app open/close and animation. It must print percentiles, the
worst frame, `late`, `lost`, painted-frame count and maximum input-queue depth.
Run the same scenario in KVM and on the ThinkPad.

### 1.2 Fix only the measured owner

The current host measurements name likely suspects, not convictions:

- anti-aliased terminal text is the largest measured single CPU draw cost;
- present/blit cost grows with the damaged pixel area and becomes the hard
  floor at high resolutions;
- radial/conic wallpaper work is acceptable only because it is cached;
- four-way SMP helps full-screen rendering, but two bands were measured slower
  than serial and parked APs currently spin;
- the Intel GPU ring is available as an opt-in command, not as the compositor's
  normal backend.

The order is therefore: tighten damaged regions, remove redundant glyph work,
reduce the bytes presented, then make proven Intel blits/fills load-bearing.
Keep the byte-identical software renderer as the oracle and fallback. Every GPU
increment must render the same scene both ways and compare hashes before it is
allowed to replace the CPU path.

### 1.3 Definition of done

- the scripted interaction run has `late = 0` and `lost = 0` on the target
  ThinkPad;
- input queue-to-routing and lightweight feedback are below 1 ms at p95;
- no individual visible frame exceeds 16.67 ms in that run;
- release jitter, vblank wait and final `fb_present` time are reported
  separately, so an under-budget CPU frame cannot hide a missed scanout;
- pointer motion, typing and window drag each have an end-to-end trace;
- idle returns to `hlt` and does not burn a core;
- the software path remains selectable and gated.

## Phase 2 — boot journal on the USB stick — implemented; expanded physical rerun pending

The complete observer, on-stick format, safety rules and extraction workflow
are specified in
[`kernel/docs/architecture/boot/persistent-boot-observer.md`](../kernel/docs/architecture/boot/persistent-boot-observer.md).

The ESP stays a boot volume, not a crash dump scratchpad. `kernel/tools/images/mkusb.sh` now
builds two fixed GPT partitions:

1. `zlOS EFI`: FAT32, read-only from the kernel, containing the tiny
   `EFI/BOOT/BOOTX64.EFI` witness, `EFI/ZLOS/ZLOS.EFI`, and the bounded latest
`EFI/ZLOS/WITNESS.LOG` written only through firmware FAT services;
2. `ZLLOG`: a bounded 64, 128, 256 or 512 MiB raw diagnostic journal with its
   own type GUID.

A selected bounded log partition keeps the format independent of unused
capacity on the physical stick. The image builder hashes the ESP before and after journal
initialization and fails if it changed. When flashing a larger physical device,
`mkusb.sh` clears the final 1 MiB to remove any stale backup GPT, moves the new
backup GPT to the device's real end, reinitializes only `ZLLOG` with the real
`READ CAPACITY` size, verifies the GPT, and prints a final inspection. This is
required: carrying the 132 MiB image's disk-size field onto a multi-gigabyte
stick would make the kernel correctly refuse it.

### 2.1 Storage seam

The xHCI side of the minimum block-device contract exists:

```text
identify / capacity / block_size / read / write / flush
```

The buffer APIs are bounded by both discovered capacity and the fixed 4 KiB DMA
staging window. `WRITE(10)`, automatic `REQUEST SENSE` after target failure,
`SYNCHRONIZE CACHE(10)`, CSW validation and bounded reset recovery are present.
The recorder refuses to write unless exactly one GPT entry matches the custom
type GUID and exact `ZLLOG` label, the GPT and entry-array CRCs pass, the range
is one of the supported 64..512 MiB sizes, and a CRC-valid superblock agrees with the disk bounds,
logical block size and partition unique GUID. The superblock also records the
image-builder's `ZLOS.EFI` hash for later identification; the kernel does not
recompute that file hash at boot. Every write is range-checked again against
the selected partition.

What remains here is factoring NVMe and xHCI behind one ordinary block-device
interface. The journal deliberately uses the smaller xHCI seam now so the
bare-metal measurement path is not blocked on that refactor.

### 2.2 Journal format

The implemented v1 format keeps evidence in RAM from the first printable boot
stage and persists fixed 64-byte records:

- text records containing the same bytes sent to serial output;
- structured records containing sequence, TSC, CPU, subsystem, event,
  severity, three numeric values and a short payload. The boot ID and outcome
  live in the enclosing slot header.

The partition has two independent 4 KiB checksummed superblocks and 31, 63,
127 or 255 two-MiB boot slots. Slot headers and every record have their own CRC. The writer
writes record data, synchronizes the device, checkpoints the slot, then
publishes the alternate superblock. A forced-power-off boot remains readable
as a recovered writing slot; a clean exit publishes a complete slot.

The first drain happens when USB mass storage becomes ready. The normal idle
path checkpoints at most once per ten seconds, `diag save` requests an explicit
checkpoint, and a clean exit completes the slot. No USB write occurs in an
interrupt handler. The panic path makes a guarded best-effort flush and stage
zero provides pre-enumeration firmware breadcrumbs. Arbitrary fatal-exception
durability still needs a reserved crash page and next-boot importer.

`tools/zllog.py` validates either a whole GPT disk/image or a raw partition and
reads or exports text, JSON and CSV without mounting it. The shipping kernel
writer is exercised against a file-backed fake USB device across clean boots,
torn superblock publication, torn slot completion, rotation, corrupt identity
and out-of-partition-write checks. A fresh native OVMF UEFI64 run then proved
the real image and xHCI path across two clean boots: `ZLLOG` mounted active and
the extractor returned 407 globally contiguous CRC-valid records (`1..407`),
zero drops and zero warnings. The current post-telemetry rerun produced one
COMPLETE boot with 351/351 records (`1..351`), zero drops/warnings, 46 matched
submits/completions, exact pointer edges, nonzero input-to-present frame
correlations, six decoded xHCI timeout snapshots, privacy redaction and
cache-type decoding. Exact comparisons kept
the first MiB, pre-log gap, tail/backup GPT and both EFI executables identical.
`verify-efi.sh` remains green. The older physical v6 build proved automatic
ThinkPad mount and power-cut recovery; this expanded build's fresh physical
rerun is still the manual gate.

The journal also carries the typed automatic audit trail documented in
[`kernel/docs/automatic-system-audit.md`](../kernel/docs/automatic-system-audit.md):
process/app lifecycle, every Ring-3 syscall result, zlfs operations, owned
window operations, and asynchronous TCP/DNS/HTTP outcomes. `zllog.py audit`
correlates begin/result IDs and reports incomplete or unmatched boundaries
instead of hiding them. Host/power-cut evidence exists; the fresh physical
audit-expanded boot remains a separate gate.

Exact host workflow, from the repository root:

```sh
cd kernel
./tools/images/mkusb.sh
./tools/checks/verify-efi.sh
cd ..
./tools/zllog.py inspect kernel/zlOS-usb.img
./tools/zllog.py read kernel/zlOS-usb.img --latest
./tools/zllog.py audit kernel/zlOS-usb.img --latest
./tools/zllog.py export kernel/zlOS-usb.img --all \
  --json /tmp/zllog.json --csv /tmp/zllog.csv --text /tmp/zllog.txt
python3 tools/test_zllog.py
python3 kernel/tests/host/zllog_e2e_test.py
```

## Phase 3 — make persistence the default, not a demo — implemented locally

Do not write another filesystem. Put the existing `zlfs` behind the common
block-device seam and make it the ordinary named-file service.

Implemented order:

1. auto-mount a positively identified zlfs volume after storage discovery;
2. expose a small kernel file API: open/read/write/close/list/remove/rename;
3. move Settings from its raw private sector to `/system/settings`;
4. make the editor and Files app use only named files;
5. migrate browser history, bookmarks and downloads;
6. retire the ten RAM slots only after every caller is gone and a migration
   gate proves it.

All six are now in the source. `ls` lists named zlfs files, `files` opens the
graphical manager, `format`/`mkfs` is the only explicit initialisation route,
and Settings, editor and browser state no longer depend on numbered RAM slots.
The power-fail depth is now implemented for new v2 volumes: every replacement
is copy-on-write, data is flushed before metadata, the inactive checksummed
directory generation is written before its header, and mount selects the newest
complete generation. `fstest` injects a cut after every block write and cold
mounts exactly the old or new bytes. Version-1 media remains mountable through
its legacy path rather than being silently reformatted. Directories and
permissions still wait until processes make ownership real.

## Phase 4 — real 64-bit user mode and processes — isolation slice implemented

Do not build Rings 1 and 2. Modern x86 operating systems use Ring 0 for the
kernel and Ring 3 for user code; Rings 1 and 2 add transitions and policy with
no useful isolation boundary for zlOS.

The current 32-bit Ring-3 proof stays as a regression test. The real path is:

1. a 64-bit syscall entry with a dedicated kernel stack and a conservative
   `iretq` return first; add a `sysret` fast path only after canonical-address
   and flags checks are proved;
2. one process object owning a PID, kernel stack, user stack, page tables,
   handles and state;
3. one PML4 per process: user pages marked U/S, kernel and device mappings
   supervisor-only, executable permission explicit, guard pages around stacks;
4. page faults kill the offending process and report CR2/error bits instead of
   halting the machine;
5. `copy_from_user` / `copy_to_user` validate the complete range and every page
   before touching a pointer;
6. integrate the existing scheduler with process CR3/kernel-stack switching;
7. load one tiny zl user program, then move the zl interpreter behind the same
   syscall ABI.

Start with value-only syscalls, then add console, time, yield, exit and the file
API. Window/input syscalls or IPC come after one non-graphical user program can
run, fault, be killed and leave the desktop alive.

Definition of done is not "CPL printed 3". A deliberately malicious program
must fail to read kernel memory, write device memory, pass a crossing pointer,
disable interrupts, or crash another process.

The isolation/process slice now meets those hostile gates in native UEFI. Two
process objects own independent PML4s, RX images, NX guarded user stacks, TSS
kernel stacks, saved register frames and handle tables. Cooperative yield
switches CR3/kernel stacks and resumes both images in deterministic `AB12`
order; PIT interrupts also preempt two non-yielding infinite loops and advance
both (`PQ`); a `#GP` in one leaves its sibling runnable. Full-range copy validation,
conservative `iretq`, eight handles per process and `/system/user.bin` remain.
Bounded nonblocking PID-addressed IPC now copies four-deep/64-byte messages
across separate address spaces with full source/destination validation and
sender identity; the EFI `h1o2` exchange is the gate. It is joined by an
owner-bound text/window/input ABI: real WM open/present, retained
client rendering, bounded event polling and automatic close-on-exit/fault pass
in EFI. What remains is deliberately named: a persistent desktop spawn/reap
service, pixel/configure window depth and moving the zl interpreter behind the
same boundary.

## Phase 5 — real hardware network, then browser depth — VM/software slice implemented

The upper network stack is already link-independent through `net_link(send,
poll, mac)`. Keep that seam.

Priority:

1. add NIC selection so virtio-net is one driver rather than the network API;
2. get the shortest real packet path first: USB CDC-ECM/tethering or a supported
   USB Ethernet adapter;
3. build the Intel I219/e1000e path for `8086:0d4f` with a Linux host probe,
   explicit DMA regions, bounded reset, descriptor-ring counters and a carrier
   check; prove real frames only after the Lenovo Ethernet dongle is present;
4. add DHCP and DNS configuration instead of hard-coded QEMU addresses;
5. deepen TCP with the cases the browser can expose: reordering, retransmit,
   congestion behaviour, large responses and connection reuse;
6. tackle AX201 Wi-Fi as its own gated driver. Stages 0-1 now provide the
   manual, read-only exact-device/CSR `wifi` probe plus a bounds-checked API-77
   TLV parser and exact SHA-256 artifact allowlist. The focused suite passes
   54/54 with no device writes. Firmware boot, scan, association and packets
   remain separate gates; see
   [`../kernel/docs/ax201-wifi.md`](../kernel/docs/ax201-wifi.md).

The browser work after a real link is reliability and usefulness: heap-backed
bounded storage, larger documents, persistent history/bookmarks/downloads,
more host-render oracles, and explicit refusals for unsupported web features.
Chrome pixel parity is not a goal.

Generic NIC selection, DHCP, virtio-net and the e1000/e1000e path including
device `8086:0d4f` now exist. Both emulated drivers pass DHCP, 20/20 ping,
DNS/NXDOMAIN and zero-error gates; browser history/bookmarks/page saves are
persistent. Triple-duplicate-ACK fast retransmit and guarded plain HTTP/1.0
keep-alive reuse now pass scripted TCP/HTTP gates. A standards-based xHCI
CDC-ECM class driver also passes 18/18 in QEMU with USB storage, keyboard and
tablet attached concurrently. Physical CDC-ECM/tether carrier and physical I219
carrier through the Lenovo dongle remain open. AX201 stage 0 is implemented
locally but its physical read-only probe and every device-writing/connection
stage remain open.

## Phase 6 — Intel and the other drivers

Keep the existing hardware-development rule: safe/read-only probes in Linux,
then opt-in zlOS writes, then automatic boot use only after repeated evidence.

For Intel display/GPU:

1. preserve UEFI GOP as the always-boot fallback;
2. persist the complete Intel probe and every modeset step to `ZLLOG`;
3. prove the zlOS modeset path on the ThinkPad, including a second modeset and
   teardown, before making it automatic;
4. wire the proven GPU ring into compositor operations incrementally, with
   scene-hash agreement and automatic software fallback;
5. continue hotplug, external outputs, power features and HDA audio only where
   the hardware can provide a real oracle.

For every other driver, require three layers: host parser/state-machine tests,
QEMU integration where a model exists, and a hardware transcript where it does
not. Driver counters and last-error state belong in System Monitor and ZLLOG.

## Phase 7 — appearance and applications — kernel-app workflow gate green

The northstar visual system stays. Polish follows the frame contract so visual
effects never make the system feel worse.

Application work splits in two:

- kernel-resident apps can gain complete workflows now using zlfs and the
  existing UI toolkit;
- third-party/user apps wait for the Ring-3 process and window/input API, then
  move out of the kernel one at a time.

Prioritise apps that exercise the platform: terminal, editor/Files, System
Monitor, browser, Settings, package/app catalog. Games and visual demos remain
useful performance and input tests, but they do not define platform completion.

The final BIOS and UEFI exercises each pass all 32 workflows, including SMP,
input, GPU, modeset, explicit zlfs creation/list, editor save and post-close
file proof. Third-party migration remains blocked on the multi-process
window/input ABI named in Phase 4.

## The first execution batch — local portion complete

1. **Prior physical proof, fresh build pending:** automatic ThinkPad mount and
   power-cut extraction worked on v6; repeat with the expanded telemetry image.
2. **Implemented through CPU present:** driver-boundary input sequence/time is
   correlated with the next painted frame; physical scanout/photon observation
   and the repeatable interaction-percentile probe remain.
3. **Implemented and host-tested:** xHCI `WRITE(10)`, sense and flush support.
4. **Implemented and QEMU/fake-device-tested:** the two-partition image,
   bounded RAM recorder, durable journal, diagnostics and host extractor.
   Major xHCI, NVMe, GPU, display and network timeout paths also retain bounded
   raw before/trigger snapshots; TCP records post-recovery state.
5. **Pending hardware:** run the same performance probe on the ThinkPad and
   accept or reject the landed retained-composition/input/pacing result there.

That batch gives every later track evidence it currently lacks. It also keeps
the main priority honest: the first optimisation is chosen by a trace from the
machine the OS is meant to run on.

## Trade-offs locked for now

- **Speed over visual excess:** keep the northstar, drop or cache any effect
  that breaks the frame contract.
- **Ring 0 + Ring 3 over all four rings:** more rings are not more isolation.
- **Existing zlfs over a new filesystem:** deepen one durable path.
- **Raw log partition over logging into FAT:** fewer dependencies during boot
  and no risk to the bootloader file.
- **Wired/USB networking before AX201:** packets first, radio firmware later.
- **Software renderer retained:** GPU acceleration may fail; bootability may not.
- **One hardware target at a time:** the ThinkPad is the oracle, QEMU is the
  reproducible harness, and neither substitutes for the other.

## Revisit when the system grows

- Move from the flat zlfs namespace to directories and per-process permissions
  after process identities exist.
- Replace the fixed boot journal with a file-backed system logger after the
  storage stack is reliable early enough in boot.
- Add shared libraries and a stable user ABI only after multiple Ring-3 programs
  expose the duplication.
- Add Wi-Fi, Bluetooth, audio and broader GPU generations only after the wired,
  display and process spines are load-bearing on the target laptop.
