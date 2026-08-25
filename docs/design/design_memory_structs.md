# Design: Raw Memory and Structs for zl

**Status:** proposal
**Author:** language/compiler design pass, 2026-08-01
**Scope:** W5 "systems capability" — the raw-memory/struct half. Companion to
`design_ffi_syscalls.md` (the FFI half). Hard prerequisite for `design_kernel.md`
(W6) and `design_game_system.md` (W8), both of which are already written against
the primitives specified here. No code is changed by this document.

---

## 0. TL;DR

The dilemma this feature is usually killed by:

> A struct that is **boxed** cannot be handed to C — its bytes are `Value`
> structs with a `double num` and three pointers, not the layout `WNDCLASSA`
> needs. A struct that is **unboxed** does not fit in a `Value` — there is no
> field in `src/runtime/interp.c:34`'s `Value` that can hold 72 arbitrary bytes.

The way out is to refuse the dilemma:

> **A struct is never a value. A struct is an ADDRESS, and `struct` is a
> compile-time name for an offset table.**

`p.x` compiles to a sized load at `p + 0`. `p` itself is an ordinary zl number
holding a base address. Nothing enters the `Value` union, `src/runtime/runtime.h` is
untouched, the native tag scheme is untouched, and the interpreter's `double num`
holds the address **exactly** — Win64 user-mode virtual addresses are 47 bits and
2^47 < 2^53 with six bits to spare (§2.2). The `double num` weakness is thereby
confined to exactly one documented place, `peek64` of a value above 2^53 (§3.2),
which is the same place `design_kernel.md` §2 already found it.

The feature is three layers:

1. **Raw memory** — `alloc` `reserve` `peek8/16/32/64` `poke8/16/32/64`
   `copy_mem` `fill_mem` `sext`. Plain builtins, no grammar change.
   `design_game_system.md` is already written against exactly this API and
   nothing else, and `design_kernel.md` §11 risk 1 names these exact spellings as
   its dependency.
2. **`struct`** — a declaration that computes offsets and `sizeof` at compile
   time and lowers `.field` to one sized load. Zero new reserved words
   (contextual, the `extern` trick of `design_ffi_syscalls.md` §2.5a). This is
   the layer that earns its keep: `poke32(p + 9, v)` is a bug you cannot see;
   `p.style = v` is the same instruction with the offset computed for you.
3. **Fixed arrays and arrays of structs** — `world[i].hp`, lowered to
   `base + i*sizeof(T) + off`.

**One layout algorithm, in one file, used by all five engines.** The C backends
must *not* emit a real C `struct` and lean on MSVC's `offsetof`, even though they
could — §5.1 argues that at length, and it is the single most consequential
decision here after §2.

Costs paid honestly and up front: reference semantics only (no by-value struct
arguments, no struct returns — already non-goals in `design_ffi_syscalls.md` §8);
`free` is a no-op in v1 and reclamation is by region reset; nothing is
bounds-checked by default; `src/backends/native/nativegen.c` has no data section and no heap at all
today, so it grows both.

---

## 1. Motivation

### 1.1 What W5 actually asks for

`docs/archive/prompts/OVERNIGHT_CAMPAIGN.md` W5, lines 83–84:

> - raw memory: `alloc(n)`, `free(p)`, `peek(p)`, `poke(p,v)`, `peek8/poke8`, pointer arithmetic
> - structs / records with a fixed memory layout, and fixed-size typed arrays

Those two lines are this document. The other four lines of W5 are
`design_ffi_syscalls.md`. Neither half is useful alone: FFI without structs can
only call functions that take scalars, and structs without FFI are a faster way
to store a `Point`. Together they are the whole systems tier — which is why W6
and W8 both list *both* as blockers and neither has started.

### 1.2 The syntax already exists, and shipping it is keeping a locked promise

Three facts that make this a smaller change than it sounds:

- **`MASTER_PLAN.md` §4.3 already locked `.` as "reaching into a thing"**, with
  `f.age` as the given example. The rule list is not being extended; it is being
  *implemented*.
- **`src/frontend/parser.c` already parses it.** Line 350 builds an `N_MEMBER` node for
  `expr . IDENT` today. The AST shape exists.
- **`src/runtime/interp.c:1396` is the entire current implementation:**

  ```c
  case N_MEMBER:
      runtime_error("member access (.) isn't supported yet");
  ```

  `.` is a parsed, reserved, unimplemented piece of syntax with no competing
  claimant. `src/backends/c/compile.c` does not handle `N_MEMBER` at all (it falls into its
  `default:` at line 282 and exits); `src/backends/c/compilef.c`/`src/backends/llvm/compilel.c` print "not
  supported yet (type %d)". Nothing has to be taken away from anyone.

`design_type_system.md` deferred this deliberately — its non-goals list says
*"Records / structs / typed fields — The language has no record literal. Typing
members waits for members to exist"* and *"Sized integers (`i32`, `u8`),
unsigned types — Sizes matter for FFI and packed data; neither exists yet."*
This document makes members exist and makes sizes exist.

### 1.3 The consumers are already written

This is the unusual and pleasant situation: two design documents in this same
directory are already written against an API that does not exist, and they agree
with each other. That is a specification handed over, not a blank page.

`design_game_system.md` §2.2 reads `MSG` out of raw memory:

```
pm_kind = peek32(pm_msg + 8)
if pm_kind == 256 { key_edge_push(peek32(pm_msg + 16)) }
```

and spells `WNDCLASSA`'s 72-byte layout out field by field *by hand, in a
markdown table*, with the comment that getting it wrong "is the single likeliest
source of a silent crash." That table is precisely what a `struct` declaration
should be, and it doubles as a golden test (§5.2).

`design_kernel.md` §8.2's page allocator is written in zl against `poke8`,
`peek32`, `peek64`, and a 128 KiB static `.bss` reservation, and its §11 risk 1
says outright:

> This document depends on `peek8/16/32/64`, `poke*`, a way to take the address
> of a `.bss` reservation, and 4 KiB-aligned reservations. **W5 owns the
> spelling; W6 owns the requirement.**

§3.1 and §3.4 below are the answer to that, using those exact names.

### 1.4 What the boxed `Value` costs, measured against a real workload

`design_game_system.md` §"pixel format" already did this arithmetic and reached
the conclusion this design has to honour:

> storage: raw `alloc()`, not a zl list — `StretchDIBits` needs contiguous
> bytes; a boxed list is also 57,600 `Value` allocations

A 320×180 framebuffer as a zl list is 57,600 heap objects that a C function
cannot read. As `alloc(320*180*4)` it is one 230 KB block that GDI blits
directly. There is no representation of a boxed aggregate that fixes this — the
problem is not speed, it is that **`Value` has a shape and C has a different
shape**, and no amount of optimisation reconciles two shapes. The only fix is a
region of memory that is not made of `Value`s at all.

---

## 2. The central decision: a struct is an address

### 2.1 The three candidates, weighed

