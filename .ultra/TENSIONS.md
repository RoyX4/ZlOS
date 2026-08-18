# TENSIONS — what's broken, uncertain, or wrong

<!-- P-02: honest self-critique. Every failed quality gate lands here rather than
     being quietly passed. Never delete a tension to make the board look clean —
     resolve it and mark it RESOLVED with the date.

Format:

### T-1 | YYYY-MM-DD | open
**Tension:** what's broken, uncertain, or in conflict
**Gate:** which quality gate this failed, if any (1-5)
**Cost:** what it's blocking or degrading right now
**Resolution:** what would settle it — or RESOLVED YYYY-MM-DD + how
-->

### T-1 | 2026-08-17 | open
**Tension:** At the ThinkPad's native 2560×1440 the framebuffer back buffer
switches itself off, and four features die silently with it. Chain, each link
read from source: `efi.c:231` takes the mode firmware is already in (2560×1440);
`kernel.zl:1830` only re-modesets `if px_w() < 1900`, so it is skipped;
`fb.c:155` computes `back_on = (2560*1440 <= 1920*1200)` = **0**. Lost: the back
buffer, subpixel AA (`fb.c:290` guards on `back_on`), fast `fb_get_px` (falls to
the 30–50× VRAM read path), and window dragging entirely (`bg_ok = 0`).
No log line says any of this happened.
**Gate:** none failed — this is pre-existing, found during planning.
**Cost:** Blocks the entire desktop plan on the target hardware. A working
touchpad would still be pointing at a slideshow: every shadow, rounded corner and
AA glyph becomes a per-pixel read-modify-write against uncached write-combining
VRAM.
**Caveat — this is verified by reading, not observed.** The ThinkPad has never
booted zlOS. The arithmetic is deterministic; the premise (that firmware hands
over 2560×1440) is inferred from `efi.c` plus HANDOFF's panel spec.
**Resolution:** Force a 2560×1440 BGA mode in QEMU and confirm the degradation is
real. Then size the back buffer from the actual mode, and make the fallback
**loud** in the boot log rather than silent. Check the new size against the
224 MiB DMA arena — at 4K the back buffer would end at 223.6 MiB, 368 KiB of
margin, which is the repo's own recurring bug class.

### T-2 | 2026-08-17 | open
**Tension:** `i2c_hid.c` is a transport with no decoder.
`i2c_hid_read_report()` fills a buffer and `i2c_hid_byte(i)` returns **raw
undecoded bytes**. There is no HID report descriptor parser and no `(x, y,
buttons)` anywhere in the tree. A precision touchpad reports absolute
coordinates plus contact count plus tip-switch — it is not a mouse.
**Gate:** none.
**Cost:** "The touchpad works" is three jobs, not one: I2C transport on real
hardware, a report decoder, and a pointer policy (absolute→screen scaling,
tap-to-click, two-finger scroll). Phase 1 of the desktop plan is sized wrong
until this is acknowledged.
**Resolution:** Scope Phase 1 as three items. The decoder can be written and
unit-tested against a captured report dump before the hardware works.

### T-3 | 2026-08-17 | open
**Tension:** `intel.c`'s write paths are gated behind `lt_armed` and most have
**never executed**. A prior audit found 86 verified defects, including a
panel-damage class and a wrong DPLL override bit.
**Gate:** none.
**Cost:** "The code exists" is not "the code works" anywhere in `intel.c`.
Any claim about it must be checked for an actual caller, and for whether
anything arms `lt_armed`, before being believed.
**Resolution:** Exercise the write paths from `kernel/hosttest/` against the live
GPU — seconds per iteration, no reboots. Blocked on a free console (see
STATE.md § Blocked).

### T-4 | 2026-08-17 | open
**Tension:** Two objectives, and ULTRA CODE can only track one.
`kernel/HANDOFF.md` says the point of the project is the Intel display driver.
The work actually commissioned on 2026-08-17 was the desktop architecture. These
imply different primary metrics and different "highest-leverage next move"s.
**Gate:** 2 — METRICS.json has no primary metric, so no cycle can be judged.
**Cost:** Blocks every ULTRA CODE cycle. Gate 2 cannot pass until it is settled.
**Resolution:** One answer from Roy. Asked 2026-08-17.

---

## Overnight run — 2026-08-17/18, branch `desktop/overnight-compositor`

Assumptions taken without asking, per the OVERNIGHT-PROMPT brief. Each is one
line beginning ASSUMED:, recorded so a later session can overturn it cheaply.

ASSUMED: no multi-agent fan-out for this run. The brief's HARD SAFETY RULE 5
  and CLAUDE.md both say this box (8 logical cores, 15 GB, OOM-killed twice)
  must not pair QEMU with a fan-out, and this run is a continuous stream of
  QEMU gates. Working solo, sequentially, gates in the background.
