# What to do next — one ranked list across all four fronts

**2026-08-19 · worktree `zl-linux-fleet` · branch `fleet/audit-2026-08-19` · base `main` @ `3f00366`**

The four boards rank within their own subsystem. This page ranks across all of them, by
the ordering principle `docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md` already uses:

> leverage = (what the fix unblocks, or the failure it prevents) ÷ (what it costs),
> with a deliberate thumb on the scale for changes that turn a **silent** failure into a
> **loud** one.

**✓ = re-derived by hand.** Unmarked items are agent leads — reproduce first.

---

## 0. The one that changes the plan, not the code

### ✓ The render engine has been unblocked for three commits
[`UNBLOCKED-render-engine.md`](UNBLOCKED-render-engine.md)

`.ultra/STATE.md` and four other documents call `RENDER_SURFACE_STATE` the blocker on
RCS and say it needs a Gen9 PRM this machine does not have. It landed at **`793763a`**,
three commits before `HEAD`, as `kernel/src/drivers/display/assets/gpu_surface.inc` — lifted out of Mesa's decoder
database, which ships zlib-compressed inside libgallium, which is exactly why the
original exhaustive search could not find it.

The render engine is the **only** path to the measured 48× on alpha blend and 96× on
radial glow. The blitter that `.ultra/STATE.md`'s current "Next action" exercises
measures ~1× for fill and 0.85× for copy — the CPU wins.

**Cost: editing five documents.** Highest-leverage item in the run by a wide margin.

---

## 1. Before anything touches hardware

All ✓ verified, all in the ignition path.

| # | fix | why now |
|---|---|---|
| 1 | **`intel.c:435` — `intel_mmio()` returns `u32`** [→](CRITICAL-gpuring-bar-truncation.md) | Every GPU ring register access truncates a 64-bit BAR. `intel.c:423` assembles the >4 GiB address carefully and the accessor discards half of it. `hosttest/gputest.c:33` stubs it to `0`, so no test can see it. **One line, plus three declarations.** |
| 2 | **`gpuring.c:493` — GGTT overlap** [→](VERIFIED-ggtt-span-collision.md) | Cursor and self-test share 3 of 4 graphics pages. Physical spans are asserted; graphics spans are not asserted anywhere. **One constant, plus the assert that catches it.** |
| 3 | **`intel.c:2555` — link training returns 1 on a black panel** [→](CRITICAL-display-failure-reporting-inverted.md) | The one unchecked `intel_dpcd_write` in a function where every other one is checked. On a machine whose only diagnostic is the screen. **One `if`.** |
| 4 | **`intel.c:3882` — `MS_STEP_SOFT` fails the whole modeset** [→](CRITICAL-display-failure-reporting-inverted.md) | A backlight step failing makes `panel_up` report failure on a working panel. **One parameter to `ms_do`.** |

Items 3 and 4 are about being able to *believe* the next hardware run. Do them before
flashing anything.

---

## 2. One-line fixes with visible consequences

| # | fix | consequence today |
|---|---|---|
| 5 | ✓ **`wm.c:1450` — `code == KEY_TAB`** [→](VERIFICATION-LOG.md) | Alt+Tab has never fired. **One token.** Second instance of the class `STATE-OF-THE-PROJECT.md` ranks 3rd project-wide |
| 6 | ✓ **`kernel.zl:2994` — check the button mask** [→](CRITICAL-browser-cluster.md) | Hovering a link fires a full fetch. The same file does this correctly twice elsewhere |
| 7 | ✓ **`kernel.zl:2993` — delete it** [→](CRITICAL-browser-urlbar-keys.md) | The URL bar double-types every character, and Enter/Esc/Backspace are dead on the ThinkPad's own keyboard. `APP_SNAKE` and `APP_MENU` were checked and are safe |
| 8 | ✓ **call `settings_load` at boot** [→](VERIFICATION-LOG.md) | Settings are written to NVMe on every change and never read back. **One call** |
| 9 | ✓ **`anim_tick` — sample `idt_ticks()`** [→](LOOK-BOARD.md) | Every animation duration is a frame count, so speed tracks host load. The clock is already declared in the same file and used by two other subsystems |

