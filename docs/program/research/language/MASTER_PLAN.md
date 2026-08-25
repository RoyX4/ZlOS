# MASTER PLAN — The Full Stack

**Owner:** Zac
**Written:** 2026-07-29
**Last updated:** 2026-07-31
**Supersedes:** `old/PLAN.md` (skeleton), `old/LOOK.md` (dead syntax mockup) — both archived, do not build from them
**Status:** 🏆 FLOOR 1 COMPLETE — the language is SELF-HOSTING. 🏆 FLOOR 4 "delete C from the OUTPUT" COMPLETE — `nativeval.c` compiles `compiler.zl` to a kernel32-only native compiler that reproduces the interpreter's output byte-for-byte. **Five execution engines** (interpreter, boxed C, unboxed C, own x86-64, LLVM IR), **93 builtins**, **64 stdlib modules written in zl**, one gate (`verify.ps1`).
**Current direction (2026-07-30 →):** *speed + C-class capability, on Windows.* See §3.2 and `docs/OVERNIGHT_CAMPAIGN.md`. The floors below are unchanged and still the map; the near-term work is widening and speeding up the seed before climbing further.

---

## 0. The one sentence

> Build my own CPU, assembler, programming language, self-hosting compiler, operating system, and antivirus — every layer by hand — as one connected system.

### The companion documents

This file is the **source of truth for what the project is and why**. It is not the only file,
and it deliberately does not duplicate the others:

| Document | Owns |
|----------|------|
| `MASTER_PLAN.md` *(this file)* | the floors, the locked design decisions, the risks, the reasoning |
| `docs/OVERNIGHT_CAMPAIGN.md` | the wave-by-wave execution plan and the log of what shipped |
| `docs/REFERENCE.md` | the accurate language reference — syntax, operators, all 93 builtins, the stdlib list |
| `README.md` | what exists, how to build it, how to run the self-hosting proof |
| `bench/README.md` | the engines, the benchmarks, and the measured numbers |
| `docs/design/*.md` | design proposals — type system, floats, maps, modules, optimizer, native runtime |
| `verify.ps1` | the gate. Not a document; the thing all of the above must stay true against. |

---

## 1. The rule that keeps this alive

**Impressiveness = difficulty × completion.**

A 10/10-hard project at 40% done scores near zero. A 6/10 project at 100% beats it every time.

Three hard laws that follow from this:

| Law | Meaning |
|-----|---------|
| **Finish floors** | Never start floor 7 while floor 1 is at 50%. |
| **Every floor ships alone** | Each floor is a complete, demoable thing even if I stop forever after it. |
| **Never half a tower** | I always have a *growing* tower, never a *broken* one. |

If this project dies, it will die of **starting floor 4 while floor 1 was at 60%**. That is the single biggest risk. Everything below is built to prevent it.

---

## 2. The shape of the tower

I start in the **middle** and grow both directions. The language is the only layer that is both *finishable alone* and *useful immediately*.

```
        ↑ UP — flashy, AI-facing, impressive
   [8]  cross-device: one program everywhere
   [2]  AI compiles English threat descriptions → rules
   [3]  rule language + Aegis loads rules live   →  the Aegis wire
   ────────────────────────────────────────────────
   [1]  LANGUAGE + SELF-HOSTING COMPILER   ★ THE SEED          ✅ DONE
   ────────────────────────────────────────────────
   [4]  my own machine-code output (no LLVM)                   ✅ headline DONE
   [5]  my own assembler
   [6]  my own tiny OS
   [7]  my own CPU (simulator → FPGA)
        ↓ DOWN — deep, silicon, hardcore
```

**Work UP** when I want a flashy win. **Work DOWN** when I want deep engineering. The seed supports both, and neither direction blocks the other.

**Where the tower actually is (2026-07-31):** floors 1 and 4 are standing. Nothing else has
started, and that is on purpose — the current work is **widening and speeding up the seed**
(§3.2) rather than adding a floor. Widening the seed is not floor-hopping: every floor above
and below is built *in* zl or *by* zl, so a faster, more capable seed makes all of them
cheaper. The line that must not be crossed is starting floor 6 while this is unfinished.

---

## 3. Status board — what actually exists right now

| Thing | State | Where |
|-------|-------|-------|
| Syntax design | ✅ **Locked** | §4 below |
| **Lexer** | ✅ **Built, compiles /W4 clean, working** | `lexer.c` / `lexer.h` |
| **Parser** | ✅ **Built, working — precedence correct, parses all of hello.zl** | `parser.c` / `parser.h` |
| **Interpreter** | ✅ **Built + real file I/O (read/write/dir/procs) + 93 builtins** | `interp.c`, `os_win.c` |
| **C backend (boxed)** | ✅ **WORKS — .zl → out.c → real standalone .exe. Output byte-identical to interpreter. Whole language.** | `compile.c`, `runtime.c/.h` |
| **Self-hosting** | 🏆 **ACHIEVED. `compiler.zl` (lexer+parser+codegen, all in zl) compiles its OWN source. Fixpoint proven: interpreter→gen1.c→zlc.exe→gen2.c, gen1.c == gen2.c byte-identical. The property is `f(f(x)) == f(x)`, NOT a frozen hash — the hash and the line count change legitimately every time `compiler.zl` changes.** | `compiler.zl`, `verify.ps1` |
| **x86-64 backend** | ✅ **Real machine code, own PE writer, no C compiler in the output. Integer subset.** | `nativegen.c` |
| **No-C runtime + native self-host** | 🏆 **All 8 bricks done. `nativeval.exe compiler.zl` → `nvout.exe` (33 KB, kernel32 only), which recompiles `compiler.zl` byte-identically.** | `nativert.c`, `nativeval.c` |
| **Unboxed C backend** | ✅ **`compilef.c` — raw `long long` C, numeric subset. The proof that unboxing is worth ~2× on `fib` alone.** | `compilef.c` |
| **LLVM backend** | ✅ **`compilel.c` — emits LLVM IR (`out.ll`), then `clang -O2`. Handles arithmetic, comparisons, logic, variables, `if`/`else`, `while`, `break`/`continue`, user functions with `alloca` locals, and string literals. All 6 `run_tests` programs compile through it and match the interpreter byte-for-byte.** | `compilel.c` |
| **Builtins** | ✅ **93**, parity-checked between `interp.c` and `runtime.c` (66 → 93 in wave 1) | `docs/REFERENCE.md` |
| **Standard library** | ✅ **64 modules, all written in zl** (49 → 64) | `stdlib/*.zl` |
| **The gate** | ✅ `verify.ps1` — fixpoint + 3-engine agreement, one command | §3.1 |
| **Benchmarks** | ✅ **All engines measured** — unboxed 30–80× the interpreter, boxed C only 7–13× | `bench/`, `bench/README.md` |
| **Type system** | 📄 **Designed, not built** — gradual, opt-in, the unboxing enabler | `docs/design/design_type_system.md` |
| **Aegis (kernel antivirus, C)** | ✅ **Already built & working** | `C:\Users\royx4\kernel-defender-lab` |

Aegis is a real head start. Floor 3 has a target that already exists, and floor 6 has a person who has already written kernel-mode code.

