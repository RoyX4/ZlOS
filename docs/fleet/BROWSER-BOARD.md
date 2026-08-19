# The browser — what 25 agents found

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366`**

Wave 4: 8 lenses on parsing and rendering, 7 on the network stack, 6 on robustness and
security, 4 on capability and bounded scope.

**Items marked ✓ were re-derived by hand.** Everything else is a lead.

The browser is the project's headline capability — 2,617 code lines that fetch
`http://example.com/` by name off the real internet and render it. The wave's verdict:
**the engine is better than its harness, and the app around it is where the damage is.**

---

## The security answer is mostly a negative, and it is a real result

The `memory-safety` lens enumerated **all 33 fixed buffers** across the eight browser and
network files, traced every write whose length derives from network bytes, and verified
each clamp arithmetically rather than by reading its comment:

> **There is no unclamped network-derived write anywhere** in `html.c`, `layout.c`,
> `browser.c`, `http.c`, `tcp.c`, `net.c` or `dns.c`.

The `fuzz-html` lens agrees independently — *"`html.c` is genuinely hard to fault"*:
there is no tag-name buffer at all (`tag_of` compares in place, `html.c:86-91`),
attribute values pass through `arena_put`'s single bounds check, NUL is length-carried
rather than terminator-carried, and no non-advancing iteration could be constructed.

The `fuzz-css` lens enumerated **all 34 loops** in `css.c` and proved every cursor
strictly advances — malformed CSS cannot hang the parser.

And `dns.c` gets right the one thing the brief expected it to get wrong: `skip_name`
(`dns.c:185-200`) never follows a compression pointer at all, and `read_name` carries two
independent budgets (`MAX_STEPS=128`, `jumps>8`). **Structurally immune to the classic
DNS pointer loop.**

That is a genuinely defensible parser layer for hostile input in ring 0 with no heap.

### The exceptions

| ✓ | `http.c:89` | **[9-byte overflow of `req[512]`.](CRITICAL-http-request-overflow.md)** The one memory-safety hit, and it is an *outbound* buffer — which is likely why it survived reviews aimed at hostile input |
| | `css.c:124` | `trim_cs`'s trailing-comment recovery is **quadratic** — a 100 KB run of `/*` freezes the machine. Not live: `css.c` is not compiled |
| | `browser.c:103` | `sset` over-reads `html.c`'s deliberately NUL-free arena — a link click copies 127 bytes of whatever follows the href |
| | `http.c:94` | a raw CR/LF inside a quoted `href` reaches the request line unfiltered — **request splitting** |

---

## The app is where it breaks

All from [`CRITICAL-browser-cluster.md`](CRITICAL-browser-cluster.md) and
[`CRITICAL-browser-urlbar-keys.md`](CRITICAL-browser-urlbar-keys.md).

1. **✓ The CSS engine is dead code.** `css.c` + `csstest.c` = 1,032 lines, in no build,
   with no caller. The browser has no cascade. `land-gate.sh` flagged it; it was
   explained away alongside `crypto.c`, for which the explanation was correct.
2. **✓ Pointer motion navigates.** `kernel.zl:2994` calls `br_click` on every `EV_MOUSE`
   without checking the button mask. Hovering a link fires a fetch.
3. **✓ The URL bar double-types, and Enter/Esc/Backspace are dead on PS/2.**
   `kernel.zl:2993` returns before the `nav_to_char` translation — which has exactly one
   call site, and the browser is not it.
4. **A 3xx wedges the browser in "fetching…" forever.** `HTTP_REDIRECT` exists in
   `http.h:21` and no branch in `browser_tick` consumes it. **Three lenses reported this
   independently.**
5. **Any response over ~49 KB deadlocks the fetch permanently** (`http.c:227`) —
   `tcp_recv(resp, 0)` is a no-op, so a full buffer stops draining TCP. **Two lenses,
   independently.** No crafted input needed.
