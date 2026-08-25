# Persistent boot observer: make the ThinkPad explain itself

**First usable implementation built 2026-08-20; QEMU and hardware proof are
tracked below.** This is the tool that turns a
bare-metal boot from "it felt slow" or "it returned to firmware" into evidence
Codex can inspect after the USB stick is plugged back into Linux.

The whole-runtime capture tiers, performance correlation, retention policy and
laboratory trace mode are specified in
[`always-on-telemetry.md`](../../always-on-telemetry.md). It is not an instruction trace. Recording every executed instruction would
change timing, overflow the stick and produce unusable data. It is a flight
recorder for subsystem boundaries, errors, counters and latency phases.

## User workflow

```text
flash zlOS image
    -> boot the ThinkPad
    -> use it normally or reproduce one failure
    -> shut down or force power off
    -> plug the same stick back into Linux
    -> run the zllog extractor
    -> inspect one text report plus structured JSON
```

The host commands are:

```text
tools/zllog.py read /dev/disk/by-partlabel/ZLLOG --latest
tools/zllog.py export /dev/disk/by-partlabel/ZLLOG --json boots.json --csv frames.csv --text boot.txt
```

`inspect` validates the GPT and both superblocks. `read`/`extract` accepts a
whole GPT image or the raw `ZLLOG` partition device. `export` writes structured
JSON, record CSV and the recovered text transcript. Initialization deliberately
requires the whole GPT image; it refuses a raw or ambiguously labelled target.

## Why a separate raw partition

The USB image should have two GPT partitions:

1. `zlOS EFI`, FAT32: firmware boot files plus one bounded latest-boot witness.
2. `ZLLOG`, 64 MiB by default and selectable up to 512 MiB: a raw append-only
   diagnostic journal.

The kernel still never writes the ESP. The only exception is the 20 KiB UEFI
stage-zero application, while firmware still owns its FAT driver: it replaces
`EFI/ZLOS/WITNESS.LOG` on each boot and caps it at 64 KiB. That file closes the
one gap the raw journal cannot cover: firmware rejection, `LoadImage`, and
`StartImage` all happen before zlOS owns xHCI. All high-volume evidence remains
in the raw bounded journal, with sector writes, checksums and generations.

## Firmware-boundary witness

Firmware now starts `EFI/BOOT/BOOTX64.EFI`, a deliberately tiny stage zero. It
prints immediately through UEFI `ConOut`, saves status breadcrumbs, and uses a
normal full device path to chainload the real kernel at
`EFI/ZLOS/ZLOS.EFI`. There is no prompt or command.

The latest `WITNESS.LOG` distinguishes three previously identical black-screen
failures:

- no file: firmware never entered stage zero;
- an EFI error line: stage zero entered and names the failed UEFI operation;
- `LOAD_IMAGE ... 0` followed by `START_IMAGE CALL`: firmware accepted the
  kernel and the failure is inside zlOS before `ZLLOG` became writable.

Both EFI files use image base zero, 4 KiB file alignment, subsystem version
0.0, no DLL characteristic, and a non-empty relocation directory. The FAT BPB
now records its real partition start, LBA 2048. `ZLLOG` stores the SHA-256 of
`ZLOS.EFI`, not the witness, so a journal names the code that actually ran.

The writer must refuse unless all of these agree:

- the GPT partition type GUID is the dedicated zlOS log GUID;
- the partition label is `ZLLOG`;
- the first sector contains the zlOS journal magic and supported version;
- the recorded USB identity/capacity matches the device selected at boot;
- the target LBA range is wholly inside that partition.

It never writes LBA 0, the GPT, the ESP, "the first USB disk" or an unlabelled
device. Refusal is safer than guessing the boot stick.

## Storage transport: implemented

`xhci.c` now provides caller-buffer `READ(10)` and `WRITE(10)`, `REQUEST
SENSE`, `SYNCHRONIZE CACHE(10)`, complete CSW result state and bounded one-shot
Bulk-Only recovery. Every transfer is capacity checked and limited to its 4 KiB
staging window. It never retries a failed write blindly.

The minimum storage seam is:

```text
identify / capacity / block_size / read / write / flush
```

The observer currently binds that seam directly to xHCI. Moving it behind a
generic block-device interface, then adding NVMe, remains follow-up work.

## Capture before USB is ready

Logging starts before PCI, xHCI or storage exists. A fixed RAM ring receives
every early record and a text tee receives the existing boot messages. When the
correct `ZLLOG` partition becomes writable, the observer writes a boot header
then drains the early rings in original sequence order.

