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

## Finishing the display: `docs/display-roadmap.md`

The decision (2026-08-17): **complete the display subsystem entirely before any
GPU work.** No ring buffers, no blitter, no execution engine until every item in
that roadmap is done.

Ordered by dependency, and the order is not the appealing one — the two most
interesting phases are blocked behind a parser and a grind:

```
0  close what is open      wire into zlOS, second modeset, LT retry,
                           EDID over AUX, X-tiled scanout
1  VBT parsing             unblocks 2 and 3; a parser, no hardware sequence
2  de-hardcode pipe/port   42 *_A registers; mechanical; blocks 3/4/5
3  HDMI, then external DP  HDMI first - no training, no panel power, no T12
4  hotplug                 zero lines today; needs a real interrupt path
5  planes, rotation, scaling, tiling
6  colour (gamma, CSC)
7  PSR / DRRS
8  audio over HDMI/DP      needs an HDA driver alongside
```

State measured rather than remembered: **one port, one pipe, one panel, one mode,
polled.** 42 `*_A` registers, no VBT parser, no HDMI port bring-up (the DPLL
clock math exists and is verified), and zero hotplug or interrupt code.

## What is actually a wall: `docs/what-is-actually-impossible.md`

**The project's thesis is that one person can do what is assumed to need a team,
so "too big for one person" is not a valid objection here — it is the hypothesis
under test.** That document re-grades the whole board against that bar. Summary:

- **Nothing on the board is impossible because of size.** Refuted by Terry Davis
  (TempleOS, ~10 yr solo), nakst (Essence, 9 yr solo), Kling, Bellard — and
  partly by this repo already.
- **The real walls are all one shape: a vendor holds a signing key, or the
  target is a competitor's shipping product.** There are about six, each with a
  legitimate route around: WiFi/CNVi (→ USB tether), Widevine (→ nothing to
  build), cellular, Bluetooth, Secure-Boot *distribution* (→ enrol your own key),
  GPU 3D (→ software rasterizer). **Modesetting is deliberately not on this
  list** — no blob, no signature, which is why it was reachable.
- **What kills solo OS projects is the breadth tax and stopping, not
  difficulty.** The golden-transcript gates and this file's verified-not-intended
  discipline are the defence. Don't let them slip.
- **Highest impressive-per-remaining-work item on the whole board:** zlOS booting
  the ThinkPad from `raw_boot.asm`, lighting its own panel at 2560×1440 through
  its own modeset, drawing its own desktop — no GRUB, no Linux, no blob. Blocked
  on **one missing caller**: nothing in the kernel arms `lt_armed`.

## The three "what about…" questions, answered: `docs/beyond-the-kernel.md`

Other languages on zlOS, the internet, and rewriting the C in zl. Asked and
measured 2026-08-17. Short version:

- **Other languages** — no heap, no ELF loader, no per-process address space
  (all four cores share one CR3, `smp.c:129`), and `fs_save`/`fs_load` is a RAM
  slot array, not a filesystem. Cheapest real win is hosting **zl's own
  interpreter** (1,900 lines) on zlOS; highest leverage is a **WASM interpreter**
  (~8k), which buys every language at once. POSIX ELF is a second project.
- **Internet** — zero network code today, but `lspci` says the ThinkPad has a
  real wired NIC (`8086:0d4f`, `Kernel driver in use: e1000e`). Wired is
  bounded: ~10k lines to HTTPS, of which TLS is 5–8k.
