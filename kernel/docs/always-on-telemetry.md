# Always-on telemetry architecture

This is the operating contract for zlOS's whole-runtime flight recorder. It
exists to answer two different questions with one evidence stream:

1. What failed, changed state, timed out, or was dropped?
2. Why did a keypress, pointer move, frame, disk request, or network request
   take as long as it did?

It is intentionally not a byte-for-byte recording of the machine. Recording
every instruction, packet payload, pixel store, and MMIO read would change the
timing being measured, exhaust the USB, and collect private content. The useful
definition of “everything” is: every important transition is reconstructable,
every hot path has loss-visible counters, and an anomaly retains bounded raw
state from before and after it.

## Data path

```text
fault/IRQ lane ----> reusable 64-cell emergency lane \
driver exact events -------------------------------> 4096-record RAM ring
hot-path counters --> atomic totals/maxima --------/          |
input/frame state --> sampled records + 8-frame history       |
                                                              v
                                                    normal-context flusher
                                                              |
                                              CRC records -> sync -> header
                                                              |
                                    bounded 64..512 MiB ZLLOG GPT partition
                                                              |
                                      tools/zllog.py text/JSON/CSV export
```

No interrupt, input callback, frame paint, allocator, or device completion
writes to USB. Producers append to RAM or increment atomic counters. The
normal-context flusher owns USB I/O. Its own traffic is excluded from workload
counters, but warnings and failures raised while it writes are deferred through
the emergency lane rather than hidden.

## Five capture tiers

### 1. Exact events

These are admitted once per occurrence, subject to the explicit overload rule
below:

- CPU faults and heap-corruption panics;
- driver ready/refused transitions;
- xHCI, SCSI and NVMe command submission, completion and timeout;
- DNS, TCP, HTTP and TLS failure/state transitions;
- user-mode syscall entry and exit;
- task creation, exit and scheduler lifecycle transitions;
- successful filesystem create/write/delete mutations and failed operations;
- input, network, storage, allocation and recorder drops;
- display-mode selection/refusal and framebuffer geometry/cache state.

Each fixed 64-byte record has a global sequence, TSC, CPU, severity, subsystem,
event, three numeric fields, 12 payload bytes, and its own CRC. Numeric values
carry operation IDs, correlation/span IDs, states, statuses, sizes, register
snapshots, or hashes. They do not carry arbitrary user content.

Fault and IRQ producers use a separate reusable emergency lane. This matters:
letting an IRQ interrupt a normal ring append could make the diagnostic system
corrupt itself. Normal context drains that lane later. A fatal exception that
halts the CPU is exact in RAM, but cannot safely promise a USB write after the
kernel is already broken. The normal panic path attempts a final synchronous
checkpoint only when a validated target exists and recorder I/O is not already
active. A future reserved crash page/next-boot importer remains necessary for
durable evidence from an arbitrary fatal exception.

RAM admission is priority-aware. Sampled records stop at 2,048 queued records,
ordinary exact transitions stop at 3,584, and the final 512 cells are reserved
for warnings, faults and drop boundaries. Every refused admission increments
the recorder-overwrite total. This means overload is visible and high-rate
samples cannot evict the event explaining the failure.

### 2. Counters and maxima

High-rate repetition increments atomic RAM totals instead of generating a
record each time. Totals and observation counts use explicit high/low carry so
frame bytes and pixels cannot wrap silently after a few seconds. Changed totals
are checkpointed once per second and again on mount and clean completion. Every
counter record includes the exact 64-bit total, the bounded one-second delta,
observation count, and maximum where meaningful.

Current counters cover:

- timer, keyboard, mouse and stray interrupts;
- xHCI events, commands, transfers and USB journal bytes;
- MMIO and NVMe completion polling;
- network receive, transmit and drops;
- allocations, frees, requested bytes and heap refusals;
- scheduler switches/yields and syscalls;
- filesystem reads, writes and mutations;
- painted, late and missed frames;
- damaged pixels, presented bytes and input queue pressure;
- recorder overwrites and checkpoint latency.

