# BROWSER-STORAGE-PROMPT — get the browser's arrays out of BSS

Written 2026-08-19 for a fresh session, on branch `desktop/browser-next` in
`/home/roy/Documents/repos/zl-browser2`. Read this, then
[`browser-render-run.md`](../../evidence/browser-render-run.md) §11 and §12, which are where
every number below was measured.

**This is one change, and every remaining rendering improvement is behind it.**

---

## The problem, measured

A real page — the English Wikipedia article on Linux, 596 KB over verified TLS
— exhausts the parser and the stylesheet engine at once:

```
nodes        8192 / 8192      1,673 dropped
text arena   196,607 / 196,608
MAX_SELS     384              css_overflowed() == 1
```

None of them can grow. The browser is already **1.95 MB of the kernel's
3.34 MB BSS**, and `link.ld` leaves this:

```
$ ./build.sh && nm kernel.elf | grep __kernel_end
__kernel_end = 5.878 MiB   ceiling 6.000 MiB   headroom 127,872 bytes
```

`sizeof(struct sel)` is **112 bytes**, so the selector array alone would want
458 KB to hold what Wikipedia's skin needs. There is no version of this that
fits in what is left.

**The consequence, and it is why this is the next job rather than a cleanup:**
`@media` evaluation landed this session and bought almost nothing — 235 rules
before, 235 after — because `MAX_SELS` stops the parse long before it reaches
most of the blocks. At `MAX_SELS 4096` the same sheet yields **375**. The
feature is written and gated and it cannot pay off until the storage moves.

---

## The pattern, already proven in this tree

`png.c` does exactly this and it works. **The caller supplies the storage:**

```c
void png_set_arena(unsigned int *px, int npx);   /* png.h */
```

- the kernel hands it a region from `memmap.h`
- the host harnesses hand it a static buffer, or `mmap` the same address
- **no shipping file grew an `#ifdef`**, which is the property that matters —
  `fbbench.c` states the rule and `browsertest.c`/`browsershot.c` follow it

`png.c` was written with a 2 MiB static arena first, and it would not link.
That is how the constraint was found; do not rediscover it.

---

## The increments, in order

### 1. `html.c` first — it is the array that runs out first

```c
void html_set_arena(struct node *nodes, int max, char *arena, int size);
```

~590 KB of BSS comes back immediately. `HTML_MAX_NODES` can then go to 32768
and `ARENA` to 1 MiB or beyond.

**Gate:** `htmltest` stays green (384 checks), and a real article parses with
`html_dropped() == 0` and the arena not full. `kernel/hosttest/parsestat`-shaped
measurement is in `browser-render-run.md` §11; rebuild it rather than trusting
the numbers.

### 2. `layout.c`'s `runs[]` and `css.c`'s `sels[]`/`decls[]`/`arena[]`

Same shape. Then raise `MAX_SELS` to 4096 and **re-measure Wikipedia's skin** —
that is the number that tells you whether this whole exercise worked. Expect
235 → 375 rules taken and `css_overflowed()` to go to 0.

### 3. `browser.c`'s `doc[]`

`DOC_MAX` is 256 KB and the article is 596 KB, so it truncates (honestly, and
it says so on screen). Raising it is free once the storage is not BSS.

---

## `memmap.h` — read its header before adding a region

It states the rule and this session broke it anyway:

> ADDING A REGION: put its base here in ascending order, add it to the ordering
> chain below, and have the owning file assert that its highest byte lands
> under the next base. All three, or the check has a hole in it.

**THERE ARE TWO FIXED-ADDRESS MAPS IN THIS KERNEL** and that is the trap:

| | |
|---|---|
| `memmap.h` | the high-RAM regions, checked by `_Static_assert` |
| `kernel.zl`'s block at 32 MiB | `SNAKE_X`, `SNAKE_Y`, `FS_META`, `FS_DATA`, `LINE_BUF`, `HIST_BUF` — `check-memmap.sh` derives these from the source |

