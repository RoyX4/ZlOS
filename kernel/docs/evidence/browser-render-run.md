# browser-render-run — making pages look right

The record of the run against [`BROWSER-RENDER-PROMPT.md`](../archive/prompts/BROWSER-RENDER-PROMPT.md),
on `desktop/browser-next`. Written as it happened; every number here is a
measurement with the command that produced it, not a recollection.

Order of work was the brief's own: **the URL bar first, because it blocked
hand-testing, then images, flex, grid and more CSS.**

---

## 1. The URL bar — the diagnosis in the brief was wrong

The brief said the bug was NOT diagnosed and named two hypotheses, both about
the keyboard: either the earlier characters never reached `browser_key`, or
something re-armed the select-all. **Neither is what happened, and the symptom
it reasoned from was a coincidence of one URL.**

`browser_click` had **no URL-bar hit test at all**. Clicking the bar did
nothing, so the bar was still unfocused when typing began. Every character then
fell through `browser_key`'s *unfocused* switch — which drops anything that is
not a shortcut — until the string's first `l`, because

```c
case 'l': case 'L': url_focus = 1; url_sel_all = 1; return 1;
```

is the focus shortcut. That `l` was swallowed arming select-all, and the very
next character cleared the buffer. So `https://en.wikipedia.org/wiki/Linux`
leaves the tail after its first `l` — and in that URL the first `l` is the **L
of Linux**, giving exactly `inux`.

"The survivors are the characters after the first SHIFTED one" is true of that
string and of nothing else. It is also the reason the bug survived: it points
at the keyboard layer, where nothing is wrong.

**A second bug fell out of the same read.** `wm.c` delivers `EV_MOUSE` on the
press, on *every motion sample while the button is held* (the `GRAB_APP` path,
which exists so a slider keeps working when the pointer leaves it), and again
on the release with an empty mask — and `br_click` was never given the button
mask, so it could not tell them apart. Every link click navigated at least
twice and pushed duplicate entries onto an eight-slot history.

### Fixed

- `browser_click(cx, cy, btn)` — the mask now goes through `kernel.zl` and
  `runtime_kernel.c`, and the app differences the press edge itself.
- A click in the URL bar focuses it and selects all, as the keystroke did.
- Back is a button, not only a keystroke. It was drawn as one from the first
  commit and reachable only from the keyboard.
- A click anywhere else defocuses. Not politeness: with the bar focused every
  key is text, so a bar that keeps focus leaves PgDn and the arrows dead with
  nothing on screen saying why.
- The chrome rectangles are recorded **at paint time** and read back by the hit
  test, which is the rule `browser_link_at` already stated for `view_x/view_y`
  and for the same reason: the status strip moves everything below it by a
  variable amount.

### The evidence

`browsertest`'s `t_chrome_click` types a URL after a **click**, with an `l` in
the middle. Against the pre-fix `browser_click`:

```
82 checks, 10 failed
  FAIL  a press in the URL bar was not handled
  FAIL  clicking the URL bar did not focus it
  FAIL  nothing was fetched: the characters typed before the URL's first 'l'
        were dropped, which is the bug this test exists for
  FAIL  one click on a link unwound 2 history entries, wanted 1 - two means
        the release navigated as well
  ...
```

**`browsertest` could have caught this all along**, and that is the more useful
lesson. The harness was never the limitation — *every* existing URL-bar test
pressed `l` first, so all of them focused the bar by the one route that worked
and not one could observe the route that did not. That is exactly the failure
`browsertest.c`'s own header warns about: a test that cannot observe the
property it names reports PASS.

### And the gate that drives the real machine

`probe-urlbar.py` boots, reads the browser's client rect off the compositor's
serial log rather than guessing coordinates (`probe-drag.py`'s literal default
was once off the right edge of the screen, so the press landed on nothing and
it reported a working drag as a no-op), clicks the bar with an absolute tablet
event, types the address, and measures the ink.

**Its assertion is self-calibrating and that is the point.** "The ink is at
least N pixels wide" needs an N, and an N is a guess about a font at a
resolution on a day — the exact kind of number `CLAUDE.md` records costing this
project a bisect. So it types the whole address, measures, then presses
BackSpace until four characters remain and measures again, **in the same boot
with the same font**. A working bar gives the long string several times the ink
of the short one; the bug gives them the *same*, because with the bug the bar
only ever held four characters.

**It took four wrong runs to get there, and each was wrong in a way worth
keeping**, because every one of them was a measurement that looked like a
result:

| run | what it reported | what was actually true |
|---|---|---|
| 1 | "the compositor reported no window rects" | `wm_report`'s loop was only on the `w`-command path, which nothing with a framebuffer reaches. It had never run on a shipped boot, so **every pointer gate reading those lines was aiming at nothing** — the same "reachable from the text shell and from nothing else" regression `browser-status.md` records for the network gates |
| 2 | the whole address typed, and the gate failed | It typed into the **shell**. The browser is the last of four windows and sits entirely inside the shell's rect, so the press landed on the shell. Good proof that keystroke delivery works; no proof of what was asked |
| 3 | identical title-bar ink for all four Alt+Tab rotations | `ink_extent` returned `None` every time, because the title rect was computed from a regex group that had been discarded — it came out zero-tall. **A measurement that returns the same sentinel for every input is not a measurement** |
| 4 | ink extent 790 px for 35 characters *and* for 4 | The extent was measuring the bar's own right-hand border, which is ink at a fixed column. The number was real, reproducible, and about the chrome |

