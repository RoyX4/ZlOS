# Reach gaps — how zl connects to the outside world

Drafted 2026-08-03. The eleven planning docs cover what zl can do INTERNALLY. This covers the
missing dimension: how zl connects OUTWARD — whether other code can call zl, whether zl ships as
anything but a whole program, whether it runs anywhere but a Windows console. This is how
languages actually SPREAD, and zl currently has almost none of it.

**Every successful language won on a REACH story, not a feature list:**
- **C** — everything links to it; it is the universal ABI.
- **Python** — glue + embeddable + a huge stdlib.
- **Lua** — embeddable in one header; that IS its whole success (games, Redis, nginx, Neovim).
- **JavaScript** — it runs in the browser.
- **Go** — one static binary, cross-compiled anywhere, great tooling.
- **Rust** — C-compatible output + cargo.

zl's reach today: **it produces a Windows `.exe` and nothing else.** That is the narrowest reach
of any language on that list.

---

## 1. Output formats — zl makes ONE thing

| Can zl produce… | today | why it matters |
|---|---|---|
| `.exe` (standalone) | ✓ | the only thing it makes |
| `.dll` (shared library) | ✗ | so no other program can load zl code as a plugin |
| `.lib` / `.a` (static library) | ✗ | so zl code cannot be linked into a C/C++/Rust build |
| `.obj` C-callable exports | ✗ | so a zl function cannot be exposed as a C symbol |
| `.sys` (kernel driver) | ✗ | roadmap Floor 6 (Aegis) |
| `.wasm` | ✗ | so zl cannot run in a browser, edge, or serverless |

**The gap:** a language that only makes whole executables cannot be a LIBRARY for anyone else.
`nativegen` already emits a PE — teaching it to emit a DLL (add an export table) or an object file
(so a linker can consume it) is the mechanism, and it is not huge because the hard part (writing PE
sections and symbols) already exists.

## 2. Embedding — can a C/C++ program run zl inside itself?

**No.** This is the Lua question, and it is the single biggest missed reach opportunity, because
**zl is architecturally almost perfect for it** — the interpreter is a small, self-contained C
program (src/runtime/interp.c + src/frontend/parser.c + src/frontend/lexer.c, ~3,200 lines) with no runtime dependencies.

What is missing is a clean **embedding API** — a header a host program includes to:

    zl_state *s = zl_new();              // create an interpreter
    zl_eval(s, "x = 40 + 2");            // run a string
    zl_value v = zl_get(s, "x");         // read a variable back
    zl_register(s, "host_fn", my_c_fn);  // expose a C function to zl
    zl_free(s);

Lua's entire ecosystem exists because it shipped exactly this. zl has the engine; it lacks the
public API and the "no globals, multiple independent states" refactor that embedding needs (right
now the interpreter uses global state — you cannot have two zl programs in one host process).

**This is arguably zl's strongest natural reach story:** a tiny, readable, embeddable scripting
language with a kernel-capable native backend — a niche Lua owns and few others contest.

## 3. Cross-platform — zl runs in ONE place

- **Windows/x86-64 only.** `os_win.c` is the OS boundary (thin — a real asset), `nativegen` emits
  PE specifically, the calling convention is Win64.
- No Linux, no macOS, no ARM. MASTER_PLAN makes Windows the permanent PRIMARY, which is a fine
  focus — but "primary" should not mean "only," because it caps who can even try the language.
- **The LLVM backend is the cheap escape:** LLVM already targets everything, so cross-compilation
  is largely a `compilel` feature (roadmap item, design_crosscompile.md). `nativegen` cross-target
  is the expensive path and a deliberate non-goal.

## 4. Data interop — can zl EXCHANGE with other tools?

Even without linking, tools interoperate by exchanging data files. zl is shaky here:
- **JSON is BROKEN right now** (json_parse/json_pretty abort — STDLIB_PLAN). The most common
  interchange format does not work.
- **Binary formats are impossible** — strings cannot hold a NUL byte (GAPS_REALWORLD 1.1), so no
  protobuf, msgpack, images, or any binary exchange.
- **CSV silently merges records** on a quoted field (wave3).
- stdin/stdout are line-oriented only — zl cannot be a clean pipe filter (`cmd | zl | cmd`) for
  binary or for streaming data.
**So even the loosest form of interop — swap a file, pipe a stream — is unreliable today.**

## 5. Calling conventions & ABI — can separately-built zl modules interoperate?

- No stable ABI. If zl ever compiles modules separately (rather than whole-program), there is no
  defined way for one compiled module to call another. Today everything is compiled as one unit.
- This ties to imports (item 3): textual inclusion sidesteps the ABI question, but real separate
  compilation (faster builds, shipping compiled libraries) needs a defined calling convention.

## 6. Distribution — can a stranger GET zl?

- No installer, no `winget`/`brew`/`apt` package, no download. Build from source with Visual Studio.
- No package registry for zl LIBRARIES (the crates.io / PyPI equivalent) — item 24.
- No versioning story, no `zl.mod` manifest, no dependency resolution.
(Covered in LANGUAGE_GAP Tier 6; repeated here because distribution is part of reach.)

---

## THE REACH ROADMAP — pick a story, then build to it

zl cannot have every reach story at once. Ranked by fit to zl's actual assets (tiny no-runtime
output, self-contained interpreter, kernel-capable native backend, self-hosting):

1. **EMBEDDABLE SCRIPTING LANGUAGE (the Lua story) — best fit, and under-contested.** Needs: an
   embedding API (`zl_new`/`zl_eval`/`zl_register`), and killing global state so multiple states
   coexist. Medium effort, enormous reach — every C/C++/game/tool that wants a scripting layer
   becomes a potential zl host. **This is the recommendation.**
2. **TINY STANDALONE TOOLS (the Go story, narrowed).** Already half-true — 1.5 KB no-runtime
   binaries that start faster than C. Needs: `args` (item 8), a real install story, and
   cross-compilation via LLVM. Cheap, and it plays to a proven strength.
3. **A LIBRARY OTHERS LINK (the C story).** Needs: `.dll`/`.lib`/`.obj` output with C-callable
   exports and a stable ABI. Medium-hard, but it is what makes zl code reusable outside zl.
4. **BROWSER / EDGE (the JS story).** WASM output. Large, and a different world (no threads, no
   FFI, sandboxed) — a later bet, but it is the single biggest reach multiplier if taken.
5. **KERNEL (the Aegis story).** `.sys` output — Floor 6. Narrow but strategic, and zl is unusually
   close via nativegen's freestanding output.

**The one-line strategy:** zl's output today is a Windows `.exe` and nothing else, which is the
narrowest reach possible. The highest-leverage, best-fitting expansion is **embedding** — turn the
already-small, already-self-contained interpreter into something a host program can include in one
header. That is how Lua, a language far simpler than zl, reached everywhere. It needs an API and a
de-globalisation pass, not new language features — so it is orthogonal to the whole feature roadmap
and could proceed in parallel.

**What this adds to the plan that no other doc had:** REACH is a first-class axis. A language can be
feature-complete and still reach nobody. zl should decide its reach story (recommendation:
embedding first, tiny-tools second) alongside deciding its features — because the best features are
worthless if zl's only output is an executable that runs on one OS.
