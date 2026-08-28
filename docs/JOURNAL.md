# Journal

What happened, and what the tree looked like when it happened. Appended
automatically by `tools/journal.sh` from the post-commit hook — newest first.

Git stores the diff; this stores the context around it: outstanding hazard
counts, whether the docs still agreed with the code, which files moved. That is
the part that makes a change legible later, and the part nobody writes down.

## 2026-08-28 — test: record green closure gate evidence (70bbcea)

`codex/presswork-local-closure` · 31 files changed, 379 insertions(+), 378 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>31 file(s)</summary>

- `kernel/build_identity_embed.zl`
- `kernel/docs/UNRELEASED-CHANGELOG.md`
- `kernel/docs/provenance-viewer.html`
- `kernel/docs/receipts/app-lifecycle-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-bios32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-bios64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-native-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-raw-bios-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-routes-qemu-2026-08-22.json`
- `kernel/docs/receipts/benchmark-host-2026-08-23.json`
- _…and 19 more_

</details>

## 2026-08-28 — Merge pull request #6 from RoyX4/design/presswork (682b802)

`main` · 148 files changed, 25192 insertions(+), 9092 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

## 2026-08-28 — chore: refresh complete closure evidence (14f3c2c)

`codex/presswork-local-closure` · 35 files changed, 1139 insertions(+), 889 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>35 file(s)</summary>

- `kernel/build_identity_embed.zl`
- `kernel/docs/UNRELEASED-CHANGELOG.md`
- `kernel/docs/provenance-viewer.html`
- `kernel/docs/receipts/app-lifecycle-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-bios32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-bios64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-native-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-raw-bios-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-routes-qemu-2026-08-22.json`
- `kernel/docs/receipts/benchmark-host-2026-08-23.json`
- _…and 23 more_

</details>

## 2026-08-28 — fix(evidence): derive wrapper inventory counts (ca91510)

`codex/presswork-local-closure` · 1 file changed, 24 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `kernel/tools/generators/gen-evidence-registry.py`

</details>

## 2026-08-28 — fix(test): synchronize the network fetch gate (f3a929c)

`codex/presswork-local-closure` · 3 files changed, 55 insertions(+), 39 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>3 file(s)</summary>

- `kernel/tools/checks/check-land-gate.py`
- `kernel/tools/checks/verify-net.sh`
- `kernel/tools/probes/probe-net.py`

</details>

## 2026-08-28 — chore: refresh system repair provenance (51ea034)

`codex/presswork-local-closure` · 9 files changed, 39 insertions(+), 39 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>9 file(s)</summary>

- `kernel/build_identity_embed.zl`
- `kernel/docs/receipts/source-snapshot-2026-08-24.json`
- `kernel/docs/receipts/source-snapshot-build-inputs-2026-08-24.tar`
- `kernel/metadata/build-graph.json`
- `kernel/metadata/build-identity.json`
- `kernel/metadata/dependency-lock.json`
- `kernel/metadata/license-registry.json`
- `kernel/metadata/toolchain-manifest.json`
- `kernel/metadata/wrapper-registry.json`

</details>

## 2026-08-28 — fix(test): allow rtc time to advance during boot (8422208)

`codex/presswork-local-closure` · 1 file changed, 21 insertions(+), 7 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `kernel/tools/checks/verify-clock.sh`

</details>

## 2026-08-28 — fix(desktop): prevent recursive system panel frames (c3c4192)

`codex/presswork-local-closure` · 5 files changed, 12 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>5 file(s)</summary>

- `kernel/app_manifest_embed.zl`
- `kernel/metadata/app-manifest.json`
- `kernel/src/kernel.zl`
- `kernel/tools/generators/gen-app-manifest.py`
- `kernel/tools/probes/probe-app-routes.py`

</details>

## 2026-08-28 — chore: refresh launch coverage provenance (93011aa)

`codex/presswork-local-closure` · 9 files changed, 27 insertions(+), 27 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>9 file(s)</summary>

- `kernel/build_identity_embed.zl`
- `kernel/docs/receipts/source-snapshot-2026-08-24.json`
- `kernel/docs/receipts/source-snapshot-build-inputs-2026-08-24.tar`
- `kernel/metadata/build-graph.json`
- `kernel/metadata/build-identity.json`
- `kernel/metadata/dependency-lock.json`
- `kernel/metadata/license-registry.json`
- `kernel/metadata/toolchain-manifest.json`
- `kernel/metadata/wrapper-registry.json`

</details>

## 2026-08-28 — fix(desktop): restore complete app launch coverage (b6eb558)

`codex/presswork-local-closure` · 8 files changed, 67 insertions(+), 22 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>8 file(s)</summary>

