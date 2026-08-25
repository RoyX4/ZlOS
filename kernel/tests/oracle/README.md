# zlOS Fidelity Oracle

We are cloning a desktop mockup (`ds.html`, 1280x800) into zlOS. This directory
answers one question with a number instead of an opinion:

> **how far is what zlOS actually draws from what the mockup actually draws, in
> the part of the screen I am responsible for?**

`kernel/tests/refrender/` renders the mockup to PNG. This
directory photographs zlOS and compares the two, per region.

```
kernel/tests/refrender/out/reference-1280x800.png  <- the target
kernel/tests/oracle/out/zlos-desktop.png           <- what we ship
kernel/tests/oracle/regions.json                    <- 62 named boxes
kernel/tests/oracle/diff-regions.py                 <- four numbers per box
```

---

## The four tools

| file | what it does | needs QEMU |
|---|---|---|
| `gen-regions.py` | reads `ds.html`, writes `regions.json` | no (needs `node`) |
| `shot-zlos.py` | boots zlOS at 1280x800, writes a PNG | **yes**, ~2 min |
| `diff-regions.py` | scores zlOS against the reference, per region | no, ~2 s |
| `animate.py` | drives one interaction and asks whether pixels moved | **yes**, ~3 min |
| `zlosboot.py` | the shared boot module (not a command) | — |

`out/` is gitignored. `regions.json` is committed, so `diff-regions.py` needs
neither `node` nor `ds.html`.

### Reproducing everything in this file

```bash
cd kernel/tests/oracle
./gen-regions.py                              # regions.json, from ds.html
./diff-regions.py --selftest                  # the measures check themselves
./shot-zlos.py --shot desktop                 # a real boot; run it in the background
./diff-regions.py --zlos out/zlos-desktop.png \
                  --ref ../refrender/out/reference-1280x800.png --visible-only
./animate.py --do drag --frames 8
```

**Start the two QEMU ones in the background and go do something else** — this
box has 4 physical cores and the boot is TCG-emulated, so it takes as long as
the host load says it takes. Do not run two at once. Nothing in here fails
because a boot was slow: every wait polls the serial log for a marker the guest
printed, never a wall clock.

---

## Why QEMU, and not `kernel/tests/host/wmshot`

`wmshot` renders the compositor from Linux userspace in milliseconds with no
boot at all, which makes it the obvious choice right up until you read it.

**Its apps are four hand-written C functions inside `wmshot.c`** — `APP_SHELL`,
`APP_MONITOR`, `APP_ABOUT`, `APP_FILES` — and its wallpaper, header and dock
are its own `desk_draw()`, about forty lines of C that exist in that file and
nowhere else. The real desktop and its twenty-odd apps are in `kernel/src/kernel.zl`.

So `wmshot` renders **a** desktop, not **the** desktop. Scoring it against the
mockup would measure a test fixture nobody ships and publish a fidelity number
for pixels no user will ever see. It remains the right tool for compositor
geometry — `wmtest` asserts, `wmshot` shows — and it is the wrong tool for
fidelity. The difference is not one of speed.

---

## What resolution zlOS actually boots at

**1920x1200.** Measured, on this branch, 2026-08-20:

```
$ ./shot-zlos.py --how toggle --shot x
booted at 1920x1200
```

You cannot get 1280x800 by asking the bootloader. GRUB's default list
(`mkiso.sh:32`) starts at `1280x720`, and then `kernel.zl:4541` runs a modeset
ladder over the top of whatever it was handed:

```zl
if px_w() > 0 {
    if px_w() < 1900 {
        if set_res(1920, 1200) == 0 { ... 1920x1080 ... 1600x900 ... 1280x800 }
    }
}
```

Anything under 1900 wide is immediately re-modeset up to 1920x1200 by zlOS's
own driver, so `ZLOS_GFXMODE=1280x800` changes nothing.

### The two ways down to 1280x800, and why the default is the second one

**`--how toggle`** types the shell's `mode` command (`term.c:181` → code 110,
`kernel.zl:1688`), which toggles between exactly two modes. The framebuffer
really does change, and the kernel says so:

```
fb: 1280x800x32 cell 8x16 ui 1x, back ON  (4000 KiB/mode)
now 1280 x 800, pitch 5120
mode set by zlOS, no BIOS involved
```

**and the desktop does not follow it.** The screenshot from that path has the
windows still at their 1920x1200 positions — System Monitor half off the right
edge, Browser hanging off the bottom — the wallpaper repainted as flat black,
and **no dock on screen at all**. `layout()` recomputes the variables the boot
placement reads; it does not move windows the compositor has already opened.
That is a real zlOS bug, found by this harness, and it is not fixed here.

