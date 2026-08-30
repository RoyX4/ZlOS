#!/usr/bin/env python3
"""Generate and verify the documentation capsule in every tracked directory."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
CAPSULE_FILES = (
    "README.md",
    "AGENTS.md",
    "CLAUDE.md",
    "STATUS.md",
    "TODO.md",
    "RULES.md",
    "VALUES.md",
    "SOP.md",
)
ROOT_PRESERVED = {"README.md", "AGENTS.md", "CLAUDE.md", "TODO.md"}
GENERATED_INDEX = "docs/DIRECTORY-CAPSULE-INDEX.md"
GENERATED_START = "<!-- BEGIN GENERATED: directory-docs -->"
GENERATED_END = "<!-- END GENERATED: directory-docs -->"
LOCAL_START = "<!-- BEGIN LOCAL: directory-docs -->"
LOCAL_END = "<!-- END LOCAL: directory-docs -->"
LOCAL_PATTERN = re.compile(
    re.escape(LOCAL_START) + r"\n(.*?)\n" + re.escape(LOCAL_END), re.DOTALL
)


PURPOSES = {
    ".": "Repository root for the zl language toolchain and the zlOS operating system.",
    ".design": "Machine-readable record of the selected visual direction.",
    ".github": "GitHub-facing contribution guidance and automation configuration.",
    ".github/scripts": "Helper scripts invoked by GitHub automation.",
    ".github/workflows": "Continuous integration, boot, documentation, and scheduled workflows.",
    ".ultra": "Evidence-backed integration tensions and their OPEN or CLOSED state.",
    ".vscode": "Shared VS Code tasks, launch configuration, and C/C++ indexing settings.",
    "bench": "Benchmark programs and runners for measured language-engine comparisons.",
    "docs": "Repository-wide project status, architecture, program, evidence, and history.",
    "docs/archive": "Repository-level historical material that is no longer current authority.",
    "docs/archive/backups": "Preserved backup inputs retained for provenance rather than active use.",
    "docs/archive/backups/ci-session-scratch": "Recovered CI-session scratch material retained as a historical input.",
    "docs/archive/handoffs": "Superseded repository handoffs retained for history.",
    "docs/archive/prompts": "Completed or superseded prompts retained to explain how work was commissioned.",
    "docs/archive/superseded": "Plans and scorecards replaced by newer project authorities.",
    "docs/design": "Design proposals, decisions, implementation plans, and visual prototypes.",
    "docs/design/prototypes": "Preserved interactive desktop design prototypes.",
    "docs/design/prototypes/compare": "Generated comparison sheets for preserved desktop prototypes.",
    "docs/design/prototypes/render": "Rendered scene captures from preserved desktop prototypes.",
    "docs/design/specs": "Frozen machine-readable specifications for desktop design directions.",
    "docs/evidence": "Measured repository-level receipts and dated audit evidence.",
    "docs/evidence/integration": "Receipts for branch, worktree, clone, and publication integration.",
    "docs/evidence/status-audits": "Dated project-state audits retained as evidence snapshots.",
    "docs/evidence/visual-diffs": "Curated before-and-after visual comparisons with retained context.",
    "docs/fleet": "Audit boards, review findings, and verification logs for coordinated work.",
    "docs/program": "Normalized product program, phase model, contracts, registries, and status data.",
    "docs/program/research": "Complete source-research shelf feeding the normalized product program.",
    "docs/program/research/language": "Language-specific research and implementation-gap analysis.",
    "editors": "Editor and language-tool integrations.",
    "editors/vscode-zl": "VS Code extension for zl syntax, formatting, snippets, and run commands.",
    "editors/vscode-zl/snippets": "VS Code snippet definitions for zl source.",
    "editors/vscode-zl/syntaxes": "TextMate grammar definitions for zl syntax highlighting.",
    "examples": "Runnable zl examples that also participate in repository test coverage.",
    "freestanding": "Bounded no-libc proof lane for generated zl programs.",
    "gates": "Resource-contained landing-gate orchestration and policy.",
    "kernel": "zlOS product root, build entry points, source manifest, and kernel orientation.",
    "kernel/apps": "zlOS applications and games written in zl.",
    "kernel/boot": "BIOS, raw, long-mode, and native UEFI entry code and linker layouts.",
    "kernel/docs": "Kernel documentation front door and retained implementation history.",
    "kernel/docs/architecture": "Current zlOS architecture contracts grouped by subsystem.",
    "kernel/docs/architecture/boot": "Boot-route, handover, and loader architecture contracts.",
    "kernel/docs/architecture/system": "System-wide ABI, audit, memory, process, and service contracts.",
    "kernel/docs/archive": "Kernel documentation that is historical rather than current authority.",
    "kernel/docs/archive/audits": "Superseded kernel audit snapshots retained for provenance.",
    "kernel/docs/archive/handoffs": "Older kernel handoffs retained as dated history.",
    "kernel/docs/archive/prompts": "Completed or superseded kernel work prompts.",
    "kernel/docs/archive/superseded": "Kernel plans replaced by current contracts or evidence.",
    "kernel/docs/concepts": "Bounded feasibility, ownership, and system-concept explanations.",
    "kernel/docs/desktop": "Desktop interaction, pacing, workspace, and visual-system documentation.",
    "kernel/docs/drivers": "Current driver documentation grouped by hardware domain.",
    "kernel/docs/drivers/display": "Display and Intel graphics implementation documentation.",
    "kernel/docs/drivers/network": "Network-device implementation and evidence ladders.",
    "kernel/docs/evidence": "Dated zlOS implementation, QEMU, and physical evidence receipts.",
    "kernel/docs/evidence/current-visuals-2026-08-29": "Dated visual evidence bundle captured on 2026-08-29.",
    "kernel/docs/evidence/display": "Structured display research and modesetting evidence.",
    "kernel/docs/evidence/exercises": "Curated exercise bundles retained with provenance and limits.",
    "kernel/docs/evidence/exercises/2026-08-24": "Exercise bundles captured on 2026-08-24.",
    "kernel/docs/evidence/exercises/2026-08-24/bios": "Retained BIOS exercise frames and transcript from 2026-08-24.",
    "kernel/docs/evidence/exercises/2026-08-24/physical-touchpad-2026-08-24": "Retained physical touchpad evidence bundle from 2026-08-24.",
    "kernel/docs/evidence/exercises/2026-08-24/uefi": "Retained native UEFI exercise frames and transcript from 2026-08-24.",
    "kernel/docs/evidence/mp00": "MP-00 foundation execution and coverage receipts.",
    "kernel/docs/features": "Feature-specific zlOS contracts and implementation notes.",
    "kernel/docs/guides": "Operator, contributor, and hardware runbooks.",
    "kernel/docs/plans": "Current or explicitly status-qualified kernel implementation plans.",
    "kernel/docs/receipts": "Machine-readable evidence, recovery archives, and provenance receipts.",
    "kernel/docs/reference": "Stable zlOS specifications grouped by system and interface.",
    "kernel/docs/reference/system": "Stable system, memory, input, and boot references.",
    "kernel/docs/reference/ui": "Stable UI conventions and interface reference.",
    "kernel/docs/research": "Source-heavy comparative and hardware research.",
    "kernel/metadata": "Generated registries and manifests derived from source, policy, and receipts.",
    "kernel/src": "Shipped zlOS implementation grouped by architecture and subsystem.",
    "kernel/src/arch": "Architecture-specific zlOS implementation.",
    "kernel/src/arch/x86": "x86 and x86-64 CPU, interrupt, paging, syscall, and low-level support.",
    "kernel/src/core": "Core kernel services shared across boot routes and subsystems.",
    "kernel/src/core/boot": "Typed boot handover and recovery-policy integration.",
    "kernel/src/drivers": "Hardware drivers grouped by device domain.",
    "kernel/src/drivers/display": "Framebuffer, BGA, Intel display, and GPU-assist drivers.",
    "kernel/src/drivers/display/assets": "Generated or captured binary include data consumed by display drivers.",
    "kernel/src/drivers/input": "PS/2, USB HID, I2C HID, and input-routing drivers.",
    "kernel/src/drivers/network": "PCI, virtio, Ethernet, and Wi-Fi network-device drivers.",
    "kernel/src/drivers/storage": "NVMe and storage-device drivers.",
    "kernel/src/fs": "zlfs filesystem implementation.",
    "kernel/src/graphics": "Graphics implementation grouped by rendering concern.",
    "kernel/src/graphics/fonts": "Font data, glyph rendering, and text-rasterization support.",
    "kernel/src/graphics/framebuffer": "Framebuffer primitives and presentation support.",
    "kernel/src/graphics/icons": "Icon assets and icon rendering support.",
    "kernel/src/graphics/image": "Image decoding and image-surface support.",
    "kernel/src/graphics/ui": "UI controls, themes, shell surfaces, and application-facing widgets.",
    "kernel/src/graphics/windowing": "Window manager, compositor, retained surfaces, and input dispatch.",
    "kernel/src/net": "Network stack protocols, sockets, DHCP, DNS, HTTP, and TLS support.",
    "kernel/src/runtime": "Kernel-side bridge for executing zl semantics.",
    "kernel/src/web": "Browser parsing, style, layout, scripting, and rendering support.",
    "kernel/tests": "zlOS host tests, fixtures, independent oracles, and reference renderers.",
    "kernel/tests/fixtures": "Stable test inputs consumed by zlOS harnesses.",
    "kernel/tests/host": "Host-compiled harnesses for kernel components and hardware-facing logic.",
    "kernel/tests/oracle": "Independent visual oracle and comparison tooling.",
    "kernel/tests/refrender": "Browser-based reference renderer for visual comparison.",
    "kernel/tests/refrender/vendor": "Pinned third-party JavaScript used by the reference renderer.",
    "kernel/tools": "Kernel checks, generators, image builders, probes, and runners.",
    "kernel/tools/checks": "Deterministic static, host, image, and boot verification entry points.",
    "kernel/tools/generators": "Generators for registries, manifests, evidence views, and source data.",
    "kernel/tools/images": "Disk, ISO, USB, and boot-image construction helpers.",
    "kernel/tools/probes": "Runtime probes that inspect QEMU or retained evidence outputs.",
    "kernel/tools/run": "Bounded launchers for QEMU and local kernel exercises.",
    "learn": "Graded zl learning exercises and their runner.",
    "learn/solutions": "Reference solutions for the graded zl exercises.",
    "src": "Host language implementation and ownership map.",
    "src/backends": "Language output backends grouped by target strategy.",
    "src/backends/c": "Boxed and experimental unboxed C code generators.",
    "src/backends/llvm": "Active LLVM IR speed backend.",
    "src/backends/native": "Direct native executable generators and native-runtime experiments.",
    "src/frontend": "Shared lexer, parser, token, and AST implementation.",
    "src/runtime": "Reference interpreter, compiled runtime, and Linux OS bridge.",
    "src/selfhost": "Bounded compiler and frontend work written in zl itself.",
    "src/tools": "Language-aware developer tools such as zlfmt.",
    "stdlib": "Tracked zl library modules and algorithm examples.",
    "tests": "Language conformance and regression programs.",
    "tools": "Repository maintenance, documentation, parity, audit, and preflight tools.",
}


CLASS_INFO = {
    "source": ("active source", "Source exists here, but existence does not prove reachability, tests, boot, or hardware behavior."),
    "proof": ("bounded proof lane", "Results apply only to this lane and do not promote the complete language or product."),
    "tests": ("active verification support", "Tests describe exercised behavior; they do not prove unexecuted routes or physical hardware."),
    "tooling": ("active tooling", "A tool is useful only when its invocation, failure behavior, and consumer are verified."),
    "documentation": ("current or status-qualified documentation", "Documents must state whether they are plans, current authority, research, or historical evidence."),
    "evidence": ("retained evidence", "The contents preserve bounded observations; they are not automatically current project state."),
    "archive": ("archived history", "The contents are retained for provenance and are not active instructions or queues."),
    "generated": ("generated truth surface", "The owning source and generator are authoritative; direct edits are not."),
    "vendor": ("third-party vendored material", "Any claim requires verified upstream provenance and a pinned version; do not assume either is complete."),
    "support": ("active repository support", "Changes affect the owning integration only and require its focused validation."),
}


def git_lines(*args: str) -> list[str]:
    result = subprocess.run(
        ["git", *args], cwd=ROOT, check=True, text=True, stdout=subprocess.PIPE
    )
    return [line for line in result.stdout.splitlines() if line]


def tracked_directories() -> list[str]:
    directories = {"."}
    for name in git_lines("ls-files"):
        # Capsule files do not keep an otherwise-deleted project directory alive.
        # Coverage is derived from actual project content and parent structure.
        if Path(name).name in CAPSULE_FILES or name == GENERATED_INDEX:
            continue
        parent = Path(name).parent
        while parent != Path("."):
            directories.add(parent.as_posix())
            parent = parent.parent
    return sorted(directories, key=lambda item: (item != ".", item))


def classify(path: str) -> str:
    parts = set(Path(path).parts)
    if path == ".":
        return "support"
    if "vendor" in parts:
        return "vendor"
    if "archive" in parts:
        return "archive"
    if "evidence" in parts or path == ".ultra":
        return "evidence"
    if path in {".design", "kernel/metadata", "kernel/src/drivers/display/assets"}:
        return "generated"
    if path == "kernel/docs/receipts":
        return "evidence"
    if "tests" in parts or path.startswith("kernel/tests"):
        return "tests"
    if path == "docs" or path.startswith("docs/") or path.startswith("kernel/docs"):
        return "documentation"
    if path in {"examples", "freestanding", "learn", "learn/solutions", "bench", "src/selfhost"}:
        return "proof"
    if (
        path.startswith("tools")
        or path.startswith("kernel/tools")
        or path.startswith("gates")
        or path.startswith("editors")
        or path.startswith(".github")
        or path == ".vscode"
        or path == "src/tools"
    ):
        return "tooling"
    if path.startswith("src") or path.startswith("kernel") or path == "stdlib":
        return "source"
    return "support"


def purpose(path: str) -> str:
    return PURPOSES.get(path, f"Project-owned content under `{path}`.")


def from_dir(path: str, target: str) -> str:
    source = ROOT if path == "." else ROOT / path
    return os.path.relpath(ROOT / target, source).replace(os.sep, "/")


def link(path: str, label: str, target: str) -> str:
    return f"[{label}]({from_dir(path, target)})"


def local_block(existing: str | None, kind: str) -> str:
    if kind in {"archive", "evidence", "generated", "vendor"}:
        return "<!-- Immutable class: add current work or claims at the owning source or in a superseding receipt. -->"
    if existing:
        match = LOCAL_PATTERN.search(existing)
        if match:
            return match.group(1)
    return "<!-- Add verified directory-specific notes here. -->"


def wrapped(body: str, existing: str | None, kind: str) -> str:
    local = local_block(existing, kind)
    return (
        "<!-- Managed by tools/directory-docs.py. Edit only the LOCAL block. -->\n"
        f"{GENERATED_START}\n{body.rstrip()}\n{GENERATED_END}\n\n"
        f"{LOCAL_START}\n{local}\n{LOCAL_END}\n"
    )


def direct_inventory(path: str, directories: list[str]) -> tuple[list[str], list[str]]:
    base = Path(".") if path == "." else Path(path)
    children = []
    for candidate in directories:
        if candidate == ".":
            continue
        parent = Path(candidate).parent
        if parent == base:
            children.append(Path(candidate).name + "/")

    files = []
    prefix = "" if path == "." else path + "/"
    for name in git_lines("ls-files"):
        if not name.startswith(prefix):
            continue
        rest = name[len(prefix) :]
        if "/" not in rest and rest not in CAPSULE_FILES:
            files.append(rest)
    return sorted(children), sorted(files)


def class_rules(kind: str) -> list[str]:
    common = [
        "Do not infer completion from a file, symbol, target, or document existing.",
        "Keep host, build, QEMU, graphical, and physical evidence explicitly separate.",
    ]
    specific = {
        "source": ["Keep implementation with its owning subsystem and update consumers, manifests, and focused checks together."],
        "proof": ["Preserve the stated scope of the proof lane; do not generalize a bounded result."],
        "tests": ["Make every check capable of failing for the behavior it claims to cover."],
        "tooling": ["Keep tools deterministic, bounded, and explicit about skipped or unavailable checks."],
        "documentation": ["Label plans, decisions, current state, evidence, and history so they cannot be confused."],
        "evidence": ["Preserve dated observations; add a superseding receipt instead of rewriting history."],
        "archive": ["Do not execute or cite archived material as current authority without revalidation."],
        "generated": ["Change the owning source or generator and regenerate; do not hand-edit generated truth."],
        "vendor": ["Preserve upstream bytes and provenance; isolate local adaptation outside the vendor directory."],
        "support": ["Keep changes narrow and validate them through the owning tool or integration."],
    }
    return common + specific[kind]


def class_values(kind: str) -> list[str]:
    values = {
        "source": ["Clear ownership", "Small reversible changes", "Reachability before claims"],
        "proof": ["Bounded claims", "Reproducibility", "Explicit limitations"],
        "tests": ["Refutability", "Determinism", "Independent evidence"],
        "tooling": ["Automation", "Fail-closed behavior", "Readable diagnostics"],
        "documentation": ["Truthful status", "Durable context", "One clear authority"],
        "evidence": ["Provenance", "Immutability of observations", "Evidence-lane separation"],
        "archive": ["Historical fidelity", "Clear deactivation", "Recoverable context"],
        "generated": ["Determinism", "Traceable inputs", "No silent drift"],
        "vendor": ["Provenance", "Minimal divergence", "License fidelity"],
        "support": ["Compatibility", "Narrow scope", "Operational clarity"],
    }
    return values[kind]


def render(path: str, filename: str, directories: list[str], existing: str | None) -> str:
    kind = classify(path)
    state, boundary = CLASS_INFO[kind]
    location = "repository root" if path == "." else f"`{path}/`"
    project_status = link(path, "project status", "docs/PROJECT-STATUS.md")
    code_map = link(path, "code map", "docs/CODE-MAP.md")
    remaining = link(path, "remaining-work map", "docs/REMAINING-WORK.md")
    program = link(path, "complete program", "docs/program/README.md")
    plan = link(path, "directory documentation plan", "docs/design/directory-documentation-system.md")

    if filename == "README.md":
        child_dirs, files = direct_inventory(path, directories)
        child_text = "\n".join(f"- `{item}`" for item in child_dirs) or "- None."
        file_text = "\n".join(f"- `{item}`" for item in files) or "- No direct tracked project files."
        body = f"""# {path if path != '.' else 'zl-linux'}

