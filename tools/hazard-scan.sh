#!/usr/bin/env bash
# hazard-scan.sh - mechanical gates for the bug classes CLAUDE.md records.
#
# Ladder rung 6: these are checks, not judgment calls. None of this should ever
# be asked of a model - a compiler and a regex are exact and cannot hallucinate.
#
# Where possible the check is FUNCTIONAL rather than textual: instead of reading
# buildefi.sh to see whether a guard is configured, we compile a probe and prove
# the guard actually fires. Checking that the smoke alarm is on the wall is not
# the same as pressing the test button - and in this repo the difference was 34
# live defects.
#
# Exit 1 on any hit. Run locally, or from .github/workflows/gates.yml.

set -uo pipefail
cd "$(dirname "$0")/.."

COUNT_ONLY=0
[ "${1:-}" = "--count" ] && COUNT_ONLY=1

fail=0
hit()  { echo "  HAZARD: $*"; fail=1; }
ok()   { [ "$COUNT_ONLY" -eq 1 ] || echo "  ok      $*"; }
skip() { [ "$COUNT_ONLY" -eq 1 ] || echo "  skip    $*"; }
warn() { [ "$COUNT_ONLY" -eq 1 ] || echo "  WARN:   $*"; }

tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT

# The exact CFLAGS buildefi.sh uses, parsed out of the script so this test can
# never drift from the real build.
efi_cflags() {
    sed -n '/^CF="/,/"$/p' kernel/buildefi.sh \
        | sed 's/^CF="//; s/"$//' | tr -d '\\' | tr '\n' ' '
}

[ "$COUNT_ONLY" -eq 1 ] || echo "== 1. the EFI warning guard must actually FIRE, not merely be configured =="
# buildefi.sh targets x86_64-unknown-windows, which is LLP64: unsigned long is 4
# bytes there, 8 everywhere else. Five pointer truncations once sat in the boot
# path because -w silenced the warnings that name this class.
#
# The four -Werror= flags are placed after -w on the theory that clang applies
# flags left to right. That is true of -Wno-<group>, but NOT of -w, which is a
# blanket suppression that wins regardless of position. Measured:
#
#   -w              -Werror=pointer-to-int-cast   -> exit 0   (guard inert)
#   -Wno-everything -Werror=pointer-to-int-cast   -> exit 1   (guard fires)
#
# So this check compiles a probe that MUST be rejected. If it compiles, the
# guard is decorative.
if command -v clang >/dev/null 2>&1; then
    cat > "$tmp/probe.c" <<'EOF'
int g;
unsigned long trunc(void) { return (unsigned long)&g; }
EOF
    read -r -a CFA <<< "$(efi_cflags)"
    if [ "${#CFA[@]}" -eq 0 ]; then
        skip "could not parse CF= out of kernel/buildefi.sh"
    elif clang "${CFA[@]}" -c "$tmp/probe.c" -o "$tmp/probe.o" >/dev/null 2>&1; then
        # Known-outstanding, so this warns rather than failing - otherwise every
        # PR is red for a defect it did not introduce, and a permanently red
        # gate is one nobody reads. Check 2 is what actually holds the line.
        warn "the EFI build ACCEPTS a 64-bit pointer truncated into 'unsigned long'."
        warn "  the four -Werror= flags in kernel/buildefi.sh are inert: -w is a"
        warn "  blanket suppression that a later -Werror= does not survive."
        warn "  fix: replace '-w' with '-Wno-everything' in CF (verified), then"
        warn "  repair the sites check 2 reports. See section 8b of"
        warn "  docs/design/ci-and-agent-pipeline.md."
    else
        ok "guard fires: a truncating pointer cast is rejected by the EFI flags"
    fi
else
    skip "clang not installed"
fi

