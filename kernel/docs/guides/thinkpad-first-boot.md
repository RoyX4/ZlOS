# Booting zlOS on the ThinkPad — first time

For the X1 Carbon Gen 8. Everything here is a step you take at the machine; the
VM cannot do any of it. Written the night before, so read the whole thing once
before starting — two of these steps are the ones that waste an hour if missed.

## What you are booting

`zlOS-usb.img` — **GPT, an EFI System Partition, and no GRUB anywhere in this
path.** **Corrected 2026-09-04:** this used to say 64 MB with one partition and
"`BOOTX64.EFI` *is* zlOS". `mkusb.sh` now builds 132 MiB by default — a 62 MiB
ESP plus a 64 MiB `ZLLOG` journal partition (`--log-mb` up to 512) — and
`EFI/BOOT/BOOTX64.EFI` is a 20 KiB witness (`boot/efi_stage0.c`) that records
its entry and chainloads the real kernel, `EFI/ZLOS/ZLOS.EFI`, which
`buildefi.sh` compiles into a PE32+ EFI application. See `HANDOFF.md` §"The USB
boot flight recorder".

Verify it is ours, not GRUB's, before you write it, and check **`ZLOS.EFI`,
not `BOOTX64.EFI`** (corrected 2026-09-01: the command below used to name the
witness stub, which prints 0 because it is too small to carry the string):

```bash
cd ~/Documents/repos/zl-linux/kernel && ./tools/images/mkusb.sh
mtype -i zlOS-usb.img@@1M ::/EFI/ZLOS/ZLOS.EFI | strings | grep -c zlOS
```

That should print a non-zero number (17 on the 2026-09-01 build). Running the
same check against `EFI/BOOT/BOOTX64.EFI` prints **0** — not because it is
GRUB, but because the witness stub is too small to carry the string. The
genuine GRUB contrast is the EFI binary inside `zlOS.iso`, which also prints **0**.

## The two things that waste an hour

**1. Secure Boot must be off.** Our `BOOTX64.EFI` is unsigned, so the firmware
will simply refuse it — usually with no useful message. F1 at power-on →
Security → Secure Boot → **Disabled** → F10 to save.

**2. There is no serial port on this laptop.** Everything the boot log says goes
to a COM1 that does not exist, so the *screen* is your only diagnostic. If the
panel stays dark you get nothing at all. That is why `verify-efi.sh` exists and
why the GOP check in it matters.

## Writing the stick

`mkusb.sh` will do it, and makes you retype the device name first:

```bash
cd ~/Documents/repos/zl-linux/kernel && ./tools/images/mkusb.sh /dev/sdX
```

Find `/dev/sdX` with `lsblk` **after** plugging the stick in. Your internal disk
is `nvme0n1` — it holds your running Kali. Never that one.

## Booting it

F12 at power-on → pick the USB device → it should go straight to zlOS. No menu,
no timeout; the firmware hands control to the kernel directly.

## What a good boot looks like

```
  zlOS starting

  [  OK  ] UEFI application - no GRUB, no bootloader, 64-bit from the start
  [  OK  ] stack established, 256 KiB
  [  OK  ] COM1 initialised, 115200 8N1
  [  OK  ] framebuffer console, <WIDTH>x<HEIGHT>
  [  OK  ] GDT loaded - 64-bit flat segments, 4-level paging, SSE on
  [  OK  ] IDT installed, PIC remapped, interrupts ON
  [  OK  ] APIC: IRQs via I/O APIC at 0x..., 4 CPU(s)
  [  OK  ] PIT timer running at 100 Hz on IRQ0
  [  OK  ] keyboard on IRQ1 - no more polling the port
  ...
  ready.
```

Two lines to check specifically:

- **"UEFI application"** — if it instead says *multiboot handoff* or *our
  bootloader (raw_boot)*, you booted the wrong image.
- **"framebuffer console"** — if it says *VGA text console, 80x25*, the firmware
  gave us no GOP and **the panel will be black**. The kernel now prints a line
  saying so, but you will not see it, because there is nowhere for it to go.

## What to run first, in this order

