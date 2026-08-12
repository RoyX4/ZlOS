# Design: FFI, Direct Syscalls, and Freestanding Output

**Status:** proposal
**Author:** language/compiler design pass, 2026-07-31
**Scope:** W5 "systems capability" — the FFI half. Companion to the raw-memory/struct
half. No code is changed by this document.

---

## 0. TL;DR

The native backend already produces a **freestanding, CRT-free PE that imports only
kernel32** (`nativert.c` / `nativeval.c`: custom entry at RVA 0x1000, `VirtualAlloc`
for the heap, `ExitProcess` to leave, `dumpbin /imports` shows no msvcrt/ucrtbase/
vcruntime). So "freestanding output" is not a thing to build from scratch — it is a
thing to *formalize and expose*. What is genuinely missing is the ability to reach
**any** function in **any** DLL, which is what makes zl "as good as C" for systems work.

This proposal adds, in order:

1. **Freestanding, made explicit.** Expose the subsystem (console vs GUI) and entry
   point that the native PE writer already hardcodes, so a zl program can declare it
   builds a windowed app with no CRT. Nothing new is invented; `write_pe` in
   `nativert.c` grows two knobs.
2. **Raw FFI primitives.** Add `LoadLibraryA` / `GetProcAddress` / `FreeLibrary` to the
   runtime's import table (kernel32 already provides all three, so the "no libc"
   property survives), plus a hand-assembled **generic call trampoline** `zl_ccall`
   that marshals a runtime-known number of zl values into the x64 calling convention —
   shadow space, 16-byte alignment, register-then-stack argument placement — and boxes
   the return. Exposed as the builtins `dll(path)`, `sym(handle, name)`,
   `call(fnptr, args)`.
3. **`extern` sugar** over those builtins, so a Win32 function reads like a normal zl
   call, plus a **typed marshaller** (pointers, `u32`/`i64`, and `f64` args in `xmm`).
4. **Callbacks** — a compiler-generated thunk that lets a zl function be passed where C
   wants a function pointer.
5. **An `asm` escape hatch** — inline machine code spliced straight into the native
   `.text` stream.

Every construct on this list is a way to violate every invariant the language protects
(the low-bit tag, stack alignment, register discipline, memory safety). So the second
half of this document is a single consistent **security gate**: FFI, `asm`, and
freestanding entry all require the language's existing `!` "I mean it" danger marker,
and all three are exempt from the three-engine byte-identical test the way file I/O
already is.

---

## 1. Motivation

### 1.1 What "as good as C" actually requires

`OVERNIGHT_CAMPAIGN.md` W5 lists six systems capabilities. Three of them — raw memory,
structs, fixed-layout records — are the companion agent's half. The other three are
this document:

> - direct Win32 syscalls without libc (`nativert.c` already proves kernel32-only works)
> - freestanding output: no CRT, custom entry point, `-nostdlib`
> - a real FFI so zl can call any DLL

Without FFI, zl can compute but cannot *do* anything the runtime's ~30 builtins don't
already hardcode. Every new OS capability (a window, a socket, a registry key, a thread)
would otherwise mean hand-assembling another routine into `nativert.c`. FFI collapses
that infinite tail into one trampoline: once zl can call `GetProcAddress` and then call
the pointer it returns, the entire Win32 surface — and every third-party DLL — is
reachable from zl source with no C and no compiler.

### 1.2 Freestanding is already done — this is a trap worth naming

It is tempting to write a "freestanding output" epic. Do not. Read `nativert.c`'s
`emit_main` (line 1235) and `write_pe` (line 1460):

- The entry point is the code at RVA 0x1000. There is no CRT `mainCRTStartup`, no
  `argc`/`argv` setup, no atexit table. The program starts at our first byte.
- The heap is `VirtualAlloc`, not `malloc`. Output is `WriteFile`, not `printf`.
- The only imports are seven kernel32 functions. `raw_rt.exe` is 3580 bytes.

That *is* `-nostdlib` with a custom entry point. The native backend has been
freestanding since brick 1. The real gaps are narrow:

1. The subsystem is hardcoded to console (`pu16(opt+68, 3)` in `write_pe`). A GUI
   program needs subsystem 2 and must not spawn a console.