ASSUMED: `kernel/fb.c` is MINE to edit and stage. It is uncommitted at the
  start of the run but is NOT on the brief's mid-flight list (rule 4), and its
  only uncommitted content is the shadow-skip optimisation and `fb_phys()` that
  DECISIONS.md #12 and desktop-TODO 0i already describe as shipped desktop work.
ASSUMED: `kernel/kernel.zl` IS on the mid-flight list, so anything I add there
  will be committed together with another session's in-flight edits. I cannot
  separate them by file. Mitigation: I do not touch kernel.zl until Groups A
  and B (pure C, pure fb.c) are landed, and when I do I record the fact in the
  commit message. Overturn by asking Roy to commit or stash their kernel.zl.
ASSUMED: "regenerate the atlas at 48x48" (A1) means emit a SECOND, larger atlas
  rather than replacing the 24x24 one, so the 1x path on sub-1400px screens
  keeps a properly filtered source instead of a runtime downscale.

### T-5 | 2026-08-18 | RESOLVED 2026-08-18 — and my first conclusion was WRONG

**What I claimed:** that `-device usb-tablet` *steals* the pointer, that zlOS
has no tablet driver, and that `try.sh` should drop the line. **Two of those
three are false.** The measurements were right; the conclusion was not.

**What is actually true.** Another session was, at that moment, writing a USB
HID pointer driver — `xhci_ptr_ready/abs/x/y/btn/poll` in `xhci.c` — and
rewiring the `mouse_x`/`mouse_y`/`mouse_btn` builtins to *prefer* it, falling
back to PS/2. So the machine has two pointers and reads whichever is present:

| machine | events sent | IRQ12 | pointer ends at | |
|---|---|---|---|---|
| PS/2 + tablet | 21 **relative** | 1 byte | 960,600 → never moves | the tablet is what zlOS reads, and it ignores relative events |
| PS/2 + tablet | **absolute**, to 0.75/0.50 of the screen | **0** | **1439,599** | exact: 0.75×1920 = 1440, 0.50×1200 = 600 |
| PS/2 only | 21 relative | clean ×3 | 400,300 → 510,400 | exact |

**So both paths work, and my probes were sending the wrong event type for the
machine they had built.** `probe-drag.py` reporting "dragging is a no-op" was
my harness's fault, not the kernel's — and "removing the tablet made it pass"
is precisely the kind of confirmation that feels like a diagnosis and is not
one. The honest reading only appeared after sending absolute events *with* the
tablet still attached, which is the experiment that could have refuted me.

**What stays true:** zlOS's PS/2 decode is correct (exact tracking without the
tablet), and the stray IRQ12 byte before any input — the 0xFA ACK from
`mouse_cmd(0xF4)` — is real and harmless; the resync heuristic absorbs it. An
even earlier reading of mine blamed a *framing* bug for the dead pointer. Also
wrong, for the same reason: a plausible mechanism is not a measurement.

**Resolution:** `try.sh` needs no change — retracted. The pointer probes now
default to the tablet configuration `try.sh` actually gives and send absolute
events to match; `--no-tablet` exercises the PS/2 fallback (the laptop's
TrackPoint is PS/2, so both matter). `probe-mouse.py` additionally asserts the
pointer lands *where it was sent* rather than merely somewhere, which is what
let it previously pass while the pointer was pinned at 0,0.

### T-6 | 2026-08-18 | RESOLVED 2026-08-18
**Tension:** `desktop-TODO` 0b and `DECISIONS.md` #6 both say the clip rectangle
is **exactly two functions** — `fb_fill_px` and `put_pixel` — and that
"everything else is built on those two". Three functions write the back buffer
directly and never call either: `draw_glyph`'s subpixel and AA fast paths,
`fb_gradient`'s `back_on` branch, and `fb_scroll`.
**Gate:** the new `clip_check()` in `hosttest/fbbench.c`. With only the two
functions changed, **2,184,000 pixels escaped the scissor at 1920x1200** and
3,566,400 at 2560x1440 — the wallpaper gradient and the text, i.e. most of a
desktop. It passed at 3840x2160 *only* because `back_on` is 0 there, so those
paths fall back to `put_pixel`, which was clipped. That is a good demonstration
of why "it looked right" is not a gate.
**Resolution:** RESOLVED — all three now fold the scissor into their loop
bounds rather than testing per pixel, so clipped drawing stays fast. Zero
pixels escape at all three resolutions, and all three scene hashes are
unchanged with the scissor at full screen. `DECISIONS.md` #6 and
`desktop-TODO` 0b corrected in place.

