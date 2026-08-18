# The browser track — and why it is not the impossible one

The fifth brief, and the one the existing docs say cannot be done. They are
right about the thing they measured and wrong about the thing you want.

---

## How to start it

Open a session in `~/Documents/repos/zl-browser` and paste only this:

```
Read kernel/docs/BROWSER-PROMPT.md in full and execute it exactly as written.
You are in a git WORKTREE on branch desktop/browser - confirm with
`git branch --show-current` before your first commit. Other sessions run in
sibling worktrees; §3's ownership split keeps you apart. §1 is a hard
prerequisite - read it before writing any browser code. Work the queue in
order, gate every item, commit on green. Agents are authorised; §8 says how.
Do not stop until I tell you to.
```

---

## 0. The honest framing, because the docs currently get this wrong

`feature-catalogue.md` lists a web browser as ❌ and
`OVERNIGHT-PROMPT.md` explains why:

> **A browser** | Unbounded. Chromium is 6.38M lines, Ladybird 314K, all of
> zlOS is 11,374

**Every number there is correct and the conclusion does not follow.** It
measures the *maximal* version of the capability and reports it as the
capability. Chromium is unbounded. Ladybird is unbounded *for this project*. A
browser is not.

The same document contains its own counter-example, four rows down:

> **Fits on a floppy** | MenuetOS, KolibriOS (1.44 MB, *with* a GUI **and
> browser**)

KolibriOS ships an operating system, a windowing GUI **and a web browser** in
**1.44 MB**. That is an existence proof sitting in the project's own feature
table, unremarked.

**So the question is not "can zlOS have a browser".** It is *which* browser,
and the honest answer is a line count, not a yes or a no.

### What is genuinely out of reach, and why

| | why |
|---|---|
| **JavaScript** | a JS engine is its own multi-year project. Not "hard" — a different project. |
| **HTTPS** | `crypto.c` has SHA-1, SHA-256, HMAC and PBKDF2 — **543 lines of hashing and no cipher**. TLS needs AES-GCM, ECDHE and certificate-chain validation. That is a track of its own, and doing it *badly* is worse than not doing it. |
| **Full CSS** | the cascade, specificity, float, flex, grid, and a box model with margin collapsing. |

### What is genuinely reachable, and why

**zlOS already has the two hardest parts of a browser**, which is the thing
nobody has noticed:

- **A layout engine.** `ui.c` is 395 lines of flowing-cursor layout — place a
  box, advance the cursor, wrap the row at the content width. That is
  *precisely* inline text flow, which is what an HTML document mostly is.
- **A text renderer that is genuinely good.** Real TrueType glyphs,
  gamma-correct linear-light blending, subpixel LCD rendering, **proportional
  advances**, a clip rectangle and damage tracking. Rendering a paragraph is a
  solved problem here.

A browser on top of those is a **fetcher, a parser and a box model**.

---

## 1. HARD PREREQUISITE — read before writing a line

**There is no network driver.** None. No e1000, no virtio-net, no rtl8139 — and
the desktop header draws `net up`, which is decorative (finding 0.4 of
`SYSTEM-PROMPT.md`).

**A browser with no network is a file viewer.** That is not nothing — see Item
0 — but do not build the rest before the transport exists.

The system track's item 7 may be building `virtio-net`. **Check first**:

```bash
git log --all --oneline | grep -i "virtio-net\|network"
ls kernel/net*.c kernel/virtio_net.c 2>/dev/null
```

If it exists, use it. If it does not, **Item 1 below is yours** and you should
tell the system track so you do not both write it.

**The pattern is already proven in-tree**: `virtio_gpu.c` is 495 lines and
drives a virtqueue — descriptor table, avail ring, used ring, 37 references.
`virtio-net` is the same shape with two queues instead of one. That is why it
is the realistic choice and not e1000.

---

## 2. The honest budget

| piece | lines | leaning on |
|---|---|---|
| `virtio_net.c` | ~400 | the virtqueue pattern in `virtio_gpu.c` |
| ARP + IPv4 + ICMP | ~300 | — |
| TCP, **client only, one connection** | ~900 | the hard one. See §5. |
| HTTP/1.0 client | ~150 | — |
| HTML tokenizer + tree | ~450 | no heap: a fixed node array |
| box model + inline flow | ~500 | **`ui.c` does most of this already** |
| the browser app | ~350 | `wm.c`, `ui.c`, `term.c` for the URL bar |
| **total** | **~3,050** | against zlOS's current 11,374 |

