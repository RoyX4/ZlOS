# Design: Maps / Dictionaries for zl — revision 2

**Status:** proposal, supersedes `docs/design/design_maps.md` · **Wave:** W2 (syntax
and surface) · **Date:** 2026-08-02
**Scope:** one new value type, one literal form, six built-ins. No code is
changed by this document.

`design_maps.md` (2026-07-29) argued for maps on self-hosting grounds and
proposed an assoc-list implementation behind an `m[k]` façade. Four days of
measurement have moved four of its load-bearing facts, and one of them inverts
the central recommendation. This document is a revision, not a restart: §3 lists
exactly what changed and why, and everything not listed there still stands.

---

## 0. TL;DR

```
dists = ["A": 0, "B": nil()]      # literal
dists["B"] = 4                    # in-place write, already parses today
print(dists["B"])                 # 4
print(get(dists, "Z"))            # nil   — the total read
print(dists["Z"])                 # runtime error — the strict read
for k in keys(dists) { ... }      # insertion order, guaranteed
```

The six decisions, each argued below:

| Question | Answer | Section |
|---|---|---|
| Literal | `[k: v]`, empty map is `[:]` | §4 |
| Implementation | **hash table from day one**, not an assoc-list | §3.1 |
| Key types | `str`, `num`, `bool`. Not lists, not `nil`, not NaN | §6 |
| Hash | FNV-1a 64-bit, one implementation shared verbatim by all engines | §7 |
| Iteration order | **defined**: insertion order | §8 |
| Missing key | `m[k]` is a runtime error; `get(m,k[,default])` is the total form | §9 |

---

## 1. What changed since revision 1, and why

| # | `design_maps.md` said | Now | Cause |
|---|---|---|---|
| 1 | "ship the assoc-list first, hash table later when a profile shows it hot" | **Hash table in v1** | The profile exists. §2 quantifies it: the assoc-list *is* what the corpus already has, hand-written, in `dijkstra.zl` and `dict.zl`. Shipping it as a builtin buys nothing but syntax. |
| 2 | "One lexer change is required: `:` is not in the allowed symbol set (`lexer.c:192`)" | **No lexer change at all** | The ternary landed. `:` is accepted at `lexer.c:256` and the comment above it (`lexer.c:252-255`) already reasons about the character. |
| 3 | "indexed assignment is a semantics change in the two back-ends… interp: `can only assign to a name for now` (`interp.c:677`)" | **Indexed assignment already works for lists**, including the compound `op=` form | `interp.c:1563-1580`. The error text is now `"can only assign to a name or a list index"` (`interp.c:1582`). A map lvalue is one more arm in a `switch` that already has the shape. |
| 4 | "`values_equal` … compares numbers and strings and refuses lists/maps — a good, simple key contract" | **False. Lists compare structurally** | `interp.c:1244-1252` recurses into list elements with a depth guard. Equality no longer excludes lists, so "lists can't be keys" has to be argued on mutability grounds instead — see §6. |
| 5 | "Ordering: … the spec is unordered" | **Insertion order is guaranteed** | The three-engine byte-identical gate makes unspecified order a liability, not freedom. §8. |
| 6 | "Missing key reads `nil`; no crash" | **`m[k]` errors; `get` is the total form** | Two reasons that did not exist on 07-29: `nil()` is already the sentinel in 33 corpus functions, and a nullable/Option type is now on the roadmap, which makes `m[k] : T` versus `get(m,k) : T?` the difference between a usable typed map and one that needs a nil-check at every read. §9. |
| 7 | Two engines to update (`interp.c`, `compile.c`) | **Five** | `compilef.c`, `compilel.c`, `nativegen.c` exist now. §11 states what each does — for three of them the answer is "refuse, loudly". |
| 8 | "`values(m)` / `del(m,k)`? Omit until a real program needs them" | **`values` ships in v1**, `del` still deferred | `stdlib/dict.zl:38-44` already ships `dvalues`; the real program asked. |

Unchanged and still endorsed from revision 1: the literal shape `[k: v]`, the
`m[k]` / `m[k] = v` access syntax, mutable-in-place semantics, `keys(m)` +
`for` rather than a new loop form, and the "expose nothing but built-ins so the
representation can be swapped" discipline.

---

## 2. The performance argument, quantified

### 2.1 The two files that say it out loud

