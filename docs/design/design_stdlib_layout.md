# Design Proposal — Standard Library Layout for zl

**Status:** proposal · **Author:** design pass · **Date:** 2026-07-29
**Floor:** widens Floor 1 (language). Depends on the `include` mechanism from
[`design_modules.md`](design_modules.md); does not block any other floor.

---

## 1. Motivation

`stdlib/` today is ~40 loose `.zl` files. Each one was written **standalone** —
every helper it needs is either a built-in or re-declared inline — and each ends
with a `print(...)` self-test block. That was the right call *before* code
sharing existed: a file had no way to pull in another, so self-containment was
forced.

Once `include` lands (Option A, textual, flat global namespace — see
`design_modules.md`), that same self-containment becomes a liability. Two
concrete problems are already baked into the current tree:

1. **Name collisions.** With a flat namespace, every `fn` name is global. The
   existing files already collide: `render` is defined in **3** files;
   `tokenize`, `show`, `roman_to_int`, `int_to_roman`, `check`, and `bfs` are
   each defined in **2**. The moment two of those are included into one program,
   codegen emits two `zl_fn_render(...)` definitions and the C compiler errors.
   `math_basics.zl` even redefines the built-in `abs`.

2. **Self-tests run on include.** 37 of the files end with a top-level
   `print(...)` self-test. Because `include` splices text and top-level
   statements execute, `include "stdlib/sorting.zl"` doesn't just import
   `bubble_sort` — it also runs sorting's entire demo and prints to stdout.
   Library code and test code are currently the same file.

A third, quieter hazard is the **global-variable leak** (README "Scope gotcha"):
a function that assigns a top-level-named variable mutates that global. A shared
library multiplies the blast radius of that footgun.

The goal of this proposal is a **layout and set of conventions** that make the
existing `include` mechanism usable at library scale — without new language
features. It answers four questions: how modules are **named**, how a module
file is **structured**, how modules **depend** on each other, and what the
**core set** should be.

---

## 2. Design constraints (what the language forces)

The layout is dictated by three properties of zl-as-it-is:

| Property | Consequence for the stdlib |
|---|---|
| **Flat global namespace** (`include` splices text; no `.` member access) | Function names must be globally unique. There is no scoping to hide them — the *name itself* is the namespace. |
| **Top-level statements execute** on include | A library file must contain **definitions only**. No top-level `print`, no top-level driver code. Tests move to separate files. |
| **`fn` declarations are hoisted; include-once dedupes** | Include order does not matter, and diamond dependencies (`a`→`b`, `a`→`c`, both →`core`) are safe automatically. Modules may `include` their own dependencies freely. |
| **Top-level globals leak into functions** | Library modules must not declare top-level mutable variables. Only `fn`s and, if truly needed, prefixed `UPPER_SNAKE` constants. |

Everything below follows from this table. The stdlib is not being redesigned
around a hypothetical module system — it is being organized around the flat
`include` we are actually shipping.

---

## 3. Approach

### 3.1 Naming: the prefix *is* the namespace

Because names are global, every exported function is prefixed with its module's
short name and an underscore:

```
str_trim        list_reverse      math_gcd
str_pad_left    list_map          math_pow
str_repeat      sort_bubble       set_add
```

Rules:

- **`<module>_<verb_noun>`**, all `snake_case`. The prefix is the module's
  canonical short name (see §6 core set).
- The prefix guarantees uniqueness across the whole library, which is exactly
  what the flat namespace requires. `sort_reverse` and `str_reverse` coexist;
  today's bare `reverse_list` / `reverse_string` do not scale.
- **Constants:** `UPPER_SNAKE` with the same prefix — `MATH_PI_MILLI`,
  `STR_ASCII_A`. Rare; prefer functions.
- **Private helpers** a module needs internally but does not export get the same
  prefix plus a leading underscore: `_str_scan_ws`. They are still global (the
  language cannot hide them), so the underscore is a *convention* signalling "do
  not call from outside", not an enforcement.
- This convention retires the current bare names (`trim`, `swap`, `show`,
  `render`) as a mechanical rename. `min2`/`max2` become `math_min`/`math_max`;
  the built-in `abs` is **not** redefined — modules use the built-in.

Rationale over the alternative (keep bare names, rely on include-once): bare
names only survive as long as no two modules pick the same word, and §1 shows
they already have. A prefix is 4–6 characters and removes the entire class of
collision.

### 3.2 File structure: definitions-only, tests-alongside

Split each concept into **two files**:

```
stdlib/str                 # proposed module name; definitions only
stdlib/test/str_test       # proposed test name; runs the self-test
```

The proposed `str` module under `stdlib/` contains only `fn` definitions and a
header comment. It never prints and never runs driver code, so including it has **no side
effects** — it only makes functions available.

The proposed `str_test` under `stdlib/test/` is a normal program:

```
# proposed stdlib string test
include "../str.zl"

print("== str ==")
print("str_trim('  hi ') -> '" + str_trim("  hi ") + "'")
# ... the self-test that used to live at the bottom of str.zl
```

