# zl Roadmap

Written 2026-08-03. Scoped from `docs/design/LANGUAGE_GAP.md` (the complete gap list) and
the design docs written 2026-08-02.

**The governing decision** — MASTER_PLAN §10, 2026-08-02: *language completeness before
optimisation*. Speed is already solved where the language reaches. Coverage is the ceiling.

**Size estimates are rough** — order-of-magnitude, to decide sequencing, not to plan a
schedule. They assume the pattern that is working: one agent per feature, small scope,
commit on green.

---

## Where the 5,370 lines are today

| | lines |
|---|---|
| `interp.c` | 1,624 |
| `parser.c` | 1,125 |
| `runtime.c` | 1,040 |
| `nativegen.c` | 509 |
| `compile.c` | 496 |
| `compilel.c` | 448 |
| `lexer.c` | 431 |
| `compilef.c` | 185 |
| **engine total** | **5,370** |
| `compiler.zl` | 664 |
| stdlib (96 modules) | 16,335 |
| tests (8 suites) | 6,133 |

Reference: Zig's self-hosted compiler is ~197,500 lines. A *complete* zl is plausibly
15,000–20,000 lines of engine. That is the scale being planned for.

---

## Phase 0 — finish what is in flight

Running now, no new work needed.

- ~33 confirmed bugs (`x % 0` process kill, `break` escaping functions, backend
  divergences, nativegen globals, LLVM `sdiv` UB)
- **Floats in the fast backends** — currently `2.5 * 2.0` silently gives `4`. Step one is
  making it *refuse* rather than truncate; real doubles in `compilel` second.
- The remaining 8 feature specs

**Exit criterion:** no known-and-unfixed correctness bug; every engine either supports
floats or refuses them loudly.

---

## Phase 1 — the unblockers

Nothing else is worth building until these three are done. Each is small; each is load-bearing.

| # | Item | ~lines | Why first |
|---|---|---|---|
| 1.1 | **Imports** | ~300 | 96 stdlib modules are unusable without it. Reuse today is copy-paste, and a bug fixed in one copy stays broken in the others — already paid once, in `sortx.zl` |
| 1.2 | **Scoping decision** | 0 (a decision) | An assignment inside a function writes the *global* slot. Until this is settled, no type inference pass can be written, because a global that is int at top level and str inside a function has no type. See `design_scoping_decision.md` |
| 1.3 | **Error handling** | ~600 | zl has *no way to fail*. Everything built after this would otherwise need retrofitting across 96 modules. It is also destroying test coverage now — ~20 assertions were deleted because `check(this errors)` cannot be written. See `design_error_handling.md` |

**Why imports first:** highest work-to-value ratio in the entire document. One feature turns
96 standalone programs into a standard library.

---

## Phase 2 — describing data

The measurement that drives this phase: 51 of 111 `.zl` files use heterogeneous lists, but
only **8** are genuinely dynamic. The other ~43 are structs, tuples and generics with no
type to express them.

| # | Item | ~lines | Notes |
|---|---|---|---|
| 2.1 | **Records / tuples** | ~400 | `.` is *already parsed* — `parser.c` builds `N_MEMBER` and the only consumer is an "isn't supported yet" error. Design is a naming layer over lists, so interp/compile/nativegen need **zero changes**. See `design_records.md` |
| 2.2 | **Maps** | ~600 | `dijkstra.zl` and `astar.zl` both do O(n) list scans and say why in comments. See `design_maps_v2.md` |
| 2.3 | **Optionals** | ~300 | 13 files, 33 functions using `nil()` as a sentinel. See `design_nullable.md` |
| 2.4 | **Sized integers** | ~800 | i8–i64, u8–u64. Prerequisite for FFI and for any file format |

**Blocking question to settle before 2.1:** `design_records.md` proposes `rec` as a *value*
backed by a list (Floor 1); `design_memory_structs.md` proposes `struct` as an *address*
with a compile-time offset table (Floor 5, for FFI). Both claimed `.`. They are probably
two features sharing one layout function, but building the wrong one first is the most
expensive mistake available here.

---

## Phase 3 — everyday language

| Item | ~lines |
|---|---|
| `switch` / `match` | ~200 desugared, more with a jump table |
| Closures / functions as values | ~500 |
| Constants | ~100 |
| Multi-line list literals | ~50 |
| Destructuring (`a, b = pair`) | ~150 |
| Operators: `<<`, `>>`, `**`, `??` | ~150 |
| **`//` integer division** | ~100 — and this partly *fixes* the float divergence, because one operator currently has to mean two things |
| Multiple return values | ~200 |
| Default / named arguments | ~250 |

---

## Phase 4 — usable by someone other than the author

Everything here is cheap and disproportionately valuable. None of it touches the compiler.

