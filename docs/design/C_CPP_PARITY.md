# Every C and C++ keyword and library function vs zl

Compiled 2026-08-03. The complete inventory the roadmap is measured against. Status key:
**✓ has** · **✗ missing** · **~ partial** (exists but weaker) · **n/a** (does not fit zl's model,
by design). zl's 93 builtins and 96 stdlib modules were checked against each entry.

The headline before the detail: **zl's maths is nearly complete, its strings are rich, and its
control flow is solid. The gaps are almost entirely (1) types, (2) memory, (3) OS access, and
(4) the machinery C++ built to work around not having a garbage collector.**

---

## PART 1 — C KEYWORDS (all 44, C89 → C23)

### Control flow — zl is complete here
| C | zl |
|---|---|
| `if` `else` | ✓ (plus `elif`, which C lacks) |
| `while` `do` `for` | ✓ (plus `loop`, and `for i = a to b step n`) |
| `switch` `case` `default` | ✗ — item 10 |
| `break` `continue` | ✓ |
| `goto` | n/a — deliberately not wanted |
| `return` | ✓ |

### Types — zl's biggest gap
| C | zl |
|---|---|
| `int` `char` `short` `long` | ~ one number type, stored as double |
| `float` `double` | ~ interpreter only; fast backends silently truncate |
| `signed` `unsigned` | ✗ — item 15 (sized ints) |
| `_Bool` / `bool` | ✓ |
| `void` | n/a |
| `struct` | ✗ — item 4 |
| `union` | ✗ |
| `enum` | ✗ — item 12 |
| `typedef` / `type` | ✗ — item 23 (aliases) |
| `_Complex` `_Imaginary` | ~ stdlib/complex.zl |
| `_BitInt` (C23) | ✗ |
| `_Decimal32/64/128` | ✗ |

### Storage & qualifiers — mostly n/a for a dynamic language
| C | zl |
|---|---|
| `const` / `constexpr` (C23) | ✗ — item 12 |
| `static` | n/a |
| `extern` | ✗ — needed for FFI, item 16 |
| `register` | n/a |
| `auto` | n/a |
| `volatile` | ✗ — matters for hardware/kernel, Floor 6 |
| `restrict` | n/a |
| `inline` | n/a (backend decides) |
| `_Atomic` / `_Thread_local` | ✗ — item 18 (threads) |
| `_Alignas` `_Alignof` | ✗ — matters for FFI/structs |
| `_Noreturn` | ✗ |
| `_Static_assert` / `static_assert` | ~ `assert` is runtime only |
| `_Generic` | ✗ — a form of it comes via comptime, item 17 |
| `sizeof` | ✗ — needed for FFI/structs, item 15 |
| `typeof` (C23) | ~ `type()` is runtime, not compile-time |
| `nullptr` (C23) | ~ `nil` |
| `true` `false` (C23) | ✓ |
| `alignof` `thread_local` | ✗ |

**C keyword score: zl has ~18 of 44 outright, and most of the missing ones are the type and
memory keywords — exactly items 4, 12, 15, 16 on the roadmap.**

---

## PART 2 — C++ ADDITIONAL KEYWORDS (~40 beyond C)

### Object orientation
| C++ | zl |
|---|---|
| `class` | ✗ — records (item 4) + methods (item 39) together approximate it |
| `public` `private` `protected` | ✗ — item 23 |
| `virtual` | ✗ — interfaces (item 38) are the zl-shaped answer |
| `this` | ✗ |
| `friend` | n/a |
| `operator` (overloading) | ✗ — roadmap "operator overloading" |
| `explicit` `mutable` | ✗ |

### Memory & lifetime — the C++ machinery for having no GC
| C++ | zl |
|---|---|
| `new` `delete` | ✗ — item 40 (allocators) is zl's chosen shape instead |
| `nullptr` | ~ `nil` |
| smart pointers (`unique_ptr` etc.) | ✗ — see STL below |

