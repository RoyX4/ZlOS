# zl — THE ULTIMATE PLAN

The single phased journey from where zl is now to the end goal, pulling together every planning
document into one ordered build list. Written 2026-08-03.

**The end goal, in one sentence:** a complete, fast, self-hosted systems language — C-class power,
Zig-class capability, a quarter of C++'s keywords, security-native, that controls Windows down to
the kernel, eventually written entirely in itself with no C left.

Each phase lists: **GOAL** (what you can do after it), **BUILD** (the items), **NEEDS** (what it
depends on), **~SIZE** (rough engine lines), **DOCS** (where the design lives).

Legend: ✅ done · 🔄 in progress · ⬜ queued

---

## PHASE 0 — Foundation ✅ (this session)
**GOAL:** a correct engine, a fast compiled path for the core language, a real library layer, and
the whole plan mapped.
- ✅ 24 engine bugs fixed; push O(n²) regression caught (61.8s→1.9s); 2,107 tests green
- ✅ LLVM backend: floats, strings, lists, control-flow, `str()` in expressions — at C parity
- ✅ own x86-64 backend: floats (arithmetic + whole-number print)
- ✅ 13 stdlib modules (iterx, textkit, jsonkit, setkit, dictkit, mathkit, b64kit, urlkit, csvkit,
  randkit, hashkit, argkit) + 3 real tools (wordfreq, csvstats, texttools — wc/grep)
- ✅ scoping decision made (Option 2), keyword map (~26 total), VS Code syntax highlighting
- ✅ ~16 planning docs
**RESULT:** zl computes real things (a CRC32, a CSV stats tool) — but only on the interpreter, and
every program copy-pastes its libraries.

## PHASE 1 — Finish the fast path 🔄 (running now)
**GOAL:** the LLVM backend compiles REAL programs at C speed, not just feature demos.
- 🔄 LLVM ↔ runtime.c bridge so all ~93 builtins work in compiled expressions
- ⬜ (later) the same for the own-x64 backend — deprioritised
**NEEDS:** Phase 0. **~SIZE:** ~200 lines. **DOCS:** in-code; PLAN_unboxing.md.
**RESULT:** `wordfreq`/`csvstats`/`texttools` compile at C speed. zl is usable two ways —
interpreter for iteration, LLVM for speed.

## PHASE 2 — Make it usable ⬜ (the cheap wins — huge value per line)
**GOAL:** zl can write a real CLI tool, a playable terminal game, and a clean script.
- **Imports** — one line instead of copy-paste; 13 modules become a real stdlib. **THE hinge.**
- `args` — read command-line arguments (zl cannot write a CLI tool without it, ~30 lines)
- `sleep` + `kbhit` + VT console mode — a playable terminal snake/pong, no FFI
- lex `0xFF` / `1e6` / `.5` (silently fail today); fix `contains`-on-strings; multi-line strings
- stack traces + line numbers in runtime errors (today: no line, no function)
- a REPL
**NEEDS:** Phase 0. **~SIZE:** ~800 lines total, tiny each. **DOCS:** LANGUAGE_GAP, GAPS_REALWORLD,
BUILTIN_FUNCTIONS, design_tooling.
**RESULT:** a stranger can write a `wc`/`grep`/`todo` tool or a snake game. zl feels alive.

## PHASE 3 — The unblocker decisions, built ⬜
**GOAL:** the two biggest features (type system, closures) become buildable; the language can fail.
- **Scoping (Option 2)** — assignment binds locally, `global` opts in. Two-phase so the fixpoint
  never breaks. Unblocks per-function type inference AND closures.
- **Error handling** — errors as values + `?`. Stops every later feature needing a retrofit; gives
  back the ~20 test assertions deleted this session.
**NEEDS:** Phase 2. **~SIZE:** ~700 lines. **DOCS:** design_scoping_decision, design_error_handling.
**RESULT:** the roadmap's root dependencies are cleared. Everything downstream unlocks.

