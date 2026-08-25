> **AUDITED 2026-08-19 · MOSTLY DONE.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. The track ran to completion and merged last, as the most expensive landing (47 hunks). Nine of ten items are verifiably done, but the merge chose `overnight-compositor` as the `kernel.zl` trunk, so several landed in a different SHAPE than specified. Its headline "4× SMP" is contradicted by a measured 1.64×, and the six probes it commissioned are invoked by nothing.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**

# The platform track — apps, speed, and the things that bite

Companion to `evidence/desktop-v10-plan.md`, which is the **look**. This is everything
else, and the two are deliberately separable so they can run at the same time
in two sessions without touching each other.

---

## How to start it

Open a session in `~/Documents/repos/zl-apps` and paste only this:

```
Read kernel/docs/archive/prompts/PLATFORM-PROMPT.md in full and execute it exactly as written.
You are in a git WORKTREE on branch desktop/apps-in-windows - check with
`git branch --show-current` before your first commit. Another session is
working on the LOOK in a different worktree; the file ownership split in §2 is
what keeps you both out of each other's way. Work the queue in order, gate
every item, commit on green. Use agents, skills and workflows freely - the
brief authorises them and §1 says how to do it without OOM-killing the box.
Do not stop until I tell you to.
```

A short bootstrap pointing at a file beats pasting 400 lines: the file survives
a context reset and can be re-read to recover.

---

## 0. Read first, in this order

```
kernel/HANDOFF.md                    what is VERIFIED, not intended
CLAUDE.md                            the traps, all paid for at least once
kernel/docs/evidence/desktop-overnight-run.md what landed 2026-08-17/18 and what it cost
kernel/docs/archive/superseded/desktop-wiring.md  how zl and C meet, and why it works
kernel/docs/archive/superseded/desktop-TODO.md  the numbered task list
kernel/ui.h                          the layering contract - read this twice
```

**Everything is already designed.** Do not re-plan. Build it.

---

## 1. HARD RULES — these override everything below

### 1.1 You are in a worktree. Stay in it.

One `.git` shared by two sessions cost a **corrupted object store** and three
commits on the wrong branch, both on 2026-08-18 (T-10, T-12). A worktree gives
you your own index and refs while sharing the object store. **Never `git
checkout` a different branch here, and never touch the other worktree.**

`git status` before every commit. Stage files **by name**. Never `git add -A`.

### 1.2 Resources: 4 cores, 15 GB, OOM-killed twice

- `cut -d' ' -f1-3 /proc/loadavg` before anything heavy. Above ~4, wait.
- **ONE QEMU at a time.** Never QEMU plus a multi-agent fan-out.
- Agents are authorised and encouraged — but for **reading and analysis**, which
  is cheap, not for parallel builds. A fan-out of file-readers is fine; three
  agents each booting QEMU is how the box died.
- Run gates in the background (`run_in_background: true`) and collect them.

### 1.3 Four build scripts, four source lists

`build.sh` (32-bit), `build64.sh`, `buildefi.sh` (UEFI/clang/LLP64), `mkdisk.sh`
(raw). **`verify.sh` sees only the first.** Adding one `.c` broke the UEFI and
raw builds **twice** on 2026-08-18. Either add to all four, or ride along inside
an existing translation unit the way `font_prop.inc` does in `fb.c`.

Item 4 in the queue fixes this properly. Until then, assume it will bite you.

### 1.4 Gates

`verify.sh` is 32-bit BIOS **only**. Also run:

| | covers |
|---|---|
| `verify-efi.sh` | the 64-bit UEFI application — the laptop's real path, and the only gate where `fb.c`'s SIMD is live |
| `verify-raw.sh` | our own bootloader |
| `hosttest/{wmtest,inputtest,tritest,fbbench}` | no QEMU, seconds, run them constantly |

**A gate must never wait a fixed wall-clock time.** Poll for the expected
output. A timing-sensitive gate already cost this project a false regression.

