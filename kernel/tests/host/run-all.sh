#!/usr/bin/env bash
# Rebuild every host executable, then run the same inventory used by CI.
# A missing executable must never inherit a passing result from an older build.
# Instruments, hardware-only checks and fixture arguments belong to test-policy.json;
# keeping a second skip list here made zlfsseed/dpll_test false failures and left
# automatic fixtures out of the local run.
set -euo pipefail
cd "$(dirname "$0")"

# These are the standard kernel build prerequisites, including the root zl
# toolchain. Generate identity before execution so the receipt names this source.
python3 ../../tools/generators/gen-build-identity.py --write --selftest
python3 ../../tools/generators/gen-test-inventory.py --write --selftest

python3 - <<'PY'
import json
from pathlib import Path

inventory = json.loads(Path('../../metadata/test-inventory.json').read_text())
targets = [row['name'] for row in inventory['targets'] if row['form'] == 'compiled']
for name in targets:
    if Path(name).name != name:
        raise ValueError('host executable must be a local filename: ' + name)
for name in targets:
    Path(name).unlink(missing_ok=True)
print(f'host build: removed {len(targets)} previous executables')
PY

build_log=$(mktemp)
trap 'rm -f "$build_log"' EXIT
if ./build.sh >"$build_log" 2>&1; then
    echo "host build: PASS"
else
    cat "$build_log"
    exit 1
fi
python3 ../../tools/run/run-host-tests.py --run --selftest
