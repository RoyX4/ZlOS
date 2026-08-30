#!/usr/bin/env python3
"""score-apps.py - the per-app fidelity table, worst first.

  ./score-apps.py                one row per app, ranked by score
  ./score-apps.py --json         the same rows, machine-readable
  ./score-apps.py --measure structure    rank by one measure instead

Reads out/app-shots.json (written by shot-apps.py) and runs diff-regions.py
once per app, over that app's WINDOW in each image - the reference's centred
box in app-<id>.png and the rectangle zlOS's own compositor reported. Needs no
QEMU: it is arithmetic over PNGs that already exist.

Terminal, Files and System Monitor are scored differently and the difference is
stated in the table rather than hidden: they are open at boot in both renders,
at the SAME rectangle, so they come from the desktop shot against
regions.json's own win_term / win_files / win_mon. Everything else needs the
two-map comparison because the reference centres a single-app shot and zlOS
cannot be put in that state (see shot-apps.py's header).

THE `ref canvas` COLUMN IS NOT AN EXCUSE, IT IS A FACT ABOUT THE REFERENCE.
kernel/tests/refrender/ds.html fails to load seven modules that were never delivered,
so in the REFERENCE render the drawing surface of all 24 games plus Renderer,
Framebuffer, Console (tty1), Font Atlas and Image Viewer is BLANK. zlOS draws
those canvases. A high score on one of those rows is mostly measuring a hole in
the reference, and reading it as a zlOS defect would send someone to fix
something that is already right - so the column is printed beside the number.
"""
import argparse, json, os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "out")
REF = os.path.join(os.path.dirname(HERE), "refrender", "out")
DIFF = os.path.join(HERE, "diff-regions.py")

# The apps whose canvas the reference itself does not draw. Established before
# this script existed and recorded here so the table can say so per row.
BLANK_IN_REF = {"render", "fb", "tty", "font", "img"}

# Open at boot in BOTH renders, at the same rectangle - so they are scored with
# the shared region map and no per-app rects at all.
BOOT_TRIO = {"term": "win_term", "files": "win_files", "mon": "win_mon"}


def run(argv):
    p = subprocess.run(argv, capture_output=True, text=True)
    if p.returncode not in (0, 2):
        return None, (p.stderr or p.stdout).strip().splitlines()[-1:]
    try:
        return json.loads(p.stdout), None
    except json.JSONDecodeError:
        return None, (p.stderr or p.stdout).strip().splitlines()[-1:]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", default=os.path.join(OUT, "app-shots.json"))
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--measure", default="score",
                    choices=("score", "colour", "palette", "hue", "structure"))
    args = ap.parse_args()

    man = json.load(open(args.manifest))
    rows, failed = [], []

    # ---- the boot trio, from the desktop shot against the shared map.
    desk = man.get("desktop")
    ref_desk = os.path.join(REF, "reference-1280x800.png")
    if desk and os.path.exists(desk) and os.path.exists(ref_desk):
        doc, err = run([sys.executable, DIFF, "--zlos", desk, "--ref", ref_desk,
                        "--only", ",".join(BOOT_TRIO.values()), "--json"])
        if doc:
            back = {v: k for k, v in BOOT_TRIO.items()}
            for r in doc["results"]:
                r["app"] = back[r["id"]]
                r["how"] = "boot desktop"
                rows.append(r)
        else:
            failed.append(("boot trio", err))

    # ---- everything else, window against window.
    for rid, info in sorted(man.get("apps", {}).items()):
        if "png" not in info:
            failed.append((rid, [info.get("error", "not captured")]))
            continue
        ref = os.path.join(REF, f"app-{rid}.png")
        if not os.path.exists(ref):
            failed.append((rid, ["no reference render"]))
            continue
        doc, err = run([sys.executable, DIFF,
                        "--zlos", info["png"], "--ref", ref,
                        "--regions", os.path.join(OUT, f"win-{rid}.json"),
                        "--zlos-regions", os.path.join(OUT, f"win-{rid}.zlos.json"),
                        "--json"])
        if not doc or not doc["results"]:
            failed.append((rid, err or ["no region scored"]))
            continue
        r = doc["results"][0]
        r["app"] = rid
        r["how"] = "window crop"
        if not info.get("size_match", True):
            r["how"] += " SIZE"
        rows.append(r)

    key = args.measure
    rows.sort(key=lambda d: -d[key])

    if args.json:
        json.dump({"rows": rows, "failed": failed}, sys.stdout, indent=1)
        print()
        return 0

    print(f"per-app fidelity, {len(rows)} app(s), ranked by {key}, worst first")
    print(f"{'app':<13}{'colour':>8}{'palette':>9}{'hue':>7}{'struct':>8}"
          f"{'score':>8}  {'verdict':<9}{'worst':<10}{'ref canvas':<11}how")
    print("-" * 104)
    for d in rows:
        blank = "BLANK" if (d["app"].startswith("g_")
                            or d["app"] in BLANK_IN_REF) else "drawn"
        print(f"{d['app']:<13}{d['colour']:>8.3f}{d['palette']:>9.3f}"
              f"{d['hue']:>7.3f}{d['structure']:>8.3f}{d['score']:>8.3f}  "
              f"{d['verdict']:<9}{d['worst_measure']:<10}{blank:<11}{d['how']}")
    if rows:
        print("-" * 104)
        n = len(rows)
        print(f"{'MEAN':<13}"
              f"{sum(d['colour'] for d in rows) / n:>8.3f}"
              f"{sum(d['palette'] for d in rows) / n:>9.3f}"
              f"{sum(d['hue'] for d in rows) / n:>7.3f}"
              f"{sum(d['structure'] for d in rows) / n:>8.3f}"
              f"{sum(d['score'] for d in rows) / n:>8.3f}")
        drawn = [d for d in rows
                 if not (d["app"].startswith("g_") or d["app"] in BLANK_IN_REF)]
        if drawn:
            n = len(drawn)
            print(f"{'MEAN*':<13}"
                  f"{sum(d['colour'] for d in drawn) / n:>8.3f}"
                  f"{sum(d['palette'] for d in drawn) / n:>9.3f}"
                  f"{sum(d['hue'] for d in drawn) / n:>7.3f}"
                  f"{sum(d['structure'] for d in drawn) / n:>8.3f}"
                  f"{sum(d['score'] for d in drawn) / n:>8.3f}"
                  f"  * the {n} apps whose canvas the reference DOES draw")
    for rid, err in failed:
        print(f"  NOT SCORED  {rid:<12} {' '.join(err or [])}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
