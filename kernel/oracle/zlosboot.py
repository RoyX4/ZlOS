#!/usr/bin/env python3
"""zlosboot.py - boot zlOS at a chosen size and open a named app.

The shared half of shot-zlos.py and animate.py. It is a MODULE and not a
`probe-*.py` because a module name with a hyphen in it cannot be imported -
which is why exercise.py's own header explains that probe-term.py and
probe-net.py each carry a private copy of the same twenty lines. One copy, in a
file whose name Python will accept.

Nothing in here sleeps for a boot or for a command. Every wait polls the serial
log for a marker the guest itself printed, per the project rule that a gate
must never be timing-sensitive.
"""
import os, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
KERNEL = os.path.dirname(HERE)
sys.path.insert(0, KERNEL)
from exercise import Serial, Qmp, qemu_argv, build, PROMPT  # noqa: E402,F401

OUT = os.path.join(HERE, "out")

# Design-unit constants for the catalog path. Copied from apps_registry.zl
# rather than re-derived, deliberately and for the reason probe-catalog.py
# gives: a layout change THERE should be what breaks this, instead of
# independent arithmetic here drifting quietly away from it.
TITLE_H = 28
MENU_ROWS = 10
# The dock's TOP, measured from the bottom of the screen: kernel.zl's
# DOCK_H (43) + DOCK_BOT (9). It was 64, from the 56px slab the reference's
# 43px pill replaced - so every dock click landed 12px above the strip.
DOCK_H = 52
CAT_HEADER = 26
CAT_TILE_W = 130
CAT_TILE_H = 108
REG_FIRST = 14                 # apps_registry.zl:59 - catalog index i is id 14+i

# word -> the open_app() code it dispatches to. These seven are every code
# kernel.zl's open_app() handles; everything else in the suite exists only
# behind the catalog.
WORD_APPS = {
    "files": 108, "edit": 105, "snake": 103, "paint": 100,
    "cube": 118, "anim": 97, "mouse": 120,
}


class LoggedSerial(Serial):
    """Serial that also keeps everything it ever received.

    THIS IS NOT A CONVENIENCE. Serial.wait() CONSUMES the buffer up to and
    including the marker it matched, so by the time the boot has been waited
    for, every `wm: win N title x,y WxH` line the compositor printed on the way
    up is gone. probe-drag.py documents the same trap from the other side -
    waiting for `compositor:` a second time burns the whole timeout and then
    falsely reports that no rectangles were printed - and animate.py hit it
    exactly once: "the compositor reported no window title rects" on a boot
    that had printed several.

    pump() is the only place bytes enter the buffer, so overriding it captures
    everything regardless of who consumes what afterwards.
    """

    def __init__(self, path):
        super().__init__(path)
        self.all = ""

    def pump(self):
        n = len(self.buf)
        r = super().pump()
        if len(self.buf) > n:
            self.all += self.buf[n:]
        return r


def guest_ui(width):
    """What ui() returns in the guest at this width. fb.c:726-729, exactly.

    NOT a width threshold. probe-catalog.py uses `2 if W >= 1400 else 1`, which
    disagrees with fb.c for every mode from 1400 to 2879 - including 1920x1200,
    the mode zlOS actually boots into. Reproducing the real formula costs three
    lines and removes a whole class of clicks landing nowhere.
    """
    q8 = (width * 256 + 960) // 1920
    q8 = max(256, min(768, q8))
    return (q8 + 128) // 256


