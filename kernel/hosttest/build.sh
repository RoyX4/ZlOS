#!/usr/bin/env bash
# Compile zlOS's Intel driver as a Linux program, against the real GPU.
# The driver source is used unmodified - that is the whole point.
set -e
cd "$(dirname "$0")"
gcc -O1 -g -Wall -DZL_64 -Wno-unused-function -o intel_probe \
    intel_probe.c ../intel.c
echo "built ./intel_probe   (run: sudo ./intel_probe [--unsafe])"
