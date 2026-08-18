# zl Language Reference

A quick reference for the zl language: syntax, operators, and the built-in
functions. (See `MASTER_PLAN.md` for the design, `README.md` for the build.)

## Syntax

Every construct below is exercised by **`examples/syntax_tour.zl`**, which is
the tiebreaker if this page and the language disagree — it either runs or it
does not:

```bash
./interp examples/syntax_tour.zl
```

The shape of the language in one breath: no declaration keyword, no type
annotations, blocks in `{ }` with no significant indentation, no parens
required around conditions, and word operators (`and` `or` `not`) instead of
`&& || !`.

```
# comment to end of line

x = 5                      # variables - no keyword
name = "Zac"               # strings
nums = [1, 2, 3]           # lists

print("hi " + name)        # + joins text

print(f"{name} is {x * 2}") # f-string: {expr} is real code, not a template
label = x > 3 ? "big" : "small"   # ternary; the untaken branch never runs

import strx                # runs stdlib/strx.zl - its fns become callable

if age >= 18 {             # blocks use { }
    print("adult")
} elif age >= 13 {         # 'elif' is a keyword; 'else if' also works
    print("teen")
} else {
    print("kid")
}

for n in nums { print(n) } # for-in over a list
i = 0
while i < 10 {             # while
    if i == 3 { continue } # skip this iteration
    if i > 6 { break }     # leave the loop
    i += 1                 # compound assignment
}

fn double(n) {             # functions
    return n * 2
}

nums[0] = 99               # index assignment (mutable lists)
grid = [[1, 2], [3, 4]]
grid[0][1] = 77            # nested index assignment

nums = push(nums, 4)       # push RETURNS the grown list - always reassign
```

### Three things that catch people

- **`push` is not reliably in-place.** A bare `push(xs, v)` can leave `xs`
  unchanged: the caller holds its own copy of the list header, so the grown
  length is only visible through the return value. Write `xs = push(xs, v)`.
  Same for anything that "modifies" a list. `xs[i] = v` *is* in-place.
- **Only parameters are frame-scoped.** Recursion works because each call gets
  fresh slots for its parameters — but a plain local inside a function is the
  same variable as a same-named local in its caller. Make helpers' working
  variables parameters, or give them distinct names.
- **`nil` is a builtin function, not a literal.** `x = nil()` is how you get
  it; `x = nil` is a runtime error. Likewise `pi()` and `e()`.

### Not in the language

No `&&` / `||` / `!` (the words `and` / `or` / `not` are the operators; a lone
`!` means "I mean it, this is dangerous"). No bitwise operators — `band`,
`bor`, `bxor`, `bnot`, `shl`, `shr` are builtins. No `switch`, no classes, no
exceptions, no `try`. No integer type: every number is a double, so integers
are exact to 2^53. No namespaces — `import` splices a module's definitions
straight into the global scope, which is why stdlib modules prefix their
names.

## Operators

| Kind | Operators |
|------|-----------|
| Arithmetic | `+` `-` `*` `/` `%` |
| Compare | `==` `!=` `<` `>` `<=` `>=` |
| Logic (words) | `and` `or` `not` · `true` `false` |
| Assignment | `=` · `+=` `-=` `*=` `/=` `%=` |
| Index | `x[i]` (read and write) |
| Danger marker | `!` (means "I mean it") |

## Keywords

All 16, and there are no others (`lexer.c:48-55`):

`if  elif  else  for  in  fn  return  while  not  and  or  true  false
break  continue  import`

Built-ins below are ordinary identifiers, not keywords — and so are `do`,
`loop`, `to`, `step` and `nil`, none of which are zl keywords.

## Built-in functions (93)

### I/O
`print(...)` · `input()` · `read(path)` · `write(path, text)` ·
`write_bytes(path, [bytes])` · `dir(path)` · `procs()`

### Numbers & math
`num(x)` · `int(x)` · `abs(x)` · `min(a,b)` · `max(a,b)` · `sign(x)` ·
`sqrt(x)` · `pow(x,y)` · `floor(x)` · `ceil(x)` · `round(x)` · `gcd(a,b)` ·
`sin(x)` · `cos(x)` · `tan(x)` · `log(x)` · `exp(x)` · `atan(x)` ·
`asin(x)` · `acos(x)` · `log2(x)` · `log10(x)` · `hypot(a,b)` · `trunc(x)` ·
`clamp(x,lo,hi)` · `fmod(a,b)` · `pi()` · `e()` · `hex(n)` · `sum(list)`

### Bitwise
zl has no bitwise *operators*; these builtins cover it. Arguments are taken
as 64-bit signed integers. `shr` is arithmetic (sign-filling); a shift count
outside `0..63` gives `0`.