**`--how src` (the default)** builds a variant `kernel.zl` whose ladder is
replaced by `set_res(1280, 800)` and boots that with `ZLOS_SRC` — the mechanism
`mkiso.sh` documents and `probe-drag.py` already uses for its 2560x1440 case.
`layout()` then runs once, at 1280x800, before a single window opens. The
patched source is written to `out/kernel-1280x800.zl`; the tracked `kernel.zl`
is never touched, and the substitution is asserted to match exactly once so a
re-indent cannot silently turn it into a no-op.

**Do not pick between these on the score.** Measured, both against the real
reference: mode-toggle mean score **0.719**, variant-source mean **0.768** — the
*broken* frame scores *better*, because its flat-black wallpaper happens to sit
closer to the reference's dark background and it has fewer saturated pixels for
`hue` to disagree about. A lower number on a frame that is laid out for the
wrong resolution is measuring the wrong thing more precisely. This is the
clearest warning in this file about reading an aggregate mean as a ranking.

`--width`/`--height` are parameters throughout. The day that ladder learns a new
mode, nothing here needs an edit.

---

## `regions.json` — 62 boxes, and where each number came from

Generated by `gen-regions.py` from `ds.html`. Every region carries a
`derivation` string, and `diff-regions.py` prints its first sentence, because
the numbers are not all worth the same:

| class | meaning | examples |
|---|---|---|
| **EXACT** | out of the mockup's source | every app window; `dock_strip`; `dock_band`; `topbar_band` |
| **DERIVED** | arithmetic on CSS declarations parsed out of the file | desktop icons; `wallpaper` |
| **PARTIAL** | part of it is not derivable, and it says which part | `topbar_strip` |

Window rects come from **running the mockup's own `APPS`/`UTILS`/`GAME_APPS`
`.map()` in node**, not from reimplementing the index arithmetic in Python — a
reimplementation is transcription wearing a function's clothes, it agrees today
and drifts silently later.

`dock_strip` is worth reading as the worked example of DERIVED-but-exact:
15 pinned tiles of 33px + a 1px separator with 2px side margins + one 33px
"show applications" button, 16 gaps of 5px, padding 5/7px and a 1px border
(content-box, so padding and border add) → **629 x 45**, centred, `bottom:9px` →
**[325, 746, 629, 45]**. Every one of those numbers is read out of the style
attribute; the dock contains no text, so nothing had to be measured. Compare it
against the rendered reference and it lands on the dock exactly.

`topbar_strip` is PARTIAL and honest about it: `top:8px` and `height:32px` are
exact, but four of the island's ten flex children are text (Activities, the
clock, the tray, the bell) and text width needs a font metric the source does
not contain — so the region is the **full-width strip** at the island's exact
band. It therefore includes wallpaper either side, which dilutes `colour` and
`palette` and does not dilute `structure`.

Regeneration:

```bash
./gen-regions.py --html ~/Downloads/ds.html
```

It refuses rather than guesses: a style anchor that matches zero or two
elements is a fatal error, not a fallback.

`regions.json` records the SHA-256 of the `ds.html` it was built from, and
`diff-regions.py --json` echoes it as `regions_sha`. Verified 2026-08-20: that
hash is `1302282882…` and it is byte-identical to `kernel/tests/refrender/ds.html`,
the copy the reference PNGs were rendered from. If the two ever diverge, the
region map and the reference are describing different mockups and every number
below is meaningless — check it before believing a regression.

---

## What the four numbers mean

All four are **0.0 = identical, 1.0 = maximally different**. They are **not on
a common scale** and pretending they are is the easiest way to make this lie.

| measure | answers | scales with area? |
|---|---|---|
| `colour` | is this region the right brightness and hue overall | yes |
| `palette` | do the same colours appear, in the same proportion (position-blind) | yes |
| `hue` | is the accent the right hue, or missing | **no** |
| `structure` | is the furniture in the same place (brightness- and contrast-blind) | yes |
| `score` | the worst of the four | — |

`worst_measure` tells you which one produced `score`. Read them together: a
`score` of 0.5 whose `worst_measure` is `structure` is a layout problem; the
same 0.5 on `hue` is a colour problem, and the fix is nothing alike.

`hue` prints a reason, because `1.000` means two different things:

```
dock_strip   ... 1.000  WRONG  hue   -> the reference has an accent, zlOS has none
win_term     ... 0.646  WRONG  hue   -> zlOS hue 192 deg vs reference 75 deg
```

The first is a missing element. The second is a wrong colour. Both are real,
and only one of them is a palette bug.

### The measures are checked, not asserted

```bash
$ ./diff-regions.py --selftest
case                         colour  palette      hue   struct   expected to fire
----------------------------------------------------------------------------------
identical                     0.000    0.000    0.000    0.000   none
accent hue swapped            0.017    0.021    0.561    0.018   hue
toolbar moved 140px           0.049    0.000    0.000    0.267   structure
antialias jitter only         0.019    0.001    0.000    0.004   none
----------------------------------------------------------------------------------
selftest: ok - each measure fires on its own failure and not on antialiasing
```