- `kernel/app_manifest_embed.zl`
- `kernel/build_identity_embed.zl`
- `kernel/docs/evidence/app-registry-truth-2026-08-22.md`
- `kernel/metadata/app-manifest.json`
- `kernel/metadata/build-identity.json`
- `kernel/src/kernel.zl`
- `kernel/tools/generators/gen-app-evidence.py`
- `kernel/tools/probes/probe-app-routes.py`

</details>

## 2026-08-28 — chore: refresh register route provenance (42ffdc9)

`codex/presswork-local-closure` · 9 files changed, 29 insertions(+), 29 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>9 file(s)</summary>

- `kernel/build_identity_embed.zl`
- `kernel/docs/receipts/source-snapshot-2026-08-24.json`
- `kernel/docs/receipts/source-snapshot-build-inputs-2026-08-24.tar`
- `kernel/metadata/build-graph.json`
- `kernel/metadata/build-identity.json`
- `kernel/metadata/dependency-lock.json`
- `kernel/metadata/license-registry.json`
- `kernel/metadata/toolchain-manifest.json`
- `kernel/metadata/wrapper-registry.json`

</details>

## 2026-08-28 — fix(desktop): complete presswork register routes (63b9e6f)

`codex/presswork-local-closure` · 8 files changed, 289 insertions(+), 148 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>8 file(s)</summary>

- `kernel/app_manifest_embed.zl`
- `kernel/build_identity_embed.zl`
- `kernel/metadata/app-manifest.json`
- `kernel/metadata/build-identity.json`
- `kernel/src/kernel.zl`
- `kernel/tools/checks/check-shell-layout.py`
- `kernel/tools/probes/probe-app-lifecycle.py`
- `kernel/tools/probes/probe-app-routes.py`

</details>

## 2026-08-28 — chore: refresh raw loader provenance (02e4110)

`codex/presswork-local-closure` · 8 files changed, 38 insertions(+), 38 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>8 file(s)</summary>

- `kernel/build_identity_embed.zl`
- `kernel/docs/receipts/source-snapshot-2026-08-24.json`
- `kernel/docs/receipts/source-snapshot-build-inputs-2026-08-24.tar`
- `kernel/metadata/build-graph.json`
- `kernel/metadata/build-identity.json`
- `kernel/metadata/dependency-lock.json`
- `kernel/metadata/license-registry.json`
- `kernel/metadata/toolchain-manifest.json`

</details>

## 2026-08-28 — fix(boot): raise raw loader capacity (adf8d1a)

`codex/presswork-local-closure` · 6 files changed, 45 insertions(+), 37 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>6 file(s)</summary>

- `kernel/boot/link-raw.ld`
- `kernel/boot/raw_boot.asm`
- `kernel/boot/raw_entry.S`
- `kernel/docs/reference/system/memory-model.md`
- `kernel/src/arch/x86/memmap.h`
- `kernel/src/core/arena.c`

</details>

## 2026-08-28 — chore: refresh rail gate provenance (f1256f9)

`codex/presswork-local-closure` · 9 files changed, 97 insertions(+), 85 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>9 file(s)</summary>

- `kernel/build_identity_embed.zl`
- `kernel/docs/receipts/source-snapshot-2026-08-24.json`
- `kernel/docs/receipts/source-snapshot-build-inputs-2026-08-24.tar`
- `kernel/metadata/build-graph.json`
- `kernel/metadata/build-identity.json`
- `kernel/metadata/dependency-lock.json`
- `kernel/metadata/license-registry.json`
- `kernel/metadata/toolchain-manifest.json`
- `kernel/metadata/wrapper-registry.json`

</details>

## 2026-08-28 — test: gate the presswork rail route (25ec889)

`codex/presswork-local-closure` · 3 files changed, 16 insertions(+), 3 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>3 file(s)</summary>

- `gates/land-gate.sh`
- `kernel/tools/checks/check-land-gate.py`
- `kernel/tools/probes/probe-rail.py`

</details>

## 2026-08-28 — chore: refresh presswork application manifest (57931c2)

`codex/presswork-local-closure` · 2 files changed, 2 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `kernel/app_manifest_embed.zl`
- `kernel/metadata/app-manifest.json`

</details>

## 2026-08-28 — test: record green presswork landing gate (1c9a8a8)

`codex/presswork-close` · 34 files changed, 466 insertions(+), 456 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>34 file(s)</summary>