## Purpose

{purpose(path)}

## Classification

- **Class:** `{kind}`
- **State:** {state}
- **Evidence boundary:** {boundary}

## Start here

- Read the {project_status} for current truth.
- Read the local [status](STATUS.md), [tasks](TODO.md), [rules](RULES.md),
  [values](VALUES.md), and [operating procedure](SOP.md).
- Editing agents must also read [AGENTS.md](AGENTS.md).

## Child directories

{child_text}

## Direct tracked contents

{file_text}

This inventory is local orientation, not a completion claim. See the {code_map}
for repository-wide ownership."""
    elif filename == "CLAUDE.md":
        rule_text = "\n".join(f"- {item}" for item in class_rules(kind))
        body = f"""# Local agent contract: {location}

## Scope

{purpose(path)} This file adds local constraints to every ancestor `AGENTS.md`;
it does not replace them.

## Read first

- Local [README](README.md), [status](STATUS.md), [tasks](TODO.md),
  [rules](RULES.md), and [SOP](SOP.md).
- Canonical {project_status} and {code_map}.

## Local contract

{rule_text}

Before reporting completion, re-read [STATUS.md](STATUS.md), run the smallest
relevant deterministic checks, and state every unrun evidence lane."""
    elif filename == "STATUS.md":
        body = f"""# Status: {location}