If USB never initializes, nothing can persist to that USB. The last stage code
must still be displayed on screen. Persistence cannot depend on hardware that
failed before it existed.

## On-stick format

The implemented v1 type GUID is
`a2bc51d4-225d-4ad4-8db5-b0095953aa19`. It uses two 4 KiB superblocks, A and B.
Each contains:

```text
magic / format_version / generation / active_boot_slot
next_sequence / completed_boots / device_identity / CRC / commit marker
```

Update the older copy only after boot-slot data is durable. A torn write leaves
the other generation readable.

The rest of the selected partition is divided into fixed 2 MiB boot slots:
31, 63, 127 or 255 slots for 64, 128, 256 or 512 MiB. A slot contains a
header followed by 64-byte records. One record contains:

```text
sequence / TSC / CPU / severity / subsystem / event
three numeric values / short text-or-payload bytes / record CRC
```

Fixed records mean the Linux extractor can skip a damaged record and continue.
When slots fill, overwrite the oldest completed boot, never the current one.
A forced power-off may lose the newest records; it must not erase older boots.

## What it records

### Boot identity and hardware

- git/build identifier and image checksum;
- boot path: BIOS, Multiboot64 or UEFI;
- CPU model, core count, TSC frequency and active timer source;
- usable physical-memory map and every reserved arena;
- framebuffer address, resolution, pitch, pixel format and effective cache type;
- PCI devices, BAR widths/addresses and the driver selected for each device;
- xHCI ports/endpoints, USB identity, NVMe identity and capacity;
- Intel display pipe/mode/link state and every modeset step result.

### Performance phases

For every sampled interaction and every slow frame:

```text
device timestamp
  -> input queue enqueue
  -> route begin/end
  -> ZL app tick/draw begin/end
  -> compositor begin/end
  -> vblank wait begin/end
  -> fb_present begin/end
  -> flip/presentation observation
```

Also record frame-release error relative to the predicted deadline, damaged
pixels/rectangles, input queue depth, painted/late/lost counts, backend
(GOP/CPU/Intel), and whether the hardware cursor was active.

Do not log every ordinary frame forever. The current implementation retains the
first eight late frames, representative later late frames, new peaks and severe
frames, one in 60 healthy painted frames, button/wheel edges, queue drops, and
one in 16 ordinary pointer/batch events. Key-event timing and pressure are
retained but printable character identity is redacted. Idle frames emit nothing.
The frame path only appends to RAM; normal-context idle checkpoints are limited
to once per ten seconds. The observer must not become the performance bug.

### Driver and application evidence

- timeout start/end and the register values that decided success or failure;
- bounded before/trigger raw snapshots for xHCI, NVMe, GPU, Intel vblank and
  network waits, plus TCP post-recovery state;
- interrupt and completion counters, bounded rather than per-packet spam;
- USB/NVMe SCSI or command status plus request-sense data;
- network link, DHCP, ARP, DNS, TCP and TLS state transitions;
- browser navigation stage, response sizes and explicit parser/refusal errors;
- heap/arena high-water marks and allocation failures;
- process faults: PID, RIP, CR2, page-fault bits and syscall number;
- panic reason and the last completed named milestone.

This gives future driver, network, browser, filesystem and Ring-3 work the same
persistent evidence instead of inventing a new debug channel for each feature.

## What “log the whole kernel” should mean

It should mean that every important state transition is reconstructable, not
that every machine instruction, pixel store, MMIO poll and packet byte is
written to USB. A literal instruction log is the equivalent of filming every
movement of every atom in a workshop: it is larger than the work, changes the
timing being investigated, and can make the storage logger the slowest driver
in the machine.

The useful full-kernel design is tiered:

1. **Always exact:** boot milestones, driver state changes, command submission
   and completion, timeouts, faults, panics, process/app lifecycle, filesystem
   mutations, syscalls which cross a protection boundary, and every dropped or
   refused operation.
2. **Counters by default:** IRQs, MMIO polls, packets, allocations, scheduler
   decisions, rendered pixels and queue depths. Emit a periodic aggregate and
   every exceptional change instead of one record per repetition.
3. **Sampled hot paths:** ordinary pointer movement and healthy frames. Keep
   exact key/button edges, slow frames and queue drops, as the current recorder
   already does.