def catalog_apps():
    """{display name: catalog index} parsed out of apps_registry.zl.

    Parsed rather than transcribed so a new app becomes available to the oracle
    with no edit here, and a renamed one fails loudly instead of silently
    clicking the tile next to it.
    """
    src = open(os.path.join(KERNEL, "apps_registry.zl")).read()
    ids = {m.group(1): int(m.group(2))
           for m in re.finditer(r"^(APP_\w+)\s*=\s*(\d+)", src, re.M)}
    # ONLY reg_name's body. reg_mono() has the identical `if id == APP_X {
    # return "..." }` shape and returns two-letter monograms, so an unscoped
    # scan offers "CB" and "TT" as app names beside "Clipboard" and
    # "Tic-Tac-Toe" - half of them ambiguous, all of them wrong to type.
    body = re.search(r"fn reg_name\(id\)\s*\{(.*?)\n\}", src, re.S)
    if not body:
        raise SystemExit("apps_registry.zl: reg_name() not found")
    names = {}
    for m in re.finditer(r"if id == (APP_\w+)\s*\{\s*return \"([^\"]+)\"",
                         body.group(1)):
        if m.group(1) in ids:
            names.setdefault(m.group(2), ids[m.group(1)])
    # ...and the per-slice tables. reg_name() only answers for the ids IT lists;
    # everything from 40 up is answered by a slice file, so a parser reading
    # apps_registry.zl alone knows about none of the ~27 apps added there and
    # calls every one of them an unknown --app - which is most of the reason
    # only three of 62 regions can be photographed with their window open.
    # Slices spell the id as a NUMBER rather than an APP_* constant, which is
    # why this is a second pass and not a wider regex.
    import glob as _glob
    for path in sorted(_glob.glob(os.path.join(KERNEL, "apps_*.zl"))):
        sl = open(path, encoding="utf-8", errors="replace").read()
        # A slice defines its own APP_* constants in its own file, so resolve
        # against the union rather than against apps_registry.zl's set only.
        sids = dict(ids)
        # NOT just APP_*: the game slices name theirs G3_TETRIS, G4_REVERSI and
        # so on. Match any top-level UPPERCASE constant assigned a plain
        # integer, which is what every slice's id table actually looks like.
        sids.update({m.group(1): int(m.group(2))
                     for m in re.finditer(r"^([A-Z][A-Z0-9_]*)\s*=\s*(\d+)\s*(?:#|$)",
                                          sl, re.M)})
        for fn in re.finditer(r"fn \w+_name\(id\)\s*\{(.*?)\n\}", sl, re.S):
            for m in re.finditer(r'if id == (\w+)\s*\{\s*return "([^"]+)"', fn.group(1)):
                key = m.group(1)
                val = int(key) if key.isdigit() else sids.get(key)
                if val is not None:
                    names.setdefault(m.group(2), val)
    # THE CATALOG INDEX IS DENSE NOW, not id - REG_FIRST.
    #
    # REG_COUNT is gone: the id space was carved into reserved per-slice ranges
    # with holes in it (34..39 unallocated, and one spare id at the end of each
    # slice), so the catalog walks a dense list built by reg_exists() and a tile
    # index no longer equals an id offset. Reading REG_COUNT and assuming
    # contiguity would click the wrong tile - silently, and for every app past
    # the first hole.
    #
    # So mirror reg_exists() by parsing the gaps it names, rather than
    # hardcoding them here where they would go stale independently.
    first = re.search(r"^REG_FIRST\s*=\s*(\d+)", src, re.M)
    last = re.search(r"^REG_LAST\s*=\s*(\d+)", src, re.M)
    if not first or not last:
        raise SystemExit("apps_registry.zl: REG_FIRST/REG_LAST not found")
    base, top = int(first.group(1)), int(last.group(1))
    if base != REG_FIRST:
        raise SystemExit(f"apps_registry.zl REG_FIRST is {base}, this module "
                         f"was written for {REG_FIRST} - re-check cat_hit()")
    ex = re.search(r"fn reg_exists\(id\) \{(.*?)\n\}", src, re.S)
    if not ex:
        raise SystemExit("apps_registry.zl: reg_exists() not found")
    holes = set()
    for m in re.finditer(r"if id >= (\d+) \{ if id <= (\d+) \{ return 0", ex.group(1)):
        holes.update(range(int(m.group(1)), int(m.group(2)) + 1))
    for m in re.finditer(r"if id == (\d+) \{ return 0", ex.group(1)):
        holes.add(int(m.group(1)))
    dense = [i for i in range(base, top + 1) if i not in holes]
    index = {i: k for k, i in enumerate(dense)}
    return {name: index[i] for name, i in names.items() if i in index}


# ---- pointer ----------------------------------------------------------------
def at(qmp, x, y, w, h, btn=None):
    """Absolute pointer events - zlOS prefers the usb-tablet when it is there,
    and relative deltas sent to an absolute device move it exactly zero pixels
    while looking, from outside, like a dead pointer."""
    ev = [{"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / w)}},
          {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / h)}}]
    if btn is not None:
        ev.append({"type": "btn", "data": {"down": btn, "button": "left"}})
    qmp.cmd("input-send-event", events=ev)


def click(qmp, x, y, w, h, settle):
    at(qmp, x, y, w, h, btn=True)
    settle(0.3)
    at(qmp, x, y, w, h, btn=False)
    settle(0.4)


# ---- screen -----------------------------------------------------------------
def size_of(qmp, tmp):
    from PIL import Image
    p = os.path.join(tmp, "probe.ppm")
    if not qmp.screendump(p):
        raise SystemExit("screendump failed - QEMU is not giving us a frame")
    with Image.open(p) as im:
        return im.size


