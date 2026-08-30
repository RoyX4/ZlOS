# Master-program validation receipt — 2026-08-23

## Result

The plan-first gate passes mechanically and survived a manual contradiction
pass against the live zlOS receipt, the complete 906-row catalogue, all three
implementation backlogs, the driver/app and visual/browser deep dives, and
their adversarial corrections.

Command:

```sh
python3 tools/validate_master_program.py --self-test
```

Result:

```text
master-program self-test: PASS: missing-feature, duplicate-feature, unknown-prefix, unknown-contract
master-program: PASS: features=906 prefixes=25 phases=21 research_contracts=174 registries=609 current_named=61 catalogue=1 games=24
registry-counts: AGT=24 APP=83 BLK=25 BUS=28 CUR=38 DEV=20 FSP=22 FUT=12 GAME=24 GPU=25 INPUT=14 MEDIA=8 NIC=16 OPS=28 PERIPH=4 PLAT=38 PWR=7 RADIO=5 SENSOR=4 SVC=116 USB=14 VM=12 ZLP=42
```

The validator also checks:

- exact prefix counts and uniqueness for the 906 canonical feature IDs;
- every normalized decision, release-note change/issue and provenance-viewer
  feature reference uses canonical IDs, and the status ledger binds all hashes;
- the ordered MP-00 through MP-20 phase set and acyclic direct-dependency
  manifest;
- all 64 core, 56 driver/app and 54 visual/browser/app research contracts;
- exact specialized-registry family counts and unique IDs;
- a valid primary phase on every driver/service/application row;
- all 61 current named implementations, the separate All Applications surface,
  and GAME-001 through GAME-024;
- generated-file freshness, required suite files, final newlines, trailing
  whitespace and local Markdown links.

## Baseline and evidence ceiling

The first freeze used committed HEAD
`b8a00ec45ac1f9f955ba79ed63f0072540067d4e` plus a large concurrent uncommitted
shared implementation batch. The authoritative local behavior receipt is
`kernel/docs/evidence/performance-architecture-implementation-2026-08-22.md`; it reports
the exact host/QEMU gates and explicitly open physical gates.

No kernel build, QEMU boot or physical run was performed merely to write this
plan. Earlier receipts are not upgraded. The validator proves inventory and
dependency completeness, not implementation completion.

## Refutations incorporated during this pass

- Current zlOS already has substantive retained composition, crash-consistent
  named files, cooperative multi-process Ring 3, virtio/e1000 networking and
  browser persistence; the program preserves/deepens them rather than rebuilding
  old research assumptions.
- The first normalized driver registry missed long-tail distinct providers.
  The corrected registry adds QEMU fw_cfg/KVM clock, Apple SMC/ANS/AGX,
  OpenCores I2C, ISA PnP/8237 DMA/VM86, virtio console, SPI/ILI9341, EGA, exFAT,
  NTFS import and event/pipe/socket/legacy filesystem providers.
- Hardware evidence stays class-specific: native xHCI storage cannot prove USB
  HID/audio/video; an Intel host harness cannot prove native zlOS modesetting.
- I2C-HID remains an incomplete raw transport until reports become normalized
  input events; the destination row explicitly forbids a diagnostic-only claim.
- No-IOMMU operation is explicitly non-isolated; capability, credential,
  usercopy, package, service, app, browser and public-demo hostile cases remain
  mandatory gates.
- Every current app name is explicit, including Menu, About, Colour Picker,
  Clocks & Timers and Console (tty1); All Applications remains separate and the
  Maze/blank-ID false green remains a required mutation test.
- Office creation, print/scan, assistive providers, service lifecycle, browser
  IPC and public-demo data-plane authentication are present rather than hidden
  behind generic app/service labels.

## Completeness boundary

“Complete” means every selected product capability and every distinct observed
hardware/protocol/app/service job is retained in the program. It does not mean:

- duplicating the same AHCI, calculator or file-manager job once per donor;
- rewriting all 307 Astral recipes, 128 banan ports or 223 Serenity utilities as
  first-party zlOS code;
- listing every vendor/device ID, helper function, asset or historical branch as
  a separate product feature;
- treating a source file, build target, recipe, screenshot, mock UI or external
  Linux driver as working zlOS functionality.

