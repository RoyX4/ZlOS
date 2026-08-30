# Design: Proper Floating-Point Support in zl

**Status:** proposal
**Author:** language/compiler design pass, 2026-07-29
**Scope:** Floor 1 hardening + a concrete extension of Floor 4 (native x86-64 backend). No code is changed by this document.

---

## 0. TL;DR

zl already *stores* every number as a C `double` in the interpreter and the C
backend, so float arithmetic mostly works there. Three things are missing or
broken:

1. **Formatting control.** Printing is hardwired to "collapse whole-valued
   doubles to `%lld`, otherwise `%g`." There is no way to force decimals, set a
   precision, or print `10.0` as `10.0`. `%g` also silently switches to
   scientific notation and caps at 6 significant digits.
2. **Division semantics are undefined on paper and inconsistent in practice.**
   `/` is true (float) division in the interpreter and C backend, but there is
   no integer-division operator, and the **native backend does integer division
   for `/`** — so `7 / 2` is `3.5` interpreted and `3` native.
3. **The native x86-64 backend has no floats at all.** It is int64-only:
   literals go through `atoll` (so `3.14` becomes `3`), and every operator is a
   GPR integer instruction. Any program that touches a fraction breaks the
   "three engines produce identical output" invariant.

This proposal: **lock one numeric type (`f64`)**, **define one canonical
number-to-string format implemented identically in all three engines** (do not
lean on libc `%g`), add explicit integer-division / rounding built-ins, and
extend the native backend to real SSE2 `f64` arithmetic and a hand-assembled
float formatter. Each stage ends with a `run_tests.ps1`-style cross-engine
equality gate.

---

## 1. Motivation

### 1.1 The invariant that is currently protected by avoidance

`run_tests.ps1` runs every test through interpreter, C backend, and native
backend and asserts byte-identical output. The suite passes **only because it
was written to dodge the problem** — its own comment says so:

```
# NOTE: native backend is integer-only, so division is integer
# division. Use exact divisions here so all 3 engines agree.
"print(100 / 4)"   # 25 — chosen because it divides evenly
```

The moment a real program computes `100 / 3`, `avg = sum / n`, or any
measurement, the three engines disagree. A self-hosting systems language that
cannot represent `3.5` consistently across its own backends has a hole in Floor
1, and it will get worse as Floor 4 replaces the C backend entirely.

### 1.2 What "floats work in two of three engines" hides

Because `src/runtime/interp.c` and `src/runtime/runtime.c` both use `double num;`, the following already
run correctly under `interp.exe` and the C backend:

```
x = 3.14
print(x * 2)        # 6.28
print(10 / 3)       # 3.33333
```

This is a trap: it *looks* done. The native backend gives `3`, `0`, `3` for the
same program. The gap is invisible until someone compiles a float program with
`nativegen.exe`, and there is no diagnostic — it silently truncates.

### 1.3 Why formatting is a language-design issue, not a print() detail

Today `value_to_string` (in both `src/runtime/interp.c` and `src/runtime/runtime.c`) is:

```c
if (v.num == (long long)v.num)
    snprintf(buf, sizeof(buf), "%lld", (long long)v.num);   // 10.0 -> "10"
else
    snprintf(buf, sizeof(buf), "%g",  v.num);               // 6 sig figs, sci-notation
```

Consequences a user hits immediately:

- `print(10.0)` → `10`. You cannot tell a float from an int in output, and you
  cannot force a decimal point.
- `print(1.0 / 3.0)` → `0.333333` (6 sig figs; precision is just gone).
- `print(1e20)` → `1e+20`; `print(0.00001)` → `1e-05` (surprise sci-notation).
- No fixed-precision money/format: no `2` in `$3.50`.
- **It depends on libc.** Floor 4's endgame deletes libc; `%g` is a
  Ryū/Grisu-class algorithm we would then have to reproduce byte-for-byte to
  keep the native backend in agreement. If instead we *define* zl's own format,
  the native implementation becomes tractable (see §5).

So float printing must become **a specified language behavior with its own
formatter**, owned by zl, not a passthrough to `%g`.

