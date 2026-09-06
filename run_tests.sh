#!/usr/bin/env bash
# run_tests.sh - Linux equivalent of the original project's run_tests.ps1.
#
# 1. Runs every tests/*.zl through the interpreter (the full language).
# 2. Cross-checks that the C backend (compile -> gcc) produces byte-identical
#    output to the interpreter for every one of those files.
# 3. Cross-checks the hand-assembled native x86-64 backend (integer subset
#    only) against the interpreter on a small integer-only smoke program.
# 4. Checks the self-hosting fixpoint (./verify_selfhost.sh) - compiler.zl
#    compiled by the interpreter must compile itself to byte-identical output.
set -uo pipefail
cd "$(dirname "$0")" || exit

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

echo "== semantic refusal and evaluation-order regressions =="
cat > "$tmp/too_long.zl" <<'EOF'
print("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa")
EOF
./interp "$tmp/too_long.zl" >"$tmp/too_long.interp" 2>&1 && long_i=0 || long_i=$?
./compile "$tmp/too_long.zl" >"$tmp/too_long.compile" 2>&1 && long_c=0 || long_c=$?
if [ "$long_i" -ne 0 ] && [ "$long_c" -ne 0 ] &&
   grep -q "string literal exceeds" "$tmp/too_long.interp" &&
   grep -q "string literal exceeds" "$tmp/too_long.compile"; then
    echo "  ok    over-limit string literals are refused, never truncated"
else
    echo "  FAIL  over-limit string literal was accepted or changed silently"; fail=1
fi

cat > "$tmp/for_nonlist.zl" <<'EOF'
for x in 7 { print(x) }
EOF
./interp "$tmp/for_nonlist.zl" >"$tmp/for_nonlist.interp" 2>&1 || true
( cd "$tmp" && "$OLDPWD/compile" for_nonlist.zl >/dev/null 2>&1 &&
  gcc -O2 -D_strdup=strdup -I"$OLDPWD/src/runtime" -o for_nonlist.bin out.c \
      "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c" -lm 2>/dev/null )
"$tmp/for_nonlist.bin" >"$tmp/for_nonlist.compiled" 2>&1 || true
if grep -q "'for' can only loop over a list" "$tmp/for_nonlist.interp" &&
   grep -q "'for' can only loop over a list" "$tmp/for_nonlist.compiled"; then
    echo "  ok    interpreter and C backend both reject for over a non-list"
else
    echo "  FAIL  for-over-non-list semantics diverged"; fail=1
fi

# F-4 (2026-09-04): a missing argument used to silently bind to a same-named
# GLOBAL in the interpreter (L/N gave 0, C already failed at gcc, just with
# no message naming the problem) - interp now refuses it directly, with a
# message; C's refusal is still gcc's own (compile.c emits a call with the
# wrong argument count and lets the C compiler say so). compilel/nativegen
# are checked in their own smoke-test sections below.
cat > "$tmp/arity.zl" <<'EOF'
fn f(a, b) { return a - b }
print(f(1))
EOF
./interp "$tmp/arity.zl" >"$tmp/arity.interp" 2>&1 && arity_i=0 || arity_i=$?
( cd "$tmp" && "$OLDPWD/compile" arity.zl >/dev/null 2>&1 &&
  gcc -O2 -D_strdup=strdup -I"$OLDPWD/src/runtime" -o arity.bin out.c \
      "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c" -lm ) >"$tmp/arity.compile" 2>&1
arity_c=$?
if [ "$arity_i" -ne 0 ] && grep -q "f expects 2 arguments, got 1" "$tmp/arity.interp" &&
   [ "$arity_c" -ne 0 ]; then
    echo "  ok    a wrong-arity call is refused (interp names it, C fails at gcc)"
else
    echo "  FAIL  wrong-arity call was accepted or changed silently"; fail=1
fi

cat > "$tmp/stdlib_regressions.zl" <<'EOF'
import strx
import listx
print(word_count(""))
print(word_count("a  b"))
print(chunk([1, 2, 3], 2))
EOF
if [ "$(./interp "$tmp/stdlib_regressions.zl" 2>&1 | tr '\n' ' ')" = "0 2 [[1, 2], [3]] " ] &&
   ./interp stdlib/json_parse.zl >/dev/null 2>&1 &&
   ./interp stdlib/json_pretty.zl >/dev/null 2>&1; then
    echo "  ok    stdlib edge contracts and JSON self-tests"
