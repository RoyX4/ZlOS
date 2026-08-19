# browser-storage-run — the arrays came out of BSS

The record of the run that worked from
[`BROWSER-STORAGE-PROMPT.md`](BROWSER-STORAGE-PROMPT.md), on branch
`desktop/browser-next` from `2e9ad77`. Every number below has the command that
produced it beside it, and every one was re-measured on this box rather than
copied out of [`browser-render-run.md`](browser-render-run.md).

**The one-line result:** a real page now parses whole. Nodes went from
`8192/8192 with 7,807 dropped` to `15,574/32,768 with 0 dropped`, and
`css_overflowed()` went from 1 to 0.

---

## 1. The corpus, and why it is reproducible

Every measurement uses the English Wikipedia article on Linux and the two
stylesheets it links, fetched fresh:

```
curl 'https://en.wikipedia.org/wiki/Linux' -o linux.html          982,395 bytes
# the two <link rel=stylesheet> hrefs out of it, in document order:
sheet1.css  271,848 bytes      sheet2.css  6,658 bytes
```

**271,848 is the same byte count `browser-render-run.md` §12 measured**, which
is the first thing that had to be true before any of the rest could be
compared. §11's "2 files, 278 KB" is 278,506 here.

The corpus is **not committed** — it is somebody else's content and would be
stale in a month. What is committed is the instrument.

## 2. `hosttest/parsestat.c`, because the last measurement was thrown away

Every cap figure in §11 and §12 came from a program that was not kept, and the
prompt's own instruction was "rebuild it rather than trusting the numbers".
That is the right instruction and a bad position to put someone in, so it is a
file now:

```
./parsestat page.html sheet.css [viewport]
```

It asserts nothing on purpose. Its right answer depends on which page you fed
it, so a "0 failed" line would be a claim about the web rather than about this
code. It measures; you read it.

**The baseline was taken against a pristine `git archive HEAD` tree**, not
against the working copy — the same discipline `hosttest/build.sh` records for
the stale `fuzz` link line, and for the same reason: a baseline compiled
against the new headers is not a baseline.

## 3. Before and after, measured

```
$ ./parsestat linux.html skin.css 1036
```

| | before (`2e9ad77`, pristine) | after |
|---|---|---|
| **nodes** | **8,192 / 8,192** | 15,574 / 32,768 |
| **nodes dropped** | **7,807** | **0** |
| text arena | **196,607 / 196,608** | 356,587 / 1,048,576 |
| rules taken | 235 | **410** |
| selectors | — *(no accessor existed)* | 614 / 4,096 |
| declarations | 741 | 1,263 / 32,768 |
| css string arena | 14,736 | 23,171 / 65,536 |
| **`css_overflowed()`** | **1** | **0** |
| runs | 9,886 / 12,288 | 13,795 / 65,536 |
| height | 10,017 px, 1,218 lines | 20,896 px, 1,421 lines |

Two things in that table are worth more than the rest.

**`css_overflowed()` 1 → 0 is the whole point of the exercise.** §12 predicted
375 rules taken at `MAX_SELS 4096` against sheet1 alone, and sheet1 alone now
gives **exactly 375**. The prediction was right, the mechanism was right, and
the only thing standing in the way was where the array lived.

**The run array was never the binding limit — it was next in line.** 9,886 of
12,288 looks comfortable until you notice it was produced by a parse that had
already thrown away 7,807 nodes. Raising the node cap without raising this one
would have moved the truncation rather than removed it, and the number to watch
for that is 13,795, not 9,886.

### And the BSS it gave back

The point of the exercise, in the one number `memmap.h` tells everyone to
check before adding an array. Clean rebuild — `rm -f _*.o kernel.elf` first,
one `build.sh`, because two concurrent runs share the same object files and
the artifact is then nobody's:

| | `__kernel_end` | headroom under `link.ld`'s 6 MiB | `.bss` |
|---|---|---|---|
| before | 0x005E1280 = 5.880 MiB | **126,336 B** | 3,354,624 B |
| after | 0x00416D70 = 4.089 MiB | **2,003,600 B** | 1,476,584 B |

