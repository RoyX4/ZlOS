# Gaps found by building real programs

Written 2026-08-03. The six existing planning docs (`LANGUAGE_GAP.md`,
`C_CPP_PARITY.md`, `FUNCTION_INVENTORY.md`, `BUILTIN_FUNCTIONS.md`, `NOVEL_IDEAS.md`,
`ZL_1.0_VISION.md`) catalogue the missing features **abstractly** — from the language's
own structure and from comparison against C/C++/Python/Rust/Go/JS. This doc is the
**complement**: what actually bit when agents sat down and tried to build three real
programs, wall by wall. It does not repeat the abstract lists — it references them by
filename and records only what building revealed that they missed or under-stated.

The three programs, and their one-line verdicts:

- **web-server** — a basic HTTP/1.1 accept loop. *Fails at `socket()` and at five more
  layers underneath it.*
- **terminal game** — a real-time snake/pong. *Fails at one wall: non-blocking keyboard
  input. Almost everything else already works.*
- **xxd hex dumper** (with a `wc -w` cross-check) — the sharpest C-parity probe. *Fails at
  line 1 on any real binary file; works only on NUL-free text.*

Every wall below was run against `interp.exe` on 2026-08-03, or read out of the source —
not inferred. Probes were tiny (leading-underscore scratch files).

---

## 1. NEW GAPS — not in any existing doc

These are the highest-value findings: things the abstract lists do not contain at all, or
reduce to something much smaller than they actually are.

### 1.1 Strings cannot hold a NUL byte, there is no byte container, and reading binary silently truncates — *(web-server, xxd)*

This is the single sharpest concrete failure found, and the most under-documented.

Measured:

```
chr(0)                          -> runtime error: chr(0): a zl string cannot hold a NUL byte
write_bytes("_p.bin",[72,0,66,67])   writes 4 bytes to disk (confirmed: wc -c = 4)
read("_p.bin"); len(...)        -> 1        (truncated at the first 0x00, SILENTLY, exit 0)
```

Consequences the docs miss:

- `read()`/`len()`/`at()`/`slice()` all use `strlen`, so **any byte stream is silently
  cut at its first `0x00`** with no error — `len()` just returns a wrong small number.
- `write_bytes` (list -> file) exists, but there is **no `read_bytes` inverse** (file ->
  list). zl can *write* an arbitrary binary file but cannot *read* one back.
- The only binary-capable container is a **list of boxed byte-values**, each a separately
  malloc'd Value behind an 8-byte pointer (~64x memory blow-up, never freed), and it
  bridges to nothing — no socket/write path consumes it.

What the docs say instead: `LANGUAGE_GAP.md` (Tier 5b) frames strings only as a **Unicode**
problem (`len("café") -> 5`) and literally calls them *"byte arrays"* — they demonstrably
are not, they cannot hold a `0` byte. `FUNCTION_INVENTORY.md`/`BUILTIN_FUNCTIONS.md` list
`read_bytes`/`bytes` as *missing convenience builtins*. None of the six state (a) that
reading binary is therefore **impossible**, (b) the **silent, no-error** truncation failure
mode, or (c) the **write/read asymmetry**. This turns a "missing builtin" into a structural
inability to represent or read binary — the wall a network program and a hex dumper both
hit first.

### 1.2 `num()` / `int()` silently return `0` on bad input — a silent-corruption defect in *existing* builtins — *(web-server)*

Measured: `num("abc") -> 0`, `num("12ab") -> 12`, `int("xyz") -> 0`, all exit 0, no signal.

The docs list a *missing* `parse` builtin (blocked on errors, item 5) and note error
handling is absent — but never flag that the builtins that **already exist** actively
swallow garbage into a plausible-looking `0`. This is worse than "no validation is
available": validation is *actively unsafe*, because the primitive lies. A malformed
`Content-Length: xyz` or a bad port becomes `0` with no way to tell it apart from a real
zero — the network boundary cannot be validated even before error handling exists.

### 1.3 No I/O multiplexing — *both* concurrency routes are closed, not just threads — *(web-server)*

