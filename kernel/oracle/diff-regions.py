#!/usr/bin/env python3
"""diff-regions.py - score zlOS against the reference, one region at a time.

  ./diff-regions.py --zlos out/zlos-desktop.png \\
                    --ref ../refrender/out/reference-1280x800.png
  ./diff-regions.py ... --json                 machine-readable, for a gate
  ./diff-regions.py ... --only win_term,dock_band
  ./diff-regions.py ... --worst 10             only the regions furthest off
  ./diff-regions.py --selftest                 check the measures themselves


WHY NOT ONE RMS NUMBER
----------------------
Per-pixel RMS over a region is dominated by text antialiasing. Two renderers
that agree on every design decision still disagree on the grey ramp inside
every glyph edge, and a region that is 30% text is then 30% noise. Run it on
this corpus and every region comes back "about 0.25 off", which is the same as
having no oracle: it cannot distinguish a window that is 40px too tall from a
window whose font hints differently.

So there are FOUR measures, and they fail in different directions on purpose.
A region is only in trouble when the RIGHT one is bad:

  colour      mean absolute RGB error, normalised. Answers "is this region
              roughly the right brightness and hue". Antialiasing moves it a
              little; a wrong background moves it a lot.

  palette     Earth-Mover distance between the two colour histograms, over a
              coarse RGB quantisation. Answers "do the same colours appear, in
              the same proportion". Position-blind by construction, so moving a
              button does not disturb it. EMD rather than a bin-by-bin distance
              because bin-by-bin calls #b8e838 and #b9e838 as different as
              #b8e838 and black - a quantisation boundary would dominate.

  hue         angular distance between the two regions' dominant SATURATED
              hues. THIS is what catches "the accent is cyan, not lime", and it
              is separate from `palette` because of a measured failure: an
              accent covers about 4% of a window, so swapping lime for cyan
              moves `palette` by 0.021 - real, but indistinguishable from noise
              at a glance. hue is area-independent and scored that swap at
              0.561. It is the only measure here that does not scale with how
              much of the region the difference covers.

  structure   coarse-pooled luminance AND coarse-pooled gradient energy, each
              standardised and compared as a mean absolute difference; the
              worse of the two. Answers "is the furniture in the same place",
              blind to overall brightness and contrast. Pooling to ~40x40px
              cells is what removes antialiasing.

`score` is the worst of the four, because the point of the exercise is to find
what is wrong, not to average it away with what is right.

All four are 0.0 = identical, 1.0 = maximally different. THEY ARE NOT ON THE
SAME SCALE, and pretending otherwise would be the easiest way to make this
lie: colour, palette and structure all scale with how much of the region the
difference covers, and hue does not. --selftest prints the four measures
against four synthetic pairs whose right answer is known by construction
(including an antialias-only control that all four must ignore), which is the
evidence for every claim in this docstring. Run it; it needs no QEMU and no
reference render. README.md has what the numbers mean on the real corpus.

Nothing here needs numpy tricks explained: the images are 1280x800 and the
whole sweep takes under two seconds.
"""
import argparse, json, os, sys

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))

QUANT = 6            # RGB levels per axis for the palette histogram: 6^3 = 216
GRID = 16            # structure grid is at most GRID x GRID cells


def load(path, want):
    if not os.path.exists(path):
        raise SystemExit(f"{path} does not exist")
    with Image.open(path) as im:
        a = np.asarray(im.convert("RGB"), dtype=np.float32)
    if (a.shape[1], a.shape[0]) != tuple(want):
        raise SystemExit(
            f"{path} is {a.shape[1]}x{a.shape[0]} but the region map is in "
            f"{want[0]}x{want[1]} coordinates. Rescaling here would be the "
            f"worst possible fix - it would smear exactly the edges the "
            f"structure measure exists to find. Re-shoot at the right size.")
    return a


