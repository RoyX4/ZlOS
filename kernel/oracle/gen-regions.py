#!/usr/bin/env python3
"""gen-regions.py - build regions.json out of the reference mockup itself.

The region map is the shared coordinate system of the whole fidelity oracle:
diff-regions.py scores per region, and ~50 later agents each read ONE number
out of it as their done-condition. So every number in it has to be traceable
to a byte in ds.html rather than to somebody's eye - a hand-transcribed rect
is a silent, permanent lie about where a thing is, and it would be believed
because it looks like data.

Three classes of number, and this file keeps them apart on purpose:

  EXACT      taken from the mockup's own source. Every app window: ds.html
             declares w/h/x/y literally, and UTILS/GAME_APPS compute theirs
             from an index with a .map() - which is JS, so this runs THE
             ACTUAL JS in node rather than reimplementing the formula in
             Python. Reimplementing it would be transcription wearing a
             function's clothes: it would agree today and drift silently.

  DERIVED    computed from CSS declarations parsed out of the file. The dock
             is the good case: every one of its children is a fixed pixel
             size (33px tiles, 5px gap, 1px separator, 5px/7px padding), so
             its box is arithmetic on numbers this script reads out of the
             style attribute, not a guess.

  PARTIAL    the top island bar. Four of its ten children are text - the
             clock, "Activities", the tray, the bell - and text width needs a
             font metric that does not exist in the source. So the island's
             y and height ARE exact (top:8px, height:32px) and its width is
             NOT derivable; the region is a full-width strip at that exact y,
             and it says so in its own `derivation` field.

Every region carries `derivation`, and diff-regions.py prints it, so nobody
has to come back here to find out how much a number is worth.

  ./gen-regions.py                      -> regions.json
  ./gen-regions.py --html /path/ds.html -o somewhere.json

Needs node (only to run the mockup's own .map() calls). regions.json is
committed, so nothing downstream needs node.
"""
import argparse, hashlib, json, os, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_HTML = os.path.expanduser("~/Downloads/ds.html")


# ---- reading numbers out of an inline style attribute -----------------------
def find_style(html, needle, what):
    """The style="..." string of the one element whose style contains `needle`.

    Anchored on a substring that is unique in the file rather than on a line
    number: line numbers move, and a region map that silently starts reading
    a different element is exactly the failure this whole file exists to
    prevent. So a miss, or an ambiguous hit, is fatal here.
    """
    hits = [m for m in re.finditer(r'style="([^"]*)"', html) if needle in m.group(1)]
    if len(hits) != 1:
        raise SystemExit(f"{what}: wanted exactly one element whose style contains "
                         f"{needle!r}, found {len(hits)}. The mockup changed - "
                         f"re-anchor this rather than guessing the box.")
    return hits[0].group(1)


def px(style, prop, what):
    """One `prop:<n>px` value out of a style string, as a float."""
    m = re.search(re.escape(prop) + r"\s*:\s*(-?[\d.]+)px", style)
    if not m:
        raise SystemExit(f"{what}: no {prop} in {style[:120]!r}")
    return float(m.group(1))


# A CSS length. Bare `0` is legal and unitless, and `margin:0 2px` in this file
# uses it - a regex that demanded `px` on both halves silently failed there.
LEN = r"(-?[\d.]+)px|(0)(?![\d.])"


def _len(m, i):
    return float(m.group(i) if m.group(i) is not None else m.group(i + 1))


def px_pair(style, prop, what):
    """`prop:<a> <b>` -> (a, b). CSS shorthand, vertical then horizontal."""
    m = re.search(re.escape(prop) + r"\s*:\s*(?:" + LEN + r")\s+(?:" + LEN + r")",
                  style)
    if not m:
        raise SystemExit(f"{what}: no two-value {prop} in {style[:120]!r}")
    return _len(m, 1), _len(m, 3)


def js_px(html, anchor, key, what):
    """A `key:'33px'` out of the JS style object named by `anchor`.

    ANCHORED, and that is not pedantry: the unanchored version of this took
    the first `width:'..px'` anywhere in the script, which is the 26px window
    title-bar button, and it produced a dock box 112px too narrow that looked
    entirely plausible.
    """
    i = html.find(anchor)
    if i < 0:
        raise SystemExit(f"{what}: anchor {anchor!r} not found")
    m = re.search(re.escape(key) + r"\s*:\s*'(-?[\d.]+)px'", html[i:i + 400])
    if not m:
        raise SystemExit(f"{what}: no {key} within 400 chars of {anchor!r}")
    return float(m.group(1))


