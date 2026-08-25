# zl — HANDOFF (read this first in a new chat)

Last updated 2026-08-03. This is the "start a new conversation and know everything" file. If you are
a fresh session picking up zl, read this top to bottom, then `ULTIMATE_PLAN.md`.

---

## 1. WHAT ZL IS

A self-hosting systems programming language, built from scratch by Zac. Repo: `C:\Users\royx4\my-stack`
(Windows, MSVC + clang + PowerShell). The engine is ~11,000 lines of C. It has FIVE ways to run a
program, and two of them are 100% hand-written (no borrowed compiler):

1. `interp.exe` — tree-walking interpreter. The REFERENCE: whatever it does is correct. Runs the whole language.
2. `compile.exe` — emits C (ARCHIVED, don't develop).
3. `compilef.exe` — emits unboxed C (ARCHIVED).
4. `compilel.exe` — emits **LLVM IR** → clang. The SPEED backend. Matches `clang -O2` on integers.
5. `nativegen.exe` — emits **x86-64 machine code directly** into a .exe. 100% hand-written, no C
   compiler, ~1.5 KB output. This is the "own compiler top to bottom" one.

Plus `compiler.zl` (716 lines) — the compiler written IN zl, which compiles itself byte-identically.
That is the self-hosting PROOF. The C engine is the real day-to-day compiler; `compiler.zl` is not
yet the one you run.

**Two "no C" ideas, do not confuse them:** (a) no C in the OUTPUT — DONE (Floor 4: the LLVM and x64
backends produce programs that never touch a C compiler). (b) no C in the SOURCE — NOT done, and
deliberately LAST (Phase 12): the engine is C until zl is feature-complete enough to rewrite it in
itself pleasantly.

---

## 2. HOW TO BUILD & TEST (with the real gotchas)

- **Build:** `& cmd.exe /c "C:\Users\royx4\my-stack\build.bat"` — takes ~2.6s.
  GOTCHA: `cmd /c build.bat` after a PowerShell `Set-Location` SILENTLY DOES NOTHING (cmd doesn't
  inherit the dir), prints "not recognized", exits 1, and leaves the OLD binary. Always verify with
  `(Get-Item interp.exe).LastWriteTime` after building.
- **The gate:** `.\verify.ps1` must print `VERIFY: GREEN`. It checks the self-hosting fixpoint
  (`compiler.zl` compiles itself byte-identically, via the C backend) + 3-engine agreement. The
  invariant is `f(f(x)) == f(x)` — compare gen1 to gen2, NEVER a stored hash (the hash legitimately
  changes when `compiler.zl` changes; it has been `ECE14032…` for a while).
- **Tests:** 8 suites in `tests/`, 2,107 checks, currently 0 failing. Run `.\interp.exe tests\<name>.zl`.
  Count PASS/FAIL with a SUBSTRING match — the suites don't anchor those tokens at line start.
- **clang:** `"C:\Program Files\LLVM\bin\clang.exe"` (not on PATH). Compile an LLVM program:
  `.\compilel.exe f.zl` writes `out.ll`, then `clang out.ll -O2 -o x.exe`.
- **Test a compiled program:** always diff its output against `.\interp.exe f.zl` BYTE-FOR-BYTE. The
  interpreter is the oracle.

---

## 3. THE RULES (learned the hard way this session)

1. **`verify.ps1` GREEN before any commit.** Non-negotiable.
2. **NEVER `git add -A` while a background workflow owns the repo.** It scoops up the workflow's
   in-progress files. Use explicit paths: `git add stdlib/foo.zl`.
3. **Disjoint file ownership when running parallel work.** The engine core (`lexer.c parser.c
   interp.c runtime.c compilel.c nativegen.c compiler.zl` + `build.bat`/`verify.ps1`) = ONE writer at
   a time. Pure-zl stdlib/examples (new `.zl` files) = safe to build in parallel, tested against a
   FROZEN interpreter snapshot (`_interp_stdlib.exe` or copy `interp.exe` to `_interp_wN.exe`).
4. **interp.c and runtime.c are MIRRORED** for the builtins — a bug fixed in one only is invisible to
   the gate (both engines agree while being wrong). Fix both.
5. **Commit-on-green, even mid-task.** Work in git is durable; a process death loses only the
   completion signal, not committed code. Workflows die often this session — commit early.
6. **Don't force-resume a workflow the runtime says is "still running"** — it's between serial agents
   (rebuilding, ~60s of silence looks like death but isn't). Check the task state, not just timestamps.
7. **`Remove-Item` with wildcards/quoted-paths trips a sandbox guard** — avoid it; use unique temp
   filenames instead of cleaning up.

---

## 4. STATE RIGHT NOW (41 commits this session, f609034 → HEAD)

**DONE:**
- Engine: 24 bugs fixed, `push` O(n²) regression caught (61.8s→1.9s via tip-tracking), 71 broken
  tests → 2,107 passing / 0 failing.
- **LLVM backend brought to the full language:** floats, strings, lists, control-flow, `str()` in
  expressions — all at C parity, all verified byte-identical to the interpreter.
- x64 backend: gained blocks/bools/break/continue + floats (SSE2).
- **13 pure-zl stdlib modules:** iterx (map/filter/reduce), textkit (words/char-tests), jsonkit
  (correct JSON), setkit, dictkit (the map stand-in), mathkit, b64kit, urlkit, csvkit, randkit,
  hashkit (a real CRC32!), argkit. Plus 3 real tools: wordfreq, csvstats, texttools (wc/grep).
- Scoping decision MADE (Option 2), VS Code syntax highlighting extension (installed).
- ~17 planning docs (see §6).

**RUNNING NOW:** the LLVM↔runtime.c bridge workflow (`wf_bac2d9e0-42d`) — a 4-stage serial chain
making all ~93 builtins work in compiled expressions by linking `runtime.c`. Stage 1 (the bridge +
`abs`) is in progress; it has added the `zlx_` helpers to `runtime.c`. Owns `compilel.c` + `runtime.c`
until done. Design + the two traps (link change, symbol collision with compilel's inline `@zl_push`)
are in its prompt.

**KNOWN STILL-BROKEN:** ~33 catalogued engine bugs (`x % 0` crashes the process, `break` escapes a
function into the caller's loop, backend divergences, the LLVM `sdiv` zero-guard); the interpreter
LEAKS memory (61 mallocs / 15 frees — a `range()` repro hit 51.8 GB and crashed the PC on 2026-08-03,
so never run unbounded allocation); `contains` is list-only (use `find` for strings); no multi-line
string literals; `0xFF`/`1e6`/`.5` don't lex.

---

## 5. THE PLAN — with Zac's current focus

Full detail in `ULTIMATE_PLAN.md` (14 phases). Zac's steer as of 2026-08-03: **make the LANGUAGE
great first; keep the AI-in-compiler, the security/capability features, and the OS/CPU/kernel tiers
DEFERRED for a while.** So the near-term order is the pure-language spine, NOT the deep/AI tiers:

- **Phase 1 (running):** finish the LLVM fast path (the bridge).
- **Phase 2 — NEXT — the cheap high-value tier:**
  - **IMPORTS** ← the hinge. Today every program copy-pastes its libraries (all 13 modules are
    inlined into each demo). Imports turns them into a real loadable stdlib. ~300 engine lines.
  - `args` (CLI tools), `sleep` + `kbhit` + VT mode (a playable terminal game), lexer fixes
    (`0xFF`/`1e6`/`.5`), fix `contains`-on-strings + multi-line strings, stack traces in errors, a REPL.
- **Phase 3:** build the scoping decision (Option 2, two-phase) + error handling. Unblocks the type
  system and closures.
- **Phase 4:** structs/records, maps, tuples, optionals, enums (collapses 51 "fake-struct" files to 8).
- **Phase 5:** switch, closures-that-capture, sized integers, constants, destructuring; fix the ~33 bugs.
- **Phase 6:** the gradual type system → unboxing (the measured 4–6× speed win).
- **Phase 7:** FFI → controls Windows (process/registry/memory/CPU/GUI/network/graphics). Zac likes
  this "control the PC" tier — it's fine, it's Ring 3 = FFI + libraries, not the deferred AI/OS stuff.
- **DEFERRED FOR NOW (Zac's call):** Phase 8 comptime + the `ai` keyword (Floor 2), Phase 9
  security-native (capabilities/taint), Phase 11 the kernel `.sys` driver, Phase 12 delete-C-from-source,
  Phase 13 the OS/CPU floors. All designed, all waiting — not now.

**The spine everything reduces to:** imports · closures · maps/structs · errors · sized ints · FFI ·
comptime. **The ordering root:** the scoping decision (Phase 3) gates the type system + closures.

---

## 6. WHERE THE KEYWORDS & SYNTAX LIVE (Zac asks about these a lot)

- **The 15 current keywords** (`lexer.c:48-53`): `if elif else for in fn return while not and or
  true false break continue`. That's the ENTIRE reserved vocabulary. Everything else (`print`, `push`,
  `len`, all 93 builtins) is a plain function, not a keyword.
- **`KEYWORDS_MAP.md`** — every Python/C/C++ keyword mapped to zl's plan; the projected FINAL count
  is **~26** keywords for the whole language (vs Python 35, C 44, C++ 97). The design rule: challenge
  every new keyword — can it be a function, a reused word, or a type name first?
- **`docs/design/ZL_1.0_VISION.md`** — proposed SYNTAX for the spine features (imports, records, maps,
  closures, errors, sized ints, FFI) shown as real code, plus the critical-path graph.
- **`docs/design/design_switch_case.md`** (+ `.v2`) — the `switch` syntax (reuses `to`/`in`/`else`,
  one new keyword, desugars to if/elif).
- **`docs/design/BUILTIN_FUNCTIONS.md`** — all 93 builtins by category + what a complete prelude needs.
- **Current surface syntax:** `x = 5` (no keyword for vars), `#` comments, `{}` blocks, `if/elif/else`,
  `while`, `for v in xs`, `for i = a to b step n`, `do-while`, `loop`, `fn name(...) { return }`,
  lists `[1,2,3]`, ternary `a ? b : c`, `in`/`not in`, string escapes, f-strings `f"x is {v}"`.

---

## 7. THE DESIGN DOCS (in docs/design/ unless noted) — every one slots into a phase

- `LANGUAGE_GAP.md`, `GAPS_REALWORLD.md`, `GAPS_REALWORLD_2.md` — what's missing, found by AUDIT and
  by trying to BUILD real programs (the second kind is sharper). Key finds: strings can't hold binary
  (no byte buffer); first-class functions ALREADY WORK (map/filter/reduce buildable now — proven by
  `iterx`); `num()`/`int()` silently return 0 on bad input.
- `C_CPP_PARITY.md`, `FUNCTION_INVENTORY.md`, `TOP_LIBRARIES.md` — every C/C++/Python/Rust/Go/JS
  keyword & function & top-library mapped to zl. Finding: most top libraries are a C lib + a wrapper,
  so FFI (not reimplementation) is the ecosystem path.
- `design_records.md`, `design_maps_v2.md`, `design_nullable.md`, `design_error_handling.md`,
  `design_scoping_decision.md` (DECIDED), `design_type_system.md`, `PLAN_unboxing.md`,
  `design_sized_integers.md`, `design_closures.md`, `design_comptime.md`, `design_ffi_v2.md` —
  HOW each feature gets built.
- `NOVEL_IDEAS.md` — the differentiators (ai keyword, capabilities-as-types, taint types) — DEFERRED.
- `KERNEL_CONTROL_GAPS.md`, `GRAPHICS_PLAN.md`, `REACH_GAPS.md` — the deep tiers (Ring 3 = FFI +
  library; Ring 0 = driver; graphics = BMP-now → window-via-FFI; reach = embedding/ecosystem).
- `STDLIB_PLAN.md` — the 96 modules organised + the broken ones (old json/csv — now replaced by
  jsonkit/csvkit).
- Top-level: `MASTER_PLAN.md` (philosophy + the decision log), `ROADMAP.md` (66 items), `ULTIMATE_PLAN.md`
  (the 14 phases), `bench/README.md` (the measured speed numbers).

---

## 8. THE IMMEDIATE NEXT MOVE

Let the bridge chain finish Phase 1 (it owns `compilel.c`/`runtime.c` right now). The moment it frees
the repo, start **IMPORTS** (Phase 2) — the single highest-leverage move: it turns today's 13
copy-pasted modules into a real standard library and makes every subsequent feature pleasant to test.
Read `design_selfhost_parity.md` and how stdlib modules are used today (textual copy-paste) first.

Zac works in a blunt, simple, example-driven style; asks for popup questions on real forks (not
chat-typed); wants heavy automation; does NOT want machine/RAM/cost caveats raised. Verify by reading
source, don't recite. Keep the AI/OS/CPU tiers off for now — focus on the core language.