**1,878,040 bytes — 1.79 MiB — of BSS came back, and the kernel's room to grow
went up 15.9x.** `memmap.h` said "149 KiB is the number to check before adding
any array anywhere in this kernel"; it is 1,956 KiB now, and that sentence has
been updated rather than left to be believed.

## 4. What moved, and the shape it took

The pattern is `png.c`'s and the argument is `png.h`'s: **the caller supplies
the storage.**

```c
void html_set_arena(void *nodes, int max_nodes, char *arena, int arena_bytes);
void css_set_arena (void *sels, int max_sels, void *decls, int max_decls,
                    char *arena, int arena_bytes);
void lay_set_arena (struct lay_run *runs, int max_runs);
```

`browser.c` hands over one slice of `memmap.h`'s new `HI_DOM` region;
`htmltest`, `csstest`, `fuzz` and `parsestat` hand over ordinary statics and
still need no kernel, no framebuffer and no fixed address. **No shipping file
grew an `#ifdef`**, which is the property `fbbench.c` states and the one that
mattered.

| cap | was | is | why that number |
|---|---|---|---|
| `HTML_MAX_NODES` | 8,192 | 32,768 | 2.1x the measured need, 2.25 MiB |
| `HTML_ARENA` | 196,608 | 1,048,576 | 2.9x the measured need |
| `CSS_MAX_SELS` | 384 | 4,096 | §12's measured requirement |
| `CSS_MAX_DECLS` | 3,072 | 32,768 | the same ~8-per-selector ratio |
| `CSS_ARENA` | 24,576 | 65,536 | **a struct field, not a budget — see below** |
| `LAY_MAX_RUNS` | 12,288 | 65,536 | 4.8x the measured need |
| `DOC_MAX` | 262,144 | 2,097,152 | the article twice over |

Total **10.50 MiB** on the 64-bit build, 9.75 on the 32-bit — they differ
because `struct lay_run` holds a pointer, which is exactly why `layout.h` sizes
its region from `sizeof` while `html.h` and `css.h` use asserted literals. The
larger has to fit. Region is 16 MiB, `_Static_assert`ed against the span, with
six more asserts that each sub-arena lands 8-aligned.

### Fail closed, and it is tested

All three headers promise the same thing in the same words: until the caller
hands over storage, the file does nothing and **says so**. `htmltest`'s
`t_no_arena` and `csstest`'s section 0 are the only moments in a process's life
when that can be checked, so they run first and never again.

This is not ceremony. The alternative design — a small built-in fallback array —
is what these files had, and it would make every other assertion in both
harnesses pass anyway, on a truncated document, silently. It is also a real
bounds check: without the guard in `html_parse`, `node_new` returns -1 for the
root, `push()` stacks -1, and the next `emit_text` indexes `nodes[-1]`.

## 5. `CSS_ARENA` is 65,536 and that is a ceiling, not a choice

`struct comp` holds each interned selector part as an **`unsigned short`
offset**, so no offset can exceed 65,535 however much memory the caller hands
over. Reserving the 512 KB that the other caps' ratio suggested would have been
458 KB of region `css.c` can provably never address — a number that lies in the
direction nobody checks.

Measured: 375 selectors intern 21,160 bytes, so 65,536 is room for roughly
**1,160 selectors' worth of parts** — three times the entire old `MAX_SELS`,
and 2.8x what the largest sheet anyone has pointed this at needs. So
`CSS_MAX_SELS 4096` is **not reachable on a sheet of distinct selectors**; the
arena fills first, at about 1,160.

That is written into `css.h` rather than left to be discovered. Both refusals
set `css_overflowed()`, so it fails the same visible way either way, and the
fix if a sheet ever needs more is one line: widen `struct comp`'s four offsets
to `int`, which costs `sizeof(struct sel)` 112 → 160 bytes and nothing else now
that the array is not BSS. It is **not** done here because no measured sheet
needs it, and a hot matcher is a bad place to change a field width on spec.