- `docs/program/FEATURE-STATUS.json`
- `docs/program/PARTIAL-CLOSURE.json`
- `kernel/build_identity_embed.zl`
- `kernel/docs/provenance-viewer.html`
- `kernel/docs/receipts/app-lifecycle-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-bios32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-bios64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-native-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-raw-bios-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-routes-qemu-2026-08-22.json`
- _…and 22 more_

</details>

## 2026-08-28 — fix(evidence): bind final artifacts before release joins (26b912b)

`codex/presswork-close` · 37 files changed, 737 insertions(+), 673 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>37 file(s)</summary>

- `gates/land-gate.sh`
- `kernel/build_identity_embed.zl`
- `kernel/docs/UNRELEASED-CHANGELOG.md`
- `kernel/docs/provenance-viewer.html`
- `kernel/docs/receipts/app-lifecycle-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-bios32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-bios64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-native-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-raw-bios-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-routes-qemu-2026-08-22.json`
- _…and 25 more_

</details>

## 2026-08-28 — fix(gates): repair executable and inventory closure (3edcae4)

`codex/presswork-close` · 41 files changed, 853 insertions(+), 601 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>41 file(s)</summary>

- `gates/land-gate.sh`
- `kernel/build_identity_embed.zl`
- `kernel/docs/receipts/app-lifecycle-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-bios32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-bios64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-native-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-raw-bios-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-routes-qemu-2026-08-22.json`
- `kernel/docs/receipts/benchmark-host-2026-08-23.json`
- `kernel/docs/receipts/cpu-fault-invalid-opcode-qemu-2026-08-23.json`
- _…and 29 more_

</details>

## 2026-08-27 — fix(gates): close presswork integration gaps (c0a94fa)

`codex/presswork-close` · 10 files changed, 294 insertions(+), 222 deletions(-)

| | |
|---|---|
| EFI truncation sites | ? |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | STALE |

<details><summary>10 file(s)</summary>

- `TODO.md`
- `docs/evidence/grub-bios64-unwatched-2026-08-27.md`
- `docs/program/FEATURE-STATUS.json`
- `docs/program/PARTIAL-CLOSURE.json`
- `gates/land-gate.sh`
- `kernel/HANDOFF.md`
- `kernel/metadata/wrapper-registry.json`
- `kernel/src/core/README.md`
- `kernel/tools/checks/check-land-gate.py`
- `kernel/tools/checks/check-zl-dispatch.py`

</details>

## 2026-08-27 — chore: boot receipts and build identity from the verified gate runs (d6c860d)

`design/presswork` · 6 files changed, 53 insertions(+), 26 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | STALE |

<details><summary>6 file(s)</summary>

- `docs/JOURNAL.md`
- `kernel/build_identity_embed.zl`
- `kernel/docs/receipts/app-manifest-native-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-raw-bios-qemu-2026-08-22.json`
- `kernel/metadata/boot-media-ids.json`
- `kernel/metadata/build-identity.json`

</details>

## 2026-08-27 — fix(gates): a crashed emulator was reported as a failed kernel, by all five (144941c)

`design/presswork` · 11 files changed, 254 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | STALE |

<details><summary>11 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/evidence/qemu-segfaults-2026-08-27.md`
- `kernel/HANDOFF.md`
- `kernel/tools/checks/qemu-crash-selftest.sh`
- `kernel/tools/checks/qemu-crash.sh`
- `kernel/tools/checks/verify-64.sh`
- `kernel/tools/checks/verify-efi.sh`
- `kernel/tools/checks/verify-iso.sh`
- `kernel/tools/checks/verify-raw.sh`
- `kernel/verify.sh`
- `tools/hazard-scan.sh`

</details>

## 2026-08-27 — fix(gates): normalise the app count in the golden transcript, not just the sha (f469db8)

`design/presswork` · 4 files changed, 78 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>4 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/evidence/one-fact-many-copies-2026-08-27.md`
- `kernel/tests/fixtures/golden.txt`
- `kernel/verify.sh`

</details>

## 2026-08-27 — fix(gates): the kernel printed a hardcoded app count, so every gate hardcoded one too (b71cf81)

`design/presswork` · 7 files changed, 95 insertions(+), 14 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>7 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/evidence/truncation-incident-2026-08-27.md`
- `kernel/build_identity_embed.zl`
- `kernel/metadata/boot-media-ids.json`
- `kernel/metadata/build-identity.json`
- `kernel/tools/generators/gen-app-manifest.py`
- `kernel/verify.sh`

</details>

## 2026-08-27 — docs: journal for 2c09b16 (2bf82c0)

`design/presswork` · 1 file changed, 19 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-27 — docs: the 64-bit BIOS boot route is broken and no gate watches it (2c09b16)

`design/presswork` · 3 files changed, 119 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/evidence/grub-bios64-unwatched-2026-08-27.md`
- `kernel/HANDOFF.md`

