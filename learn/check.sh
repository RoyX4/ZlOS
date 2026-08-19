#!/usr/bin/env sh
# Run the learning exercises. From the repo root:
#   learn/check.sh            # all exercises
#   learn/check.sh 03         # just exercise 03
#   learn/check.sh 03 --sol   # the worked solution instead
set -u
cd "$(dirname "$0")/.." || exit 1
[ -x ./interp ] || { echo "interp not built - run ./build.sh first"; exit 1; }

pick="${1:-}"
dir=learn
case "${2:-}" in --sol|--solutions) dir=learn/solutions ;; esac

fail=0
for f in $dir/*.zl; do
    case "$pick" in "" ) ;; *) case "$f" in *"/$pick"_*) ;; *) continue ;; esac ;; esac
    out=$(./interp "$f" 2>&1)
    if [ $? -eq 0 ]; then
        printf '  PASS  %s\n' "$f"
    else
        fail=1
        printf '  TODO  %s\n        %s\n' "$f" "$out"
    fi
done
[ $fail -eq 0 ] && echo "all green" || echo "keep going - the message above names the failing assert"
exit 0