```
stdlib/dijkstra.zl:12   # No maps in zl, so "distances" is a list of [node, dist] pairs and we use
stdlib/dijkstra.zl:13   # linear helpers to read/write it.

stdlib/astar.zl:25      # Representation choice: zl has no maps, and dijkstra's list-of-pairs lookup
stdlib/astar.zl:26      # is a linear scan per probe. A grid has dense integer keys instead, so each
stdlib/astar.zl:27      # per-cell table below (g score, came-from, closed, in-open) is a FLAT list of
stdlib/astar.zl:28      # length rows*cols indexed by  r * cols + c , allocated with fill() and
stdlib/astar.zl:29      # written in place with x[i] = v. Each probe is then a single index read.
```

These two comments are the whole case. `astar.zl` reached O(1) probes by
hand-building a perfect hash — it has dense integer keys, so `r * cols + c`
*is* the hash function, and `fill()` + `x[i] = v` *is* the table. It pays for
that with a `rows*cols` allocation per table even when the search touches a
dozen cells. `dijkstra.zl` has string keys, could not hand-build a hash, and
fell back to linear scans.

A third file did the same thing a third way: `stdlib/dict.zl:1-2` is *"an
associative map as a list of `[key, value]` pairs"*, i.e. the exact assoc-list
that revision 1 proposed shipping as the builtin.

**The corpus has already implemented revision 1's proposal, three times, by
hand.** Adding it as a builtin would replace `dget(d, k)` with `d[k]` and change
nothing else. That is the fact that inverts the recommendation.

### 2.2 Counting dijkstra's inner loop

Let `n` = nodes, `m` = directed edges. Reading `stdlib/dijkstra.zl`:

- **Init** (`:68-73`): `dj_set` (`:49-63`) is called `n+1` times. Each call
  scans the `k` pairs it has and rebuilds the list from `[]` with `push` per
  pair. Summed: **n²/2 comparisons and n²/2 heap `Value` allocations**, before
  the algorithm starts.
- **Min-scan** (`:82-92`), once per outer iteration: walks all `n` dists
  entries, and for each one calls `contains(dk_visited, dk_dp[0])`
  (`:87`). `contains` is a linear scan (`interp.c:619-628`) over a list that
  averages `n/2` entries. That is `n²/2` `values_equal` calls per outer
  iteration and, over `n` iterations, **n³/2**.
- **Adjacency** (`:99`): `dj_neighbors` (`:29-36`) scans the whole graph list
  per outer iteration → **n²**.
- **Relaxation** (`:100-108`): per edge, one `dj_get` (`:39-46`, ≤n compares)
  and, when the edge relaxes, one `dj_set` (≤n compares plus a full
  list rebuild). Over all edges → **~2mn compares and up to mn allocations**.

At `n = 1000`, `m = 5000` (a sparse graph, average degree 5):

| | probe operations (`values_equal`) | heap `Value` allocations |
|---|---|---|
| `dijkstra.zl` as written | ~5.1 × 10⁸ | ~5.5 × 10⁶ |
| the same algorithm rewritten as well as it can be *without* maps | ~6 × 10⁶ | ~5 × 10⁶ |
| with maps (`dists`, `visited`, adjacency all maps) | ~1 × 10⁶ | ~2 × 10³ |

Asymptotically: **O(n³ + mn) → O(n² + m)**.

### 2.3 The honest part of that table

The n³ term is not the map's fault and I am not going to claim it is. It comes
from nesting an O(n) `contains` inside an O(n) scan, and a careful author could
delete it today by storing a visited flag inside each `[node, dist]` pair. That
is the middle row: **a map-less dijkstra can reach O(n² + mn)**, no better,
because `dj_get` and `dj_set` stay linear no matter how the visited set is
stored.

So the map's *irreducible* win is the `mn → m` term and the elimination of the
per-relaxation list rebuild: ~6× fewer probes and ~2500× fewer allocations
against a well-written map-less baseline, and ~500× / ~2700× against the code
that is actually in the tree. Both numbers are worth having. The allocation
column is the one I would lead with, given the measured cost of a boxed 48-byte
`Value` and `zl_nil`'s per-value `memset` (`runtime.c:124`).

At the size `dijkstra.zl`'s own self-test uses — `n = 5` — none of this is
visible. That is the point: the linear-scan representation is not slow, it is
*unusable at scale*, and the file has never been run at a scale that would show
it.

### 2.4 What this does not fix

Even with maps, the min-scan is O(n) per iteration, so dijkstra stays O(n²).
Getting to O((n+m) log n) needs `stdlib/heapq.zl` and a decrease-key, and
decrease-key needs a node→heap-position lookup — which is a map. Maps are the
prerequisite for that change, not a substitute for it. One step at a time.

---

## 3. Representation

### 3.1 Hash table, not assoc-list