2. There is no way for a zl program to *say* "I am freestanding / I am a GUI app" — the
   choice is baked into the C of the backend, not driven by the zl source.
3. The C backend (`compile.c`) and LLVM backend (`compilel.c`) still link the CRT.
   Freestanding there means passing `-nostdlib -e <entry>` to clang and providing an
   entry — a separate, smaller concern noted in §6.4, not the main line.

So the freestanding "feature" is: lift those two hardcoded PE fields into options and
let the zl source select them. That is Stage 1, and it is small precisely because the
hard part shipped already.

### 1.3 The invariant FFI cannot preserve, and why that's fine

`verify.ps1` asserts interpreter == C backend == native backend, byte for byte. FFI
breaks that on contact: `call(GetProcAddress(...))` returns a raw address that differs
every run and across engines; `MessageBoxA` has a side effect no diff can capture. This
is not new. File I/O already returns process-specific data, and the design of the test
suite already carves out non-deterministic behavior. **FFI, `asm`, and freestanding
programs are graded by their own targeted tests, not by the cross-engine equality gate.**
§7 makes this a rule rather than an accident.

---

## 2. Design decisions (proposed, to be locked)

### 2.1 The primitive is three builtins, not a keyword

zl's iron rule (locked spec §4.4, restated in `design_types.md`): **every reserved word
is a name the user loses forever, and the list is frozen at the current keywords.** The
type-system proposal added *zero*. This one does too, at the primitive layer.

The FFI primitive is three ordinary builtins — identifiers, exactly like `poke` and
`dir`:

| Builtin | Signature | Returns |
|---------|-----------|---------|
| `dll(path)`          | load a DLL by name         | an opaque handle (a tagged int: the `HMODULE`) |
| `sym(handle, name)`  | resolve an exported symbol | a function pointer (a tagged int address) |
| `call(fnptr, args)`  | invoke it                  | the return value, boxed as a tagged int |

```
user32 = dll("user32.dll")
mbox   = sym(user32, "MessageBoxA")
call!(mbox, [0, cstr("Built with zl"), cstr("zl"), 0])
```

Rationale: builtins are how zl already exposes the OS (`read`, `write`, `dir`, `procs`).
FFI is the same category of thing. No grammar change, no lexer change, no reserved word.
`extern` (§2.5) is *sugar over these three*, added only if the ergonomics justify it —
and it is designed so it need not be a keyword either.

### 2.2 Pointers are tagged integers — and this is safe on Win64

A zl value is one 8-byte word; the low bit tags integers (`(n<<1)|1`), leaving **63
bits** of magnitude. Win64 user-mode virtual addresses are bounded at **2^47 − 1** (the
low half of canonical address space; the high half is kernel-only). 2^47 fits in 63 bits
with 16 bits to spare. Therefore:

> **A native pointer round-trips losslessly through a zl tagged integer.**
> `HMODULE`, `FARPROC`, a `cstr` buffer address, a `VirtualAlloc` result — all are just
> tagged ints. `dll`/`sym` return them; `call` and the raw-memory `peek`/`poke` consume
> them.

This is the single decision that makes the whole FFI layer typeless and cheap: there is
no new "pointer" value kind, no boxing, no heap object. It also dovetails with the
companion raw-memory half — `alloc(n)` returns a tagged-int address, `call` accepts it,
they interoperate for free.

Rejected alternative — a dedicated boxed pointer object (`[+0]=type 4, [+8]=addr`): type
safety at the cost of an allocation per pointer and a second representation the
trampoline would have to unwrap. Not worth it; the 63-bit headroom makes tagging exact.

*One caveat, recorded honestly:* a raw `read()` of 8 arbitrary bytes reinterpreted as a
pointer could in principle set the top bit; the tag would then be wrong. Marshalling
(§2.4) untags with arithmetic `sar` and only ever tags addresses that came from the OS,
which are always < 2^47. Addresses never manufactured by zl arithmetic are safe.

### 2.3 v1 marshalling: everything is a 64-bit integer

`call(fnptr, args)` in v1 treats every element of `args` as a 64-bit integer argument:
it untags each tagged int (`sar rax,1`) and drops it into the ABI slot. That is enough
for the overwhelming majority of Win32:

- Integers, `BOOL`, handles, `HWND`, flags → already tagged ints.
- Strings → the caller converts with **`cstr(s)`**, which returns the address of a
  NUL-terminated arena copy. `zl_cstr` **already exists in `nativert.c`** (line 876) —
  v1 marshalling is mostly wiring, not new machine code.
- Buffers → a raw-memory `alloc(n)` address (companion half), passed as an int.

Floats and by-value structs are **out of v1** (see §2.4 and non-goals). This mirrors the
MASTER_PLAN discipline the float doc cites: "correctness and coverage first, the typed/
fast path later." Integer-only FFI reaches ~90% of the Win32 API on day one.

### 2.4 The x64 calling convention, stated exactly (this is load-bearing)

The trampoline must reproduce the Microsoft x64 ABI precisely or it corrupts the callee's
stack. The rules, and how `zl_ccall` satisfies each:

| ABI rule | Detail | Trampoline's job |
|----------|--------|------------------|
| First 4 integer args | `rcx, rdx, r8, r9` | move `argv[0..3]` into these |
| Args 5+ | pushed to the stack at `[rsp+0x20]`, `[rsp+0x28]`, … **left to right** | store the tail after shadow space |
| **Shadow space** | caller *always* reserves 32 bytes above the return address, even for ≤4 args — the callee owns it | subtract 0x20 before every call |
| **16-byte alignment** | `rsp` must be a multiple of 16 **at the `call` instruction**; since `call` pushes an 8-byte return address, the callee sees `rsp ≡ 8 (mod 16)` at entry | `and rsp,-16` then fix parity for the pushed args |
| Return | integer/pointer in `rax`; `f64` in `xmm0` | box `rax` as a tagged int |
| Nonvolatile | `rbx, rbp, rdi, rsi, r12–r15` preserved across the call | trampoline saves what it uses |
| Float args | first 4 float args in `xmm0–xmm3`, **and** the shadow of the matching integer register is still consumed | v2 only |

`nativert.c` already lives by these rules — `and rsp,-16` (the brick-1 lesson, line 177),
`st_arg32`/`st_arg0` for the 5th+ Win32 argument (line 139), and the note that "every
routine keeps rsp 16-byte aligned at nested call sites." `zl_ccall` generalizes the
fixed-arg-count call sites already in `emit_read`/`emit_write_file` into a **loop over a
runtime-known argc**.

Concrete shape of the trampoline (pseudo-assembly; encodings follow the existing
`nativert.c` idiom of hand-built byte arrays with computed displacements):

```
; zl_ccall(rcx = fnptr, rdx = argv ptr, r8 = argc)   -> rax = raw return
zl_ccall:
    push rbp
    mov  rbp, rsp
    push rsi ; push rdi ; push rbx          ; save nonvolatiles we clobber
    mov  rsi, rdx                            ; rsi = argv
    mov  rbx, rcx                            ; rbx = fnptr
    mov  rdi, r8                             ; rdi = argc

    ; ---- reserve the stack: 0x20 shadow + 8*max(0,argc-4), 16-aligned ----
    lea  rax, [r8-4]
    test rax, rax
    cmovs rax, <0>                           ; stackargs = max(0, argc-4)
    lea  rax, [rax*8 + 0x20]                 ; bytes = shadow + 8*stackargs
    add  rax, 15
    and  rax, -16                            ; round the reservation to 16
    sub  rsp, rax
    and  rsp, -16                            ; and land rsp on a 16 boundary

    ; ---- register args: argv[0..3] -> rcx,rdx,r8,r9 (untagging each) ----
    ;   for i in 0..min(argc,4): reg[i] = sar(argv[i], 1)
    ; ---- stack args: argv[4..] -> [rsp+0x20 + 8*(i-4)] (untagged) ----

    call rbx                                 ; rsp ≡ 0 (mod 16) here, correct
    ; rax = raw return; leave it raw, caller boxes with zl_int

    lea  rsp, [rbp - 24]                     ; drop the arg area
    pop  rbx ; pop rdi ; pop rsi
    leave
    ret
```

The register-arg and stack-arg placement are two short unrolled/looped copies; because
`nativert.c` computes every jump displacement mechanically (`jz_fwd`/`land`/`jmp_back`,
line 122) rather than hand-counting — the brick-2 lesson — the loop is safe to write.