</details>

## 2026-08-27 — docs: register check-version.py and its selftest in the checks capsule (dbfa270)

`design/presswork` · 3 files changed, 32 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>3 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`
- `kernel/tools/checks/README.md`

</details>

## 2026-08-27 — fix(identity): the OS reported three different versions, two of them on screen at once (b088f2c)

`design/presswork` · 24 files changed, 542 insertions(+), 105 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | STALE |

<details><summary>24 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/evidence/one-fact-many-copies-2026-08-27.md`
- `kernel/HANDOFF.md`
- `kernel/app_manifest_embed.zl`
- `kernel/build_identity_embed.zl`
- `kernel/docs/receipts/app-manifest-grub-bios32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi32-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-grub-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-native-uefi64-qemu-2026-08-22.json`
- `kernel/docs/receipts/app-manifest-raw-bios-qemu-2026-08-22.json`
- `kernel/metadata/app-manifest.json`
- `kernel/metadata/boot-media-ids.json`
- _…and 12 more_

</details>

## 2026-08-27 — feat(desktop): integrate presswork shell with current main (5fc72be)

`codex/presswork-close` · 152 files changed, 8805 insertions(+), 6956 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>64 file(s)</summary>

- `.github/scripts/render-desktop.sh`
- `.github/workflows/desktop-shot.yml`
- `.ultra/TENSIONS.md`
- `TODO.md`
- `docs/PROJECT-STATUS.md`
- `docs/evidence/DIRECTORY-DOCUMENTATION-2026-08-26.md`
- `docs/program/APPLICATIONS.md`
- `docs/program/FEATURE-MAP.md`
- `docs/program/PHASES.md`
- `docs/program/PRODUCT-IMPLEMENTATION-ORDER.md`
- `docs/program/README.md`
- `docs/program/VALIDATION-RECEIPT.md`
- _…and 52 more_

</details>

## 2026-08-27 — docs: register run-all.sh in the tests/host capsule, and the journal for 116b88a (c5ed985)

`design/presswork` · 2 files changed, 26 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `kernel/tests/host/README.md`

</details>

## 2026-08-27 — fix(gates): a host test that stopped LINKING stopped RUNNING, and nothing said so (116b88a)

`design/presswork` · 9 files changed, 300 insertions(+), 46 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | STALE |

<details><summary>9 file(s)</summary>

- `docs/JOURNAL.md`
- `kernel/build_identity_embed.zl`
- `kernel/metadata/build-identity.json`
- `kernel/src/graphics/windowing/wm.c`
- `kernel/tests/host/run-all.sh`
- `kernel/tests/host/settingstest.c`
- `kernel/tests/host/toasttest.c`
- `kernel/tests/host/wmtest.c`
- `kernel/tests/host/wmtest_feel.c`

</details>

## 2026-08-27 — fix(term): break the shell's wrap on words, and hang the continuation (7f42013)

`design/presswork` · 8 files changed, 311 insertions(+), 51 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>8 file(s)</summary>

- `docs/evidence/truncation-incident-2026-08-27.md`
- `kernel/app_manifest_embed.zl`
- `kernel/build_identity_embed.zl`
- `kernel/metadata/app-manifest.json`
- `kernel/metadata/build-identity.json`
- `kernel/src/graphics/windowing/term.c`
- `kernel/src/kernel.zl`
- `kernel/tests/host/termwrap.c`

</details>

## 2026-08-27 — fix(files): stop drawing the window's own name, and design the empty state (d1586a3)

`design/presswork` · 5 files changed, 36 insertions(+), 13 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>5 file(s)</summary>

- `kernel/app_manifest_embed.zl`
- `kernel/build_identity_embed.zl`
- `kernel/metadata/app-manifest.json`
- `kernel/metadata/build-identity.json`
- `kernel/src/kernel.zl`

</details>

## 2026-08-27 — docs: fix a dangling link and refresh the checks capsule (b49158f)

