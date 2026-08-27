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

---

## The compiler bug the depth pass exposed

The kernel built clean - 0 undefined symbols, 5.59 MB - and halted on boot with

```
kernel runtime error: builtin not available in the kernel subset
```

which is `kfatal()`: a zl call fell off the end of the runtime's builtin table.
`check-zlcalls.py` reported **"every call site resolves"** at the same moment, so
the checker and the kernel disagreed about what a resolved call is.

**Root cause: `NAMESET_MAX` in `src/backends/c/compile.c` was 1024 and the kernel
reached 1083 zl functions.** `set_add()` dropped the last 59 silently - no count, no
warning, no failure path at all. A dropped name is not a compile error: `set_has()`
simply reports it unknown, so every call to it is emitted as a dynamic `zl_calln()`,
which the runtime answers with `kfatal()`. Build succeeds, kernel halts.

The comment directly above that constant records the SAME bug at 256, and then says:

> "1024 is headroom, not a guess - the kernel is nowhere near it either way."

It was at 1083.

### What was changed, and why the cap is the smaller half

`NAMESET_MAX` is 4096 now, but raising it only buys another scale - that is what the
previous raise did. The defect is that `set_add()` had no failure path. A silent drop
inside a compiler surfacing as a runtime halt in source that never changed is the most
expensive shape a bug can take in this tree, because every instinct points at the code
that moved rather than at the tool.

It is a hard error now, naming the function it could not fit and stating the exact
consequence that used to be silent.

### Two instruments improved on the way

- **The dispatcher's fatal did not name the builtin.** It said only "builtin not
  available in the kernel subset", which costs a full build-and-boot cycle to turn
  into a symbol - and the boot is the expensive half. It names it now. The message is
  assembled at the call site rather than routed through `kfatal`'s hash, because that
  hash is a wire ID for the flight recorder and two different missing builtins must
  not log as one incident.
- **`check-zlcalls.py` verifies that a name exists in the zl sources, not that the
  backend bound it.** It is green on exactly this failure and stayed green throughout.
  Closing that gap means comparing the function count against the backend's cap, or
  scanning the generated `out.c` for `zl_calln()` of a name that is a known zl
  function - the second is the direct test and neither is written yet.

### The method that found it

Three hypotheses were killed by measurement rather than argument:

1. *Builtins conditionally compiled out.* Preprocessed `runtime_kernel.c` with the
   kernel's own flags and diffed against the source-text harvest: 759 both sides,
   zero difference.
2. *C calling zl by name.* Zero `zl_calln(` outside generated files.
3. *A zero-argument binding quirk.* `label_windows()` and `rail_reg_rows()` are
   zero-arg and bind directly.

What settled it was compiling `origin/main`'s `kernel.zl` with the same compiler and
diffing the output: **origin/main emits 2 direct `zl_fn_layout()` calls; this tree
emitted 2 dynamic `zl_calln("layout")` for the identical construct.** Same source
shape, different binding, therefore the tool - not the source.
