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

> **DONE, 2026-08-19.** Both files landed on `main` in the eleven-track merge
> and both have been corrected on `desktop/browser-next`:
> `feature-catalogue.md` §12 (the whole networking table, since `TCP/IP stack
> ❌` above a working browser is a contradiction, not a stale row), its
> "explicitly not worth taking" list, and §"Why a browser is in a category of
> its own"; and the exclusion table in `OVERNIGHT-PROMPT.md`. The original
> text was kept and annotated rather than deleted — the reasoning is still
> correct about the thing it actually measured, and the *shape* of the mistake
> is the reusable part.

---

## The corrected row

| | |
|---|---|
| **A browser** | ⚠️ **Bounded, and built.** **It fetches `https://en.wikipedia.org/` by name over verified TLS 1.3, and renders it with images, flexbox and grid.** ~14,550 lines across the whole stack. |

> **The row above has been corrected twice, in the same direction each time.**
> It first read "no JavaScript, no HTTPS" and then "~2,900 lines"; both were
> true when written. HTTPS landed with a real certificate chain check, a
> bounded JavaScript interpreter landed, and then images, flex, grid and the
> box model landed. Measured now with `wc -l`: html 795, css 1,592, layout
> 2,373, browser 1,429, png 944, js 1,146, plus the network and crypto stack —
> **14,552 lines.**
>
> Keeping the count honest matters more here than usual, because this file's
> whole argument is that the original "unbounded, therefore ❌" was a
> measurement of the maximal version reported as the capability. A stale
> *optimistic* number would be the same error with the sign flipped.

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
| `dns.c` + `dns.h` — UDP + a resolver | *(not in the brief)* | **283** | 424 | done, gated |
| the browser app's URL bar, Back and history | — | *(in browser.c)* | — | done, gated |
| **total** | **~3,050** | **2,617** | **3,968** | |

Every budgeted piece is built: **2,617 code lines against a ~3,050 estimate**,
which is 14% under.

