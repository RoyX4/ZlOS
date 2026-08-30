# App parity with the prototype — pick this up cold

Everything needed to continue is here. No chat transcript required.

## What this is

`docs/design/presswork-prototype.html` is the design source — `design.h` names it
and says **"if this header and the prototype disagree, THE PROTOTYPE WINS."** It
carries **fourteen per-app renderers**, ~41,000 characters of window body, named
`R.shell`, `R.files`, `R.mon`, `R.edit`, `R.log`, `R.hex`, `R.calc`, `R.net`,
`R.clock`, `R.sys`, `R.set`, `R.disk`, `R.regs`, `R.type`.

The instruction being worked to: **every app rebuilt to match the HTML exactly,
whether or not it already existed.**

Extract any renderer with:

```bash
python3 - <<'PY'
js=open('docs/design/presswork-prototype.html').read()
i=js.find("R.net = ()")          # <- the one you want
d=0; j=js.index('{', i)
for k in range(j, len(js)):
    if js[k]=='{': d+=1
    elif js[k]=='}':
        d-=1
        if d==0: print(js[i:k+1]); break
PY
```

## The skeleton every body shares

    head line  →  table or cards  →  rule  →  detail well  →  band

## Done — 6 of 14

| app | reference | where | commit |
|---|---|---|---|
| kernel log | `R.log` | `kernel/apps/apps_sys2.zl` `kl_ui` | `65a23df` |
| hex viewer | `R.hex` | `kernel/apps/apps_sys2.zl` `hx_ui` | `77860bc` |
| system info | `R.regs` | `kernel/apps/apps_system.zl` `si_draw` | `9a7d4c4` |
| calculator | `R.calc` | `kernel/apps/apps_system.zl` `ca_draw` | `167b4c7` |
| clocks | `R.clock` | `kernel/apps/apps_system.zl` `clk_draw` | `7f6daf5` |
| network | `R.net` | `kernel/apps/apps_sys3.zl` `s3nw_head` | uncommitted |

## Open — corrected 2026-08-28 after a three-agent read of the prototype

**The previous version of this section was wrong about `R.set`, and wrong in the
direction that costs the most.** It listed `R.set` as open and said it "must be
reconciled against the five panes already in `kernel/src/graphics/ui/settings.c`".
Both halves are false:

- `R.set` was rebuilt in zl and landed in `3b8692b`. `kernel.zl:8359` dispatches
  `APP_SETTINGS` to `settings_body`, with the reference's own four tabs
  (`ladder` / `focus` / `boundary` / `press`) at `set_tab_body`.
- `settings.c`'s draw path is **dead**. `settings_draw` and `settings_event` have
  no caller in the kernel — `grep` finds only `kernel/tests/host/settingstest.c`.
  The single live entry point into that file is `settings_load()`, bound as the
  `set_load` native and called once at boot. Its five panes, sidebar and accent
  chips are dead on screen and live on disk.

Anyone following the old line would have rebuilt a pane that already existed, or
"reconciled" the shipping pane against a file nothing draws. A handoff doc that
says work is open when it is done is not a harmless lag; it is a instruction to
redo it. This section is now written per-app with what was measured, not
per-app with what was remembered.

| Body | State | What is actually missing |
|---|---|---|
| `R.sys` | ~90% | The 14-row contrast table is a structural match and recomputes live. Needs the band status, and one invented caption: it prints "logical, hyperthreaded" as a literal where `cpu_tpc()` is bound and can gate the word. |
| `R.set` | rebuilt, controls thin | 8 of the reference's 12 controls are absent. Present and live: the knockout toggle, the focus-bar slider. Absent: the `surface ladder` and both PRESS segmented controls, the `ui scale` slider, the `occlusion edge` toggle, and six PRESS toggles. Four of those six gate things `kernel.zl` currently draws unconditionally (module grid, crop marks, memory ruler, printer's slug) and are the cheapest group left. |
| `R.mon` | not started | Still the pre-PRESSWORK toolkit. **Its CPU chart was fabricated** — see below. Of fifteen reference figures, five read directly, three need honest re-labelling, and two (per-core CPU percentages, the ten-row process table) have no measurement anywhere in this kernel and must be replaced rather than copied. |
| `R.shell` | transcript only | No `.well` sunken body, no button row, no `exit 0 · 1.42 s`. The transcript is one ink where the reference uses three. |
| `R.edit` | body only | No head row, no gutter, no well, and **no binary branch** — a binary file renders as a field of blank cells. Tab (code 9) is silently discarded, so the reference's `tab 4` would advertise a setting that does not exist. |

