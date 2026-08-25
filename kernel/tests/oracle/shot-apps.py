#!/usr/bin/env python3
"""shot-apps.py - photograph EVERY app in ONE boot, and score each window.

  ./shot-apps.py                     every app the catalog lists
  ./shot-apps.py --only calc,term    just these (reference ids)
  ./shot-apps.py --limit 5           the first five, for a smoke run
  ./shot-apps.py --no-build          boot whatever zlOS.iso already holds

Output, per app <id>:
  out/zlos-app-<id>.png              the whole 1280x800 screen
  out/win-<id>.json                  the region map for THIS app's window
  out/app-shots.json                 the manifest: every rect, every failure


WHY ONE BOOT AND NOT FIFTY-THREE
--------------------------------
shot-zlos.py --app X builds, boots, opens one app and dies: about two minutes
each, so the 53-app sweep it was meant to produce is nearly two hours of QEMU
on a box where one core is already the bottleneck. Nothing in that two minutes
is per-app except the last eight seconds.

So this boots once and drives the catalog like a person would: the catalog
stays open for the whole run, each tile opens its app ON TOP of it, the frame
is photographed, and Ctrl+W closes the app and hands focus back to the catalog.
Fifty-three apps cost one boot plus about eight seconds each.

Ctrl+W and not a click on the close box, deliberately: the close box is
arithmetic over a rectangle this script would have to re-derive from wm.c's
title_control_rect, and getting it wrong lands the click in the app instead -
silently, since a click that hits nothing looks exactly like a click that hit
the wrong thing. wm.c routes EV_CHAR 23 to `wm_close(focus_win)`, and the app
that just opened IS the focus window; there is no geometry to get wrong.


WHY THE SHOT IS SCORED OVER THE WINDOW AND NOT THE SCREEN
---------------------------------------------------------
The reference's own per-app render (refrender/out/app-<id>.png, produced by
render-ref.mjs --app) shows ONE window, alone, CENTRED. zlOS cannot be put in
that state from here: the shell reads its commands off the serial line and
wm.c routes serial bytes to the FOCUS WINDOW, so closing or hiding the Terminal
is closing the only thing that can be typed at. Every zlOS shot therefore has
the boot desktop - Terminal, Files, System Monitor - and the catalog behind the
app.

Scoring those two whole screens against each other measures the composition
difference and drowns the app in it. So the comparison is per WINDOW: the app's
own rectangle in each image, which is the same SIZE in both because
apps_registry.zl's reg_w/reg_h are the reference's own numbers. That is the
number "how faithful is this app" actually asks for, and diff-regions.py grew
one option (--zlos-regions) to express it: same region ids, same rect sizes,
different origins, because the two renders put the same window in different
places.
"""
import argparse, json, os, re, sys, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from zlosboot import (Machine, OUT, KERNEL, HERE, catalog_apps, guest_ui,  # noqa: E402
                      at, click, wheel, type_line, WIN_REPORT, CAT_HEADER,
                      CAT_TILE_W, CAT_TILE_H)

REF_OUT = os.path.join(KERNEL, "tests", "refrender", "out")
DS = os.path.join(KERNEL, "tests", "refrender", "ds.html")

# ds.html:1214's own constants, used by render-ref.mjs to centre an --app shot.
# Repeated here rather than re-derived: they are what decides where the window
# in app-<id>.png actually is, and a different guess here would compare the
# reference's wallpaper against zlOS's window.
REF_TOP, REF_DOCK, REF_W, REF_H = 48, 62, 1280, 800

# kernel.zl:267 and ui.c:147. The compositor reports a window's CLIENT rect
# (wm_report), and the frame is the client grown by the title bar above it and
# the 2px border either side - client_of() in wm.c, read in reverse.
TITLE_H, BORDER = 36, 2