**Never build on a red gate.** Log it to `.ultra/TENSIONS.md`, revert if needed,
move to the next independent item.

### 1.5 Do not touch the display driver

`intel.c` write paths can **damage hardware** — the panel's 500 ms T12 delay is
real. `kernel/docs/archive/superseded/display-roadmap.md` is another session's. Not yours.

---

## 2. FILE OWNERSHIP — the split that lets two sessions run

| yours (this track) | theirs (the look track) |
|---|---|
| `kernel/term.c` | `kernel/ui.c` |
| `kernel/wm.c` — *behaviour* only | `kernel/ui.h` — *theme struct* only |
| `kernel/smp.c`, `kernel/fb3d.c` | `kernel/gen_prop_font.py`, `gen_icons.py` |
| `kernel/input.c` | `kernel/font_prop.*` |
| the four build scripts | — |
| `kernel/hosttest/*` | `kernel/hosttest/wmshot.c` |
| `kernel/probe-*.py` | — |

**`fb.c` and `kernel.zl` are shared.** Both tracks need them. Rules:

- **`fb.c`** — you own the *memory map*, `fb_present`, the blit and SIMD. They
  own blending, gradients and effects. Announce in the commit message which
  half you touched.
- **`kernel.zl`** — you own the **bottom** (the boot sequence, `app_event`,
  `run_command`, the demos). They own the **middle** (`draw_*`, colours,
  layout). Keep edits surgical and commit often, so a conflict is small.

If you must touch one of theirs, do it in one commit that changes nothing else.

---

## 3. THE WORK QUEUE — in order, each with a gate

### Item 1 — prove the terminal actually works

`term.c` is wired and **nobody has typed a command into it**. Everything below
assumes it works.

`probe-shot.py -k <keys>` sends characters over serial, so this is a gate, not
a hand test. Boot, press `w`, then type:

```
help        -> the app list appears in the scrollback
uptime      -> a seconds figure
fib 20      -> 6765, proving the ARGUMENT parser works
nonsense    -> "unknown command: nonsense"   <- the one that matters most
clear       -> the scrollback empties
```

**The unknown-command path is the important one.** A shell that silently
ignores what you typed is worse than one with no commands.

**Known cosmetic bug to fix here:** two `zl>` prompts appear — one captured
into the scrollback from the old shell before the compositor starts, one live.
It disappears when Item 2 lands, but confirm that rather than assuming it.

**Gate:** a new `probe-term.py` that types those five and asserts each result
appears in the serial log.

### Item 2 — the compositor becomes the boot state

At the bottom of `kernel.zl`:

```
if wm_available() == 0 {
    while running == 1 { ...the existing shell loop, UNTOUCHED... }
} else {
    wm_session()
}
```

`verify.sh` boots `-kernel -display none`, so `px_w() == 0`, `wm_available()`
is 0, and the plain text shell runs exactly as today.

**Gate:** `verify.sh` transcript **byte-identical**. This is the single most
likely thing in the whole queue to go red. Also `verify-efi.sh` — that path DOES
have a framebuffer, so it will take the compositor branch.

### Item 3 — C4: delete the sticker-drag machinery

Once `wm_frame` owns the screen, nothing calls `bg_snap`, `bg_rest`, `grab` or
`stamp`. Delete the builtins in `runtime_kernel.c`, then `fb_bg_snapshot`,
`fb_bg_restore`, `fb_grab`, `fb_stamp`, `bg_buf`, `sp_buf` from `fb.c`.

**Keep `fb_pointer_show`/`fb_pointer_hide`** — the 11×17 cursor save-under is a
different and correct technique.

This takes with it:
- the **640×480 drag ceiling** (the terminal is 1256×944 — nearly 4× over, the
  real reason it could never be dragged)
- the **12 px shadow smear** (`fb_shadow` reaches `x + w + 28` at `u = 2`; the
  drag erased only `w + 16`)
- **128–176 MiB** of fixed high RAM

