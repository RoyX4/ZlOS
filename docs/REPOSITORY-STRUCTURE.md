# Repository Structure

This layout borrows the useful common pattern from mature language and OS
repositories: a small orientation surface at the root, stable source ownership,
and separate homes for tests, tools, current plans, evidence, and history.

## Current Layout

```text
zl-linux/
|-- README.md                 project front door
|-- build.sh                  language build entry point
|-- *.c, *.h, *.zl            language implementation, path-stable for now
|-- stdlib/                   zl standard library
|-- tests/                    language tests
|-- examples/                 source examples
|-- freestanding/             no-libc proof lane
|-- kernel/                   zlOS source, manifests, gates, and host tests
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
    |-- evidence/             dated implementation and run receipts
    `-- archive/              old prompts, audits, and handoffs
```

## Why Source Has Not Moved Yet

The root language files and flat `kernel/` source are not visually ideal, but
their paths are embedded in build scripts, `kernel/SOURCES`, host harnesses,
editor configuration, documentation, and self-hosting checks. Moving them as a
cosmetic pass would turn organization work into a broad build-system migration.

A later source migration should first introduce one authoritative source
manifest or build generator. Only then should the language implementation move
under `src/` or kernel files split by subsystem. The move and every consumer
must land together.

## Placement Rules

- Current queues and truth maps stay near `docs/README.md`.
- Proposals stay in `docs/design/`; a proposal is never completion evidence.
- Measured outputs worth retaining go in an `evidence/` directory.
- Superseded prompts, dated handoffs, and backup copies go in `archive/`.
- Generated binaries, disk images, screenshots, caches, and compiler output are
  ignored. Curated proof belongs under `docs/`, with a short receipt.
- Feature maturity lives in program ledgers and receipts, not in source-folder
  names such as `done/` or `half-done/`.

## Reference Patterns

The useful pattern is consistency, not copying another repository's names.
Zig keeps `src`, `lib`, `test`, `tools`, and `doc` separate; SerenityOS uses
clear product ownership such as `Kernel`, `Userland`, `Tests`, `Toolchain`, and
`Documentation`; Redox keeps `src`, `recipes`, `scripts`, `config`, and build
machinery distinct. The local cognitive-substrate project adds a strong
`START-HERE` plus purpose-based `docs/` subfolders. zl-linux follows the same
idea while preserving paths its current build still owns.