def ref_apps():
    """[(id, name, w, h)] for every app the reference registers.

    Parsed out of ds.html's three tables rather than transcribed, for the same
    reason zlosboot.catalog_apps() parses apps_registry.zl: a renamed app must
    fail loudly here instead of silently pairing with the wrong picture.
    """
    src = open(DS, encoding="utf-8", errors="replace").read()
    out = []
    # GAME_APPS / UTILS: ['id','Name',...,w,h,'ico'] rows fed through a .map()
    # that prefixes the id. The prefix is in the .map(), so it is read from
    # there rather than assumed.
    for block, pref, dw, dh in (("GAME_APPS", "g_", None, None),
                                ("UTILS", "u_", 470, 352)):
        m = re.search(r"const %s = \[(.*?)\n\]\.map" % block, src, re.S)
        if not m:
            raise SystemExit(f"ds.html: could not find the {block} table")
        for row in re.finditer(r"\[([^\[\]]+)\]", m.group(1)):
            f = [s.strip() for s in row.group(1).split(",")]
            ident = f[0].strip("'\"")
            name = f[1].strip("'\"")
            if dw is not None:
                out.append((pref + ident, name, dw, dh))
            else:
                out.append((pref + ident, name, int(f[3]), int(f[4])))
    # APPS: one object literal per line, each with its own w/h.
    m = re.search(r"const APPS = \[(.*?)\n\];", src, re.S)
    if not m:
        raise SystemExit("ds.html: could not find the APPS table")
    for row in re.finditer(r"\{id:'([^']+)',\s*name:'([^']+)'.*?w:(\d+),\s*h:(\d+)",
                           m.group(1)):
        out.append((row.group(1), row.group(2), int(row.group(3)), int(row.group(4))))
    return out


def ref_rect(w, h):
    """Where render-ref.mjs --app puts a window of this size. buildPatch(), exactly."""
    x = max(0, round((REF_W - w) / 2))
    y = max(REF_TOP, round(REF_TOP + (REF_H - REF_TOP - REF_DOCK - h) / 2))
    return [int(x), int(y), int(w), int(h)]


def frame_of(cx, cy, cw, ch, u):
    """The FRAME rect, from the client rect the compositor reported.

    wm.c's client_of(): x+2, y+title_h, w-4, h-title_h-2. Inverted here, so the
    rectangle compared is the window a person sees - title bar, border, and all
    - which is what the reference's `pos` box is too.
    """
    th = TITLE_H * u
    b = BORDER
    return [cx - b, cy - th, cw + 2 * b, ch + th + b]


def region_doc(rid, rect, note):
    """A one-region map in diff-regions.py's own schema."""
    return {
        "schema": 1,
        "note": note,
        "generated_by": "kernel/tests/oracle/shot-apps.py",
        "source": {"file": DS, "bytes": 0, "sha256": "per-app window map"},
        "screen": {"w": REF_W, "h": REF_H},
        "regions": [{"id": rid, "kind": "window", "ws": None, "visible": True,
                     "rect": rect,
                     "derivation": "EXACT. " + note}],
    }


def open_catalog(m, ceiling):
    """`apps`, once. Returns the catalog's client rect."""
    seen = len(m.ser.all)
    type_line(m.ser, "apps", ceiling)
    if not m.ser.wait("wm: win ", ceiling)[0]:
        raise SystemExit("`apps` opened no window. Serial tail:\n" + m.ser.all[-800:])
    m.ser.drain(1.0)
    rows = WIN_REPORT.findall(m.ser.all[seen:])
    if not rows:
        raise SystemExit("the catalog opened but reported no rectangle - "
                         "WIN_REPORT is stale against wm_report()")
    return tuple(int(v) for v in rows[-1][1:])