Exact original names and source evidence remain in the research shelf; the
master registries normalize them into one clean-room implementation contract or
an explicit third-party compatibility package.

## Weakest links

1. The active source checkout is concurrently dirty. Product implementation
   must occur in a persistent isolated worktree and reconcile only deliberate
   live changes.
2. The 906 source catalogue is in the sibling `zl` repository. The generated
   feature map is self-contained, but regeneration requires that evidence shelf.
3. The broad future hardware list is architecture-complete at the protocol/
   controller-family level, not a promise that every PCI/USB revision works
   without its own qualification receipt.
4. ThinkPad panel, I219/dongle, physical USB class devices, Intel render/modeset,
   suspend and later architecture gates remain physical work, not plan proof.

The next authorized action is the first isolated MP-00/MP-01 implementation
slice. No later subsystem should jump ahead of its machine-checked dependencies.

## Active isolated implementation bridge — 2026-08-23

Implementation is active in the sibling worktree
`/home/roy/Documents/repos/zl-linux-master-program-foundation` on branch
`codex/master-program-foundation`, based on
`b8a00ec45ac1f9f955ba79ed63f0072540067d4e`. The changes remain uncommitted;
the branch name alone is therefore not the receipt. Read that worktree's
`kernel/HANDOFF.md` and generated registries directly until the batch is
reconciled deliberately.

Its current joined build identity is
`85027b159c9a594045c2f900e5971bb3408dd418dd61a373625425fba9030d13`.
Current machine-checked evidence covers 9 byte-reproducible artifacts, 6 QEMU
boot routes, 62/62 app identities with open-ready-close proof, 16 initialization
stages, 15 locked host tools plus 2 firmware blobs, 19/19 planted verifier
canaries, 66 mandatory landing seams, and 48 passing host targets with zero
failures. The SOURCES transaction also passes disposable stale-recovery,
incomplete-state refusal and live-owner refusal tests. The current exception
path also has a bounded checksummed crash record: 27 sanitizer-backed host
checks pass, and QEMU proves invalid-opcode vector 6, the exact `crash_test_ud2`
instruction pointer, an independently recomputed checksum, and a halted guest.

The exact 123-file build-input closure is now independently reconstructable
from a deterministic 5,294,080-byte ustar archive with SHA-256
`cb17bcd596060f72169288807375816d7c7653852eb213261247c72ea95697b2`.
Missing members, an invented custody claim, archive-hash drift and byte
corruption all fail its self-test. This remains `PARTIAL_CURRENT`: the archive
is unsigned, has zero off-host copies, is not a whole-repository snapshot and
still lives in the same uncommitted worktree.

The current kernel toolchain is now a standalone manifest rather than a version
string: 7 exact tool binaries, 4 compiler/linker/ABI lanes and 82 external
compiler/sysroot headers are byte-bound. Macro probes distinguish BIOS/raw
ILP32, Multiboot LP64 and UEFI LLP64, and environment injection fails closed.
The dependency lock also corrected two omitted native-UEFI authorities, Clang
and `lld-link`, and preserves the invoked multi-call alias separately from its
resolved binary. EV-005 remains `PARTIAL_CURRENT`: there are zero hermetic
builds, no signed toolchain attestation and no compiler/sysroot source archive.

The current build graph now joins all 123 declared inputs through 404 nodes and
663 typed edges to 268 lane-qualified logical objects and all 9 artifacts.
There are zero orphan inputs. `freestanding/zl_freestanding.h` is the one
conservative scope-only input and is labelled that way instead of being called
compiled. EV-007 remains `PARTIAL_CURRENT`: intermediate objects have no
individual binary provenance receipts and future package/service outputs are
not graph nodes.

EV-018 now has a generated 28-field, 152-byte structured-event contract and a
real bounded pointer-free core. Its dedicated receipt records 37 passing
ASan/UBSan assertions plus ILP32 and UEFI LLP64 compilation. The joined evidence
still records zero target emitters, zero QEMU/native routes, single-owner
serialization and no durable anchor or service export policy. This is a
host-proved foundation, not a shipped audit service.

