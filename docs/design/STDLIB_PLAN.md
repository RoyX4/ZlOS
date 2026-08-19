> **AUDITED 2026-08-19 · PARTLY OPEN.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. Its stated prerequisite landed and its stated top priority did not: imports are fully implemented and gate-covered, while all seven Part 3 bugs — including the `json_parse` / `json_pretty` abort this doc calls highest priority — reproduce verbatim on main today. Keep Part 3 as a live bug list; Parts 1 and 2 need rewriting.
>
> **What is still open from this document is in
> [`docs/STATE-OF-THE-PROJECT.md`](../STATE-OF-THE-PROJECT.md) — read that first, and do not
> work from the task list below.**

# Standard library plan — organising 96 modules and finding the holes

Drafted 2026-08-03. zl ships **96 stdlib modules today**, but they are a FLAT grab-bag: real
data structures sit next to toy demos, nothing can be imported (item 3), and wave3 found several
are outright broken. This plans the library as a real thing: a namespace tree, a demos-vs-stdlib
split, the missing modules, and a quality pass. For comparison, Python ships ~200 modules, Go ~150,
Rust's std is smaller but its ecosystem (crates.io) enormous.

**Depends on imports (item 3).** Until modules can be imported, none of this organisation is
reachable. So imports is the prerequisite for the whole library story.

---

## PART 1 — the 96 modules, sorted into a real tree

Proposed namespace, `std.<area>.<module>`, so `import std.collections.dict` or `import dict`.

### std.collections (13) — mostly solid
`dict` `set` `hashset` `deque` `heapq` `stack_queue` `linkedlist` `binarytree` `trie`
`bitset` `fenwick` `segtree` `unionfind`
GAPS: none of these are TYPED (need generics, item 38) and dict/set are O(n) until maps (item 4).
Missing: `orderedmap` `priorityqueue`(heapq covers it) `ringbuffer` `graph` (astar/dijkstra are
loose, should live under a `graph` type).

### std.string (10)
`strx` `string_utils` `strsearch` `string_analysis` `wordwrap` `diff` `lcs` `levenshtein`
`template_engine` `fmt`
GAPS: real `regex` (regex_match is basic — item 28), `unicode` (item 13), proper `format` (item 36).

### std.math (14) — a genuine strength
`math_basics` `mathx` `stats` `linalg` `matrix` `matrix_ops` `complex` `fraction` `bignum`
`geometry` `numtheory` `combinat` `calculus` `poly`
GAPS: `decimal` (exact money), `units` (novel idea 7), `bigfloat`. This area is close to complete.

### std.encoding (10) — HAS BUGS, see Part 3
`json_parse` `json_pretty` `jsonw` `csv_parse` `csvw` `base64` `base_convert` `encoding` `ini`
`xmlmini` `markdown`
GAPS: `toml` `yaml` `msgpack` `url` (partial) `hex` (builtin-ish) `utf8` `protobuf`.

### std.crypto (2) — thin
`sha256` (NIST-verified, real) `hash`
GAPS: `md5` `sha1` `sha512` `sha3` `hmac` `crc32` `aes` `chacha20` `rsa` `random_bytes` (secure).
Most are mechanical pure-zl — a good self-contained expansion area.

### std.algorithms (9)
`sorting` `sortx` `quicksort` `searching` `astar` `dijkstra` `graph_search` `huffman` `bloom`
GAPS: these want CLOSURES (item 11) to take comparators/predicates — right now they hardcode.

### std.random (2)
`randlib` `noise`
GAPS: the `randint` builtin is capped at lo+32767 (item 29 bug — fix first). Missing `choice`
`sample` `gauss` `distributions`.

### std.time (2)
`datetime` `date_calc`
GAPS: `sleep` (item 25), `duration`, `timer`/monotonic, `timezone`, `stopwatch`.

### std.graphics (5)
`bmp` `color` `easing` `noise` `graphx`
GAPS: `png` (everyone uses it, BMP nobody does), `svg`, `canvas`, `image` (load/resize/crop).

### std.cli (3)
`argparse` `ansi` `text_table`
GAPS: the `args` builtin does not exist (item 8) so argparse has nothing to parse. Missing
`terminal` (cursor/raw/size), `prompt`, `progress`, `spinner`, `color` output helpers.

### std.testing (2)
`testlib` `benchlib`
GAPS: needs `assert`-that-errors once error handling lands (item 5), `mock`, coverage.

### std.util (5)
`memo` `list_reduce` `listx` `expr_vars` `statemachine`
GAPS: `iter`/`itertools` (needs closures), `functools`, `uuid`(has it), `logging`, `config`.

