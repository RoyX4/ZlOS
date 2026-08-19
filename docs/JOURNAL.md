# Journal

What happened, and what the tree looked like when it happened. Appended
automatically by `tools/journal.sh` from the post-commit hook — newest first.

Git stores the diff; this stores the context around it: outstanding hazard
counts, whether the docs still agreed with the code, which files moved. That is
the part that makes a change legible later, and the part nobody writes down.

## 2026-08-20 — fix(apps): the catalog could not open, and could not draw - two silent faults (1c4a417)

`claude/compassionate-curie-a0599c` · 11 files changed, 422 insertions(+), 50 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>11 file(s)</summary>

- `kernel/apps_games3.zl`
- `kernel/apps_games4.zl`
- `kernel/apps_registry.zl`
- `kernel/apps_sys2.zl`
- `kernel/apps_sys3.zl`
- `kernel/check-appids.py`
- `kernel/check-zlcalls.py`
- `kernel/input.c`
- `kernel/kernel.zl`
- `kernel/oracle/zlosboot.py`
- `kernel/xhci.c`

</details>

## 2026-08-20 — test(wm): prove the pixels MOVE - and fix the stale-binary hole that hid it (468a657)

`claude/compassionate-curie-a0599c` · 8 files changed, 96 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>8 file(s)</summary>

- `kernel/hosttest/inputtest.c`
- `kernel/hosttest/inputtest_feel.c`
- `kernel/hosttest/inputtest_hid.c`
- `kernel/hosttest/toasttest.c`
- `kernel/hosttest/wmbench.c`
- `kernel/hosttest/wmshot.c`
- `kernel/hosttest/wmtest.c`
- `kernel/hosttest/wmtest_feel.c`

</details>

## 2026-08-20 — feat(apps): every window is the reference's own size, and a check that says so (96017a4)

`claude/compassionate-curie-a0599c` · 3 files changed, 220 insertions(+), 43 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/apps_registry.zl`
- `kernel/hosttest/apps53.py`
- `kernel/kernel.zl`

</details>

## 2026-08-20 — feat(shell): `apps` opens the catalog - and the catalog still does not open (6f6f736)

`claude/compassionate-curie-a0599c` · 3 files changed, 27 insertions(+), 12 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/kernel.zl`
- `kernel/oracle/zlosboot.py`
- `kernel/term.c`

</details>

## 2026-08-20 — fix(oracle): aim at the dock's grid button, and say plainly that it still fails (bc9b5c1)

`claude/compassionate-curie-a0599c` · 1 file changed, 22 insertions(+), 7 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/oracle/zlosboot.py`

</details>

## 2026-08-20 — fix(chrome): the app catalog was unreachable by pointer (35640e5)

`claude/compassionate-curie-a0599c` · 1 file changed, 13 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/kernel.zl`

</details>

## 2026-08-20 — fix(oracle): it can now open the 27 apps the slices added, not just 20 (ca6ea67)

`claude/compassionate-curie-a0599c` · 1 file changed, 25 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/oracle/zlosboot.py`

</details>

## 2026-08-20 — fix(apps): catalog tiles for the game slices, so ~30 apps stop reading "??" (63c9207)

`claude/compassionate-curie-a0599c` · 3 files changed, 36 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/apps_games3.zl`
- `kernel/apps_games4.zl`
- `kernel/apps_registry.zl`

</details>

## 2026-08-20 — feat(apps): the last of the 53 - Text Editor, named and sized as the reference (4a5317b)

`claude/compassionate-curie-a0599c` · 1 file changed, 3 insertions(+), 3 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/kernel.zl`

</details>

## 2026-08-20 — refactor(games): three game colours became theme roles, so they stop being literals (ba02b01)

`claude/compassionate-curie-a0599c` · 1 file changed, 16 insertions(+), 9 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_games4.zl`

</details>

## 2026-08-20 — fix(oracle): the catalog index is dense, so stop assuming id - REG_FIRST (278d58d)

