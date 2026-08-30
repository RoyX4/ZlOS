# Decision: the Global Assignment Rule

**Status:** ✅ DECIDED 2026-08-03 — **Option 2 adopted** (assignment binds locally; explicit
`global` opts into write-through). Recorded in MASTER_PLAN §10. Unblocks closures and the type
system. Land in the two phases in §7 so the fixpoint never goes red.
**Author:** language design pass, 2026-08-02; decision ratified 2026-08-03
**Scope:** the meaning of `x = e` inside a function body. Touches `src/runtime/interp.c`
(one function), `src/backends/c/compile.c` (one condition), `src/backends/c/compilef.c` (one condition),
`src/backends/native/nativeval.c`, `src/backends/llvm/compilel.c`, `src/backends/native/nativegen.c`, `src/frontend/parser.c`, `src/frontend/lexer.c`, and
`src/selfhost/compiler.zl`. No code is changed by this document.

---

## 0. TL;DR

Today, an assignment inside a function writes the **global** slot whenever a
global of that name exists. Only parameters and `for`-loop variables bind
locally. This is deliberate and documented, and `src/selfhost/compiler.zl` relies on it.

It also makes per-function type inference **unsound**, and it is the mechanism
behind a live class of bug that 16 stdlib files work around by hand-prefixing
their locals.

Three things this document establishes that were not previously written down:

1. **The corpus cost of removing the rule is 168 assignment sites in 24 of 111
   `.zl` files** — 116 `global` declarations if declared per-function, 78 if
   declared per-file. Not the whole corpus. `src/selfhost/compiler.zl` needs **8**.
2. **The engines already disagree.** `src/runtime/interp.c`, `src/backends/c/compile.c`, `src/backends/c/compilef.c` and
   `src/backends/native/nativeval.c` implement write-through. **`src/backends/llvm/compilel.c` and `src/backends/native/nativegen.c` do
   not** — they give every name a function assigns its own slot. The LLVM
   backend, the designated speed backend, is on the *other* side of this
   decision already and nothing in `run_tests.ps1` or `verify.ps1` notices.
3. Therefore the choice is not "change the language or leave it alone". It is
   **"which of the two semantics already in the tree wins"**.

**Recommendation: Option 2 — assignment binds locally; an explicit `global`
declaration opts a function into writing through.** Landed in two phases so the
fixpoint never goes red. Details in §7.

---

## 1. What the rule actually is

### 1.1 The interpreter

`src/runtime/interp.c:173-185`:

```c
/* assign: update an existing variable anywhere up the chain,
 * otherwise create it in the current (innermost) scope.
 *
 * NOTE, deliberate and shared with the C backend: assigning a name
 * inside a function writes the GLOBAL of that name when one exists.
 * That is what lets the self-hosted compiler share a cursor across
 * functions. Only parameter binding is scoped; see env_define. */
static void env_assign(Env *e, const char *name, Value val)
{
    Var *found = env_find(e, name);
    if (found) { found->val = val; return; }
    env_define(e, name, val);
}
```

`env_find` (`src/runtime/interp.c:139-145`) walks the parent chain. A call frame's parent is
`g_global`, not the caller (`src/runtime/interp.c:1404`), so the chain is exactly two links:
locals, then globals. That is why *function-to-function* locals do not collide
and only top-level names leak in.

Two names are exempt, and both are exempt because the *language* introduces
them rather than the programmer assigning them:

- **parameters**, bound with `env_define` (`src/runtime/interp.c:163-171`), which always
  makes a fresh slot in the call scope;
- **`for`-loop variables**, bound at call time by `define_loop_vars`
  (`src/runtime/interp.c:1375-1384`), with the reason spelled out at `src/runtime/interp.c:1364-1367`:
  "`for i in [7,8] {}` inside a function used to leave a top-level `i` holding 8."

So the rule is narrower than "zl has no local scope". It is precisely: **plain
assignment is the only construct that writes through.**