The fix for #2 is a pointer drag rather than Alt+Tab: a full Alt+Tab rotation
produced identical ink four times, so the modifier is not reaching `route_key`
on this machine. That belongs to the input track, and **a gate that depends on
a second subsystem's modifier handling is a gate that will fail for the wrong
reason** — so it moves the shell with a drag, which `probe-drag.py` already
proves works here.

The fix for #4 is the ink **count**, which is proportional to the text where the
extent is not. Measured: 876 ink pixels at 4 characters, 2,767 at 35 — about 61
pixels per character over ~630 of fixed chrome. Linear, and the constant does
not swamp it.

```
  browser client rect 242,248 1036x702  title 242,192 1036x56
  moving the shell's title bar 82,104 1236x56 out of the way
  35 characters -> ink reaches 790px, 2767 pixels
   4 characters -> ink reaches 790px,  876 pixels
  ok   35 typed characters put far more ink in the bar than 4  2767 vs 876
  urlbar gate ok: a click focuses the bar and every character typed arrives
```

~~**What this gate has NOT had is a mutation check against a pre-fix kernel.**~~
**Closed — see §10.** It was the weakest claim in this section and it is now
demonstrated rather than argued: the pre-fix kernel FAILS this gate (832 ink
pixels against 3,512), the fixed one passes, and the restore was md5-verified.

---

## 2. A gate that could not run at all

`build.sh`'s `httptest` line was never updated when the TLS transport landed in
`http.c`, so the harness had **18 undefined references and produced no binary**.
Confirmed pre-existing by rebuilding `HEAD`'s `http.c` against `HEAD`'s
`httptest.c`: the same 18.

`browser-status.md` cited "91 checks, 0 failed" for it throughout. That was true
when written and had not been true since — **a citation that aged into a
falsehood without a line of the file changing.**

A gate that cannot BUILD looks exactly like a gate that passes, because
`build.sh` prints a screenful of successful builds around the failure. That is
the same shape as the `gpu_fillrate` error this script's own comments already
record surviving at the end of a successful run.

Fixed by linking `tls.c crypto.c x509.c ecdsa.c rsa.c roots.c entropy.c
hostmachine.c`. It builds, and reports 91 checks, 0 failed against the current
`http.c` including this run's changes to it.

**The reusable part: a gate's own build is part of what has to be re-run.**

---

## 3. The link ceiling is 149 KiB away, and that decided the image design

`link.ld` asserts `__kernel_end` stays under `0x00600000`. Measured with
everything in:

```
__kernel_end = 0x005DAAC0 = 5.854 MiB   ceiling 6.000 MiB
HEADROOM: 152,896 bytes = 149.3 KiB
text 1,750,279   data 332   bss 3,339,328
```

A decoded picture is four bytes a pixel and a page has several, so a pixel
arena is **megabytes**. A static 2 MiB array in `png.c` could not have linked,
and the error it produced would have read *"the kernel image has grown into the
raw-boot stack at 6 MiB"* — naming the stack rather than the picture that took
the space.

So the pictures live in the fixed high-RAM map, exactly as `fb.c`'s back buffer
and every DMA arena do. `memmap.h` gained a region, with the four things that
file's own rule demands — a base, a row in the table, an ordering assert, and an
assert from the owning file:

```c
#define HI_IMG          0x02000000UL          /* png.c - decoded pictures   */
#define HI_IMG_SCRATCH  (HI_IMG + 0x200000UL) /* browser.c - base64 scratch */
#define HI_IMG_END      (HI_IMG + 0x400000UL)
```

**And that exposed a second, quieter problem.** `arena.c` asserted the program
arena ends below `HI_BG` at 128 MiB. That assert still *passed* — 24 MiB is
under 128 MiB — but it now named a region two regions away, because the picture
arena at 32 MiB had become the real neighbour above it. **A ceiling test that
names the wrong neighbour keeps passing while the actual neighbour is
overrun**, which is precisely the class of hole `memmap.h` was written to end.
It now asserts against `HI_IMG`. `arenatest`: 62 checks, 0 failures.

No `#ifdef` was added to any shipping file. The harnesses `mmap` the same
address the kernel hardcodes, which is the rule `fbbench.c` states.

---

## 4. Images

`png.c` is a complete still-image PNG decoder — DEFLATE (stored, fixed and
dynamic Huffman) with its own 32 KiB window, the five scanline filters, colour
types 0/2/3/4/6, bit depths 1/2/4/8/16, `tRNS`, and Adam7 interlacing. No heap,
no libc, no floating point.

`html.c` gained `html_src`, `html_attr_w` and `html_attr_h`: an `<img>` kept
only its `alt` in the shared href slot, which was right while an image was a
box with a caption and stops being right the moment the picture is fetched.

`browser.c` owns the policy: a `data:` URI is decoded inline with no network at
all, and network subresources are fetched **strictly one at a time after the
document**, because `tcp.c` holds exactly one connection. A picture that has
not arrived lays out at its declared size and paints the same honest empty
frame it always did, so nothing on the page moves when it lands.

`http.c` gained a per-fetch accept mask. Widening the type check to always
allow images would have meant a *document* fetch silently accepting a JPEG and
handing 40 KB of binary to the HTML parser; `http_reset()` restores the strict
default, so forgetting to call `http_accept` fails closed.