The docs note "no threads" (item 18) and list the blocking socket API
(`socket/listen/accept/recv/send`, `FUNCTION_INVENTORY.md:166`) — but never mention
`select`/`poll`/`epoll`, non-blocking sockets, or an event loop. That omission makes
concurrency look like **one** missing feature. In reality the standard **no-threads** server
design — one thread multiplexing many sockets, i.e. how nginx/redis/node work — is *also*
absent. With no `sleep` either (§2), even a naive poll can't be paced. So a zl server is
provably limited to **one client at a time, fully blocking**: client B waits until client A
disconnects. Closing concurrency needs an answer to *both* routes.

### 1.4 The interpreter never enables Windows VT processing — a portability trap — *(terminal game)*

Confirmed by reading source: `interp.c` contains **zero** `SetConsoleMode` /
`ENABLE_VIRTUAL_TERMINAL_PROCESSING` calls (grepped), and `os_win.c` includes no console
setup at all (only `os_dir` + `os_procs`). Every colour and cursor move in `life.zl` /
`maze.zl` is an ANSI escape, and they work **only because Windows Terminal turns VT on by
default.** On stock `conhost.exe` every escape prints as literal garbage (`ESC[2J…`) and the
output is unreadable. No doc mentions this. The interpreter should set the console mode
itself; until it does, "it renders" is terminal-dependent luck.

### 1.5 No `flush()`, and `print` always appends a newline — so animation must rebuild the whole frame — *(terminal game)*