else
    echo "  FAIL  stdlib edge or JSON regression"; fail=1
fi

# F-5 (2026-09-04): call_builtin's/runtime.c's args[] is exactly nargs long
# and never zeroed, so a builtin that checked its arguments' TYPES but not
# that they EXISTED read uninitialised heap - ASan-confirmed heap overflow
# for replace() and insert() with too few arguments.
cat > "$tmp/underarg.zl" <<'EOF'
print(replace("a","b"))
EOF
./interp "$tmp/underarg.zl" >"$tmp/underarg.interp" 2>&1 && underarg_i=0 || underarg_i=$?
( cd "$tmp" && "$OLDPWD/compile" underarg.zl >/dev/null 2>&1 &&
  gcc -O2 -D_strdup=strdup -I"$OLDPWD/src/runtime" -o underarg.bin out.c \
      "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c" -lm &&
  ./underarg.bin ) >"$tmp/underarg.c" 2>&1
underarg_c=$?
if [ "$underarg_i" -ne 0 ] && grep -q "replace needs 3 arguments, got 2" "$tmp/underarg.interp" &&
   [ "$underarg_c" -ne 0 ] && grep -q "replace needs 3 arguments, got 2" "$tmp/underarg.c"; then
    echo "  ok    a too-few-arguments builtin call is refused, interp and C"
else
    echo "  FAIL  under-argument builtin call was accepted or changed silently"; fail=1
fi

# F-7 (2026-09-04): break/continue outside a loop used to compile silently
# and misbehave at run time (a break inside a called function stopped the
# CALLER's loop; a top-level break left the interpreter's g_breaking flag
# set for the next block). Now it is a parse-time refusal in every engine
# that shares the frontend.
cat > "$tmp/break_in_fn.zl" <<'EOF'
fn f() { break }
EOF
./interp "$tmp/break_in_fn.zl" >"$tmp/break_in_fn.interp" 2>&1 && bif_i=0 || bif_i=$?
./compile "$tmp/break_in_fn.zl" >"$tmp/break_in_fn.compile" 2>&1 && bif_c=0 || bif_c=$?
if [ "$bif_i" -ne 0 ] && grep -q "'break' outside a loop" "$tmp/break_in_fn.interp" &&
   [ "$bif_c" -ne 0 ] && grep -q "'break' outside a loop" "$tmp/break_in_fn.compile"; then
    echo "  ok    'break' outside a loop is a parse error, interp and compile"
else
    echo "  FAIL  break-outside-a-loop was accepted or changed silently"; fail=1
fi

# F-9 (2026-09-04): parse_unary() and parse_not() recurse on THEMSELVES for
# a chain of `-`/`not` without ever passing back through parse_expr(), so
# interp.c's bracket-only pre-scan could not see them - 30000 `-` or 20000
# `not` used to segfault the interpreter (200 is already well past the
# depth limit of 64, no need to go anywhere near the numbers that found it).
python3 -c "print('print(' + '-' * 200 + '1)')" > "$tmp/deep_dash.zl" 2>/dev/null ||
    { printf 'print('; for _ in $(seq 1 200); do printf -- '-'; done; printf '1)\n'; } > "$tmp/deep_dash.zl"
python3 -c "print('print(' + 'not ' * 200 + 'true)')" > "$tmp/deep_not.zl" 2>/dev/null ||
    { printf 'print('; for _ in $(seq 1 200); do printf 'not '; done; printf 'true)\n'; } > "$tmp/deep_not.zl"
deep_ok=1
for f in deep_dash deep_not; do
    timeout 5 ./interp "$tmp/$f.zl" >"$tmp/$f.interp" 2>&1; ec_i=$?
    timeout 5 ./compile "$tmp/$f.zl" >"$tmp/$f.compile" 2>&1; ec_c=$?
    if [ "$ec_i" -eq 139 ] || [ "$ec_i" -eq 134 ] || [ "$ec_c" -eq 139 ] || [ "$ec_c" -eq 134 ] ||
       ! grep -q "nested too deeply" "$tmp/$f.interp" || ! grep -q "nested too deeply" "$tmp/$f.compile"; then
        deep_ok=0
        echo "  FAIL  $f: interp exit=$ec_i compile exit=$ec_c (want a clean refusal, not a crash)"
    fi