> **That table is the ORIGINAL build and is kept as the record of the costing,
> which was good.** It is not the current size. Measured 2026-08-19 with
> `wc -l`, after HTTPS, JavaScript, images, flex, grid and the box model:
>
> | | then | now |
> |---|---|---|
> | `html.c` | 613 | **795** |
> | `css.c` | *(did not exist)* | **1,592** |
> | `layout.c` | 535 | **2,373** |
> | `browser.c` | 382 | **1,429** |
> | `png.c` | *(did not exist)* | **944** |
> | `js.c` | *(did not exist)* | **1,146** |
> | `tls.c` + `x509.c` + `ecdsa.c` + `rsa.c` + `roots.c` + `crypto.c` + `entropy.c` | *(did not exist)* | **~4,400** |
> | whole stack incl. net/tcp/http/dns/virtio_net | 3,968 | **14,552** |
>
> The costing was good BOTH times, and the second time taught something the
> first did not: **flex, grid, float and positioning are one change, not four.**
> They all need the same box, so doing them as four separate increments would
> have meant building that box three times over. The brief's ~1,200 + ~800 +
> ~800 split was right about the total and wrong about the seams. The brief's costing was good, and it was good because it
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
code pre div span img` and tables. Entities named, decimal and hex. Malformed
markup recovers rather than faults. A real type scale, a real bold face,
margin collapsing, hanging list markers, and reflow on resize.

**And, since 2026-08-19:** **real decoded PNG images** (from the network and
from inline `data:` URIs), **flexbox**, **CSS grid**, a real box model
(width/min/max, `box-sizing`, padding that insets, borders, block backgrounds,
`margin: 0 auto` centring, `overflow: hidden`), **floats** and **positioning**.
See [`browser-render-run.md`](evidence/browser-render-run.md) for the measurements.

**Refuses, and says so on screen rather than in a comment:**

| | why |
|---|---|
| **JavaScript** | a JS engine is its own multi-year project. Not "hard" — a different project. |
| ~~**HTTPS**~~ | **BUILT.** TLS 1.3 with AES-128-GCM, ECDHE, and a real X.509 chain check to a carried root — the padlock is earned. The refusal below stood only while `crypto.c` was 543 lines of hashing with no cipher. What is still refused is a chain to a CA not in the trust store, by name, via `x509_why()`. |
| **HTTP/1.1** | chunked transfer encoding and the rest of the 1.1 framing contract are not implemented. The 1.0 client now offers optional keep-alive, but reuses only a length-delimited response whose peer explicitly returned `Connection: keep-alive`; close-delimited bodies still end at FIN and there is no pipelining. |
| ~~**Full CSS**~~ | **MOSTLY BUILT, and the sentence below was wrong in the way this file exists to catch.** The cascade and specificity were already there; float, flex, grid, positioning and a real box landed 2026-08-19. "Two box types is enough for a document and not enough for a web app" was true about the CODE and wrong about the BOUNDARY — it named an *absence* as a limit. Flex and grid have specifications; they are finite and they are built. |
| **Pixel parity with another browser** | **This** is the genuinely unbounded one, and it is refused on purpose. A specification is finite; matching Chrome exactly is not a specification, it is a moving target. |

The two struck rows are kept rather than deleted because the shape of the
mistake is the reusable part — the same reason the "unbounded, therefore ❌"
row at the top of this file is kept. What is still refused, precisely:
pseudo-classes, `calc()`, media queries, grid areas/spans/`order`, baseline
alignment, `position: sticky` (lays out as relative), cross-host image fetches
(needs a second DNS state machine), APNG, colour management, and 16-bit PNG
output. Every one of those is named in the code that refuses it.

---

## How it is gated

`html.c` and `layout.c` contain **no pixels and no theme by construction**.
Both reach for exactly one thing outside themselves — a function that measures
a string — so injecting a synthetic one turns them into ordinary Linux
programs. That is what makes the gates cheap enough to run on every change
instead of once at the end.

```bash
kernel/tests/host/htmltest       # 101 checks, 0 failed
```

```bash
kernel/tests/host/browsershot out.ppm    # the same page at 760/480/300px
```

```bash
kernel/tests/host/nettest        # 152 checks, 0 failed
```

```bash
kernel/tests/host/tcptest        # 110 checks, 0 failed
```

```bash
kernel/tests/host/httptest       # 91 checks, 0 failed
```

> **That number was true when it was written and had not been true since.**
> `http.c` gained the TLS transport when https landed, and `build.sh`'s
> `httptest` line was never updated - so the harness had **18 undefined
> references and produced no binary at all**. Confirmed pre-existing by
> rebuilding `HEAD`'s `http.c` against `HEAD`'s `httptest.c`: the same 18.
>
> A gate that cannot BUILD looks exactly like a gate that passes, because
> `build.sh` prints a screenful of successful builds around the failure -
> the same shape as the `gpu_fillrate` error this script's own comments
> already record surviving at the end of a successful run. Fixed by linking
> `tls.c crypto.c x509.c ecdsa.c rsa.c roots.c entropy.c hostmachine.c`; it
> now builds and reports 91 checks, 0 failed against the current `http.c`.
>
> The reusable part: **a gate's own BUILD is part of what has to be re-run.**
> Every citation in this file is a measurement, and this one aged into a
> falsehood without a single line of the file changing.

```bash
kernel/tests/host/browsertest    # 58 checks, 0 failed
```

```bash
kernel/tests/host/fbtext         # 45 checks, 0 failed
```

```bash
kernel/tools/probes/probe-net.py            # the only gate that is not a host test
```

`probe-net.py` is the odd one out and deliberately so: every gate above runs
the shipping source as an ordinary Linux program, and none of them can prove
the DMA arena is where the driver thinks it is, that the feature handshake
matches what a device really offers, or that a descriptor ring survives contact
with QEMU. It boots the kernel with a virtio-net card attached, types `net`,
and asserts on the readout — card found, arena backed by RAM, handshake, link,
MAC, ARP answered, twenty pings with zero loss, DNS resolving a real name and
refusing one that does not exist, and the driver's own truncated/runt/
bad-checksum/double-return counters at zero.

The host tests and this one answer different questions and both are needed:
`nettest`/`tcptest`/`httptest`/`dnstest` are the machine on the other end of
the wire, proving the state machines against sequences a real peer will not
produce on demand. This is the wire.

`fbtext` is the newest and it exists because of a hole the others did not
cover: **both text regressions this browser suffered shipped with every gate
above green.** Italic silently rendering upright is not a crash, not a wrong
number and not a failed assertion; six heading sizes collapsing onto two does
not move a line count, because a heading that already fits on one line still
fits when it is set too small. `browsershot` drew both bugs faithfully and
nobody diffed the pixels.

So it asserts the two things a picture and a line count both miss — that a
requested pixel size is the size that gets drawn, and that a style flag changes
the pixels. **Against the shim it replaced it reports 12 failures**, which is
the only reason to believe it would catch the next one.

```bash
kernel/tests/host/fuzz 3000 1    # ~400,000 checks per seed, 0 failed
```

The fuzzer feeds garbage to every layer that takes bytes from somewhere else —
markup from a server, frames from anyone on the segment, segments from anyone
who can guess a port, a response from the server we asked. It is deterministic
(a seeded LCG, and a failure prints its seed) and it is built **with** ASan and
UBSan, because a clean run without them proves almost nothing. It asserts
invariants after every iteration rather than only checking for a fault: a run
outside the content box, a tree edge pointing outside its array, or a receive
buffer whose length goes negative are all silent corruption.

It found four layout defects in the first four thousand iterations, none of
which any hand-written test had reached, because all four need a width-to-font
ratio nobody would type:

| | |
|---|---|
| **nested list indentation was never clamped** against the content width, so a dozen nested `<ul>` on a narrow window pushed the whole page outside the box — where `fb_clip` hides it, leaving a blank window and nothing to say why | fixed |
| **`<pre>` ran off the edge.** By design it does not wrap — but this browser has no horizontal scrolling and never will, so text past the edge is not *preserved*, it is invisible. It now breaks at the box edge as a last resort, keeping the spacing and keeping the text reachable | fixed, and it is a deliberate departure from what `<pre>` means |
| **an ordered-list marker was placed with no width check at all** — the last run still escaping a box narrower than `10.` | fixed |
| **a font size could round to zero.** `h6` is `em * 9 / 10`, which is 0 for any `em` below 2; a zero line height makes a run taller than the line box containing it, so the document lays out above its own origin at a negative y | fixed |

None is reachable with the kernel's 16 and 32 pixel fonts. All four were real,
and the first two would bite a real page on a narrow window.

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

## The network path had no gate, and could not be reached at all

`STATE-OF-THE-PROJECT.md` §2.9 found that the browser's network path is covered
by no gate: no QEMU invocation in the repo attached a network card, so
`net_gate()` — a complete bring-up test bound to `N` at `kernel.zl:2124` — had
never run. That is correct. **It is also not the whole problem, and the fix it
proposes is not sufficient.**

§2.9's fix is "two QEMU flags in `try.sh`'s `COMMON[]` plus a `probe-net.py`
shaped like `probe-term.py`". The flags are necessary. They are not enough,
for two reasons that only showed up on the first run:

**1. Serial keystrokes cannot reach the shell in the shipped boot state.**
When there is a framebuffer the compositor is the top of the system and the
text shell's loop is never entered:

```
kernel.zl:3873   if wm_boot == 1 { ser_out("zl> ") }
kernel.zl:3901   if wm_boot == 1 { while wm_run() == 1 { wm_frame() }  running = 0 }
```

`wm_frame()` drains PS/2 and USB HID; nothing in the compositor path looks at
COM1. The `"zl> "` on the serial log is a **courtesy string** printed so
harnesses do not report "booted but no prompt" — its own comment says so. It is
not a shell waiting for input. Measured, with a NIC attached and the prompt on
the log:

```
sending 'h' -> ''      sending 'N' -> ''
sending 'W' -> ''      sending 'E' -> ''
```

Keys must go through QMP `input-send-event`, exactly as `probe-term.py`
documented for the same reason. (`probe-shot.py`'s `-k` flag sends keys over
serial, so it is subject to this too.)

**2. The three network gates had no typed name, so nothing could run them.**
`run_command` dispatches `N`/`W`/`E` (78/87/69 — the card and ARP probe, TCP +
HTTP/1.0, and a real website by name), and `term.c`'s word table had an entry
for none of them. That table's own header describes this exact failure:

> COMPLETENESS IS THE POINT, not convenience. Ten of run_command's commands had
> no name here … which was invisible while single keypresses still worked and
> became **a straight capability regression the moment the compositor became
> the boot state**: those ten could be reached from the text shell and from
> nothing else. Anything run_command dispatches should be typeable.

It happened again, to these three. So §2.9's "the ARP gate is written and
reachable, and nothing runs it" understates it: **in the shipped boot state it
was not reachable.** `net`/`arp`, `web`/`http` and `fetch`/`site` were added to
the table, and `probe-net.py` types `net` rather than pressing a key.

---

## HTTPS: the decision, and the file the audit said did not exist

`BROWSER-PROMPT.md` §5 says do nothing about HTTPS for now and say so on
screen, and explicitly forbids a half-TLS that skips certificate validation.
**That decision stands.** What follows is only the evidence about what exists,
because two documents disagree about it and one of them is wrong.

`docs/STATE-OF-THE-PROJECT.md` §7.1 says of `crypto.c`: *"**No such file has
ever existed in this repo.** Strike the citation."* **That is incorrect, and
the citation should not be struck.** The audit used `git ls-files | grep -i
crypto` and `git log --all`, and neither sees `refs/wip/*` — `--all` covers
`refs/heads`, `refs/remotes` and `refs/tags` only. Checked directly:

```
$ git for-each-ref --format='%(refname)' refs/wip |
    while read r; do git cat-file -e "$r:kernel/src/net/crypto.c" 2>/dev/null &&
    echo "$r HAS crypto.c"; done
refs/wip/zl-linux HAS crypto.c
refs/wip/tmp-wtclean HAS crypto.c
refs/wip/tmp-wtw0 HAS crypto.c

$ git cat-file -s refs/wip/zl-linux:kernel/src/net/crypto.c
21270
$ git show refs/wip/zl-linux:kernel/src/net/crypto.c | wc -l
543
```

543 lines — SHA-1, SHA-256, HMAC-SHA1, HMAC-SHA256, PBKDF2, AES-128 — plus
`hosttest/cryptotest.c` (246 lines) against **published** vectors: FIPS 180-1,
FIPS 180-4, RFC 2202, RFC 4231, RFC 6070, FIPS-197 C.1, RFC 4493 and IEEE
802.11i. So `BROWSER-PROMPT.md` §0/§5's citation of "543 lines" is exact.

**The audit's load-bearing conclusion survives intact**, and it is the one that
matters: `crypto.c` is in **no `SOURCES` on any ref**, checked across every
`refs/wip` and `main`. A file in no build is not in the kernel, so the shipped
kernel has no ciphers *and no hashes*. The browser's home page claimed "there
is no cipher in this kernel - only hashes", and the "only hashes" half was
false. That string is fixed.

Two distinct things, which is why the correction is worth writing down rather
than just fixing:

| | |
|---|---|
| "no crypto is linked into the kernel" | **true**, and it is why the HTTPS refusal is honest |
| "no crypto has ever been written here" | **false** — 543 gated lines exist in `refs/wip` |

Landing `crypto.c` is therefore a *decision*, not a *writing task*, and it is
not on the browser's critical path: TLS needs AES-GCM, ECDHE and X.509 chain
validation on top of what that file has, and §5 is right that this is a
separate track with its own review. **Nothing here depends on it yet.**

---

## Known limits of what is built

- `<pre>` does not wrap and has no horizontal scroll, so a long preformatted
  line is clipped. That is what `<pre>` means, and the clip is `fb_clip`, so
  nothing escapes — but it is a limit, not a feature.
- ~~Bold and oblique are **synthesised** — a double strike and a shear.~~
  **Superseded on the merge, twice.** Bold is now a REAL DRAWN WEIGHT: this
  tree ships `prop16b`/`prop24b`/`prop32b`, so `<strong>` is a bold face, not a
  double strike. **Oblique is still synthesised** — the upright atlas sheared
  about the cell bottom — so `<em>` leans but keeps roman letterforms, where a
  real italic redraws them. It is an oblique; calling it italic is the usual
  abuse of the word.
- Headings are **six sizes again**, not three or two. `layout.c` emits `em*2`,
  `em*3/2`, `em*5/4`, `em*11/10`, `em`, `em*9/10`; the text engine now takes a
  pixel height rather than a role, so all six survive. The largest atlas is
  32px, so h1 at `em` 24 is a 1.5× bilinear upscale and is measurably softer
  than a natively rasterized 48px would be. That was chosen over generating a
  `prop48` — see the decision note in `fb.c`.
- ~~Documents are capped at 24 KB, the node array at 1024 and the run array at
  2048.~~ **Every one of those three numbers was already wrong when you read
  it** — they were 256 KB, 8192 and 12288 by the time this line was last
  touched, and are now 2 MiB, 32768 and 65536. A cap restated in prose is a cap
  that goes stale in the optimistic direction, which is the whole reason
  `HTML_MAX_NODES` is exported from `html.h` instead of written down twice.
  **Ask the code:** `browser_doc_cap()`, `html_node_cap()`, `css_sel_cap()`,
  `lay_run_cap()`, and `hosttest/parsestat` to see what a real page does to
  them.
- The arrays are still fixed and there is still no heap — but they are no
  longer **this code's** arrays. `html.c`, `css.c`, `layout.c` and `png.c` are
  handed their storage by the caller, which is why the caps could move at all:
  they were 1.95 MB of a kernel BSS with 126 KB of link headroom left. See
  [`browser-storage-run.md`](evidence/browser-storage-run.md). Overflow still
  **truncates and says so on screen**; it does not scribble.
- Baselines are approximated by bottom-aligning the glyph cell. Correct only
  because every glyph comes from one atlas; wrong the moment a second face with
  different metrics arrives.

---

Brief: [`BROWSER-PROMPT.md`](archive/prompts/BROWSER-PROMPT.md) · Contract: [`../ui.h`](../ui.h)