---

## 2. Design decisions (proposed, to be locked)

### 2.1 One numeric type: `f64`

Keep the current model: there is exactly one number type, IEEE-754 `double`.
Rationale:

- It is what `src/runtime/interp.c`/`src/runtime/runtime.c` already do — least churn, no new type
  system.
- The locked spec (§4.4 of MASTER_PLAN) has no `int`/`float` keywords and wants
  the reserved-word list frozen at 11. A single `number` type keeps it there.
- The self-hosting compiler currently relies on **integer-valued doubles**
  behaving like integers (indices, table keys, char codes). A single `f64` type
  preserves that: integers up to 2^53 are exact, and the canonical formatter
  (§3) prints them without a fractional part.

Rejected alternative — **two types (`int` + `float`) with operand-dependent
`/`** (C / Python-2 style): more expressive, but it needs new keywords or type
inference, contradicts the locked spec, complicates the self-hosting compiler,
and collides with the Floor-4 low-bit-tag plan (§6). Not worth it for Floor 1.

> Forward note: exact-integer performance and the Floor-4 tagged-value runtime
> can still be recovered *without* a user-visible second type via NaN-boxing —
> see §6.

### 2.2 Division and modulo — make the semantics explicit

| Operator / builtin | Meaning | Rounding | All engines must agree |
|--------------------|---------|----------|------------------------|
| `a / b`            | true float division | none | `7 / 2 == 3.5` |
| `a // b` *(new)*   | integer division | **truncate toward zero** | `7 // 2 == 3`, `-7 // 2 == -3` |
| `a % b`            | remainder | matches `//` (truncated) | `7 % 2 == 1`, `-7 % 2 == -1` |
| `int(x)`           | already exists: truncate toward zero | — | unchanged |

Truncate-toward-zero is chosen (not Python floor-division) because it is what
`int()` already does, what x86 `idiv` already does, and what C `%` already does
— so the native backend needs **no** extra fix-up code to agree with the other
two engines. `a % b` is defined as `a - (a // b) * b`, i.e. current behavior
(`fmod`-of-truncation), just written down.

`//` is added rather than overloading `/` so that "I want an integer result" is
explicit and the common `sum / n` never silently truncates. It costs zero
reserved words (it is a symbol, like `/`). If a symbol operator is undesirable,
an `idiv(a, b)` built-in is an equivalent, keyword-free fallback.

### 2.3 Canonical number → string (owned by zl, not `%g`)

Define one deterministic format, implemented identically in interpreter, C
backend, and native backend:

- **Whole values** (`x == trunc(x)` and `|x| < 2^63`) print with **no decimal
  point**: `10.0` → `10`, `-3.0` → `-3`. (Preserves current integer-looking
  output the self-hosting compiler depends on.)
- **Fractional values** print in **fixed-point** with trailing zeros trimmed and
  a fixed maximum precision `P` (proposed `P = 15` significant... — see below),
  round-half-to-even at the last kept digit. `0.5` → `0.5`, `1.0/4.0` → `0.25`.
- **No implicit scientific notation.** Very large/small magnitudes print in full
  fixed-point (or clamp to a defined `inf`-style token beyond the formatter's
  range — decided in Stage 0).
- **Special values:** `nan`, `inf`, `-inf` as literal tokens.
- **Explicit precision** is available through a new `fmt(x, decimals)` built-in
  that returns a string (e.g. `fmt(3.14159, 2)` → `"3.14"`), leaving `print`'s
  default untouched.

The important property: **the format is simple enough to hand-assemble** (split
sign; integer part via the existing itoa loop on the truncated magnitude;
fractional part via a "multiply by 10, extract integer digit" loop; round at
digit `P`). That is what makes native-backend agreement achievable without
reproducing Ryū. Precision `P` is chosen so the fractional loop stays short and
the three engines round identically; the exact `P` and the rounding rule are the
one thing to pin down in Stage 0 with a shared test vector.

### 2.4 New / changed built-ins (all plain identifiers, no new keywords)

