# Design: Records for zl — naming the positions of a list

**Status:** proposal · **Floor:** 1 (widening the seed) · **Date:** 2026-08-02
**Scope:** adds one contextual keyword-shaped word (`rec`), one node type, and a
lowering rule. Adds **zero** reserved words, **zero** lexer characters, and
**zero** new construction syntax. No code is changed by this document.
Line citations are against the tree as it stood on 2026-08-02; `lexer.c`,
`parser.c` and the numeric backends were being edited concurrently (floats), so
treat line numbers as pointers, not anchors.

---

## 0. TL;DR

A record is **a set of names for the positions of a list**. `rec Image { w, h, px }`
says that in a value of type `Image`, position 0 is called `w`. `img.w` lowers to
`img[0]` — literally, in the parser, before any engine sees it.

Five consequences, and they are the whole design:

1. **Tuples and records are one feature, and the tuple already shipped.**
   `[a, b]` is zl's tuple. `rec` is the naming layer over it. There is no second
   value type, no `(a, b)` syntax, and no new bracket.
2. **Nothing breaks at construction.** `stdlib/bmp.zl:41` keeps returning
   `[nw_w, nw_h, fill(...)]` and that value is already a legal `Image`.
   Migration is per-*call-site*, not per-module and not big-bang.
3. **`str()` output is unchanged**, because the value is unchanged. That matters
   more than it sounds: `stdlib/bmp.zl:198` compares results by
   `str(ck_got) == str(ck_want)` against list literals like `[20, 40, 64]`
   (`bmp.zl:237`). A record that printed `Rgb(20, 40, 64)` would break 13 of
   that file's assertions on day one.
4. **Records are nominal to the type checker and structural at runtime.** This
   is a real cost, stated in §5 rather than discovered later.
5. **The unboxing win is the point.** In `compilel.c` a record whose fields are
   all scalars is a flat LLVM struct and `.w` is a constant-index
   `getelementptr` — no bounds check, no `Value`, no 48-byte copy. That is the
   only reason to build this rather than keep writing the field names in
   comments.

**Recommendation: build it, and build it before general type inference.** §9
gives the load-bearing argument: a record's field types are declared *at the
record*, so they are immune to the per-function inference unsoundness that zl's
global-assignment rule creates. Records are the one piece of static typing that
works in today's language without fixing scoping first.

---

## 1. Motivation — measured, not asserted

### 1.1 How much of the corpus is a record wearing a list costume

Counting multi-element list literals in `return` position across `stdlib/`:

```
grep -rn "return \[[^]]*,[^]]*\]" stdlib/*.zl
```

**60 sites across 28 of the 96 stdlib modules.** That is a floor, not a ceiling —
it misses records built by `push` (`stdlib/astar.zl:157`), records assembled into
a variable before return, and every *consumer* of a record (`img[2]`, `pair[1]`,
`parsed[0]`), which is where the readability cost actually lands.

This matches the brief's independent tiering: ~23 files are "Tier B — needs
tuples/records, nothing dynamic about them."

### 1.2 The declarations already exist — in English, unchecked

This is the strongest argument in the document, and it is entirely quotable.
Every one of these modules opens by *writing down a record type in a comment*,
because the author needed one and the language would not take it:

`stdlib/bmp.zl:4-6`:

```
# Representation: an image is a 3-element list  [w, h, px].
#   px is ONE flat list of w*h*3 numbers (0..255), not a list of rows.
```

`stdlib/dijkstra.zl:3-6`:

```
# Graph shape: a list of [node, [[neighbor, weight], ...]] pairs.
#   node       : any comparable value (we use strings here)
#   neighbor   : a node id that appears as some pair's node
#   weight     : a non-negative number
```

That is `name : type`, one field per line, in a comment. The syntax proposed in
§3 is that comment with the `#` removed.

`stdlib/argparse.zl:3-8`:

```
# parse_args(argv) returns a 3-element list:  [flags, options, positionals]
#   flags       - list of bare names, dashes stripped   ["v", "r", "f"]
#   options     - assoc list of [key, value] pairs, last one wins
#   positionals - everything left over, in order
# The list is the whole parse result; has_flag/get_opt read it back.
```

`stdlib/astar.zl:5-7`:

```
# A cell is a [row, col] pair. Every step costs 1 and moves are 4-directional
```

### 1.3 The workarounds are visible and expensive

**Accessor functions written by hand.** `argparse.zl:123-138` is three functions
whose entire body is a field read:

```
fn has_flag(parsed, name) {
    return contains(parsed[0], name)
}
fn positionals(parsed) {
    return parsed[2]
}
```

`parsed[0]` and `parsed[2]` are `.flags` and `.positionals` spelled the hard
way. `stdlib/astar.zl:40-42` does the same for `LAST_EXPANDED`.

**Index arithmetic leaking into logic.** `bmp.zl:51`:

```
if st_x < 0 or st_y < 0 or st_x >= st_img[0] or st_y >= st_img[1] {
```

