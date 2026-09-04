#!/usr/bin/env python3
"""Join current MP-00 receipts without promoting their evidence ceilings."""

from __future__ import annotations

import argparse
import ast
import copy
import hashlib
import json
import os
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
METADATA = KERNEL_ROOT / "metadata"
ROOT = KERNEL_ROOT.parent
OUTPUT = METADATA / "evidence-registry.json"
INPUTS = (
    "build-identity.json",
    "docs/receipts/source-snapshot-2026-08-24.json",
    "app-manifest.json",
    "app-evidence.json",
    "artifact-registry.json",
    "hardware-receipt-plan.json",
    "init-registry.json",
    "dependency-lock.json",
    "docs/receipts/dependency-archives-host-2026-08-29.json",
    "wrapper-registry.json",
    "toolchain-manifest.json",
    "build-graph.json",
    "address-space-registry.json",
    "license-registry.json",
    "adversarial-registry.json",
    "test-inventory.json",
    "tests/host/test-run-receipt.json",
    "docs/receipts/reproducible-build-2026-08-22.json",
    "docs/receipts/benchmark-host-2026-08-23.json",
    "docs/receipts/build-benchmark-host-2026-08-29.json",
    "performance-registry.json",
    "docs/receipts/page-table-native-uefi64-qemu-2026-08-29.json",
    "docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json",
    "docs/receipts/cpu-fault-invalid-opcode-qemu-2026-08-23.json",
    "docs/receipts/cpu-fault-native-uefi64-qemu-2026-08-29.json",
    "docs/receipts/cpu-fault-general-protection-native-uefi64-qemu-2026-08-29.json",
    "docs/receipts/cpu-fault-double-fault-native-uefi64-qemu-2026-08-29.json",
    "docs/receipts/visual-qemu-2026-08-29.json",
    "visual-golden-registry.json",
    "visual-registry.json",
    "accessibility-registry.json",
    "security-registry.json",
    "decision-ledger.json",
    "release-notes.json",
    "provenance-viewer.json",
    "docs/receipts/event-trace-host-2026-08-24.json",
    "event-schema.json",
    "observability-registry.json",
)


