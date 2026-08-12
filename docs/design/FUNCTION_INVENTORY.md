# Functions from every major language vs zl

Compiled 2026-08-03, extending C_CPP_PARITY.md beyond C/C++ into Python, Rust, Go, JavaScript,
and the systems/domain functions any modern language ships. **✓ has · ✗ missing · ~ partial.**

The same two multipliers keep reappearing: **closures (item 11)** unlocks the whole
map/filter/reduce/enumerate family across ALL of these languages at once, and **structs/maps
(item 4)** unlock the collection methods. Almost nothing here is its own feature — it is library
code waiting on one of those two.

---

## PYTHON built-ins & stdlib

### Built-in functions
✓ `len sum min max abs round range print input sorted`(sort)`type exit hex`(hex/str)`ord`(code)`chr`
✗ **`enumerate zip map filter reduce any all` — the functional core, all need CLOSURES (item 11)**
✗ `sorted(key=...)` `min/max(key=...)` — sort/min/max WITH a function, needs closures
✗ `reversed` `bin oct` `repr ascii format` `isinstance id hash`(~) `iter next` `open`(streaming)
✗ `dict() set() tuple() frozenset()` — the type constructors (item 4)
✗ `divmod pow(mod) complex bytes bytearray memoryview vars dir(introspection) globals locals eval exec`
Note: `eval`/`exec` zl gets nearly free — the interpreter IS an evaluator (ties to comptime, item 17)

### String methods zl lacks
✗ `zfill center ljust rjust`(~pad) `partition rpartition casefold expandtabs translate maketrans`
✗ `isdigit isalpha isalnum isspace isupper islower istitle` — per-string predicates (ctype gap)
✗ `format() % formatting encode decode` — format specifiers (item 36)
✓ has: `startswith endswith strip split join replace find count title upper lower splitlines`

### List / dict / set methods zl lacks
✗ list: `pop extend copy clear` (has push/insert/remove/index/count/sort/reverse)
✗ dict: `keys values items get setdefault update pop popitem` — ALL need maps (item 4)
✗ set: `union intersection difference symmetric_difference issubset add discard` — stdlib/set.zl partial

### `itertools` — none of it (all need closures + generators)
✗ `chain combinations permutations product groupby accumulate cycle repeat takewhile dropwhile islice starmap tee zip_longest count`

### `functools`
✗ `reduce partial lru_cache`(memoize) `cmp_to_key wraps` — stdlib/memo.zl covers caching partially

### `collections`
✗ `Counter defaultdict OrderedDict namedtuple ChainMap` — Counter and namedtuple are high-value,
  both need maps/records (item 4)

### `math` / `statistics` extras
✗ `factorial comb perm isqrt prod dist gcd`(✓)`lcm` — stdlib/numtheory.zl, combinat.zl have some
✗ statistics: `mean median mode stdev variance quantiles` — stdlib/stats.zl covers these ✓~

### `random` extras
✗ `choice choices sample shuffle uniform gauss randrange` — stdlib/randlib.zl has shuffle;
  and `randint` is capped at lo+32767 (item 29 bug)

### `os` / `sys`
✓ `environ`(env) `listdir`(dir) `exit`
✗ `getcwd mkdir rmdir remove rename path.join path.exists path.split walk getpid` — item 50
✗ `sys.argv` — **CANNOT read arguments (item 8)**, `sys.platform stdin/stdout/stderr`
✗ `system popen subprocess` — CANNOT launch a program (item 51)

### Language constructs
✗ comprehensions (item 31) · generators/`yield` (item 20) · decorators · `with` context managers
✗ `*args **kwargs` unpacking (item 37) · slicing with step (item 33)

---

## RUST std

### Iterator adapters — THE crown jewel, and zl has NONE (all need closures)
✗ `map filter fold collect enumerate zip chain take skip rev flatten flat_map windows chunks
   step_by scan peekable cycle take_while skip_while` 
✗ `sum product min_by_key max_by_key position find any all count last nth partition`
This one family is the single biggest ergonomic gap versus every modern language.

### `Option` / `Result` — item 5 / 5b
✗ `Some None Ok Err ? unwrap unwrap_or expect map_or and_then ok_or is_some is_none`
✗ the `?` operator — early-return-on-error, the thing that makes Rust error handling pleasant

### `Vec` / slice methods zl lacks
✗ `drain retain dedup split_at truncate resize swap rotate_left split_first chunks windows`
✓ has push/insert/remove/contains/reverse/sort/len/concat

### `String`
✗ `chars bytes char_indices split_whitespace parse to_uppercase`(has upper)`push_str`
✓ has lines/trim/split/replace/etc.

### Traits — item 38 (interfaces)
✗ `Iterator Display Debug Clone PartialEq Ord Hash From Into Default TryFrom`
These are how Rust makes custom types work with the stdlib; zl's interfaces (item 38) are the analog

### Macros
✗ `println! format! write! vec! matches! assert_eq!` — comptime (item 17) may cover some

---

## GO std

### Concurrency — zl has ZERO (item 18)
✗ goroutines `go f()` · channels `chan` · `select` · `sync.Mutex sync.WaitGroup sync.Once`
✗ `context` · `defer`(item 14) · `panic`/`recover`