`design/presswork` · 3 files changed, 67 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/design/desktop-deciding.md`
- `kernel/tools/checks/README.md`

</details>

## 2026-08-27 — docs: record pull request status fix (a27cd74)

`main` · 1 file changed, 23 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-27 — fix: preserve unverified pull request status (ed210af)

`codex/full-program` · 7 files changed, 35 insertions(+), 21 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>7 file(s)</summary>

- `.github/workflows/docs.yml`
- `TODO.md`
- `docs/program/FEATURE-STATUS.json`
- `docs/program/PARTIAL-CLOSURE.json`
- `kernel/metadata/evidence-registry.json`
- `kernel/metadata/wrapper-registry.json`
- `tools/todo.sh`

</details>

## 2026-08-27 — docs: record whole-tree audit closure (2d7ea8a)

`main` · 1 file changed, 29 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-27 — fix: close whole-tree audit findings (6ebb2d8)

`codex/full-program` · 105 files changed, 2543 insertions(+), 1758 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 4 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>105 file(s)</summary>

- `TODO.md`
- `docs/PROJECT-STATUS.md`
- `docs/evidence/status-audits/README.md`
- `docs/evidence/status-audits/WHOLE-TREE-CODEX-AUDIT-2026-08-26.md`
- `docs/program/FEATURE-STATUS.json`
- `docs/program/PARTIAL-CLOSURE.json`
- `docs/program/PARTIAL-CLOSURE.md`
- `docs/program/README.md`
- `freestanding/runtime_kernel.c`
- `kernel/README.md`
- `kernel/app_manifest_embed.zl`
- `kernel/boot/boot64.S`
- _…and 93 more_

</details>

## 2026-08-26 — fix: reconcile evidence build identities (7686f86)

`codex/full-program` · 34 files changed, 5434 insertions(+), 5187 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | STALE |

<details><summary>34 file(s)</summary>

- `.ultra/TENSIONS.md`
- `docs/program/FEATURE-STATUS.json`
- `kernel/build_identity_embed.zl`
- `kernel/docs/UNRELEASED-CHANGELOG.md`
- `kernel/docs/provenance-viewer.html`
- `kernel/docs/receipts/source-snapshot-2026-08-24.json`
- `kernel/docs/receipts/source-snapshot-build-inputs-2026-08-24.tar`
- `kernel/metadata/accessibility-registry.json`
- `kernel/metadata/build-graph.json`
- `kernel/metadata/build-identity.json`
- `kernel/metadata/decision-ledger.json`
- `kernel/metadata/dependency-lock.json`
- _…and 22 more_

</details>

## 2026-08-26 — docs: make research index self-contained (69f11cf)

`main` · 1 file changed, 9 insertions(+), 9 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `kernel/docs/research/starred-repository-research.md`

</details>

## 2026-08-26 — docs: document every project directory (c9e6e69)

`codex/full-program` · 875 files changed, 18194 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 0 |
| docs vs tree | agree |

<details><summary>875 file(s)</summary>

- `.design/AGENTS.md`
- `.design/CLAUDE.md`
- `.design/README.md`
- `.design/RULES.md`
- `.design/SOP.md`
- `.design/STATUS.md`
- `.design/TODO.md`
- `.design/VALUES.md`
- `.github/AGENTS.md`
- `.github/CLAUDE.md`
- `.github/README.md`
- `.github/RULES.md`
- _…and 863 more_

</details>

## 2026-08-25 — fix: align boot receipt harness paths (a5c6f4e)

`main` · 5 files changed, 11 insertions(+), 7 deletions(-)

| | |
|---|---|
| EFI truncation sites | 7 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>5 file(s)</summary>

- `kernel/tests/host/efi_runtime_diag_test.py`
- `kernel/tools/checks/verify-64.sh`
- `kernel/tools/checks/verify-efi.sh`
- `kernel/tools/checks/verify-iso.sh`
- `kernel/tools/checks/verify-raw.sh`

</details>

## 2026-08-25 — fix: repair boot verification after restructure (4b53256)

`main` · 4 files changed, 60 insertions(+), 27 deletions(-)

| | |
|---|---|
| EFI truncation sites | 7 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>4 file(s)</summary>

- `kernel/tests/fixtures/golden.txt`
- `kernel/tests/host/efi_runtime_diag_test.py`
- `kernel/tools/generators/write-app-manifest-boot-receipt.py`
- `kernel/verify.sh`

</details>

## 2026-08-25 — fix: include reorganized language frontend in kernel builds (55078d3)

`main` · 5 files changed, 19 insertions(+), 5 deletions(-)

| | |
|---|---|
| EFI truncation sites | 7 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>5 file(s)</summary>

- `kernel/build.sh`
- `kernel/build64.sh`
- `kernel/buildefi.sh`
- `kernel/tools/checks/check-build-contract.py`
- `kernel/tools/images/mkdisk.sh`

</details>

## 2026-08-25 — fix: preserve kernel build diagnostics in CI (f0061fd)

`main` · 4 files changed, 20 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 7 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>4 file(s)</summary>

- `kernel/tools/checks/verify-efi.sh`
- `kernel/tools/checks/verify-iso.sh`
- `kernel/tools/checks/verify-raw.sh`
- `kernel/verify.sh`

</details>

## 2026-08-25 — docs: record build-identity repair (08b030c)

`main` · 2 files changed, 32 insertions(+)

| | |
|---|---|
| EFI truncation sites | 7 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>2 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`

