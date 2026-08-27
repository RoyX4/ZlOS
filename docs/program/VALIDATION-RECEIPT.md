# Master-program validation receipt — 2026-08-22

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
master-program: PASS: features=906 prefixes=25 phases=21 research_contracts=174 registries=611 research_docs=55 repositories=33 current_named=63 catalogue=1 games=24
registry-counts: AGT=24 APP=83 BLK=25 BUS=28 CUR=40 DEV=20 FSP=22 FUT=12 GAME=24 GPU=25 INPUT=14 MEDIA=8 NIC=16 OPS=28 PERIPH=4 PLAT=38 PWR=7 RADIO=5 SENSOR=4 SVC=116 USB=14 VM=12 ZLP=42
```

The validator also checks:

- exact prefix counts and uniqueness for the 906 canonical feature IDs;
- the ordered MP-00 through MP-20 phase set and acyclic direct-dependency
  manifest;
- all 64 core, 56 driver/app and 54 visual/browser/app research contracts;
- exact specialized-registry family counts and unique IDs;
- a valid primary phase on every driver/service/application row;
- all 63 current named implementations, the separate All Applications surface,
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