### Templates / generics / compile-time
| C++ | zl |
|---|---|
| `template` `typename` | ✗ — generics, item 38-ish |
| `concept` `requires` (C++20) | ✗ — interfaces, item 38 |
| `constexpr` `consteval` `constinit` | ✗ — **comptime, item 17, is zl's version and potentially stronger** |
| `decltype` | ✗ |
| `auto` (type inference) | ✗ — the type system, Phase 7 |

### Errors
| C++ | zl |
|---|---|
| `try` `catch` `throw` | ✗ — item 5 (errors); zl may choose values-not-exceptions |
| `noexcept` | ✗ |

### Namespaces / modules
| C++ | zl |
|---|---|
| `namespace` `using` | ✗ — item 3 (imports) + item 23 |
| `module` `import` `export` (C++20) | ✗ — item 3 |

### Casts & RTTI
| C++ | zl |
|---|---|
| `static_cast` `dynamic_cast` `reinterpret_cast` `const_cast` | ~ zl is dynamically typed; conversions are `num`/`int`/`str` |
| `typeid` | ~ `type()` |

### Coroutines (C++20)
| C++ | zl |
|---|---|
| `co_await` `co_yield` `co_return` | ✗ — async, item 43; generators, item 20 |

### Char types
| C++ | zl |
|---|---|
| `wchar_t` `char8_t` `char16_t` `char32_t` | ✗ — real Unicode, item 13 |

**C++ keyword score: zl has almost none of these — but that is the point. Half are OOP machinery
zl replaces with records + interfaces + methods, and half are workarounds for C++ having no GC and
no comptime. zl's `comptime` (17) is arguably more powerful than `template` + `constexpr` combined,
because it is the whole language at compile time rather than a restricted sublanguage.**

---

## PART 3 — C STANDARD LIBRARY (by header)

### `<math.h>` — zl is NEARLY COMPLETE
✓ `sin cos tan asin acos atan exp log log10 log2 pow sqrt ceil floor fmod round trunc hypot`
✓ `fabs`→`abs`, plus `gcd sign pi e` (some are C++17 or non-standard extras)
✗ `atan2 sinh cosh tanh asinh acosh atanh cbrt exp2 expm1 log1p frexp ldexp modf nan nextafter copysign`
✗ `isnan isinf isfinite` — classification predicates
**Verdict: the everyday maths is all there; the gaps are hyperbolic, decomposition, and float
classification. ~15 small builtins would close it.**

### `<string.h>` — zl covers the common cases differently
✓ `strlen`→`len`, `strstr`→`find`/`contains`, `strtok`→`split`, `strcmp`→ `==`/`<`, `strcat`→`+`
✓ zl adds: `at code chr concat count ends index_of index_at join lines lower ltrim pad repeat replace reverse rtrim slice starts str swapcase title trim upper`
✗ `memcpy memmove memset memcmp` — no raw memory (item 16/50 territory)
✗ `strdup strncpy strncat strspn strcspn strpbrk` — mostly subsumed by slice/find
**Verdict: strings are a strength. The gaps are the raw-memory functions, which belong with FFI.**

### `<stdio.h>` — zl is WEAK here
~ `printf`→`print` but NO format string (`%d`, `%5.2f`) — item 36
~ `fopen`/`fread`/`fwrite`→`read`/`write`/`write_bytes` but WHOLE-FILE ONLY, no streaming/seeking
✓ `getchar`/`gets`→`input` (line-based)
✗ `scanf sscanf sprintf snprintf fprintf` — no formatted I/O
✗ `fseek ftell rewind feof ferror fflush setvbuf` — no stream cursor
✗ `remove rename tmpfile` — no file management (item 50)
✗ `perror` — no errno
**Verdict: file I/O is read-it-all / write-it-all. Streaming, seeking, and formatted output are
all missing — items 26, 36, 50.**

### `<stdlib.h>` — mixed
✓ `atoi atof`→`num`/`int`, `rand`→`random`/`randint`, `srand`→`seed`, `qsort`→`sort`, `abs`✓,
  `exit`✓, `getenv`→`env`, `strtol strtod`→`num`
