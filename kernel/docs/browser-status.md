# A browser in zlOS — what is actually true

`feature-catalogue.md` marks a web browser ❌ and `OVERNIGHT-PROMPT.md` explains
why:

> **A browser** | Unbounded. Chromium is 6.38M lines, Ladybird 314K, all of
> zlOS is 11,374

**Every number there is correct and the conclusion does not follow.** It
measures the *maximal* version of the capability and reports it as the
capability. Chromium is unbounded. Ladybird is unbounded *for this project*. A
browser is not — and the same document contains its own counter-example four
rows down, where KolibriOS ships an OS, a GUI **and a browser** in 1.44 MB.

Those two files live on other branches. **When this branch merges, both rows
should be corrected** to what is written below. That correction is part of the
work, not an afterthought: this project already had to publicly correct a "95%
achievable" that was really 20% (`DECISIONS.md` #26), and the same standard
applies in the optimistic direction.

---

## The corrected row

| | |
|---|---|
| **A browser** | ⚠️ **Bounded, and partly built.** The maximal version is out of reach. A document renderer is ~3,000 lines of code; **1,118 of them exist and work.** No JavaScript, no HTTPS, no network yet — see the table below. |

---

## Why it was reachable, which is the part nobody had noticed

zlOS already had **the two hardest parts of a browser** before anyone called
them that:

- **A layout engine.** `ui.c` is 395 lines of flowing-cursor layout — place a
  box, advance the cursor, wrap the row at the content width. That is
  *precisely* inline text flow, which is what an HTML document mostly is.
- **A text renderer that is genuinely good.** Real TrueType glyphs,
  gamma-correct linear-light blending, subpixel LCD rendering, proportional
  advances, a clip rectangle and damage tracking.

On top of those, a browser is a fetcher, a parser and a box model.

`layout.c` does not reuse `ui.c` directly, and the reason is worth stating
rather than hiding: `ui.c`'s cursor is a single global tuned for widgets — one
row height, no nesting, no per-run font size, and no way to lay out without
drawing. A document needs all four. What was taken is the algorithm and the
discipline, not the code.

---

## Where it got to

Counted two ways, because the budget in the brief is a code estimate and these
files carry the project's comment density. `wc -l` is the honest headline; the
code column excludes blank and comment-only lines.

| piece | budgeted | code | `wc -l` | state |
|---|---|---|---|---|
| `html.c` + `html.h` — tokenizer + tree | ~450 | **449** | 613 | done, gated |
| `layout.c` + `layout.h` — box model + inline flow | ~500 | **402** | 535 | done, gated |
| `browser.c` — the app | ~350 | **267** | 382 | document + viewport + links; URL bar is item 7 |
| `fb.c` — sized/bold/oblique text | — | — | **+124** | additive; nothing existing changed |
| `kernel.zl` — the app's policy | — | — | **+23** | one `app_draw` branch, one event branch |
| `virtio_net.c` | ~400 | **382** | 628 | done, gated |
| `net.c` + `net.h` — ARP, IPv4, ICMP | ~300 | **359** | 579 | done, gated |
| TCP, client, one connection | ~900 | 0 | 0 | item 3 |
| HTTP/1.0 | ~150 | 0 | 0 | item 4 |
| **total so far** | **~2,000 of ~3,050** | **1,859** | **2,884** | |

The five budgeted-and-built pieces came in at **1,859 code lines against a
2,000 estimate** — the estimate has held to within 10% twice. The `wc -l`
figure is larger because roughly a third of every file here is the reasoning
behind it.

### One open bug, stated rather than buried

**One echo reply is not delivered to the ICMP layer on every bring-up after the
first.** Reproducible on every run; never on the first bring-up. What has been
measured, and what it rules out:

| | |
|---|---|
| a packet capture (`filter-dump`) | **42 echo requests, 42 replies.** The peer answered every one — the frame is not lost on the wire |
| the lost reply's wire latency | **3 µs.** It was there; the guest then spun ~500 ms and gave up |
| driver counters | frames delivered match frames the stack counted. No runts, no truncations, no transmit-queue stalls |
| transmit ring | `avail == used` throughout — the device keeps up, the ring is never overrun |
| receive ring size | identical behaviour at 16 and 32 descriptors, so it is not a wrap |
| sequence matching | `unmatched-echo 0` — no reply arrived bearing an id/sequence we were not waiting for |
| every IP drop path | now counted individually: short, bad-version, bad-ihl, fragment, bad-checksum, not-ours. **All zero** |
| a second queue consumer | the item-1 raw ARP probe drained the same queue; that was fixed (one layer owns a receive queue) and the loss did not change |
| the peer re-resolving us | a gratuitous ARP at bring-up changed nothing, and was reverted rather than left in looking useful |