### 1.2 Verified behaviour

`_scope_probe.zl` (scratch file, gitignored):

```
counter = 100
fn bump()     { counter = counter + 1  return counter }
fn shadow(n)  { tmp = n * 2            return tmp }
tmp = 7
print(bump()) print(counter) print(shadow(5)) print(tmp)
```

`interp.exe` prints `101 / 101 / 10 / 10`. The first two lines are the feature:
`bump` mutated shared state. The last line is the footgun: `shadow`'s obvious
local `tmp` overwrote a top-level `tmp` it has no relationship with.

### 1.3 The documented footgun

`README.md:86-92` calls it "Scope gotcha (important)" and "a real footgun — it
can cause infinite loops", closing with "This is a deliberate property the
self-hosting compiler relies on, not a bug."

Sixteen stdlib modules restate it in their own headers and work around it by
prefixing. `stdlib/sortx.zl:29-33` is the most precise statement in the repo:

```
# LOCAL NAMES. Every local below is prefixed sx_. A plain assignment inside
# a function writes an EXISTING global of that name - README.md "Scope
# gotcha (important)" - so an unprefixed local silently overwrites a
# caller's global that happens to share the name. Parameters and for-loop
# variables bind to their own slot and are safe unprefixed.
```

`stdlib/astar.zl:31-35` documents the same convention but over-applies it —
"every parameter and local carries a two-letter tag" — so `path_cost(pc_path)`
(`astar.zl:65`) prefixes a **parameter**, which was never at risk. The
workaround has outgrown the hazard, which is itself a cost: contributors are
paying a naming tax on constructs that are already safe.

The 16 files that document the rule in their own header:
`ansi, astar, base64, benchlib, bitops, bitset, color, datetime, diff, fenwick,
ini, noise, poly, sortx, statemachine, trie, xmlmini`.

---

## 2. The finding that changes the argument: the engines already disagree

The rule is described as "shared with the C backend". It is shared with *some*
of the backends.

**Write-through (matches the interpreter):**

| Engine | Evidence |
|---|---|
| `src/runtime/interp.c` | `env_assign`, §1.1 |
| `src/backends/c/compile.c` | `src/backends/c/compile.c:416-418` — a local is declared only `if (!is_param && (!set_has(&g_globals, ...) \|\| set_has(&loopvars, ...)))`. A name that is a global gets no local declaration, so the emitted C writes the file-scope `Value v_NAME`. |
| `src/backends/c/compilef.c` | `src/backends/c/compilef.c:142`, the same condition. |
| `src/backends/native/nativeval.c` | `src/backends/native/nativeval.c:32-37`: "A function assigning to a name that IS a top-level global writes THROUGH to the global - that is the 'function-locals leak into globals' footgun the self-host relies on." |

**Local-by-default (does *not* match the interpreter):**

`src/backends/llvm/compilel.c` — the LLVM backend — builds its local set from *every* assigned
name with no global exemption (`src/backends/llvm/compilel.c:366-368`):

```c
    g_locals.count = 0;
    for (int i=0;i<fn->nkids;i++) set_add(&g_locals, fn->kids[i]->text);
    collect_vars(fn->a, &g_locals);
```

and then (`src/backends/llvm/compilel.c:376-380`):

```c
    /* locals live in alloca slots, never in the @v_ globals */
    for (int i=0;i<g_locals.count;i++) {
        fprintf(out, "  %%l_%s = alloca i64\n", g_locals.names[i]);
        fprintf(out, "  store i64 0, ptr %%l_%s\n", g_locals.names[i]);
    }
```

`var_slot` (`src/backends/llvm/compilel.c:84-87`) then resolves the name to `%l_NAME` for the
whole body. So in `compilel`, `bump()` in §1.2 reads a zeroed alloca, returns
`1`, and leaves the global at `100`. Reads diverge too, not just writes.

