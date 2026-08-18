# Merge evidence — what eight parallel tracks actually did, and what will bite

Written 2026-08-18 from a full re-read of 37 Claude Code sessions (140 MB of
transcripts) plus direct measurement of every branch. Supersedes
`INTEGRATION-PLAN.md`, which was written earlier against a base that turned out
to be the wrong one.

**Everything marked VERIFIED below was run as a command in this repo and its
output read.** Anything inferred says so. Numbers nobody measured are not here.

---

## 1. The structure is not what it looks like

Every `zl-*` directory under `~/Documents/repos/` is a **worktree of one git
dir** (`zl-linux/.git`). Not clones. 15 worktrees, one object store.

### `main` is not the parent of this work

This is the single most important correction, and it invalidates most
measurements taken against `main`:

```
git merge-base --octopus <all 8 tracks>   ->  d61a481
git merge-base --is-ancestor main desktop/overnight-compositor  ->  NO
git rev-list --left-right --count main...d61a481  ->  2  33
```

`44346d6` is the merge-base with `main`. It is **not** where the tracks diverged
from each other. They share a **33-commit spine** above `main`, and `main` sits
33 commits *behind* it with 2 doc commits of its own. `main` is a stale sibling,
not an ancestor.

Consequence: **any diff taken against `44346d6` overstates the work.** Two
findings that were wrong for exactly this reason:

