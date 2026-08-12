# Design: Tooling — Formatter, Module System, Debug Info, LSP

**Status:** proposal / nothing built
**Author:** tooling design pass, 2026-08-02
**Scope:** four things zl has none of. Touches `lexer.c`, `parser.c`/`parser.h`,
`interp.c`, `compilel.c`, and adds up to three new tools. Changes no language
semantics except where explicitly called out (§3.2). No code is changed by this
document.

---

## 0. TL;DR

zl runs. It compiles itself. It has 96 stdlib modules, 8 suites, 2107 checks,
four backends, and one of them is at parity with `clang -O2` on integer code.
It also has no way to say "use that other file", no way to lay out code
consistently, no way to find out *where* a program failed, and no editor
integration. Those are the four things between "a language that works" and "a
language somebody uses on a Tuesday".

Three findings that change the obvious ordering:

1. **The module system is not an ergonomics problem, it is a correctness
   problem.** `include` does not exist — not in `lexer.c`, `parser.c`,
   `interp.c`, or `compiler.zl`. The workaround already in the tree is *copying
   library source into the file that needs it*. `tests/test_algorithms.zl`
   states it in its own header: the modules under test are "inlined below,
   copied verbatim from stdlib/sortx.zl, stdlib/astar.zl and
   stdlib/dijkstra.zl". **The suite therefore proves nothing about the shipped
   module.** Ten `.zl` files define `fn check(label, got, want)` verbatim.

2. **The AST has no source position.** `Token` carries a line (`lexer.h:23-27`);
   `Node` does not (`parser.h:33-42`). Everything downstream inherits that:
   `runtime_error` at `interp.c:219-226` prints `runtime error: %s` with no
   file, no line, no function. Debug info, LSP diagnostics, and any error
   message from a running program all need the same ~60-line change, and none of
   them can be built before it.

3. **The formatter is cheaper *and* more dangerous than it looks.** The AST
   pretty-printer everyone reaches for cannot be written safely, because
   `lexer.c:88` silently truncates any token text at 127 characters
   (`if (len >= MAX_TEXT) len = MAX_TEXT - 1;   /* never overflow */`) and
   `lexer.c:272-273` throws comments away entirely. A tool that rebuilds source
   from tokens or from the tree deletes every comment in the corpus and silently
   corrupts any string literal ≥128 bytes. The right formatter is not a
   pretty-printer at all — see §3.

**Recommended order:** (0) line numbers in `Node` → (1) re-indenter →
(2) `include` → (3) debug info via LLVM metadata → (4) LSP.

**Cheapest single item that most improves daily use:** the re-indenter, ~150
lines, no parser change, no AST change, comments and long strings safe by
construction. **Highest-value item overall:** `include`, and it is blocked on
`docs/design/design_scoping_decision.md`.

---

## 1. What is actually there today

| Capability | Status | Evidence |
|---|---|---|
| Full AST for the whole surface syntax | yes | `parser.h:11-31`, 22 node kinds |
| Tree walker to copy for a printer | yes | `print_node`, `parser.c:981-1030` |
| Line number on tokens | yes | `Token.line`, `lexer.h:26` |
| Column number on tokens | **no** | `Token` is `{type, text[128], line}` |
| Line number on AST nodes | **no** | `parser.h:33-42` |
| Comments in the token stream | **no** | `lexer.c:272-273` skips to EOL |
| Parser error recovery | **no** | `parse_error` → `exit(1)`, `parser.c:67-72` |
| Any include/import | **no** | zero hits in `lexer.c`, `parser.c`, `interp.c`, `compiler.zl` |
| Runtime error location | **no** | `interp.c:219-226` |
| Any `!dbg` metadata in LLVM output | **no** | zero hits in `compilel.c` |
| Multi-line string literals | **no** (helpfully) | `lexer.c:160-165` hard-errors |

Two of those "no"s are load-bearing in ways that are not obvious, and they are
where the interesting design work is.

---

## 2. Item 1 — the module system

### 2.1 The problem is not reuse, it is drift

`docs/design/design_modules.md` (2026-07-29) already proposed this and costed
Option A (textual include) at ~40 lines. It was not built. In the four days
since, the corpus grew to 111 `.zl` files and the absence turned into
duplication with a correctness consequence:

- `binary_search` exists in `stdlib/searching.zl`, `stdlib/sortx.zl`, **and**
  `tests/test_algorithms.zl`. Three copies, one of which is the test.
- `fn check(label, got, want)` — the entire assertion harness — is defined
  verbatim in **10** separate files.
- `tests/` is 6,557 lines. A meaningful fraction of that is library code pasted
  in so the test can call it.

