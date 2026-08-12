#!/usr/bin/env bash
# build.sh - builds every host tool of the zl-linux toolchain.
# Linux equivalent of the original project's build.bat (which needed MSVC).
set -euo pipefail
cd "$(dirname "$0")"

CC=${CC:-gcc}
CFLAGS="-O2 -Wall -D_strdup=strdup"

echo "==> interp    (tree-walking interpreter)"
$CC $CFLAGS -DBUILD_PARSER -DBUILD_INTERP -o interp lexer.c parser.c interp.c runtime.c os_linux.c -lm

echo "==> compile   (zl -> C -> gcc -> native ELF)"
$CC $CFLAGS -DBUILD_PARSER -DBUILD_INTERP -o compile compile.c lexer.c parser.c -lm

echo "==> nativegen (zl -> hand-assembled x86-64 -> ELF, no C compiler)"
$CC $CFLAGS -DBUILD_PARSER -DBUILD_INTERP -o nativegen nativegen.c lexer.c parser.c -lm

echo "==> compilel  (zl -> LLVM IR -> clang -> native ELF)"
$CC $CFLAGS -DBUILD_PARSER -DBUILD_INTERP -o compilel compilel.c lexer.c parser.c -lm

echo "==> lexer, parser (standalone demo binaries)"
$CC $CFLAGS -o lexer_demo  lexer.c  -lm
$CC $CFLAGS -DBUILD_PARSER -o parser_demo parser.c lexer.c -lm

echo
echo "built: interp compile compilel nativegen lexer_demo parser_demo"
echo
echo "usage:"
echo "  ./interp program.zl                  # run directly"
echo "  ./compile program.zl && gcc -O2 -D_strdup=strdup -o program out.c runtime.c os_linux.c -lm"
echo "  ./compilel program.zl && clang -O2 out.ll -o program   # unboxed subset"
echo "  ./nativegen program.zl               # integer subset only -> ./native_out"