**About a quarter of the existing OS, to add a browser.** That is a real
number, honestly arrived at, and it is the thing to decide on — not a
philosophical question about whether it is possible.

Compare: the compositor, toolkit and terminal together were ~2,000 lines and
took a day and a half.

---

## 3. OWNERSHIP

| yours | others |
|---|---|
| `kernel/virtio_net.c`, `net.c`, `tcp.c` *(new)* | `ui.c`, `gen_*` → look |
| `kernel/http.c`, `html.c`, `layout.c` *(new)* | `term.c`, `smp.c` → platform |
| `kernel/browser.c` *(new)* | `cursor.c`, `settings.c` → feel |
| | `fs.c`, `rtc.c`, `clip.c` → system |

**Shared:**
- **`ui.c`** — you will want to *read* its layout logic. **Do not change it**;
  the look track owns it. If you need a variant, write `layout.c` and say in
  the comment which parts you took and why they could not be shared.
- **`kernel.zl`** — you own **only** the browser app's `app_draw` branch and
  its command. One contiguous block.
- **`nvme.c` / `fs.c`** — the system track owns these. A page cache is a
  *later* nicety; do not build one until a real filesystem exists.

---

## 4. THE WORK QUEUE

### Item 0 — a browser with no network, first

**Do this before the driver.** Render a local HTML file from the RAM
filesystem, in a window.

It de-risks the entire back half: the parser, the box model and the renderer
get finished and gated with **no network variables in the picture at all**. If
the layout is wrong, you find out now, not while also debugging a TCP
retransmit.

**Gate:** a hand-written HTML file with headings, paragraphs, bold, a list and
a link renders in a window, and the window can be **resized and the text
reflows**. Reflow is the assertion that proves it is a layout engine and not a
fixed drawing.

### Item 1 — `virtio-net` (only if the system track has not)

Two virtqueues, receive and transmit. Follow `virtio_gpu.c`'s shape exactly —
the descriptor/avail/used dance is identical and already debugged once.

**DMA buffers must be checked against the high-RAM map** at the top of `fb.c`:
bg 128 MiB, sp 160, sched 176, back 192, nvme 208, xhci 224, virtio-gpu 240. A
fixed address colliding with a neighbour is **this project's recurring bug,
five times so far**. Do not guess an address; compute it from the map and
assert it.

**Gate:** send an ARP request in QEMU's user-mode network, receive the reply,
print both MACs. That is link-up proven end to end, and it is a two-frame test.

### Item 2 — ARP, IPv4, ICMP

Get `ping` working against QEMU's gateway (10.0.2.2).

**Gate:** an ICMP echo round-trip with a plausible time, printed in the
terminal. Then run it 20 times and report **loss and jitter** — a stack that
works once and drops every fourth packet looks identical on a single ping.

### Item 3 — TCP, client, one connection

The hard one. Scope it hard:

- **one** connection at a time, client only, no listen
- a fixed receive buffer, no reassembly queue beyond in-order + one hole
- retransmit on timeout, no fast-retransmit, no SACK
- congestion control: **slow start only**, no AIMD tuning

**The trap:** TCP is a state machine and the tempting shortcut is to write it
as a straight line of `if`s. Write the state enum first, then the transitions.
Every bug you avoid here is one you would otherwise find with a packet capture
and a bad afternoon.

**Gate:** a host harness — `hosttest/tcptest.c` — driving the state machine
against **scripted packet sequences** with no QEMU. Include the ugly ones: a
SYN-ACK that never comes, a duplicate ACK, a FIN mid-transfer, a segment
arriving twice. This is exactly the code where a harness beats a boot by an
order of magnitude.

### Item 4 — HTTP/1.0

`GET`, `Host:`, follow up to 5 redirects, read `Content-Length`, refuse
anything not `text/html` or `text/plain` for now.

**HTTP/1.0, not 1.1** — no chunked encoding, no keep-alive, no pipelining.
Close the connection to end the body. That is a decision, not a limitation: it
removes an entire class of parsing from the first version.