def crop(img, rect):
    x, y, w, h = rect
    x0, y0 = max(0, x), max(0, y)
    x1, y1 = min(img.shape[1], x + w), min(img.shape[0], y + h)
    if x1 <= x0 or y1 <= y0:
        return None
    return img[y0:y1, x0:x1]


# ---- the three measures -----------------------------------------------------
def colour_error(a, b):
    """Mean absolute RGB error, 0..1. 255 per channel is the worst case."""
    return float(np.abs(a - b).mean() / 255.0)


def palette_distance(a, b):
    """Earth-Mover distance between quantised colour histograms, 0..1.

    Position-blind: it sees WHICH colours are present and in what proportion,
    and nothing about where they are. That is the whole point - it is the
    measure that survives a layout difference and does not survive a palette
    one.

    EMD over the 1-D cumulative histogram of a Hilbert-free raster order would
    be meaningless, so the ground distance is done per channel: three 1-D EMDs
    over the marginal histograms, averaged. Cheap, and it has the property that
    matters - a colour that moved one quantisation step costs a little, and one
    that moved across the range costs a lot, which a bin-by-bin chi-square gets
    exactly backwards at a bin boundary.
    """
    total = 0.0
    for c in range(3):
        ha = np.histogram(a[:, :, c], bins=QUANT, range=(0, 256))[0].astype(np.float64)
        hb = np.histogram(b[:, :, c], bins=QUANT, range=(0, 256))[0].astype(np.float64)
        ha /= max(ha.sum(), 1.0)
        hb /= max(hb.sum(), 1.0)
        # 1-D EMD is the L1 distance between the cumulative distributions,
        # divided by the number of steps to normalise to 0..1.
        total += float(np.abs(np.cumsum(ha) - np.cumsum(hb)).sum()) / (QUANT - 1)
    return min(1.0, total / 3.0)


def hue_distance(a, b):
    """Angular distance between the two regions' dominant saturated hues, 0..1.

    THE AREA-INDEPENDENT ONE, and it is here because the other three are not.
    Every measure above scales with how much of the region the difference
    covers, which is mathematically honest and practically useless for an
    accent: a lime toolbar swapped for a cyan one covers about 4% of a window,
    so it moves `palette` by about 0.02 and could be mistaken for noise. Hue
    does not care how much of the region the accent occupies, only whether it
    is the same hue.

    Guarded by mass, because "a handful of stray saturated pixels" is not an
    accent: at least 0.5% of the region must be saturated. If neither image has
    that, there is no accent to disagree about and this is 0. If exactly one
    does, the accent is present in one render and absent in the other, which is
    a real and severe difference - 1.0.
    """
    def dominant(img):
        mx = img.max(axis=2)
        mn = img.min(axis=2)
        chroma = mx - mn
        sat = np.where(mx > 1e-6, chroma / np.maximum(mx, 1e-6), 0.0)
        mask = (sat > 0.35) & (mx > 64)
        n = int(mask.sum())
        if n < 0.005 * img.shape[0] * img.shape[1]:
            return None, n
        r, g, bl = img[:, :, 0][mask], img[:, :, 1][mask], img[:, :, 2][mask]
        # hue as an angle, then a circular mean - averaging raw hue degrees
        # puts the mean of red-ish 350 and red-ish 10 at 180, which is cyan.
        mxv = np.maximum(np.maximum(r, g), bl)
        mnv = np.minimum(np.minimum(r, g), bl)
        c = np.maximum(mxv - mnv, 1e-6)
        h = np.where(mxv == r, ((g - bl) / c) % 6,
            np.where(mxv == g, (bl - r) / c + 2, (r - g) / c + 4)) * 60.0
        rad = np.deg2rad(h)
        return float(np.rad2deg(np.arctan2(np.sin(rad).mean(),
                                           np.cos(rad).mean())) % 360.0), n

    ha, na = dominant(a)
    hb, nb = dominant(b)
    if ha is None and hb is None:
        return 0.0
    if ha is None or hb is None:
        return 1.0
    d = abs(ha - hb) % 360.0
    return float(min(d, 360.0 - d) / 180.0)