✗ `malloc calloc realloc free` — NO manual memory (Phase 5, item 40)
✗ `bsearch` — stdlib/searching.zl has it
✗ `system` — CANNOT launch a program (item 51)
✗ `atexit abort qsort_r div ldiv` 
✗ `mblen mbtowc` — no multibyte (item 13)
**Verdict: the memory functions and `system()` are the notable gaps — both high on the roadmap.**

### `<ctype.h>` — MISSING as builtins
✗ `isalpha isdigit isalnum isspace isupper islower ispunct isxdigit iscntrl isprint isgraph`
✗ `toupper tolower` — zl has `upper`/`lower` on whole strings, not per-char
**Verdict: no character-class predicates. stdlib/string_analysis.zl covers some; ~12 small
builtins or one `ctype` module would close it.**

### `<time.h>` — PARTIAL
✓ `time`→`now`
~ `localtime gmtime mktime strftime` → stdlib/datetime.zl and date_calc.zl (partial)
✗ `clock` — no CPU-time / high-res timer (needed for a profiler, item 30)
✗ `difftime asctime ctime nanosleep` — no `sleep` (item 25)
**Verdict: has wall-clock time, lacks a monotonic timer and sleep.**

### Smaller headers
| header | zl |
|---|---|
| `<assert.h>` `assert` | ✓ (runtime) |
| `<errno.h>` `errno` `strerror` | ✗ — tied to error handling, item 5 |
| `<stdarg.h>` `va_start` etc. | ✗ — varargs, roadmap |
| `<setjmp.h>` `setjmp`/`longjmp` | ✗ — non-local jumps |
| `<signal.h>` `signal`/`raise` | ✗ — signal handling |
| `<locale.h>` | ✗ |
| `<stdint.h>` `int8_t`…`uint64_t` | ✗ — **item 15, the FFI prerequisite** |
| `<stdbool.h>` | ✓ |
| `<limits.h>` `<float.h>` | ~ some constants; no `INT_MAX` etc. |
| `<inttypes.h>` `<wchar.h>` `<wctype.h>` | ✗ |
| `<threads.h>` (C11) | ✗ — item 18 |
| `<stdatomic.h>` | ✗ |

---

## PART 4 — C++ STANDARD LIBRARY (the STL)

### Containers — zl has some as stdlib MODULES, none as language types
| C++ | zl |
|---|---|
| `std::vector` | ✓ the list is this |
| `std::string` | ✓ |
| `std::array` | ✗ — fixed-size, needs sized types |
| `std::map` `std::unordered_map` | ~ stdlib/dict.zl (O(n)!), needs item 4/maps |
| `std::set` `std::unordered_set` | ~ stdlib/set.zl, stdlib/hashset.zl |
| `std::pair` `std::tuple` | ✗ — item 4 (records/tuples) |
| `std::optional` | ✗ — item 5b (nullable) |
| `std::variant` `std::any` | ✗ — tagged unions / enums |
| `std::deque` | ~ stdlib/deque.zl |
| `std::list` `std::forward_list` | ~ stdlib/linkedlist.zl |
| `std::stack` `std::queue` `std::priority_queue` | ~ stdlib/stack_queue.zl, heapq.zl |
| `std::bitset` | ~ stdlib/bitset.zl |
| `std::span` `std::string_view` | ✗ — no non-owning views (needs references, item 42) |
| `std::bitset` `std::valarray` | ~ / ✗ |
**Verdict: most containers exist as O(n)-ish stdlib modules but NOT as first-class typed
containers. maps (item 4) and generics (item 38) turn these from library hacks into real types.**

### `<algorithm>` — zl has the common ones via stdlib
✓ `sort`✓ (builtin), `find`→`find`/`index_of`, `count`→`count`, `reverse`→`reverse`,
  `min`/`max`✓, `binary_search`→stdlib/searching.zl, `accumulate`→`sum`