> **Corrected 2026-09-04:** on a GOP boot the compositor is the boot state and
> the shell is a Terminal window that takes a **word plus Enter**, not a single
> key (`HANDOFF.md` §"How a harness types a command"). The key column below is
> the old text shell's; the Terminal words that reach the same commands
> (`term.c:355-381`, codes equal to the old key's ASCII) are `help` (`h`),
> `cpu` (`z`), `pci`/`hw` (`k`), `smp`/`cores` (`*`), `nvme`/`disk` (`o`),
> `usb` (`u`), `i2c`/`touchpad` (`?`); the rest are in that table.

| Key | Why this one, and what proves it worked |
|---|---|
| `h` | the shell responds at all — i.e. the keyboard is delivering |
| `z` | CPUID: should say Comet Lake, 4 cores, and **not** "running under a hypervisor" |
| `k` | **the big one.** On real silicon this reports the Intel GPU — `intel_ok()`, live mode, stride, EDID over GMBUS, panel power, backlight. In QEMU it only ever said "no Intel GPU (expected in a VM)" |
| `*` | SMP: wakes the other 3 cores with INIT/SIPI on real hardware |
| `o` | NVMe against the real controller — but see the warning below |
| `u` | xHCI on real silicon. This driver had four bugs that only appear outside QEMU (64-bit BARs, address truncation, firmware ownership, scratchpads) — this is where they would show |
| `?` | **the touchpad.** Zero test coverage when written; QEMU has no Intel LPSS I2C. (**2026-09-04:** `tests/host/i2ctest.c` exists and it has run on the laptop — `HANDOFF.md` 2026-08-24 note; `i2c` is now a read-only status command.) See below |
| `x` | the pointer. TrackPoint is PS/2 so it should track; it also prints its IRQ12 count and final position on exit |

**`o` (NVMe) writes to LBA 1000 of whatever NVMe controller it finds.** In the VM
that is a throwaway 64 MB image. On the laptop it is **your actual disk**. Do not
press `o` on the ThinkPad unless you have decided that is fine — check what
`nvme_find()` selects first.

## The touchpad, `?`

This is the one thing the VM structurally cannot test, and the reason it is worth
booting at all for input work.

`i2c_hid.c` is 315 lines and complete, but has **never executed** (**corrected
2026-09-04:** 649 lines, plus `i2c_touch.c`; it has executed on the ThinkPad —
see `HANDOFF.md`'s 2026-08-24 touchpad note). It expects the
Synopsys DesignWare I2C controller Intel puts at PCI `00:15.1` — under Linux the
pad shows up as `SYNA8006:00 06CB:CD8B Touchpad` on `i2c-2`.

It has no ACPI interpreter, so it cannot read the slave address from `_DSM`.
Instead it **probes**: walks 7-bit addresses 0x08–0x77 asking each for a HID
descriptor, and accepts the one that answers with length 30 and version 0x0100.
Slower than a table, but it cannot be wrong about a machine it was not written
for.

What `?` should print if it works: a DesignWare component-type match, the
address it found, and the VID/PID — expect **06CB:CD8B**.

Known weaknesses to expect on the first run:

- `i2c_init()` tries controller index 1 then 0, but once `i2c_found` is set a
  failed DesignWare check leaves no way to fall back. If it picks the wrong LPSS
  block it will not recover.
- The report buffer sits at `0x0C900000` (210 MiB) — **corrected 2026-09-04:**
  that address collided with `fb.c`'s blur arena and moved; `i2c_hid.c:155`
  `HID_BUF` is `HI_HID` = `0x0B800000` (184 MiB) from `memmap.h`. That is plain RAM the CPU
  writes itself, not DMA — but it is worth confirming the UEFI memory map calls
  that region usable, because the recurring bug class in this kernel is exactly
  "a buffer outside usable RAM".

## If it does not boot

Capture, in this order:

1. Did the firmware reject it? (Secure Boot still on → no zlOS output at all)
2. Did anything appear on the panel? Photograph it — the last `[  OK  ]` line is
   the whole diagnosis
3. If the panel is black but the machine is clearly alive (fans, caps lock LED
   responding), suspect the GOP path — `probe-uefi.py` in the VM shows which
   display devices publish a usable GOP
4. `?` hanging rather than printing means the I2C probe is spinning; that is
   `i2c_hid.c`, and it is the expected first-run risk

## What is NOT in scope for this boot

The cold-start modeset and anything arming `lt_armed` in `intel.c` (**2026-09-04:**
the `P` diagnostic's `panel_up()` — `kernel.zl:5146` → `intel_bringup_panel`,
`intel.c:4502` — now arms it, so do not type that casually either) — that is a
separate piece of work with a hardware-damage hazard list (violating the panel's
500 ms T12 delay, or driving AUX into an unpowered panel). Section 4.1 of
`kernel/docs/evidence/display/gen9-modeset-plan.json`. Do not go there casually.
