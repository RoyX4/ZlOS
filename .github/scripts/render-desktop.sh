#!/usr/bin/env bash
# render-desktop.sh <label> [repo-root]
#
# Build wmshot and render the host compositor harness at two resolutions into
# ./shots/. The image is not kernel.zl's shell and labels that boundary.
# Used by .github/workflows/desktop-shot.yml, and useful by hand:
#
#     bash .github/scripts/render-desktop.sh mine && xdg-open shots/mine-1024x768.png
#
# No QEMU, no GPU. wm.c/ui.c/fb.c are freestanding C against memory, so this is
# a compile and a memcpy, not a boot.

set -euo pipefail
label="${1:-head}"
root="${2:-$(cd "$(dirname "$0")/../.." && pwd)}"
root="$(cd "$root" && pwd)"
cd "$root"
mkdir -p shots

# Same source set as kernel/tests/host/build.sh uses for wmshot. GNU find's
# `%s` is file size, not path; keep these as an array so every directory is one
# compiler argument regardless of whitespace.
mapfile -t include_dirs < <(find kernel/src -type d -print | sort)
includes=()
for dir in "${include_dirs[@]}"; do includes+=("-I$dir"); done
gcc -O2 -w "${includes[@]}" -o /tmp/wmshot-"$label" \
    kernel/tests/host/wmshot.c \
    kernel/src/graphics/windowing/wm.c \
    kernel/src/graphics/ui/ease.c \
    kernel/src/graphics/ui/notify.c \
    kernel/src/graphics/ui/snap.c \
    kernel/src/graphics/ui/ui.c \
    kernel/src/graphics/ui/uikit.c \
    kernel/src/graphics/windowing/wmglue.c \
    kernel/src/graphics/ui/settings.c \
    kernel/tests/host/hoststubs.c \
    kernel/src/graphics/framebuffer/fb.c \
    kernel/src/drivers/input/input.c \
    kernel/src/graphics/fonts/font8x16.c \
    kernel/src/graphics/fonts/font_aa.c \
    kernel/src/graphics/fonts/font_sub.c \
    kernel/src/graphics/icons/icons.c

for size in 1024x768 1920x1200; do
    w="${size%x*}"; h="${size#*x}"
    rendered=$(/tmp/wmshot-"$label" "shots/$label-$size.ppm" "$w" "$h")
    printf '%s\n' "$rendered"
    grep -Fq 'host compositor harness; zl shell not included' <<< "$rendered"
    if command -v magick >/dev/null 2>&1; then
        magick "shots/$label-$size.ppm" "shots/$label-$size.png"
    else
        convert "shots/$label-$size.ppm" "shots/$label-$size.png"
    fi
    rm -f "shots/$label-$size.ppm"
done

ls -la shots/