`src/backends/native/nativegen.c` is further out again: it has no global storage at all.
`local_slot` (`src/backends/native/nativegen.c:62-72`) hands every name an `rbp`-relative slot, and
`nlocals` is reset per function (`src/backends/native/nativegen.c:298-299`) with top-level names
collected into the same array for the entry function (`src/backends/native/nativegen.c:468-469`).

**Why this was never caught:** `run_tests.ps1` runs three engines — interpreter,
`compile.exe`, `nativegen.exe` — over six hand-written integer programs
(`run_tests.ps1:1-15, 82-91`). None of them writes a global from inside a
function. `src/backends/llvm/compilel.c` and `src/backends/c/compilef.c` appear in **neither** `run_tests.ps1`
nor `verify.ps1`. The gate is green because the divergent case is not in it.

> *Epistemic status:* the `compilel`/`nativegen` divergence is read off the
> source, not executed — this pass is read-only and does not build. It should be
> turned into a test before anyone relies on it either way. The
> `interp.exe` numbers in §1.2 were run.

This reframes everything below. "Keep the rule" is not the zero-cost option; it
means **changing `src/backends/llvm/compilel.c` and `src/backends/native/nativegen.c` to adopt write-through**, in the
backend whose entire value is that it emits `add i64` into SSA registers.

---

## 3. Why this blocks the type system

`docs/design/design_type_system.md` needs to know, for each name, one type.
Under the current rule it cannot decide that per function.

1. **A local is not a local until you have read the whole file.** Whether
   `tmp = n * 2` in `shadow` binds a fresh slot or writes a global depends on
   whether *some other part of the program* assigns `tmp` at top level.
   `src/backends/c/compile.c` already has to do this — it collects `g_globals` before emitting
   any function (`src/backends/c/compile.c:470-478`) precisely so `emit_function` can answer
   the question. An inference pass would need the same pre-pass, which is
   affordable but means **no function can be typed in isolation.**

2. **A global's type is a join over every write in the program.** With no union
   types (cut in `design_type_system.md` §6 and in `design_types.md`), the join
   of `int` and `str` is `any`, so any global written inconsistently anywhere
   falls back to boxed — including at all the *other* sites that were consistent.

3. **Annotation stops being local, which breaks the stated contract.** The
   settled syntax is `name: type = value`. Written inside a function, on a name
   that happens to be a global, it annotates *the global*. So the promise in
   `design_types.md` §4 — "per-declaration opt-in… you pay only for what you
   annotate" — is false: you also constrain every other function that assigns
   that name. `design_type_system.md:169` already says "Re-annotating a name in
   the same scope is an error, not a redeclaration"; under write-through,
   two functions annotating their own obvious locals are *in the same scope*
   and collide.

4. **It interacts with the `+` overload.** `+` is `num,num->num` **or**
   `str,str->str` and that is a tested guarantee (`tests/test_syntax.zl`). To
   emit an unboxed `add i64` you need both operand types. If one operand is a
   name whose type is a whole-program join that landed on `any`, you cannot.
   The globals are exactly the hot cursors (`src/selfhost/compiler.zl`'s `spos`/`pos`,
   `json_parse.zl`'s `gPos`, `calc_repl.zl`'s `POS`), so this is not a corner.

None of these is fatal on its own. Together they mean the inference pass has a
qualitatively different shape depending on this decision, so it has to be
decided first.

---

## 4. The census

**Method.** A brace-depth scan of all 111 `.zl` files. A **write-through site**
is an assignment (`=` or `op=`) inside a function body to a name that is also
assigned at top level in the same file, **excluding** that function's parameters
and its `for`-loop variables (both bind, §1.1). Comments are stripped
crudely (`#` outside string literals). Files are standalone today — the include
system in `docs/design/design_modules.md` is a proposal and `grep` finds no
`include` handling in `src/frontend/lexer.c`, `src/frontend/parser.c`, `src/runtime/interp.c` or `src/selfhost/compiler.zl` — so
"same file" is the whole story. This is a heuristic scanner, not the parser;
treat the counts as accurate to within a couple of sites.

