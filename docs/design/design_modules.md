# Design Proposal — An Include / Module System for zl

**Status:** proposal · **Author:** design pass · **Date:** 2026-07-29
**Floor:** widens Floor 1 (language). Does **not** block or depend on any other floor.

---

## 1. Motivation

Every `.zl` file is currently standalone. There is no way to reuse code across
files, so the moment a second program wants `is_digit`, `cstr`, or a string
helper, it gets copy-pasted. `src/selfhost/compiler.zl` is already ~720 lines in one file for
exactly this reason — the lexer, parser, and codegen can't be split apart even
though they are three separable concerns.

The self-hosting spec (§4.5) **deliberately cut "modules"** from Floor 1, and
that call was correct: full namespaced modules are not needed to self-host. But
a *minimal* code-sharing mechanism now buys real leverage:

- Split `src/selfhost/compiler.zl` into `src/selfhost/lexer.zl` + `src/selfhost/parser.zl` + `codegen.zl` and reuse the
  standalone stepping-stone files instead of maintaining two copies.
- Grow a tiny standard library (`stdlib/` already exists as an empty dir) that
  programs pull in rather than re-declaring string helpers.
- Keep future rule-language work (Floor 3) able to `include` shared detection
  helpers.

The design goal is the **smallest thing that removes copy-paste**, with a clean
upgrade path to namespaces if real name-collision pain ever appears.

---

## 2. Two options considered

### Option A — Textual include (`#include`-style)

Splice another file's text into this file before lexing. One flat program, one
global namespace. This is what C, assembly `%include`, and PHP `require` do.

```
include "stdlib/strings"  # proposed module name; not implemented yet

print(upper_first("zac"))   # upper_first defined in strings.zl
```

- **Pro:** near-zero implementation. It is a pure text pass *before* the lexer,
  so **the lexer, parser, and codegen are untouched**. `include` never becomes a
  token or a keyword — it costs **zero reserved words** (§4.4 stays frozen at 11).
- **Pro:** the codegen already hoists every `fn` to a forward declaration
  (`compile_program`), so a concatenated program resolves cross-file references
  automatically regardless of include order.
- **Con:** flat namespace — two files can't both define `lex()`. Collisions are
  a hard error, not a scoped shadow.
- **Con:** no encapsulation or explicit exports; everything a file defines is
  visible to everyone that includes it (transitively).

### Option B — Namespaced modules (`import ... as`)

Each file is its own namespace; you reach in with `.`.

```
import "math.zl" as math

print(math.sqrt(2))
```

- **Pro:** no collisions, real encapsulation, explicit surface via exports.
- **Con:** requires **new grammar** — the self-hosted parser (`src/selfhost/compiler.zl`,
  `parse_postfix`) does not handle `.` member access at all today. It would need
  a `.`-call form plus symbol **name-mangling** in codegen
  (`math.sqrt` → `zl_fn_math__sqrt`), an export marker, and a module resolver
  that survives self-hosting. That is a multi-session change touching all three
  stages.
- **Con:** directly contradicts §4.5's "modules cut for Floor 1" and the §8
  feature-creep risk.

### Recommendation

**Ship Option A now. Keep Option B as a documented future upgrade.**

Textual include is ~40 lines, changes nothing downstream, and immediately lets
`src/selfhost/compiler.zl` be split into files. Namespaces are a strictly larger change that
should wait until (a) `.` member access exists in the self-hosted parser and
(b) a real collision problem has actually been felt — not before. The two are
compatible: `include` can keep meaning "splice flat" even after `import`
lands for "splice into a namespace."

---

## 3. Proposed syntax (Option A)

```
include "relative/path/to/file.zl"
```

Rules:

- The directive is a **line** whose first non-blank word is `include`, followed
  by a **double-quoted path**, followed by only whitespace/comment.
- Path is resolved **relative to the directory of the file that contains the
  directive**, not the working directory. So `src/selfhost/compiler.zl` including
  `"src/selfhost/lexer.zl"` finds its sibling regardless of where the compiler is invoked.
- It is a top-level directive. By convention it goes at the top of the file, but
  because `fn` declarations are hoisted, it may appear anywhere at column 0.
- Comments after it are fine: `include "strings.zl"   # text helpers`.

Example — splitting the compiler:

```
# src/selfhost/compiler.zl (after)
include "src/selfhost/lexer.zl"
include "src/selfhost/parser.zl"
include "codegen.zl"

input = read("input.zl")
prog  = parse_program(input)
write("out.c", compile_program(prog))
```

---

## 4. Semantics

| Concern | Rule |
|--------|------|
| **What it does** | Textually replaces the `include` line with the (recursively expanded) contents of the target file, before lexing. |
| **Namespace** | Flat and global. Included definitions are indistinguishable from ones written inline. |
| **Include-once** | Each file, resolved to a canonical absolute path, is spliced **at most once** per build. A second `include` of the same file expands to nothing. This makes diamond includes (`a`→`b`, `a`→`c`, both →`util`) safe and avoids duplicate `fn` definitions. |
| **Cycles** | `a` includes `b` includes `a` is a **hard error** with the include chain printed. (Include-once alone stops infinite expansion, but an explicit cycle error is clearer than silent truncation.) |
| **Duplicate names** | Two *different* files defining the same `fn` name is a collision. Codegen would emit two `zl_fn_lex(...)` definitions → a C compiler error today. The preprocessor should detect and report it as a zl-level error with both file names. |
| **Line numbers** | Splicing shifts line numbers, so parser/lexer errors report the position in the *concatenated* text, not the original file. Acceptable for v1 (see §6). |
| **Not a keyword** | `include` is consumed by the preprocessor and never reaches the lexer, so `include` remains a legal ordinary identifier everywhere else. |

