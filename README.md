# zl-linux — the zl language, ported to Linux

A full Linux port of [`zl`](https://github.com/RoyX4/zl), a self-hosting
systems language with three independent execution engines. Every engine
that worked on Windows now works natively on Linux, with no C compiler
required for the hand-assembled path and no Windows/Wine/emulation
anywhere in the chain.

```
zl source ─► lexer ─► parser ─► ┬─ interpreter           (interp)
                                ├─ C backend    ─► out.c  ─► gcc   ─► native ELF
                                ├─ LLVM backend ─► out.ll ─► clang ─► native ELF
                                └─ x86-64 backend ──────────────────► native ELF   (NO C compiler)
```

## What changed from the Windows original

The language, grammar, lexer, parser, and value semantics are untouched -
this is a platform port, not a rewrite. Three things had to change:

1. **`os_linux.c`** replaces `os_win.c`: `os_dir`/`os_procs` now walk
   `/proc` and `dirent.h` instead of calling Win32.
2. **Real `kill`/`start`/`rm`/`copy`/`move`/`run`** in `runtime.c` and
   `interp.c`: these were simulated (`[sim] kill(...)`) even on Windows.
   They're wired to real Linux syscalls now (`fork`/`execvp`, `SIGTERM`,
   `remove`/`rename`, `system`). `poke`/`peek`/`window` stay simulated -
   see the comment above `SIMULATED[]` in `runtime.c` for why.
3. **`nativegen.c`'s output format**: the hand-assembled x86-64 backend
   used to emit a Windows PE `.exe` that imported `GetStdHandle`/
   `WriteFile`/`ExitProcess` from `kernel32.dll`. It now emits a static
   ELF64 executable that issues raw Linux syscalls (`write`, `exit`)
   directly - no import table needed at all, which is actually *simpler*
   than the PE version. Verified with `strace`: the only syscalls a
   compiled program makes are the ones it actually asked for.

`compilel.c` (the LLVM backend) also needed zero platform changes - it
emits LLVM IR, which is platform-neutral; it just needed `clang`/`llvm`
present and a Linux target triple, which clang supplies by default.

`compile.c` (the C-backend code generator) needed zero platform changes -
it was already portable C, it just used to be paired with `cl.exe` and
`os_win.c` instead of `gcc` and `os_linux.c`. While cross-checking it
against the interpreter, one real (pre-existing, non-platform) bug turned
up: list literals built from side-effecting calls,
e.g. `[dq_pop_front(q), dq_pop_front(q)]`, depend on left-to-right argument
evaluation, which C does not guarantee - gcc's order differs from MSVC's
here. Fixed by sequencing arguments through temporaries in a
statement-expression (see `emit_seq_call` in `compile.c`).

## Build

```
./build.sh
```

Builds `interp`, `compile`, `nativegen`, and the standalone `lexer`/`parser`
demo binaries. Needs `gcc` and `libm` - nothing else.

## Use

```bash
./interp program.zl                    # run directly

./compile program.zl                   # -> out.c
gcc -O2 -D_strdup=strdup -o program out.c runtime.c os_linux.c -lm

./compilel program.zl                  # -> out.ll (unboxed subset)
clang -O2 out.ll -o program

./nativegen program.zl                 # integer subset only -> ./native_out
```

`nativegen`'s integer subset: variables, `+ - * / %`, comparisons,
`and`/`or`/`not`, `if`/`else`, `while`, `break`/`continue`, user functions +
recursion, `print`, and `exit(code)`. No strings-as-values, no lists, no
real-PC-control builtins - the interpreter and C backend have the full
language, this one trades completeness for "no C compiler in the loop at
all."

## First-class functions in the C backend (new)

Passing a function as a value — `ix_sort_by(counts, neg_count)`, then
calling `key(x)` inside the callee — worked in the interpreter from the
start (its `V_FN` points at the AST node) but was never implemented in the
C backend: it emitted a function name as if it were a variable, so
`examples/wordfreq.zl` and `examples/texttools.zl` failed to compile with
`'v_neg_count' undeclared`. Verified pre-existing against the original
unmodified `compile.c`, not a port regression.

Now supported: `Value` gained `fnptr`/`fnargs`, `zl_fn()` captures a
compiled function as a value, and `zl_callv()` casts back to the arity the
call site uses. `compile.c` tracks the names bound in each scope, so a call
to a *variable* holding a function goes indirect while a call to a known
function stays a direct C call. Both examples now compile and match the
interpreter byte-for-byte.

This grew `Value` from 48 to 64 bytes; `compilel.c`'s `VALSZ` was updated
to match, which `runtime.c`'s static assert enforces.

## Editor support (VS Code)

`editors/vscode-zl/` is the syntax-highlighting extension (carried over
from the original repo, install instructions now cover Linux). Install it
with:

```bash
cp -r editors/vscode-zl ~/.vscode/extensions/vscode-zl
```

`.vscode/` in this repo adds, for anyone who opens the folder:

- **tasks** — build the toolchain, run the test suite, and compile/run the
  open `.zl` file through any of the three backends (Ctrl+Shift+B)
- **launch configs** — gdb on `interp`/`compile`/`nativegen`, plus one for
  stepping through a `nativegen`-produced raw ELF in Intel-syntax
  disassembly (no symbols, since it hand-emits its own machine code)
- **IntelliSense/clangd config** — `build.sh` generates
  `compile_commands.json` with the same `BUILD_PARSER`/`BUILD_INTERP`/
  `_strdup` defines the real build uses, so the editor doesn't show
  phantom errors. Verified: `clangd --check` reports 0 diagnostics on every
  source file.

## Test

```
./run_tests.sh
```

Runs the full `tests/*.zl` suite (2,133 assertions) through the
interpreter, cross-checks the C backend produces byte-identical output for
every one of them, runs every `examples/*.zl` program and cross-checks the
deterministic ones against a compiled build, and cross-checks the LLVM and
native x86-64 backends against the interpreter on subset smoke tests. All
green on a clean Kali Linux build. Needs `gcc`; `clang`/`llvm` for the LLVM
stage (skipped with a notice if absent).

`examples/raytracer.zl` and `examples/mandelbrot.zl` write real BMPs into
`examples_out/` — a genuine raytracer with reflections and shadows, written
in zl, rendering on Linux.

## What's not ported

`nativert.c` (1,534 lines) - the deeper "no-C-runtime" floor of the
original project's `MASTER_PLAN.md`: a hand-assembled heap allocator, file
I/O, and a self-hosting compile path with zero libc dependency, all
targeting Windows/kernel32 directly. That's a much larger, more
experimental undertaking than `nativegen.c` was (which only touched Win32
for two tiny print routines) and was left as a documented next step rather
than risking a fragile blind port of 1,500 lines of intricate hand-tuned
machine code with no interactive debugger in the loop. `nativegen.c` (the
backend actually described above) *is* fully ported and tested.

`poke`/`peek`/`window` stay simulated - `poke`/`peek` would mean patching
another live process's memory (ptrace injection), which is a different
kind of feature than "run this language on Linux," and `window` is a
GUI-toolkit builtin with no Linux equivalent implemented here.