### T-7 | 2026-08-18 | open
**Tension:** `verify-efi.sh` failed once and passed twice on the *same tree*,
minutes apart. The failure said "it started but never reached the prompt", last
line `GDT loaded - 64-bit flat segments, 4-level paging, SSE on` — which is the
exact signature the script's own header describes as the known 64-bit
`setup_idt()` death, so it reads as a real regression.

It is not one. A hand-run of the same QEMU command line reached `ready.` and
`zl>` cleanly, and two subsequent `./verify-efi.sh` runs were green.
**Gate:** 4 (the new UEFI gate), red once out of three on unchanged code.
**Cost:** this is the gate that exists *because* three others were green while
the 64-bit build was dead. A gate that cries wolf on that path is worse than
none — the next person to see it will assume flakiness and be wrong the day it
is real.
**Suspected cause, not confirmed:** `mkusb.sh` rebuilds from `efi.c`,
`_genefi.c` and `buildefi.sh`, all three of which another session was editing
at the time. A build that catches a half-written file produces exactly this.
Not a timing bug — the poll ceiling is 180 s and the boot takes seconds.
**Resolution:** have `verify-efi.sh` fail loudly and separately when the BUILD
is the thing that broke, rather than reporting a build artefact as a boot
failure. Owned by whoever owns `efi.c`; noted here so it is not mistaken for
desktop work.

Note for the record: this run's `fb.c` changes are in the 64-bit and UEFI
builds too, and the boot log shows `fb: 1280x800x32 ... back ON, drag ON` then
`fb: 1920x1200x32 ...` on that path — so `fb_setup`'s new report works there,
which is the thing that most plausibly *could* have been mine.

### T-8 | 2026-08-18 | open
**Tension:** Group C is built and tested but **not wired in**, and the wiring is
one file this run may not commit.

`wm.c`, `ui.c` and `ui.h` are complete, compile freestanding, link into the
kernel, and pass 31 assertions in `hosttest/wmtest.c`. Nothing calls them.
`kernel.zl` still ends in `while running == 1 { prompt, read a key, run_command }`.

The inversion the whole project is for — the compositor becomes the top of the
system and the shell becomes app 0 — is a `kernel.zl` change, and `kernel.zl`
is on the brief's mid-flight list (another session is editing it live; its
`mouse_x` builtin was rewired to the new USB pointer *during* this run).
Staging it would commit their unfinished work.

**Gate:** none red. Everything that exists is green.
**Cost:** the desktop still boots to the old fixed layout. Groups D2, E1–E3 and
E4–E7 all need policy in `kernel.zl` and inherit the same block.
**What is NOT blocked:** the mechanism. zl compiles *to C* (`zl_fn_<name>`
taking and returning `Value`), so the eventual wiring is small and mechanical:
`wm_hooks()` takes three function pointers, and the shims that call
`zl_fn_app_draw` / `zl_fn_app_event` / `zl_fn_app_tick` are a dozen lines.
`wm.c` is written against that signature already.
**Resolution — the C half is now DONE and shipped.** `kernel/wmglue.c` holds
every shim, and its references to the four zl functions are **weak**, so it
links today in a kernel where they do not exist and starts working the day they
do, *with no change to any C file*. `wm_bind_zl()` returns 0 and declines;
`wm_available()` checks both that there is a framebuffer and that `app_draw`
exists. Both asserted in `wmtest`.

What remains is only `kernel.zl`, written out verbatim in
**`kernel/docs/desktop-wiring.md`** — the four `fn app_*` dispatchers, the boot
sequence, and the C4 deletion. Keep the `wm_available() == 0` branch on the old
shell loop: verify.sh boots `-kernel -display none` where there is no
framebuffer, and the transcript must stay byte-identical.

### T-9 | 2026-08-18 | open
**Tension:** C4 — delete `fb_bg_snapshot` / `fb_bg_restore` / `fb_grab` /
`fb_stamp` / `bg_buf` / `sp_buf` — cannot be done yet. `kernel.zl`'s drag loop
still calls all four through the `bg_snap`/`bg_rest`/`grab`/`stamp` builtins,
so deleting them breaks the build, and `kernel.zl` is mid-flight (T-8).
**Gate:** none.
**Cost:** ~10 MiB of fixed high RAM still reserved at 128 and 160 MiB, plus the
shadow-halo smear trail measured in 0a. `wm.c` does not use any of it — it
repaints from damage — so this is dead weight the moment the wiring lands, not
a design question.
**Resolution:** delete it in the same change that wires the compositor in.
Freeing 128–176 MiB also lets the back buffer move down and cover 4K, which is
the one mode `fb_setup` currently reports as degraded.

### T-10 | 2026-08-18 | open — **REPO STATE, needs Roy's eye before anything is merged**
**Tension:** the display session's three commits landed on **my** branch, not on
`main`. `main` is still at `44346d6`. `desktop/overnight-compositor` contains,
underneath my eleven:

