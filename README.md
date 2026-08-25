# zl-linux — the zl language, and an operating system written in it

Two things live here.

> **Complete 2026-08-24 zl-linux integration:**
> [`docs/evidence/integration/MERGE-ALL-2026-08-24.md`](docs/evidence/integration/MERGE-ALL-2026-08-24.md) records the
> exact remote branch, local branch, registered-worktree and standalone-clone
> closure. The post-push audit found one extra divergent local Product Wave
> branch and merged it too. After that pass, every audited zl-linux branch and
> worktree head is an ancestor of the integration head and every audited
> worktree/clone is clean. The old directories and branch names still exist;
> they contain no unique unmerged work. The separate `RoyX4/zl` repository is
> deliberately out of scope.

**`zl`** is a self-hosting systems language with five independent ways to run a
program, fully ported from Windows to Linux. No Wine, no emulation, and no C
compiler at all on the hand-assembled path.

**`zlOS`** is an operating system written *in* zl, in `kernel/`, that boots on
x86-64 with nothing underneath it — its own bootloader, its own UEFI
application, real PCI enumeration, Intel Gen9 modesetting, xHCI/USB HID input,
NVMe, a filesystem, and a windowed compositor.

> **Where the project actually is:** [`docs/PROJECT-STATUS.md`](docs/PROJECT-STATUS.md).
> It separates the pushed integration state, the complete implementation plan,
> the other-repository research, and what remains unimplemented or physical.
> The older [`STATE-OF-THE-PROJECT.md`](docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) remains a
> detailed dated audit, not the final post-integration status.

> **Current execution order:** [`docs/EXECUTION-ROADMAP.md`](docs/EXECUTION-ROADMAP.md).
> Speed is the primary product goal. The dependency spine is bare-metal proof,
> persistent USB boot evidence, measured latency work, durable files, 64-bit
> processes, real networking and driver depth, then application/visual polish.

> **Historical scorecard:**
> [`docs/archive/superseded/ROAD-TO-TEN.md`](docs/archive/superseded/ROAD-TO-TEN.md).
> Its measurements are retained, but its ranking was superseded by the execution
> roadmap and later integration work.

Five more orientation docs worth knowing about:

- [`docs/README.md`](docs/README.md) — documentation truth states.
- [`docs/CODE-MAP.md`](docs/CODE-MAP.md) — current source ownership.
- [`docs/REPOSITORY-STRUCTURE.md`](docs/REPOSITORY-STRUCTURE.md) — placement rules.
- [`src/README.md`](src/README.md) — language-toolchain ownership.
- [`docs/GUARDS-THAT-DID-NOT-GUARD.md`](docs/GUARDS-THAT-DID-NOT-GUARD.md) — five
  checks in this tree that reported green while checking nothing. Read it before
  trusting any green result here, and before writing a new gate.

## The five ways to run

They are not equals. Two are archived, one is the reference, one is the active
speed path:

| # | Tool | Path | Status |
|---|------|------|--------|
| 1 | `interp` | tree-walking interpreter | **The reference.** Whatever it does is correct; runs the whole language |
| 2 | `compile` | zl → boxed C → gcc | **Archived — do not develop** |
| 3 | `compilef` | zl → unboxed C → gcc | **Archived.** The proof-of-concept that unboxing is the win |
| 4 | `compilel` | zl → LLVM IR → clang | **The speed backend.** The active one |
| 5 | `nativegen` | zl → x86-64 machine code → ELF | Hand-written, no C compiler in the output |

Measured on this machine, `fib(28)`:

| # | Way to run | time | vs interp |
|---|---|---|---|
| 1 | `interp` (reference) | 632 ms | 1× |
| 2 | `compile` boxed C *(archived)* | 65 ms | 10× |
| 3 | `compilef` unboxed C *(archived)* | **2 ms** | **316×** |
| 4 | `compilel` LLVM *(speed backend)* | 3 ms | 210× |
| 5 | `nativegen` hand-written x86-64 | 6 ms | 105× |

All five build and run on Linux, and all five agree with the interpreter.

The interpreter being slowest is the point: it is the thing that defines what the
language *means*, so when a backend disagrees with it, the backend is wrong.

## Build

```bash
./build.sh
```

Builds eight binaries: `interp`, `compile`, `compilef`, `compilel`, `nativegen`,
`zlfmt`, and the standalone `lexer_demo` / `parser_demo`. Needs `gcc` and `libm`
to build. `clang`/`llvm` are needed only to *use* `compilel`'s output, not to
build it.

It also generates `compile_commands.json` with the same `BUILD_PARSER` /
`BUILD_INTERP` / `_strdup` defines the real build uses, so clangd does not show
phantom errors.

