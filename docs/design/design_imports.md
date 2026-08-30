# Design: imports (Phase 2, the hinge)

Drafted 2026-08-03, ready to build the moment the LLVM-bridge workflow frees the repo. This is the
single highest-leverage next feature: it turns today's 13 copy-pasted stdlib modules into a real,
loadable standard library, and makes every subsequent feature pleasant to test.

## The problem, concretely

Today there are NO imports. To use `mathkit`, you copy its code into your file. Every demo written
this session (`csvstats`, `texttools`, `wordfreq`) inlines its helpers — the real program is ~10
lines, the pasted-in library is ~100. Worse: the `sortx` sort bug this session had to be fixed in
TWO places because the code was copy-pasted into a test file.

## The simplification we already have

**The stdlib modules already prefix every function** — `mk_factorial`, `ix_map`, `sk_union`,
`dk_get`. They do this precisely BECAUSE zl has no namespacing and a plain assignment/definition is
global. That convention is not a workaround to undo — it is exactly what makes v1 imports trivial:

**`import mathkit` = "lex, parse and EXECUTE `stdlib/mathkit.zl`, so its `fn` definitions land in the
global scope."** The `mk_` prefix already prevents collisions. No namespace machinery needed for v1.

This is the "include" model (like C's `#include` or Python's `exec` of a module), and it fits zl's
current all-functions-are-global reality perfectly.

## Syntax

    import mathkit                 # run stdlib/mathkit.zl - mk_* now callable
    import csvkit, dictkit         # several at once
    import mathkit as m            # OPTIONAL, v2 - needs namespacing, defer

v1 ships the first two forms. `as` (aliasing) needs real namespaces and is v2.

`import` becomes keyword #16 (src/frontend/lexer.c:48-53). Per KEYWORDS_MAP.md this is one of the ~14 planned
additions, so the budget is intact.

## How it works (the mechanism)

1. **Lexer** (`src/frontend/lexer.c`): add `"import"` to the keyword list.
2. **Parser** (`src/frontend/parser.c`): parse `import name1, name2, ...` at STATEMENT position into a new node,
   e.g. `N_IMPORT` holding the list of module names. (Reuse the existing identifier-list parsing.)
3. **Interpreter** (`src/runtime/interp.c`): on `N_IMPORT`, for each name:
   - Resolve the file: search `./<name>.zl`, then `./stdlib/<name>.zl`, then a global stdlib dir.
     First match wins. Error clearly (with the searched paths) if not found.
   - GUARD against double-import and CYCLES: keep a global set of already-imported module names; if
     `name` is in it, skip (a no-op). Add it BEFORE executing, so a cycle (a imports b imports a)
     terminates.
   - Read the file, lex+parse it, and EXECUTE its top-level statements in the GLOBAL scope (this is
     what defines its `fn`s). The interpreter already has the lex→parse→exec pipeline; `import`
     just points it at another file and runs it into `g_global`.
4. **The backends**: `import` is resolved at parse/load time into the combined program, so
   `src/backends/c/compile.c`/`src/backends/llvm/compilel.c`/`src/backends/native/nativegen.c` ideally never see `N_IMPORT` — the imported functions are
   already in the tree. Simplest: do the file inclusion during PARSING (splice the imported file's
   top-level defs into the program AST) so ALL engines get it for free, exactly like the for-range
   desugaring. Decide parse-time-splice vs interp-time-exec early; parse-time-splice is more work but
   makes every backend work with zero backend changes (the preferred pattern in this project).

**Recommendation: parse-time splice.** When the parser hits `import mathkit`, it loads and parses
`stdlib/mathkit.zl` and inserts its top-level function definitions into the program before continuing.
Then interp, compile, compilel and nativegen ALL handle imports with no changes — the same trick that
made loops free. The double-import set lives in the parser.

## What must NOT break

- **The fixpoint.** `src/selfhost/compiler.zl` does not use `import`, so `verify.ps1` is unaffected — but
  `src/selfhost/compiler.zl` must still PARSE a program containing `import` if it ever compiles one. Since
  `src/selfhost/compiler.zl` compiles only itself (which has no `import`), this is safe for v1. Note it.
- **Only ONE import of a file executes**, even if imported by several modules (the guard set).
- **A missing module is a clear error**, not a silent skip or a crash — list the paths searched.

## Test plan

- `import mathkit` then `print(mk_factorial(5))` → 120, on interp AND every backend.
- Two files importing the same module → its functions defined once, no double-def error.
- A→B→A cycle → terminates, both defined once.
- Missing module → a clear "module 'foo' not found (searched ./foo.zl, ./stdlib/foo.zl)".
- Rewrite ONE existing demo (e.g. `texttools.zl`) to use `import textkit` / `import iterx` instead of
  inlined helpers, and confirm identical output — the proof it turns copy-paste into a real library.
- verify.ps1 GREEN, all 8 suites unaffected.

## Size & payoff

~250–350 engine lines (lexer +1 keyword, parser +the splice + the guard set + file loading, error
messages). Payoff: the 13 modules become `import`-able; every program shrinks; every future bug fix
lands ONCE instead of in N copies; and every later feature (structs, maps, errors) becomes pleasant
to test because tests can `import` the thing under test instead of pasting it.

**This is Phase 2 item #1 and the hinge of the whole plan** — after it, the library layer stops being
throwaway and becomes permanent infrastructure. Build it first, the moment the repo is free.
