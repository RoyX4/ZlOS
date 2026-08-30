# What zl still needs to be a finished language

Written 2026-08-02. This is the complete gap list — the reference for "what is left",
kept in one place so it does not have to be re-derived. Every claim here was checked
against the source or measured, not assumed. Where something is uncertain it says so.

The decision this document serves: **MASTER_PLAN §10 (2026-08-02) — language
completeness before optimisation.** Speed is already solved where the language reaches
(`compilel` matches `clang -O2` on integer benchmarks). Coverage is the ceiling.

---

## Where zl stands

**Has:** 93 builtins · 96 stdlib modules · 5,370 lines of engine C · 664-line
self-hosted compiler · 5 engines (interpreter, boxed C, unboxed C, LLVM IR, own x86-64)
· self-hosting fixpoint · 2,107 passing checks · 6 working examples.

**Reference point:** Zig's self-hosted compiler is ~197,500 lines. That is the honest
size of a complete self-hosted systems language, and the number to measure against —
not LLVM's ~30 million. See `reference_compiler_sizes` in memory.

---

## Tier 0 — the two that block everything else

### 1. No imports

`stdlib/astar.zl` contains no `import`, `include` or `use`. Nothing does. **Every .zl
file is standalone.** To use a stdlib module you copy its source into your file.

Evidence: `tests/test_algorithms.zl` is 37 KB because `stdlib/sortx.zl` is pasted
inside it. When the circular-sort-oracle bug was fixed on 2026-08-02 it had to be fixed
in **both copies** — that is the tax, already paid once.

So the "96 stdlib modules" are not a library. They are 96 standalone programs that get
copied from. One feature turns them into a real standard library.

Estimated cost: a keyword, a module search path, and a name-collision rule. Likely a
few hundred lines. **Highest work-to-value ratio of anything in this document.**

### 2. No memory management

Counted 2026-08-02: `src/runtime/interp.c` has **61 mallocs and 15 frees**; `src/runtime/runtime.c` has **59
and 12**. No garbage collector, no reference counting, no arena — grepped for all
three, none present.

zl leaks by construction. Irrelevant for short programs (process exit reclaims
everything) and **fatal for anything long-running**: a server, a game loop, a REPL, or
a compiler processing many files. It is also why `examples/maze.zl` reached an 11 GB
working set before the tip-tracking fix in `77a577e`.

This one has no cheap answer. The options — GC, refcounting, arenas, or manual
alloc/free with `defer` — are a language-identity decision, not an implementation
detail, and MASTER_PLAN's "power over safety" identity argues for the manual end.
It should be decided before the systems tier (Floor 5), because FFI and raw memory
both assume an answer.

---

## Tier 1 — describing data

| Missing | What you write today instead |
|---|---|
| **Structs / records** | `p = [10, 20]` and remember which index is `x` — ~23 files do this |
| **Maps** | a list of `[key, value]` pairs, scanned linearly. `stdlib/dijkstra.zl` says so in a comment: *"No maps in zl"* |
| **Tuples** | same as structs |
| **Enums** | bare numbers, and you remember what `2` meant |
| **Optionals** | `nil()` as a sentinel — 13 files, 33 functions |
| **Sized integers** (i8–i64, u8–u64) | one number type, stored as a double |
| **Slices** | `slice()`, which copies |

Designs in flight: `design_records.md`, `design_maps_v2.md`, `design_nullable.md`,
`design_sized_integers.md`.

**The key measurement:** 51 of 111 .zl files use heterogeneous lists, but only **8** are
irreducibly dynamic. The other ~43 are structs, tuples and generics with no type to
express them. Adding records/maps/optionals/generics shrinks the type system's job from
51 files to 8.

---

## Tier 2 — failing

**There is no way to fail in zl.** No exceptions, no error returns, no `Result`, no
`try`. A function that fails returns `nil()` and hopes the caller checks. A builtin
that fails kills the process.