## Use

```bash
./interp program.zl                    # run directly

./compile program.zl                   # -> out.c
gcc -O2 -D_strdup=strdup -Isrc/runtime -o program out.c \
    src/runtime/runtime.c src/runtime/os_linux.c -lm

./compilel program.zl                  # -> out.ll (unboxed subset)
clang -O2 out.ll -o program

./nativegen program.zl                 # integer subset only -> ./native_out
```

`nativegen`'s integer subset: variables, `+ - * / %`, comparisons, `and`/`or`/`not`,
`if`/`else`, `while`, `break`/`continue`, user functions and recursion, `print`,
and `exit(code)`. No strings-as-values, no lists, no real-PC-control builtins. The
interpreter and the C backend have the full language; this one trades completeness
for having no C compiler in the loop at all.

## zlOS

`kernel/` is an operating system written in zl. The constraint that makes it
interesting is that there is no layer to call into — every one of these had to be
written rather than used:

| layer | what that meant |
|---|---|
| Boot | Own bootloader, own UEFI application, long mode entry, page tables, memory map |
| Display | PCI probe, BGA and Intel Gen9 modesetting, pixel clock derived from the link M/N registers |
| Input | xHCI host controller and a USB HID driver, including the firmware handoff |
| Storage | NVMe queues, and `zlfs`, which survives power cycles |
| Runtime | APIC, scheduler, and a windowed compositor with eight apps |

### Gates

Everything here boots QEMU under TCG, so speed depends on host load. Start these
in the background rather than blocking on them.

| Command | Roughly | What it proves |
|---|---|---|
| `kernel/verify.sh` | ~1 min | BIOS boot against a golden transcript |
| `kernel/tools/checks/verify-raw.sh` | 1-3 min | Our own bootloader; polls for its marker |
| `kernel/tools/checks/verify-iso.sh` | ~1.5 min | BIOS **and** UEFI through GRUB |
| `kernel/tools/checks/verify-efi.sh` | ~1 min | zlOS as its **own** UEFI application — the laptop's real path |

`verify-efi.sh` exists because the other three were all green while the 64-bit
build was dead: the first two boot the 32-bit kernel, and `verify-iso.sh`'s "UEFI"
case boots *GRUB's* `bootx64.efi`, which multiboot-loads that same 32-bit kernel.
Nothing exercised `kernel/boot/efi.c` or the path a real machine takes. Run
`verify-efi.sh` before believing a change is safe on hardware.

**Do not run several QEMU instances alongside a fan-out of agents.** This box has
4 cores and 15 GB; doing so has produced a gate reporting a regression that did
not exist, and an OOM kill of the agent process itself. Check
`cut -d' ' -f1-3 /proc/loadavg` first.

### Two hazards

**`kernel/src/drivers/display/intel.c` can damage hardware**, not merely fail — violating the panel's
500 ms T12 power-cycle delay, or driving AUX into an unpowered panel. Develop it
from Linux userspace against the live GPU via `kernel/tests/host/`, which is seconds
per iteration with no reboots. Map 8 MiB of BAR0, not 16.

**Never put a pointer through `unsigned long` in the EFI build.**
`kernel/buildefi.sh` targets `x86_64-unknown-windows`, which is LLP64:
`unsigned long` is 4 bytes there and 8 everywhere else. This bit twice, and below 4 GiB the truncation is harmless,
which is exactly why QEMU never showed it. `kernel/tools/checks/wguard.sh` is the check and it
runs in all three directions. Details in [`CLAUDE.md`](CLAUDE.md).

## What changed from the Windows original

The language, grammar, lexer, parser and value semantics are untouched — this is a
platform port, not a rewrite. Three things had to change:

1. **`src/runtime/os_linux.c`** replaces `os_win.c`: `os_dir` / `os_procs` walk `/proc` and
   `dirent.h` instead of calling Win32.
2. **Real `kill`/`start`/`rm`/`copy`/`move`/`run`** in `src/runtime/runtime.c` and `src/runtime/interp.c`.
   These were simulated (`[sim] kill(...)`) even on Windows; they are wired to
   real syscalls now (`fork`/`execvp`, `SIGTERM`, `remove`/`rename`, `system`).
3. **`src/backends/native/nativegen.c`'s output format.** It used to emit a Windows PE that imported
   `GetStdHandle`/`WriteFile`/`ExitProcess` from `kernel32.dll`. It now emits a
   static ELF64 that issues raw Linux syscalls directly — no import table at all,
   which is *simpler* than the PE version. Verified with `strace`: the only
   syscalls a compiled program makes are the ones it asked for.

