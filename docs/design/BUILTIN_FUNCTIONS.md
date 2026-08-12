# Built-in function vocabulary — what zl has, what a complete prelude needs

Compiled 2026-08-03. These are the "words" of the language — `print`, `len`, `push` and their
kin. zl ships **93 builtins today**. This lists them by category and marks every function a
complete built-in set should have. **✓ has · ✗ missing · ~ partial · [needs X] blocked on a feature.**

A builtin is a verb the language knows with no import. Some gaps below are true builtins waiting
to be written (cheap); others are blocked on closures, maps, or errors and become library
functions once those land — those are tagged.

---

## OUTPUT & INPUT
✓ `print` `input` `read` `write` `write_bytes` `env` `dir` `procs` `now` `exit`
✗ `println` — print controlling the trailing newline
✗ `eprint` / `eprintln` — write to stderr (stdout and stderr are one channel today)
✗ `printf` / `format` / `sprintf` — formatted output with `%d`, `{:.2f}` (item 36)
✗ `debug` / `inspect` — print a value with its structure and type, for debugging
✗ `read_line` `read_char` `read_bytes` `read_all` — granular input (only whole-line `input` today)
✗ `args` — **read command-line arguments (item 8). zl cannot write a CLI tool without it**
✗ `append` — add to a file without rewriting it

## TYPE & CONVERSION
✓ `bool` `int` `num` `str` `type` `nil`
✗ `float` — explicit float conversion (no float type distinct from num yet)
✗ `is_num is_str is_list is_bool is_nil is_fn` — type PREDICATES (only `type()==...` today)
✗ `parse` — string → typed value with failure (needs errors, item 5)
✗ `char` / `ord` — zl has `chr`/`code`, same idea, different names (fine)
✗ `sizeof` — byte size of a value (needs sized types, item 15)
✗ `clone` / `copy` — explicit deep copy

## MATH — zl is strong here
✓ `abs acos asin atan ceil cos exp floor fmod gcd hypot log log10 log2 pow round sign sin sqrt tan trunc`
✓ constants `e pi` · `clamp max min`
✗ `atan2` — two-argument arctangent (angle of a vector) — high value, tiny
✗ `sinh cosh tanh asinh acosh atanh` — hyperbolic
✗ `cbrt exp2 expm1 log1p` — extra exponentials
✗ `factorial comb perm lcm isqrt` — combinatorics/integer (stdlib has some)
✗ `prod` — product of a list (companion to `sum`)
✗ `degrees radians` — angle conversion
✗ `copysign` `nextafter` — sign/float manipulation
✗ `is_nan is_inf is_finite` — float classification (needed once floats are real)
✗ `lerp` — linear interpolation, a graphics/game staple
✗ `mean median stdev` — stats (stdlib/stats.zl ✓, not builtin)

## BITWISE
✓ `band bor bxor bnot shl shr` · `hex`
✗ `popcount` — count set bits (one CPU instruction)
✗ `clz ctz` — count leading / trailing zeros
✗ `rotl rotr` — bit rotation
✗ `byteswap` — endian flip (needed for binary formats and networking)
✗ `bit_get bit_set bit_clear bit_toggle` — single-bit access (stdlib/bitops.zl ✓)

## STRING — zl is rich here
✓ `at chr code contains count ends find has index_of index_at join len lines lower ltrim`
NOTE: `concat` is LIST-ONLY (`concat needs two lists`), not a string builtin — string joining is the
`+` operator exclusively. (corrected 2026-08-03 after a real-world probe caught the mislabel)
✓ `pad repeat replace reverse rtrim slice split starts swapcase title trim upper`
✗ `chars` — string → list of characters
✗ `bytes` — string → list of byte values
✗ `is_digit is_alpha is_alnum is_space is_upper is_lower` — character-class predicates (ctype gap)
✗ `capitalize` — first letter up, rest down (has `title` which does every word)
✗ `center ljust rjust zfill` — alignment/padding variants (`pad` is one direction)
✗ `replace_first` `replace_n` — replace only some (has `replace` = all)
✗ `rsplit split_n` — split from the right, or limited count
✗ `strip_prefix strip_suffix` — remove a known start/end
✗ `format` — the big one, `%`/`{}` substitution with width and precision (item 36)
✗ `matches` / `capture` — regex operations (item 28)