The consequence is precise: **fixing a bug in `stdlib/sortx.zl` does not make
`tests/test_algorithms.zl` fail differently, because the test is not calling
`stdlib/sortx.zl`.** The 2107 checks are green against copies. This is the same
class of failure as the self-hosting gate described in the brief — `verify.ps1`
proves closure over `compiler.zl`, not coverage — and it has the same shape:
a green signal that does not mean what it looks like it means.

That is why this is first on value. It is not "it would be nice to reuse code".
It is "the test suite is testing a photocopy".

### 2.2 Take Option A from design_modules.md; do not redesign it

`design_modules.md` §7 recommends textual include over namespaced import,
because `include` is a pre-lexer text pass — the lexer, parser, and all six
codegens are untouched, and it costs zero reserved words. That reasoning is
still correct and I am not relitigating it. Two things have changed since it was
written, and both are additions, not reversals.

**Addition 1 — the line-origin map is now mandatory, not optional.**
`design_modules.md` §6.1 lists "line numbers drift after splicing" as a known
limitation with a cheap deferred fix. Once `Node` carries a line (§3.1 below),
that limitation stops being cosmetic: every error message, every `!DILocation`,
and every LSP diagnostic would point into a synthetic concatenated buffer that
exists nowhere on disk. The `line_origin` array — one `(file, line)` pair per
spliced output line, built during `expand` — has to ship *with* include, not
after it. Budget **+60 lines** on top of the §5.3 estimate, and make it a
`const char *origin_file` / `int origin_line` pair that `Token` carries instead
of a bare `line`.

**Addition 2 — include's safety depends on an unresolved decision.**
Textual include is one flat global namespace. zl's assignment rule
(`docs/design/design_scoping_decision.md`) says a plain assignment inside a
function writes the **global** slot when a global of that name exists. Combine
the two and an included library's ordinary local variable silently clobbers the
includer's global of the same name. This is not hypothetical — it is why
`stdlib/sortx.zl` prefixes its locals `sx_`, why `stdlib/bignum.zl` prefixes
`bn_`/`ba_` (`stdlib/bignum.zl:9-18`), and why `design_scoping_decision.md`
counts 16 stdlib files doing this by hand. Today the convention is a nuisance.
Under `include` it becomes the only thing preventing action at a distance across
files, enforced by nothing.

If `design_scoping_decision.md` lands on Option 2 (assignment binds locally,
explicit `global`), include is safe and the prefix convention can be retired
gradually. If it lands the other way, include ships with a hard rule that every
stdlib file must prefix its locals, and ideally a lint that checks it. **Either
way, sequence include after that decision.** That is the reason it is not
number one in the build order despite being number one in value.

### 2.3 Cost

| Piece | Lines | Where |
|---|---:|---|
| `expand_includes()` text pass | 60 | `lexer.c`, called by `lex_file` |
| `line_origin` map + `Token` origin fields | 60 | `lexer.c`, `lexer.h`, error printers |
| Cycle / include-once guard | 20 | `lexer.c` |
| Self-hosted `expand` in zl | 45 | `compiler.zl` (sketch exists, §5.4) |
| De-inlining the 8 suites | ~-1,500 | `tests/*.zl`, deletion |
| **Total new code** | **~185** | |

**Unlocks:** tests that test the shipped module; `compiler.zl` splittable into
lexer/parser/codegen; a stdlib that is a library instead of a snippet archive;
prerequisite for any package manager later.

---

## 3. Item 2 — the formatter

### 3.1 The pretty-printer is a trap

The obvious design — walk the AST, emit canonical source, ~300 lines, reuse
`print_node`'s shape — does not survive contact with `lexer.c`.

**Comments are not in the tree.** `lexer.c:272-273`:

```c
} else if (c == '#') {                        /* comment to end of line */
    while (peek(lx) != '\n' && peek(lx) != '\0') advance(lx);
```

They are consumed inside `skip_whitespace` and never become tokens. An
AST-driven formatter would emit a corpus with every comment deleted. In this
corpus that is catastrophic, not cosmetic: `examples/life.zl` opens with 25
consecutive comment lines that are the entire explanation of the program, and
the stdlib's contracts are documented in comments (`stdlib/dict.zl:1-2`).

**Token text is silently truncated.** `lexer.c:88`:

```c
if (len >= MAX_TEXT) len = MAX_TEXT - 1;   /* never overflow */
```

