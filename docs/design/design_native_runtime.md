# Design: the no-C native runtime (Floor 4, step 6)

The goal of "delete C entirely": a native x86-64 backend that compiles the FULL language
(strings, lists, dynamic values) plus a **runtime written in hand-assembled x86-64 using only
Windows syscalls — no libc**. When this exists and can compile `src/selfhost/compiler.zl`, there is no C
anywhere in the toolchain's output.

This doc is the implementation spec so the multi-session build is mechanical, not exploratory.

## 1. Value representation — low-bit tagging

Every zl value is 8 bytes (one machine word).

- **Integer:** `(n << 1) | 1`. Low bit set → it's an int. Recover with `sar rax, 1`.
- **Pointer:** an 8-byte-aligned heap address (low bit 0). Points to a **heap object**:
  ```
  [ 8 bytes: type tag ]  [ payload... ]
      0 = string           [8: length][length bytes, NUL-padded to 8]
      1 = list             [8: count ][count * 8-byte value slots]
      2 = bool             [8: 0 or 1]        (or fold bools into tagged ints)
      3 = nil              (a single shared nil object)
  ```
- Booleans can be tagged ints (0/1) to avoid heap objects; simplest is `false=(0<<1)|1`, `true=(1<<1)|1`
  and treat truthiness by the untagged int. Decide at implementation: fold bools into ints.

Rationale: ints are the common case and stay register-cheap (one shift); everything else is a
heap object behind a pointer, uniform to handle.

## 2. Heap — bump allocator over VirtualAlloc  ✅ primitive proven (brick 1)

At program start, reserve a large arena once:
```
VirtualAlloc(NULL, 64 MB, MEM_COMMIT|MEM_RESERVE=0x3000, PAGE_READWRITE=0x04)  -> base
```
Store `base` and a bump pointer in two reserved global slots (in .bss/.data). `zl_alloc(nbytes)`:
round nbytes up to 8, return current bump, advance bump. No free (a compiler runs once; fine).
**Alignment lesson (from brick 1): VirtualAlloc requires `and rsp,-16` before the call.**

## 3. Runtime helper routines (the "native runtime")

nativegen emits `call` to these hand-assembled routines instead of inline logic — exactly like the
C backend emits `zl_binop(...)`. Custom calling convention (we own both sides): **args in rcx, rdx
(and rsi/rdi for helpers), result in rax; helpers preserve rbx/rsi/rdi/rbp per Win64 if they call
Win32 APIs, and must `and rsp,-16` before any Win32 call.**

| routine | signature | does |
|---------|-----------|------|
| `zl_alloc` | (rcx=nbytes) → rax=ptr | bump-allocate from the arena |
| `zl_int` | (rcx=n) → rax | tag: `(n<<1)|1` |
| `zl_untag` | (rcx=v) → rax | `sar rcx,1` (assumes int) |
| `zl_add` | (rcx=a, rdx=b) → rax | if both ints → tagged int add; else if both strings → concat; else if both lists → concat |
| `zl_sub/mul/div/mod` | (a,b) → rax | untag, op, retag (ints only; error on non-int) |
| `zl_cmp_lt/gt/le/ge/eq/ne` | (a,b) → rax | compare; return tagged bool (0/1) |
| `zl_truthy` | (rcx=v) → rax(0/1) | int!=0, non-empty string, non-empty list, non-nil |
| `zl_str_lit` | (rcx=ptr, rdx=len) → rax | make a heap string object from static data |
| `zl_concat` | (a,b) → rax | allocate len(a)+len(b), copy both, new string object |
| `zl_str_eq` | (a,b) → rax | byte compare |
| `zl_list_new` | (rcx=count) → rax | allocate a list object of `count` slots |
| `zl_list_set` | (list, i, v) | write slot (used to build a literal) |
| `zl_index` | (obj, i) → rax | list[i] or string char |
| `zl_len` | (v) → rax | string length or list count (tagged int) |
| `zl_push` | (list, v) → rax | new list = old + one slot |
| `zl_print` | (rcx=v) | **tag dispatch**: int → itoa+WriteFile; string → bytes+WriteFile; list → "[..]" |
| `zl_read/zl_write/zl_dir` | ... | file I/O via CreateFileA/ReadFile/WriteFile/FindFirstFileA — NO fopen |

`zl_print`'s itoa and the WriteFile path are already proven in src/backends/native/nativegen.c (print_int/print_str) —
port them into the runtime and add a tag check at the top.

## 4. Codegen changes (a new backend file, e.g. `src/backends/native/nativeval.c`)