Recorder omissions increment both a durable total and the slot's dropped count;
explicit drop records are retained when priority capacity still exists.
Overload therefore cannot look like a quiet system.

### 3. Sampling

Ordinary pointer movement and healthy frames are deliberately sampled. Button
and wheel edges, queue drops and errors remain exact. Printable key identity is
redacted; timing, event kind and queue pressure survive.

- one in 16 ordinary pointer moves/input batches, with exact screen x/y,
  buttons, queue depth, hardware-boundary TSC and input sequence when retained;
- one in 60 healthy painted frames;
- the first eight late frames, later representative late frames, new peaks,
  severe frames and all frames during a triggered window;
- idle frames emit nothing.

Every retained screen record correlates the oldest queued input sequence with:

```text
input dequeue -> app tick -> compositor/damage -> vblank wait -> present
```

The input timestamp begins when the PS/2 packet or xHCI report completes in
software, before later polling and queueing. It therefore measures
driver-boundary-to-CPU-present, not physical click-to-photon latency and not
actual scanout. The frame includes input-to-present time, queue depth, damaged rectangles, damaged
pixels, bytes presented, missed deadlines, and the phase timings. This can
distinguish input backlog, slow zl app code, expensive repainting, vblank wait,
and a slow final framebuffer copy. Counting pixels and bytes makes comparisons
across resolutions meaningful.

The current instrumented build adds one record beside every retained frame,
under the identical admission policy. It splits compositor time into cached
desktop/wallpaper restoration, C window chrome, zl application drawing and
animation/effect work. It also carries the pre-paint WM damage-rectangle area,
window visits and app-draw call count. The remainder from total compositor time
is loop/intersection/cursor overhead. These are RAM appends only; ordinary
unretained frames get no record and no USB I/O.

### 4. Triggered bursts

A timeout or the first late frame opens a one-second detailed window. During
that window all logical pointer and painted-frame samples are retained. A late
frame also emits the preceding eight-frame RAM history before continuing, with
a ten-second history cooldown. Driver timeout triggers retain the deciding
status, poll count, operation, and completion code at the failure boundary.

The xHCI early-boot diagnostic persists bounded TRBs, transfer events and
endpoint contexts in its separate firmware witness. Runtime xHCI MMIO/command/
transfer waits, NVMe status/completion waits, GPU forcewake/ring waits, Intel
vblank waits, network waits and TCP terminal timeouts now emit typed bounded
`SNAPSHOT` pairs immediately before and at the deciding timeout; TCP also emits
the post-recovery state. The extractor names the operation and phase and prints
both raw words. Future larger descriptor/register dumps must stay bounded and
be split into typed records rather than changing the fixed record format.

### 5. Laboratory tracing

`kernel/trace-qemu.sh` runs a bounded, disposable QEMU experiment. Its default
mode captures interrupts, resets, guest errors, and unimplemented operations.
The explicit `--instructions` mode captures QEMU execution blocks for one
narrow investigation. It has time and output-size limits and boots the USB
image snapshot-on, so the laboratory trace cannot mutate the journal image.

The kernel also detects the Intel Processor Trace CPUID capability and records
whether the machine offers it. Normal boots never write Intel PT MSRs. Enabling
PT still requires a reserved ToPA buffer, kernel/user filtering, overflow
handling, a start/stop trigger, and an offline decoder tied to the exact image.
It must be implemented as a narrow laboratory mode, never as the always-on
recorder.

## Retention and power loss

The partition may be 64, 128, 256 or 512 MiB, producing 31, 63, 127 or 255
two-MiB segments. `mkusb.sh --log-mb N` builds the chosen bounded history size;
the default remains 64 MiB. A long-running session no
longer stops when one segment fills: it seals the full segment, synchronizes
it, publishes the alternate superblock, writes a continuation checkpoint, and
starts the next segment. Sequence numbers remain globally contiguous. After 31
segments, the oldest completed segment is reused; the active segment is never
the overwrite target.