```
a7fa676  docs: display roadmap status, and why phase 8 stays unwritten
b822c45  feat(intel): external DisplayPort - phase 3
f073f74  feat(intel): hotplug and DRRS/PSR - phases 4 and 7
```

They touch `kernel/intel.c` (+378), `kernel/docs/display-roadmap.md` and
`kernel/hosttest/intel_probe.c` — no overlap at all with the desktop work, so
nothing is corrupted and nothing is lost.

**Cause:** two sessions, one checkout, one `.git`. The brief's first action was
`git switch -c desktop/overnight-compositor`, which changed the checked-out
branch **for both sessions**, so the display session's subsequent commits went
where HEAD pointed — here.
**Gate:** none. Everything builds and every gate is green.
**Cost:** if the display session believes its phases 3, 4 and 7 are on `main`,
they are not. Anyone branching from `main` will not have them.
**Resolution:** Roy's call, and deliberately not mine — rewriting shared history
is on the brief's forbidden list. The cheap fix is
`git branch -f main f073f74` (or a fast-forward of `main` to those three, which
are already a linear prefix of my branch) and then rebasing or leaving the
desktop commits above it. **Do not force-push anything anywhere.**
**Worth taking as a lesson:** `git switch -c` is not a private act in a shared
checkout. A worktree would have been the right isolation
(`git worktree add ../zl-desktop -b desktop/overnight`), and CLAUDE.md's own
note about concurrent sessions says so.

### T-11 | 2026-08-18 | open — **latent, EFI only, NOT introduced tonight**
**Tension:** the GOP framebuffer address is **truncated to 32 bits** in the EFI
build, and the `-Werror` guard added for exactly this bug class does not catch
it.

`buildefi.sh` targets `x86_64-unknown-windows` — LLP64, where `unsigned long`
is **4 bytes**. Proven rather than assumed: `_Static_assert(sizeof(unsigned
long) == 8)` fails on that target.

The whole chain is `unsigned long`:

```
efi.c:250    fb_addr = (unsigned long)gop->mode->framebuffer_base;   <- UINT64 loses its top half HERE
efi.c:287    console_init_efi(fb_addr, ...)
console.c:172  console_init_efi -> fb_setup(addr, ...)
fb.c:430     fb_setup(unsigned long addr) -> fb_base = (unsigned char *)addr
```

and back out through `fb_phys()` → `console_vram()` → the `vram` zl builtin.

**Why the guard misses it.** `CLAUDE.md` names this class — *"never put a
pointer through `unsigned long` in the EFI build"* — and `buildefi.sh` carries
`-Werror=pointer-to-int-cast`, `-Werror=int-to-pointer-cast` and two others.
Compiling both cast shapes with the **exact** build line produces **no
diagnostic at all**. Those flags catch pointer↔`int`; this is a UINT64 narrowed
by an *explicit* cast, and no warning catches an explicit cast. The guard is
narrower than the rule it was written to enforce.

**Gate:** none red. `verify-efi.sh` is green, and stays green, because OVMF puts
the framebuffer low.
**Severity: latent, not active.** A GOP framebuffer base is normally a PCI BAR
in the 32-bit MMIO window, which is why QEMU and OVMF never show it. Firmware
that places it above 4 GiB gives a black screen, or writes into whatever lives
at the truncated address. Same shape as the two `lidt`/`lgdt` truncations
CLAUDE.md records — those were also invisible in QEMU.
**Resolution:** three declarations become `unsigned long long` — `efi.c`'s
`fb_addr`, `console_init_efi` in both `efi.c` and `console.c`, and `fb_setup`'s
parameter. **Changing only `fb_setup` would be WORSE than leaving it**:
`console.c`'s declaration would then disagree with the definition about the
size of a register argument. Two of those three files were mid-flight all
night, so this is the display session's to make, not mine. A comment at
`fb.c`'s `fb_setup` points at it.

### T-12 | 2026-08-18 | RESOLVED 2026-08-18 — git object corruption, one commit
**Tension:** `.git` had **four zero-length object files**, all stamped 10:58,
and `HEAD` plus the branch ref pointed at one of them. Every git command
failed with `object file ... is empty`.

Not disk space — 364 GB free. Not a partial write of real data either: the
files were **zero bytes**, which is what an interrupted or raced object write
leaves behind. Two sessions share one `.git` in this checkout, and the display
session had been committing to the same branch all night (T-10). That is the
most likely cause and it is not provable after the fact.

**Blast radius, established before touching anything:**

| | |
|---|---|
| commits lost | **one** — the map-hardening commit, its tree and its blob |
| content lost | **none** — it was in the working tree, verified by grep before any repair |
| `main` | intact |
| the display session's commits | intact (`c181310`, `44346d6` both readable) |
| my other 27 commits | intact |
| all four boot gates | green throughout — the *code* was never affected |

