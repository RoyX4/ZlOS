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

## Open — 8 bodies

`R.shell`, `R.files`, `R.edit`, `R.mon`, `R.disk`, `R.type`, `R.sys`, and
**`R.set` at 21,637 characters** — larger than the other thirteen combined, and
it must be reconciled against the five panes already in
`kernel/src/graphics/ui/settings.c`.

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