## 6. The bug raising `MAX_SELS` uncovered

**A stylesheet that exhausted the string arena was silently truncated.**
`css_overflowed()` — the entire mechanism for reporting "rules were refused" —
returned 0 while 1,558 rules were dropped.

`intern()` returns -1 when the arena is full. `parse_comp` turns that into
`return 0`, and `parse_sel` turns *that* into "drop this selector" — which is
the **same return value** it uses for `a:hover`, a deliberate refusal that must
*not* raise the flag. The two were indistinguishable.

Measured on the sheet `csstest` builds for exactly this case:

```
3,743 rules in    arena 65,535/65,536    2,185 selectors taken    overflow 0
```

**The gate did not catch it because it was passing for the wrong reason.** At
`MAX_SELS 384` the selector array always filled first and set the flag, so "a
sheet that exhausts the arena reports overflow" had been testing the *other*
array for as long as it had existed — and its own comment said "the
two-hundred-selector case, where the ARENA is what runs out". Raising
`MAX_SELS` is what made the arena bind first and the silence visible.

The flag now goes up in `intern()`, which is the only function that knows the
difference between "no room" and "not supported". Mutation-checked both ways:

| mutation | caught by |
|---|---|
| `intern()` stops flagging a full arena | **2** assertions |
| a policy refusal (`a:hover`) wrongly flags | **1** assertion |

## 7. There are FIVE fixed-address maps in this kernel, not two

The prompt says two. Found by grepping every 7- and 8-digit hex literal in the
tree — which is the point, because reading `memmap.h` would not have found any
of them:

| file | what | state |
|---|---|---|
| `memmap.h` | the high-RAM regions | the map |
| `kernel.zl` | the block at 32 MiB | declared, `check-memmap.sh` owns the detail |
| `virtio_net.c` | `NET_BASE 0x04000000`, 1 MiB | **undeclared** |
| `intel.c` | `edid_buf 0x0C980000`, 128 bytes | **undeclared, and overlapping** |
| `arena.c` | restated `HI_IMG_BASE` | **stale** |

**`virtio_net.c` is the one that nearly cost this change.** The browser's
storage was going to 64 MiB — the round, aligned, obviously-free address — and
64 MiB is where the network card's virtqueues and frame buffers live. Its own
asserts could not have said so: they compared against `NET_FLOOR 0x03000000`
and `NET_CEIL 0x08000000`, two hand-restated copies. `NET_FLOOR` names png.c's
*base*, so the assert read "I am above the picture arena" while comparing
against the address the picture arena starts at.

**`intel.c` is a real overlap, not a near miss.** `edid_buf` was `0x0C980000`,
which is 9.5 MiB into `fb.c`'s 16 MiB cached-blur arena (`HI_BLUR`). Neither
file knew; nothing failed to build, because nothing was checked. It bites only
on the machine that has both a panel to read an EDID from and a desktop drawing
blurs — the ThinkPad — and 128 bytes of a cached blur is a smear, not a crash,
so nothing would ever have reported it. That is the `i2c_hid.c` failure
`memmap.h`'s header opens by describing, verbatim, one driver over and two
reviews later.

**`arena.c`'s copy was green for the wrong reason.** It restated
`HI_IMG_BASE 0x02000000` with the comment "png.c's arena landed at 32 MiB" —
the *pre-fix* address, stale since the picture arena moved to 48. Its ceiling
assert kept passing because 24 MiB is under both numbers.

All four are in `memmap.h` now, and the ordering chain runs from 8 MiB to 256 —
it used to start at `HI_IMG`, so everything below the picture arena was outside
the only check there was.

**And "unbroken" was wrong when I first wrote it.** A review by a different
model family (Gemini 3.1 Pro via Cursor) checked that exact claim and found the
chain stepping straight over `HI_APSTK`, the SMP AP stacks at 168 MiB: the
region is *defined* in `memmap.h` but appeared **zero** times in the assert
list, so `HI_BACK < HI_SCHED` jumped the region between them. That is two of
the three things this file's own "ADDING A REGION" rule demands, i.e. the hole
the rule exists to describe — in the assert list of the file that states it.

