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
> primitives.* — `kernel/hosttest/fbbench.c:360`

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

## CONFIRMED — the HTTP body pointer is narrowed to 32 bits, and C code widens it back

**Agent claim** (bug class `llp64`, severity `high`): *`http_body_addr()` narrows
`&resp[body_at]` to `u32` and two reachable callers widen it back and dereference it.*

**Confirmed.**

```c
/* kernel/http.c:278 */
u32 http_body_addr(void)  { return (u32)(uptr)(resp + body_at); }
```

The narrowing is **deliberate and documented** — `http.c:31` says it *"narrows to u32 ON
PURPOSE (zl reads the body through a …)"*, because zl numbers are doubles and the
builtin at `runtime_kernel.c:1278` has to return one. For the zl path that is a
considered trade.

**The defect is the C caller:**

```c
/* kernel/browser.c:426 */
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
both keyboards emit `KEY_TAB` (0x103).* — `kernel/wm.c:1450`

**Confirmed, and the repo documents the rule that is being broken.**

```c
/* kernel/wm.c:1448-1452 */
static void route_key(int type, int code, int mods)
{
    if (type == EV_KEY_DOWN && code == '\t' && (mods & MOD_ALT)) {
        cycle_focus();
        return;
    }
```

`'\t'` is 9. But `kernel/keycodes.h:10-21` is explicit that key codes live above 0x100
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

**Why it is worth flagging beyond itself:** `docs/STATE-OF-THE-PROJECT.md` ranks
*"Fix `key()` at `kernel.zl:1407` — one token"* third on the whole-project board. This
is a **second instance of the same class** — a character literal used where a key code
is required — in a different file. Worth a sweep for the rest: any comparison of an
event `code` against a character literal for a key that has a `KEY_*` constant.

---

## CONFIRMED — `intel.c` has no `static` functions at all

```
$ grep -cP '^static\s+[\w \*]*\bintel_\w+\s*\(' kernel/intel.c
0
$ grep -cP '^(?!static)[A-Za-z_][\w \*]*\bintel_\w+\s*\(' kernel/intel.c
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

**False.** `kernel/wm.c:1281` calls it. The agent's *conclusion* — that drag-to-edge
snapping is a one-way door — is nonetheless **correct**, via the `z != SNAP_NONE` guard
at `wm.c:1358`.

Full chain, the duplicate `SNAP_NONE` definitions, and the fix:
[`VERIFIED-WM-SNAP.md`](VERIFIED-WM-SNAP.md).

This is the reference case for why the boards are not authoritative: right conclusion,
false evidence, and the false evidence is what a reader would have quoted.

---

## Method

Where an agent and this log disagree, **this log was checked against the tree and the
agent's report was not.** Every entry above carries the command or the line range that
establishes it, so any of it can be refuted the same way it was made.