Revision 1's KISS argument was correct on 07-29 and is wrong now, for a reason
that is about *what shipping means*, not about performance philosophy: the
assoc-list already ships, three times, in zl source (`dict.zl`, `dijkstra.zl`,
and the pair-list shape in ~23 more files). A builtin that reimplements it in C
delivers syntax and roughly a 3-5× constant factor from dropping the
interpreter loop, and leaves every asymptotic complaint in the corpus exactly
where it is. `astar.zl` would still hand-roll its flat table, because a builtin
assoc-list would be *slower* than the flat list it has now.

The cost is real and revision 1 stated it accurately: ~150 lines instead of
~40, plus hashing, collisions, and resize. I accept that cost. It is one file's
worth of well-understood code, written once, and §7/§8 are structured
specifically so that the parts most likely to diverge between engines cannot
affect observable output.

### 3.2 The layout: entries array + index table

Two arrays, the layout CPython uses:

```c
typedef struct { Value key; Value val; unsigned long long hash; } MapEntry;

/* inside Value, for V_MAP: */
MapEntry *entries;   /* dense, in insertion order          */
int       nentries;  /* live + tombstoned                  */
int       nlive;     /* what len(m) reports                */
int      *index;     /* open-addressed slots -> entry idx  */
int       nslots;    /* power of two                       */
```

`index` holds entry indices (or -1 for empty), probed with linear probing on
`hash & (nslots-1)`. Lookup: hash, probe `index`, compare `entries[i].hash`
first (an integer compare rejects almost every mismatch) and only then
`values_equal`. Insert appends to `entries` and writes one `index` slot. Grow
when `nlive * 4 >= nslots * 3`.

Why this and not plain open addressing over the entries themselves:

- **Iteration is a straight walk of `entries`** — which is exactly what §8
  needs, and it is also the fastest possible iteration, no empty slots skipped.
- **`index` is `int`, not `MapEntry`** — a resize rehashes 4-byte slots, not
  48-byte-`Value`-bearing entries.
- The memory overhead is `nslots * 4` bytes on top of the entries, versus
  open addressing's ~33% of `sizeof(MapEntry)` wasted in empty slots.

The one cost: deletion leaves a tombstone in `entries`, and iteration must skip
it. §10 defers `del` for exactly this reason, so v1 pays none of it.

### 3.3 Mutable in place — unchanged from revision 1

`m[k] = v` and `set(m, k, v)` mutate. `push` returning a new list
(`interp.c:550-574`) is not a precedent to follow here: `push` gets away with it
because of the `tip` trick (`interp.c:556-563`) that lets an append reuse the
same array when nothing else has appended to it. There is no equivalent trick
for a hash table that has to resize, and building a table in a loop with
copy-on-write is quadratic — which is precisely the cost §2.2 measures in
`dj_set`. `set` returns the map (not `nil`) so that `d = set(d, k, v)` — the
shape `dict.zl:4-14` uses — keeps working during a port.

---

## 4. The literal: `[k: v]`