`src/backends/llvm/compilel.c` needed zero platform changes — LLVM IR is platform-neutral.
`src/backends/c/compile.c` needed zero platform changes either, but cross-checking it against the
interpreter surfaced one real pre-existing bug: list literals built from
side-effecting calls, e.g. `[dq_pop_front(q), dq_pop_front(q)]`, depend on
left-to-right argument evaluation, which C does not guarantee — gcc's order
differs from MSVC's. Fixed by sequencing through temporaries in a
statement-expression (`emit_seq_call` in `src/backends/c/compile.c`).

## Test

```bash
./run_tests.sh
```

Runs the full `tests/*.zl` suite through the interpreter (2,133 assertions, as
counted by `report()` at runtime), cross-checks the C backend produces
byte-identical output for every one, runs every `examples/*.zl` program and
cross-checks the deterministic ones, and cross-checks the LLVM and native x86-64
backends against the interpreter on subset smoke tests. Needs `gcc`; the LLVM
stage is skipped with a notice if `clang` is absent.

`examples/raytracer.zl` and `examples/mandelbrot.zl` write real BMPs into
`examples_out/` — a raytracer with reflections and shadows, written in zl.

## Formatting

```bash
./zlfmt file.zl              # formatted, to stdout
./zlfmt --write file.zl      # rewrite in place
./zlfmt --check stdlib/*.zl  # exit 1 and list files that need formatting
./verify_fmt.sh              # prove it cannot damage a file
```

`zlfmt` re-indents; it does not reformat. It rewrites leading whitespace, strips
trailing whitespace, and copies every other byte through untouched. That restraint
is load-bearing: `src/frontend/lexer.c:272-273` discards comments and `src/frontend/lexer.c:88` truncates
token text at 128 bytes, so any formatter that *rebuilt* source from the token
stream would delete every comment in the corpus and silently corrupt long string
literals. `verify_fmt.sh` proves the token stream is byte-identical before and
after, line numbers included.

Of the 324 `.zl` files in the tree, 25 currently have inconsistent indentation
(`./zlfmt --check`, 2026-08-19).

## Learning the language

Ten graded exercises with failing asserts live in `learn/`, solutions in
`learn/solutions/`:

```bash
learn/check.sh          # what is still red
./interp learn/01_basics.zl
```

[`docs/LEARNING.md`](docs/LEARNING.md) has the ladder and the four places zl will
bite you: `else` placement, pure list builtins, non-indexable strings, and
doubles-only numbers.

## Editor support

`editors/vscode-zl/` is the VS Code extension — syntax highlighting, formatting
via `zlfmt`, snippets, and running files (`Ctrl+F5` interpreter,
`Ctrl+Shift+F5` LLVM backend).

```bash
./editors/vscode-zl/install.sh
```

**Do not install it by copying the folder into `~/.vscode/extensions/`.** Current
VS Code loads only what is listed in `extensions.json`, and a folder-drop never
gets an entry, so the extension is silently ignored and `.zl` files stay "Plain
Text" with no error explaining it. `code --install-extension` is what writes that
entry.

`.vscode/` also ships build tasks, gdb launch configs for `interp`/`compile`/
`nativegen` (including one for stepping a `nativegen`-produced raw ELF in
Intel-syntax disassembly, since it has no symbols), and clangd config.

## What is not ported

`src/backends/native/nativert.c` (1,534 lines) — the deeper "no C runtime" floor: a hand-assembled
heap allocator, file I/O and a self-hosting compile path with zero libc
dependency, all targeting Windows/kernel32 directly. Left as a documented next
step rather than a fragile blind port of 1,500 lines of intricate hand-tuned
machine code with no interactive debugger in the loop. `src/backends/native/nativegen.c` *is* fully
ported and tested.

`poke` / `peek` / `window` stay simulated. `poke`/`peek` would mean patching
another live process's memory via ptrace injection, which is a different feature
from "run this language on Linux", and `window` is a GUI-toolkit builtin with no
Linux equivalent here.

## Comparison

[`docs/evidence/COMPARE-BOREDOS.md`](docs/evidence/COMPARE-BOREDOS.md) measures zlOS against
[BoredOS](https://boredos.dev) — same target, opposite strategy — and is blunt
about where BoredOS is ahead.

## Repository preservation audit

[`docs/evidence/integration/GIT-WORKTREE-AUDIT-2026-08-24.md`](docs/evidence/integration/GIT-WORKTREE-AUDIT-2026-08-24.md)
records the exhaustive ZL-only worktree audit, the preservation branches pushed
to GitHub, the evidence archives, and the verification boundary for each branch.