| Field | Current value |
|---|---|
| Purpose | {purpose(path)} |
| Class | `{kind}` |
| Directory state | {state} |
| Product completion | Not determined by this directory |
| Evidence boundary | {boundary} |

The canonical current repository and implementation state is the
{project_status}. Product work is tracked in the {remaining} and {program}.

An empty local task list, a complete file inventory, or a passing documentation
check must never be read as implementation, boot, release, or hardware proof."""
    elif filename == "TODO.md":
        if kind in {"archive", "evidence", "generated", "vendor"}:
            queue = (
                f"This directory is classified as `{kind}`, so it has no default active implementation queue. "
                "Changes must originate in the current owner, generator, or a new superseding receipt."
            )
        else:
            queue = (
                "No directory-specific task is assigned by the generated layer. Add only explicit, "
                "owned work to the LOCAL block below."
            )
        body = f"""# Tasks: {location}

{queue}

Use the {remaining} for the current cross-directory queue and the {program} for
the complete product backlog.

## Completion rule

A checked task must name its outcome and required evidence. An empty list means
only "no local task recorded"; it does not mean this area or feature is complete."""
    elif filename == "RULES.md":
        rule_text = "\n".join(f"- {item}" for item in class_rules(kind))
        body = f"""# Rules: {location}

These rules supplement the root working agreement.