### 3.1 — The gate: `verify.ps1`

One command decides whether a change is allowed to exist. It checks exactly two things:

| # | Check | Passes when |
|---|-------|-------------|
| 1 | **Self-hosting fixpoint** | the interpreter compiles `compiler.zl` → `gen1.c`; `gen1.c` is built into a native compiler; that compiler compiles `compiler.zl` → `gen2.c`; **`gen1.c` == `gen2.c`** (SHA256) |
| 2 | **Three-engine agreement** | `run_tests.ps1` is 6/6 — interpreter, C backend and x86-64 backend print identical output for every test program |

**Read check 1 correctly.** The fixpoint is the property **`f(f(x)) == f(x)`** — gen1 compared to gen2, in this run. It is **not** a fixed hash and **not** a fixed line count. It was 936 lines, then 887, and it moves again every time `compiler.zl` changes. A changed hash is not a regression; a `gen1 != gen2` is. Anywhere this document or any other says "the fixpoint hash", read "gen1 == gen2".

Rule: **`.\verify.ps1` must be GREEN before any commit.** Red means the change broke the language — fix that one change or revert it.

### 3.2 — Where the effort is going right now

Floor 1 is finished and Floor 4's headline ("no C in the output") is finished, so the tower is no longer the bottleneck — **capability and speed are.** The standing objective is now:

> **Make zl as capable as C/C++, and fast — staying on Windows/x86.**

That work is organised as waves in **`docs/OVERNIGHT_CAMPAIGN.md`** (the operational plan; this document stays the strategic one):

| Wave | What | State |
|------|------|-------|
| W0 | 5 stdlib libraries, `docs/REFERENCE.md`, `verify.ps1` | ✅ green |
| W1 | LLVM control flow · builtins 66→93 · stdlib 54→64 · `bench/` harness · type-system design doc | ✅ green |
| W2 | syntax and surface — `elif`, `do/while`, ranged `for`, `switch`, string interpolation, destructuring, `//` and `/* */` comments, ternary, more escapes | 🔨 in progress |
| W3 | **the gradual type system** (`docs/design/design_type_system.md`) — the unboxing enabler | planned |
| W4 | speed — constant folding, interned strings, killing the copy-on-write list clone, `-O3`/LTO, runtime optimisation | planned |
| W5 | systems capability — raw memory, structs, fixed layout, freestanding/no-CRT output, inline asm, a real FFI | planned |
| W6–W8 | kernel/driver · the OS floor · a game system (the end-to-end proof) | planned |

**The measured number that steers W3 and W4** (`bench/README.md`, best of 5):

| Engine | Speedup vs interpreter |
|--------|-----------------------|
| unboxed C (`compilef`) / own x86-64 (`nativegen`) / LLVM (`compilel`) | **30–80×** |
| boxed C (`compile`) | **7–13×** |

Same generated-then-`-O2`'d C on both sides of that gap. The 4–6× difference is **purely tagged values plus a malloc per operation** — nothing else. That is the case for the type system, measured rather than argued, and it is why W3 comes before W4.

Two more findings from the same run, both uncomfortable and both important:

- **`b5_string` is only 1.3× faster compiled**, because interpreter and compiled code spend their time inside the *same* `runtime.c` string routines. Compiling the control flow is worthless when the work is in the runtime — so W4 must optimise the **runtime**, not just codegen.
- **Coverage is the real gap.** The fast engines are fast partly because they refuse to compile most of the language: 2 of 5 benchmarks cannot run on any of them at all.

---

## 4. The language — locked spec

### 4.1 Identity

- **Category:** systems language (same shelf as C / C++ / Rust — runs with nothing underneath it, can write a driver or an OS)
- **The bet:** **TOTAL CONTROL OF MY PC.** Nothing off-limits, nothing hidden. Files, processes, memory, hardware, GUI, kernel — all first-class, no ceremony.
- **Priority:** power > safety. I accept I can crash my own machine.
- **Not:** beginner-friendly. Not "safe". Not a teaching language.
- **The one novel idea:** dangerous operations still work, but must be marked `!` ("i mean it") — a middle path between C (no guardrails at all) and Rust (compile-time walls that stop you).

### 4.2 Syntax — decided, do not re-litigate

```
# variables — no keyword
x = 5
name = "Zac"

# print — + joins text
print("hi " + name)

# blocks use { }
if age >= 18 {
    print("adult")
} else {
    print("kid")
}

# lists + loops
names = ["Zac", "Sam", "Alex"]
for n in names {
    print(n)
}

# functions
fn double(n) {
    return n * 2
}

# the part that's MINE — controlling the PC
for f in dir("Downloads") {
    if f.age > 30d {
        rm(f)
    }
}
kill("chrome.exe")
start("notepad.exe")

# raw memory — total power, ! required
poke("game.exe", 0x4000, 999)!

# GUI — driving existing apps
w = window("Calculator")
w.click("5")
print(w.text())
```

### 4.3 The rule list

| Rule | Decision |
|------|----------|
| Blocks | `{ }` — never `end`, never indentation. Nothing invisible can break. |
| Statement end | **newline** — no semicolons |
| Compare | `==` `!=` `>=` `<=` `>` `<` |
| Functions | `fn name(args) { }`, `return` |
| Loops | `for x in y { }` |
| Reaching into a thing | `.` — `f.age`, `w.text()` |
| Text joining | `+` |
| Comments | `#` to end of line |
| Booleans / logic | words: `not`, `and`, `or`, `true`, `false` |
| `!` | **danger marker ONLY.** Never negation. Keeps it rare enough to still look scary. |
| Built-ins (`print`, `kill`, `dir`, `poke`, `window`) | **plain identifiers, NOT keywords.** Adding 200 more built-ins costs zero reserved words. |
| Banned forever | header files, `printf`/`%s`, `cout <<`, semicolons, `let` |

### 4.4 Reserved words (the complete list)

```
if  else  for  in  fn  return  while  not  and  or  true  false  break  continue
```

Fourteen words. (Originally eleven; `while`, `break` and `continue` were added when loop
control was built — a deliberate, one-time widening, not drift.) Every word added here is a
name users lose forever, so this list stays small on purpose. Note what is **not** here: all
93 built-ins are ordinary identifiers, and the type names in the W3 design (`int`, `float`,
`str`, `bool`, `list`, `any`) are recognised **only in type position**, so the type system
costs zero reserved words.

### 4.5 The self-hosting minimum

The language **must** have these before the compiler can be written in itself:

- strings / text
- lists
- records (a thing with named parts) — needed for AST nodes
- functions + recursion
- maps / lookup tables — needed for keywords and variables
- read/write files
- if / loops

**Deliberately cut** (not needed to self-host, so not built early): classes, inheritance, threads, generics, exceptions, modules.

### 4.6 What the language grew *after* self-hosting (2026-07-30 →)

The minimum in §4.5 was for reaching the fixpoint. Everything since has been about being
*worth using*. Nothing here reopens §4.2 — it is all additive.