**Repair**, after checking no git process was running:
`.git/logs/HEAD` backed up → `git update-ref` back to the last readable commit
(`73cdc30`) → delete the four empty objects (they contain nothing to lose) →
`git reset` to rebuild the index, whose cache-tree also referenced a dead
object → re-stage and re-commit. `git fsck` reports **0 errors**.

**Worth keeping:** the reflog is plain text and survived intact when the object
store did not. It is what made "exactly one commit, and here is its message"
answerable in seconds rather than a guess. Read `.git/logs/HEAD` first, before
anything else, when git itself will not talk to you.

**Standing risk:** T-10's cause is unfixed. Two sessions, one `.git`, one
branch. A worktree (`git worktree add ../zl-desktop -b …`) gives each its own
index and ref namespace while sharing the object store, and is what should have
been used from the start.


### T-13 | 2026-08-18 | RESOLVED — the harness could not drive its own desktop
**Tension:** `wm_frame()` reads `input.c`'s event queue and nothing else. zl's
`key_get()` read COM1 directly. The two never met, so a key typed over serial
could reach the old text shell and could NOT reach the compositor.

`desktop-v10-plan.md` §1a proposed `probe-shot.py -k w` as the gate for typed
commands, on the assumption that it worked. It did not, and the failure mode was
silent: the keys went into the void and the screenshot looked like a shell that
had ignored them.

**Blast radius had it shipped:** every gate and probe in this repo drives zlOS
over serial. `verify.sh`, `verify-raw.sh`, `exercise.py`'s 24-step sweep and
seven `probe-*.py` scripts. The moment the compositor became the boot state on
any machine with a framebuffer, all of them would have stopped working — and
`verify.sh` would have stayed GREEN throughout, because it boots
`-display none` where there is no framebuffer and the text shell still runs.
That is the same shape as "three gates were green while the 64-bit build was
dead".

**Resolution:** COM1 is a third source feeding the one queue, alongside PS/2 and
USB. EV_CHAR only, no synthesised key event — a serial byte has no press,
release or modifier state, and synthesising one would have changed `input_key()`
for the editor.

**The trap inside the fix:** an undecoded port floats high, so "is LSR bit 0
set" is TRUE forever on a machine with no UART and RBR reads 0xFF forever. That
is the ThinkPad, which has no serial port and for which the screen is the only
diagnostic. `ser_rx()` probes the scratch register once and answers -1 forever
after if nothing decodes it. zl's old `ser_ready()` had exactly this bug and it
had simply never been booted on hardware without a UART.

### T-14 | 2026-08-18 | RESOLVED — the raw bootloader loads a FIXED size
**Tension:** `raw_boot.asm` reads `CHUNKS` × 32 KiB whatever the kernel's
actual size. CHUNKS was 40 = 1.25 MiB; the kernel was 1.23 MiB. **84 KiB of
headroom**, and v10 §6.8's type scale adds 250 KiB.

**Why it is worse than a build failure:** there is no error. The loader reads
its 40 chunks, jumps to 1 MiB, and executes a kernel whose tail never arrived.
The symptom is a hang or a triple fault a long way from the cause, on the one
boot path with no serial console.

**Resolution:** CHUNKS is 60 (1.875 MiB, the largest whole number that fits
behind the boot sector in the 2 MiB image), and **`mkdisk.sh` now refuses** to
build an image whose kernel exceeds what the loader will read. The check reads
CHUNKS out of the assembly source, so the two numbers cannot drift.


### T-15 | 2026-08-18 | RESOLVED — the compositor could not see the mouse
**Tension:** zlOS drives two pointers. `xhci.c` reads an absolute usb-tablet;
`idt.c` reads a relative PS/2 mouse. The `mouse_x` builtin has preferred the
tablet since it was written — a tablet cannot drift, and on a UEFI laptop the
PS/2 emulation dies with ExitBootServices.

`input.c`'s `pump_mouse()` read `idt_mouse_x()` **and nothing else.**

**Why it was invisible for as long as it was.** While the shell owned the
screen, the shell called `mouse_x()` directly and got the right answer; the
event queue's mouse events were nobody's input. The moment `wm_frame()` became
the top of the system the queue became the compositor's ONLY source of pointer
events — and on any machine with a usb-tablet, which is what QEMU gives and
what `try.sh` attaches, `idt_mouse_x()` never changes, so `pump_mouse` saw no
change and pushed nothing. No dragging, no window focus by click, no close box,
no dock, no menu. The entire pointer half of the desktop.