`fb.c` gained `fb_image` — the first rectangle of real pixels this kernel has
ever drawn; every other path draws a glyph, an icon or a solid. It **averages
when downscaling**, because a photograph arriving in a third of its width
through nearest-neighbour does not read as "smaller", it reads as noise. The
average is **weighted by alpha**, which is what stops transparent black — what
encoders routinely leave behind soft edges — dragging every edge toward black.

### Verification, independent of the harness that shipped with it

A decoder tested only against files made by its own author agrees with its
author. So, separately from `pngtest`:

**A colour-type and bit-depth matrix, encoded by python's `zlib`**, decoded
through `png.c` under ASan+UBSan. Every case correct: five grey depths
(1/2/4/8/16), RGB 8 and 16, palette 1/2/4/8 with `tRNS` transparency, grey+alpha,
and RGBA. And the strongest single result —

> **the Adam7-interlaced file decodes pixel-identical to the non-interlaced one.**

That invariant is easy to state and very easy to get wrong.

**90,000 mutated PNGs**, six seeds, under ASan+UBSan, with **every chunk CRC
repaired after mutation** — without that repair the integrity check rejects
14,995 of every 15,000 inputs and the fuzz only ever tests the CRC, never the
inflate, the Huffman tables or the filters where an out-of-bounds actually
lives. No fault, no accepted-image with inconsistent geometry, no failure that
forgot to set a reason.

*(That fuzzer's first version crashed — in **its own** CRC repair pass, where
`n - i - 12` went negative and the cast to unsigned made the bound check pass.
ASan reported it inside the harness, which looks exactly like a crash in the
decoder until you read the stack.)*

`fb_image` has nine assertions in `fbtext` (45 checks → 54). **Five deliberate
mutations, all caught:** pure nearest-neighbour downscaling (2 assertions —
a checkerboard must go grey), an unweighted colour average (1 — the halo),
dividing before multiplying in the source mapping (1 — the blank stripe),
ignoring alpha (2), and removing the clip (SIGSEGV — the clip is load-bearing).

### End to end

`browsertest`'s `t_images` loads a page carrying a genuine 424-byte RGBA PNG as
a `data:` URI, produced by an unrelated encoder, with transparent corners and an
opaque centre. The whole pipeline runs with no machine on the other end of a
wire: `html.c` parses the `src`, `browser.c` base64-decodes it, `png.c` inflates
and un-filters it, `layout.c` sizes the run to its intrinsic 32×32, and the
paragraphs either side survive. Eight malformed `data:` URIs — bad base64, empty
payload, no base64 marker, valid base64 that is not a PNG, a signature with
nothing after it — each degrade to the placeholder box without touching the
page, and a good image still decodes after all of them.

The same image is now **on the home page**, which is the gate document. A
feature the visual gate does not exercise is a feature the visual gate cannot
catch a regression in — and this browser has already shipped two text
regressions with every assertion green because the one thing that would have
shown them was a picture nobody diffed.

---

## 5. CSS, flex and grid

`css.h`'s scope paragraph said float, positioning, flexbox and grid were
"outside the bounded version of this capability". **That was the same mistake
`browser-status.md` documents about "a browser is unbounded", one level down:
it named the maximal version of a capability and reported it as the
capability.** Flex and grid have specifications. What is genuinely unbounded is
pixel parity with Chrome, and that is still refused.

Both headers keep the original text, struck through and annotated, rather than
deleting it — the shape of the mistake is the reusable part.

`layout.c` grew from 820 lines to 2,348. Its header keeps the old
"TWO BOX TYPES, and no more" paragraph struck through, and names the specific
error: *"it declared a boundary where there was only an ABSENCE. Nobody had
written flex; that is not the same claim as flex being out of reach."*

### The design that made it fit with no heap

The obvious implementation of flex is "build a box tree, measure it, position
it", and a box tree needs an allocator this kernel does not have. It is also
not needed, because **runs are pure position data in a flat array**:

```c
int start = nruns;                     /* lay the child out at the origin */
...                                    /* however complicated it is       */
int end = nruns;                       /* runs[start..end) are its runs   */
measure_range(start, end);             /* its bounding box, for free      */
translate_range(start, end, dx, dy);   /* and now it is somewhere else    */
```

When a flex item's final main size differs from the size it was measured at,
`nruns` is truncated back to `start` and the item is laid out again. Re-layout
is cheap and exact where a heap is not available at all, and it composes
recursively — so nested flex inside grid inside flex needed no code of its own.

The cursor was twelve globals saved and restored around every child layout,
which is a bug waiting for the thirteenth to be forgotten in one of the
restores. It is now split by **lifetime**: `struct flowpos` advances and is
never restored, `struct flowenv` is scoped and is always restored as one
assignment. `env = save` cannot forget a field.

### What it refuses, and says so

Grid named lines, `grid-template-areas`, spans and explicit placement (`css.h`
carries no field for any of them, so they cannot reach `layout.c` at all);
`order`; `align-content` on a multi-line container; baseline alignment (treated
as flex-start — one glyph atlas means every baseline is the same fraction of
the line box, so a true baseline pass would agree with start most of the time
and be subtly wrong the rest); `inline-flex`/`inline-grid` lay out as
block-level; `position: sticky` as relative and `fixed` as absolute.

**And no horizontal scrolling, ever** — which is load-bearing. `emit_pre`
already argued it: a line past the right edge is not *preserved*, it is
invisible with nothing on screen to say so. So a box wider than its container
is clamped, flex items that cannot shrink are scaled down anyway, and grid
tracks that do not fit are scaled. Each is a deliberate departure from CSS made
for that one reason, and the harness asserts the invariant it buys: **no run
outside the content box.**