**Then move the back buffer** into the freed span so 4K stops borrowing the drag
arena. `fb_setup` already computes its base; only the map comment and the
`BIG_LIMIT` arithmetic change.

**Gate:** all four boots; `fbbench` shows 3840×2160 with `back ON, drag ON`.

### Item 4 — one source list, not four

The hazard in §1.3, fixed. Options, pick one and say why:

- a shared `SOURCES` file all four scripts read
- or a single `sources.mk`
- or fold the rare ones into `build.sh` with a target flag

**Gate:** add a throwaway `.c`, confirm **all four** builds pick it up, remove
it. Prove the fix rather than asserting it.

### Item 5 — snake stops owning the screen

**Convert `snake_game` first.** Its state is already in raw memory (`SNAKE_X`,
`SNAKE_Y`), so there is least to hoist — the brief calls it out as the right
first conversion.

```
app_draw(APP_SNAKE, x, y, w, h, focus)   draw the board from the raw arrays
app_event(APP_SNAKE, ...)                arrow keys turn it
app_tick(APP_SNAKE, win)                 move one step, return 1 if it moved
```

**Delete its `while` loop.** That loop is the whole reason "press any key to
exit" exists.

**Gate:** snake keeps playing **while another window is dragged over it**. That
is the assertion — not "snake runs".

### Item 6 — then paint, cube_demo, anim, mousedemo

Same pattern. Leave the editor (most state to hoist) and delete `windows_demo`
(superseded — `w` already points at `wm_session`).

**Gate:** every one of them runs in a window, and **five `while` loops are
gone**. Grep for `press any key` afterwards and expect nothing.

### Item 7 — G2: SMP band rendering. The 4× nobody has taken.

Three of four cores sit parked in `cli; hlt` (`smp.c:79`). The back buffer
splits into disjoint horizontal bands with **no lock needed** — disjoint writes
to plain RAM.

This was excluded from the overnight run because concurrency bugs unsupervised
are the wrong risk. **Roy is awake; that objection is gone.** It is the largest
remaining speed lever in the project.

Do it carefully:
- bands must be **disjoint by construction**, not by convention — compute them
  once, assert they tile the buffer exactly with no overlap and no gap
- the cores need a barrier at frame end. No allocator, so a fixed array of
  volatile flags, one cache line apart
- `fb_present` must not start until every band reports done
- **the FNV scene hash must be unchanged.** Same pixels, or it is not the same
  renderer

**Gate:** `fbbench` whole-desktop redraw, best of 6, with the hash unchanged.
**Report the real number.** If it is not ~4×, say what it actually is and why.
DECISIONS.md #25 is an optimisation argued from an instruction count that
measured 25% slower — do not repeat it in either direction.

### Item 8 — T-11: the EFI address truncation

`buildefi.sh` targets `x86_64-unknown-windows`, which is **LLP64: `unsigned
long` is 4 bytes**. The GOP framebuffer address is truncated at
`efi.c:250`:

```
efi.c:250    fb_addr = (unsigned long)gop->mode->framebuffer_base;   <- UINT64 loses its top half
efi.c:287    console_init_efi(fb_addr, ...)
console.c    console_init_efi -> fb_setup(addr, ...)
fb.c         fb_base = (unsigned char *)addr
```

The four `-Werror` flags in `buildefi.sh` **do not catch it** — verified by
compiling both cast shapes with the exact build line. They catch pointer↔`int`;
this is a UINT64 narrowed by an *explicit* cast.

**The fix is three declarations, all to `unsigned long long`:** `efi.c`'s
`fb_addr`, `console_init_efi` in both `efi.c` and `console.c`, and `fb_setup`'s
parameter. **Changing only one is worse than changing none** — the declaration
would then disagree with the definition about the size of a register argument.

Latent, not active: a GOP base is normally a PCI BAR below 4 GiB, which is
exactly why QEMU never shows it — the same reason the two `lidt`/`lgdt`
truncations in CLAUDE.md stayed invisible.