done
if [ "$deep_ok" -eq 1 ]; then
    echo "  ok    200-deep unary '-' and 'not' chains are refused cleanly, interp and compile"
else
    fail=1
fi

# F-15 (2026-09-04): fopen("rb") on a directory succeeds on Linux, and
# ftell on the resulting stream can return -1 - (size_t)-1 + 1 wraps to 0,
# so read() used to hand back a zero/undersized buffer it then wrote a
# NUL terminator into regardless, instead of refusing a non-regular-file.
cat > "$tmp/read_dir.zl" <<'EOF'
print(read("/tmp"))
EOF
./interp "$tmp/read_dir.zl" >"$tmp/read_dir.interp" 2>&1 && readdir_i=0 || readdir_i=$?
( cd "$tmp" && "$OLDPWD/compile" read_dir.zl >/dev/null 2>&1 &&
  gcc -O2 -D_strdup=strdup -I"$OLDPWD/src/runtime" -o read_dir.bin out.c       "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c" -lm &&
  ./read_dir.bin ) >"$tmp/read_dir.c" 2>&1
readdir_c=$?
if [ "$readdir_i" -ne 0 ] && grep -q "not a regular file" "$tmp/read_dir.interp" &&
   [ "$readdir_c" -ne 0 ] && grep -q "not a regular file" "$tmp/read_dir.c"; then
    echo "  ok    read() of a directory is refused, interp and C"
else
    echo "  FAIL  read() of a directory was accepted or changed silently"; fail=1
fi

# F-16 (2026-09-04): kill(n) forwarded ANY pid straight to kill(2) with
# SIGTERM - kill(-1) sends SIGTERM to every process this user owns. This
# must be refused BEFORE any syscall, so the check below never actually
# calls kill(-1) - it only asserts the REFUSAL message, which is the only
# thing that can be checked here without risking the box it runs on.
cat > "$tmp/kill_neg.zl" <<'EOF'
kill(-1)
EOF
./interp "$tmp/kill_neg.zl" >"$tmp/kill_neg.interp" 2>&1 && killneg_i=0 || killneg_i=$?
if [ "$killneg_i" -ne 0 ] && grep -qi "refus" "$tmp/kill_neg.interp"; then
    echo "  ok    kill(-1) is refused before any syscall"
else
    echo "  FAIL  kill(-1) was accepted or refused without saying so"; fail=1
fi