## LIST — has the basics, missing the functional family
✓ `concat contains count drop fill first flat has index_of insert last len push range`
✓ `remove repeat reverse slice sort sum take`
✗ `pop` — remove and return the last item (very common, cheap, no feature needed)
✗ `shift unshift` — remove/add at the front
✗ `extend` — append a whole list (has concat, which copies)
✗ `clear copy` — empty / duplicate
✗ `nth` — get item N with bounds handling
✗ `unique` / `dedup` — remove duplicates
✗ `chunk` — split into groups of N
✗ `window` — sliding windows of N
✗ `rotate` — shift elements around
✗ `zip` — pair up two lists — **[needs closures for the general form, but the 2-list case is cheap]**
✗ `sample shuffle` — random selection (stdlib/randlib.zl has shuffle)
--- ALL of these need CLOSURES (item 11), and are the single biggest ergonomic gap: ---
✗ `map` — transform every item
✗ `filter` — keep items matching a test
✗ `reduce` / `fold` — combine into one value
✗ `each` / `for_each` — run a function per item
✗ `find_by` — first item matching a test (has `find` for strings only)
✗ `any all none` — does any/every/no item match
✗ `enumerate` — pair each item with its index
✗ `sort_by min_by max_by` — sort/min/max using a key function
✗ `group_by partition` — split by a classifier
✗ `count_where index_where` — count/locate by a test
✗ `take_while drop_while` — take/skip while a test holds

## MAP — none exist yet (item 4); these are the vocabulary once maps land
✗ `keys values entries` — the three views
✗ `get` — lookup with a default for missing
✗ `set` / `put` — insert or update
✗ `has_key` `delete` `merge` `size`

## RANDOM
✓ `randint random seed` — but `randint` is capped at lo+32767 (item 29 bug), fix it
✗ `choice` — pick one random item from a list
✗ `choices sample` — pick several (with/without replacement)
✗ `uniform gauss` — random float in a range / normal distribution
✗ `shuffle` — stdlib only

## TIME
✓ `now`
✗ `sleep` — pause execution (item 25) — needed for almost any real tool
✗ `clock` / `monotonic` — high-resolution timer (for a profiler, item 30)
✗ `timestamp` — unix seconds
✗ `format_time parse_time` — human ↔ machine time (stdlib/datetime.zl partial)

## FILESYSTEM (item 50 — mostly FFI)
✓ `read write write_bytes dir`
✗ `exists size mtime is_dir is_file` — file info
✗ `delete rename copy move mkdir rmdir` — file management
✗ `read_lines read_bytes` — granular reads
✗ `walk glob` — tree traversal / pattern match
✗ `watch` — notify on change
✗ `tempfile cwd chdir` — working directory / temp

## PROCESS & SYSTEM (item 51 — FFI)
✓ `procs env exit`
✗ `run` / `exec` — launch a program and capture output — **the headline gap**
✗ `spawn` — launch without waiting
✗ `kill signal wait` — control a process
✗ `pid hostname username platform` — identity
✗ `setenv cpu_count total_memory` — system info

## ENCODING & HASH
✓ `hex` (~) · sha256 (stdlib, NIST-verified)
✗ `to_base64 from_base64` — stdlib/base64.zl (not builtin)
✗ `to_hex from_hex` — bytes ↔ hex string
✗ `url_encode url_decode` — stdlib partial
✗ `hash` — a general-purpose hash builtin
✗ `md5 sha1 sha512 crc32 hmac` — the rest of the hash family (mechanical, cheap)
✗ `json_encode json_decode` — stdlib/json (currently BROKEN, wave3 finding) — item promote

## ERROR & CONTROL (item 5)
~ `assert` — exists (stdlib), should be a builtin
✗ `error` / `panic` — raise a failure with a message
✗ `try catch` — or the errors-as-values equivalent
✗ `todo unreachable` — mark incomplete/impossible code (Rust-style, cheap and useful)
✗ `recover` — catch a panic

## FUNCTIONAL (item 11 — needs closures)
✗ `apply call` — invoke a function value with a list of args
✗ `compose` — chain two functions
✗ `partial curry` — pre-fill some arguments
✗ `memoize` — cache results (stdlib/memo.zl partial)

---

## THE SHAPE OF THE GAP

zl's 93 builtins are **strong on math, strong on strings, complete on basic list ops** — and
have three holes:

1. **The functional family** — `map filter reduce each enumerate zip any all sort_by` — ~15
   functions, ALL blocked on one feature (closures, item 11). This is the biggest and most-felt gap.
2. **The map vocabulary** — `keys values get set` — blocked on maps (item 4).
3. **OS verbs** — `run sleep exists delete kill args` — blocked on FFI (item 16), except `args`
   and `sleep` which are cheap standalones.

**Cheap wins that need NO feature — bankable any time, ~1-5 lines each:**
`pop shift unshift extend clear copy nth prod atan2 factorial lcm isqrt popcount clz ctz byteswap
chars bytes capitalize is_digit is_alpha choice sleep args println eprint todo unreachable
md5 sha1 crc32 to_hex from_hex` — roughly **30 builtins** that would round out the prelude without
touching the type system, closures, or FFI. A single "prelude polish" pass could ship all of them.

**The recurring number:** ~30 cheap standalone builtins, then everything else is downstream of the
same spine — closures, maps, errors, FFI.
