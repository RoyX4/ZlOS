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

echo "== imports (Phase 2 hinge) =="
mkdir -p "$tmp/imp"
cat > "$tmp/imp/numkit.zl" <<'EOF'
fn nk_double(n) { return n * 2 }
fn nk_square(n) { return n * n }
EOF
printf 'import numkit\nprint(nk_double(21))\nprint(nk_square(7))\n' > "$tmp/imp/p.zl"
# a local ./<name>.zl must resolve, and every backend must see the splice
( cd "$tmp/imp" && "$OLDPWD/interp" p.zl ) > "$tmp/imp/interp.out" 2>&1
if [ "$(tr '\n' ' ' < "$tmp/imp/interp.out")" = "42 49 " ]; then
    echo "  ok    local module resolves, interpreter"
else
    echo "  FAIL  local module"; cat "$tmp/imp/interp.out"; fail=1
fi
# stdlib resolution + demo code must NOT run on import
printf 'import mathkit\nprint(mk_factorial(5))\n' > "$tmp/imp/s.zl"
if [ "$(./interp "$tmp/imp/s.zl" 2>&1 | tr '\n' ' ')" = "120 " ]; then
    echo "  ok    stdlib module resolves, demo code not run"
else
    echo "  FAIL  stdlib import ran demo code or wrong result"; fail=1
fi
# cycle a->b->a must terminate; double import must be a no-op
printf 'import b\nfn a_hi() { return "a" }\n' > "$tmp/imp/a.zl"
printf 'import a\nfn b_hi() { return "b" }\n' > "$tmp/imp/b.zl"
printf 'import a\nimport a\nprint(a_hi())\nprint(b_hi())\n' > "$tmp/imp/c.zl"
if [ "$( ( cd "$tmp/imp" && timeout 10 "$OLDPWD/interp" c.zl ) 2>&1 | tr '\n' ' ')" = "a b " ]; then
    echo "  ok    cycle terminates, double import is a no-op"
else
    echo "  FAIL  cycle or double-import"; fail=1
fi
# a missing module must say so clearly, not crash
printf 'import nosuchthing\n' > "$tmp/imp/m.zl"
# capture first: interp correctly exits 1 here, and `set -o pipefail` would
# make the pipeline fail on that even though grep matched.
./interp "$tmp/imp/m.zl" > "$tmp/imp/m.out" 2>&1 || true
if grep -q "module 'nosuchthing' not found" "$tmp/imp/m.out"; then
    echo "  ok    missing module gives a clear error"
else
    echo "  FAIL  missing module error"; fail=1
fi
# the payoff: the imports rewrite of texttools matches the inlined original
if diff -q <(./interp examples/texttools.zl 2>&1) \
           <(./interp examples/texttools_imports.zl 2>&1) >/dev/null; then
    echo "  ok    texttools_imports matches the inlined original byte for byte"
else
    echo "  DIFF  texttools_imports differs from the original"; fail=1
fi

echo "== W5: hex literals + raw memory =="
cat > "$tmp/w5.zl" <<'EOF'
print(0xFF)
print(0x8E)
print(0xDEAD)
print(255 == 0xFF)
p = alloc(64)
poke8(p, 0x41)
poke16(p + 8, 0xBEEF)
poke32(p + 16, 0xDEADBEEF)
poke64(p + 24, 0x1122334455)
print(peek8(p))
print(peek16(p + 8))
print(peek32(p + 16))
print(peek64(p + 24))
fill_mem(p + 32, 0xFF, 4)
print(peek32(p + 32))
copy_mem(p + 40, p, 1)
print(peek8(p + 40))
print(sext(0xFF, 8))
print(sext(0x7F, 8))
free(p)
EOF
./interp "$tmp/w5.zl" > "$tmp/w5.interp" 2>&1
if grep -q "^255$" "$tmp/w5.interp" && grep -q "^3735928559$" "$tmp/w5.interp" && grep -q "^-1$" "$tmp/w5.interp"; then
    echo "  ok    hex literals and raw memory, interpreter"
else
    echo "  FAIL  w5 primitives"; cat "$tmp/w5.interp"; fail=1
fi
# the C backend is the kernel-track backend: it must agree exactly
( cd "$tmp" && "$OLDPWD/compile" w5.zl >/dev/null 2>&1 &&   gcc -O2 -D_strdup=strdup -I"$OLDPWD" -o w5.bin out.c "$OLDPWD/runtime.c" "$OLDPWD/os_linux.c" -lm 2>/dev/null )
if [ -x "$tmp/w5.bin" ]; then
    "$tmp/w5.bin" > "$tmp/w5.c.out" 2>&1
    if diff -q "$tmp/w5.interp" "$tmp/w5.c.out" >/dev/null; then
        echo "  ok    C backend matches on raw memory"
    else
        echo "  DIFF  C backend disagrees on raw memory"; fail=1
    fi