Nothing was actually at risk: `fb.c:196` already asserts
`AP_STACK_BASE + AP_STACK_SPAN <= HI_SCHED`, which is the owning file doing its
half. The same review also claimed fb.c *fails* to verify that, and **that part
is wrong** — reproduced before repeating, per the cross-model rule, and it does
not survive a `grep`. One real finding out of five questions, and it was
against the one sentence I had least evidence for.

The two links are in now.

```
   8 MiB  arena.c       zl program arena      16 MiB
  32 MiB  kernel.zl     snake, fs, shell       1 MiB
  48 MiB  png.c         pictures + scratch     4 MiB
  52 MiB  intel.c       EDID scratch           1 page
  64 MiB  virtio_net.c  rings + frames         1 MiB
  80 MiB  browser.c     doc, tree, CSS, runs  16 MiB
 128 MiB  fb.c          the back buffer       48 MiB
```

## 8. Two harness bugs the caps exposed

Both are the `HTML_MAX_NODES` lesson — *a limit a test restates by hand is a
limit the test stops checking* — and both were already written down in this
repo before they happened again.

**`csstest`: `ok(css_overflowed(), "a sheet past the limits reports overflow")`
built 4,000 rules against a selector cap of 384.** At 4,096 the sheet *fits*,
so the assertion would have claimed overflow was reported when it was not. It
now sizes from `css_sel_cap()`. Three other checks read `<= 24576` and
`<= 3072` against arrays whose size is now the caller's; they read
`css_arena_cap()` and `css_decl_cap()`.

**`fuzz`: `CHECK(nodes <= 1024)` and `html_arena_used() <= 32768`** against
arrays of 8,192 and 196,608. A generated 8 KB document never came near either,
so both asserted nothing about the array and everything about the generator.

`fuzz` also links `css.c` (because `layout.c` calls `css_compute`) and was
never handing it arrays — so every `style=` attribute in the generated markup
would have silently applied nothing, and the fuzzer would have stopped
exercising the inline-style path while reporting the same number of checks.

## 9. `css_sels()` exists now

`css_rules()` counts rule *blocks*. `p, a, span { }` is one rule and three
selectors, and it is the **selector** array that refuses work — `css.c`'s own
comment says `MAX_RULES` "IS NOT ENFORCED ANYWHERE and never was". There was no
accessor for `nsels`.

So every table anyone has ever written about this engine compared "rules taken"
against a cap named `MAX_SELS`: two different counters. That is most of why
"235 rules" read like a stylesheet limit rather than an array limit for as long
as it did.

## 10. AND THE PAGE LOOKS WORSE. That is the real result, so it is here.

`browsershot` renders the article at three widths. Compared against the same
binary built from a pristine `git archive HEAD`, on the same input:

| | pristine `2e9ad77` | after |
|---|---|---|
| the banner | **"document truncated: larger than this kernel's buffer"** | none |
| sheet1, 271,848 bytes | **211 rules taken, `overflow 1`** | 375 taken, `overflow 0` |
| sheet2, 6,658 bytes | **0 rules taken, `overflow 1`** | 35 taken, `overflow 0` |
| runs at 760 px | 4,600 | 13,976 |
| height at 760 px | 23,469 px | 49,947 px |
| **how it looks** | **readable nav lists** | **columns squeezed to one letter wide** |

The pristine render is *nicer to look at*. It gets there by having thrown away
two thirds of the document and every rule in the second stylesheet.

**The cause is measured, not guessed.** The same new binary with
`ZLOS_EXTRA_CSS` unset — whole document, no external skin — renders the clean
readable lists again, at 16,944 runs. So the squeezing comes from **applying
more of the stylesheet**, not from parsing more of the document.

And that is exactly the gap `browser-render-run.md` §11 measured and this
change was never going to close. Counted again on today's sheet1:

```
2,434 selectors
  1,172 (48%)  use a pseudo-class or an attribute selector -> REFUSED by design
  1,200 (49%)  are type/class/id/descendant -> supported
     56        display:none declarations in the sheet
```

Half the skin cannot match. The half that can now *all* applies — including
every width, flex and grid rule that assumes the navigation it cannot see has
already been hidden. Before, `MAX_SELS` cut the sheet off before most of it
landed, and the accidental result looked tidier.

**So: capacity was the binding limit, this removed it, and the next limit is
now the visible one.** That is progress that looks like a regression on screen,
and reporting it the other way round would be the easiest kind of lie to tell
here. `BROWSER-STORAGE-PROMPT.md`'s "After this lands" list already names
pseudo-class selectors as item 1; this is what item 1 looks like from the other
side.

## 11. The gates

Every one green, and the two that grew did so because assertions were added,
not because a count moved on its own.

```
htmltest  399 (+15)   csstest 386 (+14)   pngtest 146   browsertest 124
fbtext     54   jstest    52   nettest  152   tcptest  110   dnstest   97
httptest   91   x509test  35   tlstest    6   ecdsatest 20   cryptotest 27
arenatest  62   wmtest ok  inputtest ok  systest ok
browsertest_san  124 under ASan+UBSan
fuzz             402,427 checks, 0 failed  (3000 iters, seed 1, ASan+UBSan)
memmap-guard-test.sh   20 passed, 0 failed   (was 10 passed 2 FAILED at HEAD)
check-memmap.sh        no overlaps in kernel.zl's block

./build.sh      32-bit multiboot   0 undefined symbols
./build64.sh    64-bit multiboot   0 undefined symbols, ELF64
./buildefi.sh   UEFI (clang/LLP64) BOOTX64.EFI, 1,493,504 bytes
./verify.sh     ok  kernel boots, shell responds, transcript matches golden.txt
```

**`verify.sh` is the one that matters most here** and it is the reason `HI_DOM`
is at 80 MiB rather than at some round address above `HI_BACK`. `HANDOFF.md`
records that no gate passes `-m`, so every gate boots QEMU's default 128 MiB —
which means **a new fixed buffer placed above 128 MiB is dead code that still
passes review**. 80..96 MiB is under that, so this region is backed by real
guest RAM on every gate run, and a boot that matches the golden transcript is a
boot that actually touched it.

**`memmap-guard-test.sh` was 10 passed, 2 failed at `2e9ad77`** — verified
against a pristine `git archive HEAD`, not the working copy, because a gate
that is red before you start is a gate you will blame yourself for. Both
failures were one cause: `HI_APSTK` (the AP stacks) was inserted between
`HI_BACK` and `HI_SCHED`, `BACK_LIMIT` became `HI_APSTK - HI_BACK` = 40 MiB,
and neither the break that perturbs it nor the literal that pins it followed.
So the script whose entire purpose is to prove `_Static_assert`s are not
decorations was printing its own failure message — *"the map was broken and
NOTHING complained"* — about a guard that worked. Fixed, and the script now
covers ten owners and ten breaks.

**One trap worth naming, because it bit four of the new cases at once:** gcc
escapes apostrophes in diagnostics. `"...into fb.c's back buffer"` is printed
as `fb.c\'s`, so a grep pattern containing the apostrophe silently never
matches and `expect_break` reports "the map was broken and NOTHING complained"
about a guard that fired perfectly. Stop the substring before the possessive.


---

## What this run should be judged on

Not "the caps are bigger". Caps are one edit. The three things worth checking:

1. **`css_overflowed()` is 0 on a real stylesheet**, which is the measurable
   claim the whole change was made for, and it reproduces §12's predicted 375
   exactly.
2. **The silent-arena-overflow bug**, which was invisible until the other cap
   moved, and whose gate had been passing for the wrong reason since it was
   written.
3. **The memory map is now one map.** Four regions that were not in it are, one
   of them overlapping a live neighbour.
