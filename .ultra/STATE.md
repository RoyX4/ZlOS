# STATE — zl-browser2, branch `desktop/browser-next`

Updated 2026-08-19 ~15:05 local. (Box clock drifts; prefer
`gh api repos/RoyX4/zl-linux --jq .pushed_at` for real times.)

> The section below this one is the OLD state for `zl-main`'s eight-track
> merge. It is kept because that objective is not this one and neither is
> finished by the other; this file is per-repo and the copy was inherited.

## Objective (this increment)

`kernel/docs/BROWSER-RENDER-PROMPT.md`: fix the URL-bar bug that blocks
hand-testing, then make pages LOOK right — images, flex, grid, more CSS.

**Success metric:** every host gate green, the QEMU boot gate green, and the
new features visible in `browsershot`'s picture rather than only in a count.

## Where the increment stands

### Done and verified

- **The URL-bar bug is fixed and the brief's diagnosis was wrong.** No hit test
  existed on the URL bar; typing fell through the unfocused key switch until
  the string's first `l`, which is the focus shortcut. `t_chrome_click` reports
  **10 failures** against the pre-fix `browser_click`.
- **A second bug fell out of it:** `br_click` never received the button mask,
  so every link click navigated on the press *and* the release.
- **Images end to end.** `png.c` (944 lines) — DEFLATE, the five filters,
  colour types 0/2/3/4/6, depths 1–16, `tRNS`, Adam7. Verified against a
  reference encoder (python zlib): the interlaced file decodes pixel-identical
  to the non-interlaced one. 90,000 mutated PNGs under ASan+UBSan, no fault.
- **Flex, grid, the real box, floats and positioning.** `layout.c` 820 → 2,357.
- **CSS** 704 → 1,592 lines, with the box/flex/grid properties.
- **Gates:** htmltest 384 (was 101), csstest 356, browsertest 103 (was 58, and
  also clean under ASan+UBSan), pngtest 146, fbtext 54, httptest 91, fuzz
  ~400k/seed × 3, nettest/tcptest/dnstest/arenatest/systest/x509test/ecdsatest,
  the four-build source gate, **verify.sh (QEMU boot) green**, and
  **`probe-urlbar.py` green on the real machine.**

- **The link ceiling is the binding constraint now:** `__kernel_end` is
  `0x005DB0C0` = 5.856 MiB against link.ld's 6 MiB assert, leaving **151,360
  bytes** for the whole kernel to grow. Re-measure with
  `./build.sh && nm kernel.elf | grep __kernel_end` before adding any array.
- **Worst-case layout recursion is ~92 KB of stack** (measured by `ulimit -s`
  bisection, 2,336 bytes/frame × `LAY_DEPTH_MAX` 40) against the compositor's
  256 KiB. A hostile page can take a third of the stack. `LAY_DEPTH_MAX` is the
  only knob and the cost is linear in it.

### Search from the URL bar — added after the brief, on request

Typing words instead of an address searches. `looks_like_url()` + `url_encode()`
in `browser.c`, `URL_MAX` 128 → 256, endpoint is one constant.

**Google itself was never broken** — `https://www.google.com/search?q=` returns
HTTP 200 over a verified chain. It returns **no document**: 71 KB parses to 19
nodes and 151 characters of `<noscript>` "enable JavaScript" text, and a
mainstream User-Agent changes nothing (measured both ways). So search points at
an endpoint that serves documents — 615 nodes, 1,534 px of laid-out results.

Also corrected four pre-existing assertions that expected `BR_NO_DNS` for every
hostname. That was never browser behaviour; it was a harness with no resolver.
`browsertest` 103 → **124 checks**.

### Found and fixed along the way

- **`httptest` and `fuzz` could not build at all** — stale link lines since TLS
  landed in `http.c`. Pre-existing, confirmed against a clean `HEAD` tree.
  `fuzz` is the gate that previously found four real layout defects.
- **P1, from the cross-model review:** the picture arena was placed at
  `0x02000000`, on top of `SNAKE_X`, `FS_META`, `FS_DATA`, `LINE_BUF` and
  `HIST_BUF`. A server PNG would have overwritten the RAM filesystem. Moved to
  48 MiB; `memmap.h` now declares the zl block so the compiler catches the
  class. **`check-memmap.sh` had printed those addresses in the same session
  and the output was read without being joined up.**
- **P1:** signed overflow in `b64_decode`, firing on the home page's own inline
  image every time, under 103 green checks. `browsertest_san` now exists.
- **P2 ×2:** a cancelled image fetch could be parsed as a document; two
  unbounded port accumulators (one pre-existing in `parse_url`).
- **`wm_report`'s per-window rects were never printed on the shipped boot
  path.** The marker had been moved to the boot path and the loop left behind,
  so every pointer gate reading those lines was aiming at nothing —
  `probe-drag.py` without `--grab` included. Loop added where the windows open.

### OPEN — not finished, do not treat as done

