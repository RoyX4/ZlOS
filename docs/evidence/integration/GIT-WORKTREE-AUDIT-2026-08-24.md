# ZL Git and worktree preservation audit — 2026-08-24

## Result

Every recoverable first-party ZL change found under the active repository and
worktree roots was committed and pushed to GitHub. Overlapping experiments were
kept on separate branches; none was merged into `main`, force-pushed, reverted,
or deleted.

After the final fetch, both canonical repositories reported zero commits in
local branches that were absent from all remote branches. Every live worktree
reported a clean short status.

## GitHub branch receipts

| Project | Branch | Preservation commit | Purpose |
|---|---|---:|---|
| `zl` | `codex/local-zl-snapshot-2026-08-24` | `c3f2535` | Linux host layer, design work, and repository research |
| `zl` | `codex/github-repo-patterns` | `769306d` | GitHub repository-pattern research |
| `zl-linux` | `ci/gates-and-agent-brief` | `b445029` | Three previously local CI commits |
| `zl-linux` | `codex/local-zlos-snapshot-2026-08-24` | `85fcdf5` | Shared-checkout roadmap and telemetry snapshot |
| `zl-linux` | `files-app` | `6b40dfb` | Persistent zlfs and Files app work |
| `zl-linux` | `secret/desktop-foundry` | `1c96bfe` | Foundry desktop mock and design receipt |
| `zl-linux` | `codex/master-program-foundation` | `11c711e` | MP-00 registries, gates, receipts, and provenance foundation |
| `zl-linux` | `codex/audit-trail` | `359cdb4` | ThinkPad input, Wi-Fi/I219 quarantine, USB diagnostics, and physical evidence |
| `zl-linux` | `codex/performance-architecture` | `e0782d8` | Performance architecture implementation and archived QEMU evidence |
| `zl-linux` | `codex/product-wave1` | `187a25e` | Typed boot handover and product-wave snapshot |

Every preservation commit above was compared with the same GitHub branch using
`git ls-remote` and was present there. The audit branch additionally carries
this report and its README link, so its live branch head advances past the
implementation/evidence commit shown in the table.

## Worktrees checked

- `/home/roy/Documents/repos/zl`
- `/home/roy/.codex/worktrees/zl-github-repo-patterns`
- `/home/roy/Documents/repos/zl-linux`
- `/home/roy/.cursor/worktrees/zl-linux/files-app`
- `/home/roy/Documents/repos/zl-linux-desktop-foundry`
- `/home/roy/Documents/repos/zl-linux-master-program-foundation`
- `/home/roy/Documents/repos/zl-linux-audit-trail`
- `/home/roy/Documents/repos/zl-linux-performance-architecture`
- `/home/roy/Documents/repos/zl-linux-product-wave1`
- three registered detached Claude worktrees under
  `/home/roy/Documents/repos/zl-linux/.claude/worktrees/`

The audit clone also contains three ignored copies of those Claude worktree
directories. Their `.git` files point back to the registered worktrees in the
canonical `zl-linux` repository. They contain no modified or untracked unique
files; their only status entries are missing generated exercise frames. They
are stale directory copies, not additional work that needs a branch.

## Generated output policy

Source, tests, docs, manifests, compact receipts, and physical USB logs were
committed. Reproducible ELF host-test binaries, Python caches, temporary EFI
objects, and machine-local agent state were ignored.

Raw QEMU frame directories were preserved losslessly rather than discarded:

- `physical-debug-qemu-evidence-2026-08-24.tar.zst`: 4,807,812 bytes,
  SHA-256 `b532db579734aeede053120a2031d06bc62c3b6700b5e6419fad5475676336cb`
- `performance-architecture-evidence-2026-08-24.tar.zst`: 6,476,691 bytes,
  SHA-256 `652707a96fb3f8f85ac1ab150eceaec870bf8b212a31437c0a622e37e6df020b`

Together they preserve 561 MiB of raw captures without filling Git history
with hundreds of individual PPM files.

## Verification boundary

- The `zl` Linux interpreter build ran `tests/test_syntax.zl`: 235 passed,
  0 failed.
- The Files branch's focused `fstest` passed every case and ended with
  `all good: 0 failure(s)`.
- The Foundry HTML passed `xmllint --html --noout` after removal of stray text
  before its doctype.
- The physical audit branch had already passed its focused I2C, touchpad,
  input bridge, ZLLOG, source-route, native UEFI, and EFI gates before this Git
  preservation pass. Its USB image was also flashed and read back successfully.
- The MP-00, product-wave, performance, shared-checkout, and CI branches were
  preserved as branch snapshots. This pass checked their diffs and secret
  patterns but did not claim a new complete landing-gate result for them.

Three pushes initially launched the repository's full pre-push suite
concurrently. They were stopped because the combined compiler/QEMU load was
unsafe on the four-core host. The already durable commits were then pushed with
hooks bypassed. That is why these preservation branches are not release or
merge approval.