4. **Triggered burst capture:** when a timeout, fault or latency threshold
   fires, retain a bounded before/after window with raw descriptors, registers
   and correlation IDs. The xHCI EP0 v4-v6 trace is the first concrete example.
5. **Instruction tracing only as a laboratory mode:** QEMU execution tracing or
   future Intel Processor Trace can answer a narrow control-flow question into
   a dedicated RAM buffer. It must never be the normal USB journal mode.

The shared event catalogue, atomic counter bank, spans, triggered frame history,
interrupt-safe emergency lane, normal-context flusher, explicit drop accounting
and full-slot continuation are now implemented. The remaining SMP generalisation
is a separate ring per active CPU; zlOS currently runs kernel producers on CPU 0.
Larger snapshots should be split into numbered chunks or placed in a separate
bounded blob area rather than changing the fixed 64-byte record contract.

That architecture can cover xHCI, NVMe, Intel display, input, networking, TLS,
browser navigation, filesystems, user-mode processes and app lifecycle through
the same extractor. It also preserves privacy and usefulness: log packet and
request metadata by default, not arbitrary payloads, passwords, document text
or every framebuffer byte.

## Physical ThinkPad proof — 2026-08-21

The v6 image completed the first real automatic journal mount. The Imation
stick's selected superblock is B generation 2 with slot 0 active. A hard power
off left that slot in the expected recoverable `WRITING` state; the extractor
recovered 1,036 CRC-valid records with contiguous sequence 1..1036, zero
recorder drops, zero input drops and no error/fatal record. `STORAGE_READY` is
record 214 and names the exact 64 MiB partition geometry.

The first refused scan is also present at records 14-15. That is intentional
history: it proves automatic recovery occurred. What was wrong was presenting
that early result forever in the boot line and firmware variable. The current
tree refreshes the boot-visible state after the final USB scan and publishes
`storage-ready` to the EFI variable only after the first journal checkpoint is
durable.

## Flush policy

USB writes are slow and may stall. Never write from a hard interrupt handler.
Append in RAM, then flush:

- after USB storage and `ZLLOG` become ready;
- after named boot milestones;
- after an explicit `diag save` command;
- periodically at a low rate while a diagnostic probe is active;
- on clean shutdown;
- best-effort on panic only when the storage path is idle and already proven.

Data records are written first, followed by a cache flush, then the newer
superblock commit. `SYNCHRONIZE CACHE` failure is itself retained in RAM and
shown on screen; the journal never claims durability it did not receive.

## Host extractor output

The Linux-side tool reads the partition read-only and produces:

1. a concise human report: boot outcome, hardware changes, slowest phases and
   first failing milestone;
2. JSON containing every valid structured record;
3. CSV frame and input latency tables;
4. the mirrored serial/console text;
5. a warning for torn, corrupt or unsupported records without discarding later
   valid data.

Codex can then compare two boots by build ID, show regressions and identify the
first divergent event. No private Linux data belongs in this format; it records
only zlOS boot/runtime state.

## Implementation and proof status

- Done: GUID, byte-exact v1 format, record/super/slot CRCs, host encoder,
  inspector, extractor and JSON/CSV/text exporter.
- Done: default 132 MiB GPT image containing a 62 MiB ESP and a 64 MiB
  `ZLLOG`, with 128/256/512 MiB journal variants; initialization proves the ESP
  hash is unchanged. Stage zero is the sole bounded firmware-time writer to
  the ESP.
- Done: bounded xHCI read/write/flush/sense transport. `msctest` has 43 passing
  checks and the existing xHCI starvation harness remains green.
- Done: early RAM text/event capture, global sequence rebasing across boots,
  durable WRITING checkpoints, COMPLETE commits and safe variable-slot rotation.
- Done: logical input queue/drop records plus attributed compositor phases.
- Done: ASan/UBSan fake-device tests inject torn slot/super writes, require
  global contiguous sequences, and hash every byte outside `ZLLOG`.
- Done in QEMU: two native UEFI64 boots over xHCI storage produced 407
  contiguous CRC-valid records. GPT, ESP, `BOOTX64.EFI`, gaps and backup GPT
  stayed byte-identical; only the 64--128 MiB journal range changed.
- Done in the current writable QEMU rerun: 351/351 records in one COMPLETE
  boot, zero drops/warnings, exact command pairing, pointer edges, correlated
  frame latency, privacy redaction, decoded timeout snapshots and framebuffer
  cache state. Exact containment and both EFI executable comparisons remained
  clean.