| corpus | files | fns | top-level names | assignments inside fns | **write-through sites** | files affected |
|---|---:|---:|---:|---:|---:|---:|
| `stdlib/` | 96 | 844 | 506 | 3,705 | **59** | 10 |
| `tests/` | 8 | 185 | 207 | 663 | **20** | 8 |
| `examples/` | 6 | 144 | 361 | 1,059 | **70** | 5 |
| `src/selfhost/compiler.zl` | 1 | 46 | 9 | 154 | **19** | 1 |
| **total** | **111** | **1,219** | **1,083** | **5,581** | **168** | **24** |

Migration size under an explicit-marker scheme:

| declaration granularity | declarations needed |
|---|---:|
| one `global x` per (function, name) | **116** |
| one file-level declaration per (file, name) | **78** |

**`stdlib/` in full** (10 of 96 files, 59 sites):

| file | function → globals written | intent |
|---|---|---|
| `testlib.zl` | `check` → `g_pass`, `g_fail` | deliberate |
| `astar.zl` | `astar`, `bfs_path` → `LAST_EXPANDED` | deliberate |
| `json_parse.zl` | `skipWs`, `scanString`, `scanNumber`, `scanLiteral`, `scanArray`, `parse` → `gPos`, `gSrc` | deliberate (parser cursor) |
| `lisp_interp.zl` | `lp_next`, `lp_parseList`, `lp_parse`, `lp_envSet` → `LP_pos`, `LP_toks`, `LP_env` | deliberate |
| `expr_vars.zl` | `p_advance`, `eval_stmt`, `run_program` → `P_POS`, `P_TOKS`, `P_ENV` | deliberate |
| `sha256.zl` | `check` → `PASSES`, `FAILS` | deliberate |
| `bmp.zl` | `check` → `checks_run`, `checks_bad` | deliberate |
| `strsearch.zl` | `kmp_scan`, `kmp_search` → `hits` | deliberate |
| `ansi.zl` | `colour_code`, `style_code`, `strip_ansi` → `i`, `n` | **accidental** |
| `hash.zl` | `fnv1a`, `djb2`, `sdbm`, `crc32`, `adler32` → `i` | **accidental** |

The last two rows are the sortx-shaped bug, still in the tree. `ansi.zl:135-160`
uses `i` and `n` as ordinary loop counters inside `strip_ansi`; `ansi.zl:270`
and `ansi.zl:278` use `n` and `i` as demo-loop counters at top level.
`hash.zl:35-41` uses `i` inside `fnv1a`; `hash.zl:154`, `:164`, `:174` use `i`
at top level. I traced the demo loops in both files and could **not** find a
call path where the colliding function runs inside the colliding loop, so these
are **latent, not currently firing** — the outputs are correct today by luck of
call ordering. `sortx.zl` shows zero sites because it was fixed this session by
prefixing every local `sx_`.

`src/selfhost/compiler.zl` (19 sites, 8 declarations) — the case the rule exists for:

| function | globals written |
|---|---|
| `next_token` (`:59`) | `spos` ×10 |
| `lex` (`:117`) | `src`, `spos` |
| `advance` (`:147`) | `pos` |
| `parse_program` (`:391`) | `toks`, `pos` |
| `compile_program` (`:647`) | `g_funcs` ×2, `g_globals` ×2 |

That is the whole dependency. Eight declarations across five functions.

