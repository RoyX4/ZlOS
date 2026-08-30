#!/usr/bin/env bash
# build.sh - builds every host tool of the zl-linux toolchain.
# Linux equivalent of the original project's build.bat (which needed MSVC).
set -euo pipefail
cd "$(dirname "$0")"

CC=${CC:-gcc}
CFLAGS="-O2 -Wall -D_strdup=strdup -I. -Isrc/frontend -Isrc/runtime"

FRONTEND="src/frontend/lexer.c src/frontend/parser.c"
RUNTIME="src/runtime/runtime.c src/runtime/os_linux.c"

echo "==> interp    (tree-walking interpreter)"
$CC $CFLAGS -DBUILD_PARSER -DBUILD_INTERP -o interp $FRONTEND src/runtime/interp.c $RUNTIME -lm

echo "==> compile   (zl -> boxed C -> gcc -> native ELF)   [ARCHIVED]"
$CC $CFLAGS -DBUILD_PARSER -DBUILD_INTERP -o compile src/backends/c/compile.c $FRONTEND -lm

echo "==> compilef  (zl -> unboxed C -> gcc -> native ELF) [ARCHIVED]"
$CC $CFLAGS -DBUILD_PARSER -DBUILD_INTERP -o compilef src/backends/c/compilef.c $FRONTEND -lm

echo "==> nativegen (zl -> hand-assembled x86-64 -> ELF, no C compiler)"
$CC $CFLAGS -DBUILD_PARSER -DBUILD_INTERP -o nativegen src/backends/native/nativegen.c $FRONTEND -lm

echo "==> compilel  (zl -> LLVM IR -> clang -> native ELF)  [SPEED BACKEND]"
$CC $CFLAGS -DBUILD_PARSER -DBUILD_INTERP -o compilel src/backends/llvm/compilel.c $FRONTEND -lm

echo "==> zlfmt     (re-indenter; --check is gate-able)"
$CC $CFLAGS -DBUILD_PARSER -o zlfmt src/tools/zlfmt.c src/frontend/lexer.c -lm

echo "==> lexer, parser (standalone demo binaries)"
$CC $CFLAGS -o lexer_demo src/frontend/lexer.c -lm
$CC $CFLAGS -DBUILD_PARSER -o parser_demo $FRONTEND -lm

# compile_commands.json - so clangd / VS Code IntelliSense resolve the
# BUILD_PARSER / BUILD_INTERP / _strdup defines the same way the real build
# does. Without it every file shows phantom errors in the editor.
echo "==> compile_commands.json (for clangd / IntelliSense)"
{
    printf '[\n'
    first=1
    for f in \
        src/frontend/lexer.c \
        src/frontend/parser.c \
        src/runtime/interp.c \
        src/runtime/runtime.c \
        src/runtime/os_linux.c \
        src/backends/c/compile.c \
        src/backends/c/compilef.c \
        src/backends/llvm/compilel.c \
        src/backends/native/nativegen.c; do
        [ -f "$f" ] || continue
        [ $first -eq 1 ] || printf ',\n'
        first=0
        printf '  {"directory": "%s", "file": "%s/%s", "command": "%s %s -DBUILD_PARSER -DBUILD_INTERP -c %s"}' \
            "$PWD" "$PWD" "$f" "$CC" "$CFLAGS" "$f"
    done
    printf '\n]\n'
} > compile_commands.json

echo
echo "built: interp compile compilef compilel nativegen zlfmt lexer_demo parser_demo"
echo
echo "usage:"
echo "  ./interp program.zl                  # run directly"
echo "  ./compile program.zl && gcc -O2 -D_strdup=strdup -Isrc/runtime -o program out.c src/runtime/runtime.c src/runtime/os_linux.c -lm"
echo "  ./compilel program.zl && clang -O2 out.ll -o program   # unboxed subset"
echo "  ./nativegen program.zl               # integer subset only -> ./native_out"