- Done in QEMU after the failed ThinkPad attempt: stage zero persisted every
  successful handoff through `LoadImage`, called `StartImage`, and the separate
  `ZLOS.EFI` reached the desktop and mounted `ZLLOG`. The gate first caught an
  invalid in-memory chainload (`EFI_UNSUPPORTED`) before the USB was flashed;
  the shipping full-device-path chainload is the rerun that passed.
- Proved on the physical ThinkPad: Lenovo entered stage zero, opened the ESP,
  resolved the full child device path, loaded `ZLOS.EFI`, and called
  `StartImage`; every firmware status through that boundary was success. The
  child then reset before the old build could mount `ZLLOG`, which ruled out
  the boot menu, Secure Boot, GPT/FAT and PE loading as causes.
- Corrected for the next physical run: `ZLOS.EFI` now persists its own bounded
  checkpoints through the memory-map and ExitBootServices boundary, the EFI
  path allocates real compositor pages below 4 GiB instead of writing an
  unreserved 128--168 MiB guess, the 64-bit data descriptor no longer sets the
  reserved L bit, and boot ownership is GDT/IDT/timer -> xHCI -> ZLLOG before
  settings, Intel discovery or desktop painting. OVMF reaches the prompt with
  all of those boundaries present.
- Prepared again on the physical Imation stick: byte-exact corrected EFI files,
  clean device-sized GPT/FAT, BPB hidden-sector value 2048, empty witness and
  journal, current partition GUID in the explicit `zlOS USB` entry, and
  `BootNext` set to it.
- Proved on the next physical run: the corrected child crossed EFI and reached
  the 2560x1440 desktop with its dynamically allocated backbuffer. The journal
  then reported error 1, isolating the remaining failure to MSC initialization
  before capacity/GPT/superblock checks or any write.
- Not yet proved: whether the slot-context/automatic-retry correction reaches
  `ZLLOG` on the ThinkPad,
  physical xHCI timing, and a real USB stick's forced-power-loss/cache behaviour.
  Those are hardware tests, not claims inferred from QEMU.

### First complete physical child boot

The next ThinkPad run crossed the entire EFI boundary and reached the live
desktop at the panel's native 2560x1440 mode. `WITNESS.LOG` proves the child was
loaded at `0x477CF000`, GOP exposed VRAM at `0xC0000000`, the new 14,745,600-byte
backbuffer allocation succeeded at `0x469BF000`, the memory map was captured,
and zlOS reached `BEFORE_EXIT_BOOT_SERVICES`. The invalid data-segment flag and
unreserved first framebuffer clear are therefore closed on the physical path.

The journal did not mount. The live `diag` report was RAM-only with 1,677
buffered records, zero drops and error 1; the on-stick superblocks remained at
their pristine generations. Error 1 is specifically xHCI mass-storage
initialization, before capacity, GPT, superblock or any write. The storage
format and identity checks were not reached.

The next image fixes the strict-controller defect found at that boundary:
Configure Endpoint now copies all four dwords of Intel's live output slot
context before changing Context Entries. The old path copied DW0/DW1 but
zeroed DW2/DW3, including device address and slot state; QEMU accepted that
invalid partial context while real xHCI may reject it. The exact physical
Imation `0718:067d` / `08:06:50` descriptor and both 32-byte/64-byte context
layouts are now host regressions. Failed mounts retry automatically with
bounded backoff and deduplicated refusal records, and the graphical boot line
shows the deepest MSC stage, port, slot, completion code and USB identity—no
`diag` command is required for the next result.

The physical Imation stick now contains that image, its ESP and kernel hashes
match the build byte-for-byte, its backup GPT is at the physical end, and both
fresh ZLLOG superblocks validate against the device's real 7,570,752-block
capacity. The explicit firmware entry names the new partition GUID and
`BootNext` is set. Future releases can perform that NVRAM refresh safely with
`./tools/images/mkusb.sh --boot-next /dev/device`; this matters because every fresh GPT has a
new unique partition GUID and an older entry otherwise falls through to Linux.

### Second and third physical runs: MSC still fails, but the fallback worked

The slot-context/retry image again reached the native 2560x1440 desktop, but
the returned stick remained byte-pristine at ZLLOG generation 1/0 with no boot
slot. `WITNESS.LOG` again reaches `BEFORE_EXIT_BOOT_SERVICES` with successful
stage-zero handoff, GOP, dynamic backbuffer and memory-map capture. Therefore
the context fix was necessary hardening but not the complete physical MSC root
cause, and no physical performance conclusion can yet be extracted.