`claude/compassionate-curie-a0599c` · 1 file changed, 26 insertions(+), 5 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/oracle/zlosboot.py`

</details>

## 2026-08-20 — fix(flappy): the position is the integral, not a rounded copy of one (16db5a7)

`claude/compassionate-curie-a0599c` · 1 file changed, 30 insertions(+), 18 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_games4.zl`

</details>

## 2026-08-20 — feat(design): nine more colour roles, because twenty could not say it (634ef98)

`claude/compassionate-curie-a0599c` · 3 files changed, 41 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/kernel.zl`
- `kernel/ui.c`
- `kernel/ui.h`

</details>

## 2026-08-20 — fix(sys2): the stat strip's hairline is DP(1), not 1 (4eb2cd5)

`claude/compassionate-curie-a0599c` · 1 file changed, 4 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_sys2.zl`

</details>

## 2026-08-20 — fix(apps): the catalog lists every app, and no blank tiles (afa8202)

`claude/compassionate-curie-a0599c` · 1 file changed, 52 insertions(+), 6 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_registry.zl`

</details>

## 2026-08-20 — test(games): the rules of apps_games4.zl, asserted against the shipping source (7ac9e17)

`claude/compassionate-curie-a0599c` · 3 files changed, 774 insertions(+), 46 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/apps_games4.zl`
- `kernel/hosttest/games4_rules.zl`
- `kernel/hosttest/zladdrs.sh`

</details>

## 2026-08-20 — fix(sys3): one sys3_new, at the reference's own position, and no desk_ox (1d67cf2)

`claude/compassionate-curie-a0599c` · 1 file changed, 36 insertions(+), 9 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_sys3.zl`

</details>

## 2026-08-20 — feat(apps): sys2_new opens the six where the reference puts them (dd8a252)

`claude/compassionate-curie-a0599c` · 1 file changed, 47 insertions(+), 9 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_sys2.zl`

</details>

## 2026-08-20 — fix(apps): slice ticks were not dispatched either - animated apps never advance (2c46de2)

`claude/compassionate-curie-a0599c` · 1 file changed, 6 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_registry.zl`

</details>

## 2026-08-20 — fix(games): games4_new resets state, and there is only one of it (1fd856c)

`claude/compassionate-curie-a0599c` · 1 file changed, 35 insertions(+), 10 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_games4.zl`

</details>

## 2026-08-20 — fix(games3): one games3_new, not two - the build could not link with both (b11008d)

`claude/compassionate-curie-a0599c` · 2 files changed, 1332 insertions(+), 30 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/apps_games3.zl`
- `kernel/apps_sys3.zl`

</details>

## 2026-08-20 — feat(apps): the six reference system apps - ids 40..45, off the widget catalogue (b48cc82)

`claude/compassionate-curie-a0599c` · 1 file changed, 18 insertions(+), 5 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `freestanding/runtime_kernel.c`

</details>

## 2026-08-20 — feat(games): Sokoban, Flappy, Missile Command, Blackjack, Frogger (00cfe55)

`claude/compassionate-curie-a0599c` · 1 file changed, 323 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_games4.zl`

</details>

## 2026-08-20 — feat(apps): games3_new, and a mouse move that changes nothing repaints nothing (d1b7604)