**Untagging happens in the trampoline, not the caller**, so `call([..])` accepts ordinary
tagged zl ints. A value that must be passed *raw* (an address that shouldn't be shifted)
is already a tagged int holding the address, and `sar` recovers it exactly (§2.2).

### 2.5 `extern` — sugar, and keyword-free by construction

Writing `dll`/`sym`/`call` by hand is tedious for a whole API. The sugar makes a foreign
function look native:

```
extern "user32.dll" MessageBoxA(hwnd, text, caption, flags) -> i32
...
MessageBoxA(0, "Hi", "zl", 0)          # reads like any zl call
```

**Do not make `extern` a keyword.** Two keyword-free ways to get this, decide in Stage 0:

- **(a) Contextual keyword.** `extern` is special *only* as the first token of a
  statement immediately followed by a string literal. Everywhere else `extern` is a
  normal identifier, so no name is stolen. The lexer stays unchanged (`extern` lexes as
  `T_IDENT`); the parser recognizes the shape `IDENT("extern") STRING IDENT ( … )` in
  statement position. This is the honest cost: a tiny parser special-case, zero reserved
  words. **Recommended.**
- **(b) Pure builtin, no sugar.** `MessageBoxA = bind("user32.dll", "MessageBoxA")`
  where `bind` returns a *callable value* and `N_CALL` on a callable value dispatches
  through `zl_ccall`. This needs the interpreter/backends to accept "call a value that
  is neither a builtin name nor a user `fn`," which is a real semantic addition — but it
  also gives first-class function pointers for free, which callbacks (§2.7) want anyway.

Lead with (a) for readability; (b) is the deeper change and pairs naturally with
callbacks. The `-> i32` return annotation reuses the type-system tokens (`design_types.md`
already lexes type names as identifiers) and drives the typed marshaller.

### 2.6 The typed marshaller (v2)

v1 is integer-only. v2 attaches a type list to each foreign function so the marshaller
can:

- **auto-`cstr` a string argument** (so `MessageBoxA(0,"Hi","zl",0)` works without manual
  `cstr`),
- **place `f64` args in `xmm0–xmm3`** (with the paired integer-register shadow consumed,
  per the ABI), using `movq xmm, r` (`66 48 0F 6E`) — the encodings the float doc already
  specced for the SSE build-out,
- **box the return by declared type**: `i32`/`i64`/`ptr` → tagged int; `f64` → the
  float representation the float-doc backend introduces; `void` → `nil`.

Type tags are plain identifiers: `i32 u32 i64 u64 ptr str f64 void`. They are *not*
keywords; they are recognized positionally inside an `extern` signature, the same way
`design_types.md` recognizes `num`/`str` after a `:`.

### 2.7 Callbacks — a compiler-generated thunk

Some APIs (`EnumWindows`, `qsort`, window procedures) take a function pointer that *they*
call, with the **C** ABI. A zl `fn` uses our own convention (tagged args). Bridging needs
a **thunk**: a small stub with the C ABI that boxes its incoming raw integer args as
tagged ints, calls the zl function's native entry, then untags the return.

```
; thunk for a 2-arg C callback -> zl fn at off_zlfn
thunk:
    push rbp ; mov rbp,rsp ; sub rsp,0x20
    lea  rcx, [rcx*2+1]        ; box arg0  (rcx = (rcx<<1)|1)
    lea  rdx, [rdx*2+1]        ; box arg1
    call off_zlfn              ; zl fn, returns a tagged value in rax
    sar  rax, 1               ; unbox to a raw C return
    leave ; ret
```

The thunk lives in `.text` (already executable), so on the native backend `cbthunk(f)`
just returns the thunk's address as a tagged int. v1: **max 4 integer args, integer
return, native backend only.** Floats, >4 args, and interpreter-side callbacks (which
would need `VirtualAlloc(PAGE_EXECUTE_READWRITE)` to build a thunk at runtime) are
non-goals for the first cut.

### 2.8 The `asm` escape hatch

The total escape: raw machine code spliced into the output.

```
asm!([0x48, 0x31, 0xC0])       # xor rax,rax  — emitted verbatim into .text
```

Design decisions:

- **A builtin, `asm`, not a keyword.** Argument is a **list literal of byte values** —
  it must be a compile-time-constant list, because the bytes are placed at codegen time,
  not evaluated at runtime.
- **Native backend only.** `asm` on the interpreter, C backend, or LLVM backend is a
  **hard compile error** with a clear message. It cannot be portable by definition.
- **It runs in the current frame with our register conventions.** The block is
  responsible for preserving anything it clobbers and leaving `rsp` as it found it.
  A result is whatever it leaves in `rax` (a following statement can read it only via a
  second construct; v1 treats `asm(...)` as a statement, not an expression).
- **It requires the `!` danger marker** (§7). `asm` without `!` is refused by the parser.

Rationale: this is the primitive that makes the *rest* of the systems layer bootstrappable
— any instruction the backend doesn't emit (a `cpuid`, an `rdtsc`, an `int 3`, a `syscall`)
becomes reachable without touching the C of `nativegen.c`. It is also the single most
dangerous construct in the language, which is exactly why it is gated hardest.

---

## 3. Current-state map (for the implementer)

| Concern | Interpreter (`interp.c` / `os_win.c`) | Native (`nativert.c` / `nativeval.c`) |
|---|---|---|
| Freestanding | n/a (links CRT, is the dev tool) | **already freestanding**, kernel32-only PE |
| Custom entry | n/a | entry at RVA 0x1000, `emit_main`/nativeval entry |
| Subsystem | n/a | **hardcoded console** (`write_pe`, `pu16(opt+68,3)`) |
| DLL load | `<windows.h>` available in `os_win.c` | **absent** — must add to IAT |
| Call convention plumbing | C compiler does it | `st_arg32`/`st_arg0`, `and rsp,-16` (fixed argc) |
| String → C ptr | trivial in C | **`zl_cstr` exists** (`nativert.c:876`) |
| Pointer value | a `double` holding an address (lossy > 2^53!) | a tagged int (exact to 2^63) |
| `asm` | — | — |

Two things to read from this table:

1. The **native backend is where FFI belongs first** — it is already CRT-free and its
   pointers are exact tagged ints. The interpreter's numbers are `double`, so an address
   above 2^53 loses precision there; interpreter-side FFI (§6.3) needs care and is
   secondary.
2. Most of the machinery — the ABI call sites, `and rsp,-16`, `zl_cstr` — **already
   exists in `nativert.c`**. FFI is largely generalizing fixed call sites into a
   trampoline and adding three imports, not green-field assembly.

---

## 4. Staged implementation sketch

Each stage is independently shippable and ends with a **targeted** verification (not the
cross-engine gate — see §7). Native-backend files (`nativert.c`, `nativeval.c`) are
serial-only per the campaign's disjoint-ownership rule.

### Stage 0 — Lock the spec (no code)

- Ratify §2: builtins-first primitive; pointers as tagged ints; v1 integer-only
  marshalling; `!`-gated FFI/`asm`; cross-engine exemption (§7).
- Decide `extern` sugar form (contextual keyword vs callable-value builtin) — §2.5.
- Freeze the type-tag identifier set for `extern` signatures — §2.6.
- **Verify:** reviewed and frozen. Nothing to run.

### Stage 1 — Freestanding, exposed

Lift what `nativert.c`/`nativeval.c` already hardcode into options.

1. Parameterize `write_pe`: a `subsystem` field (3 = console, 2 = GUI → `pu16(opt+68, …)`)
   and an optional entry-symbol override (default: RVA 0x1000, unchanged).
2. Drive it from the zl source with a directive that costs no keyword — a pragma-style
   builtin call at top level, e.g. `subsystem!("gui")`, read by the backend before
   codegen.
3. Document in `REFERENCE.md` that `nativegen`/`nativeval` output is `-nostdlib` with a
   custom entry — stating the existing reality, not adding behavior.

- **Verify:** the existing console programs still build and run byte-identically to
  before (this stage must be a no-op for them); a `subsystem!("gui")` program produces a
  PE that runs without opening a console window. `dumpbin /imports` still shows
  kernel32-only.

### Stage 2 — Raw FFI primitives (integer-only)

The core stage. All in `nativert.c` (runtime) + `nativeval.c` (builtin dispatch).

1. **Grow the import table** in `nativert.c`'s `write_pe` from 7 to 10 kernel32 imports:
   add `LoadLibraryA`, `GetProcAddress`, `FreeLibrary`. Mechanical: extend `nrva[]`,
   the IAT slot `#define`s (`IAT_*`), and the hint/name block. Still no libc.
2. **`emit_dlopen`** (`zl_dlopen`): `cstr` the name (reuse `zl_cstr`), `call [LoadLibraryA]`,
   tag the `HMODULE` in `rax` with `zl_int`.
3. **`emit_dlsym`** (`zl_dlsym`): untag the handle, `cstr` the name, `call [GetProcAddress]`,
   tag the `FARPROC`.
4. **`emit_ccall`** (`zl_ccall`) — the trampoline of §2.4: reserve shadow + stack, place
   up to 4 register args (untagging), place the tail on the stack (untagging), align,
   `call`, box `rax`.
5. **Builtins** `dll`/`sym`/`call` in `nativeval.c`'s dispatch, lowering to the three
   routines. `call`'s second argument is a zl list; the backend passes its slot base and
   count as `argv`/`argc`.

- **Verify:** a compiled zl program calls `GetTickCount` (0 args, returns a number that
  increases), then `Beep(750, 300)` (2 args, audible), then `MessageBoxA` via
  `user32.dll` (4 args incl. two `cstr` strings). A separate check confirms
  `sym(dll("kernel32.dll"), "GetProcAddress")` returns a nonzero address — i.e. FFI can
  resolve the very function it is built on. Not part of `verify.ps1`'s diff.

### Stage 3 — `extern` sugar + typed marshaller + float args

1. Parser: recognize the `extern "lib" Name(params) -> ret` form (contextual, per §2.5),
   desugaring to a hidden `dll`+`sym` at load and a `call` at each use, carrying the type
   list.
2. Typed marshaller in `zl_ccall` (or a `zl_ccall_typed` variant): auto-`cstr` `str`
   args, box the return by declared type, and route `f64` args to `xmm0–xmm3`
   (`movq xmm,r`) with the paired shadow integer register consumed.
3. Depends on the float doc's SSE backend for `f64` boxing/return; if that hasn't
   landed, ship `i32/i64/ptr/str/void` typing and defer `f64` with a clear "float FFI
   needs Stage 2 of design_floats" note.

- **Verify:** `extern "user32.dll" MessageBoxA(hwnd, text, caption, flags) -> i32`
  called with bare string literals (no manual `cstr`); an `f64`-taking function
  (e.g. `sqrt` from the CRT math DLL, if float landed) returns the right value.

### Stage 4 — Callbacks

1. `emit_thunk` generator (§2.7) parameterized by arg count and target zl-fn offset.
2. Builtin `cbthunk(f)` returns the thunk address as a tagged int.

- **Verify:** pass a zl comparator through a C `qsort` (from a CRT DLL) or a zl callback
  to `EnumWindows` and observe it fire once per top-level window.

### Stage 5 — `asm` escape hatch

1. Builtin `asm(list-of-bytes)`, native-backend-only, spliced into `.text` at codegen;
   hard error on interpreter/C/LLVM.
2. Enforce the `!` danger marker in the parser (§7).

- **Verify:** an `asm!([...])` block that (say) sets `rax` via a `mov`/`ret` sequence and
  whose effect is observed; the same program is rejected with a clear diagnostic under
  `interp.exe` and `compile.exe`.

---

## 5. Real syntax, end to end

What a zl program looks like once all stages land:

```
subsystem!("gui")                      # Stage 1: freestanding GUI, no console, no CRT

extern "user32.dll" MessageBoxA(hwnd, text, caption, flags) -> i32   # Stage 3 sugar
extern "kernel32.dll" GetTickCount() -> u32

start = GetTickCount()

# ... work ...

took = GetTickCount() - start
MessageBoxA(0, "done in " + took + " ms", "zl", 0)     # str args auto-cstr'd

# Lower level, no sugar (Stage 2), for a DLL discovered at runtime:
h  = dll("winmm.dll")
beep = sym(h, "Beep")
call!(beep, [1000, 200])               # freq 1000 Hz for 200 ms

# The escape hatch (Stage 5), for an instruction the backend won't emit:
asm!([0x0F, 0x31])                     # rdtsc  -> edx:eax
```

Note every side-effecting foreign call carries `!`. `MessageBoxA(...)` reached via
`extern` also requires the marker — the sugar does not launder away the danger; see §7.

---

## 6. Interactions and secondary paths

### 6.1 Companion raw-memory half

`alloc`/`free`/`peek`/`poke` (the other agent) and FFI share one currency: **the tagged-int
address** (§2.2). `alloc(64)` returns an address `call` can pass as an `LPVOID` output
buffer; after the call, `peek(addr)` reads what the C function wrote. They were designed
to compose; neither needs to know about the other beyond "addresses are tagged ints."

### 6.2 `os_win.c` stays the interpreter's only `<windows.h>` file

Interpreter-side FFI (§6.3) must keep `os_win.c`'s invariant intact: it is *the only file
that includes `<windows.h>`* (its header comment says so, to stop `TokenType`-style name
clashes). So the interpreter's `dll`/`sym`/`call` implementations live in `os_win.c`
behind small `os_*` shims (`os_dlopen`, `os_dlsym`, `os_invoke`), called from
`interp.c`'s `call_builtin` — never `<windows.h>` in `interp.c`.

### 6.3 Interpreter FFI is real but secondary

The interpreter *can* do FFI: `os_win.c` has `LoadLibrary`/`GetProcAddress` for free, and
a libffi-style integer trampoline is a few dozen lines of C. Two caveats make it
secondary to the native path:

- **Address precision.** Interpreter numbers are `double`; addresses above 2^53 lose
  low bits. Mitigation: store FFI handles/pointers in an integer-exact side channel, or
  accept the (rare on today's Win64 layouts) risk and document it. The native backend
  has no such problem (tagged ints are exact to 2^63).
- **The equality gate.** Interpreter and native FFI will not agree byte-for-byte (§1.3),
  so they are tested separately regardless.

Ship native FFI first (Stage 2); add interpreter FFI when a program needs to be *run*
(not compiled) against a real DLL.

### 6.4 LLVM/C backends and `-nostdlib`

`compile.c` and `compilel.c` emit C / LLVM IR that clang links against the CRT. Genuine
freestanding there means emitting `-nostdlib -e <entry>` and a `_start`. That is a
separate, smaller epic (clang does the ABI work; we only supply the entry and avoid CRT
calls). It is **not** on this document's main line — the native backend is already
freestanding, which is the property W5 actually asks for. Noted so the option is on the
map.

---

## 7. Security and stability

Every construct here is a licensed invariant violation. The language already has the
right primitive for "this is dangerous, I mean it": the **`!` danger marker** (`N_DANGER`,
`parser.h:19`; `REFERENCE.md` "Danger marker `!` — means 'I mean it'"). This proposal
puts it to work as the single, consistent FFI/`asm` gate.

**The rule:** `call`, `asm`, `subsystem`, and any `extern`-declared foreign call **must**
carry the `!` marker at the call/use site. The parser refuses the un-marked form. This is
not decoration — it makes every place the program can escape zl's safety grep-able
(`grep -n '!' `) and reviewable, exactly the way the codebase already treats `kill!`,
`rm!`, `poke!`.

Per-construct hazards and mitigations:

| Construct | What it can break | Mitigation |
|---|---|---|
| `call(fnptr, args)` | Wrong argc/types → callee reads garbage past the args, or an ABI mismatch corrupts the caller's stack and returns into hyperspace. A hostile/incorrect `fnptr` executes arbitrary code. | `!` gate; v1 fixed integer marshalling with a documented contract; the trampoline saves nonvolatiles and restores `rsp` from `rbp` so a *convention* mistake by the callee can't unwind the trampoline itself. |
| Marshalling | A zl value the callee treats as a pointer but which isn't → wild read/write. Tag/untag mismatch (§2.2 caveat) → off-by-one-bit address. | Untag with arithmetic `sar`; only tag OS-returned addresses; `cstr`/`alloc` are the sanctioned pointer sources. |
| `dll`/`sym` | Loading an attacker-controlled DLL path = code execution (classic DLL-hijack). `sym` of a missing name returns 0; calling 0 faults. | `!` gate; document that `dll` takes a *trusted* path; `call` should be preceded by a nonzero-pointer check the program is responsible for (v1 does not auto-check — noted). |
| Freestanding / custom entry | No CRT means no stack-overflow guard page setup beyond what the OS gives, no `/GS` cookies, no atexit. A `subsystem!("gui")` program that returns instead of `ExitProcess` falls off the end of `.text`. | The backend always terminates `main` with `ExitProcess` (it already does, `emit_main` line 1382); document that freestanding code owns its own stack discipline. |
| `asm(bytes)` | Total escape — can violate the tag invariant, misalign `rsp` (breaking every subsequent Win32 call, the brick-1 lesson), clobber a nonvolatile the caller relied on, or write anywhere. | Hardest gate: `!` **and** native-backend-only **and** compile-time-constant bytes; documented as "you now own the ABI." |
| Callbacks | The thunk runs on a stack the *foreign* code set up; a zl fn that allocates heavily can outrun assumptions; reentrancy into the interpreter is unsupported. | v1 native-only, ≤4 int args; thunk is minimal and preserves nothing it doesn't set. |

**Simulated-mode analog.** The interpreter already treats `kill`/`poke`/`rm` as
*simulated* no-ops (`interp.c:190`, the `SIMULATED[]` table) so running untrusted zl on
the dev machine is safe. FFI/`asm` should join that table: under the interpreter's
default (safe) mode, `dll`/`sym`/`call`/`asm` are simulated (log the intended call,
return `nil`/0) unless an explicit opt-in flag is set. The native backend, which is only
ever produced deliberately, performs them for real. This keeps "run a random `.zl` file
in the interpreter" from being a code-execution vector while leaving the compiled path
fully capable.

---

## 8. Non-goals (explicit)

- **By-value struct arguments / returns.** The x64 ABI passes small structs in registers
  and large ones by hidden pointer, with rules that depend on struct layout. That waits
  on the companion struct/record feature and is a v3 concern. v1/v2 pass struct *pointers*
  (a tagged-int address), which covers the majority of Win32.
- **Varargs to C (`printf`-style).** The ABI's vararg float-duplication rule
  (floats go in *both* the xmm and the shadow GPR) is a special case; defer until a real
  need appears. Most Win32 is fixed-arity.
- **SEH / `__try`/`__except`.** Catching a hardware exception from foreign code needs
  unwind tables the native backend doesn't emit. A fault in FFI'd code faults the process,
  by design.
- **Unloading correctness / refcounting DLLs.** `FreeLibrary` is exposed but the runtime
  does not track handles; double-free/use-after-free of a module is the program's
  responsibility (consistent with the no-GC arena model).
- **Non-Windows targets.** This is a Win64 design (`LoadLibraryA`, PE, the MS x64 ABI).
  A SysV/ELF port is a separate document.
- **Interpreter-side callbacks and `asm`.** Runtime thunk/code generation
  (`PAGE_EXECUTE_READWRITE`) in the interpreter is deferred; `asm` is native-only forever.
- **Cross-engine byte-identical FFI.** Impossible in principle (§1.3); replaced by
  targeted per-construct tests.

---

## 9. Definition of done

1. Spec §2 locked; `extern` form and type-tag set frozen (Stage 0).
2. `nativert.c`/`nativeval.c` emit `zl_dlopen`/`zl_dlsym`/`zl_ccall`; the import table is
   kernel32-only at 10 functions; `dumpbin /imports` shows no CRT.
3. A compiled zl program calls `GetTickCount`, `Beep`, and `MessageBoxA` correctly, and
   resolves `GetProcAddress` through its own FFI.
4. `subsystem!("gui")` produces a working windowed, CRT-free PE; console programs are
   unaffected.
5. `extern` sugar and (if float has landed) `f64` FFI work; a zl callback fires from a
   real Win32 enumerator.
6. `asm!([...])` splices bytes on the native backend and is a hard, clearly-diagnosed
   error on the other three engines.
7. Every FFI/`asm`/freestanding site requires `!`; the interpreter simulates them by
   default (added to `SIMULATED[]`); `verify.ps1`'s cross-engine gate is untouched
   (FFI programs live in a separate targeted suite).
8. `REFERENCE.md` documents `dll`/`sym`/`call`/`cstr`/`asm`/`extern`/`subsystem`, the
   `!` requirement, and the pointer-as-tagged-int contract.
```