## Three defects the audit found, all fixed 2026-08-28

1. **The System Monitor's CPU chart was drawn, not measured.** Seven `line()`
   calls at fixed offsets `-3, -1, -6, -3, -7, -4, -6` — a shape with no data
   behind it, in an instrument, in a system whose entire argument is that its
   instruments are read. It also filled with `TH_ACCENT` where `design.h:659`
   says instruments "fill is ZD_STEEL and never" the overprint. The measurement
   existed the whole time: `wm_sn()`/`wm_sf(i)` expose a 256-deep ring of
   per-frame microseconds, bound to zl all along. Now drawn as the reference's
   spark, scaled against `wm_budget()` rather than against its own maximum — a
   frame-time chart normalised to its own peak always looks the same, which is
   the one thing it must never do.

2. **`wm_frame()` was being used as a value.** `kernel.zl` read the refresh card
   from `wm_frame()`, which is bound as `{ wm_frame(); return zl_nil(); }` — the
   compositor's per-frame *driver*. So the card printed nil as `0`, and drawing
   the System pane re-entered the compositor from inside `app_draw`, draining
   input twice. Now `wm_painted()`, which is the counter.

3. **`rail_win()` was the fifth table keyed by position, and it was stale.** It
   still held the slot order from before the register became fourteen rows, so
   `rail_app(3)` said `APP_EDIT` while `rail_win(3)` returned `browser_win`: the
   browser wore register 04 with the editor's subtitle, the editor wore 05 with
   `dmesg`, and slots 11–13 got no label at all. It is not replaced with a
   corrected table — `rail_cmd` was deleted for being exactly this, and
   `label_windows` carries a comment recording the same class a third time. It
   now *asks*: `rail_app` says which app a slot is, `wm_app()` says which window
   shows it. Add a rail row and the labelling follows with no second edit.

**And one measurement that existed but could not be read.** `wins[win].app_us`
has been measured per window since the two `rdtsc` went into `app_draw_dispatch`,
and no accessor existed in any `.c`, `.h` or `.zl` in the tree. So the settings
pane printed `wm_us()` — the whole compositor frame, every window's work — under
the label `THIS PANE, LAST DRAW`, while the comment directly above that row
asserted it was the per-window figure. `wm_win_us(win)` now exports it. A
measurement that cannot be read is indistinguishable from one never taken.

## How to see an app on screen

Every register app has a shell word, added for exactly this reason — the rail can
open them with a click and the palette names them, but a probe can only type:

    klog dmesg   hexv   calcapp   netapp   clocks   disku   sysinfo
    palette   ctxmenu   activities   lock

```bash
cd kernel
python3 tools/generators/gen-app-manifest.py --write
./build.sh
timeout 800 python3 tools/probes/probe-shot.py -k netapp --settle 2 -o /tmp/x
```