with matching guards at `lexer.c:168`, `:200`, `:211`. `MAX_TEXT` is 128
(`lexer.h:10`). A 200-character string literal lexes to its first 127 bytes with
no diagnostic. Nothing in the corpus currently hits this — I grepped `stdlib/`
for string literals ≥110 characters and found none — so the bug is latent. But a
tool whose entire job is *rewrite this file in place from the token stream*
turns a latent truncation into silent data loss on the day someone writes a long
literal. There is no round-trip test that would catch it.

Making the pretty-printer safe therefore means: teach the lexer to emit
`T_COMMENT` tokens, decide an attachment rule (leading/trailing/dangling — the
part of every formatter that is actually hard), give `Node` a comment list, and
make token text heap-allocated or verify a re-lex round trip. That is a real
project — call it **550-700 lines** — and it lands in `lexer.c` and `parser.c`,
the two files most likely to be in flight for other reasons.

### 3.2 The re-indenter is the version worth building

Do not reconstruct source. **Only rewrite leading whitespace.**

```
for each line:
    lex the line          -> tokens, used only for classification
    depth -= count of leading T_SYMBOL "}"
    emit  (4 * depth spaces) + line-with-leading-whitespace-stripped
    depth += (T_SYMBOL "{" count) - (T_SYMBOL "}" count)
```

Every byte of the line after its indent is copied verbatim from the original
buffer. The lexer is used to *classify* braces — so a `"{"` inside a string
literal or inside a `#` comment does not move the indent — but never to
*reconstruct* text. Truncation cannot bite, because truncated text is never
emitted. Comments cannot be lost, because comment bytes are copied like any
other bytes. F-string braces are already handled by the lexer as a single
`T_FSTRING` token (`lexer.h:19`), so `f"{x}"` cannot corrupt the depth count.

This works because of an accident of the language that is worth noting: zl has
**no multi-line string literals**. `lexer.c:160-165` hard-errors on a newline
inside a string ("string never closed"). Every physical line is a complete
lexical unit. That is what makes line-granular rewriting sound; in Python or C
it would not be.

**What it cannot do:** wrap long lines, normalise spaces around operators,
align `=`, or reorder anything. **What it does do:** fix the inconsistency that
actually exists. Measured across `stdlib/`: 96 files use a 4-space indent
somewhere, and **22 of them also contain statement lines indented by exactly 2**
— `stdlib/huffman.zl` (74 such lines), `stdlib/graphx.zl` (73),
`stdlib/sudoku.zl` (62), and `stdlib/bignum.zl:9-13`, whose whole body is
2-space. Zero files use tabs; zero use brace-on-next-line. So the corpus is
already consistent on brace style and whitespace character, and inconsistent on
exactly one axis — indent width — which is the one axis a re-indenter fixes.

This is the honest version of "a formatter pays off immediately in consistency
across 96 stdlib modules". The payoff is real but narrower than it sounds,
because the corpus is ~77% consistent already. The reason to build it anyway is
that it costs almost nothing and it makes the *next* 96 modules free.

### 3.3 Cost

| Option | Lines | Comments safe? | Long strings safe? | Parser change? |
|---|---:|---|---|---|
| A — AST pretty-printer | 300 | **no, deletes all** | **no, truncates** | no |
| B — comment-aware pretty-printer | 550-700 | yes | needs heap tokens | yes, `lexer.c` + `parser.c` |
| **C — line-granular re-indenter** | **150** | **yes, by construction** | **yes, by construction** | **no** |

**Recommend C.** Ship it as `fmt.exe <file>` writing to stdout, plus a
`--write` flag and a `--check` mode that exits non-zero — the `--check` mode is
what makes it usable in `verify.ps1` later. Upgrade to B only if line-wrapping
becomes a felt problem, and only after `T_COMMENT` exists for the LSP anyway
(§5), at which point B's marginal cost drops to ~350.

**Unlocks:** consistent diffs; a `--check` gate; and a forcing function to add
`T_COMMENT` when you do want B.

---

## 4. Item 3 — debug info

### 4.1 The prerequisite: `Node.line`

Nothing here is possible until the AST knows where it came from. Today:

```c
typedef struct Node {
    NodeType type;
    char     text[MAX_TEXT];
    struct Node *a, *b, *c;
    struct Node **kids;
    int          nkids;
} Node;                                  /* parser.h:33-42 — no line */
```

`node_new` at `parser.c:35` is the single allocation point for every node, and
the parser's cursor (`parser.c:59-62`) has `cur()->line` available at that
moment. So the change is: add `int line;` to `Node`, set it in `node_new` from
`cur()->line`, done. Roughly **60 lines** once you count the error printers that
should start using it.

The first payoff is not a debugger. It is that `runtime_error`
(`interp.c:219-226`) currently prints:

```
runtime error: index out of range
```

