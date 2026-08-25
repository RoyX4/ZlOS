# MP-00 exact build-input source snapshot

## Outcome

The current dirty/untracked zlOS build can now be reconstructed at the exact
build-input level. Hashes in `build-identity.json` previously detected drift but
could not restore a file after it changed. The new deterministic archive stores
all 123 files in that identity:

- archive: [`receipts/source-snapshot-build-inputs-2026-08-24.tar`](receipts/source-snapshot-build-inputs-2026-08-24.tar);
- receipt: [`receipts/source-snapshot-2026-08-24.json`](receipts/source-snapshot-2026-08-24.json);
- archive bytes: 5,294,080;
- archive SHA-256:
  `cb17bcd596060f72169288807375816d7c7653852eb213261247c72ea95697b2`;
- joined build identity:
  `85027b159c9a594045c2f900e5971bb3408dd418dd61a373625425fba9030d13`.

This is `PARTIAL_CURRENT`, not complete source custody. The archive and receipt
are uncommitted in the same worktree, unsigned, have zero off-host copies and do
not contain the entire repository/history. They preserve the exact active build
closure only.

## Deterministic format

[`../gen-source-snapshot.py`](../gen-source-snapshot.py) reads the canonical
source-file map from `build-identity.json`, refuses any stale hash, and emits a
POSIX ustar archive with:

- lexicographically sorted safe relative paths;
- regular files only, with no traversal or symlink admission;
- original executable/read modes;
- normalized UID, GID and modification time;
- no unlisted member;
- per-member byte count and SHA-256 repeated in the receipt.

The receipt additionally binds the Git head/branch, canonical dirty-status
digest, recursive submodule status, generator hash and explicit custody gaps.
The archive and receipt omit themselves from the dirty-status digest to avoid a
self-identity cycle.

## Rejection proof

Run from the isolated implementation root:

```sh
python3 kernel/tools/generators/gen-source-snapshot.py --check --selftest
```

The self-test proves nonzero rejection of:

1. a missing build input;
2. a claim that an off-host copy exists when none does;
3. an archive SHA-256 change;
4. changed bytes inside an otherwise named archive.

The landing gate regenerates and immediately checks the snapshot after checking
the build identity. `gen-evidence-registry.py` then joins the receipt while
retaining all three custody gaps.

## Recovery use

Extraction must target a new, empty directory. Never overwrite the live dirty
worktree merely because a hash differs.

```sh
mkdir /path/to/empty-recovery-directory
tar -xf kernel/docs/receipts/source-snapshot-build-inputs-2026-08-24.tar \
  -C /path/to/empty-recovery-directory
```

After extraction, verify each receipt member and rebuild in an isolated
worktree. This archive is not a substitute for a Git commit, signed release,
remote backup, dependency archive or whole-repository disaster-recovery copy.
