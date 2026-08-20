# Cloning ds-reference.html into zlOS

The brief: reproduce a desktop mockup — all 53 of its apps and its entire look
— inside zlOS, exactly rather than "inspired by". This page is where that work
is described. Read it before touching the desktop.

## The reference, and the two things wrong with it

`docs/design/ds-reference.html` (300 KB, 4338 lines, sha256 `13022828…`) is
vendored into this repo on purpose: gates read it, and a path in `~/Downloads`
is not a dependency a build can have.

Two facts about it change what "clone this" can possibly mean, and both were
established by measurement rather than assumed:

1. **It cannot render on its own.** It loads a template runtime from
   `<script src="./support.js">` and that file was never delivered.
   `kernel/refrender/support.js` is a reimplementation of that runtime — a
   React-based `x-dc` shim supporting `{{ }}` binding, `<sc-for>`, `<sc-if>`,
   camelCase style objects, `style-hover`, refs and handlers. With it the
   reference renders, deterministically (byte-identical PNGs across runs).
2. **Seven more of its modules are missing too** (`zl-fs`, `zl-tar`,
   `zl-games`, `zl-wasm`, `zl-raster`, `zl-vt`, `zl-gl`). They are loaded in
   `try`/`catch`, so windows, toolbars and status bars render fully — but every
   `<canvas>` is **blank**. That means **all 24 games, plus Renderer,
   Framebuffer, Console (tty1), Font Atlas and Image Viewer, have no artwork in
   the reference at all.** For those, the only thing that can be cloned is the
   *shell*; the content has to be written from the real rules of the thing. Any
   claim of "pixel parity" on a game is therefore false by construction.

## The oracle: how "exact same" became a number

"Exact same" is unfalsifiable as an instruction — fifty agents produce fifty
readings of it. `kernel/oracle/` replaces it with a measurement.

```
kernel/refrender/  renders the reference to PNG at 1280x800   (no QEMU)
kernel/oracle/     boots zlOS at 1280x800 and screenshots it  (QEMU, ~2 min)
kernel/oracle/diff-regions.py   scores the two, per region
```

`regions.json` holds 62 named boxes — one per app window at the reference's own
coordinates, one per chrome element. Four complementary measures per region,
because no single one works:

| measure | what it catches | what it cannot |
|---|---|---|
| `colour` | gross fill errors | almost nothing — both are dark UIs, it is 0.03–0.12 everywhere and is **not** a done-condition |
| `palette` | which colours appear, and in what proportion | diluted by area |
| `hue` | "the accent is cyan, not lime" | nothing about position |
| `structure` | "the toolbar is in the wrong place" | colour |

Two calibration anchors, both measured: **antialiasing alone costs 0.019**, so
nothing can score below that; and the **whole-screen baseline was 0.522** before
any of this work, `worst_measure structure`.

`animate.py` exists for a separate reason: a green build proves nothing about
motion. A previous run had 9 of 14 event handlers updating state and never
calling `wm_dmg(win)` — it compiled, passed every gate, and repainted nothing.

## One palette, and the chain that proves it

`kernel/design.h` is the only file in the tree where a colour literal may
appear. Everything else names a token or calls `ui_color()` with a role.

`kernel/hosttest/palette.c` enforces a three-link chain:

```
ds-reference.html  ->  design.h    every token occurs LITERALLY in the artifact
design.h           ->  ui.c        every role holds the token it names
design.h           ->  settings.c  the accent list names tokens, entry 0 is ZD_ACCENT
```

The first link is the one that matters and the one that did not exist before:
the gate used to compare ui.c against a mockup that had itself been transcribed
*from* kernel.zl, so "agree with the reference" and "agree with ourselves" were
the same statement. Now, picking a colour by eye and writing a plausible comment
over it is a build failure.

The palette is **lime-on-grey**: an eight-step surface ladder `#07080a … #474b50`,
a seven-step text ramp `#eef0f2 … #5c6167`, accent `#b8e838`, and
`OK #a9e34b / BAD #ff6a50 / WARN #f5b93c` quoted from the reference's own line
3046. The previous blue-slate cyan `#60D2EB` is retired. **This was a decision,
not a discovery** — the reference wins on colour — and reversing it means
editing `design.h` and nothing else.

## Motion: seven curves, not one smoothstep

`kernel/ease.c` implements one fixed-point cubic-bezier evaluator, because CSS's
named timing functions are themselves cubic-beziers. So the reference's seven
animations are five constants rather than five hand-tuned polynomials:

| name | curve | duration | used by |
|---|---|---|---|
| `zwin` | `cubic-bezier(.2,.85,.3,1)` | 200 ms | window open |
| `zpop` | ease-out | 100 ms | menus, chips, tiles |
| `zpress` | ease | 250 ms | button press |
| `zpulse` | ease-in-out | 1000 ms | attention |
| `zov` | ease-out | 160 ms | overlays, modals |
| `ztoast` | ease-out | 160 ms | toasts |
| `zsweep` | linear | 7000 ms | wallpaper sweep |

