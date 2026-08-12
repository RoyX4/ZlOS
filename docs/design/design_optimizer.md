# Design Proposal: An Optimizer for the x86-64 Backend

**Status:** proposal / not built
**Author:** design pass, 2026-07-29
**Scope:** four optimization passes bolted onto the native backend (`nativegen.c`, Floor 4 §5.4) so the machine code it emits stops being naive stack-machine churn and starts approaching what a C++ compiler at `-O1` would produce for the same integer subset. Adds one AST pass and one small instruction-level IR between the AST walk and the byte encoder. Changes no language semantics, no PE writer, no runtime. Every existing `.zl` program keeps producing a correct `native.exe`.

---

## 0. The one-line pitch

> Keep values in registers instead of pushing and popping the stack for every operand, fold arithmetic the compiler can do at build time, clean up the obvious junk in the byte stream, and delete code that can never run — so `fib(30)` runs at C-ish speed instead of paying for a `mov imm64 / push / pop` on every leaf.

The §5.4 "honest note" says the plan is *correctness first, optimizer later, optional.* This is that later. It is opt-in at the tool level (a `-O` flag), so the unoptimized path stays exactly as it is today for debugging and for proving correctness.

---

## 1. Motivation

### 1.1 The code we emit today is embarrassingly slow — on purpose

`nativegen.c` is a textbook **stack machine** (its own header comment says so). Every binary operation does this (`gen_expr`, `N_BINARY`, lines 166–172):

```
gen_expr(n->a);          // left  -> rax
b(0x50);                 // push rax
gen_expr(n->b);          // right -> rax
mov rcx, rax             // 48 89 C1
b(0x58);                 // pop rax
emit_binop(n->text);     // e.g. add rax,rcx  (48 01 C8)
```

And every integer literal is a full 10-byte `movabs` (`N_NUMBER`, line 143):

```
48 B8 <imm64>            // mov rax, imm64
```

So the expression `2 * 3 + 4` — three constants, two operators — compiles to roughly **40+ bytes** of `movabs / push / mov / pop / op` that computes a value (`10`) the compiler already knows. Worse, every variable read is a fresh memory load (`N_IDENT`, line 147: `mov rax,[rbp+disp32]`) even when the same variable was loaded into a register two instructions ago, and every local write goes straight back to the frame. A hot loop like `fib` or `sum 1..n` pays this tax on every single leaf, every iteration.

That is the gap between zl-native and C++: not instruction selection (our `add`/`imul`/`idiv` are already the right opcodes) but the **stack-machine calling discipline around them** and the **absence of any constant evaluation**.

### 1.2 Why now, and why it's cheap

The integer subset is *frozen and verified* (§5.4 sub-steps 3–5: factorial, fib, comparisons, print all pass as native `.exe`). That is the ideal moment to optimize: a fixed, small, well-tested input language and a regression harness (`run_tests.ps1`, exit-code assertions) already exist. An optimizer added now can be validated by "same exit code / same printed output, fewer bytes, faster wall-clock" against that harness. No moving target.

### 1.3 What "C++ speeds" honestly means here

