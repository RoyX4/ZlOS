# Gaps found by building real programs

Rewritten 2026-08-03, expanding the three-program draft to **eight build targets plus two
engine audits**. The planning docs (`LANGUAGE_GAP.md`, `ROADMAP.md`, `C_CPP_PARITY.md`,
`FUNCTION_INVENTORY.md`, `BUILTIN_FUNCTIONS.md`, `NOVEL_IDEAS.md`, `ZL_1.0_VISION.md`,
`REACH_GAPS.md`) catalogue the missing features **abstractly** — from the language's structure
and from comparison against C/C++/Python/Rust/Go/JS/Zig. This doc is the **complement**: what
actually bit when agents sat down and tried to build real programs, wall by wall. It does not
repeat the abstract lists — it references them by filename+item and records only what building
revealed that they missed, under-stated, or **got wrong**.

Section §1.1 keeps its number: `REACH_GAPS.md` §4 cites "GAPS_REALWORLD 1.1" for the byte-layer
finding.

The eight targets and their one-line verdicts:

- **web-server** (HTTP/1.1 accept loop) — *fails at `socket()`, and at five more layers beneath it.*
- **tui-app** (arrow-key file browser) — *rendering is nearly there (`stdlib/ansi.zl` is good);
  interactive control is a wall — no raw input, no terminal size, no timed read.*
- **game** (real-time snake/pong) — *one hard wall: non-blocking keyboard input. Plus four new
  footguns beneath it.*
- **build-tool** (package manager) — *fails on line 1 (`args()`) and at its core action
  (`run()` only simulates); three of four remaining pillars each independently blocked.*
- **data-cli** (CSV filter → group-by → sum → table) — **BUILDS AND RUNS AS SPECIFIED** on the
  interpreter (as a hardcoded script). The first probed real tool that fully works.
- **JSON pretty-printer** — *gets further than xxd/wc; defeated on **number fidelity**, objects,
  and `\u` escapes.*
- **xxd hex dumper** (+ `wc -w` cross-check) — *dies line 1 on any real binary; works on NUL-free text.*
- **semantic-consistency audit** (`interp.c`/`parser.c`/`lexer.c`) — 13 verified core-semantics
  inconsistencies beyond the two known ones.
- **Zig-parity audit** — measures zl against Zig's actual feature set; finds two whole clusters
  the docs never name (the integer model, and the "no hidden X" disciplines).

Every measured line below was run against `interp.exe` on 2026-08-03 or read from source — not
inferred. Probes were tiny (leading-underscore scratch files). Claims marked **(verified here)**
were re-run first-hand while writing this doc.

---

## 1. NEW GAPS — not in any existing doc

The highest-value section: things the abstract lists do not contain at all, get wrong, or
reduce to something much smaller than they are. Severity tags: **BLOCKER** (can't build it) ·
**PAINFUL** (possible but awful) · **MINOR** (small friction).

### 1.1 Strings cannot hold a NUL byte; there is no byte container; reading binary truncates silently — *(web-server, xxd, JSON)* · BLOCKER

The single sharpest concrete failure, and the most under-documented. Measured:

```
chr(0)                              -> runtime error: chr(0): a zl string cannot hold a NUL byte
write_bytes("_p.bin",[72,0,66,67])  writes 4 bytes to disk (wc -c = 4)
read("_p.bin"); len(...)            -> 1   (truncated at first 0x00, SILENTLY, exit 0)
```

Consequences the docs miss: `read`/`len`/`at`/`slice` all use `strlen`, so **any byte stream is
silently cut at its first `0x00`**; there is **no `read_bytes` inverse** to `write_bytes` (zl can
write an arbitrary binary file but not read one back); the only binary-capable container is a
**list of boxed byte-values** (~64× memory, never freed) that bridges to nothing. `LANGUAGE_GAP.md`
frames strings only as a Unicode problem and literally calls them *"byte arrays"* — they cannot
hold a `0` byte. This turns a "missing builtin" into a **structural inability to represent or read
binary** — the wall a network program, a hex dumper, and a JSON tool all hit. `REACH_GAPS.md` §4
already leans on this finding for its interop verdict.

### 1.2 The reference interpreter itself is lossy at the print boundary — number-to-text uses `%g` (6 sig digits) — *(JSON, data-cli)* · BLOCKER for any float-carrying round-trip

**This is the headline new finding.** `str()`/`print()` render every non-integer number with C's
default `%g` (6 significant digits) and every integer-valued float without its decimal point
(`interp.c:108-112`). Measured **(verified here)**:

```
str(370.0)                          -> "370"     (decimal point unrecoverable)
str(1.0)                            -> "1"
str(num("3.14159265358979"))       -> "3.14159" (10 digits of precision gone)
100000000.5                        -> "1e+08"
2.0/3.0                            -> "0.666667"
```

All silent, exit 0. Why it is not in any doc — and worse: `LANGUAGE_GAP.md` Tier 5 presents the
interpreter's floats as the **correct reference** (`7/3 = 2.33333` vs the fast backends' `2`). In
fact the reference interpreter **loses precision at the print boundary**, so *any* float-carrying
round-trip — JSON, CSV money columns, config, coordinates — is corrupted **regardless of backend**.
`str(1.0) -> "1"` combined with `type(1.0) == type(1) == "num"` means zl cannot store, distinguish,
or emit the JSON number-integer-vs-float distinction at all. This is upstream of `item 36` (format
specifiers): even with a perfect `%.15g` you would still be feeding it a value already rounded to 6
digits. "Pretty-print this JSON" silently degrades to "…provided no number needs more than 6
significant figures."

