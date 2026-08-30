#!/usr/bin/env python3
"""Rebuild the desktop-redesign prototypes from the workflow agent transcripts.

WHY THIS EXISTS. The four prototypes were written into the session scratchpad under
/tmp, and /tmp was cleared before they were committed. Everything the agents did
survives in ~/.claude/projects/.../subagents/workflows/*/agent-*.jsonl, because each
Write and Edit is recorded there with its full arguments and a timestamp. So the
files are not lost, they are unreplayed.

The method is a replay, not a guess: collect every Write/Edit that targeted a given
path, order them by timestamp across ALL transcripts (four separate agents touched
each prototype - build, look, dark, check), and apply them in that order.

A failed Edit must not be applied. The model retries a failed edit with different
text, so applying both the failed attempt and its successful retry corrupts the
file. Every tool_use is therefore matched to its tool_result by id, and results
carrying is_error are dropped.
"""
import json, glob, os, sys, re

WF = os.path.expanduser(
    "~/.claude/projects/-home-roy-Documents/"
    "b3441b19-d599-43e9-a549-9bf8efa9e91f/subagents/workflows")
OUT = os.path.dirname(os.path.abspath(__file__))

TARGETS = re.compile(r'(proto|mock)-(plate|folio|datum|raking|index)\.html$')


def walk(node, out):
    """Yield every dict that looks like a tool_use or tool_result block."""
    if isinstance(node, dict):
        if node.get("type") in ("tool_use", "tool_result"):
            out.append(node)
        for v in node.values():
            walk(v, out)
    elif isinstance(node, list):
        for v in node:
            walk(v, out)


def collect():
    ops, failed = [], set()
    for path in glob.glob(os.path.join(WF, "*", "agent-*.jsonl")):
        for line in open(path, errors="replace"):
            if '"tool_use"' not in line and '"tool_result"' not in line:
                continue
            try:
                entry = json.loads(line)
            except Exception:
                continue
            ts = entry.get("timestamp", "")
            blocks = []
            walk(entry.get("message", {}), blocks)
            for b in blocks:
                if b.get("type") == "tool_result":
                    if b.get("is_error"):
                        failed.add(b.get("tool_use_id"))
                    else:
                        # an Edit that did not apply reports it in the text
                        txt = str(b.get("content", ""))[:400]
                        if "String to replace not found" in txt or "not found in file" in txt:
                            failed.add(b.get("tool_use_id"))
                    continue
                if b.get("name") not in ("Write", "Edit"):
                    continue
                fp = (b.get("input") or {}).get("file_path", "")
                if not TARGETS.search(fp or ""):
                    continue
                ops.append((ts, os.path.basename(fp), b.get("id"),
                            b["name"], b["input"]))
    ops.sort(key=lambda o: o[0])
    return ops, failed


def replay(ops, failed):
    files, stats = {}, {}
    for ts, name, tid, tool, inp in ops:
        if tid in failed:
            stats.setdefault(name, {"write": 0, "edit": 0, "skipped": 0})["skipped"] += 1
            continue
        st = stats.setdefault(name, {"write": 0, "edit": 0, "skipped": 0})
        if tool == "Write":
            files[name] = inp.get("content", "")
            st["write"] += 1
        else:
            cur = files.get(name)
            if cur is None:
                st["skipped"] += 1
                continue
            old, new = inp.get("old_string", ""), inp.get("new_string", "")
            if old and old in cur:
                files[name] = (cur.replace(old, new) if inp.get("replace_all")
                               else cur.replace(old, new, 1))
                st["edit"] += 1
            else:
                st["skipped"] += 1
    return files, stats


def main():
    ops, failed = collect()
    if not ops:
        sys.exit("no Write/Edit operations found - are the transcripts still there?")
    files, stats = replay(ops, failed)
    os.makedirs(OUT, exist_ok=True)
    print(f"{len(ops)} operations replayed, {len(failed)} failed ids skipped\n")
    for name in sorted(files):
        body = files[name]
        with open(os.path.join(OUT, name), "w") as fh:
            fh.write(body)
        s = stats[name]
        dark = body.count('data-theme="dark"') + body.count("data-theme='dark'")
        print(f"  {name:<20} {len(body.splitlines()):>5} lines  "
              f"{len(body):>7} B   writes={s['write']} edits={s['edit']} "
              f"skipped={s['skipped']}  dark-rules={dark}")


if __name__ == "__main__":
    main()