{rule_text}

Placement follows the {code_map}. Status and completion claims follow the
{project_status}."""
    elif filename == "VALUES.md":
        value_text = "\n".join(f"- **{item}.** Apply it to every change in this directory." for item in class_values(kind))
        body = f"""# Values: {location}

{value_text}

Across all classes, truthful evidence outranks optimistic wording, stable
ownership outranks cosmetic movement, and a smaller verified result outranks a
larger unverified claim."""
    elif filename == "SOP.md":
        body = f"""# Standard operating procedure: {location}

1. Read the local `README.md`, `STATUS.md`, `TODO.md`, and `RULES.md`.
2. Confirm current authority in the {project_status} and ownership in the {code_map}.
3. Inspect `git status` and preserve unrelated or concurrent work.
4. Define the smallest outcome and the evidence lane it requires.
5. Make the change within this directory's `{kind}` boundary.
6. Run the smallest relevant deterministic check and prove it can observe the
   changed behavior where practical.
7. Update local tasks or status only when the evidence supports the wording.
8. Report passed, failed, skipped, QEMU-only, and physical-only evidence
   separately.

For capsule maintenance, follow the {plan} and run
`python3 tools/directory-docs.py --check` from the repository root."""
    else:
        raise ValueError(f"unsupported capsule file: {filename}")
    return wrapped(body, existing, kind)


def render_index(directories: list[str]) -> str:
    rows = []
    for path in directories:
        display = "." if path == "." else path + "/"
        base = "." if path == "." else path
        readme = "../README.md" if path == "." else f"../{base}/README.md"
        status = "../STATUS.md" if path == "." else f"../{base}/STATUS.md"
        todo = "../TODO.md" if path == "." else f"../{base}/TODO.md"
        state = CLASS_INFO[classify(path)][0]
        desc = purpose(path).replace("|", "\\|")
        rows.append(
            f"| [`{display}`]({readme}) | `{classify(path)}` | {state} | {desc} | [status]({status}) | [tasks]({todo}) |"
        )
    body = "\n".join(rows)
    return f"""# Directory capsule index