- **WiFi and Bluetooth** — Roy wants both. Plan: **`docs/wireless-plan.md`**.
  That doc also **corrects two wrong claims** made earlier the same day in
  `what-is-actually-impossible.md` and `beyond-the-kernel.md`:
  - **Bluetooth is NOT a wall.** `lsusb` → `8087:0026 Intel AX201 Bluetooth`,
    **on USB**, `Class=Wireless / RF / Bluetooth` (224/1/1) — the standard class
    every BT dongle implements, and **HCI is a published Bluetooth SIG
    standard**, not a vendor protocol. Its endpoints are control + interrupt IN
    + bulk IN/OUT, and **`xhci.c` already implements all three**. ~6,800 lines
    to a working BT keyboard; **~2,000 to "zlOS lists nearby devices"**. Build
    against a **CSR dongle** (ROM firmware, no upload) before touching Intel's
    704 KB `ibt-0040-0041.sfi`.
  - **The iwlwifi blob is 1.3 MB, not 2–3 MB** (1,406,572 bytes, measured), and
    it is redistributable — so the AX201 is Kind-1 hard (~40k lines of
    undocumented protocol), not vendor-gated.
  - **WiFi is only gated if you let Intel pick the radio.** The **AR9271 USB
    dongle's firmware is 50 KB and open source** (`open-ath9k-htc-firmware`,
    already at `/lib/firmware/htc_9271.fw`). ~8,000 lines to WPA2 association;
    **~3,800 to "zlOS printed my SSID list"**. An ESP32 over UART is ~400 lines
    if you just want the network stack unblocked now.
  - Still true: **USB tethering or a USB NIC** is the shortest path to packets,
    and `xhci.c:1709 configure_bulk()` is why.
- **A browser** — `feature-catalogue.md` says don't; **that call was wrong as
  stated** and `beyond-the-kernel.md` §2b supersedes it. "Unbounded" is true of
  a Chrome-compatible browser, not of a *document* browser — Dillo, NetSurf and
  w3m have shipped that for decades. A document browser (HTML+CSS, no JS) is
  **~13,200 lines** plus the ~10k network stack. **The gate is a heap** (~300
  lines) — which means building one is a *decision to relax a stated design
  constraint*, not just work. Useful: the HTML→CSS→layout→paint branch needs no
  network and is testable in QEMU against local files.
  Also found: **the fonts are baked at build time into three fixed sizes only**
  (8×16, 16×32, 24×48, monospace, from DejaVu Sans Mono via `gen_hd_font.py`).
  There is **no runtime rasterizer** — see the correction below.
- **C → zl** — the blocker is the compiler, not the kernel. The kernel builds
  with `compile`, the backend the root README marks **ARCHIVED**. Measured:
  `sizeof(Value)` 40 B, builtins dispatched through a **309-entry strcmp chain**
  with `band` at #300, and every number a `double` so **64-bit BARs and DMA
  addresses are not representable** — the project's own recurring bug class.
  `compilel` emits real `i64`… until you use a bitwise operator, at which point
  the return type degrades to `double` and every operand is boxed and dispatched
  by name. 10M iterations: **C 7 ms, zl arithmetic 4 ms, zl bitwise 999 ms.**

**The actionable part:** making `band/bor/bxor/shl/shr` native `i64` instructions
in `compilel.c` is ~1,500 lines of compiler work total with the other three
prerequisites, needs **no laptop, no panel, no hardware access**, cannot break a
running driver, and is testable on the host against the interpreter. It is the
only major item in that document with that property.

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

Worse than unproven, on `i2c_hid.c`: it is a **transport with no decoder**.
`i2c_hid_byte(i)` returns raw undecoded bytes. Nothing turns a touchpad report
into an x, a y and a button — that code does not exist yet.

## The desktop — BUILT, and it is what boots

The inversion landed 2026-08-18. `wm_frame()` is the top of the system on any
machine with a framebuffer; the shell is window 0 inside it, with a scrollback
and typed commands. On a machine with no framebuffer the old text shell runs
exactly as it always did, and `verify.sh` still diffs it byte-for-byte against
`golden.txt`.

**Verified by boot, not by assertion:** `shots/v10-final.png`. All four boot
paths green — `verify.sh`, `verify-raw.sh`, `verify-efi.sh`, and the ISO.
`wmtest` 69 · `inputtest` 17 · `tritest` 9 · `fbbench` all green.

**Why the desktop looked small on a big screen, and where the effects went:**
`docs/desktop-scale-and-effects.md`. Short version: `ui()` was `cell_w() / 8`,
so it was 1 or 2 and never more, while the layout is written in 800 design
units - at 3840 wide that is 1920 units of space for an 800-unit design. It is
derived from the screen now (1..4) and the console cell is a separate question.

What the v10 pass added, with the numbers, is `docs/desktop-v10-plan.md` §8.
The five that matter most to somebody touching this next:

1. **Serial is an input source now**, not a thing the shell polls. `wm_frame()`
   reads `input.c`'s queue and nothing else, so a byte only `key_get()` could
   see was a byte the desktop could never see — every gate and probe in this
   repo would have gone blind the moment the compositor booted. `ser_rx()`
   probes the UART's scratch register first, because **an absent UART floats
   high**: the naive "is LSR bit 0 set" is true forever on the ThinkPad and
   would have injected an endless stream of 0xFF keystrokes on the one machine
   with no other diagnostic.
2. **C4 deleted the sticker-drag machinery** — `bg_buf` and `sp_buf` at 128 and
   160 MiB, `fb_bg_snapshot`/`fb_bg_restore`/`fb_grab`/`fb_stamp`. The back
   buffer moved into their 48 MiB and **now covers 3840×2160**: a whole-desktop
   redraw there went 44 ms → **9.71 ms**, and a full-screen fill 7.97 → 0.71
   cyc/px. Scene hashes byte-identical at every mode.
3. **`raw_boot.asm` loads a FIXED number of chunks.** It was 40 (1.25 MiB)
   against a 1.23 MiB kernel. A kernel over that limit is not a build error —
   it is silently truncated and jumped into. `mkdisk.sh` refuses to build such
   an image now, and CHUNKS is 60.
4. **The wallpaper is a cached bitmap**, and that is arithmetic rather than
   taste: a translucent full-screen pass is 22 cyc/px ≈ 22 ms at 1920×1200, the
   compositor redraws the wallpaper inside *every* damage rectangle, and the
   v10 background is six such passes. Cached it is 1.5 cyc/px. At 4K it does
   not fit the arena, refuses, says so, and falls back to the plain gradient.
5. **THE COMPOSITOR COULD NOT SEE THE MOUSE, and no gate could have caught
   it.** zlOS drives two pointers - an absolute usb-tablet through `xhci.c` and
   a relative PS/2 mouse through `idt.c` - and the `mouse_x` builtin has
   preferred the tablet since it was written. `input.c`'s `pump_mouse()` read
   `idt_mouse_x()` **and nothing else**. While the shell owned the screen that
   was invisible, because the shell called `mouse_x()` directly. The moment
   `wm_frame()` became the top of the system the queue was the compositor's
   only source of pointer events, so on any machine with a tablet attached -
   which is what QEMU gives and what `try.sh` attaches - **no EV_MOUSE was
   pushed at all**. No dragging, no clicking, no dock, no menu.

   Every gate in this repo drives zlOS by TYPING, and a dock that does nothing
   photographs identically to one that works. `probe-dock.py` exists because
   of this, and `inputtest` now asserts the preference directly.

6. **`console_mute()`** stops the console painting while the compositor owns the
   screen. The tee into term.c's scrollback and the write to COM1 both keep
   going — which is what keeps every gate reading exactly what it read before.

### The design docs, still worth reading

The shell owns the main loop and windows are a demo it launches. A real desktop
inverts that. Designed 2026-08-17:

- **`docs/DECISIONS.md` — every decision from 2026-08-17 in one page**, including
  the two that turned out wrong and why. Read this before the others.
- `docs/desktop-build-guide.md` — start here. What a compositor is, in plain
  words, and the build order.
- **`docs/desktop-TODO.md` — the ordered task list. Pick it up here.**
- `docs/desktop-plan.md` — the decisions, the evidence, and the line numbers.
- `docs/desktop-toolkit.md` — **the layer that was missing from every earlier
  plan.** An **immediate-mode** toolkit (`ui.c`), chosen because a retained
  widget tree needs a heap and *is* a list of children — zlOS has neither.
  Widgets return whether they fired; nothing is allocated. Hit testing re-runs
  `app_draw` with drawing off, the same trick as `intel_modeset_dry()`.
- `docs/desktop-look.md` — why it looks blocky. **The renderer is not the
  problem** — real TrueType glyphs, subpixel LCD rendering and gamma-correct
  linear-light blending are all already in. **Read "TrueType" precisely:** the
  shapes come from DejaVu Sans Mono, but `gen_hd_font.py` rasterizes them **at
  build time** into three fixed coverage atlases — `font8x16`, `font16x32`,
  `font24x48`, monospace only. There is **no runtime rasterizer and no arbitrary
  size**. Fine for a desktop, a hard blocker for a browser. It is three
  resampling bugs, the
  worst being `fb_icon24` nearest-neighbour upscaling every icon at 2×
  (`fb.c:929`).
