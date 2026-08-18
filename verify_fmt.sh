#!/usr/bin/env bash
# verify_fmt.sh - proves zlfmt cannot damage a source file.
#
# The formatter's whole safety argument is "it only rewrites leading
# whitespace". This checks that claim against the real corpus rather than
# trusting it, on every .zl file in the tree:
#
#   1. it lexes         - zlfmt exits 1 on a file the lexer rejects
#   2. idempotent       - format(format(x)) == format(x)
#   3. content intact   - strip every line's leading/trailing whitespace from
#                         the input and the output; they must be identical
#   4. token stream identical, INCLUDING line numbers
#
# Check 4 is the strong one and subsumes the rest. zlfmt never adds or removes
# a line, so if the token dumps match byte for byte then the parser cannot
# distinguish the two files - the reformat is semantics-preserving by
# construction, not by inspection. A formatter that dropped a comment or
# truncated a long string literal would fail 3 and 4 immediately.
#
# Runs in about a second. No QEMU, no host-load sensitivity.
set -uo pipefail
cd "$(dirname "$0")"

[ -x ./zlfmt ]      || { echo "verify_fmt: ./zlfmt not built - run ./build.sh"; exit 2; }
[ -x ./lexer_demo ] || { echo "verify_fmt: ./lexer_demo not built - run ./build.sh"; exit 2; }

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

files=0; changed=0; fail=0

while IFS= read -r f; do
    files=$((files + 1))

    if ! ./zlfmt "$f" > "$tmp/a.zl" 2> "$tmp/err"; then
        echo "  FAIL  $f - does not lex: $(cat "$tmp/err")"; fail=1; continue
    fi

    cmp -s "$f" "$tmp/a.zl" || changed=$((changed + 1))

    ./zlfmt "$tmp/a.zl" > "$tmp/b.zl"
    cmp -s "$tmp/a.zl" "$tmp/b.zl" || { echo "  FAIL  $f - not idempotent"; fail=1; }

    sed 's/^[ \t]*//; s/[ \t]*$//' "$f"        > "$tmp/o.strip"
    sed 's/^[ \t]*//; s/[ \t]*$//' "$tmp/a.zl" > "$tmp/n.strip"
    cmp -s "$tmp/o.strip" "$tmp/n.strip" || { echo "  FAIL  $f - non-whitespace bytes changed"; fail=1; }

    ./lexer_demo "$f"        > "$tmp/o.tok" 2>/dev/null
    ./lexer_demo "$tmp/a.zl" > "$tmp/n.tok" 2>/dev/null
    cmp -s "$tmp/o.tok" "$tmp/n.tok" || { echo "  FAIL  $f - token stream differs"; fail=1; }
done < <(find . -name '*.zl' -not -path './.git/*' | sort)

echo "----------------------------------------------"
echo "  $files files checked, $changed would be re-indented"
if [ $fail -eq 0 ]; then
    echo "  PASS - lexes, idempotent, content intact, token stream identical"
else
    echo "  FAIL - see above"
fi
exit $fail
