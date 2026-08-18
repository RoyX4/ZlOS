# LEARNINGS — what this project taught us

<!-- Durable, project-specific knowledge: how this codebase actually behaves,
     what turned out to be false, decisions and why. Not a work log — STATE.md
     covers status. Something belongs here only if a future session would get it
     wrong without being told.

Format:

### YYYY-MM-DD — short title
**Learned:** the fact, stated plainly
**Because:** what happened that taught it
**So:** what to do differently as a result
-->

### 2026-08-17 — `fb.c` is not a compositor
**Learned:** It is an immediate-mode drawing library plus three unrelated tricks:
one full-screen back buffer, **exactly one** dirty box (`mark()` grows a single
rect), and a background-snapshot + sprite grab/stamp drag. There is no window
list, no z-order, and **no clipping** — every primitive clips to the screen and
nothing else.
**Because:** It was described as "the compositor" going into the desktop planning
session. Reading it showed the drag mechanism only works for one window over a
background that never changes, which is why the terminal is fixed in place and
why exactly two windows are draggable.
**So:** Do not plan around "the existing compositor." The compositor does not
exist yet. The enabling change is a clip rect — and it is small, because
`fb_fill_px` and `put_pixel` are the only two functions that clamp, and every
other primitive is built on them.

### 2026-08-17 — the zl kernel subset has no lists, and that decides the architecture
**Learned:** `kernel/README.md:154` states it outright. `runtime_kernel.c:471`
makes calling a function value a named fault. So a window table — which is a
list — cannot live in zl at all.
**Because:** It explains what otherwise looks like laziness: `raise_windows()`
is a hand-unrolled if-ladder over exactly two windows, window state is the loose
globals `mon_x`/`mon_y`/`ab_x`/`ab_y`/`mon_open`/`ab_open`/`focus_win`, and the
click hit-test is a hardcoded nest six `if`s deep. None of that is a style
choice; it is the only thing the language permits.
**So:** Mechanism (window table, z-order, hit-test, focus, damage, routing) goes
in C. Policy (theme, layout, what each app draws, what the dock launches) stays
in zl. Any plan that puts the window loop in zl is dead on arrival — check this
constraint before proposing a data structure.

### 2026-08-17 — every zl builtin call is a linear string-compare chain
**Learned:** `zl_calln` in `freestanding/runtime_kernel.c:483` walks **301
`streq` comparisons** in source order. The graphics vocabulary sits at positions
255–287. `mouse_x`/`mouse_y`/`mouse_btn` are 285/286/287 and are called three
times per frame — ~860 string compares before the main loop decides anything.
Each `Value` is ~56 bytes and travels through varargs by value.
**Because:** Measured while deciding where the per-window repaint loop belongs.
**So:** Cost scales with call *count*, not work done. A loop that iterates N
windows × ~30 primitives from zl pays ~250 compares per primitive; from C it pays
none. This is a second, independent reason the repaint loop belongs in C — and a
reason to prefer few coarse builtins over many fine ones when extending the
zl↔C surface.

### 2026-08-17 — a silent fallback is worse than a missing feature
**Learned:** `fb.c`'s `back_on = 0` path is *correct* — drawing straight to VRAM
works. The defect is that it is silent, and that four unrelated features
(back buffer, subpixel AA, fast pixel readback, window dragging) are all keyed
off that one flag. They vanish together with no log line.
**Because:** Found while tracing why the desktop would behave on the ThinkPad's
2560×1440 panel (T-1).
**So:** When a capability degrades, say so in the boot log. And treat one flag
gating four features as a smell — the next person will debug "dragging is
broken" and never look at the resolution.

## 2026-08-18 — the overnight desktop run

**A gate that only runs in the degraded configuration tests the wrong code.**
`fb_clip` with only the two functions the plan named passed at 3840×2160 and
leaked 2,184,000 pixels at 1920×1200. It passed at 4K *because* the back buffer
is off there, so the three unfixed fast paths fell back to the one path that
was fixed. If I had only tested the mode I was chasing, I would have shipped a
broken keystone with a green light.

**"It moved" is not "it moved where it was told".** `probe-mouse.py` asserted
only that the pointer changed position, and passed for weeks while the pointer
was being driven to 0,0 — both axes pinned at their clamp. Any test whose
predicate is weaker than the property you care about will eventually pass while
the property is false.

**Run the experiment that could refute you, not the one that agrees.** I
concluded "the usb-tablet steals the pointer" from two measurements that were
both consistent with it, and removing the tablet made the test pass — which
felt like confirmation. It was not. Sending *absolute* events with the tablet
still attached showed the tablet path works perfectly, and my harness had been
sending the wrong event type. Retracted in T-5, and the commit says so.

**Check what the compiler already does before hand-writing the optimisation.**
Before SIMD, `objdump` on the `-O2` object showed GCC already vectorising
`fb_rrect`, `gamma_init` and `dmg_add`, and *not* `fb_fill_px`, `fb_present` or
`fb_gradient` — because the "very-cheap" cost model refuses runtime trip
counts. That five-minute check is what turned "SIMD something" into "SIMD these
two loops", and it is the cheap version of the mistake DECISIONS.md #25 records.

**Ask where the feature is enabled before using it.** SSE is switched on in
`boot64.S` and nowhere else — the 32-bit entry never touches CR4. Unconditional
SSE in `fb.c`, which is compiled into both, would have faulted the exact kernel
`verify.sh` boots. `#ifdef __SSE2__` tracks the build with nothing to keep in
sync, and `objdump | grep -c xmm` on the 32-bit object proves it mechanically
rather than by argument.

**Code with no caller is not code that works.** `wm.c`'s modal branch could
never execute because nothing set `WF_MODAL` — the same hazard `HANDOFF.md`
names for `intel.c`. Found by re-reading my own file against this project's
known bug classes, which is worth doing to new code and not only to old.

**A refusal must say so.** Two silent fallbacks found in the same audit:
`wm_open` returning −1 with no message, and `fb_setup` rejecting an unsupported
depth without a word. Both are the bug class desktop-TODO 0a was written about,
reappearing in new code written by someone who had just fixed the old one.

**Concurrent sessions in one checkout are a real hazard, not a theoretical
one.** The build broke mid-run because another session had added externs for
functions it had not written yet; `verify-efi.sh` went red once on unchanged
code, almost certainly catching a half-written file; and `mouse_x` was rewired
underneath me while I was measuring it. `git status` before every gate, and
treat a single red as "re-run it" rather than "bisect it".
