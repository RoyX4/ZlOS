# Current boot-feature classification

Date: 2026-08-28

This receipt maps existing current-build boot evidence into the canonical
906-feature ledger. It changes classification, not boot code, artifacts, or the
evidence ceiling of an earlier run.

## Subject identity

The joined artifact, initialization, and host-test registries agree on the
exact current `build_identity` recorded in generated
`docs/program/FEATURE-STATUS.json`; this page does not duplicate that volatile
value.

The artifact registry contains nine exact reproducible artifacts and six
current-build QEMU routes:

- raw BIOS;
- native UEFI64;
- GRUB BIOS32 and UEFI32;
- GRUB BIOS64 and UEFI64.

The initialization registry binds 18 ordered stages to all six routes. The
current host receipt binds 76 classified targets and includes a passing
288-check typed-handover mutation gate.

## Promoted rows

The following rows move only from `PLANNED_UNPROVED` to `PARTIAL_CURRENT`:

| Feature | Current bounded evidence | Still open |
|---|---|---|
| BT-001 BIOS boot | raw and GRUB BIOS QEMU routes, exact artifacts and identities | physical BIOS, network/recovery layouts |
| BT-002 UEFI boot | native UEFI64 and GRUB UEFI32/64 QEMU routes | physical firmware/GOP, complete fallback and hostile-map coverage |
| BT-003 raw-sector boot | exact reproducible raw payload/image and QEMU ready path | versioned recovery layout, physical media |
| BT-004 Multiboot boot | four GRUB routes plus hostile typed-handover host checks | complete module admission, physical proof |
| BT-006 boot-media matrix | six retained route/artifact bindings | network boot and the complete partition/filesystem/configuration cross-product |
| BT-008 typed boot handover | all entry families, 288 host checks and six QEMU ready paths | exact origin/kernel identities, entropy, normalized maps, physical proof |
| BT-017 initialization DAG | 18 ordered stages with dependencies, obligations and route coverage | general deadlines, rollback, reverse teardown, physical proof |

No row is marked complete. BT-005 network boot remains planned/unproved, and
the absent combinations in BT-006 are not inferred from the six retained
routes.

## Verification

```sh
python3 tools/gen_feature_status.py --write --selftest
python3 tools/gen_feature_status.py --check --selftest
python3 tools/validate_master_program.py --self-test
tools/doc-check.sh
```

Exact current maturity counts remain in generated `program/FEATURE-STATUS.json`.
All six boot routes were rerun after the later kernel and
evidence repairs and now bind the subject identity above. No physical boot is
claimed.
