# Code Map

This map describes the current `zl-linux` checkout. It is intentionally about
where things live, not what is complete.

## Source Areas

| Path | Status | What belongs here |
|---|---|---|
| `*.c`, `*.h`, `*.zl` at repo root | active | The zl language implementation: lexer, parser, interpreter, C backends, LLVM backend, native backend, runtime, and formatter. |
| `stdlib/` | active | Tracked zl library modules and algorithm examples. Some are real library pieces; some are still demo-shaped. |
| `tests/` | active | Tracked zl test programs. |
| `examples/` | active | Tracked runnable examples. Generated output belongs in `examples_out/`. |
| `kernel/` | active | zlOS: boot, drivers, memory, display, input, storage, network, compositor, apps, and gates. |
| `freestanding/` | active proof lane | No-libc generated-zl proof for the kernel track. |
| `bench/` | active tooling | Bench scripts and benchmark trees. |
| `tools/` | active tooling | Repo maintenance: doc checks, hazard scan, parity helpers, preflight, journal helpers. |
| `gates/` | active tooling | Landing gate wrapper logic. |
| `learn/` | active teaching | Small learning exercises and solutions. |
| `editors/` | active support | Editor integration, currently VS Code language support. |

## Documentation Areas

| Path | Status | What belongs here |
|---|---|---|
| `docs/README.md` | current index | Truth-state map for docs: current, partial, evidence, archived. |
| `docs/PROJECT-STATUS.md` | current status | Cross-repo commit state and the boundary between plan, integration, and implementation. |
| `docs/REPOSITORY-STRUCTURE.md` | current layout policy | Placement rules and the deferred source-migration contract. |
| `docs/STATE-OF-THE-PROJECT.md` | current but aging | Open-item audit. Keep hardware and QEMU claims separated. |
| `docs/EXECUTION-ROADMAP.md` | current but aging | Priority order. Treat later implementation notes as updates, not proof of physical hardware. |
| `docs/program/` | current program | Full feature/contract/phase program and partial-closure data. |
| `docs/design/` | mixed proposals | Design notes and proposals. A file here is not implemented unless its own status says so and current code confirms it. |
| `docs/fleet/` | audit boards | Review findings, boards, and verification logs. |
| `docs/evidence/` | measured history | Merge, pointer, and comparison records retained as evidence. |
| `docs/shots/` | curated evidence | Small visual evidence images that were intentionally kept. |
| `docs/archive/` | stale/history | Superseded plans, old prompts, backups, and documents kept for context only. |
| `kernel/docs/` | kernel records | Current kernel references and plans, with dated receipts in `evidence/` and old prompts/audits/handoffs in `archive/`. |

## Generated Output

These are not source and should stay out of Git unless deliberately curated as
evidence under `docs/`.

| Path | Source |
|---|---|
| `interp`, `compile`, `compilef`, `compilel`, `nativegen`, `zlfmt` | Built by `./build.sh`. |
| `out.c`, `outf.c`, `out.ll`, `native_out` | Compiler/backend output. |
| `examples_out/` | Example program artifacts. |
| `kernel/*.elf`, `kernel/*.img`, `kernel/*.iso`, `kernel/*.EFI` | zlOS boot/build artifacts. |
| `kernel/_gen*.c`, `kernel/out.c` | Generated C from zl kernel sources. |
| `kernel/shots/`, `kernel/exercise-out/`, `/shots/` | Visual/QEMU run output. |
| `kernel/hosttest/*` binaries | Host harness build output; the `.c` files are source. |

## What Not To Move Casually

Do not split `kernel/` source files into subdirectories as a cosmetic cleanup.
`kernel/SOURCES`, build scripts, docs, and gates assume the current paths. A real
kernel-source split should first add a source manifest or build generator, then
move files with the gates updated in the same change.

Do not move root language implementation files into `src/` just for neatness.
That can be a good cleanup later, but it needs every script, doc, editor config,
and self-hosting path updated together.

Do move generated artifacts out of Git, and do archive docs that already say
they are superseded.
