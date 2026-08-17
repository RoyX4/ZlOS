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
- Panel power delays read from firmware: T1+T3 200 ms, **T10 50 ms**, **T11+T12 500 ms**
  (this line used to say "T9 50 ms" — wrong. `intel_pp_t9()` and `intel_pp_t10()`
  read each other's field: PP_OFF_DELAYS 28:16 is T10, 12:0 is T9. The 50 ms is
  T10. The real T9 is 260 ms and lives in VBT, not in this register.)
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

## Stage 1 of that plan is DONE — every conflict settled on the real panel

`hosttest/modeset_test.c --survey` is read-only and runs with i915 up. **21 passed,
0 failed** (2026-08-17 — 18 for the four conflicts below, 3 more for the pixel
clock). It settles by measurement what the sources disagreed on:

| Was contested | Settled |
|---|---|
| C1 TRANSCONF | **0x7F008** reads `C0000000` (b31+b30). 0x6F008 decodes as hsync 2608..2640 — it really is TRANS_HSYNC |
| C2 PIPE_SRCSZ | **0x6001C** reads `09FF059F` = 2560×1440. 0x6F01C reads 0 |
| C5 unlock key | PP_CONTROL top half is `0000` — **no key**, as the plan said |
| C7 cycle delay | PP_CONTROL[8:4] = 6 → **T12 500 ms**. PP_DIVISOR ref divider 0, which the PRM forbids ⇒ dead register |

**The buf-trans table is `skl_u_trans_edp`, 10/10 exact.** That was the one fact
"only in VBT and not discoverable from any register" — it is discoverable, by
reading back what firmware programmed. The board is low-vswing eDP, **max vswing
level 3**, I_boost 0, and firmware has already set the balance-leg disable bit
(`DISPIO_CR_TX_BMU_CR0 = 08800000`) exactly as the plan's step 32 prescribes.

Also captured: **`saved_port_bits = 0x00000010`** (DDI_A_4_LANES set, port
reversal clear), CDCLK 337500 kHz, DPLL0 locked at rate_idx 1 (HBR).
`TRANS_DDI_FUNC_CTL = 82010006` — the plan predicted `0x82000006` + PHSYNC. Match.

**PSR is ON** (`EDP_PSR_CTL = 81F00406`). It must be cleared in Phase B step 4
before anything else, and it is why the frame counter reads `0 -> 0 = 0.0 Hz`
when the screen is idle.

## The pixel clock is MEASURED now: 241,690 kHz, 59.998 Hz

It never had been, and every bandwidth, watermark and link-rate number rested on
an assumed 60 Hz. It did **not** need PSR disabled. A DP link runs at a fixed
symbol rate, so the transcoder holds a ratio reconciling it with the pixel clock,
and `PIPE_LINK_M1/N1` *is* `pixel_clock : link_clock`. Read-only, exact, no
timer, correct while PSR is on.

Four independent sources agree, three of them registers this driver has to get
right anyway:

| Source | Says |
|---|---|
| `PIPE_LINK_M1/N1` = 0x72943 / 0x80000 × 270000 kHz | 241,690 kHz |
| `PIPE_DATA_M1/N1` = 0x7E55EF29 / 0x800000, via bpp and lanes | 241,690 kHz (delta 0) |
| Panel EDID detailed timing descriptor (`0x5E69` × 10 kHz) | 241,690 kHz |
| Frame counter, *with the screen busy* | 60.0 Hz |

So the assumed 60 Hz was right to 0.004% and **plan step 26 holds: 4 lanes @ HBR
is the only working point** (5,800,560 kbps needed; 4×RBR gives 5,184,000).
Confirmed with a real number rather than inherited from an assumption.

Panel is an **LG LP140QH2-SPD**, 309×174 mm, hsync **positive**, vsync negative —
which is exactly what `TRANS_DDI_FUNC_CTL = 0x82010006` encodes (b16 set, b17
clear). Another cross-check that landed.

**Do not trust the frame counter for this.** It is not reliably zero, it is
*intermittently* zero — 0.0 Hz idle, a correct 60.0 Hz with a terminal scrolling.
It passes in testing and returns 0 in the field.
`intel_pixel_clock_khz()` uses M/N first and keeps the counter only as the
fallback for a path with no M/N, which on this hardware means HDMI.

## M/N is implemented and verified bit-for-bit against firmware

Plan step 45 had **no implementation at all** — not a defective write path, an
absent one, and a hard blocker for Phase H. `intel_mn_compute()` /
`intel_mn_program()` now exist, and because firmware has already solved the same
problem for the same mode, the computation has a known-correct answer to be
checked against:

