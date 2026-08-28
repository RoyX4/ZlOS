#!/usr/bin/env bash
# verify-net.sh - prove virtio_net fetches http://example.com/ under QEMU user-net.
#
# The synchronized probe waits for each guest result before submitting the
# next command. This keeps DHCP/network bring-up and the browser fetch ordered.
set -euo pipefail
KERNEL_ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$KERNEL_ROOT"

python3 tools/probes/probe-net.py --fetch