def slice_between(html, start_pat, end_pat, what):
    a = re.search(start_pat, html)
    if not a:
        raise SystemExit(f"{what}: start pattern {start_pat!r} not found")
    b = re.search(end_pat, html[a.start():])
    if not b:
        raise SystemExit(f"{what}: end pattern {end_pat!r} not found after start")
    return html[a.start():a.start() + b.end()]


# ---- the app tables, evaluated as the JS they are ---------------------------
def run_js(html):
    """Evaluate the mockup's own app tables and initial state, in node.

    GAME_APPS and UTILS place their windows with
        x:120 + (i % 6) * 26,  y:70 + (i % 5) * 24
    inside a .map(). Rewriting that in Python is how a region map starts
    agreeing with the mockup and quietly stops. So hand the real declarations
    to node and take back JSON.
    """
    apps_js = slice_between(html, r"const GAME_APPS = \[",
                            r"\.concat\(UTILS\)\.concat\(GAME_APPS\);", "app tables")
    consts = re.search(r"const TOP = (\d+), DOCK = (\d+);", html)
    if not consts:
        raise SystemExit("TOP/DOCK constants not found")
    pin = re.search(r"const PIN = (\[[^\]]*\]);", html)
    if not pin:
        raise SystemExit("dock PIN list not found")
    open0 = re.search(r"open:\{([^}]*)\},\s*focus:'(\w+)'", html)
    if not open0:
        raise SystemExit("initial `open:` state not found")
    ws0 = re.search(r"min:\{\},\s*prev:\{\},\s*ws:(\d+)", html)
    if not ws0:
        raise SystemExit("initial workspace not found")
    desk0 = re.search(r"desk:(\[\{p:'[^\]]*\}\]),", html)
    if not desk0:
        raise SystemExit("initial desktop icons not found")

    script = f"""
{apps_js}
const TOP = {consts.group(1)}, DOCK = {consts.group(2)};
const PIN = {pin.group(1)};
const OPEN0 = {{{open0.group(1)}}};
const FOCUS0 = '{open0.group(2)}';
const WS0 = {ws0.group(1)};
const DESK0 = {desk0.group(1)};
console.log(JSON.stringify({{
  apps: APPS.map(a => ({{id:a.id, name:a.name, w:a.w, h:a.h, x:a.x, y:a.y, ws:a.ws}})),
  TOP, DOCK, PIN, OPEN0, FOCUS0, WS0, DESK0,
  dockTiles: (() => {{
    const pinned = PIN.map(id => APPS.find(a => a.id === id)).filter(Boolean);
    const running = APPS.filter(a => OPEN0[a.id] && !PIN.includes(a.id));
    return pinned.concat(running).length;
  }})()
}}));
"""
    with tempfile.NamedTemporaryFile("w", suffix=".mjs", delete=False) as fh:
        fh.write(script)
        path = fh.name
    try:
        r = subprocess.run(["node", path], capture_output=True, text=True)
        if r.returncode != 0:
            raise SystemExit("node failed evaluating the mockup's app tables:\n"
                             + r.stderr[-2000:])
        return json.loads(r.stdout)
    finally:
        os.unlink(path)