Item 9 must land **before** SMP bands, or a 1.78× faster redraw makes every animation
1.78× faster rather than smoother.

---

## 3. Make the gates able to fail

`docs/GUARDS-THAT-DID-NOT-GUARD.md` lists five checks that report green while checking
nothing. The fleet found **seven more of the same kinds** — the document is right and
incomplete. [→](CRITICAL-gates-that-cannot-fail.md)

| # | fix |
|---|---|
| 10 | ✓ `land-gate.sh:137` — count and print skips. `verify-efi.sh` currently vanishes silently if it loses its executable bit |
| 11 | ✓ `run_tests.sh:192` — add the `else` that says the kernel gate was skipped |
| 12 | ✓ `hazard-scan.sh:86` — expand the file list instead of scraping it. Scans 6 of ~50 TUs [→](CRITICAL-ci-truncation-gate-is-blind.md) |
| 13 | ✓ `fbbench.c` — put `fb_fill_blend` and `fb_rrect_blend` in the hashed scene. The two most performance-critical primitives in `fb.c` have no correctness coverage, under a comment claiming every risky primitive appears |
| 14 | ✓ `wguard.sh:45` — make it parse `CF=` out of `buildefi.sh` (`hazard-scan.sh:31-34` already has `efi_cflags()` for exactly this) and assert per-flag instead of `-ge 4`. Today it never reads the flag line it exists to guard, and it is not in CI [→](CRITICAL-gates-that-cannot-fail.md) |
| 15 | — add `-Wconversion` or a narrowing check on `uptr`. Neither the gate nor the four `-Werror=` flags can see the `intel.c:435` shape |

Every one of these must be **watched going red** before it is trusted — the repo's own
rule from `.ultra/TENSIONS.md` T-2.

---

## 4. Memory safety and permanent hangs

| # | fix |
|---|---|
| 15 | ✓ **`http.c:89` — `build_request` writes 9 bytes past `req[512]`** [→](CRITICAL-http-request-overflow.md). Add the reserve **and** a `_Static_assert` tied to the tail literal |
| 16 | ✓ **`http.c:229`** — a full response buffer deadlocks the fetch permanently. `tcp_recv(resp, 0)` was meant to drain-and-discard, but `tcp_recv`'s own **correct** `if (max <= 0) return 0` trust-boundary guard makes it a no-op. Neither function is wrong. Needs a named `tcp_discard()` [→](VERIFICATION-LOG.md) |
| 17 | ✓ **`HTTP_REDIRECT`** — `http.c:267` sets it; `browser_tick` (`browser.c:422-432`) handles only `HTTP_DONE`, `HTTP_REFUSED`, `HTTP_ERROR` then `return 0`. `fetching` stays 1 forever. The state exists for the *successful* redirect case — `http.h:21` says "3xx with a Location, **under the redirect limit**" — so `http.c` detects and validates the redirect, then hands the caller a state it does not know [→](VERIFICATION-LOG.md) |
| 18 | ✓ **`xhci_ram_ok()`** zeroes the live DCBAA scratchpad pointer; the zl builtin `usb_ram` reaches it at any time [→](CRITICAL-gates-that-cannot-fail.md) |
| 19 | **`xhci.c:989`** — `reset_endpoint` points any endpoint at EP0's ring and wipes EP0's producer state, reachable with a bulk DCI from `:2127` |

Items 16 and 17 together mean the two most common non-trivial HTTP outcomes — a redirect
and a largish page — both end in a permanent silent hang.

---

## 5. Before turning SMP bands on

### ✓ The APs run with no IDT [→](CRITICAL-smp-bands-have-no-idt.md)

