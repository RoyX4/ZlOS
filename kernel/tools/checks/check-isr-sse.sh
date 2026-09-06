#!/usr/bin/env bash
# check-isr-sse.sh - nothing an interrupt handler CALLS may touch SSE/x87.
#
# WHY THIS EXISTS. idt.c and apic.c are built -mgeneral-regs-only, so a
# handler's own body never touches xmm. That flag guards ONE FILE and cannot
# see the handler's callees. input.c said "the gate checks that by
# disassembly" - there was no such gate. On 2026-09-04 the gcc 64-bit build of
# zllog_event_irq (reached from keyboard_isr's ring-full drop branch) contained
# four movd-to-xmm instructions: 256 unread scancodes and the next IRQ1 would
# have returned to the zl interpreter with xmm0-3 changed, where every number
# is a double. verify-64.sh boots that kernel with no keyboard attached, so no
# boot gate could have shown it.
#
# WHAT IT DOES. Compiles the handler files and the files their callees live
# in with build64.sh's exact flags, takes every `__attribute__((interrupt))`
# function (plus user64_timer_dispatch, which the hand-written IRQ0 stub calls
# in ring-0 context with no FPU save) as a root, follows the calls in each
# root's body three levels deep - reading RELOCATIONS, because under
# -mcmodel=large a call is `movabs $0,%rax; call *%rax` and the callee's name
# never appears as `<name>` - and fails if any reached function contains an
# xmm/ymm/zmm/x87 instruction. Two planted defects (one level, and two levels
# across translation units) run first so the check is known to bite.
#
# The syscall path (syscall_isr -> user64_dispatch) is deliberately NOT a
# root: it is entered from ring 3 only, and the entry stub saves the user's
# FPU state with fxsave before any C runs.
#
# No QEMU. About ten seconds. Run from anywhere.
set -u
cd "$(dirname "$0")/../.."          # kernel/

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT INT TERM

INCLUDES=$(find src boot -type d -printf ' -I%p' | sort)
CFLAGS="-m64 -O2 -ffreestanding -nostdlib -fno-stack-protector -fno-pic
        -fno-builtin -mno-red-zone -mcmodel=large -DZL_64
        -Wall -Wextra -Wno-unused-parameter
        -I.. -I../src/frontend -I../src/runtime $INCLUDES"

# The handler files, and every file a handler's callee may live in. Add a
# file here when a handler grows a call into a new subsystem; the check
# then reports "unresolved" for a callee it cannot find, which also fails.
HANDLERS="src/arch/x86/idt.c src/arch/x86/apic.c"
CALLEE_FILES="src/core/zllog.c src/arch/x86/support.c src/arch/x86/usermode.c
              src/arch/x86/cpu.c src/core/crash.c src/core/console.c
              src/drivers/input/input.c src/core/sched.c boot/gdt64.c"
EXTRA_ROOTS="user64_timer_dispatch"

compile() {   # $1 = source, $2 = extra flags, $3 = out
    # shellcheck disable=SC2086
    gcc $CFLAGS $2 -c "$1" -o "$3" 2>"$TMP/cc.err" || { echo "  compile failed: $1"; cat "$TMP/cc.err"; return 1; }
}

