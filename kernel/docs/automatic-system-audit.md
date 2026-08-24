# Automatic system audit trail

zlOS now records this chain automatically:

```text
process/app start
    -> syscall
        -> file, network or window operation
            -> result or explicit error
                -> process/app exit or fault
```

This is metadata evidence, not screen recording or keylogging. It is part of
the existing always-on `ZLLOG` flight recorder and uses the same bounded,
power-cut-safe USB journal. There is no diagnostic mode to remember to turn on.

## What is recorded

Three stable record types were added to the 64-byte ZLLOG format:

- `lifecycle`: object kind, object ID, start/ready/exit/fault/refusal,
  parent/owner ID and a numeric detail;
- `operation-begin`: correlation ID, actor kind/ID, stable operation code and
  object ID;
- `operation-result`: the same correlation ID and operation code, signed
  result, explicit error number and numeric detail such as byte count.

Current producers are:

| Boundary | Automatic evidence |
|---|---|
| 32-bit Ring 3 | process start/exit and every syscall enter/result |
| 64-bit Ring 3 | loaded/ready/exit/fault and every syscall enter/result, including all refusal returns |
| zlfs | format, mount, find, create, read, write, delete, rename and sync result/error |
| compositor apps | app and window start/refusal/exit, window open/close result |
| Ring-3 windows | owned open, present, poll and close result/error plus lifecycle |
| TCP | socket start/ready/exit/fault, async connect result, send/receive/close/abort result |
| DNS | query start/ready/fault/exit and async resolve result, including cache, refusal and timeout |
| HTTP/TLS | request start/ready/fault/exit and final status/error when the asynchronous request terminates |

Existing ZLLOG records still provide boot milestones, driver commands,
interrupt/counter totals, redacted input timing, frame attribution, storage
failures, faults and panic boundaries. The typed audit records add causality;
they do not replace those lower-level records.

## Privacy, bounds and failure honesty

- File contents, window text, HTTP bodies, packet payloads and printable key
  identity are never stored.
- Filenames, DNS names and peer IP addresses are represented by stable FNV-1a
  tokens. These tokens support correlation but are not encryption; somebody
  who already knows a candidate name can test it.
- Reads/writes carry indices, counts and outcomes, not buffers.
- The existing 4096-record RAM admission limits, emergency lane, drop counter,
  two-MiB rotating slots and 64..512 MiB partition limit still apply.
- A result without a begin means the begin was dropped. A begin without a
  result means the operation was interrupted by a crash/power cut or its result
  was dropped. The decoder says this explicitly; it never invents completion.
- Normal idle checkpoints happen about every ten seconds. A hard power cut can
  lose only the unflushed RAM tail. Clean shutdown and explicit diagnostic save
  attempt a final checkpoint.
- The kernel writes only to the single GPT partition whose type, `ZLLOG` label,
  CRCs, unique GUID, bounds and device capacity all validate. It never chooses
  an arbitrary USB disk or writes diagnostics to the EFI partition.

## Read it after a real boot

From the repository root, with the zlOS USB inserted in Linux:

```sh
sudo tools/zllog.py audit /dev/disk/by-partlabel/ZLLOG --latest
sudo tools/zllog.py audit /dev/disk/by-partlabel/ZLLOG --latest --json
sudo tools/zllog.py read /dev/disk/by-partlabel/ZLLOG --latest
sudo tools/zllog.py export /dev/disk/by-partlabel/ZLLOG --latest \
  --json /tmp/zlos-all.json --csv /tmp/zlos-all.csv --text /tmp/zlos-all.txt
```

`audit` is the short causal view. `read` shows every ZLLOG record. `export`
creates files Codex can inspect without touching the USB again.

The causal view prints completed pairs as:

```text
#begin..#result audit ID operation actor#ID object=ID -> result=N error=N detail=N
```

It separately counts lifecycle records, completed operations, incomplete
operations and unmatched results for each boot segment.

## First physical test

This destroys the selected USB stick. Resolve the whole-disk path first; never
guess it.

```sh
cd kernel
lsblk -o NAME,SIZE,TYPE,TRAN,MODEL,MOUNTPOINTS
./mkusb.sh --log-mb 512 /dev/sdX
```

