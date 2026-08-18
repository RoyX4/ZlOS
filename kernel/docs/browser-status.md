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
| **A browser** | ⚠️ **Bounded, and built.** The maximal version is out of reach. A document renderer was costed at ~3,000 lines of code; **2,617 of them exist and work, and they fetch a page over the network and render it.** No JavaScript, no HTTPS — see the table below. |

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
| `tcp.c` + `tcp.h` — client, one connection | ~900 | **513** | 752 | done, gated |
| `http.c` + `http.h` — HTTP/1.0 | ~150 | **245** | 332 | done, gated |
| the browser app's URL bar, Back and history | — | *(in browser.c)* | — | done, gated |
| **total** | **~3,050** | **2,617** | **3,968** | |

Every budgeted piece is built: **2,617 code lines against a ~3,050 estimate**,
which is 14% under. The brief's costing was good, and it was good because it
was arrived at by looking at what the tree already had rather than by guessing. The `wc -l`
figure is larger because roughly a third of every file here is the reasoning
behind it.

### One known defect, now precisely characterised

**One frame per bring-up is reported by the device and never written by it.**
Always the same frame: the **33rd**, which is the first wrap of the 32-entry
receive ring, on descriptor id 0.

It was chased twice. The first attempt eliminated six hypotheses and closed
nothing; the mistake was comparing a packet capture from one build against
guest counters from another. Taking both **in the same run** settled it in
three steps:

| | |
|---|---|
| wire vs driver, same run | 45 inbound frames, 45 trace entries, **exactly one mismatch** — at index 32, descriptor 0. The device reported the correct length for the frame that really arrived and left the buffer holding its previous contents |
| clear the buffer before re-posting | the mismatch changes from a plausible stale ARP frame to **all zeros** — which is how "the device did not write it" became established rather than assumed |
| detect and drop | the driver's frame counts now match the wire exactly, where before it reported one ARP frame that never existed |

Ruled out by experiment rather than by reasoning: compiler ordering (a read
barrier changes nothing — it is kept anyway, because it is correct), descriptor
id reuse, used-ring index drift, the transmit ring, the ring being exactly
full, ring size, and the peer.

The consequence is one lost packet per bring-up, which the ICMP layer reports
honestly as loss — that is what the 20-ping gate is for. What is fixed is that
it is no longer *silent*: the previous contents of a buffer are no longer
delivered upward as though they had just arrived.

Two of the changes made while chasing it are correct practice regardless of
the cause, and both came from what the tree already knew: a read barrier
between the used-index check and the buffer read, and clearing a receive
buffer before handing it back — which is the rule `xhci.c` already documents
for its report buffers, and which this driver had for its initial post and not
for its re-post.

Six wrong hypotheses on the first attempt is itself the finding: `handle_ip`
dropped malformed frames with a bare `return` and no counter, so the failure
could not be located from outside the guest at all. Every drop path is now
named, and `net_ping_run` records **which** pings were lost as a bitmask
rather than just how many — "5 of 20 lost" and "the 7th was lost" are
different bugs.

---|---|
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
| **HTTP/1.1** | chunked transfer encoding, keep-alive and pipelining are requirements there, not options. 1.0 ends a body by closing the connection, which the TCP layer already handles. A decision, not a gap. |
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

```bash
kernel/hosttest/tcptest        # 110 checks, 0 failed
```

```bash
kernel/hosttest/httptest       # 91 checks, 0 failed
```

```bash
kernel/hosttest/browsertest    # 58 checks, 0 failed
```

`browsertest` covers the app's logic with the drawing stubbed rather than
linked — URL parsing (the one place the browser takes whatever a person typed),
the history stack and its eight-slot cap, and the URL bar's key machine. The
network under it is real, so "did it parse the port" is answered by inspecting
the SYN that went out rather than by an accessor that exists only for the test.

Both harnesses were mutation-tested, and that is the part worth keeping. Of
fourteen deliberate mutations, twelve were caught immediately and **two were
not** — and both survivors were weak *assertions*, not correct code:

- `about:home` not being pushed into the history was invisible, because the
  browser is a single global with no teardown and the test's reset could never
  produce a genuinely empty history. Moved to a test that runs first, where
  the property is observable exactly once per process.
- control characters reaching the URL buffer was invisible, because the first
  printable keystroke clears the select-all and wiped them before the
  assertion looked. Now a real character is typed first.

A test that quietly cannot observe the property it names is worse than no
test: it reports PASS.

`httptest` drives `http.c` over the **real** `tcp.c`, with the response
arriving as scripted TCP segments — so it exercises the same reassembly path
the kernel uses rather than a mock of it. It passed all 91 checks the first
time it ran, which is not a claim worth making on its own: six deliberate
mutations of `http.c` — dropping bare-LF header support, accepting every
content type, not clamping the body to `Content-Length`, treating a 3xx with
no `Location` as a redirect, making header names case-sensitive, and dropping
the truncation flag — are each caught by between 1 and 16 assertions. The
tests bite; the code passed.

### The adversarial review, and what it found

§8 asks for "an adversarial reviewer on the TCP state machine — have one try to
construct a packet sequence that wedges it." One did, with runnable repros, and
found **thirteen defects**. All thirteen are fixed and all thirteen now have
regression tests. The three that mattered most:

| | |
|---|---|
| **A remote out-of-bounds write from three segments.** `seq_lt` is modular, so a segment 2³¹ *ahead* of the window reads as 2³¹ *behind* it. Converting that difference to an `int` length gave `INT_MIN`, `dlen - skip` overflowed, and the receive buffer's tail went two gigabytes negative — after which an ordinary segment wrote outside the array entirely. Confirmed under ASan and UBSan. | CRITICAL |
| **Acknowledged bytes that were never stored.** A full receive buffer silently discarded the overflow while `rcv_nxt` advanced by the whole segment, so the peer was told data had arrived that had been binned. The application then received a stream with a hole cut out of it and no error anywhere. | HIGH |
| **A close that raced the congestion window truncated the stream, and a loss before it wedged the connection forever.** The FIN was emitted at `snd_nxt` — in the middle of what the application had handed over — and the retransmit timer resent bare FINs in preference to the lost data, so the peer could never acknowledge the FIN and the single connection slot was held permanently. | HIGH |

Plus: a RST was accepted across the entire forward half of the sequence space
(a coin flip for an off-path packet), a bare FIN from the distant past
half-closed the connection, a zero window was rewritten to a full segment, and
the out-of-order slot could be stranded for the life of the connection.

The review was worth several times what it cost. It is also a reminder of the
asymmetry in this whole file: the 89 checks written alongside the code all
passed, because tests written by the author test what the author was thinking
about.

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
