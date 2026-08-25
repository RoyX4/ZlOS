# Physical input and recorder diagnosis — 2026-08-23

This is the evidence-backed answer for the ThinkPad boot that reached the
desktop but felt laggy, showed an early recorder failure/RAM-only boundary,
and made keyboard use in Terminal feel wrong.

## What the USB journal proved

The pre-fix physical run was recovered directly from the Imation stick:

- boot 4 retained 860/860 CRC-valid records with zero drops;
- the boot reached the desktop and completed milestones 210, 211, 212, 213,
  216, 206 and 207;
- Intel I219 was detected and deliberately skipped before any unsafe PCH MMIO;
- the internal keyboard was live through PS/2 IRQ1; no USB HID boot keyboard
  was present;
- logical key press, character and release events were recorded with no input
  drops or stuck repeat evidence;
- the first ZLLOG mount attempt failed at MSC stage 1 because no root port was
  connected yet after the xHCI reset;
- a bounded automatic retry later found the stick, completed USB mass-storage
  commands and emitted `storage-ready`; the boot screen then reported the
  persistent ZLLOG journal active.

So the early RAM-only result was a transient USB rediscovery boundary, not a
corrupt journal and not lost evidence. `diag` now performs one current bounded
mount attempt before reporting its mode, matching the existing automatic retry
and `diagsave` behavior.

## Why Terminal felt slow

The physical trace separated input transport from drawing:

- key events reached the queue in order and the queue did not drop them;
- measured input-to-present samples were 16,438 us and 17,745 us;
- a typed character damaged roughly 259,253 screen pixels;
- Terminal app drawing cost about 2.9 ms on those full-client refreshes.

The cause was `wm_dmg(shell_win)`: every printable character and Backspace
invalidated the complete retained Terminal client. The next frame rasterised
all visible scrollback text even though only the bottom prompt row changed.

The compositor now supports bounded client-local retained-surface refresh.
Printable characters and Backspace refresh only the prompt row. Enter still
invalidates the whole client because it can append output and scroll every
visible row. A missing/invalid surface, resize, animation or failed target bind
falls back to the old full redraw, so the optimization cannot expose stale
heap pixels.

## Verification

- `verify-sources.sh`: all four source targets green.
- optimized `wmtest`: zero failures, including partial update, unchanged pixels
  outside the strip, and moving the partially updated retained surface.
- `termwrap`: zero failures.
- `inputtest_hid`: PS/2 and USB mappings, modifiers, held keys and repeat pass.
- ZLLOG kernel end-to-end: 5/5 pass, including transient MSC auto-recovery.
- ZLLOG extractor: 15/15 pass.
- `verify-efi.sh`: EFI gate green.
- focused graphical UEFI exercise: help, real input events, existing desktop
  session and halt pass 4/4.

## Flashed physical artifact

The exact target was `/dev/sda`, model `ImationFlashDriv`, serial
`07B70D07914C6D7E`. It now contains the fixed image with a 62 MiB FAT ESP and a
512 MiB ZLLOG partition.

- image SHA-256: `ad3b254e2f1f5963ce3ded9fb491a452fc3882d3b2216b4d46ce8ac278cf6d3d`
- ZLLOG GUID: `5bef0337-aff5-4398-9752-e24017486e61`
- embedded/USB kernel SHA-256:
  `6d22cf5df1dc8c0df3111f905879abe4e6b57869f487e731b377ccc41f7b1224`
- embedded/USB stage-0 SHA-256:
  `812c42295b0beab5b4d06608772e427a42e4e314513bf796bb6e73737612444a`
- GPT verification: no problems found.
- both journal superblocks valid; generation 1 selected.
- both EFI files match the flashed image byte-for-byte.

The remaining proof is physical: boot this new image, type quickly in Terminal,
run `diag`, move windows, and shut down normally. Then extract the new journal
and compare prompt-row damage, app paint time, total frame time, input latency,
missed deadlines and drops against the pre-fix run.

## 2026-08-24: TrackPoint works, touchpad does not

That split is expected from the actual hardware paths, not evidence that the
whole mouse stack is broken:

- the red TrackPoint is PS/2 (`isa0060/serio1`) and reaches the existing IRQ12
  relative-mouse path;
- the Synaptics pad is `SYNA8006:00 06CB:CD8B` behind Intel LPSS I2C #1 and
  needs the separate HID-over-I2C stack;
- the compositor accepting TrackPoint motion proves pointer events, routing
  and drawing work; it says nothing about the I2C transport.