The test runner (`run_tests.ps1`, or a new `run_stdlib_tests.ps1`) globs
`stdlib/test/*_test.zl` and runs each through the interpreter (and, where the
subset allows, the C backend), asserting expected output. This preserves the
"every module is verified" property the current self-tests give, while making
the library files importable.

Migration is mechanical: for each existing file, move the `# --- self-test ---`
block into `stdlib/test/<mod>_test.zl` with an `include` line at the top, and
prefix the function names.

### 3.3 Module file template

```
# str.zl -- string utilities for the zl standard library
# Exports: str_trim, str_repeat, str_pad_left, str_reverse, str_is_palindrome
# Depends: (none — built-ins only)

include "core.zl"          # only if this module actually calls core_* helpers

# Strip leading and trailing spaces from s.
fn str_trim(s) {
    ...
}

# ... more fns, each with a one-line doc comment ...

# NO top-level statements below this line.
```

The header block is the module's contract: **Exports** lists the public surface,
**Depends** lists the modules it `include`s. Both are checkable by eye and by a
trivial script.

---

## 4. How modules depend on each other

### 4.1 A module includes what it uses

If `sort.zl` calls `list_swap`, it puts `include "list.zl"` at its top. It does
**not** assume the top-level program already included `list.zl`. Because of
include-once, if the program includes both `sort.zl` and `list.zl` directly,
`list.zl`'s text is still spliced exactly once. Dependencies are therefore
**explicit and self-satisfying** — you can include the proposed `sort` stdlib module and it
works whether or not you also included its dependencies.

### 4.2 Layering to keep the graph acyclic

Modules are assigned to **tiers**, and a module may only depend on **lower** or
same-tier-but-already-listed modules. This prevents cycles (which `design_modules.md`
makes a hard error) and keeps the dependency graph readable:

```
Tier 0  core          (no stdlib deps; built-ins only)
Tier 1  str  math  list          (may depend on core)
Tier 2  sort  set  dict          (may depend on Tier 0–1)
Tier 3  io  fmt  json            (may depend on Tier 0–2)
```

The rule of thumb: **a module never reaches upward.** `str` may not depend on
`json`; `json` depends on `str`. If two modules genuinely need each other, that
is the signal to extract the shared part down into `core` (or a new low tier),
not to create a cycle.

### 4.3 `core` is the only universal dependency

`core.zl` holds the handful of helpers that almost everything wants and that are
awkward to keep re-deriving: `core_abs` is unnecessary (built-in exists), but
things like `core_show(list)` (stringify a list), `core_assert(cond, msg)`, and
small guards belong here. Keep `core` **tiny** — it is included transitively by
most programs, so every function in it is paid for everywhere. When in doubt,
leave it out of core.

---

## 5. Directory layout

```
stdlib/
  core.zl            # Tier 0: show, assert, tiny universal helpers
  str.zl             # Tier 1
  math.zl
  list.zl
  sort.zl            # Tier 2
  set.zl
  dict.zl
  io.zl              # Tier 3
  fmt.zl
  json.zl
  test/
    core_test.zl     # each includes ../<mod>.zl and runs the old self-test
    str_test.zl
    math_test.zl
    ...
  README.md          # the export/dependency index (§7)
```

The current grab-bag of ~40 topic files (`brainfuck.zl`, `sudoku.zl`,
`tic_tac_toe.zl`, `dijkstra.zl`, …) is **not** stdlib — those are *example
programs*, not reusable library modules. They should move to `examples/` (which
already exists), leaving `stdlib/` for genuine, prefixed, side-effect-free
modules. This is the single biggest cleanup: distinguishing "library" from
"demo".

---

## 6. Proposed core set

A first library that is small, non-overlapping, and covers what real zl programs
(including the compiler itself) actually reach for. Each row is one module file.

| Module | Prefix | Key exports | Depends |
|---|---|---|---|
| **core** | `core_` | `core_show(xs)`, `core_assert(c,msg)`, `core_bool_str(b)` | — |
| **str** | `str_` | `str_trim`, `str_repeat`, `str_pad_left`, `str_pad_right`, `str_reverse`, `str_starts_with`, `str_is_palindrome`, `str_index_of`, `str_replace` | core |
| **math** | `math_` | `math_min`, `math_max`, `math_sign`, `math_pow`, `math_gcd`, `math_lcm`, `math_is_even`, `math_is_odd`, `math_clamp` | — |
| **list** | `list_` | `list_reverse`, `list_swap`, `list_map_incr`, `list_sum`, `list_max`, `list_contains`, `list_slice`, `list_range` | — |
| **sort** | `sort_` | `sort_bubble`, `sort_is_sorted`, `sort_insertion` | list |
| **set** | `set_` | `set_new`, `set_add`, `set_has`, `set_to_list` (list-backed) | list |
| **dict** | `dict_` | `dict_new`, `dict_put`, `dict_get`, `dict_has`, `dict_keys` (parallel-list-backed until a map type exists — see `design_maps.md`) | list |
| **io** | `io_` | `io_read_lines(path)`, `io_write_lines(path,xs)`, `io_read_ints(path)` | str, list |
| **fmt** | `fmt_` | `fmt_int_pad`, `fmt_table_row`, `fmt_repeat_char` | str |
| **json** | `json_` | `json_escape`, `json_pretty(str)` | str |

