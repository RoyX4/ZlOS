# Verification log — fleet findings checked by hand against the tree

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366`**

Findings that were re-derived from source by the orchestrating session rather than
taken on the agent's word. Both outcomes are recorded — confirmations and corrections —
because a run that only publishes its hits is not evidence of anything.

Larger items have their own files, linked from [`README.md`](README.md).

---

## CONFIRMED, narrowed — `fbbench.c`'s hash scene omits the alpha-blend primitives

**Agent claim** (lens `fb-blend`, severity `high`):
> *The only correctness net over `fb.c` hashes a scene containing zero translucency
> primitives.* — `kernel/tests/host/fbbench.c:360`

**Too strong as stated.** The hashed `scene()` (`fbbench.c:365-377`) does contain
blending work: `fb_shadow` at line 369, and `fb_text_aa` at 355 which goes through
coverage blending. So "zero translucency primitives" is false.

**The narrowed claim is true and is the one that matters.** The two explicit
alpha-blend primitives are declared at `fbbench.c:172-173`:

```c
void fb_fill_blend (int x, int y, int w, int h, unsigned int rgb, int a);
void fb_rrect_blend(int x, int y, int w, int h, int r, unsigned int rgb, int a);
```

and they appear in the file **only** as timing benchmarks:

```c
/* fbbench.c:323-324 */
static void b_blend(void)   { fb_fill_blend (200,200,600,460, 0x0055D6FF, 160); }
static void b_rrblend(void) { fb_rrect_blend(200,200,600,460, 10, 0x0055D6FF, 160); }
```

Neither is called from `scene()`. So the hash — the only *correctness* net over `fb.c`
— never executes `fb_fill_blend` or `fb_rrect_blend` with any alpha value at all.

**Why that is the expensive gap specifically.** `kernel/docs/gpu-next.md` measures
`fill_blend` at **122 Mpix/s against the GPU's 5930 — a 48× gap** — and identifies alpha
blending as the operation the whole visual direction rests on. It is simultaneously the
most performance-critical primitive in `fb.c` and the one with no correctness coverage.

**And the scene's own comment overclaims it** (`fbbench.c:361-364`):

> *"Every primitive that has a correctness risk appears at least once."*

That sentence is false for the two primitives with the largest correctness risk in the
file. Correct the comment whether or not the scene changes — right now it is evidence
*for* a claim the code refutes.

**Fix:** add one `fb_fill_blend` and one `fb_rrect_blend` at a mid alpha (160 matches
the existing benchmark) into `scene()`, regenerate the golden hash in a single
deliberate commit, and diff it on purpose. The repo's own rule applies: changing the
scene changes the number, so do it once and say so.

---

## CONFIRMED — `anim_tick` counts frames, not time, and the clock is already in the file

**Agent claim** (lens `motion`): *`anim_tick()` advances exactly one table index per
compositor pass, never per unit of elapsed time.*

**Confirmed.** The whole advance is one line:

```c
/* kernel/src/graphics/windowing/wm.c:431-445 */
static void anim_tick(void)
{
    for (int i = 0; i < ANIM_MAX; i++) {
        if (!anims[i].kind) continue;
        wm_damage_win(anims[i].win);
        ...
        if (++anims[i].frame >= anims[i].len) anims[i].kind = ANIM_NONE;
    }
}
```

No time source is consulted. `anim_tick` has one caller, `wm.c:1598`, once per
compositor pass. So `anims[i].len` is a count of *passes*, and `wm.c:299`'s
*"four frames at 100 Hz is 40 ms"* holds only if a pass costs exactly 10 ms.

**The fix is small because the clock is already here.** `idt_ticks()` is declared at
`wm.c:133` and used by other code in the same file:

```
wm.c:1070:  * where windows are. idt_ticks() is 100 Hz, which is ample: the window is 40
wm.c:1085:      unsigned now = idt_ticks();
wm.c:1518:  * idt_ticks() is 100 Hz, which is 10 ms of resolution against a 16.67 ms
```

Two subsystems in `wm.c` already reason correctly about the tick's 100 Hz / 10 ms
resolution. The animation timeline is the one that does not use it. `anim_tick` should
sample `idt_ticks()` and advance `frame` by elapsed ticks, exactly as `wm.c:1085` does.

**Watch out for the name collision:** `kernel.zl:1146` also defines `fn anim_tick(aw, ah)`,
which is the Animation *app* (`kernel.zl:3276`) and is unrelated. Patching that one does
nothing for the compositor.

---

## CONFIRMED — the HTTP body pointer is narrowed to 32 bits, and C code widens it back

**Agent claim** (bug class `llp64`, severity `high`): *`http_body_addr()` narrows
`&resp[body_at]` to `u32` and two reachable callers widen it back and dereference it.*

**Confirmed.**

```c
/* kernel/src/net/http.c:278 */
u32 http_body_addr(void)  { return (u32)(uptr)(resp + body_at); }
```

The narrowing is **deliberate and documented** — `http.c:31` says it *"narrows to u32 ON
PURPOSE (zl reads the body through a …)"*, because zl numbers are doubles and the
builtin at `runtime_kernel.c:1278` has to return one. For the zl path that is a
considered trade.

**The defect is the C caller:**

```c
/* kernel/src/web/browser.c:426 */
doc_set((const char *)(uptr)http_body_addr(), http_body_len());
```

`browser.c` is C. It has direct access to the real pointer, and instead round-trips it
through the 32-bit accessor and re-widens the result. If UEFI loads `BOOTX64.EFI` above
4 GiB, this reconstructs a pointer with its top 32 bits zeroed and copies
`http_body_len()` bytes from it. **Silent** — no fault, per this repo's own rule that an
out-of-bounds read that does not fault landed in some other mapping.

None of `buildefi.sh`'s four `-Werror=` flags fire, because the narrowing and the
widening are in different translation units and each is individually well-formed.

**Fix:** `browser.c:426` should take the body pointer directly rather than through the
zl accessor — add a `const char *http_body_ptr(void)` for C callers and leave
`http_body_addr()` to zl. The agent reports this is the only surviving instance of the
"widen a 32-bit accessor result back into a pointer" shape in the EFI source set.

---

## CONFIRMED — Alt+Tab never fires. One token.

**Agent claim** (lens `wm-focus`, severity `high`): *`route_key` tests `'\t'` (9) but
both keyboards emit `KEY_TAB` (0x103).* — `kernel/src/graphics/windowing/wm.c:1450`

**Confirmed, and the repo documents the rule that is being broken.**

```c
/* kernel/src/graphics/windowing/wm.c:1448-1452 */
static void route_key(int type, int code, int mods)
{
    if (type == EV_KEY_DOWN && code == '\t' && (mods & MOD_ALT)) {
        cycle_focus();
        return;
    }
```

`'\t'` is 9. But `kernel/src/drivers/input/keycodes.h:10-21` is explicit that key codes live above 0x100
*specifically* so they cannot be confused with characters:

> *"…above 0x100 where it cannot collide with a character. That boundary is load
> bearing: `code >= KEY_NONCHAR` is exactly the test for 'this key has no character',
> and `input.c`, `xhci.c` and the zl shell all rely on it."*

```c
#define KEY_NONCHAR   0x100
#define KEY_TAB       0x103
```

Both keyboards deliver `KEY_TAB`, not 9:

| path | site | result |
|---|---|---|
| PS/2 set 1, scancode `0x0F` | `input.c:155` | `return KEY_TAB;` |
| PS/2, scancode `0x2B` | `input.c:227` | `return KEY_TAB;` |
| USB HID usage `0x2B` → char 9 | `xhci.c:1508` → `input.c:633` `key_of_char` | `return KEY_TAB;` |

`route_key` is fed by `input_code()` (`wm.c:1507`), which returns `last.code`
(`input.c:786`) — the **key** code. `0x103 != 9`, so the branch is unreachable from any
keyboard the OS supports.

**Fix:** `code == KEY_TAB`. One token, and `wm.c` already includes the header that
defines it.

**Why it is worth flagging beyond itself:** `docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md` ranks
*"Fix `key()` at `kernel.zl:1407` — one token"* third on the whole-project board. This
is a **second instance of the same class** — a character literal used where a key code
is required — in a different file. Worth a sweep for the rest: any comparison of an
event `code` against a character literal for a key that has a `KEY_*` constant.

---

## CONFIRMED — `intel.c` has no `static` functions at all

```
$ grep -cP '^static\s+[\w \*]*\bintel_\w+\s*\(' kernel/src/drivers/display/intel.c
0
$ grep -cP '^(?!static)[A-Za-z_][\w \*]*\bintel_\w+\s*\(' kernel/src/drivers/display/intel.c
308
```

All 308 function definitions in a 5,182-line file are exported. Consequences: the
compiler cannot report dead code in this file, there is no interface/implementation
boundary to reason about, and the orphan count is therefore soft — some of the 101
call-site-free functions are unfinished display phases and some are internal helpers
that were never marked `static`.

Detail and the orphan grouping: [`GROUND-TRUTH-CORRECTIONS.md`](GROUND-TRUTH-CORRECTIONS.md).

---

## CORRECTED — `snap_release()` does have a shipping caller

**Agent claim** (lens `wm-drag`, severity `critical`): *"`snap_release()` has no
reachable caller in the shipping tree at all."*

**False.** `kernel/src/graphics/windowing/wm.c:1281` calls it. The agent's *conclusion* — that drag-to-edge
snapping is a one-way door — is nonetheless **correct**, via the `z != SNAP_NONE` guard
at `wm.c:1358`.

Full chain, the duplicate `SNAP_NONE` definitions, and the fix:
[`VERIFIED-WM-SNAP.md`](VERIFIED-WM-SNAP.md).

This is the reference case for why the boards are not authoritative: right conclusion,
false evidence, and the false evidence is what a reader would have quoted.

---

## CONFIRMED — `HTTP_REDIRECT` is produced and consumed by nobody

**Agent claim** (lenses `browser-app`, `http`, `memory-safety` — three, independently):
*a 3xx wedges the browser in "fetching…" forever.*

**Confirmed.**

```
$ grep -n "HTTP_REDIRECT" kernel/*.c kernel/*.h
kernel/src/net/http.c:267:        state = HTTP_REDIRECT;
kernel/src/net/http.h:21:    HTTP_REDIRECT,     /* 3xx with a Location, under the redirect limit */
```

`http.c` **enters** the state. `browser_tick` never leaves it:

```c
/* kernel/src/web/browser.c:422-432 */
int s = http_poll();
if (s == HTTP_DONE)    { fetching = 0; ... return 1; }
if (s == HTTP_REFUSED) { fetching = 0; status = BR_BAD_TYPE; return 1; }
if (s == HTTP_ERROR)   { fetching = 0; status = BR_FAILED;   return 1; }
return 0;
```

Three terminal states handled out of four. On `HTTP_REDIRECT`, `fetching` stays 1,
`status` is never set, and `browser_tick` returns 0 on every subsequent call. There is no
timeout on that path.

**The sharp part is the comment.** `http.h:21` reads *"3xx with a Location, **under the
redirect limit**"* — this state exists for the case where the redirect is **valid and
followable**. `http.c` does the work of detecting a 3xx, finding the `Location`, and
checking the redirect count, and then hands its caller a state the caller has never heard
of. The success path of a feature that was deliberately built is the one that hangs.

**Fix:** in `browser_tick`, on `HTTP_REDIRECT`, call `navigate(http_location(), …)` —
`http_location()` already exists at `http.c:283`. That is what the state was for.

---

## CONFIRMED — a full response buffer deadlocks the fetch, because a correct guard disables the drain

**Agent claim** (lenses `http` and `memory-safety`, independently, severity critical):
*any response over ~49 KB deadlocks the fetch permanently — `tcp_recv(resp, 0)` is a
no-op, so a full buffer stops draining TCP.*

**Confirmed.** Two functions, each individually reasonable.

```c
/* kernel/src/net/http.c:224-230 */
int avail = tcp_available();
if (avail > 0) {
    int room = HTTP_BUF - resp_len;
    if (avail > room) { avail = room; truncated = 1; }
    if (avail > 0) resp_len += tcp_recv(resp + resp_len, avail);
    else           tcp_recv(resp, 0);      /* ← meant to drain and discard */
}
```

```c
/* kernel/src/net/tcp.c */
int tcp_recv(u8 *out, int max)
{
    if (max <= 0) return 0;              /* a trust boundary, so it is checked */
    ...
}
```

The `else tcp_recv(resp, 0)` is clearly *intended* to keep draining the TCP receive
buffer and throw the excess away once `truncated` is set — that is the only reading of
why it is there at all. But `tcp_recv`'s `if (max <= 0) return 0` guard — which is
**correct**, and whose comment correctly identifies it as a trust boundary — makes the
call a no-op.

So once `resp_len == HTTP_BUF`: `room` is 0, `avail` becomes 0, the `else` fires, nothing
drains, the TCP receive window never reopens, and `state` stays `HTTP_RECEIVING`
**forever**. No timeout exists on that path.

**This is the most instructive bug in the run.** Neither function is wrong. A defensive
guard added at a trust boundary silently disabled a drain in a caller that was written
assuming `tcp_recv(buf, 0)` meant *"discard everything available."* Nothing in either
file records that assumption, so nothing could have flagged its removal.

**Fix:** give `tcp.c` an explicit discard with a name that cannot be mistaken for a
length:

```c
int tcp_discard(int max);   /* drop up to max bytes from the receive buffer, return count */
```

and call `tcp_discard(tcp_available())` from the `else`. Leave `tcp_recv`'s guard exactly
as it is — it is right, and it is right for the reason its comment gives.

**Then assert it:** `httptest.c` has 91 checks over the real `tcp.c` and none of them
fill the buffer. A test that pushes `HTTP_BUF + 1` bytes and asserts the fetch still
reaches a terminal state would have caught this, and would go red today.

---

## CONFIRMED — settings are written to NVMe and never read back

**Agent claim** (bug class `orphans`): *`settings_load` has no caller: the Settings app
writes to NVMe on every gesture and nothing ever reads it back.*

**Confirmed.**

```
$ grep -rn "settings_load\|settings_save" kernel/ freestanding/ | grep -v 'out.c\|_gen'
kernel/src/graphics/ui/settings.c:125:     * settings_load clamped both ways with the named constants…   ← comment
kernel/src/graphics/ui/settings.c:127:     * earlier, surviving in a second place - so settings_load's…   ← comment
kernel/src/graphics/ui/settings.c:157: int settings_save(void);
kernel/src/graphics/ui/settings.c:185:     settings_save();                                              ← the one real caller
kernel/src/graphics/ui/settings.c:441: int settings_save(void)
kernel/src/graphics/ui/settings.c:483: int settings_load(void)                                           ← definition
kernel/tests/host/settingstest.c:34: int  settings_load(void);                             ← prototype only
```

`settings_save` has exactly one caller, at `settings.c:185`, and its own comment
(`settings.c:169`) says so approvingly. `settings_load` has **none** outside the test
harness.

So zlOS persists settings correctly on every change and never loads them at boot.
**Settings do not survive a reboot**, and the reason is a missing call rather than a
missing feature — `settings_load` is written, careful, and documents its own discipline:

```c
/* kernel/src/graphics/ui/settings.c:478-482 */
/* Read them back. Returns 1 if a good block was found and applied, 0 if the
 * defaults are in force - and in every 0 case it has printed WHY.
 *
 * Never writes. A load that repaired the block would turn a read-only boot into
 * a write, which is exactly what "never write on boot" forbids. */
```

**Fix:** one call in `kernel.zl`'s boot path. This is the cleanest
*exists ≠ reachable* instance the run found — a complete, tested, self-documenting
primitive with a user-visible consequence and no caller. It is the sixth such primitive
in a repo whose `HANDOFF.md:473-483` already lists five.

---

## NOT A DEFECT — "three forcewake domains" is a measurement, not a claim about the code

**Agent claim** (lens `gpu-forcewake`): *"The brief is wrong that three domains are
implemented: only RENDER and BLITTER."*

**The agent is right about the code, and there is no repo error here.** Recording it so
nobody re-files it as one.

```
$ grep -cn "forcewake\|FORCEWAKE" kernel/src/drivers/display/intel.c
0

$ grep -n "FORCEWAKE_.*_GEN9" kernel/src/drivers/display/gpuring.c
127:#define FORCEWAKE_BLITTER_GEN9     0x0A188u
128:#define FORCEWAKE_ACK_BLITTER_GEN9 0x130044u
129:#define FORCEWAKE_RENDER_GEN9      0x0A278u
130:#define FORCEWAKE_ACK_RENDER_GEN9  0x00D84u
```

Two domains, selected per engine at `gpuring.c:151,155`. And `intel.c` contains **no**
forcewake code at all — its only power gating is the display power wells at
`PWR_WELL_CTL`, a different mechanism. So my brief was wrong to look for forcewake in
`intel.c`; the agent corrected it.

**But the "three domains" wording is not a documentation defect.** It comes from a commit
subject:

```
85516b7 feat(gpu): all three forcewake domains confirmed on silicon - RCS unblocked
```

*Confirmed* and *implemented* are different statements. A host-side survey establishing
that three domains ack on this part is a measurement; a driver choosing to implement the
two it needs (RENDER for RCS, BLITTER for BCS) is a design decision. Both can be true at
once, and `.ultra/STATE.md` does not contain the word "forcewake" at all.

No correction is required. The genuine open items in this area are the ones the same
agent raised about *discipline* rather than count — forcewake taken and never released
on the success path (`gpuring.c:325`), and the host survey reading RCS registers while
holding only the BLITTER domain (`hosttest/gpu_ring.c:315`). Those are in the driver
board and have not been hand-verified.

---

## Method

Where an agent and this log disagree, **this log was checked against the tree and the
agent's report was not.** Every entry above carries the command or the line range that
establishes it, so any of it can be refuted the same way it was made.