[ "$COUNT_ONLY" -eq 1 ] || echo "== 2. count real truncation sites in the EFI translation units =="
# Uses the compiler as the oracle, not a regex - no false positives from integer
# arithmetic that merely mentions 'unsigned long'.
if command -v clang >/dev/null 2>&1; then
    read -r -a CFA <<< "$(efi_cflags)"
    # Force the guard on for counting, whatever buildefi.sh currently does.
    # Exact-element match only: substring replacement would turn -fshort-wchar
    # into -fshort-Wno-everythingchar, which clang rejects.
    for i in "${!CFA[@]}"; do
        [ "${CFA[$i]}" = "-w" ] && CFA[$i]="-Wno-everything"
    done
    files=$(sed -n '/^for f in/,/do$/p' kernel/buildefi.sh | tr ' \\' '\n\n' | grep '\.c$')
    total=0; bad=0
    # buildefi.sh runs from kernel/, and CF carries a relative -I.. — so this
    # must compile from there too or every include resolves to the wrong tree.
    pushd kernel >/dev/null || exit 1
    for f in $files; do
        [ -f "$f" ] || continue
        EXTRA=(); case "$(basename "$f")" in idt.c|apic.c) EXTRA=(-mgeneral-regs-only);; esac
        n=$(clang "${CFA[@]}" "${EXTRA[@]}" -c "$f" -o "$tmp/o.o" 2>&1 | grep -c 'error:')
        if [ "$n" -gt 0 ]; then
            [ "$COUNT_ONLY" -eq 1 ] || printf "     %-36s %s\n" "$f" "$n"
            total=$((total+n)); bad=$((bad+1))
        fi
    done
    popd >/dev/null
    if [ "$COUNT_ONLY" -eq 1 ]; then echo "$total"; exit 0; fi
    base=$(grep -E '^efi_truncation_sites=' tools/hazard-baseline.txt 2>/dev/null | cut -d= -f2)
    base=${base:-0}
    if [ "$total" -gt "$base" ]; then
        hit "$total truncation sites across $bad files - baseline is $base."
        hit "  this change ADDED $(( total - base )). Harmless below 4 GiB, which"
        hit "  is exactly why QEMU never shows them and why two shipped before."
    elif [ "$total" -gt 0 ]; then
        warn "$total truncation sites across $bad files (baseline $base, not worse)"
        [ "$total" -lt "$base" ] && warn "  improved - lower efi_truncation_sites to $total in tools/hazard-baseline.txt"
    else
        ok "no truncation sites in the EFI build"
    fi
else
    skip "clang not installed"
fi

echo "== 3. gates must poll for a marker, never race a wall clock =="
# A gate that fails for reasons unrelated to the code costs a bisect every time.
for g in kernel/verify*.sh; do
    [ -f "$g" ] || continue
    if grep -qE 'timeout' "$g" && ! grep -qE 'while|until|CEILING|kill -0' "$g"; then
        hit "$g uses timeout with no polling loop"
    fi
done
grep -qE 'CEILING|kill -0' kernel/verify-raw.sh 2>/dev/null && ok "verify*.sh poll rather than racing a clock"

echo "== 4. generated sources and build outputs must not be tracked =="
found=0
while IFS= read -r f; do
    [ -n "$f" ] && { hit "tracked build output: $f"; found=1; }
done < <(git ls-files 2>/dev/null \
         | grep -E '(^|/)(_gen[a-z0-9]*\.c|out\.c|out\.ll)$|\.(o|bin|elf|img|iso)$' || true)
[ "$found" -eq 0 ] && ok "no generated .c or build outputs tracked"

echo "== 5. the interpreter and the C backend must agree =="
# The LLVM backend is knowingly on the other side of the scoping decision and is
# deliberately NOT asserted here - see AGENTS.md section 3.
if [ -x ./interp ] && [ -x ./compile ]; then
    printf 'counter = 100\nfn bump() { counter = 7  return 0 }\nbump()\nprint(counter)\n' > "$tmp/s.zl"
    a=$(./interp "$tmp/s.zl" 2>&1)
    if ./compile "$tmp/s.zl" >/dev/null 2>&1 \
       && gcc -O2 -D_strdup=strdup -I. -o "$tmp/s.bin" out.c runtime.c os_linux.c -lm 2>/dev/null; then
        b=$("$tmp/s.bin" 2>&1)
        [ "$a" = "$b" ] && ok "interp and C backend agree (both '$a')" \
                        || hit "scoping divergence: interp '$a' vs C backend '$b'"
    else
        skip "C backend did not build"
    fi
else
    skip "./interp or ./compile not built (run ./build.sh)"
fi

echo
[ "$fail" -ne 0 ] && { echo "hazard-scan: FAILED"; exit 1; }
echo "hazard-scan: clean"