A zl number is a double, so it holds every integer up to `2^53` exactly, some
above it, and nothing outside `+/-2^63`. These six builtins therefore either
give you the right bits or raise: an **operand** that is NaN or outside
`+/-2^63`, and a **result** the double cannot hold exactly, are both runtime
errors. They never round an answer and hand the rounded value back — that
used to make `bxor(x, 1)` a no-op above `2^53` and cost you any bit set in
the top 11 bits of a word. Size alone is fine: `shl(1,63)` is exact and
works. The same 53-bit ceiling already bounds a *literal*
(`4611686018427387903` parses as `...904`), so lifting it means giving zl an
exact integer type, not changing these builtins.

`int(x)` and `hex(n)` are conversions, not bit operations, and lose nothing,
so they never raise: `int()` truncates toward zero and stays defined on the
infinities, and `hex()` prints a value's exact digits past `2^63` rather
than saturating.

`band(a,b)` · `bor(a,b)` · `bxor(a,b)` · `bnot(a)` · `shl(a,n)` · `shr(a,n)`

### Strings
`len(s)` · `at(s,i)` · `code(s)` · `chr(n)` · `str(x)` · `slice(s,a,b)` ·
`find(s,sub)` · `has(s,sub)` · `starts(s,pre)` · `ends(s,suf)` ·
`upper(s)` · `lower(s)` · `trim(s)` · `repeat(s,n)` · `replace(s,old,new)` ·
`count(s,sub)` · `pad(s,width)` · `split(s,sep)` · `join(list,sep)` ·
`lines(s)` · `reverse(s)` · `ltrim(s)` · `rtrim(s)` · `title(s)` ·
`swapcase(s)` · `index_at(s,i)` (char code at `i`, `-1` if out of range)

### Lists
`len(xs)` · `push(xs,v)` · `first(xs)` · `last(xs)` · `insert(xs,i,v)` ·
`remove(xs,i)` · `contains(xs,v)` · `index_of(xs,v)` · `sort(xs)` ·
`reverse(xs)` · `sum(xs)` · `range(n)` / `range(a,b)` · `concat(a,b)` ·
`fill(n,v)` · `flat(xs)` (one level) · `take(xs,n)` · `drop(xs,n)`

### Types & misc
`type(x)` -> "num"/"str"/"bool"/"list"/"nil" · `bool(x)` · `nil()` ·
`assert(cond [, msg])` · `seed(n)` · `random()` [0,1) · `randint(lo,hi)` ·
`now()` (ms since process start) · `exit(code)` · `env(name)` (`""` if unset)

## Standard library (stdlib/*.zl)

64 modules, written in zl. A sample of the newer ones:

| Module | What it gives |
|--------|---------------|
| `stats.zl`   | mean, median, variance, stdev, min/max |
| `listx.zl`   | unique, take, drop, zip, flatten, chunk |
| `strx.zl`    | word_count, title_case, palindrome, caesar |
| `mathx.zl`   | factorial, lcm, is_prime, primes, clamp, power |
| `dict.zl`    | associative map (dset/dget/dhas/dkeys) |
| `matrix.zl`  | 2D matrices: identity, transpose, matmul |
| `vec.zl`     | vector math: add, dot, cross, length |
| `set.zl`     | set ops: union, intersect, difference |
| `randlib.zl` | shuffle, choice, dice, coin, sample |
| `jsonw.zl`   | value -> JSON text |
| `fmt.zl`     | commas, lpad, center, progress bars |
| `testlib.zl` | a check()/report() test framework |
| `bitops.zl`  | bitwise in pure zl: masks, popcount, to_bits/from_bits |
| `sortx.zl`   | merge/insertion/selection/counting sort, binary_search |
| `combinat.zl`| permutations, combinations, power_set, next_permutation |
| `deque.zl`   | double-ended queue + fixed-capacity ring buffer |
| `heapq.zl`   | binary min-heap / priority queue, heap_sort |
| `pathx.zl`   | basename, dirname, extname, path_join, normalize |
| `csvw.zl`    | CSV writer (the complement of csv_parse.zl) |
| `strsearch.zl`| KMP search, all-matches, longest_common_substring |
| `graphx.zl`  | topo_sort, has_cycle, connected_components, bipartite |
| `memo.zl`    | memoisation + an LRU cache |

Run any of them: `./interp stdlib/<name>.zl`

## Running / compiling

```
./interp prog.zl                # run on the tree-walking interpreter
./compile prog.zl               # -> out.c  (boxed C, then cl/clang)
./compilef prog.zl              # -> outf.c (unboxed long long - fast numeric)
./compilel prog.zl              # -> out.ll (LLVM IR, then clang -O2)
./nativegen prog.zl              # -> ./native_out (own x86-64, no C compiler)
```

Engine coverage differs. Only the interpreter and the boxed C backend run the
whole language; the unboxed engines (`compilef`, `compilel`, `nativegen`)
handle the integer subset — functions, `if`/`while`/`break`/`continue`, and
printing numbers and string literals. See `bench/README.md` for measured
speeds: unboxed engines are 30-80x the interpreter, the boxed C backend only
7-13x. That gap is the cost of boxing, and the reason for the type system.
