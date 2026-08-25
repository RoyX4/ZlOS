#!/usr/bin/env bash
# Named 64-bit multiboot ISO route. Packaging remains centralized in mkiso.sh;
# this wrapper exists so gates, receipts and users never depend on a hidden env.
set -euo pipefail
KERNEL_ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$KERNEL_ROOT"
export ZLOS_ISO_BITS=64
exec ./tools/images/mkiso.sh