echo "== C backend: cross-check against interpreter =="
for t in tests/*.zl; do
    name=$(basename "$t" .zl)
    ( cd "$tmp" && "$OLDPWD/compile" "$OLDPWD/$t" >/dev/null 2>&1 && \
      gcc -O2 -D_strdup=strdup -I"$OLDPWD/src/runtime" -o "$name.bin" out.c "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c" -lm 2>"$name.cc.err" )
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

# F-8 (2026-09-04): parse() used to reset g_hidden=0 for EVERY parse,
# including the nested one that loads an imported module - so a module's
# own for-range loop minted the same __zl_lim0/__zl_st0 hidden globals the
# importing program's for-range loops already used. mx_count()'s inner
# `for j = 1 to 5` then clobbered the outer `for i = 1 to 3`'s own counter
# every time it was called from inside that loop's body: 5 rows instead of
# 3, in both the interpreter and the C backend.
mkdir -p "$tmp/f8"
cat > "$tmp/f8/modx.zl" <<'EOF'
fn mx_count() {
    n = 0
    for j = 1 to 5 { n = n + 1 }
    return n
}
EOF
cat > "$tmp/f8/e07_import_collide.zl" <<'EOF'
import modx
for i = 1 to 3 {
    print(i, mx_count())
}
print("end")
EOF
f8_want="1 5
2 5
3 5
end"
f8_i=$( ( cd "$tmp/f8" && "$OLDPWD/interp" e07_import_collide.zl ) 2>&1 )
( cd "$tmp/f8" && "$OLDPWD/compile" e07_import_collide.zl >/dev/null 2>&1 &&
  gcc -O2 -D_strdup=strdup -I"$OLDPWD/src/runtime" -o e07.bin out.c \
      "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c" -lm )
f8_c=$( cd "$tmp/f8" && ./e07.bin 2>&1 )
if [ "$f8_i" = "$f8_want" ] && [ "$f8_c" = "$f8_want" ]; then
    echo "  ok    an imported module's for-range does not clobber the caller's, exactly 3 rows"
else
    echo "  FAIL  import for-range collision (interp: $f8_i | C: $f8_c)"; fail=1
fi

echo "== F-1: run declines cleanly on a lex/parse error (kernel exec.c) =="
# exec.c's own hosttest (kernel/tests/host/exectest.c) deliberately links
# NEITHER the real lexer/parser/interp NOR fs.c's weak symbols - it is
# testing the "interpreter not linked yet" (EX_LOADED) contract, and wiring
# the real frontend in there would break every one of those assertions.
# So this harness is a second, smaller one: exec.c UNMODIFIED, the REAL
# hosted lexer.c/parser.c/interp.c/runtime.c, and a fake single-file
# "filesystem" - the same seam trick exectest.c uses, just pointed at a real
# frontend instead of leaving it unlinked. Before F-1, a syntax error in the
# "file" called exit(1) inside die()/parse_error() - fatal under
# ZL_FREESTANDING (exit is k_exit(), which kfatals then spins forever) and
# merely abrupt here (it would kill THIS TEST PROCESS, not just fail an
# assertion). After F-1, zl_parse_guarded/zl_lex_guarded catch it and exec.c
# takes its EX_FAIL decline path exactly like a too-deep-to-parse program
# already did.
cat > "$tmp/f1_exec_test.c" <<'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned int u32;
static char typed_arg[256];
static char said_buf[4096]; static size_t said_n;
const char *term_argstr(void) { return typed_arg; }
void term_say(const char *s) { size_t n = strlen(s);
    if (said_n + n < sizeof(said_buf)) { memcpy(said_buf + said_n, s, n); said_n += n; said_buf[said_n] = 0; } }
static unsigned long cap = 1 << 20, avail;
unsigned long arena_capacity(void)  { return cap; }
unsigned long arena_available(void) { return avail; }
unsigned long arena_base_addr(void) { return 0; }
void *arena_alloc(unsigned long n)  { return malloc(n ? n : 1); }
void arena_reset(void)              { avail = cap; }
void fb_text_prop(int x, int y, const char *s, unsigned int rgb) { (void)x; (void)y; (void)s; (void)rgb; }
int  fb_text_prop_w(const char *s) { (void)s; return 0; }
int  fb_text_prop_h(void) { return 16; }
static const char *fake_name = "bad.zl";
static const char *fake_src;                 /* the "file's" content       */
int fs_mounted(void)              { return 1; }
int fs_find(const char *n)        { return (n && !strcmp(n, fake_name)) ? 1 : -1; }
u32 fs_size(int idx)              { return idx == 1 ? (u32)strlen(fake_src) : 0; }
int fs_read(int idx, void *dst, u32 max) {
    if (idx != 1) return -1;
    u32 n = (u32)strlen(fake_src); if (n > max) n = max;
    memcpy(dst, fake_src, n); return (int)n;
}
extern int exec_run(void);
extern int exec_state(void);
#define EX_FAIL 9
#define EX_RAN  8
int main(void) {
    avail = cap;
    /* 1) a LEX error: an unterminated string */
    fake_src = "print(\"x";
    snprintf(typed_arg, sizeof typed_arg, "%s", fake_name);
    int st = exec_run();
    if (st != EX_FAIL) { printf("FAIL lex error -> state %d, want EX_FAIL(9)\n", st); return 1; }
    if (!strstr(said_buf, "run:") || !strstr(said_buf, "lexer refused")) {
        printf("FAIL lex error message: %s\n", said_buf); return 1;
    }
    /* 2) a PARSE error: an unmatched brace */
    fake_src = "if true { print(1)";
    said_n = 0; said_buf[0] = 0;
    st = exec_run();
    if (st != EX_FAIL) { printf("FAIL parse error -> state %d, want EX_FAIL(9)\n", st); return 1; }
    /* 3) proof this isn't just "always EX_FAIL": a good program still runs */
    fake_src = "print(1 + 2)";
    said_n = 0; said_buf[0] = 0;
    st = exec_run();
    if (st != EX_RAN) { printf("FAIL good program -> state %d, want EX_RAN(8)\n", st); return 1; }
    printf("ok\n");
    return 0;
}
EOF
# interp.c's own main() is compiled out with -Dmain=... (it is only ever
# suppressed by -DZL_FREESTANDING otherwise, and this harness wants the
# hosted setjmp/longjmp path, not the kernel one - that path is separately
# proven to compile clean under the kernel's own -Wall -Wextra -Werror -m32
# -ffreestanding flags, see the F-1 report).
if cc -O0 -w -c -DBUILD_INTERP -Dmain=zl_unused_main -D_strdup=strdup \
      -Isrc/frontend -Isrc/runtime -o "$tmp/f1_interp.o" src/runtime/interp.c 2>"$tmp/f1_exec_test.err" &&
   cc -O0 -w -o "$tmp/f1_exec_test" "$tmp/f1_exec_test.c" kernel/src/core/exec.c \
      src/frontend/lexer.c src/frontend/parser.c "$tmp/f1_interp.o" \
      src/runtime/runtime.c src/runtime/os_linux.c \
      -DBUILD_PARSER -DBUILD_INTERP -D_strdup=strdup -Isrc/frontend -Isrc/runtime -lm 2>>"$tmp/f1_exec_test.err"; then
    # the harness's stdout/stderr also carries the REAL die()/parse_error()
    # messages and the real print(1+2) output ("3") - only its own verdict
    # line, printed last, says whether the assertions passed.
    out=$(timeout 5 "$tmp/f1_exec_test" 2>&1)
    if [ "$(printf '%s\n' "$out" | tail -1)" = "ok" ]; then
        echo "  ok    a lex error, a parse error, and a good program all decline/run correctly through run"
    else
        echo "  FAIL  F-1 exec harness"; echo "$out"; fail=1
    fi
