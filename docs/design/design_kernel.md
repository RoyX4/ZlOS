# Design: a Kernel / Driver Target for zl (wave W6)

**Status:** proposal — **BLOCKED, do not start**
**Author:** language/compiler design pass, 2026-07-31
**Scope:** a new backend `kernelgen.c` (forked from `nativegen.c`), a small set of
privileged-instruction intrinsics, and a separate verification harness
`verify_kernel.ps1`. No existing engine, no existing file, and **no part of
`verify.ps1`** is touched by this work. No code is changed by this document.

---

## 0. TL;DR — and the ordering, stated honestly up front

`OVERNIGHT_CAMPAIGN.md` says of W6: *"Freestanding + no-libc + structs + raw
memory make this possible, not before."* That sentence is correct and this
document does not soften it.

**W6 cannot start.** Four things it needs do not exist:

| Missing thing | Where it must come from | Today |
|---|---|---|
| raw memory (`peek8/16/32/64`, `poke*`, pointer arithmetic) | **W5** | `peek`/`poke` are in the `SIMULATED[]` stub list, `interp.c:190` — they print a message and do nothing |
| structs with a fixed, known memory layout | **W5** | nothing; there is no aggregate type at all |
| freestanding output (no CRT, own entry, own section layout) | **W5** | closest thing is `nativert.c` → kernel32-only, which is *not* the same as no-OS |
| port I/O + privileged instructions (`in`/`out`/`lgdt`/`lidt`/`cli`/`hlt`) | **W6, specified here (§7)** | nothing |

What *does* exist, and is real groundwork rather than a hand-wave:

- **`pe_min.zl`** hand-builds a working PE32+ `.exe` from zl using `write_bytes`
  and byte-level appenders (`u8/u16/u32/u64/pad/ustr`). A UEFI application is a
  PE32+ file. **`pe_min.zl` is ~90% of a UEFI image writer already** (§3.2 lists
  the exact deltas — there are five of them).
- **`nativegen.c`'s `write_pe()`** does the same thing in C, with a computed
  section layout, and is what a kernel backend should fork.
- **`nativegen.c` is already an exact-int64 engine.** `N_NUMBER` goes through
  `atoll(n->text)` (l.142), *not* through a double, and every operator is a GPR
  instruction (`add`/`sub`/`imul`/`cqo;idiv`, l.98-102). This matters enormously
  and is the subject of §2.