Keep the working integer `src/backends/native/nativegen.c` intact. `src/backends/native/nativeval.c` is the boxed version:
- Every expression leaves a **tagged value** in rax (not a raw int).
- `N_NUMBER` → `mov rcx, n; call zl_int` (or inline `mov rax,(n<<1)|1`).
- `N_STRING` → emit bytes into a data region; `lea rcx,[rip+data]; mov edx,len; call zl_str_lit`.
- `N_LIST` → `zl_list_new` + `zl_list_set` per element.
- `N_BINARY op` → evaluate both (stack machine), `call zl_<op>`.
- `N_CALL` to a builtin → `call zl_<builtin>`; to a user fn → the existing call convention but values are tagged.
- `if/while` conditions → `call zl_truthy; test rax,rax; jz ...`.
- functions/recursion → unchanged shape; args and returns are tagged values.

## 5. The self-host finish

Once `src/backends/native/nativeval.c` compiles the full language, point it at `src/selfhost/compiler.zl` and the C toolchain files:
1. `nativeval.exe src/selfhost/compiler.zl` → `native_compiler.exe` (a native zl compiler, built by our native backend).
2. `native_compiler.exe src/selfhost/compiler.zl` → reproduces itself → **native self-host, zero C anywhere.**
3. Delete `src/backends/c/compile.c` and `src/runtime/runtime.c` usage from the
   mainline; C only remains as historical bootstrap.

## 6. Build order (each brick verifiable on its own)

Bricks 2–5 all live in **`src/backends/native/nativert.c`**, which emits `raw_rt.exe`. The file grows
one brick per commit and the proof program at its entry point accumulates, so
every brick re-verifies all the earlier ones on every run.

1. ✅ **DONE** heap via VirtualAlloc (`src/backends/native/nativeheap.c` → `raw_heap.exe`)
2. ✅ **DONE** tagged ints, `zl_alloc`/`zl_int`/`zl_untag`/`zl_mul`, `zl_write` tag dispatch
3. ✅ **DONE** heap strings: `zl_str_lit`, `zl_concat`, `zl_str_eq`, `zl_len`
4. ✅ **DONE** `zl_add` runtime dispatch (int / string / list), `zl_lt/le/gt/ge`, `zl_eq/ne`, `zl_truthy`
5. ✅ **DONE** lists: `zl_list_new/set/index/push/concat`, recursive `zl_write`
6a. ✅ **DONE** `zl_at` (returns a 1-char string), `zl_has` (substring) — the string builtins the self-host needs
7. ✅ **DONE** file I/O via syscalls: `zl_cstr` (NUL-terminate), `zl_read` (CreateFileA/ReadFile into a 1 MB buffer, length = bytes read), `zl_write_file` (CreateFileA CREATE_ALWAYS/WriteFile), both `CloseHandle` when done. No fopen. **All 7 self-host builtins now exist in machine code.**
6. ✅ **DONE** `src/backends/native/nativeval.c` — the boxed-value backend. Includes src/backends/native/nativert.c under `NATIVERT_LIB` to reuse the runtime. Stages: expressions → if/while → functions+recursion+global-scope → lists/index/for/builtins. Matches the interpreter on the full 6-program suite.
8. ✅ **DONE 🏆 NATIVE SELF-HOST.** `nativeval src/selfhost/compiler.zl → nvout.exe` (33 KB machine code, kernel32-only, no libc); `nvout.exe` compiles `src/selfhost/compiler.zl` → `out.c` **byte-identical** to the interpreter's (SHA256 ECE14032…, 887 lines).

**Status: ALL 8 bricks done. "Delete C from the OUTPUT" is complete** — the native compiler imports nothing but kernel32.

### The two bugs that stood between brick 6 and the self-host

1. **Fixed-array overflow in the generator.** The boxed backend emits a runtime
   call per operation, so compiling src/selfhost/compiler.zl made thousands of call sites —
   `cfix[256]` overflowed and corrupted memory (nativeval.exe itself crashed).
   Fix: `cfix` → 262144; `ufix`/`gnames`/`flocals`/`fnames` enlarged; PE layout
   grown (.text 512 KB, .idata 256 KB); nativeval arena → 64 MB.
2. **`+` must stringify.** The interpreter's `binop_plus` is num+num→add,
   list+list→concat, and *everything else* → `to_string(l)+to_string(r)`.
   src/selfhost/compiler.zl leans on this (`"zl_list_n(" + len(...)`). The old `zl_add`
   returned false for string+int, which propagated until `write()` dereferenced
   a tagged int as a string pointer → 0xC0000005. Fix: added `zl_int_str` and
   `zl_to_string`, rerouted `zl_add`'s general case through
   `zl_concat(to_string(a), to_string(b))`. Found by bisecting src/selfhost/compiler.zl
   (lexer OK → parser OK → compile_program crashed → string+int concat).

### What remains ("delete C from the SOURCE")