`examples/` carries the heaviest load (70 sites, 54 declarations) and it is
worth naming why: `raytracer.zl`'s `scene_hit` writes `hit_t`/`hit_id`, `sky`
writes `sky_r`/`sky_g`/`sky_b`, `trace` writes `out_r`/`out_g`/`out_b`. Those
are not shared state — they are **multiple return values**, encoded as globals
because zl has no tuple. Same for `calc_repl.zl`'s parallel `ND_*` node arrays.
This is the heterogeneity finding again from a different angle: several
"deliberate" globals are workarounds for missing records/tuples, and would
disappear if those landed.

---

## 5. The options

### Option 1 — keep the rule, make inference whole-program

Nothing in the corpus changes. `src/selfhost/compiler.zl` untouched. Fixpoint untouched.
The inference pass gets a pre-pass that collects top-level names (exactly
`src/backends/c/compile.c:470-478`), resolves each in-function assignment against it, and
iterates a join to fixpoint over globals.

- **Breaks in stdlib:** nothing. Zero edits.
- **Breaks in `src/selfhost/compiler.zl`:** nothing.
- **Fixpoint:** unaffected. The gate's only input is `src/selfhost/compiler.zl`, whose
  behaviour is unchanged.
- **Cost to the type system:** all four problems in §3. Concretely: inference
  is non-modular; annotations inside functions are non-local and can collide
  between unrelated functions; a single inconsistent write anywhere de-optimises
  a global everywhere; and there is no way for a programmer to say "this really
  is my local" short of renaming it.
- **Cost elsewhere:** `src/backends/llvm/compilel.c` and `src/backends/native/nativegen.c` must be *changed* to
  implement write-through (§2). For `compilel` that means globals it cannot
  keep in SSA registers — every write-through name becomes a `load`/`store`
  against `@v_NAME` on every touch. LLVM's mem2reg cannot promote a global the
  way it promotes an alloca, because a call might read it. The parser cursor
  in a self-hosted `src/selfhost/compiler.zl` is the hottest variable in the program, and
  this option pins it to memory.
- **Leaves the footgun.** The two accidental collisions in §4 stay latent, and
  the naming tax on 16 stdlib files stays owed.

### Option 2 — assignment binds locally; explicit `global` opts in

```
fn advance() {
    global pos
    pos = pos + 1
}
```

Reads are unchanged (`env_find` still walks to the globals). Only `env_assign`
changes, and only for names not declared `global` in the enclosing function.

- **Breaks in stdlib:** 59 sites in 10 of 96 files; 36 `global` declarations.
  Two of those files (`ansi.zl`, `hash.zl`) are *fixed* rather than broken —
  the latent collisions become impossible. The other 86 stdlib files are
  untouched.
- **Breaks in `src/selfhost/compiler.zl`:** 19 sites, 8 declarations, 5 functions (§4).
  **And `src/selfhost/compiler.zl` must also learn to parse `global`**, since it compiles
  itself. That is the real risk and §7 is built around it.
- **Breaks in tests/examples:** 20 sites / 18 declarations and 70 / 54.
  `tests/*.zl` is almost entirely one pattern — `check()` writing `g_pass`/
  `g_fail`, in all 8 files.
- **Fixpoint:** this is a flag day if done in one step. `verify.ps1` compares
  gen1 to gen2 within a single run, so it does not care about history — but it
  does require that whatever `src/selfhost/compiler.zl` says, `src/selfhost/compiler.zl`-compiled-by-itself
  agrees. A `src/selfhost/compiler.zl` containing `global` cannot be compiled by a
  `src/selfhost/compiler.zl` that does not know the word. §7 sequences around that.
- **Cost to the type system:** this is the option that *removes* cost. Every
  unannotated assignment in a function is a genuine local, inferrable from its
  own function. Globals are written only where declared, so the join set for a
  global is small and syntactically enumerable. Annotation becomes local and the
  `design_types.md` §4 promise becomes true. Modular checking becomes possible.