</details>

## 2026-08-25 — fix: generate exact build identity per route (832b4f5)

`HEAD` · 12 files changed, 45 insertions(+), 17 deletions(-)

| | |
|---|---|
| EFI truncation sites | 7 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>12 file(s)</summary>

- `gates/check-contained-gate.py`
- `gates/land-gate.sh`
- `gates/run-land-gate-contained.sh`
- `kernel/build.sh`
- `kernel/build64.sh`
- `kernel/buildefi.sh`
- `kernel/metadata/README.md`
- `kernel/tools/checks/check-build-contract.py`
- `kernel/tools/checks/verify-efi.sh`
- `kernel/tools/checks/verify-iso.sh`
- `kernel/tools/checks/verify-raw.sh`
- `kernel/tools/images/mkdisk.sh`

</details>

## 2026-08-25 — docs: record path-reference repair (f7aaa08)

`main` · 1 file changed, 17 insertions(+)

| | |
|---|---|
| EFI truncation sites | 7 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-25 — docs: repair restructured path references (fa25f1f)

`codex/full-program` · 1 file changed, 13 insertions(+), 12 deletions(-)

| | |
|---|---|
| EFI truncation sites | 7 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `.ultra/TENSIONS.md`

</details>

## 2026-08-25 — refactor: reorganize zl and zlos repository layout (4dfdbd9)

`codex/full-program` · 676 files changed, 29188 insertions(+), 6020 deletions(-)

| | |
|---|---|
| EFI truncation sites | 7 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>676 file(s)</summary>

- `.github/scripts/render-desktop.sh`
- `.github/workflows/boot.yml`
- `.github/workflows/desktop-shot.yml`
- `.github/workflows/nightly.yml`
- `.gitignore`
- `.ultra/TENSIONS.md`
- `.vscode/tasks.json`
- `CLAUDE.md`
- `README.md`
- `bench/README.md`
- `bench/ab.sh`
- `bench/abtree.sh`
- _…and 664 more_

</details>

## 2026-08-20 — chore: journal ride-along for 75c8997 (b8a00ec)

`main` · 1 file changed, 18 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-24 — chore: journal ride-along for e8faa50 (72630cc)

`secret/desktop-foundry` · 2 files changed, 19 insertions(+), 3 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`

</details>

## 2026-08-24 — design: preserve Foundry desktop mock (e8faa50)

`secret/desktop-foundry` · 2 files changed, 781 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `.design/log.json`
- `docs/design/foundry-desktop.html`

</details>

## 2026-08-20 — docs: round 2 pushed - origin/main is 67ce0fd (75c8997)

`main` · 2 files changed, 47 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/evidence/MERGE-ROUND-2.md`

</details>

## 2026-08-20 — test: update golden.txt for the round-2 boot output (67ce0fd)

`main` · 3 files changed, 21 insertions(+), 3 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`
- `kernel/golden.txt`

</details>

## 2026-08-20 — chore: move the corrupt examples/Zaccoding.zl out of the tree (4dbc5c8)

`main` · 2 files changed, 22 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`

</details>

## 2026-08-20 — docs: correct the superseded-branch reasoning - git diff A..B is not a merge (c25c7ea)

`main` · 3 files changed, 30 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `.gitignore`
- `docs/evidence/MERGE-ROUND-2.md`
- `examples/Zaccoding.zl`

</details>

## 2026-08-20 — merge: main into compassionate-curie - round-2 catch-up (52d6dc9)

`claude/compassionate-curie-a0599c` · 305 files changed, 31489 insertions(+), 2009 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>16 file(s)</summary>

- `freestanding/runtime_kernel.c`
- `kernel/.gitignore`
- `kernel/HANDOFF.md`
- `kernel/SOURCES`
- `kernel/apps_registry.zl`
- `kernel/browser.c`
- `kernel/docs/visual-speed-northstar.md`
- `kernel/fb.c`
- `kernel/golden.txt`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/fbtext.c`
- `kernel/hosttest/wmtest.c`
- _…and 4 more_

</details>

## 2026-08-20 — fix(memmap): the arena was 6 MiB out of place, and nothing could see it (018abc6)

`main` · 3 files changed, 36 insertions(+), 5 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `kernel/arena.c`
- `kernel/check-memmap.sh`
- `kernel/memmap.h`

</details>

## 2026-08-20 — merge: main into desktop/browser-next - catching up 150 commits (76ea304)

`desktop/browser-next` · 321 files changed, 751874 insertions(+), 3499 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>18 file(s)</summary>

- `.gitignore`
- `.ultra/STATE.md`
- `CLAUDE.md`
- `freestanding/runtime_kernel.c`
- `kernel/HANDOFF.md`
- `kernel/SOURCES`
- `kernel/arena.c`
- `kernel/browser.c`
- `kernel/check-memmap.sh`
- `kernel/fb.c`
- `kernel/hosttest/build.sh`
- `kernel/hosttest/wmtest.c`
- _…and 6 more_

</details>

## 2026-08-19 — docs(fleet): wguard.sh was listed as both a lead and verified - resolved (fc6fac6)

`fleet/audit-2026-08-19` · 2 files changed, 25 insertions(+), 7 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-gates-that-cannot-fail.md`

