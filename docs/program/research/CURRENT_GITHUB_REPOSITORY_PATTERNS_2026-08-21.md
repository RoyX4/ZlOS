# Current GitHub repository patterns worth taking for zl

**Snapshot:** 2026-08-21. This is a focused read of five current, high-signal
projects: four launched in 2026 and one mature systems project used as a control.
It compares their README, repository root, contributor/development documentation,
and declared verification path with this checkout's public onboarding surface.

This is not a popularity contest. Star counts only helped select projects that
many people are actively encountering. Claims about a project's design are based
on the linked source files; claims about performance remain the projects' own
claims unless their local harness was run here.

## Bottom line

zl already has the rare part: a concrete, impressive, machine-checkable claim.
`verify.ps1` can prove a self-hosting fixpoint and backend agreement. The newer
repositories do a better job making their equivalent first proof impossible to
miss, then progressively revealing the deeper material.

The best move is **not** to copy their branding, package manager, or directory
count. It is to make the first ten minutes of zl unambiguous:

```text
what it is -> install/build prerequisites -> run one program -> run the proof
-> find the exact document for development, architecture, and language reference
```

That would make the existing work legible to a stranger without flattening zl
into another ordinary compiler project.

## The projects inspected

| Project | Why it is here | Current evidence inspected | Pattern worth taking |
|---|---|---|---|
| [firecrawl/anydoc](https://github.com/firecrawl/anydoc) | Created 2026-08-03; Rust core with CLI, Node, Python, and WASM interfaces | root tree, [README](https://github.com/firecrawl/anydoc/blob/main/README.md), `bench/` | Start with runnable commands for every supported entry point; publish a benchmark method beside the result. |
| [PrimeIntellect-ai/prime-agent](https://github.com/PrimeIntellect-ai/prime-agent) | Created 2026-05-08; current agent tool with a multi-package tree | root tree, [README](https://github.com/PrimeIntellect-ai/prime-agent/blob/main/README.md), [AGENTS.md](https://github.com/PrimeIntellect-ai/prime-agent/blob/main/AGENTS.md), development guide | Separate user install from contributor build; put machine-readable working rules at the root; verify release downloads. |
| [rtk-ai/rtk](https://github.com/rtk-ai/rtk) | Created 2026-01-22; a single Rust CLI distributed to real users | root tree, [README](https://github.com/rtk-ai/rtk/blob/master/README.md), [INSTALL.md](https://github.com/rtk-ai/rtk/blob/master/INSTALL.md), architecture guide | Put a platform-aware install matrix and a verification command immediately after installation; keep deeper architecture one click away. |
| [Graphify-Labs/graphify](https://github.com/Graphify-Labs/graphify) | Created 2026-04-03; documentation-heavy code-intelligence tool | root tree, [README](https://github.com/Graphify-Labs/graphify/blob/main/README.md), [ARCHITECTURE.md](https://github.com/Graphify-Labs/graphify/blob/main/ARCHITECTURE.md) | Treat architecture prose as a contract: state module boundaries, data shape, and test the document's symbols so it cannot silently drift. |
| [ghostty-org/ghostty](https://github.com/ghostty-org/ghostty) | Mature, actively developed native systems application; deliberately a control rather than a new project | root tree, [README](https://github.com/ghostty-org/ghostty/blob/main/README.md), [HACKING.md](https://github.com/ghostty-org/ghostty/blob/main/HACKING.md) | Keep README user-facing; give contributors one definitive development document with build, run, tests, platform requirements, linting, and debugging. |

The metadata snapshot was taken from each repository's GitHub API on 2026-08-21:
anydoc was created 2026-08-03, prime-agent 2026-05-08, rtk 2026-01-22, and
graphify 2026-04-03. They were all pushed to within roughly a day of the
snapshot. That makes them genuinely current examples, but not proof that every
claim in their README is independently validated.

## What they are doing that maps directly to zl

### 1. The first successful command is obvious

anydoc gives a runnable CLI command, then equally concrete Node, Python, WASM,
and Rust examples. rtk follows installation with a named verification step and
a short "Quick Start." Ghostty's README stays at the product level and sends a
future contributor directly to `HACKING.md` rather than mixing every build
detail into its landing page.

**zl similarity:** the repository already has real commands, not aspirational
ones: `build.bat`, `interp.exe`, `run_tests.ps1`, and `verify.ps1`.

**zl gap:** the public README's build block does not yet lead a newcomer from a
fresh Windows machine to one source file running and then to the green proof.
It does not name the Visual Studio/MSVC prerequisite, distinguish the current
Windows path from unsupported paths, or put `verify.ps1` in the normal
onboarding sequence. A reader sees the trophy before they see the shortest
route to reproduce it.

**Take:** add one compact `Quick start (Windows)` section above the architecture
diagram:

```powershell
git clone https://github.com/RoyX4/zl
cd zl
cmd /c build.bat
.\interp.exe hello.zl
.\verify.ps1       # must finish with VERIFY: GREEN
```

The final command must match the current script's actual success line before it
is published. Add one sentence naming the exact Visual Studio component and one
link to the contributor document for the full toolchain. Do not invent a Linux
install story until it exists.

### 2. The README is a front door, not the entire house

The newer projects use a clear depth ladder:

```text
README: product, install, first run, proof, links
  -> development/contributing: source build, tests, release process
  -> architecture: module boundaries and invariants
  -> reference: full API or language detail
```

Prime Agent's README links separately to quickstart, CLI reference, skills,
providers, and development. Ghostty does the same for contributing and hacking.
Graphify's architecture document names each module's input/output contract,
instead of asking a contributor to discover its shape by reading the whole tree.

**zl similarity:** this material already exists: `README.md`, `HANDOFF.md`,
`MASTER_PLAN.md`, `ROADMAP.md`, `docs/REFERENCE.md`, design documents, benchmark
notes, and the verification gate.

**zl gap:** their ownership overlaps and their dates/status claims differ.
`START_NEW_CHAT.md`, `HANDOFF.md`, `MASTER_PLAN.md`, and `ROADMAP.md` were
written for different moments and audiences. A newcomer cannot safely infer the
canonical document for current work from their names alone.

**Take:** publish a tiny canonical doc map, not a rewrite of the plans:

| Need | One canonical place |
|---|---|
| What zl is; install; first run; proof | `README.md` |
| Exact language semantics and standard library | `docs/REFERENCE.md` |
| Current priority and deferred work | one current roadmap/handoff document |
| Long-lived choices and full-stack thesis | `MASTER_PLAN.md` |
| How to change, build, test, and avoid stale binaries | `CONTRIBUTING.md` or `HACKING.md` |
| Architecture/module boundaries | `docs/ARCHITECTURE.md` |

Leave historical handoffs available, but mark them historical in their title
and point back to the current owner. This is documentation routing, not a
proposal to delete the full plans.

### 3. The claim, the evidence, and its limitation sit together

anydoc places performance tables next to a link to its benchmark harness and
explains its corpus/measurement limits. Prime Agent states that its installer
downloads a versioned release and verifies SHA-256. Ghostty has named commands
for debug build, unit tests, a focused test filter, and a Valgrind run.

**zl similarity:** `verify.ps1` is stronger evidence than almost every
"self-hosting" README claim: its comparison is a property checked in the
current run (`gen1 == gen2`), not a stale hash. `run_tests.ps1` gives cross-engine
agreement. The repository also records measured benchmark results.

**zl gap:** the evidence boundary is scattered. The README says both what works
and how to build, but its simple test section does not tell a new contributor
which command is the release gate, what it validates, or what it does *not*
validate (for example, a backend subset is not complete-language parity).

**Take:** add a small, brutally exact proof table:

| Claim | Command | Pass condition | Does not prove |
|---|---|---|---|
| Compiler fixpoint | `verify.ps1` | fresh `gen1.c == gen2.c` | every language feature is correct |
| Backend agreement | `run_tests.ps1` | every declared case agrees | untested programs / missing backend features |
| Performance | benchmark harness | recorded command + machine | performance on every machine/program |

This protects the strongest thing zl has: it makes the headline reproducible
without overstating it.

### 4. Machine-readable contributor constraints are now normal repository UX

Prime Agent, Graphify, and Ghostty all place `AGENTS.md` at the repository root.
Prime Agent's file spells out permitted checks, prohibited commands, test
location, protocol compatibility, dependency policy, and changelog rules.
Ghostty additionally supplies vetted agent prompts and states an AI disclosure
policy in its development guide.

**zl similarity:** its key constraints are already known and valuable: do not
trust a stale Windows binary, preserve the self-hosting fixpoint, compare
compiled output to the interpreter, and keep mirrored implementations in sync.

**zl gap:** those constraints live in handoffs and global local configuration,
so a fresh clone or an outside contributor does not inherit them.

**Take:** add a short repo-local `AGENTS.md` only after the canonical doc map is
chosen. It should link out rather than duplicate policy, and contain the facts
that make a bad change likely:

- primary supported build environment and the stale-binary check;
- exact required gate after compiler/runtime changes;
- interpreter-as-oracle and byte-for-byte comparison rule;
- mirrored-source pairs and ownership/concurrency rule;
- explicit scope for generated files and artifacts.

This is not agent-specific ceremony. It is a way to preserve hard-won project
knowledge for humans, future sessions, and tools.

### 5. Architecture docs become useful when they name seams, not aspirations

Graphify documents a pipeline, names each module's input/output shape, calls
out the most likely misuse, and has a test that imports every documented symbol.
anydoc maps several document formats into a shared model, then renders through
one serializer. Ghostty distinguishes its public user-facing product from
development tooling and platform packages.

**zl similarity:** its main seams already exist: lexer, parser, interpreter,
compiler backends, runtime, OS layer, self-hosting compiler, standard library,
and test/proof scripts.

**Take:** one architecture page should show these flows and boundaries, then
link to source rather than narrating implementation line-by-line:

```text
.zl -> lexer -> AST/parser -> interpreter (semantic oracle)
                       |-> C / LLVM / x86-64 emitters -> executable
                       |-> compiler.zl -> generated compiler -> fixpoint check

runtime and interpreter: semantic parity boundary
os_*: platform boundary
tests + verify.ps1: evidence boundary
```

The useful invariant is not "there are five backends." It is "a language change
must either be implemented and checked across every supported execution path,
or each unsupported path must refuse it loudly." That is the architecture rule
new contributors need.

## What not to copy

- Do not turn zl into a TypeScript monorepo or add a package manager just because
  the surveyed projects have one. Their distribution shapes solve their problems,
  not zl's.
- Do not claim a one-line cross-platform install. The checked public path is
  Windows/MSVC; an honest Windows quickstart beats a false universal installer.
- Do not replace the self-hosting proof with badges, screenshots, or a fixed
  hash. The fresh fixpoint property is the better evidence.
- Do not make `AGENTS.md` a second roadmap. Its job is to prevent predictable
  damage and route a contributor to the authoritative documents.
- Do not copy benchmark numbers. Copy anydoc's habit of recording method,
  hardware, corpus limits, and the harness location.

## Recommended order

1. **README quickstart and proof block.** Smallest change with the largest
   improvement to a stranger's first hour. Verify every listed command on a
   clean Windows shell before publication.
2. **One document map plus a contributor guide.** Decide current ownership;
   replace duplicated instructions with links. Mark superseded handoffs clearly,
   do not erase their historical reasoning.
3. **Short repository `AGENTS.md`.** Distill the real gates and parity hazards
   after their owners are settled.
4. **Architecture page with enforced anchors.** Start with the pipeline and
   invariants. Later, add a lightweight check that the listed source files and
   commands still exist, following Graphify's anti-drift idea.
5. **Distribution only after the above.** A release artifact, package manager,
   or installer is worthwhile when it can invoke the same verification story and
   tell a new user exactly what is supported.

## Evidence links

- [anydoc README and benchmark method](https://github.com/firecrawl/anydoc/blob/main/README.md)
- [Prime Agent README](https://github.com/PrimeIntellect-ai/prime-agent/blob/main/README.md) and [root agent rules](https://github.com/PrimeIntellect-ai/prime-agent/blob/main/AGENTS.md)
- [rtk README](https://github.com/rtk-ai/rtk/blob/master/README.md) and [install guide](https://github.com/rtk-ai/rtk/blob/master/INSTALL.md)
- [Graphify README](https://github.com/Graphify-Labs/graphify/blob/main/README.md) and [architecture contract](https://github.com/Graphify-Labs/graphify/blob/main/ARCHITECTURE.md)
- [Ghostty README](https://github.com/ghostty-org/ghostty/blob/main/README.md) and [development guide](https://github.com/ghostty-org/ghostty/blob/main/HACKING.md)