EV-026 now has a generated decision/reversal/deprecation ledger. Nineteen
high-impact records are bound to exact source hashes and unique source text;
superseded decisions retain reciprocal replacement links and obsolete evidence.
All 47 labels in the older desktop/graphics decision index now have generated
source-heading, section-hash and feature-scope semantics. Repository-wide
history outside that file, independent approval, automatic migration and a
user-visible target history viewer remain explicit gaps.

EV-027 now has a generated machine ledger and human unreleased changelog. It
derives 17 current change candidates, six potential user-visible changes, three
migrations, 12 current defects/blockers and three recovery paths from lower-
level manifests without reading the joined evidence registry back into itself.
Published entries remain exactly zero: there is no version, channel, signature,
redistribution authority, complete user-data migration inventory or signed
previous generation.

EV-028 now has a generated read-only provenance model and self-contained static
HTML evidence room. It projects exact origin, licensing/signature state, 9
artifacts, 62 applications, 17 security claims, 17 changes and 11 health areas
without external requests. It explicitly shows zero cryptographic signatures
and zero admitted per-app permission grants. This remains host-static: no booted
zlOS route, live health stream, current screenshot, target assistive workflow or
authenticated portal has been proved.

This is not MP-00 completion. The same joined registry preserves 3 explicit
hardware skips, 11 non-runs, zero native-target benchmark measurements, 4 host
frame metrics over the 16.667 ms budget, zero exhaustive failure-injection
families, 4 hostile-input family gaps, no physical exact-hash artifact proof,
and a public-release block because the repository has no license file that
establishes redistribution grants for its 123 build inputs. The crash record is
not a durable crash bundle: general registers, symbolization, persistence,
recovery, and offender-only user-fault containment remain unproved.

`FEATURE-STATUS.json` now joins that evidence to all 906 canonical feature
atoms: 7 are `PROVED_CURRENT`, 22 are `PARTIAL_CURRENT`, and 877 remain
`PLANNED_UNPROVED`. That conservative default is intentional. A current source
asset without a feature-specific receipt is not silently promoted.

## Exact 23-partial closure authority — 2026-08-24

[`PARTIAL-CLOSURE.md`](PARTIAL-CLOSURE.md) and its generated JSON companion now
define the exact closure contract for every one of the 23 `PARTIAL_CURRENT`
rows. The generator rejects a missing row, unknown dependency, hidden external
authority, false local-completion claim and missing promotion rule. It reports
three remaining rows whose current contract can be closed locally and nine rows with named
external authority; the remaining rows depend on later security,
accessibility, service, provenance or target foundations.

The ledger does not convert an external event into paperwork. License choice,
trusted signing/off-host custody, physical-hardware qualification, genuine
independent rejection and an authorized signed release remain explicit gates.
Each row states deterministic, QEMU and physical/external proof separately so
future work cannot promote a host harness as product or physical evidence.

The first EV-004 closure slice now expands the dependency lock from 15 command
binaries and two firmware blobs to an installed-runtime SBOM: 91 exact runtime
files resolve through 156 recursively installed binary packages, with package,
version, architecture, source-package/version and dependency edges retained.
Eight negative mutations are rejected. The result deliberately remains partial:
zero package source archives are retained and an offline rebuild has not been
proved, so package metadata is not mislabelled reconstructable supply custody.

EV-008 is now `PROVED_CURRENT`. All 141 current shebang wrappers are hashed and
classified, including 17 legacy/action/probe scripts with no complete static
failure policy; those scripts are explicitly outside the sole supported landing
authority rather than silently treated as gates. The contained landing gate has
66 mandatory seams and its self-test plants both a masked child failure and a
masked final exit. Both turn the closure gate red. The original 23-row batch is
retained with EV-008 closed, leaving 22 current partial rows.

EV-026 now has exact generated semantics for all 47 legacy decision labels:
source heading, line, section hash, feature scope and evidence ceiling. Its
legacy semantic gap is zero. It remains partial because system-wide decisions
outside that source, independent approval, automatic migration and a booted
history viewer are still absent.

The EV-014 runner can generate a seven-process-sample v2 receipt with p50, p95
and peak values. Its first bounded run was deliberately terminated when host
load exceeded the containment tolerance and did not replace the artifact. The
frozen v1 receipt therefore remains authoritative and still records no
percentiles/peak, four over-budget metrics and zero native-target measurements.