def _gradient(img):
    g = img.mean(axis=2)
    gx = np.zeros_like(g)
    gy = np.zeros_like(g)
    gx[:, 1:] = np.abs(np.diff(g, axis=1))
    gy[1:, :] = np.abs(np.diff(g, axis=0))
    return gx + gy


def _pool(m, ny, nx):
    """Mean-pool an array down to ny x nx. Not a resize - the averaging IS the
    antialias filter, and a resampling filter with its own kernel would put a
    different smear on each image."""
    h, w = m.shape
    ys = np.linspace(0, h, ny + 1).astype(int)
    xs = np.linspace(0, w, nx + 1).astype(int)
    out = np.zeros((ny, nx), dtype=np.float64)
    for j in range(ny):
        for i in range(nx):
            blk = m[ys[j]:max(ys[j] + 1, ys[j + 1]), xs[i]:max(xs[i] + 1, xs[i + 1])]
            out[j, i] = blk.mean() if blk.size else 0.0
    return out


def _standardise(m):
    """Zero mean, unit standard deviation. Makes the comparison blind to
    overall brightness and to overall contrast, so a darker theme is not
    reported as a misplaced one - that is the colour measure's job."""
    s = m.std()
    return (m - m.mean()) / s if s > 1e-6 else np.zeros_like(m)