### DEMOS — NOT stdlib, should move to examples/ (12)
`game_of_life` `sudoku` `tic_tac_toe` `brainfuck` `lisp_interp` `rpn_calc` `tiny_calc`
`maze_solve` `markov` `roman_numerals` `roman_math` `fibonacci`
These are programs, not reusable libraries. They inflate the "96 modules" count and belong in
examples/. Real count of genuine library modules is closer to **~84**.

---

## PART 2 — the missing CATEGORIES (whole areas with nothing)

Ranked by how badly a real program wants them (cross-ref FUNCTION_INVENTORY.md):

1. **std.net** — NOTHING. `http` `socket` `tcp` `udp` `dns` `url` `websocket`. Gated on FFI (item
   16). This is the single biggest missing category — no zl program can touch the network.
2. **std.os** — thin. `process` (run/spawn/kill — item 51), `path` (join/split/normalise),
   `fs` (copy/move/delete/walk/watch — item 50), `env`(✓), `args`(item 8). Mostly FFI-gated.
3. **std.concurrency** — NOTHING. `thread` `channel` `mutex` `atomic` `pool` `async` (item 18).
4. **std.io** — thin. Streaming reads/writes, buffered IO, `reader`/`writer` interfaces (needs
   item 38), stdin/stdout/stderr as streams. Today everything is whole-file.
5. **std.compress** — NOTHING. `gzip` `zlib` `zip` `deflate`. Pure-zl writable, FFI-free.
6. **std.db** — NOTHING. `sqlite` (FFI), a pure-zl `kvstore`, `csv-as-table`.
7. **std.log** — NOTHING. Levelled logging, formatters, sinks. Pure zl, cheap, high-value.
8. **std.serialize** — partial. A unified `encode`/`decode` over json/toml/msgpack.

---

## PART 3 — the QUALITY PASS (wave3 found these BROKEN today)

Before adding modules, the existing ones need fixing. wave3 (2026-08-03) confirmed:

- **`json_parse` and `json_pretty` ABORT** with a runtime error — their list-detection idiom was
  invalidated by today's `values_equal` fix (commit 9576640). **DEAD RIGHT NOW.** Highest priority.
- **`jsonw` emits INVALID JSON** — never escapes backslashes or control characters.
- **`csv_parse` silently MERGES records** — one unescaped quote swallows the next row.
- **`ini` write() breaks its own documented round-trip** — anonymous-section keys mis-filed.
- **`base64` decode** turns malformed input into arbitrary bytes, silently drops an unpadded group.
- **`strx.caesar()`** silently deletes characters on a negative shift.
- **`fmt.commas()`** mangles any non-integer (groups from the end of the whole string).

**Lesson: the 96 modules are almost entirely UNTESTED.** The 2,107 passing checks cover the ENGINE,
not the library. A stdlib test suite (one test file per module) is a roadmap item in its own right,
and it should come with imports — you cannot cleanly test a module you have to copy-paste.

---

## THE LIBRARY ROADMAP, ordered

1. **Imports (item 3)** — nothing else here is reachable without it. Prerequisite.
2. **Fix the broken modules** — json first (it is dead), then jsonw/csv/ini/base64. Quality before quantity.
3. **A stdlib test suite** — one test per module, so regressions like the json break are caught. Ships with imports.
4. **Reorganise** into the `std.<area>` tree; move the 12 demos to examples/.
5. **The cheap pure-zl expansions** (no FFI, no closures needed): crypto family (md5/sha1/crc32/hmac),
   compression (gzip/zip), more encodings (toml/url/utf8), `log`, `duration`.
6. **The closure-gated upgrades** (after item 11): give sorting/searching/graph real comparator and
   predicate arguments; add `iter`/`itertools`.
7. **The FFI-gated categories** (after item 16): `std.net`, `std.os.process`, `std.fs`, `std.db.sqlite`,
   terminal raw mode. This is where "zl controls the PC" becomes real, as libraries not engine work.
8. **The type-gated upgrades** (after generics, item 38): make collections TYPED — `list[int]`,
   `map[str, int]` — turning the O(n) dict/set into real typed containers.

**The pattern one more time:** the library splits cleanly into three tiers by what unblocks it —
**pure zl now** (crypto, compression, encodings, logging), **needs closures** (functional algorithms),
**needs FFI** (net, os, db). And ALL of it needs imports first. So the honest first move for the
library is not a new module at all — it is imports plus fixing the JSON that is broken today.