| Item | ~lines | Why |
|---|---|---|
| **Stack traces + line numbers in runtime errors** | ~200 | Today a failure three functions deep says `runtime error: at needs a string and a number` — no line, no function, no trace |
| **Command-line arguments** | ~30 | zl cannot write a real CLI tool today |
| **REPL** | ~150 | How people learn a language |
| **Formatter** | ~400 | The parser already builds a full AST; a pretty-printer over it is cheap and pays off across 96 modules |
| **Syntax highlighting** (VS Code / vim) | ~100 config | zl is currently written in black and white |
| **Install story** | — | Today: clone a repo, install Visual Studio, run `build.bat`. That is the gap between a project and a language |
| **Unicode / UTF-8** | ~800 | `len("café")` returns 5. `upper("café")` gives `CAFé`. `at("café",3)` returns half a character. zl is English-only |
| Profiler | ~300 | The `push` regression was found by hand-bisecting 7 commits |

---

## Phase 5 — memory

Counted 2026-08-02: `interp.c` has **61 mallocs / 15 frees**; `runtime.c` **59 / 12**. No GC,
no refcounting, no arena. zl leaks by construction — fine for short programs, fatal for a
server, a game loop, a REPL or a compiler processing many files.

This is a language-identity decision, not an implementation detail. Options: GC ·
refcounting · arenas · manual `alloc`/`free` with `defer`. MASTER_PLAN's "power over
safety" identity argues for the manual end.

Must be settled before Phase 6, because FFI and raw memory both assume an answer.
Then `defer` (~300 lines) falls out of it.

---

## Phase 6 — power

| Item | ~lines | Notes |
|---|---|---|
| **`comptime`** | ~800 | zl is unusually well placed: the interpreter is *in the same binary* as the compiler, so `eval()` already exists. Zig had to build a separate evaluator. This is also the mechanism **Floor 2** needs — AI compiling English threat descriptions into frozen, readable rules. Hard constraint: the fixpoint requires compiling the same source twice to give byte-identical output, which an LLM call does not satisfy unless cached and committed |
| **FFI** | ~1,000 | zl cannot call C or Windows at all. `nativegen` emits kernel32-only binaries with no libc, so without FFI there is nothing to call |
| **Syscalls / raw memory** | ~800 | Floor 5 |
| Generics | ~600 | ~11 stdlib modules are polymorphic and cannot say so |
| Threads / atomics | ~800 | |

---

## Phase 7 — speed, at last

| Item | ~lines | Worth |
|---|---|---|
| De-string `zl_binop` | ~150 | 46 `strcmp`s per `b2_arith` iteration, 110M per run. One file, no signature change, zero fixpoint risk — the recommended first stage in `PLAN_unboxing.md` |
| Shrink `Value` 48 → 16 bytes | ~400 | ~1.44 KB of ABI copying per iteration today |
| **Type system** | ~2,000 | The mechanism for unboxing |
| **Unboxing** | ~1,500 | **4–6×**, measured |
| Widen fast backends to strings/lists | ~1,500 | The real ceiling — `b5_string` is only 1.3× faster compiled, because both engines sit in the same `runtime.c` routines |

**Not on this list, deliberately: writing our own optimiser passes.** Measured 2026-08-02,
`compilel` matches `clang -O2` (67ms vs 71, 58 vs 61, 33 vs 33, 62 vs 62). The own-x64
backend is level with C on call-heavy code and ~4× off only on tight loops — it lacks *loop*
optimisation specifically. LLVM gives C parity for free.

---

## Phase 8 — the floors

Back to MASTER_PLAN's tower, now that the language can carry them.

- **Floor 2** — AI compiles English threat descriptions into readable, frozen rules.
  Needs `comptime`. This is the differentiator nobody else has.
- **Floor 3** — self-evolving detection rules → the Aegis wire
- **Floor 5** — own assembler
- **Floors 6–8** — OS, CPU, cross-device. Years out, deliberately deferred.

---

## Rough totals

| Phase | ~lines |
|---|---|
| 1 unblockers | 900 |
| 2 data shapes | 2,100 |
| 3 everyday | 1,200 |
| 4 usability | 1,800 |
| 5 memory | 1,000+ |
| 6 power | 4,000 |
| 7 speed | 5,500 |
| **total added** | **~16,500** |
| **engine after** | **~22,000** |

Against Zig's ~197,500 that looks small — but Zig's figure includes its standard library,
package manager, build system and multi-target backends. For a single-target, self-hosted
systems language, ~20k is the right order of magnitude.

---

---

# THE LIVE PLAN — ordered, 2026-08-03

This supersedes the phase table above for day-to-day sequencing. The phases explain *why*;
this list is *what next*.

**Now (running):** LLVM and x86-64 backends compile the whole language, not just integers.

