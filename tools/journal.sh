#!/usr/bin/env bash
# journal.sh - append what actually happened to docs/JOURNAL.md.
#
#   tools/journal.sh              record HEAD
#   tools/journal.sh <rev>        record a specific commit
#
# Git already stores the diff. What it does not store is the state of the tree
# around a change: which gates were green, how many hazards were outstanding,
# whether the docs still agreed with the code. That context is what makes a
# commit legible six months later, and it is exactly what nobody writes down.
#
# Cheap by construction - it records what the tools already measured. Nothing
# here boots an emulator.

set -uo pipefail
cd "$(dirname "$0")/.."

REV="${1:-HEAD}"
OUT=docs/JOURNAL.md
mkdir -p docs

sha=$(git rev-parse --short "$REV" 2>/dev/null) || { echo "journal: no such rev: $REV"; exit 1; }

# Already recorded? Keep this idempotent - a post-commit hook can fire twice.
if [ -f "$OUT" ] && grep -qF "($sha)" "$OUT"; then
    echo "journal: $sha already recorded"
    exit 0
fi

# An amend (or a rebase) gives the same logical commit a new sha, so a naive
# append records it twice - and the workflow this hook recommends is exactly
# `git commit --amend`. If the newest entry names a commit that is no longer an
# ancestor of HEAD, that commit was rewritten away: drop its entry rather than
# stacking a second one beside it.
if [ -f "$OUT" ]; then
    top=$(grep -m1 '^## ' "$OUT" | grep -oE '\(([0-9a-f]{7,})\)$' | tr -d '()')
    if [ -n "$top" ] && ! git merge-base --is-ancestor "$top" HEAD 2>/dev/null; then
        first=$(grep -n '^## ' "$OUT" | head -1 | cut -d: -f1)
        second=$(grep -n '^## ' "$OUT" | sed -n 2p | cut -d: -f1)
        if [ -n "$second" ]; then
            { head -n $((first - 1)) "$OUT"; tail -n +"$second" "$OUT"; } > "$OUT.tmp"
        else
            head -n $((first - 1)) "$OUT" > "$OUT.tmp"
        fi
        mv "$OUT.tmp" "$OUT"
        echo "journal: dropped rewritten entry $top"
    fi
fi

subject=$(git log -1 --format=%s "$REV")
branch=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
files=$(git show --stat --format='' "$REV" | tail -1 | sed 's/^ *//')
# The hardware clock on this box drifts and NTP corrects it mid-session, so a
# local timestamp can be days out. Use the commit's own date, which travels
# with it, rather than `date`.
when=$(git log -1 --format=%cs "$REV")

changed=$(git show --name-only --format='' "$REV" | grep -v '^$' | head -12)
nfiles=$(git show --name-only --format='' "$REV" | grep -cv '^$')

trunc=$(tools/hazard-scan.sh --count 2>/dev/null || echo "?")
pins=$(grep -vcE '^\s*#|^\s*$' tools/engine-parity-expected.txt 2>/dev/null || echo "?")
rot=$(grep -vcE '^\s*#|^\s*$' tools/doc-check-ignore.txt 2>/dev/null || echo "?")
docs_ok=$(tools/doc-check.sh >/dev/null 2>&1 && echo "agree" || echo "STALE")

if [ ! -f "$OUT" ]; then
cat > "$OUT" <<'HEAD'
# Journal

What happened, and what the tree looked like when it happened. Appended
automatically by `tools/journal.sh` from the post-commit hook — newest first.

Git stores the diff; this stores the context around it: outstanding hazard
counts, whether the docs still agreed with the code, which files moved. That is
the part that makes a change legible later, and the part nobody writes down.

HEAD
fi

entry=$(mktemp)
{
    echo "## $when — $subject ($sha)"
    echo
    echo "\`$branch\` · $files"
    echo
    echo "| | |"
    echo "|---|---|"
    echo "| EFI truncation sites | $trunc |"
    echo "| pinned engine divergences | $pins |"
    echo "| baselined doc rot | $rot |"
    echo "| docs vs tree | $docs_ok |"
    echo
    if [ "$nfiles" -gt 0 ]; then
        echo "<details><summary>$nfiles file(s)</summary>"
        echo
        echo "$changed" | sed 's/^/- `/; s/$/`/'
        [ "$nfiles" -gt 12 ] && echo "- _…and $((nfiles - 12)) more_"
        echo
        echo "</details>"
        echo
    fi
} > "$entry"

# Newest first: splice after the header block, not at the end of the file.
if grep -q '^## ' "$OUT"; then
    first=$(grep -n '^## ' "$OUT" | head -1 | cut -d: -f1)
    head -n $((first - 1)) "$OUT" > "$OUT.tmp"
    cat "$entry" >> "$OUT.tmp"
    tail -n +"$first" "$OUT" >> "$OUT.tmp"
    mv "$OUT.tmp" "$OUT"
else
    cat "$entry" >> "$OUT"
fi
rm -f "$entry"

echo "journal: recorded $sha ($trunc truncation sites, docs $docs_ok)"