and could print the line. For a language whose corpus includes a 974-line test
file, that difference is worth more per day than a debugger will be. Do this
first regardless of which tooling item ships.

Add a column to `Token` at the same time (`lexer.h:23-27` has `line` only). It
is a handful of lines in `lexer.c` and the LSP cannot exist without it; doing it
twice is silly.

### 4.2 Debug info splits into one cheap job and one expensive one

**Cheap: LLVM metadata in `compilel.c`.** `compilel.c` emits LLVM IR as *text*
and hands it to `clang` (per the brief, and there are currently zero `!dbg`
references in the file). That means we never touch DWARF or CodeView bytes
ourselves. Emit `!DIFile`, `!DICompileUnit`, one `!DISubprogram` per `N_FN`, a
`!DILocation` attached to each emitted statement, and `!DILocalVariable` +
`llvm.dbg.declare` per local. clang generates DWARF with `-g`, and generates
**CodeView from the same metadata** with `-gcodeview` on an MSVC target. One
implementation, both formats, both debuggers. Estimate **250-400 lines** of
additional emission plus the `Node.line` prerequisite.

There is a second accident working in our favour: zl has exactly one number type
(a double — `interp.c:264` comment, "zl's only number type is a double"), so the
`!DIBasicType` table today is essentially one entry. That gets meaningfully
harder the moment `design_records.md` and `design_maps_v2.md` land and there are
aggregate types to describe. **If debug info is wanted at all, it is cheapest to
prototype now and extend, rather than to retrofit onto records later.**

**Expensive: `nativegen.c`.** We write the PE ourselves. CodeView means emitting
`.debug$S`/`.debug$T` sections and, for anything a modern debugger will load
comfortably, a PDB — a format whose practical reference is Microsoft's
`microsoft-pdb` repo rather than a specification. Realistically **1,500-3,000
lines**, high risk, and it buys source-level debugging on the backend that
MASTER_PLAN §10 explicitly designates as *maintained, not optimised*, and that
is ~4x off LLVM on tight loops anyway.

**Recommendation: do the LLVM half; do not do the nativegen half.** Extend §10's
policy from "nativegen is not optimised" to "nativegen is not debugged" — debug
on `interp.exe` (line numbers, §4.1) or on the LLVM build, and treat the
own-x64 backend as an artifact you verify by output equality in
`run_tests.ps1`, which is what it already is.

**Unlocks:** breakpoints and stepping in WinDbg/Visual Studio/lldb on the fast
path; usable stack traces; profilers that attribute samples to zl lines rather
than to `zl_binop`.

---

## 5. Item 4 — the LSP

### 5.1 The blocker is real and it is `parse_error`

The brief asked me to confirm that the parser exits on first error. Confirmed
(`parser.c:67-72`):

```c
static void parse_error(const char *msg)
{
    fprintf(stderr, "line %d: %s (got '%s')\n",
            cur()->line, msg, cur()->text);
    exit(1);
}
```

Called from at least 15 sites (`parser.c:102, 322, 349, 732, 789, 799, 826, 863,
871`, plus direct `exit(1)` for f-string errors at `:164, :170, :234, :251`). A
language server whose only reaction to an in-progress edit is to terminate the
process is not a language server. Half-typed code is the *normal* state of a
buffer.

Converting this is mechanical but not small: `parse_error` records into a
diagnostic list and returns; each call site needs a defined recovery — the usual
choice is synchronise to the next `T_NEWLINE` or closing `}` — and every caller
must tolerate a `NULL` or error node coming back. That is ~200 lines of careful
surgery across `parser.c`, and it changes behaviour for every existing tool that
links the parser. It should be gated behind a flag (`parse_recovering()`) so
`interp.exe` and the backends keep exiting on first error and nothing about the
self-hosting gate moves.

### 5.2 What is *not* a blocker

Incremental parsing. The brief flags it, and for a large language it is the hard
part — but zl files are small. The largest in the tree is `tests/test_text.zl`
at 974 lines; `compiler.zl` is 15,902 bytes. A full re-lex and re-parse on every
keystroke is microseconds. **Skip incremental parsing entirely.** Build it if
and when a file gets big enough to notice, which on this corpus is never.

One caveat: `node_new` (`parser.c:35`) mallocs and nothing frees. Re-parsing on
every keystroke leaks a whole tree each time. At tree-per-parse sizes this is
megabytes per session — survivable, but the honest fix is an arena per document
version, freed wholesale on the next parse. ~40 lines.

### 5.3 Cost