The following run proved the independent firmware-variable fallback on the
actual ThinkPad. `ZlBootDiag` was checksum-valid and reported
`storage-refused / msc-init` at tick 189 while the xHCI controller was running
(`USBCMD=1`). It also exposed a diagnostic-selection bug: the retained summary
was port 10, slot 4, USB `8087:0026` (the internal Intel AX201 Bluetooth radio),
not the Imation stick. Linux confirms the complete root-port topology: Imation
`0718:067d` is port 4, camera is port 8, fingerprint reader is port 9 and
Bluetooth is port 10. The old "deepest stage across every device" field let a
later equal-depth non-storage device replace the disk's boundary. Its
`msc_result=not-ready` meant that no Bulk-Only command had run; completion
`0xffffffff` was the sentinel for not attempted, not an xHCI error code.

The next image replaces that lossy v1 summary with checksum-protected
`ZLDIAG2`. It retains a bounded entry for every attempted root port: USB
identity, MSC stage/slot/completion, independent enumeration
stage/completion, speed, connected/enabled state, MSC-candidate flag and the
port's raw `PORTSC`. Enumeration itself now distinguishes connected, reset,
slot enable, address, EP0 correction and device-descriptor boundaries. The
compact screen summary keeps the first port on equal-depth ties, but the saved
port table is authoritative. A regression models Imation on port 4 and Intel
Bluetooth on port 10 and proves neither can overwrite the other. The decoder
still reads the real v1 record and now renders `0xff/0xffffffff` as
`not-attempted` and MSC result 2 as `not-ready`.

The v2 image then produced the first unambiguous physical transport result.
Imation `0718:067d` was connected and enabled at port 4, high-speed ID 3,
slot 1, and failed the attempted configuration-header control transfer with
xHCI completion code 4 (`USB Transaction Error`) at tick 189. Camera port 8,
fingerprint port 9 and Bluetooth port 10 failed at the exact same boundary and
completion code. ZLLOG stayed pristine. This rules out the Imation's Bulk-Only
interface, SCSI commands, capacity, GPT and journal code: none had run. It
isolates a shared default-control-endpoint defect.

That sequence found a real publication defect but did not, by itself, identify
the complete physical root cause. The xHCI producer rules require software to
finish a TRB before handing it to the consumer; Linux's control-transfer
builder keeps the first TRB on the opposite cycle until the complete request
is built. zlOS now does the same across ordinary and Link-wrapped transfers.
The first physical image with that change still stopped at configuration
header: Imation timed out, while camera, fingerprint and Bluetooth returned
code 4 on Setup. ZLLOG remained pristine. The earlier statement that the
publication race alone explained the physical failure is therefore withdrawn.

The host regression directly inspects the real EP0 ring and proves both normal
and wraparound control transfers remain invisible until the final cycle-bit
commit. The 32-bit, 64-bit and EFI builds, complete xHCI/input suite, MSC suite,
native OVMF EFI gate, forced runtime-variable refusal, journal tests and
ASan/UBSan writer recovery remain green.

The new physical record exposed three further differences from a production
USB stack. First, cached spin counts were being used as timeouts, so a nominal
five-million-poll control wait could expire in milliseconds. Second, the
ownership handoff used only a compiler barrier instead of Linux's DMA write
barrier. Third, each descriptor was attempted once even though Linux retries
it up to three times specifically for flaky real devices. Control and bulk
waits now use PIT-backed real-time deadlines; publishing Setup and consuming
Event TRBs use `sfence` and `lfence`; descriptor reads get three bounded
attempts; transaction errors Reset Endpoint; and a real timeout issues Stop
Endpoint before replacing the dequeue ring. The host regression injects a
failed first Setup and proves reset, dequeue replacement and success on attempt
two. The remaining proof is physical: pass config-header, identify the MSC
interface and begin capacity/GPT checks or mount ZLLOG.

Before that rerun, the failure path was hardened further. EP0 and bulk waits
now match the exact TRB addresses belonging to the current request, not merely
the same slot/endpoint, so a late completion from an earlier timed-out transfer
cannot complete the next one. Doorbell writes are read back from the PCIe BAR
before polling, forcing posted writes to reach the controller. The automated
EFI gate now boots with xHCI storage, keyboard and mouse together instead of
testing the disk in isolation; both the ordinary journal boot and forced-GPT
runtime-refusal boot pass this multi-device scan.