| Added | Detail |
|-------|--------|
| **`while`, `break`, `continue`** | plus `else if` chains |
| **Compound assignment** | `+= -= *= /= %=` |
| **Index assignment** | `x[i] = v`, including nested `grid[0][1] = 77` |
| **93 builtins** | up from 66. Bitwise (`band` `bor` `bxor` `bnot` `shl` `shr`) — zl has no bitwise *operators*, these cover it. Plus `asin` `acos` `log2` `log10` `hypot` `trunc` `clamp` `fmod`, `ltrim` `rtrim` `title` `swapcase` `index_at`, `concat` `fill` `flat` `take` `drop`, and `now()` / `exit()` / `env()`. Full list: `docs/REFERENCE.md`. |
| **64 stdlib modules** | up from 49, all written in zl — data structures, algorithms, parsers, even a Lisp interpreter and a Brainfuck interpreter. Adding these costs nothing in C, which is the point. |

**Still deliberately absent, and load-bearing to remember when planning:** no first-class
function values, no structs/records, no maps as a native type (`stdlib/dict.zl` fakes it),
numbers are all doubles. W5 brings structs and raw memory; the rest stay cut until something
real demands them.

### 4.7 The gradual type system — designed, not built

`docs/design/design_type_system.md` is the full spec. The one-paragraph version:

> Optional annotations — `x: int = 5`, `fn f(a: int, b: float) -> int { }`. Six types:
> `int`, `float`, `bool`, `str`, `list[T]`, `any`. **`any` *is* today's boxed value**, so
> unannotated code — every one of the 64 stdlib modules, and `compiler.zl` itself — compiles
> to exactly what it compiles to now, with zero edits. Annotated code unboxes. Boxing happens
> only at enumerated boundaries.

Two things make it the right next big move rather than a nice-to-have:

1. **It is the measured bottleneck.** §3.2's 4–6× gap between boxed and unboxed is the cost
   of tagged values and malloc-per-op. Types are the only mechanism that removes it without
   lying about the language.
2. **Hindley-Milner is ruled out on evidence, not taste.** `stdlib/jsonw.zl`,
   `stdlib/json_parse.zl`, `stdlib/lisp_interp.zl` and `compiler.zl`'s own
   `["bin", "+", L, R]` AST are all legitimately heterogeneous. An inference algorithm that
   rejects the language's own standard library is a rewrite, not a feature.

The governing invariant is **Annotation Neutrality**: adding or removing annotations must
never change a program's output. Annotated code may be *rejected*; it may never quietly do
something else. The staged plan (syntax-only first, provably inert) is §8 of the design doc.

---

## 5. The floors, in detail

Time estimates are **honest guesses, and they are optimistic**. Treat the ranges as the real number.

---

### 5.1 — FLOOR 1: Language + self-hosting compiler ★ THE SEED   ✅ COMPLETE

**Trophy:** 🏆 *"My compiler compiles itself."* — **claimed.**
**Estimate:** 2–4 months → realistically **4–8 months** part-time.
**Blocks:** everything. Nothing else can start until this is real.

**The bootstrap plan — two separate "borrow then delete" swaps:**

| Swap | Borrow | Delete when |
|------|--------|-------------|
| **What it's written in** | C (I already write C for Aegis) | the compiler is rewritten in my own language → **self-hosting** |
| **What it outputs through** | possibly LLVM (fast, every chip free) | I write my own machine-code backend → **floor 4** |

These are independent and happen years apart. The language works the entire time.

**Sub-steps, each with a pass/fail test:**

| # | Step | DONE means |
|---|------|-----------|
| 1 | Syntax design | ✅ §4 is locked |
| 2 | **Lexer** | ✅ `lexer.exe hello.zl` prints correct tokens |
| 3 | **Parser** | ✅ prints a tree for `2 + 3 * 4` with `*` bound tighter; parses all of hello.zl |
| 4 | Tree-walking interpreter | ✅ `print("hello")` prints; hello.zl runs end-to-end |
| 5 | Core language | ✅ variables, `if`, `for`, `fn`, lists, strings, recursion all run |
| 6 | PC-control built-ins ← IN PROGRESS | `read()`/`write()`/`write_bytes()`/`dir()`/`procs()`/`env()` are REAL (touch the disk / the OS, via os_win.c). `rm`/`kill`/`start`/`poke`/`peek` still SIMULATED (destructive → earn real behaviour after VM testing). Real raw memory is scheduled as W5. |
| 7 | Write real programs in it | ✅ 3 real programs: `count.zl` (line counter), `search.zl` (grep, 24 hits), `report.zl` (writes REPORT.md). Design pain found+fixed: (a) no string tools → added `len`/`lines`/`ends`/`has`; (b) `\n` escapes were literal → lexer now translates escapes. |
| 8 | Compiler backend | ✅ `.zl` → `out.c` (via compile.exe) → real `.exe` (via cl + runtime.c). test.zl & hello.zl compile and run standalone, output byte-identical to the interpreter. NOTE: emitting C is the "borrowed backend" (like LLVM) — Floor 4 replaces it with own machine code. |
| 9 | **Rewrite compiler in itself** | ✅ 🏆 **SELF-HOSTING ACHIEVED.** `compiler.zl` = full lexer+parser+C-codegen written in zl. It compiles its own source; the self-compiled native compiler reproduces byte-identical output (gen1.c == gen2.c). Both the line count and the hash are *derived*, not fixed — they move whenever compiler.zl changes. The fixpoint is the property `f(f(x)) == f(x)`: compare gen1 to gen2, never to a number written down last week. Guarded permanently by `verify.ps1` check 1 (§3.1). |
| 10 | Delete the C | **half done.** C is gone from the **output** (Floor 4 brick 8 — `nvout.exe` is kernel32-only). C is still the **source** of the toolchain; removing it means rewriting the backend in zl and moving the C files to `bootstrap/`. Deliberately deprioritised 2026-07-30 in favour of speed + capability (§3.2). |

**Step 7 is the one people skip, and it's the one that matters.** A language nobody has written real programs in is full of design mistakes you can't see yet. Do not go to step 8 without it.

**The moment it becomes real:** step 4. After the interpreter runs one line, you own a working programming language. Everything after that is widening it.

---

### 5.2 — FLOOR 2: AI compiles English threat descriptions into rules

**Trophy:** 🏆 *"I describe a virus in English, and my compiler turns it into kernel-speed detection code — the same way every time."*
**Estimate:** +3 months → realistically **4–6 months**.
**Needs:** **Floor 3 complete** (the rule language must exist before AI can target it).

> **History:** this floor originally meant "AI resolves synonyms at compile time" — built for an English-like beginner language. §4 locked a symbol syntax instead, which deleted the synonym problem entirely. Repointed 2026-07-29 to the job below, which genuinely needs judgment.

**The job:** you write a threat in plain English. At **compile time**, an AI turns it into a floor-3 rule. The AI's output is then **frozen into the source** — readable, reviewable, editable.

```
# what you write
detect "ransomware" {
    ai: "anything spawned by an Office app that rapidly
         encrypts or renames large numbers of user files"
}

# what the compiler freezes into place — you can read and edit it
detect "ransomware" {
    if it.parent in OFFICE_APPS
       and it.writes > 50
       and it.window < 10s
       and it.entropy_delta > 0.7 {
        block(it)!
    }
}
```

