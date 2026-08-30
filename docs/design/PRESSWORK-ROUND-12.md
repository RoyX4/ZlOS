<!-- Round-twelve fleet audit. 195 agents, 62 claimed, 56 confirmed after a
three-lens adversarial verification, 6 refuted. Saved here because the
round-eleven list lived in /tmp and /tmp has eaten four prototypes on this box. -->

# PRESSWORK Round 12 — fleet audit

**Tree:** `/home/roy/Documents/repos/zl-linux-presswork`, branch `design/presswork`, HEAD `a1bb161`.
**Authority:** `docs/design/presswork-prototype.html` and nothing else.

> **Reading the citations.** Every `kernel/src/kernel.zl` and `kernel/apps/apps_sys2.zl` line number below is **HEAD-relative**. Both files are dirty in the working tree (`git status --short` shows ` M` on each); the pending diff inserts at kernel.zl 3273, 11745, 13897, 14268, so working-tree lines below 3272 sit **+23** from the numbers here. Verify with `git show HEAD:<file> | sed -n '<N>p'`, not a bare `sed`.

---

## Opening

Fifty-six findings survived adversarial three-lens verification; six were refuted. Thirty-one of the fifty-six are in `kernel/src/kernel.zl`, thirteen in `kernel/apps/`, nine in `kernel/src/graphics/`, three in `design.h`/`ease.h`. The single most important is **the raster strip's plot geometry** (`kernel/src/kernel.zl:2017-2018`): `RASTER_FOOT`/`RASTER_HEAD` are applied to the full 30 dp border-box band where the authority's canvas is the 28 dp padding box (proto:2476, `(h - 10)`; proto:2478, `h - 5 - bh`), so every column is drawn **22/18 = 1.22×** its specified height and the base rule sits 3 dp low. That is the one defect where a *measured* instrument is silently rescaled: the strip is the evidence the whole PRESS argument rests on, the numbers feeding it are read correctly, and the drawing of them lies. `probe-field.py:89` checks only the band's outer height and never touches the plot's interior, so nothing in the tree can disagree. The sharpest *invented* figure is second: `s3nw_mtu()` returns the literal `1500` for every driver (`kernel/apps/apps_sys3.zl:1602`) when `virtio_net.c:128-130` defines `FRAME_MAX (BUF_SZ - HDR_LEN)` = **2036** — a written number that is also simply wrong, in a project whose stated rule is that every figure is read and never written.

---

## Survivors, ranked by how badly a reader is misled

