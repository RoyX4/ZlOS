#!/usr/bin/env bash
# Capture the standard set of reference PNGs into kernel/refrender/out/.
#
#   ./shots.sh              every shot: the default desktop + one per app id
#   ./shots.sh term files   just those app ids
#
# The app ids are read out of the running reference (state.pos is seeded from
# its own APPS/UTILS/GAME_APPS tables), never transcribed here — so this stays
# correct if ds.html gains or loses an app.
#
# One browser is launched for the whole run, but the page is reloaded for every
# shot so the seeded PRNG and frozen clock reset and shot N does not depend on
# shot N-1. Expect a couple of minutes for the full set.
set -euo pipefail

cd "$(dirname "$0")"

if [ ! -d node_modules/playwright-core ]; then
  echo "shots.sh: installing playwright-core (one-off)" >&2
  npm install --no-audit --no-fund
fi

mkdir -p out

if [ $# -eq 0 ]; then
  node render-ref.mjs --batch --quiet
else
  for app in "$@"; do
    node render-ref.mjs --app "$app" --quiet
  done
fi

echo
echo "out/ now holds $(ls -1 out/*.png 2>/dev/null | wc -l) PNGs, $(du -sh out | cut -f1) total"
