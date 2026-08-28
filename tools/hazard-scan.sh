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
cd "$(dirname "$0")/.." || exit
REPO_ROOT=$PWD

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
    local cf includes
    cf=$(sed -n '/^CF="/,/"$/p' kernel/buildefi.sh \
        | sed 's/^CF="//; s/"$//' | tr -d '\\' | tr '\n' ' ')
    includes=$(cd kernel && find src boot -type d -printf ' -I%p' | sort)
    printf '%s\n' "${cf//\$INCLUDES/$includes}"
}

efi_sources() {
    local f loop
    loop=$(sed -nE 's/^for f in (.*); do$/\1/p' kernel/buildefi.sh)
    for f in $loop; do
        if [ "$f" = '$CORE' ]; then
            grep -vE '^[[:space:]]*(#|$)' kernel/SOURCES
        else
            printf '%s\n' "$f"
        fi
    done

    # Include the C translation units compiled outside the shared-source loop.
    sed -nE 's/^clang \$CF.* -c ([^ ]+\.c) -o .*/\1/p' kernel/buildefi.sh
}

# Extract flags written between `$CF` and `-c <source>` on explicit compile
# lines. The freestanding frontend files require these defines; omitting them
# measures missing host headers rather than pointer truncation.
efi_source_flags() {
    local wanted="$1"
    awk -v wanted="$wanted" '
        $1 == "clang" && $2 == "$CF" {
            for (i = 3; i <= NF; i++) {
                if ($i == "-c" && $(i + 1) == wanted) {
                    for (j = 3; j < i; j++)
                        printf "%s%s", (j == 3 ? "" : " "), $j
                    print ""
                }
            }
        }
    ' "$REPO_ROOT/kernel/buildefi.sh"
}

if [ "${1:-}" = "--check-efi-parser" ]; then
    flags=$(efi_cflags)
    sources=$(efi_sources | awk '!seen[$0]++')
    case "$flags" in *'$'*) echo "hazard-scan parser: FAIL: unexpanded flag variable"; exit 1;; esac
    case "$sources" in *'$'*) echo "hazard-scan parser: FAIL: unexpanded source variable"; exit 1;; esac
    while IFS= read -r f; do
        [ -z "$f" ] && continue
        grep -qxF "$f" <<< "$sources" || {
            echo "hazard-scan parser: FAIL: missing SOURCES entry $f"
            exit 1
        }
    done < <(grep -vE '^[[:space:]]*(#|$)' kernel/SOURCES)
    for f in ../src/frontend/lexer.c ../src/frontend/parser.c ../src/runtime/interp.c boot/efi_stage0.c; do
        grep -qxF "$f" <<< "$sources" || {
            echo "hazard-scan parser: FAIL: missing explicit EFI source $f"
            exit 1
        }
    done
    for spec in \
        '../src/frontend/lexer.c|-DZL_FREESTANDING|-DBUILD_PARSER' \
        '../src/frontend/parser.c|-DZL_FREESTANDING|-DBUILD_INTERP' \
        '../src/runtime/interp.c|-DZL_FREESTANDING'; do
        IFS='|' read -r source required_a required_b <<< "$spec"
        parsed=" $(efi_source_flags "$source") "
        case "$parsed" in *" $required_a "*) ;; *)
            echo "hazard-scan parser: FAIL: $source missing parsed flag $required_a"
            exit 1
        esac
        if [ -n "$required_b" ]; then
            case "$parsed" in *" $required_b "*) ;; *)
                echo "hazard-scan parser: FAIL: $source missing parsed flag $required_b"
                exit 1
            esac
        fi
    done
    echo "hazard-scan parser: PASS: expanded flags and complete EFI source manifest"
    exit 0
fi

