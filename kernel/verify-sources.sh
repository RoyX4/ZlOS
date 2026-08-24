#!/usr/bin/env bash
# verify-sources.sh - does adding one source file reach every kernel build?
#
# There used to be four copies of the source list, and adding a .c to the build
# you were testing with silently missed the other three. It broke the UEFI and
# the raw builds TWICE on 2026-08-18, and verify.sh cannot see it because
# verify.sh only ever builds the first one.
#
# ./SOURCES is now the single list. This proves that rather than asserting it,
# the way the task asked for: drop a throwaway .c in, build all four, and look
# for it INSIDE each kernel output. The UEFI kernel is ZLOS.EFI; BOOTX64.EFI is
# intentionally the independent stage-zero witness and must not contain the
# kernel source list. Not "the build succeeded" - a build
# that ignored the file succeeds too. The marker string has to actually be in
# the binary.
#
# Cleanup runs from an EXIT trap for ordinary exits.  A fixed transaction
# directory also makes the mutation recoverable after SIGKILL, terminal loss,
# or host restart; anonymous mktemp backups cannot provide that guarantee.
set -uo pipefail
SCRIPT_PATH=$(realpath "${BASH_SOURCE[0]}")

selftest_recovery() {
    local fixture owner rc ready
    set -e
    fixture=$(mktemp -d)
    owner=""
    cleanup_fixture() {
        if [ -n "$owner" ] && kill -0 "$owner" 2>/dev/null; then
            kill "$owner" 2>/dev/null || true
            wait "$owner" 2>/dev/null || true
        fi
        rm -rf -- "$fixture"
    }
    trap cleanup_fixture EXIT

    cp "$SCRIPT_PATH" "$fixture/verify-sources.sh"
    chmod +x "$fixture/verify-sources.sh"

    # A dead owner left fully backed-up state. Recovery must restore every
    # authority byte-for-byte and remove all probe debris.
    printf '%s\n' 'mutated SOURCES' > "$fixture/SOURCES"
    printf '%s\n' 'mutated identity' > "$fixture/build-identity.json"
    printf '%s\n' 'mutated embed' > "$fixture/build_identity_embed.zl"
    mkdir "$fixture/.verify-sources-state"
    printf '%s\n' 'original SOURCES' > "$fixture/.verify-sources-state/SOURCES"
    printf '%s\n' 'original identity' > "$fixture/.verify-sources-state/build-identity.json"
    printf '%s\n' 'original embed' > "$fixture/.verify-sources-state/build_identity_embed.zl"
    printf '%s\n' '99999999' > "$fixture/.verify-sources-state/pid"
    : > "$fixture/_srcprobe.c"
    : > "$fixture/__srcprobe.o"
    (cd "$fixture" && ./verify-sources.sh --recover-only) >/dev/null
    grep -qx 'original SOURCES' "$fixture/SOURCES"
    grep -qx 'original identity' "$fixture/build-identity.json"
    grep -qx 'original embed' "$fixture/build_identity_embed.zl"
    [ ! -e "$fixture/.verify-sources-state" ]
    [ ! -e "$fixture/_srcprobe.c" ]
    [ ! -e "$fixture/__srcprobe.o" ]

    # Missing backups are ambiguous. The verifier must leave them untouched
    # and fail instead of guessing which live file is authoritative.
    mkdir "$fixture/.verify-sources-state"
    printf '%s\n' '99999999' > "$fixture/.verify-sources-state/pid"
    set +e
    (cd "$fixture" && ./verify-sources.sh --recover-only) >/dev/null 2>&1
    rc=$?
    set -e
    [ "$rc" -eq 2 ]
    [ -d "$fixture/.verify-sources-state" ]
    rm -rf -- "$fixture/.verify-sources-state"

    # A live verifier owns the transaction. A second verifier must refuse and
    # preserve all state. argv[0] deliberately contains verify-sources.sh,
    # matching the production ownership check against /proc/<pid>/cmdline.
    mkdir "$fixture/.verify-sources-state"
    printf '%s\n' 'owned SOURCES' > "$fixture/.verify-sources-state/SOURCES"
    printf '%s\n' 'owned identity' > "$fixture/.verify-sources-state/build-identity.json"
    printf '%s\n' 'owned embed' > "$fixture/.verify-sources-state/build_identity_embed.zl"
    bash -c 'exec -a verify-sources.sh sleep 30' &
    owner=$!
    ready=0
    for _ in $(seq 1 50); do
        if tr '\0' ' ' < "/proc/$owner/cmdline" 2>/dev/null | grep -q 'verify-sources.sh'; then
            ready=1
            break
        fi
        sleep 0.01
    done
    [ "$ready" -eq 1 ]
    printf '%s\n' "$owner" > "$fixture/.verify-sources-state/pid"
    set +e
    (cd "$fixture" && ./verify-sources.sh --recover-only) >/dev/null 2>&1
    rc=$?
    set -e
    [ "$rc" -eq 2 ]
    [ -d "$fixture/.verify-sources-state" ]

    cleanup_fixture
    trap - EXIT
    echo "verify-sources recovery selftest: PASS: stale recovery, incomplete refusal, live-owner refusal"
}

if [ "${1:-}" = "--selftest-recovery" ]; then
    selftest_recovery
    exit 0
fi

cd "$(dirname "$0")"

PROBE=_srcprobe.c
MARKER=ZL_SOURCES_PROBE_9c3f1a
STATE_DIR=.verify-sources-state
SOURCES_BAK=$STATE_DIR/SOURCES
IDENTITY_BAK=$STATE_DIR/build-identity.json
IDENTITY_EMBED_BAK=$STATE_DIR/build_identity_embed.zl