**Why this is the strongest version of the floor:**

| Property | Why it matters |
|----------|---------------|
| **Zero runtime AI** | No API call in the kernel. Ever. The AI ran months ago on your machine. |
| **100% deterministic** | Same input → same binary, forever. Frozen output = a normal build. |
| **Reviewable** | The generated rule is source code you can read, diff, and hand-edit. |
| **Real judgment** | "Describe malware behavior in English" is a genuinely fuzzy problem. Unlike synonyms, you can't design it away. |
| **Research-backed** | This is exactly the *Compiled AI* pattern (arXiv 2604.05150) — LLM at compile time, deterministic at runtime. |

**Sub-steps:**
1. Design the `ai:` block — how a description is written and where output lands
2. Compile-time hook that calls the model **once**, then caches by content hash
3. Freeze generated rules into source, clearly marked as generated
4. Human review gate — **nothing AI-written reaches the kernel unreviewed**
5. Prove determinism: rebuild 10× offline, get byte-identical output every time

**Hard constraint:** AI output is a **draft that a human approves**, never something that auto-installs into a kernel driver. Step 4 is not optional.

**Blocks nothing.** If it's never built, floors 3–8 are unaffected.

---

### 5.3 — FLOOR 3: Self-evolving detection rules → the Aegis wire

**Trophy:** 🏆 *"My language writes my antivirus's brain."*
**Estimate:** +3 months → realistically **4–6 months**.
**Needs:** Floor 1 complete (through step 8, real codegen).

**The problem it solves:** Aegis currently has detection rules **hardcoded in C**. Adding one detection means recompiling a kernel driver. That's slow, risky, and means the AV can never learn.

**The fix:** detections become text in my language:

```
when process starts {
    if it.parent == "winword.exe" and it.writes > 50 {
        block(it)!
        alert("possible ransomware")
    }
}
```

...compiled to fast code that Aegis loads at runtime without rebuilding the driver.

**Why this floor is the keystone:** it is the wire that turns "a language" + "an antivirus" from two separate projects into **one system**. Without it, they're unrelated hobbies. With it, they're a story.

**Sub-steps:**
1. Design the rule sub-language (a restricted dialect — no raw memory, no `!`, safe by construction because it runs in the kernel)
2. Compile rules to a bytecode Aegis can load
3. Write the loader/VM inside Aegis (kernel-mode — **this is dangerous work, VM only**)
4. Prove it: add a new detection **without recompiling the driver**
5. (Optional) rules that rewrite themselves based on what they catch

**Hard constraint:** step 3 runs in kernel mode. A bug is a BSOD, and a *bad* bug on a real machine is data loss. **All of floor 3 step 3+ is VM-only until proven.** Aegis's existing 13 verified phase tags set the precedent — keep it.

---

### 5.4 — FLOOR 4: My own machine-code output (no LLVM)   🏆 HEADLINE DONE

**Estimate:** +2 months → realistically **3–5 months**.
**Needs:** Floor 1 complete.
**State:** all 8 no-C-runtime bricks done; `nativeval.c` compiles `compiler.zl` into a
kernel32-only `nvout.exe` that reproduces the interpreter's `out.c` byte-for-byte. The
remaining piece of the floor — deleting C from the *source* — is deprioritised (§3.2, step 10).

Stop borrowing the C compiler. Emit x86-64 machine code directly and write a real Windows PE `.exe` file, byte by byte.

**Sub-steps:**
1. ~~Write a valid PE header + hand-encoded machine code~~ ✅ **DONE** — `native.c` → `raw.exe` (ExitProcess(42), exit code 42). First C-free executable.
2. ~~Import WriteFile/GetStdHandle → real output from raw code~~ ✅ **DONE** — `nativep.c` → `rawp.exe` prints "Hello from raw machine code!". Zero C compiler in the output.
3. ~~Emit x86-64 from the AST (real codegen, not hardcoded)~~ ✅ **DONE** — `nativegen.c` is a REAL x86-64 backend: stack-machine codegen + backpatched jumps. Integer subset (vars, + - * / %, comparisons, and/or/not, if/else, while, exit()). Verified: sum1..10→55, factorial(5)→120, if/else+mul→42, 100%7→2, all as native .exe via exit code. `nativegen.exe file.zl` → native.exe, no cl.
4. ~~User functions + recursion (own calling convention)~~ ✅ **DONE** — nativegen.c emits native calls (args pushed right-to-left, result in rax, caller cleanup, backpatched call targets). Verified as native .exe: recursive factorial(5)=120, recursive fib(10)=55, add(mul(3,4),5)=17. Fixed an else-less-`if` NULL-block bug along the way.
5. ~~Working native `print`~~ ✅ **DONE** — `nativegen.c` emits a hand-assembled `print_int` routine (integer→decimal ASCII + WriteFile) + a 3-function import table (GetStdHandle/WriteFile/ExitProcess). Verified: prints fib 0..9, squares 1..5, all as native x86-64, clean exit. (Fixed a real bug: fall-through exit still used the old ExitProcess IAT RVA 0x2038 after the table grew to 3 imports → access violation; now uses 0x2058.)
6. **The no-C runtime** (the real "delete C entirely" work) — reimplement runtime.c (~450 lines) as hand-assembled x86-64 using ONLY Windows syscalls, no libc. Bricks 1–5 live in **`nativert.c`**, which emits `raw_rt.exe` (a runnable proof program) and grows one brick per commit:
   1. ~~Heap from the OS~~ ✅ **DONE** — `nativeheap.c` → `raw_heap.exe` calls VirtualAlloc. No libc. (Lesson: VirtualAlloc needs `and rsp,-16` strict 16-byte alignment.)
   2. ~~Tagged value format~~ ✅ **DONE** — ints = `(n<<1)|1`, pointers even → heap objects. `zl_alloc`/`zl_int`/`zl_untag`/`zl_mul` + `zl_write` tag dispatch. Negatives work (`sar`, not `shr`). Verified: `42 / 42 / 42 / -7 / 0 / nil`. (Lesson: backpatch every jump — never hand-count a displacement.)
   3. ~~String objects on the heap~~ ✅ **DONE** — `[+0]=0 [+8]=len [+16]=bytes`; `zl_str_lit`, `zl_concat`, `zl_str_eq` (byte compare via `repe cmpsb`, not pointer compare), `zl_len`. Verified incl. empty-string identity.
   4. ~~Polymorphic operators~~ ✅ **DONE** — `zl_add` dispatches int+int / str+str / list+list; `zl_lt/le/gt/ge` compare **tagged values directly** (order survives the tag); `zl_eq/ne`; `zl_truthy`.
   5. ~~List objects~~ ✅ **DONE** — `[+0]=1 [+8]=count [+16]=slots`; `zl_list_new/set/index/push/concat`. `push` is copy-on-write. `zl_write` **recurses** for nested lists: `[world, [10, 20, 30]]`.
   6a. ~~String builtins~~ ✅ **DONE** — `zl_at` (returns a 1-char string, so `at(src,i)=="("` works), `zl_has` (substring).
   7. ~~File I/O via syscalls~~ ✅ **DONE** — `zl_read`/`zl_write_file` via CreateFileA/ReadFile/WriteFile + CloseHandle, `zl_cstr` to NUL-terminate. No fopen.
   6. ~~Boxed-value native backend (`nativeval.c`)~~ ✅ **DONE** — walks the AST, emits x86-64 that computes with tagged values by calling the runtime. Full language: expressions, if/while/for, functions+recursion, global scope (incl. the leak-to-global gotcha), lists, indexing, the 7 builtins. Matches the interpreter on the whole 6-program suite.
   8. ~~Point it at compiler.zl → native self-host~~ ✅ **DONE 🏆** — `nativeval compiler.zl → nvout.exe` (33 KB, **kernel32-only, no libc**); `nvout.exe` recompiles compiler.zl to `out.c` **byte-identical** to the interpreter's (SHA256 match, 887 lines).
   *(**ALL 8 bricks done — "delete C from the OUTPUT" complete.** Two bugs stood in the way: cfix[256] overflow when a big program made thousands of runtime-call sites, and `+` needing to stringify (num+num=add, list+list=concat, else `to_string(l)+to_string(r)`) — compiler.zl relies on `"..."+len(...)`. Both fixed. What remains is "delete C from the SOURCE": rewrite nativeval in zl, then move the C files to `bootstrap/`.)*

   **Note on step 10 of Floor 1 ("delete the C"):** the two removals are separate. Bricks 2–8 remove C from the **output**. Removing C from the **source** happens after brick 8, by rewriting the backend in zl; the C files then move to `bootstrap/` as the historical seed (every self-hosting language keeps one — Go until 1.5, Rust to this day). Adding new **stdlib modules** costs nothing either way since they are written in zl; adding new **C builtins** before the switch does, because each one becomes another hand-assembly job. So: no new C builtins until brick 8.

