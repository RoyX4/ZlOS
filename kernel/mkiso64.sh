#!/usr/bin/env bash
# Named 64-bit multiboot ISO route. Packaging remains centralized in mkiso.sh;
# this wrapper exists so gates, receipts and users never depend on a hidden env.
set -euo pipefail
cd "$(dirname "$0")"
export ZLOS_ISO_BITS=64
exec ./mkiso.sh