`src/backends/native/nativeval.c` and `src/backends/native/nativert.c` are still
C. The final step of MASTER_PLAN Floor 1 step 10
is to rewrite that backend in zl so the toolchain has no C source either, then
move the C files to `bootstrap/`. That is a separate, later climb; the OUTPUT is
already C-free today. `raw_rt.exe` is 3580 bytes of code and imports **7** kernel32 functions (VirtualAlloc, GetStdHandle, WriteFile, ExitProcess, CreateFileA, ReadFile, CloseHandle). No libc.

### brick 7 lessons

- **Same-process write-then-read needs the handle closed.** With the write handle still open, opening the file for read either shares-violates or reads 0 bytes. `src/selfhost/compiler.zl` reads-then-writes-then-exits so it never hits this, but `CloseHandle` after each op makes the runtime correct for all cases.
- **`ReadFile` writes only the low 32 bits of `*lpNumberOfBytesRead`.** Zero the full 8-byte slot first, or the string length gets garbage in its high bits.
- The bug that looked like a runtime crash was in the **proof**, not the runtime: `zl_read` returns the object in `rax`, but `zl_len` reads its argument from `rcx`. Missing `mov rcx,rax` → `len` dereferenced a stale `rcx` → 0xC0000005. Divide-and-conquer `ExitProcess`-probes (handle value, then byte count, then returned pointer) isolated it to the caller.

### note for brick 6

The arena is currently **4 MB** (`emit_main` VirtualAlloc). That is plenty for the proof, but compiling `src/selfhost/compiler.zl` (which allocates an AST of thousands of nodes plus every emitted string) will need more — bump to the 64 MB this doc's §2 specifies when brick 6 lands. `raw_rt.exe` is 2612 bytes of code + 96 bytes of data and
imports exactly four kernel32 functions (VirtualAlloc, GetStdHandle, WriteFile,
ExitProcess). `dumpbin /imports` shows no msvcrt, no ucrtbase, no vcruntime.

### What the TROPHY actually needs (measured, 2026-07-30)

The point of this runtime is the native self-host of `src/selfhost/compiler.zl` (Floor 1 step
10). `src/selfhost/compiler.zl` calls exactly **7** builtins — measured by scanning its source:

```
print  len  at  has  push  read  write
```

`print`, `len`, `push` are already done (bricks 2–5). So the native self-host is
gated on only **four more routines**, not the full builtin set:

| routine | job | brick |
|---|---|---|
| `zl_at` | char at index of a string → 1-char string (or byte) | 6 (small) |
| `zl_has` | is substring b inside string a → tagged bool | 6 (nested byte loop) |
| `zl_read` | file → heap string via CreateFileA/ReadFile | 7 |
| `zl_write_file` | heap string → file via CreateFileA/WriteFile | 7 |

The other ~24 builtins (`num int abs min max slice find ends upper lower split
join lines sort contains index_of dir procs input kill start rm poke peek`) are
needed only to run the **stdlib** natively, never to self-host. They are a long
tail of easy byte loops — build them any time, or leave them on the C backend
forever. Do NOT let them block brick 8.

### Decisions made during implementation (were open in this doc)

| Question | Decision | Why |
|---|---|---|
| bools: heap objects or tagged ints? | **tagged ints** — `false`=1, `true`=3 | no allocation, and `ne` becomes `xor rax,2` |
| does `print` add the newline? | split into **`zl_write`** (no newline) and **`zl_print`** (= write + `\n`) | lists must print their elements without newlines, so `zl_write` has to recurse |
| where does the list loop counter live? | **the stack frame** (`[rbp-16]`), not a register | `zl_write` recurses into itself for nested lists; a register would be clobbered |
| negative integers | `sar` (arithmetic), never `shr`; `print_int` emits `-` | `shr` turns −7 into a huge positive |

### Lessons that cost real time

1. **brick 1:** `VirtualAlloc` needs strict 16-byte stack alignment — `and rsp,-16`.
2. **brick 2:** never hand-count a jump displacement. `src/backends/native/nativeheap.c` wrote them as
   literals in comments; at 600+ bytes that is a guaranteed bug. `src/backends/native/nativert.c` uses
   `jz_fwd()` / `land()` / `call_to(&off_routine)` so every displacement is computed.
   Bricks 2–5 each ran correctly on the first attempt because of this.
3. Any routine that calls another must have a real prologue, or it breaks the
   16-byte alignment chain for whatever eventually reaches a Win32 call.

## 7. Risks / notes

- **Register discipline:** Win64 nonvolatile = rbx,rbp,rdi,rsi,r12-r15. Runtime helpers that call
  Win32 must preserve these and align the stack (`and rsp,-16`).
- **GC:** none. Bump-allocate, never free. A compile run is short-lived; the 64 MB arena is plenty.
  (A real GC is a separate, optional future project.)
- **Effort:** ~7 bricks, each a focused hand-assembly session. This is the honest "no C" finish —
  weeks of careful machine-code work, fully mapped here so execution is mechanical.
