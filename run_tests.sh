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

echo "== examples: interpreter runs clean =="
mkdir -p examples_out
for ex in examples/*.zl; do
    name=$(basename "$ex" .zl)
    [ "$name" = "calc_repl" ] && { echo "  skip  $name (interactive REPL)"; continue; }
    if timeout 300 ./interp "$ex" >"$tmp/ex_$name.out" 2>&1; then
        echo "  ok    $name"
    else
        echo "  FAIL  $name"; tail -2 "$tmp/ex_$name.out"; fail=1
    fi
done

# Only the deterministic examples can be diffed against a compiled build:
# maze uses rand(), and life/vm print elapsed-time measurements that
# legitimately differ between an interpreted and a compiled run.
echo "== examples: C backend cross-check (deterministic ones) =="
for name in csvstats wordfreq texttools; do
    ( cd "$tmp" && "$OLDPWD/compile" "$OLDPWD/examples/$name.zl" >/dev/null 2>&1 && \
      gcc -O2 -D_strdup=strdup -I"$OLDPWD" -o "ex_$name.bin" out.c "$OLDPWD/runtime.c" "$OLDPWD/os_linux.c" -lm 2>"ex_$name.cc.err" )
    if [ -x "$tmp/ex_$name.bin" ]; then
        "$tmp/ex_$name.bin" > "$tmp/ex_$name.c.out" 2>&1
        if diff -q "$tmp/ex_$name.out" "$tmp/ex_$name.c.out" >/dev/null; then
            echo "  match $name"
        else
            echo "  DIFF  $name"; fail=1
        fi
    else
        echo "  BUILD FAIL $name"; cat "$tmp/ex_$name.cc.err"; fail=1
    fi
done

echo "== LLVM backend: unboxed-subset smoke test =="
cat > "$tmp/llvm_smoke.zl" <<'EOF'
fn fib(n) {
    if n < 2 { return n }
    return fib(n - 1) + fib(n - 2)
}
fn sum_to(n) {
    total = 0
    i = 1
    while i <= n {
        total = total + i
        i = i + 1
    }
    return total
}
print(fib(20))
print(sum_to(100))
print("llvm backend works")
EOF
if command -v clang >/dev/null; then
    ( cd "$tmp" && "$OLDPWD/compilel" llvm_smoke.zl >/dev/null 2>&1 && \
      clang -O2 out.ll -o llvm_smoke.bin 2>/dev/null )
    if [ -x "$tmp/llvm_smoke.bin" ]; then
        "$tmp/llvm_smoke.bin" > "$tmp/llvm_smoke.llvm.out" 2>&1
        ./interp "$tmp/llvm_smoke.zl" > "$tmp/llvm_smoke.interp.out" 2>&1
        if diff -q "$tmp/llvm_smoke.interp.out" "$tmp/llvm_smoke.llvm.out" >/dev/null; then
            echo "  ok    LLVM backend matches interpreter"
        else
            echo "  DIFF  LLVM backend output mismatch"; fail=1
        fi
    else
        echo "  BUILD FAIL compilel"; fail=1
    fi
else
    echo "  skip  (clang not installed)"
fi

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