**Why no gate caught it, and none could have.** Every gate and probe in this
repo drives zlOS by TYPING. `verify.sh`, `verify-raw.sh`, `exercise.py` and
seven `probe-*.py` scripts. A dock that does nothing photographs identically to
a dock that works, and the compositor's own assertions (`wmtest`) inject
pointer state through stubs that write `idt_mouse_*` — so the harness had the
same bug as the kernel and they agreed with each other.

**Resolution:** `pump_mouse()` prefers the tablet, exactly as the builtin does —
one rule, one place. `inputtest` gained five assertions covering the
preference, the fallback, and the case where both move at once. `probe-dock.py`
is new and drives the POINTER rather than the keyboard: hover changes a tile,
press changes it again, a click opens a window, the start button opens the
menu, a click outside dismisses it.

**The lesson, which is T-13's with a different subject:** when a rewrite moves
the top of the system, everything the old top read DIRECTLY becomes something
the new top must read through its own plumbing — and the plumbing may only
carry half of it. T-13 was the same shape for the keyboard. Two instances now,
which makes it a rule: enumerate what the old top read, and check each one
reaches the new top.


### T-16 | 2026-08-18 | RESOLVED — a pattern, not an incident: built, asserted, uncalled
**Tension:** five separate things in `wm.c`/`fb.c` were complete, correct, and
had **no caller anywhere**. Each looked done from the inside and did nothing
from the outside:

| | had | was missing |
|---|---|---|
| `WF_MODAL` | a setter, and a branch in `route_mouse` | anything that set it — until the start menu |
| `wm_resize()` | clamping, damage on both sides | anything that called it — until the resize grip |
| `fb_blur_cache` | 7.37 ms measured, three correctness assertions | anything that blurred — until the menu and the dock |
| the animation timeline | five kinds, eight slots, 12 assertions | anything that started one — and `wm_open` used a **separate** legacy counter, so `wm.c` carried two animation systems and one never ran |
| `MOD_SUPER` | tracked in `input.c` from the day it was written | any event to fire on: a modifier emits none |

