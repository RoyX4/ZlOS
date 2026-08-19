# The browser: four confirmed criticals, and 1,032 lines of dead CSS engine

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366`**

From the 25-agent browser wave. The four below were re-derived from source by hand.
The browser is the project's headline capability — *"it fetches `http://example.com/` by
name, off the real internet, and renders it"* — and on the target hardware it is
currently very hard to drive.

Keyboard defects have their own file: [`CRITICAL-browser-urlbar-keys.md`](CRITICAL-browser-urlbar-keys.md).

---

## 1. The entire CSS engine is dead code — and the repo already saw the symptom

```
$ grep -n css kernel/SOURCES
  (no output)

$ grep -rn 'css\.c|css\.h|css_' kernel/build*.sh kernel/mkdisk.sh \
        kernel/layout.c kernel/browser.c kernel/html.c
  (no output)

$ wc -l kernel/css.c kernel/hosttest/csstest.c
  704 kernel/css.c
  328 kernel/hosttest/csstest.c
 1032 total
```

`kernel/SOURCES` is *"one file, read by all four kernel builds"* (`CLAUDE.md`). `css.c`
is not in it, is referenced by no build script, and is called by nothing in `layout.c`,
`browser.c` or `html.c`. **1,032 lines of parser and tests, compiled into nothing.**

The browser has no cascade at all. What styles a document today is `layout.c`'s
hardcoded `head_size()` table (`layout.c:296-307`) plus size/style/colour threaded
through `walk()` as parameters.

### The part worth sitting with

`.ultra/STATE.md` records, under *"Known-red, and not mine"*:

> `gates/land-gate.sh` fails one stage: the reverse-SOURCES sweep flags `crypto.c` and
> `css.c` as tracked but uncompiled. **Proven pre-existing** […] and `crypto.c`'s
> absence from `SOURCES` looks deliberate.

The gate **found this**. It was read as a gate quirk about `crypto.c`, and `css.c` rode
along in the same sentence. For `crypto.c` the reading is right — `browser.c`'s HTTPS
refusal depends on there being no crypto in the build. For `css.c` it is not: nothing
depends on CSS being absent, and its absence is why the browser cannot style anything.

**A red gate was correctly diagnosed for one file and the second file in the same
message was never separately explained.** That is the finding, more than the missing
line in `SOURCES`.

### It is not one line to fix

Adding `css.c` to `SOURCES` makes it compile, not work. `html.c:421-423` says
*"attributes: only `href` and `alt` are kept; the rest are parsed and discarded"* — so
there are no `class` or `id` attributes stored, and a selector engine has nothing to
match against. The bounded fix is: retain `class`/`id`/`style` in `html.c`, expose the
ancestor path, add `css.c` to `SOURCES` and `hosttest/build.sh`, and call it from
`layout.c`'s `walk()`.

---

## 2. Pointer **motion** navigates — hovering a link fires a fetch

```zl
# kernel/kernel.zl:2994
if ety == 4 { if br_click(ex, ey) == 1 { wm_dmg(win) }  return 1 }
```

`ety == 4` is `EV_MOUSE`. The button mask arrives in `ecode`:

```c
/* kernel/input.c:620 */
evq_push(EV_MOUSE, (u32)b, mods, px_x, px_y);
```

Every pointer movement pushes an `EV_MOUSE` — the coalescing above it
(`input.c:615-618`) suppresses only events where position *and* buttons are unchanged.
So a plain move delivers `EV_MOUSE` with `b == 0`, and `APP_BROWSER` **never examines
`ecode`** before calling `br_click`.

The rest of the file gets this right, which is what makes it a slip rather than a design:

```zl
kernel.zl:3067:  if band(ecode, 1) != 0 {
kernel.zl:3678:  if band(ecode, 1) == 0 { return 1 }      # menu_event
```

**Fix:** `if ety == 4 { if band(ecode, 1) != 0 { if br_click(ex, ey) == 1 { wm_dmg(win) } }  return 1 }`

---

## 3. A 3xx redirect hangs the browser in "fetching…" forever

`browser_tick` handles three terminal states — `HTTP_DONE` (`browser.c:423`),
`HTTP_REFUSED` (`:430`), `HTTP_ERROR` (`:431`) — then falls to `return 0` at `:432`.
`http.h:21` defines a **fourth**, `HTTP_REDIRECT`, which `http.c` can enter and which
nothing consumes.

There is no timeout on the fetch either, so the state is not merely wrong, it is
terminal-in-the-wrong-direction: the UI says "fetching…" and no input clears it.

*Agent finding, not hand-verified beyond confirming `HTTP_REDIRECT` exists in `http.h`
and is absent from `browser_tick`'s branch set. Re-read `browser.c:420-432` before
fixing.*

---

## 4. `parse_url` has no base, so no relative link can be followed

`parse_url` scans from index 0 (or 7 past `http://`) to the first `/` or `:` and calls
whatever it finds a host (`browser.c:307-309`). So `<a href="/about">` parses `about`
as a **hostname**, and the failure is reported as a DNS problem — a message about the
wrong thing entirely, which is the shape this repo's `browser-status.md` already
complains about for the `about:home` case.

Since `example.com`'s page is mostly one absolute link, this presents as "links
sometimes work."

---

## Also reported, not hand-verified — treat as leads

| file:line | claim |
|---|---|
| `css.c:149` | `intern()` truncates at 255 but callers store the **untruncated** length — out-of-bounds read past `arena[]`, and a 64 KB selector part matches every element |
| `css.c:511` | a `{` inside a comment inside an at-rule makes the depth counter never reach zero — the rest of the stylesheet is silently eaten |
| `css.c:675` | the specificity sort key is `(long)spec * 100000L + order`; `long` is 4 bytes under `-m32` **and** under the LLP64 EFI target, so a 3-id selector overflows and is never applied |
| `css.c:352` | `!important` does not "ignore" as `css.c:22-23` claims — the value fails to parse, so the whole declaration vanishes; `display: none !important` renders as block |
| `html.c:97` | `is_void()` is exactly `BR \|\| HR \|\| IMG` — `<meta>` and `<link>` are pushed and never close, parenting `<body>` inside `<head>` |
| `html.c:112` | `html_is_block()` returns 0 for `HT_UNKNOWN`, so no HTML5 sectioning element (`section`, `article`, `nav`, `aside`) closes an open `<p>` or breaks a line |
| `html.c:402` | tag-open has no ASCII-alpha gate, so `x<3` in body text deletes the following text and swallows the enclosing close tag |
| `browser.c:649` | link hit-testing is not clamped to the viewport — scrolled-off links stay clickable underneath the chrome |
| `browser.c:352` | a failed `navigate()` rewrites the URL bar **before** validating, and leaves the in-flight fetch running |
| `browser.c:725` | Back is drawn as an enabled control and is unreachable by every path that exists |

Several of these are memory-safety-adjacent (`css.c:149` especially) and sit directly on
a network trust boundary, in ring 0, with no heap and no process isolation. They deserve
hand-verification before anything else in this list — but note `css.c` is not currently
compiled, so none of the `css.c` items are *live* until it is added to `SOURCES`.
**Fix them as part of wiring it up, not after.**