| Piece | Lines |
|---|---:|
| Parser error recovery + diagnostic list | 200 |
| Column tracking in `Token` (shared with §4.1) | 30 |
| JSON encode/decode in C | 300 |
| LSP framing (`Content-Length`), stdio loop, dispatch | 250 |
| Document store, arena per version | 140 |
| `publishDiagnostics` | 50 |
| Symbol table walk (fns, globals) for hover / go-to-definition / completion | 300 |
| VS Code client extension (JS + package.json) | 100 |
| **Total** | **~1,370** |

That total assumes the AST already has positions (§4.1) and assumes no
semantic analysis — hover shows a signature, not a type, because zl has no
types yet. Once `design_type_system.md` and `design_records.md` land there is a
second, larger LSP project to surface them.

**Unlocks:** inline diagnostics, go-to-definition across the 96-module stdlib
(which is much more valuable *after* `include` exists, since today the answer
to "where is this defined" is "in this file, pasted"), and the single thing
that most changes how a language feels to a newcomer.

---

## 6. The order, and why

```
0. Node.line (+ Token column)      ~90 lines   unblocks 2 of the 4 items
1. Re-indenter (fmt.exe, option C) ~150 lines  blocked on nothing
2. include + line-origin map       ~185 lines  BLOCKED on design_scoping_decision.md
3. Debug info via LLVM metadata    ~350 lines  needs 0; easier before records land
4. LSP                             ~1,370 lines needs 0 and parser recovery; best after 2
```

The ordering is not purely value-descending. `include` is the highest-value item
and it is second, because it is the only one gated on an unresolved language
decision (§2.2). The re-indenter goes first among the four precisely because it
is blocked on nothing at all — it is the work you do *while* the scoping
question resolves, and it costs less than the deliberation.

Debug info before LSP is a closer call and could go either way. LSP is worth
more to a person writing zl; debug info is worth more to a person debugging the
compiler, and it gets more expensive the longer you wait, because
`design_records.md` and `design_maps_v2.md` will multiply the type metadata it
has to describe. If those designs land before you start, flip 3 and 4.

**The single cheapest item that would most improve daily use: the re-indenter,
~150 lines.** No parser change, no AST change, no lexer change, comments and
oversized string literals safe by construction, and it is the only item on this
list that can be built without coordinating with anyone.

**The single cheapest change in this document, full stop, is `Node.line` at ~60
lines** — and it is not tooling. It just makes every error zl has ever printed
say where it happened. If only one thing gets built, build that.

---

## 7. Honest total, and what I did not verify

**Total new code for all four items: ~2,150 lines**, of which ~1,370 is the LSP.
Everything except the LSP fits in ~780 lines — smaller than `nativegen.c`
(509 lines) plus `compilel.c` (448). The four items depend on: `Node.line`
(items 3 and 4), `Token` columns (item 4), parser error recovery (item 4 only),
`design_scoping_decision.md` resolving (item 2), and clang already being on the
path (item 3 — it is, `compilel.c` shells out to it today).

Deliberately excluded: a **package manager**. `include` with relative paths is
the whole module system this project needs at 111 files. Registries, version
solving, and lockfiles are a solution to a problem zl does not have and should
not manufacture.

What I did not verify, and would check before writing code:

- **Column tracking.** `Token` has `line` and no column (`lexer.h:23-27`). I
  assume adding one is a few lines in `lexer.c`'s advance path, but I did not
  read that path closely enough to cost it precisely.
- **Whether the 2107 checks actually run against inlined copies.** I established
  that copies exist (10 files defining `check`, `binary_search` in three files,
  `tests/test_algorithms.zl`'s own header saying so). I did not audit all 8
  suites to say what fraction of checks exercise a copy versus the shipped
  module. That number matters — it is the size of the correctness hole — and it
  should be measured before §2 is used to justify anything.
- **F-string interaction with the re-indenter.** I am relying on `T_FSTRING`
  being one token (`lexer.h:19`) so braces inside it never reach the depth
  counter. `parser.c:164-254` re-lexes f-string interiors via `lex_text`, which
  suggests the outer token does hold the whole literal — but I did not confirm
  that the *lexer* keeps it whole rather than the parser reassembling it. If it
  does not, the re-indenter needs one extra guard.
- **Whether `compilel.c`'s IR emission has a natural per-statement hook** to
  hang `!DILocation` on. I confirmed there is no `!dbg` today; I did not read
  its emission structure to confirm the insertion point is one place rather than
  forty.
- **PDB cost.** The 1,500-3,000 line estimate for CodeView in `nativegen.c` is
  from general knowledge of the format, not from having written one. Treat it as
  "large and risky" rather than as a number. It is a recommendation to *not*
  build, so the imprecision is cheap.