`.ultra/STATE.md` §4 ranks this change "the best win-to-risk on the board" and
`gpu-next.md` calls its risk "real but bounded." The 1.78× is real. The risk is that any
fault inside a band job triple-faults the laptop — instant reset, no output, on the
machine with no serial port. The barrier at `smp.c:177` is also unbounded.

| # | fix |
|---|---|
| 20 | bound the barrier — the `spins` idiom already exists five lines below at `smp.c:184` |
| 21 | give the APs an IDT, even a `kfatal`-only one. Converts a silent reboot into a message on the only diagnostic there is |
| 22 | **then** turn bands on, and measure |

Also ✓: `smp_go()` is **not** "reachable only from the old text shell" — `term.c:190`
and `:204` map the typed words `smp` and `cores`, and `probe-smp.py:143` types `"smp\n"`
into the compositor. Six documents say otherwise.

---

## 6. Documentation that routes work wrongly

Ranked here because a wrong doc costs more than a wrong line — nobody tries what they are
told is absent.

| # | correct |
|---|---|
| 23 | ✓ `.ultra/STATE.md` §Open 2 and its `Blocked:` line — `RENDER_SURFACE_STATE` |
| 24 | ✓ five sites in three files claiming nothing arms `lt_armed` [→](PANEL-POWER-IS-REACHABLE.md). `HANDOFF.md:10` already carries the correction while `:209`, `:313` and `:365` still assert the falsehood |
| 25 | ✓ `kernel/docs/driver-build-order.md` needs the audit banner the other twenty-one planning docs carry. **Its Wave 0 is fully landed** [→](GROUND-TRUTH-CORRECTIONS.md) |
| 26 | ✓ `STATE-OF-THE-PROJECT.md` §2.1 — `hosttest/palette.c` **is** covered now |
| 27 | `browser-status.md` — the "every drop path is counted" claim is half true, and the `tcp.c` line count is 752 against an actual 900 |

---

## What the run found that is *good*

Recorded deliberately, because the fastest way to damage this codebase is to refactor a
part that is already right.

- **The browser's parsers are defensible.** All 33 fixed buffers enumerated; no unclamped
  network-derived write. `html.c` has no tag-name buffer to overflow. All 34 loops in
  `css.c` provably advance. `dns.c` is structurally immune to the compression-pointer
  loop — `skip_name` never follows a pointer at all.
- **The subpixel font atlas carries FreeType's real LCD filter**, proved from the bytes.
- **The theme layer is real** — 12 roles, and `wm.c` contains zero colour literals.
- **The SSE paths are correct**, tails and prologues included.
- **`memmap.h` and its span asserts** are the right shape, and caught a collision
  (`HI_APSTK`) that the docs never found. GGTT space needs the same treatment.
- **`net.c:58-64`'s drop-counter doctrine** is correct and already followed by eight files.

---

## The honest caveats

1. **No gate was run.** No agent executed QEMU, a build, or a harness — `CLAUDE.md`
   records this box OOM-killing an agent process at 7.9 GB doing exactly that, and
   another session held a live QEMU throughout. Every finding here is a **static read**.
   The `build_request` overflow, the `KEY_TAB` mismatch and the URL-bar double-type are
   all cheap to confirm at runtime, and should be.
2. **The unmarked items are leads.** Roughly 640 findings came back; a few dozen were
   hand-verified. At least two agent findings had correct conclusions resting on false
   evidence, and two of *my own* documents were broken by the refutation stage and
   corrected.
3. **The weakest link, unprompted:** the ranking above assumes the four boards' severity
   labels are comparable. They were assigned by different agents against different
   subsystems, and a "critical" in the look wave is not the same animal as a "critical"
   in the driver wave. The ordering within each numbered section is defensible; the
   ordering *between* sections 2, 3 and 4 is a judgement call, not a measurement.