**Why it's worth it:** this is the step where you stop being a "language on top of someone else's compiler" and start owning the whole path from text to CPU. It also makes floors 5–7 possible — you can't target your own CPU through LLVM.

**Honest note:** the resulting compiler will produce *slow* code at first. No optimizer. That's fine and expected — correctness first, and an optimizer is its own (optional) project later.

#### 5.4a — The "no LLVM" title, honestly (updated 2026-07-30)

The floor is titled "no LLVM" and that title is still true of what it delivered: `nativegen.c`
and `nativeval.c` emit x86-64 and write the PE by hand, with no LLVM and no C compiler
anywhere in the output path. **That has been achieved and it is not being given back.**

What changed is that LLVM was then *added alongside it* as a **speed** backend, not as a
replacement for the own-codegen path:

- `compilef.c` — unboxed `long long` C. Built first as a cheap experiment to prove unboxing
  is where the speed is (~2× on `fib` from unboxing alone). It worked, so:
- `compilel.c` — the LLVM backend. `.zl` → `out.ll` → `clang -O2` → `.exe`. It now handles
  arithmetic, comparisons, logic, variables, `if`/`else`, `while`, `break`/`continue`, user
  functions with `alloca` locals, and string literals — **all 6 `run_tests` programs compile
  through it and match the interpreter byte-for-byte.** LLVM IR is *typed*, which is exactly
  why it is the natural consumer for the W3 type system.

So the repo now has **five** engines and each earns its place:

| Engine | File | What it is for |
|--------|------|----------------|
| interpreter | `interp.c` | the reference semantics; the slow, complete, always-right one |
| boxed C | `compile.c` + `runtime.c` | the only *compiled* engine that runs the whole language |
| unboxed C | `compilef.c` | the unboxing proof; numeric subset |
| own x86-64 | `nativegen.c` / `nativeval.c` | Floor 4's trophy and the road to Floors 5–7 — no toolchain at all |
| LLVM | `compilel.c` | the daily speed target, and the type system's consumer |

**The rule that keeps this from becoming floor-hopping:** LLVM is a *second* backend, exactly
the way §7 says the own-CPU backend will be. **Own codegen stays alive and must keep passing
`verify.ps1`.** Never let the fast backend rot the hand-built one.

---

### 5.5 — FLOOR 5: My own assembler

