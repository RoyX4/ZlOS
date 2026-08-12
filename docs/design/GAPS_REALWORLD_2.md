# Real-world gaps, round 2 — corrections and the consistency audit

2026-08-03. The 5 probes that dropped on API errors were re-run and completed (build-tool, TUI,
data-CLI, semantic-consistency, Zig). They produced one roadmap-changing CORRECTION, a pile of
concrete consistency bugs, and new Zig deltas. This supplements GAPS_REALWORLD.md.

---

## THE CORRECTION — first-class functions already work (verified live)

Multiple prior docs (LANGUAGE_GAP, design_type_system, C_CPP_PARITY, FUNCTION_INVENTORY) say or
imply zl has "no closures / no first-class functions" and that this blocks the entire
map/filter/reduce family. **That is wrong.** Verified on interp.exe 2026-08-03:

```
fn inc(x) { return x + 1 }
g = inc              g(3)              -> 4     functions are storable + callable
fn apply2(f,v){ return f(f(v)) }  apply2(inc,10) -> 12    higher-order works
fn mymap(xs,f){ out=[]  for x in xs { out=push(out,f(x)) }  return out }
mymap([1,2,3], inc)  -> [2, 3, 4]     MAP/FILTER/REDUCE ARE WRITABLE TODAY
```

What does NOT work is a **closure that captures a local**:
```
fn make_adder(n){ fn adder(x){ return x+n }  return adder }
make_adder(5)  ->  runtime error: 'n' doesn't exist yet
```

**So the real state, precisely:**
- **First-class functions: WORK.** Pass, store, return, call-by-name — all fine.
- **Closures-that-capture: BROKEN.** An inner function cannot see an outer function's locals.

**Roadmap impact — significant:** the whole functional family (`map filter reduce each any all
zip enumerate sort_by group_by`) is buildable as `std.iter` **right now**, with named-function
arguments, no engine change. It was never blocked. What "item 11" should actually mean is narrowed
to: (a) capturing closures, and (b) anonymous `fn(x){...}` inline sugar — both real, both smaller
than "no functional programming at all." The single highest-leverage library win just became free:
write `std.iter` today.

---

## CONSISTENCY BUGS — the audit of what already exists (semantic-consistency probe)

These are defects in shipped behaviour, verified live. Several are real bugs, not missing features.

1. **for-range leaks its loop variable to a global; for-in does not.** `for i = 1 to n` writes a
   global `i`; `for i in xs` localises it. `define_loop_vars` only handles the for-in node, not the
   desugared range assignment. A real asymmetry bug — and exactly the scoping issue the Option 2
   decision addresses, but the range form needs specific handling.
2. **`%` silently truncates BOTH operands to int.** `5.5 % 2 == 1` while `fmod(5.5,2) == 1.5` and
   `7/2 == 3.5`. The only arithmetic operator that truncates. LANGUAGE_GAP documented the `/`
   ambiguity but not this.
3. **Booleans have 3+ contradictory identities:** `true + 1 == "true1"` (string), error under
   `- * <`, numeric under `sum()`, and `1 == true` is FALSE. A value that behaves as string, number,
   and error depending on context.
4. **`and`/`or` coerce their result to bool** instead of returning an operand, so `x = a or default`
   (the universal fallback idiom) is impossible — while the ternary DOES return operands, making the
   inconsistency visible.
5. **Total order exists in `sort()` but nowhere else.** `sort` can order strings (value_compare has
   a full type order), but `<` `>` `min` `max` all REFUSE non-numbers. You can sort strings but not
   compare two of them.
6. **Lists mix reference and value semantics per operation.** `xs[i] = v` mutates in place through
   every alias; `push/insert/remove/concat/sort/reverse` return fresh copies. Same type, two memory
   models, no marker for which is which.
7. **Strings have `len()` but reject `s[i]` and `for c in s`.** "only lists can be indexed", "for can
   only loop over a list". A string is sequence-like for length but not for access.
8. **break/continue outside a loop are silently accepted** and set a global flag that is NEVER
   cleared, truncating the tail of every subsequent block. A stray top-level `break` makes later
   blocks run only their first statement. Silent, spooky-action-at-a-distance.
9. **Chained comparison `1 < 2 < 3` errors** ("this operator needs numbers") — the inner `<` yields a
   bool the outer `<` rejects. Maths-looking text that fails at runtime.
10. **`0xFF`, `1e6`, `.5` do not lex.** `0xFF` splits into `0` + identifier `xFF`; `.5` is "expected
    a value". Hex and exponent and leading-dot float literals are unsupported and fail misleadingly.
11. **`.` and `!` are parse-accepted DEAD syntax.** `p.x` always raises "member access isn't
    supported yet"; `x!` evaluates to `x` with no effect. The grammar accepts syntax the engine
    rejects.
