#!/usr/bin/env bash
# run_tests.sh - Linux equivalent of the original project's run_tests.ps1.
#
# 1. Runs every tests/*.zl through the interpreter (the full language).
# 2. Cross-checks that the C backend (compile -> gcc) produces byte-identical
#    output to the interpreter for every one of those files.
# 3. Cross-checks the hand-assembled native x86-64 backend (integer subset
#    only) against the interpreter on a small integer-only smoke program.
set -uo pipefail
cd "$(dirname "$0")"

fail=0
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

echo "== interpreter: full test suite =="
for t in tests/*.zl; do
    name=$(basename "$t" .zl)
    out=$(./interp "$t" 2>&1)
    echo "$out" > "$tmp/$name.interp.out"
    grep -q " 0 failed" <<<"$out" && echo "  ok    $name" || { echo "  FAIL  $name"; fail=1; }
done

echo "== C backend: cross-check against interpreter =="
for t in tests/*.zl; do
    name=$(basename "$t" .zl)
    ( cd "$tmp" && "$OLDPWD/compile" "$OLDPWD/$t" >/dev/null 2>&1 && \
      gcc -O2 -D_strdup=strdup -I"$OLDPWD" -o "$name.bin" out.c "$OLDPWD/runtime.c" "$OLDPWD/os_linux.c" -lm 2>"$name.cc.err" )
    if [ -x "$tmp/$name.bin" ]; then
        "$tmp/$name.bin" > "$tmp/$name.native_c.out" 2>&1
        if diff -q "$tmp/$name.interp.out" "$tmp/$name.native_c.out" >/dev/null; then
            echo "  match $name"
        else
            echo "  DIFF  $name"; fail=1
        fi
    else
        echo "  BUILD FAIL $name"; cat "$tmp/$name.cc.err"; fail=1
    fi
done

echo "== native x86-64 ELF backend: integer-subset smoke test =="
cat > "$tmp/nat_smoke.zl" <<'EOF'
fn fact(n) {
    if n <= 1 { return 1 }
    return n * fact(n - 1)
}
print(fact(10))
print("hello from native")
x = 5
while x > 0 {
    print(x)
    x = x - 1
}
exit(42)
EOF
( cd "$tmp" && "$OLDPWD/nativegen" nat_smoke.zl >/dev/null 2>&1 )
if [ -x "$tmp/native_out" ]; then
    "$tmp/native_out" > "$tmp/nat_smoke.native.out"; code=$?
    "$OLDPWD/interp" "$tmp/nat_smoke.zl" > "$tmp/nat_smoke.interp.out"
    if diff -q "$tmp/nat_smoke.interp.out" "$tmp/nat_smoke.native.out" >/dev/null && [ "$code" -eq 42 ]; then
        echo "  ok    native backend matches interpreter, exit code 42"
    else
        echo "  DIFF  native backend output or exit code mismatch"; fail=1
    fi
else
    echo "  BUILD FAIL nativegen"; fail=1
fi

echo
if [ "$fail" -eq 0 ]; then
    echo "ALL GREEN"
else
    echo "SOME CHECKS FAILED"
fi
exit $fail
