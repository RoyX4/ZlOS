#!/usr/bin/env bash
# install-hooks.sh - install the git hooks. Run once per clone or worktree.
#
#     tools/install-hooks.sh
#
# GitHub branch protection needs Pro on a private repo, so `main` cannot be
# server-side gated here. This is the local substitute: the fast gates run
# before a push, and a push that would go red is stopped before it leaves the
# machine.
#
# It is deliberately only the FAST gates (~1 min, no emulator). A pre-push hook
# that boots QEMU four times is a hook people uninstall.
#
# Bypass when you mean to:  git push --no-verify

set -euo pipefail
cd "$(dirname "$0")/.."

# Worktrees keep their hooks in the main repo's common dir, so ask git rather
# than assuming .git is a directory - zl-apps, zl-browser, zl-exec and zl-feel
# are all worktrees of this repo.
hooks_dir=$(git rev-parse --git-common-dir)/hooks
mkdir -p "$hooks_dir"

cat > "$hooks_dir/pre-push" <<'HOOK'
#!/usr/bin/env bash
# Installed by tools/install-hooks.sh. Bypass with: git push --no-verify
set -uo pipefail

root=$(git rev-parse --show-toplevel)
[ -x "$root/tools/preflight.sh" ] || exit 0

echo "pre-push: running the fast gates (git push --no-verify to skip)"
if ! "$root/tools/preflight.sh"; then
    echo
    echo "pre-push: BLOCKED - the fast gates are not clean."
    echo "  fix, or push anyway with:  git push --no-verify"
    exit 1
fi
HOOK

chmod +x "$hooks_dir/pre-push"

# ---------------------------------------------------------------------------
# post-commit: write down what happened and refresh the derived docs.
#
# It deliberately does NOT amend the commit or commit on your behalf. Amending
# from a hook rewrites history under you and breaks the moment the commit is
# already pushed; a hook that silently makes its own commits is worse. It
# updates the files and tells you they are waiting.
# ---------------------------------------------------------------------------
cat > "$hooks_dir/post-commit" <<'HOOK'
#!/usr/bin/env bash
# Installed by tools/install-hooks.sh.
set -uo pipefail

root=$(git rev-parse --show-toplevel)
cd "$root" || exit 0
[ -x tools/journal.sh ] || exit 0

# Guard against recursion: `git commit` inside a post-commit hook would fire it
# again. Nothing here commits, but the guard costs nothing and outlives this file.
[ -n "${ZL_HOOK_RUNNING:-}" ] && exit 0
export ZL_HOOK_RUNNING=1

tools/journal.sh HEAD >/dev/null 2>&1
tools/todo.sh        >/dev/null 2>&1
docs=$(tools/doc-check.sh 2>&1)
if ! echo "$docs" | tail -1 | grep -q 'agree'; then
    echo
    echo "post-commit: THE DOCS NO LONGER MATCH THE TREE"
    echo "$docs" | grep '^  STALE' | head -8
    echo "  (full report: tools/doc-check.sh)"
fi

changed=$(git status --porcelain docs/JOURNAL.md TODO.md 2>/dev/null)
if [ -n "$changed" ]; then
    echo "post-commit: refreshed docs/JOURNAL.md and TODO.md."
    echo "  They ride along with your NEXT commit - do not amend them in."
    echo "  A journal entry names the sha of the commit it describes, so"
    echo "  amending it into that commit changes the sha it just recorded."
fi
HOOK

chmod +x "$hooks_dir/post-commit"

echo "installed: $hooks_dir/pre-push"
echo "           $hooks_dir/post-commit"
echo
echo "  pre-push    runs tools/preflight.sh (~1 min, no emulator)"
echo "              bypass with: git push --no-verify"
echo "  post-commit appends to docs/JOURNAL.md, regenerates TODO.md, and warns"
echo "              if the docs stopped matching the tree. Never commits for you;"
echo "              the refresh rides along with your next commit."
