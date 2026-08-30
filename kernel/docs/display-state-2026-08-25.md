# The display, measured — 2026-08-25

A read-only orientation pass over the display subsystem. Nothing was changed.
Every number below carries the command that produced it; where a tracked
document disagrees with the tree, the tree wins and the document is named.

Baseline: branch `codex/local-zlos-snapshot-2026-08-24`, HEAD `85fcdf5`, dirty
tree (another session was reorganising `docs/` during this pass).

---

## The one sentence

**The Intel driver is a complete, register-verified eDP modeset for exactly one
port, one pipe, one panel and one mode — and above it sits a genuinely good
compositor. What separates the two halves is not code, it is callers: 59 of the
305 `intel_*` functions have no call site anywhere in the shipping tree, and 40
of those 59 are precisely roadmap phases 0.3 through 7.**

"Written and cross-checked against firmware" is not "runs". The roadmap's own
STATUS table says *done* for phases 1–7; that word means the first thing, not
the second, and reading it as the second is the trap this page exists to close.

---

## Layer 1 — `intel.c`, the modeset driver

### The ignition path is real and complete, and it is manual

Three tracked documents still say *nothing in the kernel arms `lt_armed`*. That
has been false for a while. The full chain, verified end to end:

```
kernel/kernel.zl:2047     if cmd == 80 {              # the 'P' shell command
kernel/kernel.zl:2063         fb = panel_up()
freestanding/runtime_kernel.c:1976   "panel_up"  -> intel_bringup_panel()
kernel/intel.c:4372       u32 intel_bringup_panel(void)
kernel/intel.c:4417           intel_link_train_arm(1)
kernel/intel.c:4418           intel_modeset_run(0)
kernel/intel.c:4419           intel_link_train_arm(0)
kernel/kernel.zl:2083     while pk == 0 { pk = key_get()  idle() }
kernel/kernel.zl:2084     panel_console()  -> intel_panel_takeover()  (intel.c:4470)
```

Reproduce:

```sh
git grep -n 'intel_link_train_arm' -- kernel/ freestanding/ | grep -v hosttest
grep -n 'panel_up\|panel_console' freestanding/runtime_kernel.c kernel/kernel.zl
```

Two things follow, and they pull in opposite directions:

- Phase 0.1 is **not** "one caller away". It is wired.
- Nothing arms it **at boot**. A human has to press `P` at the zlOS shell. So
  the highest-value item on the whole board is gated behind a keystroke nobody
  has pressed on the ThinkPad.

### `STATE-OF-THE-PROJECT.md` §5.1 is now CLOSED

That section says typing `P` lights the panel and then **halts the machine**,
because `key()` fell through the builtin dispatcher into `kfatal`. Fixed in the
current tree — `kernel.zl:2083` is now `while pk == 0 { pk = key_get() idle() }`,
and `key_get` is a zl function (`zl_fn_key_get`), a direct call that cannot
reach the dispatcher at all.

```
$ python3 kernel/check-zlcalls.py
check-zlcalls: 852 zl functions, 707 builtins, every call site resolves
```

No unresolved call remains. §5.1 should be marked closed.

### What has a caller, and what does not

```
$ # 305 intel_* definitions in intel.c; call sites searched across
$ # kernel/*.{c,h} + freestanding/*.c, excluding hosttest/ and _gen*
intel_* defined in intel.c ............................... 305
  no call site in shipping source ........................ 139
     of those, used by the hosttest/ harness only ......... 80
     of those, used by NOTHING at all ..................... 59
```

The 80 are not dead — they are the read-only introspection surface the Linux
host harness drives, and that harness *is* the development loop. Leave them.

The 59 are the real map of the frontier:

| Roadmap phase | uncalled |
|---|---|
| 0.3 link-training retry (`intel_link_train_auto`, `intel_lt_*`) | 8 |
| 1 VBT (`intel_vbt_find`, `intel_vbt_block`) | 2 |
| 2 pipe generality (`intel_pipe_select`, `intel_pipe_current`) | 2 |
| 3 external DP (`intel_ext_port_prepare`, `intel_ext_port_probe`) | 2 |
| 4 hotplug (`intel_hpd_*`) | 6 |
| 5 planes and scalers (`intel_plane_setup`, `intel_scaler_*`) | 6 |
| 6 colour (`intel_gamma_set/get`, `intel_csc_mode`) | 3 |
| 7 PSR and DRRS (`intel_psr_*`, `intel_drrs_*`) | 11 |
| loose accessors and save/restore pairs | 19 |
| **total** | **59** |

Phase 2 deserves its own note, because the name misleads. There are still 40
`*_A`-suffixed register macros in `intel.c`, which reads like "hardcoded to pipe
A" — it is not. `intel.c:135` is

```c
#define PIPE_REG(base)    ((base) + (u32)cur_pipe * PIPE_STRIDE)
```

so every one of them is genuinely pipe-indexed. The generality is implemented.
What is missing is that `intel_pipe_select()` has no caller, so `cur_pipe`
(`intel.c:125`) is `PIPE_A` for the entire life of the process. **The mechanism
exists; nothing ever selects a second pipe.** That shape — mechanism present,
selector unwired — repeats for VBT, hotplug, PSR, DRRS, scalers and colour.