**Gate:** `verify-efi.sh` green, plus a `_Static_assert` that the address type
is at least pointer-sized, so it cannot regress silently.

### Item 9 — B5: `tsc()`, and frame time on screen

`cpu_tsc()` and `cpu_tsc_khz()` exist (`cpu.c:212`, `cpu.c:220`) and are **not
exposed to zl**. `idt_ticks()` is 100 Hz — far too coarse to measure a 16 ms
frame.

Add the builtin, then put frame time in the System Monitor window.

**Gate:** a plausible frame time visible in a boot screenshot. "Plausible"
means it agrees with `fbbench`'s figure for the same scene, not merely that a
number appears.

### Item 10 — the editor, last

Most state to hoist, highest chance of a mess. Do it only when 5 and 6 are
green and committed.

---

## 4. WHEN THE QUEUE IS DONE — the anti-idle backlog

In order, forever. This is what "keep going" means.

1. Re-run **every** gate from scratch, all four boots plus every harness.
2. `fbbench` again; replace the table in `kernel/docs/desktop/desktop-polish-and-speed.md` with the
   new numbers. Never a number you did not measure.
3. Write a test for anything you changed that has none.
4. **Re-read `fb.c`, `wm.c`, `ui.c`, `term.c` for this project's known bug
   classes** — a fixed address that could collide with a neighbour; a `u32`
   holding a 64-bit address; an off-by-one in a clip or damage rect; **a silent
   fallback that should print a line**. Turn this on the code *you* wrote, not
   only the old code. It found four real bugs on 2026-08-18, three of them in
   code written that same night.
5. Extend the toolkit where behaviour is missing, not styling (that is theirs).
6. Improve any doc that was wrong or missing when you tried to follow it.
7. Go back to 1.

---

## 5. Using agents and workflows

**Authorised, and encouraged.** Roy asked for it explicitly. Within §1.2:

**Good uses**
- fan out **readers** over `kernel.zl` to inventory every `while` loop and what
  state each demo holds, before converting any of them
- an adversarial reviewer on the SMP band code — concurrency is exactly where a
  second opinion earns its cost
- a doc-writing agent to keep `archive/superseded/desktop-TODO.md` in step while you build

**Bad uses**
- anything that boots QEMU in parallel with your own gate
- delegating a change you have not scoped — you own the result either way

**If you delegate, you own collection.** Never end a turn with "waiting for
agents". Wait, integrate, then answer.

---

## 6. Standing hazards, each paid for once already

- **The zl parser takes no multi-line call arguments.** One line per call.
- **The zl kernel subset has no runtime strings.** Literals exist; comparing two
  runtime strings does not. That is why `term.c` holds the command table, in C.
- **The zl kernel subset has no lists.** `zl_list_n` is a hard fault. Data lives
  in raw memory with an index — snake already does this.
- **`draw_sysmon`-style bodies must be position-pure** — every coordinate from
  the `x,y,w,h` passed in. The app contract depends on it.
- **Damage is frame PLUS shadow**, and the shadow size varies with elevation.
  `shadow_reach()` in `wm.c` exists for this; use it.
- **Local timestamps are unreliable** — the RTC runs slow and NTP corrects
  mid-session. For "when did X happen", use `gh api repos/RoyX4/zl-linux --jq
  .pushed_at`.

---

## 7. How you know you are done with an item

Not "it looks right". One of:

- a **gate command and its output**, pasted
- a **screenshot** you actually looked at, for anything visual
- a **number you measured**, for anything about speed
- an **assertion added to a harness**, for anything invisible

An instruction count is not a measurement. "It works" is not a report.

---

Look track: [`desktop-v10-plan.md`](../../evidence/desktop-v10-plan.md) · Contract:
[`../ui.h`](../../../src/graphics/ui/ui.h) · Wiring: [`desktop-wiring.md`](../superseded/desktop-wiring.md) ·
History: [`desktop-overnight-run.md`](../../evidence/desktop-overnight-run.md) · Blocks:
`../../.ultra/TENSIONS.md`