12. **f-string slots cannot contain a string literal** — the lexer ends the f-string at the first
    inner `"`, so `f"{join(x, "-")}"` is unwriteable.

---

## data-CLI probe — concrete bugs while building a CSV tool

- **Builtin `sort` accepts a second comparator argument and SILENTLY IGNORES it** — sorts by natural
  order, no error. A silent-wrong-result footgun (distinct from "no sort-by-key builtin").
- **A blank CSV line parses to a phantom `['']` row**, which then crashes column access. One blank
  line kills the tool. Concrete csv_parse.zl bug.
- **`str()` drops the trailing `.0`** on whole floats (`str(370.0)`→`"370"`) and formats to 6 sig
  figs with no precision control — money/decimal columns cannot preserve two places.
- **Bounds-check asymmetry:** `at(s,i)` out of range returns `""` (lenient); `list[i]` out of range
  is a FATAL uncatchable error. Opposite policies for the two container types.
- **`else`/`else if` must be on the SAME line as the preceding `}`.** `} \n else {` is a parse error.
  Cuddled-else mandatory; Allman/BSD style is a syntax error.

## TUI probe

- **No terminal-size discovery at all** — no builtin, and `env('COLUMNS')`/`env('LINES')` are empty.
  A TUI must hard-code its dimensions.
- **TUI keys need read-WITH-TIMEOUT, not just non-blocking poll.** Arrow/Home/End/function keys arrive
  as multi-byte escape sequences; you must read a byte, then read more with a short timeout to see if
  it is a sequence. GAPS_REALWORLD framed input as blocking-vs-nonblocking; timed reads are a third mode.
- **ansi.zl is missing exactly the two most TUI-specific escapes:** the alternate-screen switch
  (`ESC[?1049h/l`) and any terminal-size helper — while having full colour/cursor/clear coverage.

## build-tool probe

- **Can't even probe for a file:** no `exists`, `read` aborts on a missing file, no `try`. The only
  way to test if a manifest exists is `dir()` the parent and string-scan names.
- **`dir()` isn't composable into `walk`** — bare names, no `is_dir`, so a recursive tree walk can't
  be written in userland.
- **Incremental build is impossible in principle:** no file `mtime`, and `now()` is process-relative
  (resets each run), so "built at T" cannot be persisted and compared.
- **A zl build tool cannot launch zl's own compiler** because `run` is simulated. The self-hosting
  ECOSYSTEM is blocked at the root: a toolchain that compiles itself cannot be managed by a program
  written in the language it builds.

## Zig deltas not on the roadmap

- **No-hidden-allocations as a principle** — zl allocates on every `+`, `push`, `range`, concat,
  f-string, with no way to declare or verify a function is allocation-free.
- **Defined overflow + wrapping/saturating arithmetic** (`+%`, `+|`, `@addWithOverflow`) — zl numbers
  are f64, so there is no exact 64-bit int, no overflow trap.
- **SIMD / `@Vector`** — no vector types; directly relevant to the raytracer and games zl ships.
- **Tagged unions with EXHAUSTIVE switch** — the planned switch is non-exhaustive if/elif sugar with
  no payload capture; Zig's compile-error-on-missing-case is absent.
- **Arbitrary-bit-width and 128-bit ints** (`u1`, `i7`, `u128`) — item 15 plans only i8–i64/u8–u64,
  leaving bitfield packing and wide crypto with no type.
- **Explicit cast vocabulary** (`@intCast`, `@truncate`, `@bitCast`, `@ptrCast`) — once sized ints
  exist there is no defined checked-narrowing or bit-reinterpret op.
- **Inline `test "..." {}` blocks run by `zl test`** — source-local tests, a headline Zig ergonomic.
- **Blocks/switch as value-expressions** with labeled value-break — zl's switch is
  statement-not-expression.
- **No-hidden-control-flow as a commitment** — `+` runtime-type-dispatched and `x/0` killing the
  process both violate it.

---

## What changes on the roadmap

1. **WRITE `std.iter` NOW.** First-class functions work; map/filter/reduce/sort-by/group-by are a
   library file away, no engine change. This was wrongly believed blocked. Highest-value free win.
2. **Fix the real bugs found:** for-range global leak, sort-ignores-comparator, break-flag-not-cleared,
   csv blank-line crash. These are live defects, same class as the JSON break.
3. **Lex `0xFF` / `1e6` / `.5`** — three literal forms silently fail. Small lexer fix.
4. **Correct the prior docs' "no first-class functions" claim** — it is false and it mis-shaped the
   closure item.
5. **A timed read is a third input mode** (beyond blocking/non-blocking) — the TUI-key requirement.
6. **Several Zig deltas are worth adding:** defined integer overflow, exhaustive switch with payload
   capture, and the no-hidden-allocations principle (which explicit allocators, item 40, partly serves).
