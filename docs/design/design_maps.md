# Design: Maps / Dictionaries for zl

**Status:** proposal · **Floor:** 1 (widening the seed) · **Date:** 2026-07-29

Adds a key→value lookup type to zl. This is the last item on the self-hosting
minimum list in `MASTER_PLAN.md` §4.5 ("maps / lookup tables — needed for
keywords and variables") that the language does not yet have as a first-class
value. Today `compiler.zl` fakes lookup with parallel lists and linear scans;
a real map type removes that wart and makes the self-hosted compiler shorter.

---

## 1. Motivation

- **Self-hosting need.** Keyword sets, symbol→node-type tables, and
  variable→slot maps are all dictionaries. `compiler.zl` currently walks
  parallel lists by hand. A map type collapses that boilerplate.
- **Real programs need it.** Counting word frequencies, grouping `dir()`
  results by extension, memoising recursion — all want `m[key]`.
- **It's the only missing self-host primitive.** Strings, lists, records
  (via lists), functions, recursion, and file I/O all exist. Maps close the
  gap listed in the locked spec. This is *widening the seed*, not a new floor —
  exactly the kind of work §1 of the plan endorses.

Scope discipline (per the plan's anti-feature-creep law, risk #3): this adds
**one value type and four built-ins**. No generics, no ordered/insertion
guarantees beyond what falls out for free, no custom hashing hooks.

---

## 2. Representation: assoc-list first, hash table later

Two honest options:

| | Assoc-list (parallel key/value arrays) | Real hash table |
|---|---|---|
| Lookup | O(n) linear scan | O(1) average |
| Code | ~40 lines, reuses list machinery | ~150 lines, hashing + collision + resize |
| Bug surface | tiny | real (resize, tombstones, hash of mixed types) |
| Fits current codebase | yes — mirrors how `V_LIST` already works | no — first non-array container |

**Recommendation: ship the assoc-list first.** Rationale:

1. It matches the existing style. A `Value` already carries `items`/`nitems`
   for lists (`runtime.h:18`, `interp.c:36`). A map is literally *"a list of
   values plus a parallel list of keys"* — add one field, `keys`, and reuse
   everything else.
2. The self-hosting tables it unblocks are small (11 keywords, a dozen node
   types, a handful of locals per function). O(n) on n≈12 is free.
3. KISS/YAGNI. A hash table is premature until a real program is measured to
   be slow. The plan's own note — *"slow correct code is a finished floor"* —
   applies.

The map is exposed **behind built-ins and `m[k]` only**, never as raw fields
in zl. That means the assoc-list → hash-table swap is a pure runtime change
later, with **zero source or syntax churn** — the same discipline the plan
uses for the C backend ("borrow then delete").

---

## 3. Proposed syntax

### 3.1 The literal — reuse `[ ]` with `:` pairs

`{ }` is already blocks and cannot be a map literal without ambiguity. Rather
than burn a new bracket, overload the list literal: a `[ ... ]` whose elements
are `key : value` pairs is a map. This is the Swift/Ruby-ish shape and needs no
new keyword (the plan guards its 11-word list, §4.4).

```
scores = ["ada": 90, "linus": 85, "grace": 99]
empty  = [:]                      # the empty map (a bare [] stays the empty list)
```

- Keys and values are ordinary expressions.
- `[:]` is the empty-map spelling — unambiguous, since `[]` already means the
  empty list.
- **Keys should be strings or numbers.** Equality reuses the existing
  `values_equal` (`interp.c:504`), which compares numbers and strings and
  refuses lists/maps — a good, simple key contract.

**One lexer change is required:** `:` is not in the allowed symbol set
(`lexer.c:192`, `strchr("(){}[],.+-*/%=!<>", c)`). Add `:` to that string.
One character, no other lexer work.

**Fallback (zero parser change):** if adding literal syntax is deferred, ship
maps built-in-only in v1 — `m = map()` then `set(m,k,v)`. The literal is pure
sugar and can land a step later. Recommended path is to do the literal too, but
it is separable.

### 3.2 Access — `m[key]`

Reads already parse. `m["ada"]` produces an `N_INDEX` node today
(`parser.c:207`, `parse_postfix`) — the interpreter just needs to accept a map
there, not only a list.

```
print(scores["ada"])              # 90
print(scores["nobody"])           # nil   (missing key -> nil, not an error)
```

### 3.3 Write — `m[key] = value`

This **already parses** too. `parse_statement` accepts any expression as an
assignment target (`parser.c:444-449`), so `m["ada"] = 91` builds an
`N_ASSIGN` whose target is an `N_INDEX`. Both engines currently *reject*
non-identifier targets on purpose:

- interp: `"can only assign to a name for now"` (`interp.c:677`)
- compile: assumes `n->a->text` is an ident (`compile.c:230`)

So indexed assignment is a semantics change in the two back-ends, **not** a
grammar change. Wiring it up also gives lists `xs[i] = v` for free.

### 3.4 Iteration

`for k in keys(m) { ... }` — no new loop form. `keys(m)` returns a list and
the existing `for ... in <list>` handles it (`interp.c:689`). Deliberately no
`for k, v in m` destructuring — that would need tuple support the language
doesn't have and doesn't need to self-host.

---

## 4. Built-ins (four, all plain identifiers)

Built-ins cost zero reserved words (§4.3), so these are just names:

| Call | Result |
|---|---|
| `get(m, k)` | value for `k`, or `nil` if absent. Same as `m[k]`. |
| `get(m, k, default)` | value for `k`, or `default` if absent. |
| `set(m, k, v)` | store `v` under `k`. Same as `m[k] = v`. |
| `has_key(m, k)` | `true` / `false` |
| `keys(m)` | a list of the keys |

Plus two **extensions to existing built-ins** (not new names):

- `len(m)` → number of entries (extends `len`, `interp.c:241` / `runtime.c:222`).
- `print(m)` → renders `{ada: 90, linus: 85}` (extends `value_to_string`).

Optional, only if a real program asks for them: `values(m)`, `del(m, k)`. Left
out of v1 by YAGNI.

### 4.1 Mutation semantics — and the tradeoff

`push` returns a **new** list (`interp.c:302`), matching the global
immutability preference. Maps face a real tension:

- **Immutable** (`set` returns a fresh copy): consistent with `push`, but
  building a keyword table in a loop is O(n²) copies, and `m[k]=v` as an
  lvalue makes no sense.
- **Mutable** (`set` and `m[k]=v` change the map in place): O(1)-per-insert,
  matches how the eventual hash table naturally behaves, and matches the
  language's stated identity — *power over safety* (§4.1).

**Recommendation: mutable maps.** For a systems language whose whole bet is
total control, an in-place dictionary is the honest default, and it avoids the
quadratic blowup on exactly the self-hosting tables that motivated the feature.
This is a conscious departure from the general immutability guideline, made
because the map's primary use is a mutable lookup table. `m[k]=v` and
`set(m,k,v)` are two spellings of the same mutation.

---

## 5. Semantics (the contract)

- **Missing key reads `nil`.** `m["absent"]` and `get(m,"absent")` return
  `nil`; no crash. Use `has_key` to distinguish "absent" from "present but
  nil". (`get` with a default covers the common case.)
- **Keys must be string or number.** A list/map/nil key is a runtime error
  (`values_equal` already declines them).
- **Last write wins.** Setting an existing key overwrites; setting a new key
  appends.
- **Truthiness:** an empty map is falsy, a non-empty map is truthy — mirrors
  the list rule (`interp.c:61`).
- **Equality** of two maps is out of scope for v1 (`==` on maps returns
  `false` via the `default` arm of `values_equal`). Not needed to self-host.
- **Ordering:** with the assoc-list, iteration is insertion order. This is an
  *implementation detail*, not a promise — the future hash table may reorder.
  Don't write programs that depend on it; the spec is "unordered".

---

## 6. Implementation sketch

The value model is duplicated in two places by design: `interp.c` has its own
`Value` (with `V_FN`) for interpreting; `runtime.h`/`runtime.c` has the `Value`
that **compiled** programs link against. Maps must land in both, kept in lock-
step exactly as `V_LIST` already is.

### 6.1 The type (both `interp.c` and `runtime.h`)

Add `V_MAP` to the `ValueType` enum and one field to `Value`:

```c
typedef enum { V_NIL, V_NUM, V_STR, V_BOOL, V_LIST, V_MAP /*, V_FN in interp*/ } ValueType;

typedef struct Value {
    ValueType      type;
    double         num;
    char          *str;
    struct Value **items;   /* V_LIST items, and V_MAP values   */
    struct Value **keys;    /* V_MAP keys, parallel to items     */  /* <-- new */
    int            nitems;   /* entry count for both list and map */
    /* Node *fn;  (interp.c only) */
} Value;
```

A map reuses `items` for values and adds a parallel `keys` array; `nitems` is
the entry count. Everything that already allocates/copies `items` gets a
sibling line for `keys`.

### 6.2 Interpreter (`interp.c`)

1. **New AST node `N_MAP`** in `parser.h` (sibling to `N_LIST`), emitted by the
   `[` branch of `parse_primary` when it sees `key : value` (peek for `:` after
   the first element; `[:]` → empty map). Store keys in `kids[0,2,4…]` and
   values in `kids[1,3,5…]`, or add a second kid array — simplest is to store
   pairs flat and index by 2.
2. **`eval` N_MAP:** allocate `keys`+`items`, evaluate each pair.
3. **`eval` N_INDEX:** if `obj.type == V_MAP`, linear-scan `keys` with
   `values_equal`; return the matching value or `make_nil()`. Keep the existing
   list path.
4. **`exec` N_ASSIGN with an `N_INDEX` target:** eval the container and the key;
   if map, `map_set` (overwrite-or-append); if list, bounds-check and overwrite
   `*obj.items[i]`. This replaces the current hard error at `interp.c:677`.
5. **`call_builtin`:** add `get` / `set` / `has_key` / `keys`; extend `len` to
   count map entries.
6. **`value_to_string`:** add a `V_MAP` arm printing `{k: v, ...}`.
7. **`is_truthy`:** `V_MAP` → `nitems > 0`.

A single helper does the core work:

```c
/* index of key in map, or -1 */
static int map_find(Value m, Value key) {
    for (int i = 0; i < m.nitems; i++)
        if (values_equal(*m.keys[i], key)) return i;
    return -1;
}
```

`get`/`has_key` call it; `set` calls it then either overwrites `items[i]` or
grows both arrays by one (the same `realloc`-and-append pattern `push` uses).

### 6.3 Compiled runtime (`runtime.c` / `runtime.h`)

Mirror the interpreter. New public helpers:

```c
Value zl_map_n(int npairs, ...);            /* zl_map_n(2, k0,v0, k1,v1)     */
Value zl_get(Value m, Value key);           /* value or nil                  */
Value zl_set(Value c, Value key, Value v);  /* map key-set OR list index-set */
Value zl_has_key(Value m, Value key);
Value zl_keys(Value m);
```

- `zl_map_n` parallels `zl_list_n` (`runtime.c:38`) — varargs, but reads pairs.
- `zl_set` is the one lvalue helper for `container[key] = value` and serves both
  maps (key-set) and lists (index-set), so `xs[i]=v` works too.
- `get` / `has_key` / `keys` are also reachable as **built-ins through
  `zl_calln`** with no new code, because `compile.c` already routes any
  non-user-function identifier to `zl_calln(name, ...)` (`compile.c:143`). So
  `zl_calln` grows the same four `if (strcmp(name,...))` arms as the
  interpreter's `call_builtin`. Only the *literal* and the *lvalue* need
  dedicated helpers.
- Extend `zl_index` (`runtime.c:163`) for `V_MAP`, `to_string` for the `{…}`
  form, `zl_truthy`, and `zl_len_list`/`len`.

### 6.4 Compiler (`compile.c`) — surprisingly small

Because built-ins already dispatch through `zl_calln`, `get`/`set`/`has_key`/
`keys` need **no compiler change**. Only two spots move:

1. **`emit_expr` N_MAP:** emit `zl_map_n(<npairs>, k0, v0, k1, v1, …)`, exactly
   like the existing `N_LIST` → `zl_list_n` arm (`compile.c:169`).
2. **`emit_stmt` N_ASSIGN:** when the target is `N_INDEX`, emit
   `zl_set(<container>, <key>, <value>);` instead of `v_name = …`
   (`compile.c:230`). When the target is `N_IDENT`, keep today's path.

`collect_vars` needs no change — an indexed assignment doesn't declare a new C
variable.

### 6.5 The lexer

One character: add `:` to the symbol set at `lexer.c:192`. Nothing else.

---

## 7. What lands where — checklist

| File | Change |
|---|---|
| `lexer.c` | add `:` to the allowed-symbol `strchr` (1 char) |
| `parser.h` | add `N_MAP` node type |
| `parser.c` | `parse_primary` `[`-branch: detect `:` → build `N_MAP`; `[:]` empty |
| `interp.c` | `V_MAP` + `keys` field; `eval` N_MAP & N_INDEX(map); N_ASSIGN indexed lvalue; `map_find`; `get`/`set`/`has_key`/`keys`; extend `len`, `value_to_string`, `is_truthy` |
| `runtime.h` | `V_MAP` + `keys` field; prototypes for `zl_map_n`/`zl_get`/`zl_set`/`zl_has_key`/`zl_keys` |
| `runtime.c` | those helpers; extend `zl_index`, `to_string`, `zl_truthy`, `len`, `zl_calln` |
| `compile.c` | `emit_expr` N_MAP → `zl_map_n`; `emit_stmt` N_ASSIGN indexed → `zl_set` |

---

## 8. Test plan (the pass/fail gate)

Follow the project's three-engine discipline (`run_tests.ps1`): the same
program must produce identical output under the interpreter, the C backend, and
— where it stays in the integer/heap subset — leave the x86-64 backend
untouched (maps are heap objects, so they belong to floor-4 brick 4/5, not this
change; the native backend is explicitly out of scope here).

Minimum programs, each asserted byte-identical interp-vs-compiled:

1. **Build & read:** `m = ["a":1,"b":2]; print(m["a"]); print(m["b"])` → `1`,`2`.
2. **Missing key:** `print(m["z"])` → `nil`; `print(get(m,"z",0))` → `0`.
3. **Overwrite:** `m["a"]=9; print(m["a"])` → `9`; `print(len(m))` → `2`.
4. **Grow in a loop:** count word frequencies from a `split` list; assert a
   known total. (This is the O(n²) stress test that justifies mutable maps.)
5. **has_key / keys:** `print(has_key(m,"a"))`, `for k in keys(m){print(k)}`.
6. **List indexed-assign fell out for free:** `xs=[1,2,3]; xs[1]=9; print(xs)`.

A map is "real" when program #1 runs and agrees across both engines — the same
bar the plan sets for every other feature (§5.1, "the moment it becomes real").

---

## 9. Open questions (decide at implement time, not now)

- **Literal syntax now or later?** Recommended now (cheap), but the built-in-
  only fallback (§3.1) is a clean deferral if the parser peek is fussy.
- **`values(m)` / `del(m,k)`?** Omit until a real program needs them.
- **Hash-table upgrade trigger?** When a self-hosted-compiler profile shows map
  lookup as hot. Not before. The built-in surface is designed so the swap is
  invisible to zl source.
</content>
</invoke>