def grab(qmp, tmp, name="frame"):
    """One frame, as an HxWx3 uint8 array."""
    import numpy as np
    from PIL import Image
    p = os.path.join(tmp, name + ".ppm")
    if not qmp.screendump(p):
        raise SystemExit("screendump failed")
    with Image.open(p) as im:
        return np.asarray(im.convert("RGB"), dtype=np.uint8)


def type_line(ser, cmd, ceiling):
    """Type one shell command and prove the shell TOOK it.

    term.c echoes a submitted line to serial inside its Enter branch and
    nowhere else, so the echo is the exact marker for "this line was
    submitted". Without waiting for it, a picture can be of a frame no command
    ever ran in - the documented way probe-shot.py once lied.
    """
    ser.send(cmd + "\r")
    ok, out = ser.wait(cmd + "\n", ceiling)
    if not ok:
        raise SystemExit(f"the shell never echoed {cmd!r} - the line was not "
                         f"submitted, so anything measured after it would be a "
                         f"frame no command ran in")
    return out


def reach_size(ser, qmp, tmp, want_w, want_h, ceiling):
    """Get the guest to `want_w x want_h`, or say precisely why it cannot.

    zlOS does not boot at 1280x800 and cannot be made to by asking the
    bootloader: GRUB's list (mkiso.sh:32) starts at 1280x720 and kernel.zl:4541
    then re-modesets ANYTHING under 1900 wide up to 1920x1200 with its own
    driver. The only runtime lever is the shell's `mode` command
    (term.c:181 -> code 110, kernel.zl:1688), which TOGGLES:

        nw = 1280; nh = 800;  if px_w() == 1280 { nw = 1920; nh = 1200 }

    So: measure, toggle, measure again - at most twice, because a toggle with
    period 2 that has not landed after two presses never will.
    """
    for attempt in range(3):
        w, h = size_of(qmp, tmp)
        if (w, h) == (want_w, want_h):
            return w, h, attempt
        if attempt == 2:
            raise SystemExit(
                f"zlOS is at {w}x{h} and will not go to {want_w}x{want_h}.\n"
                f"  The only runtime lever is the shell's `mode` command and it\n"
                f"  toggles between exactly two modes (kernel.zl:1688):\n"
                f"      nw = 1280  nh = 800   if px_w() == 1280 {{ nw = 1920  nh = 1200 }}\n"
                f"  Any other size needs that ladder changed, or a variant source\n"
                f"  booted with ZLOS_SRC. Nothing here will fake it by scaling.")
        out = type_line(ser, "mode", ceiling)
        got, more = ser.wait("modesetting with our own driver", ceiling)
        out += more
        if not got:
            raise SystemExit("`mode` was submitted but printed nothing - the "
                             "command did not run")
        # its own confirmation line ("now W x H, pitch N"), not a wall clock
        got, more = ser.wait("pitch", ceiling)
        out += more
        if not got:
            if "the card refused that mode" in out:
                raise SystemExit(f"the card refused the mode change:\n{out[-400:]}")
            raise SystemExit(f"`mode` neither confirmed nor refused:\n{out[-400:]}")
        ser.drain(1.5)               # let the full-screen damage repaint
    raise SystemExit("unreachable")


# ---- opening an app ---------------------------------------------------------
# ---- booting AT a size, rather than switching to it afterwards ---------------
# The ladder kernel.zl runs at boot, matched with flexible whitespace so that
# re-indenting kernel.zl does not silently turn this into a no-op. It is
# asserted to match EXACTLY ONCE.
LADDER = re.compile(
    r"if px_w\(\)\s*>\s*0\s*\{\s*"
    r"if px_w\(\)\s*<\s*1900\s*\{\s*"
    r"if set_res\(1920,\s*1200\)\s*==\s*0\s*\{\s*"
    r"if set_res\(1920,\s*1080\)\s*==\s*0\s*\{\s*"
    r"if set_res\(1600,\s*900\)\s*==\s*0\s*\{\s*"
    r"set_res\(1280,\s*800\)\s*"
    r"\}\s*\}\s*\}\s*\}\s*\}")