Non-goals for v1: selective import, renaming, private/`export` visibility,
conditional includes, search paths / include dirs. All deferred to Option B or
later.

---

## 5. Implementation sketch

The whole feature is **one function that runs between "read the file" and
"lex the text"**, in both the C bootstrap compiler and the self-hosted one.

### 5.1 Pipeline change

```
before:  path ─► read_whole_file ─► lex ─► parse ─► codegen
after:   path ─► expand_includes ─► lex ─► parse ─► codegen
                 └── new pass, returns one spliced string ──┘
```

Nothing after `expand_includes` knows includes exist.

### 5.2 The algorithm

```
expand(path, seen, stack):
    canon = absolute(path)
    if canon in stack:          error "include cycle: " + join(stack + [canon])
    if canon in seen:           return ""          # include-once
    add canon to seen
    push canon onto stack

    base = directory_of(canon)
    out  = ""
    for each line in read(canon):
        if line is `include "X"` (ignoring leading spaces / trailing comment):
            out = out + expand(join(base, X), seen, stack) + "\n"
        else:
            out = out + line + "\n"

    pop stack
    return out
```

Top-level call: `source = expand(entry_path, {}, [])`, then feed `source` to the
existing lexer.

### 5.3 C bootstrap (`src/frontend/lexer.c` / small new `preprocess.c`)

`read_whole_file` already exists. Add `char *expand_includes(const char *path)`
that implements §5.2 over the buffer (split on `\n`, `strncmp` the trimmed line
against `"include"`, extract the quoted path, `realpath`/`_fullpath` for the
canonical key, recurse). Then change `lex_file` to call it instead of
`read_whole_file`. `seen` can be a small array of strings; the include depth is
tiny, so linear scan is fine.

### 5.4 Self-hosted (`src/selfhost/compiler.zl`)

This is the load-bearing part — it must work in zl itself, using only existing
built-ins (`read`, `len`, `at`, `slice`, `find`, `has`, `lines`, `split`,
`push`, `+`). All of these already exist. Sketch:

```
g_seen = []          # canonical paths already spliced

fn dir_of(path) {
    i = len(path)
    while i > 0 and at(path, i - 1) != "/" and at(path, i - 1) != "\\" {
        i = i - 1
    }
    return slice(path, 0, i)          # "" for a bare filename
}

fn is_include(line) {
    t = trim_left(line)               # small helper
    return slice(t, 0, 8) == "include "
}

fn include_path(line) {
    a = find(line, "\"")
    b = find(slice(line, a + 1, len(line)), "\"")
    return slice(line, a + 1, a + 1 + b)
}

fn expand(path) {
    for p in g_seen { if p == path { return "" } }   # include-once
    g_seen = push(g_seen, path)
    base = dir_of(path)
    out  = ""
    for line in lines(read(path)) {
        if is_include(line) {
            out = out + expand(base + include_path(line)) + "\n"
        } else {
            out = out + line + "\n"
        }
    }
    return out
}
```

Then the main block becomes:

```
input = expand("input.zl")
prog  = parse_program(input)
write("out.c", compile_program(prog))
```

Needed support: a two-line `trim_left` helper, and an explicit-cycle check (an
`active`/stack list alongside `g_seen`) if we want the nicer cycle error rather
than relying on include-once to break the loop. `lines()` and `read()` already
exist, so this is genuinely small.

### 5.5 Self-hosting guard

`src/selfhost/compiler.zl` compiles itself, so `expand` must survive the fixpoint: run
`interp.exe src/selfhost/compiler.zl` (now splitting via `include`) to produce `gen1.c`,
build `zlc.exe`, recompile → `gen2.c`, and assert `gen1.c == gen2.c` exactly as
the existing proof does. The include pass has no effect on emitted C (it only
changes *which text* is parsed), so the fixpoint should hold unchanged.

---

## 6. Known limitations & cheap mitigations

1. **Line numbers drift** after splicing. v1 accepts this. Cheap fix when it
   hurts: keep a parallel `line_origin` array (built during `expand`) mapping
   each output line back to `(file, line)`, and have error printers consult it.
   No lexer/parser change required.
2. **No encapsulation.** Everything is global. This is the Option A trade and
   the trigger to build Option B — do it only when a real collision forces it.
3. **Whole-file granularity.** You get all of a file's definitions or none.
   Acceptable; selective import belongs to Option B.

---

## 7. Recommendation summary

- **Build Option A (textual `include`) now.** ~40 lines, one new pass, zero
  changes to lexer/parser/codegen, zero new reserved words, survives
  self-hosting.
- **First payoff:** split `src/selfhost/compiler.zl` into `src/selfhost/lexer.zl` / `src/selfhost/parser.zl` /
  `codegen.zl` and seed a proposed `strings` module under `stdlib/`.
- **Defer Option B (namespaced `import ... as`)** until `.` member access exists
  in the self-hosted parser *and* a concrete name-collision problem is felt.
  Ship the small thing; keep the door open.
