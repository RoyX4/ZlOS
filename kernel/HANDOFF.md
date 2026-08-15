# zlOS — where things stand

Read this first in a new session. Everything below is verified, not remembered.

## What this is

An OS written in `zl` (Roy's own language) at `~/Documents/repos/zl-linux/kernel/`.
Boots three ways with no GRUB: BIOS multiboot, our own 512-byte bootloader
(`raw_boot.asm`), and as a native UEFI application (`efi.c`).

**The point of the project is the Intel display driver — the DPLL and a
cold-start modeset.** The laptop is a test PC. Optimise for that.

## The development loop that matters

`kernel/hosttest/` compiles **the same `intel.c` that ships in the kernel** as a
Linux program against the real GPU's PCI BAR. Seconds per iteration instead of
write-USB → reboot → read-screen.

```
cd kernel/hosttest
./gpu-dev.sh probe          # read everything (safe, i915 keeps running)
./gpu-dev.sh dump a.txt     # 300 registers
./gpu-dev.sh diff a.txt b.txt
sudo ./dpll_test 2 148500   # program an UNUSED DPLL and watch it lock
```

Map **8 MiB** of the BAR, not 16 — the kernel refuses the full BAR while i915
holds it. All display registers are under 1 MiB.

## Verified working on the real hardware (Comet Lake 8086:9B41)

- Panel: **2560×1440 on the eDP transcoder**, 2720×1481 total, X-tiled, stride 10240
- **DPLL programming PROVEN**: 720p/1080p/1440p/4K60 all computed, programmed,
  and **locked**, with the hardware restored exactly afterwards
- **AUX/DPCD works**: panel is DPCD 1.2, max 2.7 Gbps HBR, 4 lanes, **no rate table**
- Link training register offsets confirmed against the live trained link
- Panel power delays read from firmware: T1+T3 200 ms, T9 50 ms, **T11+T12 500 ms**
- DDI buffer translation table read off this machine (matches `skl_u_ddi_translations_edp`)

## Gen9 register traps — all of these cost real debugging time

| Trap | Truth |
|---|---|
| eDP TRANSCONF | **0x7F008**, not 0x6F008 (which is HSYNC and reads plausible) |
| eDP timings | **0x6F000**, not transcoder A at 0x60000 |
| PLANE_STRIDE units | 64 B linear, **512 B X-tiled**, 128 B Y-tiled |
| PP_CONTROL unlock key | **none on Gen9** — 0xABCD is pre-DDI only |
| Panel cycle delay | **PP_CONTROL[8:4]**; PP_DIVISOR is dead on CNP/CMP |
| Watermark fields | **lines 18:14, blocks 9:0** (narrow) |
| DP_TP_STATUS | **does not exist on DDI A** |
| DPLL enable regs | 0/1 = 0x46010/0x46014, **2/3 = 0x46040/0x46060** — not an array |
| DPLL0 | **feeds CDCLK. Never disable it.** |
| DPLL CFGCR | indexed from **DPLL1**, not DPLL0 (DPLL0 has none) |
| DPLL lock time | PRM says 5 ms; **measured ~80 ms** on a cold WRPLL |
| DP intermediate rates | 2.16/3.24/4.32 Gbps are **eDP-only**, need a DPCD rate table |

Full researched plan with 13 source conflicts: `kernel/docs/gen9-modeset-plan.txt`

## The one thing blocking a cold-start modeset

It needs the display to itself. gnome-shell + Xwayland hold `/dev/dri/card0`,
so unbinding i915 under the live session kills the desktop, not just a console.

**Ask Roy to do this, then say go:**
```
Ctrl+Alt+F3
sudo systemctl stop lightdm
```
Recovery: `sudo systemctl start lightdm`.

## Everything else in the kernel

15 drivers, ~6,000 lines, all ours: `pci` `bga` `intel` `xhci` `efi` `apic`
`virtio_gpu` `cpu` `nvme` `sched` `smp` `i2c_hid` `input` + two SMP trampolines.

64-bit, 4 cores woken via INIT/SIPI, multitasking scheduler, NVMe persistence,
USB mass storage, USB HID keyboard, event-based input with modifiers and repeat,
a line editor with history.

**Unproven:** `i2c_hid.c` (QEMU has no Intel LPSS I2C) and the cold-start modeset.

## The recurring bug class — check this FIRST

Five times now: **a DMA buffer outside guest RAM, or an address truncated to
32 bits.** Symptoms look like protocol bugs.

- zlOS needs `-m 256` minimum; the DMA arena starts at 224 MiB
- `u32 reg = xop + OFFSET` where xop is a 64-bit BAR → **reads correct, writes
  vanish**, 64-bit build only
- Every driver now ships a `*_ram_ok()` probe

## Verify before believing anything

```
cd kernel
./verify.sh        # BIOS golden transcript
./verify-raw.sh    # our own bootloader
./verify-iso.sh    # UEFI
./try.sh serial    # drive it from the terminal
```

`try.sh` (GUI mode) is written but **NOT yet verified to boot** — that was in
progress when the session ended.