Four synthetic pairs whose right answer is known by construction, including an
**antialias-only control** that all four must ignore — which is the whole
argument for not using a single per-pixel RMS number. It runs in under a second
and needs no QEMU and no reference render.

It earned its keep on the first draft, twice:

- the original gradient-correlation `structure` scored **0.002** on a toolbar
  that had moved 140 pixels. A solid block has gradient only at its border, so
  in any region that also contains text — i.e. every region here — dense glyph
  edges dominate the gradient map and a block sliding across it barely
  registers. It is now the worse of two standardised coarse maps, luminance and
  gradient: **0.267** on the same case.
- an accent swapped lime → cyan moved `palette` by **0.021**. Real, and
  indistinguishable from noise at a glance, because a window's accent is about
  4% of its area and every area-scaled measure is diluted by that. `hue` was
  added, is area-independent, and scores that swap **0.561**.

---

## The baseline: what unmodified zlOS scores today

Measured 2026-08-20 on `claude/compassionate-curie-a0599c`, `--how src`, against
`kernel/refrender/out/reference-1280x800.png`.

```
region                  rect                    colour  palette    hue  struct   score  verdict   worst
dock_strip              [325, 746, 629, 45]      0.115    0.058  1.000   0.494   1.000  WRONG     hue
  -> the reference has an accent, zlOS has none
dock_band               [0, 738, 1280, 62]       0.093    0.009  1.000   0.486   1.000  WRONG     hue
  -> the reference has an accent, zlOS has none
deskicon_readme         [22, 16, 78, 75]         0.108    0.032  1.000   0.434   1.000  WRONG     hue
deskicon_notes          [22, 104, 78, 75]        0.080    0.046  1.000   0.593   1.000  WRONG     hue
deskicon_kernel_zl      [22, 192, 78, 75]        0.088    0.029  1.000   0.462   1.000  WRONG     hue
  -> zlOS has an accent, the reference has none  (zlOS draws wallpaper there)
win_files               [238, 346, 672, 352]     0.048    0.008  1.000   0.457   1.000  WRONG     hue
wallpaper               [912, 456, 320, 240]     0.033    0.003  0.000   0.668   0.668  WRONG     structure
win_term                [132, 60, 600, 368]      0.043    0.011  0.646   0.401   0.646  WRONG     hue
  -> zlOS hue 192 deg vs reference 75 deg
win_mon                 [758, 76, 424, 376]      0.058    0.012  0.104   0.557   0.557  WRONG     structure
screen                  [0, 0, 1280, 800]        0.055    0.007  0.413   0.522   0.522  WRONG     structure
topbar_strip            [0, 8, 1280, 32]         0.061    0.008  0.117   0.415   0.415  WRONG     structure
topbar_band             [0, 0, 1280, 48]         0.055    0.005  0.000   0.413   0.413  WRONG     structure
------------------------------------------------------------------------------------------------------
MEAN                                             0.070    0.019  0.607   0.492   0.768
```

Read from that, before anyone starts:

- **`colour` is already low everywhere (0.033–0.115) and it means almost
  nothing.** Both renders are dark UIs on dark backgrounds; mean absolute error
  cannot tell them apart. Do not use `colour` as a done-condition.
- **`palette` is low everywhere too (0.003–0.058)** for the same reason, plus
  the area dilution above. It is a supporting number, not a gate.
- **`structure` is the real signal and it is 0.4–0.67 across the board.** zlOS's
  furniture is not where the mockup's furniture is. That is the bulk of the
  work.
- **`hue` at 1.000 on the dock** is the single most actionable line in the
  table: zlOS's dock is not on screen at 1280x800 at all — the Browser window is
  laid out overlapping it — while the reference has a lime-accented dock at
  [325, 746, 629, 45].
- **`hue` at 1.000 on the three desk icons, in the other direction**: zlOS
  paints wallpaper there and the wallpaper is saturated, while the mockup's
  icon tiles are not. zlOS has no desktop icons.

### What "good" looks like

There is no measured "this region is done" number yet, because nothing has been
cloned yet — every visible region is currently `WRONG`. Set your own gate from
the bands the tool prints, and from the two anchors that *are* measured:

| band | `score` | what it means here |
|---|---|---|
| `match` | < 0.10 | nothing on this corpus is here yet |
| `close` | 0.10 – 0.20 | plausible target for a cloned region |
| `off` | 0.20 – 0.40 | recognisably the same thing, wrong in detail |
| `WRONG` | > 0.40 | **every visible region today** |

Two measured anchors to calibrate against:

- **antialiasing alone costs 0.019** (selftest control). A cloned region cannot
  score better than that, and anything under ~0.05 is at the noise floor.
- **the whole-screen baseline is 0.522**, `worst_measure structure`. If your
  region is not below that you have not moved.

A reasonable done-condition for one region, and the one to argue with rather
than adopt blindly:

```bash
./diff-regions.py --zlos out/zlos-desktop.png \
  --ref ../refrender/out/reference-1280x800.png \
  --only win_term --json --fail-over 0.20
```

Exit 2 means over the bar. Use `--json`; the table is for humans.

---

## Per-app shots

`shot-zlos.py --app <name>` opens an app before the picture and **proves it
opened** — the compositor's own `wm: win N title ...` report has to gain a line,
because a screenshot cannot tell "the app opened" from "the app refused and the
desktop looks the same".

```bash
./shot-zlos.py --list-apps          # what is reachable, and how
./shot-zlos.py --app files --shot files
./shot-zlos.py --app Calculator --shot calc
```

Two routes, and they are genuinely different mechanisms:

- **shell word** — `files`, `edit`, `snake`, `paint`, `cube`, `anim`, `mouse`.
  These are the seven command codes `kernel.zl`'s `open_app()` handles. Typed,
  echoed, verified; no pointer, no geometry, nothing to land in the wrong place.
- **catalog** — the other twenty apps exist only behind Start → All
  Applications → a tile, so reaching them is three real clicks. The tile
  arithmetic is done in the catalog window's **own reported client rect**, so a
  catalog that opened somewhere unexpected fails loudly instead of silently
  clicking the wallpaper.

Both tables are parsed out of `apps_registry.zl` at run time, so adding an app
there makes it available here with no edit, and renaming one breaks this loudly.

The reference side names its per-app renders `app-<id>.png` (`app-term.png`,
`app-g_snake.png`, `app-u_clip.png`), not `reference-<id>.png`. `--ref` takes a
path; pass whichever exists.

---

## `animate.py` — a green build proves nothing about motion

A previous run of this desktop had **9 of 14 event handlers updating their state
and never calling `wm_dmg(win)`**. It compiled clean. Every gate was green.
Nothing repainted. From outside, "the model changed and the screen did not" and
"nothing happened" are the same picture.

`animate.py` is the difference engine for that. It drives one interaction, takes
N frames, and reports per consecutive pair: how many pixels changed, their
bounding box, which named regions they fall in, and an ASCII map of where.

```bash
./animate.py --do drag              # drag the topmost window by its title bar
./animate.py --do menu              # open the start menu, then hover down it
./animate.py --do hover-dock        # sweep the pointer along the dock
./animate.py --do type              # type at the shell
./animate.py --do app --app anim    # the positive control: a demo that animates
./animate.py --do idle              # the negative control: nothing is driven
```

Read `--do idle` first. It is the control: anything that changes there is the
guest moving on its own, and every other interaction should be read against it
rather than against zero.

Two red verdicts:

- **NOTHING MOVED** — every frame byte-identical to the one before. If the
  interaction should change the screen, that is the `wm_dmg()` failure.
- **FULL REPAINT EVERY FRAME** — over 90% of the screen changed on every pair.
  Correct output, wrong cost: damage tracking has stopped narrowing anything.

Frame comparison is **exact inequality, not a threshold**. These are two dumps
of the same framebuffer, not two photographs — there is no sensor noise to
filter, and a threshold would hide precisely the one-shade hover highlight this
tool exists to find.

The grab point for `--do drag` is the compositor's own reported title rect,
never a constant: the literal `2110,120` default in `probe-drag.py` was off the
right edge of a 1920-wide screen, the press landed on nothing, and the gate
reported "dragging is a no-op" for a drag that worked perfectly.

---

## Known limits — read before believing a number

1. **`topbar_strip` is a full-width strip, not the island.** Four of the
   island's children are text and text width is not in the source. `colour` and
   `palette` on that region are diluted by wallpaper; `structure` is not.
2. **The measures are not on a common scale.** `hue` is area-independent and the
   other three are not. Always read `worst_measure`.
3. **The aggregate MEAN is not a ranking.** Demonstrated above: a visibly broken
   frame scored *better* on the mean than a correct one.
4. **`--how src` boots a patched `kernel.zl`.** It is one substitution, asserted
   to match exactly once, and it changes only which mode the kernel asks for at
   boot. It is not the byte-identical shipped kernel, and it is the only way to
   get a correctly laid-out 1280x800 desktop today.
5. **The default view is workspace 1.** `regions.json` marks 12 regions
   `visible` and 50 not; `--visible-only` is what you almost always want for the
   boot desktop.
6. **`gen-regions.py` needs `node`.** `regions.json` is committed, so nothing
   downstream does.