**All three items that were open have been closed.** What is open now:

- **The node array and text arena are exhausted by one real article.** Measured
  on the English Wikipedia article for Linux: 8192/8192 nodes with 1,673
  dropped, and the text arena 196,607/196,608 full. They cannot grow in BSS —
  the browser is already 1.95 MB of it and the kernel has **132,544 bytes**
  left under link.ld's ceiling. The fix is the one `png.c` already proved: the
  CALLER supplies the storage and it lives in the high-RAM map, which also
  hands ~1.95 MB of BSS back. Bounded, and not done.
- ~~`@media` is skipped entirely~~ **DONE** — evaluated now (`screen`, `all`,
  `min-width`, `max-width`, comma lists), everything else refuses. `csstest`
  356 → 372, mutation-checked both directions. **It barely helped, and that is
  the finding**: `MAX_SELS = 384` stops the parse before it reaches most
  blocks. At 4096 the same sheet yields 375 rules instead of 235 — but
  `sizeof(struct sel)` is 112 bytes, so 4096 costs 458 KB against 131 KB of
  headroom. Not raised: spending the kernel's last headroom on a partial win
  would strand the next person entirely.
- **Pseudo-class selectors are refused, and they are 480 of the selector parts
  in Wikipedia's skin.** Of 709 rules outside `@media`, only 160 have a
  selector this engine supports. Fetching the CSS was necessary and is not
  sufficient.
- **The network now comes up at boot** (`net_boot()` in kernel.zl) — silent
  with no card, one line with one, a named message for each failure step, and
  `verify.sh` still matches golden.txt.
- Nothing is committed. ~8,300 insertions across 35 files, uncommitted.
- `kernel/hosttest/browsershot.ppm` and `wmshot.ppm` are tracked AND matched by
  `kernel/.gitignore`, so the rule is inert and every gate run makes a ~5 MB
  diff. Untracking them is a call for whoever owns those goldens.

### The HTTPS/JavaScript objective — audited in the kernel, and it passes

All five items were already wired; driving them in QEMU is what mattered.
`https` in the shell now reports **HTTPS OK: 2817 pixels, HTTP 200, body
120361 bytes, title: Wikipedia** over a chain-verified TLS 1.3 handshake.

That audit caught two regressions from this session's subresource work, both
fixed: the gates reported a working fetch as "HTTPS FAILED, browser status 8"
(the new `BR_IMAGES` state was not in their enumeration, and 8 is *success* for
a document gate), and they printed `HTTP 0 body 0` because they read `http.c`
while it was describing a stylesheet — `br_code()`/`br_doclen()` now carry the
document's own numbers.

**Trap that cost the most time: `./build.sh` rebuilds `kernel.elf`, but the
QEMU harness boots `zlOS.iso`.** Only `mkiso.sh` rebuilds that, and
`exercise.py`'s `build()` is what calls it. A probe that skips `build()` reads
a stale kernel and every conclusion from it is wrong.

### Closed this run

- **`probe-urlbar.py` mutation check** — pre-fix kernel FAILS it (832 vs 3512
  ink), fixed kernel passes, restore md5-verified. The replacement boundary
  must be `/* ---- keys ---`; cutting to `browser_url_focus` deletes
  `browser_key` and the link fails.
- **`x509_why()` lied on the SUCCESS path**, not the failure path — every
  non-matching root wrote its reason and a later success did not clear it, so a
  verified handshake reported "RSA signature but the issuer key is not RSA".
  Cleared at every accepting return. The test that exposed it was itself stale
  (it used google.com as "a CA we do not carry" after GTS Root R1 was added);
  it now narrows the trust store instead of trusting a third party's CA choice.
  `tlstest` 13/1 → **16/0**.
- **Alt+Tab had never worked on any keyboard.** `wm.c` kept a private copy of
  the keycode table, missing `KEY_TAB`, so the handler compared `code == '\t'`
  (9) against `KEY_TAB` (0x103). Deleted the copy, included `keycodes.h`.
  Gated by `wmtest` driving real PS/2 scancodes (its `idt_scan` was a constant
  0, which is why no keyboard path through `wm.c` was reachable from it),
  mutation-checked, and confirmed on the real machine over USB HID.

### Added this run, after the brief

- **Search from the URL bar** — words go to a search endpoint, addresses are
  fetched. Google itself works and returns **no document** (19 nodes of
  `<noscript>`; a mainstream User-Agent changes nothing).
- **External stylesheets are fetched.** `URL_MAX` 256 → 1024 and `http.c`'s
  `REQ_MAX` 512 → 1536, because Wikipedia's first stylesheet path is 522
  characters and `build_request` silently built a SHORTER request rather than
  failing — a request for a different resource, whose 404 reads as the
  server's fault.

### Deliberately refused, with reasons in the code

Pixel parity with Chrome; grid areas/spans/`order`/baseline alignment; APNG,
colour management and 16-bit PNG output; cross-host image fetches (needs a
second DNS state machine); `calc()`; `position: sticky` (lays out as relative).