The picture arena was placed at `0x02000000` because every assert in
`memmap.h` passed. It landed on all six of the above, and a 132×132 PNG from
any server would have written through the RAM filesystem. `check-memmap.sh`
had printed those exact addresses in the same session and nobody joined them
up. `memmap.h` now declares `ZL_LOW_BASE`/`ZL_LOW_END` so the compiler checks
across the boundary — **the assert exists now, keep it honest.**

Free space today: `HI_IMG` is 48–52 MiB, `HI_BACK` starts at 128 MiB. 52–128
MiB is unclaimed.

---

## Traps that will cost you an hour each

**`./build.sh` rebuilds `kernel.elf`. The QEMU harness boots `zlOS.iso`.**
Only `mkiso.sh` rebuilds that, and `exercise.py`'s `build()` is what calls it.
A probe that skips `build()` reads a stale kernel and every conclusion from it
is wrong. This cost the most time of anything this session: I added a debug
print, did not see it, and concluded the code was not executing when it simply
was not in the image. **`grep` the generated `_gen.c` and compare mtimes before
believing "the code is wrong".**

**A gate's own build is part of what has to be re-run.** `httptest` and `fuzz`
both had link lines that went stale when TLS landed and produced no binary at
all, while `browser-status.md` went on citing "91 checks, 0 failed". A gate
that cannot build looks exactly like one that passes, because `build.sh` prints
a screenful of successes around the failure.

**zsh does not word-split an unquoted `$VAR`.** `gcc $SRC` passes one argument.
Use `${=SRC}`. It silently produced six "BUILD FAILED" lines that were nothing
of the kind.

**`or` appears only in comments in `kernel.zl`.** Do not assume its precedence
against `==`; write two `if`s.

---

## The gates, all of which must stay green

```
htmltest 384  csstest 372  pngtest 146  browsertest 124  fbtext 54  jstest 52
nettest  152  tcptest 110  dnstest  97  httptest  91  x509test 35  tlstest  6
ecdsatest 20  cryptotest 27  wmtest ok  inputtest ok  arenatest 62  systest ok
fuzz ~400,000 checks/seed under ASan+UBSan
browsertest_san — the same 124 checks under ASan+UBSan
```

Then, one at a time and never beside an agent fan-out:

```
./verify.sh          BIOS boot vs the golden transcript
./probe-urlbar.py    click the URL bar, type, measure the ink
./verify-sources.sh  the one list reaches all four builds
```

`verify-sources.sh` is the one that matters most here: it proves a new `.c`
reaches the 32-bit, 64-bit, UEFI and raw-disk builds. It edits `SOURCES` and
restores it from an EXIT trap, so **do not run it while anything else is
building.**

---

## What NOT to do

- **Do not raise `MAX_SELS` "a bit" to fit the headroom.** 1567 is affordable
  today and it is a trap: it strands whoever adds the next array, and the
  number in `memmap.h` — *"127 KiB is the number to check before adding any
  array anywhere in this kernel"* — would have been spent by the person who
  wrote it down.
- **Do not put the arrays at a fixed address inside `html.c`/`layout.c`.**
  Their whole value is that they link and run as ordinary Linux programs with
  no framebuffer, no decoder and no kernel. `layout.c` reaches for exactly two
  things outside itself and that is deliberate. Inject the storage; do not
  hardcode it.
- **Do not fan out agents and run QEMU at the same time.** 4 usable cores.
  `CLAUDE.md` records an OOM kill of the agent process at 7.9 GB.

---

## After this lands

In order, each unblocked by the one before:

1. **Pseudo-class selectors.** 480 of the selector parts in Wikipedia's skin.
   Of 709 rules outside `@media`, only **160** have a selector this engine
   supports. Biggest correctness gap once the caps are gone.
2. **Cross-host subresources.** An `<img>` or stylesheet on another host is
   refused today, because it needs a second DNS lookup running underneath the
   first and the resolver's state machine is one lookup for one fetch. That is
   why most logos on a real page still show the placeholder box.
3. **`browsershot.ppm`, `wmshot.ppm` and three ELF binaries are tracked build
   outputs.** Every gate run dirties ~5 MB. `git rm --cached` is the fix and it
   touches shared history, so it is Zac's call, not a session's.