else
    echo "  BUILD FAIL w5 C backend"; fail=1
fi
# design_memory_structs.md is explicit: bare peek/poke must STAY simulated,
# so no existing program silently starts writing memory.
printf 'poke(1000, 65)!
' > "$tmp/bare.zl"
if ./interp "$tmp/bare.zl" 2>&1 | grep -q "\[sim\]"; then
    echo "  ok    bare peek/poke still simulated (per design doc)"
else
    echo "  FAIL  bare poke became real - the design doc forbids this"; fail=1
fi

# design_kernel.md §2: a zl number is a double, so a bit pattern above 2^53
# cannot round-trip. That is how a GDT entry gets silently corrupted. The
# guard must REFUSE, and the documented two-halves workaround must work.
printf 'p = alloc(16)\npoke64(p, 0x00AF9A000000FFFF)\n' > "$tmp/big.zl"
./interp "$tmp/big.zl" > "$tmp/big.out" 2>&1 || true
if grep -q "above 2\^53" "$tmp/big.out"; then
    echo "  ok    poke64 refuses >2^53 instead of corrupting"
else
    echo "  FAIL  poke64 silently accepted a >2^53 value"; cat "$tmp/big.out"; fail=1
fi
printf 'p = alloc(16)\npoke32(p, 0x0000FFFF)\npoke32(p + 4, 0x00AF9A00)\nprint(peek32(p))\nprint(peek32(p + 4))\n' > "$tmp/halves.zl"
if [ "$(./interp "$tmp/halves.zl" 2>&1 | tr '\n' ' ')" = "65535 11508224 " ]; then
    echo "  ok    two-halves descriptor write works (the documented fix)"
else
    echo "  FAIL  two-halves workaround"; fail=1
fi

echo "== freestanding: zl with NO libc (the kernel-track proof) =="
if ./freestanding/build.sh freestanding/demo.zl "$tmp/fs.bin" >"$tmp/fs.build" 2>&1; then
    undef=$(nm -u "$tmp/fs.bin" 2>/dev/null | wc -l)
    if [ "$undef" -eq 0 ]; then
        echo "  ok    binary has 0 undefined symbols (genuinely libc-free)"
    else
        echo "  FAIL  $undef undefined symbols - not freestanding"; fail=1
    fi
    # capture first: ldd exits non-zero for a STATIC binary, and
    # `set -o pipefail` would fail the pipeline even though grep matched.
    ldd "$tmp/fs.bin" > "$tmp/fs.ldd" 2>&1 || true
    if grep -q "not a dynamic executable" "$tmp/fs.ldd"; then
        echo "  ok    statically linked, no dynamic loader"
    else
        echo "  FAIL  binary is dynamically linked"; fail=1
    fi
    "$tmp/fs.bin" > "$tmp/fs.out" 2>&1
    if diff -q <(./interp freestanding/demo.zl 2>&1) "$tmp/fs.out" >/dev/null; then
        echo "  ok    freestanding output matches the interpreter"
    else
        echo "  DIFF  freestanding output differs from the reference"; fail=1
    fi
else
    echo "  BUILD FAIL freestanding"; tail -3 "$tmp/fs.build"; fail=1
fi
# the kernel target (serial instead of syscalls) must also compile clean
if gcc -O2 -ffreestanding -nostdlib -fno-stack-protector -fno-pic -no-pie \
       -DZL_KERNEL_SERIAL -c -I. freestanding/runtime_kernel.c -o "$tmp/rk.o" 2>/dev/null; then
    echo "  ok    kernel/serial target compiles (the seam swaps cleanly)"
else
    echo "  FAIL  kernel/serial target does not compile"; fail=1
fi

# The kernel is a separate gate (design_kernel.md §10) but a broken build
# should surface here too. Skipped cleanly if QEMU is not installed.
if command -v qemu-system-i386 >/dev/null; then
    echo "== kernel: boots in QEMU and matches its golden transcript =="
    if kout=$(./kernel/verify.sh 2>&1); then
        echo "  $kout"
    else
        echo "  FAIL  kernel gate"; echo "$kout" | head -8; fail=1
    fi
    if command -v nasm >/dev/null; then
        if rout=$(./kernel/verify-raw.sh 2>&1); then
            echo "  $rout"
        else
            echo "  FAIL  raw-bootloader gate"; echo "$rout" | head -8; fail=1
        fi
    fi
fi

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