## PHASE 4 — Describing data (spine, part 1) ⬜
**GOAL:** stop faking structs with lists. 40+ files become statically typed.
- **Records/structs** — `.` is already parsed; a naming layer over lists, near-zero backend change
- **Maps** — `dijkstra`/`astar` stop doing O(n) scans (they say so in their own comments)
- **Tuples** (already: `[a,b]`; records name the slots), **optionals** (`T?`), **enums**
**NEEDS:** Phase 3. **~SIZE:** ~2,000 lines. **DOCS:** design_records, design_maps_v2,
design_nullable.
**RESULT:** the heterogeneity problem (51 files) collapses to the 8 that are genuinely dynamic.

## PHASE 5 — Everyday language ⬜
**GOAL:** the language feels modern and complete to write.
- `switch`/`match` (reuses `to`/`in`/`else`, one new keyword), **closures that capture**
- constants, destructuring (`a, b = pair`), operators (`<<`, `**`, `??`, `//`)
- **sized integers** (`u8`, `i32`, `u64`) — also the prerequisite for FFI
- fix the ~33 catalogued bugs (`x % 0` crash, `break` escaping functions, etc.)
**NEEDS:** Phase 3–4. **~SIZE:** ~1,800 lines. **DOCS:** design_switch_case, design_closures,
design_sized_integers, the bug catalogue.
**RESULT:** zl reads like a real modern language; the whole functional family + comprehensions land.

## PHASE 6 — The type system + speed ⬜
**GOAL:** fast everywhere, not just on numbers.
- **Gradual type system** — opt-in annotations, the mechanism for unboxing
- **Unboxing** — kill the 48-byte boxed Value + string-keyed operator dispatch: the measured 4–6×
- widen the fast backends to fully cover strings/lists (finish what Phase 1 started)
**NEEDS:** Phase 3 (scoping) + Phase 4 (records). **~SIZE:** ~4,000 lines. **DOCS:** design_type_system,
PLAN_unboxing.
**RESULT:** real programs (not just numeric loops) run at C speed. The core language is *fast*.

## PHASE 7 — Controlling the machine (the FFI tier) ⬜
**GOAL:** zl controls Windows — files, processes, the registry, windows, the network, the kernel
(Ring 3). **This is the identity: a systems language that controls the PC.**
- **FFI** — call any Windows/C DLL function. THE master unlock; needs sized ints (Phase 5).
- `stdlib/win/*` — process control (`run`/`spawn`/`kill`), registry, memory (`ReadProcessMemory`),
  CPU (affinity, `CPUID`), GUI (`MessageBox`, windows), hotkeys, clipboard, ETW telemetry
- **Networking** — sockets, HTTP (via FFI to `ws2_32`) · **Graphics** — a live window (`CreateWindowEx`)
- inline assembly (`CPUID`, `RDTSC`)
**NEEDS:** Phase 5 (sized ints). **~SIZE:** ~1,200 engine lines + libraries. **DOCS:** design_ffi_v2,
KERNEL_CONTROL_GAPS, GRAPHICS_PLAN, ROADMAP items 50–65.
**RESULT:** zl does everything C++ does for PC control — because it wraps the same DLLs. Ring 3 done.

## PHASE 8 — The power tier ⬜
**GOAL:** the features that make zl distinctive, not just complete.
- **`comptime`** — run zl at compile time (the interpreter is already in the compiler, so nearly
  free). This is the mechanism for **Floor 2: the `ai` keyword** — English threat descriptions
  compiled into frozen, readable detection rules. The thing nobody else has.
- threads / async · generics · `defer` · byte buffers (also needed earlier for binary I/O)
**NEEDS:** Phase 6. **~SIZE:** ~3,000 lines. **DOCS:** design_comptime, NOVEL_IDEAS.
**RESULT:** metaprogramming, concurrency, and the AI-in-compiler differentiator.

## PHASE 9 — Security-native (the differentiating identity) ⬜
**GOAL:** security expressed in the language, not bolted on — zl's real niche.
- **Capabilities as types** — a function can't touch files/net/memory unless handed the capability.
  Also THE clean answer to the memory-runaway problem (nothing allocates without an allocator).