**Gate:** fetch a page from a local `python3 -m http.server` and print the
headers and byte count.

### Item 5 — the HTML parser

**No heap.** A fixed array of nodes with parent/child *indices*, not pointers —
same shape as `wm.c`'s `zorder`, and for the same reason.

Support, and no more: `html head body title h1..h6 p br hr a ul ol li strong em
b i code pre div span img`.

**Be liberal.** Real HTML is malformed. Unclosed `<p>`, stray `</div>`,
attributes without quotes. Never fault on bad input — **recover and carry on**,
because a browser that refuses a page is a browser nobody uses.

**Gate:** parse a deliberately malformed document and produce a sane tree.
Assert the node count and the depth, not "it did not crash".

### Item 6 — layout, leaning on what exists

Two box types only: **block** (stacks vertically, full width) and **inline**
(flows, wraps at the content width). That is `ui.c`'s flowing cursor with a
different caller.

Then: font size per heading level, margins from a small fixed table, and
`<a>` in the accent colour.

**Gate:** the same document at three window widths. Text reflows; nothing
overlaps; nothing escapes the client area — the last one is free, because
`fb_clip` guarantees it.

### Item 7 — the browser app

URL bar (reuse `term.c`'s line-editing shape), Back, and a scroll region
(`ui_scroll_begin`/`ui_scroll_end` already exist and are asserted).

**Gate:** type a URL, fetch, render, scroll, go back. Screenshot it.

---

## 5. What to do about HTTPS

**Nothing, for now, and say so on screen.**

Most of the real web is HTTPS-only, so a plain-HTTP browser reaches local
servers, a lot of documentation mirrors, and not much else. That is worth
having and it is worth being honest about.

Do **not** write a half-TLS that skips certificate validation. A browser that
shows a padlock it has not earned is worse than one that says "http only".
`crypto.c` gives you the hashes; AES-GCM and ECDHE and chain validation are a
separate track with its own review.

**When a URL is `https://`, refuse it and say why.** Same standard as the
`net up` label.

---

## 6. What "done" means

- a gate command **and its output**
- a **screenshot** for anything rendered
- for the network stack, a **host harness** — packet sequences beat boots
- a number you **measured**

---

## 7. Hazards

- **No heap, no lists, no runtime strings** in the zl kernel subset. The parser
  and the stack live in **C**; zl gets the browser app's policy only.
- **Multi-KB buffers in BSS are fine; multi-MB go in the high-RAM map** — and
  check the map before choosing an address.
- **Network buffers are DMA.** The card writes them. Same rules as `xhci.c` and
  `nvme.c`, and the same recurring bug class.
- **Four build scripts, four source lists** — `build.sh`, `build64.sh`,
  `buildefi.sh`, `mkdisk.sh`. This track adds *seven* files. Add each to all
  four and build all four, or you will discover it two days later.
- **The zl parser takes no multi-line call arguments.**

---

## 8. Agents

Authorised, and this track has two ideal uses:

- **An adversarial reviewer on the TCP state machine.** Concurrency and
  protocol state are exactly where a fresh reader earns their cost. Have one
  try to construct a packet sequence that wedges it.
- **Fan out readers** over `virtio_gpu.c`, `xhci.c` and `nvme.c` to extract the
  common DMA/queue idiom before writing `virtio_net.c`. Three drivers already
  solved this; do not invent a fourth shape.

**Not** for parallel QEMU boots — five sessions now share a 4-core box.

**If you delegate, you own collection.** Never end a turn waiting on an agent.

---

## 9. Update the docs when you know more

`feature-catalogue.md` says a browser is ❌ and `OVERNIGHT-PROMPT.md` says
unbounded. **Both should be corrected to say what is actually true**: the
maximal version is out of reach, a bounded one is ~3,000 lines, and here is
where it got to.

That correction is part of the work, not an afterthought. This project already
had to publicly correct a "95% achievable" that was really 20% (DECISIONS.md
#26) — the same standard applies in the optimistic *and* the pessimistic
direction.

---

System track (network, storage): [`SYSTEM-PROMPT.md`](SYSTEM-PROMPT.md) ·
Contract: [`../ui.h`](../ui.h) · Feature table:
[`feature-catalogue.md`](feature-catalogue.md)
