#!/usr/bin/env python3
"""diff-regions.py - score zlOS against the reference, one region at a time.

  ./diff-regions.py --zlos out/zlos-desktop.png \\
                    --ref ../refrender/out/reference-1280x800.png
  ./diff-regions.py ... --json                 machine-readable, for a gate
  ./diff-regions.py ... --only win_term,dock_band
  ./diff-regions.py ... --worst 10             only the regions furthest off


WHY NOT ONE RMS NUMBER
----------------------
Per-pixel RMS over a region is dominated by text antialiasing. Two renderers
that agree on every design decision still disagree on the grey ramp inside
every glyph edge, and a region that is 30% text is then 30% noise. Run it on
this corpus and every region comes back "about 0.25 off", which is the same as
having no oracle: it cannot distinguish a window that is 40px too tall from a
window whose font hints differently.

So there are three measures, and they fail in different directions on purpose.
A region is only in trouble when the RIGHT one is bad:

  colour      mean absolute RGB error, normalised. Answers "is this region
              roughly the right brightness and hue". Antialiasing moves it a
              little; a wrong background moves it a lot.

  palette     Earth-Mover distance between the two colour histograms, over a
              coarse RGB quantisation. Answers "do the same colours appear, in
              the same proportion". THIS is what catches "the accent is cyan,
              not lime": swapping one hue for another leaves mean error small
              (both are mid-bright) and moves this a long way. It is
              position-blind by construction, so moving a button does not
              disturb it. EMD rather than a bin-by-bin distance because
              bin-by-bin calls #b8e838 and #b9e838 as different as #b8e838 and
              black - a quantisation boundary would otherwise dominate.

  structure   gradient magnitude, downsampled to a coarse grid, compared as a
              normalised correlation. Answers "is the furniture in the same
              place". Antialiasing is a high-frequency signal and the
              downsample removes it; an edge that moved 40px survives it. THIS
              is what catches "the toolbar is in the wrong place".

`score` is the worst of the three, because the point of the exercise is to find
what is wrong, not to average it away with what is right.

All three are 0.0 = identical, 1.0 = maximally different, so a later agent
gating on "my region is under 0.15" is asking the same question of every
measure. See README.md for what the numbers actually mean on this corpus.

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


def structure_distance(a, b):
    """1 - correlation of coarse gradient-energy maps, 0..1.

    Gradient energy is "where are the edges"; pooling to a coarse grid throws
    away the antialias ramp (a 1-2px signal) and keeps the furniture (tens of
    px). Correlation rather than absolute difference so a region that is
    uniformly busier does not score as misplaced - being darker is the colour
    measure's job.
    """
    h, w = a.shape[0], a.shape[1]
    ny, nx = max(2, min(GRID, h // 4)), max(2, min(GRID, w // 4))
    ga, gb = _pool(_gradient(a), ny, nx).ravel(), _pool(_gradient(b), ny, nx).ravel()
    ga -= ga.mean()
    gb -= gb.mean()
    na, nb = np.linalg.norm(ga), np.linalg.norm(gb)
    if na < 1e-9 and nb < 1e-9:
        return 0.0            # both flat: no structure to disagree about
    if na < 1e-9 or nb < 1e-9:
        return 1.0            # one flat, one not: maximally different
    return float(min(1.0, max(0.0, (1.0 - float(ga @ gb) / (na * nb)) / 2.0)))


# ---- report -----------------------------------------------------------------
def band(score):
    if score < 0.10:
        return "match"
    if score < 0.20:
        return "close"
    if score < 0.40:
        return "off"
    return "WRONG"


def main():
    ap = argparse.ArgumentParser()
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
        st = structure_distance(ca, cb)
        rows.append(dict(id=r["id"], kind=r["kind"], rect=r["rect"],
                         visible=r.get("visible", True),
                         colour=round(col, 4), palette=round(pal, 4),
                         structure=round(st, 4),
                         score=round(max(col, pal, st), 4),
                         verdict=band(max(col, pal, st)),
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
                                   f"over {QUANT}-level histograms",
                           structure=f"1 - correlation of {GRID}x{GRID}-pooled "
                                     "gradient energy, halved to 0..1",
                           score="max of the three"),
                       results=rows), sys.stdout, indent=1)
        print()
    else:
        print(f"zlOS  {args.zlos}")
        print(f"ref   {args.ref}")
        print(f"      {want[0]}x{want[1]}, {len(rows)} regions\n")
        print(f"{'region':<24}{'rect':<22}{'colour':>8}{'palette':>9}"
              f"{'struct':>8}{'score':>8}  verdict   src")
        print("-" * 96)
        for d in rows:
            print(f"{d['id']:<24}{str(d['rect']):<22}{d['colour']:>8.3f}"
                  f"{d['palette']:>9.3f}{d['structure']:>8.3f}{d['score']:>8.3f}"
                  f"  {d['verdict']:<9} {d['derivation']}")
        if rows:
            print("-" * 96)
            print(f"{'MEAN':<24}{'':<22}"
                  f"{sum(d['colour'] for d in rows) / len(rows):>8.3f}"
                  f"{sum(d['palette'] for d in rows) / len(rows):>9.3f}"
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
