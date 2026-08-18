#!/usr/bin/env bash
# install.sh - build zlfmt, package this extension, and install it into VS Code.
#
# Why this exists instead of the one-line `cp -r` the README used to give:
# copying a folder into ~/.vscode/extensions/ does NOT register an extension
# with current VS Code. The editor keeps a manifest at
# ~/.vscode/extensions/extensions.json and only loads what is listed there;
# `code --list-extensions` reads the same file. A folder-drop is invisible to
# both, which is exactly the "I installed it but nothing happens" symptom.
#
# `code --install-extension` is what writes that manifest entry.
set -euo pipefail
cd "$(dirname "$0")"
REPO=$(cd ../.. && pwd)

command -v code >/dev/null || { echo "install: 'code' is not on PATH"; exit 1; }
command -v npx  >/dev/null || { echo "install: needs npx (node) to package the .vsix"; exit 1; }

# The formatter is a real binary; the extension shells out to it.
if [ ! -x "$REPO/zlfmt" ]; then
    echo "==> building zlfmt"
    ( cd "$REPO" && gcc -O2 -Wall -D_strdup=strdup -DBUILD_PARSER -o zlfmt zlfmt.c lexer.c -lm )
fi

VERSION=$(python3 -c "import json;print(json.load(open('package.json'))['version'])")
VSIX="/tmp/zl-lang-$VERSION.vsix"

echo "==> packaging $VSIX"
npx --yes @vscode/vsce@latest package --allow-missing-repository --skip-license -o "$VSIX" >/dev/null

echo "==> installing"
code --install-extension "$VSIX" --force

echo
echo "installed:"
code --list-extensions --show-versions | grep '^zl\.' || echo "  (not listed - something went wrong)"
echo
echo "Point the extension at the formatter by adding this to VS Code settings:"
echo "    \"zl.formatterPath\": \"$REPO/zlfmt\""
echo "(or just open $REPO as a workspace folder - it finds ./zlfmt on its own)"
echo
echo "Reload VS Code (Ctrl+Shift+P -> Developer: Reload Window) to pick it up."
