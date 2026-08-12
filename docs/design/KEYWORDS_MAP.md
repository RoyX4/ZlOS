# Every Python/C/C++ keyword, mapped into zl's plan

Compiled 2026-08-03. Takes all ~150 keywords across Python (35), C (44) and C++ (~97, minus the
overlaps), and assigns each a status in zl. The payoff is the count at the bottom: **how many
keywords zl actually ends up with** once the whole language is built. Answer up front: **~26** -
still under Python, half of C, a quarter of C++.

Status legend:
- **HAS** - zl already has it as a keyword
- **fn** - zl does this with a plain function, not a keyword (the zl way)
- **+kw** - zl will ADD this as a new keyword (counts toward the final total)
- **feature** - zl gets the capability via a roadmap feature, NOT a new keyword
- **OS** - only needed at the OS/hardware tier (Floor 6-7), deferred
- **no** - deliberately not wanted

---

## Control flow
| keyword | langs | zl |
|---|---|---|
| `if` `else` | all | **HAS** |
| `elif` | Py, zl | **HAS** (C/C++ use `else if`) |
| `while` `for` `break` `continue` `return` | all | **HAS** |
| `in` | Py, zl | **HAS** |
| `do` | C, C++ | **HAS** (zl has do-while already) |
| `switch` `case` `default` | C, C++ | **+kw** `switch` only - `case`/`default` avoided by reusing `{}`/`to`/`in`/`else` (item 10) |
| `goto` | C, C++ | **no** - deliberately rejected, the one universally-regretted keyword |
| `pass` | Py | **fn** - not needed, zl uses `{}` |

## Functions
| keyword | langs | zl |
|---|---|---|
| `fn` (`def`) | zl (Py) | **HAS** |
| `return` | all | **HAS** |
| `lambda` | Py | **feature** - anonymous `fn(x){...}` sugar (item 21), reuses `fn`, no new keyword |
| `yield` | Py, C++(co_) | **feature** - generators (item 20) |
| `inline` | C, C++ | **fn** - the backend decides, not the programmer |
| `operator` | C++ | **feature** - operator overloading, likely via a function name convention |
| `noexcept` `explicit` `mutable` `friend` | C++ | **no** - C++ machinery zl does not reproduce |
| `this` | C++ | **feature** - methods (item 39), likely an implicit receiver |

## Logic & values
| keyword | langs | zl |
|---|---|---|
| `and` `or` `not` | Py, C++, zl | **HAS** |
| `true` `false` | all | **HAS** |
| `nil` (`None`/`nullptr`) | zl | **HAS** (as `nil()`) |
| `is` | Py | **feature** - identity vs equality; may add if needed, low priority |
| `xor bitand bitor compl and_eq ...` | C++ | **fn** - zl's bitwise are functions (`band bor bxor bnot`), no keywords |

## Types (zl's biggest growth area)
| keyword | langs | zl |
|---|---|---|
| `int char float double void short long` | C, C++ | **feature** - one number type now; sized types via item 15 |
| `signed` `unsigned` | C, C++ | **+kw**-ish - `u8 i32 u64` etc as type names (item 15); not 7 separate keywords |
| `bool` (`_Bool`) | C, C++ | **HAS** |
| `struct` | C, C++ | **+kw** `rec` (item 4) - records |
| `union` | C, C++ | **+kw** for tagged unions (with enums) |
| `enum` | C, C++ | **+kw** `enum` (item 12) |
| `typedef` | C | **+kw** `type` - aliases (item 23) |
| `const` | C, C++ | **+kw** `const` - constants (item 12) |
| `auto` `decltype` `typename` | C++ | **feature** - type inference (the type system), no keyword |
| `wchar_t char8_t char16_t char32_t` | C++ | **feature** - real Unicode (item 13), not 4 char types |
| `sizeof` `alignof` `alignas` | C, C++ | **feature**/`OS` - need sized types first (item 15) |