**Why this shape is so easy to produce here.** Everything in this repo is
written mechanism-first and gated hard, so a primitive arrives with tests, a
measurement and a comment explaining its design — and passes every check while
being unreachable. `HANDOFF.md` already names this for `intel.c` ("the code
exists is not the code works — check for an actual caller"). It had reappeared
in three more files.

**Resolution:** all five now have callers, and each caller has a gate that
would notice if it lost one. The general fix is the discipline, not the code:
**a primitive is not done when it passes its test, it is done when something
calls it and a gate covers the call.** For visual work specifically, the
assertion has to check the *pixels* and not the state — `wm_anim_alpha()`
reported a fade correctly for hours while nothing drew one.


### T-17 | 2026-08-18 | ACTIVE — T-10 bit again: another session committed MY working tree
**Tension:** while the desktop session was mid-flight, a second session on the
same checkout committed `kernel/keycodes.h` — a file the desktop session had
uncommitted edits in (`KEY_SUPER`, for the Super-tap gesture). Their commit
message is "fix: track kernel/keycodes.h, which input.c includes but git never
had", so the intent was to start tracking an untracked file, and it swept in
whatever was in the working tree at that instant.

**Nothing was lost this time** and the content is correct — `KEY_SUPER` is in
the commit and the file reads clean. But it is luck, not design: the same
sequence with a half-finished edit in the tree commits a broken file under
someone else's message, and `git status` afterwards shows nothing wrong.

Their other four commits touched `apic.c`, `cpu.c`, `pci.c` and language work,
none of which overlapped — checked, both times, before staging.

**Standing risk, unchanged since T-10 and now with three incidents:** two
sessions, one `.git`, one branch. A worktree (`git worktree add ../zl-desktop`)
gives each its own index and ref namespace while sharing the object store. The
only reason this session did not use one is that the entire desktop state was
UNCOMMITTED when it started - 56 modified and untracked files, including the
plan document itself - so a worktree checkout would have started from a tree
that did not contain the work. That is worth knowing as the actual blocker
rather than "we forgot".


### T-18 | 2026-08-18 | ACTIVE — the toolkit is unreachable from where the apps are
**Tension:** `ui.c` is a complete immediate-mode toolkit - label, bar, button,
sep, space, toggle, slider, num, list_row, scroll - built, asserted in
`wmtest`, and documented as the layer every earlier plan was missing.

**It has no zl bindings.** `ui_scale` and `ui_theme` are the only `ui_*`
builtins in `runtime_kernel.c` and neither is a widget.

Every app in zlOS lives in `kernel.zl`. So not one of them can call a single
widget. The start menu written today draws its rows with raw `label()` and
`rrblend()` calls, and that is how this was noticed - not by an audit, by
hitting the wall while building something.

**What it cost:** `desktop-northstar-feasibility.md` scored the toolkit at 90%
and the whole mockup at 65%. Both counted what was BUILT rather than what was
REACHABLE. The corrected number is ~35%, and the correction is larger than any
of the work done today. Third time this document has been wrong; the first two
were the same mistake in the renderer and the language.

**Not resolved.** The fix is a set of builtins - `ui_begin`, `ui_button`,
`ui_slider`, `ui_list_row`, `ui_scroll`, `ui_fired` - plus the decision about
how an immediate-mode API with an out-parameter (`ui_toggle(s, int *on)`)
crosses into a language with no pointers. That is real design, not plumbing,
and it is the single highest-leverage item on the desktop board: thirteen
applications are waiting behind it and each one is much cheaper with it than
without.

### T-19 | 2026-08-18 | RESOLVED — desk_draw ignored the rectangle it was given
**Tension:** `desk_draw(x, y, w, h)` is called ONCE PER DAMAGE RECTANGLE, up to
eight times a frame. The wallpaper honoured the rectangle. The header and the
dock did not - both redrew in full, every call.

That was survivable while the dock was a flat gradient. Today it became a
cached-blur blit plus a full-width translucent tint, which from fbbench's own
numbers is 245,760 px at 1.54 cyc/px plus 22.16 cyc/px - about 5.8M cycles,
**2.5 ms**. Eight damage rectangles is **20 ms of dock alone** against a
16.67 ms budget, and seven of those eight usually do not touch the dock at all.

**Found by being asked "why is it so slow", not by a gate.** Nothing in this
kernel had ever timed a frame: `idt_ticks()` is 100 Hz, which is 10 ms of
resolution against a 16.67 ms budget. desktop-TODO 0h has said "add a tsc()
builtin and put frame time on screen - DO THIS BEFORE ANY PERFORMANCE WORK"
since it was written, and the v10 run did the performance work first and the
measurement never.

**Resolution:** the furniture is drawn only when the rectangle overlaps it -
which is not an optimisation of the drawing, it is noticing that `desk_draw`
was handed a rectangle and ignored it. And `wm_frame()` now times itself with
the TSC in MICROseconds (a cheap frame is well under 1 ms; integer
milliseconds would report every one of them as zero), only counting frames that
actually repaint, and the tray shows the last frame and the peak.

### T-20 | 2026-08-18 | RESOLVED — the desktop was slow, and there was no instrument
**Tension:** asked directly why it felt slow. Nothing in this kernel had ever
timed a frame — `idt_ticks()` is 100 Hz, 10 ms of resolution against a 16.67 ms
budget — so every performance claim about the compositor was arithmetic from
`fbbench`'s per-primitive numbers, which is not the same thing as a frame.

**desktop-TODO 0h has said this since it was written:** *"add a tsc() builtin
and put frame time on screen. DO THIS BEFORE ANY PERFORMANCE WORK. Optimising
without measurement is guessing."* The v10 run did the performance work first
and the measurement never.

**Measured, once the instrument existed:**

| | last | peak |
|---|---|---|
| after boot | 12,207 us | 129,883 us |
| **hovering nine dock tiles** | **7,426 us** | **18,455 us** |

7.4 ms to light up one chip, peaking over the entire frame budget.

**Cause, and it is two mistakes stacked.** `desk_draw(x, y, w, h)` is called
ONCE PER DAMAGE RECTANGLE, up to eight a frame. The header and the dock ignored
the rectangle and redrew in full every call — mistake one, fixed by honouring
it. But even one dock repaint was ~2.4 ms of blend: the strip is 245,760 px and
the tint over it is 22.16 cyc/px, and it was being recomposited from
blur-then-tint every time anything in the dock changed.

**None of it ever changes.** The blur is of a fixed wallpaper, the tint over it
is fixed, and eight of nine tiles look the same as they did last frame. So the
whole static composite — glows, vignette, header, blurred and tinted dock,
start button, nine tiles at rest — is baked into the wallpaper bitmap once, and
a frame pays one blit at 1.54 cyc/px for all of it. What is left live is the
hovered tile, nine small open-app markers, and the tray numbers.

**The instrument stays.** `wm_frame()` times itself with the TSC in
microseconds, counts only frames that repaint, resets its peak once boot
settles, and both numbers are in the tray where "state: compositor" used to be
— a label that had said the same thing on every boot since it was written.
`probe-frame.py` drives real interaction and reads them back.

### T-21 | 2026-08-18 | RESOLVED — the scissor was write-only, and an app paid for it
**Tension:** with the dock baked in, a drag still cost 19,399 us per frame on a
quiet box - over the 16.67 ms budget.

**A drag is not slow because of the window being dragged. It is slow because of
the window UNDERNEATH it.** `app_draw` is called once per damage rectangle, and
dragging across the shell damages a band of it - two or three rows out of
thirty-seven. `term_draw` walked its WHOLE scrollback every time, laid out
every line, blitted it glyph by glyph, and the scissor rejected it a pixel at a
time afterwards. fbbench measures forty lines of AA text at 4.588 ms.

**Why it was invisible:** `fb_clip()` had been WRITE-ONLY since it was built.
That is fine while every customer is an `fb_*` primitive, because those fold
the scissor into their own loop bounds and a clipped draw genuinely costs no
more per pixel than an unclipped one. **An app cannot do that.** It has no way
to ask what band it is being asked to paint, so it paints everything.

`ui_scroll` already had the right rule and said so - "rows outside the viewport
are rejected, not drawn and clipped" - and nothing outside `ui.c` could follow
it.

**Resolution:** `fb_clip_top/bot/left/right`. `term_draw` skips rows outside the
band. The scissor is still the correctness guarantee; this is about not doing
the work first.

**The general lesson:** a correctness mechanism that is invisible to its
callers makes them do redundant work. Any clip, damage or visibility system
should be readable by whoever draws inside it, or "it is clipped anyway"
quietly becomes "it is drawn and then discarded".

**AND THE FIX IS NOT MEASURED, WHICH IS ITS OWN TENSION.** The before run was
at host load 2.25 and reported 19,399 us; the after run was at load 7.43 and
reported 16,000 us. That is not an A/B - the second had three times the
contention of the first - and every re-run since has landed at load 5-7,
because a second session shares this four-core box. So the change stands on the
argument that it removes provably-discarded work, not on a stopwatch. Stating
which of the two a claim rests on is the whole point; CLAUDE.md already records
a bisect lost to exactly this ("baseline and modified passing and failing
together, tracking host load").

**What this needs:** a quiet box, or a frame benchmark that runs on the HOST
like fbbench does for fb.c. The second is the better answer - fbbench compiles
the shipping fb.c and times it with a cycle counter and no QEMU at all, and
there is no reason a wm.c/term.c frame could not be timed the same way. That is
the real fix for "the stopwatch does not work here".

### T-22 | 2026-08-18 | RESOLVED — fb_shadow clipped to the screen, not the scissor
**Tension:** with a stopwatch that finally worked (`hosttest/wmbench`), a drag
frame attributed as: the shell's scrollback 38%, the monitor's contents 19%,
**chrome and wallpaper 55%** - the biggest single chunk, and not an app at all.

`wm.c` calls `chrome()` once per window per damage rectangle, and a drag
damages a band. `fb_shadow` - the widest, softest, most per-pixel loop in
`fb.c` - clipped to the SCREEN and left `put_pixel` to reject the rest one
pixel at a time. Every call walked a whole window's shadow band to produce a
sliver of it.

**It predates fb_clip and was never wrong.** desktop-TODO 0b listed the five
DIRECT back-buffer writers that needed the scissor folded into their loop
bounds; `fb_shadow` is not one of them, because it goes through `put_pixel` and
was therefore always correct. Correct-but-slow is the harder kind to find:
nothing is ever wrong, no assertion fails, and no screenshot differs.

**Resolution:** the scissor is folded into its loop bounds like every other
primitive in the file. `chrome + wallpaper` went 6,651 -> 4,213 us/frame, and
`fbbench`'s scene hashes are **byte-identical at all three modes** - the change
moves no pixels, which is the proof that it only skips discarded work.

**Same shape as T-21, one file over.** A clip that its callers cannot read
makes them do work that is thrown away; a clip that a PRIMITIVE does not fold
into its bounds does the same thing one level down.

### T-23 | 2026-08-18 | RESOLVED — the benchmark had to be debugged before it could be believed
**Tension:** `wmbench` was built to escape host-load contamination, and its
first two runs of identical code reported 12,061 and 20,210 us/frame - a 70%
swing.

**A cycle counter is not immune to a busy machine.** `rdtsc` counts at a
constant rate regardless of core frequency, so a contended or downclocked core
inflates the cycle count for identical work. It is immune to QEMU, which is a
smaller claim than it first looks.

Fixed by interleaving: all four scenarios run inside each repetition, so they
share the same span of time and the DIFFERENCES survive drifting load even when
the absolutes do not. desktop-TODO 0c used the same interleaved A/B to prove
the damage list cost nothing.

**Second bug, worse:** the attribution subtracted unsigned cycle counts. When a
component's cost fell inside the noise, drawing LESS measured slower, the
subtraction wrapped, and the harness printed
`667233241162193 us (8421476900969%)`. A benchmark that prints an absurd number
is worse than one that prints nothing, because the absurd number is the one
that gets quoted. Signed now, and a delta at or below zero is reported as
"below the noise floor" - which is a finding, not an error.

**Standing rule for anything measured on this box:** report the percentages,
and label the absolutes as load-dependent.
