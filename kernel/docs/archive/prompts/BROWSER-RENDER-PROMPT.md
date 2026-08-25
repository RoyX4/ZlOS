# BROWSER-RENDER-PROMPT — make pages LOOK right

Written 2026-08-19 for a fresh session, on branch `desktop/browser-next` in
`/home/roy/Documents/repos/zl-browser2`. Read this, then
`kernel/docs/browser-status.md`.

## Two things you are about to be told to do that are already done

**GOOGLE WORKS.** Verified TLS 1.3, chain checked to GTS Root R1:

```
www.google.com     roots=3  cert_ok=1 certverify=1 -> HANDSHAKE VERIFIED
en.wikipedia.org   roots=3  cert_ok=1 certverify=1 -> HANDSHAKE VERIFIED
github.com         roots=3  cert_ok=0              -> REFUSED (Sectigo, not carried)
```

RSA verification (`rsa.c`) landed and is wired into `x509.c`. The trust store
is ISRG X1, ISRG X2 and GTS Root R1.

**AES-256 IS NOT NEEDED AND NEVER WAS.** TLS 1.3 has three ciphersuites and
`TLS_AES_128_GCM_SHA256` is the MANDATORY one - every server must support it,
so offering only that is compatible everywhere. AES-128 is not a weakened AES;
it is a shorter key on the same unbroken cipher. Adding AES-256 is ~40 lines
and buys compatibility with nothing. Do not spend a session on it.

If a site fails now it is a CA we do not carry, not a cipher. Check
`x509_why()` before assuming otherwise.

## The actual state

Verified HTTPS, JavaScript (a bounded interpreter, not an engine), CSS
selectors and cascade, tables, and it boots. In-kernel proof:

```
GET https://www.wikipedia.org/   (verified, not just encrypted)
HTTPS OK: 1582 pixels of laid-out page
HTTP 200   body 114688 bytes     title: Wikipedia
```

## 1. ~~FIX THIS FIRST~~ — the URL bar loses characters. **FIXED, and the diagnosis in this section was wrong.**

> **DONE.** `browser_click` had no URL-bar hit test at all. Clicking the bar
> did nothing, so the bar was still unfocused when typing began; every
> character fell through `browser_key`'s *unfocused* switch, which drops
> anything that is not a shortcut — until the first `l`, which **is** the
> focus shortcut (`case 'l': case 'L': url_focus = 1; url_sel_all = 1;`).
> That `l` was swallowed arming select-all, and the very next character
> cleared the buffer.
>
> So `https://en.wikipedia.org/wiki/Linux` leaves the tail after the string's
> first `l` — and in that one URL the first `l` is the **L of Linux**, which
> is `inux`. Exactly the reported symptom, from a cause in the mouse path.
>
> **"The survivors are the characters after the first SHIFTED one" was a
> coincidence of that single URL**, and it is the reason this sat undiagnosed:
> it points at the keyboard layer, where nothing is wrong. Neither of the two
> hypotheses below is what happened. The characters *did* reach `browser_key`,
> and nothing *re-armed* select-all — it was armed for the first time, by the
> keystroke everyone assumed was being typed into a focused bar.
>
> Fixed by giving `browser_click` a real chrome hit test: the URL bar focuses
> and selects all on click, Back is now a button and not only a keystroke, and
> a click anywhere else defocuses (with the bar focused every key is text, so
> a bar that kept focus left PgDn and the arrows dead with nothing saying why).
>
> **A second bug fell out of the same read.** `wm.c` delivers `EV_MOUSE` on
> the press, on *every motion sample while the button is held*, and again on
> the release — and `br_click` was not given the button mask at all, so it
> could not tell them apart. Every link click navigated at least twice and
> pushed two identical entries onto an eight-slot history. The mask now goes
> through `kernel.zl` and `browser_click` differences it itself.
>
> **`browsertest` could have seen it all along**, and this is the more useful
> lesson. The harness was never the limitation — *every* existing URL-bar test
> pressed `l` first, so all of them focused the bar by the one route that
> worked, and not one could observe the route that did not. That is precisely
> the failure this file's own header warns about: a test that cannot observe
> the property it names reports PASS. `t_chrome_click` in `browsertest.c` now
> types a URL after a *click*, with an `l` in the middle; run against the
> pre-fix `browser_click` it reports **10 failures**.

The original text is kept below because the shape of the mistake is the
reusable part: a symptom was described precisely, a mechanism was inferred
from it that fit perfectly, and the inference named the wrong subsystem.

> Typing `https://en.wikipedia.org/wiki/Linux` leaves **`inux`** in the bar. The
> doubled-keystroke bug above it is fixed (commit `a96fb93`); this is a second,
> separate one and it is NOT diagnosed.
>
> What is known: the survivors are exactly the characters after the first SHIFTED
> one. `browser_key`'s `url_sel_all` clears the buffer on the first printable
> key, so a select-all still armed when the `L` arrives would wipe everything
> before it - which matches exactly. What is unknown is *why* it is still armed:
> either the earlier characters never reached `browser_key`, or something
> re-arms it. Different bugs, different fixes. Distinguish them before patching.
>
> Reproduce with no network: boot, click the URL bar, type a mixed-case string,
> screenshot. Anything shorter than what was typed is this bug.
>
> **`browsertest` cannot see it.** It calls `browser_key()` directly, one call
> per character. Four bugs this session lived in the wiring between the
> compositor and the app and were all found by photographing the screen:
> italic rendering upright, an SVG `<title>` overwriting the document title, the
> doubled keystroke, and this. Budget for looking, not only asserting.