def tile_click(m, cat, idx, u):
    """Scroll the catalog to tile `idx` and click it. Returns (tx, ty, scroll)."""
    cx, cy, cw, ch = cat
    cols = max(1, cw // (CAT_TILE_W * u))
    vis = max(1, (ch - CAT_HEADER * u) // (CAT_TILE_H * u))
    col, row = idx % cols, idx // cols
    # PARK THE POINTER IN THE CATALOG FIRST. route_wheel delivers to the window
    # under the pointer and deliberately does not focus it, so a wheel sent
    # with the pointer anywhere else scrolls something else or nothing.
    at(m.qmp, cx + cw // 2, cy + ch // 2, m.w, m.h)
    m.ser.drain(0.2)
    # RESET, then scroll. cat_scroll is a global in apps_registry.zl that
    # survives the catalog being raised, lowered and covered, so "wheel down N"
    # is only correct from a known zero. cat_event clamps at 0, so over-
    # scrolling up is the reset and needs no state here.
    wheel(m.qmp, 40)
    m.ser.drain(0.4)
    scroll = max(0, row - vis + 1)
    if scroll:
        wheel(m.qmp, -scroll)
        m.ser.drain(0.6)
    tx = cx + col * CAT_TILE_W * u + CAT_TILE_W * u // 2
    ty = cy + CAT_HEADER * u + (row - scroll) * CAT_TILE_H * u + CAT_TILE_H * u // 2
    click(m.qmp, tx, ty, m.w, m.h, m.ser.drain)
    return tx, ty, scroll


# The six apps the catalog does NOT list, and how each is really reached.
# apps_registry.zl only owns ids from REG_FIRST (14) up; Terminal, Files,
# System Monitor, Text Editor, Snake and Settings are all below it and predate
# it, so no tile exists for any of them. Three are already open at boot and
# three are shell words - except Settings, which is a dock tile and nothing
# else, and is therefore the one app in the reference this script cannot reach.
#
# `files` is NOT here even though "files" is a shell word: open_app(108) finds
# the boot Files window already open, raises it and returns WITHOUT calling
# wm_report, so there is no rectangle on the serial line to photograph against.
# It is one of the three that come from the desktop shot instead.
WORD_OF = {"edit": "edit", "g_snake": "snake"}
BOOT_OPEN = {"term", "mon", "files"}


def close_box(cx, cy, cw, u):
    """Where the close box's CENTRE is, from the reported client rect.

    wm.c's title_control_rect() for TITLE_CLOSE, composed with client_of()
    inverted. Used only for windows opened by a shell WORD: open_app() restores
    focus to the shell afterwards (deliberately - you typed the word mid-command
    line), so Ctrl+W there would close the Terminal and take the serial console
    with it. Everything opened from the catalog is focused and uses Ctrl+W.
    """
    cs, gap, s2, th = 26 * u, 6 * u, 8 * u, TITLE_H * u
    return cx + cw - (s2 + cs) + cs // 2 + BORDER, cy - th + (th - cs) // 2 + cs // 2


def shoot(m, path):
    from PIL import Image
    ppm = os.path.join(m.tmp, "shot.ppm")
    if not m.qmp.screendump(ppm):
        return None
    with Image.open(ppm) as im:
        im = im.convert("RGB")
        if im.size != (m.w, m.h):
            return None
        im.save(path)
    return path


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="", help="comma-separated reference ids")
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--settle", type=float, default=1.2)
    ap.add_argument("--cmd-timeout", type=float, default=60.0)
    ap.add_argument("--boot-timeout", type=float, default=300.0)
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()

    cat_idx = catalog_apps()                       # zlOS name -> tile index
    refs = ref_apps()                              # reference id, name, w, h
    want = {s.strip() for s in args.only.split(",") if s.strip()}

    todo, words, unmatched = [], [], []
    for rid, name, w, h in refs:
        if want and rid not in want:
            continue
        if not os.path.exists(os.path.join(REF_OUT, f"app-{rid}.png")):
            unmatched.append((rid, name + " - no reference render exists"))
            continue
        if rid in WORD_OF:
            words.append((rid, name, WORD_OF[rid], w, h))
        elif name in cat_idx:
            todo.append((rid, name, cat_idx[name], w, h))
        elif rid in BOOT_OPEN:
            unmatched.append((rid, name + " - open at boot; scored from the "
                                     "desktop shot against regions.json"))
        else:
            unmatched.append((rid, name + " - no catalog tile and no shell "
                                     "word (dock tile only)"))
    todo.sort(key=lambda t: t[2])
    if args.limit:
        todo = todo[:args.limit]
        words = words[:args.limit]

    print(f"{len(todo)} catalog app(s) + {len(words)} shell-word app(s), "
          f"{len(unmatched)} not reachable this way")
    for rid, why in unmatched:
        print(f"  skipped: {rid:<10} {why}")
    if not todo and not words:
        return 1

    os.makedirs(OUT, exist_ok=True)
    manifest = {"screen": [REF_W, REF_H], "apps": {}, "unmatched": dict(unmatched)}
    t0 = time.time()
    total = len(todo) + len(words)
    with Machine(REF_W, REF_H, do_build=not args.no_build,
                 boot_timeout=args.boot_timeout,
                 cmd_timeout=args.cmd_timeout, how="src") as m:
        u = guest_ui(m.w)
        n = [0]

        def capture(rid, name, rw, rh, seen, extra):
            """One app: read its rectangle off the serial log, photograph, record."""
            n[0] += 1
            tag = f"[{n[0]}/{total}] {rid:<12}"
            m.ser.drain(args.settle)
            rows = WIN_REPORT.findall(m.ser.all[seen:])
            if not rows:
                print(f"{tag} NO WINDOW REPORTED  {extra}")
                manifest["apps"][rid] = dict(error="no window reported", **extra)
                return None
            _, cx, cy, cw, ch = (int(v) for v in rows[-1])
            fr = frame_of(cx, cy, cw, ch, u)
            png = os.path.join(OUT, f"zlos-app-{rid}.png")
            if shoot(m, png) is None:
                print(f"{tag} SCREENDUMP FAILED")
                manifest["apps"][rid] = dict(error="screendump failed", **extra)
                return None
            rr = ref_rect(rw, rh)
            # THE TWO RECTANGLES MUST BE THE SAME SIZE or the crops cannot be
            # compared at all. They are, by construction - reg_w/reg_h are the
            # reference's own numbers - so a mismatch is a real defect in the
            # window size, recorded rather than silently rescaled away.
            size_ok = (fr[2], fr[3]) == (rr[2], rr[3])
            json.dump(region_doc("win_" + rid, rr,
                                 "the reference's centred window box for "
                                 f"{name}, from render-ref.mjs buildPatch()"),
                      open(os.path.join(OUT, f"win-{rid}.json"), "w"), indent=1)
            json.dump(region_doc("win_" + rid, fr,
                                 f"where zlOS actually put {name}, from the "
                                 "compositor's own wm_report client rect"),
                      open(os.path.join(OUT, f"win-{rid}.zlos.json"), "w"), indent=1)
            manifest["apps"][rid] = dict(name=name, png=png, zlos_frame=fr,
                                         ref_frame=rr, size_match=size_ok, **extra)
            print(f"{tag} {name:<20} zlOS {str(fr):<24} ref {rr}"
                  f"{'' if size_ok else '   SIZE MISMATCH'}")
            return (cx, cy, cw, ch)

        # ---- the boot desktop, before anything is opened on top of it.
        # Terminal, Files and System Monitor are already up and are the three
        # windows regions.json's default_view describes, so they are scored
        # against reference-1280x800.png with the SHARED region map and need no
        # per-app rectangle at all.
        m.ser.drain(args.settle)
        desk = os.path.join(OUT, "zlos-app-desktop.png")
        print(f"boot desktop -> {shoot(m, desk)}")
        manifest["desktop"] = desk

        # ---- shell words FIRST, while the Terminal still has focus. They are
        # closed with a click on the close box and not with Ctrl+W: open_app()
        # deliberately hands focus back to the shell (you typed the word
        # mid-command-line), so Ctrl+W here would close the Terminal and take
        # the serial console with it.
        for rid, name, word, rw, rh in words:
            seen = len(m.ser.all)
            type_line(m.ser, word, args.cmd_timeout)
            got = capture(rid, name, rw, rh, seen, {"how": "shell word " + word})
            if got and rid not in BOOT_OPEN:
                bx, by = close_box(got[0], got[1], got[2], u)
                click(m.qmp, bx, by, m.w, m.h, m.ser.drain)

        # ---- then the catalog, which stays open for the whole sweep.
        if todo:
            cat = open_catalog(m, args.cmd_timeout)
            print(f"catalog client {cat}, ui {u}x")
            for rid, name, idx, rw, rh in todo:
                seen = len(m.ser.all)
                tx, ty, scroll = tile_click(m, cat, idx, u)
                if capture(rid, name, rw, rh, seen,
                           {"tile": idx, "click": [tx, ty], "scroll": scroll}):
                    # Ctrl+W over the serial line arrives as EV_CHAR 23 and
                    # wm.c closes the FOCUS window - which is the one the tile
                    # just opened, with no geometry to get wrong.
                    m.ser.send("\x17")
                    m.ser.drain(0.6)
        json.dump(manifest, open(os.path.join(OUT, "app-shots.json"), "w"), indent=1)
    ok = sum(1 for v in manifest["apps"].values() if "png" in v)
    print(f"\n{ok}/{total} captured in {time.time() - t0:.0f}s -> {OUT}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