**The number that matters**: a quarter of the way through a window open, the
reference's curve is at 740/1000 and the smoothstep it replaced is at 155.
`hosttest/easetest.c` prints both. That 4.8× difference in early travel is most
of why the two desktops felt unalike, and no screenshot can show it.

Durations live in `ease.h` in **milliseconds**, converted to ticks in exactly one
place, so a change to the PIT rate cannot silently change how the desktop feels.

## What is approximated, and why

Recorded here rather than claimed as parity.

- **`backdrop-filter: blur(18/20/22px)`** is not reproduced. `fb.c`'s blur is a
  cache of a rectangle, not a live filter under a rounded mask. Behind the top
  island and the dock sits wallpaper — a smooth gradient with no high-frequency
  detail — so the 82% tint does nearly all the work. **This claim is about this
  wallpaper**: over a busy window the same shortcut would look obviously wrong.
- **The eight font sizes** (9, 9.5, 10, 10.5, 11, 11.5, 12, 12.5 px) are
  collapsed onto `fb.c`'s three atlases, ±1.5 px of type. Shipping eight bitmap
  atlases is arithmetically impossible against the kernel budget; runtime glyph
  scaling from one high-resolution atlas is the intended fix and is **not done**.
  This is the largest single divergence and it is visible on every screen at once.
- **`letter-spacing`** has no equivalent — `fb.c` has no tracking.
- **The clock reads `22:00`, the reference reads `Tue 22:00`.** zl's kernel
  subset has no runtime string type and no concatenation, so a weekday name
  cannot be joined to a time.
- **The 26 pill-button variants** are collapsed to three sizes plus `mono` and
  `bold` flags, per `reference-widgets.md` §13.1's own recommendation. The
  reference does not collapse cleanly; a port differs by 1–4 px in places.

## Where the reference is wrong, and is deliberately not copied

`kernel/docs/reference-widgets.md` §20 indexes 18 internal inconsistencies. The
ones that changed a decision:

- **The CPU sparkline clips.** `66 - v/100*66*1.9` puts anything above 52.63%
  outside its own viewBox. `uitest` asserts the reference's gain *would* clip.
- **Ink-on-accent is written four ways**, three of them hard-coding `#fff` on a
  light lime — a contrast failure. `ui_ink_on()` computes it from WCAG
  luminance with no floating point.
- **`pcolGrid` is missing the scrollbar gutter** its two siblings have, so the
  reference's own System Monitor headers drift from its rows when scrolled. One
  track table now serves header and row, so that is structurally impossible.
- **There is no hover and no focus ring anywhere in 4338 lines** — five hover
  rules total, `:focus` never appears, `cursor:default` 75 times against
  `cursor:pointer` 6. Its interaction language is *selection*, not pointer
  state. Any hover or focus treatment in zlOS is therefore **new design** and is
  marked as such where it appears.

## Layout of the work

| path | what |
|---|---|
| `kernel/design.h` | every design token, with the evidence for each |
| `kernel/ease.c` + `kernel/ease.h` | the motion curves |
| `kernel/ui.c`, `kernel/uikit.c` | the widget toolkit, ~82 functions |
| `kernel/docs/reference-widgets.md` | 68 widget records, 178 line citations |
| `kernel/refrender/` | the `x-dc` shim + reference renderer |
| `kernel/oracle/` | the fidelity measurement |
| `kernel/apps_sys2.zl`, `kernel/apps_sys3.zl`, `kernel/apps_games3.zl`, `kernel/apps_games4.zl` | id-disjoint app slices |

**The slice rule matters.** The registry dispatches through seven if-chains on
`id`. Several agents editing all seven in one working tree clobber each other,
and a duplicate id is worse than a merge conflict: `APP_CATALOG` is a dispatch
*floor*, so `id >= APP_CATALOG` routes into the registry and two apps sharing an
id compile clean and misroute at runtime. Each slice owns one file and one
disjoint range; the registry gains one line per slice per chain.

## Gates

```bash
cd kernel && ./build.sh                       # 0 undefined symbols
cd kernel/hosttest && ./build.sh
./palette ./uitest ./wmtest ./wmtest_feel ./easetest ./settingstest
./kernel/check-memmap.sh
cd kernel/oracle && ./diff-regions.py --zlos out/zlos-desktop.png \
    --ref ../refrender/out/reference-1280x800.png
```

`toasttest` fails 6 assertions. **Confirmed pre-existing** by building it
against `wm.c` from `117e53b`, before any of this work — same 6. It came in with
the desktop merge and is tracked separately.