[ "$COUNT_ONLY" -eq 1 ] || echo "== 1. the EFI warning guard must actually FIRE, not merely be configured =="
# buildefi.sh targets x86_64-unknown-windows, which is LLP64: unsigned long is 4
# bytes there, 8 everywhere else. Five pointer truncations once sat in the boot
# path because -w silenced the warnings that name this class.
#
# The build once placed the four -Werror= flags after -w on the theory that
# clang applies flags left to right. That is true of -Wno-<group>, but NOT of
# -w, which is a blanket suppression that wins regardless of position. Measured:
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
        hit "the EFI build accepts a 64-bit pointer truncated into 'unsigned long'"
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
    files=$(efi_sources | awk '!seen[$0]++')
    total=0; bad=0
    # buildefi.sh runs from kernel/, and CF carries a relative -I.. — so this
    # must compile from there too or every include resolves to the wrong tree.
    pushd kernel >/dev/null || exit 1
    for f in $files; do
        [ -f "$f" ] || continue
        SOURCE_FLAGS=()
        read -r -a SOURCE_FLAGS <<< "$(efi_source_flags "$f")"
        EXTRA=("${SOURCE_FLAGS[@]}")
        case "$(basename "$f")" in idt.c|apic.c) EXTRA+=(-mgeneral-regs-only);; esac
        # Match buildefi.sh exactly. Omitting ZL_KERNEL_SERIAL makes one
        # unrelated standalone-translation-unit error appear in every file and
        # used to manufacture a seven-item TODO from a green EFI build.
        n=$(clang "${CFA[@]}" "${EXTRA[@]}" -DZL_KERNEL_SERIAL \
            -c "$f" -o "$tmp/o.o" 2>&1 | grep -c 'error:')
        if [ "$n" -gt 0 ]; then
            [ "$COUNT_ONLY" -eq 1 ] || printf "     %-36s %s\n" "$f" "$n"
            total=$((total+n)); bad=$((bad+1))
        fi
    done
    popd >/dev/null || exit
    if [ "$COUNT_ONLY" -eq 1 ]; then echo "$total"; exit 0; fi
    base=$(grep -E '^efi_truncation_sites=' tools/hazard-baseline.txt 2>/dev/null | cut -d= -f2)
    base=${base:-0}
    if [ "$total" -gt "$base" ]; then
        # Advisory: the count is clang-version dependent, so this comparison
        # cannot be the gate. CI compares base against HEAD in one container -
        # see .github/workflows/gates.yml - which is compiler-independent.
        warn "$total truncation sites across $bad files - local baseline is $base."
        warn "  (baseline is advisory; a different clang counts differently)"
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
for g in kernel/verify.sh kernel/tools/checks/verify-*.sh; do
    [ -f "$g" ] || continue
    if grep -qE 'timeout' "$g" && ! grep -qE 'while|until|CEILING|kill -0' "$g"; then
        hit "$g uses timeout with no polling loop"
    fi
done
grep -qE 'CEILING|kill -0' kernel/tools/checks/verify-raw.sh 2>/dev/null && ok "verify*.sh poll rather than racing a clock"

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
       && gcc -O2 -D_strdup=strdup -Isrc/runtime -o "$tmp/s.bin" out.c src/runtime/runtime.c src/runtime/os_linux.c -lm 2>/dev/null; then
        b=$("$tmp/s.bin" 2>&1)
        [ "$a" = "$b" ] && ok "interp and C backend agree (both '$a')" \
                        || hit "scoping divergence: interp '$a' vs C backend '$b'"
    else
        skip "C backend did not build"
    fi
else
    skip "./interp or ./compile not built (run ./build.sh)"
fi