The reinserted Imation recorder was still pristine (`completed_boots=0`). The
reported physical boot therefore did not leave a durable ZLLOG slot, so there
was no I2C failure record to extract. Do not describe that run as logged.

The source audit found a more important safety defect before asking for another
physical probe: `i2c_find()` selected every Intel class `0c/80` function and
called `pci_enable()` before checking identity. On this ThinkPad those matches
are `8086:02e8` I2C #0, `8086:02e9` I2C #1, and `8086:02a4` PCH SPI flash. The
last device must never be treated as an I2C candidate.

The transport preparation is now bounded to exact `02e8/02e9` identities and
matches the required CML-LP sequence:

- use the existing PCI D3hot-to-D0 transition before MMIO;
- assert/release the LPSS function and iDMA reset bits;
- publish the DesignWare child remap address;
- program HCNT 191 / LCNT 345 for the measured 216 MHz CML clock and the
  firmware-declared 400 kHz bus, instead of the previous ~1.09 MHz values;
- try LPSS #1 first and fall back to #0 after a failed DesignWare identity;
- retain the final PCI device, reset, timing and `TX_ABRT_SOURCE` state in the
  terminal diagnostic and emit bounded driver-state ZLLOG records.

`hosttest/i2ctest.c` is the non-hardware regression: 10/10 checks pass,
including proof that the class-identical SPI flash controller is never enabled.
Strict 32-bit and 64-bit builds of the shipping transport and runtime bridge
also pass with warnings treated as errors.

This does **not** make the cursor move yet. The language can express the missing
logic; nobody has written the HID report-descriptor decoder and touchpad pointer
policy yet. The next safe physical milestone is a descriptor/report capture
from the corrected transport. Only that capture can settle the Synaptics report
layout before decoding, absolute coordinate scaling, tap/button and scroll are
wired into `input.c`.

## 2026-08-24: recovered "keyboard is weird" boot

The next forced-shutdown run was recoverable from the active WRITING slot even
without `diagsave` or a clean halt:

- image ID `106074bcc6f2b60580f3b487b5f1d97a926b4d7a97acae3d54ad57a7b11808ea`;
- boot 2, slot 0, 2,876/2,876 CRC-valid records;
- zero recorder drops and zero input drops;
- the internal keyboard reported on PS/2 IRQ1;
- xHCI reported no HID boot keyboard, so this was not a USB-keyboard path;
- `diag` submitted as command 200 with a four-character word and completed
  successfully;
- `clear` submitted and completed three separate times;
- five other entered lines were rejected as unknown with word lengths 4, 10,
  1, 1 and 1;
- Right (`0x111`), Down (`0x113`) and Delete (`0x119`) press/release events
  reached the logical input queue after `diag`;
- the Terminal window was explicitly closed by a pointer click near the end of
  the trace; the OS did not crash and the keyboard queue did not wedge.

Printable identity remains intentionally redacted in ZLLOG, so the recorder is
not a keylogger. The command-submit records and non-printing key identities are
enough to isolate this without recovering private text.

### Root cause 1: Terminal discarded editing keys

`input.c` correctly translated both PS/2 and USB HID navigation keys. The loss
was above the driver: `app_event()` translated only Enter, Backspace and Escape
for character-oriented apps. Any other `EV_KEY_DOWN` returned immediately, so
Terminal never received arrows, Home, End or Delete. Its line buffer also had
no insertion point or history; printable characters could only append, and
Backspace could only remove the final character.

That is why the transport could be healthy while the keyboard felt broken.
Correcting a typo with ordinary terminal habits silently did nothing, then
Enter submitted a different line from the one the user thought they had
edited.

Terminal now owns a bounded line editor:

- Left/Right move the insertion point;
- Home/End move to either boundary;
- Backspace deletes before the insertion point;
- Delete deletes at it;
- printable characters insert at it;
- Up/Down browse sixteen in-memory history entries and restore the unfinished
  draft on the way back down;
- duplicate consecutive commands are not added twice.

Only Terminal receives this character-oriented navigation route. Browser,
Files, games and registry apps retain their existing raw-key paths.

### Root cause 2: the model changed but old glyph pixels survived

The PS/2-only end-to-end gate found a second bug after all editing assertions
passed: `clear` was echoed and executed, but a before/after screenshot contained
exactly the same 9,519 sampled ink pixels.

