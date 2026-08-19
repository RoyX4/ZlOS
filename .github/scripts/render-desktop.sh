#!/usr/bin/env bash
# render-desktop.sh <label>
#
# Build wmshot and render the compositor at two resolutions into ./shots/.
# Used by .github/workflows/desktop-shot.yml, and useful by hand:
#
#     bash .github/scripts/render-desktop.sh mine && xdg-open shots/mine-1024x768.png
#
# No QEMU, no GPU. wm.c/ui.c/fb.c are freestanding C against memory, so this is
# a compile and a memcpy, not a boot.

set -euo pipefail
label="${1:-head}"
root="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$root"
mkdir -p shots

# Same line as kernel/hosttest/build.sh uses for wmshot.
gcc -O2 -w -o /tmp/wmshot-"$label" \
    kernel/hosttest/wmshot.c \
    kernel/wm.c kernel/ui.c kernel/wmglue.c kernel/fb.c kernel/input.c \
    kernel/font8x16.c kernel/font_aa.c kernel/font_sub.c kernel/icons.c

for size in 1024x768 1920x1200; do
    w="${size%x*}"; h="${size#*x}"
    /tmp/wmshot-"$label" "shots/$label-$size.ppm" "$w" "$h"
    if command -v magick >/dev/null 2>&1; then
        magick "shots/$label-$size.ppm" "shots/$label-$size.png"
    else
        convert "shots/$label-$size.ppm" "shots/$label-$size.png"
    fi
    rm -f "shots/$label-$size.ppm"
done

ls -la shots/
