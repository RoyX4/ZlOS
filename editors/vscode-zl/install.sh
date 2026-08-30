#!/usr/bin/env bash
# install.sh [editor] - build zlfmt, package this extension, install it.
#
#   ./install.sh            # VS Code  (default)
#   ./install.sh cursor     # Cursor
#
# Cursor is a VS Code fork: same manifest format, same CLI verbs, its own
# extensions dir at ~/.cursor/extensions/extensions.json. Verified that
# `cursor --install-extension` exists. Without this, .zl files are Plain Text in
# Cursor - which wastes the one thing Cursor is actually best at, on the main
# language of this repo.
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

EDITOR_CMD="${1:-code}"
case "$EDITOR_CMD" in
    code|cursor|codium|code-insiders) ;;
    *) echo "install: unknown editor '$EDITOR_CMD' (try: code, cursor)"; exit 1 ;;
esac

command -v "$EDITOR_CMD" >/dev/null || { echo "install: '$EDITOR_CMD' is not on PATH"; exit 1; }
command -v npx  >/dev/null || { echo "install: needs npx (node) to package the .vsix"; exit 1; }

# The formatter is a real binary; the extension shells out to it.
if [ ! -x "$REPO/zlfmt" ]; then
    echo "==> building zlfmt"
    ( cd "$REPO" && gcc -O2 -Wall -D_strdup=strdup -DBUILD_PARSER \
        -Isrc/frontend -o zlfmt src/tools/zlfmt.c src/frontend/lexer.c -lm )
fi

VERSION=$(python3 -c "import json;print(json.load(open('package.json'))['version'])")
VSIX="/tmp/zl-lang-$VERSION.vsix"

echo "==> packaging $VSIX"
npx --yes @vscode/vsce@latest package --allow-missing-repository --skip-license -o "$VSIX" >/dev/null

echo "==> installing"
"$EDITOR_CMD" --install-extension "$VSIX" --force

echo
echo "installed:"
"$EDITOR_CMD" --list-extensions --show-versions | grep '^zl\.' || echo "  (not listed - something went wrong)"
echo
echo "Point the extension at the formatter by adding this to $EDITOR_CMD settings:"
echo "    \"zl.formatterPath\": \"$REPO/zlfmt\""
echo "(or just open $REPO as a workspace folder - it finds ./zlfmt on its own)"
echo
echo "Reload $EDITOR_CMD (Ctrl+Shift+P -> Developer: Reload Window) to pick it up."