---

# STATE — zl-main, the integration worktree

Updated 2026-08-18 ~21:10 local. (Box clock drifts; prefer
`gh api repos/RoyX4/zl-linux --jq .pushed_at` for real times.)

## Objective

Land eight parallel zlOS tracks onto `main` without silently losing work, and
without shipping the class of bug that merges clean and fails at runtime.

**Success metric:** `main` builds, links, and passes the boot gates, with a named
resolution recorded for all 55 multi-touch files.

## Where the increment stands

### Done and verified

- **Safety net complete.** 0 branches unpushed (was 10). 130 MB of per-worktree
  tars at `~/zlos-freeze-20260818-2054/`, a verified 13 MB bundle at
  `~/zl-freeze-20260818-2056.bundle`, 15 `prelanding/*` tags pushed, and 101
  uncommitted files captured as `refs/wip/*` commits (also pushed) **without
  touching any working tree**.
- **Evidence written.** `docs/MERGE-EVIDENCE.md` — 37 sessions re-read, every
  branch measured. Supersedes `docs/INTEGRATION-PLAN.md`.
- **L0 landed** — `claude/amazing-robinson-19793a`. Untracks `kernel/_genefi.c`.
- **L1 landed** — the 33-commit shared spine (`d61a481`). `main` now carries
  `kernel/wm.c`, `ui.c`, `ui.h`, `wmglue.c`, `term.c` for the first time.
- **L1 follow-up landed** — `da34635` cherry-picked. See TENSIONS T-1.
- **Seven tracks landed.** `main` 104 -> 152 commits. In order:
  `amazing-robinson`, the spine, `overnight-compositor`, `value-16`,
  `dma-map-hid-arena`, `quirky-pare`, `browser`.
  **overnight, value-16 and dma-map merged with ZERO conflicts** once the spine
  was in. Trunk-first ordering matters: landing `apps-in-windows` first (per the
  earlier plan's build-script argument) converted five clean merges into
  conflicts, and was rolled back. See T-7.
- **Hand-resolved:** `idt.c` (duplicate `idt_set_pointer_bounds`/`mouse_irqs`/
  `idt_mouse_irqs` and two clamp variable pairs collapsed to one),
  `HANDOFF.md` (both sides kept), `fb.c` (browser's synthesised rich text
  replaced by a shim onto this tree's real bold atlas — see T-8),
  `kernel.zl` (5 hunks; four pure unions, one hand-placed — see T-9).
- **Compile state:** `./build.sh`, `kernel/build{,64,efi}.sh` and
  `kernel/hosttest/build.sh` (17 harnesses) all pass. **No boot gate has run** —
  the box has been at loadavg 14-17 from Cursor and the gates guard at 4.0.

Effect on divergence, measured:

| Track | behind main's old base | behind main now |
|---|---:|---:|
| `lang/value-16` | 41 | 8 |
| `fix/dma-map-hid-arena` | 42 | 9 |
| `desktop/browser` | 44 | 11 |
| `desktop/apps-in-windows` | 43 | 13 |
| `desktop/overnight-compositor` | 48 | 15 |
| `desktop/feel-and-control` | 48 | 15 |
| `desktop/exec-track` (carries `system-track`) | 40 | 28 |

Most of the apparent 40–50 commit gap was shared history `main` did not have.

### In flight

- Boot gate running against `main @ 91bbad8` via `gates/land-gate.sh`. The
  compile steps pass; the QEMU steps are **waiting on the load guard** — the box
  is at loadavg 13.8, driven by Cursor, not by this work.

### Next, in order

1. Finish the gate on `main @ 91bbad8`. Do not advance on red.
2. Write the three checkers in `docs/MERGE-EVIDENCE.md` §3 (address table,
   symbol table, builtin CALL/REG table) **before** any further landing. They are
   the only detectors for the silent-failure class.
3. Rewrite `kernel/check-memmap.sh` — it iterates a hardcoded nine-name list with
   no `DISK_SCRATCH` and no discovery, so it cannot catch the collision it exists
   to catch.
4. **L2 — `desktop/apps-in-windows`.** Must precede the five tracks that append
   `gcc` lines to the build scripts, or their appends land and are then deleted.
   Requires adding 17 `.c` files to `kernel/SOURCES`.
5. L3 — `desktop/overnight-compositor`. The expensive one: `kernel.zl`, 13
   conflict hunks / 898 conflicted lines against apps.

## Constraints that bind this work

- 15 worktrees share one `.git`. Never run destructive git while another session
  holds a worktree.
- 8 cores / 15 GB, QEMU under TCG. Gates run sequentially and backgrounded,
  never alongside an agent fan-out. Guard on **available memory** as well as
  loadavg — the documented prior kill here was an OOM.
- `git add -A` in `zl-linux` would stage 226 MB of untracked `kernel/exercise-out`.
  Always use an explicit pathspec.
