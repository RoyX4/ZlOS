#!/usr/bin/env bash
# doc-check.sh - are the docs still true?
#
# Documentation rots silently. This repo has already produced every failure mode
# below at least once:
#
#   - .ultra/STATE.md said the compositor was "built, tested and unreachable -
#     nothing calls them", while kernel.zl called wm_open_p in six places.
#   - CLAUDE.md described kernel/tools/checks/verify-efi.sh, verify_fmt.sh, zlfmt.c and
#     examples/syntax_tour.zl as parts of the repo. None were in git.
#   - CLAUDE.md said the four -Werror= flags were "fatal". They were inert.
#
# The checks here are mechanical. Anything needing judgement belongs in a
# registered claim (section 4), where a doc states something and names the
# command that proves it.
#
# Exit 1 if any doc is provably stale.

set -uo pipefail
cd "$(dirname "$0")/.." || exit

IGNORE=tools/doc-check-ignore.txt
PATHS_ONLY=0
LINK_SELFTEST=0
case "${1:-}" in
    --paths-only) PATHS_ONLY=1 ;;
    --selftest-links) LINK_SELFTEST=1 ;;
    "") ;;
    *) echo "usage: $0 [--paths-only|--selftest-links]" >&2; exit 2 ;;
esac

markdown_link_refs() {
    sed -E 's/`[^`]*`//g' "$1" 2>/dev/null \
        | grep -oE '\[[^]]*\]\([^) ]+' \
        | sed -E 's/^.*\]\(//' \
        | sort -u || true
}

if [ "$LINK_SELFTEST" -eq 1 ]; then
    fixture=$(mktemp -d); trap 'rm -rf "$fixture"' EXIT
    touch "$fixture/present.md"
    printf '%s\n' '[ok](present.md)' '`fs[0](5)`' '[bad](missing.md)' > "$fixture/index.md"
    refs=$(markdown_link_refs "$fixture/index.md")
    grep -qxF 'present.md' <<< "$refs" \
        && grep -qxF 'missing.md' <<< "$refs" \
        && ! grep -qxF '5' <<< "$refs" \
        && [ -e "$fixture/$(printf '%s\n' "$refs" | grep '^present')" ] \
        && [ ! -e "$fixture/$(printf '%s\n' "$refs" | grep '^missing')" ] || {
            echo "doc-check link selftest: FAIL"
            exit 1
        }
    echo "doc-check link selftest: caught missing target and ignored inline code"
    exit 0
fi

fail=0
# Path-looking references that belong to this checkout. The leading character is
# included to avoid matching repo paths inside sibling links such as
# ../../../zl/docs/..., then stripped before checking.
REPO_REF='(^|[^A-Za-z0-9_./-])(src|kernel|docs|tools|examples|tests|freestanding|stdlib|editors|\.github)/[A-Za-z0-9_./-]+\.(c|h|zl|sh|md|txt|json|yml|yaml|S|asm|ld|inc|py|mjs|js)\b'
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
DOCS=$({ git ls-files '*.md' 2>/dev/null
         git ls-files --others --exclude-standard '*.md' 2>/dev/null
       } | sort -u \
       | grep -vE '^(TODO\.md|docs/JOURNAL\.md|\.github/pull_request_template\.md|docs/archive/|docs/program/research/|kernel/docs/archive/)')
[ -n "$DOCS" ] || { echo "doc-check: no tracked .md files"; exit 0; }

echo "== 1. every file path a doc points at must exist =="
missing=0
for d in $DOCS; do
    # Only paths that look like real repo files: a/b.ext with a known extension.
    # Deliberately narrow - a wide regex turns prose into false positives.
    for ref in $(grep -ohE "$REPO_REF" "$d" 2>/dev/null | sed -E 's/^[^A-Za-z0-9_.-]//' | sort -u); do
        # A path in kernel/HANDOFF.md is relative to kernel/, not the repo
        # root. Accept either resolution before calling it missing.
        if [ -e "$ref" ] || [ -e "$(dirname "$d")/$ref" ] \
            || { [[ "$d" == kernel/* ]] && [ -e "kernel/$ref" ]; }; then
            continue
        fi
        # A clean checkout deliberately lacks ignored generated files. Docs may
        # describe those outputs without requiring somebody to build them first.
        if git check-ignore -q "$ref" 2>/dev/null \
            || git check-ignore -q "$(dirname "$d")/$ref" 2>/dev/null; then
            continue
        fi
        hit "$d references $ref, which does not exist"
        missing=$((missing+1))
    done
done
[ "$missing" -eq 0 ] && ok "every referenced path exists"

echo "== 1b. every local Markdown link must resolve from its document =="
# REPO_REF catches repo-root paths in prose. This second pass catches ordinary
# relative Markdown links such as ../metadata/foo.json. Strip inline code first
# so source text like `fs[0](5)` is not mistaken for a Markdown link.
broken_links=0
LINK_DOCS=$({ git ls-files '*.md' 2>/dev/null
              git ls-files --others --exclude-standard '*.md' 2>/dev/null
            } | sort -u \
            | grep -vE '^(TODO\.md|docs/JOURNAL\.md|\.github/pull_request_template\.md)')
for d in $LINK_DOCS; do
    while IFS= read -r ref; do
        case "$ref" in ''|http://*|https://*|mailto:*|data:*|\#*|/*) continue ;; esac
        ref=${ref%%#*}; ref=${ref#<}; ref=${ref%>}
        [ -e "$(dirname "$d")/$ref" ] && continue
        hit "$d links to $ref, which does not resolve from that document"
        broken_links=$((broken_links+1))
    done < <(markdown_link_refs "$d")
done
[ "$broken_links" -eq 0 ] && ok "every local Markdown link resolves"

echo "== 2. files the docs describe must actually be in git =="
# verify-efi.sh was described as essential by CLAUDE.md and existed on exactly
# one laptop. A doc describing an untracked, non-ignored file is a promise the
# repo cannot keep. Ignored build outputs such as kernel/out.c are different:
# docs may mention them precisely to say they are generated and must not be
# tracked.
untracked=0
for d in $DOCS; do
    for ref in $(grep -ohE "$REPO_REF" "$d" 2>/dev/null | sed -E 's/^[^A-Za-z0-9_.-]//' | grep -E '\.(c|h|zl|sh)$' | sort -u); do
        real="$ref"; [ -e "$real" ] || real="$(dirname "$d")/$ref"
        if [ ! -e "$real" ] && [[ "$d" == kernel/* ]]; then real="kernel/$ref"; fi
        [ -e "$real" ] || continue
        git check-ignore -q "$real" 2>/dev/null && continue
        git ls-files --error-unmatch "$real" >/dev/null 2>&1 \
            || { hit "$d describes $real, which exists but is NOT tracked"; untracked=$((untracked+1)); }
    done
done
[ "$untracked" -eq 0 ] && ok "every described file is tracked"

if [ "$PATHS_ONLY" -eq 1 ]; then
    echo
    [ "$fail" -ne 0 ] && { echo "doc-check: docs are STALE"; exit 1; }
    echo "doc-check: doc paths agree with the tree"
    exit 0
fi

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