else
    echo "  BUILD FAIL F-1 exec harness"; cat "$tmp/f1_exec_test.err"; fail=1
fi
# and the hosted CLI itself: a syntax error must still exit 1 with the
# message, unchanged from before F-1 (the trap is only armed inside
# zl_parse_guarded/zl_lex_guarded, which the hosted CLI's own main() does
# not call for a lexer error, and zi_killed distinguishes a real syntax
# error, exit 1, from the pre-existing too-deep-to-parse refusal, exit 2).
printf 'print("unterminated\n' > "$tmp/synerr.zl"
./interp "$tmp/synerr.zl" >"$tmp/synerr.out" 2>&1; synerr_rc=$?
if [ "$synerr_rc" -eq 1 ] && grep -q "string never closed" "$tmp/synerr.out"; then
    echo "  ok    a syntax error still exits 1 with the message"
else
    echo "  FAIL  syntax error exit code/message changed (rc=$synerr_rc)"; cat "$tmp/synerr.out"; fail=1
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
( cd "$tmp" && "$OLDPWD/compile" w5.zl >/dev/null 2>&1 &&   gcc -O2 -D_strdup=strdup -I"$OLDPWD/src/runtime" -o w5.bin out.c "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c" -lm 2>/dev/null )
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

echo "== C backend: an elif chain emits linear C =="
# The parser stores `elif` as an else block holding one N_IF; emitted as a
# nested `else { if ... }` one indent deeper per branch, the output was
# quadratic - 3,000 branches were 72.6 MB of C and a 30,000-branch probe filled
# a 7.7 GB tmpfs (2026-09-04). Flat `else if` is 566 KB for the same chain.
python3 - > "$tmp/elif_chain.zl" <<'EOF'
n = 2000
s = 'x = 5\nif x == 0 { print(0)'
for i in range(1, n): s += ' } elif x == %d { print(%d)' % (i, i)
print(s + ' } else { print(-1) }')
EOF
( cd "$tmp" && timeout 20 "$OLDPWD/compile" elif_chain.zl >/dev/null 2>&1 ); elif_rc=$?
elif_bytes=$(stat -c%s "$tmp/out.c" 2>/dev/null || echo 0)
if [ "$elif_rc" -eq 0 ] && [ "$elif_bytes" -gt 0 ] && [ "$elif_bytes" -lt 2000000 ]; then
    echo "  ok    2,000-branch elif chain compiles to $elif_bytes bytes of C (linear)"
else
    echo "  FAIL  2,000-branch elif chain: compile rc=$elif_rc, out.c $elif_bytes bytes (quadratic or hung)"; fail=1