`st_img[0]` is the width. A reader has to hold the comment on line 4 in their
head to read line 51. `bmp_save` re-unpacks the same three positions
(`bmp.zl:140-142`) because there is no other way to name them.

**Sentinel records.** `bmp.zl:63` returns `[0 - 1, 0 - 1, 0 - 1]` for an
out-of-range read — a three-element list that is *not* a colour but has the
same shape as one. Nothing in the language can tell them apart. (This is a Tier
C / nullable problem as much as a record problem; see §13.)

**Cross-referenced comments admitting the gap.** `astar.zl:27-29`:

```
# Representation choice: zl has no maps, and dijkstra's list-of-pairs lookup
# is a linear scan per probe. A grid has dense integer keys instead, so each
# per-cell table below (g score, came-from, closed, in-open) is a FLAT list of
```

and `dijkstra.zl:12`: *"No maps in zl, so 'distances' is a list of [node, dist]
pairs"*. `dijkstra.zl:39-45` is then a linear scan over those pairs. Note what
this file actually needs: `dj_get`/`dj_set` want a **map**, and the
`[node, dist]` pair it scans wants a **record**. The two features are
complementary, not competing — `design_maps.md` handles the first.

### 1.4 What this is not

It is not a fix for `stdlib/jsonw.zl`, `json_parse.zl`, `lisp_interp.zl`, or
`compiler.zl`'s `["bin", "+", L, R]` AST. Those are the brief's Tier A —
irreducibly dynamic, eight files — and they should stay lists. A record type
that tried to swallow them would be reinventing sum types, which is a much
larger feature with a much worse cost/benefit ratio at this stage.

---

## 2. One feature or two? (tuples vs records)

The question deserves a direct answer because getting it wrong doubles the
surface area permanently.

| | Two types (`(a,b)` tuple + `rec` record) | One type (record is a named list) | One type (record is a new boxed object) |
|---|---|---|---|
| New value types at runtime | 1 or 2 | **0** | 1 |
| New syntax | `(a, b)`, `t.0` | **none** | `Name{...}` or `Name(...)` |
| Existing 60 sites keep working | no | **yes** | no |
| `str()` output changes | yes | **no** | yes |
| `len()`, `push()`, `contains()` still apply | no | **yes** | needs re-implementing |
| Backend work in the 3 boxed engines | real | **zero** | real |
| Cost | two concepts users must distinguish | records are structural at runtime (§5) | a second container to maintain forever |

**Decision: one feature. The tuple is `[a, b]` and already exists; the record is
a naming layer over it.**

The reasoning is not primarily economy — it is that zl *already made this
choice*, four years of stdlib ago. Every module in §1.2 uses a list as a tuple
and documents the field names in a comment. Introducing a separate tuple type
now would mean 60 sites are written in the wrong one. The cheapest correct move
is to make the thing they already wrote *mean* something.

`(a, b)` as tuple syntax is additionally blocked: `(` already opens a grouped
expression in `parse_primary`, so `(a)` is `a` and `(a, b)` would need a
lookahead to disambiguate. Not fatal, but it buys nothing here.

**Consequence to accept:** there is no "anonymous record". A two-field thing you
did not bother to declare is a list and gets no field names. That is fine — it
is the status quo, and `rec` is opt-in.

---

## 3. Declaration syntax

### 3.1 The word

```
rec Image {
    w:  int
    h:  int
    px: list[int]
}

rec Cell {
    row: int
    col: int
}

rec Args {
    flags:       list[str]
    options:     list[list[str]]
    positionals: list[str]
}
```

`rec` is recognised **only** as the first token of a top-level statement
immediately followed by an identifier and `{`. Everywhere else it is an ordinary
identifier and a legal variable name. The lexer is untouched — `rec` lexes as
`T_IDENT`. This is the exact mechanism `design_memory_structs.md:453-462` chose
for `struct` and `design_ffi_syscalls.md` §2.5(a) chose for `extern`.

### 3.2 The reserved-word budget, and a correction to MASTER_PLAN

`MASTER_PLAN.md:226-229` states the reserved list and calls it "Fourteen words":

```
if  else  for  in  fn  return  while  not  and  or  true  false  break  continue
```

**The code has fifteen.** `lexer.c:48-54`:

```c
static const char *keywords[] = {
    "if", "else", "elif", "for", "in", "fn", "return", "while",
    "not", "and", "or",          /* word operators, not && || */
    "true", "false",
    "break", "continue",         /* loop control */
    NULL
};
```

`elif` is in the lexer and not in the plan. Separately, `compiler.zl:36-37`
knows **twelve** — it omits `elif`, `break` and `continue` entirely, which is
one more instance of the `design_selfhost_parity.md` subset gap and needs no
action here.

This document adds **zero** reserved words, so all three lists stay as they are.
Fixing the 14/15 discrepancy in `MASTER_PLAN.md` §4.4 is a one-line edit that
belongs to whoever next touches that file, not to this proposal.

### 3.3 Why `rec` and not `struct`

