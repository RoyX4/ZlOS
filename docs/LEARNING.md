# Learning zl

The fastest way to learn a language you wrote is to be a *user* of it — to hit
the gaps from the outside instead of the inside. This is that path: ten graded
exercises in `learn/`, each a real `.zl` file with stubbed functions and
`assert`s that fail until you fill them in.

## The loop

```bash
./build.sh                    # once — builds interp, compile, nativegen, zlfmt
learn/check.sh                # see what's still red
```

Open `learn/01_basics.zl` in VS Code, fill in the functions, then press
**Ctrl+Alt+R** to run it (keybinding in `~/.config/Code/User/keybindings.json`,
bound to the *run current .zl file (interpreter)* task). Or from a terminal:

```bash
./interp learn/01_basics.zl     # from the repo root
zl learn/01_basics.zl           # from anywhere — ~/.local/bin/zl wrapper
```

Note **Ctrl+Shift+B** is the *build toolchain* task (`./build.sh`), not run.

Silence plus `01 ok` means done. An `assertion failed: greet` names exactly
which assert broke. Move to `02`. Repeat.

Stuck? The worked answer is in `learn/solutions/` — but write yours first,
then diff. Reading a solution you didn't struggle for teaches nothing.

```bash
learn/check.sh 05             # run just exercise 05
learn/check.sh 05 --sol       # run the solution for 05
learn/check.sh "" --sol       # prove all solutions still pass
```

## The ladder

| # | File | What it drills |
|---|------|----------------|
| 01 | `01_basics.zl` | variables, `str()`, joining text with `+` |
| 02 | `02_branching.zl` | `if` / `else if` / `else`, `and` / `or` / `not`, `%` |
| 03 | `03_loops.zl` | `while`, `for ... in`, `break`, `continue`, `range()` |
| 04 | `04_recursion.zl` | functions calling themselves, `type()` |
| 05 | `05_lists.zl` | `push` / `remove` / `contains`, index assignment |
| 06 | `06_strings.zl` | `at` / `code` / `chr` / `split` / `join` / `index_at` |
| 07 | `07_grids.zl` | nested lists, `grid[y][x] = v`, Conway's Life |
| 08 | `08_stdlib.zl` | `import`, `mathx`, `dict`, `sortx` |
| 09 | `09_files.zl` | `read` / `write` / `lines`, CSV round-trip |
| 10 | `10_capstone_rpn.zl` | a stack machine — an RPN calculator with error handling |

After 10, the honest next step is not another exercise. It's
`examples/vm.zl` and `examples/raytracer.zl` — read them, then break them on
purpose and fix them. Then `stdlib/` (64 modules, all written in zl), then
`src/selfhost/compiler.zl` / `src/selfhost/lexer.zl` / `src/selfhost/parser.zl`, which is zl reading zl.

## Four things that will bite you

These are the places zl differs from what your fingers expect. Each one is a
deliberate design choice, not an omission.

**1. `else` must sit on the same line as the closing brace.**

```zl
if a { ... } else { ... }        # fine
if a { ... }
else { ... }                     # line N: expected a value (got 'else')
```

The parser reads a statement as complete at the `}`. A newline ends it. This
is the same rule Go enforces with semicolon insertion, arrived at from the
other direction.

**2. List builtins are pure. Index assignment is not.**

```zl
push(xs, 4)          # does nothing to xs — the result is thrown away
xs = push(xs, 4)     # this is what you meant
xs[0] = 99           # mutates in place
```

`push`, `remove`, `insert`, `concat`, `sort`, `reverse` all return a *new*
list. Only `xs[i] = v` writes through. If a loop is mysteriously not
accumulating, this is why, 90% of the time.

**3. Strings are not indexable.** `s[0]` is an error; strings are not lists.

```zl
at(s, i)          # the character at i, as a one-character string
index_at(s, i)    # its character code, or -1 out of range
code("a")         # code of the first character
chr(97)           # back to "a"
```

**4. A number is a double.** There is no integer type. That is exact up to
2^53 and the bitwise builtins (`band`, `shl`, …) *raise* rather than hand back
a silently-rounded answer above it. See `docs/REFERENCE.md` — the reasoning
there is worth reading before you write anything bit-twiddly.

## What the editor gives you

The VS Code / Cursor extension (`editors/vscode-zl`, installed as `zl.zl-lang`)
does syntax highlighting, keyword/builtin autocomplete (Ctrl+Space), snippets,
and format-on-demand through `zlfmt`. Cursor Tab also works once the file is
language **zl**, not Plain Text — install with `./editors/vscode-zl/install.sh cursor`. The repo's `.vscode/tasks.json` already defines:

- **build toolchain** — `./build.sh`
- **run tests** — `./run_tests.sh`
- **run current .zl file (interpreter)** — bound to Ctrl+Alt+R
- compile the current file through the C, LLVM, or native x86-64 backend

and `.vscode/launch.json` will drop gdb into `src/runtime/interp.c` with your `.zl` file as
the argument — which is the *other* way to learn this language: watch the
interpreter execute your own program, statement by statement.

## Where the answers live

- `docs/REFERENCE.md` — syntax, all 93 builtins, the stdlib table
- `stdlib/*.zl` — 64 modules of idiomatic zl, all readable in one sitting
- `examples/*.zl` — ten complete programs, raytracer through VM
- `tests/*.zl` — the language's own conformance tests; the spec in executable form