Revision 1's shape survives. What has changed is that the objection it worried
about (`:` doesn't lex) is gone, and a new one (the ternary owns `:`) has to be
answered.

### 4.1 `:` already lexes

```c
lexer.c:252  /* '?' and ':' are only ever the two halves of a ternary. Neither
lexer.c:256  if (strchr("(){}[],.+-*/%=!<>?:", c) == NULL) {
```

No lexer edit. Revision 1's §3.5 ("One character: add `:` to the symbol set at
`lexer.c:192`") is obsolete — that landed with the ternary. Note that
`design_type_system.md:275-283` still describes adding `:` at `lexer.c:198`;
that instruction is also stale, and for the same reason.

### 4.2 The slot is free — verified

```
$ cat _mapprobe2.zl
y = ["a": 1]
print(y)
$ ./interp.exe _mapprobe2.zl
line 1: expected ']' (got ':')
```

A `:` in that position is a hard parse error today, so no existing program can
be reinterpreted by giving it a meaning. That is the same clearance argument
`design_type_system.md:173-238` makes for annotations, and it holds here for
the same reason.

### 4.3 The ternary does not collide — verified for the common case, reasoned for the other

The worry: `[a ? b : c]` — does the list literal steal the ternary's `:`?

No, and the direction of the nesting is why. `parse_primary`'s `[`-branch calls
`parse_expr` for each element (`parser.c:315-316`), `parse_expr` is
`parse_ternary` (`parser.c:551-554`), and `parse_ternary` consumes its own `:`
with `expect_text(":")` at `parser.c:546` before it ever returns. The list
literal's loop therefore only ever sees a `:` that no ternary claimed.

Verified:

```
$ cat _mapprobe1.zl
x = [1 ? 2 : 3]
print(x)
print(len(x))
$ ./interp.exe _mapprobe1.zl
[2]
1
```

One element, a ternary, still a list. The reverse case — a map whose *key* is a
ternary, `[c ? a : b : v]` — parses correctly by the same mechanism:
`parse_ternary` takes `c ? a : b`, its else-branch recursion (`parser.c:547`)
parses `b` and returns at the second `:`, which the list literal then sees as
the pair separator. **I could not run that one** — it needs the feature to
exist — so treat it as reasoned, not measured, and make it a test in §12.

The genuinely unreachable shape is a map whose key is an *incomplete* ternary,
which is not a thing.

### 4.4 The grammar

```
list-or-map := '[' ']'                                  -> empty list
             | '[' ':' ']'                              -> empty map
             | '[' expr (',' expr)* ']'                 -> list
             | '[' expr ':' expr (',' expr ':' expr)* ']'  -> map
```

The decision point is one token of lookahead *after* the first element is
parsed: if the next token is `:`, it is a map; otherwise a list. Mixing
(`[1, "a": 2]`) is a parse error naming the offending token, not a coercion.

`[:]` for the empty map is unambiguous because `[]` already means the empty
list and `[:]` is currently a parse error. This is the one place where the
spelling is arbitrary rather than derived; the alternative, a `map()` builtin,
is strictly worse because it makes the empty case look nothing like the
non-empty one.

### 4.5 Printing

`print(m)` renders `[a: 1, b: 2]`, empty map as `[:]` — mirroring the literal,
not `{…}` as revision 1 proposed. `{` means a block everywhere else in the
language and there is no reason to introduce a second meaning for it in output
that a reader might copy back into source.

Note that this is a readability argument, not a round-trip argument: list
printing is *already* lossy (`print([1, 2, "a"])` gives `[1, 2, a]`, verified),
so printed output has never been re-parseable source and this does not change
that.

---

## 5. Access and the built-ins

| Call | Result | Cost |
|---|---|---|
| `m[k]` | value for `k`. **Runtime error if absent** (§9) | O(1) |
| `m[k] = v` | store, in place | O(1) amortized |
| `get(m, k)` | value for `k`, or `nil()` if absent | O(1) |
| `get(m, k, default)` | value for `k`, or `default` if absent | O(1) |
| `set(m, k, v)` | store; returns `m` | O(1) amortized |
| `has_key(m, k)` | `true` / `false` | O(1) |
| `keys(m)` | list of keys, insertion order | O(n) |
| `values(m)` | list of values, same order | O(n) |

Plus extensions to things that already exist, all of which are one arm each:

- `len(m)` → `nlive` (`interp.c:448`, `runtime.c`'s `len`).
- `k in m` → `has_key(m, k)`. This is nearly free: `in` already dispatches on
  the container's type (`interp.c:1340-1342`, *"'x in xs' IS contains(xs, x),
  and 'sub in text' IS has(text, sub)"*). One more branch.
- `is_truthy` (`interp.c:66-78`) → `V_MAP` is `nlive > 0`, mirroring `V_LIST`.
- `value_to_string` → the `[k: v]` form of §4.5.
- `==` → structural, and see §5.1.

No name collisions: `get`, `set`, `keys`, `values`, `has_key` and `del` are not
built-ins today (checked against the `strcmp(name, ...)` ladder in `interp.c`)
and no file in `stdlib/`, `tests/`, `examples/` or `compiler.zl` defines a
function with any of those names. `has` and `at` are taken, which is why the
membership test is `has_key` and not `has`. User functions win over built-ins
anyway (`interp.c:1404-1425` tries the user function first), so a program that
later defines `fn set(...)` shadows the builtin rather than colliding with it.

### 5.1 Map equality is order-insensitive

`values_equal_depth` currently returns 0 for anything it does not know
(`interp.c:1253`). Maps should not stay in that bucket, because lists do not:
`interp.c:1244-1252` compares lists structurally with a depth guard.

Two maps are equal when they have the same live key set and every key maps to
an equal value. **Order is deliberately not part of it**, even though §8 makes
order observable. This is the one place where those two decisions have to be
consciously decoupled, and it is the right split: insertion order exists so
that *output* is reproducible, not so that two tables built by different routes
count as different. The implementation is `nlive` equal, then one lookup per
entry — O(n), reusing the depth guard so a self-referential map cannot loop.

### 5.2 Iteration

`for k in keys(m)` works with zero new machinery (`interp.c:1596-1608` loops
over a list). `for k in m` directly is proposed as optional sugar: one arm in
`N_FOR`, and it saves materialising an O(n) keys list per loop.

If it lands, it must yield **keys**, for a consistency reason rather than a
convention one: whatever `in` tests as a membership question is what `for ... in`
should hand you. `x in xs` tests list elements and `for x in xs` yields
elements; `k in m` tests keys, so `for k in m` yields keys. Deliberately no
`for k, v in m` — that needs tuples, which the language does not have.

---

## 6. Key types

**Allowed: `str`, `num`, `bool`.** These are exactly the types `values_equal`
compares by value rather than by structure (`interp.c:1240-1243`).

**Rejected, with reasons:**

- **Lists.** Revision 1 said `values_equal` "refuses lists" — it does not, it
  compares them structurally (`interp.c:1244-1252`), so a list key is
  *definable*. It is still wrong, and the reason is mutation, not equality:
  lists are mutable in place (`interp.c:1563-1580`), so `k[0] = 9` after
  `m[k] = 1` silently moves the key's hash out from under the table and the
  entry becomes unreachable. That is a corruption bug with no error message,
  which is a worse outcome than "you can't do that".
- **`nil`.** It is the language's "no value" marker and §9 makes it the
  absence signal returned by `get`. A `nil` key would make `get(m, nil())`
  meaningless to read.
- **Functions** (`V_FN`): no equality at all today
  (`interp.c:1253`, *"V_FN: identity-free"*).
- **Maps**: same mutation argument as lists.

**Two number cases need explicit handling or the table breaks its own equality
contract:**

- **NaN.** `NaN != NaN` under `interp.c:1241`, so a NaN key could never be
  found again — an entry that consumes a slot and is unreachable forever.
  Reject it at insert with a runtime error rather than silently accepting it.
- **`-0.0`.** `-0.0 == 0.0` is true under `interp.c:1241`, but the two have
  different bit patterns, so hashing the raw bytes would put them in different
  buckets while equality insists they are the same key. Normalise `-0.0` to
  `0.0` before hashing. This is a two-line fix and a multi-hour bug if missed.

Number and string keys are distinct: `values_equal_depth` returns 0 on a type
mismatch at `interp.c:1238`, so `m[1]` and `m["1"]` are different keys. Worth
stating in the manual because it is the first thing a JSON-shaped program hits.

Once floats land as a distinct type from ints (in progress), `1` and `1.0` as
keys need a ruling. I am not making it here — it belongs with whatever
`design_floats.md` decides about `==` across the two, and a map that disagreed
with `==` would be a bug in the map. Flagged in §13.

---

## 7. Hashing

**Use FNV-1a, 64-bit, over the key's bytes.** Not sha256, not siphash.

The candidates, against zl's actual constraints:

| | FNV-1a | sha256 (`stdlib/sha256.zl` exists) | siphash |
|---|---|---|---|
| Lines of C | ~15 | ~200 | ~80 |
| Cost per short key | ~1 ns | ~1 µs | ~10 ns |
| HashDoS resistant | no | yes | yes |
| Deterministic across runs | yes | yes | only with a fixed key |

- **sha256 is the wrong tool by three orders of magnitude.** A cryptographic
  digest for a bucket index would make the O(1) lookup slower than the O(n)
  scan it replaces for any table under a few hundred entries — which is every
  table in the corpus. `stdlib/sha256.zl` exists for content hashing and should
  stay there.
- **siphash solves a problem zl does not have.** HashDoS matters when an
  adversary chooses your keys — a web server. zl programs read files the user
  named. Worse, siphash's defence is a *random* per-process key, and a random
  key makes bucket order vary run to run, which is precisely the thing §8 and
  the byte-identical three-engine gate cannot tolerate. A fixed-key siphash
  gives up the security property and keeps the cost.
- **FNV-1a is also what the corpus already uses.** `stdlib/hash.zl:35-43`
  implements `fnv1a` in zl, with a careful `mul32` (`:29-32`) that works around
  the 2^53 double-precision limit. Matching it in C means the language's own
  documented hash and its builtin hash are the same algorithm, and a zl program
  can predict a bucket if it ever needs to.

Per-type input to the hash:

- `V_STR`: the bytes up to the NUL. (Strings cannot contain a NUL —
  `lexer.c:205-209` refuses it — so length is unambiguous.)
- `V_NUM`: the 8 bytes of the `double`, after the `-0.0` normalisation of §6.
- `V_BOOL`: the 8 bytes of its `num` field (0 or 1), tagged so `true` and
  `1` land in different buckets — they are different keys under
  `interp.c:1238`.

**Parity requirement.** The hash must be byte-identical between `interp.c` and
`runtime.c` or a program's `keys(m)` differs between the interpreter and the
compiled binary and the three-engine gate fails. `runtime.c` mirrors
`interp.c`'s builtins by design and that parity is load-bearing. The safe way to
get it is one copy of the function in a header both include, not two copies that
"look the same".

**But note what §8 buys here**: with insertion-ordered iteration, the hash
function never reaches observable output at all. If the two copies *did*
diverge, both programs would still print the same thing — you would lose a
performance property, not correctness. That is a deliberate design margin, and
it is why §8's decision is worth its cost.

---

## 8. Iteration order: defined, insertion order

Revision 1 said *"the spec is unordered"*. That is the standard answer and it is
wrong for this language.

**The argument is the test gate, not ergonomics.** `verify.ps1` and
`run_tests.ps1` assert byte-identical stdout across the interpreter and the
compiled backend. Any test that prints a map, or loops over `keys(m)`, produces
output whose order comes from the hash table's internals. Under "unordered",
passing that test requires `interp.c` and `runtime.c` to agree on the hash
function, the initial slot count, the growth threshold, the probe sequence, and
the resize trigger — five independent implementation details, in two files that
are maintained separately, where a divergence shows up as a mysterious test
failure in a program that never mentions hashing.

Defining insertion order deletes that entire coupling. Any correct
implementation, in any of the five engines, produces the same order, because
the order is a property of the program rather than of the table. §3.2's layout
gives it for free — `entries` is already dense and append-ordered.

Secondary benefits, all real but none decisive on their own: JSON round-trips
preserve field order (`stdlib/jsonw.zl`, `json_pretty.zl`, `json_parse.zl` are
three of the eight irreducibly-dynamic files and all of them care); diffing two
program outputs stays meaningful; and a reader debugging a table sees the order
they built it in.

The cost, stated plainly: **`del` becomes a tombstone**, because removing an
entry from the middle of `entries` would either shift every later entry (O(n))
or break the order. That is why `del` is deferred to v2 (§10) — v1 pays nothing
for a promise it is not yet exercising. When `del` does land, the choice is
tombstone-plus-compact-on-resize (what CPython does), and iteration gains a
`is_live` skip.

Python made this exact call in 3.7 for the same practical reason after 3.6 got
it as an accident of the compact-dict layout. The precedent is not the argument,
but it is worth knowing that the layout and the guarantee arrive together.

---

## 9. Missing keys, in a language with no exceptions

zl has no `try`. Every error is fatal (`runtime_error`). So the question is
sharper than usual: any design that errors is a design that kills the program.

**Decision: `m[k]` on an absent key is a runtime error. `get(m, k)` returns
`nil()`. `get(m, k, default)` returns `default`.** This reverses revision 1 §5
("Missing key reads `nil`; no crash").

Three arguments, in increasing order of weight:

1. **Consistency with the operator's other meaning.** `xs[i]` out of range is
   already a hard error — `"list index out of range"` at `interp.c:1517` for
   reads and `"index-assign out of range"` at `interp.c:1570` for writes. If
   `m[k]` silently returned `nil`, `[` would mean "must exist" on one container
   and "maybe" on the other. One operator, two contracts, is how people get
   surprised.
2. **`nil` is already overloaded.** 33 functions across 13 corpus files use
   `nil()` as a sentinel — `dijkstra.zl:23-27` uses it for *unreachable*, and
   `astar.zl` returns it for *no path*. A map storing `nil` as a legitimate
   value (which `dijkstra.zl:71` does, literally: `dj_set(dk_dists, node,
   nil())`) cannot distinguish "absent" from "present and nil" through a
   nil-returning read. Under revision 1's design, porting `dijkstra.zl` to maps
   would require `has_key` before every read anyway.
3. **The type system, and this is the decisive one.** A nullable/Option type is
   on the roadmap for exactly the 33-sentinel-function problem. If `m[k]`
   returns nil on absence, then `m[k]` is typed `T?` *forever*, and every read
   from a typed `map[str, int]` needs a nil-check before it can be used as an
   `int` — the map is un-unboxable at its most common operation. With the
   strict read, `m[k] : T` and `get(m, k) : T?`, and the checked and unchecked
   paths are spelled differently at the call site where the reader can see
   them.

The counter-argument deserves stating: `has_key(m,k)` followed by `m[k]` is two
lookups. Revision 1 was right to fear that under the assoc-list, where it meant
two O(n) scans. Under §3.1's hash table it is two O(1) probes, and the common
case has a one-lookup spelling anyway — `get(m, k, default)`.

The error message should name the key: `map has no key "foo"`. A fatal error
whose text does not tell you which lookup died is barely better than a nil.

---

## 10. Deliberately not in v1

- **`del(m, k)`.** The tombstone question (§8). No corpus file deletes from its
  hand-rolled dict — `dict.zl` has no `ddel`. When it lands: mark the entry
  dead, decrement `nlive`, leave `index` pointing at it as a probe-chain
  tombstone, compact on the next resize.
- **`map[K, V]` as a type.** `design_type_system.md:260-266` defines the type
  grammar as `int | float | bool | str | any | list[type]` — no map. The natural
  extension is `map[K, V]` alongside `list[T]`, and I recommend that spelling
  over the cuter `[K: V]` purely for consistency with `list[T]`. But it is that
  document's call to make, not this one's.
- **Nested-target assignment beyond one level** (`m[a][b] = v`). `dict.zl:8`
  already writes `d[i][1] = v`, so this shape exists for lists; whether it
  generalises cleanly to maps is an implementation question I have not checked
  against `parser.c:929-935`.
- **Maps in `nativegen.c` / `nativeval.c`.** Heap objects, out of scope — same
  boundary `design_memory_structs.md` draws.
- **`sort(m)`, map comprehensions, default-dicts.** No.

---

## 11. Where it lands, in five engines

| File | Change | Size |
|---|---|---|
| `lexer.c` | **none** (§4.1) | 0 |
| `parser.h` | one `N_MAP` appended to the end of `NodeType` — appended, so no existing value shifts, which `compilel.c:105` and `compilef.c` depend on since they print raw `type %d` | 1 line |
| `parser.c` | `parse_primary`'s `[`-branch (`:311-320`): after the first `parse_expr`, peek for `:`; plus the `[:]` case | ~20 lines |
| `interp.c` | `V_MAP` + the four fields; `eval` `N_MAP`; `eval` `N_INDEX` map arm (`:1511-1519`); `exec` `N_ASSIGN` map lvalue arm (`:1563-1580`); the hash table itself; `get`/`set`/`has_key`/`keys`/`values`; extend `len`, `value_to_string`, `is_truthy` (`:66-78`), `values_equal_depth` (`:1236-1257`), `in` (`:1340-1342`) | ~220 lines |
| `runtime.h` | `V_MAP` in the enum (`:12`) + fields in `Value` (`:14-22`); prototypes | ~10 lines |
| `runtime.c` | the same hash table, sharing one hash header with `interp.c`; extend `zl_index` (`:357`), `zl_set` (`:349` — note the name is already taken and already means list index-set, so the map case is an arm inside it, not a new function), `to_string`, `zl_truthy`, `len`, `zl_calln` | ~220 lines |
| `compile.c` | `emit_expr` `N_MAP` → `zl_map_n(...)` beside the existing `N_LIST` arm; nothing else, because built-ins already route through `zl_calln` | ~15 lines |
| `compilef.c` | **refuse**: maps are boxed heap objects, this backend is the unboxed integer subset | 1 line |
| `compilel.c` | **refuse**, same reason | 1 line |
| `nativegen.c` | **refuse**, same reason | 1 line |

The three refusals are the honest answer, not a gap. Those backends already
refuse lists and strings; a map is one more thing on that list. Their error must
name `N_MAP` rather than printing `type %d` for a node number nobody can look
up — which is a small, separate improvement worth making at the same time.

---

## 12. The self-hosting problem

`compiler.zl` is the zl-in-zl compiler, and `verify.ps1` gates the build on it
reaching a fixpoint. Two facts from `docs/design/design_selfhost_parity.md`:

- `compiler.zl` **does not support the ternary at all** — the parity table at
  `design_selfhost_parity.md:118` records it as **HANG**. So `:` is not a token
  `compiler.zl` handles anywhere, and a map literal is not a small addition to
  an existing colon path — there is no existing colon path.
- `compiler.zl` **does not support index assignment** either
  (`design_selfhost_parity.md:126`, **BROKEN C**), so `m[k] = v` compounds the
  same gap.

That sounds worse than it is, because of how the gate actually works:
`verify.ps1` compares gen1 to gen2 within a single run and its only input is
`compiler.zl` itself. It proves closure over one file, not coverage — the same
document measures `compiler.zl` mishandling 63 of 110 `.zl` files while the gate
stays green. So:

**Maps can land in the C toolchain without touching `compiler.zl`, and the gate
stays green, as long as `compiler.zl` itself uses no map.** That is the same
deal `elif`, f-strings, the ternary and index-assign already have.

The rule that follows, and it should be written into the implementation task:
**do not rewrite `compiler.zl`'s keyword lookup to use a map in the same change
that adds maps.** Revision 1's §1 pitched exactly that ("makes the self-hosted
compiler shorter") and it is a trap — it converts a feature addition into a
bootstrap change, where a failure gives you a compiler that cannot compile the
compiler that produces it. Land maps; land `compiler.zl` support for `:` and
map literals as a separate, separately-verified step; only then use maps inside
`compiler.zl`.

One live trap to check when that day comes: `compiler.zl` concatenates numbers
onto strings to emit C, and anything that changes number formatting can make
gen1 write `3` where gen2 writes `3.0` and flip the fixpoint. A map's `len` and
`keys` feed such concatenations. Not a reason to avoid maps, a reason to run the
fixpoint check before assuming.

---

## 13. Test plan

Three-engine discipline: interpreter and C backend must produce byte-identical
output; the three fast backends must **refuse with a named error**, and that
refusal is itself a test.

1. **Build and read.** `m = ["a": 1, "b": 2]` then `print(m["a"])`,
   `print(m["b"])`, `print(len(m))` → `1`, `2`, `2`.
2. **Empty.** `print([:])` → `[:]`; `print(len([:]))` → `0`;
   `if [:] { }` does not run; `if ["a":1] { }` does.
3. **Missing key, both spellings.** `print(get(m,"z"))` → `nil`;
   `print(get(m,"z",0))` → `0`; `print(m["z"])` → the error of §9, non-zero
   exit, and the message contains `z`.
4. **Overwrite and grow.** Insert 10,000 keys in a loop, overwrite half,
   assert `len` and a spot value. This is the test that would have passed under
   an assoc-list and taken 10⁸ operations to do it.
5. **Insertion order is the contract.** Build a map by inserting `k9…k0`,
   assert `keys(m)` prints in that exact order, from both engines, and assert
   that overwriting an existing key does *not* move it to the end.
6. **The ternary cases** (§4.3). `[1 ? 2 : 3]` stays a one-element list (this
   one passes today and must keep passing); `[c ? "a" : "b" : 1]` is a
   one-entry map. The second is the one I could not verify by running it.
7. **Key hazards.** `m[-0]` and `m[0]` are the same key; `m[1]` and `m["1"]`
   are different keys; a NaN key errors; a list key errors.
8. **Equality.** Two maps built in opposite insertion orders with the same
   contents are `==`; a map with an extra key is not.
9. **`in` and `for`.** `"a" in m` → `true`; `for k in keys(m)` visits every
   key once.
10. **Port `dijkstra.zl`** to maps and assert its self-test output is
    byte-identical to today's. That is the acceptance test for this whole
    document: the file that motivated the feature must produce the same answers
    with a shorter body.
11. **The backends refuse.** A `.zl` file with a map literal, run through
    `compilef`, `compilel` and `nativegen`, exits non-zero with a message
    naming maps.

The feature is "real" when tests 1 and 10 agree across the interpreter and the
C backend.

---

## 14. Open questions and things I am not sure about

Stated as uncertainty rather than guessed at:

- **`1` vs `1.0` as keys, once floats are a distinct type.** Must follow
  whatever `==` decides; I have not read `design_floats.md`'s ruling on
  cross-type numeric equality and will not pre-empt it. If `1 == 1.0` is true,
  the hash must be equal too, which means hashing an integral float by its
  integer value.
- **`m[a][b] = v`.** `dict.zl:8` does this for lists today. I did not read
  `parser.c:929-935` closely enough to say whether the map case falls out or
  needs work.
- **`for k in m` as sugar** (§5.2). Recommended, but genuinely optional; the
  argument for it is one saved allocation per loop and the argument against is
  that `for` over a list yields values while `for` over a map would yield keys.
  I think the `in`-consistency argument settles it, but it is the weakest
  decision in this document.
- **Whether `set` should return the map or nothing.** I chose "the map" to ease
  porting from `dict.zl`'s `return d` shape, but it makes `set` the only
  mutating builtin with a useful return value, which is its own small
  inconsistency.
- **Initial slot count.** 8 is the obvious default. Whether the ~23 corpus
  files that would become maps mostly hold 2-4 entries — in which case 4 is
  better and a resize is rare — I did not measure.
- **The exact interpreter/compiled speedup.** §2's numbers count probe
  operations and allocations, which is what changes asymptotically. I did not
  benchmark wall-clock, because the engine is being rebuilt as I write and
  timing it now would be measuring a moving target. The counts are derived from
  the source and are the honest claim; the wall-clock number should be measured
  after test 10 lands, against `dijkstra.zl` at n≈1000 rather than its n=5
  self-test.