The script asks you to type the exact device path again, flashes it, moves the
backup GPT to the real end of the stick, reinitializes only the device-sized
ZLLOG partition and validates it. Replace `/dev/sdX` with the whole USB disk,
not a partition such as `/dev/sdX1`.

Then:

1. Safely remove the stick and boot the ThinkPad from it.
2. Open and close several apps.
3. Create, save, read, rename and delete a file.
4. Open the browser and attempt one reachable and one deliberately bad name.
5. Run the Ring-3 process self-test if it is exposed in that build.
6. Wait at least 15 seconds after the final action so the periodic checkpoint
   can finish. For the recovery test, power-cut once during activity instead.
7. Boot Linux, insert the stick and run the `audit` command above.

Expected proof is not merely “records exist.” The newest boot must show matched
start/result records for the exercised operations, app/window exits, explicit
errors for the deliberate failure, valid CRCs, zero extractor warnings and a
visible incomplete tail only in the deliberate power-cut case.

## Evidence status

### Physical audit boots: third boundary isolated; I219 boot freeze quarantined

The first audit-expanded ThinkPad boot on 2026-08-22 did not reach the desktop.
The visible framebuffer stopped on `settings: no NVMe, load skipped` and the
machine was force-powered off after about five minutes. That line was not the
actual stop: ZLLOG survived the cut with a valid WRITING slot, **421/421 valid
records and zero drops**, and proved execution continued through Intel xHCI
recovery, mass-storage discovery and the first durable journal checkpoint.

The receipt exposed a real dependency error at the physical 2560x1440 mode:
`wm_boot_start()` constructed three windows and requested six retained client/
shell surfaces before `heap_up()` had initialized the allocator. All six
allocations were explicitly refused. QEMU's smaller GOP mode did not make that
ordering failure visible in the same way. The corrected order is now:

```text
xHCI enumerate -> ZLLOG mount -> arena -> heap/VMM -> compositor -> Ring 3
```

Boot milestones 200..207 are synchronously checkpointed after recorder mount,
arena, heap, VMM, compositor, Ring-3 self-test, network boot and immediately
before the graphical event loop. A future physical stop therefore identifies
the exact call that failed to return instead of leaving the last visible line
as misleading evidence. The corrected build is locally green in the complete
four-target source gate, native UEFI gate, UEFI exercise **32/32**, compositor
suite, heap **1,923/1,923**, decoder **15/15** and persistent writer **5/5**.
Only the corrected physical rerun can prove that the dependency repair closes
the ThinkPad failure.

The second physical boot proved that repair was necessary but not sufficient.
The recovered generation-2 WRITING slot contained **706/706 valid records,
zero drops, 52 lifecycle records, 51 completed operations, zero incomplete
operations and zero unmatched results**. Milestones 200 through 205 were
durable. Milestone 205 proves the allocator, compositor and complete Ring-3
self-test returned; milestone 206 was absent, placing the stop inside the
synchronous `net_boot()` call. The framebuffer still displayed the older
Settings line because no graphical frame had been presented yet.

The physical inventory explains the next boundary. The X1 Carbon Gen 8 has an
onboard Intel I219 (`8086:0d4f`) but no attached wired carrier, and its AX201
Bluetooth device (`8087:0026`, USB device class `E0`) appeared on four xHCI
companion root ports in the boot trace. CDC-ECM discovery re-enumerated every
unclaimed connected port before checking PCI Ethernet, even when the cached
device descriptor already proved the device was Bluetooth. The corrected path
now retains `bDeviceClass`, rejects known non-CDC classes before destructive
re-enumeration, and keeps only class `00`, `02`, and `EF` devices as possible
CDC candidates. The I219 initializer is also idempotent so `net_up()` followed
by `ip_auto()` cannot reset its live rings twice. Synchronous sub-milestones
210..215 delimit discovery, driver initialization, and DHCP attachment on the
next physical run.

Two firmware-adjacent warnings reported during the same test were checked from
Linux rather than attributed to zlOS. The RTC really contained
`2001-01-01`; synchronized Linux time was written back as UTC and verified.
The ThinkPad fan tachometer reports the impossible sentinel `65535 RPM`, while
CPU/package temperatures were approximately 55--56 C and Linux logged no
thermal fault. That is an embedded-controller/fan-sense hardware issue, not the
zlOS Settings or network stop; no manual fan override was applied.