| File | vs `44346d6` (wrong) | vs `d61a481` (real) |
|---|---|---|
| `kernel/wm.c` overnight | +1149/**-0** | +354/**-22** |
| `kernel/wm.c` feel | +1091/**-0** | +289/**-15** |
| `kernel/term.c` overnight | +277/**-0** | +84/**-18** |

`wm.c` does not exist at `44346d6` at all — it was created at `738f6ba`, inside
the spine. It is an ordinary hard three-way merge, not a free append.

### Eight tracks are really seven

```
git merge-base --is-ancestor desktop/system-track desktop/exec-track  ->  YES
```

`system-track`'s tip `038bd6d` is an ancestor of `exec-track`. zlfs, RTC,
clipboard, snapping, toasts, `verify-disk.sh` and `verify-clock.sh` all arrive
inside `exec-track` at zero cost. This is the only containment relationship —
all 56 ordered pairs were checked.

### Three `claude/*` branches are empty

`cranky-cray-b340d0`, `lucid-varahamihira-4cd47d`, `pensive-pike-2b5b01` are all
at `44346d6` with 0 files changed. Their sessions did real debugging; none of it
was committed.

### Bill of materials

151 distinct files touched vs `d61a481`. **96 single-touch, 55 multi-touch.**
Most contended: `mkdisk.sh` (7 tracks), then `wm.c`, `kernel.zl`, `fb.c` and
`runtime_kernel.c` at 6 each.

---

## 2. The landmines — merge clean, then fail

These are the reason this analysis was worth doing. Git resolves the loud
conflicts fine. These are the quiet ones: **two tracks solved the same problem,
named it differently, and git keeps both.**

### 2.1 `LINE_BUF` and `DISK_SCRATCH` at the same address — VERIFIED

```
claude/quirky-pare-05454c  kernel.zl:581   LINE_BUF     = 0x02030000
desktop/system-track       kernel.zl:1045  DISK_SCRATCH = 0x02030000
desktop/system-track       kernel.zl:643   LINE_BUF     = 0x02020000   <- never saw the move
```

quirky-pare moved `LINE_BUF` out of FS_DATA's slot 7. system-track independently
placed `DISK_SCRATCH` at the address quirky-pare moved *to*, reasoning correctly
against the layout it could see. The hunks are ~460 lines apart on branches with
different bases, so **a 3-way merge takes both with no conflict.**

zl has no static asserts. No build error, no link error, no gate fires. The
shell's line buffer and the disk scratch buffer share memory.

**Fix: set `DISK_SCRATCH = 0x02040000` at the exec-track landing.**

### 2.2 Two snake games — VERIFIED

```
overnight-compositor:  sn_rand  sn_reset  sn_event  sn_step  sn_draw
apps-in-windows:       snake_rand snake_start snake_key snake_step snake_draw
```

No name collision, so a union merge ships both. Same shape elsewhere: the resize
grip (`in_resize_grip` vs `in_grip`), the frame timer, window snapping (feel's
`wm_snap` vs system's `snap_zone_for_point`), and the console mute flag
(`console_mute`/`con_mute` vs `console_quiet`/`con_quiet`).

### 2.3 `wm_frame_us` defined twice with different types — VERIFIED

```
overnight  wm.c:1096   int wm_frame_us(void)          { return (int)frame_us; }
apps       wm.c:821    unsigned int wm_frame_us(void) { return ft_us; }
```

Different regions of one file, so a merge may take both — duplicate symbol at
**link** time. Worse than the link error: the bodies compute different things (a
rolling 16-frame average vs an instantaneous value), so picking either silently
changes what the number on screen means.

### 2.4 Four incompatible high-RAM memory maps

Not two. `fb.c` carries four mutually incompatible views of physical RAM:

| Map | Tracks | Shape |
|---|---|---|
| A | system, feel, browser, exec | `HI_BG` 0x08000000, `HI_BACK` 0x0C000000 |
| B | overnight, value-16 | `HI_BACK` 0x08000000, `HI_BLUR` 0x0C000000, `BACK_LIMIT` 48 MiB |
| C | apps | `HI_BACK` 0x08000000, `HI_APSTK` 0x0A800000, `BACK_LIMIT` 40 MiB |
| D | dma-map (`kernel/memmap.h`) | HI_BACK 128 / HI_HID 184 / HI_BLUR 192 MiB |

Mercifully, A and B *renamed* regions, so most bad mixes fail loudly on an
undefined macro. Two that don't:

- **`STACK_BASE = 0x0A800000`** (168 MiB) is set in `smp_trampoline{,64}.S` on
  all eight branches and appears in **no** map except apps's `HI_APSTK`. Maps B
  and D let the back buffer span 128–176 MiB, putting the AP stacks inside it.
  The `_Static_assert`s in every `fb.c` compare only constants that file knows
  about; `STACK_BASE` is not one, so the check reads as coverage without being it.
- **`intel.c`'s `edid_buf`** is still hardcoded at `0x0C980000` on dma-map —
  9.5 MiB inside that same branch's own `HI_BLUR` arena. dma-map fixed the HID
  half of its bug and left the EDID half. `intel.c:763` already exposes
  `intel_set_edid_buffer()`, so the fix is one call.

### 2.5 The build model is a decision, not a conflict

`apps-in-windows` **rewrote** all four build scripts to read one `kernel/SOURCES`
file (`build.sh` +20/**-46**, `build64.sh` +17/-45, `mkdisk.sh` +19/-46) while
five other tracks append `gcc` lines into exactly the region apps deleted.

Keeping apps's scripts — which is right — silently drops **17 new `.c` files**
from the build unless they are added to `SOURCES`. No build error, no link error
if they are self-contained:

```
clip.c fs.c notify.c rtc.c snap.c        (system-track, also via exec-track)
settings.c                                (feel)
browser.c dns.c html.c http.c layout.c net.c tcp.c virtio_net.c   (browser)
arena.c exec.c interp_kernel.c            (exec-track)
```

This also forces the ordering: **apps must land before the five tracks that
append**, or their appends land and are then deleted.

### 2.6 `kernel.zl` — the one genuinely expensive object

overnight and apps both rewrote **12 of the same base functions**: `draw_screen`
(base 8 lines → overnight 245, apps 11), `run_command`, `wm_session`,
`app_event`, `app_draw`, `app_tick`, `draw_window`, `draw_desk`, `about_body`,
`sysmon_body`, `key_full`, `open_menu`. Overnight adds 32 new functions and
deletes 7; apps adds 23 and deletes 8.

No ordering makes this cheaper. It is a two-design collision resolved by picking
a trunk and hand-porting the other side's apps.

One trap inside it: **`fn ui()` is `ui_scale()` on overnight and `cell_w()/8` on
apps.** Taking apps's hunk compiles, links, boots, and silently restores the
"everything is tiny at 4K" regression overnight diagnosed and fixed.

### 2.7 `Value` shrinks 64 → 16 bytes on `lang/value-16`

The guard rails hold — `compilel.c`'s `VALSZ` is updated and `runtime.c`
static-asserts it. The hazard is the **new rule** the anonymous union creates:
reading a non-active member now yields garbage where it used to yield zero. Every
builtin added by another track (browser +299, exec +321, apps +145, overnight
~27) must check `a[i].type` before reading `a[i].num` or `a[i].str`.

There is no test for this. It is a read-every-new-builtin job.

### 2.8 zl call sites can outlive their builtins

overnight **removed** the `bg_snap`/`bg_rest`/`grab`/`stamp` builtins and the C
behind them. Any surviving `kernel.zl` hunk that still calls them fails at
**runtime**, not build time. zl has no compile-time check that a call site
resolves to a registered builtin.

---

## 3. The three checkers to write before merging anything

Each turns a silent failure into a loud one. All three were independently named
"best idea on the board" by the adversarial reviewers.

1. **Address table** — sweep every `NAME = 0x...` in `kernel.zl`, every
   `#define HI_*`/`BACK_LIMIT`/`SP_LIMIT` in `fb.c` and `memmap.h`, `STACK_BASE`
   in `smp_trampoline{,64}.S`, `HID_BUF` in `i2c_hid.c`, `edid_buf` in
   `intel.c`. Sort by address, flag repeats. Catches §2.1 and §2.4.
2. **Symbol table** — every top-level definition per file per head. Two scans:
   same name twice, and same purpose under different names. Git cannot see the
   second column at all. Catches §2.2 and §2.3.
3. **Builtin CALL/REG table** — every call token in `kernel.zl` vs every
   `streq(name, "...")` in `freestanding/runtime_kernel.c`. Catches §2.8.

`kernel/check-memmap.sh` as it stands **cannot** catch §2.1: it iterates a
hardcoded nine-name list (`SNAKE_X SNAKE_Y FS_META FS_DATA FS_SLOT LINE_BUF
LINE_MAX HIST_BUF HIST_N`) with no `DISK_SCRATCH` and no discovery. Replace the
list with a sweep before trusting it.

---

## 4. Ordering, and the honest cost

Ordering was measured four ways: 163 / 181 / 185 / 190 conflicted-file events.
**Ordering buys ~10%.** It is not the lever. The lever is resolving the memory
map and the build contract as their own steps before any file that reads them.

The order that does matter, and why:

| # | Land | Why here |
|---|---|---|
| 0 | `claude/amazing-robinson` | untracks `_genefi.c`; removes a conflict source from every landing behind it |
| 1 | **the spine** (`git merge d61a481`) | **zero conflicts, measured** — every track already contains it. Brings `wm.c`/`ui.c` to `main` for the first time |
| 2 | `apps-in-windows` | the `SOURCES` build model must precede the five tracks that append |
| 3 | `overnight-compositor` | `kernel.zl` — the expensive one |
| 4+ | value-16, dma-map, lewin, feel, quirky, exec(+system), browser | |

**Steps 0–2 are cheap and mechanical.** Step 3 is the monster and it is one file.

### On the effort estimate

The synthesised plan's summary says 7 days. Summing its own phases gives ~15.
The estimate is internally inconsistent and neither number was measured — treat
both as guesses. What *is* measured: step 1 has zero conflicts, and `kernel.zl`
has 13 conflict hunks and 898 conflicted lines between overnight and apps.

### What is not verified

- **Nothing has been merged or built.** Every number here is `git merge-tree` /
  `git merge-file` / `git diff` — text-level only.
- Total sequential cost is **not** measured. Conflicts accumulate as landings
  occur, so the real figure is above the sum of pairwise numbers. That sum was
  not run and is not quoted.
- The three checkers in §3 are specified, not written.

### Weakest link

`kernel.zl`. 898 conflicted lines in a language with no compiler, no linker, no
type checker and no static asserts. The entire verification surface is the
CALL/REG table and booting the ISO — and `./build.sh` makes `kernel.elf` while
every `probe-*.py` boots `zlOS.iso`, which only `mkiso.sh` makes. That mismatch
already cost one session three consecutive wrong diagnoses.

Every other hard part has a mechanical backstop. This one has a person reading
carefully.


---

## Outcome (2026-08-19)

**All eleven tracks are on `main`.** 104 → 214 commits, pushed as a
fast-forward. Gate green in one uninterrupted pass: toolchain, kernel 32/64/EFI,
SOURCES coverage, hosttest build, reverse-SOURCES sweep, `mkiso`, and every boot
gate — `verify.sh`, `verify-iso.sh`, `verify-efi.sh`, `verify-raw.sh`,
`verify-disk.sh`, `verify-clock.sh`. 26 harnesses pass, 0 fail.

Landing order, and what each cost:

| # | Track | Conflicts |
|---|---|---|
| 1 | `claude/amazing-robinson` | clean |
| 2 | **the 33-commit spine** (`d61a481`) | clean — and it collapsed everything else |
| 3 | `desktop/overnight-compositor` | clean |
| 4 | `lang/value-16` | clean |
| 5 | `fix/dma-map-hid-arena` | clean |
| 6 | `claude/quirky-pare` | 1 file |
| 7 | `desktop/browser` | 2 files |
| 8 | `claude/ecstatic-lewin` | 5 files |
| 9 | `desktop/feel-and-control` | 13 files |
| 10 | `desktop/exec-track` (carries `system-track`) | 14 files |
| 11 | `desktop/apps-in-windows` | **47 hunks** |

**Landing the spine first is the single decision that mattered.** Before it,
sequential merging measured 163–190 conflicted-file events. After it, five
tracks merged with zero conflicts. The 163 was measured against a `main` that
was missing 33 commits every branch already had.

Trunk-first also beat build-model-first: landing `apps-in-windows` early (per
this document's own earlier reasoning about `SOURCES`) converted five clean
merges into conflicts and was rolled back.

### The landmines, and which were real

Every silent-collision class predicted in §2 occurred. Counted:

- **Eight app-id collisions.** `APP_SNAKE`/`APP_BROWSER`, `APP_SNAKE`/`APP_SETTINGS`
  (in `wmglue.c`, whose own comment says zl and C apps share one namespace),
  `APP_SNAKE`/`APP_RUN`, and four at once from apps. Every one silent: two arms
  of one dispatch matching the same id, the second unreachable.
- **Two snakes**, resolved — `sn_*` kept, apps's `snake_*` (104 lines) deleted.
- **`wm_focus` meant opposite things** on `main` and `exec-track`: getter here,
  setter there. A one-argument call to a zero-argument builtin, in a language
  with no arity check.
- **`console_mute` vs `console_quiet`** across three layers — two flags, *both*
  builtins registered, `kernel.zl` calling both.
- **The scroll wheel had to be taken as a unit**: the knock switches the device
  to 4-byte packets while the reader framed on 3.
- **`LINE_BUF`/`DISK_SCRATCH` at `0x02030000`** — predicted, and it did not fire,
  because `quirky-pare` landed before `exec-track` and the address was set to
  `0x02040000` at the exec landing.
- **`HI_APSTK` was missing from `memmap.h`**, so `BACK_LIMIT` spanned 128–176 MiB
  while `STACK_BASE` sits at 168 MiB. The framebuffer back buffer would have
  grown over the stack of every application processor. Only `apps-in-windows`
  had noticed. Now declared, with three `_Static_assert`s.

### The class this document under-weighted

Deletions that keep their callers. Not a conflict, not a duplicate — a merge
takes one side's removal and the other side's use, and the result compiles or
doesn't depending on luck:

- `fb.c`'s entire band-parallel block (91 lines) removed, four call sites kept
- `editor_key` given apps's signature over `main`'s body, calling a
  `redraw_editor` apps had deleted
- `verify-raw.sh` losing `OUT=$(mktemp)` while two uses remained, under `set -u`
- apps's static-desktop mouse loop (182 lines) surviving into `main`'s text
  shell, calling functions over globals that do not exist here

### `kernel/check-zl-calls.sh`

Written during the exec landing, because zl has no compile-time check that a
call site resolves. It found one on its first run and it is **not merge
damage**: `key()` has been called and defined nowhere on every branch since
`b55f3f9`, on the panel-handover path immediately after *"press a key to move
the console onto it"*. Tracked in the script's header rather than guessed at.

### Two gate defects found by using them

- **A gate that could not fail.** The first `land-gate.sh` piped every step
  through `tail`, so `$?` was always `tail`'s. It reported green on a tree with
  11 undefined references.
- **A gate that assumed the other design.** `verify-iso.sh` (from apps) greps
  for `compositor: [1-9]`. On apps that marker is emitted at boot because apps's
  boot path *is* `wm_session()`; here `wm_boot_start()` opens the windows and
  `wm_session()` is only the `w` command. Neither branch could catch this alone.

### Still open

- `key()` — see above. One line, needs someone who knows which builtin was meant.
- `font_big.c`, `icons_rgb.c` — referenced by nothing at all.
- The eight `prelanding/*` tags are the rollback points and are pushed. Three
  extra worktrees (`zl-merge`, `zl-apps-merge`, and the six `claude/*`) can be
  pruned once the tree has been used for a while.