def variant_source(want_w, want_h, dest):
    """kernel.zl with its boot modeset ladder replaced by one exact mode.

    WHY THIS EXISTS, measured on this branch 2026-08-20:

    The other route to 1280x800 is the shell's `mode` command, and it works -
    the framebuffer really does become 1280x800 and the kernel says so:

        fb: 1280x800x32 cell 8x16 ui 1x, back ON  (4000 KiB/mode)
        now 1280 x 800, pitch 5120

    ...and the DESKTOP does not follow it. The screenshot from that path shows
    windows still at their 1920x1200 positions (System Monitor half off the
    right edge, the Browser hanging off the bottom), the wallpaper repainted as
    flat black, and no dock on screen at all. layout() recomputes the variables
    the boot placement reads; it does not move windows the compositor has
    already opened. So a picture taken that way is a 1920x1200 desktop cropped
    to 1280x800, and scoring it against the mockup would blame the design for a
    relayout bug.

    Booting AT the size is a different thing and it is the honest one: layout()
    runs once, at 1280x800, before a single window is opened.

    This is the mechanism mkiso.sh documents (ZLOS_SRC) and probe-drag.py
    already uses for its 2560x1440 case - "boot a VARIANT kernel, one that asks
    for a different resolution, without editing the tracked kernel.zl, which
    another session may be part-way through".
    """
    src = open(os.path.join(KERNEL, "kernel.zl")).read()
    hits = LADDER.findall(src)
    if len(hits) != 1:
        raise SystemExit(
            f"kernel.zl's boot modeset ladder matched {len(hits)} times, "
            f"expected 1. It has been edited; re-read kernel.zl around "
            f"`if px_w() < 1900` and update LADDER in zlosboot.py rather than "
            f"booting a kernel this has silently failed to patch.")
    patched = LADDER.sub(f"if px_w() > 0 {{\n        set_res({want_w}, {want_h})\n    }}",
                         src)
    if patched == src:
        raise SystemExit("the ladder substitution changed nothing")
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    with open(dest, "w") as fh:
        fh.write(patched)
    return dest


def win_count(log):
    return len(re.findall(r"wm: win \d+ title", log))