## Memory (the OS/systems tier)
| keyword | langs | zl |
|---|---|---|
| `new` `delete` | C++ | **feature** - zl's memory model is allocators (item 40), not new/delete |
| `volatile` | C, C++ | **OS** - hardware memory that changes on its own; needed for drivers/OS |
| `register` | C, C++ | **no** - the backend allocates registers |
| `static` `extern` | C, C++ | **feature** - `extern` folds into FFI (item 16); `static` not needed |
| `restrict` | C | **no** - a C aliasing hint zl does not expose |
| `_Atomic` `thread_local` (`_Thread_local`) | C, C++ | **feature** - threads (item 18) |
| `!` (raw-memory danger marker) | zl | **+kw**-ish - a MARKER, not a word; already in zl's design for OS work |

## OOP & abstraction
| keyword | langs | zl |
|---|---|---|
| `class` | C++ | **feature** - records + methods together (items 4, 39) |
| `public` `private` `protected` | C++ | **+kw** `pub` - one visibility marker, not three (item 23) |
| `virtual` | C++ | **feature** - interfaces (item 38) |
| `template` `concept` `requires` | C++ | **feature** - generics + interfaces (items 38) and comptime (17) |

## Errors
| keyword | langs | zl |
|---|---|---|
| `try` `catch`/`except` `throw`/`raise` `finally` | Py, C++ | **feature** - errors-as-values (item 5); may add `try` as sugar for `?` |

## Imports & scope
| keyword | langs | zl |
|---|---|---|
| `import` | Py, C++, (zl item 3) | **+kw** `import` (item 3) |
| `from` `as` `using` `namespace` `export` `module` | Py, C++ | **+kw** - `from`/`as` likely; the rest fold into imports (item 3) |
| `global` | Py | **+kw** `global` - the scoping decision (2026-08-03) adds exactly this |
| `nonlocal` | Py | **feature** - closures (item 11); may not need a keyword |
| `del` | Py | **fn** - a function if needed, not a keyword |
| `with` | Py | **feature** - `defer` (item 14) covers the cleanup use |

## Compile-time & modern
| keyword | langs | zl |
|---|---|---|
| `constexpr` `consteval` `constinit` `_Generic` `static_assert` | C++, C | **feature** - `comptime` (item 17), ONE keyword replacing all of these |
| `co_await` `co_yield` `co_return` `async` `await` | C++, Py | **feature** - async (item 43); `async`/`await` may become 2 keywords |
| `assert` | Py, zl(fn) | **fn** - zl has it as a function |
| `ai` (compile-time AI) | zl-novel | **+kw** - the Floor 2 differentiator (NOVEL_IDEAS) |
| casts: `static_cast dynamic_cast reinterpret_cast const_cast typeid` | C++ | **fn** - zl uses `num`/`int`/`str`/`type` functions |

---

## THE COUNT — what zl ends up with

Starting from 15, the keywords zl will ADD (`+kw`):

    switch  rec  enum  union  type  const  pub  import  from  as  global  ai
    async  await                                  (+ the `!` marker, not a word)

That is **~14 new keywords**, for a total around **26-29** for the WHOLE language including OS work.

Compare:
| language | keywords | does it do OS/systems? |
|---|---|---|
| **zl (projected, complete)** | **~26** | yes |
| Python | 35 | no |
| C | 44 | yes |
| C++ | ~97 | yes |

**The headline: zl aims to be a complete systems language - structs, generics, errors, async,
comptime, raw memory, OS-capable - with FEWER keywords than Python, which does none of that.**

How? Three moves, visible all through this table:
1. **Functions, not keywords** - `print len push assert del sizeof-once-typed` and all bitwise ops
   are functions. C++ makes `operator`, `typeid`, casts into keywords; zl makes them functions.
2. **One keyword for a family** - `comptime` replaces `constexpr`+`consteval`+`constinit`+`_Generic`+
   `static_assert` (5 -> 1). `pub` replaces `public`+`private`+`protected` (3 -> 1). Sized types are
   type NAMES (`u8`), not 7 keywords (`signed`/`unsigned`/`short`/`long`/...).
3. **Reuse what exists** - `switch` reuses `to`/`in`/`else`/`{}`; `defer` covers `with`; the type
   system covers `auto`/`decltype`/`typename` with no keyword at all.

The one deliberate rejection is `goto` - the single keyword almost every language regrets shipping.

**This is the real design achievement to aim for: C++ capability at a quarter of C++'s keyword
count, and below Python's.** Every "+kw" above should be challenged - can it be a function, a reused
keyword, or a type name instead? - before it's allowed to grow the count.