- **Taint/trust types** — untrusted data can't reach a dangerous sink uncheck ed (compile-time)
- two-views-of-one-file (`!` danger marker made visible), enforced `explain`
**NEEDS:** Phase 6 (type system) + Phase 8 (comptime). **~SIZE:** ~2,000 lines. **DOCS:** NOVEL_IDEAS.
**RESULT:** the security-native systems language — a niche no mainstream language occupies. Ties to Aegis.

## PHASE 10 — Reach & ecosystem ⬜
**GOAL:** other people can actually use zl.
- package manager + `zl test` + a **formatter** (parser has the AST — cheap) + an **LSP**
- **embedding API** (`zl_new`/`zl_eval`/`zl_register`) — the Lua story; zl embeddable in any C/C++ app
- output a `.dll`/`.lib` others can link · **cross-compilation** (via LLVM) · an installer
**NEEDS:** Phase 5+. **~SIZE:** ~3,000 lines + tooling. **DOCS:** REACH_GAPS, design_tooling,
design_crosscompile.
**RESULT:** zl is installable, embeddable, and shareable — it reaches beyond its author.

## PHASE 11 — The kernel / driver tier (Ring 0) ⬜
**GOAL:** zl becomes the kernel, not just talks to it.
- inline assembly for privileged instructions (`RDMSR`, control registers)
- **the `.sys` driver compile target** — freestanding PE, kernel ABI, `/INTEGRITYCHECK` signing.
  `nativegen` already emits freestanding kernel32-only PE, so this is the same discipline retargeted.
  Validated against **Aegis** (13.5k lines of C driver).
**NEEDS:** Phase 7 (FFI) + Phase 5 (sized ints). **~SIZE:** large. **DOCS:** KERNEL_CONTROL_GAPS
(Ring 0), MASTER_PLAN Floor 6.
**RESULT:** zl can write kernel-mode code — Aegis-class antivirus, drivers, the deepest tier.

## PHASE 12 — THE ENDGAME: remove C from the source ⬜
**GOAL:** zl is written entirely in itself. No C anywhere.
- rewrite the full compiler in zl — now EASY, because zl has structs, maps, errors, a type system
  (all the things that make writing a compiler pleasant, which is exactly why this is LAST)
- `compiler.zl` (already a self-hosting proof) becomes the REAL, complete compiler
- delete the ~11,000 lines of C engine
**NEEDS:** Phases 4–6 (structs/maps/errors/types make the rewrite tractable). **~SIZE:** a rewrite,
not new lines. **DOCS:** MASTER_PLAN Floor 4→source, design_selfhost_parity.
**RESULT:** a language that builds itself, in itself, with zero C. Floor 4 was "no C in the OUTPUT";
this is "no C in the SOURCE."

## PHASE 13 — The floors beyond ⬜ (years out, deliberately)
From MASTER_PLAN's tower, once the language stands entirely on itself:
- own assembler · own tiny OS · own CPU · cross-device (one program everywhere)
**DOCS:** MASTER_PLAN Floors 5–8, design_kernel, design_game_system.

---

## THE SHAPE OF IT

- **Phases 0–1:** correct + fast core (mostly done).
- **Phases 2–3:** usable + unblocked (imports, args, scoping, errors) — the cheap, high-leverage tier.
- **Phases 4–6:** a real, complete, fast language (structs, maps, the type system, unboxing).
- **Phases 7–9:** the identity (controls Windows via FFI; comptime + AI; security-native).
- **Phases 10–11:** reach (ecosystem, embedding) + depth (the kernel).
- **Phases 12–13:** the endgame (delete C) and the far tower (OS, CPU).

**The spine that everything reduces to (from every gap doc):** imports · closures · maps/structs ·
errors · sized ints · FFI · comptime. Seven features. Build those and the rest is library code.

**The single ordering rule:** the scoping decision (Phase 3) is the root — it gates the type system
and closures. Do it before Phase 4+. Everything else follows the dependency arrows above.

**The next concrete move:** Phase 1 finishing (running), then **imports** — the hinge that turns
today's 13 copy-pasted modules into a real standard library.