`claude/compassionate-curie-a0599c` · 1 file changed, 1 insertion(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_games3.zl`

</details>

## 2026-08-20 — fix(apps): slice apps could not be opened at all - reg_new had no path to them (56d5679)

`claude/compassionate-curie-a0599c` · 4 files changed, 1893 insertions(+), 17 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>4 file(s)</summary>

- `kernel/apps_games3.zl`
- `kernel/apps_games4.zl`
- `kernel/apps_registry.zl`
- `kernel/apps_sys2.zl`

</details>

## 2026-08-20 — feat(apps): eight games in slice 3 - Tetris, Pong, Breakout, Minesweeper, 2048, Life, Asteroids, Invaders (980b1b4)

`claude/compassionate-curie-a0599c` · 1 file changed, 2198 insertions(+), 14 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_games3.zl`

</details>

## 2026-08-20 — feat(games): Simon - playback as a clock, not a loop (b67a978)

`claude/compassionate-curie-a0599c` · 1 file changed, 194 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_games4.zl`

</details>

## 2026-08-20 — feat(games): Reversi - eight-ray legality, the flip, and passing (42f4fc1)

`claude/compassionate-curie-a0599c` · 1 file changed, 286 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_games4.zl`

</details>

## 2026-08-20 — feat(games): 15 Puzzle - a parity-correct shuffle, and the reference's game shell (8cd1bb5)

`claude/compassionate-curie-a0599c` · 1 file changed, 353 insertions(+), 13 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_games4.zl`

</details>

## 2026-08-20 — fix(apps): the slice dispatch was dead code in all seven chains (6c7e03a)

`claude/compassionate-curie-a0599c` · 1 file changed, 9 insertions(+), 9 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/apps_registry.zl`

</details>

## 2026-08-20 — docs: real paths in the clone doc's file table (14632af)

`claude/compassionate-curie-a0599c` · 1 file changed, 2 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `docs/design/ds-clone.md`

</details>

## 2026-08-20 — docs: one page for the clone, linked from HANDOFF (f675fe6)

`claude/compassionate-curie-a0599c` · 2 files changed, 201 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/design/ds-clone.md`
- `kernel/HANDOFF.md`

</details>

## 2026-08-20 — fix(design): box the wallpaper's conic wedges - they were lighting the screen (cd57d0d)

`claude/compassionate-curie-a0599c` · 3 files changed, 172 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `freestanding/runtime_kernel.c`
- `kernel/console.c`
- `kernel/kernel.zl`

</details>

## 2026-08-20 — feat(apps): four id-disjoint slices, so apps can be written in parallel (af89618)

`claude/compassionate-curie-a0599c` · 6 files changed, 164 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>6 file(s)</summary>

- `kernel/apps_games3.zl`
- `kernel/apps_games4.zl`
- `kernel/apps_registry.zl`
- `kernel/apps_sys2.zl`
- `kernel/apps_sys3.zl`
- `kernel/kernel.zl`

</details>

## 2026-08-20 — feat(chrome): boot to the reference's own three-window composition (472ef9d)

`claude/compassionate-curie-a0599c` · 2 files changed, 47 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/kernel.zl`
- `kernel/oracle/shot-zlos.py`

</details>

## 2026-08-20 — fix(ui): the toolkit refuses an out-of-order call instead of faulting (cf54612)

`claude/compassionate-curie-a0599c` · 2 files changed, 78 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/hosttest/uitest.c`
- `kernel/uikit.c`

</details>

## 2026-08-20 — feat(design): the wallpaper stops being teal - the largest hue error on screen (6b30c14)

`claude/compassionate-curie-a0599c` · 3 files changed, 95 insertions(+), 26 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/hosttest/palette.c`
- `kernel/kernel.zl`
- `kernel/ui.c`

</details>

## 2026-08-20 — feat(ui): the shared widget toolkit - 55 widgets out of reference-widgets.md (5f0c1fc)

`claude/compassionate-curie-a0599c` · 10 files changed, 2734 insertions(+), 15 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>10 file(s)</summary>

- `kernel/.gitignore`
- `kernel/SOURCES`
- `kernel/design.h`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/palette.c`
- `kernel/hosttest/settingstest.c`
- `kernel/hosttest/uitest.c`
- `kernel/ui.c`
- `kernel/ui.h`
- `kernel/uikit.c`

</details>

## 2026-08-20 — feat(chrome): the dock is the reference's pill, and tiles stop being tinted (8a4fbcb)

`claude/compassionate-curie-a0599c` · 1 file changed, 89 insertions(+), 53 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/kernel.zl`

</details>

## 2026-08-20 — feat(chrome): the top bar becomes the reference's floating island (4f4adaf)

`claude/compassionate-curie-a0599c` · 1 file changed, 164 insertions(+), 16 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/kernel.zl`

</details>

## 2026-08-20 — feat(motion): the reference's seven curves, not one smoothstep (7a1010b)

`claude/compassionate-curie-a0599c` · 12 files changed, 58 insertions(+), 8 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>12 file(s)</summary>

- `kernel/.gitignore`
- `kernel/hosttest/cryptotest`
- `kernel/hosttest/easetest`
- `kernel/hosttest/fbtext`
- `kernel/hosttest/inputtest_feel`
- `kernel/hosttest/inputtest_hid`
- `kernel/hosttest/palette`
- `kernel/hosttest/termwrap`
- `kernel/hosttest/walltest`
- `kernel/hosttest/wmtest.c`
- `kernel/hosttest/wmtest_feel`
- `kernel/hosttest/wmtest_feel.c`

</details>

## 2026-08-20 — fix(oracle): aim the drag at a rect the boot path actually prints (37a4238)

`claude/compassionate-curie-a0599c` · 3 files changed, 96 insertions(+), 19 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `kernel/oracle/animate.py`
- `kernel/oracle/zlosboot.py`

</details>

## 2026-08-20 — docs: extract the complete widget specification from ds-reference.html (a2c5693)

`claude/compassionate-curie-a0599c` · 1 file changed, 1803 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `kernel/docs/reference-widgets.md`

</details>

## 2026-08-20 — fix(oracle): keep the full serial transcript, and add the README (11c3dc8)

`claude/compassionate-curie-a0599c` · 6 files changed, 480 insertions(+), 11 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>6 file(s)</summary>

- `docs/JOURNAL.md`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/wmtest_feel`
- `kernel/oracle/README.md`
- `kernel/oracle/animate.py`
- `kernel/oracle/zlosboot.py`

</details>

## 2026-08-20 — feat(oracle): animate.py, and hue reports WHY it fired (6315f35)

`claude/compassionate-curie-a0599c` · 4 files changed, 90 insertions(+), 12 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>4 file(s)</summary>

- `docs/JOURNAL.md`
- `kernel/hosttest/build.sh`
- `kernel/oracle/diff-regions.py`
- `kernel/refrender/README.md`

</details>

## 2026-08-20 — feat(oracle): boot at 1280x800, and four complementary fidelity measures (6c0e0d6)

`claude/compassionate-curie-a0599c` · 9 files changed, 1019 insertions(+), 74 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>9 file(s)</summary>

- `docs/JOURNAL.md`
- `kernel/SOURCES`
- `kernel/ease.c`
- `kernel/ease.h`
- `kernel/hosttest/easetest`
- `kernel/hosttest/easetest.c`
- `kernel/oracle/animate.py`
- `kernel/oracle/diff-regions.py`
- `kernel/wm.c`

</details>

## 2026-08-20 — feat(refrender): shots.sh captures all 54 reference frames, plus README (117e53b)

`claude/compassionate-curie-a0599c` · 2 files changed, 244 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/refrender/README.md`
- `kernel/refrender/support.js`

</details>

## 2026-08-20 — feat(design): the palette gate now checks the chain, not itself (c4c8581)

`claude/compassionate-curie-a0599c` · 15 files changed, 1310 insertions(+), 233 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>15 file(s)</summary>

- `docs/JOURNAL.md`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/fbtext`
- `kernel/hosttest/inputtest_feel`
- `kernel/hosttest/palette`
- `kernel/hosttest/palette.c`
- `kernel/hosttest/termwrap`
- `kernel/hosttest/walltest`
- `kernel/hosttest/wmtest_feel`
- `kernel/oracle/diff-regions.py`
- `kernel/oracle/shot-zlos.py`
- `kernel/oracle/zlosboot.py`
- _…and 3 more_

</details>

## 2026-08-20 — feat(refrender): support.js shim renders the ds.html desktop reference (84a8195)

`claude/compassionate-curie-a0599c` · 4 files changed, 793 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>4 file(s)</summary>

- `kernel/refrender/.gitignore`
- `kernel/refrender/package.json`
- `kernel/refrender/render-ref.mjs`
- `kernel/refrender/support.js`

</details>

## 2026-08-20 — feat(oracle): region map generated from the mockup, not transcribed (a4ae44b)

`claude/compassionate-curie-a0599c` · 3 files changed, 1314 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/oracle/.gitignore`
- `kernel/oracle/gen-regions.py`
- `kernel/oracle/regions.json`

</details>

## 2026-08-20 — feat(design): ONE palette source, and it is the reference's lime-on-grey (506a6a9)

`claude/compassionate-curie-a0599c` · 8 files changed, 9282 insertions(+), 62 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>8 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`
- `docs/design/ds-reference.html`
- `kernel/design.h`
- `kernel/refrender/ds.html`
- `kernel/refrender/vendor/react-dom.js`
- `kernel/refrender/vendor/react.js`
- `kernel/ui.c`

</details>

## 2026-08-20 — merge: desktop/storage-recovered, and port every app onto the theme roles (57ee9e9)

`claude/compassionate-curie-a0599c` · 16 files changed, 677 insertions(+), 134 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>9 file(s)</summary>

- `freestanding/runtime_kernel.c`
- `kernel/HANDOFF.md`
- `kernel/apps_games1.zl`
- `kernel/apps_games2.zl`
- `kernel/apps_registry.zl`
- `kernel/apps_system.zl`
- `kernel/apps_utils.zl`
- `kernel/hosttest/wmshot.c`
- `kernel/kernel.zl`

</details>

## 2026-08-19 — feat(apps): 3 system apps, RNG seeding, and a proven interactive probe (94cd4c8)

`desktop/app-suite` · 8 files changed, 435 insertions(+), 10 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>8 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`
- `kernel/apps_games1.zl`
- `kernel/apps_games2.zl`
- `kernel/apps_registry.zl`
- `kernel/apps_system.zl`
- `kernel/apps_utils.zl`
- `kernel/probe-catalog.py`

</details>

## 2026-08-19 — feat(apps): app-suite registry, catalog window, 11 utilities, 6 games (24c06bd)

`desktop/app-suite` · 11 files changed, 1731 insertions(+), 22 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>11 file(s)</summary>

- `compile.c`
- `kernel/apps_common.zl`
- `kernel/apps_games1.zl`
- `kernel/apps_games2.zl`
- `kernel/apps_registry.zl`
- `kernel/apps_system.zl`
- `kernel/apps_utils.zl`
- `kernel/check-zl-calls.sh`
- `kernel/kernel.zl`
- `kernel/mkdisk.sh`
- `kernel/raw_boot.asm`

</details>

## 2026-08-19 — chore: regenerated journal/TODO (post-commit hook) (2c2092f)

`main` · 1 file changed, 17 insertions(+)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |
## 2026-08-19 — docs: the journal entry for the commit that added the journal (b3f5fd2)

`ci/gates-and-agent-brief` · 1 file changed, 2 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 34 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-19 — chore: regenerated journal/TODO (post-commit hook) (3e6497c)

`main` · 1 file changed, 18 insertions(+)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-19 — chore: the journal and TODO the post-commit hook regenerated (0a4bc8f)

`main` · 2 files changed, 24 insertions(+), 16 deletions(-)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`

</details>

## 2026-08-19 — Merge branch 'ci/gates-and-agent-brief' (18f34e3)

`main` · 29 files changed, 2733 insertions(+), 73 deletions(-)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `.gitignore`
- `kernel/hosttest/wmshot.c`

</details>


## 2026-08-19 — ci: write down what happens, keep the docs true, regenerate the TODO (cb17faf)

`ci/gates-and-agent-brief` · 9 files changed, 719 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 34 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>9 file(s)</summary>

- `.github/workflows/docs.yml`
- `TODO.md`
- `docs/JOURNAL.md`
- `tools/doc-check-ignore.txt`
- `tools/doc-check.sh`
- `tools/doc-claims.txt`
- `tools/install-hooks.sh`
- `tools/journal.sh`
- `tools/todo.sh`

</details>

## 2026-08-18 — ci: preflight.sh and a pre-push hook, since main cannot be protected (46f4bd2)

`ci/gates-and-agent-brief` · 2 files changed, 132 insertions(+)

| | |
|---|---|
| EFI truncation sites | 34 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>2 file(s)</summary>

- `tools/install-hooks.sh`
- `tools/preflight.sh`

</details>

