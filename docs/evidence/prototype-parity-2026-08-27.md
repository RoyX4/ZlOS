# What the OS does not have that the prototype does

An item-by-item comparison of `docs/design/presswork-prototype.html` — which
`kernel/src/graphics/ui/design.h` names as the source, adding "if this header
and the prototype disagree, THE PROTOTYPE WINS" — against the shipped desktop.

## The register was pointing at the wrong apps

The prototype's rail lists **fourteen** apps. The OS listed twelve, and eleven
of the twelve were different ones.

That looked like a large gap and was not one. **Every app the prototype names
already exists in this OS.** The manifest carries 63 named implementations;
thirteen of the fourteen matched by name outright, and the fourteenth — the
prototype's `13 registers · pipe A` — is this tree's System Info:

| prototype | OS registry id |
|---|---|
| 04 editor · fb.c | Text Editor, 12 |
| 05 kernel log · dmesg | Kernel Log, 40 |
| 06 hex · rd0 s0 | Hex Viewer, 42 |
| 07 calculator · int64 | Calculator, 33 |
| 08 network · enp0s31f6 | Network, 50 |
| 09 clocks · monotonic | Clocks & Timers, 31 |
| 10 system · 0.3 | System, 71 |
| 12 disk usage · zlfs | Disk Usage, 46 |
| 13 registers · pipe A | System Info, 32 |
| 14 type · DejaVu | Type, 72 |

The registry id **is** the app id — `APP_EDIT` is 12 and the manifest's Text
Editor is entry 12 — so the four parallel rail tables now name the prototype's
fourteen directly. The catalogue row survives as row 15: the prototype had
fourteen apps and needed no way to reach a fifteenth, and this OS has 49 more.

### And the launch path did not follow

`rail_launch` had a branch for each of the original apps and then fell through
to `rail_cmd(slot)` — a **fifth** table keyed by register position, which said
"slot 4 is the editor, slot 5 is paint". Under the new register slot 4 is the
kernel log. Clicking it would have opened the editor, and every screenshot of
the rail would have looked perfect.

Deleted rather than updated, and the launch keys off the APP now. The nine
registry apps go through `reg_open(id)`, which is the click path: raise if
already open, otherwise open **with** focus.

`probe-rail.py` is the check. It clicks row 05 specifically, because that is the
row `rail_cmd` used to mis-route, and it asserts the app id out of the kernel's
own `wm:lifecycle v=1 event=open ... app=N` line rather than counting pixels.
That distinction is not theoretical: the first version only counted pixels on
the desk, and it **passed while opening the wrong app**.

## Two differences that must NOT be copied

The prototype is a mock in places, and matching a mock is not parity.

- **`VOL OK`.** The OS reads MUTE, from bits 0–1 of port `0x61` — the PC speaker
  gate, which is literally the only audio output this machine has. It is an
  instrument, not a caption. The prototype's `OK` is invented.
- **Two workspaces.** The OS has three, and they work. Reducing the count to
  match a picture would delete a capability.

This is the same line the earlier parity pass drew when it dropped the
prototype's `LOAD 0.58`: no load average exists, so the figure was not printed.

## Five surfaces the OS does not have at all

The prototype has five things that live above the whole desktop rather than in
a window — and none of them can BE a window, because a window would appear in
the register, take focus, and be listed by the very thing it is drawn over:

| prototype id | what it is | status |
|---|---|---|
| `#palette` `#presults` | command palette, ten rows, live count | **built** |
| `#overview` `#ogrid` | activities — every window as a scaled plate | open |
| `#ctxmenu` | right-click context menu | open |
| `#winmenu` | per-window menu | open |
| `#lock` `#locksheet` | lock screen, knockout header, big clock | open |

`wm.c` gained one hook for them. `wm_overlay()` registers a layer painted after
every window **and** after the toast, inside the same damage rectangle — last
in, last drawn, because a modal a toast can cover is not modal. It is bound
through `wmglue.c` as a weak symbol, so a tree whose `kernel.zl` has not grown
`overlay_draw` simply does not register the layer.

The scrim is `shade`, not a fill: it reads each pixel back and scales it, so
what is behind stays legible as shape while losing its claim on attention. 5/8,
because the prototype's `rgba(0,0,0,.38)` leaves 0.62 and 5/8 is 0.625 — the
nearest eighth, exact in integer maths, in a drawing path with no floats.