fi
rm -f "$tmp/out.c"

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
        if rout=$(./kernel/tools/checks/verify-raw.sh 2>&1); then
            echo "  $rout"
        else
            echo "  FAIL  raw-bootloader gate"; echo "$rout" | head -8; fail=1
        fi
    fi
    # The native-EFI gate earns its minute, unlike the ISO one which is left
    # out for cost. Both gates above boot the 32-BIT kernel, and so does
    # verify-iso.sh's "UEFI" case (that one is GRUB's bootx64.efi loading it).
    # So without this, NOTHING here covers kernel/efi.c, the 64-bit build, or
    # the path a real laptop takes - and that gap already allowed a latent
    # boot-killer to sit in the tree while all the other gates stayed green:
    # struct idt_ptr/gdt_ptr were 6 bytes instead of 10 in the EFI build, so
    # lidt/lgdt took the top half of each base from adjacent memory.
    if [ -f /usr/share/OVMF/OVMF_CODE_4M.fd ] && command -v qemu-system-x86_64 >/dev/null; then
        if eout=$(./kernel/tools/checks/verify-efi.sh 2>&1); then
            echo "  ok    zlOS boots as its own UEFI application (64-bit)"
        else
            echo "  FAIL  native-EFI gate"; echo "$eout" | head -8; fail=1
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
      gcc -O2 -D_strdup=strdup -I"$OLDPWD/src/runtime" -o "ex_$name.bin" out.c "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c" -lm 2>"ex_$name.cc.err" )
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
      clang -O2 out.ll "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c" -I"$OLDPWD/src/runtime" \
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

# F-3 (2026-09-04): compilel and nativegen are integer-only backends, so a
# literal past 2^63 must be a REFUSAL, not the silent atoll() saturation to
# LLONG_MAX this used to be.
cat > "$tmp/big_lit.zl" <<'EOF'
print(10000000000000000000000000000000000000000)
EOF
if command -v clang >/dev/null; then
    ( cd "$tmp" && "$OLDPWD/compilel" big_lit.zl ) >"$tmp/big_lit.compilel.out" 2>&1
    if [ $? -ne 0 ] && grep -q "not exactly representable" "$tmp/big_lit.compilel.out"; then
        echo "  ok    compilel refuses a >2^63 literal"
    else
        echo "  FAIL  compilel accepted (or mis-refused) a >2^63 literal"; fail=1
    fi
fi
( cd "$tmp" && "$OLDPWD/nativegen" big_lit.zl ) >"$tmp/big_lit.nativegen.out" 2>&1
if [ $? -ne 0 ] && grep -q "not exactly representable" "$tmp/big_lit.nativegen.out"; then
    echo "  ok    nativegen refuses a >2^63 literal"
else
    echo "  FAIL  nativegen accepted (or mis-refused) a >2^63 literal"; fail=1
fi

# F-4 (2026-09-04): compilel and nativegen used to hand a missing argument 0
# with no diagnostic at all.
if command -v clang >/dev/null; then
    ( cd "$tmp" && "$OLDPWD/compilel" arity.zl ) >"$tmp/arity.compilel.out" 2>&1
    if [ $? -ne 0 ] && grep -q "f expects 2 arguments, got 1" "$tmp/arity.compilel.out"; then
        echo "  ok    compilel refuses a wrong-arity call"
    else
        echo "  FAIL  compilel accepted (or mis-refused) a wrong-arity call"; fail=1
    fi
fi
( cd "$tmp" && "$OLDPWD/nativegen" arity.zl ) >"$tmp/arity.nativegen.out" 2>&1
if [ $? -ne 0 ] && grep -q "f expects 2 arguments, got 1" "$tmp/arity.nativegen.out"; then
    echo "  ok    nativegen refuses a wrong-arity call"
else
    echo "  FAIL  nativegen accepted (or mis-refused) a wrong-arity call"; fail=1
fi

# F-10(b) (2026-09-04): llvm.fptosi.sat converts a NaN index to 0 (its own
# documented saturation behaviour), which is a valid index into any
# non-empty list - so xs[nan] used to silently read/write element 0
# instead of "list index out of range".
cat > "$tmp/idx_nan.zl" <<'EOF'
xs = [10, 20, 30]
i = 0 / 0
print(xs[i])
EOF
./interp "$tmp/idx_nan.zl" >"$tmp/idx_nan.interp" 2>&1
if command -v clang >/dev/null; then
    ( cd "$tmp" && "$OLDPWD/compilel" idx_nan.zl >/dev/null 2>&1 &&
      clang -O2 out.ll "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c"             -I"$OLDPWD/src/runtime" -D_strdup=strdup -o idx_nan.bin -lm 2>/dev/null &&
      ./idx_nan.bin ) >"$tmp/idx_nan.llvm" 2>&1
    if grep -q "list index out of range" "$tmp/idx_nan.interp" &&
       grep -q "list index out of range" "$tmp/idx_nan.llvm"; then
        echo "  ok    a NaN list index is refused, interp and LLVM"
    else
        echo "  FAIL  NaN list index was accepted (or refused differently)"; fail=1
    fi