| | **(A) Boxed struct object** | **(B) Boxed handle, raw payload** | **(C) Address only** |
|---|---|---|---|
| Representation | new `V_STRUCT`; fields are `Value`s in `items[]` | new `V_STRUCT`; `Value` gains `void *raw` + a layout id | no new representation at all; the value is a `V_NUM` holding a base address |
| Can C read it? | **no** — the bytes are 32-byte `Value`s | yes, via the payload pointer | yes — the bytes *are* the C layout |
| `Value` / `src/runtime/runtime.h` change | one new enum member | **new field in `Value`, in `src/runtime/interp.c` *and* `src/runtime/runtime.h`** | **none** |
| Native tag scheme | new heap-object tag (2 and 4+ are free) | new heap-object tag | none — an address is already a tagged int |
| Lifetime / `type()` / printing | natural | natural | **absent** — `type(p)` says `num`, `print(p)` prints a number |
| Duplicates `design_maps.md`? | yes, almost exactly | partly | no |
| Field access cost, native | untag, load `items[i]`, deref | untag, load `raw`, sized load | **one instruction** (§5.4) |
| Safety | full | partial (size is known) | none by default |

**(A) is out on the first row.** A struct whose bytes are `Value`s cannot be
passed to `CreateWindowExA` no matter how nice it is to use. It would also be a
second implementation of `design_maps.md`'s assoc-list with a compile-time key
set — a duplicate feature wearing a different hat.

**(B) is the serious rival and it is not obviously wrong.** A boxed header
carrying `{layout_id, size, raw}` buys `type(p) == "Point"`, a printable struct,
a size for bounds checks, and a place to hang a destructor later. Its cost is
concrete and it is the reason it loses:

- `Value` is defined **twice**, in `src/runtime/interp.c:34` and `src/runtime/runtime.h:14`, and the two
  must stay identical or the C backend miscompiles silently. Adding a field means
  editing both, plus every `make_*` constructor, plus `value_to_string`, plus
  `eval_binary`'s tag dispatch, plus `zl_binop`.
- `design_type_system.md` §3.1 already refused to pass `Value` by value across an
  ABI boundary for exactly this class of fragility ("a 32-byte aggregate is
  passed indirectly, and getting that subtly wrong produces silent corruption,
  not a link error"). Growing `Value` moves it from 32 bytes to 40 and re-opens
  that question everywhere at once.
- It buys safety that §3.5 declines to enforce anyway, on a language whose locked
  identity (`MASTER_PLAN.md` §4.1) is *power > safety, explicitly not Rust*.
- Every backend gains a second dereference on the hot path, and the hot path is
  a per-pixel loop.

**(C) wins, and the reason it is affordable is §2.2.**

### 2.2 The enabling fact: a Win64 address fits in a `double` exactly

This is load-bearing, so it is stated precisely.

- Windows x64 user-mode virtual addresses occupy `0x0000000000000000` –
  `0x00007FFFFFFFFFFF`: **47 bits**, max ≈ 1.407 × 10^14.
- A C `double` represents every integer up to 2^53 exactly: 9.007 × 10^15.
- 2^53 / 2^47 = 64. **There are six spare bits.**

Therefore an address round-trips through `src/runtime/interp.c`'s `double num` with no loss,
and through `src/backends/native/nativert.c`'s `(n<<1)|1` tagged int with no loss (48 < 63, which
`design_ffi_syscalls.md` §2.2 already established for `FARPROC`), and through
`src/backends/c/compilef.c`/`src/backends/llvm/compilel.c`/`src/backends/native/nativegen.c`'s `i64` trivially. **All five engines
represent a pointer exactly.**

This sharpens — and partially corrects — `design_ffi_syscalls.md` §6.3, which
worried that "addresses above 2^53 lose low bits" in the interpreter and called
the risk "rare on today's Win64 layouts." It is not rare, it is *impossible*
under a 47-bit VA. One honest caveat, recorded as risk R3: Intel's **LA57**
5-level paging extends user VA to 57 bits (2^56 ≈ 7.2 × 10^16 > 2^53). On
Windows it is opt-in per-process and not the default; if it ever becomes default,
interpreter pointers become lossy and (B) has to be re-litigated. Nothing else in
this design changes.

### 2.3 What "address only" costs, stated before anyone discovers it

| Cost | Detail | Verdict |
|---|---|---|
| **Reference semantics** | `b = a` copies the address, not the bytes. Mutating `b.x` changes `a.x`. | **zl already works this way for lists.** `env_assign` (`src/runtime/interp.c:171`) stores the `Value` struct by value, which copies `items` — the *pointer* — so two names share one array and `x[i] = v` is visible through both. Structs are consistent with the aggregate the language already has, not a new exception. |
| **No `type()`, no `print()`** | `type(p)` is `"num"`; `print(p)` prints an address. | Accepted, and §7 makes "never print an address" a test-suite rule anyway, because addresses differ per run and would break the three-engine gate on contact. A `dump(Point, p)` debug builtin is a cheap v2 nicety. |
| **No by-value struct args or returns** | `f(p)` passes the address; `return p` returns the address. | Already a locked non-goal: `design_ffi_syscalls.md` §8, "By-value struct arguments / returns … waits on the companion struct/record feature and is a v3 concern. v1/v2 pass struct *pointers*." This document supplies exactly the pointers it asked for. |
| **No lifetime tracking** | Nothing knows a block was released. | §3.3's bump-only policy makes use-after-`free` *impossible* in v1 by never reusing memory; use-after-`release` is real and is what checked mode (§3.5) exists for. |
| **A struct handle is forgeable** | `p = 12345` then `p.x` reads address 12345. | True. So is `poke32(12345, 0)`. This is the systems tier; §3.5 states the posture rather than pretending otherwise. |

### 2.4 Struct-ness is static, and that is the whole trick

There is no runtime struct tag, so `p.x` cannot dispatch on `p`. The compiler
must know *statically* that `p` is a `Point`. That knowledge comes from an
annotation — reusing the `name: type = value` syntax and the `N_TYPE` node from
`design_type_system.md` §2.6:

```
p: Point = alloc!(sizeof(Point))
p.x = 3                              # -> poke32(p + 0, 3)
print(p.y)                           # -> peek32(p + 4)
```

**Structs need the annotation *syntax*; they do not need the type *checker*.**
That is a small, statable dependency: `parse_type()`, the `N_TYPE` node, and the
three parse sites in `design_type_system.md` §2.6. If the type system lands
first, this document consumes it. If this lands first, it ships `parse_type()`
and the type system consumes it. Either order works; both must not be built
twice. Stage 0 picks one.

An unannotated `p.x` is a **compile-time error** — "`p` has no struct type;
annotate it `p: SomeStruct = …`" — not a runtime error, because there is nothing
at runtime to check.

---

## 3. The raw-memory layer

### 3.1 The builtins