```
inputs: pixel 241690 kHz, link 270000 kHz symbol, 4 lanes, 24 bpp
  DATA_M1 (with TU)  firmware 7E55EF29   ours 7E55EF29   MATCH
  DATA_N1            firmware 00800000   ours 00800000   MATCH
  LINK_M1            firmware 00072943   ours 00072943   MATCH
  LINK_N1            firmware 00080000   ours 00080000   MATCH
```

M is **truncated**, not rounded — rounding misses. N is rounded *up* to a power
of two then capped (0x800000 data, 0x80000 link). `--survey` is 21/21 now.

## The rest of the pipe path — also absent, also now verified

Steps 46, 49–54 had **no code at all**. Each is a couple of lines and each one
wrong is a black screen with no error bit. Same method: compute, compare to what
firmware left. `./gpu-dev.sh probe` checks all of these every run.

```
TRANS_DDI_FUNC_CTL  firmware 82010006   ours 82010006   MATCH
TRANS_MSA_MISC      firmware 00000021   ours 00000021   MATCH
PIPE_MISC           firmware 00000000   ours 00000000   MATCH
WM_LINETIME         firmware 0000005B   ours 0000005B   MATCH
```

Three things the hardware corrected in the plan:

| Plan says | Hardware says |
|---|---|
| `WM_LINETIME` → **90** | **91** (`0x5B`). 2720×8000/241690 = 90.03 and i915 uses DIV_ROUND_**UP** — the plan truncated |
| cursor DDB 0..7, plane 8..891 | plane **0..858**, cursor **859..891** — inverted, and exactly fills 0..891 |
| cursor watermark 8 blocks | **13** blocks (`CUR_WM(0) = 8000000D`) — the plan's 8 is one short of what firmware asks |

Also settles plan uncertainty **#12**: `PLANE_WM` bit 30 (IGNORE_LINES) is **0**
in all eight of firmware's levels, so writing 0 is right. Firmware's real
per-level plane watermarks are 41/74/91/99/157/182/195/222 blocks — the plan
guessed "~21 blocks" for level 0 and suggested 256 as a safe margin; the real
number is 41 and 256 would have been a 6× over-allocation.

**C9 is still NOT settled.** Firmware's largest values (222 blocks, 11 lines) fit
inside *both* the narrow 18:14/9:0 and the wide 26:14/11:0 encodings, so this
data cannot distinguish them. Still using narrow, still on the plan's authority.

## The backlight was writing to the wrong registers

Two layouts exist and this code had the other one:

```
SKL / SPT   0xC8254 packs both: freq 31:16, duty 15:0
CNP / CMP   0xC8254 = freq, all 32 bits.  0xC8258 = duty, all 32 bits.   <- us
```

Measured: `FREQ 0x5EB2` = 24242 clocks of 24 MHz = **990 Hz**, `DUTY 0x556E` =
21870 = **90% brightness**. Under the packed reading, `intel_backlight_max()`
computed `0x5EB2 >> 16` = **0**, so `intel_backlight_set()` hit its `if (!max)`
and silently did nothing, and `intel_backlight_get()` returned the *period* as
the brightness.

It failed **safe** — max reading 0 meant it bailed before writing, so it never
corrupted the period. But it was dead code that looked live, and it is one of the
few write paths *not* behind `lt_armed`, so it would have been the first thing to
run for real.

## Still true, and the thing to fix next

**Nothing in the kernel ever arms `lt_armed`.** Only `hosttest/dpll_test.c` and
`hosttest/intel_probe.c` call `intel_link_train_arm()`. Every write path in
`intel.c` is unreachable from zlOS itself — the driver reads the display
correctly and cannot yet touch it.

## The ordered modeset exists now — 35 steps, and it can be read before it runs

`intel_modeset_run(port)` walks Phases B→H in the plan's order. Set the mode
first (`intel_modeset_set_from_hw()` takes it off the running panel), then run.

**`intel_modeset_dry(port)` walks the whole sequence writing nothing** — it needs
neither `lt_armed` nor a detached i915, so the order is reviewable on a live
desktop. `./gpu-dev.sh probe` prints it. That matters because the sequence is now
the *only* genuinely untested thing left: every primitive under it was checked
against what firmware programmed, but the order has never executed.

Dry mode is a macro, not a flag, for a real reason: C evaluates arguments
eagerly, so `ms_do(3, "...", intel_dc_states_block())` would touch hardware even
in a dry run. `MS_STEP` defers the call.

**Verified dry.** A full dry run followed by a register diff: all 25 registers
the sequence writes are byte-identical, and `PWR_WELL_CTL_DRIVER`,
`PWR_WELL_CTL_BIOS` and `DC_STATE_EN` are unchanged across two runs. The 22
registers that *did* move are i915's own — `PIPEDSL`, the frame and flip
counters, timestamps, a page-flipped `PLANE_SURF`, and `DDI_BUF_CTL` bit 7 which
is read-only idle status.