Records are periodically checkpointed from normal context, currently no more
than once every ten seconds during normal use, and on clean completion. A hard
power cut can lose the newest RAM-only interval but cannot erase older durable
segments. Each record, slot header, and alternating superblock has independent
CRC/commit validation so the extractor can recover around torn metadata.

Periodic checkpoints write at most 512 records (32 KiB) per pass. Failed writes
use exponential 2..64-second backoff rather than blocking and retrying every
idle interval. A faster USB reduces those checkpoint stalls. A larger stick
increases history only when the image is deliberately built with a larger
`ZLLOG` partition; spare capacity is ignored. Neither one fixes screen lag:
frame presentation remains a RAM/framebuffer-cache and compositor problem,
while the USB is only the delayed evidence sink.

## Privacy and payload rules

Default telemetry records sizes, hashes, states, IDs, timing and result codes.
Boot text capture stops at the exact `system ready` boundary, and printable
input codes are redacted. It does not record passwords, desktop command text,
HTTP bodies, TLS plaintext, packet payloads, file contents, browser history, or
framebuffer screenshots.
Laboratory payload capture must be explicit, narrow and disposable.

## Current proof and remaining physical gate

Host tests cover CRC parsing, wrong-device refusal with zero writes, torn
super/slot recovery, automatic mount retry, 64-bit counter carry, privacy
redaction, reusable IRQ capture, priority admission, pointer/frame decoding,
64 and 128 MiB formats, exact write containment, and full-segment continuation
with contiguous sequences.
The normal and instruction QEMU trace modes are bounded by construction.

The current disposable writable OVMF/xHCI proof, rerun after the counter,
privacy, pointer-timestamp, cache-type, priority-admission and command-pairing
changes, produced one COMPLETE boot with 351/351 CRC-valid records, globally
contiguous sequence 1..351, no extractor warnings and zero dropped records.
It contains 46 exact command submissions and 46 completions. The shell's
`quit` boundary is an explicit kernel submit at record 335 and completion at
336. USB pointer press/release records 301/307 retain buttons 1/0 and input
sequences 1/4. Five retained frame extensions carry nonzero input sequence and
driver-boundary-to-present latency; the first reports 30,743 us, 2,304,000
damaged pixels and 9,216,000 presented bytes. Three xHCI MMIO timeouts each
retain a decoded before/trigger snapshot pair, six snapshot records total.
Both 1280x800 and 1920x1200 framebuffer states decode as uncacheable in QEMU.
Printable command characters decode as redacted zeroes and `quit` is absent
from persistent text.

Exact containment passed: the first MiB, the 63..64 MiB pre-journal gap and
the 128..132 MiB tail/backup-GPT range stayed byte-identical; BOOTX64.EFI and
ZLOS.EFI extracted before and after are byte-identical; only the bounded
journal and stage-zero's intentionally replaceable witness can change. GPT
verification remained clean. Those timings describe QEMU/KVM, not the
ThinkPad.

The expanded recorder is now physically proven. A 2026-08-21 ThinkPad session
completed with 6,503/6,503 CRC-valid records, contiguous sequence 1..6503, zero
drops and no extractor warnings. It retained 1,291 frame/extension pairs, 608
cursor-only correlations and automatic storage recovery. The live display
record changed from uncacheable to write-combining; present-size regression
measured about 7,089 MB/s and seven full-screen presents had a 2.070 ms median.

That run identified compositor/drawing as the next owner but predates the new
four-way paint split. The exact Imation stick is now flashed with the paint-
phase build and a 512 MiB journal. The remaining gate is one short physical
session that reproduces a slow interaction and extracts at least one decoded
`frame-paint-phases` record; that record, not inference, chooses the next code
optimization.