### CSS

`css.c` grew 704 → 1,592 lines; `csstest.c` 328 → 1,082. Two limits were raised
with a measured cost, not a guess: `MAX_DECLS` 1024 → 3072 and `ARENA` 12288 →
24576, **+24,576 bytes of BSS measured with `size` on `css.o` built at the
kernel's own flags**. One ordinary modern rule expands to 19 declarations, not
8, so 1024 would have made the declaration array the first limit to bite — and
it bites by refusing the rest of the sheet.

Two real bugs were caught by its own gate before mutation testing: `width: 2em`
and `min-height: 2em` both computed to 800, because the parent was split by
*property* instead of by *unit*. An em is a share of the font wherever it
appears; only a percentage on a box property is a share of the containing block.

Known divergences, stated rather than discovered later: `align-content` is
parsed and dropped (there is no field, and aliasing it onto `align-items` would
move a single-line row on the cross axis, which a real browser does not do);
there is no `border-style` field, so `border-style: none` computes the width to
0 but `border-width: 2px` with no style draws where Chrome would not; one
`overflow` field serves both axes; margin and padding percentages still resolve
against the font size, which is wrong per spec, is what shipped, and is now
asserted explicitly so changing it is visible.

---

## 6. The gates

Every number below was produced by running the command, on this branch, after
all of the above landed.

| gate | before | after |
|---|---|---|
| `htmltest` | 101 | **376 checks, 0 failed** |
| `csstest` | 328-line harness | **356 checks, 0 failed** |
| `browsertest` | 58 | **103 checks, 0 failed** |
| `pngtest` | — | **146 checks, 0 failed** |
| `fbtext` | 45 | **54 checks, 0 failed** |
| `httptest` | *could not build* | **91 checks, 0 failed** |
| `fuzz` | *could not build* | **~400,000 checks/seed × 3 seeds, 0 failed** (ASan+UBSan) |
| `nettest` | 152 | 152 checks, 0 failed |
| `tcptest` | 110 | 110 checks, 0 failed |
| `dnstest` | 97 | 97 checks, 0 failed |
| `arenatest` | 62 | 62 checks, 0 failures |
| `browsershot` | picture | picture — monotonic reflow at 760/480/300 px |

**The fuzzer is the headline.** It previously found four real layout defects
that no hand-written test had reached, because all four needed a
width-to-font ratio nobody would type. It has not built since https landed, so
it had not run against any of that. It now runs against a completely rewritten
box model — flex, grid, floats, absolute positioning — and reports **no run
outside the content box, no tree edge outside its array, and no negative
buffer length in 1.2 million checks.**

### And two gates that could not build at all

Both `httptest` and `fuzz` had stale link lines that were never updated when
the TLS transport landed in `http.c`. Both produced no binary. Both were
confirmed pre-existing against a clean `git archive HEAD` tree — **and the
first A/B I ran was wrong in a way worth recording**: it compiled HEAD's
`layout.c` against the *new* `css.h`, which failed at COMPILE and never reached
the link, so "0 undefined references" was true for entirely the wrong reason.
A measurement that can be right by accident is not a measurement.

---

## 7. The adversarial review found a P1 that every gate had passed

`CLAUDE.md` requires a different model family to grade anything
security-, data- or irreversibility-adjacent. `png.c` decodes bytes off a
network, so this qualified. Codex reviewed the uncommitted diff with the
untrusted-input paths named explicitly.

**It found four real defects. All four are fixed. Every one of them was sitting
under a completely green gate sweep.**

### P1 — the picture arena landed on the RAM filesystem

`HI_IMG` was placed at `0x02000000` because it is 2 MiB aligned, it is above
`arena.c`'s program arena, and **every assert in `memmap.h` passed.**

It is not free. `kernel.zl` keeps a *second* fixed-address block there:

```
0x02000000 .. 0x02000400  SNAKE_X / SNAKE_Y
0x02010000 .. 0x02010040  FS_META
0x02011000 .. 0x02025000  FS_DATA        81,920 bytes
0x02030000 .. 0x020300C8  LINE_BUF
0x02031000 .. 0x02032000  HIST_BUF
```

A 4 MiB picture arena based at `0x02000000` covers all six. The bundled 32×32
home-page image already overwrote both Snake arrays; a 132×132 PNG from any
server would have written through the filesystem's metadata and data.

**`check-memmap.sh` printed those exact addresses earlier in this same session
and I read the output without joining it up.** That is the whole failure. The
asserts could not help, because `memmap.h`'s asserts only compare `memmap.h`'s
own regions to each other — which is the identical hole, one map over, that
this file's header describes `i2c_hid.c` falling into: *"the bases are each
individually sensible and only collide when you subtract them."*

Two maps that do not know about each other are two maps that will collide.

Fixed three ways, because moving the region alone would leave the trap armed:
- `HI_IMG` moved to `0x03000000` (48 MiB), clear of the whole zl block.
- `memmap.h` now **declares** `ZL_LOW_BASE`/`ZL_LOW_END` and asserts
  `HI_IMG >= ZL_LOW_END`, so the compiler checks across the boundary.
  Mutation-checked: putting `HI_IMG` back at 32 MiB fails the build with that
  assert's own message.
- `check-memmap.sh` and `memmap.h` now each say, in their output and their
  header, that neither of them is the whole map.