| Name | Signature | Notes |
|------|-----------|-------|
| `//` | operator | integer (trunc) division — §2.2 |
| `fmt` | `fmt(x, decimals) -> str` | fixed-precision formatting |
| `round` | `round(x) -> number` | round-half-to-even to integer value |
| `floor` | `floor(x) -> number` | toward −∞ |
| `ceil` | `ceil(x) -> number` | toward +∞ |
| `abs` | `abs(x) -> number` | magnitude |

`num` (parse) and `int` (truncate) already exist and are unchanged. This set is
the minimum that makes floats *usable*; transcendental math (`sqrt`, `sin`, …)
is explicitly out of scope for this proposal.

### 2.5 Lexer gaps (small, optional)

`lex_number` (src/frontend/lexer.c:103-115) already accepts `3.14` (digit, dot, digit). It
does **not** accept:

- exponent form `1e9`, `2.5e-3`
- leading-dot `.5`
- hex/underscore separators

Recommend adding **exponent support only** (a `number` type is nothing without
`6.022e23`); leading-dot and separators are cosmetic and can wait. This is a
~10-line change to `lex_number` and does not touch the grammar (the token stays
`T_NUMBER`, text passed through to `atof`/`strtod`).

---

## 3. Current-state map (for the implementer)

| Concern | Interpreter (`src/runtime/interp.c`) | C backend (`src/runtime/runtime.c`) | Native (`src/backends/native/nativegen.c`) |
|---|---|---|---|
| Value repr | `double num` (l.34) | `double num` (src/runtime/runtime.h:18) | **int64 in `rax`** |
| Literal | `atof`-parsed double | `zl_num(<text>)` emitted (src/backends/c/compile.c:160) | **`atoll(text)`** (l.142) → truncates |
| `+ - *` | double ops | `zl_binop` double ops (src/runtime/runtime.c:146-148) | `add`/`sub`/`imul` GPR (l.98-100) |
| `/` | **true division** | **true division** (src/runtime/runtime.c:148) | **`cqo;idiv`** integer (l.101) |
| `%` | trunc remainder | `(long long)a % (long long)b` (src/runtime/runtime.c:149) | `cqo;idiv` rdx (l.102) |
| compare | double compare | double compare | `cmp`+`setcc` (l.104-114) |
| print | `%lld`/`%g` (l.77-81) | `%lld`/`%g` (src/runtime/runtime.c:77-80) | itoa loop `emit_print_int` (l.296) |

**Reading it:** the two left columns are already float-capable and only need the
*formatting* and *`//`* work (Stages 1). The right column needs the whole SSE
build-out (Stages 2–3).

---

## 4. Staged implementation sketch

Each stage is independently shippable and ends with a cross-engine equality
gate, matching the repo's existing methodology.

### Stage 0 — Lock the spec + shared test vectors (no code)

- Ratify §2: single `f64` type, `//` trunc-toward-zero, `%` defined, canonical
  format, precision `P`, `inf`/`nan` tokens.
- Write a **golden file** of `(input expression → exact expected string)` pairs
  covering: whole doubles, `0.5`, `1/3`, negatives, `round`/`floor`/`ceil`
  boundaries (`0.5`, `2.5`, `-0.5`), large magnitudes, `nan`/`inf`.
  This file is the oracle every engine is later diffed against.
- **Verify:** the golden file is reviewed and frozen. Nothing to run yet.

### Stage 1 — Formatting + `//`/rounding in interpreter and C backend

Lowest risk, closes the *usability* gap immediately for the two mature engines.

1. Replace the `%g` branch of `value_to_string` (src/runtime/interp.c:77-81) and its twin in
   `src/runtime/runtime.c` (l.74-81) with a shared **canonical formatter** that both files
   compile (put it in one small `.c`/`.h` so the two engines cannot drift).
2. Add `//` to the parser precedence table (same level as `/`), and to the
   interpreter's binop dispatch + `zl_binop` (src/runtime/runtime.c:144-149).
3. Add `fmt`, `round`, `floor`, `ceil`, `abs` to `call_builtin`
   (src/runtime/interp.c:213…) and `zl_calln` (src/runtime/runtime.c:181…).