Then crop and LOOK. Do not judge from a full-screen render — five visual misreads
happened that way in one session, and one of them was "corrected" by a
measurement that was itself wrong (counting a neighbouring window's pixels as the
subject's).

## The four zl constraints — each found by hitting it

| constraint | consequence |
|---|---|
| string **literals**, no string **values** | `label == "-"` cannot work. Answer structural questions with integers. This is why `rail_name`/`rail_sub`/`rail_icon`/`rail_app` are four parallel tables, not one table of records. |
| a call must fit **one line** | no wrapped argument lists; hoist to a local first |
| **no floats** in the drawing path | scale in 1/256ths, ratios as integers |
| a native gets **`Value[8]`** | `ui_grid_cell` takes ten args, so align + size + mono are packed into one `mode`: size in the low byte, `0x100` right-aligns, `0x200` selects mono |

## The table vocabulary (new — bind date 2026-08-28)

uikit's table widgets existed since the widget set was written and **nothing
outside C could reach them**. Now bound in `freestanding/runtime_kernel.c`:

    ui_grid("64|76|84|*")      declare tracks, design units, * takes the remainder
    ui_colhead(x,y,w,"a|b|c", -1, 0)   the header — READS the grid's columns
    ui_colhead_h()  ui_growh()         heights
    ui_grow(x,y,w,idx,selected)        a row background
    ui_gcell (x,w,y,h,col,str,rgb,mode)
    ui_gcelln(x,w,y,h,col,num,rgb,mode)
    ui_gspanx(x,w,col)  ui_gspanw(x,w,col)   a track's own left edge / width

**`ui_colhead` iterates the grid's column count.** Without a preceding `ui_grid`
it draws the band and none of the labels — the header appears empty. That cost a
build cycle; do not rediscover it.

## The rule that matters most

**Read a figure, never write one.** Every app so far has had at least one value
the reference invents and this machine genuinely knows, or the reverse. Where the
OS cannot measure what the reference prints, say what is true instead and put the
reason in the source.

Instances already recorded:

- hex bytes: the reference **synthesises** `(off*31 + i*17 + (i&3)*41) & 0xFF`;
  this reads `peek8` at `0x100000`. Its band says "kernel image - live memory".
  Relabelling real RAM as the reference's "rd0 zlfs superblock" would make a
  truthful instrument lie.
- system info: the reference reads the Intel display block, which is **zero under
  QEMU** — the same reason the rail's PWR row shows WARN. It shows cpuid and the
  APIC instead, each with a real decode.
- clocks: source is `PIT, 100 Hz on IRQ0`, what this kernel programs, not the
  reference's `apic timer, 1000.15 Hz`. Third card is `TICKS`, not `process` —
  no per-process accounting exists here.
- calculator: `C` where the reference has `.` — a decimal point in a calculator
  with no floats is a key that would do nothing.
- rail: VOL reads **MUTE** from port `0x61`, the PC speaker gate. The reference's
  `OK` is a mock. And `WS_N` stays 3 because three workspaces work.
- lock sheet: the passphrase field is **empty**. The reference draws eleven
  bullets; nothing is typed here and there is no auth behind it.

**And the one I got wrong:** the network head hardcoded `e1000e` because the
reference said so — this QEMU config runs **virtio-net**, and the driver row two
lines below said exactly that. It reads `s3nw_drv(sel)` now. On hardware where
the guess happened to be right it would have looked correct and still been
invented. Read the value.

## Shell surfaces — all five done earlier

`#palette` `#ctxmenu` `#winmenu` `#overview` `#lock`, on one compositor overlay
layer painted after every window **and** after the toast, under a scrim measured
at exactly 5/8 across the whole screen including the rail. `wm_overlay()` in
`wm.c`; bound through `wmglue.c` as a weak symbol.

**Restore the scissor before drawing an overlay.** The window loop narrows the
clip to each window's frame and then its client and leaves it there. The palette
drew clipped to the Terminal and the System Monitor appeared to be *on top of a
modal*. `wm.c` already did the same restore twelve lines earlier for `sweep_draw`.

## Known-broken, not caused by this work

`grub-bios64` stalls after `keyboard on IRQ1`. Proven pre-existing by an A/B
against `7d1a11b` in a detached worktree — byte-identical failure signature.
`tools/preflight.sh` does not run `verify-64.sh`, which is why it rotted. See
`docs/evidence/grub-bios64-unwatched-2026-08-27.md`, and **do not** "fix" the
visibility by adding the gate to preflight: that blocks every push without fixing
the boot.

QEMU segfaults on this box — five times in one day, always at binary offset
`6ee234` reading `0x10`, only under `verify-efi.sh`. The gates now say
`CRASH QEMU ITSELF crashed` instead of blaming the kernel. See
`docs/evidence/qemu-segfaults-2026-08-27.md`.