Direct cost already paid: ~20 test assertions were **deleted** on 2026-08-02 because the
engine now correctly raises on those inputs and there is no way to write
`check(this expression errors)`. Every hardening fix that converts silently-wrong into
a raise destroys coverage until this exists.

Design in flight: `design_error_handling.md`.

---

## Tier 3 — everyday language features

| Missing | Note |
|---|---|
| `switch` / `match` | `if`/`elif` chains instead. Design exists, being revised |
| **Closures / functions as values** | `stdlib/sortx.zl` hardcodes its comparison because a comparator cannot be passed |
| **Methods** | everything is a loose function |
| **Multiple return values** | return a list, unpack by index |
| **Default and named arguments** | every call passes everything |
| **Varargs** | grepped `src/frontend/parser.c` — no support |
| **Constants** | everything is mutable |
| **Multi-line list literals** | `stdlib/astar.zl` writes a grid one row at a time and says why in a comment |
| **Shadowing** | no `let`, so no shadowing — and see the scoping problem below |
| **Iterators / generators** | `for x in y` works on lists only; you cannot make your own iterable |

---

## Tier 4 — power

| Missing | Why it matters |
|---|---|
| **`comptime`** | Zig's killer feature, and zl is unusually well placed — the interpreter is *in the same binary* as the compiler, so `eval()` is already there. It is also the mechanism Floor 2 needs (AI compiles English threat descriptions into frozen, readable rules) |
| **`defer`** | cleanup that cannot be forgotten — pairs with whatever memory answer Tier 0.2 picks |
| **FFI** | zl cannot call C or Windows at all. `nativegen` emits kernel32-only binaries with no libc, so without FFI there is nothing to call |
| **Syscalls / raw memory** | Floor 5. Designed in `design_ffi_syscalls.md`, not built |
| **Threads / atomics** | cannot do two things at once |
| **Generics** | ~11 stdlib modules are polymorphic and cannot say so |
| **Operator overloading** | — |
| **Conditional compilation** | no `#ifdef` equivalent |
| **Inline assembly** | — |

---

## Tier 5 — correctness gaps that are not features

These are defects rather than missing features, but they cap what the language can do.

- **No floats in the fast backends.** `print(2.5 * 2.0)` gives `4` on both `nativegen`
  and `compilel`; `print(7 / 3)` gives `2` where the interpreter gives `2.33333`.
  Silently, no error. Being fixed now.
- **`+` is overloaded and it is a tested guarantee** — `add(2,3)=5` but
  `add("n=",5)="n=5"`. No integer add can be emitted without knowing both operand types.
- **An assignment inside a function writes the GLOBAL slot** when a global of that name
  exists. Deliberate — it is what lets `src/selfhost/compiler.zl` share a cursor across functions —
  but it makes per-function type inference unsound, and it caused a real bug in
  `stdlib/sortx.zl`. Must be settled before any inference pass. See
  `design_scoping_decision.md`.
- **Recursion is capped** at `MAX_CALL_DEPTH` 2000 in the interpreter, and the C backend
  has no guard at all — the same program compiled dies with `STATUS_STACK_OVERFLOW` and
  no output.
- **`src/selfhost/compiler.zl` implements only a subset** of the surface syntax and mishandles 63 of
  110 .zl files while `verify.ps1` stays green — the gate proves closure over one file,
  not coverage.

---

## Tier 5b — measured failures a user hits on day one

All of these were run against `interp.exe` on 2026-08-02, not inferred.

### Text is bytes, not characters

```
len("cafe")   -> 4      correct
len("café")   -> 5      WRONG, should be 4
upper("café") -> CAFé   the é is not uppercased
at("café", 3) -> <半>    returns HALF a character
```

Strings are byte arrays. Any accented letter, any emoji, any non-English text has the
wrong length, cannot be indexed safely, and does not case-convert. **zl is English-only.**
Fixing this is ~800 lines and touches every string builtin in both `src/runtime/interp.c` and
`src/runtime/runtime.c`.

