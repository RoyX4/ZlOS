# Dependency-lock feature classification - 2026-08-29

This receipt classifies EV-004 against the canonical 906-feature ledger. It
does not promote source custody, licensing authority or toolchain hermeticity.

## Result

EV-004 is `PROVED_CURRENT`. The exact current subject is the `build_identity`
field in generated `docs/program/FEATURE-STATUS.json` and the bound receipt;
this page does not duplicate that volatile value.

The generated lock binds 20 build, packaging, verification and QEMU commands;
two OVMF firmware blobs; 101 runtime files; 92 seed packages; 160 recursively
resolved binary packages; and 104 exact source-package/version sets. Every
runtime byte has a package owner, every package dependency resolves inside the
closure, and every binary package resolves to one exact source set.

## Offline proof

The local content-addressed cache retains:

- 160 exact `.deb` archives;
- 104 source-package sets containing 335 `.dsc`, upstream and Debian source
  files; and
- 16 signed-index, source-definition and archive-keyring metadata files.

`verify-dependency-archives.py --check --selftest` performs a network-free
verification. It hashes every retained file, opens every binary archive,
validates every source-member hash from its `.dsc`, checks all binary/source
relationships, and resolves every declared dependency edge from the seed set.
The receipt records zero undeclared dependency edges and source coverage for
all 160 binary packages.

The rejection suite catches missing binaries, altered archive manifests,
missing source sets, package/source relationship drift, invented offline
resolution and direct archive-byte mutation. The base lock separately catches
command and firmware drift, unresolved dependencies, missing source
relationships, unknown reverse edges and unknown roots.

## Evidence ceiling

The 1.6 GB archive cache is retained only on the current host. The exact older
`linux-libc-dev` binary and matching Linux source set still exist in Kali's
pool but are outside the current signed rolling index, so the receipt labels
them as unindexed pool recovery rather than signed-current-index retrieval.

Off-host signed custody remains EV-002 work. Licensing remains EV-003 work. A
network-disabled hermetic rebuild and signed toolchain attestation remain
EV-005 work. Those boundaries do not reopen EV-004's exact current local
dependency lock and offline resolution contract.
