#!/usr/bin/env python3
"""Generate the conservative 906-row implementation maturity ledger."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import subprocess
import sys
from collections import Counter
from pathlib import Path

import validate_master_program as master


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "docs/program/FEATURE-STATUS.json"
DEFAULT_EVIDENCE_ROOT = ROOT
BUILD_INPUT_COUNT = len(json.loads(
    (ROOT / "kernel/metadata/build-identity.json").read_text()
).get("source_files_sha256", {}))
SCOPE_ONLY_COUNT = json.loads(
    (ROOT / "kernel/metadata/build-graph.json").read_text()
).get("counts", {}).get("scope_only_inputs")
HOST_COUNTS = json.loads(
    (ROOT / "kernel/tests/host/test-run-receipt.json").read_text()
).get("counts", {})

OVERRIDES = {
    "EV-001": ("PROVED_CURRENT", (("plan", "docs/program/FEATURE-MAP.md"),),
               ("python3 tools/gen_feature_status.py --check --selftest",),
               ("docs/program/FEATURE-STATUS.json",),
               ("missing feature", "duplicate feature", "unproved promotion"),
               (), "generated 906-row ledger is current; later feature receipts are still mostly absent"),
    "EV-002": ("PARTIAL_CURRENT", (("implementation", "kernel/docs/receipts/source-snapshot-2026-08-24.json"),),
               ("python3 kernel/tools/generators/gen-source-snapshot.py --check --selftest",),
               ("kernel/docs/receipts/source-snapshot-2026-08-24.json",
                "kernel/docs/receipts/source-snapshot-build-inputs-2026-08-24.tar"),
               ("missing input", "archive hash drift", "archive byte corruption", "invented custody"),
               ("0 off-host copies", "unsigned receipt", "not a whole-repository snapshot"),
               f"all {BUILD_INPUT_COUNT} exact build inputs are reconstructable, but the archive remains unsigned in the same worktree"),
    "EV-003": ("PARTIAL_CURRENT", (("implementation", "kernel/metadata/license-registry.json"),),
               ("python3 kernel/tools/generators/gen-license-registry.py --check --selftest",),
               ("kernel/metadata/license-registry.json",), ("invented grant", "false release green"),
               ("0 license files", f"{BUILD_INPUT_COUNT} inputs lack an established redistribution grant"),
               "build-input inventory exists, but public redistribution authority is absent"),
    "EV-004": ("PROVED_CURRENT", (
                   ("implementation", "kernel/metadata/dependency-lock.json"),
                   ("implementation", "kernel/docs/receipts/dependency-archives-host-2026-08-29.json")),
               ("python3 kernel/tools/generators/gen-dependency-lock.py --check --selftest",
                "python3 kernel/tools/checks/verify-dependency-archives.py --check --selftest"),
               ("kernel/metadata/dependency-lock.json",
                "kernel/docs/receipts/dependency-archives-host-2026-08-29.json"),
               ("binary drift", "missing firmware identity", "missing package",
                "unresolved package dependency", "missing source relationship",
                "archive byte drift", "invented offline resolution"),
               ("archive cache is retained only on the current host",
                "one stale Linux binary/source pair is outside the current signed rolling index",
                "off-host custody and hermetic rebuild remain EV-002/EV-005 work"),
               "20 commands, 2 firmware blobs, 101 runtime files, 160 binary archives and 104 source-package sets are exact and resolve offline with zero undeclared dependency edges"),
    "EV-005": ("PARTIAL_CURRENT", (("implementation", "kernel/metadata/toolchain-manifest.json"),
                                    ("implementation", "kernel/metadata/dependency-lock.json")),
               ("python3 kernel/tools/generators/gen-toolchain-manifest.py --check --selftest",),
               ("kernel/metadata/toolchain-manifest.json", "kernel/metadata/dependency-lock.json"),
               ("missing target lane", "wrong LLP64", "missing warning policy",
                "external header drift", "environment injection"),
               ("0 hermetic builds", "unsigned toolchain", "compiler/sysroot sources unarchived"),
               "7 tools, 4 ABI lanes and 82 external headers are exact locally, but the toolchain is not hermetic or source-custodied"),
    "EV-006": ("PROVED_CURRENT", (("implementation", "kernel/docs/receipts/reproducible-build-2026-08-22.json"),),
               ("python3 kernel/tools/checks/check-reproducible-build.py --check --selftest",),
               ("kernel/docs/receipts/reproducible-build-2026-08-22.json",),
               ("byte drift in every artifact", "missing artifact", "stale resume snapshot"), (),
               "two independent complete recipe runs match for all nine artifacts on the current build identity"),
    "EV-007": ("PARTIAL_CURRENT", (("implementation", "kernel/metadata/build-graph.json"),
                                    ("implementation", "kernel/metadata/artifact-registry.json")),
               ("python3 kernel/tools/generators/gen-build-graph.py --check --selftest",),
               ("kernel/metadata/build-graph.json", "kernel/metadata/artifact-registry.json"),
               ("missing source", "missing lane", "orphan source", "missing artifact"),
               (f"{SCOPE_ONLY_COUNT} conservative scope-only inputs", "no per-object binary receipts",
                "future package/service outputs absent"),
               f"all {BUILD_INPUT_COUNT} inputs have recipe positions across 4 lanes and the nine exact artifacts are current-build bound"),
    "EV-008": ("PROVED_CURRENT", (("implementation", "kernel/metadata/wrapper-registry.json"),
                                     ("implementation", "kernel/metadata/adversarial-registry.json")),
               ("python3 kernel/tools/generators/gen-wrapper-registry.py --check --selftest",
                "python3 kernel/tools/checks/check-land-gate.py --selftest"),
               ("kernel/metadata/wrapper-registry.json", "kernel/metadata/adversarial-registry.json"),
               ("missing wrapper", "duplicate wrapper", "missing policy", "hidden authority boundary",
                "deleted verifier", "optional verifier", "missing route", "masked final exit", "masked child failure"),
               (), "all 156 current shebang wrappers are inventoried; the contained landing authority rejects child/final exit masking"),
    "EV-009": ("PROVED_CURRENT", (("implementation", "kernel/metadata/artifact-registry.json"),
                                   ("implementation", "kernel/metadata/app-evidence.json")),
               ("python3 kernel/tools/generators/gen-artifact-registry.py --check --selftest",),
               ("kernel/metadata/artifact-registry.json", "kernel/metadata/app-evidence.json"),
               ("artifact mismatch", "wrong build identity"), (),
               "all nine exact artifacts and six QEMU boot routes are bound to the current build identity"),
    "EV-010": ("PROVED_CURRENT", (("plan", "docs/program/PROOF-GATES.md"),
                                   ("implementation", "kernel/metadata/evidence-registry.json")),
               ("python3 kernel/tools/generators/gen-evidence-registry.py --check --selftest",),
               ("docs/program/PROOF-GATES.md", "kernel/metadata/evidence-registry.json"),
               ("physical overclaim", "hidden hardware skip"), (),
               "the vocabulary and joined receipt preserve distinct evidence ceilings"),
    "EV-011": ("PROVED_CURRENT", (("plan", "docs/program/FEATURE-MAP.md"),),
               ("python3 tools/gen_feature_status.py --check --selftest",),
               ("docs/program/FEATURE-STATUS.json",), ("hidden gap", "unproved promotion"), (),
               "every feature has a conservative current maturity row; most remain planned/unproved"),
    "EV-012": ("PROVED_CURRENT", (("implementation", "kernel/metadata/test-inventory.json"),
                                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("python3 kernel/tools/run/run-host-tests.py --selftest",),
               ("kernel/metadata/test-inventory.json", "kernel/tests/host/test-run-receipt.json"),
               ("missing target", "promoted instrument", "hidden skip"), (),
               f"the current-build-bound {HOST_COUNTS.get('targets')}-target receipt records "
               f"{HOST_COUNTS.get('passed')} passes, {HOST_COUNTS.get('skipped-hardware')} "
               f"hardware skips and {HOST_COUNTS.get('not-run')} explicit manual/instrument non-runs"),
    "EV-013": ("PARTIAL_CURRENT", (("implementation", "kernel/metadata/adversarial-registry.json"),),
               ("python3 kernel/tools/generators/gen-adversarial-registry.py --check --selftest",),
               ("kernel/metadata/adversarial-registry.json",), ("23 planted verifier mutations",),
               ("one mutation per every future required field is not yet possible",),
               "23 current verifier canaries proved red; two hostile-corpus families and all globally exhaustive failure-family claims remain open"),
    "EV-014": ("PARTIAL_CURRENT", (("implementation", "kernel/docs/receipts/benchmark-host-2026-08-23.json"),
                                     ("implementation", "kernel/tools/run/run-benchmarks.py")),
               ("python3 kernel/tools/run/run-benchmarks.py --check --selftest",),
               ("kernel/docs/receipts/benchmark-host-2026-08-23.json",),
               ("hidden regressions", "distribution summary drift", "raw output drift",
                "missing executable identity", "target overclaim"),
               ("0 native-target measurements",
                "host timings were collected on one contended desktop",
                "no QEMU or physical performance distribution"),
               "all 7 current host frame metrics retain raw samples and p50/p95/peak values; current budget misses remain red and target performance remains unmeasured"),
    "EV-015": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/metadata/hardware-receipt-plan.json"),
                   ("implementation", "kernel/metadata/hardware-receipt-template.json"),
                   ("implementation", "kernel/tools/generators/gen-hardware-receipt-plan.py"),
                   ("implementation", "kernel/metadata/artifact-registry.json"),
               ),
               ("python3 kernel/tools/generators/gen-hardware-receipt-plan.py --check --selftest",
                "python3 kernel/tools/generators/gen-artifact-registry.py --check --selftest"),
               ("kernel/metadata/hardware-receipt-plan.json",
                "kernel/metadata/artifact-registry.json"),
               ("wrong artifact", "missing firmware", "missing topology",
                "missing raw evidence", "missing lifecycle", "invented physical pass",
                "foreign build", "bad media readback", "evidence path escape",
                "content digest drift", "zero-receipt overclaim"),
               ("0 of 6 physical route cases have validated receipts",
                "0 exact executable or boot-medium artifact hashes have native physical proof"),
               "six exact-hash route plans, seven lifecycle scenarios and ten raw-evidence classes are current and mutation-checked; no physical run is claimed"),
    "EV-016": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/tools/run/run-visual-receipt.py"),
                   ("implementation", "kernel/docs/receipts/visual-qemu-2026-08-29.json"),
                   ("implementation", "kernel/metadata/visual-registry.json"),
               ),
               ("python3 kernel/tools/run/run-visual-receipt.py --check --selftest",
                "python3 kernel/tools/generators/gen-visual-registry.py --check --selftest"),
               ("kernel/docs/receipts/visual-qemu-2026-08-29.json",
                "kernel/metadata/visual-registry.json"),
               ("missing route", "foreign build", "wrong artifact", "missing state",
                "image byte drift", "pixel drift", "blank capture",
                "missing serial identity", "dimension drift", "hidden variant gap"),
               ("41 historical visual assets remain unbound",
                "scale/theme/locale/accessibility/ui-state/backend matrices remain incomplete",
                "no current video or physical-display receipt"),
               "four nonblank current-artifact screenshots bind BIOS and native UEFI desktop/Paint states to exact images, serial logs, routes, QEMU binaries and build identity"),
    "EV-017": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/core/crash.c"),
                   ("implementation", "kernel/src/arch/x86/idt.c"),
                   ("implementation", "kernel/boot/gdt64.c"),
                   ("implementation", "kernel/docs/receipts/cpu-fault-invalid-opcode-qemu-2026-08-23.json"),
                   ("implementation", "kernel/docs/receipts/cpu-fault-native-uefi64-qemu-2026-08-29.json"),
                   ("implementation", "kernel/docs/receipts/cpu-fault-general-protection-native-uefi64-qemu-2026-08-29.json"),
                   ("implementation", "kernel/docs/receipts/cpu-fault-double-fault-native-uefi64-qemu-2026-08-29.json"),
                   ("implementation", "kernel/metadata/observability-registry.json")),
               ("python3 kernel/tools/checks/verify-crash.py --check --route bios32 --fault ud2 --selftest",
                "python3 kernel/tools/checks/verify-crash.py --check --route native-uefi64 --fault ud2 --selftest",
                "python3 kernel/tools/checks/verify-crash.py --check --route native-uefi64 --fault gp --selftest",
                "python3 kernel/tools/checks/verify-crash.py --check --route native-uefi64 --fault double-fault --selftest",
                "python3 kernel/tools/generators/gen-observability-registry.py --check --selftest",
                "kernel/tests/host/crashtest"),
               ("kernel/kernel.elf", "kernel/zlOS.iso",
                "kernel/zlOS-usb.img",
                "kernel/docs/receipts/cpu-fault-invalid-opcode-qemu-2026-08-23.json",
                "kernel/docs/receipts/cpu-fault-native-uefi64-qemu-2026-08-29.json",
                "kernel/docs/receipts/cpu-fault-general-protection-native-uefi64-qemu-2026-08-29.json",
                "kernel/docs/receipts/cpu-fault-double-fault-native-uefi64-qemu-2026-08-29.json"),
               ("wrong vector", "wrong checksum", "wrong symbol", "missing registers",
                "wrong register", "invented upper register", "stale artifact",
                "guest not halted", "escaped emergency stack", "hidden crash gap"),
               ("0 durable crash receipts",
                "broader exception-vector and spurious-interrupt coverage remains absent",
                "no emergency-stack guard page, stack symbols, persistence or recovery service"),
               "four exact current QEMU receipts prove versioned 240-byte crash records: BIOS32/native-UEFI64 UD2, native-UEFI64 error-code GP, and an IST1-contained double fault with independently observed halted RSP; broader vectors and durable recovery remain open"),
    "EV-018": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/tests/host/trace_event.c"),
                   ("implementation", "kernel/tests/host/trace_event.h"),
                   ("implementation", "kernel/metadata/event-schema.json"),
                   ("implementation", "kernel/metadata/observability-registry.json")),
               ("python3 kernel/tools/checks/verify-event-trace.py --check --selftest",
                "python3 kernel/tools/generators/gen-event-schema.py --check --selftest",
                "python3 kernel/tools/generators/gen-observability-registry.py --check --selftest"),
               ("kernel/docs/receipts/event-trace-host-2026-08-24.json",
                "kernel/metadata/event-schema.json", "kernel/metadata/observability-registry.json"),
               ("missing event field", "overlapping event field", "hidden overwrite",
                "invented concurrency", "invented target emitter", "hidden observability gap"),
               ("0 target emitters", "single-owner core only", "no QEMU/native route",
                "no durable tamper-evident anchor or service export policy"),
               "the current 28-field schema retains a dated 37-check host receipt; no current-build or target emitter proof exists"),
    "EV-019": ("PARTIAL_CURRENT", (("implementation", "kernel/metadata/adversarial-registry.json"),
                                     ("implementation", "kernel/tests/host/heaptest.c"),
                                     ("implementation", "kernel/src/arch/x86/page_table_txn.c")),
               ("python3 kernel/tools/generators/gen-adversarial-registry.py --check --selftest",
                "python3 kernel/tools/run/run-host-tests.py --run --selftest",
                "python3 kernel/tools/checks/write-page-table-receipt.py --check --selftest"),
               ("kernel/metadata/adversarial-registry.json",
                "kernel/tests/host/test-run-receipt.json",
                "kernel/docs/receipts/page-table-native-uefi64-qemu-2026-08-29.json"),
               ("lost allocation sweep", "page-table failure drift", "hidden failure gap"),
               ("all 7 failure families remain non-exhaustive",
                "provider, service and package injection are absent",
                "queue, I/O and lifecycle seams remain bounded subsets",
                "no physical-hardware failure-injection receipt exists"),
               "current host proof covers all 32 positions in a bounded heap transaction and all 512 page-table writes with exact rollback; no broad family is complete"),
    "EV-020": ("PARTIAL_CURRENT", (("implementation", "kernel/metadata/adversarial-registry.json"),),
               ("python3 kernel/tools/generators/gen-adversarial-registry.py --check --selftest",
                "python3 kernel/tools/checks/check-elf-permissions.py --selftest",
                "python3 kernel/tools/checks/verify-dependency-archives.py --receipt-check --selftest"),
               ("kernel/metadata/adversarial-registry.json",
                "kernel/docs/receipts/dependency-archives-host-2026-08-29.json"),
               ("lost ELF proof", "archive evidence drift", "unearned font promotion",
                "hidden hostile gap"),
               ("no font parser/corpus exists", "no typed IPC envelope parser/corpus exists"),
               "7 of 9 hostile families have current executed host proof; minimized ELF and archive cases now join the sanitizer-backed PNG/network/web/certificate/filesystem evidence"),
    "EV-021": ("PARTIAL_CURRENT", (("implementation", "kernel/metadata/performance-registry.json"),
                                     ("implementation", "kernel/tools/generators/gen-performance-registry.py")),
               ("python3 kernel/tools/generators/gen-performance-registry.py --check --selftest",
                "python3 kernel/tools/run/run-benchmarks.py --check --selftest",
                "python3 kernel/tools/run/run-build-benchmark.py --check --selftest",
                "python3 kernel/tools/run/run-host-tests.py --selftest"),
               ("kernel/metadata/performance-registry.json",
                "kernel/docs/receipts/benchmark-host-2026-08-23.json",
                "kernel/docs/receipts/build-benchmark-host-2026-08-29.json",
                "kernel/tests/host/test-run-receipt.json"),
               ("hidden product-build gap", "hidden regression", "frame summary drift",
                "invented product latency", "invented target proof"),
               ("any current host category outside its declared guard remains an open regression",
                "the product artifact build has no current timed distribution",
                "input/I/O/network/launch/memory values are host-gate runtimes, not product latency",
                "0 native-target and 0 physical categories"),
               "all seven categories are measured on the current host; measured regressions remain red and product/target latency remains unmeasured"),
    "EV-022": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/tools/checks/check-visual-goldens.py"),
                   ("implementation", "kernel/metadata/visual-golden-registry.json"),
                   ("implementation", "kernel/metadata/visual-registry.json"),
               ),
               ("python3 kernel/tools/checks/check-visual-goldens.py --check --selftest",
                "python3 kernel/tools/generators/gen-visual-registry.py --check --selftest"),
               ("kernel/metadata/visual-golden-registry.json",
                "kernel/metadata/visual-registry.json"),
               ("missing golden", "hidden variant gap", "stable region defect",
                "declared live region noise", "dimension drift", "unearned variant promotion"),
               ("goldens cover only 2 routes and 2 states",
                "scale/theme/locale/accessibility/ui-state/backend matrix incomplete",
                "no physical display comparison"),
               "four current strict RGB goldens reject every stable-region pixel change while four named live-telemetry masks prevent timing noise"),
    "EV-023": ("PARTIAL_CURRENT", (("implementation", "kernel/metadata/accessibility-registry.json"),),
               ("python3 kernel/tools/generators/gen-accessibility-registry.py --check --selftest",),
               ("kernel/metadata/accessibility-registry.json",),
               ("invented semantic tree", "hidden assistive gap", "target overclaim"),
               ("9 capabilities missing", "0 complete target workflows"),
               "dated host evidence covers keyboard/focus primitives; current-build semantic and assistive workflows remain absent"),
    "EV-024": ("PARTIAL_CURRENT", (("implementation", "kernel/metadata/security-registry.json"),),
               ("python3 kernel/tools/generators/gen-security-registry.py --check --selftest",),
               ("kernel/metadata/security-registry.json",),
               ("production overclaim", "hidden DMA risk", "invented DMA proof"),
               ("17 residual-risk claims open", "0 production-complete security claims"),
               "current artifact/QEMU evidence covers two limited claims, historical host evidence covers four, and eleven foundations remain missing"),
    "EV-025": ("PARTIAL_CURRENT", (("plan", "docs/program/VALIDATION-RECEIPT.md"),),
               ("python3 tools/validate_master_program.py --self-test",),
               ("docs/program/VALIDATION-RECEIPT.md",), ("missing feature", "unknown contract"),
               ("no independent reviewer receipt exists for the active implementation batch",),
               "research plans were adversarially reviewed; active code promotion still needs independent rejection authority"),
    "EV-026": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/metadata/decision-ledger.json"),
                   ("plan", "docs/program/VALIDATION-RECEIPT.md")),
               ("python3 kernel/tools/generators/gen-decision-ledger.py --check --selftest",
                "python3 tools/validate_master_program.py"),
               ("kernel/metadata/decision-ledger.json",
                "kernel/docs/mp00-decision-ledger-2026-08-24.md"),
               ("missing decision", "unknown status", "invented source",
                "lost replacement", "deleted obsolete evidence", "hidden history gap"),
               ("system-wide decision inventory is incomplete",
                "no signed or independent approval", "no history viewer"),
               "19 high-impact decisions/reversals and all 47 legacy decision sections have exact normalized source semantics; repository-wide history, approval and the target viewer remain open"),
    "EV-027": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/metadata/release-notes.json"),
                   ("plan", "docs/program/VALIDATION-RECEIPT.md")),
               ("python3 kernel/tools/generators/gen-release-notes.py --check --selftest",),
               ("kernel/metadata/release-notes.json", "kernel/docs/UNRELEASED-CHANGELOG.md",
                "kernel/docs/mp00-release-notes-2026-08-24.md"),
               ("missing change", "invented release", "hidden release block",
                "missing known issue", "missing migration recovery",
                "deleted superseded history", "unknown release feature"),
               ("0 published entries", "no version/channel/signature",
                "public redistribution blocked", "migration inventory incomplete",
                "no previous signed rollback generation"),
               "17 source-bound change candidates, 12 blockers and 3 recovery paths are generated; this remains an unreleased development batch with zero published entries"),
    "EV-028": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/metadata/provenance-viewer.json"),
                   ("plan", "docs/program/VALIDATION-RECEIPT.md")),
               ("python3 kernel/tools/generators/gen-provenance-viewer.py --check --selftest",),
               ("kernel/metadata/provenance-viewer.json", "kernel/docs/provenance-viewer.html",
                "kernel/docs/mp00-provenance-viewer-2026-08-24.md"),
               ("missing artifact", "invented signature", "hidden license block",
                "invented app permission", "invented runtime route",
                "missing focus style", "external request", "missing HTML artifact"),
               ("no booted zlOS app route", "0 signatures",
                "0 per-app permission grants", "no live health stream",
                "no current screenshot or target accessibility receipt"),
               "self-contained host viewer projects the generated application manifest alongside 9 artifacts, 17 security claims and 11 health areas; it remains static and untrusted as a release attestation"),
    "BT-001": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/metadata/artifact-registry.json"),),
               ("kernel/tools/checks/verify-raw.sh",
                "kernel/tools/checks/verify-iso.sh",
                "kernel/tools/checks/verify-64.sh"),
               ("kernel/zlOS.img", "kernel/zlOS.iso", "kernel/zlOS64.iso"),
               ("missing ready marker", "wrong build identity", "wrong app manifest"),
               ("no current physical BIOS artifact receipt",
                "network and recovery boot layouts are absent"),
               "current-build QEMU proves raw BIOS plus GRUB BIOS with 32-bit and 64-bit payloads; physical BIOS and the complete supported-layout contract remain open"),
    "BT-002": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/metadata/artifact-registry.json"),),
               ("kernel/tools/checks/verify-efi.sh",
                "kernel/tools/checks/verify-iso.sh",
                "kernel/tools/checks/verify-64.sh"),
               ("kernel/BOOTX64.EFI", "kernel/zlOS-usb.img",
                "kernel/zlOS.iso", "kernel/zlOS64.iso"),
               ("firmware exit failure", "missing ready marker",
                "wrong build identity", "wrong app manifest"),
               ("no current physical UEFI/GOP receipt",
                "firmware fallback and hostile memory-map coverage are incomplete"),
               "native UEFI64 and GRUB UEFI with 32-bit and 64-bit payloads are current-build QEMU routes; physical firmware and the complete fallback contract remain open"),
    "BT-003": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/metadata/artifact-registry.json"),),
               ("kernel/tools/checks/verify-raw.sh",),
               ("kernel/kernel_raw.elf", "kernel/zlOS.img"),
               ("oversized payload", "missing marker", "wrong build identity"),
               ("no versioned recovery layout", "no current physical raw-media receipt"),
               "the exact raw-loader payload and disk image are reproducible and boot current-build QEMU; versioned recovery and physical-media proof remain open"),
    "BT-004": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/metadata/artifact-registry.json"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("kernel/tools/checks/verify-iso.sh",
                "kernel/tools/checks/verify-64.sh",
                "python3 kernel/tools/run/run-host-tests.py --run --selftest"),
               ("kernel/kernel.elf", "kernel/kernel64.elf",
                "kernel/zlOS.iso", "kernel/zlOS64.iso"),
               ("hostile Multiboot geometry", "unknown handover flag",
                "missing marker", "wrong build identity"),
               ("module-list admission is incomplete",
                "no current physical Multiboot receipt"),
               "four current-build GRUB BIOS/UEFI routes and the 288-check handover gate cover the retained Multiboot paths; complete module admission and physical proof remain open"),
    "BT-006": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/metadata/artifact-registry.json"),),
               ("python3 kernel/tools/generators/gen-artifact-registry.py --check --selftest",),
               ("kernel/metadata/artifact-registry.json",),
               ("missing route", "wrong route artifact", "wrong build identity",
                "physical overclaim"),
               ("network boot is absent", "partition/filesystem/config combinations are incomplete",
                "all nine exact artifacts lack current physical proof"),
               "six retained BIOS/UEFI and raw/GRUB/native routes are current-build QEMU evidence; the complete media/configuration cross-product and physical lane remain open"),
    "BT-008": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/core/boot/boot_handover.c"),
                   ("implementation", "kernel/docs/architecture/boot/typed-boot-handover.md"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json"),
                   ("implementation", "kernel/metadata/artifact-registry.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",
                "python3 kernel/tools/generators/gen-artifact-registry.py --check --selftest"),
               ("kernel/src/core/boot/boot_handover.c",
                "kernel/src/core/boot/boot_handover.h",
                "kernel/tests/host/test-run-receipt.json",
                "kernel/metadata/artifact-registry.json"),
               ("mutation after seal", "unknown flag", "non-zero reserved byte",
                "framebuffer overflow", "hostile Multiboot geometry",
                "empty exact-identity claim", "entry-route validation failure"),
               ("shipping loaders do not populate exact origin or kernel identity",
                "no entropy field", "foreign memory-map formats are not normalized",
                "no physical ThinkPad receipt for the current artifacts"),
               "all entry families construct and validate one sealed 256-byte record, the current host gate passes 288 checks, and six current-build QEMU routes reach ready; exact identities, entropy, normalized maps and physical proof remain open"),
    "BT-017": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/metadata/init-registry.json"),
                   ("implementation", "kernel/metadata/artifact-registry.json")),
               ("python3 kernel/tools/generators/gen-init-registry.py --check --selftest",
                "python3 kernel/tools/generators/gen-artifact-registry.py --check --selftest"),
               ("kernel/metadata/init-registry.json",),
               ("missing stage", "unknown dependency", "dependency cycle",
                "route omission", "wrong build identity"),
               ("no general deadline engine", "no reverse teardown execution",
                "provider rollback remains stage-specific", "no physical route receipt"),
               "the current registry binds 18 ordered stages and their dependencies, criticality and failure behavior to all six QEMU routes; general deadlines, rollback and reverse teardown remain open"),
    "BT-029": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/rtc.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json"),
                   ("implementation", "kernel/docs/receipts/rtc-clock-qemu-2026-08-29.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",
                "kernel/tools/checks/verify-clock.sh"),
               ("kernel/src/arch/x86/rtc.c",
                "kernel/tests/host/test-run-receipt.json",
                "kernel/docs/receipts/rtc-clock-qemu-2026-08-29.json"),
               ("BCD and binary decoding", "12/24-hour conversion",
                "torn-read refusal", "bounded UIP wait", "calendar admission",
                "Unix epoch conversion", "controlled QEMU clock bases"),
               ("hardware clock timezone policy is absent", "no synchronization service",
                "no drift/accuracy or suspend-resume contract",
                "no current physical RTC receipt", "locale presentation remains in-process"),
               "50 current-build host assertions and three exact-kernel controlled QEMU bases prove bounded RTC decoding and admission; timezone, synchronization, drift, suspend and physical accuracy remain open"),
    "FS-001": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/drivers/storage/block.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",),
               ("kernel/src/drivers/storage/block.c",
                "kernel/tests/host/test-run-receipt.json"),
               ("invalid geometry", "out-of-range LBA", "dirty-cache saturation",
                "bounded service", "explicit flush"),
               ("NVMe is the only target provider", "requests are not typed or asynchronous",
                "no discard, cancellation, detach or partial-completion contract",
                "no current physical-device receipt"),
               "the current 10-assertion block host gate proves bounded read/write/cache/flush behavior over one provider; the complete provider-neutral asynchronous contract and physical proof remain open"),
    "FS-005": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/drivers/storage/block.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",),
               ("kernel/src/drivers/storage/block.c",
                "kernel/tests/host/test-run-receipt.json"),
               ("read-your-write", "dirty ordering", "full-cache refusal",
                "one-block writeback budget", "forced drain", "telemetry"),
               ("no concurrent-owner model", "no memory-pressure integration",
                "device loss and retry/reset behavior are absent",
                "no current QEMU fault-injection or physical receipt"),
               "a current-build host receipt proves the fixed 128-page cache, oldest-sequence writeback, bounded service and refusal behavior; concurrency, pressure and device-loss semantics remain open"),
    "FS-011": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/fs/fs.c"),
                   ("implementation", "kernel/docs/features/storage-and-files.md"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",),
               ("kernel/src/fs/fs.c", "kernel/tests/host/test-run-receipt.json"),
               ("format and mount", "named create/read/write/delete/rename",
                "512/4096-byte geometry", "separate-process cold remount",
                "old-or-new replacement recovery"),
               ("flat 32-entry namespace", "23-byte ASCII names",
                "contiguous runs and no free-space index", "no permissions or directories",
                "no current QEMU persistence or physical-media receipt"),
               "the current 133-assertion host gate proves zlfs v2 named-file behavior, hostile admission and cold-process recovery; general filesystem semantics and current target persistence remain open"),
    "FS-015": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/fs/fs.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",),
               ("kernel/src/fs/fs.c", "kernel/tests/host/test-run-receipt.json"),
               ("checked length overflow", "growth relocation", "neighbor preservation",
                "failed-copy rollback", "4096-byte geometry"),
               ("no explicit truncate API", "no sparse files or hole policy",
                "no concurrent mappings", "growth can require half the remaining space",
                "no target or physical receipt"),
               "host proof covers checked contiguous-run growth, relocation and rollback without overlap; truncate, sparse, mapping and target behavior remain open"),
    "FS-020": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/drivers/storage/block.c"),
                   ("implementation", "kernel/src/fs/fs.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",),
               ("kernel/src/drivers/storage/block.c", "kernel/src/fs/fs.c",
                "kernel/tests/host/test-run-receipt.json"),
               ("explicit filesystem sync", "bounded dirty drain",
                "data-before-metadata sequence", "flush failure refusal"),
               ("NVMe command completion is the lowest stated boundary",
                "no device power-loss cache contract", "no per-file fsync or barriers",
                "no current QEMU power-cut or physical durability receipt"),
               "current host tests prove the software sync boundary and ordered cache drain; stable-media meaning through controller caches and physical power loss remains open"),
    "FS-021": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/fs/fs.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",),
               ("kernel/src/fs/fs.c", "kernel/tests/host/test-run-receipt.json"),
               ("dual checksummed generations", "data-before-header publication",
                "seven replacement cut points", "newest-generation corruption fallback",
                "failed-directory rollback"),
               ("transaction scope is one flat-directory mutation",
                "there is no append journal or replay log", "generation wrap policy is narrow",
                "no multi-object transaction", "no current target or physical power-cut receipt"),
               "zlfs v2 has a current host-proved dual-generation transaction protocol with old-or-new recovery; general journal/replay and target power-loss proof remain open"),
    "FS-024": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/fs/fs.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",),
               ("kernel/src/fs/fs.c", "kernel/tests/host/test-run-receipt.json"),
               ("bad magic", "future version", "whole-block checksum corruption",
                "geometry overflow", "entry range overflow", "corrupt newest generation"),
               ("no fuzzed corpus receipt", "no cycle/link graph exists to validate",
                "mount limits are zlfs-specific", "no current QEMU or physical corrupt-media receipt"),
               "the current host gate rejects malformed zlfs superblocks, geometry and entries and safely falls back from a corrupt generation; broader corpus and target-media admission remain open"),
    "KR-001": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/core/pmm.h"),
                   ("implementation", "kernel/src/core/pmm.c"),
                   ("implementation", "kernel/src/core/process_memory.h"),
                   ("implementation", "kernel/src/core/process_memory.c"),
                   ("implementation", "freestanding/runtime_kernel.c"),
                   ("implementation", "kernel/src/kernel.zl"),
                   ("implementation", "kernel/verify.sh"),
                   ("implementation", "kernel/tests/fixtures/golden.txt"),
                   ("implementation", "kernel/tests/host/pmmtest.c"),
                   ("implementation", "kernel/tests/host/processmemorytest.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json"),
                   ("implementation", "kernel/docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",
                "kernel/verify.sh",
                "kernel/tools/checks/verify-efi.sh"),
               ("kernel/tests/host/test-run-receipt.json", "kernel/kernel.elf",
                "kernel/zlOS-usb.img",
                "kernel/docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json"),
               ("firmware usable-range admission", "fixed 0..320 MiB reservation",
                "typed owner release", "reserved and foreign-owner refusal",
                "double-free and unaligned-free refusal", "deterministic exhaustion",
                "zero-on-allocation and reuse", "UEFI and Multiboot map parsing",
                "exact per-owner usage/high-water/availability/refusal accounting",
                "owner quota and failed-shrink refusal",
                "failure-atomic process-frame lifecycle",
                "bounded anonymous-frame zero/rollback/reclaim"),
               ("bounded to the first 1 GiB and pages at or above 320 MiB",
                "production accounting covers only the fixed two-slot UEFI64 process diagnostic and its bounded anonymous window",
                "no concurrent or interrupt-context locking contract",
                "Multiboot has an exact golden boot gate but no dedicated allocator receipt",
                "no current physical-hardware receipt"),
               "current-build host checks prove map admission, exact owner accounting/quotas and failure-atomic process and anonymous-frame ownership; exact Multiboot and native-UEFI64 QEMU routes restore allocator baselines, and UEFI reclaims two process address spaces plus bounded anonymous pages to zero owner totals; general consumers, SMP, memory above 1 GiB and physical proof remain open"),
    "KR-002": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/core/heap.c"),
                   ("implementation", "kernel/tests/host/heaptest.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",),
               ("kernel/src/core/heap.c", "kernel/tests/host/heaptest.c",
                "kernel/tests/host/test-run-receipt.json"),
               ("checked size and 16-byte alignment", "explicit counted OOM",
                "24-bit tags preserved by realloc", "32-position allocation failure sweep",
                "bounded freed-block poison", "4000-operation payload/invariant stress",
                "constant allocator step bound"),
               ("poison covers an 8-byte guard rather than every freed payload byte",
                "tags are diagnostic integers rather than an ownership policy",
                "failure injection covers allocation only",
                "host execution does not prove target concurrency or interrupt safety",
                "no current physical-hardware receipt"),
               "the current-build host gate proves checked allocation, tags, deterministic refusal and a bounded freed-block poison guard; target concurrency, wider poisoning and physical proof remain open"),
    "KR-015": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/core/pmm.h"),
                   ("implementation", "kernel/src/core/pmm.c"),
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/src/core/process_memory.c"),
                   ("implementation", "kernel/src/core/anon_memory.c"),
                   ("implementation", "kernel/tests/host/pmmtest.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json"),
                   ("implementation", "kernel/docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",
                "kernel/tools/checks/verify-efi.sh"),
               ("kernel/tests/host/test-run-receipt.json", "kernel/zlOS-usb.img",
                "kernel/docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
               ("exact per-owner physical-page usage", "bounded owner high-water",
                "quota-aware available pages", "per-owner refusal totals",
                "allocation refusal at quota", "failed quota shrink is non-mutating",
                "typed PMM owner identifier provenance", "metadata-to-owner-sum invariant",
                "fixed process owner limit of 16 pages",
                "anonymous owner limit of 32 pages", "target owner totals return to zero"),
               ("no unified service accounting", "no page/file-cache accounting",
                "no pinned-memory accounting", "no DMA accounting",
                "no surface accounting", "no complete kernel-total attribution",
                "no allocation-origin provenance beyond the typed owner identifier",
                "no pressure/reclaim policy", "no concurrent or SMP ownership contract",
                "no current physical-hardware receipt"),
               "current host and native-UEFI64 QEMU evidence proves exact bounded physical-page accounting and quotas for two fixed process-image owners and two anonymous-memory owners; the full process/service/cache/pinned/DMA/surface/kernel accounting feature remains open"),
    "KR-003": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/address_space_contract.json"),
                   ("implementation", "kernel/tools/generators/gen-address-space-registry.py"),
                   ("implementation", "kernel/metadata/address-space-registry.json")),
               ("python3 kernel/tools/generators/gen-address-space-registry.py --check --selftest",
                "kernel/tools/checks/check-memmap.sh --selftest",
                "python3 kernel/tools/checks/check-memmap-mirror.py"),
               ("kernel/metadata/address-space-registry.json",),
               ("physical overlap", "inverted user stack", "missing address category",
                "foreign build identity", "derived-field drift", "source-address drift",
                "duplicate low buffer", "memory-map mirror drift"),
               ("the zl-low shared reservation is an envelope rather than an internal-size manifest",
                "dynamic user virtual bases are selected at runtime",
                "the separate KR-004 transaction receipt covers the heap window, not every mapped region",
                "source/build evidence does not prove QEMU or physical execution"),
               "the generated joined map proves 19 physical and 6 user-template regions do not overlap and match 24 source authorities; general virtual-area allocation and physical proof remain open"),
    "KR-004": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/page_table_txn.h"),
                   ("implementation", "kernel/src/arch/x86/page_table_txn.c"),
                   ("implementation", "kernel/src/arch/x86/paging.c"),
                   ("implementation", "kernel/tests/host/pagetxntest.c"),
                   ("implementation", "kernel/docs/receipts/page-table-native-uefi64-qemu-2026-08-29.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",
                "kernel/tools/checks/verify-efi.sh"),
               ("kernel/tests/host/test-run-receipt.json", "kernel/zlOS-usb.img",
                "kernel/docs/receipts/page-table-native-uefi64-qemu-2026-08-29.json"),
               ("512-entry reserve without mutation", "complete pre-write validation",
                "all 512 nth-write failures", "apply-flush failure", "post-flush corruption",
                "explicit rollback flush", "failed rollback flush is fatal", "commit finality",
                "native UEFI heap-window alias before commit"),
               ("callers must provide exclusive page-table ownership",
                "no general page-table allocator or teardown service",
                "fixed Ring-3 private tables are built before CR3 publication but do not use the journal",
                "future live page-table writers must adopt the transaction core",
                "32-bit paging remains disabled", "no current physical-hardware receipt"),
               "a current host receipt proves exact full-range rollback under every injected write and flush failure, and native UEFI QEMU proves transactional heap-window and framebuffer-cache commits; SMP ownership, general lifecycle, other users and physical hardware remain open"),
    "KR-005": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/src/core/process_lifecycle.c"),
                   ("implementation", "kernel/src/core/user_process_service.c"),
                   ("implementation", "kernel/src/core/process_memory.h"),
                   ("implementation", "kernel/src/core/process_memory.c"),
                   ("implementation", "kernel/tests/host/processmemorytest.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json"),
                   ("implementation", "kernel/boot/gdt64.c"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
                   ("implementation", "kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json"),
                   ("implementation", "kernel/docs/receipts/user-process-exit-native-uefi64-qemu-2026-09-03.json")),
               ("python3 kernel/tools/run/run-host-tests.py --run --selftest",
                "kernel/tools/checks/verify-efi.sh",
                "kernel/tools/probes/probe-user-process.py --no-build",
                "kernel/tools/probes/probe-user-process-exit.py --no-build"),
               ("kernel/tests/host/test-run-receipt.json", "kernel/zlOS-usb.img",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json",
                "kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json",
                "kernel/docs/receipts/user-process-exit-native-uefi64-qemu-2026-09-03.json"),
               ("two unique PML4 roots", "separate user and TSS kernel stacks",
                "AB12 context resume", "supervisor mapping fault",
                "all-or-nothing eight-frame acquisition", "replacement and final reclamation",
                "persistent external-file spawn, normal exit, contained fault and reap"),
               ("fixed two-process service",
                "teardown is not exercised under concurrent service load",
                "no userspace spawn/wait/process-handle ABI",
                "no concurrent PID-reuse receipt",
                "no current physical-hardware receipt"),
               "current host and native-UEFI64 QEMU receipts prove failure-atomic owned frames, two disjoint address spaces, successful external execution, persistent kernel-owned spawn/reap and exact reclamation; userspace authority, concurrent teardown/PID reuse and physical proof remain open"),
    "KR-006": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/src/arch/x86/idt.c"),
                   ("implementation", "kernel/boot/gdt64.c"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("kernel/tools/checks/verify-efi.sh",),
               ("kernel/zlOS-usb.img",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
               ("iretq Ring 3 entry", "int 0x80 return", "CLI general-protection fault",
                "kernel and device page faults", "TSS I/O denial"),
               ("no SMEP or SMAP enablement receipt", "x86-64 route only",
                "no capability-mediated privileged-operation model",
                "no current physical-hardware receipt"),
               "current QEMU proves a bounded x86-64 CPL3/TSS/page-permission boundary; stronger hardware protections, architecture parity and physical proof remain open"),
    "KR-007": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/docs/architecture/system/user-process-abi.md"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("kernel/tools/checks/verify-efi.sh",),
               ("kernel/zlOS-usb.img",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
               ("overflow refusal", "complete fixed-range admission before dereference",
                "cross-guard pointer refusal", "bounded non-NUL name copy"),
               ("fixed code/stack ranges instead of arbitrary mapping walks",
                "no demand-fault or copyout-fault recovery", "no hostile copy corpus",
                "no current physical-hardware receipt"),
               "source-bound full-span checks and a current QEMU crossing-pointer refusal cover the fixed process layout; general mapped-memory copy recovery remains open"),
    "KR-008": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/metadata/artifact-registry.json"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("kernel/tools/checks/verify-efi.sh",
                "python3 kernel/tools/generators/gen-artifact-registry.py --check --selftest"),
               ("kernel/zlOS-usb.img", "kernel/metadata/artifact-registry.json",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
               ("RX user code", "RW/NX user stack", "supervisor-only inherited mappings",
                "non-RWX kernel load segments"),
               ("no authorized writable-to-executable transition protocol",
                "no system-wide audit of every dynamic mapping",
                "no SMEP/SMAP enablement receipt", "no current physical-hardware receipt"),
               "the current fixed user layout and shipped ELF segments are non-WX and hostile supervisor access faults in QEMU; dynamic-transition and physical proof remain open"),
    "KR-009": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/src/arch/x86/idt.c"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("kernel/tools/checks/verify-efi.sh",),
               ("kernel/zlOS-usb.img",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
               ("absent lower-stack PTE", "exact CR2 guard address",
                "non-present user-write error 0x6", "offender vector 14",
                "sibling trace G and independent exit", "kernel remains alive"),
               ("fixed one-page lower user-stack guard is directly fault-observed",
                "kernel-stack guards are source- and use-observed but not overflow-fault-injected",
                "no emergency IST guard page",
                "no general process lifecycle or recovery service",
                "no current physical-hardware receipt"),
               "one current native-UEFI64 QEMU probe proves the fixed lower user-stack guard faults at its exact address and removes only the offender; fixed kernel-stack guards are separately source- and use-observed, while direct overflow injection, emergency guards and physical proof remain open"),
    "KR-010": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/core/anon_memory.h"),
                   ("implementation", "kernel/src/core/anon_memory.c"),
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/src/arch/x86/user_syscalls.json"),
                   ("implementation", "kernel/tests/host/anonmemorytest.c"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("kernel/tests/host/anonmemorytest",
                "kernel/tests/host/user-syscalls-test.sh",
                "kernel/tools/checks/verify-efi.sh"),
               ("kernel/tests/host/test-run-receipt.json",
                "kernel/zlOS-usb.img",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
               ("32-page typed anonymous window", "reserved versus committed state",
                "zero-filled PMM commitment", "failure-atomic OOM and PTE rollback",
                "collision and owner refusal", "mixed and holey reclaim",
                "hardware accessed/dirty-bit admission", "Ring-3 reserve/commit/release",
                "exact reserved and released page faults", "sibling survival",
                "final PMM baseline restoration"),
               ("fixed virtual window and caller-selected page indices",
                "no virtual-area allocator or demand paging",
                "no file-backed or shared mappings", "no concurrent teardown protocol",
                "no current physical-hardware receipt"),
               "current host and native-UEFI64 QEMU evidence proves bounded reserve/commit/release, zero fill, failure-atomic OOM/rollback, collision/ownership checks, exact reserved/released faults and reclaim; general virtual-memory services and physical proof remain open"),
    "KR-017": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/boot/gdt64.c"),
                   ("implementation", "kernel/src/arch/x86/address_space_contract.json"),
                   ("implementation", "kernel/metadata/address-space-registry.json"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("python3 kernel/tools/generators/gen-address-space-registry.py --check --selftest",
                "kernel/tools/checks/verify-efi.sh"),
               ("kernel/zlOS-usb.img", "kernel/metadata/address-space-registry.json",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
               ("two private two-page supervisor RW/NX TSS stacks",
                "one absent lower guard per fixed process stack",
                "bounded high-water observations for both stack slots",
                "safe kernel-stack-before-CR3 return order",
                "syscall, timer-preemption and fault-path use"),
               ("fixed two-process service rather than general per-thread stack ownership",
                "no direct kernel-stack overflow fault injection",
                "no guarded emergency IST stack", "no scalable thread-stack allocator",
                "no SMP interrupt-nesting or current physical-hardware receipt"),
               "current source-bound layout plus native-UEFI64 QEMU prove two guarded supervisor-only TSS stacks are selected and used with bounded high-water across syscall, preemption and fault paths; general thread lifecycle, direct overflow, IST, SMP and physical proof remain open"),
    "KR-031": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/src/arch/x86/idt.c"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
                   ("implementation", "kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json"),
                   ("implementation", "kernel/docs/receipts/user-process-exit-native-uefi64-qemu-2026-09-03.json")),
               ("kernel/tools/checks/verify-efi.sh",
                "kernel/tools/probes/probe-user-process.py --no-build",
                "kernel/tools/probes/probe-user-process-exit.py --no-build"),
               ("kernel/zlOS-usb.img",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json",
                "kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json",
                "kernel/docs/receipts/user-process-exit-native-uefi64-qemu-2026-09-03.json"),
               ("offender vector 13", "sibling trace K and independent exit",
                "kernel remains alive", "external invalid image faulted and was reaped while desktop remained live",
                "external valid image exited normally and was reaped while desktop remained live"),
               ("only a bounded two-process service is covered",
                "fault-vector and malformed-frame coverage is incomplete",
                "no current physical-hardware receipt"),
               "current QEMU probes prove a GP-faulted process does not stop its sibling or kernel and contrast that with a successful external workload; general malformed-state and physical proof remain open"),
    "KR-036": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/user_syscalls.json"),
                   ("implementation", "kernel/src/arch/x86/user_syscalls_generated.h"),
                   ("implementation", "kernel/tools/generators/gen-user-syscalls.py"),
                   ("implementation", "kernel/tests/host/test-run-receipt.json"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("kernel/tests/host/user-syscalls-test.sh",
                "kernel/tools/checks/verify-efi.sh"),
               ("kernel/src/arch/x86/user_syscalls_generated.h",
                "kernel/tests/host/test-run-receipt.json",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
               ("duplicate/unordered/gap mutation refusal", "positive non-sign-bit IDs",
                "stale generated-header refusal", "zero", "first gap", "sign bit",
                "all bits set", "target ENOSYS result"),
               ("ABI table covers x86-64 only", "dispatch bodies remain hand-written",
                "no generated argument/type metadata", "no compatibility translation layer",
                "no current physical-hardware receipt"),
               "ABI version 1 generates the admitted 1..24 table and current host/QEMU gates reject unsigned unknown IDs with ENOSYS; typed arguments, compatibility and physical proof remain open"),
    "KR-037": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/docs/architecture/system/user-process-abi.md"),
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("kernel/tools/checks/verify-efi.sh",),
               ("kernel/zlOS-usb.img",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
               ("documented x86-64 calling convention", "negative errno values",
                "eight whole-file handles", "time/yield", "bounded IPC and window calls",
                "six-call Ring 3 lifecycle"),
               ("ABI is x86-64-only and version 1 has no translation layer",
                "no signal contract", "no compatibility or deprecation tooling",
                "no generated argument/layout/error/handle manifest",
                "no current physical-hardware receipt"),
               "the documented version-1 x86-64 ABI, generated number admission and current QEMU lifecycle are real, but signals, compatibility and physical qualification are absent"),
    "KR-022": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/arch/x86/usermode.c"),
                   ("implementation", "kernel/src/arch/x86/idt.c"),
                   ("implementation", "kernel/src/core/process_lifecycle.c"),
                   ("implementation", "kernel/src/core/scheduler_policy.c"),
                   ("implementation", "kernel/src/core/user_process_service.c"),
                   ("implementation", "kernel/tests/host/processlifecycletest.c"),
                   ("implementation", "kernel/tests/host/schedulerpolicytest.c"),
                   ("implementation", "kernel/tests/host/userprocessservicetest.c"),
                   ("implementation", "kernel/docs/receipts/scheduler-native-uefi64-qemu-2026-08-29.json"),
                   ("implementation", "kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json"),
                   ("implementation", "kernel/docs/receipts/user-process-exit-native-uefi64-qemu-2026-09-03.json")),
               ("kernel/tests/host/processlifecycletest",
                "kernel/tests/host/schedulerpolicytest",
                "kernel/tests/host/userprocessservicetest",
                "kernel/tools/checks/verify-efi.sh",
                "kernel/tools/probes/probe-user-process.py --no-build",
                "kernel/tools/probes/probe-user-process-exit.py --no-build"),
               ("kernel/zlOS-usb.img",
                "kernel/tests/host/test-run-receipt.json",
                "kernel/docs/receipts/scheduler-native-uefi64-qemu-2026-08-29.json",
                "kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json",
                "kernel/docs/receipts/user-process-exit-native-uefi64-qemu-2026-09-03.json"),
               ("separate PML4 and kernel stacks", "cooperative AB12 resume",
                "timer-preempted non-yielding PQ loops",
                "faulted-process sibling survival", "126-check bounded policy",
                "105-check lifecycle-policy coordinator", "persistent ST12 kernel-turn service",
                "external-file spawn, normal exit, fault observation and reap command routes"),
               ("the persistent Ring-3 service has exactly two fixed process slots",
                "no general priority or deadline contract beyond bounded round robin",
                "no per-CPU run-queue ownership or process migration",
                "no userspace process-management or cancellation API",
                "the separate eight-slot kernel task demo is cooperative and lacks FPU/SSE state",
                "no current physical-hardware scheduler receipt"),
               "current host and native-UEFI64 QEMU receipts prove a bounded fair two-slot persistent process scheduler and command route; priorities, per-CPU ownership, migration, userspace authority and physical qualification remain open"),
    "KR-027": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/core/process_lifecycle.h"),
                   ("implementation", "kernel/src/core/process_lifecycle.c"),
                   ("implementation", "kernel/tests/host/processlifecycletest.c"),
                   ("implementation", "kernel/src/core/user_process_service.c"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
                   ("implementation", "kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json"),
                   ("implementation", "kernel/docs/receipts/user-process-exit-native-uefi64-qemu-2026-09-03.json")),
               ("kernel/tests/host/processlifecycletest",
                "kernel/tests/host/userprocessservicetest",
                "kernel/tools/checks/verify-efi.sh",
                "kernel/tools/probes/probe-user-process.py --no-build",
                "kernel/tools/probes/probe-user-process-exit.py --no-build"),
               ("kernel/tests/host/test-run-receipt.json", "kernel/zlOS-usb.img",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json",
                "kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json",
                "kernel/docs/receipts/user-process-exit-native-uefi64-qemu-2026-09-03.json"),
               ("signed exit status -7 retained", "exact GP vector/error/address retained",
                "parent-only observation and child-safe reap", "terminal scheduler detach before reap",
                "external fault observed then reaped through desktop commands",
                "external normal exit status 37 observed then reaped through desktop commands"),
               ("no userspace wait syscall or process-handle ABI",
                "parent/child authority is host-proved but not exposed as a target service",
                "no concurrent PID-reuse or cancellation receipt",
                "no current physical-hardware receipt"),
               "host and native-UEFI64 QEMU evidence retain exact exit/fault custody and bounded reap without stale-generation aliasing; userspace wait authority, concurrency and physical proof remain open"),
    "KR-028": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/src/core/process_lifecycle.h"),
                   ("implementation", "kernel/src/core/process_lifecycle.c"),
                   ("implementation", "kernel/tests/host/processlifecycletest.c"),
                   ("implementation", "kernel/src/core/user_process_service.c"),
                   ("implementation", "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json")),
               ("kernel/tests/host/processlifecycletest",
                "kernel/tests/host/userprocessservicetest",
                "kernel/tools/checks/verify-efi.sh"),
               ("kernel/tests/host/test-run-receipt.json", "kernel/zlOS-usb.img",
                "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"),
               ("slot plus nonzero generation identity", "stale-generation refusal",
                "generation exhaustion retires slot", "scheduler owns exact lifecycle handle",
                "resource release precedes identity reap"),
               ("handles are kernel-internal and not an opaque userspace ABI",
                "PID remains the current IPC selector",
                "no cross-process delegated authority or revocation model",
                "no current physical-hardware receipt"),
               "exact generation-tagged handles now prevent internal stale-slot aliasing across lifecycle and scheduling; userspace exposure, delegated authority and physical proof remain open"),
    "KR-032": ("PARTIAL_CURRENT", (("implementation", "kernel/src/core/crash.c"),
                                     ("implementation", "kernel/src/arch/x86/idt.c"),
                                     ("implementation", "kernel/boot/gdt64.c"),
                                     ("implementation", "kernel/docs/receipts/cpu-fault-invalid-opcode-qemu-2026-08-23.json"),
                                     ("implementation", "kernel/docs/receipts/cpu-fault-native-uefi64-qemu-2026-08-29.json"),
                                     ("implementation", "kernel/docs/receipts/cpu-fault-general-protection-native-uefi64-qemu-2026-08-29.json"),
                                     ("implementation", "kernel/docs/receipts/cpu-fault-double-fault-native-uefi64-qemu-2026-08-29.json")),
               ("python3 kernel/tools/checks/verify-crash.py --run --route bios32 --fault ud2 --no-build --selftest",
                "python3 kernel/tools/checks/verify-crash.py --run --route native-uefi64 --fault ud2 --no-build --selftest",
                "python3 kernel/tools/checks/verify-crash.py --run --route native-uefi64 --fault gp --no-build --selftest",
                "python3 kernel/tools/checks/verify-crash.py --run --route native-uefi64 --fault double-fault --no-build --selftest",
                "kernel/tests/host/crashtest"),
               ("kernel/kernel.elf", "kernel/zlOS.iso", "kernel/zlOS-usb.img"),
               ("wrong vector", "wrong checksum", "wrong symbol", "missing registers",
                "wrong register", "invented upper register", "stale artifact", "guest not halted"),
               ("no broad kernel-fatal exception-vector or spurious-interrupt QEMU matrix",
                "double fault stops safely but has no recovery or emergency-stack guard page",
                "offender-only user-fault containment is limited to the native-UEFI64 two-slot harness; no general process lifecycle recovery"),
               "all CPU exception vectors have typed entry frames; four current QEMU receipts prove UD2, error-code GP and IST1 double-fault stop paths, while the bounded native-UEFI64 two-slot harness proves one Ring-3 GP offender can be removed as its sibling exits; broader vectors, spurious interrupts and general recovery remain open"),
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def phase_dependencies() -> dict[str, list[str]]:
    text = master.read(master.PROGRAM / "PHASES.md")
    result = {}
    for phase, raw in master.DEPENDENCY_ROW_RE.findall(text):
        result[phase] = [] if raw.strip() == "none" else [part.strip() for part in raw.split(",")]
    return result


def evidence_entry(kind: str, relative: str, evidence_root: Path) -> dict:
    root = ROOT if kind == "plan" else evidence_root
    path = root / relative
    if not path.is_file():
        raise ValueError(f"missing {kind} evidence: {path}")
    value = None
    if path.suffix == ".json":
        value = json.loads(path.read_text())
    return {
        "root": kind,
        "path": relative,
        "sha256": digest(path),
        "schema": value.get("schema") if isinstance(value, dict) else None,
        "result": value.get("result", value.get("outcome")) if isinstance(value, dict) else "SOURCE",
    }


def git_snapshot(path: Path) -> dict:
    identity = json.loads((path / "kernel/metadata/build-identity.json").read_text())
    return {
        "path": ".",
        "build_identity": identity["identity_sha256"],
        "generation_git_context": identity["git"],
        "evidence_ceiling": "build-input generation context, not current checkout or artifact commit attestation",
    }


def validate_decision_feature_links(decisions: dict, feature_ids: set[str]) -> None:
    if decisions.get("schema") != "zlos.decision-ledger.v1" \
            or decisions.get("result") != "PASS_WITH_OPEN_GAPS":
        raise ValueError("decision ledger is absent or overpromoted")
    unknown = sorted({
        feature_id
        for record in decisions.get("records", [])
        for feature_id in record.get("features", [])
        if feature_id not in feature_ids
    })
    if unknown:
        raise ValueError("decision ledger references unknown feature IDs: " + ", ".join(unknown))


def validate_release_feature_links(release_notes: dict, feature_ids: set[str]) -> None:
    if release_notes.get("schema") != "zlos.release-notes.v1" \
            or release_notes.get("result") != "PASS_UNRELEASED_WITH_BLOCKERS":
        raise ValueError("release notes are absent or overpromoted")
    linked = [
        feature_id
        for section in ("changes", "known_issues")
        for record in release_notes.get(section, [])
        for feature_id in record.get("feature_ids", [])
    ]
    unknown = sorted(set(linked) - feature_ids)
    if unknown:
        raise ValueError("release notes reference unknown feature IDs: " + ", ".join(unknown))


def validate_provenance_feature_link(provenance: dict, feature_ids: set[str]) -> None:
    if provenance.get("schema") != "zlos.provenance-viewer.v1" \
            or provenance.get("result") != "PASS_STATIC_VIEWER_WITH_OPEN_GAPS":
        raise ValueError("provenance viewer is absent or overpromoted")
    if provenance.get("feature_id") not in feature_ids:
        raise ValueError("provenance viewer references an unknown feature ID")


def validate_dependency_archive_receipt(receipt: dict, dependency: dict,
                                        build_identity: str, evidence_root: Path) -> None:
    expected_packages = [row["name"] for row in dependency.get("packages", [])]
    expected_sources = [
        (row["name"], row["version"])
        for row in dependency.get("source_packages", [])
    ]
    counts = receipt.get("counts", {})
    if receipt.get("schema") != "zlos.dependency-archive-receipt.v1" \
            or receipt.get("result") != "PASS_LOCAL_OFFLINE_ARCHIVE" \
            or receipt.get("build_identity") != build_identity \
            or receipt.get("dependency_lock_sha256") != digest(
                evidence_root / "kernel/metadata/dependency-lock.json"
            ):
        raise ValueError("dependency archive receipt is absent, failed or foreign")
    if [row.get("package") for row in receipt.get("binary_archives", [])] != expected_packages \
            or [(row.get("name"), row.get("version"))
                for row in receipt.get("source_archives", [])] != expected_sources \
            or [row.get("package") for row in receipt.get("relationships", [])] != expected_packages:
        raise ValueError("dependency archive receipt coverage drifted")
    if counts.get("binary_archives") != len(expected_packages) \
            or counts.get("source_packages") != len(expected_sources) \
            or counts.get("packages_with_source_archives") != len(expected_packages) \
            or counts.get("undeclared_dependency_edges") != 0:
        raise ValueError("dependency archive receipt counts drifted")
    if receipt.get("offline_resolution") != {
            "network_used": False,
            "all_seed_packages_reachable": True,
            "all_dependency_edges_declared": True,
            "all_binary_archives_present": True,
            "all_source_archives_present": True,
    }:
        raise ValueError("dependency archive receipt hides an offline-resolution gap")
    if len(receipt.get("archive_manifest_sha256", "")) != 64 \
            or len(receipt.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("dependency archive receipt identity is incomplete")


def validate_rtc_receipt(receipt: dict, build_identity: str) -> None:
    expected_bases = [
        "2026-08-18T14:37:05",
        "2026-12-31T23:59:07",
        "2000-02-29T12:00:00",
    ]
    if receipt.get("schema") != "zlos.rtc-clock-qemu-receipt.v1" \
            or receipt.get("result") != "PASS" \
            or receipt.get("build_identity") != build_identity:
        raise ValueError("RTC QEMU receipt is absent, failed or foreign")
    if len(receipt.get("artifact", {}).get("sha256", "")) != 64 \
            or len(receipt.get("harness", {}).get("sha256", "")) != 64:
        raise ValueError("RTC QEMU receipt lacks artifact/harness identity")
    cases = receipt.get("cases", [])
    if [row.get("base") for row in cases] != expected_bases \
            or any(row.get("boot_offset_seconds", -1) < 0 or
                   row.get("boot_offset_seconds", 121) > receipt.get("max_boot_seconds", -1)
                   for row in cases):
        raise ValueError("RTC QEMU receipt case coverage or offset drifted")


def validate_heap_host_receipt(receipt: dict, build_identity: str) -> None:
    if receipt.get("schema") != "zlos.host-test-run-receipt.v1" \
            or receipt.get("outcome") != "PASS" \
            or receipt.get("build_identity") != build_identity:
        raise ValueError("heap host receipt is absent, failed or foreign")
    rows = [row for row in receipt.get("results", [])
            if row.get("name") == "heaptest"]
    if len(rows) != 1 or rows[0].get("status") != "passed":
        raise ValueError("heap host result is absent or failed")
    commands = rows[0].get("commands", [])
    if len(commands) != 1 or commands[0].get("exit_code") != 0 \
            or commands[0].get("timed_out") is not False:
        raise ValueError("heap host command did not complete successfully")
    output = commands[0].get("output_tail", "")
    required = (
        "heap REFUSED 32 bytes: injected allocation failure",
        "(stress: 4000 operations",
        "worst case over the whole run: alloc 3 steps, free 4 steps",
        "2162 checks, 0 failures",
        "ok    tags, a 32-position allocation-failure sweep, and freed-block poison",
    )
    if any(marker not in output for marker in required):
        raise ValueError("heap host result lost a required bounded observation")


def validate_address_registry(registry: dict, build_identity: str,
                              evidence_root: Path) -> None:
    if registry.get("schema") != "zlos.address-space-registry.v1" \
            or registry.get("result") != \
            "PASS_CURRENT_GENERATED_NON_OVERLAP_WITH_DYNAMIC_USER_TEMPLATE" \
            or registry.get("build_identity") != build_identity:
        raise ValueError("address-space registry is absent, stale or foreign")
    expected_counts = {
        "physical_regions": 19,
        "user_template_regions": 6,
        "source_assertions": 24,
        "categories": 8,
    }
    required_categories = {
        "kernel", "user", "device", "stack", "heap", "shared", "guard", "dynamic"
    }
    physical = registry.get("physical", {}).get("regions", [])
    user = registry.get("user_slot_template", {}).get("regions", [])
    categories = {row.get("category") for row in physical + user}
    if registry.get("counts") != expected_counts \
            or set(registry.get("categories", [])) != required_categories \
            or categories != required_categories:
        raise ValueError("address-space registry count/category coverage drifted")
    for rows, start_key, end_key in (
            (physical, "start", "end"), (user, "start_offset", "end_offset")):
        ordered = sorted(rows, key=lambda row: row.get(start_key, -1))
        for row in ordered:
            start = row.get(start_key)
            end = row.get(end_key)
            if not isinstance(start, int) or not isinstance(end, int) or end <= start \
                    or row.get("bytes") != end - start \
                    or row.get("range") != f"0x{start:016x}..0x{end:016x}":
                raise ValueError("address-space registry has invalid derived ranges")
        if any(left[end_key] > right[start_key]
               for left, right in zip(ordered, ordered[1:])):
            raise ValueError("address-space registry contains an overlap")
    identified = [registry.get("contract", {}), registry.get("generator", {})]
    identified.extend(registry.get("source_assertions", []))
    if len(identified) != 26 or any(
            not row.get("path")
            or digest(evidence_root / row["path"]) != row.get("sha256")
            for row in identified):
        raise ValueError("address-space registry source identity drifted")
    gaps = registry.get("known_gaps", [])
    if len(gaps) != 4 or not any("rollback" in gap for gap in gaps) \
            or not any("physical" in gap for gap in gaps):
        raise ValueError("address-space registry hid its evidence ceiling")


def validate_page_table_receipt(receipt: dict, build_identity: str,
                                evidence_root: Path) -> None:
    if receipt.get("schema") != "zlos.page-table-native-uefi64-qemu-receipt.v1" \
            or receipt.get("result") != \
            "PASS_TRANSACTION_CORE_AND_NATIVE_HEAP_WINDOW_WITH_OPEN_GAPS" \
            or receipt.get("route") != "native-uefi64" \
            or receipt.get("build_identity") != build_identity:
        raise ValueError("page-table receipt is absent, failed or foreign")
    identified = [receipt.get("artifact", {}), receipt.get("harness", {})]
    identified.extend(receipt.get("implementation", []))
    identified.append(receipt.get("host_receipt", {}))
    identified.append(receipt.get("generator", {}))
    if len(identified) != 7 or any(
            not row.get("path")
            or digest(evidence_root / row["path"]) != row.get("sha256")
            for row in identified):
        raise ValueError("page-table receipt source, host or artifact identity drifted")
    assertions = receipt.get("assertions", [])
    if len(assertions) != 3 \
            or assertions[0].get("entries") != 512 \
            or assertions[0].get("nth_write_failures") != 512 \
            or assertions[0].get("exact_rollback") is not True \
            or assertions[0].get("rollback_flush_failure_is_fatal") is not True \
            or assertions[1].get("alias_probe_passed_before_commit") is not True \
            or assertions[2].get("transaction_committed") is not True \
            or assertions[2].get("journal_marker") != "cache=write-combining":
        raise ValueError("page-table transaction observations drifted")
    if receipt.get("host_receipt", {}).get("checks") != 26 \
            or receipt.get("host_receipt", {}).get("target") != "pagetxntest":
        raise ValueError("page-table host target binding drifted")
    gaps = receipt.get("known_gaps", [])
    if len(gaps) != 6 or not any("Ring-3" in gap for gap in gaps) \
            or not any("physical" in gap for gap in gaps):
        raise ValueError("page-table receipt hides its remaining coverage gaps")


def validate_pmm_receipt(receipt: dict, build_identity: str,
                         evidence_root: Path) -> None:
    if receipt.get("schema") != \
            "zlos.physical-page-allocator-native-uefi64-qemu-receipt.v1" \
            or receipt.get("result") != \
            "PASS_BOUNDED_TYPED_ALLOCATOR_WITH_OPEN_GAPS" \
            or receipt.get("route") != "native-uefi64" \
            or receipt.get("build_identity") != build_identity:
        raise ValueError("physical allocator receipt is absent, failed or foreign")
    identified = [receipt.get("artifact", {}), receipt.get("harness", {})]
    identified.extend(receipt.get("implementation", []))
    identified.append(receipt.get("host_receipt", {}))
    identified.append(receipt.get("consumer_receipt", {}))
    identified.append(receipt.get("generator", {}))
    required_paths = {
        "kernel/zlOS-usb.img",
        "kernel/tools/checks/verify-efi.sh",
        "kernel/src/core/pmm.h",
        "kernel/src/core/pmm.c",
        "freestanding/runtime_kernel.c",
        "kernel/src/kernel.zl",
        "kernel/tests/host/pmmtest.c",
        "kernel/tests/host/test-run-receipt.json",
        "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json",
        "kernel/tools/checks/write-pmm-receipt.py",
    }
    if {row.get("path") for row in identified} != required_paths or any(
            digest(evidence_root / row["path"]) != row.get("sha256")
            for row in identified):
        raise ValueError("physical allocator source, host or artifact identity drifted")
    assertions = receipt.get("assertions", [])
    if [row.get("id") for row in assertions] != [
            "bounded-firmware-map-admission", "typed-owner-release",
            "exact-owner-accounting-and-quota", "zero-reuse-and-exhaustion",
            "typed-process-and-anonymous-consumer"]:
        raise ValueError("physical allocator assertion coverage drifted")
    bounded, ownership, accounting, reuse, consumer = assertions
    if bounded.get("managed_floor_bytes") != 320 * 1024 * 1024 \
            or bounded.get("managed_limit_bytes") != 1024 * 1024 * 1024 \
            or bounded.get("admitted_pages", 0) < 1 \
            or bounded.get("free_pages_after_selftest") != bounded.get("admitted_pages") \
            or bounded.get("firmware_conventional_memory_only") is not True \
            or bounded.get("fixed_zlos_region_excluded") is not True \
            or ownership.get("wrong_owner_refused_without_mutation") is not True \
            or ownership.get("reserved_release_refused") is not True \
            or ownership.get("double_free_refused") is not True \
            or ownership.get("unaligned_release_refused") is not True \
            or accounting.get("exact_used_pages") is not True \
            or accounting.get("exact_high_water_pages") is not True \
            or accounting.get("exact_available_pages") is not True \
            or accounting.get("exact_refusal_count") is not True \
            or accounting.get("allocation_refused_at_limit") is not True \
            or accounting.get("limit_shrink_below_usage_refused_without_mutation") is not True \
            or accounting.get("metadata_owner_sum_invariant") is not True \
            or accounting.get("target_owner_totals_restored_to_zero") is not True \
            or reuse.get("zero_on_every_allocation") is not True \
            or reuse.get("deterministic_reuse") is not True \
            or reuse.get("host_exhaustion_observed") is not True \
            or reuse.get("target_baseline_restored") is not True \
            or consumer.get("frames_per_process") != 8 \
            or consumer.get("two_processes_disjoint") is not True \
            or consumer.get("failure_atomic_acquire") is not True \
            or consumer.get("anonymous_page_limit") != 32 \
            or consumer.get("anonymous_zero_fill") is not True \
            or consumer.get("anonymous_failure_atomic") is not True \
            or consumer.get("anonymous_target_reclaimed") is not True \
            or consumer.get("target_baseline_restored") is not True \
            or receipt.get("consumer_receipt", {}).get("assertions") != [
                "process-frame-reclamation",
                "anonymous-reserve-commit-release",
                "released-anonymous-page-fault",
                "process-memory-accounting"]:
        raise ValueError("physical allocator bounded observations drifted")
    host = receipt.get("host_receipt", {})
    if host.get("target") != "pmmtest" or host.get("checks", 0) < 80:
        raise ValueError("physical allocator host target binding drifted")
    gaps = receipt.get("known_gaps", [])
    if len(gaps) != 7 or not any("consumer" in gap for gap in gaps) \
            or not any("physical" in gap for gap in gaps):
        raise ValueError("physical allocator receipt hides its open gaps")


def validate_scheduler_receipt(receipt: dict, build_identity: str,
                               evidence_root: Path) -> None:
    if receipt.get("schema") != "zlos.scheduler-native-uefi64-qemu-receipt.v1" \
            or receipt.get("result") != "PASS_BOUNDED_PERSISTENT_PROCESS_GATE" \
            or receipt.get("route") != "native-uefi64" \
            or receipt.get("build_identity") != build_identity:
        raise ValueError("scheduler QEMU receipt is absent, failed or foreign")
    expected_files = {
        "kernel/zlOS-usb.img": receipt.get("artifact", {}).get("sha256"),
        "kernel/tools/checks/verify-efi.sh": receipt.get("harness", {}).get("sha256"),
        **{
            row.get("path"): row.get("sha256")
            for row in receipt.get("implementation", [])
        },
        "kernel/tests/host/test-run-receipt.json":
            receipt.get("host_receipt", {}).get("sha256"),
        "kernel/tools/checks/write-scheduler-receipt.py":
            receipt.get("generator", {}).get("sha256"),
    }
    required = {
        "kernel/zlOS-usb.img",
        "kernel/tools/checks/verify-efi.sh",
        "kernel/src/arch/x86/usermode.c",
        "kernel/src/arch/x86/idt.c",
        "kernel/src/core/process_lifecycle.h",
        "kernel/src/core/process_lifecycle.c",
        "kernel/tests/host/processlifecycletest.c",
        "kernel/src/core/scheduler_policy.h",
        "kernel/src/core/scheduler_policy.c",
        "kernel/tests/host/schedulerpolicytest.c",
        "kernel/src/core/user_process_service.h",
        "kernel/src/core/user_process_service.c",
        "kernel/tests/host/userprocessservicetest.c",
        "kernel/tests/host/test-run-receipt.json",
        "kernel/tools/checks/write-scheduler-receipt.py",
    }
    if set(expected_files) != required or any(
            digest(evidence_root / path) != expected_files[path] for path in required):
        raise ValueError("scheduler QEMU receipt source or artifact identity drifted")
    assertions = receipt.get("assertions", [])
    if [row.get("id") for row in assertions] != [
            "cooperative-context-resume", "timer-preemption",
            "faulted-process-sibling-survival", "persistent-process-service"]:
        raise ValueError("scheduler QEMU receipt assertion coverage drifted")
    if assertions[0].get("processes") != 2 \
            or assertions[0].get("trace") != "AB12" \
            or assertions[0].get("separate_address_spaces") is not True \
            or assertions[0].get("separate_kernel_stacks") is not True \
            or assertions[1].get("processes") != 2 \
            or assertions[1].get("trace") != "PQ" \
            or assertions[1].get("non_yielding") is not True \
            or assertions[1].get("minimum_preemptions") != 2 \
            or assertions[2].get("fault_vector") != 13 \
            or assertions[2].get("sibling_trace") != "K" \
            or assertions[2].get("kernel_survived") is not True \
            or assertions[3].get("processes") != 2 \
            or assertions[3].get("kernel_turns") != 4 \
            or assertions[3].get("trace") != "ST12" \
            or assertions[3].get("exit_statuses") != [11, 22] \
            or assertions[3].get("terminal_custody_observed") is not True \
            or assertions[3].get("scheduler_detached_before_identity_reap") is not True \
            or assertions[3].get("physical_frame_baseline_restored") is not True:
        raise ValueError("scheduler QEMU receipt bounded observations drifted")
    host = receipt.get("host_receipt", {})
    if host.get("path") != "kernel/tests/host/test-run-receipt.json" \
            or host.get("targets") != {
                "processlifecycletest": {"checks": 91},
                "schedulerpolicytest": {"checks": 126},
                "userprocessservicetest": {"checks": 105},
            }:
        raise ValueError("scheduler host policy/service observations drifted")
    gaps = receipt.get("known_gaps", [])
    if len(gaps) != 6 or not any("physical" in gap for gap in gaps):
        raise ValueError("scheduler QEMU receipt hides its known gaps")


def validate_user_process_receipt(receipt: dict, build_identity: str,
                                  evidence_root: Path) -> None:
    if receipt.get("schema") != "zlos.user-process-native-uefi64-qemu-receipt.v1" \
            or receipt.get("result") != "PASS_BOUNDED_USER_PROCESS_BOUNDARY" \
            or receipt.get("route") != "native-uefi64" \
            or receipt.get("build_identity") != build_identity:
        raise ValueError("user-process QEMU receipt is absent, failed or foreign")
    expected_files = {
        "kernel/zlOS-usb.img": receipt.get("artifact", {}).get("sha256"),
        "kernel/tools/checks/verify-efi.sh": receipt.get("harness", {}).get("sha256"),
        **{
            row.get("path"): row.get("sha256")
            for row in receipt.get("implementation", [])
        },
        "kernel/tests/host/test-run-receipt.json":
            receipt.get("host_receipt", {}).get("sha256"),
        "kernel/tools/checks/write-user-process-receipt.py":
            receipt.get("generator", {}).get("sha256"),
    }
    required = {
        "kernel/zlOS-usb.img",
        "kernel/tools/checks/verify-efi.sh",
        "kernel/src/arch/x86/usermode.c",
        "kernel/src/core/process_lifecycle.h",
        "kernel/src/core/process_lifecycle.c",
        "kernel/tests/host/processlifecycletest.c",
        "kernel/src/core/scheduler_policy.h",
        "kernel/src/core/scheduler_policy.c",
        "kernel/tests/host/schedulerpolicytest.c",
        "kernel/src/core/user_process_service.h",
        "kernel/src/core/user_process_service.c",
        "kernel/tests/host/userprocessservicetest.c",
        "kernel/src/core/process_memory.h",
        "kernel/src/core/process_memory.c",
        "kernel/tests/host/processmemorytest.c",
        "kernel/src/core/pmm.h",
        "kernel/src/core/pmm.c",
        "kernel/tests/host/pmmtest.c",
        "kernel/src/core/anon_memory.h",
        "kernel/src/core/anon_memory.c",
        "kernel/tests/host/anonmemorytest.c",
        "kernel/tests/host/test-run-receipt.json",
        "kernel/src/arch/x86/user_syscalls.json",
        "kernel/src/arch/x86/user_syscalls_generated.h",
        "kernel/src/arch/x86/idt.c",
        "kernel/boot/gdt64.c",
        "kernel/docs/architecture/system/user-process-abi.md",
        "kernel/tools/checks/write-user-process-receipt.py",
    }
    if set(expected_files) != required or any(
            digest(evidence_root / path) != expected_files[path] for path in required):
        raise ValueError("user-process QEMU receipt source or artifact identity drifted")
    assertions = receipt.get("assertions", [])
    if [row.get("id") for row in assertions] != [
            "ring3-syscall-lifecycle", "unknown-syscall-admission",
            "generation-safe-process-identity",
            "anonymous-reserve-commit-release",
            "reserved-anonymous-page-fault",
            "released-anonymous-page-fault",
            "privilege-and-user-copy-boundary", "separate-address-space-resume",
            "offender-only-fault-containment",
            "fault-and-exit-custody",
            "lower-stack-guard-fault-containment",
            "guarded-supervisor-tss-stacks", "process-memory-accounting",
            "process-frame-reclamation", "process-identity-reclamation",
            "persistent-process-service"]:
        raise ValueError("user-process QEMU receipt assertion coverage drifted")
    if assertions[0].get("syscalls") != 6 \
            or assertions[0].get("returned_to_kernel") is not True \
            or assertions[1].get("abi_version") != 1 \
            or assertions[1].get("probes") != [0, 25, 1 << 63, (1 << 64) - 1] \
            or assertions[1].get("result") != "ENOSYS" \
            or assertions[2].get("stale_handle_refused") is not True \
            or assertions[2].get("observed_exit_status") != -7 \
            or assertions[3].get("pages") != 2 \
            or assertions[3].get("reserved_has_frame") is not False \
            or assertions[3].get("commit_zero_filled") is not True \
            or assertions[3].get("cross_page_copy_admitted") is not True \
            or assertions[3].get("released_copy_refused") is not True \
            or assertions[4].get("offender_vector") != 14 \
            or assertions[4].get("page_fault_error") != 0x4 \
            or assertions[4].get("reserved_pte") != "absent" \
            or assertions[4].get("sibling_trace") != "V" \
            or assertions[5].get("offender_vector") != 14 \
            or assertions[5].get("page_fault_error") != 0x4 \
            or assertions[5].get("released_pte") != "absent" \
            or assertions[5].get("sibling_trace") != "R" \
            or assertions[5].get("frame_reclaimed") is not True \
            or assertions[6].get("privileged_instruction_vector") != 13 \
            or assertions[6].get("kernel_mapping_vector") != 14 \
            or assertions[6].get("device_mapping_vector") != 14 \
            or assertions[6].get("crossing_pointer_refused_before_dereference") is not True \
            or assertions[7].get("processes") != 2 \
            or assertions[7].get("trace") != "AB12" \
            or assertions[7].get("separate_pml4_roots") is not True \
            or assertions[8].get("offender_vector") != 13 \
            or assertions[8].get("sibling_trace") != "K" \
            or assertions[9].get("fault_vector") != 13 \
            or assertions[9].get("sibling_exit_status") != 7 \
            or assertions[10].get("offender_vector") != 14 \
            or assertions[10].get("page_fault_error") != 0x6 \
            or assertions[10].get("fault_address") != "user_base+4096+2048" \
            or assertions[10].get("guard_pte") != "absent" \
            or assertions[10].get("sibling_trace") != "G" \
            or assertions[10].get("kernel_survived") is not True \
            or assertions[11].get("processes") != 2 \
            or assertions[11].get("pages_per_stack") != 2 \
            or assertions[11].get("lower_guard_pte") != "absent" \
            or assertions[11].get("leaf_permissions") != \
            "supervisor-read-write-no-execute" \
            or assertions[11].get("tss_rsp0_selected") is not True \
            or assertions[11].get("paths") != ["syscall", "timer-preemption", "fault"] \
            or assertions[11].get("high_water_limit_bytes") != 8192 \
            or set(assertions[11].get("observed_high_water_bytes", {})) != \
            {"slot0", "slot1"} \
            or any(not isinstance(item, int) or not 0 < item < 8192
                   for item in assertions[11].get("observed_high_water_bytes", {}).values()) \
            or assertions[12].get("fixed_owner_count") != 2 \
            or assertions[12].get("fixed_limit_pages_per_owner") != 16 \
            or assertions[12].get("anonymous_owner_count") != 2 \
            or assertions[12].get("anonymous_limit_pages_per_owner") != 32 \
            or assertions[12].get("owner_totals_after_release") != 0 \
            or assertions[12].get("owner_high_water_bounded") is not True \
            or assertions[12].get("owner_refusals") != 0 \
            or assertions[12].get("metadata_owner_sum_invariant") is not True \
            or assertions[13].get("frames_per_process") != 8 \
            or assertions[13].get("failure_atomic_acquire") is not True \
            or assertions[13].get("two_processes_disjoint") is not True \
            or assertions[13].get("baseline_restored") is not True \
            or assertions[14].get("live_slots_after_reap") != 0 \
            or assertions[14].get("generation_history_retained") is not True \
            or assertions[15].get("kernel_turns") != 4 \
            or assertions[15].get("trace") != "ST12" \
            or assertions[15].get("exit_statuses") != [11, 22] \
            or assertions[15].get("terminal_custody_observed") is not True \
            or assertions[15].get("scheduler_detached_before_identity_reap") is not True \
            or assertions[15].get("physical_frame_baseline_restored") is not True \
            or receipt.get("host_receipt", {}).get("targets", {}).get(
                "processlifecycletest", {}).get("checks", 0) < 80 \
            or receipt.get("host_receipt", {}).get("targets", {}).get(
                "schedulerpolicytest", {}).get("checks", 0) < 100 \
            or receipt.get("host_receipt", {}).get("targets", {}).get(
                "userprocessservicetest", {}).get("checks", 0) < 100 \
            or receipt.get("host_receipt", {}).get("targets", {}).get(
                "processmemorytest", {}).get("checks", 0) < 100 \
            or receipt.get("host_receipt", {}).get("targets", {}).get(
                "anonmemorytest", {}).get("checks", 0) < 240:
        raise ValueError("user-process QEMU receipt bounded observations drifted")
    contracts = receipt.get("source_contracts", {})
    if contracts.get("process_slots") != 2 \
            or contracts.get("process_identity") != "generation-tagged slot handle" \
            or contracts.get("process_id_reuse") != \
            "allowed only after reap; stale generation rejected" \
            or contracts.get("termination_record") != \
            "distinct signed exit status or exact fault vector/error/address" \
            or contracts.get("persistent_service") != \
            "one bounded preemptible Ring-3 turn per kernel work-loop call" \
            or contracts.get("scheduler_owner") != \
            "exact generation-tagged lifecycle handle" \
            or contracts.get("scheduler_policy") != \
            "fixed-capacity round robin with one running owner" \
            or contracts.get("scheduler_failure") != \
            "lifecycle-policy disagreement fail-stops subsequent work" \
            or contracts.get("user_code") != "read-execute" \
            or contracts.get("user_stack") != "read-write-no-execute" \
            or contracts.get("lower_stack_guard_pte") != "absent" \
            or contracts.get("kernel_stack_pages") != 2 \
            or contracts.get("kernel_stack") != "supervisor-read-write-no-execute" \
            or contracts.get("lower_kernel_stack_guard_pte") != "absent" \
            or contracts.get("tss_rsp0") != \
            "process-private virtual kernel-stack top" \
            or contracts.get("return_order") != \
            "switch to permanent kernel stack before restoring kernel CR3" \
            or contracts.get("inherited_kernel_and_device_mappings") != "supervisor-only" \
            or contracts.get("physical_frame_owner") != \
            "one typed PMM owner per process slot" \
            or contracts.get("physical_frames_per_process") != 8 \
            or contracts.get("physical_frame_limit_per_process_owner") != 16 \
            or contracts.get("anonymous_frame_limit_per_process_owner") != 32 \
            or contracts.get("replacement") != \
            "allocate complete successor before releasing predecessor" \
            or contracts.get("reclamation") != \
            "preflight all frame owners before release" \
            or contracts.get("copy_validation") != \
            "complete fixed or committed-anonymous range before first byte" \
            or contracts.get("anonymous_window_pages") != 32 \
            or contracts.get("anonymous_first_pte") != 6 \
            or contracts.get("anonymous_states") != \
            ["free", "reserved", "committed", "broken"] \
            or contracts.get("anonymous_commit") != \
            "allocate and zero all frames before atomic PTE publication" \
            or contracts.get("anonymous_reclamation") != \
            "atomic unmap before owner-checked PMM release" \
            or contracts.get("hardware_pte_bits") != \
            "accessed and dirty accepted without weakening ownership checks" \
            or contracts.get("syscall_numbers") != {
                "abi_version": 1, "first": 1, "last": 24,
                "dispatch": "generated unsigned admission",
                "unknown_result": "ENOSYS"}:
        raise ValueError("user-process QEMU receipt source contract drifted")
    gaps = receipt.get("known_gaps", [])
    if len(gaps) != 10 or not any("physical" in gap for gap in gaps):
        raise ValueError("user-process QEMU receipt hides its known gaps")


def validate_user_process_command_receipt(receipt: dict, build_identity: str,
                                          evidence_root: Path) -> None:
    if receipt.get("schema") != \
            "zlos.user-process-command-native-uefi64-qemu-receipt.v1" \
            or receipt.get("result") != \
            "PASS_EXTERNAL_FILE_SPAWN_FAULT_OBSERVE_REAP" \
            or receipt.get("route") != "native-uefi64" \
            or receipt.get("build_identity") != build_identity:
        raise ValueError("user-process command receipt is absent, failed or foreign")
    identified = [receipt.get("artifact", {}), receipt.get("probe", {})]
    identified.extend(receipt.get("implementation", []))
    required = {
        "kernel/zlOS-usb.img",
        "kernel/tools/probes/probe-user-process.py",
        "freestanding/runtime_kernel.c",
        "kernel/src/kernel.zl",
        "kernel/src/graphics/windowing/term.c",
        "kernel/src/arch/x86/usermode.c",
        "kernel/src/core/process_lifecycle.c",
        "kernel/src/core/scheduler_policy.c",
        "kernel/src/core/user_process_service.c",
    }
    if {row.get("path") for row in identified} != required or any(
            digest(evidence_root / row["path"]) != row.get("sha256")
            for row in identified):
        raise ValueError("user-process command source or artifact identity drifted")
    if receipt.get("fixture") != {
            "path": "/system/user.bin",
            "bytes": 4,
            "content_hex": "61616161",
            "created_through": "Files app and disk-backed editor",
            "expected_execution": "invalid opcode fault in x86-64 mode",
            }:
        raise ValueError("user-process command fixture drifted")
    if receipt.get("assertions") != [
            "external /system/user.bin exists as 4 zlfs bytes",
            "userexec admitted pid 1000",
            "invalid 0x61 user instruction became a contained Ring-3 fault",
            "userps retained exact terminal state",
            "userreap released slot 1",
            "userps reported an empty table after reap",
            ]:
        raise ValueError("user-process command observations drifted")
    if len(receipt.get("serial_transcript_sha256", "")) != 64:
        raise ValueError("user-process command transcript identity is absent")
    gaps = receipt.get("known_gaps", [])
    if len(gaps) != 4 or not any("physical" in gap for gap in gaps) \
            or not any("normal exit" in gap for gap in gaps):
        raise ValueError("user-process command receipt hides its known gaps")


def validate_user_process_exit_receipt(receipt: dict, build_identity: str,
                                       evidence_root: Path) -> None:
    if receipt.get("schema") != \
            "zlos.user-process-exit-native-uefi64-qemu-receipt.v1" \
            or receipt.get("result") != \
            "PASS_EXTERNAL_FILE_SPAWN_EXIT_OBSERVE_REAP" \
            or receipt.get("route") != "native-uefi64" \
            or receipt.get("build_identity") != build_identity:
        raise ValueError("user-process exit receipt is absent, failed or foreign")
    identified = [receipt.get("artifact", {}), receipt.get("probe", {})]
    identified.extend(receipt.get("implementation", []))
    required = {
        "kernel/zlOS-usb.img",
        "kernel/tools/probes/probe-user-process-exit.py",
        "kernel/tests/host/zlfsseed.c",
        "kernel/src/fs/fs.c",
        "freestanding/runtime_kernel.c",
        "kernel/src/kernel.zl",
        "kernel/src/arch/x86/usermode.c",
        "kernel/src/core/process_lifecycle.c",
        "kernel/src/core/scheduler_policy.c",
        "kernel/src/core/user_process_service.c",
    }
    if {row.get("path") for row in identified} != required or any(
            digest(evidence_root / row["path"]) != row.get("sha256")
            for row in identified):
        raise ValueError("user-process exit source or artifact identity drifted")
    fixture = receipt.get("fixture", {})
    expected_hex = (
        "bb52000000b801000000cd80"
        "bb33000000b801000000cd80"
        "bb21000000b801000000cd80"
        "bb25000000b803000000cd80"
        "0f0b"
    )
    if fixture != {
            "path": "/system/user.bin",
            "bytes": 50,
            "sha256": hashlib.sha256(bytes.fromhex(expected_hex)).hexdigest(),
            "content_hex": expected_hex,
            "created_through":
                "host instrument linked to the shipping zlfs implementation",
            "expected_output": "R3!",
            "expected_exit_status": 37,
            }:
        raise ValueError("user-process normal-exit fixture drifted")
    if receipt.get("assertions") != [
            "external /system/user.bin exists as exact raw x86-64 bytes",
            "userexec admitted pid 1000",
            "the external program emitted R3! through three Ring-3 syscalls",
            "userps retained normal exit status 37",
            "userreap released slot 1",
            "userps reported an empty table after reap",
            ]:
        raise ValueError("user-process normal-exit observations drifted")
    if len(receipt.get("serial_transcript_sha256", "")) != 64:
        raise ValueError("user-process exit transcript identity is absent")
    gaps = receipt.get("known_gaps", [])
    if len(gaps) != 4 or not any("physical" in gap for gap in gaps) \
            or not any("process-handle" in gap for gap in gaps):
        raise ValueError("user-process exit receipt hides its known gaps")


def validate_feature_status_value(value: dict) -> None:
    if value.get("schema") != "zlos.feature-status.v1" \
            or value.get("result") != "PASS_WITH_OPEN_GAPS":
        raise ValueError("wrong feature-status schema/result")
    expected = master.parse_features(master.read(master.SOURCE))
    rows = value.get("features", [])
    expected_ids = [row["id"] for row in expected]
    if [row.get("id") for row in rows] != expected_ids \
            or len(set(expected_ids)) != len(expected_ids):
        raise ValueError("feature-status identity/order drift")
    maturities = {"PLANNED_UNPROVED", "PROVED_CURRENT", "PARTIAL_CURRENT",
                  "HISTORICAL_ONLY", "PARTIAL_HISTORICAL"}
    phases = set(phase_dependencies())
    for row in rows:
        if row.get("maturity") not in maturities or not row.get("known_gaps") \
                and row.get("maturity") == "PLANNED_UNPROVED":
            raise ValueError(f"{row.get('id')}: invalid maturity or hidden gap")
        if row.get("maturity") == "PROVED_CURRENT" and not row.get("evidence"):
            raise ValueError(f"{row.get('id')}: current proof has no evidence")
        if any(len(item.get("sha256", "")) != 64 for item in row.get("evidence", [])):
            raise ValueError(f"{row.get('id')}: evidence identity missing")
        if row.get("primary_phase") not in phases \
                or any(item not in phases for item in row.get("phase_dependencies", [])):
            raise ValueError(f"{row.get('id')}: unknown phase dependency")
    counts = Counter(row["maturity"] for row in rows)
    if value.get("counts") != {"total": len(rows), **dict(sorted(counts.items()))}:
        raise ValueError("feature-status counts drift")
    if len(value.get("build_identity", "")) != 64:
        raise ValueError("feature-status build identity missing")
    blockers = value.get("global_blockers", {})
    closed = {
        "current_artifact_snapshot_missing",
        "current_qemu_evidence_missing",
        "current_host_benchmark_missing",
        "current_host_test_receipt_missing",
        "future_build_graph_outputs_missing",
        "dependency_offline_resolution_missing",
        "provenance_current_screenshot_missing",
    }
    for name, item in blockers.items():
        if isinstance(item, bool) and item is not (name not in closed):
            raise ValueError(f"feature-status blocker state drift: {name}")
    for name, item in blockers.items():
        if isinstance(item, int) and not isinstance(item, bool) \
                and name not in {"decision_legacy_semantics_open", "performance_over_budget",
                                 "dependency_source_archives_missing"} \
                and item <= 0:
            raise ValueError(f"feature-status hid numeric blocker: {name}")
    for name in ("public_release_blocked",):
        if blockers.get(name) is not True:
            raise ValueError(f"feature-status hid blocker: {name}")


def build(evidence_root: Path) -> dict:
    source_text = master.read(master.SOURCE)
    features = master.parse_features(source_text)
    master.validate_features(features)
    dependencies = phase_dependencies()
    joined = json.loads((evidence_root / "kernel/metadata/evidence-registry.json").read_text())
    if joined.get("result") != "PASS_CURRENT_INDEX_WITH_MIXED_EXECUTION_EVIDENCE_AND_OPEN_GAPS":
        raise ValueError("implementation evidence registry is absent or overpromoted")
    decisions = json.loads((evidence_root / "kernel/metadata/decision-ledger.json").read_text())
    release_notes = json.loads((evidence_root / "kernel/metadata/release-notes.json").read_text())
    provenance = json.loads((evidence_root / "kernel/metadata/provenance-viewer.json").read_text())
    benchmark = json.loads((
        evidence_root / "kernel/docs/receipts/benchmark-host-2026-08-23.json"
    ).read_text())
    build_benchmark = json.loads((
        evidence_root / "kernel/docs/receipts/build-benchmark-host-2026-08-29.json"
    ).read_text())
    performance = json.loads((
        evidence_root / "kernel/metadata/performance-registry.json"
    ).read_text())
    dependency = json.loads((
        evidence_root / "kernel/metadata/dependency-lock.json"
    ).read_text())
    dependency_archive_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/dependency-archives-host-2026-08-29.json"
    ).read_text())
    rtc_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/rtc-clock-qemu-2026-08-29.json"
    ).read_text())
    scheduler_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/scheduler-native-uefi64-qemu-2026-08-29.json"
    ).read_text())
    user_process_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"
    ).read_text())
    user_process_command_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json"
    ).read_text())
    user_process_exit_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/user-process-exit-native-uefi64-qemu-2026-09-03.json"
    ).read_text())
    host_receipt = json.loads((
        evidence_root / "kernel/tests/host/test-run-receipt.json"
    ).read_text())
    address_registry = json.loads((
        evidence_root / "kernel/metadata/address-space-registry.json"
    ).read_text())
    page_table_receipt = json.loads((
        evidence_root /
        "kernel/docs/receipts/page-table-native-uefi64-qemu-2026-08-29.json"
    ).read_text())
    pmm_receipt = json.loads((
        evidence_root /
        "kernel/docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json"
    ).read_text())
    feature_ids = {row["id"] for row in features}
    validate_decision_feature_links(decisions, feature_ids)
    validate_release_feature_links(release_notes, feature_ids)
    validate_provenance_feature_link(provenance, feature_ids)
    validate_dependency_archive_receipt(
        dependency_archive_receipt, dependency, joined["build_identity"], evidence_root
    )
    validate_rtc_receipt(rtc_receipt, joined["build_identity"])
    validate_heap_host_receipt(host_receipt, joined["build_identity"])
    validate_address_registry(address_registry, joined["build_identity"], evidence_root)
    validate_page_table_receipt(
        page_table_receipt, joined["build_identity"], evidence_root
    )
    validate_pmm_receipt(pmm_receipt, joined["build_identity"], evidence_root)
    validate_scheduler_receipt(scheduler_receipt, joined["build_identity"], evidence_root)
    validate_user_process_receipt(
        user_process_receipt, joined["build_identity"], evidence_root
    )
    validate_user_process_command_receipt(
        user_process_command_receipt, joined["build_identity"], evidence_root
    )
    validate_user_process_exit_receipt(
        user_process_exit_receipt, joined["build_identity"], evidence_root
    )
    if joined.get("counts", {}).get("decision_records") != len(decisions["records"]) \
            or joined.get("open_gaps", {}).get("decision_legacy_semantics_open") \
            != decisions.get("coverage", {}).get("legacy_labels_remaining"):
        raise ValueError("joined decision-ledger evidence drift")
    if joined.get("counts", {}).get("release_change_candidates") \
            != release_notes.get("counts", {}).get("unreleased_change_candidates") \
            or joined.get("open_gaps", {}).get("release_publication_blocked") is not True:
        raise ValueError("joined release-note evidence drift")
    if joined.get("counts", {}).get("provenance_applications") \
            != provenance.get("counts", {}).get("applications") \
            or joined.get("open_gaps", {}).get("provenance_runtime_route_missing") is not True:
        raise ValueError("joined provenance-viewer evidence drift")
    rows = []
    for feature in features:
        phase, workstream = master.DOMAIN[feature["prefix"]]
        override = OVERRIDES.get(feature["id"])
        if override:
            status, refs, routes, artifacts, tests, gaps, weakest = override
            evidence = [evidence_entry(kind, path, evidence_root) for kind, path in refs]
            if feature["id"] == "EV-008":
                weakest = (
                    f"{joined['counts']['landing_gate_mandatory_seams']} mandatory landing seams "
                    "are guarded, not every repository command path"
                )
            elif feature["id"] == "EV-014":
                failed = [row["name"] for row in benchmark["metrics"]
                          if not row["within_60hz_budget"]]
                weakest = (
                    f"all 7 current host frame metrics retain raw samples and "
                    f"p50/p95/peak values; {len(failed)} exceed the 60 Hz budget "
                    f"({', '.join(failed) or 'none'}), and target performance remains unmeasured"
                )
            elif feature["id"] == "EV-021":
                regressions = performance["open_regressions"]
                weakest = (
                    f"all seven categories are measured on the current host; "
                    f"{performance['counts']['host_budget_passed']} pass and "
                    f"{performance['counts']['host_budget_failed']} fail "
                    f"({', '.join(regressions)}), including host-build p95 "
                    f"{build_benchmark['p95_us'] / 1_000_000.0:.3f} seconds against "
                    "a 60-second guard; product/target latency remains unmeasured"
                )
        else:
            status = "PLANNED_UNPROVED"
            routes, artifacts, tests, evidence = (), (), (), []
            gaps = ("no feature-specific implementation receipt is joined",)
            weakest = "the complete acceptance contract is planned but unproved"
        rows.append({
            "id": feature["id"],
            "feature": feature["feature"],
            "research_state": feature["state"],
            "primary_phase": phase,
            "phase_dependencies": dependencies[phase],
            "owner": workstream,
            "maturity": status,
            "acceptance_contract": feature["meaning"],
            "routes": list(routes),
            "artifacts": list(artifacts),
            "tests": list(tests),
            "evidence": evidence,
            "known_gaps": list(gaps),
            "rejected_claims": ["source/build/host/QEMU proof does not imply native-hardware completion"],
            "weakest_evidence": weakest,
        })
    counts = Counter(row["maturity"] for row in rows)
    return {
        "schema": "zlos.feature-status.v1",
        "result": "PASS_WITH_OPEN_GAPS",
        "source_catalogue_sha256": hashlib.sha256(source_text.encode()).hexdigest(),
        "feature_map_sha256": digest(ROOT / "docs/program/FEATURE-MAP.md"),
        "implementation_evidence_registry_sha256": digest(evidence_root / "kernel/metadata/evidence-registry.json"),
        "decision_ledger_sha256": digest(evidence_root / "kernel/metadata/decision-ledger.json"),
        "release_notes_sha256": digest(evidence_root / "kernel/metadata/release-notes.json"),
        "provenance_viewer_sha256": digest(evidence_root / "kernel/metadata/provenance-viewer.json"),
        "build_identity": joined["build_identity"],
        "implementation_tree": git_snapshot(evidence_root),
        "counts": {"total": len(rows), **dict(sorted(counts.items()))},
        "global_blockers": {
            "public_release_blocked": joined["open_gaps"]["public_release_blocked"],
            "source_snapshot_off_host_missing": joined["open_gaps"]["source_snapshot_off_host_missing"],
            "source_snapshot_signature_missing": joined["open_gaps"]["source_snapshot_signature_missing"],
            "whole_repository_snapshot_missing": joined["open_gaps"]["whole_repository_snapshot_missing"],
            "hermetic_toolchain_missing": joined["open_gaps"]["hermetic_toolchain_missing"],
            "signed_toolchain_attestation_missing": joined["open_gaps"]["signed_toolchain_attestation_missing"],
            "toolchain_source_archive_missing": joined["open_gaps"]["toolchain_source_archive_missing"],
            "dependency_offline_rebuild_missing": joined["open_gaps"]["dependency_offline_rebuild_missing"],
            "dependency_source_archives_missing": joined["open_gaps"]["dependency_source_archives_missing"],
            "per_object_provenance_receipts_missing": joined["open_gaps"]["per_object_provenance_receipts_missing"],
            "build_graph_scope_only_inputs": joined["open_gaps"]["build_graph_scope_only_inputs"],
            "future_build_graph_outputs_missing": joined["open_gaps"]["future_build_graph_outputs_missing"],
            "address_low_shared_internal_manifest_missing": joined["open_gaps"]["address_low_shared_internal_manifest_missing"],
            "address_dynamic_user_base_runtime_only": joined["open_gaps"]["address_dynamic_user_base_runtime_only"],
            "address_page_table_transaction_coverage_incomplete": joined["open_gaps"]["address_page_table_transaction_coverage_incomplete"],
            "address_physical_execution_missing": joined["open_gaps"]["address_physical_execution_missing"],
            "physical_exact_hash_artifacts": joined["open_gaps"]["physical_exact_hash_artifacts"],
            "failure_injection_open_families": joined["open_gaps"]["failure_injection_open_families"],
            "hostile_corpus_open_families": joined["open_gaps"]["hostile_corpus_open_families"],
            "performance_over_budget": joined["open_gaps"]["performance_over_budget"],
            "performance_product_build_missing": joined["open_gaps"]["performance_product_build_missing"],
            "visual_unbound_assets": joined["open_gaps"]["visual_unbound_assets"],
            "accessibility_missing_capabilities": joined["open_gaps"]["accessibility_missing_capabilities"],
            "security_open_claims": joined["open_gaps"]["security_open_claims"],
            "decision_legacy_semantics_open": joined["open_gaps"]["decision_legacy_semantics_open"],
            "decision_system_inventory_missing": joined["open_gaps"]["decision_system_inventory_missing"],
            "decision_independent_approval_missing": joined["open_gaps"]["decision_independent_approval_missing"],
            "release_generation_missing": joined["open_gaps"]["release_generation_missing"],
            "release_publication_blocked": joined["open_gaps"]["release_publication_blocked"],
            "release_signature_missing": joined["open_gaps"]["release_signature_missing"],
            "release_published_entries_missing": joined["open_gaps"]["release_published_entries_missing"],
            "release_migration_inventory_incomplete": joined["open_gaps"]["release_migration_inventory_incomplete"],
            "release_previous_rollback_missing": joined["open_gaps"]["release_previous_rollback_missing"],
            "provenance_runtime_route_missing": joined["open_gaps"]["provenance_runtime_route_missing"],
            "provenance_current_screenshot_missing": joined["open_gaps"]["provenance_current_screenshot_missing"],
            "provenance_permissions_missing": joined["open_gaps"]["provenance_permissions_missing"],
            "provenance_signatures_missing": joined["open_gaps"]["provenance_signatures_missing"],
            "provenance_live_health_missing": joined["open_gaps"]["provenance_live_health_missing"],
            "provenance_target_accessibility_missing": joined["open_gaps"]["provenance_target_accessibility_missing"],
            "provenance_remote_auth_missing": joined["open_gaps"]["provenance_remote_auth_missing"],
            "provenance_public_release_missing": joined["open_gaps"]["provenance_public_release_missing"],
            "observability_open_capabilities": joined["open_gaps"]["observability_open_capabilities"],
            "current_artifact_snapshot_missing": joined["open_gaps"]["current_artifact_snapshot_missing"],
            "current_qemu_evidence_missing": joined["open_gaps"]["current_qemu_evidence_missing"],
            "current_host_test_receipt_missing": joined["open_gaps"]["current_host_test_receipt_missing"],
            "current_host_benchmark_missing": joined["open_gaps"]["current_host_benchmark_missing"],
        },
        "features": rows,
        "generator": {"path": "tools/gen_feature_status.py", "sha256": digest(Path(__file__).resolve())},
        "evidence_ceiling": "conservative plan-to-current-evidence join; unlisted current assets are not promoted",
    }


def selftest(value: dict, evidence_root: Path) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["features"].pop()
    mutations["missing-feature"] = missing
    duplicate = copy.deepcopy(value)
    duplicate["features"][-1]["id"] = duplicate["features"][0]["id"]
    mutations["duplicate-feature"] = duplicate
    dependency = copy.deepcopy(value)
    dependency["features"][100]["phase_dependencies"] = ["MP-99"]
    mutations["unknown-dependency"] = dependency
    promotion = copy.deepcopy(value)
    target = next(row for row in promotion["features"] if row["maturity"] == "PLANNED_UNPROVED")
    target["maturity"] = "PROVED_CURRENT"
    mutations["unproved-promotion"] = promotion
    hidden = copy.deepcopy(value)
    hidden["global_blockers"]["public_release_blocked"] = False
    mutations["hidden-release-block"] = hidden
    custody = copy.deepcopy(value)
    custody["global_blockers"]["source_snapshot_off_host_missing"] = False
    mutations["invented-source-custody"] = custody
    hermetic = copy.deepcopy(value)
    hermetic["global_blockers"]["hermetic_toolchain_missing"] = False
    mutations["invented-hermetic-toolchain"] = hermetic
    graph = copy.deepcopy(value)
    graph["global_blockers"]["build_graph_scope_only_inputs"] = 0
    mutations["hidden-build-graph-superset"] = graph
    future_graph = copy.deepcopy(value)
    future_graph["global_blockers"]["future_build_graph_outputs_missing"] = True
    mutations["reopened-future-build-graph"] = future_graph
    benchmark = copy.deepcopy(value)
    benchmark["global_blockers"]["current_host_benchmark_missing"] = True
    mutations["reopened-current-host-benchmark"] = benchmark
    host_tests = copy.deepcopy(value)
    host_tests["global_blockers"]["current_host_test_receipt_missing"] = True
    mutations["reopened-current-host-test"] = host_tests
    identity = copy.deepcopy(value)
    identity["build_identity"] = "short"
    mutations["missing-build-identity"] = identity
    caught = []
    for name, mutant in mutations.items():
        try:
            validate_feature_status_value(mutant)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"feature-status selftest mutation escaped: {name}")
    decisions = json.loads((evidence_root / "kernel/metadata/decision-ledger.json").read_text())
    decision_mutant = copy.deepcopy(decisions)
    decision_mutant["records"][0]["features"] = ["EV-999"]
    feature_ids = {row["id"] for row in value["features"]}
    try:
        validate_decision_feature_links(decision_mutant, feature_ids)
    except ValueError:
        caught.append("unknown-decision-feature")
    else:
        raise ValueError("feature-status selftest mutation escaped: unknown-decision-feature")
    release_notes = json.loads((evidence_root / "kernel/metadata/release-notes.json").read_text())
    release_mutant = copy.deepcopy(release_notes)
    release_mutant["known_issues"][0]["feature_ids"] = ["EV-999"]
    try:
        validate_release_feature_links(release_mutant, feature_ids)
    except ValueError:
        caught.append("unknown-release-feature")
    else:
        raise ValueError("feature-status selftest mutation escaped: unknown-release-feature")
    provenance = json.loads((evidence_root / "kernel/metadata/provenance-viewer.json").read_text())
    provenance_mutant = copy.deepcopy(provenance)
    provenance_mutant["feature_id"] = "EV-999"
    try:
        validate_provenance_feature_link(provenance_mutant, feature_ids)
    except ValueError:
        caught.append("unknown-provenance-feature")
    else:
        raise ValueError("feature-status selftest mutation escaped: unknown-provenance-feature")
    dependency = json.loads((
        evidence_root / "kernel/metadata/dependency-lock.json"
    ).read_text())
    dependency_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/dependency-archives-host-2026-08-29.json"
    ).read_text())
    dependency_receipt["offline_resolution"]["all_source_archives_present"] = False
    try:
        validate_dependency_archive_receipt(
            dependency_receipt, dependency, value["build_identity"], evidence_root
        )
    except ValueError:
        caught.append("missing-dependency-source-archive")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: missing-dependency-source-archive"
        )
    rtc_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/rtc-clock-qemu-2026-08-29.json"
    ).read_text())
    rtc_receipt["build_identity"] = "0" * 64
    try:
        validate_rtc_receipt(rtc_receipt, value["build_identity"])
    except ValueError:
        caught.append("foreign-rtc-receipt")
    else:
        raise ValueError("feature-status selftest mutation escaped: foreign-rtc-receipt")
    heap_receipt = json.loads((
        evidence_root / "kernel/tests/host/test-run-receipt.json"
    ).read_text())
    heap_row = next(row for row in heap_receipt["results"]
                    if row["name"] == "heaptest")
    heap_row["commands"][0]["output_tail"] = "1937 checks, 0 failures"
    try:
        validate_heap_host_receipt(heap_receipt, value["build_identity"])
    except ValueError:
        caught.append("missing-heap-observation")
    else:
        raise ValueError("feature-status selftest mutation escaped: missing-heap-observation")
    address_registry = json.loads((
        evidence_root / "kernel/metadata/address-space-registry.json"
    ).read_text())
    address_registry["categories"].remove("guard")
    try:
        validate_address_registry(
            address_registry, value["build_identity"], evidence_root
        )
    except ValueError:
        caught.append("missing-address-category")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: missing-address-category"
        )
    page_table_receipt = json.loads((
        evidence_root /
        "kernel/docs/receipts/page-table-native-uefi64-qemu-2026-08-29.json"
    ).read_text())
    page_table_receipt["assertions"][0]["nth_write_failures"] = 511
    try:
        validate_page_table_receipt(
            page_table_receipt, value["build_identity"], evidence_root
        )
    except ValueError:
        caught.append("missing-page-table-injection")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: missing-page-table-injection"
        )
    pmm_receipt = json.loads((
        evidence_root /
        "kernel/docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json"
    ).read_text())
    pmm_receipt["assertions"][0]["free_pages_after_selftest"] -= 1
    try:
        validate_pmm_receipt(pmm_receipt, value["build_identity"], evidence_root)
    except ValueError:
        caught.append("invented-pmm-baseline")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: invented-pmm-baseline"
        )
    pmm_receipt = json.loads((
        evidence_root /
        "kernel/docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json"
    ).read_text())
    next(assertion for assertion in pmm_receipt["assertions"]
         if assertion["id"] == "exact-owner-accounting-and-quota")[
             "metadata_owner_sum_invariant"] = False
    try:
        validate_pmm_receipt(pmm_receipt, value["build_identity"], evidence_root)
    except ValueError:
        caught.append("invented-pmm-owner-accounting")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: invented-pmm-owner-accounting"
        )
    pmm_receipt = json.loads((
        evidence_root /
        "kernel/docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json"
    ).read_text())
    pmm_receipt["implementation"].pop()
    try:
        validate_pmm_receipt(pmm_receipt, value["build_identity"], evidence_root)
    except ValueError:
        caught.append("missing-pmm-integration")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: missing-pmm-integration"
        )
    scheduler_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/scheduler-native-uefi64-qemu-2026-08-29.json"
    ).read_text())
    scheduler_receipt["assertions"].pop()
    try:
        validate_scheduler_receipt(scheduler_receipt, value["build_identity"], evidence_root)
    except ValueError:
        caught.append("missing-scheduler-observation")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: missing-scheduler-observation"
        )
    user_process_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"
    ).read_text())
    user_process_receipt["source_contracts"]["user_stack"] = "read-write-execute"
    try:
        validate_user_process_receipt(
            user_process_receipt, value["build_identity"], evidence_root
        )
    except ValueError:
        caught.append("invented-executable-user-stack")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: invented-executable-user-stack"
        )
    user_process_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"
    ).read_text())
    next(assertion for assertion in user_process_receipt["assertions"]
         if assertion["id"] == "lower-stack-guard-fault-containment")[
             "fault_address"] = "unobserved"
    try:
        validate_user_process_receipt(
            user_process_receipt, value["build_identity"], evidence_root
        )
    except ValueError:
        caught.append("invented-user-stack-guard-fault")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: invented-user-stack-guard-fault"
        )
    user_process_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"
    ).read_text())
    next(assertion for assertion in user_process_receipt["assertions"]
         if assertion["id"] == "guarded-supervisor-tss-stacks")[
             "observed_high_water_bytes"]["slot1"] = 8192
    try:
        validate_user_process_receipt(
            user_process_receipt, value["build_identity"], evidence_root
        )
    except ValueError:
        caught.append("invented-kernel-stack-headroom")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: invented-kernel-stack-headroom"
        )
    user_process_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"
    ).read_text())
    next(assertion for assertion in user_process_receipt["assertions"]
         if assertion["id"] == "process-memory-accounting")[
             "owner_totals_after_release"] = 1
    try:
        validate_user_process_receipt(
            user_process_receipt, value["build_identity"], evidence_root
        )
    except ValueError:
        caught.append("invented-process-owner-total")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: invented-process-owner-total"
        )
    user_process_receipt = json.loads((
        evidence_root / "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"
    ).read_text())
    user_process_receipt["assertions"].pop()
    try:
        validate_user_process_receipt(
            user_process_receipt, value["build_identity"], evidence_root
        )
    except ValueError:
        caught.append("missing-process-frame-reclamation")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: missing-process-frame-reclamation"
        )
    user_process_command_receipt = json.loads((
        evidence_root /
        "kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json"
    ).read_text())
    user_process_command_receipt["fixture"]["content_hex"] = "90909090"
    try:
        validate_user_process_command_receipt(
            user_process_command_receipt, value["build_identity"], evidence_root
        )
    except ValueError:
        caught.append("invented-user-process-command-fixture")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: invented-user-process-command-fixture"
        )
    user_process_exit_receipt = json.loads((
        evidence_root /
        "kernel/docs/receipts/user-process-exit-native-uefi64-qemu-2026-09-03.json"
    ).read_text())
    user_process_exit_receipt["fixture"]["expected_exit_status"] = 0
    try:
        validate_user_process_exit_receipt(
            user_process_exit_receipt, value["build_identity"], evidence_root
        )
    except ValueError:
        caught.append("invented-user-process-exit-status")
    else:
        raise ValueError(
            "feature-status selftest mutation escaped: invented-user-process-exit-status"
        )
    print("feature-status selftest: caught " + ", ".join(caught))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--evidence-root", type=Path, default=DEFAULT_EVIDENCE_ROOT)
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        value = build(args.evidence_root.resolve())
        validate_feature_status_value(value)
        if args.selftest:
            selftest(value, args.evidence_root.resolve())
        if args.write:
            OUTPUT.write_text(json.dumps(value, indent=2) + "\n")
        elif json.loads(OUTPUT.read_text()) != value:
            raise ValueError("FEATURE-STATUS.json is stale; run --write and inspect the diff")
        print("feature-status: PASS_WITH_OPEN_GAPS: " + json.dumps(value["counts"], sort_keys=True))
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.CalledProcessError) as error:
        print(f"feature-status: FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