- `docs/desktop-polish-and-speed.md` — what makes a desktop look modern, and
  what it costs. Three facts up front: **three of four cores are parked** in
  `cli; hlt` forever (`smp.c:79`) so all drawing is single-core; **nothing
  measures a frame inside the kernel** (TSC exists in `cpu.c`, not exposed to
  zl); and the renderer is now benchmarked — see below.

- **`docs/feature-catalogue.md` — every feature found across ~15 hand-built OSes,
  organised by subsystem with a zlOS have/partial/none column.** 16 categories,
  from kernels to clipboard. Ends with a ranked shortlist of what is actually
  worth taking, and an explicit list of what is not (microkernels, capability
  security, filesystems, browsers — each needs a heap or processes).
  Top of the shortlist is still **the clip rectangle**, which now has *three*
  customers: compositor, toolkit, and the 3D rasterizer.
- `docs/os-landscape.md` — survey of ~13 hobby OSes, written for the **3D goal**.
  Headline: **SerenityOS runs Quake III on a software rasterizer, no GPU** —
  LibSoftGPU, 16×16 tiles, barycentric, SIMD. That is the path, and `fb3d.c` is
  its first step. Also: **Essence OS is one person since 2017** (nine years) with
  an animated software *vector* renderer; **Haiku is 25 years old and its Intel
  driver is modesetting only**, i.e. where `intel.c` is aiming; and **Redox has
  excellent systems engineering and a "sluggish, unpolished" UI** — a good
  desktop is not downstream of a good kernel.
- `docs/desktop-prior-art.md` — how TempleOS, SerenityOS, Essence, Haiku, Redox,
  Managarm and Asterinas do graphics. **Nobody in this space has GPU
  acceleration.** SerenityOS runs Half-Life on a SIMD *software* rasterizer.
  **Essence OS is the closest analogue** — one developer since 2017, software
  *vector* renderer **with animation**, window manager **in the kernel** (same
  choice as `desktop-plan.md`). TempleOS was 640×480/16 colours; **zlOS is
  already well past it**. Do not write a 3D driver — the *display* driver alone
  has cost a 13-conflict plan and an 86-defect audit.
  **Note:** `docs/design/GRAPHICS_PLAN.md` (2026-08-03) says the GPU is reached
  via `opengl32.dll` FFI. That is the **Windows-hosted** plan and does not apply
  here — it has been annotated. On zlOS, 3D means a software rasterizer, and
  `fb3d.c` is its first step.

  **On "can we just take Linux's driver":** no, and the doc measures why on this
  machine. `i915.ko` is **11.2 MB** uncompressed and ~100K lines; Mesa's Intel
  Vulkan driver is **24.3 MB**; the **entire zlOS kernel is 1.07 MB** and 11,374
  hand-written lines. FreeBSD runs i915 only via **LinuxKPI** — it emulates the
  Linux kernel API rather than porting the driver, and i915 assumes GEM, TTM,
  dma-buf/dma_fence locking, workqueues and a heap, none of which zlOS has by
  design. **`intel.c` already does the correct thing: borrow Linux's knowledge,
  not its code.** Also: `virtio_gpu.c:314` disables virgl on purpose — enabling
  it would give real 3D in QEMU only, never on the laptop.
- `docs/desktop-northstar-feasibility.md` — can zlOS run the `~/zl OS v10.dc.html`
  mockup? **~65%, and what is left is applications, not machinery.** This file
  has now been wrong twice in opposite directions — 95% by counting visual
  effects and ignoring the toolkit, then 20% by counting "zl has no lists" and
  "there is no layout engine" as *permanent*. Neither was a property of the
  language: `ui.c` is a layout engine and `ui_list_row` expresses a list
  without a list type. **Separating "the language cannot" from "nobody has
  written it yet" is the whole lesson**, and getting it wrong costs you the
  wrong fix.

## The renderer is benchmarked: `hosttest/fbbench.c`