6. **`parse_url` has no base URL**, so `href="/about"` parses `about` as a hostname and
   fails as a DNS error — the wrong diagnosis for the right problem.

Items 4 and 5 together mean the two most common non-trivial HTTP outcomes — a redirect
and a largish page — both end in a permanent, silent hang.

---

## Where the docs are optimistic

**`net-ip` lens, two brief claims wrong in the optimistic direction.** `browser-status.md`
says every drop path is now named and counted after the six-hypothesis investigation.
Half true: `handle_ip`'s six drops are counted and wired end to end
(`net.c:346-362` → `runtime_kernel.c:1302-1305` → printed at `kernel.zl:2859-2860`). But
`handle_arp` (`net.c:287,288,289`), `handle_icmp` (`:305`) and `net_poll_once`
(`:379`, `:383-385`) **still drop with a bare `return` and no counter** — the exact
defect `net.c:58-64`'s own doctrine block was written about.

And `hosttest/nettest.c:392`: three of the four counters that investigation produced have
**no assertion**.

**`tcp` lens:** `browser-status.md:78` says `tcp.c`+`tcp.h` is 752 `wc -l`; actual is
**900**. The adversarial-review fixes grew the file ~150 lines and the table was never
re-measured.

**`dns` lens:** `dns.c` is absent from the ASan/UBSan fuzzer (`hosttest/build.sh:191`)
while a doc says it is covered.

---

## Bounded scope — what is missing, costed honestly

Every lens was required to cost the **bounded** version and say what it excludes,
because this file's own existence traces to a doc that called a browser *"unbounded —
Chromium is 6.38M lines"* and was then built in 2,617.

| capability | bounded version | excludes |
|---|---|---|
| **HTTPS** | **Not new work.** Reportedly already built and in `SOURCES` on `desktop/browser-next`. The real blocker is **entropy** — `cpu.c:144` detects RDRAND, `crypto.c` is 543 tested lines with no fuel. Also `tcp.c:39`: the whole receive buffer is smaller than one maximum TLS record | cert pinning, revocation, a trust store the OS has nowhere to persist |
| **CSS** | wire the existing 704 lines: retain `class`/`id`/`style` in `html.c` (currently only `href` and `alt` survive, `html.c:421-423`), add to `SOURCES`, call from `layout.c`'s `walk()` | the 17 properties `layout.c` can act on; no floats, no tables, no grid |
| **Images** | ~80 lines for BMP/PPM; ~700 for PNG (inflate ~400 + chunk/filter ~300). Compositing already exists | JPEG, animation, colour management |
| **Forms** | ~150 lines. The URL bar already proves text entry works — point that machinery at an in-document field | file upload, JS validation |
| **Scripting** | `interp_kernel.c` is **721 lines of zl interpreter already in the kernel**. A DOM binding gives scripted documents in zl | **this is not JavaScript and not web compatibility.** Say so plainly |
| **Chunked encoding** | neither decoded nor *detected* — chunk-size lines render as text | — |

---

## Suggested order

1. **`kernel.zl:2994`** — check the button mask. Hover-navigates makes the browser
   unusable and it is a one-line guard.
2. **`kernel.zl:2993`** — delete it and let the browser reach `nav_to_char`. Fixes
   double-typing and the dead nav keys together. Check `APP_SNAKE`/`APP_MENU` first —
   [already done](CRITICAL-browser-urlbar-keys.md), both are safe.
3. **`http.c:89`** — the overflow, with the `_Static_assert`.
4. **`http.c:227` and `HTTP_REDIRECT`** — the two permanent hangs.
5. **Counters on `handle_arp`, `handle_icmp`, `net_poll_once`**, and assertions on the
   three unasserted ones. Small, and it closes the gap between the doctrine and the file
   that wrote it.
6. **Then** wire CSS — and fix `css.c:592`/`css.c:124` as part of wiring it, not after.