Failures report the plan step number, not "modeset failed".

Three things it needed that also did not exist:

- **A port enable/disable primitive.** `DDI_BUF_CTL_ENABLE` was defined and
  never written by anything, so `intel_link_train()` was writing training
  patterns into a port nothing had switched on. Plan implementation order #5
  calls this mandatory before any training attempt.
- **`intel_dbuf_enable()`** (step 10) and **`intel_cdclk_khz()`** (step 9).
- **`intel_iboost_set()`** (step 32), including the x4 rule — DDI A's upper
  lanes are driven by the DDI E field, so both halves need programming or two
  lanes sit on a different drive setting from the other two.

Two guards that were silently too narrow:

- `intel_pwr_well_enable()` rejected anything above index 3, which made **PW1
  (index 14) unrequestable** — plan step 6, a prerequisite for the whole display
  core. The REQ/STATE macros were always right; only the range check was wrong.
- `lt_armed` was declared halfway down the file, so a write path added above it
  failed to compile in a way that reads as "lt_armed is missing". Moved up with
  the other module state.

**Known deviation from the plan:** step 40 makes ONE training attempt and fails,
where the plan wants a bounded retry. Failing loudly is the plan's actual point,
and a correct retry must cycle the port first (4.3 #20) — worth adding once the
sequence has run once, not before.

Still missing: EDID over I2C-over-AUX (GMBUS does not serve eDP on DDI A).
`LINK_RATE_SET` Method B is **not** needed — this panel has no rate table.

## IT WORKS. The panel was lit by our own driver on 2026-08-17

`sudo ./modeset-run.sh --modeset` brought the ThinkPad's panel up from cold and
displayed a test pattern — a colour gradient with a white border, held for ten
seconds, confirmed by eye. All 34 steps green, then a clean teardown and the
desktop back with no power button.

```
*** PICTURE SHOULD BE ON SCREEN NOW - holding 10 s ***
underrun after 10 s: clear
teardown: clean
pipe off, panel off, port off
exit code: 0
```

**`underrun after 10 s: clear` is the second result.** Ten seconds of real
scanout with zero FIFO underruns, which validates the watermarks and the DDB
split derived from firmware — including the cursor's 13 blocks and the 0..858 /
859..891 division the plan had inverted.

### What the four failed attempts before it actually cost

Every one of them was a bug in the *harness or the checks*, not the modeset:

| Run | Reported | Truth |
|---|---|---|
| 1 | FAILED at step 56 | no framebuffer existed; armed a scanout of address 0 |
| 2 | 34/34 SUCCESS, dark | GGTT unreachable — 8 MiB map stops one byte short of it |
| 3 | FAILED at step 56 | plane WAS armed; `PLANE_SURFLIVE` returns address + status bits, so the equality check could never match |
| 4 | — | backlight enabled at **zero duty**: correct image on an unlit panel |

Run 1 also left the display half-configured and cost a hard power-off. That is
why `intel_modeset_teardown()` exists and why the harness calls it on the
success and failure paths both.

### The two mapping facts worth keeping

- The kernel will **not** serve one 16 MiB mapping of BAR0 — EINVAL, i915 or no
  i915. It will serve 8 MiB at offset 0 and 8 MiB at offset 8 MiB separately.
  Reserve 16 MiB with an anonymous `PROT_NONE` map and `MAP_FIXED` both halves
  into it; the driver then sees one contiguous BAR exactly as in the kernel.
- The framebuffer lives in **stolen memory** — `/proc/iomem` confirms
  `79800000-7d7fffff : Graphics Stolen Memory` inside `Reserved`, so it is never
  Linux RAM. Paint it through `resource2_wc`.

### Still true

**Nothing in the kernel arms `lt_armed`.** This runs from the host harness only.
zlOS itself still cannot light the panel — the driver can, and is proven to, but
the kernel has no caller. That is now the single thing between this and zlOS
booting on the ThinkPad with its own display.

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

`try.sh` GUI mode is **verified working** (2026-08-17). It was booting
`-kernel kernel.elf`, and QEMU's own multiboot loader never supplies the
framebuffer tag — it prints `multiboot knows VBE. we don't` — so `console_init()`
never reached `fb_setup()` and every graphical demo answered "needs the
framebuffer console". It boots `-cdrom zlOS.iso` now; GRUB supplies the tag.
Confirmed by QMP screendump: 1920×1200, desktop with windows, taskbar and a
System Monitor reading the real CPU. `virtio-gpu-pci` is still attached so `y`
keeps its own device.

**If you need `-kernel` for a fast text-only loop, that path is fine** — it boots,
the shell responds, and `verify.sh` gates it. It just cannot do graphics.