Generated by `tools/directory-docs.py`. Do not edit by hand.

This index covers the repository root and every directory containing tracked
content. Classification and local status are navigation aids; product completion
remains governed by [PROJECT-STATUS.md](PROJECT-STATUS.md) and evidence receipts.

| Directory | Class | Directory state | Purpose | Status | Tasks |
|---|---|---|---|---|---|
{body}

Total directories: **{len(directories)}**.
"""


def expected_files(directories: list[str]) -> dict[Path, str]:
    expected: dict[Path, str] = {}
    for directory in directories:
        base = ROOT if directory == "." else ROOT / directory
        for filename in CAPSULE_FILES:
            if filename == "AGENTS.md":
                continue
            if directory == "." and filename in ROOT_PRESERVED:
                continue
            target = base / filename
            existing = target.read_text() if target.exists() and not target.is_symlink() else None
            if existing and GENERATED_START not in existing:
                continue
            expected[target] = render(directory, filename, directories, existing)
    expected[ROOT / GENERATED_INDEX] = render_index(directories)
    return expected


def check(directories: list[str]) -> int:
    errors = []
    tracked = set(git_lines("ls-files"))
    for directory in directories:
        base = ROOT if directory == "." else ROOT / directory
        for filename in CAPSULE_FILES:
            target = base / filename
            if not target.exists():
                errors.append(f"missing {target.relative_to(ROOT)}")
            else:
                relative = target.relative_to(ROOT).as_posix()
                if relative not in tracked:
                    errors.append(f"untracked capsule {relative}")
        if directory != ".":
            agents = base / "AGENTS.md"
            if not agents.is_symlink() or os.readlink(agents) != "CLAUDE.md":
                errors.append(f"invalid agent link {agents.relative_to(ROOT)}")

    for target, content in expected_files(directories).items():
        if not target.exists() or target.is_symlink():
            errors.append(f"missing generated file {target.relative_to(ROOT)}")
        elif target.read_text() != content:
            errors.append(f"stale generated file {target.relative_to(ROOT)}")

    if GENERATED_INDEX not in tracked:
        errors.append(f"untracked generated file {GENERATED_INDEX}")

    if errors:
        for error in sorted(set(errors)):
            print(f"directory-docs: {error}", file=sys.stderr)
        print(f"directory-docs: FAIL ({len(set(errors))} issue(s))", file=sys.stderr)
        return 1
    print(f"directory-docs: PASS ({len(directories)} directories, 8 surfaces each)")
    return 0


def generate(directories: list[str]) -> int:
    changed = 0
    for target, content in expected_files(directories).items():
        if target.exists() and not target.is_symlink() and target.read_text() == content:
            continue
        target.write_text(content)
        changed += 1

    for directory in directories:
        if directory == ".":
            continue
        base = ROOT / directory
        agents = base / "AGENTS.md"
        claude = base / "CLAUDE.md"

        # Migrate the first generated orientation from the opposite link
        # direction without discarding its preserved LOCAL block.
        if (
            claude.is_symlink()
            and os.readlink(claude) == "AGENTS.md"
            and agents.is_file()
            and not agents.is_symlink()
            and GENERATED_START in agents.read_text()
        ):
            claude.unlink()
            agents.rename(claude)
            agents.symlink_to("CLAUDE.md")
            changed += 1
            continue

        if agents.is_symlink() and os.readlink(agents) == "CLAUDE.md":
            continue
        if agents.exists() or agents.is_symlink():
            print(f"directory-docs: refusing to replace {agents.relative_to(ROOT)}", file=sys.stderr)
            return 1
        agents.symlink_to("CLAUDE.md")
        changed += 1

    print(f"directory-docs: updated {changed} path(s) across {len(directories)} directories")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="verify without writing")
    args = parser.parse_args()
    directories = tracked_directories()
    missing_purposes = sorted(set(directories) - set(PURPOSES))
    if missing_purposes:
        print(
            "directory-docs: new directories need explicit purposes: "
            + ", ".join(missing_purposes),
            file=sys.stderr,
        )
        return 1
    return check(directories) if args.check else generate(directories)


if __name__ == "__main__":
    raise SystemExit(main())