Notes:

- Most of this already exists in the current files — it needs **renaming and
  de-duplication**, not new code. `sorting.zl`→`sort.zl`,
  `string_utils.zl`+`string_analysis.zl`→`str.zl`, `math_basics.zl`→`math.zl`,
  `hashset.zl`→`set.zl`, `text_table.zl`→`fmt.zl`, `json_pretty.zl`→`json.zl`.
- `dict`/`set` are **list-backed** stopgaps. When a real map type lands
  (`design_maps.md`), their *implementations* change but their *export names*
  stay, so callers are unaffected — the prefix convention pays off here.
- `io` is where `read`/`write` built-ins get ergonomic wrappers
  (line-splitting, int parsing) so programs stop re-implementing them.

Explicitly **out of scope** for the first core set: networking, floats (see
`design_floats.md` — blocked on language support), concurrency, regex (keep
`regex_match.zl` as an *example* until it earns library status).

---

## 7. Implementation sketch

This is a **reorganization**, not a language change. No lexer/parser/codegen
work; it rides entirely on the `include` pass from `design_modules.md`.

### 7.1 Phased migration

1. **Land `include`** (prerequisite — `design_modules.md`).
2. **Seed `core.zl`** with `core_show`/`core_assert` (extract the `show` helper
   that already recurs across files; give it one home).
3. **Convert one module end-to-end** as the pattern: pick `str`.
   - Create the proposed `str` module under `stdlib/`: prefixed, definitions-only.
   - Create its proposed `str_test` under `stdlib/test/`: `include "../str.zl"` + the moved
     self-test.
   - Verify the interpreter running the proposed string test prints the expected output.
4. **Repeat** for `math`, `list`, `sort`, `set`, `dict`, `io`, `fmt`, `json`.
5. **Move non-library files** (`sudoku`, `brainfuck`, `dijkstra`, …) to
   `examples/`.
6. **Add `run_stdlib_tests.ps1`** globbing `stdlib/test/*_test.zl`.
7. **Write `stdlib/README.md`** = the §6 table, kept in sync with the `Exports:`
   headers.

### 7.2 Mechanical rename recipe (per file)

```
1. rename file           sorting.zl -> sort.zl
2. prefix every `fn`     fn bubble_sort  -> fn sort_bubble
3. prefix every call     bubble_sort(x)  -> sort_bubble(x)   (same file + tests)
4. cut the self-test     move `# --- self-test ---`..EOF into test/sort_test.zl
5. add header            # Exports: ...   # Depends: ...
6. add include lines     include "list.zl"  (if it uses list_*)
7. run test              interp.exe <proposed sort test>
```

### 7.3 Two tiny guard scripts (optional, high value)

- **Collision check:** `grep '^fn ' stdlib/*.zl`, strip to names, assert all
  unique. Catches a missed prefix before it becomes a C compiler error. (Run
  against the current tree it reports `render`×3, `tokenize`×2, etc. — the exact
  set §1 flags.)
- **Side-effect check:** assert no `stdlib/*.zl` (excluding `test/`) contains a
  top-level `print(` at column 0. Catches a self-test left in a library file.

Both are ~5 lines and can be wired into `run_stdlib_tests.ps1` as preconditions.

### 7.4 Self-hosting guard

The stdlib reorg does not touch `src/selfhost/compiler.zl`'s emitted output, so the
self-hosting fixpoint (`gen1.c == gen2.c`) is unaffected. The *first real
customer* of the stdlib should be `src/selfhost/compiler.zl` itself: once `include` and a
`str`/`core` module exist, the compiler's inline string helpers can be replaced
with an include of the proposed `str` stdlib module — proving the library works under the same
byte-identical-recompile bar the rest of the language is held to.

---

## 8. Summary

- The flat `include` namespace forces two conventions: **prefix every export
  with its module name**, and **keep library files definition-only** with tests
  in `stdlib/test/`. Both are needed *now* — the current tree already has
  colliding names (`render`×3) and 37 side-effecting self-tests.
- **Modules include their own dependencies**; include-once + fn-hoisting make
  that safe and order-independent. A **tier rule** (core → str/math/list →
  sort/set/dict → io/fmt/json) keeps the graph acyclic.
- The **core set** is mostly a rename-and-dedupe of code that already exists,
  plus splitting genuine library modules out from example programs.
- Zero language changes. It is a layout discipline layered on the `include`
  mechanism, verifiable by two 5-line guard scripts and the existing test
  harness, and it survives the self-hosting fixpoint.
