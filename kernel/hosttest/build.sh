#!/usr/bin/env bash
# Compile zlOS's Intel driver as a Linux program, against the real GPU.
# The driver source is used unmodified - that is the whole point.
set -e
cd "$(dirname "$0")"
gcc -O1 -g -Wall -DZL_64 -Wno-unused-function -o intel_probe \
    intel_probe.c ../intel.c
echo "built ./intel_probe   (run: sudo ./intel_probe [--unsafe])"

# The modeset harness holds its own mmap and reads raw offsets, because the
# whole point of stage 1 is to settle which offsets are right - it must not go
# through the driver's opinion of where things live.
gcc -O2 -g -Wall -Wextra -o modeset_test modeset_test.c
echo "built ./modeset_test  (run: sudo ./modeset_test --survey)"