### Phase 0 leftovers, checked individually

| Item | State in the tree |
|---|---|
| 0.1 wire into zlOS | **done** — chain above; `P` never pressed on hardware |
| 0.2 second modeset (off→on→off→on) | **not done** — needs a hardware run, not code |
| 0.3 link-train retry | written (`intel_link_train_auto`) and **uncalled** |
| 0.4 EDID over AUX | **done and called** — `intel_edid_over_aux` at `intel.c:2033`, `intel_read_edid_aux` at `:2061` |
| 0.5 X-tiled scanout | primitive **done** (`intel_plane_configure_tiled`, `intel.c:3593`), **but the boot path is still linear** — `intel.c:4404` computes `stride = (w*4 + 63) & ~63` with the comment `/* linear: 64-byte multiples */` |

0.5 is the cheapest visible win left in this layer: the tiled path is written
and the only thing using it is the harness.

---

## Layer 2 — the compositor and how it looks

`wm.c` (3,154 lines) over `fb.c` (3,650) is the other half of "display", and it
is in much better shape than layer 1's frontier suggests. Rendered from the
current tree, not from a committed screenshot:

```sh
cd kernel/hosttest
gcc -O2 -w -o /tmp/wmshot wmshot.c ../wm.c ../ease.c ../notify.c ../snap.c \
    ../ui.c ../uikit.c ../wmglue.c ../settings.c hoststubs.c ../fb.c \
    ../input.c ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
/tmp/wmshot /tmp/wm.ppm 2560 1440
```

Compiles clean and renders 2560×1440 with four windows. What is on screen:
proportional anti-aliased text with correct spacing, rounded window frames with
soft shadows, a dark theme with a lime accent, a dock with legible 48 px icons,
progress meters, a toggle switch, a tabbed window, a scrollbar.

**`docs/desktop-look.md` is stale on its own headline bug.** It says the single
most visible source of blockiness is `fb_icon24` doing nearest-neighbour 2×
upscaling (`ic[y / sc][x / sc]`). That is fixed. `fb.c:3624` retains the old line
in a comment and `fb.c:3635-3647` now picks the nearest native atlas and
bilinear-resamples once. Verified by eye at 1:1 on the dock crop — the icon
edges carry real anti-aliasing.

Two defects visible in the render, neither previously written down here:

- **System Monitor clips its own content.** The volume row is cut off by the
  window's bottom edge rather than scrolled or resized to fit.
- The `zl shell` title bar renders a trailing `–` after the title string.

Both are compositor/app-layout, not driver.

---

## Documents to correct

Each of these is stale **in the pessimistic direction**, which is the expensive
direction: a wrong "not supported" never gets tested, because nobody tries what
they are told is absent.

| Document | Says | Truth |
|---|---|---|
| `kernel/HANDOFF.md` (three places) | nothing in the kernel arms `lt_armed` | `intel.c:4417` does, from `kernel.zl:2063` via `panel_up` |
| `kernel/docs/display-roadmap.md` §"Where the code actually is" | no VBT parser; hotplug zero lines; 42 `*_A` registers | parser at `intel.c:4512-4571`; `intel_hpd_*` exists; the `_A` macros are `PIPE_REG`-indexed |
| `docs/STATE-OF-THE-PROJECT.md` §5.1 | `key()` halts the kernel on the panel path | closed; `key_get()` loop, `check-zlcalls.py` clean |
| `kernel/docs/desktop-look.md` §1 | icons destroyed by nearest-neighbour upscale | fixed, bilinear + native atlas |
| `kernel/docs/display-roadmap.md` STATUS | phases 1–7 "done" | written and register-verified; **40 of their functions have no caller** |

---

## The weakest link in this page

The 59-uncalled figure is a static call-graph count over `kernel/*.{c,h}` and
`freestanding/*.c`. zl resolves builtins **by name at runtime**, so a function
reached only through a string in the builtin dispatcher would be invisible to a
plain grep for `name(`. The dispatcher was searched for the display names it
does register (`panel_up`, `panel_console`, `panel_step`, `intel_*` probes) and
none of the 59 appear there — but that check was by inspection, not by a gate.
A `check-zlcalls.py`-style checker run in the *other* direction (registered
builtin → C definition → is anything reachable) would settle it mechanically.
`check-zlcalls.py` already prints a "registered builtins with no caller" count,
which is most of the machinery needed.

`./build64.sh` was run on this tree and completed:

```
undefined symbols: 0   (0 = no libc, no OS)
size:              2873112 bytes
rc=0
```

It took over five minutes because the box was at load average 9–10 with other
sessions active — that is host load, not a build problem. No QEMU gate
(`verify.sh`, `verify-efi.sh`, `verify-raw.sh`) was run, so nothing here claims
the kernel *boots*; only that it links. The compositor translation-unit set also
compiles clean and renders, and every grep above was run against the working
tree on the stated date.