Plain identifiers, per `MASTER_PLAN.md` §4.3 ("Built-ins are plain identifiers,
NOT keywords. Adding 200 more built-ins costs zero reserved words"). No lexer
change, no grammar change, no reserved word.

| Builtin | Meaning |
|---|---|
| `alloc(n)` | `n` **zeroed** bytes from the raw region; returns the base address. 16-byte aligned. |
| `reserve(n, align)` | a **compile-time** static reservation of `n` zeroed bytes at `align`; returns a fixed address (§3.4) |
| `free(p)` | accepted and **ignored** in v1 (§3.3) |
| `mark()` / `release(m)` | region bookmark / bulk reclaim (§3.3) |
| `peek8(p)` `peek16(p)` `peek32(p)` `peek64(p)` | unsigned load of 1/2/4/8 bytes |
| `poke8(p,v)` `poke16(p,v)` `poke32(p,v)` `poke64(p,v)` | store the low 1/2/4/8 bytes of `v` |
| `copy_mem(dst, src, n)` | `memmove`, overlap-safe |
| `fill_mem(p, byte, n)` | `memset` |
| `sext(v, bits)` | sign-extend the low `bits` of `v` (§3.2) |
| `cstr(s)` | **already exists** as `zl_cstr`, `src/backends/native/nativert.c:876` — NUL-terminated copy of a zl string, returns its address |

The names `peek8/16/32/64` and `poke8/16/32/64` are chosen to match, character
for character, the ones `design_kernel.md` §8.2 and `design_game_system.md` §2.2
already wrote their zl against. That is not decoration: those files are the
acceptance tests.

**The bare `peek`/`poke` stay simulated.** They are in `src/runtime/interp.c:347`'s
`SIMULATED[]` table today and print instead of acting. Do **not** repurpose them
— any existing program calling `poke(a, v)` would silently start writing memory.
They are deprecated in favour of the sized names and left exactly as they are, so
Stage 1 provably cannot change the behaviour of any program that exists now.

### 3.2 Width, sign, and the one real hole

Loads are **unsigned**. A signed field is spelled in the *declaration*
(`hp: i16`) and the compiler emits a sign-extending load; the raw layer keeps
one helper, `sext(v, bits)`, for hand-written code. Rejected alternative: a
parallel `speek8/speek16/…` family — eight more builtins to express what a
declaration already says, and `design_game_system.md`'s `peek32(pm_msg + 8)`
would have to grow a sign that `WM_KEYDOWN` does not have.

`peek64` is the sharp edge, and it is the *same* edge `design_kernel.md` §2 found
in `pe_min.zl`:

| Engine | `peek64` result | Exact for |
|---|---|---|
| `interp.exe` (`double num`) | a `double` | \|v\| < 2^53 |
| `compile.exe` → `src/runtime/runtime.c` (`double num`) | a `double` | \|v\| < 2^53 |
| `compilef` / `compilel` / `nativegen` | `i64` | all 64 bits, two's complement |
| `nativeval` (tagged int) | 63-bit tagged | \|v\| < 2^62 |

**Rule M1: `peek64` is exact across all engines only for values below 2^53. For
an arbitrary 64-bit pattern, use two `peek32`s.** This is exactly the advice
`design_kernel.md` §4.1 already gives for GDT descriptors ("Write these as two
32-bit halves, not one 64-bit literal") for exactly this reason. Addresses are
the important case and they are always safe (§2.2). `peek8/16/32` are always
exact everywhere, in every engine, with no caveat.

The pleasant inversion worth noticing: **`f64` is the field type that is perfect
in the interpreter and awkward in `nativegen`** (which has no SSE at all — see
`design_floats.md` §1.2), while `i64` is the reverse. v1 supports `f64` fields in
the interpreter, both C backends, and LLVM; `nativegen` rejects an `f64` field
with a clear error until `design_floats.md` Stage 2 lands.

### 3.3 Allocation and lifetime: bump-only, regions, and a `free` that does nothing

`design_native_runtime.md` §7 already locked the policy:

> **GC:** none. Bump-allocate, never free. A compile run is short-lived; the
> 64 MB arena is enough.

This document keeps that and adds the refinement `design_game_system.md`
§"release(m)" asked for, for the case the quoted line does not cover — a program
that runs for minutes at 60 fps.

**Decision: `alloc` bump-allocates from a raw region that is *separate from the
object arena*, and `free(p)` is accepted and ignored in every engine.**

Three reasons, and the third is the one that matters:

1. **Separation.** `src/backends/native/nativert.c`'s arena holds tagged objects with type headers
   (`[+0] type, [+8] len, [+16] payload`). Raw bytes handed to `StretchDIBits`
   must not share a region with objects the runtime walks. One region per
   discipline.
2. **`src/backends/native/nativegen.c` has no heap at all.** It imports three kernel32 functions —
   `GetStdHandle`, `WriteFile`, `ExitProcess`. There is no `VirtualAlloc`, no
   arena, no `zl_alloc`. A real `free` in that backend means a real allocator in
   hand-assembled x86-64, which is a week of work buying nothing.
3. **A no-op `free` is the only choice that keeps the three-engine gate honest.**
   If `interp.exe` really `free`d and `native.exe` did not, a program with a
   use-after-free bug would print reused garbage in one engine and stale-but-
   intact data in the other — a *cross-engine disagreement caused by a bug in the
   test program*, which is the worst kind of red gate. With bump-only and no
   reuse anywhere, all five engines produce identical bytes even for a buggy
   program. Determinism is worth more here than reclamation.

Bulk reclamation, for the frame loop:

```
m = mark()                        # remember the region's bump pointer
... allocate freely for one frame ...
release!(m)                       # bump pointer := m. Everything after m is gone.
```

The rule the program must obey — *nothing allocated after `m` may outlive
`release(m)`* — is stated in `design_game_system.md` and nothing enforces it.
`release` carries `!` (§3.5); `mark` does not.

`alloc` **zeroes**. This costs a `memset` and buys a whole class of cross-engine
agreement: an uninitialised read returns 0 in all five engines instead of whatever
that engine's allocator last left there. Given §7's gate, that is not optional.

### 3.4 `reserve(n, align)` — the kernel's static, aligned `.bss`

`design_kernel.md` needs a 4 KiB-aligned 24 KiB page-table block, a 4 KiB-aligned
4 KiB IDT, a 16-byte-aligned 56-byte GDT, and a 128 KiB allocator bitmap, all
**before any allocator exists** and all in `.bss` (its Rule K2 forbids putting
them in a frame). Its §11 risk 2 flags that "the backend currently has no notion
of alignment on a data reservation."

```
gdt:    Gdt    = reserve(sizeof(Gdt), 16)
pml4:   ptr    = reserve(4096 * 6, 4096)
bitmap: ptr    = reserve(131072, 4096)
```

`reserve` is resolved at **compile time**: the compiler accumulates a data area,
aligns each reservation, and the call site becomes an address constant. Per
engine:

- interpreter / C backends: one zeroed static block per program, addresses handed
  out at startup.
- `src/backends/llvm/compilel.c`: a `@global` with `align 4096`.
- `src/backends/native/nativegen.c`: **a `.data` section, which does not exist today** (§5.4). This is
  the single largest piece of new work in the plan and it also retires the
  "program too big for the simple 1-page layout" bail at `src/backends/native/nativegen.c:445`.

`reserve` must be a top-level statement. Calling it inside a loop is a compile
error, not a leak.

### 3.5 Safety, and where the `!` marker goes

**What is enforced by default: nothing.** No bounds check, no null check, no
alignment check, no provenance check. `MASTER_PLAN.md` §4.1 is explicit that zl
is *power > safety, and explicitly not Rust*; a checked-by-default raw-memory
layer would contradict a locked decision, and W6 would immediately need an
escape hatch out of it.

What *is* enforced:

| Rule | Reason |
|---|---|
| **Only `alloc`, `reserve`, `cstr`, and the OS produce addresses.** There is no address-of a zl variable. | A zl local lives in a `Var` malloc'd by `env_define` in the interpreter and at `[rbp-N]` in `nativegen` — two utterly different things with different lifetimes. An `addr_of(local)` that means something different in each engine is a cross-engine defect generator. `design_game_system.md` already names "a pointer that is *not* from our arena" as the thing that breaks the model. `addr_of(p.field)` — a struct handle plus a constant — is legal, because it is just arithmetic on an address that already came from `alloc`. |
| **`reserve` is top-level only.** | It is a static reservation; a dynamic one is `alloc`. |
| **A struct field access is checked *for offset correctness* at compile time.** | This is the entire argument for having `struct` at all — see below. |

**The `!` marker rule, and an amendment to the FFI doc.** `MASTER_PLAN.md` §4.3
requires `!` stay "rare enough to still look scary," and
`design_ffi_syscalls.md` §7 makes it the FFI gate, listing `poke!` among its
examples. Those two pull in opposite directions the moment a per-pixel loop runs
57,600 `poke32`s per frame — 57,600 `!`s is not a scary marker, it is
punctuation.

> **Rule M2: `!` marks where a capability is *obtained*, not where it is
> *exercised*.** `alloc!`, `reserve!`, `release!`, `dll!`, `sym!`, `call!`,
> `asm!` carry the marker. `peek*`, `poke*`, `copy_mem`, `fill_mem`, and
> `.field` access do not.

This is greppable in the way §7 of the FFI doc wants — one `!` per region
acquired, not one per byte touched — and it reconciles that document with
`design_game_system.md`, which already writes `dll!`/`sym!`/`call!` but bare
`peek32`/`poke32`. **Stage 0 must ratify this as an explicit amendment** so the
two documents do not silently disagree; the alternative (marking every store) was
tried on paper against the framebuffer loop and is unreadable.

**Why `struct` is a safety feature even though nothing is checked at runtime.**
Compare the two ways to set `WNDCLASSA.lpfnWndProc`:

```
poke64(wc + 8, defproc)          # is it 8? or 12? nothing will tell you
wc.lpfnWndProc = defproc         # the compiler computed 8, from the declaration
```

Both emit `mov [rcx+8], rax`. The second cannot have the wrong offset. On a
72-byte structure with a 4-byte padding hole after `style`, hand-written offsets
are the "single likeliest source of a silent crash" — that is
`design_game_system.md`'s own words about its own table. Moving that table from a
markdown file into the compiler is the deliverable.

**Checked mode (`--checked`, off by default, Stage 6).** A side table of
`{base, size, live}` per `alloc`/`reserve`, consulted by every `peek*`/`poke*`/
field access: out-of-range, released-region, and null accesses become clean
runtime errors with the offending address. ~40 lines in the interpreter, a
`src/runtime/runtime.c` flag for the boxed C backend, and **not in the unboxed or native
backends at all** — those exist to be fast. It is a debugging tool, so it lives
where debugging happens.

---

## 4. Structs

### 4.1 Declaration — contextual `struct`, zero reserved words

`MASTER_PLAN.md` §4.4 freezes the reserved-word list at fourteen and says every
addition "is a name users lose forever." `design_type_system.md` added zero;
`design_ffi_syscalls.md` added zero. This adds zero.

`struct` is recognised **only** as the first token of a statement immediately
followed by an identifier and `{`. Everywhere else `struct` is an ordinary
identifier and a legal variable name. The lexer is untouched — `struct` lexes as
`T_IDENT` — and `parse_statement` gets one shape check, exactly the mechanism
`design_ffi_syscalls.md` §2.5(a) chose for `extern` and recommends leading with.

```
struct Point {
    x: i32
    y: i32
}

struct WndClassA {
    style:         u32
    lpfnWndProc:   ptr
    cbClsExtra:    i32
    cbWndExtra:    i32
    hInstance:     ptr
    hIcon:         ptr
    hCursor:       ptr
    hbrBackground: ptr
    lpszMenuName:  ptr
    lpszClassName: ptr
}
```

Fields are `name: type`, one per line — newline-terminated, per
`MASTER_PLAN.md` §4.3's "Statement end: **newline** — no semicolons". The `:`
already lexes as a single-character symbol with no lexer change
(`design_type_system.md` §2.1 verified this against `src/selfhost/compiler.zl`'s own lexer).
`struct` declarations are top-level only, and forward references are resolved in
one pass at end of file, so declaration order does not matter.

A new `NodeType` member, **appended at the end of the enum** so no existing
numeric value shifts (`src/backends/llvm/compilel.c:176` and `src/backends/c/compilef.c` print raw `type %d`):

```
N_STRUCT    /* text = struct name; kids = field decls (N_TYPE-annotated idents) */
```

### 4.2 Field types

Recognised positionally, as identifiers, in field position only — the same
mechanism `design_ffi_syscalls.md` §2.6 uses for `extern` signatures. The scalar
set is that document's frozen list plus the narrow widths it did not need:

| Field type | Size | Align | Notes |
|---|---|---|---|
| `i8` `u8` | 1 | 1 | |
| `i16` `u16` | 2 | 2 | |
| `i32` `u32` | 4 | 4 | |
| `i64` `u64` | 8 | 8 | subject to Rule M1 in the two `double`-based engines |
| `f64` | 8 | 8 | not supported by `nativegen` until `design_floats.md` Stage 2 |
| `ptr` | 8 | 8 | a raw address. **This is how you spell a string field** |
| `Name` | `sizeof(Name)` | `align(Name)` | nested, **inlined by value** |
| `T[N]` | `N * sizeof(T)` | `align(T)` | fixed array, inlined |

`str` is deliberately **not** a field type. A zl string is a heap object with a
length header; a C `char*` is an address. A field that holds text is a `ptr`, and
you put `cstr(s)` in it. Silently boxing at a struct boundary is exactly the
category of magic that makes a systems type unusable.

`bool` is not a field type either — spell it `u8` or `u32` and say which, because
`BOOL` in Win32 is 4 bytes and `bool` in C++ is 1, and guessing is how you get a
3-byte offset error.

### 4.3 Layout rules

Five rules. They reproduce the Microsoft x64 C layout, because the entire point
is to hand these bytes to code compiled by MSVC.

- **R1 — Order is declaration order. Fields are never reordered.** Not for
  packing, not ever. A reordering compiler cannot interoperate.
- **R2 — `offset(f) = round_up(cursor, align(f))`**, then `cursor += size(f)`.
- **R3 — `align(S) = max(align(f))` over all fields**, minimum 1.
- **R4 — `sizeof(S) = round_up(cursor, align(S))`.** The trailing padding is
  mandatory: without it `S[N]` misaligns every element after the first, which is
  the bug that shows up as "the 2nd sprite is fine and the 3rd is garbage."
- **R5 — `struct Name packed { … }` sets every alignment to 1**, so `offset(f) =
  cursor` and `sizeof(S) = cursor`. `packed` is a contextual word in exactly one
  position and steals no name.

An empty struct is a **compile error**, not a zero-sized or one-byte type. C and
C++ disagree about it, so there is no right answer to inherit.

Two worked examples, both of which already exist elsewhere in the repo as
independently-derived golden answers:

**`WndClassA` — must come out 72 bytes**, matching
`design_game_system.md` §2.2's hand-written table exactly:

```
off  0   style          u32   size 4  align 4
     4   -- padding 4 --                        (R2: ptr needs align 8)
     8   lpfnWndProc    ptr   size 8  align 8
    16   cbClsExtra     i32   size 4
    20   cbWndExtra     i32   size 4
    24   hInstance      ptr
    32   hIcon          ptr
    40   hCursor        ptr
    48   hbrBackground  ptr
    56   lpszMenuName   ptr
    64   lpszClassName  ptr
    72   cursor = 72, align(S) = 8, sizeof = 72     (R3, R4)
```

**`BitmapFileHeader` — must come out 14 bytes**, which is only reachable with
R5, and is why `packed` is v1 and not v2 (`design_game_system.md` writes BMPs):

```
struct BitmapFileHeader packed {
    bfType:      u16      # off 0
    bfSize:      u32      # off 2   <- unaligned; impossible without packed
    bfReserved1: u16      # off 6
    bfReserved2: u16      # off 8
    bfOffBits:   u32      # off 10
}                         # sizeof 14, NOT 16
```

Unaligned access on x86-64 is legal and correct for ordinary `mov`, so a packed
field needs no special code generation — one more reason `packed` is cheap here
and would not be on ARM.

### 4.4 `sizeof` and `offsetof` are compile-time constants

```
sizeof(Point)              -> 8
sizeof(WndClassA)          -> 72
offsetof(WndClassA, hCursor) -> 40
```

Both are builtins whose *argument* is a struct name (and field name) rather than
a value, so both get the same one-line parser special-case `struct` gets: in the
argument position of `sizeof`/`offsetof`, a bare identifier naming a declared
struct is a type reference. Both fold to an `N_NUMBER` before any backend sees
them, so **no backend needs to know they exist** — a genuinely small diff, and it
means `alloc(sizeof(Entity) * 1000)` is literally `alloc(48000)` in the emitted
code.

`sizeof` of a scalar type name (`sizeof(u32)` → 4) is also legal and useful for
stride arithmetic.

### 4.5 Lowering `.field`

```
p: Point = alloc!(sizeof(Point))
p.x = 3          ==>  poke32(p + 0, 3)
n = p.y          ==>  n = peek32(p + 4)          (u32: zero-extend)
h = e.hp         ==>  h = sext(peek16(e + 12), 16)   (i16: sign-extend)
```

The compiler resolves `p`'s declared struct, looks up `x`, and emits the sized
access at a constant offset. There is no runtime lookup, no field-name string, no
hash. **A field access costs exactly what the equivalent C costs**, and in
`nativegen` it is one instruction (§5.4).

Compound assignment falls out for free: `p.x += 1` is
`poke32(p+0, peek32(p+0) + 1)`, matching how `x[i] += 1` already works
(`src/runtime/interp.c:1448` reads before evaluating the right side — the same ordering rule
applies).

An unknown field is a compile error naming the struct and listing its fields. A
`.field` on an unannotated expression is a compile error (§2.4).

### 4.6 Nested structs and fixed arrays

```
struct Rect  { left: i32  top: i32  right: i32  bottom: i32 }
struct Sprite {
    bounds: Rect          # inlined: 16 bytes at offset 0
    frames: u32[8]        # inlined: 32 bytes at offset 16
    next:   ptr           # offset 48
}
```

- A nested struct is **inlined, not a pointer** — that is what C does and what
  `RECT` inside `WINDOWPLACEMENT` requires. `s.bounds.left` folds two constant
  offsets into one: `peek32(s + 0)`.
- `s.frames[3]` is `peek32(s + 16 + 3*4)`. A constant index folds completely; a
  variable index emits one `imul`/`shl` by the element size.
- **A struct may not contain itself** (directly or through a cycle of inlined
  fields) — that is an infinite size. Self-reference is through `ptr`, and the
  cycle check is a depth-first walk at declaration-resolution time.

### 4.7 Arrays of structs

```
struct Entity { hp: i32  x: f64  y: f64  flags: u32 }   # sizeof 32

world: Entity[] = alloc!(sizeof(Entity) * 1000)
world[3].hp = 100        ==>  poke32(world + 3*32 + 0, 100)
world[i].hp = 100        ==>  poke32(world + i*32 + 0, 100)
```

`Entity[]` in an annotation means "a pointer to a run of `Entity`", with no
length — this is a systems array, not a zl list. **`world[i]` is not bounds
checked and `len(world)` does not work on it**; the program owns the count. That
divergence from `x[i]` on a `V_LIST` (which *is* bounds-checked, `src/runtime/interp.c:1391`)
is real and is the price of contiguity. The annotation is what disambiguates the
two meanings of `[`, and it is required — an un-annotated `world[i].hp` is the
same compile error as an un-annotated `p.x`.

`world[i]` used without a following `.field` yields the *address* of element
`i` (i.e. `world + i*sizeof(T)`), which is what you pass to a function taking an
`Entity`. That falls straight out of §2's "a struct is an address" and needs no
extra rule.

### 4.8 Rejected: struct literals

`Point{x: 3, y: 4}` is the obvious ergonomic win and it is **out of v1**, for a
specific and unfixable reason: zl's block syntax is `if cond { … }`,
`while cond { … }`, `for v in seq { … }`, and `MASTER_PLAN.md` §4.3 locks braces
as *the* block delimiter ("never `end`, never indentation. Nothing invisible can
break"). With a literal form, `if p { … }` is ambiguous — `p` followed by a brace
is either a condition and a block, or a struct literal. Rust hit this and solved
it with a "no struct literal in condition position" restriction, i.e. a context-
sensitive grammar rule that a reader has to know about. Paying that to save
`alloc(sizeof(Point))` is a bad trade, and it would bend a locked rule.

The v1 idiom is fine and honest, and `alloc` zeroes:

```
p: Point = alloc!(sizeof(Point))
p.x = 3
p.y = 4
```

---

## 5. One layout algorithm, five engines

### 5.1 The C backends must not use a real C `struct`

`src/backends/c/compile.c` emits C. It *could* emit

```c
typedef struct { unsigned style; void *lpfnWndProc; /* … */ } zl_s_WndClassA;
```

and use `offsetof`, and get MSVC's layout for free, exactly as the brief
suggests. **Do not.** Two reasons, the second decisive:

1. **It creates a second source of truth.** `src/backends/native/nativegen.c` cannot ask a C compiler
   anything — it computes offsets itself or it does not run. If `src/backends/c/compile.c` uses
   MSVC's layout and `src/backends/native/nativegen.c` uses ours, the two agree only as long as our
   rule set (§4.3) matches MSVC exactly. It does today. It would silently stop
   matching the first time someone adds `f64[3]` or a `packed` nested struct or
   an over-aligned type, and there is no build error for "these two layouts
   diverged" — only wrong pixels.
2. **The divergence is invisible in the one place it matters.** Two layouts that
   differ only in *padding* produce identical output for every program that
   writes and reads its own fields — which is every test — and differ only when
   the bytes cross into a real Win32 call, which is the one path the three-engine
   gate cannot cover (§7). A defect class that is invisible to the test suite and
   fatal in production is the worst possible shape.

**Decision: one `layout.c` / `layout.h`, shared by `src/runtime/interp.c`, `src/backends/c/compile.c`,
`src/backends/c/compilef.c`, `src/backends/llvm/compilel.c`, and `src/backends/native/nativegen.c`.** It takes an `N_STRUCT` node and
produces `{size, align, [{name, offset, type, size, signed}]}`. It is ~120 lines
implementing R1–R5. Every engine reads the same table. `src/backends/c/compile.c` emits
`char`-buffer arithmetic and explicit offsets, never a C `struct`.

The cost is real and worth naming: we implement the MS x64 layout rule ourselves
rather than getting it free, and if we get it wrong we are wrong everywhere at
once. §5.2 is the mitigation, and being wrong *identically* everywhere is
strictly better than being wrong in one engine.

### 5.2 The oracle test

Because the rule we are reimplementing is MSVC's, MSVC is available as a
reference implementation:

```
for each struct S in tests/struct/*.zl:
    emit a C file declaring the equivalent real struct
    printf("%zu", sizeof(S)) and offsetof(S, f) for every field
    cl it, run it, diff against layout.c's table
```

A single `.ps1` that fails loudly if our arithmetic and MSVC's ever part company.
Seed it with `WndClassA` (must be 72, `lpfnWndProc` at 8),
`BitmapFileHeader packed` (must be 14), `MSG` (must be 48, `message` at 8,
`wParam` at 16 — `design_game_system.md` §2.2's numbers), and `Rect` (16).
Those four answers were derived independently of this document and are therefore
a real check on it.

Note this test compares against MSVC only — it does not prove clang agrees, which
matters for `src/backends/llvm/compilel.c`. Add the same file compiled with `clang` from
`C:\Program Files\LLVM\bin` and diff three ways.

### 5.3 Per-engine lowering

| Construct | `src/runtime/interp.c` | `src/backends/c/compile.c` (boxed C) | `src/backends/c/compilef.c` (unboxed C) | `src/backends/llvm/compilel.c` (LLVM) | `src/backends/native/nativegen.c` (x86-64) |
|---|---|---|---|---|---|
| `alloc(n)` | `calloc` via `os_win.c` | `zl_alloc_raw` in `src/runtime/runtime.c` | `calloc` | `@malloc`+`memset` | **`VirtualAlloc` — a 4th import** |
| `reserve(n,a)` | static block | static block | `_declspec(align(a))` array | `@g align a` | **a new `.data` section** |
| `peek32(p)` | `*(uint32_t*)(uintptr_t)p` | `zl_calln("peek32", …)` — **no `src/backends/c/compile.c` change needed** | `*(unsigned*)(char*)p` | `load i32, ptr` | `mov eax,[rax]` |
| `p.x` (u32 @ 4) | offset folded at parse | `zl_peek32_off(p, 4)` | `*(unsigned*)((char*)p+4)` | `getelementptr i8` + `load i32` | **`mov eax,[rax+4]` — one instruction** |
| `sizeof(S)` | folded to `N_NUMBER` | folded | folded | folded | folded |

One detail worth flagging in that table: **the raw-memory layer needs no
`src/backends/c/compile.c` change at all.** `src/backends/c/compile.c` emits unknown builtins as
`zl_calln("name", n, …)` (line 188) and `src/runtime/runtime.c:973`'s `zl_calln` dispatches
by name into `builtin()`. Adding `peek32` to `src/runtime/runtime.c` makes it work in the C
backend for free. Only the `.field` lowering — a new `N_MEMBER` case — touches
`src/backends/c/compile.c`.

For `src/backends/llvm/compilel.c`, **do not** declare an LLVM named struct type and use typed
`getelementptr`. Same argument as §5.1: it is a third layout authority. Emit
`getelementptr i8, ptr %p, i64 <our offset>` and a typed load. Byte offsets we
computed, always.

### 5.4 `src/backends/native/nativegen.c` — the one that has to do it itself

This backend has no C compiler behind it, so everything is explicit. What it has
today, read out of the source:

| | Today (`src/backends/native/nativegen.c`) |
|---|---|
| Sections | `.text` + a **one-page** `.idata`, with an outright bail at line 445: `"program too big for the simple 1-page layout"` |
| Data section | **none.** String literals are appended to the tail of `.text` and reached by rip-relative `lea` displacements backpatched through `sfix[]` (line 490) |
| Globals | **none.** Top-level variables are locals in `main`'s frame (`collect_locals`, line 466) |
| Heap | **none.** Three imports: `GetStdHandle`, `WriteFile`, `ExitProcess` |
| Values | exact `i64` — `atoll(n->text)` at line 142, GPR ops at 98–102 |

So the work splits cleanly into "the easy part" and "the real part."

**The easy part — field access is trivially better here than anywhere else.**
The offset is a compile-time constant, so it becomes an instruction
*displacement*. With the base address already in `rax` (the stack-machine's
accumulator) and the value to store in `rax` with the base in `rcx`:

| Field type | Load into `rax` | Bytes |
|---|---|---|
| `u8`  | `movzx rax, byte [rax+d]` | `48 0F B6 80` + `d32` |
| `u16` | `movzx rax, word [rax+d]` | `48 0F B7 80` + `d32` |
| `u32` | `mov eax, [rax+d]` (zero-extends for free) | `8B 80` + `d32` |
| `u64` | `mov rax, [rax+d]` | `48 8B 80` + `d32` |
| `i8`  | `movsx rax, byte [rax+d]` | `48 0F BE 80` + `d32` |
| `i16` | `movsx rax, word [rax+d]` | `48 0F BF 80` + `d32` |
| `i32` | `movsxd rax, dword [rax+d]` | `48 63 80` + `d32` |

| Field type | Store `rax` at `[rcx+d]` | Bytes |
|---|---|---|
| `u8`/`i8`   | `mov [rcx+d], al`  | `88 81` + `d32` |
| `u16`/`i16` | `mov [rcx+d], ax`  | `66 89 81` + `d32` |
| `u32`/`i32` | `mov [rcx+d], eax` | `89 81` + `d32` |
| `u64`/`i64` | `mov [rcx+d], rax` | `48 89 81` + `d32` |

> Verify these against a disassembler before trusting them — they are written
> from the encoding rules (REX.W + opcode + ModRM `mod=10`), and a wrong ModRM
> byte is a wrong *register*, which produces plausible-looking garbage rather
> than a fault. `design_kernel.md` §3.3 gives the same warning about its table
> for the same reason.

That is **one instruction per field access**, versus the boxed C backend's
`zl_calln` with a string compare and two 32-byte `Value` copies. The performance
argument for structs is real and it is largest exactly where
`design_game_system.md` needs it.

**The real part — `nativegen` grows a data section.** `reserve` (§3.4) and
`alloc` both need it:

1. **A `.data` section.** Compute the layout instead of asserting it: `.text`
   sized and page-aligned, then `.rodata`/`.data` (string literals move here out
   of the `.text` tail), then `.idata`. Delete the line-445 bail. Reservations are
   assigned aligned offsets within `.data` and referenced with rip-relative `lea`
   using the *existing* `sfix[]` backpatch machinery — this is the same problem
   string literals already solved, at a different offset.
2. **A fourth import, `VirtualAlloc`.** Mechanical: extend the ILT/IAT/hint-name
   block in `write_pe` from 3 entries to 4. `src/backends/native/nativert.c:1242` already shows the
   exact call shape (`VirtualAlloc(NULL, size, MEM_COMMIT|MEM_RESERVE,
   PAGE_READWRITE)`) and `src/backends/native/nativert.c:159`'s `emit_alloc` shows the bump loop to
   copy. Emit a one-time init at entry and a 6-instruction bump allocator.
3. **Rule K1 compatibility.** `design_kernel.md`'s Rule K1 forbids absolute
   addresses so a UEFI image stays relocatable. Reaching `.data` through
   `lea reg,[rip+disp32]` satisfies it, so `kernelgen.c` inherits a working
   `reserve` when it forks. Doing it with an absolute `movabs` would work in
   user mode and silently break W6. **Use rip-relative.**

### 5.5 `src/backends/native/nativeval.c` — untag, access, retag

The boxed native backend's values are `(n<<1)|1`. A struct handle is a tagged
address, so a field access is three instructions instead of one:

```
sar  rax, 1                  ; untag the base  (48 D1 F8)
mov  eax, [rax + d]          ; the field
lea  rax, [rax+rax+1]        ; retag the result (48 8D 44 00 01)
```

A 47-bit address survives `(a<<1)|1` with 15 bits to spare, per §2.2 and
`design_ffi_syscalls.md` §2.2. Note the retag is only correct for field values
below 2^62 — the same Rule M1 caveat, one engine over.

---

## 6. Interactions

### 6.1 FFI (`design_ffi_syscalls.md`)

They share one currency — the address — by construction, and that document
already anticipated this one: its §6.1 says "`alloc(64)` returns an address
`call` can pass as an `LPVOID` output buffer; after the call, `peek(addr)` reads
what the C function wrote. They were designed to compose; neither needs to know
about the other beyond 'addresses are tagged ints.'" That holds exactly.

Two amendments this document proposes and Stage 0 must ratify:

- **The `!` placement rule (M2, §3.5)** narrows that document's §7 from "every
  dangerous *use*" to "every capability *acquisition*."
- Its §8 non-goal "by-value struct arguments/returns … waits on the companion
  struct/record feature" can now be **restated rather than resolved**: by-value
  structs remain a non-goal here too (§10), because §2's decision makes a struct
  an address and there is no by-value form to pass. Anyone wanting COM vtables or
  `D3D11_BUFFER_DESC`-by-value is writing a v3 document.

### 6.2 Kernel (`design_kernel.md`)

This document answers its §11 risks 1 and 2 directly: the spellings are
`peek8/16/32/64` and `poke8/16/32/64` as it assumed, and `reserve(n, align)` is
the aligned `.bss` reservation it needed. Its §4.1 "write descriptors as two
32-bit halves" is now a general rule (M1, §3.2) rather than a local workaround.
`kernelgen.c` forks a `src/backends/native/nativegen.c` that, after Stage 4 here, already has a
computed multi-section layout and rip-relative data access — which retires two
of that document's stage-1 tasks before it starts.

Its §1.2 "the kernel target compiles a subset with no implicit allocation" is
also *helped*: struct field access allocates nothing in any engine, so structs
are usable in early boot in a way lists and string concatenation are not.

### 6.3 Game system (`design_game_system.md`)

That document is the acceptance test. Its `WNDCLASSA` table becomes a `struct`
declaration; its `pump()` loop's `peek32(pm_msg + 8)` becomes `msg.message`; its
framebuffer stays `alloc(w*h*4)` and `poke32` because per-pixel access has no
fields; its `release(m)` is §3.3's region API. Nothing in it needs rewriting to
accommodate this design, which is the strongest available evidence that the API
shape is right.

### 6.4 Type system (`design_type_system.md`)

Shares `parse_type()` and the `N_TYPE` node (§2.4) and supplies the two things
that document listed as non-goals: members, and sized integers. It does **not**
need the checker, and the checker does not need structs. If both land, a struct
name is a legal annotation type and `p: Point` is checked like any other
annotation. Stage 0 decides which builds the shared parser code.

### 6.5 Maps (`design_maps.md`)

No conflict, and the boundary is crisp: a **map** is dynamic, boxed, string-keyed,
and accessed with `m[k]`; a **struct** is static, raw, compile-time-keyed, and
accessed with `p.f`. `.` is claimed by structs and `[k]` by maps, and neither
grows into the other's syntax. A program that wants named fields and does not
care about memory layout should use a map.

### 6.6 Optimizer (`design_optimizer.md`)

Constant folding already planned there subsumes `sizeof(Entity) * 1000` and the
constant part of `base + i*32 + off`, so struct code benefits from that document
with no special-casing. Its Rule "do not peephole raw bytes" applies here too:
field-access instructions go through the instruction IR if that lands first,
otherwise straight into `code[]` like everything else.

---

## 7. Verification and the three-engine gate

`verify.ps1` asserts the self-hosting fixpoint plus byte-identical output across
interpreter, C backend, and native backend. Unlike FFI (`design_ffi_syscalls.md`
§1.3), **raw memory and structs can stay inside that gate**, and should. Four
rules make that true:

1. **Never print an address.** Addresses differ per run and per engine. Test
   programs print field *values*. This is a test-suite convention, and the review
   check is "does any expected-output file contain a large number that looks like
   a pointer."
2. **`alloc` zeroes** (§3.3), so an uninitialised read is `0` everywhere rather
   than per-allocator garbage.
3. **`free` never reuses** (§3.3), so even a buggy test program produces identical
   bytes in all five engines.
4. **Test values stay below 2^53** for `peek64`/`i64` fields — the identical
   posture `run_tests.ps1` already takes on division ("use exact divisions here
   so all 3 engines agree", quoted in `design_floats.md` §1.1). A dedicated
   proposed `wide64` test under `tests/struct/` documents the *divergence* deliberately and lives
   outside the gate.

Plus the two gates this feature adds:

- **The layout oracle** (§5.2) — our offsets versus MSVC's and clang's.
- **A round-trip suite** — for every field type and every struct in the corpus:
  write, read back, compare; and write via `p.f` / read via `peek*` at
  `offsetof`, and vice versa, so the two layers are proven to agree.

`src/selfhost/compiler.zl` uses no structs and needs none, so the **fixpoint is untouched** —
worth stating explicitly, because it is the invariant every change to the parser
threatens. The `N_STRUCT` enum member is appended (§4.1) precisely so nothing
that prints `type %d` shifts.

---

## 8. Staged plan

Each stage is independently shippable and ends with `verify.ps1` green.
`src/backends/native/nativegen.c` work is serial-only (single-owner file), per the campaign rule.

### Stage 0 — Lock the spec (no code)

Ratify §2 (struct = address), §3.3 (bump-only, `free` is a no-op, regions),
§3.5's Rule M2 **as an explicit amendment to `design_ffi_syscalls.md` §7**,
§4.3's R1–R5, and §5.1 (one layout algorithm, no C `struct`). Decide who owns
`parse_type()`/`N_TYPE` — this document or `design_type_system.md` (§2.4).
Freeze the field-type name set (§4.2).
**Verify:** reviewed and frozen; the FFI document's §7 amended in the same
commit so the two do not disagree in the repo.

### Stage 1 — Raw memory, interpreter + C backend

`alloc` / `mark` / `release` / `free` / `peek8..64` / `poke8..64` / `copy_mem` /
`fill_mem` / `sext` in `src/runtime/interp.c`'s `call_builtin` and in `src/runtime/runtime.c`'s
`builtin()`. Real memory access lives behind `os_*` shims in `os_win.c` if it
needs `<windows.h>` (that file's stated invariant). `SIMULATED[]` is **not**
touched; bare `peek`/`poke` stay simulated.
**Verify:** `verify.ps1` green (nothing existing changed); a round-trip test
writes and reads every width; interpreter and C backend agree byte for byte.

### Stage 2 — `struct`, `sizeof`, `offsetof`, `.field`

`layout.c`/`layout.h`; contextual `struct` parsing; `N_STRUCT` appended;
`sizeof`/`offsetof` folded to `N_NUMBER`; `N_MEMBER` lowered in `src/runtime/interp.c` and
`src/backends/c/compile.c`; annotation syntax (§2.4) if it does not exist yet.
**Verify:** the layout oracle (§5.2) passes on `WndClassA` (72),
`BitmapFileHeader packed` (14), `MSG` (48), `Rect` (16) against both `cl` and
`clang`; `p.f` and `peek*(p + offsetof(...))` agree; `verify.ps1` green and the
fixpoint hash unchanged.

### Stage 3 — Unboxed C and LLVM backends

`src/backends/c/compilef.c` emits direct casts; `src/backends/llvm/compilel.c` emits `getelementptr i8` with our
offsets plus typed loads. Both reject `f64` fields only if they already reject
floats.
**Verify:** four engines produce identical output on the struct suite; the
unboxed backends show the expected speedup on a struct-heavy benchmark.

### Stage 4 — `src/backends/native/nativegen.c`: data section, heap, one-instruction fields

In order: (a) computed multi-section layout, string literals moved from the
`.text` tail into `.rodata`, the line-445 bail deleted; (b) `reserve` as an
aligned `.data` offset reached rip-relative; (c) `VirtualAlloc` as a fourth
import plus the bump allocator; (d) the load/store encodings of §5.4.
**Verify:** existing native programs produce byte-identical output before and
after (a) — this sub-stage must be a provable no-op; `dumpbin /headers` shows the
new sections; a struct program agrees with the other four engines; a `reserve`d
block is 4096-aligned.

### Stage 5 — Nested structs, fixed arrays, arrays of structs, `packed`

§4.6 and §4.7 in all five engines.
**Verify:** `Sprite` (nested `Rect` + `u32[8]`) matches the oracle; a
1000-element `Entity[]` round-trips every element; a `packed` BMP header written
by zl is opened by a real image viewer.

### Stage 6 — Checked mode

`--checked` allocation table in `src/runtime/interp.c` and `src/runtime/runtime.c` only.
**Verify:** a deliberate out-of-bounds `poke32` and a use-after-`release` each
produce a clean diagnostic under `--checked` and the documented undefined
behaviour without it; `verify.ps1` runs unchecked and stays green.

---

## 9. Risks and open questions

| # | Item | Note |
|---|---|---|
| R1 | **Our layout rule drifts from MSVC's.** | The whole feature's correctness rests on §4.3 matching the compiler that produced `user32.dll`'s headers. Mitigation is §5.2's oracle, run in CI, seeded from four independently-derived golden answers. This is the highest-consequence risk here. |
| R2 | **`src/backends/native/nativegen.c`'s section rework breaks existing programs.** | It touches the PE writer, string-literal addressing, and the entry layout at once. Mitigation: Stage 4(a) must be proven a byte-for-byte no-op *before* 4(b)–(d), the same discipline `design_optimizer.md` §"Ordering rationale" imposes on its IR refactor. |
| R3 | **LA57 (5-level paging) breaks §2.2.** | 57-bit user VA exceeds 2^53 and makes interpreter pointers lossy. Opt-in per-process on Windows and not default today. If it becomes default, representation (B) from §2.1 has to be reconsidered; nothing else in this design moves. |
| R4 | **`peek64` above 2^53.** | Rule M1. Known, documented, tested-around, and identical to the hole `design_kernel.md` §2 already found. The failure is silent and gives a *plausible* wrong number, which is why it needs a named rule rather than a footnote. |
| R5 | **Bump-only means a long-running program leaks.** | `design_game_system.md`'s frame loop is the case that matters and it uses `mark`/`release`. Anything else that allocates in a loop and never releases will exhaust the region. A real allocator with reuse is a v2 document, and it will have to re-open §7's determinism argument when it lands. |
| R6 | **Reference semantics surprise someone.** | `b = a` aliases. Documented, and consistent with lists (§2.3) — but lists are the *only* precedent and most zl values copy. `REFERENCE.md` must say this in the struct section, not bury it. |
| R7 | **Annotation dependency.** | If `design_type_system.md` ships `parse_type()` differently than assumed here, §2.4/§4.7's spellings move. The *decisions* do not. Stage 0 assigns ownership so it is written once. |
| R8 | **`f64` fields on `nativegen`.** | Blocked on `design_floats.md` Stage 2 (SSE). v1 hard-errors with a message naming that document, rather than silently truncating through `atoll` the way literals do today. |
| R9 | **Scope creep into a type system.** | "While I'm here, check that `p.x` is an `i32`" is how this becomes W3. §10's list is the contract: this document computes offsets, it does not check types. |

---

## 10. Non-goals (explicit)

- **By-value struct arguments and returns.** Follows from §2 and was already
  `design_ffi_syscalls.md` §8's non-goal. `f(p)` passes an address.
- **Struct literals** (`Point{x:3}`) — §4.8, the `if p {` ambiguity.
- **Methods, constructors, destructors, inheritance.** `MASTER_PLAN.md` §4.5
  cut classes deliberately. A struct is a layout, not an object.
- **Unions and bitfields.** Overlapping fields and sub-byte fields are both real
  Win32 needs occasionally; `band`/`shl`/`shr` already cover bitfields and a
  second `struct` over the same address covers unions. Revisit only with a
  concrete blocked use case.
- **Bounds checking by default**, and any checking at all in the unboxed/native
  backends. §3.5.
- **Address-of a zl local.** §3.5. Only `alloc`, `reserve`, `cstr`, and the OS
  make addresses.
- **A real allocator with reuse, or any GC.** §3.3, consistent with
  `design_native_runtime.md` §7.
- **`str` and `list` as field types.** §4.2. A field that points at text is a
  `ptr` holding a `cstr`.
- **Generic / parameterised structs.** No.
- **Non-Windows layout rules.** This is MS x64. A SysV port inherits R1–R5 with
  different `long double` handling and is a separate document.
- **Type checking of field values.** Assigning a 70000 to a `u16` field truncates,
  silently, like C. The checker in `design_type_system.md` may catch it later.

---

## 11. Definition of done

1. §2's representation decision, §3.3's allocation policy, §3.5's Rule M2, and
   §5.1's single-layout rule are ratified, and `design_ffi_syscalls.md` §7 is
   amended in the same commit.
2. `alloc` `reserve` `free` `mark` `release` `peek8/16/32/64` `poke8/16/32/64`
   `copy_mem` `fill_mem` `sext` work in all five engines; bare `peek`/`poke`
   remain simulated and unchanged.
3. `struct` parses with **zero new reserved words**, `N_STRUCT` is appended to the
   enum, and `src/selfhost/compiler.zl`'s fixpoint hash is unchanged.
4. `layout.c` is the only implementation of R1–R5, and the oracle test agrees with
   both `cl` and `clang` on `WndClassA` = 72 / `lpfnWndProc` @ 8,
   `BitmapFileHeader packed` = 14, `MSG` = 48, `Rect` = 16.
5. `p.f` compiles to one instruction in `src/backends/native/nativegen.c` and to a direct cast in
   `src/backends/c/compilef.c`; `src/backends/native/nativegen.c` has a computed multi-section layout, a `.data`
   section reached rip-relative, and a `VirtualAlloc` heap.
6. Nested structs, fixed arrays, arrays of structs, and `packed` all round-trip,
   and a `packed` BMP header written by zl opens in a real image viewer.
7. `verify.ps1` is green: fixpoint holds, three engines 6/6, and the struct suite
   is inside the gate rather than exempted from it.
8. `docs/REFERENCE.md` documents the raw-memory builtins, `struct`, `sizeof`,
   `offsetof`, the `!` placement rule (M2), Rule M1's `peek64` caveat, and — in
   plain words — that **a struct handle is an address and assignment aliases it**.