# ---- the chrome -------------------------------------------------------------
def chrome_regions(html, js, screen_w, screen_h):
    out = []

    # --- the top island bar. EXACT in y and height, NOT derivable in width.
    island = find_style(html, "z-index:62;top:8px;left:50%", "top island bar")
    isl_top = px(island, "top", "island")
    isl_h = px(island, "height", "island")
    out.append(dict(
        id="topbar_strip", kind="chrome", ws=None, visible=True,
        rect=[0, int(isl_top), screen_w, int(isl_h)],
        derivation=("PARTIAL. y and height are exact - the island's own "
                    f"top:{isl_top:g}px / height:{isl_h:g}px. Its WIDTH is not "
                    "derivable from the source: 4 of its 10 flex children are "
                    "text (Activities, the clock, the tray, the bell) and text "
                    "width needs a font metric the file does not contain. So "
                    "this is the full-width strip at the island's exact band; "
                    "it necessarily includes wallpaper either side of the "
                    "island, which dilutes the colour measures and does NOT "
                    "dilute the structure measure.")))
    out.append(dict(
        id="topbar_band", kind="chrome", ws=None, visible=True,
        rect=[0, 0, screen_w, js["TOP"]],
        derivation=(f"EXACT. TOP = {js['TOP']} is the mockup's own constant for "
                    "'chrome at the top' (it is what window drags clamp "
                    "against). Every pixel of the island is inside it.")))

    # --- the dock. EXACT: every child is a fixed pixel size.
    dock = find_style(html, "z-index:63;left:50%;bottom:9px", "dock")
    d_bottom = px(dock, "bottom", "dock")
    d_gap = px(dock, "gap", "dock")
    d_pad_v, d_pad_h = px_pair(dock, "padding", "dock")
    d_border = px(dock, "border", "dock")
    sep = find_style(html, "width:1px;height:22px", "dock separator")
    sep_w = px(sep, "width", "dock separator")
    sep_mv, sep_mh = px_pair(sep, "margin", "dock separator")
    tile = js_px(html, "gridBtnStyle:{", "width", "dock tile")
    tiles = js["dockTiles"]
    # children: N tiles + 1 separator + 1 "show applications" button
    kids = tiles + 2
    content = (tiles * tile + (sep_w + 2 * sep_mh) + tile + (kids - 1) * d_gap)
    d_w = content + 2 * d_pad_h + 2 * d_border
    d_h = tile + 2 * d_pad_v + 2 * d_border
    d_x = (screen_w - d_w) / 2.0
    d_y = screen_h - d_bottom - d_h
    out.append(dict(
        id="dock_strip", kind="chrome", ws=None, visible=True,
        rect=[int(d_x), int(d_y), int(round(d_w)), int(round(d_h))],
        derivation=(f"EXACT. {tiles} pinned tiles of {tile:g}px + a {sep_w:g}px "
                    f"separator with {sep_mh:g}px side margins + one "
                    f"{tile:g}px 'show applications' button, {kids - 1} gaps of "
                    f"{d_gap:g}px, padding {d_pad_v:g}/{d_pad_h:g}px and a "
                    f"{d_border:g}px border (content-box, so padding and border "
                    f"add). w={d_w:g} h={d_h:g}; centred (left:50% + "
                    f"translateX(-50%)) and bottom:{d_bottom:g}px. Every one of "
                    "those numbers is read out of the style attribute; no text "
                    "is measured because the dock contains none.")))
    out.append(dict(
        id="dock_band", kind="chrome", ws=None, visible=True,
        rect=[0, screen_h - js["DOCK"], screen_w, js["DOCK"]],
        derivation=(f"EXACT. DOCK = {js['DOCK']} is the mockup's own constant "
                    "for 'chrome at the bottom'.")))

    # --- desktop icons. Content-box: the declared width is the CONTENT width.
    dsk = re.search(r"style:\{position:'absolute', zIndex:3, left:d\.x \+ 'px', "
                    r"top:d\.y \+ 'px', width:'(\d+)px',\s*"
                    r"display:'flex', flexDirection:'column', alignItems:'center', "
                    r"gap:'(\d+)px', padding:'(\d+)px (\d+)px'", html)
    if not dsk:
        raise SystemExit("desktop icon style not found - re-anchor it")
    di_w, di_gap, di_pv, di_ph = (float(v) for v in dsk.groups())
    ic = re.search(r"iconStyle:\{display:'grid', placeItems:'center', "
                   r"width:'(\d+)px', height:'(\d+)px'", html)
    lb = re.search(r"labelStyle:\{fontSize:'([\d.]+)px', textAlign:'center', "
                   r"lineHeight:([\d.]+)", html)
    if not ic or not lb:
        raise SystemExit("desktop icon tile/label style not found - re-anchor it")
    di_ih, di_lh = float(ic.group(2)), float(lb.group(1)) * float(lb.group(2))
    di_border = 1.0                               # border:'1px solid ...'
    box_w = di_w + 2 * di_ph + 2 * di_border
    box_h = di_ih + di_gap + di_lh + 2 * di_pv + 2 * di_border
    for d in js["DESK0"]:
        name = d["p"].lstrip("/").replace(".", "_").lower()
        out.append(dict(
            id="deskicon_" + name, kind="chrome", ws=None, visible=True,
            rect=[int(d["x"]), int(d["y"]), int(round(box_w)), int(round(box_h))],
            derivation=(f"DERIVED. x,y are the mockup's own desk[] entry for "
                        f"{d['p']}. Content-box: width {di_w:g} + padding "
                        f"{di_ph:g}x2 + border 1x2 = {box_w:g}. Height "
                        f"{di_ih:g} icon + {di_gap:g} gap + {di_lh:g} label line "
                        f"({lb.group(1)}px x {lb.group(2)} line-height) + "
                        f"{di_pv:g}x2 padding + 1x2 border = {box_h:g}. The label "
                        "line is the only rounded number here.")))
    return out