And the harnesses stopped hardcoding the address — they `#include "../memmap.h"`
and map `HI_IMG .. HI_IMG_END`. The literal survived exactly as long as it took
for the region to move once, and then `browsertest` mapped 32 MiB while
`browser.c` wrote to 48.

### P1 — signed overflow in the base64 decoder, on the home page's own image

`b64_decode`'s accumulator was an `int` and nothing discarded the bits it had
already emitted, so it grew six bits per character and overflowed on the
**fifth**. Undefined behaviour, not a wrapped number, on the default page,
every time.

It decoded correctly regardless, because the value read back is always the low
bits — which is exactly why nothing noticed. Now `unsigned`, and masked to the
bits still owed.

**The reason the harness missed it is the useful part.** `fuzz` is built with
ASan and UBSan; `png.c` was fuzzed under them for 90,000 iterations. But
`browser.c` sits *between* those two and was covered by neither, so 103 green
checks said nothing. `build.sh` now also builds `browsertest_san`, and it
passes 103 checks with zero sanitizer diagnostics.

### P2 — a cancelled image fetch could be parsed as a document

`navigate()` clears `img_cur`, but the `about:home` branch **returns early**,
so `fetching` stayed 1 with an image response in flight. When that response
landed, `browser_tick` saw `img_cur == -1`, took the document branch, and would
have handed the PNG's bytes to `doc_set()` — the page replaced by binary parsed
as HTML. `navigate` now resets HTTP before any of its exits.

### P2 — two unbounded port accumulators

`p = p * 10 + digit` with no ceiling, in `img_resolve` and — **pre-existing** —
in `parse_url`, which is the one place this browser takes whatever a person
typed. `:99999999999` overflows a signed int before the `p < 65536` check can
run, and a wrapped value can land back inside the valid range, so a port nobody
typed is fetched with no error. Both now saturate while accumulating, which
leaves the range check meaning what it says.

### What this run should be judged on

Twelve host gates, ~1.2 million fuzz checks, a mutation-tested assertion suite,
and a visual gate were all green while a decoded image was overwriting the
filesystem. **A green suite answers the questions it was asked.** The review
cost one call and found the one thing nobody had thought to ask.

---

## 8. One more pre-existing failure, found by checking a claim before writing it

`browser-status.md`'s headline row was rewritten in this run to say the browser
"fetches `https://en.wikipedia.org/` by name over verified TLS 1.3". That
combined a previously-verified fact with new ones, and `http.c` had been changed
this session — so it was restated, not re-measured. Restating is how the
`httptest` citation in the same file aged into a falsehood.

So it was measured. `ZLOS_NET_TESTS=1 kernel/tests/host/tlstest`:

```
ok   handshake with en.wikipedia.org completed
ok   the article decrypted to an HTTP 200
ok   and it is a whole article, not one record
     988791 bytes decrypted from Wikimedia
ok   the chain validated to a trusted root
ok   CertificateVerify proved the server holds the key
FAIL a chain to a CA we do not carry is REFUSED
     refused with: RSA signature but the issuer key is not RSA
13 passed, 1 failed
```

The claim holds. **The failing assertion is pre-existing** — a clean
`git archive HEAD` tree gives the identical 13/1.

It is not a hole: the chain **is** refused, so it fails closed. It is a wrong
DIAGNOSIS, and that matters more here than it would elsewhere, because this
browser puts `x509_why()` on the screen and `BROWSER-RENDER-PROMPT.md` tells
the next session to "check `x509_why()` before assuming otherwise". A reader
told "the issuer key is not RSA" goes looking at the cipher suite; the actual
problem is that the CA is not in the trust store. Left for the crypto track.

The live half of that gate sits behind `ZLOS_NET_TESTS=1`, which is why it had
gone unseen — the same shape as the two gates in §2 that could not build:
**coverage that exists and does not run is coverage that is not there.**

---

## 9. "Use Google too" — what that turned out to mean

The obvious reading is "make Google work". Measured through the shipping
`tls.c`/`x509.c` with the exact request `build_request()` produces, Google
**already** works and always did:

```
=== https://www.google.com/search?q=zlos ===
  handshake OK (chain verified by tls.c/x509.c)
  HTTP/1.0 200 OK
  Content-Type: text/html; charset=ISO-8859-1
  53902 bytes decrypted
```

**And the page contains no results.** Fed through `html.c`, those 71 KB parse
to **nineteen nodes and 151 characters of text**, which read:

> "Please click `/httpservice/retry/enablejs` … here if you are not redirected
> within a few seconds."

Google's `<noscript>` fallback. The results are assembled by JavaScript, so
there is no document to render — not a gap in the parser, and not something a
bigger parser fixes.

**It is also not a User-Agent problem**, which was the obvious next guess and
is worth recording precisely because it is wrong: the same request with a
mainstream Chrome User-Agent returns 91 KB and still parses to the same
nineteen nodes. So `http.c`'s "no User-Agent games either — this is what it is"
costs nothing here. A decision confirmed rather than assumed.

### A false alarm worth keeping

The first probe sent **HTTP/1.1**, and Google answered with
`Transfer-Encoding: chunked` — chunk-size prefixes sitting in the body, which
`http.c` explicitly does not decode. I was one step from building a chunked
decoder. The browser sends **HTTP/1.0**, and re-running the probe with the real
request line showed no chunking at all. **The blocker was in my probe.**
Reproduce with the thing that ships, not with something shaped like it.