def sha256(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def input_path(relative: str) -> Path:
    if relative.startswith("docs/"):
        return KERNEL_ROOT / relative
    if relative.startswith("tests/"):
        return KERNEL_ROOT / relative
    return METADATA / relative


def load(relative: str) -> dict:
    return json.loads(input_path(relative).read_text())


def landing_gate_seam_count() -> int:
    tree = ast.parse((KERNEL_ROOT / "tools/checks/check-land-gate.py").read_text())
    for statement in tree.body:
        if isinstance(statement, ast.Assign) \
                and any(isinstance(target, ast.Name) and target.id == "REQUIRED_SNIPPETS"
                        for target in statement.targets):
            value = ast.literal_eval(statement.value)
            if not isinstance(value, tuple) or not value:
                break
            return len(value)
    raise ValueError("cannot derive mandatory landing-gate seam count")


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.evidence-registry.v1":
        raise ValueError("wrong evidence-registry schema")
    if value.get("result") != "PASS_CURRENT_INDEX_WITH_MIXED_EXECUTION_EVIDENCE_AND_OPEN_GAPS":
        raise ValueError("evidence registry must preserve open gaps")
    inputs = value.get("inputs")
    expected_input_paths = [input_path(path).relative_to(ROOT).as_posix()
                            for path in INPUTS]
    if not isinstance(inputs, list) or [row.get("path") for row in inputs] != expected_input_paths:
        raise ValueError("evidence input set/order drift")
    for row, relative in zip(inputs, INPUTS):
        current = load(relative)
        if row.get("sha256") != sha256(input_path(relative)) \
                or row.get("schema") != current.get("schema"):
            raise ValueError(f"{row.get('path')}: stale evidence input binding")
        if len(row.get("sha256", "")) != 64 or not row.get("schema"):
            raise ValueError(f"{row.get('path')}: missing receipt identity")
    counts = value.get("counts", {})
    manifest = load("app-manifest.json")
    app = load("app-evidence.json")
    wrappers = load("wrapper-registry.json")
    dependency = load("dependency-lock.json")
    dependency_archives = load("docs/receipts/dependency-archives-host-2026-08-29.json")
    provenance = load("provenance-viewer.json")
    performance = load("performance-registry.json")
    hardware = load("hardware-receipt-plan.json")
    build_graph = load("build-graph.json")
    address = load("address-space-registry.json")
    host_receipt = load("tests/host/test-run-receipt.json")
    host_counts = host_receipt.get("counts", {})
    application_count = len(manifest.get("entries", []))
    build_input_count = len(load("build-identity.json").get("source_files_sha256", {}))
    expected_counts = {
        "reproducible_artifacts": 9,
        "source_snapshot_inputs": build_input_count,
        "source_snapshot_off_host_copies": 0,
        "qemu_boot_routes": 6,
        "app_identities": application_count,
        "app_lifecycle_proved": app.get("counts", {}).get("with_qemu_open_ready_close"),
        "init_stages": 18,
        "dependency_commands": len(dependency.get("commands", [])),
        "dependency_firmware_blobs": len(dependency.get("firmware", [])),
        "dependency_runtime_files": dependency.get("closure", {}).get("runtime_files"),
        "dependency_transitive_packages": dependency.get("closure", {}).get("transitive_packages"),
        "dependency_source_archives_retained": dependency.get("closure", {}).get("source_archives_retained"),
        "dependency_binary_archives_retained": dependency_archives.get("counts", {}).get("binary_archives"),
        "dependency_source_packages_retained": dependency_archives.get("counts", {}).get("source_packages"),
        "dependency_packages_with_source_archives": dependency_archives.get("counts", {}).get("packages_with_source_archives"),
        "dependency_unindexed_binary_archives": dependency_archives.get("counts", {}).get("unindexed_binary_archives"),
        "dependency_unindexed_source_packages": dependency_archives.get("counts", {}).get("unindexed_source_packages"),
        "wrapper_inventory": wrappers.get("counts", {}).get("wrappers"),
        "wrapper_named_by_landing_gate": wrappers.get("counts", {}).get(
            "named_by_landing_gate"
        ),
        "wrapper_legacy_policy_gaps": wrappers.get("counts", {}).get(
            "no_static_failure_policy"
        ),
        "toolchain_tools": 7,
        "toolchain_target_lanes": 4,
        "toolchain_external_headers": 82,
        "toolchain_hermetic_builds": 0,
        "build_graph_source_inputs": build_input_count,
        "build_graph_target_lanes": 4,
        "build_graph_artifacts": 9,
        "build_graph_orphan_inputs": 0,
        "build_graph_scope_only_inputs": build_graph.get("counts", {}).get(
            "scope_only_inputs"
        ),
        "address_physical_regions": address.get("counts", {}).get(
            "physical_regions"
        ),
        "address_user_template_regions": address.get("counts", {}).get(
            "user_template_regions"
        ),
        "address_source_assertions": address.get("counts", {}).get(
            "source_assertions"
        ),
        "address_categories": address.get("counts", {}).get("categories"),
        "page_table_transaction_receipts": 1,
        "physical_page_allocator_receipts": 1,
        "license_build_inputs": build_input_count,
        "license_files": 0,
        "host_targets": host_counts.get("targets"),
        "host_commands_executed": host_counts.get("commands_executed"),
        "host_passed": host_counts.get("passed"),
        "host_failed": host_counts.get("failed"),
        "host_hardware_skips": host_counts.get("skipped-hardware"),
        "host_not_run": host_counts.get("not-run"),
        "physical_exact_hash_proofs": 0,
        "hardware_matrix_cases": 6,
        "hardware_validated_receipts": 0,
        "hardware_physically_passed_routes": 0,
        "verifier_canaries_caught": 23,
        "landing_gate_mandatory_seams": counts.get("landing_gate_mandatory_seams"),
        "failure_seam_complete": 0,
        "hostile_host_passed": 7,
        "benchmark_measurements": 7,
        "benchmark_within_budget": counts.get("benchmark_within_budget"),
        "benchmark_over_budget": counts.get("benchmark_over_budget"),
        "benchmark_native_target_measurements": 0,
        "performance_categories": 7,
        "performance_host_budget_passed": performance.get("counts", {}).get("host_budget_passed"),
        "performance_host_budget_failed": performance.get("counts", {}).get("host_budget_failed"),
        "performance_missing_categories": 0,
        "visual_assets": 45,
        "visual_current_build_bound": 4,
        "visual_current_routes": 2,
        "visual_current_states": 2,
        "visual_strict_goldens": 4,
        "visual_dynamic_masks": 4,
        "visual_variant_dimensions_complete": 0,
        "visual_variant_dimensions_open": 6,
        "accessibility_capabilities": 15,
        "accessibility_host_passed": 4,
        "accessibility_partial": 2,
        "accessibility_missing": 9,
        "accessibility_complete_target_workflows": 0,
        "security_claims": 17,
        "security_static_proved": 1,
        "security_host_proved_limited": 4,
        "security_qemu_partial": 1,
        "security_missing": 11,
        "security_production_complete": 0,
        "decision_records": 20,
        "decision_legacy_labels_indexed": 47,
        "decision_legacy_labels_normalized": 47,
        "decision_legacy_labels_remaining": 0,
        "decision_superseded_records": 3,
        "release_change_candidates": 17,
        "release_user_visible_candidates": 6,
        "release_published_entries": 0,
        "release_migrations": 3,
        "release_known_issues": 12,
        "release_recovery_paths": 3,
        "release_available_recovery_paths": 2,
        "provenance_inputs": 22,
        "provenance_health_areas": 13,
        "provenance_artifacts": 9,
        "provenance_applications": provenance.get("counts", {}).get("applications"),
        "provenance_app_permission_grants": 0,
        "provenance_security_claims": 17,
        "provenance_change_candidates": 17,
        "provenance_cryptographic_signatures": 0,
        "observability_capabilities": 15,
        "observability_qemu_hash_only": 1,
        "observability_qemu_proved": 2,
        "observability_qemu_proved_partial": 1,
        "observability_host_limited": 1,
        "observability_host_core": 4,
        "observability_host_partial": 1,
        "observability_source_only": 0,
        "observability_missing": 5,
        "qemu_crash_receipts": 4,
        "durable_crash_receipts": 0,
        "typed_structured_event_fields": 28,
        "event_trace_host_checks": 37,
        "event_trace_compile_lanes": 3,
        "event_trace_target_emitters": 0,
        "current_build_bound_artifacts": 9,
        "historical_artifacts": 0,
        "current_build_bound_qemu_routes": 6,
        "historical_qemu_routes": 0,
        "current_build_bound_host_receipts": 6,
    }
    if counts != expected_counts:
        differences = {
            key: {"actual": counts.get(key), "expected": expected_counts.get(key)}
            for key in sorted(set(counts) | set(expected_counts))
            if counts.get(key) != expected_counts.get(key)
        }
        raise ValueError(f"evidence count drift: {differences}")
    gaps = value.get("open_gaps")
    expected_gaps = {
        "physical_exact_hash_artifacts": 9,
        "hardware_matrix_cases_without_receipts": 6,
        "host_hardware_skips": host_counts.get("skipped-hardware"),
        "host_non_runs": host_counts.get("not-run"),
        "inputs_without_redistribution_grant": build_input_count,
        "public_release_blocked": True,
        "source_snapshot_off_host_missing": True,
        "source_snapshot_signature_missing": True,
        "whole_repository_snapshot_missing": True,
        "hermetic_toolchain_missing": True,
        "signed_toolchain_attestation_missing": True,
        "toolchain_source_archive_missing": True,
        "dependency_offline_rebuild_missing": True,
        "dependency_offline_resolution_missing": False,
        "dependency_source_archives_missing": 0,
        "dependency_unindexed_archives": (
            dependency_archives.get("counts", {}).get("unindexed_binary_archives", 0)
            + dependency_archives.get("counts", {}).get("unindexed_source_packages", 0)
        ),
        "per_object_provenance_receipts_missing": True,
        "build_graph_scope_only_inputs": build_graph.get("counts", {}).get(
            "scope_only_inputs"
        ),
        "future_build_graph_outputs_missing": False,
        "address_low_shared_internal_manifest_missing": True,
        "address_dynamic_user_base_runtime_only": True,
        "address_page_table_transaction_coverage_incomplete": True,
        "address_physical_execution_missing": True,
        "failure_injection_open_families": 7,
        "hostile_corpus_open_families": 2,
        "performance_over_budget": performance.get("counts", {}).get(
            "host_budget_failed"
        ),
        "native_target_benchmark_missing": True,
        "benchmark_percentiles_missing": False,
        "performance_product_build_missing": True,
        "visual_unbound_assets": 41,
        "visual_variant_dimensions_open": 6,
        "accessibility_missing_capabilities": 9,
        "accessibility_target_workflows_missing": True,
        "security_open_claims": 17,
        "security_production_complete_missing": True,
        "decision_legacy_semantics_open": 0,
        "decision_system_inventory_missing": True,
        "decision_independent_approval_missing": True,
        "release_generation_missing": True,
        "release_publication_blocked": True,
        "release_signature_missing": True,
        "release_published_entries_missing": True,
        "release_migration_inventory_incomplete": True,
        "release_previous_rollback_missing": True,
        "provenance_runtime_route_missing": True,
        "provenance_current_screenshot_missing": False,
        "provenance_permissions_missing": True,
        "provenance_signatures_missing": True,
        "provenance_live_health_missing": True,
        "provenance_target_accessibility_missing": True,
        "provenance_remote_auth_missing": True,
        "provenance_public_release_missing": True,
        "observability_open_capabilities": 12,
        "durable_crash_receipt_missing": True,
        "typed_event_target_integration_missing": True,
        "current_artifact_snapshot_missing": False,
        "current_qemu_evidence_missing": False,
        "current_host_test_receipt_missing": False,
        "current_host_benchmark_missing": False,
    }
    if gaps != expected_gaps:
        raise ValueError("open evidence gaps were hidden or drifted")
    if not isinstance(counts.get("benchmark_over_budget"), int) \
            or counts.get("benchmark_over_budget", -1) < 0 \
            or counts.get("benchmark_within_budget", -1) + counts.get("benchmark_over_budget", -1) != 7:
        raise ValueError("benchmark joined counts are inconsistent")
    if not isinstance(counts.get("landing_gate_mandatory_seams"), int) \
            or counts["landing_gate_mandatory_seams"] < 50:
        raise ValueError("landing-gate seam count is implausibly small")
    if len(value.get("build_identity", "")) != 64:
        raise ValueError("missing joined build identity")
    if len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("missing evidence generator identity")
    bindings = value.get("evidence_bindings", [])
    if len(bindings) != 19 or sum(row.get("current_build_bound") is True for row in bindings) != 19 \
            or any(len(row.get("subject_build_identity", "")) != 64 for row in bindings):
        raise ValueError("evidence bindings were promoted or lost")
    host_receipt = value.get("host_test_receipt", {})
    if host_receipt.get("current_build_bound") is not True \
            or host_receipt.get("subject_build_identity") != value.get("build_identity"):
        raise ValueError("host-test receipt is not current-build bound")


def build() -> dict:
    documents = {path: load(path) for path in INPUTS}
    identity = documents["build-identity.json"]["identity_sha256"]
    source_snapshot = documents["docs/receipts/source-snapshot-2026-08-24.json"]
    manifest = documents["app-manifest.json"]
    artifact = documents["artifact-registry.json"]
    app = documents["app-evidence.json"]
    init = documents["init-registry.json"]
    dependency = documents["dependency-lock.json"]
    dependency_archives = documents["docs/receipts/dependency-archives-host-2026-08-29.json"]
    wrappers = documents["wrapper-registry.json"]
    toolchain = documents["toolchain-manifest.json"]
    build_graph = documents["build-graph.json"]
    address = documents["address-space-registry.json"]
    license_registry = documents["license-registry.json"]
    adversarial = documents["adversarial-registry.json"]
    test_inventory = documents["test-inventory.json"]
    tests = documents["tests/host/test-run-receipt.json"]
    reproducible = documents["docs/receipts/reproducible-build-2026-08-22.json"]
    benchmark = documents["docs/receipts/benchmark-host-2026-08-23.json"]
    build_benchmark = documents["docs/receipts/build-benchmark-host-2026-08-29.json"]
    performance = documents["performance-registry.json"]
    page_table = documents["docs/receipts/page-table-native-uefi64-qemu-2026-08-29.json"]
    pmm = documents["docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json"]
    hardware = documents["hardware-receipt-plan.json"]
    crash = documents["docs/receipts/cpu-fault-invalid-opcode-qemu-2026-08-23.json"]
    crash64 = documents["docs/receipts/cpu-fault-native-uefi64-qemu-2026-08-29.json"]
    crash_gp = documents["docs/receipts/cpu-fault-general-protection-native-uefi64-qemu-2026-08-29.json"]
    crash_df = documents["docs/receipts/cpu-fault-double-fault-native-uefi64-qemu-2026-08-29.json"]
    visual_receipt = documents["docs/receipts/visual-qemu-2026-08-29.json"]
    visual_goldens = documents["visual-golden-registry.json"]
    visual = documents["visual-registry.json"]
    accessibility = documents["accessibility-registry.json"]
    security = documents["security-registry.json"]
    decisions = documents["decision-ledger.json"]
    release_notes = documents["release-notes.json"]
    provenance = documents["provenance-viewer.json"]
    event_receipt = documents["docs/receipts/event-trace-host-2026-08-24.json"]
    event_schema = documents["event-schema.json"]
    observability = documents["observability-registry.json"]
    build_input_count = len(documents["build-identity.json"].get("source_files_sha256", {}))

    current_identities = (
        source_snapshot.get("build_identity"),
        dependency.get("build_identity"),
        dependency_archives.get("build_identity"),
        wrappers.get("build_identity"),
        toolchain.get("build_identity"),
        build_graph.get("build_identity"),
        address.get("build_identity"),
        license_registry.get("build_identity"),
        visual.get("build_identity"),
        accessibility.get("build_identity"),
        security.get("build_identity"),
        decisions.get("build_identity"),
        release_notes.get("build_identity"),
        provenance.get("build_identity"),
        performance.get("build_identity"),
        hardware.get("build_identity"),
        visual_receipt.get("build_identity"),
        visual_goldens.get("build_identity"),
        build_benchmark.get("build_identity"),
        page_table.get("build_identity"),
        pmm.get("build_identity"),
        event_schema.get("build_identity"),
        observability.get("build_identity"),
    )
    if any(item != identity for item in current_identities):
        raise ValueError("current registry build identities disagree")
    evidence_inputs = {
        "kernel/metadata/artifact-registry.json": artifact.get("build_identity", {}).get("id"),
        "kernel/metadata/hardware-receipt-plan.json": hardware.get("build_identity"),
        "kernel/docs/receipts/visual-qemu-2026-08-29.json": visual_receipt.get("build_identity"),
        "kernel/metadata/visual-golden-registry.json": visual_goldens.get("build_identity"),
        "kernel/metadata/app-evidence.json": app.get("shipped_build_identity", {}).get("id"),
        "kernel/metadata/init-registry.json": init.get("build_identity"),
        "kernel/metadata/adversarial-registry.json": adversarial.get("build_identity"),
        "kernel/docs/receipts/reproducible-build-2026-08-22.json": reproducible.get("build_identity", {}).get("id"),
        "kernel/docs/receipts/benchmark-host-2026-08-23.json": benchmark.get("build_identity"),
        "kernel/docs/receipts/build-benchmark-host-2026-08-29.json": build_benchmark.get("build_identity"),
        "kernel/metadata/performance-registry.json": performance.get("build_identity"),
        "kernel/docs/receipts/page-table-native-uefi64-qemu-2026-08-29.json": page_table.get("build_identity"),
        "kernel/docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json": pmm.get("build_identity"),
        "kernel/docs/receipts/cpu-fault-invalid-opcode-qemu-2026-08-23.json": crash.get("build_identity"),
        "kernel/docs/receipts/cpu-fault-native-uefi64-qemu-2026-08-29.json": crash64.get("build_identity"),
        "kernel/docs/receipts/cpu-fault-general-protection-native-uefi64-qemu-2026-08-29.json": crash_gp.get("build_identity"),
        "kernel/docs/receipts/cpu-fault-double-fault-native-uefi64-qemu-2026-08-29.json": crash_df.get("build_identity"),
        "kernel/docs/receipts/event-trace-host-2026-08-24.json": event_receipt.get("build_identity"),
        "kernel/tests/host/test-run-receipt.json": tests.get("build_identity"),
    }
    if any(len(item or "") != 64 for item in evidence_inputs.values()) \
            or any(subject != identity for subject in evidence_inputs.values()):
        raise ValueError("evidence identity boundary is invalid")
    if source_snapshot.get("result") != "PASS_WITH_OPEN_CUSTODY_GAP" \
            or source_snapshot.get("counts", {}).get("archived_inputs") != build_input_count \
            or source_snapshot.get("open_gaps", {}).get("off_host_copies") != 0:
        raise ValueError("source snapshot is missing or overpromoted")
    application_count = len(manifest.get("entries", []))
    if artifact.get("result") != "PASS" \
            or app.get("counts", {}).get("identities") != application_count \
            or app.get("counts", {}).get("with_qemu_open_ready_close") != application_count:
        raise ValueError("current artifact/app evidence is not passing")
    if init.get("result") != "PASS" \
            or dependency.get("result") != "PASS_WITH_OPEN_SUPPLY_GAPS" \
            or dependency.get("closure", {}).get("all_package_dependencies_resolved") is not True \
            or dependency.get("closure", {}).get("all_binary_source_relationships_resolved") is not True \
            or dependency.get("closure", {}).get("all_source_archives_retained") is not False:
        raise ValueError("init/dependency evidence is not passing")
    archive_counts = dependency_archives.get("counts", {})
    if dependency_archives.get("result") != "PASS_LOCAL_OFFLINE_ARCHIVE" \
            or dependency_archives.get("dependency_lock_sha256") != sha256(input_path("dependency-lock.json")) \
            or archive_counts.get("binary_archives") != len(dependency.get("packages", [])) \
            or archive_counts.get("source_packages") != len(dependency.get("source_packages", [])) \
            or archive_counts.get("packages_with_source_archives") != len(dependency.get("packages", [])) \
            or archive_counts.get("undeclared_dependency_edges") != 0 \
            or dependency_archives.get("offline_resolution", {}).get("network_used") is not False:
        raise ValueError("dependency archive evidence is missing or overpromoted")
    wrapper_rows = wrappers.get("wrappers", [])
    wrapper_counts = wrappers.get("counts", {})
    if wrappers.get("result") != "PASS_INVENTORY_WITH_LEGACY_POLICY_GAPS" \
            or not wrapper_rows \
            or wrapper_counts.get("wrappers") != len(wrapper_rows) \
            or wrapper_counts.get("named_by_landing_gate") != sum(
                row.get("named_by_landing_gate") is True for row in wrapper_rows
            ) \
            or wrapper_counts.get("no_static_failure_policy") != sum(
                row.get("exit_policy") in {
                    "NO_STATIC_FAILURE_POLICY", "PYTHON_UNPROVED_TOP_LEVEL", "UNKNOWN"
                }
                for row in wrapper_rows
            ) \
            or wrappers.get("authority_contract", {}).get("legacy_policy_gaps_are_not_landing_authority") is not True:
        raise ValueError("wrapper inventory is missing or overpromoted")
    if toolchain.get("result") != "PASS_WITH_OPEN_PORTABILITY_GAPS" \
            or toolchain.get("counts") != {"tools": 7, "target_lanes": 4, "external_headers": 82}:
        raise ValueError("toolchain manifest is missing or overpromoted")
    if build_graph.get("result") != "PASS_CURRENT_ARTIFACTS" \
            or build_graph.get("counts", {}).get("source_inputs") != build_input_count \
            or build_graph.get("counts", {}).get("orphan_source_inputs") != 0 \
            or not isinstance(build_graph.get("counts", {}).get("scope_only_inputs"), int) \
            or build_graph.get("counts", {}).get("scope_only_inputs") <= 0 \
            or build_graph.get("artifact_snapshot", {}).get("current_build_bound") is not True:
        raise ValueError("build graph is missing or overpromoted")
    if address.get("result") != \
            "PASS_CURRENT_GENERATED_NON_OVERLAP_WITH_DYNAMIC_USER_TEMPLATE" \
            or address.get("counts") != {
                "physical_regions": 19,
                "user_template_regions": 6,
                "source_assertions": 24,
                "categories": 8,
            } \
            or set(address.get("categories", [])) != {
                "kernel", "user", "device", "stack", "heap", "shared", "guard", "dynamic"
            } \
            or len(address.get("known_gaps", [])) != 4 \
            or not any("rollback" in gap for gap in address.get("known_gaps", [])) \
            or not any("physical" in gap for gap in address.get("known_gaps", [])):
        raise ValueError("address-space registry is missing or overpromoted")
    if license_registry.get("result") != "PASS_WITH_RELEASE_BLOCK" \
            or license_registry.get("public_release_blocked") is not True:
        raise ValueError("license release block disappeared")
    inventory_targets = test_inventory.get("targets", [])
    receipt_results = tests.get("results", [])
    if tests.get("outcome") != "PASS" or tests.get("counts", {}).get("failed") != 0 \
            or tests.get("build_identity") != identity \
            or tests.get("inventory_sha256") != sha256(input_path("test-inventory.json")) \
            or tests.get("runner_sha256") != sha256(KERNEL_ROOT / "tools/run/run-host-tests.py") \
            or [(row.get("id"), row.get("name")) for row in receipt_results] != [
                (row.get("id"), row.get("name")) for row in inventory_targets
            ]:
        raise ValueError("host-test receipt is not passing")
    if adversarial.get("result") != "PASS_WITH_OPEN_GAPS" \
            or adversarial.get("counts", {}).get("verifier_canaries_caught") != 23 \
            or adversarial.get("counts", {}).get("hostile_host_passed") != 7 \
            or len(adversarial.get("open_gaps", {}).get("hostile_corpus", [])) != 2:
        raise ValueError("adversarial evidence is missing or overpromoted")
    if benchmark.get("result") not in ("PASS", "PASS_WITH_OPEN_REGRESSIONS") \
            or benchmark.get("counts", {}).get("within_budget", -1) \
            + benchmark.get("counts", {}).get("over_budget", -1) != 7:
        raise ValueError("benchmark result/counts drifted")
    if build_benchmark.get("result") != "PASS_WITH_OPEN_REGRESSION" \
            or build_benchmark.get("within_budget") is not False \
            or build_benchmark.get("sample_count") != 7:
        raise ValueError("host-build benchmark regression disappeared or drifted")
    performance_rows = performance.get("categories", [])
    failed_performance = [row.get("category") for row in performance_rows
                          if row.get("within_budget") is False]
    performance_counts = performance.get("counts", {})
    if performance.get("result") != "OPEN_REGRESSIONS_AND_GAPS" \
            or len(performance_rows) != 7 \
            or performance_counts.get("categories") != 7 \
            or performance_counts.get("host_budget_passed") \
            + performance_counts.get("host_budget_failed") != 7 \
            or performance_counts.get("host_budget_failed") != len(failed_performance) \
            or performance_counts.get("missing") != 0 \
            or failed_performance != performance.get("open_regressions") \
            or "build" not in failed_performance \
            or performance.get("open_gaps") != ["product-build", "product-latency", "native-target", "physical"]:
        raise ValueError("performance registry is missing or overpromoted")
    page_table_assertions = page_table.get("assertions", [])
    if page_table.get("result") != \
            "PASS_TRANSACTION_CORE_AND_NATIVE_HEAP_WINDOW_WITH_OPEN_GAPS" \
            or page_table.get("route") != "native-uefi64" \
            or page_table.get("host_receipt", {}).get("sha256") != sha256(
                input_path("tests/host/test-run-receipt.json")) \
            or page_table.get("host_receipt", {}).get("target") != "pagetxntest" \
            or len(page_table_assertions) != 3 \
            or page_table_assertions[0].get("entries") != 512 \
            or page_table_assertions[0].get("nth_write_failures") != 512 \
            or page_table_assertions[0].get("exact_rollback") is not True \
            or page_table_assertions[1].get("alias_probe_passed_before_commit") is not True \
            or page_table_assertions[2].get("transaction_committed") is not True \
            or len(page_table.get("known_gaps", [])) != 6:
        raise ValueError("page-table transaction evidence is missing or overpromoted")
    pmm_assertions = pmm.get("assertions", [])
    if pmm.get("result") != "PASS_BOUNDED_TYPED_ALLOCATOR_WITH_OPEN_GAPS" \
            or pmm.get("route") != "native-uefi64" \
            or pmm.get("host_receipt", {}).get("sha256") != sha256(
                input_path("tests/host/test-run-receipt.json")) \
            or pmm.get("host_receipt", {}).get("target") != "pmmtest" \
            or pmm.get("host_receipt", {}).get("checks", 0) < 80 \
            or [row.get("id") for row in pmm_assertions] != [
                "bounded-firmware-map-admission", "typed-owner-release",
                "exact-owner-accounting-and-quota", "zero-reuse-and-exhaustion",
                "typed-process-and-anonymous-consumer"] \
            or pmm_assertions[0].get("managed_floor_bytes") != 320 * 1024 * 1024 \
            or pmm_assertions[0].get("managed_limit_bytes") != 1024 * 1024 * 1024 \
            or pmm_assertions[0].get("free_pages_after_selftest") != \
                pmm_assertions[0].get("admitted_pages") \
            or pmm_assertions[1].get("wrong_owner_refused_without_mutation") is not True \
            or pmm_assertions[2].get("host_exhaustion_observed") is not True \
            or pmm_assertions[4].get("frames_per_process") != 8 \
            or pmm_assertions[4].get("two_processes_disjoint") is not True \
            or pmm_assertions[4].get("failure_atomic_acquire") is not True \
            or pmm_assertions[4].get("target_baseline_restored") is not True \
            or len(pmm.get("known_gaps", [])) != 7:
        raise ValueError("physical allocator evidence is missing or overpromoted")
    if hardware.get("result") != "READY_FOR_PHYSICAL_EXECUTION_WITHOUT_RECEIPTS" \
            or hardware.get("physical_status") != "NOT_RUN" \
            or hardware.get("counts") != {
                "matrix_cases": 6,
                "validated_receipts": 0,
                "physically_passed_routes": 0,
                "physically_passed_artifacts": 0,
                "required_scenarios_per_case": 7,
                "required_evidence_kinds_per_case": 10,
            }:
        raise ValueError("hardware receipt plan is missing or physically overpromoted")
    crash_specs = (
        (crash, "ud2", 6, 0, 0, 32, 0x00FF, True, False),
        (crash64, "ud2", 6, 0, 0, 64, 0xFFFF, True, False),
        (crash_gp, "gp", 13, 1, 0x38, 64, 0xFFFF, True, False),
        (crash_df, "double-fault", 8, 1, 0, 64, 0xFFFF, False, True),
    )
    for receipt, fault_case, vector, has_error, error, bits, mask, bind_ip, emergency in crash_specs:
        record = receipt.get("record", {})
        if receipt.get("result") != "PASS" or receipt.get("build_identity") != identity \
                or receipt.get("fault_case") != fault_case \
                or record.get("vector") != vector \
                or record.get("has_error") != has_error \
                or record.get("error") != error \
                or record.get("version") != 3 \
                or record.get("bytes") != 240 or record.get("bits") != bits \
                or record.get("register_mask") != mask \
                or record.get("register_sp") != record.get("sp") \
                or not record.get("handler_sp") \
                or receipt.get("guest_halted_after_record") is not True:
            raise ValueError("QEMU crash evidence is missing or overpromoted")
        if bind_ip and (receipt.get("kernel_symbol_ip") != record.get("ip") \
                or receipt.get("runtime_symbol_binding", {}).get("address") != record.get("ip")):
            raise ValueError("QEMU crash runtime-IP binding was lost")
        if not bind_ip and (receipt.get("kernel_symbol_ip") is not None \
                or receipt.get("runtime_symbol_binding", {}).get("address") is not None):
            raise ValueError("double-fault evidence invented an undefined IP binding")
        if emergency:
            low = record.get("emergency_stack_low", 0)
            high = record.get("emergency_stack_high", 0)
            halted_rsp = receipt.get("halted_cpu", {}).get("rsp", 0)
            if not (low < record["handler_sp"] < high and low < halted_rsp < high):
                raise ValueError("double-fault evidence does not prove emergency-stack use")
    if visual_receipt.get("result") != "PASS_CURRENT_ARTIFACT_SCREENSHOTS" \
            or visual_receipt.get("counts") != {
                "routes": 2, "captures": 4, "states": 2, "blank_captures": 0,
            }:
        raise ValueError("current visual receipt is missing or overpromoted")
    if visual_goldens.get("result") != "PASS_CURRENT_STRICT_GOLDENS_WITH_OPEN_MATRIX" \
            or visual_goldens.get("counts", {}).get("goldens") != 4 \
            or visual_goldens.get("counts", {}).get("dynamic_masks") != 4 \
            or visual_goldens.get("counts", {}).get("open_variant_dimensions") != 6:
        raise ValueError("current visual golden evidence is missing or overpromoted")
    if visual.get("result") != "PARTIAL_CURRENT_VISUAL_EVIDENCE" \
            or visual.get("counts", {}).get("current_build_bound") != 4 \
            or visual.get("counts", {}).get("current_routes") != 2 \
            or visual.get("counts", {}).get("current_states") != 2:
        raise ValueError("visual evidence was overpromoted or drifted")
    if accessibility.get("result") != "PASS_WITH_OPEN_GAPS" \
            or accessibility.get("counts", {}).get("missing") != 9:
        raise ValueError("accessibility gaps disappeared or drifted")
    if security.get("result") != "PASS_WITH_MIXED_CURRENT_AND_HISTORICAL_EVIDENCE_AND_OPEN_GAPS" \
            or security.get("counts", {}).get("production_complete") != 0:
        raise ValueError("security evidence was overpromoted or drifted")
    if decisions.get("result") != "PASS_WITH_OPEN_GAPS" \
            or decisions.get("coverage", {}).get("normalized_records") != 20 \
            or decisions.get("coverage", {}).get("legacy_labels_indexed") != 47 \
            or decisions.get("coverage", {}).get("legacy_labels_remaining") != 0 \
            or decisions.get("counts", {}).get("status", {}).get("SUPERSEDED") != 3:
        raise ValueError("decision ledger is missing, incomplete or overpromoted")
    if release_notes.get("result") != "PASS_UNRELEASED_WITH_BLOCKERS" \
            or release_notes.get("release", {}).get("state") != "UNRELEASED_DEVELOPMENT" \
            or release_notes.get("release", {}).get("public_release_blocked") is not True \
            or release_notes.get("counts", {}).get("unreleased_change_candidates") != 17 \
            or release_notes.get("counts", {}).get("published_entries") != 0 \
            or release_notes.get("counts", {}).get("known_issues") != 12:
        raise ValueError("release notes are missing, stale or overpromoted")
    if provenance.get("result") != "PASS_STATIC_VIEWER_WITH_OPEN_GAPS" \
            or provenance.get("viewer", {}).get("external_requests") != 0 \
            or provenance.get("counts", {}).get("applications") != application_count \
            or provenance.get("counts", {}).get("applications_with_declared_permission_grants") != 0 \
            or provenance.get("counts", {}).get("cryptographic_signatures") != 0:
        raise ValueError("provenance viewer is missing, stale or overpromoted")
    if event_receipt.get("result") != "PASS_HOST_CORE_TARGET_UNINTEGRATED" \
            or event_receipt.get("proof", {}).get("host_execution", {}).get("checks") != 37 \
            or event_receipt.get("target_integration", {}).get("target_emitters") != 0:
        raise ValueError("event-trace receipt is missing or overpromoted")
    if any(len(event_receipt.get("tools", {}).get(name, {}).get("binary_sha256", "")) != 64
           for name in ("gcc", "clang")):
        raise ValueError("historical event-trace receipt has no compiler identities")
    if event_schema.get("result") != "PASS_CURRENT_SCHEMA_WITH_CURRENT_HOST_PROOF_TARGET_UNINTEGRATED" \
            or event_schema.get("counts", {}).get("wire_fields") != 28 \
            or event_schema.get("counts", {}).get("target_emitters") != 0:
        raise ValueError("event schema is missing or overpromoted")
    if observability.get("result") != "PASS_WITH_CURRENT_QEMU_AND_HOST_SCHEMA_EVIDENCE_AND_OPEN_GAPS" \
            or observability.get("counts", {}).get("durable_crash_receipts") != 0:
        raise ValueError("observability evidence was overpromoted or drifted")
    if any(row.get("physical_hardware") != "UNVERIFIED_FOR_EXACT_HASH"
           for row in artifact.get("artifacts", {}).values()):
        raise ValueError("artifact registry contains a physical overclaim")

    input_rows = []
    for path in INPUTS:
        document = documents[path]
        input_rows.append({
            "path": input_path(path).relative_to(ROOT).as_posix(),
            "sha256": sha256(input_path(path)),
            "schema": document.get("schema"),
            "result": document.get("result", document.get("outcome", "IDENTITY_ONLY")),
            "evidence_ceiling": document.get("evidence_ceiling", "see named registry"),
        })
    host = tests["counts"]
    counts = {
        "reproducible_artifacts": len(artifact["artifacts"]),
        "source_snapshot_inputs": source_snapshot["counts"]["archived_inputs"],
        "source_snapshot_off_host_copies": source_snapshot["open_gaps"]["off_host_copies"],
        "qemu_boot_routes": len(artifact["boot_routes"]),
        "app_identities": app["counts"]["identities"],
        "app_lifecycle_proved": app["counts"]["with_qemu_open_ready_close"],
        "init_stages": len(init["stages"]),
        "dependency_commands": len(dependency["commands"]),
        "dependency_firmware_blobs": len(dependency["firmware"]),
        "dependency_runtime_files": dependency["closure"]["runtime_files"],
        "dependency_transitive_packages": dependency["closure"]["transitive_packages"],
        "dependency_source_archives_retained": dependency["closure"]["source_archives_retained"],
        "dependency_binary_archives_retained": dependency_archives["counts"]["binary_archives"],
        "dependency_source_packages_retained": dependency_archives["counts"]["source_packages"],
        "dependency_packages_with_source_archives": dependency_archives["counts"]["packages_with_source_archives"],
        "dependency_unindexed_binary_archives": dependency_archives["counts"]["unindexed_binary_archives"],
        "dependency_unindexed_source_packages": dependency_archives["counts"]["unindexed_source_packages"],
        "wrapper_inventory": wrappers["counts"]["wrappers"],
        "wrapper_named_by_landing_gate": wrappers["counts"]["named_by_landing_gate"],
        "wrapper_legacy_policy_gaps": wrappers["counts"]["no_static_failure_policy"],
        "toolchain_tools": toolchain["counts"]["tools"],
        "toolchain_target_lanes": toolchain["counts"]["target_lanes"],
        "toolchain_external_headers": toolchain["counts"]["external_headers"],
        "toolchain_hermetic_builds": 0,
        "build_graph_source_inputs": build_graph["counts"]["source_inputs"],
        "build_graph_target_lanes": build_graph["counts"]["target_lanes"],
        "build_graph_artifacts": build_graph["counts"]["artifacts"],
        "build_graph_orphan_inputs": build_graph["counts"]["orphan_source_inputs"],
        "build_graph_scope_only_inputs": build_graph["counts"]["scope_only_inputs"],
        "address_physical_regions": address["counts"]["physical_regions"],
        "address_user_template_regions": address["counts"]["user_template_regions"],
        "address_source_assertions": address["counts"]["source_assertions"],
        "address_categories": address["counts"]["categories"],
        "page_table_transaction_receipts": 1,
        "physical_page_allocator_receipts": 1,
        "license_build_inputs": license_registry["counts"]["build_inputs"],
        "license_files": license_registry["counts"]["declared_license_files"],
        "host_targets": host["targets"],
        "host_commands_executed": host["commands_executed"],
        "host_passed": host["passed"],
        "host_failed": host["failed"],
        "host_hardware_skips": host["skipped-hardware"],
        "host_not_run": host["not-run"],
        "physical_exact_hash_proofs": 0,
        "hardware_matrix_cases": hardware["counts"]["matrix_cases"],
        "hardware_validated_receipts": hardware["counts"]["validated_receipts"],
        "hardware_physically_passed_routes": hardware["counts"]["physically_passed_routes"],
        "verifier_canaries_caught": adversarial["counts"]["verifier_canaries_caught"],
        "landing_gate_mandatory_seams": landing_gate_seam_count(),
        "failure_seam_complete": adversarial["counts"]["failure_seam_complete"],
        "hostile_host_passed": adversarial["counts"]["hostile_host_passed"],
        "benchmark_measurements": benchmark["counts"]["measurements"],
        "benchmark_within_budget": benchmark["counts"]["within_budget"],
        "benchmark_over_budget": benchmark["counts"]["over_budget"],
        "benchmark_native_target_measurements": benchmark["counts"]["native_target_measurements"],
        "performance_categories": performance["counts"]["categories"],
        "performance_host_budget_passed": performance["counts"]["host_budget_passed"],
        "performance_host_budget_failed": performance["counts"]["host_budget_failed"],
        "performance_missing_categories": performance["counts"]["missing"],
        "visual_assets": visual["counts"]["assets"],
        "visual_current_build_bound": visual["counts"]["current_build_bound"],
        "visual_current_routes": visual["counts"]["current_routes"],
        "visual_current_states": visual["counts"]["current_states"],
        "visual_strict_goldens": visual["counts"]["strict_current_goldens"],
        "visual_dynamic_masks": visual["counts"]["declared_dynamic_masks"],
        "visual_variant_dimensions_complete": visual["counts"]["variant_dimensions_complete"],
        "visual_variant_dimensions_open": visual["counts"]["variant_dimensions_open"],
        "accessibility_capabilities": accessibility["counts"]["capabilities"],
        "accessibility_host_passed": accessibility["counts"]["host_passed"],
        "accessibility_partial": accessibility["counts"]["partial_host_passed"],
        "accessibility_missing": accessibility["counts"]["missing"],
        "accessibility_complete_target_workflows": accessibility["counts"]["complete_target_workflows"],
        "security_claims": security["counts"]["claims"],
        "security_static_proved": security["counts"]["static_proved"],
        "security_host_proved_limited": security["counts"]["host_proved_limited"],
        "security_qemu_partial": security["counts"]["qemu_boot_reachable_partial"],
        "security_missing": security["counts"]["missing"],
        "security_production_complete": security["counts"]["production_complete"],
        "decision_records": decisions["coverage"]["normalized_records"],
        "decision_legacy_labels_indexed": decisions["coverage"]["legacy_labels_indexed"],
        "decision_legacy_labels_normalized": decisions["coverage"]["legacy_labels_semantically_normalized"],
        "decision_legacy_labels_remaining": decisions["coverage"]["legacy_labels_remaining"],
        "decision_superseded_records": decisions["counts"]["status"]["SUPERSEDED"],
        "release_change_candidates": release_notes["counts"]["unreleased_change_candidates"],
        "release_user_visible_candidates": release_notes["counts"]["user_visible_candidates"],
        "release_published_entries": release_notes["counts"]["published_entries"],
        "release_migrations": release_notes["counts"]["migrations"],
        "release_known_issues": release_notes["counts"]["known_issues"],
        "release_recovery_paths": release_notes["counts"]["recovery_paths"],
        "release_available_recovery_paths": release_notes["counts"]["available_recovery_paths"],
        "provenance_inputs": provenance["counts"]["inputs"],
        "provenance_health_areas": provenance["counts"]["health_areas"],
        "provenance_artifacts": provenance["counts"]["artifacts"],
        "provenance_applications": provenance["counts"]["applications"],
        "provenance_app_permission_grants": provenance["counts"]["applications_with_declared_permission_grants"],
        "provenance_security_claims": provenance["counts"]["security_claims"],
        "provenance_change_candidates": provenance["counts"]["change_candidates"],
        "provenance_cryptographic_signatures": provenance["counts"]["cryptographic_signatures"],
        "observability_capabilities": observability["counts"]["capabilities"],
        "observability_qemu_hash_only": observability["counts"]["qemu_hash_only"],
        "observability_qemu_proved": observability["counts"]["qemu_proved"],
        "observability_qemu_proved_partial": observability["counts"]["qemu_proved_partial"],
        "observability_host_limited": observability["counts"]["host_proved_limited"],
        "observability_host_core": observability["counts"]["host_proved_core"],
        "observability_host_partial": observability["counts"]["host_proved_partial"],
        "observability_source_only": observability["counts"]["source_only"],
        "observability_missing": observability["counts"]["missing"],
        "qemu_crash_receipts": observability["counts"]["qemu_crash_receipts"],
        "durable_crash_receipts": observability["counts"]["durable_crash_receipts"],
        "typed_structured_event_fields": observability["counts"]["typed_structured_event_fields"],
        "event_trace_host_checks": event_receipt["proof"]["host_execution"]["checks"],
        "event_trace_compile_lanes": len(event_receipt["proof"]["compile_lanes"]),
        "event_trace_target_emitters": event_schema["counts"]["target_emitters"],
        "current_build_bound_artifacts": len(artifact["artifacts"]),
        "historical_artifacts": 0,
        "current_build_bound_qemu_routes": len(artifact["boot_routes"]),
        "historical_qemu_routes": 0,
        "current_build_bound_host_receipts": 6,
    }
    value = {
        "schema": "zlos.evidence-registry.v1",
        "result": "PASS_CURRENT_INDEX_WITH_MIXED_EXECUTION_EVIDENCE_AND_OPEN_GAPS",
        "build_identity": identity,
        "inputs": input_rows,
        "evidence_bindings": [{
            "path": path,
            "subject_build_identity": subject,
            "current_build_bound": subject == identity,
            "evidence_ceiling": ("current build-bound evidence" if subject == identity else
                                 "dated evidence for its named subject build only"),
        } for path, subject in evidence_inputs.items()],
        "host_test_receipt": {
            "path": "kernel/tests/host/test-run-receipt.json",
            "inventory_path": "kernel/metadata/test-inventory.json",
            "inventory_sha256": tests.get("inventory_sha256"),
            "runner_sha256": tests.get("runner_sha256"),
            "subject_head": tests.get("git", {}).get("head"),
            "subject_build_identity": tests.get("build_identity"),
            "current_build_bound": tests.get("build_identity") == identity,
            "evidence_ceiling": tests.get("evidence_ceiling"),
        },
        "counts": counts,
        "open_gaps": {
            "physical_exact_hash_artifacts": len(artifact["artifacts"]),
            "hardware_matrix_cases_without_receipts": (
                hardware["counts"]["matrix_cases"]
                - hardware["counts"]["physically_passed_routes"]
            ),
            "host_hardware_skips": host["skipped-hardware"],
            "host_non_runs": host["not-run"],
            "inputs_without_redistribution_grant": (
                license_registry["counts"]["build_inputs"]
                - license_registry["counts"]["inputs_with_established_redistribution_grant"]
            ),
            "public_release_blocked": license_registry["public_release_blocked"],
            "source_snapshot_off_host_missing": source_snapshot["open_gaps"]["off_host_copies"] == 0,
            "source_snapshot_signature_missing": not source_snapshot["open_gaps"]["signed_attestation"],
            "whole_repository_snapshot_missing": not source_snapshot["open_gaps"]["whole_repository_snapshot"],
            "hermetic_toolchain_missing": True,
            "signed_toolchain_attestation_missing": True,
            "toolchain_source_archive_missing": True,
            "dependency_offline_rebuild_missing": not dependency["closure"]["offline_rebuild_proved"],
            "dependency_offline_resolution_missing": not all(
                dependency_archives["offline_resolution"].get(name) is expected
                for name, expected in {
                    "network_used": False,
                    "all_seed_packages_reachable": True,
                    "all_dependency_edges_declared": True,
                    "all_binary_archives_present": True,
                    "all_source_archives_present": True,
                }.items()
            ),
            "dependency_source_archives_missing": (
                dependency["closure"]["transitive_packages"]
                - dependency_archives["counts"]["packages_with_source_archives"]
            ),
            "dependency_unindexed_archives": (
                dependency_archives["counts"]["unindexed_binary_archives"]
                + dependency_archives["counts"]["unindexed_source_packages"]
            ),
            "per_object_provenance_receipts_missing": True,
            "build_graph_scope_only_inputs": build_graph["counts"]["scope_only_inputs"],
            "future_build_graph_outputs_missing": False,
            "address_low_shared_internal_manifest_missing": True,
            "address_dynamic_user_base_runtime_only": True,
            "address_page_table_transaction_coverage_incomplete": True,
            "address_physical_execution_missing": True,
            "failure_injection_open_families": len(adversarial["open_gaps"]["failure_injection"]),
            "hostile_corpus_open_families": len(adversarial["open_gaps"]["hostile_corpus"]),
            "performance_over_budget": performance["counts"]["host_budget_failed"],
            "native_target_benchmark_missing": benchmark["counts"]["native_target_measurements"] == 0,
            "benchmark_percentiles_missing": benchmark["method"]["percentiles"] == "NOT_RECORDED",
            "performance_product_build_missing": "product-build" in performance["open_gaps"],
            "visual_unbound_assets": visual["counts"]["assets"] - visual["counts"]["current_build_bound"],
            "visual_variant_dimensions_open": visual["counts"]["variant_dimensions_open"],
            "accessibility_missing_capabilities": accessibility["counts"]["missing"],
            "accessibility_target_workflows_missing": accessibility["counts"]["complete_target_workflows"] == 0,
            "security_open_claims": len(security["open_claims"]),
            "security_production_complete_missing": security["counts"]["production_complete"] == 0,
            "decision_legacy_semantics_open": decisions["coverage"]["legacy_labels_remaining"],
            "decision_system_inventory_missing": not decisions["open_gaps"]["system_wide_source_inventory_complete"],
            "decision_independent_approval_missing": not decisions["open_gaps"]["signed_or_independently_approved"],
            "release_generation_missing": not release_notes["open_gaps"]["released_generation_exists"],
            "release_publication_blocked": not release_notes["open_gaps"]["public_release_authorized"],
            "release_signature_missing": not release_notes["open_gaps"]["signed_release_notes"],
            "release_published_entries_missing": release_notes["counts"]["published_entries"] == 0,
            "release_migration_inventory_incomplete": not release_notes["open_gaps"]["user_data_migration_inventory_complete"],
            "release_previous_rollback_missing": not release_notes["open_gaps"]["previous_release_rollback_artifact_available"],
            "provenance_runtime_route_missing": provenance["open_gaps"]["runtime_zlos_app_route_missing"],
            "provenance_current_screenshot_missing": provenance["open_gaps"]["current_screenshot_receipt_missing"],
            "provenance_permissions_missing": provenance["open_gaps"]["per_app_permission_grants_missing"],
            "provenance_signatures_missing": provenance["open_gaps"]["cryptographic_signatures_missing"],
            "provenance_live_health_missing": provenance["open_gaps"]["live_health_updates_missing"],
            "provenance_target_accessibility_missing": provenance["open_gaps"]["target_accessibility_workflow_missing"],
            "provenance_remote_auth_missing": provenance["open_gaps"]["authenticated_remote_serving_missing"],
            "provenance_public_release_missing": provenance["open_gaps"]["public_release_missing"],
            "observability_open_capabilities": len(observability["open_gaps"]),
            "durable_crash_receipt_missing": observability["counts"]["durable_crash_receipts"] == 0,
            "typed_event_target_integration_missing": event_schema["counts"]["target_emitters"] == 0,
            "current_artifact_snapshot_missing": False,
            "current_qemu_evidence_missing": False,
            "current_host_test_receipt_missing": tests.get("build_identity") != identity,
            "current_host_benchmark_missing": False,
        },
        "generator": {
            "path": "kernel/tools/generators/gen-evidence-registry.py",
            "sha256": sha256(Path(__file__).resolve()),
        },
        "evidence_ceiling": (
            "current source/tooling index joined to current artifacts, QEMU routes, host tests, host benchmark and event host proof; "
            "no physical-artifact, full-workflow or public-release promotion"
        ),
        "weakest_link": (
            "off-host signed source custody, physical exact-hash proof, hardware-skipped tests, "
            "non-run instruments/manual actions, and license grants remain open"
        ),
    }
    validate(value)
    return value


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["inputs"].pop()
    mutations["missing-registry"] = missing
    stale_input = copy.deepcopy(value)
    stale_input["inputs"][0]["sha256"] = "0" * 64
    mutations["stale-input-binding"] = stale_input
    physical = copy.deepcopy(value)
    physical["counts"]["physical_exact_hash_proofs"] = 1
    mutations["physical-overclaim"] = physical
    hardware = copy.deepcopy(value)
    hardware["open_gaps"]["hardware_matrix_cases_without_receipts"] = 0
    mutations["hidden-hardware-matrix-gap"] = hardware
    wrappers = copy.deepcopy(value)
    wrappers["counts"]["wrapper_inventory"] += 1
    mutations["wrapper-count-drift"] = wrappers
    skipped = copy.deepcopy(value)
    skipped["open_gaps"]["host_hardware_skips"] = 0
    mutations["hidden-hardware-skip"] = skipped
    release = copy.deepcopy(value)
    release["open_gaps"]["public_release_blocked"] = False
    mutations["hidden-release-block"] = release
    custody = copy.deepcopy(value)
    custody["open_gaps"]["source_snapshot_off_host_missing"] = False
    mutations["invented-source-custody"] = custody
    hermetic = copy.deepcopy(value)
    hermetic["open_gaps"]["hermetic_toolchain_missing"] = False
    mutations["invented-hermetic-toolchain"] = hermetic
    dependency = copy.deepcopy(value)
    dependency["open_gaps"]["dependency_offline_resolution_missing"] = True
    mutations["lost-offline-dependency-proof"] = dependency
    graph = copy.deepcopy(value)
    graph["open_gaps"]["build_graph_scope_only_inputs"] = 0
    mutations["hidden-build-graph-superset"] = graph
    address = copy.deepcopy(value)
    address["open_gaps"]["address_page_table_transaction_coverage_incomplete"] = False
    mutations["invented-address-transaction"] = address
    failure = copy.deepcopy(value)
    failure["open_gaps"]["failure_injection_open_families"] = 0
    mutations["hidden-failure-injection-gaps"] = failure
    hostile = copy.deepcopy(value)
    hostile["open_gaps"]["hostile_corpus_open_families"] = 0
    mutations["hidden-hostile-corpus-gaps"] = hostile
    performance = copy.deepcopy(value)
    if value["open_gaps"]["performance_over_budget"]:
        performance["open_gaps"]["performance_over_budget"] = 0
        mutations["hidden-performance-regression"] = performance
    else:
        performance["counts"]["benchmark_over_budget"] = 1
        mutations["invented-performance-regression"] = performance
    native = copy.deepcopy(value)
    native["open_gaps"]["native_target_benchmark_missing"] = False
    mutations["invented-native-benchmark"] = native
    performance = copy.deepcopy(value)
    performance["open_gaps"]["performance_product_build_missing"] = False
    mutations["hidden-performance-product-build-gap"] = performance
    visual = copy.deepcopy(value)
    visual["open_gaps"]["visual_unbound_assets"] = 0
    mutations["invented-current-visual-proof"] = visual
    accessibility = copy.deepcopy(value)
    accessibility["open_gaps"]["accessibility_missing_capabilities"] = 0
    mutations["hidden-accessibility-gaps"] = accessibility
    security = copy.deepcopy(value)
    security["open_gaps"]["security_open_claims"] = 0
    mutations["hidden-security-risks"] = security
    decisions = copy.deepcopy(value)
    decisions["open_gaps"]["decision_system_inventory_missing"] = False
    mutations["hidden-decision-history-gap"] = decisions
    release = copy.deepcopy(value)
    release["open_gaps"]["release_publication_blocked"] = False
    mutations["hidden-release-note-blocker"] = release
    provenance = copy.deepcopy(value)
    provenance["open_gaps"]["provenance_runtime_route_missing"] = False
    mutations["invented-provenance-runtime"] = provenance
    observability = copy.deepcopy(value)
    observability["open_gaps"]["observability_open_capabilities"] = 0
    mutations["hidden-observability-gaps"] = observability
    identity = copy.deepcopy(value)
    identity["build_identity"] = "short"
    mutations["missing-build-identity"] = identity
    binding = copy.deepcopy(value)
    binding["evidence_bindings"][0]["current_build_bound"] = False
    mutations["lost-current-artifact-proof"] = binding
    benchmark = copy.deepcopy(value)
    benchmark["evidence_bindings"][5]["current_build_bound"] = False
    mutations["lost-current-benchmark-proof"] = benchmark
    host_tests = copy.deepcopy(value)
    host_tests["host_test_receipt"]["current_build_bound"] = False
    mutations["lost-current-host-test-proof"] = host_tests
    caught = []
    for name, mutated in mutations.items():
        try:
            validate(mutated)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"evidence-registry selftest mutation escaped: {name}")
    print("evidence-registry selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    with tempfile.NamedTemporaryFile("w", dir=HERE, delete=False, encoding="utf-8") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, OUTPUT)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if not args.write and not args.check:
        args.check = True
    try:
        value = build()
        if args.selftest:
            selftest(value)
        if args.write:
            write_atomic(value)
        if args.check:
            if not OUTPUT.is_file() or load("evidence-registry.json") != value:
                raise ValueError("evidence-registry.json is missing or stale")
        print(
            f"evidence-registry: {value['result']}: "
            f"{value['counts']['reproducible_artifacts']} artifacts, "
            f"{value['counts']['qemu_boot_routes']} routes, "
            f"{value['counts']['app_lifecycle_proved']} apps"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"evidence-registry: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