~ `lower_bound upper_bound merge unique rotate` → various stdlib modules
✗ `transform copy for_each generate` — no map/filter builtins (need closures, item 11!)
✗ `all_of any_of none_of` — need closures
✗ `next_permutation shuffle sample partition nth_element clamp`(✓ has clamp)`iota gcd`(✓)`lcm`
**Verdict: the killer gap is `transform`/`for_each`/`all_of` — all need CLOSURES (item 11). Without
functions-as-values, zl has sort but not map/filter/reduce-with-a-lambda.**

### Smart pointers & memory
✗ `unique_ptr shared_ptr weak_ptr make_unique make_shared` — these are C++'s no-GC machinery;
  zl's answer is allocators (item 40) or whatever Phase 5 decides, not reference-counted pointers
✗ `std::move std::forward` — no move semantics (zl values are already copy-or-share)
✗ `std::allocator` — item 40

### Utilities
✓ `std::swap`→ needs item 34; `std::hash`→ stdlib/hash.zl + real sha256
✗ `std::function std::bind` — CLOSURES, item 11
✗ `std::initializer_list std::tie std::get std::ref` — tied to tuples/records
✗ `std::to_string`→`str`✓, `std::stoi std::stod`→`num`/`int`✓

### `<iostream>` / `<fstream>` / `<sstream>`
✗ `cout cin cerr` streams, `<<`/`>>` operators — zl uses `print`/`input`
✗ `ifstream ofstream stringstream getline` — no stream abstraction (item 26)

### `<thread>` / `<mutex>` / `<atomic>` / `<future>`
✗ ALL of it — `thread mutex lock_guard unique_lock condition_variable atomic future promise async`
✗ `std::this_thread::sleep_for` — no sleep (item 25)
**Verdict: zero concurrency. Item 18 (threads) + the primitives around it.**

### Modern headers
| C++ | zl |
|---|---|
| `<chrono>` | ✗ — no duration types (novel idea 7: compile-time units) |
| `<filesystem>` (C++17) | ✗ — item 50 |
| `<regex>` | ~ stdlib/regex_match.zl (basic, no capture groups) — item 28 |
| `<random>` (`mt19937`, distributions) | ~ `random`/`randint` but capped at 32767 (item 29 bug) |
| `<format>` (C++20) | ✗ — item 36 |
| `<ranges>` (C++20) | ✗ — needs closures + interfaces |
| `<coroutine>` (C++20) | ✗ — item 43 |
| `<complex>` | ~ stdlib/complex.zl |
| `<numeric>` | ~ `sum`, stdlib/mathx.zl |
| `<expected>` (C++23) | ✗ — this is exactly item 5's errors-as-values shape |

---

## THE SUMMARY

Everything missing sorts into six buckets, and every bucket is already a roadmap item:

1. **Types** — struct, union, enum, sized ints, typedef, const → items 4, 12, 15, 23
2. **Memory** — malloc/free, smart pointers, allocators → Phase 5, item 40
3. **OS access** — file management, `system()`, streams, sleep, threads → items 16, 18, 25, 26, 50–66
4. **Closures** — `std::function`, `transform`, `for_each`, callbacks → item 11 (blocks map/filter/reduce)
5. **Errors** — try/catch, errno, `expected`, `optional` → items 5, 5b
6. **Formatting & Unicode** — format strings, wide chars, locales → items 13, 36

**Two things carry disproportionate weight:**
- **Sized integers (item 15)** — unlocks FFI, which unlocks all of OS access (bucket 3, ~20 entries)
- **Closures (item 11)** — unlocks the entire `<algorithm>` family (map, filter, reduce, all_of)

**And one reframing:** roughly a third of C++'s standard library — smart pointers, move semantics,
`std::allocator`, most of `<memory>` — exists ONLY because C++ has no garbage collector and no
comptime. zl does not need to copy those; it needs to make ONE good memory decision (Phase 5) and
it gets comptime (item 17) for free from having an interpreter in the compiler. So the real target
is not "reimplement the STL" — it is **types + closures + one memory model + FFI**, and the rest is
zl library code.