remove_probe_outputs() {
    rm -f "$PROBE" _*srcprobe*.o
}

recover_stale_transaction() {
    [ -d "$STATE_DIR" ] || return 0
    local owner=""
    [ -r "$STATE_DIR/pid" ] && owner=$(cat "$STATE_DIR/pid")
    if [ -n "$owner" ] && kill -0 "$owner" 2>/dev/null \
       && tr '\0' ' ' < "/proc/$owner/cmdline" 2>/dev/null \
          | grep -q 'verify-sources.sh'; then
        echo "verify-sources: another live verifier owns $STATE_DIR (pid $owner)" >&2
        exit 2
    fi
    if [ ! -f "$SOURCES_BAK" ] || [ ! -f "$IDENTITY_BAK" ] \
       || [ ! -f "$IDENTITY_EMBED_BAK" ]; then
        echo "verify-sources: incomplete recovery state in $STATE_DIR; refusing to guess" >&2
        exit 2
    fi
    cp "$SOURCES_BAK" SOURCES
    cp "$IDENTITY_BAK" build-identity.json
    cp "$IDENTITY_EMBED_BAK" build_identity_embed.zl
    remove_probe_outputs
    rm -f "$STATE_DIR/pid" "$SOURCES_BAK" "$IDENTITY_BAK" "$IDENTITY_EMBED_BAK"
    rmdir "$STATE_DIR"
    echo "verify-sources: recovered a stale interrupted transaction"
}

recover_stale_transaction
if [ "${1:-}" = "--recover-only" ]; then
    echo "verify-sources: transaction state is clean"
    exit 0
fi
if [ $# -ne 0 ]; then
    echo "usage: $0 [--recover-only|--selftest-recovery]" >&2
    exit 2
fi
if ! mkdir "$STATE_DIR"; then
    echo "verify-sources: could not acquire transaction directory $STATE_DIR" >&2
    exit 2
fi
printf '%s\n' "$$" > "$STATE_DIR/pid"
cp SOURCES "$SOURCES_BAK"
cp build-identity.json "$IDENTITY_BAK"
cp build_identity_embed.zl "$IDENTITY_EMBED_BAK"

cleanup() {
    cp "$SOURCES_BAK" SOURCES
    cp "$IDENTITY_BAK" build-identity.json
    cp "$IDENTITY_EMBED_BAK" build_identity_embed.zl
    # The build scripts derive object names as _$(basename $f .c).o and
    # friends, and $f is "_srcprobe.c" - so the objects are __srcprobe.o (TWO
    # underscores), __srcprobe64.o and _efi__srcprobe.o. The first version of
    # this named the one-underscore forms, removed nothing, and left two probe
    # objects sitting in the tree. Globbed now, so a naming change in a build
    # script cannot silently start leaking them again.
    remove_probe_outputs
    rm -f "$STATE_DIR/pid" "$SOURCES_BAK" "$IDENTITY_BAK" "$IDENTITY_EMBED_BAK"
    rmdir "$STATE_DIR"
}
trap cleanup EXIT

cat > "$PROBE" <<EOF
/* Written and deleted by verify-sources.sh. If you are reading this in a
 * working tree, that script died between writing it and its EXIT trap - it is
 * safe to delete, and \`git status\` should not be showing it at all. */
__attribute__((used))
const char zl_sources_probe[] = "$MARKER";
EOF

echo "$PROBE" >> SOURCES

# SOURCES and the probe itself are build-identity inputs.  The build routes now
# correctly fail closed if their generated identity is stale, so the mutation
# must carry a temporary identity too.  cleanup restores both generated files
# byte-for-byte before the clean rebuilds below.
if ! python3 ./gen-build-identity.py --write >/dev/null; then
    echo "  FAIL  could not generate the temporary probe build identity"
    exit 1
fi

fail=0
build_and_look() {          # build_and_look <label> <script> <output file>
    local label=$1 script=$2 out=$3
    if ! "./$script" >/dev/null 2>&1; then
        echo "  FAIL  $label - $script did not build with the probe in SOURCES"
        fail=1; return
    fi
    if [ ! -f "$out" ]; then
        echo "  FAIL  $label - $script produced no $out"; fail=1; return
    fi
    # grep the BINARY. A build that quietly ignored SOURCES still exits 0.
    if grep -qa "$MARKER" "$out"; then
        echo "  ok    $label - the new source reached $out"
    else
        echo "  FAIL  $label - $out does not contain the probe: $script is not reading SOURCES"
        fail=1
    fi
}

echo "== added $PROBE to SOURCES, building all four =="
build_and_look "32-bit BIOS " build.sh    kernel.elf
build_and_look "64-bit      " build64.sh  kernel64.elf
build_and_look "UEFI kernel " buildefi.sh ZLOS.EFI
build_and_look "raw disk    " mkdisk.sh   kernel_raw.bin

# ...and the other half of the claim: with the probe gone, it is gone from all
# four. Otherwise a stale object left in the tree would keep the marker alive
# and this whole gate would pass on a build system that reads nothing.
cleanup
trap - EXIT
echo "== removed it again =="
for pair in "build.sh kernel.elf" "build64.sh kernel64.elf" \
            "buildefi.sh ZLOS.EFI" "mkdisk.sh kernel_raw.bin"; do
    set -- $pair
    "./$1" >/dev/null 2>&1
    if grep -qa "$MARKER" "$2"; then
        echo "  FAIL  $2 still contains the probe after removal - stale object?"
        fail=1
    fi
done
[ "$fail" -eq 0 ] && echo "  ok    all four are clean again"

[ "$fail" -eq 0 ] && echo "sources gate green: one list, four builds" \
                  || echo "sources gate FAILED"
exit $fail