### Runtime errors say nothing about where

An error raised three functions deep, on line 9, reports:

```
runtime error: at needs a string and a number
```

No line number. No function name. No call stack. The lexer already tracks line numbers for
*parse* errors — the runtime error path simply does not carry them. Estimated ~200 lines to
thread a call stack through `eval`/`exec` and print it. This is the single cheapest
improvement to daily usability in the whole document.

### A program cannot read its own arguments

`zl prog.zl hello world` — the program cannot see `hello` or `world`. Grepped `src/runtime/interp.c`
for an `args`/`argv` builtin; there is none. **zl cannot write a command-line tool.**
~30 lines.

### No REPL

There is no interactive prompt — grepped for one, none exists. Every experiment needs a
file. This is how most people first learn a language.

### Missing everyday syntax

| Missing | Written today as |
|---|---|
| `a, b = pair` (destructuring) | two indexed reads |
| `x = 1 << 8` | `shl(1, 8)` |
| `x = 2 ** 10` | `pow(2, 10)` |
| `a ?? b` (default if nil) | an `if` |
| `7 // 2` (whole-number division) | **does not exist — and this is why `/` is ambiguous.** The interpreter reads `/` as real division (`7/3` = 2.33333), the fast backends as integer division (`= 2`). One operator is being asked to mean two things; adding `//` makes the split explicit instead of a silent cross-engine divergence |

### No install story, no editor support, no profiler

To use zl a stranger must clone a repo, install Visual Studio, and run `build.bat`. There is
no download, no installer, no package. There is no syntax-highlighting definition for any
editor, so zl is written in plain black and white. And there is no profiler — the `push`
regression on 2026-08-02 was found by hand-bisecting seven commits, which a profiler would
have pointed at directly.

These are not compiler problems. They are the difference between a project and a language.

## Tier 6 — tooling and library

**Tooling, none of which exists:** formatter · LSP · debug info (DWARF/CodeView) ·
package manager · build system · REPL · cross-compilation.

The formatter is the cheap one — the parser already builds a full AST, so a
pretty-printer over it is a few hundred lines and would pay off immediately across 96
modules. The LSP is the expensive one, because `src/frontend/parser.c` exits on the first error and
an LSP needs error recovery.

**Library holes, once imports exist:** networking (no HTTP, no sockets) · threads ·
compression · crypto beyond SHA-256 · filesystem operations (can read and write files,
but not copy, move, delete or walk a tree) · proper Unicode — text is bytes today.

Worth noting what *is* there: `read`, `write`, `write_bytes`, `input`, `env`, `dir`,
`procs`, `exit`, `now`, `assert`. The OS door is already open a crack.

---

## Recommended order

1. **Imports** — turns 96 modules into a library. Cheapest high-value change here.
2. **Scoping decision** — blocks type inference; a decision, not an implementation.
3. **Error handling** — everything built after it would otherwise need retrofitting
   across 96 modules, and it is destroying test coverage today.
4. **Structs, then maps, then optionals** — structs first because maps are a structure
   over key-value pairs, and structs unblock roughly twice as many files.
5. **Floats everywhere, sized integers.**
6. **`switch`, closures, constants, multi-line literals** — the everyday tier.
7. **Memory-management decision**, then `defer`.
8. **`comptime`**, then **FFI + syscalls + raw memory** (Floor 5).
9. **Only then**: the type system and unboxing — worth 4–6×, but it arrives as a side
   effect of finishing the language rather than as separate optimisation work.

The one thing NOT on this list, deliberately: **writing our own optimiser passes.**
Measured 2026-08-02, `compilel` already matches `clang -O2`. The own-x64 backend is
level with C on call-heavy code and ~4× off only on tight loops — it lacks *loop*
optimisation specifically. LLVM gives C parity for free, so that effort belongs
elsewhere until the language is finished.