- **Cost in reserved words:** `global` becomes a 15th. `MASTER_PLAN.md:223-234`
  fixes the list at fourteen and calls the last widening "a deliberate,
  one-time widening, not drift". A second one needs the same justification.
  It cannot be a *contextual* keyword the way type names are: type names are
  safe because they appear only in type position, whereas `global pos` today
  parses as two legal expression statements (`src/frontend/parser.c:927`, `:958-960`), so a
  contextual reading would silently change the meaning of existing programs.
  If the word is unacceptable, a sigil (`^pos = 1`) avoids the reserved word at
  the cost of a new token and worse readability; that is a strictly worse trade.
- **Cost in engines:** `src/backends/llvm/compilel.c` and `src/backends/native/nativegen.c` need **no change** — they
  already do this. `src/runtime/interp.c` gains ~6 lines in `env_assign`. `src/backends/c/compile.c:416`
  and `src/backends/c/compilef.c:142` swap `g_globals` for the function's declared-global set.
  `src/backends/native/nativeval.c` needs the corresponding change to its global-index path.

### Option 3 — keep the rule, require an annotation on any global whose type varies

Write-through stays. The checker demands `x: any = …` (or a concrete type) at
the top-level declaration of any global written with more than one type.

- **Breaks:** nothing at first; then whatever the checker flags.
- **Fixpoint:** unaffected.
- **Cost to the type system:** it does not solve the problem, it renames it.
  Deciding *which* globals need the annotation requires exactly the
  whole-program join of Option 1, so none of §3's structural costs go away —
  you pay them and then add ceremony on top. It also cannot fix §3.3: an
  annotation written inside a function still lands on the global.
- **And it does nothing for the footgun.** `ansi.zl`'s `i` is a number in both
  places, so it type-checks perfectly while still being a bug.

Reject. This is Option 1 with extra steps.

### Option 4 — local by default, with a file-level global declaration

Same semantics as Option 2, but the declaration is once per file at top level
(`global gPos, gSrc`) rather than once per function.

- **Migration:** 78 declarations instead of 116 — a saving of 38 lines across
  the whole corpus.
- **Cost:** re-introduces action at a distance in miniature. Adding a name to
  the file's list silently changes the meaning of every function that assigns
  it, which is the same non-locality that makes Option 1 painful, just smaller.
  A reader at the write site can no longer tell which slot is being written
  without scrolling to the header. For 38 saved lines, that is a bad trade.

Considered and rejected, but it is the strongest alternative to Option 2 and
should be re-examined if the per-function declarations turn out noisy in
practice.

---

## 6. Recommendation

**Option 2.**

The deciding argument is not the type system, though the type system is the
reason this is being asked. It is §2: **the tree already contains both
semantics, and the fast backend has the other one.** Option 1 is not "leave it
alone" — it is a decision to go and put write-through *into* `src/backends/llvm/compilel.c`, which
is the one file whose job is to keep values in registers.

Supporting arguments, in order of weight:

1. **Scope of change is small and now measured.** 168 sites, 24 files, 116
   declarations. `src/selfhost/compiler.zl` — the file the rule exists for — needs 8. The
   other 86 stdlib modules do not change at all.
2. **It makes inference modular**, which is the difference between a checker
   that can be written and one that has to be a whole-program fixpoint before
   it can type a single function.
3. **It fixes two live latent bugs and cancels a naming tax** on 16 files.
4. **Both semantics are already implemented**, so this is picking a winner
   rather than inventing one — the work in the four write-through engines is
   deleting a condition, not adding a mechanism.
5. **The explicit form documents intent at the point of use.** `global pos` in
   `advance()` says what the comment in `astar.zl:31-35` currently has to say in
   prose.

Costs accepted, stated plainly:

- A 15th reserved word, and the `MASTER_PLAN.md` §4.4 list has to be amended
  with the same "deliberate widening" note the last three got.
- Every file in §4's table needs an edit, `examples/` most of all.
- A bootstrap sequencing risk around `src/selfhost/compiler.zl`, addressed next.

---