### `strings` / `strconv` / `fmt`
✓ Contains HasPrefix Split Join Replace TrimSpace Repeat Index Count
✗ `Fields`(split on whitespace) `Map`(needs closure) `strings.Builder` `Title`
✗ `strconv.Atoi Itoa ParseFloat FormatFloat Quote` — mostly covered by num/str
✗ `fmt.Printf Sprintf Errorf Sscanf` — formatted I/O (item 36)

### `sort` / `io` / `os`
✗ `sort.Slice`(closure) `sort.Search`
✗ `io.Reader io.Writer bufio` — the stream interface abstraction (item 26 + interfaces)
✗ `os.Open Create ReadFile WriteFile Mkdir Remove Args`(item 8)`Getenv`(✓)
✗ `errors.New errors.Is errors.As fmt.Errorf %w` — wrapped errors (item 5)

### Big stdlib packages zl lacks entirely
✗ `net/http net encoding/json`(✓~)`regexp time crypto/* sync context bufio bytes`

---

## JAVASCRIPT

### Array methods — same functional family (closures, item 11)
✗ `map filter reduce reduceRight forEach find findIndex some every flatMap entries keys values`
✗ `splice shift unshift from of isArray`
✓ has includes(contains)/indexOf/slice/concat/flat/sort/reverse/join/push/fill/at

### String
✗ `replaceAll substring substr match matchAll normalize localeCompare padStart padEnd`(~pad)
✓ has split/slice/indexOf/replace/trim/repeat/toUpperCase/startsWith/endsWith/includes/charAt/charCodeAt

### Object / Map / Set — need maps (item 4)
✗ `Object.keys values entries assign freeze` · `Map Set WeakMap`

### Async & timers
✗ `Promise async await setTimeout setInterval fetch queueMicrotask` — item 43 + item 25 (sleep)

### Number / Math / misc
✗ `parseInt parseFloat toFixed`(item 36)`Number.isInteger isNaN toString(radix)`
✗ `?.` optional chaining · `??` nullish · destructuring · spread · arrow functions(item 21)

---

## SYSTEMS & DOMAIN — things every serious stdlib ships

### Hashing & crypto
✓ `sha256` (pure zl, passes NIST) — genuinely strong
✗ `md5 sha1 sha512 sha3 hmac crc32 crc16` · `aes rsa chacha20` · `random_bytes`(secure) · `bcrypt`

### Encoding
✓ `base64` `hex`(~) `json`(✓ stdlib, currently broken — wave3 finding) `csv`(✓)
✗ `url encode/decode`(stdlib partial)`utf8 encode/decode base32 base58 hex-dump protobuf msgpack`

### Compression
✗ `gzip zlib deflate lz4 zstd` — none

### Bit manipulation
✓ `band bor bxor bnot shl shr` (builtins)
✗ `popcount`(count set bits)`clz ctz`(leading/trailing zeros)`rotate byteswap parity` — hardware ops

### Time
✓ `now`
✗ `sleep`(item 25)`monotonic`(profiler, item 30)`format parse timezone duration` — chrono equivalents

### Terminal
~ stdlib/ansi.zl (colors)
✗ `cursor move clear raw-mode key-read terminal-size` — no interactive TUI (item 25-ish)

### Networking — all of it (item 27, via FFI item 16)
✗ `socket connect listen accept send recv` · `http_get http_post` · `dns_resolve ping` · `websocket`

### Filesystem — beyond read/write (item 50)
✗ `stat glob walk watch symlink copy move mkdir tempfile chmod exists size mtime`

### Process (item 51)
✗ `spawn exec pipe kill signal wait exit_code` — can see procs, can't control them

### Regex (item 28)
~ stdlib/regex_match.zl (basic)
✗ capture groups · named groups · replace-with-pattern · split-by-regex · backreferences

---

## THE PATTERN, ONE MORE TIME

Across FIVE languages the same handful of unlocks keeps appearing:

| Unlock | What it opens across all languages |
|---|---|
| **Closures (item 11)** | map · filter · reduce · enumerate · zip · sort-by-key · all iterator adapters · callbacks. THE biggest single ergonomic win — it appears in Python, Rust, Go, JS simultaneously |
| **Maps + structs (item 4)** | dict · Counter · namedtuple · Object · HashMap · every keyed collection |
| **Errors (item 5)** | Option · Result · `?` · try/catch · errors.Is · expected |
| **FFI (item 16)** | networking · process control · filesystem · terminal · crypto libs — bucket 3 |
| **Sized ints (item 15)** | the gate in front of FFI, plus popcount/clz/byteswap and binary formats |
| **comptime (item 17)** | macros · `eval`/`exec` (nearly free — interpreter is in the compiler) |
| **Threads (item 18)** | goroutines · async · channels · Promise · all concurrency |

**Concrete small wins worth banking regardless** (each is a handful of builtins, no big feature
needed): `pop clear` on lists · `enumerate zip` (once closures exist) · character predicates
(`isdigit` etc.) · `factorial lcm isqrt` · `popcount clz ctz byteswap` · `sleep` · `md5 sha1 crc32`
· `url encode/decode` · fixing `randint`'s 32767 cap (item 29).

**The strategic read is unchanged and now proven across the whole industry:** zl does not need to
reimplement five standard libraries. It needs **closures, maps/structs, errors, FFI, sized ints,
comptime, threads** — seven features — and every standard-library function above becomes zl code
written on top of them.
