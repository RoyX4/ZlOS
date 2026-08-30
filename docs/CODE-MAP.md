# Code Map

This map describes the current `zl-linux` checkout. It is intentionally about
where things live, not what is complete.

## Source Areas

| Path | Status | What belongs here |
|---|---|---|
| `src/frontend/` | active | Shared C lexer, parser, token definitions, and AST definitions. |
| `src/runtime/` | active | Reference interpreter, compiled-program runtime, and Linux OS bridge. |
| `src/backends/c/` | active and archived lanes | Boxed C backend plus the archived unboxed-C experiment. |
| `src/backends/llvm/` | active speed lane | LLVM IR backend. |
| `src/backends/native/` | active and experimental lanes | Direct executable generators and their native runtime experiments. |
| `src/selfhost/` | bounded proof lane | Compiler, lexer, parser, and native work written in zl. This is not the full production toolchain. |
| `src/tools/` | active tooling | Language-aware source tools, currently `zlfmt`. |
| `stdlib/` | active | Tracked zl library modules and algorithm examples. Some are real library pieces; some are still demo-shaped. |
| `tests/` | active | Tracked zl test programs. |
| `examples/` | active | Tracked runnable examples. Generated output belongs in `examples_out/`. |
| `kernel/apps/` | active | zlOS application and game modules written in zl. |
| `kernel/boot/` | active | BIOS, raw, 64-bit, and UEFI entry code plus linker layouts. |
| `kernel/src/` | active | zlOS implementation grouped by architecture and subsystem. |
| `kernel/tests/` | active evidence | Host harnesses, visual oracle, reference renderer, and fixtures. |
| `kernel/tools/` | active tooling | Static checks, generators, image builders, runtime probes, and VM launchers. |
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
| `docs/REPOSITORY-STRUCTURE.md` | current layout policy | Placement and source-ownership rules. |
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
| `kernel/tests/host/*` binaries | Host harness build output; the `.c` files are source. |

## Structural Guardrail

Do not flatten `kernel/src/` again or add new implementation files directly to
`kernel/`. New code belongs with its owning subsystem, must be added to
`kernel/SOURCES` when shipped, and needs its consumers updated in the same
change. Generated artifacts stay out of Git; curated proof belongs under docs.