### What was built instead

**Typing words in the URL bar searches** — the last thing between this browser
and being usable by hand, and what "use Google" means in practice.

- `looks_like_url()` — a scheme means an address, a space means a query, and a
  dot before the first `/` or `:` means a host. `node.js` is therefore treated
  as an address, which is what Chrome does too; getting that "right" needs a
  public-suffix list, a database that changes weekly. That is the unbounded
  version of the question and it is refused.
- `url_encode()` — percent-encoding, space to `+`, and every byte above 127
  escaped, because a raw high byte in a request line is how a request gets
  split by something in the middle.
- `URL_MAX` 128 → 256. 128 was already short for real addresses and far too
  short once the bar can carry an encoded query. ~1.5 KB of BSS against the
  147 KB the kernel has left — measured.
- The search endpoint is **one constant**, with the measurement behind the
  choice written beside it.

**The trap, which has a test of its own:** the URL bar calls
`navigate(url, url_len)`, so `u` and `url` are the *same buffer*. Building the
search address into `url` would overwrite the query with the prefix while still
reading it. It builds into a scratch buffer, and `t_search` drives the *bar*
rather than `go()` because only the bar's path hits it.

### And four assertions that were testing the harness

Adding a resolver to the fake network (`net_up` now seeds the resolver's ARP as
well as the gateway's — they are different hosts, and `dns_start` ARP-resolves
the resolver separately) turned four existing assertions red. They expected
`BR_NO_DNS` for any hostname, and **that was never a property of the browser** —
it was a property of a harness with no resolver configured, so `dns_start()`
returned 0 for every name. They now assert the real thing: a name takes the
name path, and `10.0.2.256` is not connected to as though it were an address.

`browsertest` 103 → **124 checks, 0 failed**, clean under ASan+UBSan.

---

## 10. The three open items, closed

### `probe-urlbar.py` now has its mutation check

The claim "this gate would have caught the bug" was argued rather than
demonstrated. Demonstrated now: `browser_click`'s body replaced with the
pre-fix one (helpers kept referenced by `(void)` casts so the mutation changes
**behaviour** and not compilability), kernel rebuilt, gate run:

```
35 characters -> ink 832 pixels
 4 characters -> ink 3512 pixels
FAIL 35 typed characters put far more ink in the bar than 4  832 vs 3512
```

And the inversion explains itself, which is what makes it convincing rather
than merely red: with the bug the bar ends up holding `inux` — four characters,
little ink — and the 31 backspaces then empty a four-character buffer, so the
long placeholder text reappears and the "short" measurement is the *larger*
one. Restored and verified by md5; gate green again.

**The boundary matters**: the replaced region ends at `/* ---- keys ---`, not at
`browser_url_focus`. Cutting to the latter deletes `browser_key` and the kernel
fails to link, which is a broken build rather than a mutation — it cost two
attempts.

### `x509_why()` was lying on the SUCCESS path

Not "the wrong reason on failure", which is what it looked like. `x509_chain_ok`
tries the top certificate against **every** root, each miss writes its reason
through `fail()`, and a later root succeeding returned 1 with the previous
root's complaint still in place. So a fully verified handshake to Google
reported *"RSA signature but the issuer key is not RSA"* — the residue of
testing an EC-signed chain against an RSA root on the way to the right one.

`browser.c` puts that string **on the screen**, and this project's own brief
tells the next session to "check `x509_why()` before assuming otherwise". **A
diagnostic that lies on the success path is worse than none, because nobody
doubts it.** Now cleared by `succeed()` at every accepting return.

**And the test that found it was itself stale.** It used `www.google.com` as
"a CA we do not carry" — true when written, false the moment GTS Root R1 was
added to `roots.c`, which the project announced as a feature. The assertion
then failed on a correctly *verified* handshake. Fixed by narrowing the trust
store instead of chasing hosts: connect to a real site and offer a store that
deliberately excludes its root. **A test whose premise is "some third party
still uses a CA we chose not to carry" decays on somebody else's schedule.**

`tlstest` 13/1 → **16 passed, 0 failed**, including a new assertion that a
verified chain leaves no stale reason behind.

### Alt+Tab had never worked, on any keyboard, ever

`wm.c` carried its own copy of the keycode numbers. It defined `KEY_SUPER`,
`MOD_ALT` and the four arrows — all of which happened to agree with
`keycodes.h` — and it did **not** define `KEY_TAB`. So the handler was written
`code == '\t'`, comparing against **9**, while every producer sends
`KEY_TAB` = **0x103**. `input.c` maps PS/2 scancode 0x0F *and* USB HID usage
0x2B to `KEY_TAB`, and even converts a literal 9 to it. 0x103 is never 9.

`keycodes.h` exists verbatim to stop this: *"a second copy of a numeric table
is a copy that eventually disagrees with the first."* `wm.c` had one, and the
one entry it was missing is the one that broke.

It read as implemented — a handler, an explaining comment, and a correct
`cycle_focus()` underneath. Fixed by deleting the private table and including
the shared header.

Gated three ways, because a host test alone would not have covered the real
keyboard:
- `wmtest` drives **real PS/2 scancodes** through the real `input.c` into
  `wm_frame()` and asserts the focus moved — and that Tab *without* Alt does
  not switch, so the fix is not "any tab cycles". `idt_scan()` was a constant
  `0` in that harness, which is why every keyboard path through `wm.c` was
  unreachable from it; it is now a queue the test can fill.