def structure_distance(a, b):
    """Mean absolute difference of standardised coarse maps, 0..1.

    Two coarse maps per image, and the answer is the worse of them:

      LAYOUT   pooled luminance. A block that moved 140px moves this a long
               way. It is what catches "the toolbar is in the wrong place".
      TEXTURE  pooled gradient energy. Catches "this panel is full of text and
               that one is empty", which pooled luminance can miss when the
               mean brightness happens to match.

    THE FIRST VERSION OF THIS WAS GRADIENT-CORRELATION ONLY, AND THE SELFTEST
    BELOW CAUGHT IT SCORING 0.002 ON A TOOLBAR THAT HAD MOVED 140 PIXELS. The
    reason is worth keeping: a solid block of colour has gradient only at its
    border, so in any region that also contains text - i.e. every region in
    this corpus - dense glyph edges dominate the gradient map and a block
    sliding across it barely registers. Pooled luminance has the opposite
    bias, which is why both are here.

    Pooling to ~40x40px cells is what removes antialiasing: a 1-2px grey ramp
    averages out, a piece of furniture tens of pixels across does not.
    """
    h, w = a.shape[0], a.shape[1]
    ny, nx = max(2, min(GRID, h // 4)), max(2, min(GRID, w // 4))
    worst = 0.0
    for f in (lambda i: i.mean(axis=2), _gradient):
        pa = _standardise(_pool(f(a), ny, nx))
        pb = _standardise(_pool(f(b), ny, nx))
        # Standardised maps live in roughly [-3, 3]; a mean absolute difference
        # of 2 is already "nothing is where it was", so halve to land 0..1.
        worst = max(worst, float(min(1.0, np.abs(pa - pb).mean() / 2.0)))
    return worst


# ---- report -----------------------------------------------------------------
def band(score):
    if score < 0.10:
        return "match"
    if score < 0.20:
        return "close"
    if score < 0.40:
        return "off"
    return "WRONG"


# ---- the check that the three measures do what the docstring claims ---------
def selftest():
    """Three synthetic pairs whose right answer is known by construction.

    The claims in this file's header are the whole reason anyone would trust a
    number out of it, and they are exactly the kind of claim that is easy to
    write and never verified. So: build the three failures each measure is
    supposed to own, and assert that the measure that owns it is the one that
    fires.

    Runs in well under a second, needs no QEMU and no reference render.
    """
    rng = np.random.default_rng(7)
    h, w = 200, 320

    def panel(bg, accent, ax):
        img = np.zeros((h, w, 3), dtype=np.float32)
        img[:, :] = bg
        img[20:44, ax:ax + 120] = accent            # an accent-coloured toolbar
        # some text-ish high-frequency detail, identical in both members of a
        # pair unless the case deliberately changes it
        img[80:180:4, 30:290:2] = (200, 205, 210)
        return img

    LIME, CYAN = (184, 232, 56), (56, 232, 224)
    base = panel((16, 18, 21), LIME, 30)

    cases = []
    # 1. identical
    cases.append(("identical", base, base.copy(), "none"))
    # 2. the accent is cyan, not lime - same brightness, same position
    cases.append(("accent hue swapped", base, panel((16, 18, 21), CYAN, 30),
                  "hue"))
    # 3. the toolbar moved 140px right - same colours, same amount of them
    cases.append(("toolbar moved 140px", base, panel((16, 18, 21), LIME, 170),
                  "structure"))
    # 4. antialiasing noise only: every edge pixel jittered by a few levels.
    #    This is the CONTROL - the thing all four measures must ignore, and
    #    the reason a single RMS number is useless on this corpus.
    noisy = base + rng.normal(0, 6, base.shape).astype(np.float32)
    cases.append(("antialias jitter only", base, np.clip(noisy, 0, 255), "none"))

    print(f"{'case':<26}{'colour':>9}{'palette':>9}{'hue':>9}{'struct':>9}   "
          f"expected to fire")
    print("-" * 82)
    bad = []
    for name, a, b, expect in cases:
        got = dict(colour=colour_error(a, b), palette=palette_distance(a, b),
                   hue=hue_distance(a, b), structure=structure_distance(a, b))
        print(f"{name:<26}{got['colour']:>9.3f}{got['palette']:>9.3f}"
              f"{got['hue']:>9.3f}{got['structure']:>9.3f}   {expect}")
        # The assertion is DISCRIMINATION, not magnitude, and that distinction
        # is the whole design. colour/palette/structure all scale with how much
        # of the region the difference covers - a 4%-area accent swap moving
        # `palette` by 0.02 is arithmetically correct, and demanding 0.15 there
        # would only mean tuning the measure until it lied. So: the measure
        # that owns a failure must clearly beat the others on it, and must
        # clearly beat its own reading on the antialias control.
        floor = 0.01
        if expect == "none":
            if max(got.values()) > 0.05:
                bad.append(f"{name}: something fired ({got}) and nothing should")
        else:
            others = [v for k, v in got.items() if k != expect]
            if got[expect] < floor:
                bad.append(f"{name}: {expect} only reached {got[expect]:.3f}, "
                           f"under the {floor} noise floor")
            if got[expect] <= max(others):
                bad.append(f"{name}: {expect}={got[expect]:.3f} did not exceed "
                           f"the others {[round(v, 3) for v in others]}")
    print("-" * 82)
    for b in bad:
        print("FAIL:", b)
    print("selftest: " + ("ok - each measure fires on its own failure and not "
                          "on antialiasing" if not bad else f"{len(bad)} problem(s)"))
    return 1 if bad else 0


def main():
    if "--selftest" in sys.argv:
        return selftest()
    ap = argparse.ArgumentParser()
    ap.add_argument("--selftest", action="store_true",
                    help="check the four measures against synthetic pairs "
                         "whose right answer is known by construction")
    ap.add_argument("--zlos", required=True)
    ap.add_argument("--ref", required=True)
    ap.add_argument("--regions", default=os.path.join(HERE, "regions.json"))
    ap.add_argument("--only", default="", help="comma-separated region ids")
    ap.add_argument("--visible-only", action="store_true",
                    help="skip regions the default view does not show")
    ap.add_argument("--worst", type=int, default=0)
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--fail-over", type=float, default=None,
                    help="exit 2 if any reported region scores above this")
    args = ap.parse_args()

    doc = json.load(open(args.regions))
    want = (doc["screen"]["w"], doc["screen"]["h"])
    zl, rf = load(args.zlos, want), load(args.ref, want)

    keep = {s.strip() for s in args.only.split(",") if s.strip()}
    rows = []
    for r in doc["regions"]:
        if keep and r["id"] not in keep:
            continue
        if args.visible_only and not r.get("visible", True):
            continue
        ca, cb = crop(zl, r["rect"]), crop(rf, r["rect"])
        if ca is None or cb is None:
            continue
        col = colour_error(ca, cb)
        pal = palette_distance(ca, cb)
        hue = hue_distance(ca, cb)
        st = structure_distance(ca, cb)
        worst = max(col, pal, hue, st)
        rows.append(dict(id=r["id"], kind=r["kind"], rect=r["rect"],
                         visible=r.get("visible", True),
                         colour=round(col, 4), palette=round(pal, 4),
                         hue=round(hue, 4), structure=round(st, 4),
                         score=round(worst, 4), verdict=band(worst),
                         worst_measure=max(
                             (("colour", col), ("palette", pal),
                              ("hue", hue), ("structure", st)),
                             key=lambda t: t[1])[0],
                         derivation=r["derivation"].split(".")[0]))

    rows.sort(key=lambda d: -d["score"])
    if args.worst:
        rows = rows[:args.worst]

    if args.json:
        json.dump(dict(zlos=os.path.abspath(args.zlos),
                       ref=os.path.abspath(args.ref),
                       regions_sha=doc["source"]["sha256"],
                       screen=doc["screen"],
                       measures=dict(
                           colour="mean absolute RGB error / 255",
                           palette="mean per-channel 1-D earth-mover distance "
                                   f"over {QUANT}-level histograms; scales with "
                                   "the AREA of the difference",
                           hue="angular distance between dominant saturated "
                               "hues / 180; area-INdependent; 1.0 means an "
                               "accent exists in one render and not the other",
                           structure="worse of two standardised "
                                     f"{GRID}x{GRID}-pooled maps (luminance and "
                                     "gradient energy), mean abs diff / 2",
                           score="max of the four - the measures are NOT on a "
                                 "common scale, so read `worst_measure` with it"),
                       results=rows), sys.stdout, indent=1)
        print()
    else:
        print(f"zlOS  {args.zlos}")
        print(f"ref   {args.ref}")
        print(f"      {want[0]}x{want[1]}, {len(rows)} regions\n")
        print(f"{'region':<24}{'rect':<22}{'colour':>8}{'palette':>9}{'hue':>7}"
              f"{'struct':>8}{'score':>8}  verdict   worst      src")
        print("-" * 108)
        for d in rows:
            print(f"{d['id']:<24}{str(d['rect']):<22}{d['colour']:>8.3f}"
                  f"{d['palette']:>9.3f}{d['hue']:>7.3f}{d['structure']:>8.3f}"
                  f"{d['score']:>8.3f}  {d['verdict']:<9} "
                  f"{d['worst_measure']:<10} {d['derivation']}")
        if rows:
            print("-" * 108)
            print(f"{'MEAN':<24}{'':<22}"
                  f"{sum(d['colour'] for d in rows) / len(rows):>8.3f}"
                  f"{sum(d['palette'] for d in rows) / len(rows):>9.3f}"
                  f"{sum(d['hue'] for d in rows) / len(rows):>7.3f}"
                  f"{sum(d['structure'] for d in rows) / len(rows):>8.3f}"
                  f"{sum(d['score'] for d in rows) / len(rows):>8.3f}")

    if args.fail_over is not None:
        bad = [d["id"] for d in rows if d["score"] > args.fail_over]
        if bad:
            print(f"\n{len(bad)} region(s) over {args.fail_over}: "
                  f"{', '.join(bad[:8])}", file=sys.stderr)
            return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
