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
echo "installed: $hooks_dir/pre-push"
echo
echo "  runs tools/preflight.sh before every push (~1 min, no emulator)"
echo "  bypass with: git push --no-verify"