The retained Terminal client surface is reused across invalidations.
`term_draw()` painted current glyphs but did not paint a background first.
Removing text from the scrollback or input model therefore drew nothing over
the previous glyphs. The same defect made Backspace look ignored even when the
line buffer was correct, and it explains why the physical trace contains three
successful `clear` commands.

`app_draw(APP_SHELL)` now clears the current clipped target to the Terminal
panel colour before drawing. A full invalidation clears the full client;
printable editing clears only the prompt-row dirty clip. The partial retained
surface path therefore remains fast while deleted pixels cannot survive.

### Verification and anti-false-green checks

- `termwrap`, compiled with warnings as errors: all pass. It directly checks
  insertion, cursor deletion, Home/End, history and draft restoration against
  shipping `term.c`.
- `inputtest_hid`: all PS/2/USB navigation mapping, modifier, held-key and
  repeat checks pass.
- `verify-sources.sh`: source-list probe reached 32-bit BIOS, 64-bit, UEFI and
  raw disk outputs, then disappeared from all four clean rebuilds.
- native UEFI `probe-term.py --uefi`: nine lines typed through the emulated
  keyboard; cursor correction, Delete, Up/Down history, command results and
  one prompt per line all pass. `clear` reduces sampled Terminal ink from
  39,685 to 21.
- PS/2-only `probe-term.py --ps2-only`: the USB keyboard is absent and the same
  nine-line gate passes through i8042. `clear` reduces sampled ink from 2,255
  to 53.

The physical retest remains the final proof. The claims above are exact for the
recovered old boot, host tests and QEMU; they do not pre-claim the new image's
behavior on the ThinkPad.

### Flashed retest artifact

The exact target was the by-id path for `ImationFlashDriv` serial
`07B70D07914C6D7E`. The 512 MiB-history image was written, synced and read back.

- kernel SHA-256, local and physical:
  `692917ed73e0d42481ad47929f357aa6502b9e787703952cc0eb9e5aa0c728d1`;
- stage-0 SHA-256, local and physical:
  `595f14669501ec8719ce9b1516e4807d3a23fc6f98d0f2ec82c133b131a5a538`;
- ZLLOG GUID: `d129026f-37b4-4c76-bf83-2468383689a8`;
- ZLLOG image ID equals the physical kernel SHA-256;
- both superblocks valid and pristine (`completed_boots=0`, no active slot);
- GPT verification reports no problems;
- read-only FAT verification reports six files and no filesystem error.

## 2026-08-24: exact physical touchpad decoder and flashed candidate

### Physical Linux evidence removed the report-layout guess

The development machine is the target `20U90041AU` ThinkPad X1 Carbon Gen 8.
Linux exposes the internal pad at all of these independent seams:

- ACPI path `\_SB.PCI0.I2C1.TPD0`, HID `SYNA8006`, CID `PNP0C50`;
- PCI `00:15.1`, Intel LPSS I2C #1 `8086:02e9`;
- HID `0018:06CB:CD8B.0001`, handled by `i2c_hid_acpi` and
  `hid-multitouch`;
- two input views, `SYNA8006:00 06CB:CD8B Mouse` and `Touchpad`.

The physical sysfs report descriptor is 665 bytes, SHA-256
`38db11f9c9f15157e31589c9fe86321acaaad0899cd8e4a994db8ab4dc98d8a8`.
It defines:

- report ID 2: two buttons plus signed relative X/Y;
- report ID 3: five precision-pad contacts, each carrying confidence, tip,
  contact ID, absolute X `0..1162` and absolute Y `0..634`;
- scan time, contact count and the clickpad's physical button;
- feature report 4 with Digitizer Input Mode;
- feature report 8 with a maximum contact count of five and pad type.

A bounded root I2C trace around `HIDIOCGFEATURE(8)` recorded the real transfer:

```text
i2c-2 #0 a=02c write [22-00-38-02-23-00]
i2c-2 #1 a=02c read 4 -> [04-00-08-05]
```

That establishes address `0x2c`, command register `0x22`, data register `0x23`
and five contacts without inferring them from a different Synaptics model.

### Root cause: discovery and live input are different I2C transactions

The corrected LPSS code could discover and read descriptors, but
`i2c_hid_read_report()` still wrote `wInputRegister` before every read. That is
the combined write/repeated-start/read shape used for register and descriptor
access. It is not the live-input shape.