echo "== 6. every version string the OS shows must be the same version =="
# "0.3" is written out six times in kernel.zl; settings.c said "zl 0.1" and the
# shell's own boot banner said "zlOS 0.10". Three answers to one question, two
# of them on screen at the same time. src/core/version.h is the single source
# and check-version.py compares kernel.zl's literals against it.
#
# A guard nobody runs is not a guard - this is the line that makes it one.
if [ -f kernel/tools/checks/check-version.py ]; then
    if out=$(python3 kernel/tools/checks/check-version.py 2>&1); then
        ok "$(printf '%s' "$out" | tail -1)"
    else
        hit "$(printf '%s' "$out" | tail -n +2 | head -3)"
    fi
    # ...AND THAT THE GUARD ABOVE CAN STILL GO RED. Every check in
    # docs/GUARDS-THAT-DID-NOT-GUARD.md was green at the moment it stopped
    # working, so "it passed" is not evidence that it looked. The selftest runs
    # the real module against synthetic trees - it never touches the checkout,
    # so it is safe alongside a boot gate.
    if [ -f kernel/tools/checks/check-version-selftest.py ]; then
        if out=$(python3 kernel/tools/checks/check-version-selftest.py 2>&1); then
            ok "$(printf '%s' "$out" | tail -1)"
        else
            hit "$(printf '%s' "$out" | grep -i fail | head -3)"
        fi
    else
        skip "check-version-selftest.py not present"
    fi
else
    skip "check-version.py not present"
fi

echo "== 7. the boot gates must tell a crashed EMULATOR from a failed kernel =="
# All five threw QEMU's wait status away and judged on log contents, so when
# qemu-system-x86_64 segfaulted under load the gate reported "the kernel never
# started". One shared helper answers it now; this proves the helper can still
# fire, and - the case that matters - that it stays silent on 143, which is the
# SIGTERM the gates send themselves on every healthy boot.
if [ -f kernel/tools/checks/qemu-crash-selftest.sh ]; then
    if out=$(bash kernel/tools/checks/qemu-crash-selftest.sh 2>&1); then
        ok "$(printf '%s' "$out" | tail -1)"
    else
        hit "$(printf '%s' "$out" | grep -i fail | head -3)"
    fi
else
    skip "qemu-crash-selftest.sh not present"
fi

echo "== 8. kernel.zl constants that mirror memmap.h must still equal it =="
# zl cannot include a C header, so a constant like
#   RULER_DMA = 0x03000000   # memmap.h HI_IMG
# is a promise kept by hand. docs/evidence/presswork-first-boot.md recorded that
# exact pair as "verified equal today, enforced by nothing" and left it there;
# the reserves it compared itself to went on to be found in eleven places, none
# agreeing. A hazard written down and not wired up is a guess with a citation.
if [ -f kernel/tools/checks/check-memmap-mirror.py ]; then
    if out=$(python3 kernel/tools/checks/check-memmap-mirror.py 2>&1); then
        ok "$(printf '%s' "$out" | tail -1)"
        printf '%s' "$out" | grep '^  note' || true
    else
        hit "$(printf '%s' "$out" | grep -A3 FAIL | head -4)"
    fi
    if out=$(bash kernel/tools/checks/check-memmap-mirror-selftest.sh 2>&1); then
        ok "$(printf '%s' "$out" | tail -1)"
    else
        hit "$(printf '%s' "$out" | grep -i fail | head -3)"
    fi
else
    skip "check-memmap-mirror.py not present"
fi

# The same, for the guard that covers EVERY header a .zl mirrors - not just
# memmap.h. It was written, it passed, and NOTHING RAN IT: a complete guard with
# no caller is the failure it exists to catch, and it had it.
if [ -f kernel/tools/checks/check-header-mirror.py ]; then
    if out=$(python3 kernel/tools/checks/check-header-mirror.py 2>&1); then
        ok "$(printf '%s' "$out" | tail -1)"
    else
        hit "$(printf '%s' "$out" | grep -A3 FAIL | head -4)"
    fi
    if out=$(sh kernel/tools/checks/check-header-mirror-selftest.sh 2>&1); then
        ok "$(printf '%s' "$out" | tail -1)"
    else
        hit "$(printf '%s' "$out" | grep -iE 'NOT CAUGHT|FAIL' | head -3)"
    fi
else
    skip "check-header-mirror.py not present"
fi

echo
[ "$fail" -ne 0 ] && { echo "hazard-scan: FAILED"; exit 1; }
echo "hazard-scan: clean"