echo "== unboxed C backend (compilef, ARCHIVED): numeric-subset smoke test =="
cat > "$tmp/f_smoke.zl" <<'EOF'
fn fib(n) {
    if n < 2 { return n }
    return fib(n - 1) + fib(n - 2)
}
print(fib(25))
EOF
( cd "$tmp" && "$OLDPWD/compilef" f_smoke.zl >/dev/null 2>&1 && gcc -O2 -o f_smoke.bin outf.c 2>/dev/null )
if [ -x "$tmp/f_smoke.bin" ]; then
    "$tmp/f_smoke.bin" > "$tmp/f_smoke.out" 2>&1
    ./interp "$tmp/f_smoke.zl" > "$tmp/f_smoke.interp" 2>&1
    if diff -q "$tmp/f_smoke.interp" "$tmp/f_smoke.out" >/dev/null; then
        echo "  ok    compilef matches interpreter"
    else
        echo "  DIFF  compilef output mismatch"; fail=1
    fi
else
    echo "  BUILD FAIL compilef"; fail=1
fi

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
# bools must print as true/false, not 1/0 - this backend had no bool type
# and disagreed with the interpreter on something as ordinary as 1 > 0.
print(1 > 0)
print(2 == 3)
print(true)
print(false)
print(not true)
print(1 < 2 and 3 > 2)
print("a" == "a")
b = 5 > 3
print(b)
# bool-returning builtins through the bridge
print(has("hello world", "world"))
print(starts("hello", "he"))
print(ends("hello", "lo"))
# LIST arguments through the bridge, both from a variable and inline
xs = [1, 2, 3, 4]
print(sum(xs))
print(contains(xs, 2))
print(index_of(xs, 3))
print(join(["a", "b", "c"], "-"))
print(sum([5, 6, 7]))
# list-RETURNING builtins: the Value list is unboxed back into a zlist
print(split("x-y-z", "-"))
print(lines("p\nq"))
print(range(5))
print(range(2, 6))
print(sort([3, 1, 2]))
print(sort(["c", "a", "b"]))
print(reverse([1, 2, 3]))
print(take([1, 2, 3, 4, 5], 2))
print(drop([1, 2, 3, 4, 5], 3))
print(reverse(sort([3, 1, 2])))
print(len(split("a,b,c", ",")))
EOF
if command -v clang >/dev/null; then
    # runtime.c/os_linux.c must be linked: any bridged builtin (has, sum,
    # join, ...) calls back into the boxed runtime through the zlx_ bridge.
    ( cd "$tmp" && "$OLDPWD/compilel" llvm_smoke.zl >/dev/null 2>&1 && \
      clang -O2 out.ll "$OLDPWD/runtime.c" "$OLDPWD/os_linux.c" -I"$OLDPWD" \
            -D_strdup=strdup -o llvm_smoke.bin -lm 2>/dev/null )
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

echo "== nativegen kernel intrinsics (the gcc-replacement path) =="
# peek/poke must round-trip through real memory. Write 65, 66 and 1000 into
# the scratch region, read them back, and exit with the low byte of the sum.
cat > "$tmp/intr.zl" <<'EOF'
buf = 0x420000
poke8(buf, 65)
poke8(buf + 1, 66)
poke32(buf + 4, 1000)
exit(peek8(buf) + peek8(buf + 1) + peek32(buf + 4))
EOF
( cd "$tmp" && "$OLDPWD/nativegen" intr.zl >/dev/null 2>&1 )
"$tmp/native_out" 2>/dev/null; icode=$?
if [ "$icode" -eq 107 ]; then          # (65+66+1000) & 255
    echo "  ok    nativegen peek/poke round-trip through real memory"
else
    echo "  FAIL  nativegen peek/poke (exit $icode, expected 107)"; fail=1
fi
# inb/outb are privileged and cannot run in user mode, so assert the emitted
# machine code is correct rather than executing it.
cat > "$tmp/pio.zl" <<'EOF'
x = inb(0x64)
outb(0x60, 255)
exit(0)
EOF
( cd "$tmp" && "$OLDPWD/nativegen" pio.zl >/dev/null 2>&1 )
dis=$(objdump -D -b binary -m i386:x86-64 -M intel "$tmp/native_out" 2>/dev/null)
if grep -q "in .*al,dx" <<<"$dis" && grep -q "out .*dx,al" <<<"$dis"; then
    echo "  ok    nativegen emits correct in/out port instructions"
else
    echo "  FAIL  nativegen port I/O encoding"; fail=1
fi

echo
if [ "$fail" -eq 0 ]; then
    echo "ALL GREEN"
else
    echo "SOME CHECKS FAILED"
fi
exit $fail