### The unlocks — worth more than everything after them
1. **Memory ceiling in the interpreter** — refuse past ~2 GB instead of eating the machine (~20 lines)
2. **Delete `compile.c`, `compilef.c`, `runtime.c`** — 5 engines → 3, after moving the gate to the native path
3. **Imports** — 96 stdlib modules become loadable instead of copy-paste
4. **Structs and maps** — name your data instead of remembering `p[0]`
5. **Errors** — a program can fail instead of dying

### Everyday language
6. Decimals working on every engine
7. Runtime errors that name the line and the function (stack traces)
8. Command-line arguments — so zl can write a CLI tool at all
9. A REPL
10. `switch` / `match`
11. Closures — functions as values, so `sort` can take a comparator
12. Constants, enums, destructuring (`a, b = pair`)
13. Unicode text — `len("café")` is 5 today

### Power
14. `defer`
15. Sized integers (`u8`, `i32`) — prerequisite for FFI and file formats
16. **FFI — calling Windows and C.** zl is sealed off entirely today
17. `comptime` — and with it Floor 2, the AI-in-compiler differentiator
18. Threads
19. *(optional)* delete C from the SOURCE — `compiler.zl` becomes the real compiler

### Language shape
20. Custom iteration — `for node in mytree` on your own types
21. Inline functions / lambdas
22. Labeled break — escape two loops at once
23. Type aliases, public/private in modules

### Tooling
24. `zl test`, `zl new`, a package manager, a build system
25. Sleep, timers, launching other programs
26. Full filesystem ops — copy, move, delete, walk
27. Networking — HTTP and sockets
28. Real regex, PNG, zip, a database
29. Fix `randint` — capped at `lo + 32767`, useless as-is
30. A debugger

**The whole list collapses to four unlocks:** imports (3), structs (4), closures (11) and
FFI (16). Most of items 20–30 are blocked behind one of those four, not behind their own
difficulty. Networking, sleep, launching programs, raw keyboard and file watching are all
Windows calls — item 16 alone unblocks a dozen entries below it.

### Convenience — from Python/JS (mostly parser sugar, ~50 lines each, unlock nothing but daily feel)
31. List comprehensions — `[x*2 for x in xs]`
32. Negative indexing — `xs[-1]`
33. Slicing — `xs[2:8]`, `xs[::2]`
34. Swap — `a, b = b, a`
35. `enumerate` — loop with the index
36. Format specifiers in f-strings — `f"{price:.2f}"`
37. Spread — `f(...args)`, `[...xs, ...ys]`

### Bigger capabilities other languages have
38. **Interfaces / traits** — "anything that can draw itself". Unlocks custom iteration (20), comparators (11) and generic containers in one feature
39. **Methods on types** — `img.save()` instead of `save(img)`
40. **Explicit allocators (Zig-style)** — every allocation is handed an allocator; a runaway becomes impossible by construction. This is the principled answer to the memory problem that crashed the machine on 2026-08-03
41. `errdefer` — cleanup that runs only on failure
42. References — pass a big value without copying
43. Async / await — nothing exists today
44. Tail-call recursion — infinite depth without the 2000-cap crash
45. Macros — code that writes code (may be subsumed by `comptime`, item 17)

### Developer experience
46. Watch mode — save, auto-rebuild, auto-rerun
47. Incremental build — rebuild only what changed
48. Per-platform compilation — `#if windows`
49. Doc comments → generated docs site

**Two of these are answers to problems already on the list.** Explicit allocators (40) is
how Zig prevents exactly the runaway that hit 51.8 GB today. Interfaces (38) is what makes
items 11, 20 and generic containers a single feature instead of three.

### Controlling the PC — the C++ parity target (ALL of it is FFI + a zl library, not engine work)

Everything here is a Windows DLL function that already exists compiled in the OS. C++ reaches
it via `#include <windows.h>`, which is just declarations. zl needs the SAME thing: FFI
(item 16) to call a DLL function, then a zl library that wraps each call. So this is not N
engine features — it is **one engine feature (FFI) plus library code written in zl.**

50. **Full filesystem** — copy, move, delete, rename, mkdir, exists, size, mtime, walk a tree, **watch a folder** (extends item 26)
51. **Process control** — launch a program and capture its output (`CreateProcess`), kill a pid, full process list with memory. zl can SEE processes today but not launch or control them
52. **Windows registry** — read and write system settings
53. **GUI** — `MessageBox`, open a window and draw on it, system-tray icon
54. **Global hotkeys** — `RegisterHotKey`, Ctrl+Shift+X captured from anywhere
55. **Input automation** — send keystrokes and mouse clicks to other apps (`SendInput`), read/write the clipboard
56. **Networking** — sockets, HTTP, DNS, ping (this is item 27, delivered via FFI to `ws2_32.dll`)
57. **System & hardware info** — CPU/RAM/battery/disk stats, USB device list, screen resolution
58. **Media** — play a sound, take a screenshot, read a webcam frame
59. **Deep control** — load a `.dll` at runtime (`LoadLibrary`/`GetProcAddress`), memory-map a file, shared memory between processes, request admin elevation, install a Windows service