`design_memory_structs.md` §4.1 already claims `struct` for a *different*
feature: a raw-memory type whose value is an address (`§2.2` — "a Win64 address
fits in a `double` exactly"), whose fields are `i32`/`u8`/`ptr`, and whose whole
purpose is byte-compatibility with MSVC's layout so you can hand the bytes to
`CreateWindowExA`. That is a systems primitive for W5.

This is not that. A `rec` is a safe, garbage-nothing, list-backed value with zl
types. Two different things must not share a spelling. §11 argues the boundary
in full; the naming decision follows from it.

`rec` over `record`: three characters, matches `fn`'s abbreviation style, and
`record` is a plausible variable name in a database-shaped program.

### 3.4 Field syntax

`name: type`, one per line, newline-terminated per `MASTER_PLAN.md:211`
("Statement end: **newline** — no semicolons"). `:` already lexes —
`lexer.c:256` accepts it in the symbol set:

```c
if (strchr("(){}[],.+-*/%=!<>?:", c) == NULL) {
```

(`:` and `?` are there for the ternary.) The type grammar is
`design_type_system.md` §2.4's verbatim, plus record names:

```
type := 'int' | 'float' | 'bool' | 'str' | 'any'
      | 'list' '[' type ']'
      | RecName
```

**Open: are annotations optional?** `rec Cell { row, col }` with no types is
attractive for migration — it names positions without committing to types, and
it is exactly what the comments in §1.2 do for `astar`. Recommendation: allow
it, treating a bare field as `any`, matching `design_type_system.md` §3.2's rule
that `any` *is* today's boxed value. Flagged as a decision in §13, because it
makes `rec` useful before the type checker exists, which is either the best or
the worst property of this design depending on how Stage 1 goes.

Declarations are top-level only. Forward references resolve in one pass at end
of file, so `rec Node { left: Node }` works and declaration order does not
matter — same rule as `design_memory_structs.md:487-489`.

### 3.5 Parser and node changes

One new `NodeType`, **appended at the end of the enum** so no existing numeric
value shifts (`compilel.c` and `compilef.c` print raw `type %d` in their
unsupported-node errors):

```
N_REC    /* text = record name; kids = field decls (idents with N_TYPE on ->a) */
```

`struct Node` does not change. Field decls reuse the `N_IDENT` + `a = N_TYPE`
shape that `design_type_system.md` §2.6 already allocates for typed parameters,
so if that document lands first this costs one `parse_type()` call site.

---

## 4. Field access

### 4.1 `.` already parses, and is currently dead

`parser.c:340-356` builds member access today:

```c
else if (is_sym(".")) {                   /* member access */
    advance();
    if (curtype() != T_IDENT) parse_error("expected a name after '.'");
    Node *m = new_node(N_MEMBER);
    m->a = node;
    set_text(m, cur()->text);
    advance();
    node = m;
}
```

and `interp.c:1521-1522` is the only consumer:

```c
case N_MEMBER:
    runtime_error("member access (.) isn't supported yet");
```

`grep -n "N_MEMBER" interp.c compile.c compilef.c compilel.c nativegen.c` returns
exactly those two hits plus the enum declaration at `parser.h:18`. So **`.` is
fully parsed, reserved by `MASTER_PLAN.md:215` ("Reaching into a thing: `.` —
`f.age`, `w.text()`"), and evaluated by nobody.** There is no compatibility
question to answer. That is unusual and worth exploiting.

### 4.2 The lowering

`img.w` where `img`'s declared type is `Image` and `w` is field 0 becomes,
structurally, `img[0]` — an `N_INDEX` with a constant `N_NUM` subscript.

Two places this could happen:

| Where | Cost | Consequence |
|---|---|---|
| In the parser, rewriting `N_MEMBER` → `N_INDEX` | needs the record table at parse time, and needs to know `img`'s type | all five engines get records for free, zero backend edits |
| In each engine, at `N_MEMBER` | five implementations | each engine can choose a better representation than "list" |

The parser cannot do it: resolving `img.w` requires knowing that `img` is an
`Image`, which is a type question, not a syntax one.

**Recommendation: a resolve pass between parse and execute, shared by all
engines** — the same slot `design_type_system.md` §5 puts local inference in. It
walks the tree once, and for every `N_MEMBER` whose object has a known record
type, it *rewrites the node in place* to `N_INDEX` with a literal subscript. An
`N_MEMBER` that survives the pass is an error at that point ("`Image` has no
field `wdith`"), not at runtime, and not five times over.

This keeps the promise that matters: after the pass, the tree handed to
`interp.c`, `compile.c`, `nativegen.c` and `compiler.zl` contains no node they
do not already handle. **The boxed engines need zero changes.**

`compilel.c` is the exception and the point — see §8.2.

### 4.3 Positional access (`t.0`) is impossible, and that is fine

Rust-style `t.0` does not work, and I traced why rather than assuming.
`lex_number` is only entered when the token's first character is a digit
(`lexer.c:290`):

```c
if (isdigit((unsigned char)c))            return lex_number(lx);
```

so the fractional-part check at `lexer.c:110` —

```c
if (peek(lx) == '.' && isdigit((unsigned char)peek_next(lx))) {
```

— only ever fires *after* digits have been consumed. A leading `.` therefore
falls through to `lex_symbol` (`lexer.c:312`), and `t.0` lexes as three tokens:
`t`, `.`, `0`. `parse_postfix` then rejects it at `parser.c:349` — "expected a
name after `.`".

So the failure is clean and in the parser, not a silent mis-lex. Making `t.0`
work would mean special-casing a digit after `.` in `parse_postfix`, which is
cheap, or in the lexer, which is not — number lexing is the most float-sensitive
path in the tree. Either way it buys nothing that `t[0]` does not already do.

Rejected. Positional access is `t[0]`, which already works and always will.

### 4.4 Field writes

`bmp.zl` mutates through the field today (`bmp.zl:56-58`):

```
st_px[st_i] = int(clamp(st_r, 0, 255))
```

after extracting `st_px = st_img[2]` on line 54. For `img.px[i] = v` to work,
`N_MEMBER` must be legal in assignment-target position. `interp.c:1554-1583`
handles exactly two target shapes today, ending at `interp.c:1582`:

```c
} else {
    runtime_error("can only assign to a name or a list index");
}
```

Because §4.2's resolve pass rewrites `N_MEMBER` to `N_INDEX` *before* execution,
`img.w = 5` arrives at `interp.c:1563` as an index assignment and works with **no
change to that switch at all**. The in-place mutation semantics of
`interp.c:1580` (`*list.items[i] = val`) are inherited exactly — records mutate
like the lists they are, including the aliasing that `astar.zl:76-77` warns
about ("zl lists are shared by reference").

Consistency, not a new rule. Good.

---

## 5. Nominal or structural — the honest answer

**Records are nominal to the checker and structural at runtime.**

Statically, `Cell` and a hypothetical `Pair` with the same two int fields are
different types and the checker rejects passing one where the other is wanted.
That is what makes annotations worth writing.

At runtime, in the three boxed engines, a `Cell` *is* a `V_LIST` of length 2.
`type(c)` returns `"list"`. Passing a raw `[3, 4]` into a function that annotates
its parameter `Cell` succeeds. Two records with the same arity are
interchangeable everywhere the checker did not look.

Three options were weighed:

| | Structural at runtime (chosen) | Runtime tag on the value | Nominal, new value type |
|---|---|---|---|
| `Value` grows | no | **yes** — a `char *recname` field on a 48-byte struct that `zl_nil` already memsets 31.2M times per benchmark run | yes |
| `str()` output | unchanged | unchanged unless you also change printing | changes |
| `bmp.zl:237`-style assertions survive | **yes** | yes | no |
| Catches `astar([3,4], ...)` at runtime | no | yes | yes |
| Migration is incremental | **yes** | mostly | no |
| Backend work | zero | three engines | five engines |

The middle column is the tempting one and it is rejected on a measured cost: the
brief established that `zl_nil` memsets 48 bytes 31.2 million times in `b2_arith`
(1.5 GB of zeroing). Widening `Value` to carry a name that only ever produces
better error messages makes the *unboxed* path — the entire reason for this
project's current direction — measurably worse to make the *boxed* path
marginally safer. Wrong trade.

**The cost, stated plainly:** records give you names and static checking. They do
not give you runtime type safety, and `assert(type(c) == "Cell")` will never
work. If that turns out to matter, the tag can be added later without changing
any syntax — `Value` is not part of zl's surface. It is not in v1.

This is the same shape as `design_type_system.md`'s governing invariant:
annotated code may be *rejected*; it may never quietly do something else. An
un-annotated program that passes `[3, 4]` around behaves identically before and
after this feature ships.

---

## 6. Construction

### 6.1 v1: nothing new

```
fn bmp_new(nw_w, nw_h) {
    return [nw_w, nw_h, fill(nw_w * nw_h * 3, 0)]
}
```

is already a valid `Image` constructor. Annotating the return type
(`fn bmp_new(nw_w: int, nw_h: int) -> Image`) makes the checker verify the
literal's arity and field types at that one site. **Zero edits to line 41.**

This is the single most important property of the design and it is why §2 chose
"record is a named list". `astar.zl:165` —

```
for as_d in [[-1, 0], [0, 1], [1, 0], [0, -1]] {
```

— is a `list[Cell]` literal with no changes. Under any design with a dedicated
construction form, that line becomes
`[Cell(-1,0), Cell(0,1), Cell(1,0), Cell(0,-1)]` and the diff touches every
literal in the corpus.

### 6.2 `Image{w: 128, h: 128, px: []}` — rejected, same reason as before

`design_memory_structs.md:664-676` already rejected brace literals for `struct`
and the argument transfers unchanged:

> With a literal form, `if p { … }` is ambiguous — `p` followed by a brace is
> either a condition and a block, or a struct literal. Rust hit this and solved
> it with a "no struct literal in condition position" restriction, i.e. a
> context-sensitive grammar rule that a reader has to know about.

`MASTER_PLAN.md:210` locks braces as *the* block delimiter ("never `end`, never
indentation. Nothing invisible can break"). Rejected.

### 6.3 `Image(128, 128, px)` — deferred, not rejected

A call-shaped constructor parses **today** with zero grammar change: it is an
`N_CALL` with an `N_IDENT` callee, exactly like `fill(...)` and `nil()`. It reads
well, it gives the checker an unambiguous construction site, and it costs no
syntax.

It is deferred out of v1 for one reason: it is pure sugar for a list literal, and
shipping it in v1 invites a second, competing idiom into 60 sites that already
have one. Ship the naming layer, migrate, and then decide whether
`Cell(r, c)` earns its place — with data.

One thing to settle *now*, before it becomes a compatibility problem: if
`Image(...)` is ever added, a record name must not also be callable as a user
function. Reserving record names in the call namespace at declaration time is a
one-line check in the resolve pass and costs nothing to do early.

---

## 7. Destructuring and pattern matching

**Out of v1. Not "later maybe" — deliberately not this feature.**

`design_maps.md` §3.4 already set this precedent for maps:

> Deliberately no `for k, v in m` destructuring — that would need tuple support
> the language doesn't have and doesn't need to self-host.

The tuple support now exists (§2), so the argument has to be remade rather than
cited. It survives:

- **Multi-assignment (`w, h, px = img`) needs a new statement form.**
  `parse_statement` currently parses one expression and checks for `=`.
  Comma-separated targets is a real grammar change, and `MASTER_PLAN.md` §4.2 is
  a locked spec whose whole point is not accumulating forms.
- **The pressure is low.** `bmp_save` unpacks three fields (`bmp.zl:140-142`) and
  that is the worst case in the corpus. Three lines is not a crisis.
- **`for r, c in cells` is the tempting one** and it is the one that most wants a
  scoping rule zl does not have. Assignment inside a function writes the global
  slot when a global of that name exists (§9.2) — a destructuring `for` would
  silently write two globals. Building destructuring before fixing scope would
  bake in a bug.

`switch`/pattern matching over record *shape* is a different feature entirely and
`design_switch_case.md` owns that question.

---

## 8. Memory layout, per engine

### 8.1 `interp.c`, `compile.c` / `runtime.c`, `nativeval.c` — nothing changes

A record is a `V_LIST`. `runtime.h:14-22`:

```c
typedef struct Value {
    ValueType      type;
    double         num;      /* V_NUM, and V_BOOL (0/1) */
    char          *str;      /* V_STR                   */
    struct Value **items;    /* V_LIST                  */
    int            nitems;
    int            cap;
    int           *tip;
} Value;
```

An `Image` is a `Value` with `type = V_LIST`, `nitems = 3`, and three `Value*`.
Field access after §4.2's rewrite is `zl_index(v, zl_num(0))`. Zero lines change
in any of these three engines, and `runtime.c`/`interp.c` parity — which the
brief calls load-bearing — is preserved trivially because neither moves.

The boxing cost is untouched: a three-field record is four `Value`s (144 bytes
of payload plus the list header) and `.w` still walks a pointer. **Records do not
make boxed code faster.** Anyone expecting a speedup from this feature alone is
going to be disappointed, and it should be said out loud.

### 8.2 `compilel.c` (LLVM) — this is the entire performance case

Given `rec Cell { row: int, col: int }`, the natural lowering is

```llvm
%rec.Cell = type { i64, i64 }
```

and `c.row` is `getelementptr inbounds %rec.Cell, ptr %c, i32 0, i32 0` — a
constant offset, no bounds check, no tag test, and eligible for SROA so a
short-lived `Cell` never reaches memory at all. Against today's
`list[int]`-of-boxed-values that is the same 4-6× the brief measured, and against
`design_type_system.md` §3.1's `%zllist = { i64 len, i64 cap, ptr data }` it
still wins on the bounds check and the indirection.

The brief established that "LLVM IS THE OPTIMISER" (`MASTER_PLAN.md` §10) and
that the LLVM backend is at parity with `clang -O2` on integer benchmarks. A flat
record struct is precisely the shape LLVM's existing passes are built to
eliminate. No new optimiser work — that is the point.

**Constraint, stated honestly:** this lowering only applies when every field's
type is representable unboxed. `rec Image { w: int, h: int, px: list[int] }` has
a `list[int]` field, so it needs `design_type_system.md` §3.1's list
representation to exist first. Until then `Image` compiles as `any` — correct,
not fast. The rule is: **a record is as unboxed as its least-unboxed field.**

### 8.3 `compilef.c` (unboxed C, numeric subset)

Same rule, stricter: all fields must be `int`/`float`/`bool`, in which case emit
a C struct with no padding surprises. Any other field type is unsupported, which
`compilef.c` already knows how to say. `Cell` qualifies; `Image` does not.

### 8.4 `nativegen.c` (x86-64 PE)

Base register plus a constant displacement — `mov rax, [rbx + 8]`. This is the
one addressing mode the backend already emits everywhere. Field offsets are
computed by the same layout pass, never by asking a C compiler, per
`design_memory_structs.md` §5.1's rule that there must be exactly one source of
truth for layout. **If `design_memory_structs.md` lands first, `rec` should reuse
its layout function verbatim rather than writing a second one** — see §11.

The brief measured `nativegen` at ~4× off on tight loops for lack of loop
optimisation. Records do not change that either way.

### 8.5 `compiler.zl`

`compiler.zl` will not implement `rec`. Per `design_selfhost_parity.md`'s
recommendation ("keep `compiler.zl` as a bounded-subset engine, but stop letting
it guess"), it should print a message and produce no `out.c` when it meets one.
This is one more entry on a list of 63 files it already mishandles; it does not
change the policy and it does not threaten the fixpoint, because
`compiler.zl` does not use records and the fixpoint's only input is
`compiler.zl` itself.

The specific fixpoint trap the brief identified — `compiler.zl` concatenating a
number onto a string to emit C, so an unboxing change could make gen1 write `"3"`
where gen2 writes `"3.0"` — is **not** on any path this feature touches. Records
add no numeric formatting.

### 8.6 What the three-engine gate will and will not prove

`verify.ps1` will stay green the moment records land, because in the boxed
engines a record is a list and nothing moved. **That greenness proves nothing
about records.** This is the same trap `design_selfhost_parity.md` §1 documents
for the fixpoint: closure is not coverage.

A record test suite therefore has to be written deliberately, and the assertion
that matters is *cross-engine*: the same `.zl` file, run through `interp.exe` and
through `compilel.c`, must print byte-identical output — because those two are
the engines where the record's *representation actually differs* (boxed list vs
flat struct). A test that only runs on the interpreter is a test of the resolve
pass, not of records.

---

## 9. Interaction with the two language-level blockers

The brief names two facts that make per-function type inference unsound. Records
interact with both, and favourably.

### 9.1 `+` is overloaded and it is a tested guarantee

`add(2,3)=5` and `add("n=",5)="n=5"` are locked by `tests/test_syntax.zl`, so no
engine can emit an integer add without knowing both operand types.

Records do not fix this in general, and they do not make it worse (no `+` on
records in v1 — see §13). What they *do* is supply operand types at a large
number of sites for free. In

```
fn manhattan(mh_a: Cell, mh_b: Cell) -> int {
    return abs(mh_a[0] - mh_b[0]) + abs(mh_a[1] - mh_b[1])
}
```

(`astar.zl:45-47`, annotated) the `+` at the top level has two `int` operands
known from the record declaration, with no inference at all. That is the
mechanism by which "23 files collapse from opaque to statically typed."

### 9.2 Global assignment makes per-function inference unsound — records are immune

This is the argument for building records **before** inference.

An assignment inside a function writes the *global* slot when a global of that
name exists. `interp.c:1554-1561` routes every `N_IDENT` target through
`env_assign`, and the stdlib depends on the behaviour so thoroughly that it
evolved a naming convention to defend against it. `bmp.zl:20-24`:

```
# Naming: assignment inside a zl function updates an existing GLOBAL of the
# same name, so every parameter and local carries a two-letter tag for its
# function (st_ = bmp_set, ln_ = bmp_line, ...), the convention bitops.zl
# uses. The demo's check() counters below rely on the opposite behaviour on
# purpose - they ARE globals, so the function updates them.
```

`astar.zl:31-35` says the same thing and names the exact bug it prevents
(`astar()`'s `as_cols` being clobbered by `cell_open()`).

Any scheme that infers a variable's type from its uses inside one function is
wrong under this rule, because another function can retype the same slot.

**A record field's type is not inferred. It is declared once, at the `rec`, and
the record is the only thing that can hold it.** `Image.w` is `int` no matter
which function is executing, because `w` is a field, not a variable. Records are
the largest slice of static typing available in today's language *without*
touching scoping — which is a genuinely hard, genuinely breaking change with 96
stdlib modules written against the current behaviour.

That, more than the LLVM lowering, is why this document argues records are the
highest-leverage next move.

---

## 10. Migration — `stdlib/bmp.zl` end to end

The requirement is that no step breaks a caller, and every step is independently
committable and independently verifiable.

**Stage 0 — today.** `bmp_new` returns `[w, h, px]` (`bmp.zl:41`). Consumers read
`st_img[0]`, `st_img[1]`, `st_img[2]` (`bmp.zl:51-58`, `140-142`). 30+ assertions
in the demo compare via `str()`.

**Stage 1 — declare, change nothing.**

```
rec Image {
    w:  int
    h:  int
    px: list[int]
}
```

Nothing else in the file changes. Every function still takes an untyped
`st_img`. The declaration is inert: no annotation refers to it, so no check runs.
`verify.ps1` green, `bmp_demo.bmp` byte-identical.

**Stage 2 — one accessor at a time.** Replace `st_img[0]` with `st_img.w` in
`bmp_set`, and nothing else in the file:

```
if st_x < 0 or st_y < 0 or st_x >= st_img.w or st_y >= st_img.h {
```

This is only legal once `st_img` has a declared type, so in practice Stage 2 is
"annotate one function's parameters, then rewrite that function's field reads":

```
fn bmp_set(st_img: Image, st_x: int, st_y: int, st_r: int, st_g: int, st_b: int) -> bool {
```

`bmp_set`'s *callers* are untouched — `bmp.zl:97`, `117`, `130` still pass a raw
list, and a raw list is a legal `Image` (§5). The `str()`-based assertions are
untouched because the value is untouched. **The unit of migration is one
function.**

**Stage 3 — the constructor.** `bmp_new` gets `-> Image`, which makes the
checker verify that line 41's literal has three fields of the right types. Line
41's *text* still does not change.

**Stage 4 — the sentinel.** `bmp_get` returns `[r, g, b]` on success and
`[0-1, 0-1, 0-1]` out of range (`bmp.zl:63`). Declare `rec Rgb { r: int, g: int, b: int }`
and both are `Rgb`; the out-of-range case is an `Rgb` holding `-1`s, which is
exactly as ugly as it is today and no uglier. Making it *properly* nullable is
Tier C and belongs to whatever design covers `nil()`-as-sentinel — 13 files, 33
functions, per the brief. Records do not solve it and this document should not
pretend otherwise.

**Stage 5 — reap.** Once every function in the file is annotated, `Image` is a
record whose `px` field is a `list[int]`, and `compilel.c` can lower the whole
module against `design_type_system.md` §3.1's representations. Not before.

**Verification at every stage:** `bmp.zl` writes real files and asserts on the
byte counts (`bmp.zl:255`, `263`) and on 20+ pixel reads. Re-running it after
each stage and diffing `examples_out/bmp_demo.bmp` against a saved copy is a
sharper regression test than anything a synthetic record suite would provide.
That is a good reason to migrate `bmp.zl` **first** even though it is not the
worst offender.

### 10.1 The same shape, applied to the other three sites read for this document

| File | Today | After |
|---|---|---|
| `astar.zl:5` | `[row, col]` documented in a comment | `rec Cell { row: int, col: int }`; `mh_a[0]` → `mh_a.row`. The literals at `astar.zl:165` and the `push` at `astar.zl:157` do not change. |
| `dijkstra.zl:3-6` | `[node, [[neighbor, weight], ...]]` | `rec Edge { to: str, w: float }` and `rec Dist { node: str, d: any }`. `dj_get`'s `dg_pair[0]`/`[1]` (`dijkstra.zl:41-42`) become `.node`/`.d`. **The linear scan stays a linear scan** — records are not maps. |
| `argparse.zl:120` | `return [flags, opts, pos]` | `-> Args`. `has_flag`/`get_opt`/`positionals` (`argparse.zl:123-138`) become one-line field reads and are then arguably deletable — but deleting them is a public-API change and a separate commit. |

`dijkstra.zl`'s `Dist.d` is `any` because the module's own header
(`dijkstra.zl:16-21`) insists unreachable is `nil()` and not a sentinel number.
That field stays boxed. Correct, and an honest example of §8.2's least-unboxed-
field rule biting.

---

## 11. Relationship to `design_memory_structs.md` — the biggest open question

Two documents now propose a thing with named fields. The boundary I believe is
right:

| | `rec` (this doc) | `struct` (`design_memory_structs.md`) |
|---|---|---|
| What the value is | a zl list | **an address** (§2.2) |
| Field types | `int` `float` `bool` `str` `list[T]` `any`, other records | `i8`…`u64` `f64` `ptr`, nested structs, `T[N]` |
| Lifetime | whatever zl values do | bump allocator, `free` does nothing (§3.3) |
| Safety marker | none needed | `!` on `alloc`/`poke` (§3.5) |
| Purpose | 60 stdlib sites; readability + unboxing | ABI compatibility with Win32/kernel |
| Floor | 1 | 5 |
| Can a field hold a zl string? | yes | **no** — §4.2: "`str` is deliberately *not* a field type" |

`design_memory_structs.md` §6.5 already drew a crisp line between `struct` and
`map` ("`.` is claimed by structs and `[k]` by maps"). This proposal **contests
that**: `rec` also wants `.`, and `rec` is a Floor-1 feature while `struct` is
Floor 5.

Three ways this resolves:

**(a) Two features, two words, `.` shared.** `.` means "field of a record" or
"field of a struct" depending on the declared type of the object, resolved by the
same pass in §4.2. Costs: two layout stories, two documents, and users must know
which one they have. This is what the table above assumes.

**(b) One feature, `rec` is `struct` with a different backing.** Unify the
declaration and let the *field types* decide: all-`i32`/`ptr` fields means raw
memory, zl-typed fields means a list. Elegant on paper and I do not trust it —
`design_memory_structs.md` §4.2 argues at length that silently boxing at a struct
boundary "is exactly the category of magic that makes a systems type unusable,"
and this makes the boxing decision implicit in a field type list.

**(c) Records first, structs later, and structs subsume records.** Ship `rec`
now (Floor 1, 60 sites, no raw memory). When W5 arrives, `struct` is a separate
word, and `rec` either stays or is quietly redefined as sugar.

**Recommendation: (a), with the layout function shared.** Whichever of the two
documents implements first owns `parse_type()`, the field-offset computation, and
the `N_MEMBER` resolve pass; the second reuses them. That is the same "Stage 0
decides which builds the shared parser code" arrangement
`design_memory_structs.md` §6.4 made with the type system. **This needs a human
decision before either is built** — it is the one place where building the wrong
thing first is expensive.

---

## 12. Staged implementation plan

Each stage is independently shippable and independently green.

**Stage 0 — decide.** Settle §11 (`rec` vs `struct` ownership), §3.4 (are field
annotations optional?), and whether `design_type_system.md` Stage 1 lands first
(it should — `rec` wants `parse_type()`). No code.

**Stage 1 — syntax only, provably inert.** `parse_statement` recognises
`rec IDENT {`, builds `N_REC`, stores it in a table. Nothing reads the table.
Add `print_node` support so `parser.exe`'s dump stays complete. Test: a file with
a `rec` declaration and no field access runs identically on all engines. This is
the same "provably inert" opening move `design_type_system.md` §8 Stage 1 uses,
and for the same reason — it decouples the grammar risk from the semantics risk.

**Stage 2 — the resolve pass, interpreter only.** Rewrite `N_MEMBER` → `N_INDEX`
for objects with a known record type; error by name on unknown fields. Requires
enough type knowledge to know `st_img` is an `Image`, so it needs annotations —
hence Stage 0's ordering constraint. `interp.c:1521`'s "isn't supported yet"
error becomes unreachable for resolved members and stays as the error for
unresolved ones.

**Stage 3 — the boxed backends.** Should be a no-op by construction (§8.1). The
deliverable is the *test* that proves it: a record-using `.zl` file whose output
is byte-identical under `interp.exe`, `compile.c` and `nativegen.c`.

**Stage 4 — migrate `bmp.zl`.** §10, all five stages, with the `.bmp` byte-diff
as the regression gate. Then `astar.zl` (`Cell` is the simplest possible record)
and `argparse.zl`.

**Stage 5 — `compilel.c` flat lowering.** Only for records whose fields are all
representable unboxed. Gated on `design_type_system.md` Stages 3-6. This is where
the speed arrives, and it arrives last.

---

## 13. What I am not sure about

Listed as questions, not hidden as assumptions.

1. **§11 is genuinely unresolved.** Whether `rec` and `struct` are one feature is
   a judgement about where zl is going, not a fact about the code. I lean two,
   but I would not build either until a human picks.
2. **Optional field annotations (§3.4) cut both ways.** `rec Cell { row, col }`
   makes records useful immediately and makes them useless for unboxing. If the
   answer is "annotations required", migration gets slower and the payoff gets
   nearer. I recommended optional; I am about 60/40 on it.
3. **`==` on records — I checked, and it already works.** I nearly wrote the
   opposite here on the strength of `design_maps.md` §3.1, which says
   `values_equal` "compares numbers and strings and refuses lists/maps". That is
   stale: `interp.c:1236-1256` compares lists element-wise and recursively,
   with a depth guard —

   ```c
   case V_LIST:
       if (l.nitems != r.nitems) return 0;
       if (l.items == r.items) return 1;   /* the same list */
       ...
       for (int i = 0; i < l.nitems; i++)
           if (!values_equal_depth(*l.items[i], *r.items[i], depth + 1)) return 0;
   ```

   So record equality is structural and **free** — `c1 == c2` works the day
   `rec` ships, with no proposal needed. It also means `astar.zl:254` —

   ```
   if first(pv_path)[0] != pv_start[0] or first(pv_path)[1] != pv_start[1] {
   ```

   — could already have been `first(pv_path) != pv_start`, and with `Cell` it
   reads that way naturally. One more site records improve. What I have *not*
   checked is whether `runtime.c` and `nativeval.c` implement the same recursive
   comparison; the brief calls interp/runtime parity load-bearing, so that is
   worth confirming before relying on it in compiled code.
4. **Records inside `str()`.** I asserted output is unchanged because the value
   is unchanged. I did not run the case where a record's *field* is another
   record. It should be unchanged for the same reason, but I did not verify it.
5. **How many of the 60 sites are genuinely single-typed.** I counted the
   syntactic shape, not the semantics. Some of those 60 are Tier A returns that
   should stay lists. The real number of migratable sites is somewhere below 60
   and I did not establish it.
6. **Whether the resolve pass is one pass.** §4.2 assumes a single walk suffices
   to know every object's record type. With chained access (`g.nodes[0].edges[1].to`)
   and function return types, it may need the same fixpoint iteration
   `design_type_system.md` §5.2 found necessary for local inference. I would
   plan for iteration and be pleasantly surprised.
7. **`nativeval.c`.** I did not read it. §8.1 assumes it inherits the boxed-list
   behaviour like the other boxed engines. Someone should check before Stage 3
   claims to be a no-op.