`ZLDIAG3` also carries the next layer of evidence without another format
iteration: last bulk xHCI completion, SCSI opcode, CSW status/residue, BOT
recovery outcome and REQUEST SENSE key/ASC/ASCQ, plus the EP0 Setup/Data/Status
TRB which generated each port's retained event. Its unused entry bits now also
retain EP0 attempt count and recovery outcome without changing v3's size. The
decoder remains backward compatible with the physical v1, v2 and first v3
records. Its v3 test injects a complete capacity/CSW/sense failure and verifies
every field.

The replacement image containing the real-time waits, DMA barriers and bounded
descriptor recovery was flashed to the exact Imation stick on 2026-08-20.
Independent read-back verified a clean device-sized GPT, pristine A/B
superblocks, exact EFI build hash and a one-shot firmware entry bound to the
regenerated ESP GUID. The stick is synced and unmounted. Physical enumeration
and journal mounting remain unverified until the next restart.

That restart has now been read back. ZLLOG is still pristine at superblock
generations 1/0 with no boot slot, but checksum-valid `ZLDIAG3` proves the new
paths actually ran. Imation, camera, fingerprint and Bluetooth each made all
three configuration-header attempts; each final Transfer Event points at the
Setup stage with completion code 4, and each port reports completed Reset
Endpoint plus Set TR Dequeue recovery. No SCSI opcode, CSW or sense transaction
was reached. This rules out a one-shot descriptor failure, too-short polling
as the complete explanation, and failure to recover a halted EP0. Four
unrelated devices failing the request immediately after a successful Device
descriptor instead makes the shared transfer-ring/dequeue transition the main
suspect. Atomic first-TRB publication remains a real fix, but is also now
physically proven insufficient on its own.

`ZLDIAG4` records that transition rather than asking the next reboot to infer
it. Every port keeps the raw Setup, Data and Status TRBs, the full 64-bit
Transfer Event, the pre-recovery output EP0 state/dequeue/DCS, software
enqueue/cycle, context size, descriptor attempt and first destination dword.
The snapshot happens before Reset Endpoint reinitializes the ring. The first
failing port also runs one bounded four-request matrix and retains all four
completion codes: repeated Device descriptor to the original buffer, Device
descriptor length 9 to the configuration buffer, Configuration descriptor to
the original buffer, and Configuration descriptor after 100 ms to the
configuration buffer. That separates ring position, buffer address, request
type and readiness in one physical run. The reader still accepts v1-v3.

The host controller regression now reproduces the physical order rather than
starting the failing request on an empty ring: Device occupies entries 0-2,
Configuration Setup fails at entry 3, EP0 is Halted at that dequeue pointer,
and recovery replaces the live ring. It proves the raw failed TD/event/context
and the port-specific copy survive that destructive reset. Parser coverage is
5/5, `xhcitest` and `msctest` report zero failures, the xHCI test is clean under
ASan/UBSan, the 32-bit/64-bit/EFI builds and exact BIOS transcript pass, the
ZLLOG suites remain 9/9 and 3/3, and the native multi-device OVMF EFI gate plus
forced runtime-variable refusal is green. This is prepared diagnostic proof;
the v4 image is not yet physical execution evidence.

The verified v4 image has now been flashed to the exact stable Imation by-id
device and armed as `BootNext=0002`. Post-write GPT and FAT checks are clean,
the device remains unmounted, both journal superblocks are valid and pristine,
and the built/USB `ZLOS.EFI` SHA-256 is
`053ac557706116a4419657977331848243b7f006bdb6162ebc135ff8cc3ce729`.
The complete 62 MiB ESP hash also matches between image and device. The
ZLLOG unique GUID is `a50b6779-e312-4f0e-8399-a3504b2b51a5`; the firmware
entry names the freshly generated ESP PARTUUID
`012ee330-58d4-448a-8f0a-c1014986dc56`. This is independently verified media,
not a claim that the ThinkPad has executed v4 yet.

The ThinkPad has now executed that v4 image. The retained EFI record validates,
while ZLLOG remains pristine at generations 1/0: storage never reached BOT.
All four connected devices completed Device descriptors, then failed the
Configuration-header Setup with cc4/residual 8 and a Halted EP0 whose hardware
dequeue exactly matches the reported Setup TRB. Imation's four post-failure
probe codes are `4/4/4/4`. Those probes prove the existing hard recovery did
not revive the endpoint; because they run after the first Config fault, they
cannot identify the clean request that poisoned it. Full-speed ports already
completed Device8 and Device18, which rules out a generic second-transfer or
ring-index-three explanation. Configuration is the shared transition.

