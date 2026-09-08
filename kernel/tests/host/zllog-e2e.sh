#!/bin/sh
# zllog-e2e.sh - landing-gate entry for zllog_e2e_test.py.
#
# The Python test builds zllogtest.c (the shipping src/core/zllog.c against a
# file-backed fake USB disk, ASan+UBSan) and drives it through torn writes,
# identity refusal, slot rotation and the IRQ lane. It passed on 2026-09-06 the
# first time anyone ran it - it was written 2026-08-30 and nothing listed it:
# gen-test-inventory.py enumerates executable .sh files, and this was a .py.
#
# sgdisk is a hard requirement. The Python file raises SkipTest without it,
# which unittest reports as exit 0 - a green that ran nothing. Refuse instead.
set -u
here=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
if ! command -v sgdisk >/dev/null 2>&1; then
    echo "zllog-e2e: sgdisk is not installed (package gdisk); refusing to report green" >&2
    exit 1
fi
exec python3 "$here/zllog_e2e_test.py" "$@"