**Estimate:** +1 month → realistically **1–2 months**.
**Needs:** Floor 4 (you'll already have written most of it).

Takes human-readable assembly (`mov rax, 5`) → machine bytes. Mostly a byproduct of floor 4 — the encoder already exists, this exposes it as a standalone tool.

**Why bother:** it's the missing rung. With it you own **text → assembly → machine code → executable** with zero external tools. It's also the tool you'll need to write the boot sector in floor 6.

---

### 5.6 — FLOOR 6: My own tiny OS

**Trophy:** 🏆 *"It boots."*
**Estimate:** +4 months → realistically **6–12 months**.
**Needs:** Floors 4 + 5.

#### How far "an OS" can go — the ladder

| Level | What it does | Time (solo) | Reachable? |
|-------|-------------|------------|-----------|
| 1. It boots | powers on, runs my code, prints text | a weekend | yes |
| **2. It's usable** | **+ keyboard, memory manager, shell, runs my compiled programs** | **6–12 months** | **← the target** |
| 3. A real small OS | + filesystem, multitasking, process isolation | +1–2 years | yes, but hard |
| 4. "Full OS" like Windows | + GPU/wifi/USB/sound drivers, GUI, apps | decades, with a team | ❌ not solo |

**Why level 4 is off the table, and it isn't skill:** Linux is ~30M lines and most of it is **drivers** — code for one specific wifi chip, one specific GPU. That work is individually easy and **infinite**. Solo projects die on unbounded work, not hard work. This is exactly why hobby OSes live in a **VM**: a VM presents a small, fixed, documented set of fake simple hardware. Four drivers instead of forty thousand.

**Target: level 2 is the finish line. Level 3 is deferred** — if I get to level 2 and still want more, level 3 becomes its own optional floor 6b, never an open-ended extension of floor 6. Decide at the floor 5 → 6 boundary, not before.

**Sub-steps (level 2):**
1. Boot sector — get the machine to run my code at power-on
2. Enter 64-bit mode
3. Screen output (text)
4. Keyboard input
5. Memory manager
6. Load and run a program compiled by my compiler
7. A shell written **in my language**

**Done means:** boot a VM, get a prompt, type a command, my language runs it.

**Advantage you already have:** you've written kernel-mode Windows code for Aegis. You already understand privilege levels, drivers, and why a bug here is a machine that won't start. Most people hit floor 6 with zero kernel experience.

---

### 5.7 — FLOOR 7: My own CPU

**Trophy:** 🏆 *"My language runs on silicon I designed."*
**Estimate:** +4 months → realistically **8–14 months**.
**Needs:** Floors 4 + 5. (Floor 6 is optional but natural.)

**Sub-steps:**
1. Design the instruction set (what instructions exist at all)
2. Write a **simulator** in software — a program that pretends to be the CPU
3. Retarget my compiler to emit *my* instructions instead of x86
4. Run a program of mine on the simulated CPU
5. Describe the CPU in Verilog/VHDL
6. Load it onto a real **FPGA** — actual physical hardware
7. Run my program on my chip

**Reference:** nand2tetris is the guided path for steps 1–4. Don't reinvent the curriculum, follow it and swap in your own language.

**Cost note:** step 6 needs an FPGA board — real money, roughly $100–400. It's the only floor with a hardware cost. Steps 1–5 are free and are ~80% of the learning.

---

### 5.8 — FLOOR 8: Cross-device — one program everywhere

**Estimate:** +3 months → realistically **4–8 months**.
**Needs:** Floor 4 (own codegen), because you need to emit for multiple targets.

The weakest-defined floor. Currently means: one program in my language runs on PC, phone, and server as one system.

**Blunt assessment:** this is the floor most likely to get cut or replaced. It's the vaguest, and it depends on ecosystem work (mobile toolchains, app signing) that has nothing to do with language design. **Leave it undefined until floor 4 is done** — by then you'll know what you actually want.

---

## 6. Dependency map — what genuinely blocks what

```
                        FLOOR 1  (language + interpreter)
                            │
              ┌─────────────┴──────────────┐
              │                            │
          FLOOR 3                      FLOOR 4
        (Aegis wire)                 (own codegen)
              │                            │
          FLOOR 2                  ┌───────┴───────┐
      (AI writes rules)            │               │
                                FLOOR 5         FLOOR 8
                              (assembler)    (cross-device)
                                   │
                           ┌───────┴───────┐
                           │               │
                       FLOOR 6         FLOOR 7
                         (OS)            (CPU)
```

**Reading it:**
- **Floor 1 blocks literally everything.** This is why it's first and why nothing else may start.
- After floor 1 the tower **forks into two independent legs.** UP (3 → 2, the Aegis story) and DOWN (4 → 5 → 6/7, the silicon story). Neither blocks the other — you genuinely get to choose by mood.
- **Floor 2 now sits above floor 3**, not beside it. AI can't generate rules until the rule language exists.
- Floors 6 and 7 are the deep end and both need 4+5 first.
- **The current work (§3.2) does not appear on this map, deliberately.** Type system, speed and
  systems capability all live *inside* the floor-1 box — they make the seed better rather than
  adding a floor. They are also, in practice, prerequisites: floor 6 needs W5's structs, raw
  memory and freestanding output before a line of OS code is worth writing.

---

## 7. The strategic tension nobody warns you about

**Building the bottom floors makes the top floors less useful.**

If your language runs on *your* OS and *your* CPU, it runs on a machine **nobody else has**. Floors 6 and 7 are the most impressive and the least practical, simultaneously.

**The resolution — write this down and don't forget it:**

> Floors 6 and 7 exist to **prove and to learn**, not to deploy.
> **Windows/x86 stays the language's primary target forever.**
> The OS and CPU are a second target, not a replacement.

Concretely: after floor 7, the compiler has **two backends** — real x86 for daily use, my-CPU for the trophy. Never let the trophy break the daily driver.

---

## 8. Risk register — the ways this actually dies

| # | Risk | How likely | The counter |
|---|------|-----------|-------------|
| 1 | **Floor-hopping** — starting floor 4 while 1 is at 60% | **Very high** | The one rule (§1). One floor at a time, each finished. |
| 2 | **Never writing programs in it** — going straight from interpreter to compiler | **High** | Floor 1 step 7 is mandatory. 3 real scripts before any backend work. |
| 3 | **Feature creep in the language** — adding classes/generics/threads because real languages have them | **High** | §4.5 cut list. If it's not needed to self-host, it doesn't get built in floor 1. |
| 4 | **Kernel work on the real machine** — a floor 3 bug bricking the daily driver | Medium | VM-only for all kernel work. Aegis already sets this precedent. |
| 5 | **Rewriting the syntax repeatedly** | Medium | §4 is **locked**. Changes only after floor 1 step 7, informed by real pain. |
| 6 | **Burnout on floor 6/7 length** | Medium | They're last for a reason. By then there are 3+ finished trophies banked. |
| 7 | **Perfectionism on the optimizer** | Low-medium | Slow correct code is a finished floor. Fast code is an optional bonus project. |
| 8 | **Breaking the fixpoint** — a change to `compiler.zl` or any core file that makes the language stop reproducing itself | Medium | `verify.ps1` before every commit (§3.1). Red = fix or revert *that one change*. Never commit past it. |
| 9 | **Backend drift** — five engines, and a feature added to only some of them | **High, and now the live one** | New builtins go into `interp.c` **and** `runtime.c` identically; parity is checked (93 = 93). `run_tests.ps1` asserts three engines agree. The unboxed engines are *allowed* to reject a program, never to answer differently. |
| 10 | **Documentation drift** — `docs/REFERENCE.md` and this file quietly going stale as waves land | **High — already happened twice** (builtin and module counts) | Counts get re-derived from the repo, not copied forward. Every wave ends with a docs pass. |
| 11 | **Optimising the wrong thing** — pouring work into codegen when the time is in the runtime | Medium | `bench/` first, then optimise, then `bench/` again. `b5_string`'s 1.3× is the standing reminder. |

Risk 1 is the killer. Risks 2 and 3 are the ones that feel like progress while actually being avoidance.
Risks 8–10 are the new ones that arrived with a bigger repo and concurrent work — they are all
prevented by the same discipline: one gate, run every time.

---

## 9. Trophy moments — the demos worth having

In the order they arrive:

| # | The demo | Floor | State |
|---|----------|-------|-------|
| 1 | Type a program in my syntax → watch it run | 1 (step 4) | ✅ **banked** |
| 2 | **"This compiler compiled itself."** | 1 (step 9) | ✅ **banked** |
| 4 | `hello.zl` → real `.exe` → double-click → runs | 4 | ✅ **banked** |
| 4b | **"…and there is no C compiler anywhere in that path."** `nvout.exe`, 33 KB, kernel32 only, output byte-identical to the interpreter's | 4 | ✅ **banked** |
| 4c | **"The same program, five engines, identical output."** interpreter · boxed C · unboxed C · own x86-64 · LLVM | 4 | ✅ **banked** |
| 4d | Annotate one hot function → watch it get 30–80× faster, with nothing else in the program changed | W3 | next |
| 3 | Add a new virus detection by writing a rule — no driver rebuild | 3 | not started |
| 3b | **Describe a virus in English → compiler freezes it into kernel-speed code** | 2 | not started |
| 5 | Boot a VM into an OS I wrote, get a prompt | 6 | not started |
| 6 | Run my program on a chip I designed, on a physical board | 7 | not started |

Each of these is independently a "that's genuinely impressive" moment. **Five are already
banked** — the project is past the point where stopping would leave nothing behind. Everything
from here is upside, which is exactly the position §1 was designed to reach.

---

## 10. Decision log

### LOCKED — do not reopen without a real reason

- Systems language, power over safety, total PC control
- Full syntax spec (§4.2, §4.3)
- 11 reserved words (§4.4)
- Built-ins are identifiers, not keywords
- `!` = danger marker only; `not` = negation
- Bootstrap in C, self-host later, delete C
- Start at floor 1, grow both ways
- Windows/x86 is the permanent primary target (§7)
- **Floor 2 = AI compiles English threat descriptions into Aegis rules** (repointed 2026-07-29; sits *above* floor 3, needs it first)
- AI never writes kernel code unreviewed — human approval gate is mandatory (§5.2 step 4)

**Added 2026-07-30 (the speed pivot):**

- **Both backends, not one.** Own x86-64 codegen (`nativegen`/`nativeval`) is kept and must
  keep passing `verify.ps1`; **LLVM (`compilel.c`) is adopted as the speed backend** — the
  same posture §7 defines for the own-CPU target. This resolves the old "LLVM or straight to
  own codegen" question: the answer was *own codegen first, then LLVM alongside it*, and it
  is only a defensible answer because own codegen shipped first.
- **The current objective is speed + C-class capability, on Windows** (§3.2). Deleting C from
  the *source* and the OS/CPU floors are deliberately deprioritised, not cancelled.
- **A type system is the mechanism for speed**, and it is gradual, opt-in, and Neutral by
  construction (§4.7). Boxing is the measured ceiling; nothing else buys the 4–6×.
- **`verify.ps1` is the single gate** and the fixpoint means `gen1 == gen2`, never a stored
  hash (§3.1).
- **`docs/OVERNIGHT_CAMPAIGN.md` owns the wave-by-wave execution plan.** This document owns
  the floors and the reasoning. When they disagree about *what is being worked on right now*,
  the campaign file wins; when they disagree about *what the project is*, this one wins.

**Added 2026-08-02 (optimiser posture + how we measure):**

- **LLVM is the optimiser. Full stop, for now.** 2026-07-30 settled *both backends exist*;
  this settles *where the effort goes*. All optimisation work targets `compilel.c` and lets
  `clang -O2` do the classic passes — constant folding, inlining, register allocation. The
  own-x86-64 backend (`nativegen`) is **maintained, not optimised**: it must keep passing
  `verify.ps1` and keep agreeing with the other engines, and that is all that is asked of it
  until the language itself is finished. Writing our own optimiser is deferred, not cancelled.
  Rationale: `nativegen` with *no optimiser at all* already matches `clang -O2` on the
  benchmark loops (47ms vs 39ms on `b1_fib`), so building our own passes would be polishing
  the part that is not the bottleneck. Get the language right on a borrowed optimiser first.
- **The bottleneck is coverage, not codegen.** The three unboxed engines are fast partly
  because they refuse most of the language — 2 of the 5 benchmarks cannot run on any of them.
  Widening what they compile is worth more than making them faster at what they already do.
- **Benchmarks must compare against OTHER LANGUAGES, not just against our own interpreter.**
  Ratios like "77x the tree-walker" say nothing about whether zl is fast — the tree-walker is
  the slow thing. From now on the harness runs the same program in C (MSVC and clang), Rust,
  Go, Java, C#, JavaScript and Python alongside all five zl engines, on the same machine, and
  reports absolute milliseconds. Zig is the language we most want on that list and is not yet
  installed.
- **Decisions get written down here.** Any judgement call an agent makes that would be
  expensive to reverse belongs in this log, dated, with its reasoning — not left implicit in
  a commit message.
- **LANGUAGE COMPLETENESS BEFORE OPTIMISATION.** Finish the language first — floats, structs,
  maps, tuples, optionals, error handling, sized integers, `switch`, `defer`, closures,
  `comptime`, FFI. Only then return to speed. Three measurements from 2026-08-02 force this
  order and it should not be re-litigated without new ones:
  1. **Speed is already solved where the language reaches.** `compilel` matches `clang -O2` on
     four integer benchmarks (67ms vs 71, 58 vs 61, 33 vs 33, 62 vs 62). There is no
     optimisation work that buys more than zero there.
  2. **Coverage, not speed, is the ceiling.** The fast backends are integer-only — no floats
     (`2.5 * 2.0` silently gave 4), no strings, no lists. Anything real falls back to the
     interpreter, which is 2–5× *slower than CPython*. Widening the fast path is worth
     multiples; optimising it is worth percent.
  3. **Optimising a language whose semantics are still moving is wasted work.** Records, maps
     and a type system all change what the backends must assume. Every pass written before
     them gets rewritten after them.
  The one apparent exception is not one: **unboxing is a language feature, not an
  optimisation** — it requires the type system, so it belongs in this phase despite being
  worth 4–6×. See `docs/design/PLAN_unboxing.md`.
  Measure against **Zig's ~197k lines**, not LLVM's ~30M: that is the honest size of a
  complete self-hosted systems language. zl is at 5,370.

**Added 2026-08-03 (two output paths, no C in either):**

- **THE OUTPUT PIPELINE IS LLVM IR AND x86-64. NOTHING ELSE.** `compilel.c` (LLVM IR →
  clang) and `nativegen.c` (x86-64 PE written directly) are the two engines that matter.
  Note clang here consumes `.ll`, not C — there is **no C compiler in the output path**.
- **`compile.c` and `compilef.c` are ARCHIVED**, along with `runtime.c` which exists to
  serve them. Kept in-tree, kept passing `verify.ps1`, but **not developed**. They were
  stepping stones: `compile.c` proved the tree could be walked into another language,
  `compilef.c` proved unboxing is worth 4–6×. Both proofs are banked. Maintaining five
  engines costs five times the parity work for two engines' worth of value.
- **THE FIRST PRIORITY IS COVERAGE, NOT SPEED.** Both fast backends must compile the
  **whole language**, not integers. Measured 2026-08-03, against ~20 AST node types:
  - `compilel.c` (448 lines) handles 14 — missing `N_STRING`, `N_LIST`, `N_INDEX`,
    `N_FOR`, `N_FN`, `N_TERNARY`, `N_MEMBER`, `N_DANGER`
  - `nativegen.c` (509 lines) handles 10 — missing those eight plus `N_BLOCK`, `N_BOOL`,
    `N_BREAK`, `N_CONTINUE`
  - Neither supports floats: `2.5 * 2.0` gives `4`, `7 / 3` gives `2`, both silently
  The foundation is already there — **`nativert.c` is 1,534 lines**, larger than
  `runtime.c`, and it is the C-free runtime the native path already links against.
- **CONSEQUENCE — the fixpoint must move off the C path.** `verify.ps1` today proves
  self-hosting *through the C backend*: `compiler.zl` emits C, `cl` compiles it, the
  result recompiles `compiler.zl`. With the C backend archived, that proof has to run
  through LLVM or native instead. This is not speculative — `nativeval.c` already
  compiles `compiler.zl` to `nvout.exe` (kernel32-only, no libc) which recompiles
  `compiler.zl` byte-identically. The native self-host exists; the gate has to be pointed
  at it.
- **The interpreter stays.** It is not an output path — it is the reference every other
  engine is checked against, and the only engine that runs the whole language today. Its
  memory leak (61 mallocs to 15 frees) is now a priority defect, not a Phase 5 item: on
  2026-08-03 a `range()` repro drove `interp.exe` to **51.8 GB** and Windows logged three
  low-virtual-memory events, taking down DWM, Explorer and the editor with it.

**Added 2026-08-03 (the scoping decision — the root of the dependency tree, now settled):**

- **THE GLOBAL ASSIGNMENT RULE IS CHANGING: assignment inside a function now BINDS LOCALLY;
  an explicit `global name` declaration opts a function into writing the global.** This is
  Option 2 in `docs/design/design_scoping_decision.md`, adopted 2026-08-03. It was the single
  highest-leverage decision on the board because it gates BOTH the type system (per-function
  inference is unsound while a plain assignment can write a differently-typed global) and
  closures (a closure cannot know if it captures a local or a global). It costs zero to decide
  and unblocks the two biggest non-FFI features.
- **Why Option 2 over keeping the rule:** the engines ALREADY disagree — `compilel.c` (the
  designated LLVM speed backend) and `nativegen.c` already give every in-function assignment
  its own slot, i.e. they already implement Option 2, and `verify.ps1` never noticed. Keeping
  the old rule would mean PESSIMISING the LLVM backend — pinning every written-through global
  to memory (no mem2reg, because a call might read it), and the parser cursor in a self-hosted
  `compiler.zl` is the hottest variable in the program. So the choice was never "change the
  language or not" — it was "which of the two semantics already in the tree wins," and the
  fast, correct one wins. Option 2 also FIXES a live footgun: 16 stdlib files hand-prefix their
  locals (`pc_path`, `rp_path`) purely to dodge accidental global collisions; two latent bugs
  (`ansi.zl`, `hash.zl`) become impossible.
- **Cost, measured:** 59 assignment sites in 10 of 96 stdlib files, 19 sites in `compiler.zl`
  (8 `global` declarations across 5 functions), plus tests/examples. Many "deliberate" globals
  are really multiple-return-values faked through globals (raytracer's `hit_t`/`hit_id`) and
  would vanish once tuples/records land — the heterogeneity finding again.
- **The one real risk is the fixpoint**, because `compiler.zl` must learn to PARSE `global`
  while remaining byte-identical through the gate. Handled by the two-phase landing in the
  design doc §7: teach every engine to parse-and-ignore `global` first (a no-op that keeps the
  gate green), then flip `env_assign` to local-by-default and add the declarations in the same
  commit. Do NOT attempt it in one step.

### OPEN — decide later, at the right moment

| Decision | Decide when |
|----------|------------|
| **Language name** | anytime — renaming is find-replace, costs nothing |
| **File extension** (currently `.zl`, placeholder) | with the name |
| ~~**LLVM or straight to own codegen**~~ | **resolved 2026-07-30 → both, see LOCKED above** |
| **Hex `0x4000`, durations `30d`** | when the parser needs them (string escapes `\n` — done) |
| **Floor 8's actual definition** | after floor 4 → **now due**, floor 4's headline is done |
| **Whether to push the OS to level 3** (filesystem + multitasking, §5.6) | at the floor 5 → 6 boundary — deliberately NOT now, it's years out |
| ~~**Optimizer: build one or not**~~ | **resolved 2026-07-30 → yes, W4, and it must be benchmark-driven** |
| **Ints vs doubles** — whether zl gets a real integer type or stays all-double | forced by W3 stage 3; see `docs/design/design_floats.md` |
| **When the OS/CPU floors resume** | after W5 (systems capability). They need structs, raw memory and freestanding output anyway, which is exactly what W5 builds. |

---

## 11. Glossary

| Term | Plain meaning |
|------|--------------|
| **Lexer** | Chops source text into words/tokens. Doesn't judge meaning. |
| **Parser** | Builds a tree from tokens. Decides what groups with what. |
| **AST** | The tree the parser builds. "Abstract Syntax Tree." |
| **Interpreter** | Walks the tree and does it, right now. |
| **Compiler** | Turns the tree into machine code to run later. |
| **Backend / codegen** | The part that emits the actual machine code. |
| **Self-hosting** | The compiler is written in the language it compiles. The trophy. |
| **Bootstrap** | Using another language to build v1, then replacing it. |
| **Fixpoint** | `f(f(x)) == f(x)`. The compiler compiles itself, and compiling itself *again* gives the same bytes. Compare gen1 to gen2 — never to a hash you wrote down earlier. |
| **LLVM** | A ready-made backend. Originally planned as a crutch to delete at floor 4; in fact floor 4 shipped first, and LLVM was then adopted *alongside* own codegen as the speed backend (§5.4a). |
| **Boxed value** | Every value carries a runtime type tag and lives behind a pointer, so `1 + 2` costs a dispatch and an allocation. What zl does today. |
| **Unboxing** | Storing a value as the bare machine representation (`i64`, `double`) because its type is known. Measured at 4–6× on the same optimised C. What the type system is *for*. |
| **Gradual typing** | Annotations are optional; anything unannotated keeps the old boxed behaviour exactly. Lets typed and untyped code mix with no rewrite. |
| **PE** | The Windows `.exe` file format. |
| **FPGA** | A chip you can rewire in software. How you get real hardware without a factory. |
| **Kernel mode** | Full-privilege code. Where Aegis lives. A bug = BSOD. |

---

## 12. Next actions

### The original five — all done (kept as the record)

1. ✅ **Build the parser.** tokens → tree, precedence proven, parses all of hello.zl.
2. ✅ **Build the tree-walking interpreter** — the moment the language became real.
3. ✅ **Add the core language** — variables, `if`, `for`, `while`, `fn`, lists, strings, recursion.
4. ✅ **Add the PC-control built-ins** — `read`/`write`/`dir`/`procs` are real; the destructive ones stay simulated until VM-tested.
5. ✅ **Write real programs in it** — `count.zl`, `search.zl`, `report.zl`, then 64 stdlib modules and, ultimately, the compiler itself. The design pain this exposed is documented in §5.1 step 7 and the README.

### The current five (2026-07-31)

1. **Finish W2 — syntax and surface.** Every item touches lexer + parser + interp + compile **and** must be mirrored in `compiler.zl` or the fixpoint dies. One feature at a time, `verify.ps1` between each. ← *immediate next*
2. **W3 stage 1 — annotations parse and nothing reads them.** The highest-value, lowest-risk slice of the type system: provably inert, gated by the skip-neutrality check over all 64 stdlib modules.
3. **W3 stages 2–3 — the type pass, then unboxed `int`/`bool` through `compilel.c`.** Record the annotated-`fib(30)`-vs-boxed-C number in the commit message. This is where the measured 4–6× gets collected.
4. **W4 — speed, benchmark-driven.** Optimise the **runtime**, not only codegen (`b5_string` is the proof). Target: within 2× of C on the integer benchmarks.
5. **W5 — systems capability.** Raw memory, structs with a fixed layout, freestanding/no-CRT output, a real FFI. This is what "as capable as C" actually means, and it is also the prerequisite for resuming Floors 6 and 7.

Everything past action 5 is a different year. Ignore it.

**Standing rules while any of this is in flight:** `verify.ps1` green before every commit; new
builtins land in `interp.c` and `runtime.c` together; `docs/REFERENCE.md` gets updated in the
same wave that changes it; never rebuild `interp.exe` while another agent is running it.

---

*Estimates are guesses and are optimistic. The plan is the map, not the schedule.*