Version 5 records the clean boundary and changes the shipping behavior where
it already disagreed with the xHCI/Linux sequence. There is now a 10 ms
SET_ADDRESS recovery interval, Intel's 1 ms quiet period after HCRST, checked
CNR/HCHalted reset sequencing, Linux-parity ISP on IN Data, and Status-only
control completion. A single bounded header probe captures the first successful
Device request and its immediate identical twin before any Configuration
request or recovery. Each side retains all three TRBs, the complete Transfer
Event, software producer state, output EP0 state/dequeue, output Slot address
and state, completion and recovery. The same v5 header records USBLEGSUP,
USBLEGCTLSTS, xECP and pre/post-reset USBSTS so firmware ownership is evidence
rather than an assumption. Per-port entries remain 104 bytes; only the one
global header grows, keeping the worst-case variable bounded to 3,540 bytes.

The v5 decoder is backward compatible with v1-v4 and has 7/7 parser tests. The
host xHCI test covers a first Device Status success immediately followed by an
identical Device Setup cc4 plus recovery, and independently proves a cc13 Data
event is ignored until Status succeeds. `xhcitest`, ASan/UBSan, `msctest`, all
kernel builds, the exact BIOS transcript and both native/forced-refusal EFI
gates are green. The observer design is verified; only a new physical run can
say whether the address/reset/Status fixes mount ZLLOG on Intel.

That v5 image has now been independently verified on the stable Imation stick
and armed as one-shot `BootNext=0002`. The physical GPT is clean and sized to
the full device, neither USB partition is mounted, and ZLLOG is pristine with
valid A/B superblocks at generations 1/0. The complete ESP is byte-identical
to the built image (SHA-256
`a5c0bcb6df9863b67bd57561617a46c9fc83b57643574b43290662820c01bec2`),
and built/USB `ZLOS.EFI` match at
`95bc371ae529dc13988caa51a29c0f9b99e22a38c8243139c26778f1d0e62531`.
The current ZLLOG GUID is `6f8be994-dd1b-4912-9216-96204b904a99` and the
firmware entry is bound to current ESP PARTUUID
`a9ef9679-e22c-4f1b-8e4e-0dd41b7a4d0a`. No Intel execution is claimed until
the next restart and Linux-side read-back.

The ThinkPad has now executed v5. The first Device18 request and its immediate
identical twin both completed through Status on the same live EP0 ring, moving
the software producer from index 3 to 6 while the Slot stayed Addressed at USB
address 1. All four devices still failed Configuration-header Setup with
cc4/residual 8. The retained ownership/reset record is cooperative and clean.
This rules out a generic second-request failure, fixed ring index, lost USB
address, malformed Configuration TD and firmware ownership as the complete
cause. It also exposed the one decisive recorder gap: the per-port trace is
attempt three after two ring replacements, not the first clean Configuration
request that followed the successful Device pair.

Version 6 closes that gap and changes recovery scope. A 32-word global record
retains the first Config TD/event, pre-recovery endpoint and Slot context,
separate Reset Endpoint and Set TR Dequeue completion codes, post-recovery EP0
context and both clean whole-enumeration outcomes. The 440-byte header plus all
31 port entries is at most 3,668 bytes, enforced below the 4 KiB firmware
variable budget. The parser remains compatible with v1-v5 and its suite is
8/8.

The class scan now disables and forgets a failed cached slot before retrying,
then performs port reset, new slot allocation, Address Device and Device
descriptor again. If that ordinary clean retry still fails, one bounded
compatibility attempt first uses Address Device BSR=1 and requests Device64 at
USB address zero, releases that temporary slot, resets and performs normal
enumeration. Storage discovery skips ports already owned by configured
keyboard or pointer drivers. Host tests prove the original Config event cannot
be overwritten by three same-slot attempts, command recovery, later ports or
the clean retries; they also prove cache invalidation and the BSR command bit.
The doorbell flush now reads USBSTS instead of the write-only doorbell, and the
control timeout is one second.

