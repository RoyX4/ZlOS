#!/usr/bin/env python3
"""Validate and generate the self-contained zlOS implementation program.

The canonical 906-row research catalogue is retained under docs/program/research.
This tool freezes its rows into FEATURE-MAP.md and rejects coverage, identity,
phase, current-app/game and registry drift without requiring another checkout.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from collections import Counter
from pathlib import Path
from urllib.parse import unquote


ROOT = Path(__file__).resolve().parents[1]
PROGRAM = ROOT / "docs" / "program"
RESEARCH = PROGRAM / "research"
SOURCE = RESEARCH / "CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md"
OUTPUT = PROGRAM / "FEATURE-MAP.md"
CROSSWALK_OUTPUT = PROGRAM / "RESEARCH-CONTRACT-CROSSWALK.md"
BACKLOG_SOURCES = {
    "core": SOURCE.parent / "IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md",
    "driver-app": SOURCE.parent / "DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md",
    "visual": SOURCE.parent / "VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md",
}
REPOSITORY_SOURCES = {
    "manifest": RESEARCH / "SOURCE_SNAPSHOT_MANIFEST_2026-08-21.md",
    "feature-matrix": RESEARCH / "ALL_33_FEATURE_MATRIX_2026-08-21.md",
    "driver-app-matrix": RESEARCH / "ALL_33_DRIVERS_AND_APPS_MATRIX_2026-08-21.md",
    "visual-app-matrix": RESEARCH / "ALL_33_VISUAL_WEB_AND_APP_EXPERIENCE_MATRIX_2026-08-21.md",
}

EXPECTED_RESEARCH_DOCS = {
    "AGENTS.md",
    "ADDITIONAL_EXECUTABLE_EVIDENCE_2026-08-21.md",
    "ALL_33_DRIVERS_AND_APPS_MATRIX_2026-08-21.md",
    "ALL_33_FEATURE_MATRIX_2026-08-21.md",
    "ALL_33_VISUAL_WEB_AND_APP_EXPERIENCE_MATRIX_2026-08-21.md",
    "ARCHITECTURE_OS_AUDIT_2026-08-21.md",
    "ARCHITECTURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md",
    "ARCHITECTURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md",
    "ARCHITECTURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md",
    "CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md",
    "CANONICAL_FEATURE_IMPLEMENTATION_CATALOG_2026-08-21.md",
    "CHATGPT_VOICE_ON_LINUX_2026-08-21.md",
    "CLAUDE.md",
    "CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md",
    "CURRENT_GITHUB_REPOSITORY_PATTERNS_2026-08-21.md",
    "DRIVER_AND_APP_AUDIT_TAXONOMY_2026-08-21.md",
    "DRIVER_AND_APP_CLEAN_ROOM_INTEGRATION_PLAN_2026-08-21.md",
    "DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md",
    "FOCUSED_REFUTATION_AND_COVERAGE_2026-08-21.md",
    "FOCUSED_REPOSITORIES_AUDIT_2026-08-21.md",
    "FOCUSED_REPOSITORIES_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md",
    "FOCUSED_REPOSITORIES_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md",
    "IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md",
    "MASTER_RESEARCH_PLAN_AND_TAXONOMY_2026-08-21.md",
    "MATURE_OS_AUDIT_2026-08-21.md",
    "MATURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md",
    "MATURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md",
    "MATURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md",
    "OS_REPOSITORY_SURVEY_2026-08-20.md",
    "PROTOS_KERNEL_AUDIT_2026-08-21.md",
    "README.md",
    "RULES.md",
    "RESPONSIVENESS_RENDERING_AND_VISUAL_POLISH_DEEP_DIVE_2026-08-22.md",
    "SOURCE_SNAPSHOT_MANIFEST_2026-08-21.md",
    "SOP.md",
    "STARRED_REPOSITORY_MAP_2026-08-21.md",
    "STATUS.md",
    "TODO.md",
    "VALUES.md",
    "VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md",
    "ZLOS_CURRENT_DRIVER_AND_APP_BASELINE_2026-08-21.md",
    "ZLOS_CURRENT_VISUAL_WEB_AND_APP_EXPERIENCE_BASELINE_2026-08-21.md",
    "ZLOS_VISUAL_BROWSER_AND_APP_EXPERIENCE_CLEAN_ROOM_PLAN_2026-08-21.md",
    "language/HANDOFF.md",
    "language/AGENTS.md",
    "language/CLAUDE.md",
    "language/MASTER_PLAN.md",
    "language/README.md",
    "language/ROADMAP.md",
    "language/RULES.md",
    "language/SOP.md",
    "language/STATUS.md",
    "language/TODO.md",
    "language/ULTIMATE_PLAN.md",
    "language/VALUES.md",
}

EXPECTED_PREFIX_COUNTS = {
    "EV": 28,
    "BT": 30,
    "KR": 40,
    "AU": 40,
    "DV": 55,
    "FS": 40,
    "NW": 35,
    "GR": 50,
    "IN": 25,
    "AD": 25,
    "DS": 40,
    "AX": 35,
    "SH": 45,
    "AP": 40,
    "WB": 50,
    "PD": 43,
    "MD": 33,
    "ST": 37,
    "DE": 40,
    "CP": 20,
    "GM": 20,
    "AG": 30,
    "OP": 30,
    "ZL": 55,
    "FH": 20,
}

EXPECTED_REGISTRY_COUNTS = {
    "AGT": 24, "APP": 83, "BLK": 25, "BUS": 28, "CUR": 38, "DEV": 20,
    "FSP": 22, "FUT": 12, "GAME": 24, "GPU": 25, "INPUT": 14, "MEDIA": 8,
    "NIC": 16, "OPS": 28, "PERIPH": 4, "PLAT": 38, "PWR": 7, "RADIO": 5,
    "SENSOR": 4, "SVC": 116, "USB": 14, "VM": 12, "ZLP": 42,
}

# Primary phase only. More detailed cross-phase prerequisites live in PHASES.md
# and the specialized registries. Every prefix is one coherent product domain.
DOMAIN = {
    "EV": ("MP-00", "Evidence, provenance and product truth"),
    "BT": ("MP-02", "Boot, firmware and architecture"),
    "KR": ("MP-03", "Kernel, memory and execution"),
    "AU": ("MP-04", "Authority, identity, IPC and security"),
    "DV": ("MP-05", "Driver framework and device lifecycle"),
    "FS": ("MP-06", "Storage, filesystems, packages and recovery"),
    "NW": ("MP-08", "Networking and remote communication"),
    "GR": ("MP-07", "Display, rendering and compositor"),
    "IN": ("MP-07", "Input and interaction"),
    "AD": ("MP-09", "Audio and media devices"),
    "DS": ("MP-10", "Design system and visual identity"),
    "AX": ("MP-10", "Accessibility and localization"),
    "SH": ("MP-07", "Session, shell and desktop"),
    "AP": ("MP-11", "Application platform and common services"),
    "WB": ("MP-13", "Browser and web platform"),
    "PD": ("MP-14", "Files, documents, office and knowledge work"),
    "MD": ("MP-14", "Media, communication and personal apps"),
    "ST": ("MP-14", "Settings, administration and recovery UX"),
    "DE": ("MP-15", "Terminal, utilities and development"),
    "CP": ("MP-15", "Compatibility, ports and virtualization"),
    "GM": ("MP-12", "Games, demos and validation experiences"),
    "AG": ("MP-17", "Agents and automation"),
    "OP": ("MP-18", "Operations, release and support"),
    "ZL": ("MP-16", "zl language, compiler and self-hosting"),
    "FH": ("MP-19", "Future hardware and product expansion"),
}

FEATURE_RE = re.compile(
    r"^\| (?P<id>[A-Z]{2}-\d{3}) \| (?P<feature>[^|]+?) \| "
    r"(?P<state>Keep|Deepen|Add|Later|Fixture|Reject) \| "
    r"(?P<meaning>.+?) \|$"
)
PHASE_RE = re.compile(r"^## (MP-\d{2}) — ", re.MULTILINE)
REGISTRY_ID_RE = re.compile(
    r"^\| ((?:PLAT|BUS|BLK|USB|INPUT|GPU|NIC|RADIO|MEDIA|PWR|SENSOR|"
    r"PERIPH|VM|FSP|SVC|CUR|GAME|APP|ZLP|DEV|AGT|OPS|FUT)-\d{3}) \|",
    re.MULTILINE,
)
DEPENDENCY_ROW_RE = re.compile(r"^\| (MP-\d{2}) \| ([^|]+?) \|$", re.MULTILINE)
LINK_RE = re.compile(r"(?<!!)\[[^\]]+\]\(([^)]+)\)")
CONTRACT_RE = re.compile(r"^### ((?:P\d+\.\d+|DA-[A-Z0-9]+|VX-\d+)) — (.+)$", re.MULTILINE)
REPOSITORY_RE = re.compile(r"^\|\s*\d+\s*\|\s*`([^`]+/[^`]+)`\s*\|", re.MULTILINE)


def fail(message: str) -> None:
    raise ValueError(message)


def read(path: Path) -> str:
    if not path.is_file():
        fail(f"missing required file: {path}")
    return path.read_text(encoding="utf-8")


def parse_features(source_text: str) -> list[dict[str, str]]:
    features: list[dict[str, str]] = []
    for line in source_text.splitlines():
        match = FEATURE_RE.match(line)
        if match:
            row = {key: value.strip() for key, value in match.groupdict().items()}
            row["prefix"] = row["id"].split("-", 1)[0]
            features.append(row)
    return features


def validate_features(features: list[dict[str, str]]) -> None:
    ids = [row["id"] for row in features]
    if len(features) != 906:
        fail(f"feature count is {len(features)}, expected 906")
    duplicates = sorted(item for item, count in Counter(ids).items() if count != 1)
    if duplicates:
        fail(f"duplicate feature IDs: {', '.join(duplicates)}")
    counts = Counter(row["prefix"] for row in features)
    if dict(counts) != EXPECTED_PREFIX_COUNTS:
        fail(f"prefix counts differ: actual={dict(counts)!r}")
    unknown = sorted(set(counts) - set(DOMAIN))
    if unknown:
        fail(f"unmapped feature prefixes: {', '.join(unknown)}")


def validate_phases() -> None:
    phases_text = read(PROGRAM / "PHASES.md")
    found = PHASE_RE.findall(phases_text)
    expected = [f"MP-{index:02d}" for index in range(21)]
    if found != expected:
        fail(f"phase order differs: actual={found!r}, expected={expected!r}")
    mapped = sorted({phase for phase, _ in DOMAIN.values()})
    unknown = sorted(set(mapped) - set(found))
    if unknown:
        fail(f"feature map references unknown phases: {', '.join(unknown)}")
    dependencies: dict[str, list[str]] = {}
    for phase, raw in DEPENDENCY_ROW_RE.findall(phases_text):
        deps = [] if raw.strip() == "none" else [item.strip() for item in raw.split(",")]
        dependencies[phase] = deps
    if list(dependencies) != expected:
        fail(f"dependency manifest order differs: actual={list(dependencies)!r}")
    for phase, deps in dependencies.items():
        unknown_deps = sorted(set(deps) - set(expected))
        if unknown_deps:
            fail(f"{phase} has unknown dependencies: {unknown_deps!r}")
        if phase in deps:
            fail(f"{phase} depends on itself")
        later = [dep for dep in deps if expected.index(dep) >= expected.index(phase)]
        if later:
            fail(f"{phase} has non-topological dependencies: {later!r}")
    reachable: set[str] = set()
    for phase in expected:
        if not set(dependencies[phase]) <= reachable:
            fail(f"{phase} dependencies are not reachable in phase order")
        reachable.add(phase)


def validate_markdown_files(paths: list[Path]) -> None:
    broken: list[str] = []
    for path in paths:
        text = read(path)
        if not text.endswith("\n"):
            fail(f"missing final newline: {path}")
        for number, line in enumerate(text.splitlines(), start=1):
            if line.rstrip() != line:
                fail(f"trailing whitespace: {path}:{number}")
        for raw_target in LINK_RE.findall(text):
            target = raw_target.strip()
            if target.startswith(("http://", "https://", "mailto:", "#")):
                continue
            target = target.split("#", 1)[0]
            if target.startswith("<") and target.endswith(">"):
                target = target[1:-1]
            resolved = (path.parent / unquote(target)).resolve()
            if not resolved.exists():
                broken.append(f"{path.relative_to(ROOT)} -> {raw_target}")
    if broken:
        fail("broken local links: " + "; ".join(broken))


def validate_program_files() -> None:
    paths = sorted(PROGRAM.glob("*.md"))
    required = {
        "AGENTS.md", "CLAUDE.md", "README.md", "RULES.md", "SOP.md",
        "STATUS.md", "TODO.md", "VALUES.md", "PHASES.md", "FEATURE-MAP.md",
        "DRIVERS.md", "SERVICES.md", "APPLICATIONS.md",
        "LANGUAGE-AGENTS-OPERATIONS.md", "PROOF-GATES.md",
        "RESEARCH-CONTRACT-CROSSWALK.md", "VALIDATION-RECEIPT.md",
        "PRODUCT-IMPLEMENTATION-ORDER.md", "PARTIAL-CLOSURE.md",
    }
    if {path.name for path in paths} != required:
        fail(f"program file set differs: actual={[path.name for path in paths]!r}")
    validate_markdown_files(paths)


def validate_research_files() -> int:
    paths = sorted(RESEARCH.rglob("*.md"))
    actual = {path.relative_to(RESEARCH).as_posix() for path in paths}
    if actual != EXPECTED_RESEARCH_DOCS:
        missing = sorted(EXPECTED_RESEARCH_DOCS - actual)
        unexpected = sorted(actual - EXPECTED_RESEARCH_DOCS)
        fail(f"research file set differs: missing={missing!r}, unexpected={unexpected!r}")
    validate_markdown_files(paths)
    return len(paths)


def validate_repositories() -> int:
    inventories: dict[str, set[str]] = {}
    for name, path in REPOSITORY_SOURCES.items():
        found = REPOSITORY_RE.findall(read(path))
        if len(found) != 33 or len(set(found)) != 33:
            fail(f"{name} repository ledger is not 33 unique rows: rows={len(found)} unique={len(set(found))}")
        inventories[name] = set(found)
    expected = inventories["manifest"]
    for name, repositories in inventories.items():
        if repositories != expected:
            missing = sorted(expected - repositories)
            unexpected = sorted(repositories - expected)
            fail(f"{name} repository set differs: missing={missing!r}, unexpected={unexpected!r}")
    return len(expected)


def extract_current_names(source_text: str) -> list[str]:
    appendix = source_text.split("## Exact current zlOS named-implementation crosswalk", 1)
    if len(appendix) != 2:
        fail("canonical current-implementation appendix missing")
    body = appendix[1].split("## Observed external application breadth", 1)[0]
    names: list[str] = []
    row_re = re.compile(r"^\| (?!Current name|Current game)([^|]+?) \|", re.MULTILINE)
    for raw in row_re.findall(body):
        name = raw.replace("`", "").strip()
        if name and not set(name) <= {"-", ":"}:
            names.append(name)
    return names


def validate_apps(source_text: str) -> None:
    apps_text = read(PROGRAM / "APPLICATIONS.md")
    names = extract_current_names(source_text)
    if len(names) != 62:
        fail(f"current names including All Applications: {len(names)}, expected 62")
    if len(set(names)) != len(names):
        fail("duplicate names in canonical current-app/game appendix")
    missing = [name for name in names if name not in apps_text]
    if missing:
        fail(f"current apps/games missing from APPLICATIONS.md: {missing!r}")
    cur_ids = sorted({int(value) for value in re.findall(r"\| CUR-(\d{3}) \|", apps_text)})
    if cur_ids != list(range(1, 39)):
        fail(f"CUR registry differs: {cur_ids!r}")
    game_ids = sorted({int(value) for value in re.findall(r"\| GAME-(\d{3}) \|", apps_text)})
    if game_ids != list(range(1, 25)):
        fail(f"GAME registry differs: {game_ids!r}")


def validate_registry_ids() -> dict[str, int]:
    paths = [
        PROGRAM / "DRIVERS.md",
        PROGRAM / "SERVICES.md",
        PROGRAM / "APPLICATIONS.md",
        PROGRAM / "LANGUAGE-AGENTS-OPERATIONS.md",
    ]
    all_ids: list[str] = []
    for path in paths:
        all_ids.extend(REGISTRY_ID_RE.findall(read(path)))
    duplicates = sorted(item for item, count in Counter(all_ids).items() if count != 1)
    if duplicates:
        fail(f"duplicate specialized registry IDs: {', '.join(duplicates)}")
    counts = Counter(item.split("-", 1)[0] for item in all_ids)
    required = {
        "PLAT", "BUS", "BLK", "USB", "INPUT", "GPU", "NIC", "RADIO",
        "MEDIA", "PWR", "SENSOR", "PERIPH", "VM", "FSP", "SVC", "CUR",
        "GAME", "APP", "ZLP", "DEV", "AGT", "OPS", "FUT",
    }
    missing = sorted(required - set(counts))
    if missing:
        fail(f"missing specialized registry families: {', '.join(missing)}")
    actual = dict(sorted(counts.items()))
    if actual != EXPECTED_REGISTRY_COUNTS:
        fail(f"specialized registry counts differ: actual={actual!r}")
    phase_row_files = [PROGRAM / "DRIVERS.md", PROGRAM / "SERVICES.md", PROGRAM / "APPLICATIONS.md"]
    valid_phases = {f"MP-{index:02d}" for index in range(21)}
    for path in phase_row_files:
        for number, line in enumerate(read(path).splitlines(), start=1):
            if not REGISTRY_ID_RE.match(line):
                continue
            phases = set(re.findall(r"MP-\d{2}", line))
            if not phases:
                fail(f"registry row has no primary phase: {path}:{number}")
            unknown = sorted(phases - valid_phases)
            if unknown:
                fail(f"registry row has unknown phase at {path}:{number}: {unknown!r}")
    return actual


def escape_table(value: str) -> str:
    return value.replace("|", "\\|").replace("\n", " ")


def render_feature_map(features: list[dict[str, str]], source_text: str) -> str:
    digest = hashlib.sha256(source_text.encode("utf-8")).hexdigest()
    lines = [
        "# Complete 906-feature execution map",
        "",
        "Generated by `tools/validate_master_program.py`; do not hand-edit.",
        "",
        f"Canonical source SHA-256: `{digest}`.",
        "",
        "Each row retains the research state and complete-product acceptance text,",
        "then assigns one primary implementation phase. Cross-phase prerequisites",
        "are defined in `PHASES.md` and the specialized registries.",
        "",
        "| ID | Feature | Research state | Primary phase | Workstream | Complete acceptance contract |",
        "|---|---|---|---|---|---|",
    ]
    for row in features:
        phase, workstream = DOMAIN[row["prefix"]]
        lines.append(
            "| {id} | {feature} | {state} | {phase} | {workstream} | {meaning} |".format(
                id=row["id"],
                feature=escape_table(row["feature"]),
                state=row["state"],
                phase=phase,
                workstream=workstream,
                meaning=escape_table(row["meaning"]),
            )
        )
    counts = Counter(row["prefix"] for row in features)
    lines.extend(
        [
            "",
            "## Mechanical totals",
            "",
            f"Total unique feature atoms: **{len(features)}**.",
            "",
            "| Prefix | Count | Primary phase |",
            "|---|---:|---|",
        ]
    )
    for prefix in EXPECTED_PREFIX_COUNTS:
        lines.append(f"| {prefix} | {counts[prefix]} | {DOMAIN[prefix][0]} |")
    lines.append("")
    return "\n".join(lines)


def contract_phase(contract_id: str) -> str:
    if contract_id.startswith("P"):
        major, minor = (int(value) for value in contract_id[1:].split("."))
        simple = {
            0: "MP-00", 1: "MP-01", 2: "MP-06", 3: "MP-03", 4: "MP-04",
            9: "MP-16", 10: "MP-17", 11: "MP-18",
        }
        if major in simple:
            return simple[major]
        if major == 5:
            return {1: "MP-00", 2: "MP-06", 3: "MP-07", 4: "MP-09", 5: "MP-08", 6: "MP-11", 7: "MP-06"}[minor]
        if major == 6:
            return {1: "MP-08", 2: "MP-08", 3: "MP-08", 4: "MP-13"}[minor]
        if major == 7:
            return {1: "MP-11", 2: "MP-12", 3: "MP-06", 4: "MP-15", 5: "MP-15"}[minor]
        if major == 8:
            return {1: "MP-05", 2: "MP-05", 3: "MP-19", 4: "MP-07", 5: "MP-14"}[minor]
        if major == 12:
            return "MP-19" if minor == 5 else "MP-20"
    da = {
        "DA-00": "MP-00", "DA-01": "MP-00", "DA-02": "MP-01", "DA-03": "MP-00",
        "DA-03F": "MP-02", "DA-04": "MP-05", "DA-05": "MP-05", "DA-06": "MP-05",
        "DA-07": "MP-05", "DA-07M": "MP-05", "DA-08": "MP-05", "DA-08O": "MP-00",
        "DA-09": "MP-07", "DA-10": "MP-07", "DA-11": "MP-06", "DA-12": "MP-05",
        "DA-13": "MP-08", "DA-14": "MP-09", "DA-15": "MP-19", "DA-15S": "MP-19",
        "DA-16": "MP-19", "DA-17": "MP-03", "DA-17L": "MP-03", "DA-18": "MP-04",
        "DA-19": "MP-04", "DA-20": "MP-07", "DA-20C": "MP-11", "DA-21": "MP-06",
        "DA-21F": "MP-11", "DA-22": "MP-08", "DA-23": "MP-11", "DA-23L": "MP-11",
        "DA-24": "MP-06", "DA-25": "MP-11", "DA-25N": "MP-11", "DA-26": "MP-04",
        "DA-27": "MP-12", "DA-28": "MP-12", "DA-29": "MP-12", "DA-30": "MP-13",
        "DA-31": "MP-12", "DA-31O": "MP-14", "DA-32": "MP-12", "DA-33": "MP-14",
        "DA-33P": "MP-14", "DA-34": "MP-14", "DA-35": "MP-17", "DA-36": "MP-02",
        "DA-37": "MP-18", "DA-38": "MP-18", "DA-39": "MP-00", "DA-40": "MP-00",
        "DA-41": "MP-01", "DA-42": "MP-01", "DA-43": "MP-05", "DA-44": "MP-00",
    }
    if contract_id.startswith("DA-"):
        return da.get(contract_id, "")
    if contract_id.startswith("VX-"):
        number = int(contract_id.split("-", 1)[1])
        if number <= 1:
            return "MP-00"
        if number <= 10:
            return "MP-10"
        if number <= 14:
            return "MP-07"
        if number <= 19:
            return "MP-11" if number != 17 else "MP-06"
        if number <= 25:
            return "MP-12"
        if number <= 33:
            return "MP-13"
        if number <= 45:
            return "MP-14"
        if number == 46:
            return "MP-17"
        if number == 47:
            return "MP-18"
        if number in {48, 49, 50}:
            return "MP-00"
        if number == 51:
            return "MP-01"
        if number == 52:
            return "MP-18"
        if number == 53:
            return "MP-01"
    return ""


def parse_contracts() -> list[tuple[str, str, str]]:
    contracts: list[tuple[str, str, str]] = []
    expected_counts = {"core": 64, "driver-app": 56, "visual": 54}
    for family, path in BACKLOG_SOURCES.items():
        found = CONTRACT_RE.findall(read(path))
        if len(found) != expected_counts[family]:
            fail(f"{family} backlog has {len(found)} contracts, expected {expected_counts[family]}")
        for contract_id, title in found:
            phase = contract_phase(contract_id)
            if not phase:
                fail(f"unmapped research contract: {contract_id}")
            contracts.append((family, contract_id, title.strip()))
    ids = [contract_id for _, contract_id, _ in contracts]
    duplicates = sorted(item for item, count in Counter(ids).items() if count != 1)
    if duplicates:
        fail(f"duplicate research contract IDs: {duplicates!r}")
    return contracts


def render_contract_crosswalk(contracts: list[tuple[str, str, str]]) -> str:
    lines = [
        "# Research implementation-contract crosswalk",
        "",
        "Generated by `tools/validate_master_program.py`; do not hand-edit.",
        "",
        "This proves that every implementation contract from the core, driver/app",
        "and visual/browser/app research backlogs has a primary destination in the",
        "master phase DAG. The original contract remains the detailed acceptance",
        "source; the master program supplies current ordering and ownership.",
        "",
        "| Source backlog | Contract | Original deliverable | Master phase |",
        "|---|---|---|---|",
    ]
    for family, contract_id, title in contracts:
        lines.append(f"| {family} | {contract_id} | {escape_table(title)} | {contract_phase(contract_id)} |")
    lines.extend(
        [
            "",
            "## Mechanical totals",
            "",
            f"Mapped contracts: **{len(contracts)}** (core 64, driver/app 56, visual/browser/app 54).",
            "",
        ]
    )
    return "\n".join(lines)


def validate_generated(path: Path, expected: str, write: bool) -> None:
    if write:
        path.write_text(expected, encoding="utf-8")
        return
    actual = read(path)
    if actual != expected:
        fail(f"{path.name} is stale; run with --write and inspect the diff")


def run_self_test(features: list[dict[str, str]]) -> None:
    mutations = {
        "missing-feature": features[:-1],
        "duplicate-feature": features + [features[0]],
        "unknown-prefix": [{**features[0], "prefix": "QQ"}] + features[1:],
    }
    caught: list[str] = []
    for name, mutant in mutations.items():
        try:
            validate_features(mutant)
        except ValueError:
            caught.append(name)
        else:
            fail(f"validator self-test mutation escaped: {name}")
    if contract_phase("DA-DOES-NOT-EXIST"):
        fail("validator self-test unknown contract escaped")
    caught.append("unknown-contract")
    print("master-program self-test: PASS: " + ", ".join(caught))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true", help="regenerate FEATURE-MAP.md")
    parser.add_argument("--self-test", action="store_true", help="prove key omissions are rejected")
    args = parser.parse_args()
    try:
        source_text = read(SOURCE)
        features = parse_features(source_text)
        validate_features(features)
        validate_phases()
        validate_apps(source_text)
        registry_counts = validate_registry_ids()
        rendered = render_feature_map(features, source_text)
        validate_generated(OUTPUT, rendered, args.write)
        contracts = parse_contracts()
        crosswalk = render_contract_crosswalk(contracts)
        validate_generated(CROSSWALK_OUTPUT, crosswalk, args.write)
        validate_program_files()
        research_doc_count = validate_research_files()
        repository_count = validate_repositories()
        if args.self_test:
            run_self_test(features)
    except ValueError as error:
        print(f"master-program: FAIL: {error}", file=sys.stderr)
        return 1
    print(
        "master-program: PASS: "
        f"features={len(features)} prefixes={len(EXPECTED_PREFIX_COUNTS)} "
        f"phases=21 research_contracts={len(contracts)} registries={sum(registry_counts.values())} "
        f"research_docs={research_doc_count} repositories={repository_count} "
        f"current_named=61 catalogue=1 games=24"
    )
    print("registry-counts: " + " ".join(f"{key}={value}" for key, value in registry_counts.items()))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
