# The lens wave 4 lost — `web:trust-boundary`

**Re-run 2026-08-19, by hand, not by an agent.** `README.md` records that wave 4
spawned 26 agents, 25 returned, and `web:trust-boundary` died to an API server
error mid-response — so the browser board is missing the one lens that asks *where
does untrusted data enter, and what decides what happens next.*

It was re-run in this session **without a fan-out**: one lens over four files is
work sized for one context, and the box was at load 17 with five sibling sessions
live. Everything below was derived directly from the tree at `main` @ `3f00366`.

**A lens that reports only bugs misleads about where the risk is.** Most of this
stack's trust boundary is genuinely well defended, and that is recorded here with
the same weight as the one defect, because the next person to harden this code
needs to know which doors are already shut.

---

## 1. The finding: fix order between two known bugs decides local vs remote

Neither bug below is new. **The coupling is**, and neither of the two documents
that own them mentions it.

`build_request` (`http.c:89`) overflows `req[512]` by 9 bytes. Re-derived here
independently of the agent that first reported it, because this repo's rule is to
reproduce before repeating:

```
REQ_MAX = 512
  "GET "                                    n = 4
  path loop, guard n < REQ_MAX-64  (448)    n -> 448 max
  " HTTP/1.0\r\nHost: "  17 bytes, UNGUARDED  n = 465
  host loop, guard n < REQ_MAX-32  (480)    n -> 480 max
  "\r\nUser-Agent: zlOS\r\nConnection: close\r\n\r\n"
                         41 bytes, UNGUARDED  n = 521   <-- 9 past req[512]
```

Confirmed: `http.c:94` guards the path loop, `:97` guards the host loop, and the
two literal writes at `:95-96` and `:101-102` are guarded by nothing.
`docs/fleet/CRITICAL-http-request-overflow.md` has it right.

That file files it as an **outbound** buffer, and notes this is "likely why it
survived reviews aimed at hostile input." That is correct **today**, and only
because of an unrelated bug:

```
$ grep -rn 'HTTP_REDIRECT' kernel/*.c kernel/*.h freestanding/*.c | grep -v '^kernel/http'
(no output - produced by http_poll:267, consumed by nobody)
```

`http_poll` reaches `HTTP_REDIRECT` at `:264-268`, and nothing in `browser.c` or
anywhere else acts on it. So a 3xx never fires a second request — which
`CRITICAL-browser-cluster.md` records as its own bug, *"3xx hangs forever"*.

Meanwhile `parse_headers` (`:173-180`) copies `Location:` **from the wire** into
`location[URL_MAX]`. That copy is correctly bounded to 255 chars.

**Put those three facts together.** The overflow is driven by `host` + `path`.
Today those come only from the URL bar, so the bug is local: a user has to type a
long enough URL. The moment anyone fixes the redirect — an obvious next step,
already on the board as a bug — a hostile server's `Location:` header becomes the
source of `host` and `path`, and **a 9-byte out-of-bounds write becomes remotely
triggerable by any site you visit.**

> **Fix `build_request` before fixing the redirect.** Fixing them in the other
> order, or fixing only the redirect, converts a local overflow into a remote one.

Neither `CRITICAL-http-request-overflow.md` nor `CRITICAL-browser-cluster.md`
says this, because each owns one half. That is exactly the seam a trust-boundary
lens exists to see, and it is why losing this lens mattered.

The fix itself is unchanged and small: guard the two literal writes, or size
`req[]` from the real maximum (`4 + 255 + 17 + 255 + 41 = 572`) instead of a
round number.

---

## 2. Defended — recorded so the risk map is not misleading

Each of these was checked and each holds. Do not spend hardening effort here.

**`http_poll` ingest is correctly clamped** (`:224-230`). `room = HTTP_BUF -
resp_len`, `avail` is clipped to `room`, and over-length responses set
`truncated` rather than writing past `resp[32768]`. The `Content-Length` path at
`:251-252` can only ever *shrink* `resp_len`, because the assignment is guarded
by `resp_len - body_at >= content_len`. A lying `Content-Length` cannot extend
the buffer, and `:162` refuses an absurd one outright.

**`type_acceptable()`** (`:198`) aborts the connection on a non-page content type
*before* the body fills the buffer — the right place for that decision.

**`dns.c` is the strongest file in the stack on this axis**, and it documents its
own trust boundary at `:12-27` rather than leaving it to be discovered:
compression pointers are bounded (`skip_name:185` does not follow the chain at
all, which is the simplest possible cycle immunity; `read_name:204` carries both
a step and a jump budget), reserved label types are rejected (`:195`), and the
transaction id, source, port and question are all checked rather than trusting
"the next UDP packet". It also states the residual weakness plainly — no DNSSEC,
no 0x20 encoding, and a tick-derived transaction id that an off-path attacker who
can guess it could poison. **That is the honest shape.**

**`html.c` cannot be made to blow the kernel stack.** This matters more here than
in a hosted browser, because `js.c:22-24` records that this kernel's compositor
already overflowed a 16 KiB stack once.

- `MAX_NODES 1024`, checked at `:127`
- `MAX_DEPTH 32`; `push:196-200` drops over-deep elements and **counts them in
  `n_dropped`** rather than growing the stack or silently corrupting it
- `close_to:213-231` cannot underflow `sp`: it searches for a matching open
  element and returns 0 without touching `sp` when there is none, so a stray
  `</div>` unwinds nothing

Consequence worth stating because nothing else in the tree does:
`layout.c:walk` recurses at `:483` on `html_first(n)`, and its depth is therefore
bounded by the tree depth, which `push`'s cap holds at ~32. **`walk` cannot be
driven into a stack overflow by nested markup.** That is a property of `html.c`,
not of `layout.c`, and it would be silently lost if anyone raised `MAX_DEPTH`
without looking at `walk`.

---

## 3. Noted, not a finding

`http_body_addr()` (`:278`) narrows a pointer to `u32`, deliberately and with the
reasoning written at `:30-33` — zl reads the body through a 32-bit address, and
the comment is explicit that it must narrow from the *full* pointer rather than
from something already truncated. That is a sound decision on the 32-bit target.

The observation is not about the decision. It is that `http.c` is one of the ~44
translation units `tools/hazard-scan.sh` never opens
(`CRITICAL-ci-truncation-gate-is-blind.md`), so **the gate for this repo's #1 bug
class could not have told you either way.** The narrowing here is fine; the point
is that nothing checked it.

---

## 4. What this lens did not cover

- **`css.c` was not examined as a trust surface**, because it is in no build and
  has no caller (`CRITICAL-browser-cluster.md`). If it is wired up, its 704 lines
  become network-facing and need this lens run against them *then* — a parser
  that has never seen hostile input is not a parser that has survived it.
- **`tls.c` / `x509.c` / `png.c` are not in this stack on `main`.** They live on
  `desktop/browser-next`, which this run does not cover. The PNG decoder in
  particular is a classic trust boundary and has had no lens run on it at all.
- **Nothing was executed.** No build, no gate, no QEMU — the fleet's standing
  constraint. Every claim here is from reading the tree, and the overflow
  arithmetic is derivation, not a crash I observed.