| # | Severity | Site | What a reader wrongly believes |
|---|---|---|---|
| 1 | wrong-visual | kernel.zl:2017 | The machine is closer to its frame budget than it is (columns 1.22× tall) |
| 2 | invented-figure | apps_sys3.zl:1602 | This NIC's frame ceiling is 1500 (it is 2036), and e1000/cdc-ecm share it |
| 3 | wrong-behaviour | apps_system.zl:100 | Three clocks are being compared (it is one reading, twice) |
| 4 | wrong-visual | apps_sys3.zl:1850 | eth0 — the only working interface — is faulted |
| 5 | wrong-behaviour | apps_sys2.zl:838 | The hex pane walks the kernel image (it reaches <1% of it) |
| 6 | invented-figure | kernel.zl:2594 | The map's low address is measured (it is typed) |
| 7 | invented-figure | apps_sys2.zl:366 | The console ring is 160 rows *as read from term.c* (it is prose) |
| 8 | wrong-behaviour | kernel.zl:2407 | A module grid is in force (the legend prints; the rules do not) |
| 9 | invented-figure | kernel.zl:12407 | This menu belongs to some window (it never says which) |
| 10 | invented-figure | kernel.zl:9738 | `ZD_TR_LAB 1.4 / ZD_TR_BIG 2.6` were read from design.h |
| 11 | wrong-behaviour | kernel.zl:1229 | `per window timing → off` switched the rail's figures off |
| 12 | missing-feature | wm.c:3359 | The band's `APP US` respects the timing switch |
| 13 | wrong-behaviour | wm.c:4227 | The drawn corner grip is where resize starts |
| 14 | wrong-visual | kernel.zl:9750 | (unreadable) — a caption is drawn over a live control row |
| 15 | invented-figure | kernel.zl:9772 | The focus-bar track's ends came from `UI_FBAR_MIN/MAX` |
| 16 | missing-feature | kernel.zl:11960 | Two disabled menu rows carry no ratio because none exists |
| 17 | missing-feature | apps_system.zl:267 | The registers pane has no derivation and no action by design |
| 18 | wrong-behaviour | wm.c:3315 | `ws NN` clears the grip (it is drawn through the diagonals) |
| 19 | missing-feature | kernel.zl:10216 | The ladder has four columns |
| 20 | missing-feature | kernel.zl:10105 | The pane states the smallest step (its own comment says so; it does not) |
| 21 | missing-feature | kernel.zl:10478 | The composite clock's own tab counts composites |
| 22 | missing-feature | kernel.zl:10231 | The ladder is complete at 18 rungs (the authority has 21) |
| 23 | wrong-visual | kernel.zl:10319 | The occlusion fallback has no comparable number |
| 24 | missing-feature | kernel.zl:3183 | The slug names every resolved setting (LADDER is absent) |
| 25 | missing-feature | kernel.zl:483 | The foot band is structural (it survives both instruments off) |
| 26 | wrong-visual | kernel.zl:11617 | Row 5 is `regdump cpuid apic`; searching "pipe" finds nothing |
| 27 | wrong-visual | apps_registry.zl:730 | Vermilion is a category, not an accent |
| 28 | wrong-visual | apps_sys3.zl:835 | Selection is a vermilion wash, not the knockout |
| 29 | wrong-visual | apps_sys3.zl:626 | The largest-files rows have no groove (drawn in the plate's own colour) |
| 30 | wrong-visual | kernel.zl:12661 | The focused tile is marked with a grey line, not the focus accent |
| 31 | wrong-visual | wm.c:3119 | With the knockout off, the focused header has no accent at all |
| 32 | wrong-visual | wm.c:3468 | Three bright bars in the quietest band are intentional |
| 33 | wrong-visual | apps_sys2.zl:766 | `<` is a latched toggle (it wears the selection face off page 0) |
| 34 | wrong-visual | kernel.zl:542 | `RAIL_IDENT_H 58` reconciles (its own comment sums to 57) |
| 35 | wrong-visual | kernel.zl:2884 | The ruler bar is centred in its row (it rides 3 dp high) |
| 36 | wrong-visual | wm.c:3043 | The focus bar starts below the header groove |
| 37 | wrong-visual | wm.c:2917 | The struck top run lies inside the plate's ring |
| 38 | wrong-visual | wm.c:3312 | Band cells are on the authority's 10 dp gutter |
| 39 | wrong-visual | kernel.zl:12639 | Activities slack is under the plates (it is spread between rows) |
| 40 | wrong-visual | kernel.zl:12700 | The lock sheet is the authority's width (460 vs 660) |
| 41 | wrong-visual | kernel.zl:12416 | The menu separator is a struck rule (it is an inset hairline) |
| 42 | wrong-visual | kernel.zl:7620 | The fourteen register apps take the prototype's rects |
| 43 | wrong-visual | apps_system.zl:346 | `0` is where a keypad puts it |
| 44 | invented-figure | design.h:740 | `ZD_MENU_W 220` is the authority's minimum (it is 214, then halved) |
| 45 | invented-figure | design.h:741 | The menu's rows sit flush to its edge |
| 46 | wrong-visual | design.h:732 | Settings cards use `.card`'s own padding |
| 47 | wrong-visual | design.h:754 | The toast's padding is symmetric (it reserves 14 dp for the mark) |
| 48 | wrong-visual | design.h:735 | The kv row is 20 dp on a 16 dp gap |
| 49 | wrong-visual | design.h:592 | Some button rung matches the authority's one button |
| 50 | wrong-visual | ease.h:83 | The toast rises 10 dp |
| 51 | wrong-behaviour | wm.c:4942 | The toast enters at RISE (it runs at TRAVEL, 160 ms) |
| 52 | wrong-visual | wm.c:3740 | ds-reference is still an authority |
| 53 | wrong-behaviour | wm.c:2175 | Windows scale and rise on open |
| 54 | missing-feature | uikit.c:2214 | A blinking caret is not implementable at this layer |
| 55 | missing-feature | ui.c:1049 | The switch knob's travel is instantaneous |
| 56 | missing-feature | wm.c:3124 | Focus is a value flip on the title, not a fade |

---

## A. The screen states something false about the machine (1–10)

**1. Raster plot rescaled — `kernel/src/kernel.zl:2017-2018`, consumed at 2265-2266.**
`base = sh - RASTER_FOOT * u` and `top = RASTER_HEAD * u` are taken against `strip_h()`, the full 30 dp border-box band. The authority's canvas is `#raster`'s clientHeight — the 28 dp padding box (proto:486-487, proto:325) — with the plot height `h - 10` (proto:2476) and the base rule at `h - 5` (proto:2481). At ui 1 the tree gives 22 dp of travel where the spec gives 18, and puts the base rule at y 26 instead of 23. **Fix:** derive `base`/`top` from the padding box, i.e. subtract the 2 dp border before applying FOOT/HEAD, and set FOOT/HEAD to 5/5 to match `h - 5` and `h - 10`. `git log -S RASTER_FOOT` shows the constant untouched since `74eb0d8`; the border-box conversion never reached it.

**2. `s3nw_mtu()` returns 1500 — `kernel/apps/apps_sys3.zl:1602`.**
The comment above it (1599-1601) claims 1500 "is virtio-net's own frame ceiling". `kernel/src/drivers/network/virtio_net.c:128-130` defines `BUF_SZ 2048`, `HDR_LEN 12`, `FRAME_MAX (BUF_SZ - HDR_LEN)` = 2036, and `virtio_net.c:488` rejects only `len > FRAME_MAX`. The same literal is returned for `net_kind()` 2 (e1000) and 3 (cdc-ecm), drawn with `num_aa` at 1846 beside genuinely-read `net_rx()`/`net_tx()`, and again in the toolbar at 1747. The authority (proto:340, proto:1817-1820) has no MTU row at all. **Fix:** add a `netdev_mtu()` accessor returning `FRAME_MAX` per driver and bind it; do not swap one literal for another. This is the identical fault this file already removed from the loopback row at 1583-1591.

**3. Three clock cards, one reading — `kernel/apps/apps_system.zl:98-104`.**
`up = idiv(ticks(), 100)` is computed once; line 99 draws it as `MONOTONIC` and line 100 draws the same variable as `SINCE BOOT`. The third card prints `ticks()`, the same quantity ×100. proto:1841-1843 is `['monotonic','boot','process'].map((n, i) => ... f(up - i * 41))` — three distinct durations. **Fix:** derive boot and process from separate sources (or draw one card and say so); two labels on one variable cannot ever diverge.

**4. Healthy interface painted as a fault — `kernel/apps/apps_sys3.zl:1849-1850.**
`dc = theme(TH_TEXT)` then `if s3nw_sel != 0 { dc = theme(TH_DANGER) }`, so selecting eth0 draws `virtio-net` in the danger colour while loopback — the device that does not exist — gets ordinary ink. proto:1817 is a plain `kv('driver', 'e1000e, 8086:15D8, irq 19')` with no `bad` flag. The justification at 1566-1568 cites `ds 4054`, the superseded predecessor, and the same file argues the opposite at 1780-1783. **Fix:** delete the conditional; `dc = theme(TH_TEXT)`.

**5. Hex viewer reaches ~24 KiB of a 2.9 MiB image — `kernel/apps/apps_sys2.zl:838-845`.**
`HX_MAXPAGE = 64` clamps a page *count* while page *size* is `hx_rows(ah) * 16` (697-702), which tracks window height — so the highest reachable address changes on resize and a held `hx_page` names different bytes at different sizes. `arena.c:91` records the image at 0x100000..0x2E15C0. proto:1764 pages by a fixed `0x1000 * S.hexPage` and proto:1786 prints `page N of 1024`. **Fix:** clamp on *address* (`base + pagesz <= image_end`) rather than page index, and draw both chips disabled at their stops — `sys2_event` (1329-1336) currently returns 1 and repaints on a no-op.

**6. `RULER_BASE = 0x00100000` — `kernel/src/kernel.zl:2594`, drawn at 2929.**
The readout's right end is read (`mem_top() = vram() + fb_pitch() * px_h()`, 2647); its left end is typed. The real authority for the load address is `. = 1M` at `kernel/boot/link64.ld:13`, and nothing binds it into zl — link64.ld:25 exports `__kernel_end` but no start symbol. It carries no `# memmap.h X` tag, so `check-memmap-mirror.py` skips it and `check-memmap.sh` lists it under "fixed addresses with no size check here". The tree's own comment at 2643-2645 says the readout exists to "name the real span". **Fix:** export a start symbol from link64.ld and a native returning it.

**7. "scrollback ring, 160 rows" — `kernel/apps/apps_sys2.zl:366-367`.**
160 is `term.c:29`'s `#define TERM_ROWS 160`, retyped across the language boundary; `term.c` exposes `term_lines()` (live count) but no capacity accessor. proto:1755 puts the ring's own description in this slot. The comment three lines above (362-365) correctly refuses to print a dropped count it cannot read, then writes the depth it also cannot read. **Fix:** add `term_rows()`.

**8. Grid legend printed with the grid off — `kernel/src/kernel.zl:2405-2409`.**
The grid-off arm calls `draw_grid_legend()` gated only on `desk_legend_on`, which is initialised to 1 at 2386 and has exactly three mentions tree-wide (2386, 2407, 2454) — no Settings row, no other writer. proto:2440-2442 blanks the legend whenever `S.cfg.grid` is false. **Fix:** gate the call on `desk_grid_on`. The crop-mark half of the same arm is correct (proto:2427) and must stay.

**9. Window menu header reads "WINDOW" — `kernel/src/kernel.zl:12407`.**
proto:2880 passes `APP[id].n + ' ' + APP[id].name` — "05 kernel log". The comment on the line directly above (12406) says the header "names ... the window's register row" and the code then names nothing. The read exists forty lines away: `ov_draw_activities` does `slot_of_app(wm_app(ow2))` → `label_num(..., osl + 1, ...)` + `rail_name(osl)` at 12664-12668. **Fix:** apply that same pair using `ov_menu_win`.

**10. Tracking note asserts two design.h values — `kernel/src/kernel.zl:9738`, repeated at 10735.**
The string "ZD_TR_LAB 1.4 and ZD_TR_BIG 2.6" restates `design.h:574-575` (`14`, `26`, ×10) with nothing reading them. proto:2181's note is `'off is what shipping without UI_F_TRACK looks like'` — no figures at all. The pane already composes decimals from integers (`set_dec`/`set_q`, 9175-9191). **Fix:** use the authority's text; the composition machinery then isn't needed. Also note this control is drawn sixth where proto:2181 puts it first.

---

## B. Controls that do not do what they say (11–18)

**11. `per window timing → off` leaves the rail printing — `kernel/src/kernel.zl:1229` and 1311.**
Both `draw_rail_row` and `draw_rail_win` gate the figure on `running == 1` alone. `set_us`'s complete reader list is 6372, 6375, 6391, 6394 (System Monitor table only), plus 9378/9398/9500. proto:446 is `body.nous .slot .us { display: none; }`, toggled at proto:2719. **Fix:** add `if set_us != 2` to both draw sites. (The rail printing kpx rather than µs is a separate documented divergence at 1097-1099 and is not this.)

**12. Status band's `APP US` has no off state — `kernel/src/graphics/windowing/wm.c:3359-3371`.**
`chrome_band()` formats `W->band_us` unconditionally; no mode is read anywhere in the band, and `ui.c` exports `ui_knockout_get`/`ui_over_get`/`ui_focus_bar_dp` but nothing for the timing mode, so `wm.c` physically cannot see the switch. proto:724 is `body.nous .sband .us { display: none; }` and proto:717-719 gives `repaint_pixels` as the second rung. **Fix:** export the mode from ui.c and branch on it; two of three rungs are currently silent on the surface the authority attaches them to.

**13. Resize grabs both whole edges — `kernel/src/graphics/windowing/wm.c:4227`, checked at 4554.**
`in_resize_grip()` returns `(x >= rx - e) || (y >= by - e)` with `e = RESIZE_EDGE(t)` = 8 dp, so an 8 dp column down the right and an 8 dp strip along the bottom never reach the client — 16 px each at ui 2, crossing any scrollbar gutter and the status band. proto:725-726 defines `.grip` as a 15 dp corner square and proto:2851-2852 is the only resize entry point; there is no edge hit test in the authority. The file contradicts itself (4205 says edges, 4290 says "A corner, not an edge"), and the drawn mark at 3520-3560 matches 4290. **Fix:** make the hit test the drawn square.

**14. PRESS caption drawn over the segment row — `kernel/src/kernel.zl:9750`.**
`set_press_ctl` draws seven rows (n=0 at 9727 … n=6 at 9749, verified verbatim at HEAD) and returns `6 * set_rowh()`. `set_press_body` adds that at 10409 and immediately calls `set_head(...)` at 10410, landing the caption and its rule inside the 26 dp row carrying `per window timing` and its three-rung bar. proto:2188 declares that segment as the tab's seventh control. The hit test is unaffected (10744 re-derives `set_rowN_y(y,6)`), so the control still works and cannot be read. **Fix:** `return 7 * set_rowh()`. The stale comment at 9677 ("PRESS now has four") should go with it.

**15. Focus-bar slider bounds typed twice — `kernel/src/kernel.zl:9772` and 10778.**
Track and click-map each carry their own `1, 6`. The real clamp is `UI_FBAR_MIN`/`UI_FBAR_MAX` at `ui.h:242-243`, enforced only at `ui.c:494-495`; `grep -rn UI_FBAR_MIN kernel/src` returns ui.h and ui.c only. proto:2084 is `slider('focus bar', 'fbar', 1, 6, ' dp')`. Raise the C max to 8 and the keyboard path at 10815 reaches 7 and 8 while the track still ends at 6. **Fix:** bind two accessors and read them at both sites. The comment at 9766-9768 already states the problem.

**16. Window menu value column absent on three of five rows — `kernel/src/kernel.zl:11960`, drawn at 12425-12427.**
`ov_win_haskey` returns 1 for rows 0 and 2 only. Consequences: `ov_win_key(1)` = `"TRAVEL 160"` (11968) has no reader; and rows 5 and 6 — the two disabled rows whose whole purpose is a live contrast ratio (proto:2875, proto:2877) — print nothing, while the comment at 11947-11948 asserts "both carry a live ratio". `ui_ratio` is bound and already used at 3074-3075. **Fix:** add a ratio branch to `ov_win_key` and admit rows 1, 5, 6. Same shape one function up: `ov_ctx_key(11)` = `"CTRL L"` (11908) is unreachable because `ov_ctx_haskey` (11893-11901) omits row 11 (proto:2898).

**17. Registers pane has neither well nor controls — `kernel/apps/apps_system.zl:267-311`.**
`si_draw` renders head and table and stops; `reg_event` has no `APP_SYSINFO` arm, so the pane consumes no clicks. proto:1886-1888 is the `dpll_from_link` derivation well and proto:1889-1890 the two controls (`re-read block` primary + `intel.c`). The pane's own comment at 207-213 explains only why register *contents* read zero under QEMU and says nothing about the well or the buttons; `grep -rn "re-read\|dpll"` over `kernel/apps/*.zl` and `kernel.zl` finds no counterpart anywhere. **Fix:** the derivation well first — it is text and arithmetic the pane already has the registers for.

**18. Band text stop measured against the wrong grip — `kernel/src/graphics/windowing/wm.c:3315`.**
`grip = W->x + W->w - 1 - UI_S3(t)` uses 12 dp (`ui.h:127`) while the grip is drawn from `ZD_GRIP` = 15 dp (`design.h:416`, `wm.c:3520`). Walked at ui 1 with r=9 the drawn grip spans `W->x+W->w-19 .. -5` over rows `W->y+W->h-19 .. -5`; the band occupies rows `-27 .. -8`, so they overlap and the clamp stops the run six pixels *inside* the grip. proto:725-726 fixes the grip at 15 dp. **Fix:** clamp against `ZD_GRIP`; also correct the stale comment at 3266.

---

## C. Readings and rows the authority has and the tree does not (19–26)

**19. No L\* column — `kernel/src/kernel.zl:10216-10219`, rows at 10125-10131.**
proto:2039 declares five columns and proto:2052 fills the third with `lstar(t[0])` for every row. There is no L\* helper in the tree: `grep -rniE '\blstar\b|l_star|to_lab|cielab' kernel/src kernel/apps` finds nothing, and `ui.h:230-232` exposes only ratio and ceil helpers, so the sRGB→L\* conversion is genuinely absent. It costs more than a column: proto:2077's ink-ramp kv is replaced at 10266 by the figure-free "four rungs. the fifth no longer clears", and proto:2073-2076's numbered well, built from live L\* deltas, has no counterpart. The values exist in the tree as *prose* (`/* L* 18.15 */`, design.h:121) — exactly the written-not-read shape. **Fix:** one `ui_lstar_q2` native; everything else follows. Note the pane's own "WHAT IS NOT HERE AND WHY" (10098-10108) does not mention this omission.

**20. Smallest-step summary row absent — `kernel/src/kernel.zl:10105`, gap at 10250→10255.**
The comment at 10104-10106 defends dropping the authority's graphite column on the grounds that "the summary row states the smallest one" — and that row is not drawn. proto:2064-2067 appends it with `minStep.toFixed(4)`, graphite's 1.0728, and `((minStep - 1) / 0.0728) + 'x the magnitude'`. The arithmetic already exists on another surface (System pane, 7065-7071). **Fix:** min of the five `ui_ratio` calls `set_step_row` already makes.

**21. PRESS tab never prints a composite count — `kernel/src/kernel.zl:10478-10499`.**
Its four readings are THIS PANE/LAST DRAW, TYPE SIZES, TYPE STYLES, GLYPH RASTERISER; only the middle two are the authority's (proto:2215-2216). proto:2213-2214 are `kv('this session', S.composites + ' composites in ' + S.t + ' s')` and `kv('extrapolated', perHour + ' per hour')`. Neither is blocked: `wm_painted()` (`wm.c:4833`) is called from zl in eight places including 6915, and a windowed per-hour rate is already implemented at 2159-2172. **Fix:** two kv rows. This is separate from the deliberately-absent raster-clock segment — a reading needs no second clock.

**22. Ladder omits three rungs — `kernel/src/kernel.zl:10221-10238`.**
`ZD_KNOCK_INK` (proto:2045), `ZD_GRID` (proto:2048) and `ZD_INK_ON` (proto:2049) are missing from eighteen drawn rows. All three are live theme roles in this file — `TH_KNOCK_INK` 35 (kernel.zl:122), `TH_GRID` 38 (:125), `TH_INK_ON` 41 (:128) — and all three are already read elsewhere (1451, 10078; 2421; 3456). `ZD_KNOCK_INK` is the sharpest: the FOCUS tab prints its ratio as a kv at 10083 while the LADDER tab hides the rung. **Fix:** three lines of the same shape as their neighbours. Also restore ZD_LIT's role text to proto:2043's "struck top, rule 2" (currently "struck top" at 10228).

**23. Occlusion note lost its ratio — `kernel/src/kernel.zl:10319`, repeated at 10766, 11690, 11734.**
proto:2144's note is `'off falls back to ZD_CUT at ' + r2('--zd-cut', '--zd-base') + ':1'`; the tree's is the constant "off falls back to the plain ZD_CUT ring". `set_r_cut()` (9609) is that exact ratio and is already drawn twice on the same tab (10336, 10370), and `set_rat_sfx_r` (used at 10391) is the helper that composes a ratio with a trailing string. **Fix:** one call.

**24. Slug omits LADDER — `kernel/src/kernel.zl:3136-3186`.**
proto:2511-2512 prints `GUT … LADDER <name> MIN STEP …`; `draw_slug` goes straight from GUT to MIN STEP. The accounting comment at 3020-3055 lists only LOAD and SKEW as deliberate drops. The name is available: `seg_name(SEG_LADDER, 0)` = `"presswork"` (9389-9391). It is missing on both surfaces the authority prints it — the rail's build line at 1412 says `"0.3 x86_64"` where proto:987 is `0.3 · x86_64 · presswork`. **Fix:** one field in each.

**25. Foot band survives both instruments off — `kernel/src/kernel.zl:483`, drawn 2811-2818.**
`foot_h()` never consults `desk_ruler_on`/`desk_slug_on` (2378-2379), `draw_foot` fills the whole 46 dp band and its lit rule unconditionally and gates only the contents (2828, 2836), and the handlers at 10712-10720 change no geometry. proto:581 is `body.noruler.noslug #foot { display: none; }`, which returns the 46 dp to `#field` (proto:526, `flex: 1`). The two single-instrument rules (proto:579-580) are implemented. **Fix:** return 0 from `foot_h()` when both are off; `field_h()` (518) already subtracts it correctly.

**26. Palette row 5 mislabelled — `kernel/src/kernel.zl:11617`.**
`ov_pal_label(5)` returns `"regdump cpuid apic"`; proto:1526 is `['regdump --pipe A', 'shell', 'regs']`. The other fifteen match character for character under a comment (11612-11615) claiming the prototype's own order. Because `ov_pal_match` (12181) filters on the label, typing "pipe" finds nothing. The dispatch is correct — row 5 runs `run_command(216, 0)` → `reg_open(32)` at 5836, matching `openWin('regs')`. **Fix:** the string.

---

## D. Colour and treatment carrying the wrong meaning (27–33)

**27. Catalogue spends vermilion on 23 tiles and steel on 14 — `kernel/apps/apps_registry.zl:730`.**
Each tile takes a 118-unit stripe in `ac_cat_color(reg_cat(id))`, which `apps_common.zl:63-67` resolves to `TH_ACCENT` for game and `TH_STEEL` for system. Counting `reg_exists()` ids (52-99): games 25-30, 34, 52-59, 61-68 = 23, plus the legend swatch at 676; system 31-33, 40-50 = 14. proto:129-131: "vermilion never fills an area wider than the focus bar except one button per view"; proto:120-123: steel "appears inside an instrument and nowhere else". The tree cites proto:129-131 by line at `apps_sys3.zl:1662-1677` and prints the rule on screen at `kernel.zl:10459`. **Fix:** category by label or by a focus-bar-width mark, not a fill.

**28. Services selection is a vermilion wash — `kernel/apps/apps_sys3.zl:835`.**
`blend(ax, ry, aw, S3SV_ROW * u, theme(TH_ACCENT), 18)` plus a solid 2 dp vermilion edge, across the full client width, 46 units tall. proto:793: "SELECTION IS THE KNOCKOUT, not a second idea" / `tr.sel td { background: var(--zd-knock) }`. The Network sidebar was already corrected and its comment at 1670-1677 names the two windows still breaking it. **Fix:** `TH_KNOCK` + `TH_KNOCK_INK`, drop the bar.

**29. Disk Usage row rule painted invisible — `kernel/apps/apps_sys3.zl:626`.**
`fill_rgb(x, ry + 22 * u, w, u, theme(TH_PANEL))` — the rule is the colour it sits on. The comment at 621-625 defends it as a faithful clone of `ds 419`; proto:787 is `td { … border-bottom: 1px solid var(--zd-cut) }`. The contradiction is inside one window: `s3du_leg` at 462 calls `pw_row_rule` and gets the visible groove (`kernel.zl:6622-6624`). **Fix:** call `pw_row_rule`.

**30. Focused Activities tile marked in `TH_KO_EDGE` — `kernel/src/kernel.zl:12661`.**
proto:869 is `.thumb.on::after { top: calc(20px * var(--ui)); … background: var(--zd-verm) }`, under proto:864-866 ("the knocked out header and the vermilion focus bar"). The defence at 12652-12659 argues vermilion-on-knockout is 1.3989:1 — but the bar starts at `ty + OV_ACT_TH * u`, *below* the header, on `ZD_BASE`, where `ZD_VERM` is 4.6319:1 (`design.h:309`). `ZD_KO_EDGE` is 0x6F6864, the same value as `ZD_LIT`. The reasoning is correct for the palette's selected row (11814-11820) and was carried to a case it does not cover. **Fix:** `theme(TH_ACCENT)`.

**31. `nokock` focused header has no accent — `kernel/src/graphics/windowing/wm.c:3119`, used at 3135.**
`ink_dim = focused ? t->knock_ink2 : t->text_dim`, and `ui.c:207-216` remaps `knock_ink2` → `ZD_TEXT_3` under knock-off — correct for `.sub` (proto:686) and `.crd` (proto:687), wrong for `.reg`: proto:684 is `body.nokock .win.focus .hdr .reg { color: var(--zd-verm); }`. With the header reduced to a wash, the register number is the only accent the fallback has. `t->accent` is in scope (used two functions up). **Fix:** branch `.reg` on knock-off to `t->accent`.

**32. `nokock` cluster rules stay bright — `kernel/src/graphics/windowing/wm.c:3467-3468`, filled at 3477.**
The ink half survives the remap correctly; the rule half does not. proto:688 sends `border-left-color` back to `var(--zd-cut)`. On `ZD_BASE` a `ZD_TEXT_3` rule is 6.68:1 where the authority asks for a 1.47:1 groove — three bright bars across the quietest band, the opposite of proto:693-694's intent. **Fix:** `rule = knock_off ? t->border : (focused ? t->knock_ink2 : t->border)`.

**33. Hex back chip wears the selection face — `kernel/apps/apps_sys2.zl:766`.**
`ui_chip(cx0, chy, "<", hx_page > 0)` passes the page test as `active`; `uikit.c:2309` forwards it to `ctl_state`, which returns `UI_ST_ON` (607-613) *before* the hover and press tests, and `pill_face` (762-769) paints ON with `UI_COLOR_KNOCK` + KO_EDGE + reversed-out ink. So off page 0 the chip latches, reads as a toggle, and stops responding to the pointer, while the forward chip at 767 (literal 0) renders correctly. proto:1783-1784 draws both as plain `btn`. **Fix:** pass 0; use the page test to *disable*, not to activate.

---

## E. Measurement and geometry that disagree (34–45)

**34. `RAIL_IDENT_H = 58` — `kernel/src/kernel.zl:542`.** The comment's own terms ("11 pad + the 24 dp identity line + 15 + 7") sum to 57, and proto:424 + proto:349/198 + proto:352/196 give 11+24+15+7 = 57 under border-box (proto:325). All three sibling blocks reconcile exactly (543, 544, RAIL_CLOCK_H). Everything below the identity is 1 dp low at ui 1 and 2 at ui 2: `rail_sect_y()` (1136), the rule under REGISTER, `rail_top_h()` (1131), `rail_reg_y()` and the register band. `check-shell-layout.py` parses it (234) but only feeds `top_walk` (152), which compares the walk against the same 58. **Fix:** 57 — and make the check compare against a decomposition, not against itself.

**35. Ruler bar drawn at its row's top — `kernel/src/kernel.zl:2866, 2884.** `ry = fy + RULE_H * u + 3 * u` is the foot's *content top* (proto:562's 3 dp padding behind the 2 dp border), and the 11 dp bar (2881) is filled from it directly. proto:563-565 centres an 11 dp bar in a 17 dp `align-items: center` row — 3 dp down. At ui 1 the bar sits at y 5..16 instead of 8..19, so its four edges, the region walk (2895-2914) and the datum overshoot (2919) all ride high, and the gap to the slug hairline (correct at `fy+24`, 2837) reads 8 dp instead of 5. **Fix:** `ry += 3 * u` for the bar's origin only; label (2873) and readout (2932) are already within a pixel.

**36. Focus bar starts one row high — `kernel/src/graphics/windowing/wm.c:3043`.** `by = W->y + t->title_h`, but `.fbar` (proto:705) is positioned against `.win`'s padding edge, so `top: var(--zd-title-h)` is `W->y + 1 + title_h` — the row *after* the header's groove at `foot = W->y + hh` (3035). `chrome_header` was converted to border-box and says so at 2969-2977; `chrome_focus_bar` was not, and `chrome_shell` calls them in sequence (3391, 3392), so the accent overwrites the first `focus_bar` pixels of `ko_edge` on every focused window. **Fix:** `by = W->y + 1 + t->title_h`.

**37. Struck top run 2 px too wide — `kernel/src/graphics/windowing/wm.c:2917`.** `run_w = W->w - 2 * r` painted at `W->x + r`; the padding box is `W->x+1 .. W->x+W->w-2`, so `left: r` (proto:607) is `W->x+1+r` and the width `W->w-2-2r`. The comment at 2860-2875 makes exactly this argument for y and leaves x on the ring's coordinates. At `r = ZD_R_BOLT = 0` on a maximised plate (3382, proto:613) the run's ends land on the ring's own columns. The `lrun` beside it is already correct (2921). **Fix:** `W->x + 1 + r`, `W->w - 2 - 2*r`.

**38. `ZD_STATUS_GAP 12` — `design.h:632`, read at `wm.c:3312`.** proto:721 is `gap: calc(10px * var(--ui))`. `grep 'gap: calc(12px'` over the authority hits once, proto:854, on `#overview .ohead`. Its neighbours carry citations (`== ZD_BAND_H`, `== ZD_GAP`); this one does not. Three gutters cost 6 dp per band at ui 1, all taken off the run before `ws NN`. **Fix:** 10, with the citation. The file already names this class at 3106-3111: "An uncited constant that is nearly right is the hardest kind to notice."

**39. Activities slack spread between rows — `kernel/src/kernel.zl:12609, 12639.** `ch = (gh - (rows-1)*GAP)/rows` then `ty = gy0 + r*(ch + GAP)`; since tile height ≤ ch, the remainder is divided evenly *between* rows. proto:2551 is `rowTop = r => rowH.slice(0,r).reduce(...) + r*14` with `rowH[r]` the tallest tile actually in that row, under proto:2546-2550: "The slack goes UNDER the plates, not around them." Visible at four or more plates; the prototype's own boot composition holds five (proto:3061). `ov_act_tile_at` (12579) repeats the expression, so the hit test tracks the error. **Fix:** accumulate real row heights in both places.

**40. `OV_LOCK_W = 460` — `kernel/src/kernel.zl:12700`.** proto:934 is `width: calc(660px * var(--ui))`; every measurement on the sheet derives from it (12702, 12704, 12766). No reason is given, unlike `OV_SHEET_W` at 11594-11597 where 580 is cited and the previous 520 called out. The same block also drops the header's ratio (proto:2620, `r2('--zd-knock','--zd-base')`) although `ui_ratio` is bound and used at 3074-3075. **Fix:** 660, and add the ratio.

**41. Menu separator wrong in colour, weight and extent — `kernel/src/kernel.zl:12416`.** `fill_rgb(mx + 8*u, ry + seph/2, mw - 16*u, 1, theme(TH_CUT))` against proto:955's `.msep { height: calc(2px * var(--ui)); background: var(--zd-lit); }` — ZD_CUT and ZD_LIT are opposite ends of the same ladder. `OV_MENU_SEPH = 7` (12014) against 2 also inflates `ov_menu_h` (12021) and `ov_menu_row_at` (12054), so the field menu's three separators (11886-11891) make it 15 dp tall and displace every row below the first. **Fix:** 2 dp, `TH_LIT`, full width, `OV_MENU_SEPH = 2`.

**42. Three register windows at non-authority rects — `kernel/src/kernel.zl:7620`, 7609; `apps_registry.zl:218, 267`.** Register 14 TYPE opens 600×520 against proto:1559's 560×400 with no justification; register 10 SYSTEM opens 560×620 against proto:1555's 600×470 with a stated reason (7605-7607) that is still a departure; and register 13 REGISTERS routes to `APP_SYSINFO` (`rail_app(12)`, 1938) whose rect is 600×470 — the SYSTEM app's, taken before the table was re-pointed — against proto:1558's 620×470. `apps_registry.zl:206-215` states the contract being broken. Register 09 CLOCKS matches, which is what shows the table was applied and these were missed. **Fix:** three rects.

**43. Keypad `0` displaced — `kernel/apps/apps_system.zl:346`.** Index 12 returns `"C"` and 13 returns `"0"`, so the bottom row reads `C 0 = +` where proto:1793's array puts `'0'` at index 12 and `'.'` at 13. The comment at 458-460 defends it by asserting the reference has `'.'` at index 12 — it has `'0'`. The decimal-point argument is sound and applies to index 13. **Fix:** `'0'` at 12, `'C'` at 13; both the reasoning and the layout survive.

**44. `ZD_MENU_W 220` and its halving — `design.h:740`, `uikit.c:1907`.** `grep -c '220px' docs/design/presswork-prototype.html` returns 0; proto:939 is `min-width: calc(214px * var(--ui))`. The sole reader is `return imax(w, DP(ZD_MENU_W) / 2);` — an uncited divide, so the real floor is 110 dp, roughly half the stated minimum. **Fix:** 214, and drop the `/ 2`.

**45. `ZD_MENU_PAD 4` — `design.h:741`, read at `uikit.c:1906, 1912, 1920`.** proto:939-942 is the complete `.menu` rule and declares no padding; `overflow: hidden` plus the 1 px border is what makes rows sit flush, and proto:950's `.mi:last-child { border-bottom: 0 }` only makes sense on a flush list. The tree paints a 4 dp frame of bare `ZD_FLOAT` and insets every row by it. Compounding: `ZD_MENU_ITEM_PX 10` (design.h:745) against proto:948's 11 gives a drawn indent of 4+10 = 14. **Fix:** `ZD_MENU_PAD 0`, `ZD_MENU_ITEM_PX 11`. (`ZD_MENU_ITEM_PY 7` disagrees with the same rule's 3 but has no reader.)

---

## F. Token values with no line in the authority behind them (46–49)

**46. `ZD_CARD_PY 10 /* == ZD_PAD */`, `ZD_CARD_PX 10` — `design.h:732-733`.** proto:748 closes `.card` with `padding: calc(6px * var(--ui)) calc(8px * var(--ui))`. The `== ZD_PAD` annotation is the tell — this is the exact fault design.h names and fixes for the window body at 375-379 ("both were reaching for ZD_PAD — the generic 10 dp spacing step, which belongs to no `.wbody` or `.sband` rule"), left unfixed one screen down. Both are live: `settings.c:387, 398, 454, 459, 533, 589` and `429-430`. **Fix:** 6 and 8.

**47. `ZD_TOAST_PY/PX 10/10` — `design.h:754-755`.** proto:963-964 is `padding: 6 / 11 / 7` with `padding-left: 14`, and the asymmetry is load-bearing: proto:968-969 puts `.toast .bar` flush at `left: 0` and the 14 dp clears it. `uikit.c:1999` sizes the toast as `2 * DP(ZD_TOAST_PY) + …` (20 dp against 13) and `uikit.c:2029` takes both for the content box, so the mark's clearance is 10 dp. design.h:752 and :756 cite the toast's width and gap by name, so the rule was read and its padding was not. **Fix:** four tokens — PY_T 6, PY_B 7, PX 11, PL 14.

**48. `ZD_KV_H 20` / `ZD_KV_GAP 16` — `design.h:735-736`.** proto:795-796 gives `min-height: calc(19px * var(--ui))` and `gap: calc(8px * var(--ui))`, and 19 is the authority's row unit (`.meter` proto:805 and `.strow` proto:470 are 19 too) — 20 is `ZD_BAND_H`, the generic band step, and 8 is exactly `ZD_GAP`. `ZD_KV_H` is live at `uikit.c:1847`; `ZD_KV_GAP` has **no reader anywhere outside design.h** — a wrong figure nothing consults. Separately the 132 dp key column (proto:799) has no token and `uikit.c:1874` right-flushes against a bare `DP(13)`. **Fix:** 19, 8, and a `ZD_KV_KEY_W 132`.

**49. No button padding rung matches — `design.h:592-597`, read at `uikit.c:654-658`.** proto:758-760 is the whole button: `height: 22`, `padding: 0 10`, and the only variant (`.btn.prim`, proto:771) changes colour alone. The tree keeps the predecessor's three-rung ladder (SM 3/9, MD 6/13, LG 7/15); no rung carries 10 horizontally. design.h:589-591 explicitly re-derived this block against PRESSWORK and re-pointed the radii but not the paddings. *Weakest link, stated:* the authority expresses the button as fixed-height-plus-zero-vertical-padding where the tree derives height from padding, so `ZD_PILL_*_PY` is a model difference; the horizontal 10 is a flat contradiction. **Fix:** one rung at PX 10 with a 22 dp height, and prove the other two have callers before keeping them.

---

## G. Motion the authority specifies, and motion it deleted (50–56)

**50–52 are one defect with three sites.** `EASE_TOAST_FROM_DY 10` (`ease.h:83`) against proto:967's `translateY(calc(4px * var(--ui)))`; `EASE_MS_TOAST 160` / `ANIM_FADE` at `wm.c:4942` against proto:966 + proto:206 (`--ms-rise: 90ms`); and the citation defending both at `wm.c:3740-3742` — "ds-reference.html line 20" — which is the superseded predecessor contradicted twice by the current authority. The distance is used a second time to size the damage rectangle (`wm.c:4933`), and `notify.c:27-28` repeats the same figures in prose. `ZD_MS_TOAST` (`design.h:521`) is defined as `ZD_MS_TRAVEL` and has exactly one tree-wide reference — its own definition. `docs/design/surface-specs-2026-08-29.md:239` already names 4 as correct. **Fix:** `EASE_TOAST_FROM_DY 4`, `ZD_MS_TOAST ZD_MS_RISE`, register the toast on that constant, delete the ds-reference comments in wm.c and notify.c.

**53. Windows open with a flourish the authority does not have — `wm.c:2175`.** `wm_anim(i, ANIM_OPEN)` scales the plate 965→1000 (`anim_permille`, 1523-1525) and adds a 10 dp rise (`anim_rect`, 1665-1668) over `ZD_MS_WIN` = TRAVEL 160 ms (1182). proto:598-603 declares one piece of motion on `.win` — `transition: border-color var(--ms-rise) var(--ease)` — and `buildWin` (proto:2274) sets `e.className = 'win'` and nothing else; the authority's only two keyframes are `bl` (proto:831) and `rise` (proto:967, applied to `.toast` alone). `ANIM_CLOSE` (2222) has the same problem and `wm.c:1157` already concedes it, citing ds-reference. **Fix:** delete both; `EASE_WIN_FROM_SCALE`/`EASE_WIN_FROM_DY` (ease.h:78-79) go with them.

**54. The caret does not blink, and the stated reason is false — `uikit.c:2212-2216`.** proto:830 is `body.blink .caret { animation: bl 1s steps(1) infinite; }`, on by default (proto:973) and driven by the motion toggle (proto:2721), so it is part of the motion contract. The comment claims "there is no per-frame clock at this layer" — `wm.c:1481-1501` implements exactly that: `wm_pulse()` reads `idt_ticks()` directly, holds no animation slot, cannot be refused, and is already exported to zl (`runtime_kernel.c:1760`). A 1 s `steps(1)` flip is that function with a threshold instead of an ease. Note the interaction with the refuted pulse finding: the authority's *one* infinite animation is this, and it is the one the tree lacks. **Fix:** threshold `wm_pulse()` in `ui_caret`.

**55. Switch knob teleports — `ui.c:1049`.** `kx = *on ? x + kw - pad - d : x + pad` — a ternary on the settled boolean; nothing in `ui_toggle` (1010-1055) reads `wm_anim_progress` or `ease_apply`. proto:846/848 give `.sw2 > i { transition: left var(--ms-rise) var(--ease) }` over a 19 dp travel. The comment block at 1012-1034 ports every other property of `.sw2` including deriving the travel as `34 - 13 - 2 == 19`, and stops one property short of the one the authority animates. **Fix:** interpolate `kx` on a RISE-length progress.

**56. Focus animates one of six transitioned properties — `wm.c:3124`.** `ink_ttl = focused ? t->knock_ink : t->text_2`, in the same function that computed `fp = anim_progress(win, ANIM_FOCUS)` at 3007 and used it at 3013 for the header band — the interpolant is in scope and unused. proto:663 + proto:671 fade the title's ink over RISE. `wm.c:1131-1133` states the gap and ships it. The other four that cut: window border (proto:602/604) at `wm.c:2872`; the trun's background (proto:609/623); the lrun's top (proto:612/624); the rail mark (proto:456/457). **Fix:** the title first — one `blend_rgb` on `fp`, beside the one already there.

---

## Refuted, grouped by the mistake

**Stale tree — reading a commit instead of the checkout.** The dmesg level-filter finding was accurate against `cb9b634` and corrected before this round; at `a1bb161` plus the working tree, `kl_filter`, `kl_shown`, `kl_count` and `KL_N` survive only inside past-tense comments at `kernel.zl:11708, 11724-11729`, and `apps_sys2.zl:134` is now `KL_SUBMAX = 16`. This is round eleven's failure repeating. The countermeasure is mechanical: cite the commit you read, and re-check any finding whose file is dirty in `git status` before reporting.

**Symbol-name search in C when the implementation lives in zl.** Two findings — "shipped data row is 26 dp" and "data row is proportional, not mono" — both cited `ui.c:1216`/`ui.c:1248`, whose only production caller is the *menu* (`kernel.zl:13652, 13685`); everything else is under `kernel/tests/host/`. The real data rows are `PW_ROWH = 20` (`kernel.zl:6628`, ~20 call sites) and `FILES_ROW = 20` (`kernel.zl:7861`), with the 1 px `TH_CUT` groove at `kernel.zl:6646` — the authority's `td` decomposition, exactly. The mono capability is likewise present and used (`kernel.zl:6654 pw_mcelly`, `8320`, `8331`). Same shape as round eleven's "there are no meshes in this kernel". Countermeasure: before declaring a capability absent, grep the zl side for the *concept*, not the C symbol — and check that the cited draw site has a shipped caller at all.

**A wrong widget under a right authority rule.** Both of the above also applied `td` (proto:784-787) to a `.mi`/`.slot` object, which the authority sizes at 26 dp (proto:170, 437, 462) and pads at 11 or 13 dp. Getting the authority line right does not make it the *governing* line. Countermeasure: name which authority selector matches the drawn object before quoting its declarations.

**Treating authority CSS as behaviour it never executes.** The `.mfill { transition: width var(--ms-settle) }` finding (proto:815) cited a real rule that cannot fire: every `.mfill` is produced as an HTML string (`meter()` proto:1578-1581, `slider()` proto:1993-1999) and reaches the DOM only via whole-subtree replacement (`redraw()`, proto:2293-2295), driven by `tick()` (proto:2691-2704) and the slider's `set()` (proto:2936-2942); `grep style.width` over the authority never touches a fill. A freshly inserted node with an inline width has no prior computed value, so the 240 ms never runs — the prototype's meters cut exactly as `uikit.c:1691` does. The dmesg finding made the same class of error on the JS side: `runPalette` (proto:2596-2609) has no log-filter branch and the prototype holds no log-filter state, so the authority ignores the level exactly as the tree does. Countermeasure: for any authority *behaviour* claim, find the line that mutates the property, not the line that declares it.

**A hard domain mistaken for a written figure.** The System Monitor's `while qi < 16` (`kernel.zl:6516`) is not a literal standing in front of a read one: `apic_redtbl` rejects `irq > 15` at `apic.c:457` *before* consulting `ioapic_pins`, and `gsi_of_irq` is a 16-entry ISA table (`apic.c:118`, filled only for `src < 16` at 265). Raising the bound to `apic_pins()` returns 0 for 16..23 and is swallowed by the loop's own guard at 6519 — zero pixels change. The claim's load-bearing citation, `apic.c:307`, is inside `apic_route_irq`, a different function. The authority's high rows (proto:1457-1461) are all MSI/MSI-X, which an IOAPIC does not route, and 33 is not a pin on a 24-pin IOAPIC. Countermeasure: before calling a bound invented, read the accessor it bounds and check whether the fix would change anything on screen.

**Reading a cost argument as an absolute prohibition.** The "a window still pulses vermilion" finding leaned on proto:2198, which objects to *infinitely* pulsing elements on a damage-driven compositor ("the damage list is never empty"). `ANIM_PULSE` fires once per failed command, runs a triangle to 48 alpha over SETTLE 240 ms (`wm.c:1402-1415`, `anim_ticks[]` 1185), retires its slot and never re-arms. The authority itself ships an infinite animation — the caret at proto:830 — so no absolute line exists. The tree comment it contradicts (`kernel.zl:11486-11488`) scopes itself to the *infinite* pulse. Countermeasure: quote the authority's stated harm and check the code has that property, not merely the same word.

---

## Do these three first

**1. `kernel/src/kernel.zl:9750` — `return 6 * set_rowh()` → `7 * set_rowh()`.** One token. It currently draws a caption, a rule and the head of a twelve-line well *inside* the live `per window timing` row (proto:2188), which makes that control unreadable — and that control is finding 11's subject, so nothing on the PRESS tab can be reviewed until this lands. Highest value per character in the whole set, and the stale comment at 9677 goes with it.

**2. `kernel/src/kernel.zl:2017-2018` + 2265-2266 — put the raster plot back on the padding box.** Two constants and two expressions. It is the only survivor where the tree silently rescales a *measurement* the reader uses to judge the machine: columns 1.22× tall and a base rule 3 dp low, against proto:2476/2478/2481. `probe-field.py:89` checks the band's outer height only, so this will not be caught later by accident — add the plot's interior to that probe in the same change.

**3. `kernel/apps/apps_sys3.zl:1602` — add `netdev_mtu()` and delete the literal.** The real ceiling is `FRAME_MAX` = 2036 (`virtio_net.c:128-130`, enforced at 488), so this is the one invented figure that is also factually wrong, returned unchanged for two drivers it does not describe, and drawn in the same `num_aa` style as genuinely-read neighbours. Fixing it with a different literal reproduces the fault; the accessor is the fix. It is also the flagship case for the project's own rule, and the same fault was already corrected on the row *next to it* (1583-1591).

**Next after those, and why they are close:** `wm.c:4227` (resize grabs an 8 dp band down both edges and steals client clicks — the highest *harm*, held back only because the ranking axis here is how misled a reader is), and the toast cluster (`ease.h:83` + `wm.c:4942` + the comment at `wm.c:3740`) — three findings retired by one small change, which also removes the last ds-reference citations from the motion layer. `notify.c:27-28` carries the same prose and should go in that commit.