</details>

## 2026-08-19 — docs(fleet): wguard.sh cannot see the flag line it exists to guard - verified (c953658)

`fleet/audit-2026-08-19` · 3 files changed, 90 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-gates-that-cannot-fail.md`
- `docs/fleet/FLEET-SYNTHESIS.md`

</details>

## 2026-08-19 — docs(fleet): HTTP_REDIRECT is produced and consumed by nobody (305ab8d)

`fleet/audit-2026-08-19` · 3 files changed, 57 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/FLEET-SYNTHESIS.md`
- `docs/fleet/VERIFICATION-LOG.md`

</details>

## 2026-08-19 — chore(fleet): journal refresh from the post-commit hook (1a99606)

`fleet/audit-2026-08-19` · 1 file changed, 19 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-19 — docs(fleet): the buffer-full deadlock, and why neither function is wrong (dea6d4d)

`fleet/audit-2026-08-19` · 3 files changed, 78 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/FLEET-SYNTHESIS.md`
- `docs/fleet/VERIFICATION-LOG.md`

</details>

## 2026-08-19 — docs(fleet): record the measured run stats, including the one agent that failed (6dbec20)

`fleet/audit-2026-08-19` · 2 files changed, 34 insertions(+), 9 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/README.md`

</details>

## 2026-08-19 — docs(fleet): the synthesis - one ranked list across all four fronts (7fcda5a)

`fleet/audit-2026-08-19` · 3 files changed, 190 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/FLEET-SYNTHESIS.md`
- `docs/fleet/README.md`

</details>

## 2026-08-19 — docs(fleet): settings are written to NVMe and never read back (e4b1151)

`fleet/audit-2026-08-19` · 2 files changed, 60 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/VERIFICATION-LOG.md`

</details>

## 2026-08-19 — docs(fleet): the bug board - what survived being attacked (6c92377)

`fleet/audit-2026-08-19` · 2 files changed, 128 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/BUG-BOARD.md`

</details>

## 2026-08-19 — docs(fleet): correct two of my own findings that adversarial verification broke (d3da1a0)

`fleet/audit-2026-08-19` · 3 files changed, 77 insertions(+), 5 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-ci-truncation-gate-is-blind.md`
- `docs/fleet/VERIFIED-ggtt-span-collision.md`

</details>

## 2026-08-19 — docs(fleet): the browser board - the engine is better than its harness (5bf762e)

`fleet/audit-2026-08-19` · 2 files changed, 145 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/BROWSER-BOARD.md`

</details>

## 2026-08-19 — docs(fleet): the driver board - 25 lenses, and a stale plan corrected in seven places (51b07f8)

`fleet/audit-2026-08-19` · 2 files changed, 152 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/DRIVER-BOARD.md`

</details>

## 2026-08-19 — docs(fleet): build_request writes 9 bytes past req[512] (0d5d28b)

`fleet/audit-2026-08-19` · 3 files changed, 137 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-http-request-overflow.md`
- `docs/fleet/README.md`

</details>

## 2026-08-19 — docs(fleet): the display driver's two failure modes are inverted (30f5fb3)

`fleet/audit-2026-08-19` · 4 files changed, 207 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>4 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-display-failure-reporting-inverted.md`
- `docs/fleet/README.md`
- `docs/fleet/VERIFICATION-LOG.md`

</details>

## 2026-08-19 — docs(fleet): the render engine is not blocked and has not been for three commits (8ee3774)

`fleet/audit-2026-08-19` · 3 files changed, 152 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/README.md`
- `docs/fleet/UNBLOCKED-render-engine.md`

</details>

## 2026-08-19 — docs(fleet): verify anim_tick by hand - the 100 Hz clock is already in wm.c (611b4c1)

`fleet/audit-2026-08-19` · 3 files changed, 68 insertions(+), 1 deletion(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/LOOK-BOARD.md`
- `docs/fleet/VERIFICATION-LOG.md`

