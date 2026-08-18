#!/usr/bin/env bash
# doc-check.sh - are the docs still true?
#
# Documentation rots silently. This repo has already produced every failure mode
# below at least once:
#
#   - .ultra/STATE.md said the compositor was "built, tested and unreachable -
#     nothing calls them", while kernel.zl called wm_open_p in six places.
#   - CLAUDE.md described kernel/verify-efi.sh, verify_fmt.sh, zlfmt.c and
#     examples/syntax_tour.zl as parts of the repo. None were in git.
#   - CLAUDE.md said the four -Werror= flags were "fatal". They were inert.
#
# The checks here are mechanical. Anything needing judgement belongs in a
# registered claim (section 4), where a doc states something and names the
# command that proves it.
#
# Exit 1 if any doc is provably stale.

set -uo pipefail
cd "$(dirname "$0")/.."

IGNORE=tools/doc-check-ignore.txt

fail=0
# Known-stale entries are baselined, so pre-existing rot does not paint every
# run red while a NEW broken reference still fails. Same reasoning as
# tools/hazard-baseline.txt: a permanently red gate is one nobody reads.
hit()  {
    if [ -f "$IGNORE" ] && grep -qxF "$*" "$IGNORE" 2>/dev/null; then
        echo "  known  $*"
        return
    fi
    echo "  STALE: $*"; fail=1
}
ok()   { echo "  ok     $*"; }
note() { echo "  note   $*"; }

# TODO.md and docs/JOURNAL.md are GENERATED, and TODO.md's whole job includes
# listing the broken references doc-check found - checking it would flag the
# report for containing the findings. The PR template is prose, not a map.
DOCS=$(git ls-files '*.md' 2>/dev/null \
       | grep -vE '^(TODO\.md|docs/JOURNAL\.md|\.github/pull_request_template\.md)$')
[ -n "$DOCS" ] || { echo "doc-check: no tracked .md files"; exit 0; }

echo "== 1. every file path a doc points at must exist =="
missing=0
for d in $DOCS; do
    # Only paths that look like real repo files: a/b.ext with a known extension.
    # Deliberately narrow - a wide regex turns prose into false positives.
    for ref in $(grep -ohE '\b(kernel|docs|tools|examples|tests|freestanding|stdlib|editors|\.github)/[A-Za-z0-9_./-]+\.(c|h|zl|sh|md|txt|json|yml)\b' "$d" 2>/dev/null | sort -u); do
        # A path in kernel/HANDOFF.md is relative to kernel/, not the repo
        # root. Accept either resolution before calling it missing.
        [ -e "$ref" ] || [ -e "$(dirname "$d")/$ref" ] \
            || { hit "$d references $ref, which does not exist"; missing=$((missing+1)); }
    done
done
[ "$missing" -eq 0 ] && ok "every referenced path exists"

echo "== 2. files the docs describe must actually be in git =="
# verify-efi.sh was described as essential by CLAUDE.md and existed on exactly
# one laptop. A doc describing an untracked file is a promise the repo cannot keep.
untracked=0
for d in $DOCS; do
    for ref in $(grep -ohE '\b(kernel|docs|tools|examples|tests|freestanding)/[A-Za-z0-9_./-]+\.(c|h|zl|sh)\b' "$d" 2>/dev/null | sort -u); do
        real="$ref"; [ -e "$real" ] || real="$(dirname "$d")/$ref"
        [ -e "$real" ] || continue
        git ls-files --error-unmatch "$real" >/dev/null 2>&1 \
            || { hit "$d describes $real, which exists but is NOT tracked"; untracked=$((untracked+1)); }
    done
done
[ "$untracked" -eq 0 ] && ok "every described file is tracked"

echo "== 3. numbers a doc cites must match what the tools measure =="
# Docs quote counts. Tools produce counts. When they disagree, the doc is stale.
if command -v clang >/dev/null 2>&1; then
    measured=$(tools/hazard-scan.sh --count 2>/dev/null || echo "")
    if [ -n "$measured" ]; then
        cited=$(grep -hoE '\*\*?([0-9]+)\*\*? (pointer-)?truncation sites?' $DOCS 2>/dev/null \
                | grep -oE '[0-9]+' | sort -u)
        if [ -z "$cited" ]; then
            note "no doc cites a truncation count (measured now: $measured)"
        else
            bad=0
            for n in $cited; do
                [ "$n" = "$measured" ] || { hit "a doc cites $n truncation sites; measured $measured"; bad=1; }
            done
            [ "$bad" -eq 0 ] && ok "cited truncation count matches measurement ($measured)"
        fi
    fi
else
    note "clang absent - skipped the measured-number comparison"
fi

echo "== 4. registered claims =="
# tools/doc-claims.txt: a doc asserts something, and names the command that
# proves it. This is where anything needing judgement goes, converted once into
# something a machine can re-check forever.
CLAIMS=tools/doc-claims.txt
if [ ! -f "$CLAIMS" ]; then
    note "no $CLAIMS"
else
    n=0
    while IFS='|' read -r doc claim cmd; do
        case "$doc" in ''|\#*) continue ;; esac
        # NOT xargs: it treats quotes as special and shreds the commands.
        doc="${doc#"${doc%%[![:space:]]*}"}"; doc="${doc%"${doc##*[![:space:]]}"}"
        claim="${claim#"${claim%%[![:space:]]*}"}"; claim="${claim%"${claim##*[![:space:]]}"}"
        cmd="${cmd#"${cmd%%[![:space:]]*}"}"; cmd="${cmd%"${cmd##*[![:space:]]}"}"
        n=$((n+1))
        if bash -c "$cmd" >/dev/null 2>&1; then
            ok "$doc: $claim"
        else
            hit "$doc claims \"$claim\" - the check failed:  $cmd"
        fi
    done < "$CLAIMS"
    [ "$n" -eq 0 ] && note "no claims registered yet"
fi

echo
[ "$fail" -ne 0 ] && { echo "doc-check: docs are STALE"; exit 1; }
echo "doc-check: docs agree with the tree"
