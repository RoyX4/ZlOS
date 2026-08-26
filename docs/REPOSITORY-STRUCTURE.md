# Repository Structure

This layout borrows the useful common pattern from mature language and OS
repositories: a small orientation surface at the root, stable source ownership,
and separate homes for tests, tools, current plans, evidence, and history.

## Current Layout

```text
zl-linux/
|-- README.md                 project front door
|-- build.sh                  language build entry point
|-- .gitattributes            LF, binary-asset, and generated-file policy
|-- .github/                  GitHub automation and CI workflows
|-- .vscode/                  shared editor tasks and language configuration
|-- .design/                  machine-readable visual-direction log
|-- .ultra/                   integration tension ledger and closure evidence
|-- src/
|   |-- frontend/             shared C lexer, parser, and AST headers
|   |-- runtime/              interpreter, boxed runtime, and Linux OS bridge
|   |-- backends/             C, LLVM, and direct-native code generators
|   |-- selfhost/             zl-written compiler, lexer, parser, and native work
|   `-- tools/                language-aware tools such as zlfmt
|-- stdlib/                   zl standard library
|-- tests/                    language tests
|-- examples/                 source examples
|-- freestanding/             no-libc proof lane
|-- bench/                    benchmark drivers and comparison trees
|-- learn/                    bounded teaching exercises
|-- editors/                  editor and language integrations
|-- kernel/                   zlOS product root
|   |-- apps/                 zl applications and games
|   |-- boot/                 entry code, EFI handoff, and linker layouts
|   |-- src/                  kernel implementation by subsystem
|   |   `-- core/boot/        typed handover and recovery policy
|   |-- tests/                host, oracle, reference, and fixture tests
|   |-- tools/                checks, generators, image builders, probes, runners
|   |-- metadata/             generated machine-readable registries
|   `-- docs/                 kernel docs grouped by purpose and subsystem
|-- tools/                    repository maintenance tools
|-- gates/                    landing orchestration
|-- docs/
|   |-- README.md             documentation front door
|   |-- PROJECT-STATUS.md     current truth and cross-repo commit state
|   |-- program/              complete destination and dependency graph
|   |-- design/               proposals and decisions
|   |-- evidence/             measured historical records
|   |-- fleet/                audit findings and boards
|   `-- archive/              superseded plans, prompts, backups, handoffs
`-- kernel/docs/
    |-- README.md             kernel documentation front door
    |-- architecture/         boot and system contracts
    |-- concepts/             bounded feasibility and ownership explainers
    |-- desktop/              desktop design, pacing, input, and workspace notes
    |-- drivers/              display and network implementation notes
    |-- evidence/             dated implementation and run receipts
    |   `-- exercises/        retained visual/QEMU/physical exercise artifacts
    |-- features/             feature-specific contracts
    |-- guides/               operator and onboarding runbooks
    |-- plans/                current or explicitly status-qualified plans
    |-- reference/            stable system and UI specifications
    |-- research/             comparative and source-heavy studies
    |-- receipts/             machine-readable evidence and recovery archives
    `-- archive/              old prompts, audits, and handoffs
```

## Source Ownership

The host language implementation lives under `src/`. Shared syntax belongs in
`src/frontend/`; executable semantics and the OS boundary belong in
`src/runtime/`; output formats belong in `src/backends/`; zl-written bootstrap
work belongs in `src/selfhost/`. `src/README.md` is the detailed ownership map.

The zlOS implementation follows the same ownership rule inside `kernel/`:
architecture, core services, drivers, filesystems, graphics, networking,
runtime, and web code have separate homes under `kernel/src/`. `kernel/SOURCES`
is the authoritative shared build manifest; boot-specific entry code remains
under `kernel/boot/`. Typed handover and recovery policy are owned together by
`kernel/src/core/boot/`; both implementation units are compiled through the
shared manifest, while persistent loader-policy wiring remains separate work.

## Placement Rules

- Current queues and truth maps stay near `docs/README.md`.
- Proposals stay in `docs/design/`; a proposal is never completion evidence.
- Measured outputs worth retaining go in an `evidence/` directory.
- Superseded prompts, dated handoffs, and backup copies go in `archive/`.
- Generated binaries, disk images, screenshots, caches, and compiler output are
  ignored. Curated proof belongs under `docs/`, with a short receipt.
- `.design/` and `.ultra/` are small tracked metadata surfaces, not source or
  scratch directories. Visual decisions go in the former; failed claims and
  evidence-backed closure records go in the latter.
- Feature maturity lives in program ledgers and receipts, not in source-folder
  names such as `done/` or `half-done/`.

## Reference Patterns

The useful pattern is consistency, not copying another repository's names.
Zig keeps `src`, `lib`, `test`, `tools`, and `doc` separate; SerenityOS uses
clear product ownership such as `Kernel`, `Userland`, `Tests`, `Toolchain`, and
`Documentation`; Redox keeps `src`, `recipes`, `scripts`, `config`, and build
machinery distinct. The local cognitive-substrate project adds a strong
`START-HERE` plus purpose-based `docs/` subfolders. zl-linux follows the same
idea while retaining zlOS's current kernel paths.