- **`nativert.c`** proves the discipline that makes hand-assembly survivable —
  `call_to()` / `jz_fwd()` / `land()` compute every displacement instead of
  hand-counting it (its own lesson #2). The kernel backend inherits that or it
  will not work.

**The design.** Boot **UEFI, not legacy BIOS** (§3 — the deciding argument is
that the repo's PE writer *is* a UEFI writer, and that a BIOS boot sector would
require a 16-bit encoder zl does not have and would never otherwise need). Fork
**`nativegen.c`** (unboxed i64), not `nativeval.c` (boxed, needs a heap), because
the kernel subset *is* the integer subset. Add **six intrinsic builtins** for
port I/O and a handful of privileged instructions. Ship in this order:
UEFI hello → serial → GDT → ExitBootServices + own paging → IDT → physical page
allocator. **Serial before interrupts, always** — you cannot debug an IDT you
cannot print from.

**Sizing, blunt:** the UEFI hello-world and the polled serial driver are each a
weekend. The GDT is an afternoon. The IDT with 256 stubs and a usable exception
dump is a week. The page allocator is a weekend *given a memory map*, and getting
the memory map correctly across `ExitBootServices` is another week of
frustration. **The language work underneath (W5) is months, and it is the real
cost.** W6 itself, once W5 lands, is ~6 focused weeks. §9 breaks this down
per-item and does not round anything down.

---

## 1. What a freestanding kernel target needs that user-mode does not

`nativert.c` already produces an executable that imports nothing but kernel32.
It is tempting to call that "freestanding." It is not. Five things change when
there is no OS at all.

### 1.1 There is no libc, and also no kernel32

`raw_rt.exe` imports 7 functions (`VirtualAlloc`, `GetStdHandle`, `WriteFile`,
`ExitProcess`, `CreateFileA`, `ReadFile`, `CloseHandle`). Every one of those
disappears. Concretely, in the current codebase:

| Current runtime service | Provided by | Kernel replacement |
|---|---|---|
| `zl_alloc` arena | `VirtualAlloc` | §8 physical page allocator + bump on top |
| `zl_print` sink | `GetStdHandle`+`WriteFile` | §7 serial port `outb` loop |
| program exit | `ExitProcess` | `cli; hlt` loop (there is nothing to exit *to*) |
| `zl_read`/`zl_write_file` | `CreateFileA` etc. | nothing in v1. No filesystem. Cut. |

The good news: only the **sink** of `emit_print_int` (nativegen.c:296) changes.
Its itoa loop is arithmetic and stays byte-for-byte identical. That is the
single biggest piece of reuse available.

### 1.2 There is no heap until you write one — and the compiler must know that

The boxed backend (`nativeval.c`) emits a `zl_alloc` call for every string, every
list, and every `+` that stringifies. In a kernel, before `pmm_init()` returns,
**an allocation is a triple fault.** There is no way to make that safe by
convention.

The decision that falls out of this: **the kernel target compiles a subset with
no implicit allocation.** No dynamic lists, no string concatenation, no `push`.
String *literals* are fine (they are static bytes in `.rodata`). Fixed-size
buffers come from raw memory. This is not a limitation to apologise for — it is
what kernel C does too (`-ffreestanding`, no `malloc` in early boot).

Enforcement: `kernelgen.c` errors out on `N_LIST`, on `+` where either side is
not provably a number, and on `push`/`concat`/`join`/`split`. A hard error at
compile time, not a runtime surprise.

> Nuance worth knowing: under UEFI, before `ExitBootServices`, you *do* have a
> heap — `BootServices->AllocatePool`. It vanishes at EBS. So the boot-phase code
> may allocate through firmware; the post-EBS kernel may not, until §8 is up.

### 1.3 You own the entry point, and its signature is not `main`

Today `nativegen`'s entry is offset 0 of `.text`, sets up a frame, runs top-level
statements, and falls through to `ExitProcess(0)` (l.452-463). A UEFI image's
entry is:

```
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
```

— i.e. **`rcx` = image handle, `rdx` = system table pointer**, Microsoft x64 ABI,
return status in `rax`. Those two register values are the only things the
firmware ever gives you, and `SystemTable` is the root of everything (console,
memory map, ExitBootServices). They must be stashed into globals in the first
instructions, before anything can clobber them.

This is a real change of shape: the entry can no longer be "top-level statements
with locals in its frame." §5.3 covers the global-storage change that forces.

### 1.4 The link layout is yours, and it must be position-independent

`write_pe()` hardcodes `.text` at RVA 0x1000, `.idata` after it, and *asserts*
`idata_rva == 0x2000` — bailing with "program too big for the simple 1-page
layout" (nativegen.c:431). A kernel image is bigger than one page. The layout has
to become computed, with at least:

```
.text    RVA 0x1000   CODE|EXECUTE|READ            0x60000020
.rodata  RVA ...      INITIALIZED_DATA|READ        0x40000040
.data    RVA ...      INITIALIZED_DATA|READ|WRITE  0xC0000040
.bss     RVA ...      UNINITIALIZED_DATA|READ|WRITE 0xC0000080  (VirtualSize > SizeOfRawData)
.reloc   RVA ...      INITIALIZED_DATA|DISCARDABLE|READ 0x42000040
```

And a hard constraint that user-mode never imposed: **the firmware may load the
image at an address other than `ImageBase`.** So:

> **Rule K1: the kernel backend must never emit an absolute address.** Every data
> reference is `lea reg,[rip+disp32]`; every call is `call rel32`. Only integer
> *values* may be absolute immediates.

`nativegen` already obeys this for string literals (`sfix[]` patches RIP-relative
`lea` displacements, l.474-479) and for imports (`emit_call_import`). The audit
item is `nativeval.c`'s globals, which are reached through a runtime-held
`G_BASE` pointer into the VirtualAlloc arena — that indirection is exactly what
must be replaced with `[rip+disp]` into `.bss` (§5.3).

With K1 held, the `.reloc` section can contain a **single dummy
`IMAGE_REL_BASED_ABSOLUTE` (type 0) entry**, which is defined as a no-op padding
relocation. Some firmware refuses images with no relocation directory; this
satisfies them at a cost of 12 bytes and zero fixups.

### 1.5 The stack is small, and nobody grows it for you

UEFI guarantees only ~128 KiB of stack and there is no guard page — overflow is
silent corruption, not an exception. `pe_min.zl` writes `SizeOfStackReserve =
1 MiB`; **UEFI ignores that field.** So:

> **Rule K2: no large stack frames.** The 128 KiB page-allocator bitmap (§8) and
> the 4 KiB IDT go in `.bss`, never in a frame. `kernelgen` should refuse any
> function whose computed `frame` (nativegen.c:455) exceeds, say, 4 KiB.

---

## 2. The integer-exactness situation (better than expected, with one sharp edge)

Kernel code is made of 64-bit bit patterns: descriptors, page-table entries,
MSRs, physical addresses. zl's number is a `double`, which is exact only to
2^53. That looks fatal. It mostly is not, and the reason is worth writing down.

**`nativegen.c` never converts a literal to a double.** Line 142:

```c
case N_NUMBER: {
    long long v = atoll(n->text);          /* source text -> int64, no double */
    b(0x48); b(0xB8); for(int i=0;i<8;i++) b((unsigned char)((v>>(8*i))&0xFF));
```

and every operator is a GPR instruction on `rax`/`rcx` (l.98-102). So in the
nativegen lineage, arithmetic is **exact int64 end to end**. The kernel target
inherits that for free. This is the strongest single reason to fork `nativegen.c`
rather than anything else.

The sharp edges that remain, and the calls:

| Edge | Detail | Decision |
|---|---|---|
| Literals above 2^63 | `atoll` saturates. `0xFFFF800000000000` (a higher-half address) cannot be written. | Change `atoll` → `strtoull` in `kernelgen.c` (one line), and add hex literals to the lexer (§6.1). |
| Signed `/` and `%` | `cqo; idiv` (l.101-102) is *signed*. An address ≥ 2^63 divides wrong. | **v1 stays entirely in the low half.** UEFI hands you an identity map of low memory; a higher-half kernel is a W7 concern, not W6. Also add `udiv(a,b)`/`umod(a,b)` intrinsics later if needed. |
| Signed `<` `>` | same problem, same fix. | same: low half only. `ucmp_lt(a,b)` if it ever bites. |
| The **interpreter** disagrees | `interp.exe` will compute these through doubles and get different answers. | Accepted and explicit: **the kernel target is not part of the 3-engine agreement invariant.** §10 explains why that is safe rather than a hole. |
| `pe_min.zl` today | Its `u64()` does `v % 4294967296` on a double. `0x00AF9A000000FFFF` = 49,381,209,046,646,783 > 2^53 — it would silently write the wrong GDT entry. | Real, demonstrable, and the reason §4.1 writes descriptors as **two 32-bit halves**. It is also a one-line proof to put in the W5 raw-memory design doc's motivation section. |

**Net:** W6 does *not* need the W3 type system. It needs a `strtoull` and hex
literals. That is a genuinely cheaper dependency than it first appears, and it is
worth knowing before anyone schedules the type system as a W6 blocker.

---

## 3. Boot: UEFI. (The decision, and why BIOS loses.)

### 3.1 The argument

| | UEFI | Legacy BIOS |
|---|---|---|
| Image format | **PE32+** — `pe_min.zl` and `write_pe()` already emit this | flat 512-byte sector, magic `0xAA55` |
| CPU mode at entry | **long mode already**, paging on, low memory identity-mapped, a working GDT | **16-bit real mode.** You must write real→protected→long by hand |
| Does zl's backend emit that? | yes, it emits x86-64 today | **no.** nativegen has no 16-bit encoder and never will need one for anything else |
| Size budget | megabytes | **512 bytes** for stage 1, then you write a stage-2 loader and a disk read routine |
| First output | `ConOut->OutputString` — text on screen before you write any driver | nothing until you write to VGA text memory or program a UART |
| Memory map | `GetMemoryMap()` returns a clean array | BIOS `INT 0x15, EAX=E820` — a real-mode call, so it must happen in stage 1, and stashed somewhere for later |
| Test loop | `qemu-system-x86_64 -bios OVMF.fd -drive format=raw,file=fat:rw:esp/` — drop the file in a directory, run | build a disk image, install a partition table, chainload |
| Real hardware | everything since ~2012 | shrinking; CSM is being removed from new firmware |

**Decision: UEFI.** Two arguments are decisive on their own.

1. **The PE writer already exists.** §3.2 shows the delta is five header fields
   and one deleted section. A BIOS path throws all of `write_pe()` away.
2. **A BIOS boot sector needs a 16-bit instruction encoder.** That is a whole
   separate encoder — different operand sizes, segment arithmetic, `0x66`/`0x67`
   prefixes with inverted meaning — used exactly once, for ~200 bytes of code,
   and never again anywhere in the project. It is pure cost.

The counter-argument, taken seriously and rejected: *"real→protected→long by hand
is the education; UEFI hides it."* It hides mode switching, and mode switching is
a one-time 60-line ritual. It does **not** hide the GDT, the IDT, paging, the
PIC, port I/O, or the memory map — you build every one of those yourself
(§4, §5, §7, §8). The learning is intact; the yak is not shaved.

> Appendix A keeps the legacy sequence written down in case a BIOS stub is ever
> wanted for its own sake. It is not on the plan.

### 3.2 The exact deltas from `pe_min.zl` to a UEFI image

Five changes. This is the whole port.

```
1. Subsystem:  3 (WINDOWS_CUI)  ->  10 (EFI_APPLICATION)
   pe_min.zl:87   pe = u16(pe, 3)      ->  pe = u16(pe, 10)
   nativegen.c:389  pu16(opt+68,3)     ->  pu16(opt+68,10)

2. Delete the .idata section entirely, and data directories [1] (import)
   and [12] (IAT). UEFI has no dynamic linking. Everything you can call
   arrives as a function pointer inside the EFI_SYSTEM_TABLE in rdx.
   pe_min.zl:97-98,104-105 and its whole .idata block (l.149-180) go away.

3. Add a .reloc section with one IMAGE_REL_BASED_ABSOLUTE entry, and point
   data directory [5] at it:
       u32 PageRVA   = 0x1000
       u32 BlockSize = 12
       u16 Entry     = 0x0000      /* type 0 = ABSOLUTE = no-op padding */
       u16 pad       = 0x0000
   Characteristics 0x42000040 (INITIALIZED_DATA|DISCARDABLE|READ).

4. NumberOfSections 2 -> 5 (.text .rodata .data .bss .reloc), and
   SizeOfImage / SizeOfHeaders recomputed. write_pe()'s hardcoded
   "idata_rva must be 0x2000" assertion (nativegen.c:431) is deleted and
   replaced with a computed layout.

5. AddressOfEntryPoint points at efi_main, whose contract is
   rcx = EFI_HANDLE, rdx = EFI_SYSTEM_TABLE*, return EFI_STATUS in rax.
   The first thing it emits is:
       mov [rip+g_image_handle], rcx
       mov [rip+g_systab],       rdx
   before any other code can touch those registers.
```

Everything else in `pe_min.zl` — MZ stub, PE signature, COFF header, PE32+ magic,
section alignment 0x1000 / file alignment 0x200, ImageBase — is already correct
for UEFI.

### 3.3 Handoff: what firmware hands you, and what you do with it

Write the file to `esp/EFI/BOOT/BOOTX64.EFI`. That path is the spec's fallback
boot path — no NVRAM boot entry, no bootloader config, nothing to install.

```
qemu-system-x86_64 -bios OVMF.fd -drive format=raw,file=fat:rw:esp -serial stdio -display none
```

`fat:rw:` synthesises a FAT filesystem from a host directory. **There is no disk
image build step.** Recompile, rerun, done — that is what makes §10's automated
gate cheap.

`SystemTable` (rdx) field offsets you will need. All 8-byte pointers unless
noted; `EFI_TABLE_HEADER` is 24 bytes:

```
EFI_SYSTEM_TABLE
  +0    Hdr (24 bytes)
  +24   FirmwareVendor      CHAR16*
  +32   FirmwareRevision    u32 (+4 pad)
  +40   ConsoleInHandle
  +48   ConIn               EFI_SIMPLE_TEXT_INPUT_PROTOCOL*
  +56   ConsoleOutHandle
  +64   ConOut              EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL*
  +72   StandardErrorHandle
  +80   StdErr
  +88   RuntimeServices*
  +96   BootServices*
  +104  NumberOfTableEntries  u64
  +112  ConfigurationTable*

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL   (from ConOut)
  +0    Reset
  +8    OutputString(This, CHAR16 *str)     <-- hello world lives here
  +48   ClearScreen

EFI_BOOT_SERVICES                 (from +96)
  +0    Hdr (24 bytes)
  +40   AllocatePages
  +48   FreePages
  +56   GetMemoryMap
  +64   AllocatePool
  +72   FreePool
  +232  ExitBootServices
  +248  Stall
  +256  SetWatchdogTimer
```

> Verify these against the UEFI spec or gnu-efi's `efiapi.h` before trusting
> them; they are written here from the standard table layout and an off-by-one in
> a function-pointer offset is a silent jump into the wrong firmware routine.

Two gotchas that cost everyone a day:

- **The watchdog.** Firmware arms a 5-minute watchdog before calling you. Call
  `BootServices->SetWatchdogTimer(0, 0, 0, NULL)` in the first few lines or the
  machine resets mid-debug and you will blame your own code.
- **`OutputString` takes UTF-16.** ASCII string literals must be widened. Emit
  UTF-16LE literals directly in `.rodata` for the boot phase; §7's serial driver
  takes plain bytes and is the reason you only need this briefly.

---

## 4. GDT — your own segment descriptors

UEFI leaves a GDT loaded, but it lives in `EfiBootServicesData`, which becomes
free memory at `ExitBootServices`. Continuing to use it is using freed memory.
So: build your own, in `.bss`, before EBS.

### 4.1 The table

Six entries, 56 bytes (the TSS descriptor is 16 bytes in long mode):

| Sel | Entry | Value | Notes |
|-----|-------|-------|-------|
| 0x00 | null | `0x0000000000000000` | required |
| 0x08 | kernel code | `0x00AF9A000000FFFF` | access 0x9A, gran 0xAF (G=1, L=1, D/B=0) |
| 0x10 | kernel data | `0x00CF92000000FFFF` | access 0x92 |
| 0x18 | user code (ring 3) | `0x00AFFA000000FFFF` | access 0xFA (DPL=3) |
| 0x20 | user data (ring 3) | `0x00CFF2000000FFFF` | access 0xF2 |
| 0x28 | TSS | 16 bytes, type 0x89 | not needed in v1; reserve the slot now so selectors never move |

**Write these as two 32-bit halves, not one 64-bit literal.** `0x00AF9A000000FFFF`
is 4.9e16 and would round if it ever passes through a double (§2). In zl:

```
# gdt_lo / gdt_hi are the two halves of each descriptor
poke32(gdt + 0,  0x00000000)  poke32(gdt + 4,  0x00000000)   # null
poke32(gdt + 8,  0x0000FFFF)  poke32(gdt + 12, 0x00AF9A00)   # 0x08 kernel code
poke32(gdt + 16, 0x0000FFFF)  poke32(gdt + 20, 0x00CF9200)   # 0x10 kernel data
poke32(gdt + 24, 0x0000FFFF)  poke32(gdt + 28, 0x00AFFA00)   # 0x18 user code
poke32(gdt + 32, 0x0000FFFF)  poke32(gdt + 36, 0x00CFF200)   # 0x20 user data
```

The GDTR is 10 bytes — `u16 limit` then `u64 base`, where limit is
`size_in_bytes - 1`:

```
poke16(gdtr + 0, 56 - 1)
poke64(gdtr + 2, gdt)
lgdt(gdtr)
```

### 4.2 Reloading CS in long mode

`lgdt` does not change the segment registers. Data segments are a `mov`, but
**CS cannot be `mov`'d and there is no long-mode far `jmp` to a rel32 target.**
The standard trick is a far return:

```
push   0x08                    ; 6A 08          new CS selector
lea    rax, [rip + .after]     ; 48 8D 05 xx xx xx xx
push   rax                     ; 50
retfq                          ; 48 CB          far return -> pops RIP then CS
.after:
mov    ax, 0x10                ; 66 B8 10 00
mov    ds, ax                  ; 8E D8
mov    es, ax                  ; 8E C0
mov    ss, ax                  ; 8E D0
mov    fs, ax                  ; 8E E0
mov    gs, ax                  ; 8E E8
```

This is ~20 bytes of fixed machine code. It is **not** expressible in zl and
never will be. It belongs in `kernelgen.c` as an emitted prologue behind one
intrinsic:

```
lgdt(gdtr_addr)     # emits lgdt + the retfq CS-reload + the data-segment movs
```

Opinionated call: `lgdt()` does the whole ritual rather than exposing `retfq`
separately. There is exactly one correct way to do this and no reason to let a
caller get it wrong.

---

## 5. Long mode, ExitBootServices, and taking the machine

Under UEFI you are **already in long mode** — CPL 0, paging enabled, low memory
identity-mapped, interrupts enabled. So "protected/long mode entry" is not a mode
switch; it is a transfer of ownership. Six steps, in this order, and the order
matters:

```
1. cli                              -- firmware's IDT still points into firmware
2. build your GDT in .bss           -- §4
3. build your page tables in .bss   -- §5.1  (must exist BEFORE EBS)
4. GetMemoryMap  ->  ExitBootServices(ImageHandle, MapKey)   -- §5.2
5. lgdt + CS reload                 -- §4.2
6. mov cr3, <your PML4>             -- now firmware's page tables can die
7. build + lidt your IDT            -- §6
8. remap and mask the 8259 PIC      -- §6.4
9. sti
```

Steps 1-3 can happen in any order. Steps 4 through 9 cannot.

### 5.1 Your own page tables

Firmware's page tables live in `EfiBootServicesData` and are free memory after
EBS. Build your own first, in `.bss`, and load CR3 immediately after EBS.

**Use 2 MiB pages, not 1 GiB pages.** 1 GiB pages need a `CPUID.80000001:EDX[26]`
(PDPE1GB) check and are absent on some older and some virtual CPUs; 2 MiB pages
are universally available with PAE, which long mode mandates. The cost is 4 extra
pages of tables and a 2048-iteration loop. Take the simplicity.

Identity-map the low 4 GiB:

```
PML4[0]        -> PDPT                       (present|write = 0x03)
PDPT[0..3]     -> PD0..PD3                   (present|write)
PDn[0..511]    -> (phys) | 0x83              (present|write|PS=1, 2 MiB page)
                   where phys = (n*512 + i) * 0x200000
```

Storage: 1 PML4 + 1 PDPT + 4 PDs = **6 pages = 24 KiB in `.bss`, 4 KiB-aligned.**
Alignment is not optional — CR3 and every table pointer must be 4 KiB aligned, so
the backend needs a way to force alignment on a `.bss` reservation (§6.2).

Mapping 4 GiB covers RAM, the LAPIC at 0xFEE00000, and the IOAPIC at 0xFEC00000.
Anything above 4 GiB is a v2 problem.

### 5.2 The ExitBootServices dance

This is the single fiddliest thing in the whole document, and it fails in a way
that looks like a hardware fault. The rules:

1. Call `GetMemoryMap(&size, buf, &mapkey, &descsize, &descver)` with `size = 0`
   to learn the required size. It returns `EFI_BUFFER_TOO_SMALL` (0x8000000000000005).
2. Allocate **more than it asked for** — allocating changes the map, which grows
   it. Add 2 descriptors' worth of slack.
3. Call `GetMemoryMap` again for real.
4. Call `ExitBootServices(ImageHandle, mapkey)` **immediately**, with no
   allocation, no print, and no firmware call in between.
5. If it returns `EFI_INVALID_PARAMETER` (0x8000000000000002), the map changed
   under you. Go back to step 3 — *not* step 1 — and retry. Retry at most a few
   times, then give up loudly.
6. **After EBS succeeds, you may not call any boot service, ever.** Including
   `ConOut->OutputString`. This is why §7's serial driver must be working
   *before* you first attempt EBS: the moment EBS returns, your only output
   device is one you wrote yourself.

> Sequencing consequence, and it is the most important scheduling fact in this
> document: **serial comes before ExitBootServices.** Not after. An EBS bug with
> no console is debugged by bisecting with `hlt` instructions, which is as slow
> as it sounds.

### 5.3 Globals must move to `.bss`

`nativeval.c` puts globals in the first `nglobals*8` bytes of the VirtualAlloc
arena and reaches them through a runtime `G_BASE` pointer (its l.38-41). There is
no VirtualAlloc. The kernel backend keeps `nativeval`'s `gnames[]` /
`global_index()` / `register_global()` machinery but changes the *storage*:

- Reserve `nglobals*8` bytes at the start of `.bss`.
- A global read becomes `mov rax,[rip+disp32]`, a write `mov [rip+disp32],rax`,
  with `disp32` backpatched exactly like `sfix[]` does for string literals
  (nativegen.c:474-479).

That is RIP-relative, so Rule K1 holds and the image stays relocatable. It is
also *faster* than the arena indirection, which is a pleasant accident.

---

## 6. IDT — interrupts and exceptions

### 6.1 Gate descriptors

256 entries × 16 bytes = **4096 bytes in `.bss`, and it must be built at runtime**
because it holds the runtime addresses of your stubs.

```
offset 0   u16  handler[15:0]
offset 2   u16  code selector      = 0x08
offset 4   u8   IST index (0..7)   = 0 in v1
offset 5   u8   type/attr          = 0x8E interrupt gate (clears IF)
                                   = 0x8F trap gate      (leaves IF set)
offset 6   u16  handler[31:16]
offset 8   u32  handler[63:32]
offset 12  u32  zero
```

IDTR is 10 bytes, same shape as GDTR: `u16 limit = 4096-1`, `u64 base`.

Use **interrupt gates (0x8E)** for everything in v1. Trap gates leave interrupts
enabled inside the handler and you have no reentrancy story yet.

Writing a gate needs a 64-bit handler address split into three fields — which is
exactly the shape zl handles fine with `shr`/`band`, since each piece is ≤ 32
bits. No exactness problem here.

Hex literals: writing `0x8E` and `0xFFFF` is not optional for this kind of code.
`lex_number` accepts decimal only. **Add `0x` hex literal lexing** — a ~15-line
change to `lexer.c` — and it benefits every other wave too. This is the only
core-language change W6 asks for, and it must be scheduled as a serial,
alone-on-the-repo change per the campaign's rule 2.

### 6.2 The 256 stubs

The CPU pushes an error code for some vectors and not others, so the stack shape
differs. The stubs normalise it. Vectors that **do** push an error code:

```
8  (#DF)  10 (#TS)  11 (#NP)  12 (#SS)  13 (#GP)  14 (#PF)
17 (#AC)  21 (#CP)  29 (#VC)  30 (#SX)
```

Everything else does not. So:

```
stub_N  (no error code):        stub_N  (error code):
    push 0        ; 6A 00           push N       ; 6A xx
    push N        ; 6A xx           jmp  common  ; E9 rel32
    jmp  common   ; E9 rel32
```

`isr_common`:

```
push rax rcx rdx rbx rbp rsi rdi r8..r15     ; 15 pushes, 8 bytes each
mov  rcx, rsp                                ; arg0 = pointer to the frame
sub  rsp, 0x20                               ; shadow space, keeps 16-byte align
call zl_isr_dispatch                          ; a normal zl function
add  rsp, 0x20
pop  r15..rax                                ; reverse order
add  rsp, 16                                 ; drop vector + error code
iretq                                        ; 48 CF
```

**Do not add naked-function syntax to zl.** The stub is fixed machine code with
no locals, no frame, and a non-standard exit — there is no version of it a zl
programmer should write. `kernelgen.c` emits all 256 stubs in a loop (`for
(int v = 0; v < 256; v++)`, ~10 bytes each, ~2.6 KiB total) and the zl side
provides one ordinary function:

```
fn zl_isr_dispatch(frame) {
    vec = peek64(frame + 15 * 8)          # after the 15 saved GPRs
    err = peek64(frame + 16 * 8)
    rip = peek64(frame + 17 * 8)
    ...
}
```

Frame layout at the pointer the stub passes in `rcx`:

```
+0    r15                    (pushed last)
+8    r14
...
+112  rax                    (pushed first)
+120  vector number          (pushed by the stub)
+128  error code             (CPU's, or the stub's 0)
+136  RIP                    \
+144  CS                      |  pushed by the CPU. In long mode
+152  RFLAGS                  |  SS:RSP are ALWAYS pushed, even
+160  RSP                     |  without a privilege change.
+168  SS                     /
```

That layout is the whole interface. A zl exception handler that prints vector,
error code, RIP, CR2 (for #PF, via `read_cr2()`) and then halts is ~30 lines and
turns every future bug from "the VM rebooted" into a diagnosable line number.
**Build it on day one of the IDT stage.**

### 6.3 Intrinsics required

zl has no way to express `in`, `out`, or any privileged instruction. The choice
is an `asm` escape hatch or dedicated intrinsics.

**Decision: intrinsics, not `asm`.** Reasons: the needed set is small and closed
(13 things); an `asm` block needs a parser, an operand-constraint language, and a
register allocator that understands clobbers — a large design surface for one
wave; and intrinsics keep zl's "builtins are ordinary identifiers" rule (§4.3 of
MASTER_PLAN) intact with zero new keywords.

| Intrinsic | Emitted as | Bytes |
|---|---|---|
| `outb(port, v)` | `mov dx,cx` `out dx,al` | `66 89 CA` `EE` |
| `inb(port)` | `mov dx,ax` `xor eax,eax` `in al,dx` | `66 89 C2` `31 C0` `EC` |
| `outw(port, v)` / `inw(port)` | as above with `66 EF` / `66 ED` | |
| `outl(port, v)` / `inl(port)` | `EF` / `ED` | |
| `cli()` | `cli` | `FA` |
| `sti()` | `sti` | `FB` |
| `hlt()` | `hlt` | `F4` |
| `lgdt(p)` | `lgdt [rip+d]` + the CS-reload ritual (§4.2) | `0F 01 15 ..` + ~20 |
| `lidt(p)` | `lidt [rip+d]` | `0F 01 1D ..` |
| `invlpg(a)` | `invlpg [rax]` | `0F 01 38` |
| `read_cr0/2/3()` `write_cr0/3(v)` | `mov rax,crN` / `mov crN,rax` | `0F 20 ..` / `0F 22 ..` |
| `rdmsr(n)` / `wrmsr(n, v)` | `rdmsr` / `wrmsr` + edx:eax splicing | `0F 32` / `0F 30` |
| `cpuid(leaf, sub, idx)` | `cpuid`, `idx` 0..3 selects eax/ebx/ecx/edx | `0F A2` |

`cpuid` returns four registers and zl has no multiple returns (that is W2's
destructuring, which may or may not land). The `idx` selector parameter avoids
the dependency entirely at the cost of re-executing `cpuid` — which is fine, it
is not in a hot loop. Opinionated and keyword-free.

Calling convention note: for the stack-machine backend, `outb(port, v)` compiles
as `gen_expr(port); push rax; gen_expr(v); pop rcx;` leaving port in `rcx` and
value in `rax` — which is exactly the `mov dx,cx` / `out dx,al` shape above.

### 6.4 The PIC

Firmware leaves the 8259 PIC in an unknown state with IRQs mapped over vectors
0x08-0x0F — i.e. on top of your exception vectors, so a spurious timer IRQ looks
like a double fault. Remap it to 0x20-0x2F, then mask everything:

```
outb(0x20, 0x11)  outb(0xA0, 0x11)        # ICW1: init, expect ICW4
outb(0x21, 0x20)  outb(0xA1, 0x28)        # ICW2: vector offsets 0x20 / 0x28
outb(0x21, 0x04)  outb(0xA1, 0x02)        # ICW3: cascade wiring on IRQ2
outb(0x21, 0x01)  outb(0xA1, 0x01)        # ICW4: 8086 mode
outb(0x21, 0xFF)  outb(0xA1, 0xFF)        # mask ALL — polling only in v1
```

Masking everything is deliberate. **v1 polls.** Interrupt-driven serial is a
refinement after the polled version is proven, not the first thing you build.

---

## 7. The serial port — the first real device, and the highest-value one

COM1 is a 16550 UART at I/O port base **0x3F8**. It is the simplest real device
on the machine, and it gives you `printf` debugging, which makes every subsequent
stage tractable. Nothing else competes for the "first driver" slot.

### 7.1 Registers

| Offset | DLAB=0 | DLAB=1 |
|---|---|---|
| +0 | RBR (read) / THR (write) | divisor low |
| +1 | IER interrupt enable | divisor high |
| +2 | IIR (read) / FCR (write) | |
| +3 | LCR line control (bit 7 = DLAB) | |
| +4 | MCR modem control | |
| +5 | LSR line status | |
| +6 | MSR modem status | |
| +7 | scratch | |

### 7.2 Init, in zl, with the intrinsics from §6.3

```
COM1 = 0x3F8

fn serial_init() {
    outb(COM1 + 1, 0x00)      # disable interrupts
    outb(COM1 + 3, 0x80)      # DLAB on
    outb(COM1 + 0, 0x01)      # divisor low  = 1  -> 115200 baud
    outb(COM1 + 1, 0x00)      # divisor high = 0
    outb(COM1 + 3, 0x03)      # DLAB off, 8 bits, no parity, 1 stop
    outb(COM1 + 2, 0xC7)      # FIFO on, clear both, 14-byte threshold
    outb(COM1 + 4, 0x0B)      # DTR | RTS | OUT2

    # loopback self-test: prove the UART is actually there
    outb(COM1 + 4, 0x1E)      # loopback mode
    outb(COM1 + 0, 0xAE)
    if inb(COM1 + 0) != 0xAE { return 0 }
    outb(COM1 + 4, 0x0F)      # normal operation
    return 1
}

fn serial_putc(c) {
    while band(inb(COM1 + 5), 0x20) == 0 { }    # LSR bit 5: THR empty
    outb(COM1 + 0, c)
}

fn serial_getc() {
    while band(inb(COM1 + 5), 0x01) == 0 { }    # LSR bit 0: data ready
    return inb(COM1 + 0)
}
```

Note this uses only existing builtins (`band` from the W1 bitwise set) plus the
new intrinsics. No structs, no raw memory, no allocation. **The serial driver is
the one stage of W6 that is not blocked on W5** — it needs `outb`/`inb`, hex
literals, and nothing else. It could be prototyped ahead of the rest.

### 7.3 Wiring it to `print`

`emit_print_int` (nativegen.c:296) is an itoa loop that ends in
`GetStdHandle`+`WriteFile`. `emit_print_str` (l.341) is the same shape. In
`kernelgen.c`, **only the sink changes**: replace the `WriteFile` tail with a
`call serial_write(buf, len)` that loops `serial_putc`. The digit-extraction
loop, the negative-number handling, the string-literal `sfix[]` machinery — all
of it survives untouched.

The payoff is that `print(x)` works in the kernel with the semantics it already
has in every other engine, and `qemu -serial stdio` puts it in your terminal.
That is the entire debugging story, obtained by changing one tail.

---

## 8. Physical page allocator

### 8.1 Input: the UEFI memory map

`GetMemoryMap` fills a buffer with `EFI_MEMORY_DESCRIPTOR`s:

```
+0   u32  Type
+4   u32  Pad
+8   u64  PhysicalStart
+16  u64  VirtualStart
+24  u64  NumberOfPages      (4 KiB pages)
+32  u64  Attribute
```

> **The trap:** you must stride by the `DescriptorSize` the call returned, **not
> by 40 and not by `sizeof`.** Firmware is allowed to make descriptors bigger
> than the struct, and some does. Iterating with a hardcoded stride reads
> garbage on exactly the machines you did not test on.

Types worth knowing:

```
1  EfiLoaderCode        3  EfiBootServicesCode      7  EfiConventionalMemory
2  EfiLoaderData        4  EfiBootServicesData      9  EfiACPIReclaimMemory
                        5  EfiRuntimeServicesCode  10  EfiACPIMemoryNVS
                        6  EfiRuntimeServicesData  11  EfiMemoryMappedIO
```

**v1 treats only type 7 as free.** Types 3 and 4 also become free after EBS, and
reclaiming them is worth ~tens of MiB — but doing so requires having already
copied everything you need out of them, and the failure mode is "the firmware's
page tables you are still standing on get handed to the allocator." Not in v1.
Write it down as a v2 item; the memory is not needed.

### 8.2 A bitmap. Not a buddy allocator.

One bit per 4 KiB frame. For the low 4 GiB: 1,048,576 frames ÷ 8 = **131,072
bytes = 128 KiB, statically reserved in `.bss`.**

Rejected alternative — a buddy allocator: it buys O(log n) contiguous
multi-page allocation and clean coalescing. Nothing in v1 allocates contiguous
multi-page runs except the page tables, which are built before the allocator
exists. It is more code, more state, and more ways to be subtly wrong, for a
benefit nothing consumes. A linear bitmap scan of 128 KiB is microseconds and the
whole implementation is ~60 lines of zl. **Bitmap.**

```
fn pmm_init(map, map_size, desc_size) {
    # 1. mark everything used
    i = 0
    while i < BITMAP_BYTES { poke8(bitmap + i, 0xFF)  i = i + 1 }

    # 2. free the EfiConventionalMemory ranges
    off = 0
    while off < map_size {
        d = map + off
        if peek32(d + 0) == 7 {
            base  = peek64(d + 8)
            count = peek64(d + 24)
            pmm_free_range(base, count)
        }
        off = off + desc_size            # NOT + 40
    }

    # 3. take back what is actually in use
    pmm_mark_used(kernel_phys_base, kernel_pages)
    pmm_mark_used(pml4_phys,        6)          # the page tables from §5.1
    pmm_mark_used(bitmap_phys,      BITMAP_BYTES / 4096)
    pmm_mark_used(0,                1)          # never hand out frame 0
}

fn pmm_alloc() {
    i = next_free_hint
    while i < TOTAL_FRAMES {
        if bit_clear(i) { set_bit(i)  next_free_hint = i + 1  return i * 4096 }
        i = i + 1
    }
    return 0                                     # 0 means out of memory
}

fn pmm_free(addr) { clear_bit(addr / 4096) }
```

`next_free_hint` turns the common case from O(n) into O(1) amortised and costs
one global.

### 8.3 The heap on top

`design_native_runtime.md` §2 describes a bump allocator over a `VirtualAlloc`
arena. **That design is reused verbatim** with one substitution: the arena comes
from `pmm_alloc_contig(N)` instead of `VirtualAlloc`. Everything downstream —
`zl_alloc`, the 8-byte rounding, "no free, a run is short-lived" — is unchanged.

That substitution is also the moment the *boxed* backend (`nativeval.c`) becomes
usable in a kernel, which is the bridge from W6 to W7's "a shell written in zl."
Worth noting; not worth building in W6.

---

## 9. Staged plan, with honest sizing

Each stage ends with something observable in QEMU. Sizes assume W5 has landed and
assume evenings-and-weekends, not full-time.

| # | Stage | Deliverable | Honest size |
|---|---|---|---|
| **0** | **W5 lands** | raw memory, structs, freestanding output | **months. This is the real cost.** |
| 0b | Hex literals in `lexer.c` | `0x8E` lexes | an afternoon, but **serial, alone**, and `verify.ps1` must stay green |
| 1 | `kernelgen.c` forked from `nativegen.c`; UEFI PE (§3.2); `efi_main` stashes rcx/rdx; `ConOut->OutputString("hi")`; QEMU+OVMF loop | **"hi" on the QEMU console** | **a weekend** |
| 2 | `outb`/`inb` intrinsics; serial driver (§7); `print` retargeted to serial | `print(42)` appears in your terminal | **a weekend** |
| 3 | GDT + `lgdt` intrinsic with the CS reload (§4) | still prints after reloading CS — proof the descriptors are right | **an afternoon** |
| 4 | Page tables in `.bss` (§5.1); GetMemoryMap; ExitBootServices retry loop; load CR3 | prints *after* EBS, on your own paging | **a week.** The retry loop and the "no calls between GetMemoryMap and EBS" rule will each cost a session |
| 5 | IDT, 256 generated stubs, `zl_isr_dispatch`, exception dump; PIC remap+mask | deliberately `int3`, then deliberately touch address 0 — get a clean `#PF` dump with CR2 | **a week** |
| 6 | Physical page allocator (§8); bump heap on top | print total/free MiB; alloc 1000 frames, free them, alloc again | **a weekend**, given stage 4 |
| 7 | `verify_kernel.ps1` (§10) | headless QEMU, golden-file diff, exit code | **a day** |

**W6 total, after W5: ~6 focused weeks.** With W5 in front of it: realistically a
quarter.

What is **not** in W6 and must not creep in: keyboard, timer/APIC, a scheduler, a
filesystem, user mode, or running a compiled zl program from disk. Those are W7 /
MASTER_PLAN Floor 6 level 2, and the estimate there is 6-12 months. The line
between W6 and W7 is: **W6 owns the machine and can print. W7 does something with
it.**

---

## 10. Verification — a second gate, deliberately separate

The campaign's rule 1 is "`.\verify.ps1` must be GREEN before any commit," and
its invariants are the self-hosting fixpoint and 3-engine byte-identical
agreement. **The kernel target cannot participate in either**, for two honest
reasons: it does not run on Windows, and its integer semantics deliberately
differ from the interpreter's double semantics (§2).

Pretending otherwise would either weaken `verify.ps1` or produce a permanently
red gate. So:

> **`kernelgen.c` is a fourth-and-a-half engine that `verify.ps1` does not know
> about.** It is gated by its own script, `verify_kernel.ps1`, which must also be
> green before a kernel commit — but a kernel failure never turns the language
> gate red, and a language change never has to satisfy the kernel gate.

`verify_kernel.ps1`:

```powershell
# build the image, drop it on the synthetic ESP, boot it headless,
# capture serial, diff against the golden transcript, always time out.
.\kernelgen.exe tests\kernel\k01_hello.zl          # -> esp\EFI\BOOT\BOOTX64.EFI
$p = Start-Process qemu-system-x86_64 -PassThru -ArgumentList `
     '-bios','OVMF.fd','-drive','format=raw,file=fat:rw:esp', `
     '-serial','file:out.txt','-display','none','-no-reboot'
if (-not $p.WaitForExit(20000)) { $p.Kill(); "TIMEOUT" }
fc.exe /b out.txt tests\kernel\k01_hello.golden
```

The `-no-reboot` matters: a triple fault otherwise reboots into an infinite loop
and the test never terminates. The timeout matters for the same reason.

**Safety, non-negotiable and consistent with MASTER_PLAN §5.3's Aegis rule:
QEMU only. Nothing produced by this wave gets written to the dev machine's ESP.**
A bug in stage 4 or 5 is at worst a VM that will not boot; the same bug on real
hardware with a corrupted `\EFI\BOOT\BOOTX64.EFI` is a machine that will not
boot.

---

## 11. Risks / open questions

| # | Item | Note |
|---|------|------|
| 1 | **W5 slips or lands with different primitives** | This document depends on `peek8/16/32/64`, `poke*`, a way to take the address of a `.bss` reservation, and 4 KiB-aligned reservations. W5 owns the spelling; W6 owns the requirement. If W5 ships something shaped differently, §4/§6/§8's snippets need rewriting, but no decision in this document changes. |
| 2 | **`.bss` alignment** | CR3 and every page-table pointer need 4 KiB alignment; the GDT/IDT want 16-byte. The backend currently has no notion of alignment on a data reservation. Small, but it is a real new feature and easy to forget until it faults. |
| 3 | **The `atoll` / signed-division edge** | §2. Mitigated by staying in the low half in v1. If a higher-half kernel is ever wanted, unsigned compare/divide intrinsics become mandatory, not optional. |
| 4 | **UEFI table offsets** | §3.3's offsets are from the standard layout and should be checked against the spec. A wrong offset is a call into an arbitrary firmware routine — no fault, just wrong behaviour, which is the worst kind. |
| 5 | **ExitBootServices retry** | The most likely source of a multi-session stall. Mitigation is §5.2's ordering rule: serial working *before* the first EBS attempt. |
| 6 | **`DescriptorSize` striding** | §8.1. Works in QEMU with a hardcoded 40 and fails elsewhere — a latent bug that only shows up on real firmware. Write the stride correctly the first time. |
| 7 | **Frame size on a 128 KiB stack** | Rule K2. `nativegen` sizes frames from `nlocals` with no cap; a zl function with a large fixed array declared as locals would silently blow the UEFI stack. Add the compile-time cap. |
| 8 | **Nothing enforces "no allocation"** | §1.2's compile-time rejection list must actually be implemented in stage 1, not deferred. A `zl_alloc` reaching a kernel binary is a fault with no diagnostic. |
| 9 | **Scope creep into W7** | "While I'm here, a keyboard driver" is how this wave becomes a year. §9's exclusion list is the contract. |

---

## 12. Definition of done

1. `kernelgen.exe prog.zl` emits `BOOTX64.EFI`, a PE32+ EFI application with a
   computed multi-section layout, no import table, and no absolute addresses.
2. It rejects, at compile time, any construct that would allocate.
3. `outb/inb/outw/inw/outl/inl/cli/sti/hlt/lgdt/lidt/invlpg/read_crN/write_crN`
   exist as intrinsics; hex literals lex.
4. Booted under QEMU+OVMF, the image prints over COM1 at 115200 **after**
   `ExitBootServices`, on its own GDT, its own page tables, and its own IDT.
5. A deliberate `#PF` produces a readable dump (vector, error code, RIP, CR2) and
   halts, rather than rebooting the VM.
6. `pmm_init` reports plausible total/free memory; alloc/free/alloc round-trips.
7. `verify_kernel.ps1` is green, headless, and times out rather than hanging.
8. `verify.ps1` is **untouched and still green** — the fixpoint holds and the
   3-engine agreement is 6/6, because nothing in this wave went near the
   language engines.

---

## Appendix A — the legacy BIOS sequence (rejected, recorded)

Kept only so the decision in §3.1 can be re-litigated with facts rather than
re-researched. If a BIOS stub is ever wanted:

```
1. Firmware loads sector 0 (512 bytes, ending 0x55 0xAA) to 0x7C00, in 16-bit
   real mode, with DL = boot drive.
2. Enable A20 (fast A20 via port 0x92 bit 1, with the keyboard-controller
   method as fallback).
3. Query the memory map: INT 0x15, EAX=0xE820 — a real-mode BIOS call, so it
   must happen here and be stashed at a known address for later.
4. Load a 32-bit GDT, set CR0.PE, far-jmp to flush -> 32-bit protected mode.
5. Build 4-level page tables, set CR4.PAE, set EFER.LME (MSR 0xC0000080),
   set CR0.PG, far-jmp through a 64-bit GDT -> long mode.
6. Because 512 bytes is nowhere near enough, sector 0 is only a stage-1 loader:
   it must also read stage 2 off the disk via INT 0x13 extensions.
```

Steps 1-4 and 6 require a **16-bit instruction encoder** that has no other use
anywhere in this project. Everything from step 5 onward is work that UEFI also
requires and that this document already specifies. That asymmetry is the whole
argument.