- Mutation-checked: putting `'\t'` back fails 2 assertions.
- **Verified on the real machine over USB HID** — four Alt+Tab presses, four
  distinct frame hashes.

---

## 11. Rendering: what actually limits a real page, measured

The brief's §2 work made the *home page* look right. A real page is a different
question, so a real page was fetched — the English Wikipedia article on Linux,
596 KB over verified TLS — and rendered.

**What came out was Wikipedia's navigation chrome**: "Jump to content", "Main
menu", the entire sidebar, stacked down the first screen with the article below
it. That looks like a layout bug. It is not.

| measured | |
|---|---|
| nodes | **8192 / 8192, 1,673 dropped** |
| text arena | **196,607 / 196,608** |
| inline `<style>` | 9,168 bytes |
| **external `<link rel=stylesheet>`** | **2 files, 278 KB** |

**The entire skin — every rule that hides that navigation — was in two external
stylesheets the browser parsed and threw away.** Not a parser limit. A missing
fetch.

### Built: external stylesheets are fetched

`html.c` keeps `<link rel=stylesheet href>` (it is `HT_UNKNOWN`, so this cannot
ride on a tag id, and `rel` and `href` have to be remembered independently
because HTML does not order attributes). `browser.c` fetches them **before the
images** — a stylesheet moves every box, a picture only fills one in, so
fetching a picture into a layout about to be rearranged is wasted work.
`http.c` accepts `text/css` on request, with the same fail-closed default.

**Two buffer sizes had to grow, and the reason is a bug class worth naming.**
Wikipedia's first stylesheet path is **522 characters**. `URL_MAX` was 256 and
`http.c`'s `REQ_MAX` 512 — and `build_request` copies the path into `req`, so
an over-long path does not fail, it **silently builds a shorter request**. That
is not a failed fetch, it is a request for a *different resource*, and the 404
that comes back reads as the server's fault. Confirmed live: my own probe had
the identical bug in its own 512-byte buffer and got a real `400 Bad request`
from Wikimedia until I fixed it.

### And what still limits it, also measured

Fetching the CSS is necessary and not sufficient. Of that 271 KB:

| | |
|---|---|
| inside `@media` — skipped entirely by design | **38%** (103 KB, 64 blocks, 16 distinct conditions) |
| rules outside `@media` | 709 |
| ...of which have a selector this engine supports | **160** |
| selector parts using a **pseudo-class** | **480** |

So the next two levers, in order, are **`@media` evaluation** (`screen`,
`min-width`, `max-width` — the conditions are simple and the machinery to skip
them already parses them) and **pseudo-class support**. Neither is unbounded;
both are specified.

The external CSS does land visibly — "Main menu" renders as a styled, bordered,
centred heading where it was bare text. It is progress with a measured ceiling,
which is the honest way to report it.

**What was NOT done, and it is the obvious next thing:** the node array and
text arena are exhausted by a real article, and they cannot grow — 1.95 MB of
the kernel's BSS is already the browser's arrays and there are 132 KB left
under the link ceiling. The fix is the one `png.c` already proved: the caller
supplies the storage, and it lives in the high-RAM map. That is a bounded
change and it is not in this run.

---

## 12. `@media`, and the limit behind it

`@media` is now **evaluated** rather than skipped. `screen`, `all`, `min-width`
and `max-width`, comma lists, `and`, `only`. Everything else — `not`, `print`,
`calc()`, `orientation`, `prefers-*`, `min-resolution`, `em` widths — **refuses
the block**, which is exactly the behaviour that shipped before. Widening what
matches can only ever be a deliberate act, never an accident of parsing.

Evaluated at **parse** time, and that is the honest limitation: a sheet is
parsed once, and re-running the queries on a resize would mean re-parsing —
which for an external sheet means re-*fetching*, since its text is not kept
after interning. A resize does not re-evaluate. That is a far smaller limit
than discarding 38% of every modern stylesheet.

`csstest` 356 → **372 checks, 0 failed**. Mutation-checked both ways, because a
matcher that says yes to everything passes every "it applied" test and one that
says no to everything passes every "it did not":

| mutation | caught by |
|---|---|
| `media_matches` always returns 1 | **10** assertions |
| `media_matches` always returns 0 | **3** assertions |

### And it barely helped, which is the finding

