# One fact, written down eleven times

A run that set out to close two visible defects and spent most of its time on a
single structural one. Recorded because the pattern is not "someone was
careless" - every copy was correct when it was written, and each went stale
alone.

## The shape

zlOS's desktop reserves three edges of the screen: a raster strip on top, a foot
along the bottom, a register rail down the left. Those are three numbers.

They were written down **eleven times**, in four languages' worth of files, and
by 2026-08-27 no two sets agreed:

| where | what it said | what it is |
|---|---|---|
| `wm.c` RESERVE_TOP/BOT (before) | `UI_DP(t,48)`, `UI_DP(t,72)` | 30, 46 |
| `wm.c` toast_rect | `72 * t->scale` | 46 |
| `wm.c` rail | reserved nothing at all | 170 |
| `snap.c` | no side reserve | 170 |
| `toasttest.c` | `(48 * 2)`, `(72 * 2)` | 60, 92 at scale 2 |
| `toasttest.c` snap oracle | `snap_rect(...)`, no rail | rail is 340 |
| `wmtest.c` | `72 * th->scale` | 92 |
| `wmtest_feel.c` | `UI_DP(th,48)`, `UI_DP(th,72)` | 60, 92 |
| `wmtest_feel.c` expectations | `gx == 0`, `gw == W` | `gx == 340` |
| `verify-{raw,iso,efi,64}.sh` | `entries=62` | 64 |
| `write-app-manifest-boot-receipt.py` | `entries: 62` | 64 |

The version string was the same disease in a second organ: `"0.3"` six times in
`kernel.zl`, `"zl 0.1"` in the About pane, and `"zlOS 0.10"` in the shell's own
boot banner. Three different answers to "what version is this", all on screen at
once, one of them eighteen pixels from another.

## Why none of it was caught

Each copy is individually plausible. `72 * t->scale` in `wm.c` even carried a
comment explaining itself - "matching kernel.zl's dock_y()" - and `dock_y()` had
been deleted. A citation to a function that no longer exists reads exactly like
a citation to one that does.

The gates did not catch it because **the gates were among the copies**. A test
that restates the number it is testing cannot disagree with the code; it can
only disagree with reality, quietly, and it always loses that argument in
silence.

And one test was not running at all. `settingstest` stopped LINKING when
`uikit.c` gained `fb_text_rich`, and:

- `build.sh` has `set -e`, so it ABORTED there - every test declared after line
  120 was never rebuilt.
- the previous run's binaries were still on disk, so running them by name after
  a failed build reported "all passed" from code that predated the change.

That is not a hypothetical. It happened in this session, to me, and I reported
`uitest: 201 checks, 0 failures` from a stale binary. The numbers turned out to
be right. The method could not have told me if they were not.

## What replaced it

- `RESERVE_TOP/BOT/LEFT(t)` read the theme, and are now defined above their
  first use rather than 500 lines below it.
- Every test asks `ui_metric(UI_METRIC_*)` instead of restating.
- The four boot gates and the receipt writer derive the app count from
  `kernel/metadata/app-manifest.json`, the way they already derived the sha beside it.
- `kernel/src/core/version.h` holds the version and the architecture. The architecture
  was not merely stale, it was **build-dependent stated as a constant**: `"i386
  - ring 0"` in a file compiled into the 64-bit kernel too.
- `kernel/tools/checks/check-version.py` compares every version literal in `kernel.zl`
  against `version.h`. Validated in both directions - it rejects a planted
  `0.4` and passes the real tree.
- `kernel/tests/host/run-all.sh` deletes every binary FIRST, checks `build.sh`'s exit
  status, and reports NOT BUILT as a FAILURE. Its test list is read out of
  `build.sh` so a new test is covered the day it is added.

## The guard I wrote was blind to one of the six

Worth recording separately, because it happened *after* the lesson above and in
spite of it.

`check-version.py`'s first version examined only literals that NAMED the system
- `"zlOS 0.3"`, `"0.3 x86_64"`, `"zlos 0.3, x86_64, single image, "`. That found
the `"zlOS 0.10"` banner immediately, which felt like proof it worked.

It was skipping `kernel.zl:780`:

```
if slot == 8 { return "0.3" }
```

A registry slot table is close to the ideal hiding place for a stale version. It
is a column of unrelated short strings - `"wire"`, `"20x15"`, `"design.h"`,
`"exec"` - and nothing about `"0.3"` sitting among them looks like a claim the
system makes about itself. The guard was written, run, seen to pass, and could
not see one of the six literals it existed to check.

The fix is that a BARE dotted literal in code is a version. That is safe here
and it was measured, not assumed: the only other bare dotted literals in
`kernel.zl` are `"6.796"`, `"6.47"` and `"0.00"`, and all three are inside
comments.

Widening what a guard inspects is how a guard starts crying wolf, so
`check-version-selftest.py` pins all four directions and `hazard-scan.sh` §6
runs it beside the guard on every preflight:

| | |
|---|---|
| A | the real tree passes |
| B | drift in the bare slot-table literal is caught - **the case the first version failed** |
| C | a version named in a trailing comment is not a claim |
| D | a `#` inside a string does not truncate the line |

Every check in `GUARDS-THAT-DID-NOT-GUARD.md` was green at the moment it stopped
working. "It passed" is not evidence that it looked.

## The rule worth keeping

**A number that describes the system belongs in one place, and a test that
restates it has stopped being a test of it.** The corollary is the expensive
half: when the single place finally moves, everything that was quietly wrong
becomes loudly wrong all at once - eight failures across three binaries here -
and that noise is the system working, not breaking.