### 1.3 No entropy and no wall clock — every run is byte-identical; `now()` is process-uptime, not wall-clock — *(game, build-tool)* · BLOCKER for reproducibility & incremental work

Measured by the game agent: `now()` reads `0` at launch (it is ms-since-process-start,
`interp.c:1184`), so `seed(now())` == `seed(0)` every run; `randint(0,99)` printed `38` on three
independent runs. Default `rand()` (no seed) is also constant. `getpid`/`timestamp`/`urandom` are
all absent. So **every game or simulation is byte-identical every launch, with no workaround.**

Two consequences the docs miss:

- **A documentation error.** `C_CPP_PARITY.md:181` ("has wall-clock time") and
  `design_comptime.md:243` ("now | Wall clock") mislabel `now()`. It is a **monotonic
  process-uptime timer starting at 0** — zl has *no* way to read the date, the unix epoch, or any
  value that varies between runs.
- **Incremental build is impossible in principle, not just missing a builtin** (build-tool). There
  is a **double clock-lock**: no file `mtime` (can't read a file's age) *and* `now()` resets each
  process (can't persist "built at T" to compare next invocation). A zl build tool must recompile
  everything, every time. `watch` mode is unimplementable at any quality because its entire
  fallback ladder — `mtime`, a `hash`/`crc32` builtin (sha256 is un-importable stdlib), `sleep`,
  and a stable clock — is sawn off simultaneously.

### 1.4 `fill(n, list)` stores n *aliases* of the same inner list — the natural 2D-grid idiom is silently broken — *(game, tui)* · PAINFUL

Measured **(verified here)**: `m = fill(3, fill(3,0)); m[0][0] = 9` yields
`[[9,0,0],[9,0,0],[9,0,0]]` — one write hit every row. A nested **literal** does *not* alias
(`g=[[0,0,0],[0,0,0]]; g[1][2]=7` changes only that cell), so the two obvious grid-construction
methods **silently disagree with no error**. Chained 2D index read/write itself works
(`g[1][2]` reads and assigns fine). This is the concrete reason `maze.zl` uses a flat 1D array
(`p = y*W + x`) — not only for speed, but because `fill`-of-rows is broken. General list-aliasing
(`b = a` shares) is noted in `design_records.md`/`design_memory_structs.md`; the **`fill`
multiplication** and the **`fill`-vs-literal asymmetry** are not.

### 1.5 Functions ARE first-class values — the docs are WRONG that they aren't — *(data-cli, semantic-consistency)* · CORRECTION, high value

Multiple docs state the opposite of what the engine does. Measured **(verified here)**:
`fn apply(f,x){return f(x)}  apply(dbl,5) -> 10`. Named functions are genuine `V_FN` values:
passable, returnable, storable, and callable-by-name. The data-cli agent wrote a full
`myfilter(xs,pred)`, `mymap`, `myreduce`, and a user `usort(xs,cmp)` — **the entire
map/filter/reduce/group-by/sort-by-key family works on the interpreter today.**

This directly contradicts `LANGUAGE_GAP.md` Tier 3 ("a comparator cannot be passed"),
`FUNCTION_INVENTORY.md`/`BUILTIN_FUNCTIONS.md` ("map/filter/reduce ALL blocked on closures,
item 11"), `design_type_system.md` ("no first-class function values at all"), and
`design_game_system.md`. The **real** limits are much narrower:

- **No anonymous `fn(x){...}` literals** — `fn` as a value expression is a parse error
  (`expected a value (got fn)`). You must name every function.
- **No lexical capture** — a named `fn` sees globals but not enclosing locals, so per-call
  parameters travel through mutated globals (**non-reentrant**). `call_env`'s parent is
  `g_global`, not the defining env.
- **The callee must be a bare identifier** — `fs[0](5)` fails with *"that is not something you
  can call"* **(verified here)**, and `makeFn()(x)` fails, because `eval_call` only accepts an
  `N_IDENT` callee.

Roadmap consequence: `item 11` (closures) is an **ergonomics** feature (anonymous literals +
capture + sugar), **not a capability unlock**. The whole `<algorithm>` family is already reachable;
it just reads verbosely. See §3 for how this rewrites the blocker ranking.

### 1.6 `builtin sort` silently ignores a comparator argument — *(data-cli, semantic-consistency)* · PAINFUL (silent wrong result)

Measured **(verified here)**: `sort([["c",1],["a",5],["b",99]], byval)` returns natural order
`[["a",5],["b",99],["c",1]]`, **not** the comparator's order — no error, exit 0, just wrong.
`sort` accepts the extra arg and throws it away. This is distinct from the documented "no
sort-by-key builtin": the builtin **looks like** it takes a comparator and lies. The workaround is
a hand-written `usort(xs, cmp)` (which works — §1.5), but a developer reaching for `sort(xs, cmp)`
gets a plausible wrong answer with no signal.

### 1.7 Numeric parsing silently corrupts on *plausible* input — *(web-server, data-cli)* · PAINFUL (sharpens the prior §1.2 finding)

The prior draft recorded `num("abc") -> 0`, `num("12ab") -> 12`. data-cli sharpens it to a value
that *looks valid*: **(verified here)** `num("1,234") -> 1` (stops at the comma, discards "234"),
`num("") -> 0`, `num("N/A") -> 0`. A thousands-separated number, an empty cell, and a sentinel all
silently become plausible wrong numbers. The core numeric operation of a data tool produces
wrong-but-believable sums. `num`/`int` are `atof`/`atoi` with no failure channel; validating any
numeric field arriving from a file or the network is **actively unsafe**, not merely unavailable.

### 1.8 The two `for` forms scope their loop variable differently — *(semantic-consistency)* · PAINFUL (correctness footgun)

Measured **(verified here)**: with a global `g=100`, `fn f(){ for g = 1 to 3 {} }` then `f()`
leaves `g == 4` — the **range** counter writes the GLOBAL. But `for g in [1,2,3]` in the same
position leaves `g == 100` — that form **localizes**. Cause: `parse_for_range` emits the counter as
a plain `N_ASSIGN`, and `define_loop_vars` (`interp.c:1416`) only pre-binds `N_FOR` nodes, so it
never sees the desugared range counter. `design_scoping_decision.md` covers assignment-writes-global
in general; the **for-range-vs-for-in asymmetry** is new, and it is a per-request/per-frame
state-corruption hazard hiding inside the more common loop form.

### 1.9 `%` is the only arithmetic operator that truncates to int, and it rounds toward zero — *(game, semantic-consistency)* · PAINFUL

Two defects in one operator (`interp.c:1389`), measured by the agents:

- `5.5 % 2` -> `1`, while `fmod(5.5, 2)` -> `1.5` and `7/2` -> `3.5`. `%` unconditionally truncates
  **both operands** to `long long`, disagreeing with its own builtin sibling `fmod` and with every
  other arithmetic operator about whether numbers are floats.
- `(0-1) % W` -> `-1`, not `W-1` (C-truncating, toward zero). Toroidal / wrap-around grid movement
  — the core of snake/pong edge-wrap — reads **out of bounds** unless you manually pre-add the
  modulus.

`LANGUAGE_GAP.md` documents the `/` integer-vs-float ambiguity but not the `%` operand truncation
(that lives only in the non-canonical `design_floats.md`) nor the negative-modulo footgun.

### 1.10 Container semantics are internally inconsistent — a cluster from the engine audit — *(semantic-consistency)* · MINOR→PAINFUL each

Six verified inconsistencies, none in any doc:

- **Strings support `len()` but reject both sequence operations.** `"hello"[0]` -> *"only lists can
  be indexed"* **(verified here)** and `for c in "hi"` -> *"'for' can only loop over a list"*. You
  must fall back to `at(s,i)` plus a manual numeric loop. A string is a sequence for `len` and for
  nothing else.
- **Bounds-check policy is opposite for the two containers.** `at("ab",2)` on a string returns `""`
  (lenient), but `list[i]` out of range is a **fatal, uncatchable, location-blind** *"runtime
  error: list index out of range"*. With no `try`/`catch` and no line numbers, a short CSV row is
  an uncatchable crash (data-cli hit exactly this).
- **Lists mix reference and value semantics per operation.** `ys = xs; xs[0] = 99` makes
  `ys[0] == 99` (index-assign mutates the shared items array in place, `interp.c:1621`), but
  `push(xs, 4)` returns a **fresh** list so `len(xs)` stays 3; `insert`/`remove`/`concat`/`sort`/
  `reverse` likewise copy. The same value is reference-typed for `[i]=` and value-typed for
  everything else.
- **Booleans carry 3+ contradictory identities.** `true + 1` -> `"true1"` (string concat),
  `true - 1` -> runtime error, `sum([true,true,true])` -> `3` (numeric), `1 == true` -> `false`
  (type-first equality). Four operations, four notions of what a bool is.
- **A total order over all types exists — reachable only through `sort()`.** `value_compare`
  (`interp.c:1314`) defines `nil < bool < num < str < list < fn`, so `sort(["banana","apple"])`
  orders strings; but `"apple" < "banana"` -> *"this operator needs numbers"* and `min("a","b")`
  -> *"min needs two numbers"*. `<`, `>`, `<=`, `>=`, `min`, `max` are all numbers-only. The obvious
  counterpart of the sort order is not exposed.
- **`and`/`or` coerce to bool instead of returning an operand.** So `x = a or default` cannot be a
  fallback idiom, even though the ternary *does* return operands — making the inconsistency visible
  in one program.

### 1.11 Terminal size has NO route at all — builtin, env, and escape-trick are each dead — *(tui, game)* · BLOCKER for responsive layout

Zero hits for `get_terminal_size`/`winsize`/console-size across all planning docs, and confirmed
dead three ways by the tui agent: (a) no builtin returns rows/cols; (b) `env("COLUMNS")` and
`env("LINES")` both return `""`; (c) the `ESC[6n` / cursor-to-999,999-then-report fallback needs a
**raw byte-by-byte read of the terminal's reply**, which is itself blocked by the missing raw-input
primitive (§1.12). So there is no route — builtin, env, or trick — to learn the field size; you
hardcode 80×24 and break on every other size and on resize. The shipped game examples
(`life.zl`, `maze.zl`) dodge this with **fixed** grids, so responsive layout never surfaced it. A
single `get_terminal_size()` (GetConsoleScreenBufferInfo on Windows) belongs on the cheap
single-program-unblock list next to `sleep`/`flush`/`kbhit`.

### 1.12 The keys a TUI needs are multi-byte escape sequences requiring a *timed* read, not just a non-blocking poll — *(tui)* · BLOCKER

The prior draft framed input as blocking (`input()`) vs non-blocking (`kbhit`). tui sharpens it:
arrow/function/Home/End keys arrive as **3+ bytes** (`ESC` `[` `A`), and distinguishing a **bare
Escape** keypress from the **start of an arrow sequence** requires reading **with a short timeout**
(the classic ESC-disambiguation problem: if no byte follows within ~50 ms it was a real ESC). zl has
neither a timed/non-blocking read **nor** `sleep` to build the timeout window, so even a
hypothetical `read_char()` + `kbhit()` could not decode arrow keys correctly. The only input
primitive today is line-buffered `input()` (`fgets`, `interp.c:460`), and `interp.c` contains **zero**
`SetConsoleMode`/`tcsetattr` — no raw/cbreak mode exists to enable at all. The multi-byte +
read-timeout requirement is documented nowhere.

### 1.13 Lexer / syntax surprises a real program trips over — *(data-cli, JSON, semantic-consistency)* · MINOR→PAINFUL

A cluster of parse-time walls, none listed:

- **Cuddled-else is mandatory.** `}` and `else` must be on the **same physical line**; `} \n else {`
  is a parse error (*"line 5: expected a value (got 'else')"*, **verified here**). Allman/BSD brace
  style is illegal. (Corollary positive, also undocumented: **multiple space-separated statements on
  one line work** — `a=1  b=2  print(a+b)`.)
- **Scientific-notation, hex, and bare-decimal literals do not lex** even though the library parses
  them. `1e3`, `6.02e23` -> *"expected )"*; `0xFF` splits into `0` + identifier `xFF`; `.5` ->
  *"expected a value"* — yet `num("1e3")` and `num("6.02e23")` parse fine. A lexer-vs-library
  asymmetry; the docs list missing `**`/`<<` operators but not missing number-literal forms.
- **An f-string slot cannot contain a string literal.** The lexer ends the f-string at the first
  inner `"`, so `f"{join(x, "-")}"` -> *"f-string has {"*. Any interpolation whose expression passes
  a string constant is impossible. (Separately, f-strings still have no format specifiers — see the
  retained §2 note.)
- **A stray `break`/`continue` outside a loop silently truncates the tail of every later block.** It
  is accepted with no error and sets the global `g_breaking`, which only a loop ever clears; so
  `break` followed by `if true { print("a") print("b") }` prints only `"a"` — `exec_block` runs the
  first statement, sees the leaked flag, and returns. No loop involved, no diagnostic.
- **Chained comparison errors.** `1 < 2 < 3` -> *"this operator needs numbers"* (the inner `<`
  yields a bool the outer `<` rejects) — mathematical-looking text that fails at runtime.
- **`.` and `!` are parse-accepted dead syntax.** `p.x` -> *"member access isn't supported yet"*
  (parsed as `N_MEMBER`, per `ROADMAP.md` item 2.1) and `x!` evaluates to `x` with no effect — the
  `!` danger marker is accepted and ignored.

### 1.14 Zig-parity gaps the docs never name — two whole clusters — *(Zig audit)* · varies

Measured against Zig's actual feature set, two clusters are entirely absent from the planning docs:

**(a) The integer / numeric model.** Every zl number is an `f64`, which forecloses Zig's whole
numeric story and is nowhere framed as a parity gap:

- **No exact 64-bit integer, no defined overflow, no wrapping/saturating operators** (`+%`, `-%`,
  `*%`, `+|`, `@addWithOverflow`). Measured: `print(9007199254740993)` -> `9007199254740992`;
  i64-max prints as `9.22337e+18`. The planned sized-int item (15) **never specifies overflow
  semantics**.
- **Arbitrary-bit-width and 128-bit integers** (`u1`, `i7`, `u128`, `i128`) — item 15 plans only
  i8–i64/u8–u64, leaving true bitfield packing (`design_memory_structs.md`'s own BMP example) and
  wide crypto math with no type.
- **SIMD / `@Vector`** — no vector types, `@splat`/`@reduce`/`@shuffle`, or lane-wise operators.
  Directly relevant to the raytracer and game examples zl already ships.
- **Explicit width/reinterpret cast vocabulary** (`@intCast`, `@truncate`, `@bitCast`, `@ptrCast`,
  `@enumFromInt`) — once sized ints exist, FFI needs defined checked-narrowing and bit-reinterpret;
  the docs mention only loose `num`/`int`/`str` conversions.
- **Internal inconsistency:** bit/precision ops raise *"needs more than 53 bits of precision"*
  (`interp.c:356`) past 2^53, but **printing** a >2^53 integer literal silently rounds
  (`9007199254740993` -> `…992`). One path guards, the other corrupts.

**(b) The two "no hidden X" disciplines.** Zig's signature guarantees, which zl actively *violates*
and which no doc frames as parity gaps:

- **No-hidden-allocations.** zl allocates implicitly on every `+`, `push`, `range`, string concat
  and f-string, with no allocator parameter and no way to declare or verify a function
  allocation-free. `item 40` (explicit allocators) is proposed as a memory-safety *fix*, but the
  **principle that an allocation must be visible in the syntax** is absent — and implicit allocation
  is the exact mechanism behind the 51.8 GB machine crash of 2026-08-03.
- **No-hidden-control-flow.** `+` is runtime-type-dispatched (`2+3=5`, `"n="+5="n=5"`, list concat)
  and `x/0` kills the process — both violate Zig's principle, which `design_error_handling.md`
  itself calls unwritten folklore. The docs treat `+` overloading as a *correctness bug*, not as a
  conscious philosophical divergence from the stated Zig benchmark.

And three more Zig features unplanned as **safety mechanisms** (not just missing keywords):

- **Exhaustive tagged-union `switch` with payload capture** (`.tag => |payload|`, compile-error on a
  missing case). `design_switch_case.md` (v2) is explicitly non-exhaustive `if`/`elif` sugar with no
  capture — so `examples/vm.zl` hand-rolls opcode dispatch as a giant `if`-ladder.
- **Source-embedded `test "..." {}` blocks** run by `zl test`, plus a separate **error-return
  trace** — zl keeps tests in separate suites and item 7's trace is single-frame.
- **Blocks/`switch` as value-expressions** with labeled value break
  (`x = blk: { …; break :blk v; }`), and **`undefined`** as a first-class initializer with
  **sentinel-terminated slices** (`[:0]u8`) for C-string interop. zl's switch is statement-only and
  labeled break (item 22) escapes loops only.

### 1.15 Build-tool composition dead-ends — *(build-tool)* · BLOCKER each

Two file-layer walls that the per-builtin missing-lists (`exists`, `is_dir`, `walk`) don't connect:

- **The optional-file-probe trilemma.** No `exists` + a missing-file `read` **aborts the process**
  + no `try`/`catch` means the only way to test whether a lockfile/manifest is present is to
  `dir()` the parent and string-scan the names. Each piece is documented alone; together they leave
  **no clean way to test for a file**.
- **`dir()` is not composable into `walk`.** It returns bare filename strings with no type
  discriminator (`interp.c:804-819`) and there is no `is_dir`/`is_file`, so given
  `["src","main.zl","README"]` you cannot tell which entry to recurse into. `dir` is usable only
  **one level deep**; a recursive project-tree walk cannot be written in userland at all.

Plus a self-hosting-specific framing: because `run()` only **simulates** (`interp.c:416-418`;
`run("clang","--version")` prints `[sim] run(clang, --version)` and returns nil), **a build tool
written in zl cannot launch zl's own compiler.** A self-hosting toolchain cannot be *managed* by a
program in the language it builds — the ecosystem story dead-ends at its first step.

---

## 2. SHARPER EXAMPLES — known gaps that a real program made vivid

Already in the abstract docs; the real programs make them concrete. Referenced, not repeated.

- **No memory management** (`LANGUAGE_GAP.md` Tier 0.2, which names *"a server"*). Made literal by
  five targets: the web-server leaks every request's parsed strings/lists forever in the accept
  loop; the game leaks 2+ whole-body list copies **per frame** (the snake deque has **zero** O(1)
  ops — no `pop`/`shift`/`unshift`; `push` returns a fresh O(n) copy, tail-removal is another O(n)
  `take`); the tui, build-tool and a REPL all leak steadily. Given the 51.8 GB machine-crash
  incident and `maze.zl`'s 11 GB working set, a long-running zl accept loop **OOMs the host, not
  the process**.
- **No error handling** (Tier 2). Fatal across targets: one dropped client crashes the *entire*
  web-server; one blank line crashes data-cli (§1.10 fatal list-OOB); a missing file aborts the
  build-tool. And errors are **location-blind** — `socket(3)` produced a bare *"runtime error:
  unknown function"* **(verified here)**: no line, no function, no stack. (Sharpening the sharpener:
  **parse** errors *do* carry line numbers — *"line 5: expected a value"* — so the machinery to fix
  the runtime path already exists on the parse path.)
- **Assignment writes the GLOBAL slot** (`design_scoping_decision.md`). In a per-request `handle()`
  or per-frame `tick()`, any local colliding with a global (`i`, `line`, `path`, `head`) silently
  clobbers state across requests/frames — data corruption **even single-threaded**. Both shipped
  examples prefix **every** local (`cv_`/`fd_` in `maze.zl`, `ng_`/`rs_` in `life.zl`) purely to
  dodge it. See §1.8 for the sharper for-range instance.
- **No maps** (item 4). web-server headers, the build-tool manifest, and JSON objects all need them.
  Sharpest JSON consequence: an empty object `{}` vs empty array `[]` are **both `type "list"`** and
  the `{}` literal is itself a parse error, so a JSON object and array are structurally
  indistinguishable and an empty object is unrepresentable. (group-by, notably, does **not** need
  maps — data-cli did it with a list of `[k,v]` pairs and nested index-assign `groups[idx][1] = …`.)
- **No `sleep`** (item 25). `sleep(50)` -> unknown function. Game/tui pacing busy-waits on `now()`
  (`life.zl`'s `pause()` spins) — the game agent measured **30 ms ≈ 14,820 interpreter iterations**,
  a full CPU core pegged at 100% doing nothing. And §1.12's ESC read-timeout cannot be built without
  it.
- **No `args`/`argv`** (item 8). Promoted hard by the new evidence: it is **line 1** of the
  build-tool (can't read its own subcommand), and forces data-cli, JSON, and xxd to hardcode their
  inputs as globals — turning every one of them from a *tool* into a fixed *script*. See §3.
- **No format string / padding** (item 36). `hex(10) -> "a"` (unpadded), `pad` is right-pad only, no
  `zfill`/`rjust`. Every fixed-width hex column (xxd's `%02x`/`%08x`, JSON's `\uXXXX`) is a hand-
  rolled `while len(h) < 4 { h = "0" + h }`. And even the interpolation half (`f"{x}"` works) can't
  reach `f"{price:.2f}"` — a parse error — so data-cli couldn't keep two decimals on a money column
  (compounded by §1.2).
- **Whole-file-only I/O** (`C_CPP_PARITY.md <stdio.h>`). `read()` slurps everything; no `read(n)`,
  no read-until-delimiter, no handle/cursor/EOF. An HTTP request has no known length up front, so
  the read-a-bit-then-decide loop every request parser and streaming tool needs cannot be expressed.
- **O(n²) string building.** `out = out + piece` in an emit loop is quadratic (strings immutable, no
  builder). Measured for list `push` in the docs, not for string concat. The O(n) fix (push pieces
  to a list, then `join`) is non-obvious. Hit by JSON and xxd.
- **No signal / `atexit` / `defer`** (`C_CPP_PARITY.md`). `runtime_error()` calls `exit(1)`
  (`interp.c:250`) and Ctrl-C kills outright, so a game/tui that hides the cursor (`CSI ?25l`, which
  `life.zl` emits) or switches to the alternate screen leaves the terminal **broken** with no
  restore hook.
- **stdlib files are self-running scripts, not importable libraries** (extends "no imports",
  item 3). `stdlib/ansi.zl` runs colour charts and `assert()`s from line 183 **on load**, so you
  cannot even copy-paste its helpers without also deleting its demo. Across all 96 modules there is
  no script/library separation — data-cli had to paste `csvParseLine` verbatim, the build-tool would
  have to paste its own JSON/INI parser.
- **Retained xxd/wc specifics** (still valid): no multi-line/triple-quoted string literals (a literal
  newline in `"…"` is a parse error); `split()` cannot collapse whitespace runs, so a basic `wc -w`
  cannot be written with the `split` you have; VT processing is never enabled by the interpreter, so
  escapes render only by Windows-Terminal luck; `print` always appends `\n` and there is no
  `flush()`, so animation must rebuild the whole frame per tick.
- **Concrete `csv_parse.zl` bugs** (extends `REACH_GAPS.md` §4's "CSV merges records"): a **blank
  line** parses to a phantom single-field `['']` row (both hand-split and stdlib `csvParse`), which
  then fatally crashes column access; and a single **stray/unescaped quote** flips the parity-based
  `csvOpenQuote` so it swallows the delimiter and **greedily merges every following line** into one
  record until a balancing quote appears — silent unbounded corruption.

---

## 3. THE BLOCKER RANKING — from the demand side

The abstract docs rank by **dependency** (what unlocks the most other features). This ranks by
**demand**: across the eight targets, which single missing feature blocked the most, weighted by
severity. Independent roadmap evidence, from the consumer side rather than the supplier side.

| Rank | Missing feature | Targets blocked | Severity signal |
|---|---|---|---|
| 1 | **`args`/`argv`** (item 8, ~30 lines) | build-tool, data-cli, JSON, xxd, web-server (config) | **line 1 of the build-tool**; turns every would-be tool into a hardcoded script. Highest demand-to-cost ratio in the whole exercise. |
| 2 | **Error handling** (item 5) | web-server, data-cli, build-tool, JSON, tui, game | Fatal in ≥3; makes crashes uncatchable + location-blind; blocks all validation & recovery. |
| 3 | **Memory management** (Phase 5) | web-server, game, tui, build-tool, REPL-class | Caps every long-running target; can OOM the **host**, not just the process. |
| 4 | **FFI + sized ints** (items 16/15) | web-server (`socket`), build-tool (`run`), tui (raw mode/term size), game (raw input/term size) | The entire OS-facing category; several targets can't emit line 1 without it. |
| 5 | **imports** (item 3) | build-tool, data-cli, JSON, ~all reuse | Copy-paste tax paid by every target; cheap; stdlib is 96 self-running scripts, not a library. |
| 6 | **Byte/binary layer** (§1.1) | web-server, xxd, JSON | 3 **hard** blockers; the most under-documented gap. |
| 7 | **Structs/records + sized ints** (items 4/15) | all eight, as PAINFUL | The everyday "describe data" gap — `sockaddr`, grid/entity, int-vs-float, manifest, CSV rows. |
| 8 | **Number fidelity** (§1.2, NEW) | JSON (fatal-for-purpose), data-cli (money) | Silent corruption in the **reference** engine; upstream of `item 36`. |
| 9 | **Cheap single-program unblocks** | `sleep` (game/tui/build), timed+raw input (game/tui), `flush`/VT/term-size (game/tui), entropy/seed (game) | Each turns one target from impossible to writable; each is a few lines. |
| 10 | **Scoping-writes-global** (§1.8) | latent in all with per-request/per-frame handlers | Correctness footgun; both shipped examples prefix every local to dodge it. |

**The correction that reorders the roadmap: closures (item 11) drop *out* of the demand-side top
tier.** The abstract docs make closures the **#1** unlock (the map/filter/reduce family across five
languages), and the prior draft of this doc predicted a CSV tool "would flip that finding
instantly." **It did not flip it.** data-cli proved that **named** higher-order functions carry the
entire filter/map/reduce/group-by/sort-with-comparator pipeline on the interpreter *today* (§1.5).
Closures remain worth building — for anonymous literals, lexical capture, and sugar — but as
**ergonomics**, not capability. The demand-side signal is blunt: for the systems-and-tools programs
zl is positioned for, the things that actually stop you are **args, errors, memory, FFI, imports,
and the byte layer** — and every one of them out-ranks closures.

---

## 4. WHAT YOU CAN ALREADY BUILD — the honest floor

The floor is **higher** than the prior draft implied, because a real data tool now fully works.
Everything below runs on the **interpreter** (the fast backends are separately disqualified — they
lack strings, lists, and real floats).

**Buildable now:**

- **A CSV analytics script — end to end.** data-cli (`read _sales.csv` → filter `units>4` → group by
  region → sum revenue → print table) **built and ran correctly** on the interpreter, output verified
  by hand. Floats, `num()`, grouping via list-of-`[k,v]` pairs with nested index-assign, and a full
  filter/map/reduce plus a user `usort(xs, cmp)` all work. Caveat: it is a **hardcoded script**
  (no `args`), and it dies on two real-CSV realities — a blank line or a stray quote (§2), and it
  can't format a money column to two decimals (§1.2/§2).
- **Named higher-order function pipelines** — `map`/`filter`/`reduce`/`group_by`/`sort-by-key` are
  all writable and working today, using named functions passed by value (§1.5). This is the single
  biggest positive correction in the whole exercise.
- **A JSON pretty-printer — partially.** Whole-file read + recursive-descent parse work (compiler.zl
  proves the parser class), `num()` reads exponent notation, and the `\"` `\\` `\n` `\t` output
  escapes exist — so **ASCII JSON with no high-precision numbers and no objects** pretty-prints.
  Defeated by §1.2 (numbers > 6 sig figs corrupt), no maps (objects), and no `\u` escapes.
- **Non-interactive ANSI terminal animation / visualizer.** `life.zl` and `maze.zl` **ship and run**.
  An animation stopped by a cycle, a fixed tick count, or completion is buildable — in a **VT-enabled**
  terminal (Windows Terminal, not stock `conhost.exe`), and it leaks steadily (fine for a bounded run).
- **A turn-based / prompt-driven text game.** Because `input()` blocks on Enter, a text adventure,
  guess-the-number, or a turn-based board game printed each move is **fully** buildable. The game
  blocker is specifically **real-time** input.
- **A line-oriented text tool on NUL-free input, with a hardcoded path.** The xxd port produced
  correct output on text; a grep-lite filter or `wc -l` (line count via `split` on `"\n"`) works,
  awkwardly. *Not* buildable: `wc -w` (no whitespace-collapsing split) and any binary input (§1.1).
- **Substantial batch text processing** — up to the **664-line self-hosted `compiler.zl`** itself,
  the existence proof for this whole class.

**Not buildable at all (state it plainly):**

- A **web server** — fails at `socket()`, and would still fail at the byte buffer, the read loop,
  concurrency, validation, memory, and error recovery even if sockets were bolted on.
- An **interactive TUI / real-time terminal game** — fails at raw + timed keyboard input, and at
  terminal-size discovery.
- A **build tool / package manager** — fails at `args()` (line 1) and `run()` (simulated); imports,
  file metadata, and a persistent clock are all missing beneath it.
- A **binary tool** — fails at reading any NUL-containing byte.
- A **faithful JSON round-trip** — fails at number fidelity, objects, and `\u` escapes.

**Positive surprises worth banking** (easy to forget zl has):

- **Functions are first-class** — named HOFs, `sort` via a hand-written comparator, returned-and-
  stored functions all work (§1.5). The docs say the opposite.
- **Structural equality works** — `[3,4] in body -> true`, so a snake self-collision test needs no
  hashing.
- **Nested list index read/write works** — `g[1][2]` reads and assigns (the aliasing hazard is
  specifically `fill`, §1.4, not indexing).
- **Floats work on the interpreter** — the "no floats in fast backends" gap is a red herring for
  interpreter programs. Caveat: they are **print-truncated** to 6 sig figs (§1.2).
- **Parse errors carry line numbers**, even though runtime errors don't — the fix for the runtime
  path already exists on the parse path.
- **f-strings exist**, and **multiple space-separated statements on one line** parse fine.

---

## Appendix — documentation errors found while building

Corrections to the planning docs themselves (a real program is the best proofreader):

1. `LANGUAGE_GAP.md`, `design_type_system.md`, `design_game_system.md`,
   `FUNCTION_INVENTORY.md`, `BUILTIN_FUNCTIONS.md` — all state or imply **"no first-class
   functions / a comparator cannot be passed."** The interpreter has genuine `V_FN` values and
   named HOFs work (§1.5). This is the highest-impact correction: it changes the roadmap weight of
   `item 11`.
2. `C_CPP_PARITY.md:181` ("has wall-clock time") and `design_comptime.md:243` ("now | Wall clock")
   — `now()` is a **process-uptime timer starting at 0**, not a wall clock (§1.3). zl has no way to
   read the date, the epoch, or any run-varying value.
3. `LANGUAGE_GAP.md` calls zl strings **"byte arrays"** — they cannot hold a `0` byte (§1.1).
4. `BUILTIN_FUNCTIONS.md:55` previously listed `concat` under STRING builtins — it is **list-only**
   (`concat needs two lists`); string joining is `+` exclusively. *(Already corrected in
   `BUILTIN_FUNCTIONS.md` on 2026-08-03 after the first real-world probe caught it — recorded here
   for provenance.)*
