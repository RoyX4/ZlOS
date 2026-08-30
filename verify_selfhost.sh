#!/usr/bin/env bash
# verify_selfhost.sh - the self-hosting fixpoint. ~35 s on this box (measured
# 2026-08-19 under load: 17 s of it is the interpreter, the rest is gcc -O2 on
# the 936-line generation-1 C). No QEMU, no hardware, no clang.
#
# THE PROPERTY IS f(f(x)) == f(x), NOT A FROZEN HASH.
#
#   gen1 = the interpreter compiling compiler.zl
#   gen2 = the binary built from gen1, compiling the same source
#
# gen1 and gen2 must be byte-identical. The hash itself is ALLOWED to change
# whenever compiler.zl changes - pinning it would make every legitimate edit to
# the compiler look like a regression, and people delete gates that cry wolf.
#
# WHY THIS EXISTS. The check used to live in verify.ps1, which did not survive
# the port from Windows. Nothing in run_tests.sh, gates/land-gate.sh or
# .github/workflows/ ran it, so from the port until 2026-08-19 the language's
# central property was unchecked. First measured run on Linux:
# 27e04c382694276d3001b57df144f001, 936 lines, identical both generations.
#
# WHAT THIS DOES *NOT* PROVE. compiler.zl is a 716-line SUBSET compiler - fn,
# if/else, while, for, and/or/not, return, lists, strings - emitting C. The
# production toolchain (interp.c, compile.c, compilel.c) is C. So a green run
# here means "the self-hosting subset still reaches its fixpoint", not "the zl
# toolchain is self-hosting". Do not let a doc upgrade that sentence.
set -u

R="$(cd "$(dirname "$0")" && pwd)"
cd "$R" || exit 1

for f in interp src/selfhost/compiler.zl src/runtime/runtime.c src/runtime/runtime.h src/runtime/os_linux.c src/runtime/os.h; do
    [ -e "$R/$f" ] || { echo "FAIL  missing $f - run ./build.sh first" >&2; exit 1; }
done

tmp=$(mktemp -d) || exit 1
trap 'rm -rf "$tmp"' EXIT

cp "$R/src/selfhost/compiler.zl" "$tmp/compiler.zl" || exit 1
cp "$R/src/runtime/"{runtime.c,runtime.h,os_linux.c,os.h} "$tmp/" || exit 1
# compiler.zl reads a fixed filename and writes a fixed one; feeding it its own
# source is what makes this a fixpoint rather than a smoke test.
cp "$tmp/compiler.zl" "$tmp/input.zl"

if ! ( cd "$tmp" && "$R/interp" compiler.zl >/dev/null 2>&1 ); then
    echo "FAIL  the interpreter could not run compiler.zl" >&2; exit 1
fi
[ -s "$tmp/out.c" ] || { echo "FAIL  generation 1 produced no out.c" >&2; exit 1; }
mv "$tmp/out.c" "$tmp/gen1.c"

if ! ( cd "$tmp" && gcc -O2 -w -D_strdup=strdup -o zlc1 gen1.c runtime.c os_linux.c -lm 2>/dev/null ); then
    echo "FAIL  generation 1 does not compile - the emitted C is broken" >&2; exit 1
fi

if ! ( cd "$tmp" && ./zlc1 >/dev/null 2>&1 ); then
    echo "FAIL  the generation-1 compiler could not compile compiler.zl" >&2; exit 1
fi
[ -s "$tmp/out.c" ] || { echo "FAIL  generation 2 produced no out.c" >&2; exit 1; }
mv "$tmp/out.c" "$tmp/gen2.c"

if cmp -s "$tmp/gen1.c" "$tmp/gen2.c"; then
    echo "  ok    self-hosting fixpoint: gen1 == gen2 ($(wc -l < "$tmp/gen1.c") lines, $(md5sum < "$tmp/gen1.c" | cut -c1-12))"
    exit 0
fi

echo "  FAIL  self-hosting fixpoint BROKEN - gen1 and gen2 differ" >&2
echo "        the compiler does not reproduce itself; first differences:" >&2
diff "$tmp/gen1.c" "$tmp/gen2.c" | head -20 >&2
exit 1