fi

# F-10(c): unary '-' checked str/list but not bool, so `-true` silently
# became a bool carrying machine value -1 instead of refusing the way
# interp.c's "cannot negate a non-number" already does.
cat > "$tmp/neg_bool.zl" <<'EOF'
print(- true)
EOF
./interp "$tmp/neg_bool.zl" >"$tmp/neg_bool.interp" 2>&1
if command -v clang >/dev/null; then
    ( cd "$tmp" && "$OLDPWD/compilel" neg_bool.zl ) >"$tmp/neg_bool.llvm" 2>&1
    if grep -q "cannot negate a" "$tmp/neg_bool.interp" &&
       grep -q "cannot negate a bool" "$tmp/neg_bool.llvm"; then
        echo "  ok    negating a bool is refused, interp and LLVM"
    else
        echo "  FAIL  negating a bool was accepted (or refused differently)"; fail=1
    fi
fi

# F-10(e): '%' used to narrow an out-of-range/NaN operand via a raw
# (long long) cast, which is undefined behaviour in C - now routed
# through exact_i64, same policy as the bitwise builtins.
cat > "$tmp/mod_huge.zl" <<'EOF'
huge = pow(10, 300)
print(huge % 3)
EOF
./interp "$tmp/mod_huge.zl" >"$tmp/mod_huge.interp" 2>&1 && modhuge_i=0 || modhuge_i=$?
( cd "$tmp" && "$OLDPWD/compile" mod_huge.zl >/dev/null 2>&1 &&
  gcc -O2 -D_strdup=strdup -I"$OLDPWD/src/runtime" -o mod_huge.bin out.c       "$OLDPWD/src/runtime/runtime.c" "$OLDPWD/src/runtime/os_linux.c" -lm &&
  ./mod_huge.bin ) >"$tmp/mod_huge.c" 2>&1
modhuge_c=$?
if [ "$modhuge_i" -ne 0 ] && grep -q "% needs 64-bit integers" "$tmp/mod_huge.interp" &&
   [ "$modhuge_c" -ne 0 ] && grep -q "% needs 64-bit integers" "$tmp/mod_huge.c"; then
    echo "  ok    '%' of an out-of-range operand is refused, interp and C"
else
    echo "  FAIL  '%' of an out-of-range operand was accepted or changed silently"; fail=1
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
# F-6 (2026-09-04): print_int treated rax as unsigned, so a negative T_INT
# printed as a huge unsigned number instead of "-1".
print(0 - 1)
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

cat > "$tmp/native_arg16.zl" <<'EOF'
fn f(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) { return a + p }
i = 0
while i < 40000 {
    f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16)
    i = i + 1
}
print(i)
EOF
( cd "$tmp" && "$OLDPWD/nativegen" native_arg16.zl >/dev/null 2>&1 )
if [ "$("$tmp/native_out" 2>&1)" = "40000" ]; then
    echo "  ok    native backend reclaims 16-argument call frames"
else
    echo "  FAIL  native backend corrupts 16-argument call frames"; fail=1
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

# The self-hosting fixpoint. Last, because it is the slowest single check here
# (~35 s: the interpreter, then gcc -O2 on the generated C) and because it is
# the one whose failure means the LANGUAGE broke rather than one backend.
echo
# Absent rather than failing is still a failure: an unchecked fixpoint that
# reports green is the exact shape this repo keeps getting bitten by.
if [ -x ./verify_selfhost.sh ]; then
    ./verify_selfhost.sh || fail=1
else
    echo "  FAIL  verify_selfhost.sh is missing - the fixpoint is unchecked"; fail=1
fi

echo
if [ "$fail" -eq 0 ]; then
    echo "ALL GREEN"
else
    echo "SOME CHECKS FAILED"
fi
exit $fail