Measured: `flush()` -> unknown function; every `print` emits a trailing `\n`. The docs list
`println` (newline control) and `eprint` (stderr) as missing, but not the **consequence**:
you cannot draw incrementally. A terminal animation must assemble the **entire frame** — all
cells + embedded newlines + a cursor-home escape — into one string and `print` it once per
tick (exactly `life.zl`'s contortion). And on a block-buffered or redirected stdout, even
that won't stream, because there is no way to force a flush.

### 1.6 No multi-line / triple-quoted string literals — *(xxd)*

Measured: a literal newline inside `"…"` is a parse error (`line 1: string never closed`).
`LANGUAGE_GAP.md` (Tier 3) lists multi-line **list** literals as missing — multi-line
**string** literals are a distinct, unlisted gap. Any embedded template, usage block, or
heredoc-style payload has to be built by `+`-joining single-line pieces.

### 1.7 f-strings exist, but have no format specifiers — *(xxd)*

Not previously recorded that zl even *has* string interpolation. It does:
`f"val={x}"` -> `val=5`. But `f"{x:02}"` is a parse error
(`f-string slot has leftover text (got ':')`). So the interpolation half of formatting is
present and the **specifier half is not** — width, precision, zero-pad. This sharpens item 36:
f-strings do **not** close the format gap. Fixed-width columns (xxd's `%02x`/`%08x`) still
need hand-rolled helpers (see §2).

### 1.8 `split()` cannot collapse whitespace runs or treat `\t`/`\n` as separators — *(xxd's `wc -w` cross-check)*

Measured: `split("the  quick", " ")` returns **3** fields — the double space yields a spurious
empty field. `split` takes one fixed literal delimiter and nothing else.
`FUNCTION_INVENTORY.md` lists Go's `Fields` / Rust's `split_whitespace` as missing *library*
functions, but never draws out the concrete result: **a basic `wc -w` cannot be written**
with the `split` you have. You must iterate character by character.

### 1.9 Documentation error (minor) — `concat` is not a string builtin

`BUILTIN_FUNCTIONS.md:55` lists `concat` under **STRING** builtins. Measured:
`concat("a","b")` -> `runtime error: concat needs two lists`. `concat` is **list-only**;
string concatenation is the `+` operator exclusively (`"a" + "b" -> "ab"`). One-line fix to
the doc.

---

## 2. SHARPER EXAMPLES — known gaps a real program made vivid

Each of these is already in the abstract docs. The real programs make them concrete.

- **No memory management** (`LANGUAGE_GAP.md` Tier 0.2, which names *"a server"*). The
  web-server makes it literal: every request's parsed strings and lists leak, forever, in
  the accept loop. Given the 51.8 GB machine-crash incident and `maze.zl`'s 11 GB working
  set, a long-running zl server **OOMs the host, not just the process**. The game leaks the
  same way, one frame at a time.
- **No error handling** (Tier 2). One dropped client crashes the *entire* server — a builtin
  that fails kills the process, and there is no `try`/`Result` to skip a bad request. And the
  error is **location-blind**: `socket(3)` produced a bare `runtime error: unknown function`
  — no line, no function, no stack. Sharpening the sharpener: **parse** errors *do* carry line
  numbers (`line 1: string never closed`), but **runtime** errors don't — confirming
  `LANGUAGE_GAP.md`'s "runtime errors say nothing about where," and showing the line-number
  machinery already exists on the parse path.
- **Assignment inside a function writes the GLOBAL slot** (`design_scoping_decision.md`). In
  a per-request `handle()` or a per-frame `tick()`, any local name colliding with a global (a
  loop `i`, a `line`, a `path`, a `head`) silently clobbers state across requests/frames — a
  data-corruption bug *even single-threaded*. Vivid proof it is already feared: **both**
  shipped examples prefix **every** local (`cv_`/`fd_` in `maze.zl`, `ng_`/`rs_` in `life.zl`)
  purely to dodge it. This footgun is latent in all three targets.
- **No maps** (item 4; `dijkstra.zl` comment *"No maps in zl"*). HTTP headers become a list of
  `[key, value]` pairs scanned linearly — O(n) per lookup.
- **No `sleep`** (item 25). Game pacing has to busy-wait on `now()` (`life.zl`'s `pause()`
  spins until `now()` advances), **pegging a CPU core at 100%**. `sleep(1)` -> unknown function.
- **`key-read`/`raw-mode` missing** (`FUNCTION_INVENTORY.md:163`). The entry is under-specified
  in the one way that matters: the game needs a **non-blocking poll** (`kbhit`: "is a key
  available, else keep ticking"), not a blocking char read. Even if `read_char()` existed it
  would still freeze the snake between keypresses. The blocking-vs-non-blocking distinction —
  the actual crux — is drawn nowhere. Today the *only* input primitive is line-buffered
  `input()` (`fgets`), which blocks until Enter.
- **No args/argv** (item 8). The xxd path must be hardcoded into the source, so xxd stops being
  a usable tool.
- **No format string / padding** (item 36). `hex(255) -> "ff"`, `hex(10) -> "a"` (unpadded);
  `pad("7",4) -> "7   "` (right-pad only, no `zfill`/`rjust`). The xxd port had to hand-write
  `hex2()`/`hex8()` loops that prepend `"0"` for *every* fixed-width column. It works
  (produced `00000000: 48 69 21 0a`) but every column is manual.
- **Whole-file-only I/O** (`C_CPP_PARITY.md` `<stdio.h>`: no streaming/seeking). An HTTP
  request has no known length up front, and `read()` slurps the whole file — there is no
  `read(16)`, no `fseek`/`feof`, no cursor. The read-a-bit-then-decide loop every request
  parser and every streaming tool needs cannot be expressed.
- **O(n²) string building.** `line = line + piece` in a loop is quadratic — strings are
  immutable and there is no string builder. The docs measured this for *list* `push`, not for
  string concatenation. The O(n) workaround (push pieces into a list, then `join()`) is
  non-obvious.
- **No signal / `atexit` cleanup** (`C_CPP_PARITY.md` `<signal.h>`, `atexit`). A game that hides
  the cursor (`CSI ?25l`, which `life.zl` emits) or switches to an alternate screen leaves the
  terminal **broken** on Ctrl-C or a crash, with no way to install a restore-on-exit handler.

---

## 3. THE BLOCKER RANKING — from the demand side

The abstract docs rank features by **dependency** (what unlocks the most other features).
This ranks by **demand**: across the three real programs, which single missing feature
blocked the most of them, weighted by severity. This is independent roadmap evidence.

| # | Missing feature | web-server | game | xxd | Why it ranks here |
|---|---|---|---|---|---|
| 1 | **Memory management** | fatal | leaks/frame | (short-lived) | Caps *every* long-running program; can OOM the **host**. 2/3 hard, latent in all. |
| 2 | **Error handling** | fatal | — | dev pain | One failure kills the process; blocks validation + recovery; location-blind errors slow all three during development. |
| 3 | **Byte/binary layer** (NUL-safe strings *or* a real buffer + `read_bytes`) | no request body | — | dies line 1 | 2/3 **hard** blockers, and the most under-documented gap (§1.1). |
| 4 | **FFI / OS access** (+ **sized ints** as its gate) | can't `socket()` | — | — | The whole networking/process/filesystem category; the web-server can't emit line 1 without it. |
| 5 | **Structs/records + sized ints** | `sockaddr` layout | grid/entity | columns-are-numbers | Felt by **all three** as PAINFUL — the everyday "describe data" gap. |
| 6 | **Scoping-writes-global** | cross-request corruption | cross-frame corruption | latent | A correctness footgun in all three; both shipped examples prefix every local to dodge it. |
| 7 | Cheap single-program unblocks | — | `sleep`, non-blocking input, `flush`, VT-mode | `args` | Each one turns a specific program from impossible to writable; each is a few lines. |

**The surprise in this table: closures did not block any of the three.** Closures are the
abstract docs' **#1** unlock (map/filter/reduce across five languages). But a server, a game,
and a hex dumper are all imperative and none is collection-transformation-shaped, so closures
never became a wall. That is partly sample selection — a CSV filter or log analyzer would flip
it instantly — but it is a real demand-side signal: **for the systems-style programs zl is
positioned for, memory + errors + the byte layer are what actually kill you, and they
out-rank closures.** The roadmap should not let the (correct) ergonomic case for closures
push memory, errors, and a byte container down the queue.

---

## 4. WHAT YOU CAN ALREADY BUILD — the honest floor

None of the three targets is buildable *as specified*. But the floor is higher than "three
failures" suggests — reduced and adjacent variants work today, on the **interpreter** (the
fast backends are separately disqualified: they lack strings, lists, and floats).

**Buildable now:**

- **Non-interactive ANSI terminal animation / visualizer.** `life.zl` (Game of Life) and
  `maze.zl` (batch solver) **ship today** and run. An autonomous animation — stopped by a
  cycle, a fixed tick count, or completion — is buildable. Caveat: only in a **VT-enabled**
  terminal (Windows Terminal, not stock `conhost.exe`, per §1.4), and it leaks steadily
  (fine for a bounded run).
- **A turn-based / prompt-driven text game.** Because `input()` blocks on Enter, a game that
  reads one whole command per turn — a text adventure, guess-the-number, a turn-based board
  game printed each move — is **fully** buildable. The game blocker is specifically
  **real-time** input, not input.
- **A line-oriented text tool on NUL-free input, with a hardcoded path.** The xxd port
  actually **produced correct output** (`00000000: 48 69 21 0a`) on text. A grep-lite filter
  or `wc -l` (line count — `split` on `"\n"` works) is buildable, awkwardly. *Not* buildable:
  `wc -w` (§1.8), and *any* binary input (§1.1).
- **Substantial batch text processing** — up to the **664-line self-hosted compiler**
  (`compiler.zl`) itself, which is the existence proof for this whole class.

**Not buildable at all (state it plainly):**

- A **web server** — fails at socket creation, and would still fail at the byte buffer, the
  read loop, concurrency, validation, memory, and error recovery even if sockets were bolted
  on.
- A **real-time terminal game** — fails at non-blocking keyboard input.
- A **binary tool** — fails at reading any NUL-containing byte.

**Positive surprises worth banking** (found while probing, easy to forget zl has):

- **Structural equality works**: `[3,4] in body -> true`, so a snake self-collision test needs
  no hashing — membership over lists just works.
- **Floats work on the interpreter.** The "no floats in fast backends" gap (`LANGUAGE_GAP.md`
  Tier 5) is a **red herring** for a terminal game, which runs on the interpreter.
- **Parse errors carry line numbers** even though runtime errors don't — the machinery to fix
  the runtime-error path (§2) already exists on the parse path.
- **f-strings exist** (§1.7) — interpolation softens, though does not close, the formatting
  story.