## 7. How to land it without breaking the fixpoint

The hazard is precise: `verify.ps1` proves `f(f(x)) == f(x)` where `x` is
`src/selfhost/compiler.zl`. If `src/selfhost/compiler.zl` starts containing a word its own parser does not
recognise, gen1 never gets produced. Two phases avoid a flag day.

**Phase A — make `global` a legal no-op.** Add the keyword to `src/frontend/lexer.c` /
`src/frontend/parser.c` and to `src/selfhost/compiler.zl`'s lexer and parser. It parses to a declaration
node that every backend *ignores*, because write-through is still the default
and so is already what `global x` means. Nothing changes semantically. Then add
the declarations to all 24 files in §4 — 116 lines that are correct under both
the old and the new rule.

- Verify: `run_tests.ps1` unchanged, `verify.ps1` green, every file in §4 gives
  byte-identical output before and after.
- At the end of Phase A, `src/selfhost/compiler.zl` both understands and uses `global`, and
  gen1 == gen2 still holds.

**Phase B — flip the default.** Change `env_assign` (`src/runtime/interp.c:180`),
`src/backends/c/compile.c:416`, `src/backends/c/compilef.c:142`, and `src/backends/native/nativeval.c`'s global-index path to
consult the function's declared-global set instead of `g_globals`.
`src/backends/llvm/compilel.c` and `src/backends/native/nativegen.c` need nothing.

- Verify: the same output-identity check over all 111 files. Anything that
  changes output is a site Phase A missed, and the check names it.
- The one place this can bite the fixpoint is the path
  `design_selfhost_parity.md` already warns about — `src/selfhost/compiler.zl` concatenating
  a number onto a string to emit C. That path is unaffected here (it is about
  numeric formatting, not scoping), but it is the reason the Phase B check must
  be *byte* identity of generated `.c`, not just identical program output.

**Phase C — close the coverage hole.** Add a scoping case (the `_scope_probe.zl`
shape from §1.2) to `run_tests.ps1`, and put `compilel.exe` and `compilef.exe`
into the harness. §2 exists because nothing was watching; the fix should include
the watcher. This is worth doing **before** Phase B, since it is what turns
"predicted from source" into "measured".

Ordering note: C, then A, then B.

---

## 8. What this unblocks in the type system

With Option 2 landed, `design_type_system.md`'s inference pass can assume:

- an unannotated assignment in a function introduces or updates a **function-local**;
- its type is inferrable from the function body alone;
- a `global`-declared name's type is the join over top-level plus the enumerable
  set of functions that declare it — a small, explicit set, not "anywhere";
- `x: int = 0` inside a function means what it looks like it means.

The `+` overload (§3.4) is still unsolved by this — it is a separate problem and
stays a separate problem. But it becomes a *local* problem: both operands of a
`+` in a function body now have types derivable from that function, which is the
precondition for emitting an unboxed add.

---

## 9. Open questions

- **The word.** `global` is the obvious spelling and costs a reserved word.
  Alternatives that cost none are all worse (§5, Option 2). Needs a yes/no from
  the §4.4 owner before Phase A starts.
- **`examples/` may be the wrong thing to migrate.** 54 of the 116 declarations
  are there, and a good share of them (`raytracer.zl`'s `hit_t`/`hit_id`,
  `calc_repl.zl`'s `ND_*`) exist only because zl has no tuples or records. If
  `design_memory_structs.md` lands first, those globals disappear rather than
  gaining declarations. Sequencing question, not a blocker.
- **Is `global` needed at file scope too?** A top-level `global x` would be a
  no-op (top level *is* global scope). Proposal: make it a parse error there, to
  keep one meaning per construct. Not strongly held.
- **Unverified:** the `src/backends/llvm/compilel.c` / `src/backends/native/nativegen.c` divergence in §2 is read from
  source, not executed. Phase C should confirm it before it is cited as settled
  fact anywhere else.
