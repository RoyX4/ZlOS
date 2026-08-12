# zl Overnight Campaign

Standing plan for continuous autonomous waves. Zac is asleep; the loop does not
stop and does not ask. Read this file at the start of every iteration — it is the
memory that survives context resets.

**Objective:** make zl as capable as C/C++, and fast.

---

## The two rules that outrank everything

1. **`.\verify.ps1` must be GREEN before any commit.**
   It checks the self-hosting fixpoint (`gen1.c == gen2.c`) and 3-engine agreement (6/6).
   Red means the change broke the language — fix it or revert that one change.
   The hash is ALLOWED to change when `compiler.zl` changes; `gen1` and `gen2` just
   have to match each other. `f(f(x)) == f(x)` is the property, not a frozen constant.

2. **Disjoint file ownership across concurrent agents.**
   Two agents editing one file corrupts both. Every wave assigns each agent files
   nobody else in that wave may touch. Core-language work (`lexer.c` `parser.c`
   `interp.c` `compile.c` `runtime.c` `compiler.zl`) runs **serially, alone**, because
   it rebuilds `interp.exe` and owns `verify.ps1`.

Corollaries: new builtins go into `interp.c` AND `runtime.c` identically (parity is
load-bearing). Never `git push` — local commits only. Never rebuild `interp.exe`
while other agents are running it.

---

## Iteration procedure

1. Read this file. Check the running workflow / last wave's results.
2. `git status` — see what the agents actually left behind.
3. Run `.\verify.ps1`. Green → commit the wave. Red → bisect, revert the offender, commit the rest.
4. Update the wave table below (mark done, record what shipped).
5. Launch the next wave as a new multi-agent workflow.
6. Schedule the next wake-up. Never stop.

Partial credit is fine: commit the agents that succeeded, re-queue the ones that failed
into the next wave rather than blocking on them.

---

## Wave plan

Later waves depend on earlier ones; do not reorder without reason.

### W1 — foundation (launched)
LLVM control flow (`if`/`while`/`break`/`continue`/functions/strings) · 10 stdlib modules
(bitops, sortx, combinat, deque, heapq, pathx, csvw, strsearch, graphx, memo) ·
gradual-type-system design doc · benchmark harness · ~30 new builtins incl. bitwise,
`now()`, `exit()`, `env()`.

### W2 — syntax and surface
The language should stop feeling small.
- `elif`, `do/while`, `for i = a to b`, `switch`/`match` with fallthrough rules
- string interpolation `"x is {x}"`
- multiple return values / destructuring `a, b = f()`
- `const`, block-scoped locals, shadowing rules
- comments `#` → also support `//` and `/* */`
- `+=` on strings and lists, `in` as an expression operator, ternary `a ? b : c`
- escape sequences in string literals (`\t`, `\\`, `\"`, `\xNN`)
Every one of these touches lexer+parser+interp+compile AND must be mirrored in
`compiler.zl` or the fixpoint dies. Serial, one feature at a time, verify between each.

### W3 — the type system (from the W1 design doc)
Gradual. `x: int`, `fn f(a: int) -> int`. Annotated code unboxes; unannotated code keeps
today's boxed semantics so all 54+ stdlib modules keep working untouched. Stages:
(a) lexer/parser accept and store annotations, everything else ignores them, fixpoint holds;
(b) a type table + local inference inside annotated bodies;
(c) `compilel.c` consumes the types and emits genuinely unboxed IR;
(d) the boxing boundary — unboxed↔Value at every call, list store, and builtin edge.

### W4 — speed
Benchmark first, optimise what the numbers say, benchmark again. Candidates:
constant folding + dead-code elimination in the AST; interned strings; avoiding the
copy-on-write list clone on every write; a real `Value` union instead of the current
representation; `clang -O3`/LTO on the LLVM path; inline caching for builtin dispatch.
Target: within 2× of C on the integer benchmarks.

### W5 — systems capability (this is what "as good as C" actually means)
- raw memory: `alloc(n)`, `free(p)`, `peek(p)`, `poke(p,v)`, `peek8/poke8`, pointer arithmetic
- structs / records with a fixed memory layout, and fixed-size typed arrays
- direct Win32 syscalls without libc (`nativert.c` already proves kernel32-only output works)
- freestanding output: no CRT, custom entry point, `-nostdlib`
- inline machine code / an `asm` escape hatch
- a real FFI so zl can call any DLL

### W6 — kernel and driver
Freestanding + no-libc + structs + raw memory make this possible, not before.
A bootable stub, protected/long-mode entry, interrupt table, a serial-port driver
written in zl. Cross-reference `nativegen.c` and `pe_min.zl` — hand-built PE already works.

### W7 — the OS floor
MASTER_PLAN Floor 5. Kernel skeleton, memory manager, task switch, a shell in zl.

### W8 — game system
Win32 window + GDI or a framebuffer, an input loop, a sprite blitter, a fixed-timestep
game loop, and one actual playable game written in zl. Good end-to-end proof: it needs
speed, structs, raw memory, and FFI all working together.

### Continuous, every wave
More stdlib modules. More builtins. More tests. Keep `docs/REFERENCE.md` accurate —
it drifts every single wave (it has already been wrong about module and builtin counts once).

---

## Log

| Wave | Shipped | Verify |
|------|---------|--------|
| W0 | 5 stdlib libs (randlib, jsonw, vec, set, fmt) + `docs/REFERENCE.md` + `verify.ps1` | GREEN |
| W1 | 14/14 agents, 4057 insertions. LLVM control flow (compilel.c 161→347: if/else, while, break/continue, functions w/ alloca locals, strings — all 6 run_tests match interp byte-for-byte). Builtins 66→93 incl. bitwise, parity verified 93=93. Stdlib 54→64. `bench/` harness. Type-system design doc. | GREEN |
| W2 | launched — 12 libraries in parallel, then 5 serial syntax stages | pending |

### Measured, W1 — the numbers that steer W3/W4

`bench/README.md`, best of 5: unboxed engines **30–80×** the interpreter; boxed C only
**7–13×**. Same generated-then-`-O2`'d C, so that 4–6× gap is purely tagged values +
malloc-per-op. **That is the type system's payoff, measured.**

Second finding, equally important: `b5_string` is only **1.3×** faster compiled, because
both engines spend their time inside the same `runtime.c` string routines. Compiling
control flow is worthless when the work is in the runtime — so W4 must optimise the
**runtime**, not only codegen. Coverage is the other gap: 2 of 5 benchmarks cannot run on
any fast engine at all.