def app_regions(js):
    out = []
    open0 = js["OPEN0"]
    for a in js["apps"]:
        out.append(dict(
            id="win_" + a["id"], kind="window", name=a["name"], ws=a["ws"],
            visible=bool(open0.get(a["id"])) and a["ws"] == js["WS0"],
            focused=(a["id"] == js["FOCUS0"]),
            rect=[a["x"], a["y"], a["w"], a["h"]],
            derivation=("EXACT. The mockup's own APPS/UTILS/GAME_APPS entry, "
                        "evaluated in node - including the .map() that computes "
                        "x/y from the index for the utilities and the games. "
                        "Windows are position:absolute inside the 1280x800 root, "
                        "so these are screen coordinates with no container "
                        "offset, and they are content-box with no border or "
                        "padding, so w/h are the rendered size.")))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--html", default=DEFAULT_HTML)
    ap.add_argument("-o", "--out", default=os.path.join(HERE, "regions.json"))
    args = ap.parse_args()

    if not os.path.exists(args.html):
        raise SystemExit(f"{args.html} not found - pass --html")
    blob = open(args.html, "rb").read()
    html = blob.decode("utf-8", "replace")

    prev = re.search(r"&quot;\$preview&quot;:\{&quot;width&quot;:(\d+),"
                     r"&quot;height&quot;:(\d+)\}", html)
    if not prev:
        raise SystemExit("the mockup's $preview size was not found - refusing to "
                         "assume 1280x800")
    W, H = int(prev.group(1)), int(prev.group(2))

    js = run_js(html)
    regions = [dict(id="screen", kind="frame", ws=None, visible=True,
                    rect=[0, 0, W, H],
                    derivation="EXACT. ds.html's own $preview width/height.")]
    regions += chrome_regions(html, js, W, H)
    regions += app_regions(js)

    # An empty patch of wallpaper, computed rather than eyeballed: the largest
    # gap this script can prove is clear of every visible window, of the dock
    # band and of the top band. It is the region that catches "the wallpaper is
    # a flat colour where the mockup has a gradient", which no window region
    # can see.
    busy = [r["rect"] for r in regions
            if r["visible"] and r["id"] not in ("screen",)
            and r["kind"] in ("window", "chrome")]
    best = None
    for x in range(0, W - 120, 8):
        for y in range(js["TOP"], H - js["DOCK"] - 120, 8):
            for w, h in ((320, 240), (240, 180), (160, 120)):
                if x + w > W or y + h > H - js["DOCK"]:
                    continue
                if any(x < bx + bw and bx < x + w and y < by + bh and by < y + h
                       for bx, by, bw, bh in busy):
                    continue
                if best is None or w * h > best[2] * best[3]:
                    best = (x, y, w, h)
                break
    if best:
        regions.append(dict(
            id="wallpaper", kind="chrome", ws=None, visible=True,
            rect=list(best),
            derivation=("DERIVED. The largest box this generator could place "
                        "that provably overlaps no visible window, no dock band "
                        "and no top band, searched on an 8px grid. It is the "
                        "only region that measures the wallpaper itself.")))

    doc = dict(
        schema=1,
        note=("Region map for the zlOS fidelity oracle. Coordinates are "
              "REFERENCE coordinates and both images are scored in them, so "
              "the zlOS screenshot must be the same size. Read `derivation` "
              "before trusting any rect: EXACT means it came out of the "
              "mockup's source, PARTIAL means part of it did not."),
        generated_by="kernel/oracle/gen-regions.py",
        source=dict(file=os.path.abspath(args.html), bytes=len(blob),
                    sha256=hashlib.sha256(blob).hexdigest()),
        screen=dict(w=W, h=H),
        default_view=dict(workspace=js["WS0"], focus=js["FOCUS0"],
                          open=sorted(js["OPEN0"])),
        regions=regions)
    with open(args.out, "w") as fh:
        json.dump(doc, fh, indent=1)
        fh.write("\n")
    vis = sum(1 for r in regions if r["visible"])
    print(f"wrote {args.out}")
    print(f"  {len(regions)} regions, {vis} visible in the default view "
          f"(workspace {js['WS0']}), screen {W}x{H}")
    for r in regions:
        if r["kind"] != "window" or r["visible"]:
            print(f"    {r['id']:<24} {str(r['rect']):<24} "
                  f"{r['derivation'].split('.')[0]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
