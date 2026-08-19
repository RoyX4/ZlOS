#!/usr/bin/env bash
# engine-parity.sh - do the execution engines implement the same language?
#
# run_tests.sh already cross-checks the C backend against the interpreter for
# every tests/*.zl, and smoke-tests compilel and nativegen. But its LLVM smoke
# program uses lists, strings and sort() - none of which exercise the semantic
# corners where the engines actually disagree. So a real divergence sat in the
# tree unnoticed:
#
#     counter = 100
#     fn bump() { counter = 7  return 0 }
#     bump()
#     print(counter)
#
#     interp 7 | compile 7 | compilel 100
#
# This runs a small battery of corner cases through every engine and compares
# each against the interpreter, which is ground truth (AGENTS.md section 3).
#
# Known divergences are PINNED in tools/engine-parity-expected.txt rather than
# treated as failures - they are real, documented, and some are waiting on the
# scoping decision in docs/design/design_scoping_decision.md. The gate fires on
# a divergence that is NOT pinned, in either direction:
#
#   - a new engine disagreement appears        -> FAIL
#   - a pinned divergence is fixed             -> FAIL, telling you to unpin it
#
# The second is deliberate. A pin that quietly stops being true is how a stale
# expectation outlives the bug it described.
#
# Only the integer subset is used: compilel and nativegen document that they do
# not accept strings, lists or f-strings, so testing those would measure the
# documented subset rather than a disagreement.

set -uo pipefail
cd "$(dirname "$0")/.."

EXPECTED="tools/engine-parity-expected.txt"
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
fail=0

for b in interp compile compilel nativegen; do
    [ -x "./$b" ] || { echo "engine-parity: ./$b not built - run ./build.sh"; exit 2; }
done

# ---- the battery -----------------------------------------------------------
# Each case is integer-only and prints one or more numbers.

case_src() {
case "$1" in
global_writethrough) cat <<'EOF'
counter = 100
fn bump() { counter = 7  return 0 }
bump()
print(counter)
EOF
;;
recursion) cat <<'EOF'
fn fact(n) { if n <= 1 { return 1 }  return n * fact(n - 1) }
print(fact(6))
EOF
;;
locals_dont_collide) cat <<'EOF'
fn setter() { tmp = 42  return 0 }
fn reader() { tmp = 1  setter()  return tmp }
print(reader())
EOF
;;
param_shadows_global) cat <<'EOF'
v = 5
fn f(v) { v = v + 1  return v }
print(f(10))
print(v)
EOF
;;
loop_var_scope) cat <<'EOF'
i = 99
fn go() { for i in [0,1,2] { }  return 0 }
go()
print(i)
EOF
;;
nested_call_depth) cat <<'EOF'
fn a(n) { return b(n) + 1 }
fn b(n) { if n <= 0 { return 0 }  return a(n - 1) + 1 }
print(a(8))
EOF
;;
integer_division) cat <<'EOF'
print(7 / 2)
print(0 - 7 / 2)
print(7 % 3)
print(0 - 7 % 3)
EOF
;;
esac
}

CASES="global_writethrough recursion locals_dont_collide param_shadows_global loop_var_scope nested_call_depth integer_division"

run_engine() {   # run_engine <engine> <srcfile> ; echoes output or __BUILDFAIL__
    # Separate declarations on purpose: `local eng="$1" d="$tmp/$eng"` expands
    # $eng before it is set, which under `set -u` kills the command
    # substitution and silently yields an empty result rather than an error.
    local eng="$1"
    local src="$2"
    local d="$tmp/engine-$eng"
    local root="$PWD"
    mkdir -p "$d"
    case "$eng" in
      interp)   ./interp "$src" 2>&1 ;;
      compile)  ( ./compile "$src" >/dev/null 2>&1 \
                  && gcc -O2 -D_strdup=strdup -I. -o "$d/a.bin" out.c runtime.c os_linux.c -lm 2>/dev/null \
                  && "$d/a.bin" 2>&1 ) || echo "__BUILDFAIL__" ;;
      compilel) ( ./compilel "$src" >/dev/null 2>&1 \
                  && clang -O2 out.ll runtime.c os_linux.c -I. -D_strdup=strdup -o "$d/a.bin" -lm 2>/dev/null \
                  && "$d/a.bin" 2>&1 ) || echo "__BUILDFAIL__" ;;
      # nativegen writes ./native_out into the current directory, so it runs in
      # a scratch dir. $src is already absolute - prefixing it with the repo
      # root produced a path that did not exist and read as a build failure.
      nativegen)( cd "$d" && "$root/nativegen" "$src" >/dev/null 2>&1 \
                  && [ -x native_out ] && ./native_out 2>&1 ) || echo "__BUILDFAIL__" ;;
    esac
}

# ---- expectations ----------------------------------------------------------
pinned() {  # pinned <case> <engine>  -> 0 if a divergence is pinned
    grep -qE "^[[:space:]]*$1[[:space:]]+$2([[:space:]]|$)" "$EXPECTED" 2>/dev/null
}

printf "%-24s %-10s %-10s %-10s\n" CASE compile compilel nativegen
printf "%s\n" "----------------------------------------------------------------"

for c in $CASES; do
    src="$tmp/$c.zl"; case_src "$c" > "$src"
    truth=$(run_engine interp "$src")
    row=$(printf "%-24s" "$c")
    for eng in compile compilel nativegen; do
        got=$(run_engine "$eng" "$src")
        if [ "$got" = "__BUILDFAIL__" ]; then
            mark="build!"
        elif [ "$got" = "$truth" ]; then
            mark="match"
            if pinned "$c" "$eng"; then
                mark="FIXED"
                echo "  ::: $c/$eng is pinned as divergent but now MATCHES - remove it from $EXPECTED"
                fail=1
            fi
        else
            if pinned "$c" "$eng"; then
                mark="differ*"
            else
                mark="DIFFER"
                echo "  ::: $c/$eng disagrees with the interpreter and is NOT pinned"
                echo "      interp: $(echo "$truth" | tr '\n' ' ')"
                echo "      $eng: $(echo "$got" | tr '\n' ' ')"
                fail=1
            fi
        fi
        row=$(printf "%s %-10s" "$row" "$mark")
    done
    echo "$row"
done

echo
echo "  match = agrees with interp   differ* = pinned, known   build! = engine rejected it"
echo
[ "$fail" -ne 0 ] && { echo "engine-parity: FAILED"; exit 1; }
echo "engine-parity: no unpinned divergence"