The third physical boot retained **719/719 valid records, zero drops, 52
lifecycle records, 51 completed operations, zero incomplete operations and
zero unmatched results**. Milestone 205 was present. Discovery milestone 210
was followed by one bounded xHCI timeout and successful recovery completions;
211 then proved discovery returned. Milestone 212 was the final record and 213
was absent, proving the permanent stop was inside `e1000_init()` for the
physical `8086:0d4f` I219. The recovered USB event is therefore evidence, not
the terminal fault.

The legacy 8254x reset/register sequence is not a valid physical I219/PCH
driver. The driver now inventories I217/I218/I219-family IDs but returns before
any MMIO write. Milestone 216 records the deliberate `8086:0d4f` quarantine;
boot then continues through 206 and 207. Automatic virtio-net and USB CDC-ECM
paths are unchanged, as are the proven 82540/82574-style paths. This removes
the desktop boot dependency without claiming that physical I219 Ethernet
works. A real I219 driver still requires the PCH MAC/PHY, ULP and
reset-arbitration sequence plus a physical carrier test.

The fourth physical image is now prepared on the only removable drive,
`/dev/sda` (3.6 GiB Imation). Independent read-back found a clean device-sized
GPT and FAT, and a fresh valid 512 MiB ZLLOG partition with GUID
`156e16f3-53e4-48db-a1b6-3dd99cc664b7`. Local and USB `ZLOS.EFI` both hash to
`f8e5efa5ac8722d00c228a0044f6d4af430fe15d05275e94996585512f15a81c`;
local and USB `BOOTX64.EFI` both hash to
`c75b81835b93ff4994ed2800d37fc6a6aa057e09c589f81d42a6d00cae970513`.
The full 62 MiB ESP also matches at
`8a00b2cbe4df8c50109d918037251c0757e3e0db913d9cfe857dc8032e050287`.
Firmware entry `Boot0002` names the new ESP PARTUUID
`42a1ea0f-1f29-4b61-9add-9d5b5ff8b1a1`, and `BootNext: 0002` is armed.

`mkusb.sh` initially returned status 141 after the successful media write and
ZLLOG initialization because its first-match `awk` exited early under
`pipefail`, giving upstream `efibootmgr` a SIGPIPE. The selector now consumes
the complete input for both the boot entry and partition lookup. A final rebuild
and flash exercised the repaired wrapper end to end and exited successfully;
every disk and firmware property above was then checked independently.
Preparation is still not execution: the next ThinkPad run must prove durable
milestones 213, 216, 206 and 207 and a visible desktop.

Local evidence on 2026-08-22 is green:

- decoder/format tests: **15/15**;
- persisted audit end-to-end tests: **5/5**;
- Intel NIC: **69/69** including zero-MMIO PCH quarantine, DHCP: **35/35**,
  IP: **152/152**, and TCP:
  **126/126** host checks;
- xHCI/CDC class filtering and USB mass-storage safety: zero failures;
- final native UEFI boot: **52 lifecycle records, 51 completed operations,
  zero incomplete operations and zero unmatched results**;
- every recorded process exit followed its correlated exit-syscall result;
- BIOS exercise: **32/32** and native UEFI exercise: **32/32**;
- native UEFI hardware-path gate: green, including Ring 3, process isolation,
  IPC, window ABI, persisted handoff and forced-recorder-refusal behavior;
- virtio, e1000 82540 emulation and USB CDC network probes: **18/18** each;
- filesystem, block, TCP, DNS, HTTP, compositor and browser host suites: green;
- browser sanitizer suite: **138/138** and browser fuzz: **402,427 checks,
  zero failures**;
- source-ownership gate: BIOS32, ELF64, native UEFI and raw-disk builds green;
- SMP probe: **4/4 cores** and zero framebuffer pixel difference.

Those results prove the audit chain in host tests and persisted QEMU media.
They do not prove the audit-expanded image on the physical ThinkPad. A fresh
physical run using the procedure above remains the final hardware gate; older
physical ZLLOG runs cannot prove these new event producers.