</details>

## 2026-08-19 — docs(fleet): the look board - the animation system has no time base (780eeed)

`fleet/audit-2026-08-19` · 2 files changed, 169 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/LOOK-BOARD.md`

</details>

## 2026-08-19 — docs(fleet): seven more gates that cannot fail, and a live DCBAA being zeroed (284f3d3)

`fleet/audit-2026-08-19` · 3 files changed, 164 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-gates-that-cannot-fail.md`
- `docs/fleet/README.md`

</details>

## 2026-08-19 — docs(fleet): the APs have no IDT, and .ultra/STATE.md ranks turning them on first (cc17df5)

`fleet/audit-2026-08-19` · 3 files changed, 159 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>3 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-smp-bands-have-no-idt.md`
- `docs/fleet/README.md`

</details>

## 2026-08-19 — docs(fleet): the browser cluster - hover navigates, and 1032 lines of dead CSS (1cb1693)

`fleet/audit-2026-08-19` · 2 files changed, 165 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-browser-cluster.md`

</details>

## 2026-08-19 — docs(fleet): the CI truncation gate scans 6 of ~50 translation units (ea3d900)

`fleet/audit-2026-08-19` · 5 files changed, 175 insertions(+), 4 deletions(-)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>5 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`
- `docs/fleet/CRITICAL-ci-truncation-gate-is-blind.md`
- `docs/fleet/README.md`
- `docs/fleet/VERIFICATION-LOG.md`

</details>

## 2026-08-19 — docs(fleet): hand-verified findings from the 100-agent audit (50a5679)

`fleet/audit-2026-08-19` · 8 files changed, 1113 insertions(+)

| | |
|---|---|
| EFI truncation sites | 0 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>8 file(s)</summary>

- `docs/fleet/CRITICAL-browser-urlbar-keys.md`
- `docs/fleet/CRITICAL-gpuring-bar-truncation.md`
- `docs/fleet/GROUND-TRUTH-CORRECTIONS.md`
- `docs/fleet/PANEL-POWER-IS-REACHABLE.md`
- `docs/fleet/README.md`
- `docs/fleet/VERIFICATION-LOG.md`
- `docs/fleet/VERIFIED-WM-SNAP.md`
- `docs/fleet/VERIFIED-ggtt-span-collision.md`

</details>

## 2026-08-19 — chore: regenerated journal/TODO (post-commit hook) (2c2092f)

`main` · 1 file changed, 17 insertions(+)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |
## 2026-08-19 — docs: the journal entry for the commit that added the journal (b3f5fd2)

`ci/gates-and-agent-brief` · 1 file changed, 2 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 34 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-19 — chore: regenerated journal/TODO (post-commit hook) (3e6497c)

`main` · 1 file changed, 18 insertions(+)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>1 file(s)</summary>

- `docs/JOURNAL.md`

</details>

## 2026-08-19 — chore: the journal and TODO the post-commit hook regenerated (0a4bc8f)

`main` · 2 files changed, 24 insertions(+), 16 deletions(-)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `TODO.md`
- `docs/JOURNAL.md`

</details>

## 2026-08-19 — Merge branch 'ci/gates-and-agent-brief' (18f34e3)

`main` · 29 files changed, 2733 insertions(+), 73 deletions(-)

| | |
|---|---|
| EFI truncation sites | 14 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | STALE |

<details><summary>2 file(s)</summary>

- `.gitignore`
- `kernel/hosttest/wmshot.c`

</details>


## 2026-08-19 — ci: write down what happens, keep the docs true, regenerate the TODO (cb17faf)

`ci/gates-and-agent-brief` · 9 files changed, 719 insertions(+), 2 deletions(-)

| | |
|---|---|
| EFI truncation sites | 34 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>9 file(s)</summary>

- `.github/workflows/docs.yml`
- `TODO.md`
- `docs/JOURNAL.md`
- `tools/doc-check-ignore.txt`
- `tools/doc-check.sh`
- `tools/doc-claims.txt`
- `tools/install-hooks.sh`
- `tools/journal.sh`
- `tools/todo.sh`

</details>

## 2026-08-18 — ci: preflight.sh and a pre-push hook, since main cannot be protected (46f4bd2)

`ci/gates-and-agent-brief` · 2 files changed, 132 insertions(+)

| | |
|---|---|
| EFI truncation sites | 34 |
| pinned engine divergences | 2 |
| baselined doc rot | 19 |
| docs vs tree | agree |

<details><summary>2 file(s)</summary>

- `tools/install-hooks.sh`
- `tools/preflight.sh`

</details>
