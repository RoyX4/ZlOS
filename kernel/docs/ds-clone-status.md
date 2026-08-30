# ds-clone status — measured, not remembered

Generated 2026-08-20 from the tree at this commit. Every line below is derived
by a script or quoted from a gate's own output. Where something is NOT DONE it
says so; where it is done but unproven it says that too, because "implemented"
and "visible on screen" have come apart repeatedly in this work.

Regenerate section 1 with `python3 kernel/tests/host/apps53.py` (exit 1 if any of
the 53 is missing). Regenerate section 3 with the command in that section.

---

## 1. The 53 apps

`apps53.py` resolves each entry of ds-reference.html's `APPS` / `UTILS` /
`GAME_APPS` arrays against the zl sources: the numeric id through whatever
constant names it, `reg_exists()` mirrored from apps_registry.zl's own gap list,
and whether a name / size / icon entry exists.

**Result: 53 PRESENT, 0 MISSING.** `REG_FIRST=14 REG_LAST=69 reg_count()=47`.
47 route through the catalog; the other 6 predate the registry and open from
the dock, the shell or the boot set.

```
  #   name                   kind     id    exists size  icon  route
  1   Clipboard              utility  14    yes    yes   no    catalog
  2   Colour Picker          utility  15    yes    yes   no    catalog
  3   Regex Tester           utility  16    yes    yes   no    catalog
  4   Base Converter         utility  17    yes    yes   no    catalog
  5   Text Diff              utility  18    yes    yes   no    catalog
  6   Checksum               utility  19    yes    yes   no    catalog
  7   Unit Converter         utility  20    yes    yes   no    catalog
  8   Sticky Notes           utility  21    yes    yes   no    catalog
  9   Keyboard Tester        utility  22    yes    yes   no    catalog
  10  Benchmark              utility  23    yes    yes   no    catalog
  11  Snake                  game     3     yes    yes   yes   dock/shell/boot
  12  Tetris                 game     52    yes    yes   yes   catalog
  13  Pong                   game     53    yes    yes   yes   catalog
  14  Breakout               game     54    yes    yes   yes   catalog
  15  Minesweeper            game     55    yes    yes   yes   catalog
  16  2048                   game     56    yes    yes   yes   catalog
  17  Conway's Life          game     57    yes    yes   yes   catalog
  18  Asteroids              game     58    yes    yes   yes   catalog
  19  Invaders               game     59    yes    yes   yes   catalog
  20  Tic-Tac-Toe            game     25    yes    yes   no    catalog
  21  15 Puzzle              game     61    yes    yes   yes   catalog
  22  Connect Four           game     29    yes    yes   no    catalog
  23  Reversi                game     62    yes    yes   yes   catalog
  24  Lights Out             game     28    yes    yes   no    catalog
  25  Simon                  game     63    yes    yes   yes   catalog
  26  Maze                   game     33    yes    yes   no    catalog
  27  Sokoban                game     64    yes    yes   yes   catalog
  28  Flappy                 game     65    yes    yes   yes   catalog
  29  Missile Command        game     66    yes    yes   yes   catalog
  30  Tower of Hanoi         game     27    yes    yes   no    catalog
  31  Nim                    game     26    yes    yes   no    catalog
  32  Blackjack              game     67    yes    yes   yes   catalog
  33  Frogger                game     68    yes    yes   yes   catalog
  34  Word Guess             game     24    yes    yes   no    catalog
  35  Terminal               system   0     yes    yes   yes   dock/shell/boot
  36  Files                  system   13    yes    yes   yes   dock/shell/boot
  37  System Monitor         system   1     yes    yes   yes   dock/shell/boot
  38  Text Editor            system   12    yes    yes   yes   dock/shell/boot
  39  Kernel Log             system   40    yes    yes   yes   catalog
  40  Renderer               system   41    yes    yes   yes   catalog
  41  Hex Viewer             system   42    yes    yes   yes   catalog
  42  Framebuffer            system   43    yes    yes   yes   catalog
  43  Console (tty1)         system   44    yes    yes   yes   catalog
  44  Calculator             system   32    yes    yes   no    catalog
  45  Font Atlas             system   45    yes    yes   yes   catalog
  46  Disk Usage             system   46    yes    yes   yes   catalog
  47  Services               system   47    yes    yes   yes   catalog
  48  Archive Manager        system   48    yes    yes   yes   catalog
  49  Image Viewer           system   49    yes    yes   yes   catalog
  50  Network                system   50    yes    yes   yes   catalog
  51  Clocks & Timers        system   30    yes    yes   yes   catalog
  52  System Info            system   31    yes    yes   no    catalog
  53  Settings               system   6     yes    yes   yes   dock/shell/boot

all 53 reference apps resolve to an id in the tree
```

Every line reads PRESENT. `size`/`icon` columns showing `no` mean the app takes
the registry's fallback rather than an explicit entry — all 53 window SIZES
were separately verified against the reference's own w/h and 0 of 53 now
mismatch (24 did before commit 96017a4).

---

## 2. The rest of the brief