| Wikipedia's skin, 271,848 bytes | rules taken |
|---|---|
| viewport 0 (queries refuse — the old behaviour) | 235 |
| viewport 1036 (the browser's real width) | 235, +4 declarations |
| **`MAX_SELS` raised 384 → 4096, viewport 1036** | **375** |

**`MAX_SELS = 384` is the binding limit, not the media queries.** The parse
stops before it ever reaches most of the blocks — `css_overflowed()` is 1 at
384 and 0 at 4096.

And it cannot be raised. Measured: `sizeof(struct sel)` is **112 bytes**.

```
MAX_SELS  384 ->  43,008 bytes   (today)
MAX_SELS 1024 -> 114,688 bytes
MAX_SELS 4096 -> 458,752 bytes   (what Wikipedia needs)
kernel headroom under link.ld    131,328 bytes
affordable today                 MAX_SELS <= 1567
```

I did **not** raise it. Spending the kernel's last 131 KB on a partial win
would make the next person's first array fail to link, and the number this run
put in `memmap.h` — "149 KiB is the number to check before adding any array
anywhere in this kernel" — would have been spent by the person who wrote it.

**So every remaining rendering lever is behind one change**: nodes, text arena,
runs, and selectors are all BSS-bound, and the browser is already 1.95 MB of a
3.34 MB BSS. The fix is the one `png.c` proved and `memmap.h` now has the
machinery for — the caller supplies the storage, it lives in the high-RAM map,
and ~1.95 MB of BSS comes back at the same time.

---

## 13. The network comes up at boot

Asked for, and it was a real gap: the browser said *"the network is not up —
run the network gate first"*. That is a fine message for a bring-up gate and a
terrible one for a browser — someone who opened a browser has already said what
they want, and being told to run a diagnostic first is the shape of a demo.
Every other device this kernel drives comes up at boot; the card was the
exception only because its bring-up started life as a test.

`net_boot()` does the four steps that matter — find the card, check the arena
is backed by RAM, the feature handshake, `ip_up`, and point at a resolver — and
reports one line:

```
  network: 10.0.2.15/24 via 10.0.2.2, resolver 10.0.2.3
  compositor: 4 windows, shell client 82,160 1236x834
```

**No card is silent; a card that FAILS is not.** Most gates here boot with no
`-device virtio-net-pci`, so complaining about a machine with no network would
make every one of those transcripts noisy — but a card that is present and does
not come up is a fault, and a boot that says nothing about it turns "the
browser cannot reach the network" into a mystery later. Each of the three
failure steps has its own message.

Verified three ways, and the middle one is the one that mattered:
- with a NIC, the line appears **before** the compositor marker;
- **with no NIC, zero lines mention the network** — so the gates that boot
  without one are unchanged;
- **`verify.sh` still matches `golden.txt`**, which was the real risk: a new
  boot line is exactly how a golden-transcript gate breaks.

`net_gate()` is untouched and still does the full diagnostic. It now finds the
link already up and says so through the `ip_live()` branch it already had —
which is also how this was first confirmed working, before the print was.

The browser's `BR_NO_NET` string no longer says "run the network gate first".
If the network is not up there is no card, and telling someone to run a
diagnostic they cannot fix anything with is worse than naming what is absent.

---

## 14. The HTTPS/JavaScript completion audit — and the two regressions it caught

A separate standing objective asked for HTTPS and JavaScript to be *fully*
wired. All five of its items were already in the tree:

| | |
|---|---|
| `browser.c` → TLS for `https://` | `http_start_tls` on all four fetch paths |
| `<script>` → `js.c` | `js_eval`/`js_output` in `doc_set` |
| the seven files in `SOURCES` | crypto, ecdsa, x509, tls, js, roots, entropy — all present |
| entropy for the ephemeral key | `rnd_bytes` with a quality tier; `q == 0` refuses the handshake |
| a clock for certificate expiry | `zl_now_z` → `tls_trust` |

**Wiring present is not working, so it was driven in the kernel** — typed
`https` into the shell over QMP, because serial keystrokes do not reach it in
the compositor boot state. That found two regressions, both mine, both from
this session's subresource work:

**1. A working fetch reported as a failure.** `BR_IMAGES` (8) — "the page is up,
its pictures are arriving" — is a state this session added. Both gate pump
loops enumerated 4 (fetching) and 7 (resolving) only, and the success test was
`if tgs == 0`. So the moment the document landed and the browser moved on to
its stylesheets, the gate exited and printed **"HTTPS FAILED, browser status
8"** for a fetch that had entirely succeeded.

The fix is not "also pump on 8" — that makes the gate wait for every picture,
and under TCG each one is its own TLS handshake. **8 is success for a document
gate.** Adding a state means revisiting everything that enumerates states;
there were two loops and one success test, and only the loops are obvious.

**2. `HTTP 0   body 0 bytes` on a successful fetch.** The gates read
`http_code()`/`http_len()` straight from `http.c`, which was the same thing
right up until the browser learned to fetch subresources — by the time the gate
prints, `http.c` is describing a stylesheet. `browser.c` now exposes the
document's own numbers (`br_code`, `br_doclen`), captured when it landed. The
raw http gate still reads `http.c` directly, because there it *is* the subject.

### And a third thing, which cost the most time

The audit kept showing stale output. `./build.sh` rebuilds `kernel.elf` — **the
harness boots `zlOS.iso`**, which only `mkiso.sh` rebuilds, and `exercise.py`'s
`build()` is what calls it. My probe never called it. So for several runs I was
reading a kernel from before the change, chasing a bug in code that was not
running, and at one point I added a debug print and concluded it "wasn't
executing" when it simply was not in the image.

Both halves of that are worth keeping: **`build()` before `qemu_argv()`, always**
— every probe in this repo that works does it — and *"the change is not in the
binary"* deserves checking before *"the code is wrong"*. `grep` the generated
`_gen.c` and compare mtimes; it takes ten seconds and I spent far longer than
that not doing it.

### The result

```
  a real website over HTTPS
  GET https://www.wikipedia.org/   (verified, not just encrypted)
  took 4 seconds
  HTTPS OK: 2817 pixels of laid-out page
  HTTP 200   body 120361 bytes
  title: Wikipedia
```

Verified TLS 1.3 to a chain-checked root, 120 KB of real page, parsed, laid out
and titled — **in the kernel, over the network that now comes up by itself.**
JavaScript is gated at `jstest` (52 checks) and end to end in `browsertest`'s
`t_scripts`, where a `<script>` calling `document.write` produces three rows in
the layout and its source never renders as text.