The invariant signature is that on the second bring-up the stack counts **one
more ARP frame and one fewer IP frame**, totalling the same 22. Not closed.

Six wrong hypotheses is itself the finding: `handle_ip` dropped malformed
frames with a bare `return` and no counter, so the failure could not be located
from outside. Every drop path is now named. That is the change worth keeping
from the whole investigation.

---

## What it does, and what it refuses

**Renders:** `html head body title h1`–`h6` `p br hr a ul ol li strong em b i
code pre div span img`. Entities named, decimal and hex. Malformed markup
recovers rather than faults. A real type scale, synthesised bold and oblique,
margin collapsing, hanging list markers, and reflow on resize.

**Refuses, and says so on screen rather than in a comment:**

| | why |
|---|---|
| **JavaScript** | a JS engine is its own multi-year project. Not "hard" — a different project. |
| **HTTPS** | `crypto.c` has SHA-1, SHA-256, HMAC and PBKDF2 — **543 lines of hashing and no cipher**. TLS needs AES-GCM, ECDHE and certificate-chain validation. A padlock that has not been earned is worse than no padlock, so an `https://` URL is refused by name. |
| **The network** | there is no driver. The desktop header's `net up` is decorative and always was. |
| **Full CSS** | the cascade, specificity, float, flex, grid. Two box types — block and inline — is enough for a document and is not enough for a web app. |

---

## How it is gated

`html.c` and `layout.c` contain **no pixels and no theme by construction**.
Both reach for exactly one thing outside themselves — a function that measures
a string — so injecting a synthetic one turns them into ordinary Linux
programs. That is what makes the gates cheap enough to run on every change
instead of once at the end.

```bash
kernel/hosttest/htmltest       # 101 checks, 0 failed
```

```bash
kernel/hosttest/browsershot out.ppm    # the same page at 760/480/300px
```

```bash
kernel/hosttest/nettest        # 152 checks, 0 failed
```

`net.c` holds no link driver — the link is two function pointers — so the whole
stack links into a harness that is the machine on the other end of the wire.
That is what makes loss and jitter *measured* numbers with a known right
answer: the harness can be told to lose one packet in four, or to alternate its
delay, and the assertions are on the values that must come back. Nothing there
depends on what a real network happened to do that afternoon.

| width | height | lines |
|---|---|---|
| 760 px | 1710 | 33 |
| 480 px | 2510 | 52 |
| 300 px | 3850 | 84 |

Narrower gives more lines and a taller document at every step. That monotonicity
is the assertion that proves it is a layout engine and not a fixed drawing, and
it is checked twice — once against synthetic metrics in `htmltest`, once against
the real font in `browsershot`, because a metric bug can hide from the first.

### Two bugs the picture found and the assertions did not

- **Inline `<code>` did not wrap.** It shares a font with `<pre>` and nothing
  else, and sharing one style bit sent it down the no-wrap path, so every
  `<code>` ran off the right edge. Every numeric invariant passed, because no
  test document contained a `<code>`. One look found it immediately.
- **An unknown element was opened and never closed.** Every unsupported tag
  shares one id, so the close path refused to match on it and one `<section>`
  nested the rest of the document inside itself.

Both now have assertions. The first fails by 122 px if the fix is reverted.

---

## Known limits of what is built

- `<pre>` does not wrap and has no horizontal scroll, so a long preformatted
  line is clipped. That is what `<pre>` means, and the clip is `fb_clip`, so
  nothing escapes — but it is a limit, not a feature.
- Bold and oblique are **synthesised** — a double strike and a shear. There is
  one weight and one slope in the atlas. Visibly not a real bold to anyone who
  looks closely, and unambiguously better than rendering `<strong>` identically
  to the text around it.
- Documents are capped at 24 KB, the node array at 1024 and the run array at
  2048. All three are fixed arrays because there is no heap. Overflow
  **truncates and says so on screen**; it does not scribble.
- Baselines are approximated by bottom-aligning the glyph cell. Correct only
  because every glyph comes from one atlas; wrong the moment a second face with
  different metrics arrives.

---

Brief: [`BROWSER-PROMPT.md`](BROWSER-PROMPT.md) · Contract: [`../ui.h`](../ui.h)
