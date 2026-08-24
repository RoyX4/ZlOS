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
    route     catalog-static | legacy-static - which source route owns it

This checker also rejects a dense catalog id that has no reference app. That is
the inverse half the old gate missed: ID 14 produced a blank tile while Maze was
excluded, so the total stayed 47 and the script printed success.

Run:  python3 kernel/hosttest/apps53.py
      python3 kernel/hosttest/apps53.py --selftest

Exit 1 if any app lacks identity, existence, size, icon or a static route, or if
the catalog contains an extra/blank id. Runtime launch/readiness/teardown remain
separate QEMU evidence and are not inferred here.
"""
import re, sys, os, glob

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REF = os.path.join(os.path.dirname(ROOT), "docs/design/ds-reference.html")

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
for f in sorted(glob.glob(os.path.join(ROOT, "*.zl"))):
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
    if FIRST <= i <= LAST and i in dense: return "catalog-static"
    return "legacy-static"

def coverage_issues(dense_ids, names, sizes, icons):
    """Return exact failed fields plus inverse blank/extra catalog ids."""
    issues = []
    expected_catalog = set()
    for _, name, _ in W:
        got = names.get(name)
        if not got:
            issues.append((name, "identity"))
            continue
        app_id = got[0]
        in_registry_range = FIRST <= app_id <= LAST
        if in_registry_range:
            expected_catalog.add(app_id)
            if app_id not in dense_ids:
                issues.append((name, "exists"))
        if app_id not in sizes and got[1] != "kernel.zl":
            issues.append((name, "size"))
        if app_id not in icons and got[1] != "kernel.zl":
            issues.append((name, "icon"))
        if route(app_id) == "-":
            issues.append((name, "route"))
    for app_id in sorted(dense_ids - expected_catalog):
        issues.append(("id %d" % app_id, "blank/extra catalog id"))
    return issues

print("reference apps: %d      REG_FIRST=%s REG_LAST=%s   reg_count()=%d\n"
      % (len(W), FIRST, LAST, len(dense)))
print("  %-3s %-22s %-8s %-5s %-6s %-5s %-5s %s"
      % ("#", "name", "kind", "id", "exists", "size", "icon", "route"))
issues = coverage_issues(set(dense), byname, has_size, has_icon)
for k, (rid, name, kind) in enumerate(W, 1):
    got = byname.get(name)
    i = got[0] if got else None
    ok_ex = "yes" if (i is not None and (i not in range(FIRST, LAST + 1) or i in dense)) else "NO"
    ok_sz = "yes" if (i in has_size or (got and got[1] == "kernel.zl")) else "no"
    ok_ic = "yes" if (i in has_icon or (got and got[1] == "kernel.zl")) else "no"
    mark = " " if got and not any(item[0] == name for item in issues) else "*"
    print("%s %-3d %-22s %-8s %-5s %-6s %-5s %-5s %s"
          % (mark, k, name, kind, i if i is not None else "-", ok_ex, ok_sz, ok_ic, route(i)))
print()
if "--selftest" in sys.argv[1:]:
    maze = byname["Maze"][0]
    clip = byname["Clipboard"][0]
    mutations = {
        "missing-existence": coverage_issues(set(dense) - {maze}, byname, has_size, has_icon),
        "blank-extra-id": coverage_issues(set(dense) | {14}, byname, has_size, has_icon),
        "missing-size": coverage_issues(set(dense), byname, has_size - {clip}, has_icon),
        "missing-icon": coverage_issues(set(dense), byname, has_size, has_icon - {clip}),
        "missing-identity": coverage_issues(set(dense), {k: v for k, v in byname.items() if k != "Clipboard"}, has_size, has_icon),
    }
    escaped = [name for name, found in mutations.items() if not found]
    if escaped:
        print("SELFTEST FAILED: mutations escaped: " + ", ".join(escaped), file=sys.stderr)
        sys.exit(2)
    print("selftest: caught " + ", ".join(mutations))

if issues:
    print("INCOMPLETE (%d fields):" % len(issues))
    for name, field in issues:
        print("   %-24s %s" % (name, field))
    sys.exit(1)
print("all %d reference apps have complete static registry fields; catalog has no blank ids" % len(W))
