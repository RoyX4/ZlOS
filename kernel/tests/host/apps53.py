#!/usr/bin/env python3
"""apps53.py - is every one of the reference's 53 apps actually in the tree?

Not "does a string with that name appear somewhere". For each of the 53 entries
in ds-reference.html's APPS / UTILS / GAME_APPS arrays this resolves, out of the
zl sources:

    id        the numeric app id, through whatever constant names it
    exists    reg_exists(id) == 1, mirrored from apps_registry.zl's own gaps
    name      reg_name(id) or a slice's <slice>_name(id)
    size      reg_w/reg_h, or the slice's, or a wm_open() literal
    icon      reg_icon or the slice's
    route     catalog | dock | shell | boot - how a person reaches it

Run:  python3 kernel/tests/host/apps53.py   (exit 1 if any of the 53 is missing)
"""
import re, sys, os, glob

KERNEL_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
REF = os.path.join(os.path.dirname(KERNEL_ROOT), "docs", "design", "ds-reference.html")

ref = open(REF, encoding="utf-8", errors="replace").read()

WANT = []
for m in re.finditer(r"\['(\w+)','([^']+)','(u\w+)'\]", ref):
    WANT.append(("u_" + m.group(1), m.group(2), "utility"))
for m in re.finditer(r"\['(\w+)',(\"[^\"]+\"|'[^']+'),'\w+',(\d+),(\d+),'(g\w+)'\]", ref):
    WANT.append(("g_" + m.group(1), m.group(2).strip("'\""), "game"))
for m in re.finditer(r"\{id:'(\w+)',\s*name:'([^']+)'[^}]*?w:(\d+),\s*h:(\d+)", ref):
    WANT.append((m.group(1), m.group(2), "system"))
seen, W = set(), []
for i, n, c in WANT:
    if i not in seen:
        seen.add(i); W.append((i, n, c))

# ---- resolve every constant in every zl file -------------------------------
consts, srcs = {}, {}
paths = [os.path.join(KERNEL_ROOT, "src", "kernel.zl")]
paths += sorted(glob.glob(os.path.join(KERNEL_ROOT, "apps", "apps_*.zl")))
for f in paths:
    s = open(f, encoding="utf-8", errors="replace").read()
    srcs[os.path.basename(f)] = s
    for m in re.finditer(r"^([A-Z][A-Z0-9_]*)\s*=\s*(\d+)\s*(?:#|$)", s, re.M):
        consts.setdefault(m.group(1), int(m.group(2)))

def num(tok):
    return int(tok) if tok.isdigit() else consts.get(tok)

# name -> (id, file). Both the registry's chain and every slice's.
byname, has_size, has_icon = {}, set(), set()
for fn, s in srcs.items():
    for m in re.finditer(r'if (?:id|app) == (\w+)\s*\{\s*return "([^"]+)"', s):
        v = num(m.group(1))
        if v is not None:
            byname.setdefault(m.group(2), (v, fn))
    for m in re.finditer(r'wm_open\((\w+),\s*"([^"]+)"', s):
        v = num(m.group(1))
        if v is not None:
            byname.setdefault(m.group(2), (v, fn))
    for fname, bucket in (("_w", has_size), ("_icon", has_icon)):
        for f2 in re.finditer(r"fn (?:reg|sys2|sys3|games3|games4)%s\(id\)\s*\{(.*?)\n\}" % fname, s, re.S):
            for m in re.finditer(r"if id == (\w+)", f2.group(1)):
                v = num(m.group(1))
                if v is not None:
                    bucket.add(v)

# ---- reg_exists(), mirrored from the registry rather than restated ---------
reg = srcs["apps_registry.zl"]
FIRST = consts.get("REG_FIRST"); LAST = consts.get("REG_LAST")
ex = re.search(r"fn reg_exists\(id\) \{(.*?)\n\}", reg, re.S)
holes = set()
if ex:
    for m in re.finditer(r"if id >= (\d+) \{ if id <= (\d+) \{ return 0", ex.group(1)):
        holes.update(range(int(m.group(1)), int(m.group(2)) + 1))
    for m in re.finditer(r"if id == (\d+) \{ return 0", ex.group(1)):
        holes.add(int(m.group(1)))
dense = [i for i in range(FIRST, LAST + 1) if i not in holes]

def route(i):
    if i is None: return "-"
    if FIRST <= i <= LAST and i in dense: return "catalog"
    return "dock/shell/boot"

print("reference apps: %d      REG_FIRST=%s REG_LAST=%s   reg_count()=%d\n"
      % (len(W), FIRST, LAST, len(dense)))
print("  %-3s %-22s %-8s %-5s %-6s %-5s %-5s %s"
      % ("#", "name", "kind", "id", "exists", "size", "icon", "route"))
missing = []
for k, (rid, name, kind) in enumerate(W, 1):
    got = byname.get(name)
    i = got[0] if got else None
    ok_ex = "yes" if (i is not None and (i not in range(FIRST, LAST + 1) or i in dense)) else "NO"
    ok_sz = "yes" if (i in has_size or (got and got[1] == "kernel.zl")) else "no"
    ok_ic = "yes" if (i in has_icon or (got and got[1] == "kernel.zl")) else "no"
    mark = " " if got else "*"
    if not got: missing.append(name)
    print("%s %-3d %-22s %-8s %-5s %-6s %-5s %-5s %s"
          % (mark, k, name, kind, i if i is not None else "-", ok_ex, ok_sz, ok_ic, route(i)))
print()
if missing:
    print("MISSING (%d):" % len(missing))
    for m in missing: print("   ", m)
    sys.exit(1)
print("all %d reference apps resolve to an id in the tree" % len(W))