All software gates for v6 are green: parser 8/8, journal 9/9, torn-write and
automatic-retry E2E 3/3, xHCI/MSC/input/compositor zero failures, xHCI
ASan/UBSan clean, 32/64/EFI builds, exact BIOS transcript, native multi-device
OVMF boot and forced-runtime-refusal EFI boot. The exact Imation target was
then revalidated by bus, vendor/model, serial, 3,876,225,024-byte capacity,
512-byte sectors, removable state, mounts/users/holders and NVMe system-root
separation before flashing.

Independent physical read-back reports a clean device-sized GPT, unmounted
partitions, pristine ZLLOG generations 1/0 and real `disk_blocks=7,570,752`.
Built and USB `ZLOS.EFI` SHA-256 is
`ea3bf7ea2b57788f26556da798dcd73aaaafa4d4fde1e3e7e4e58d6331341ada`;
stage zero is
`419278d402a8735dd9303fed3a3da5641d99fa7a7f12ab2521e661b7c21b4c35`;
the complete ESP is
`a7abe4bd6ef8dd6a0c33b6508f576c7a7ba975932bc1767a59de03b5e1bda245`
both in the image and on USB. ZLLOG GUID is
`fdd4994b-eb60-4755-ae47-2e745a698e11`; `Boot0002` names current ESP PARTUUID
`bac50c60-fb6a-403c-86d7-01dd31b52ef3`, and `BootNext=0002` is set. This is
prepared-media proof; the next ordinary restart is the physical v6 gate.

The next diagnostic no longer depends on the broken device path to report its
own failure. `efi.c` arms a single bounded `ZlBootDiag` vendor variable before
ExitBootServices and `zllog.c` replaces it on a changed storage refusal through
UEFI Runtime Services. The FNV checksum and current v6 bounded port table retain the
reason/result, per-port enumeration and MSC boundaries, PORTSC, USBSTS,
USBCMD, USB VID:PID and PIT ticks across power-off. Writes are bounded to one
per changed failure boundary. `tools/zlbootdiag.py` decodes v1 through v6;
if it remains `armed-before-exit`, the post-exit SetVariable call itself was not
supported. OVMF proves the pre-exit arm call returns EFI_SUCCESS. The automated
runtime gate then deliberately renames the ZLLOG GPT entry, boots the real EFI
kernel over xHCI, reaches the prompt, and recovers a checksum-valid
`storage-refused / gpt / MSC ready` record from OVMF's nonvolatile variable
store after ExitBootServices.

The extractor now expands a retained STORAGE_REFUSED record into readable MSC
stage, port, slot and xHCI completion code fields in JSON, CSV and text. A
paired record snapshots the live PORTSC, USBSTS and USBCMD registers. The boot
line and `diag` show the same register state. This keeps the hardware result
useful even if the desktop disappears before anyone can copy the on-screen
diagnostic, or if the timed retry succeeds and clears the visible error.

Frame records now keep the framebuffer damage rectangle count and exact pixel
area in the previously unused metadata bits beside the five phase timings.
The low three flag bits and low 32-bit compositor time remain unchanged, so
older analysis which masked those fields stays valid. The next physical trace
can therefore tell whether a late frame drew most of the panel or whether a
small damaged region is intrinsically slow.

Run the proof suite with:

```text
python3 tools/test_zllog.py
python3 kernel/tests/host/zllog_e2e_test.py
cd kernel && ./tools/images/mkusb.sh && ./tools/checks/verify-efi.sh
../tools/zllog.py read zlOS-usb.img --latest
```

The software definition of done is now met: an abrupt metadata write leaves an
older committed generation readable, a torn active slot yields its CRC-valid
monotonic record prefix, and containment tests prove no write escapes `ZLLOG`.
The older physical v6 build has already proved automatic mount and power-cut
recovery on the prepared stick and ThinkPad. The current expanded producer set
and retention policy have a fresh writable OVMF proof: one COMPLETE boot,
351/351 valid records (`1..351`), zero drops/warnings, 46 submit/completion
pairs, exact pointer edges, input-to-present correlations, six xHCI timeout
snapshots, privacy redaction and cache-type decoding. Exact comparisons kept
the first MiB, 63..64 MiB gap, 128..132 MiB tail and both EFI executables
byte-identical. A fresh ThinkPad
power-cut/extraction run of this expanded build is the remaining physical gate.

Panic paths now make a guarded best-effort final flush and stage zero supplies
the pre-enumeration firmware breadcrumbs. An arbitrary fatal exception still
needs a reserved crash page plus a next-boot importer before USB durability can
be promised when the kernel or storage path itself is already broken.