We will **not** match `-O2` (no loop unrolling, no vectorization, no global register allocation across the whole function, no strength-reduction beyond the trivial). The realistic target is **`-O1`-class code for straight-line integer arithmetic and simple loops**: constants precomputed, operands kept in registers within a basic block, no dead pushes, no unreachable tails. For `fib`/`sum`-shaped programs that closes most of the measurable gap. Setting the bar there keeps this a bounded project (risk register item #7: "perfectionism on the optimizer") instead of an open-ended one.

---

## 2. The one enabling decision: a tiny instruction IR

The single most important design choice. **Do not peephole raw bytes.** The current backend writes final machine bytes directly into `code[]` and fixes up jump/call displacements by absolute offset (`patch4`, the `fixups`/`pfix`/`psfix`/`sfix` tables). Trying to delete or rewrite bytes *after* the fact would invalidate every recorded offset — a nightmare.

Instead, insert a **thin instruction list** between the AST walk and the byte encoder:

```c
typedef enum { M_MOV_RI, M_MOV_RR, M_LOAD, M_STORE, M_PUSH, M_POP,
               M_ADD, M_SUB, M_IMUL, M_IDIV, M_CMP, M_SETCC, M_MOVZX,
               M_JMP, M_JE, M_CALL, M_LABEL, M_RET, M_RAW, ... } MOp;

typedef struct {
    MOp op;
    int rd, rs;        // register operands (enum: RAX, RCX, RBX, ...)
    long long imm;     // immediate / displacement
    int label;         // for jumps/calls/labels: a label id, NOT a byte offset
    unsigned char cc;  // condition code for SETCC/Jcc
} MInstr;
```

`gen_expr`/`gen_stmt` **append `MInstr`s** instead of calling `b()`/`bytes()` directly. Jumps and calls target **label ids**, not offsets. A final `assemble()` pass walks the `MInstr` list once, emits the exact same bytes the current `emit_*` helpers already know how to produce, and resolves labels to displacements at the end (the backpatching logic we already have, just keyed on labels instead of hand-tracked positions).

This refactor is **behavior-preserving on its own** — with no optimization passes enabled it emits byte-for-byte what we emit today — which makes it safe to land first and verify against `run_tests.ps1` before any real optimization exists. And it is the substrate that makes passes 2, 3, and 4 each a ~50-line list-to-list transform instead of a byte-surgery horror.

The hand-assembled blobs (`emit_print_int`, `emit_print_str`, lines 296–365) stay exactly as they are — they enter the IR as a single opaque `M_RAW` node that the optimizer never looks inside. Same for the PE writer (`write_pe`) and the import table. **Nothing below the IR changes.**

---

## 3. The four passes, concretely

Ordered by leverage-per-line. Each is independent; each can ship and be verified on its own.

### Stage 1 — Constant folding (AST pass, no IR needed)

The cheapest big win, and the only pass that can land *before* the IR refactor.

Add `Node *fold(Node *n)` run once on `prog` right after `parse()` in `main` (line 443). It walks the AST bottom-up and replaces evaluable subtrees with a single `N_NUMBER`:

- `N_BINARY` with both children `N_NUMBER` → compute in `long long`, replace with the literal. Covers `+ - * / % == != < > <= >= and or`.
- `N_UNARY` (`-`, `not`) over an `N_NUMBER` → fold.
- **Guards:** do **not** fold `/` or `%` when the divisor is `0` (leave the node alone so runtime behavior is unchanged — don't invent a compile error the language never had). Fold in 64-bit two's-complement to match the CPU's `imul`/`idiv` exactly, so folded and unfolded results are bit-identical.
- **Branch cleanup fallout:** once folded, `if (0)` / `if (1)` / `while (0)` have a constant condition. Record that — Stage 4 uses it to drop the dead arm.

Effect: `2*3+4` becomes `mov rax,10`. Any all-constant expression collapses to one `movabs`. Constant loop bounds and flags evaluate at build time. This is pure AST-to-AST, ~40 lines, testable in isolation ("folded program prints the same thing").

### Stage 2 — Peephole (IR pass)

A fixed-point sweep over the `MInstr` list matching short windows and rewriting them. Run it repeatedly until no rule fires (2–3 passes converge in practice). The high-value rules, all of which target the stack-machine churn from §1.1:

1. **`push R` / `pop R2` with nothing clobbering R in between → `mov R2, R`.** This is *the* rule. Almost every binary op in the current output is `... push rax ... mov rcx,rax ; pop rax ; op`. The push/pop pair collapses to a register move, killing two memory touches per operator.
2. **`mov R, imm` then `push R` … `pop R2` → `mov R2, imm`** (constant folded into the consumer, push/pop gone entirely).
3. **`mov R, R` (same reg) → delete.** Falls out of rules 1–2 and of the accumulator scheme in Stage 3.
4. **Redundant reload:** `store [rbp+d], R` immediately followed by `load R2, [rbp+d]` → keep the store, replace the load with `mov R2, R`. (A just-written value is still in a register.)
5. **`mov rax, imm` immediately overwritten by another `mov rax, ...` with no use between → drop the first.**
6. **Displacement shrink (encoding, applied in `assemble`):** `[rbp+disp]` with `-128 ≤ disp ≤ 127` uses the `disp8` form (`48 8B 45 xx`, 4 bytes) instead of the current unconditional `disp32` (`48 8B 85 xx xx xx xx`, 7 bytes, line 147). Most frames are tiny, so this shrinks nearly every variable access by 3 bytes.
7. **Strength reduction (optional):** `imul R, 2^k` → `shl R, k`. Small, safe, cheap.

Each rule is a pattern-match on 1–3 adjacent `MInstr`s. Verification: byte count drops, exit codes/output unchanged across the whole test suite.

### Stage 3 — Keep values in registers (the real speed)

Replace the strict "everything through rax + the machine stack" discipline with a **small register-backed expression evaluator** plus a **per-basic-block value cache**. Two parts:

**(a) Expression temps in registers.** Instead of `push rax` to save the left operand while computing the right, allocate the left operand into a scratch register from a small pool. A simple, robust scheme: keep the top of the expression stack in `rax` and the next-down operands in `rbx, r10, r11, r12, r13, r14, r15` (a tiny stack of registers). Only when the expression nests deeper than the pool do we spill to the real machine stack (fall back to today's `push`/`pop`). For the integer subset, expression trees are shallow — spills will be rare, so most operators become pure `reg,reg` arithmetic with **zero** memory traffic.

**(b) Local-variable caching within a basic block.** Track "which register currently holds the live value of local *v*." On `N_IDENT`, if *v* is already in a register, reuse it instead of re-emitting the `mov rax,[rbp+d]` load. On assignment, update the cache. **Invalidate the entire cache at every basic-block boundary** — i.e. at any label, jump, conditional branch, function call, or the `print`/`exit`/user-call sites — because a value cached in a caller-scratch register does not survive a `call`, and control-flow merges make cross-block claims unsound. Block-local only; no dataflow across blocks. This is the safe 80% and it kills the "reload the loop counter every iteration" cost.

**Convention bookkeeping (must-do):** today `gen_function` (line 277) and `main`'s entry only `push rbp`. If the allocator uses `rbx, r12–r15` (the registers we want *because* they'd normally survive across our own `call`s), the prologue must `push` and the epilogue (`emit_fn_epilogue`, line 189) must `pop` exactly the callee-saved registers a function actually used, or a callee will corrupt its caller's cached values. The optimizer therefore emits the save/restore set *after* it knows which registers a function touched — a second reason the IR must exist before Stage 3 (we can't know the reg set until the body is lowered). `print_int`/`print_str` clobber `rax,rcx,rdx,r8,r9`; treat every `call` as clobbering exactly those and preserving `rbx,r12–r15,rbp,rsi,rdi` — so the cache must only ever park across-call-live values in the preserved set.

This is the largest and riskiest pass. It is where the C++-ish speedup actually comes from, and it is why Stages 1–2 (which are cheap and safe) go first.

### Stage 4 — Dead-code elimination

Two flavors, both small once the IR exists:

1. **Unreachable code.** After an unconditional `M_JMP`, `M_RET`, or the `exit`/`ExitProcess` sequence, drop every instruction up to the next `M_LABEL` — it can never execute. This also sweeps up the arms that Stage 1's constant-condition analysis marked dead (`if (0) {…}` → the whole then-block; `if (1)` → drop the else and the branch test entirely; `while (0)` → drop the loop body and the back-edge). Note `gen_function` already emits a "safety return" after the body (line 291); when the body always returns, that tail becomes unreachable and this pass removes it.
2. **Dead stores (AST-level liveness, optional).** A `store` to local *v* whose value is never read before the next store to *v* or end of function is dead and can be dropped. This needs a tiny per-function liveness scan over locals. Lower priority than #1 (unreachable code is more common in practice), so it's the last thing to build.

Verification: unchanged output, strictly fewer instructions, plus a targeted test with a provably-dead branch (`if (1) { print(1) } else { print(2) }` must emit no code for `print(2)`).

---

## 4. Staged implementation sketch

Each stage is independently shippable and independently verifiable against `run_tests.ps1`. Gate every stage on "all existing tests produce identical exit codes and identical stdout."

| Stage | What | Touches | Success check |
|------|------|---------|---------------|
| **0** | `-O` flag plumbing + benchmark baseline | `main` (argv), a new `bench.ps1` timing `fib(30)`, `sum 1..1e7` | baseline byte-count + wall-clock recorded for every test; `-O` off = today's bytes exactly |
| **1** | Constant folding | new `fold()` in the AST, called after `parse()` (line 443) | folded programs print/return identically; all-constant exprs emit one `movabs` |
| **2** | Instruction IR refactor (behavior-preserving) | `gen_expr`/`gen_stmt`/`emit_*` append `MInstr`; new `assemble()`; labels replace offset fixups | **byte-for-byte identical** to pre-refactor output with no passes on |
| **3** | Peephole pass over the IR | new `peephole()` list→list, run to fixed point | same output, fewer bytes; push/pop pairs gone from disassembly |
| **4** | Register value-keeping (expr temps + block-local var cache + callee-save bookkeeping) | expr lowering, prologue/epilogue reg-save set | same output; measurable wall-clock drop on `fib`/`sum`; no `mov rax,[rbp..]` reload of a just-loaded var |
| **5** | Dead-code elimination | new `dce()` over the IR + constant-condition arm drop | same output; dead branch emits nothing; unreachable tails gone |

**Ordering rationale:** Stage 1 ships value with zero infrastructure. Stage 2 is the pivot — it must land *before* 3/4/5 and must be proven a no-op first. Stages 3–5 then compose freely on the IR. Do **not** attempt 3 before 2; register allocation over raw bytes with absolute-offset backpatching is the trap this whole design exists to avoid.

**Pass pipeline once complete:**

```
parse → fold (Stage 1)
      → lower to MInstr list
      → peephole (Stage 3, to fixed point)
      → regalloc / value-cache (Stage 4)
      → dce (Stage 5)
      → peephole again (cleanup after dce)
      → assemble to code[]  →  write_pe
```

---

## 5. Risks and honest limits

- **Scope creep is the real danger** (MASTER_PLAN risk #7). The table above is the whole project. No SSA, no global allocator, no inlining, no loop transforms. If Stages 1–4 land, we are done and the floor is "fast enough."
- **The register allocator is where bugs live.** A cache invalidation missed at a branch merge or across a `call` = silently wrong results. Mitigation: block-local only, invalidate aggressively, and lean on the exit-code test suite (a wrong register value almost always changes an observable result). Keep `-O` off as the always-available correct oracle to diff against.
- **The boxed-value backend (§5.4 step 6.7) doesn't exist yet.** This optimizer targets the *current integer subset backend*. When the no-C boxed backend lands, the IR and passes port to it, but tag-dispatch calls (`zl_add` etc.) change the clobber model — revisit Stage 3's call-clobber set then. Designing the IR now, cleanly, is what makes that port cheap.
- **Interaction with `print`/`exit`.** These are `M_RAW`/import-call sites the optimizer treats as full clobbers and basic-block boundaries. As long as that boundary is respected, the hand-assembled blobs and the PE layout are untouched — the `idata_rva == 0x2000` size assumption (line 431) still holds because optimized code is *smaller*, not larger.

---

## 6. Why this is the right shape

The design's spine is one idea: **introduce a real instruction list, then every classic optimization is a small list-to-list pass.** Constant folding rides in front of it on the AST for free; peephole, register-keeping, and DCE ride on top of it. It reuses every byte-encoding helper and the entire PE writer unchanged, it is opt-in behind `-O`, it is verifiable at every stage against a test suite that already exists, and it stops precisely at `-O1`-class output — the honest definition of "toward C++ speeds" for an integer subset, without turning the optimizer into a second compiler project.
