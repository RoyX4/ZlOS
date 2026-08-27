# PRESSWORK on real hardware-path output - first measured boot

**2026-08-27, branch `design/presswork`, HEAD `7d1a11b`.** zlOS booted under QEMU by
`kernel/tools/probes/probe-shot.py` and screenshotted at 1920x1200. Every number below
was sampled out of that framebuffer, not read off a comment.

## The focus signal is exact

The design's whole claim is that focus is a luminance event: the focused window's header
inverts to a solid light plate at 6.4796:1 against the plate, with a 3dp vermilion
fore-edge down the plate's left side. Sampled from the boot with a window focused:

```
FOCUSED window header      #B6B0AB   308 px on one scanline    == ZD_KNOCK
UNFOCUSED window header    #322B27   356 px on one scanline    == ZD_BASE
focus fore-edge            #E8734F   3 px wide                 == ZD_VERM
```

Three tokens, three exact matches, in `fb.c` output rather than in CSS. The knockout
against the plate is 6.4796:1 and the fore-edge against the plate is 4.6319:1, which are
the figures `design.h` claims.

## What the first boot did NOT show, and why

The first shot showed no knockout on any window, which read as the feature being broken.
It is not. `wm_open()` sets `focus_win` to the window it opens, so focus at boot lands on
the LAST window opened - which in that state is a chrome-less window, and a window with
no chrome has no header to knock out.

That is worth keeping as a finding in its own right: **the desktop boots with its focus
signal invisible.** The design's loudest gesture does not appear until the user touches
something. Whether the shell should focus a chrome-bearing window at boot is a shell
decision, not a chrome bug, but somebody has to make it.

## The measuring instrument

The trap here is that the crop LOOKED wrong. At a downscaled crop the knockout reads as
mid-grey against the surrounding dark, and it was called "nowhere near #B6B0AB" on that
basis. Sampling the raw PPM said otherwise, to the byte. Eyes are the right instrument
for "is the focus obvious at a glance"; they are the wrong one for "is this the right
colour", and the two questions are easy to conflate when looking at the same picture.

## Still open at the time of writing

- `wmshot` cannot render the shell. The rail, raster strip and foot are `kernel.zl` and
  do not compile into the host harness, so no wmshot render can show them. Anything
  judged from wmshot is a judgement about `wm.c` and `uikit.c` only.
- The desktop boots with nothing chrome-bearing focused (above).
- Parity gaps against the prototype: window titles are centred rather than left-aligned
  with a subtitle, the per-window module code and status band are absent, the rail rows
  carry no icons or counters, the raster strip is nearly empty, and the clock reads
  00:00.


---

## Update, same day: parity pass, and the same mistake made twice

After the parity work (`presswork-boot-parity.png`), the shell reaches the prototype
on every element it was missing: the rail carries icons, lowercase names and per-app
counters with a `REGISTER 3/12` header; title bars read `01 TERMINAL zlsh` with the
module code `M0101 4x3`; every window has a status band with its own `APP US` figure;
the raster strip carries ADVANCE / composite / IDLE WAKEUPS/H / BUDGET; the memory
ruler has coloured regions and prints its real address range.

**The knockout fires at boot, and was reported broken twice before anyone measured it.**
Sampled from `presswork-boot-parity.png`:

```
TERMINAL   header #B6B0AB x381   title ink #181411     == ZD_KNOCK / ZD_KNOCK_INK
MONITOR    header #322B27        title ink #C8C2B9     == ZD_BASE  / ZD_TEXT_2
focus bar  #E8734F x3px                                == ZD_VERM
```

It was called broken from a downscaled render on two separate occasions, hours apart,
by the same reasoning both times: at that viewing scale `#B6B0AB` against a dark
surround reads as mid-grey. It is not mid-grey. It is exactly the token.

The rule that follows, and it is worth more than the finding: **eyes answer "is the
focus obvious at a glance"; they do not answer "is this the right colour".** Both
questions are asked of the same picture, which is what makes them easy to conflate.
Sample the framebuffer for the second one, every time - the instrument is three lines
of Python and it has now been right three times against a confident wrong reading.

### What the parity pass refused to fake

- **LOAD 0.58** - dropped. No load average exists; `sched.c` keeps no run-queue history,
  and the prototype computes its figure from two mock meters.
- **MIN STEP 1.1682 / KNOCK 6.4796 / SKEW 0.25%** - these are WCAG ratios, which need
  `pow(x, 2.4)` on a linearised channel, and there are no floats in this drawing path.
  They can be printed as constants from `design.h`; they cannot be computed live.

### Hazard recorded rather than fixed

`RULER_DMA` / `RULER_DMA_END` in `kernel.zl` restate `HI_IMG` / `HI_HEAP` from
`memmap.h`. Verified equal today, enforced by nothing - zl cannot include a C header.
That is the same drift class as the window-manager reserves that said 48/72 while the
shell said 30/46/170, which is exactly how that bug survived.