**This dev box IS the test laptop** — `i7-10510U`, Comet Lake-U, same chip family
as the `8086:9B41` graphics. So `fb.c` timed here runs on the real target CPU.

`fbbench` compiles the **shipping `fb.c` unmodified** at the kernel's own `-O2`
and `mmap`s the three fixed physical addresses fb.c hardcodes. No sudo.

```
cd kernel/hosttest && ./build.sh && ./fbbench
```

Baseline measured 2026-08-17, whole desktop with 3 windows:
**19.98 ms @ 1920×1200, 26.80 ms @ 2560×1440** — a 50 and 37 fps ceiling with
nothing else running.

It also puts the resolution cliff on a stopwatch: a full-screen gradient costs
2.93 cyc/px with the back buffer on and **7.97 cyc/px** with it off, and
`bg_snapshot` returns in **0.00 ms** at 2560×1440 because `bg_ok = 0` — that is
window dragging, doing nothing, measured. And it is a **floor**: the harness uses
ordinary RAM for VRAM, so real hardware pays the 30–50× VRAM read on top.

### `fb_shadow` fixed — 4.1× on a full desktop redraw

`fb_shadow` darkened the window's **entire footprint**, and the caller drew the
window on top of ~90% of it. It now skips the covered rectangle, inset by
`SHADOW_SKIP_INSET` so the rounded corners keep their shadow.

| | before | after |
|---|---|---|
| shadow 600×460 | 4.34 ms | **0.61 ms** |
| one window, full chrome | 5.12 ms | **0.90 ms** |
| whole desktop @1920×1200 | 19.98 ms | **4.88 ms** |

**Verified pixel-identical** (FNV hash of the whole back buffer) and `verify.sh`
passes. **A static instruction count first pointed at a per-pixel divide in that
same loop — tabling it made the shadow 25% SLOWER.** The loop is not
arithmetic-bound. Measure, then optimise, then measure again.

Two things found while planning that you need before believing anything about
the desktop on real hardware:

1. **At 2560×1440 the back buffer switches itself off** (`fb.c:155` —
   `BACK_MAX` is `1920*1200`), and it takes subpixel text, fast pixel readback
   and **window dragging** with it, silently. The ThinkPad panel is 2560×1440.
   Verified by reading; **not yet observed** — zlOS has never booted on it.
2. **`fb.c` has no clipping.** Every primitive clips to the screen and nothing
   else, so there is no way to repaint part of the screen. That, not the window
   code, is what blocks a real compositor.

## The recurring bug class — check this FIRST

Six times now: **a DMA buffer outside guest RAM, on top of another buffer, or an
address truncated to 32 bits.** Symptoms look like protocol bugs.

- zlOS needs `-m 256` minimum; the DMA arena starts at 224 MiB
- `u32 reg = xop + OFFSET` where xop is a 64-bit BAR → **reads correct, writes
  vanish**, 64-bit build only
- Every driver now ships a `*_ram_ok()` probe

**The sixth was caught by reading, not by running, and that is the lesson.**
`i2c_hid.c` had `HID_BUF` at `0x0C900000` — 9 MiB inside the 16 MiB arena
`fb.c` hands out for cached blurs, and inside the span `back` occupied before
the compositor moved it. Two owners, one address range, neither aware of the
other. It had never been *observed* because it could not be: QEMU has no Intel
LPSS I2C controller, so the touchpad driver only runs on the laptop, which is
also the only machine with a panel big enough to make the framebuffer reach.
The two halves of the bug were never on the same machine as a working test.

The map is now **[`kernel/memmap.h`](memmap.h)** — declared once, with every
owner asserting its own extent against its neighbours at compile time. It
replaced a comment in `fb.c` that carried the list *and told you not to trust
it* ("do not take this list on trust, re-grep it"). That instruction was the
admission; the list was already wrong when it was written.

```
cd kernel/hosttest && ./memmap-guard-test.sh    # seconds, no QEMU, no hardware
```

12 checks: the six owners compile, five deliberate breaks are each refused by
the build (including a replay of this exact bug), and the 34 addresses that were
rebased onto the header are proven identical to the literals they replaced.
**A `_Static_assert` nobody has watched fail is a decoration, not a guard** —
that is what the negative half of that script is for.

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