Upstream Linux services an I2C-HID input interrupt with `i2c_master_recv()`:
a direct read of `wMaxInputLength`, with no register prefix. See the upstream
[`i2c-hid-core.c`](https://github.com/torvalds/linux/blob/master/drivers/hid/i2c-hid/i2c-hid-core.c)
`i2c_hid_get_input()` implementation. zlOS could therefore identify the pad
yet receive no usable movement.

`i2c_hid.c` now:

1. tries the exact, self-validating X1C8 route first: LPSS #1, address `0x2c`,
   HID descriptor register `0x20`;
2. retains the full bounded scan only for manual diagnosis/other hardware;
3. sends HID `SET_POWER(ON)`, waits through a state machine, sends `RESET`,
   consumes its zero-length completion, then powers on again;
4. reads live input directly at no more than 100 Hz. This polling is the
   bounded fallback until zlOS has the pad's ACPI GPIO interrupt route;
5. records the first reports and all startup boundaries in ZLLOG.

The state machine never sleeps in `input_poll()`. QEMU and other machines get
one quick exact-device refusal rather than an exhaustive scan in the desktop
hot path.

### Decoder and pointer policy

`i2c_touch.c` is pure transport-independent policy against the exact descriptor:

- report 2 supplies signed relative motion and ordered physical-button edges;
- report 3 tracks the first active contact and converts its absolute samples
  into relative deltas, which then use the existing Settings speed and
  acceleration curve;
- a short low-travel contact produces an ordered press and release, so a slow
  frame cannot collapse tap-to-click;
- a physical click cancels tap synthesis, preventing a double click on lift;
- two active contacts produce wheel notches and do not simultaneously move the
  pointer;
- malformed lengths are rejected and counted.

`input.c` combines the pad's deltas/buttons/wheel with the PS/2 TrackPoint. The
pad becoming ready does not disable or shadow the red TrackPoint.

### Verification

- `i2c_touchtest`: 13/13 report, movement, tap, physical-click, scroll and
  malformed-length assertions pass with `-Wall -Wextra -Werror`;
- `i2ctest`: 10/10 LPSS selection/reset/remap/timing assertions pass, including
  proof that the class-identical `8086:02a4` SPI flash controller is never
  enabled;
- `inputtest_hid`: all PS/2/USB keyboard checks plus I2C movement, ordered tap
  edges and wheel routing pass;
- 32-bit BIOS, 64-bit and native UEFI builds have zero undefined symbols;
- `verify-sources.sh`: the new source reached BIOS, 64-bit, UEFI and raw-disk
  outputs, then all four rebuilt clean without the temporary probe;
- `verify-efi.sh`: native UEFI, framebuffer, Ring 3, hostile-fault containment,
  two-process switching/preemption/IPC, window-input ABI, stage-0 persistence,
  write-combining and forced-recorder-refusal checks all pass (`EFI gate green`).

### Final physical USB receipt

Target: Imation `ImationFlashDriv`, serial `07B70D07914C6D7E`, stable by-id path
ending `usb-Imation_ImationFlashDriv_07B70D07914C6D7E-0:0`.

- image size: 608,174,080 bytes;
- stage-0 local/physical SHA-256:
  `2a020a058eb16f4177abe0270fded50a19d65a3655ffb92bb4f6b93cb78b58e3`;
- kernel local/physical SHA-256 and ZLLOG image ID:
  `3a68f08fc1205240c521a707cd1177c377c981e1f04481be2e6c394a078a0972`;
- ZLLOG GUID: `55c1c134-6a82-40e0-9bb4-6ccfad7b2cd2`;
- partition: 512 MiB, 255 two-MiB boot slots, real disk size 7,570,752
  sectors;
- both superblocks valid and pristine: `completed_boots=0`, no active slot;
- independent GPT verification: no problems;
- independent read-only FAT verification: six files, no filesystem error.

The weakest link is now singular and physical: zlOS itself has not yet driven
the pad on the ThinkPad. Boot this image, wait for the desktop, move one finger,
tap, physically click, two-finger scroll, run `i2c`, then `diagsave` and `halt`.
The next Kali boot can read the new ZLLOG and either promote this to physical
success or name the exact remaining transport stage.

## 2026-08-24: physical motion succeeded; legacy mode caused the glide

The next ThinkPad boot crossed the missing physical boundary: the internal
pad moved the zlOS cursor and its physical click produced ordered down/up
pointer events. The recovered active ZLLOG slot contains 3,596 CRC-valid
records from the current image, including the complete LPSS/HID startup:

- controller `8086:02e9` selected and configured;
- all 665 report-descriptor bytes captured;
- HID device `0x2c` accepted with the expected `06CB:CD8B` identity;
- the touchpad service entered its live state;
- later pointer records moved from `(131,334)` to `(620,509)` and retained
  physical button transitions.

This promotes controller selection, I2C transport, HID discovery, report
receipt, decoding and input/compositor routing to **physical zlOS success**.
Tap and two-finger scroll still need explicit human confirmation.

The same trace explains the reported "gliding/slippery" cursor. One motion
tail advances by exactly `(3,1)` repeatedly, then accumulated slow frames jump
by exactly `(48,16)` every few seconds. zlOS had left the device in report-2
compatibility mouse mode. That report is relative and has no scan counter;
polling without the ACPI GPIO interrupt can reapply the last non-zero delta.
The shared mouse acceleration curve then amplified the replay.

The physical descriptor also contains Digitizer Input Mode as feature report
4. The upstream Linux multitouch driver selects value `3` for a touchpad, and
the upstream I2C-HID core encodes feature reports through `SET_REPORT`. zlOS
now sends the exact ten-byte command after reset/power-on:

```text
22 00 34 03 23 00 04 00 04 03
```

That selects precision report 3, whose absolute contacts and scan time cannot
replay relative motion. If the mode switch fails, the driver records stage 7
and refuses to publish the unsafe legacy pointer. `input.c` also applies the
Settings speed to touchpad deltas without applying the TrackPoint/USB-mouse
acceleration curve; TrackPoint behavior is unchanged.

The `i2c` terminal command had a separate real source defect: it reset and
re-probed the live controller, scanned the bus, then ran a five-second busy
loop inside command dispatch. During that loop the compositor could not paint
or consume keyboard input, which looks exactly like a frozen desktop. The
recovered journal does not contain command code 63, so it does not prove that
this physical run actually submitted `i2c`. The code defect is nevertheless
removed: `i2c` is now an immediate read-only status command owned by the
automatic driver. A QEMU terminal gate submits it on hardware with no LPSS I2C
and requires the prompt to return within the ordinary command timeout.

Before reinitializing the stick, the full 512 MiB journal partition and decoded
JSON for every valid slot were archived under
[`evidence/exercises/2026-08-24/physical-touchpad-2026-08-24/`](evidence/exercises/2026-08-24/physical-touchpad-2026-08-24/).
The decompressed archive and live `/dev/sda2` both SHA-256 to
`4583cf257b32fb0e9ac8afad7dda45d1ffe0d6087b93bcf126823906ac2430c8`.

Deterministic regression coverage now includes the exact precision-mode wire
packet, short-buffer refusal, decoder policy, ordered clicks/scroll, and proof
that a 20-count touchpad move stays 20 counts even with mouse acceleration on.

Final verification and flashed artifact:

- broad host compile completed through its final `uitest` target;
- `i2ctest`: 12/12; `i2c_touchtest`: 13/13; full input bridge: all pass;
- ZLLOG decoder: 16/16;
- source-list gate: marker reached BIOS, 64-bit, native UEFI and raw outputs,
  then disappeared from all four clean rebuilds;
- native UEFI Terminal gate: ten lines pass, including bounded `i2c`; `clear`
  ink `41,134 -> 53`;
- full native UEFI gate: green through Ring 3, two-process switching,
  preemption, IPC, fault isolation, window/input, stage-0 persistence,
  write-combining and forced recorder refusal;
- final image size: `608174080` bytes, with 512 MiB ZLLOG history;
- target: Imation serial `07B70D07914C6D7E` at its stable by-id path;
- stage-0 local/physical SHA-256:
  `09d6543012006e7137f5a6fe11d33e87a2dbf64fffa5b5fe67687825d50b8ce3`;
- kernel local/physical SHA-256 and ZLLOG image ID:
  `12117bd7c01613d797bf17212abb88da1795d0f0af78c837bf4d0ccc3cfd62aa`;
- ZLLOG GUID: `346a00c9-3ee0-4534-8174-6b816427057a`; both superblocks
  valid/pristine, 255 slots, zero completed boots and no active slot;
- independent GPT verification: no problems; read-only FAT check: six files,
  no filesystem error; neither partition is mounted.

The remaining physical retest is narrow: confirm the cursor stops immediately
with the finger, run `i2c` and confirm the desktop stays responsive, then test
tap and two-finger scroll before `diagsave` and `halt`.