### And one gate that could not run at all

`hosttest/build.sh`'s `httptest` line was never updated when the TLS transport
landed in `http.c`, so the harness had **18 undefined references and produced
no binary**. `browser-status.md` still cites "91 checks, 0 failed" for it —
true when written, and untrue for every run since. Confirmed pre-existing by
rebuilding `HEAD`'s `http.c` against `HEAD`'s `httptest.c`: the same 18.

A gate that cannot build looks exactly like a gate that passes, because this
script prints a screenful of successful builds around the failure. Fixed by
linking `tls.c crypto.c x509.c ecdsa.c rsa.c roots.c entropy.c hostmachine.c`;
it now builds and reports 91 checks, 0 failed, against the current `http.c`.

## 2. ~~Then make pages look right, in this order~~ — **DONE. All four.**

> **Landed 2026-08-19.** The full account, with every number and the command
> that produced it, is in [`browser-render-run.md`](../../evidence/browser-render-run.md).
>
> | | budgeted | shipped | gate |
> |---|---|---|---|
> | **Images** | ~1,500 | `png.c` 944 + `png.h` 101 + `pngtest.c` 1,462 | 146 checks, 0 failed |
> | **Flexbox** | ~1,200 | *(all four are one rewrite of `layout.c`,* | `htmltest` |
> | **Grid** | ~800 | *820 → 2,348 lines, because they share* | 101 → **376** |
> | **More CSS** | ~800 | *one box machinery)* + `css.c` 704 → 1,592 | `csstest` **356** |
>
> Verified visually as well as numerically, which this brief was right to
> insist on: `browsershot` shows a three-card flex row with borders and equal
> heights, a 3×2 `1fr` grid, a `max-width: 460px; margin: 0 auto` block that is
> actually centred, and a **real decoded PNG carried in the page as a `data:`
> URI** — all reflowing at 760/480/300 px.
>
> The costing in the table was good again. What it did not anticipate is that
> flex, grid, float and positioning are **one** change rather than four: they
> all need the same box, so doing them separately would have meant building
> that box three times.
>
> `layout.c`'s header no longer says two box types are enough. It keeps the
> old paragraph struck through and names the error — *it declared a boundary
> where there was only an absence* — which is the same correction
> `browser-status.md` makes about "a browser is unbounded" and `css.h` now
> makes about its own scope.

The original brief is kept below, because its ordering and its costing were
both right and are worth reusing.

> Measured against what a real page needs, biggest visual win first.
>
> | | lines | why this order |
> |---|---|---|
> | **Images** | ~1,500 | every logo and photo is a placeholder box today. PNG first (zlib inflate + the filters), JPEG second and only if wanted - JPEG is a bigger lift for less of the web's *structure*. |
> | **Flexbox** | ~1,200 | how modern pages are ARRANGED. Wikipedia's portal renders as a giant stacked "Wikipedia" purely because there is no flex. |
> | **Grid** | ~800 | after flex; shares most of the box-sizing machinery. |
> | **More CSS** | ~800 | float, position, borders, block backgrounds. |
>
> `layout.c`'s header says two box types are enough for a document. That was true
> and is now the thing to change - read it before you do, it explains the design
> it is defending.

**AIM AT "a person reads it and it looks right", NOT at matching Chrome.**
Flex and grid have specifications, so they are bounded and buildable. Pixel
parity with Chrome is not, and chasing it is the one genuinely unbounded thing
in this project.

## 3. Also open, lower value

- **No certificate revocation checking** of any kind.
- **Only ECDSA P-256/P-384 and RSA.** Ed25519 chains would be refused.
- `HTTP_BUF` is 128 KB and `DOC_MAX` 256 KB. A 1.5 MB article truncates - and
  says so, which is correct, but it is a limit.

## On parallel agents and workflows

**Writing code in parallel: yes.** The four items in §2 touch different files
(`png.c`, `layout.c`, `css.c`) and are genuinely independent. Two or three
agents writing host-gated code at once is fine.

**Running gates in parallel: no, and this is measured.** This box has 4 cores.
This session hit load 27 and a QEMU boot that normally takes 12 s took minutes;
`CLAUDE.md` records an OOM kill of the agent process itself at 7.9 GB. Every
QEMU gate must run **one at a time**, and never beside an agent fan-out.

A workflow that fans out to write code and then funnels to ONE sequential gate
stage is the shape that works here. A workflow that runs `verify.sh` in
parallel branches will produce failures that are about load and not about code
- the exact false-negative `CLAUDE.md` warns about, and which cost this session
real time twice.

## The rule this session kept proving

Five of six bugs were invisible to a passing suite. Four only appeared at a
**boundary** - a forged root with a byte-identical name, an off-by-one harmless
at RSA-2048 and fatal at RSA-4096, a deadlock only on a page larger than the
buffer, a gate whose budget ran out before the work did.

Test at the maximum and at the adversarial case. The comfortable case passed
every time.