| item | state | evidence |
|---|---|---|
| lime palette applied throughout | **DONE** | `hosttest/palette` exit 0: "38 tokens checked, 0 not found" — every colour in `design.h` occurs literally in `ds-reference.html`, every `ui_theme` role holds the token it names, `settings.c` names tokens not literals, `kernel.zl` carries no second copy. |
| radius scale | **DONE** | `design.h` `ZD_R_4..ZD_R_20`, measured with counts (11px most common, 26 uses). Windows 16px, dock 16px, island 16px, chips 11/12px. |
| blur / glow / shadow | **PARTIAL** | Glow and shadow are real: `fb_glow`, `rrblend` shadows under island and dock, the wallpaper's 3 radial glows + 2 boxed conic wedges. `backdrop-filter` blur is **NOT** reproduced — `fb.c`'s blur is a cached rectangle, not a live filter under a rounded mask. Documented as an approximation in `ds-clone.md`. |
| **zwin** | **DONE, pixel-proven** | `wmtest`: drawn left edge 306→304→303→302→301, settles at 300; negative control asserts a settled edge does not move. |
| **zpop / zov** | **NOT DONE** | `ANIM_FADE` is wired to exactly one caller (`open_menu()`, kernel.zl:4446). Curve + duration unit-tested in `easetest`; never pixel-proven, and menus are the only thing that uses it. |
| **zpress** | **NOT DONE** | `ease_press_scale()` exists and is unit-tested; **nothing calls `wm_anim(w, ANIM_PRESS)`**. Not triggered anywhere. |
| **zpulse** | **NOT DONE** | Wired to one rare state (`term_bad()`, kernel.zl:3671). Not on the dock's running-app dot, which is what the reference pulses. |
| **ztoast** | **NOT DONE** | `notify.c` contains no animation call at all. |
| **zsweep** | **NOT DONE** | Not implemented. `grep -c 'zsweep\|SWEEP' kernel/src/kernel.zl kernel/src/graphics/framebuffer/fb.c` → 0, 0. |
| Settings as a real app | **PARTIAL** | `settings.c` is 637 lines, drives 6 real sinks (accent, scale, pointer speed, accel, subpixel, animation), sized to the reference's 486x332. Its internal layout is NOT the reference's sidebar + cards. |
| VFS tree | **NOT DONE** | `grep -c 'VFS0\|vfs_' kernel/src/kernel.zl` → 0. Files reads real zlfs instead; the reference's synthetic tree is absent. |
| 3 workspaces | **PARTIAL** | `ws_cur` / `cur_ws()` / `set_ws()` exist and the island's pips render current-vs-other. Windows are **not** filtered by workspace, so switching changes the indicator and nothing else. |
| per-app internal layouts | **PARTIAL** | The 27 apps written this session use the shared toolkit against `reference-widgets.md` (toolbars, tab strips, column grids, stat strips, sidebars, status bars, mono panels). The 20 older registry apps were not re-laid-out. **None has been seen on screen.** |

---

## 3. Fidelity oracle — current numbers

```
kernel/tests/oracle/shot-zlos.py --shot status
kernel/tests/oracle/diff-regions.py --zlos out/zlos-status.png \
    --ref ../refrender/out/reference-1280x800.png
```

    dock_strip              [325, 746, 629, 45]      0.172    0.059  1.000   0.483   1.000  WRONG     hue        EXACT
    screen                  [0, 0, 1280, 800]        0.040    0.009  0.080   0.532   0.532  WRONG     structure  EXACT
    win_files               [238, 346, 672, 352]     0.025    0.010  0.000   0.489   0.489  WRONG     structure  EXACT
    win_mon                 [758, 76, 424, 376]      0.050    0.012  0.106   0.434   0.434  WRONG     structure  EXACT
    wallpaper               [912, 456, 320, 240]     0.032    0.005  0.000   0.384   0.384  off       structure  DERIVED
    win_term                [132, 60, 600, 368]      0.035    0.003  0.027   0.253   0.253  off       structure  EXACT
    topbar_strip            [0, 8, 1280, 32]         0.064    0.044  0.143   0.212   0.212  off       structure  PARTIAL
    MEAN                                             0.040    0.012  0.750   0.363   0.839

    verdicts across all 62 regions:   52 WRONG   10 off   0 close   0 match

Baseline before this work was 0.522 on `screen` with **62 WRONG**. Movement
since: `win_term` 0.646 → 0.253 (hue 0.646 → 0.027), `wallpaper` 0.668 → 0.384,
`topbar_strip` 0.415 → 0.212, `win_mon` 0.557 → 0.434.

**Read the 52 WRONG honestly.** Only three windows are open at boot, so ~50 of
the 62 regions are comparing zlOS *wallpaper* against a reference *window*.
Those cannot improve until the app is open in the shot. The numbers that mean
anything today are the three open windows, the wallpaper and the top bar.

Two anchors, both measured: antialiasing alone costs **0.019**, so nothing can
score below that; `dock_strip` reads hue **1.000** because the reference's dock
has a lime focused tile and zlOS's has none at boot.

---

## 4. Blocked on / why this keeps pausing

**Not blocked.** Nothing here risks destroying existing work.

The pauses are mine and they are a reporting habit, not a stall: after each
agent wave I integrate, verify, commit and then write a summary, which ends the
turn. Work has resumed every time and the branch has never been left dirty. The
fix is to chain the next wave before summarising rather than after.

**The largest real gap** is that six of the seven animations are implemented and
unit-tested but wired to nothing, so the desktop does not actually perform them.
That is a bigger honesty problem than a low fidelity score, because `easetest`
passing 20 assertions makes it look done. It is not done.

**Second largest:** 47 apps have still never been rendered. The catalog blocker
was fixed at `1c4a417` (APP_CATALOG and APP_FILES were both id 13 — a duplicate
that raised the Files window and reported success). Per-app shots are now
possible and have not yet been taken in bulk.