4. Optional: exponent lexing (§2.5).

- **Verify:** interpreter output == C-backend output == golden file, for every
  vector in Stage 0. (Native backend intentionally *not* yet included — it is
  expected to fail floats until Stage 3.)

### Stage 2 — Native backend: SSE2 `f64` arithmetic

Turn `src/backends/native/nativegen.c` from int64-only to `f64`. Represent **every** number as a
double so the native model matches the interpreter exactly (uniform `f64` — do
not try to keep a fast dual int/float path in v1; correctness and cross-engine
agreement first, per MASTER_PLAN §5.4's "correctness first, optimizer later").

Bricks (each a few tens of bytes of encoder work):

1. **`f64` constant pool.** Emit an `.rdata`-style block of 8-byte doubles;
   `N_NUMBER` becomes `movsd xmm0, [rip+kN]` (`F2 0F 10 05 <disp32>`) with the
   disp back-patched, replacing the `atoll` + `mov rax,imm64` path (l.141-143).
2. **Keep the existing stack machine, move values through GPR.** The current
   codegen `push rax` / `pop rcx` framework (l.167-170) can stay: after
   computing a subexpression in `xmm0`, `movq rax, xmm0`
   (`66 48 0F 7E C0`) then `push rax`; to reload, `pop rcx; movq xmm1, rcx`.
   This reuses all existing push/pop/backpatch logic — minimal structural
   change. (`movsd`/`movq` need no 16-byte alignment, sidestepping the
   `and rsp,-16` pain noted in the Floor-4 heap brick.)
3. **Arithmetic** in `emit_binop` (l.96): `+ → addsd` (`F2 0F 58 C1`),
   `- → subsd` (`5C`), `* → mulsd` (`59`), `/ → divsd` (`5E`). `xmm0 op xmm1`.
4. **`//` and `%`:** `//` = `divsd` then truncate (`cvttsd2si rax,xmm0`
   `F2 48 0F 2C C0`, then `cvtsi2sd xmm0,rax` `F2 48 0F 2A C0`). `%` =
   `a - trunc(a/b)*b` using the same converts. (Matches §2.2 trunc semantics
   with no sign fix-ups.)
5. **Compare:** `ucomisd xmm0,xmm1` (`66 0F 2E C1`) + `setcc` on the resulting
   flags, producing a `0.0`/`1.0` double for boolean results. Handle the
   unordered (NaN) case explicitly via the parity flag so `nan == nan` is false.
6. **Unary neg** (l.152): flip the sign bit with `xorpd xmm0, [rip+signmask]`
   (mask `0x8000000000000000`), instead of `neg rax`.
7. **`and`/`or`/`not`/truthiness:** a value is truthy iff `!= 0.0`; reuse
   `ucomisd` against a zero constant to produce the 0/1 the existing logic path
   expects (l.156-165).

- **Verify:** a temporary harness that prints the **raw 8-byte pattern** (or
  compares via exit code on integer-valued results) shows native arithmetic
  matches the interpreter for `+ - * / // %` and comparisons on float inputs.
  Printing is still wrong until Stage 3 — gate on arithmetic only here.

### Stage 3 — Native backend: canonical float printer

Replace/extend `emit_print_int` (l.296) with a hand-assembled implementation of
the §2.3 canonical format. Because the format is *defined by us*, this is
tractable:

1. Move the number to a GPR bit-pattern; branch out `nan`/`inf`/`-inf`
   (exponent all-ones) to emit the literal tokens.
2. Emit `-` if the sign bit is set; take the magnitude (`xorpd` sign clear).
3. **Integer part:** `cvttsd2si` the truncated magnitude into `rax` and reuse
   the *existing itoa loop* (l.310-318) to emit its decimal digits. (For
   `|x| >= 2^63`, fall back to a defined big-value path decided in Stage 0.)
4. **Fractional part:** if the residue is non-zero, emit `.`, then loop up to `P`
   times: `frac *= 10`, `d = cvttsd2si(frac)`, emit `'0'+d`, `frac -= d`.
   Round-half-to-even at digit `P`, then strip trailing zeros.
5. Wire `print` dispatch (l.216-231): since all numbers are now `f64`, the
   numeric branch always calls this one formatter (which itself prints
   whole values without a dot, per §2.3).

- **Verify:** add float programs to `run_tests.ps1` and require
  **interpreter == C backend == native == golden file**. This is the real
  finish line: the "three engines agree" invariant now holds *with* fractions,
  and the test suite no longer has to dodge non-even division.

### Stage 4 (forward-looking) — reconcile with the Floor-4 tagged-value runtime

MASTER_PLAN §5.4 brick 2 plans a low-bit tag (`ints = (n<<1)|1`, even =
pointer). That scheme is integer-centric and **has no room for an arbitrary
`f64`**. Options, to decide at the Floor-4 boxed-value boundary (not now):

- **NaN-boxing (recommended):** keep `f64` unboxed and fast; encode pointers and
  small-int fast-paths inside the ~2^51 NaN payload space. Preserves single
  `number` type at the language level while giving Floor-4 its tagged pointers.
- **Heap-boxed doubles + float tag:** simpler to implement, but every float
  allocates — bad for the numeric core.
- **Two representations with runtime promotion:** most code, most divergence
  risk.

This stage is a *note*, not work to schedule under this proposal; it exists so
Stage 2's "uniform `f64`" decision is made with the endgame in view.

---

## 5. Why this ordering

- **Stage 1 before Stage 2/3:** the interpreter and C backend are where users
  actually run programs today; fixing formatting there delivers the visible win
  first and produces the golden oracle the native work is graded against.
- **Define the format before writing the native printer:** a zl-owned fixed
  format is hand-assemblable; libc `%g` is not, and the Floor-4 no-libc endgame
  would otherwise force us to reimplement Ryū just to keep the backends in sync.
- **Uniform `f64` in the native backend before any int/float fast-path:**
  correctness and the three-engine invariant first; an optimized dual path is an
  optional later refinement, consistent with MASTER_PLAN's "slow correct code is
  a finished floor."

## 6. Risks / open questions

| # | Item | Note |
|---|------|------|
| 1 | **Rounding parity** | The one true hazard: interpreter (`snprintf`/libm) and the hand-rolled native printer must round *identically*. Mitigation: zl defines its own round-half-to-even at digit `P`; do **not** delegate to `%g`. The Stage-0 golden file is the contract. |
| 2 | **Precision `P`** | Too large → long fractional loop and double-rounding artifacts; too small → visible precision loss. Pick empirically in Stage 0 against the golden vectors. |
| 3 | **Self-hosting compiler regression** | `src/selfhost/compiler.zl` assumes integer-valued doubles print as integers and that `/`-on-indices behaves. §2.1/§2.3 preserve both; still, re-run the fixpoint (`gen1.c == gen2.c`) after Stage 1. |
| 4 | **`%` on floats** | Defined as truncated remainder (current behavior, written down). If `fmod`-style is ever wanted it is a separate decision. |
| 5 | **NaN in comparisons** | Native `ucomisd` sets the parity flag on unordered; Stage 2 must handle it so `nan == nan` is false in all three engines. |
| 6 | **Big magnitudes / `2^63` overflow in the printer** | `cvttsd2si` is undefined past int64 range; Stage 0 must define the large-value print path (full fixed-point vs. a token). |

## 7. Definition of done

1. Spec §2 locked; Stage-0 golden file frozen.
2. `interp.exe` and the C backend emit the canonical format and support
   `//`/`round`/`floor`/`ceil`/`abs`/`fmt`; both match the golden file.
3. `nativegen.exe` does real SSE2 `f64` arithmetic and prints via the canonical
   formatter.
4. `run_tests.ps1` gains float programs and the comment "native backend is
   integer-only, use exact divisions" is **deleted** — because it is no longer
   true. All engines agree on `100 / 3`.
5. `src/selfhost/compiler.zl` still self-hosts to a byte-identical fixpoint.