# the names of every __attribute__((interrupt)) function in the given sources
interrupt_roots() {   # the attribute may share the definition's line or precede it
    for f in "$@"; do
        awk 'function grab(s,   name) {
                 if (match(s, /[A-Za-z_][A-Za-z0-9_]*[[:space:]]*\(/)) {
                     name = substr(s, RSTART, RLENGTH); sub(/[[:space:]]*\($/, "", name);
                     if (name != "void" && name != "__attribute__") { print name; return 1 }
                 }
                 return 0 }
             /__attribute__\(\(interrupt\)\)/ { rest = $0; sub(/.*__attribute__\(\(interrupt\)\)/, "", rest);
                                                 if (grab(rest)) next; want = 1; next }
             want { if (grab($0)) want = 0 }' "$f"
    done | sort -u
}

# disassembly of one function inside one object, empty if not defined there
func_body() {   # $1 = object, $2 = symbol
    objdump -dr "$1" | awk -v s="<$2>:" '
        index($0, s) { on = 1; next }
        on && /^[0-9a-f]+ <.*>:$/ { exit }
        on { print }'
}

# every symbol a function body references, as <name> or as a relocation
body_callees() {   # stdin = body
    grep -oE '<[A-Za-z_][A-Za-z0-9_]*>|R_X86_64_[A-Z0-9]+[[:space:]]+[A-Za-z_][A-Za-z0-9_]*' |
        sed -E 's/^<//; s/>$//; s/^R_X86_64_[A-Z0-9]+[[:space:]]+//' | sort -u
}

SSE_RE='%[xyz]mm[0-9]|\bf(ld|st|add|sub|mul|div|xch|ninit|ldcw|nstcw)|\bmovap|\bmovup|\bmovs[sd] |\bcvt'

check_tree() {   # $1 = objects to search, $2 = root symbols; prints offenders
    local objs="$1" queue="$2" seen="" sym obj body found level=0
    # one symbol table over every object: a relocation to a DATA symbol (an
    # fxsave area, a counter) is not a call and is skipped; a root that this
    # build does not define (a 32-bit-only handler) is noted, not failed
    # shellcheck disable=SC2086
    nm -P $objs 2>/dev/null | awk '{print $1, $2}' | sort -u > "$TMP/symtab"
    while [ -n "$queue" ] && [ $level -lt 4 ]; do
        local next=""
        for sym in $queue; do
            case " $seen " in *" $sym "*) continue ;; esac
            seen="$seen $sym"
            if grep -qE "^$sym [BbDdRrCcGgSs]$" "$TMP/symtab"; then continue; fi   # data, not a call
            found=0
            for obj in $objs; do
                body=$(func_body "$obj" "$sym")
                [ -n "$body" ] || continue
                found=1
                if [ $level -gt 0 ] && printf '%s\n' "$body" | grep -qE "$SSE_RE"; then
                    echo "OFFENDER $sym in $(basename "$obj"):"
                    printf '%s\n' "$body" | grep -E "$SSE_RE" | head -4 | sed 's/^/    /'
                fi
                next="$next $(printf '%s\n' "$body" | body_callees | grep -vx "$sym")"
                break
            done
            if [ $found -eq 0 ]; then
                if [ $level -eq 0 ]; then echo "NOTE root $sym is not defined in this build (32-bit only?)" >&2
                else echo "UNRESOLVED $sym (add its file to CALLEE_FILES)"; fi
            fi
        done
        queue="$next"; level=$((level + 1))
    done
}

fail=0

echo "== A. planted defects: an xmm callee one level down, and two levels across TUs =="
cat > "$TMP/handler.c" <<'EOF'
extern void helper(unsigned a, unsigned b);
__attribute__((interrupt)) void isr(void *frame) { (void)frame; helper(1, 2); }
EOF
cat > "$TMP/helper.c" <<'EOF'
void helper(unsigned a, unsigned b) { __asm__ volatile("movd %0, %%xmm0" :: "r"(a + b) : "xmm0"); }
EOF
cat > "$TMP/handler2.c" <<'EOF'
extern void mid(unsigned a, unsigned b);
__attribute__((interrupt)) void isr2(void *frame) { (void)frame; mid(1, 2); }
EOF
cat > "$TMP/mid.c" <<'EOF'
extern void leaf(unsigned v);
void mid(unsigned a, unsigned b) { leaf(a + b); }
EOF
cat > "$TMP/leaf.c" <<'EOF'
void leaf(unsigned v) { __asm__ volatile("movd %0, %%xmm0" :: "r"(v) : "xmm0"); }
EOF
for f in handler helper handler2 mid leaf; do
    extra=""; case $f in handler*) extra="-mgeneral-regs-only" ;; esac
    # shellcheck disable=SC2086
    gcc $CFLAGS $extra -c "$TMP/$f.c" -o "$TMP/$f.o" || { echo "  planted compile failed: $f"; exit 1; }
done
if check_tree "$TMP/handler.o $TMP/helper.o" "$(interrupt_roots "$TMP/handler.c")" | grep -q '^OFFENDER helper'; then
    echo "  ok    planted xmm callee reported"
else
    echo "  FAIL  planted xmm callee NOT reported - the check is blind"; fail=1
fi
if check_tree "$TMP/handler2.o $TMP/mid.o $TMP/leaf.o" "$(interrupt_roots "$TMP/handler2.c")" | grep -q '^OFFENDER leaf'; then
    echo "  ok    planted two-level cross-TU xmm callee reported"
else
    echo "  FAIL  planted two-level xmm callee NOT reported - the walk is blind past level 1"; fail=1
fi

echo "== B. the real handlers and everything they reach =="
objs=""
for f in $HANDLERS; do
    o="$TMP/h_$(basename "$f" .c).o"
    compile "$f" "-mgeneral-regs-only" "$o" || exit 1
    objs="$objs $o"
done
for f in $CALLEE_FILES; do
    o="$TMP/c_$(basename "$f" .c).o"
    compile "$f" "" "$o" || exit 1
    objs="$objs $o"
done
# shellcheck disable=SC2086
roots="$(interrupt_roots $HANDLERS) $EXTRA_ROOTS"
echo "  roots: $(echo $roots | tr '\n' ' ')"
real=$(check_tree "$objs" "$roots")
if [ -n "$real" ]; then
    printf '%s\n' "$real" | sed 's/^/  /'
    echo "  FAIL  a function reachable from an interrupt handler uses SSE/x87, or a callee could not be resolved"
    fail=1
else
    echo "  ok    no SSE/x87 in any function reachable from the interrupt handlers (gcc -m64)"
fi

exit $fail