def open_by_catalog(ser, qmp, idx, W, H, name):
    """Start button -> All Applications -> the tile at `idx`.

    The same three clicks probe-catalog.py makes, with two differences: ui
    comes from fb.c's real formula rather than a width threshold, and the tile
    arithmetic is done in the catalog's OWN reported client rect, so a catalog
    that opened somewhere unexpected fails here instead of silently sending the
    third click into the wallpaper.
    """
    u = guest_ui(W)
    settle = ser.drain
    dock_y = H - DOCK_H * u

    # THE DOCK'S GRID BUTTON, not a start button. The full-width bar that
    # carried one was replaced by the reference's floating island, and the
    # route to the catalog is now "Show Applications" at the end of the dock -
    # past the divider, which is why this is not simply slot 0's x.
    # Mirrors kernel.zl's dock_slot_at(): dock_x0() + DOCK_N*PITCH + GAP + 1.
    DOCK_PADX, DOCK_GAP, DOCK_TW, DOCK_PITCH, DOCK_N = 7, 5, 33, 38, 9
    DOCK_BAR_W = DOCK_PADX * 2 + DOCK_N * DOCK_TW + (DOCK_N - 1) * DOCK_GAP \
                 + DOCK_GAP * 2 + 1 + DOCK_TW
    bar_x = (W - DOCK_BAR_W * u) // 2
    grid_x = bar_x + DOCK_PADX * u + DOCK_N * DOCK_PITCH * u + DOCK_GAP * u + 1
    click(qmp, grid_x + DOCK_TW * u // 2, dock_y + 22 * u, W, H, settle)
    settle(1.0)

    # ONE CLICK, not two. The grid button opens the catalog directly; the

    # second click here used to pick "All Applications" out of the start

    # menu, and against the new chrome it lands on the desktop and

    # dismisses what the first click just opened.
    settle(1.5)
    # ser.all, never ser.buf: wait() consumes, so the buffer holds only
    # whatever arrived since the last match. The full transcript is the only
    # place the catalog's own report is guaranteed to still be.
    rows = re.findall(r"wm: win (\d+) title \d+,\d+ \d+x\d+ "
                      r"client (\d+),(\d+) (\d+)x(\d+)", ser.all)
    if not rows:
        raise SystemExit("'All Applications' did not open a window - the "
                         "catalog is not up, so a tile click would land on "
                         "whatever is underneath it")
    _, cx, cy, cw, _ = (int(v) for v in rows[-1])

    cols = max(1, cw // (CAT_TILE_W * u))
    col, row = idx % cols, idx // cols
    tx = cx + col * CAT_TILE_W * u + CAT_TILE_W * u // 2
    ty = cy + CAT_HEADER * u + row * CAT_TILE_H * u + CAT_TILE_H * u // 2
    print(f"  catalog tile {idx} ({name}) at {tx},{ty} "
          f"(catalog client {cx},{cy}, {cw}px wide, {cols} cols, ui {u}x)")
    click(qmp, tx, ty, W, H, settle)
    settle(2.0)


def open_app(ser, qmp, name, W, H, ceiling, cat=None):
    """Open `name`, and PROVE a window appeared. Returns the new window count.

    The proof is the compositor's own `wm: win N title ...` report rather than
    a pixel delta: a delta cannot tell "the app opened" from "a tooltip
    appeared", and an app that refuses to open leaves the desktop looking
    exactly like a desktop.
    """
    cat = cat if cat is not None else catalog_apps()
    ser.drain(0.5)
    before = win_count(ser.all)
    if name in WORD_APPS:
        print(f"opening {name!r} by shell word")
        type_line(ser, name, ceiling)
        ser.drain(2.0)
    elif name in cat:
        print(f"opening {name!r} through the catalog")
        open_by_catalog(ser, qmp, cat[name], W, H, name)
    else:
        near = [k for k in list(WORD_APPS) + list(cat) if name.lower() in k.lower()]
        raise SystemExit(f"unknown app {name!r}"
                         + (f" - did you mean {near}?" if near else ""))
    ser.drain(0.5)
    after = win_count(ser.all)
    if after <= before:
        raise SystemExit(f"no new window was reported ({before} -> {after}). "
                         f"{name!r} did not open, so anything measured after "
                         f"this would be of a desktop it is not on.")
    print(f"  {after - before} new window(s) reported by the compositor")
    return after


class Machine:
    """A booted zlOS at a known size. Use as a context manager.

        with Machine(1280, 800) as m:
            m.qmp, m.ser, m.tmp, m.w, m.h
    """

    def __init__(self, width=1280, height=800, do_build=True,
                 boot_timeout=300.0, cmd_timeout=60.0, tablet=True, how="src"):
        self.want = (width, height)
        self.do_build = do_build
        self.boot_timeout = boot_timeout
        self.cmd_timeout = cmd_timeout
        self.tablet = tablet
        if how not in ("src", "toggle"):
            raise SystemExit("how must be 'src' or 'toggle'")
        self.how = how
        self.proc = None

    def __enter__(self):
        if self.how == "src":
            # Boot AT the size. See variant_source() for the measured reason
            # the `toggle` route produces a desktop that is laid out for
            # another resolution.
            src = variant_source(self.want[0], self.want[1],
                                 os.path.join(OUT, "kernel-%dx%d.zl" % self.want))
            os.environ["ZLOS_SRC"] = os.path.abspath(src)
            # ...and ask GRUB for it too, so the kernel's own set_res is a
            # confirmation rather than the only thing standing between the
            # desktop and a 1280x720 layout.
            os.environ["ZLOS_GFXMODE"] = f"{self.want[0]}x{self.want[1]},auto"
            print(f"booting a variant source: kernel.zl with its modeset ladder "
                  f"replaced by set_res({self.want[0]}, {self.want[1]})")
            if not self.do_build:
                print("  NOTE: --no-build, so this boots whatever zlOS.iso "
                      "already contains. If the last build was not this "
                      "variant, the size check below will catch it.")
        if self.do_build:
            build(False)
        self.tmp = tempfile.mkdtemp(prefix="zlos-oracle-")
        ser_path = os.path.join(self.tmp, "ser")
        qmp_path = os.path.join(self.tmp, "qmp")
        self.proc = subprocess.Popen(
            qemu_argv(self.tmp, False, ser_path, qmp_path, tablet=self.tablet),
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        self.ser, self.qmp = LoggedSerial(ser_path), Qmp(qmp_path)
        ok, log = self.ser.wait("ready.", self.boot_timeout)
        if not ok:
            raise SystemExit("never booted. serial so far:\n" + log[-2000:])
        if not self.ser.wait(PROMPT, 60)[0]:
            raise SystemExit("booted but no shell prompt")
        self.boot_size = size_of(self.qmp, self.tmp)
        print(f"booted at {self.boot_size[0]}x{self.boot_size[1]}")
        if self.how == "src":
            if self.boot_size != self.want:
                raise SystemExit(
                    f"the variant kernel was told to set_res{self.want} and "
                    f"came up {self.boot_size[0]}x{self.boot_size[1]} - the "
                    f"card refused the mode. Refusing to fall back to the "
                    f"`mode` toggle, which lays the desktop out for the wrong "
                    f"size (see variant_source).")
            self.w, self.h = self.boot_size
        else:
            self.w, self.h, toggles = reach_size(
                self.ser, self.qmp, self.tmp, self.want[0], self.want[1],
                self.cmd_timeout)
            print(f"at {self.w}x{self.h}"
                  + (f" after {toggles} `mode` toggle(s)" if toggles
                     else " with no mode change"))
        return self

    def __exit__(self, *exc):
        if self.proc is not None:
            self.proc.kill()
            self.proc.wait()
        return False