**The whole "control the PC" block is gated on ONE thing: item 16, FFI.** Ship FFI and items
50–59 stop being engine work and become a `stdlib/windows.zl` that anyone can write and
extend. This is precisely how C++ does it — C++ is not more powerful at PC control, it just
already ships the bindings. This is the single highest-leverage capability in the whole
roadmap for the "systems language that controls Windows" identity in MASTER_PLAN section 4.

### Low-level networking and kernel — the deepest tier

Same rule as the PC-control block, with ONE exception noted below: almost all of this is
FFI (item 16) plus a zl library wrapping a DLL. Context: this serves the Aegis kernel-AV
and RedOps authorized-pentest projects, which already work at this layer in C.

60. **Raw sockets** — UDP, ICMP/ping, custom protocols via `ws2_32.dll`. (Windows blocks raw
    *TCP send* at the socket layer — that needs item 61.) Pure FFI.
61. **Packet capture, injection and interception** — send any packet, sniff all traffic, and
    intercept/modify/drop/re-inject in flight, via **WinDivert** or **npcap** (the driver
    Wireshark uses). Both ship a signed driver and expose a DLL, so from zl this is
    `stdlib/windivert.zl` — FFI, no engine work. Crafting the packet bytes zl can already do
    with `write_bytes`; this adds the send/recv/intercept path.
62. **Firewall / packet filtering at kernel level** — the Windows Filtering Platform (WFP).
    A callout driver is item 66; the userland config side is FFI to `fwpuclnt.dll`.
63. **Talk TO a kernel driver** — `DeviceIoControl`: send a control code plus a buffer to a
    driver, get bytes back. Pure FFI. This is exactly how the Aegis user-mode client already
    drives its own kernel driver, so there is a working C reference in kernel-defender-lab.
64. **Load / start / stop a driver** — the Service Control Manager APIs (`OpenSCManager`,
    `CreateService`, `StartService`). FFI.
65. **ETW / kernel telemetry** — subscribe to Event Tracing for Windows: process starts,
    network connections, file and registry activity, straight from the kernel's own feed.
    FFI to `advapi32`/`tdh.dll`. Aegis already consumes ETW-TI in C.

66. **BE the kernel — write a driver IN zl.** THE ONE EXCEPTION: this is not FFI, it is a new
    COMPILE TARGET. A `.sys` is a freestanding PE — no libc, no CRT, a strict kernel ABI, only
    kernel-exported functions available, `/INTEGRITYCHECK` signing required. This is
    MASTER_PLAN **Floor 6** and it is genuinely hard. But zl is unusually close: `nativegen`
    already emits kernel32-only PE binaries with no runtime, which is the same freestanding
    discipline pointed at a different import set, and Aegis exists as the driver to validate
    against. Order: sized ints (15) → FFI (16) → this.

**The rule holds one more time: items 60–65 are FFI + a library. Only item 66 (writing the
driver itself) is real engine work, and it is Floor 6.**

The ring split, at a glance (full detail in KERNEL_CONTROL_GAPS.md):

| capability | ring | zl needs |
|---|---|---|
| Edit your own program's memory | 3 | bytes + pointers |
| Read/write ANOTHER program's RAM (debugger / AV scanner) | 3 | FFI → ReadProcessMemory |
| Control the CPU — pin to a core, read cycle counter, CPUID | 3 | FFI + inline assembly |
| Talk to a driver, load a driver, read kernel telemetry (ETW) | 3 | FFI |
| Physical RAM, CPU control registers, MSRs, syscall hooks | 0 | write a .sys driver in zl (Floor 6) |
 So "control the kernel and packets
like C++" is, again, almost entirely downstream of item 16.

**Grand total: ~66 items. The real spine is still five unlocks** — imports (3), structs (4),
closures (11), FFI (16), comptime (17) — with **sized integers (15)** as the gate in front of
FFI, because a Windows/driver function's arguments (`DWORD`, `HANDLE`, `LPWSTR`, control codes)
cannot be described without exact-width types. Everything from PC control (50–59) through
networking and kernel (60–65) is one library layer on top of FFI. The path to the whole
bottom half of this roadmap is: **sized ints → FFI → the wrapping libraries.**

## What "done" means

zl is finished as a language when a stranger can:

1. Install it without cloning a repo or installing Visual Studio
2. Write a program that imports a library, uses a struct and a map, handles an error, and
   prints a stack trace when it fails
3. Call a Windows API from it
4. Compile it to a 1.5 KB binary that runs at C speed

Items 1–3 are Phases 1–6. Item 4 already works, for integers.
