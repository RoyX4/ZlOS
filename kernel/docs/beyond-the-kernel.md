# Beyond the kernel — other languages, the internet, and C → zl

Three questions asked on 2026-08-17, answered with measurements taken the same
day rather than estimates. Everything below was run on this box against this
tree; commands are included so any claim can be re-checked.

**The short version:**

| Ask | Verdict | Real blocker |
|---|---|---|
| Run other languages on zlOS | **Doable, ~2–3k lines, in a specific order** | No heap, no ELF loader, no address spaces. Not a language problem. |
| Internet | **Doable and bounded, ~10k lines to HTTPS** | Zero network code exists. The wired NIC on the test laptop *does* enumerate. |
| WiFi | **Not on this laptop's own radio.** Use a wire or USB | AX201 is CNVi — no self-contained card, boots empty, needs a 2–3 MB signed Intel blob and an undocumented protocol. **xHCI already does bulk endpoints, so USB tethering is the short path.** |
| A browser | **Buildable if you pick which one.** ~13k lines for a real document browser | Needs a heap, a runtime font rasterizer, and PNG/inflate — none of which exist yet. See §2b. |
| Rewrite all C in zl | **Yes, and it's finite** | zl's fast path drops to a boxed `double` and a name-string dispatch the moment you use a bitwise operator — and a driver is nothing but bitwise operators. Compiler work, not kernel work. |

> **Correction, same day.** An earlier version of this document said a browser
> was "unbounded — don't." That conflated *a browser that matches Chrome on the
> live web* (genuinely unbounded) with *a browser that renders documents*
> (bounded, and shipped by Dillo, NetSurf, w3m and Lynx for decades). Roy pushed
> back and was right. §2b is the actual plan.

The last row is the important one and it is the least obvious, so it is the
longest section. It is also the cheapest to fix relative to what it unlocks.

---

## Where the code actually is

Measured with `wc -l`, excluding generated files:

| | Lines |
|---|---|
| Hand-written driver C (22 files, `apic.c`…`xhci.c`) | **12,138** |
| Generated data tables (fonts, icons) | 17,021 |
| Assembly (`boot.S`, trampolines, `raw_boot.asm`) | 749 |
| `kernel.zl` | **2,233** → generates `out.c`, 3,313 lines |

So "rewrite all of C in zl" means **12,138 lines**, not 32k. The font and icon
tables are Python output and would stay Python output. The 749 lines of assembly
cannot be zl in any version of this — there is no `asm` primitive, and a
multiboot header and an SMP trampoline are not expressible in a high-level
language by construction.

---

## 1. Running other languages on zlOS

### What is missing, verified by grep

```bash
grep -rn '\b(malloc|kmalloc|free)\s*\(' kernel/*.c      # → nothing
grep -rln 'Elf64_Ehdr|elf_load|exec\(|process_create'   # → nothing
grep -rn 'cr3|pml4' kernel/*.c                          # → smp.c only, one shared CR3
```

Four things stand between zlOS and running a foreign program, and they are
independent of which language that program is written in:

1. **No heap.** Zero allocator calls in the kernel. This is deliberate
   (`STATE.md` hard constraints, `runtime_kernel.c:1–25`), and every runtime
   worth hosting — Lua, Python, a JS engine, even a Forth with a dictionary —
   wants one.
2. **No ELF loader.** Nothing parses a program header or relocates a symbol.
3. **No address spaces.** `smp.c:129–134` copies the BSP's CR3 to every core on
   purpose — all four cores share one page table. There is no per-process
   memory, so a hosted program can scribble on the kernel.
4. **No filesystem.** `fs_save`/`fs_load` in `kernel.zl:969–977` look like one
   but are a fixed metadata table at `FS_META` and fixed-size slots at
   `FS_DATA`, written with `poke32` and `copy_mem`. It is a RAM slot array with
   a friendly name — nothing survives a reboot through it, and there are no
   filenames.

`sched.c` (292 lines) is real, but it schedules *kernel threads* sharing one
address space, which is the cooperative half of the problem, not the isolation
half.

### The ladder — stop at the rung that is actually worth it

**Rung 1 — run zl on zlOS.** The obvious first move and nobody has done it. The
whole toolchain is smaller than it looks:

| | Lines |
|---|---|
| `lexer.c` | 457 |
| `interp.c` | 1,900 |
| `runtime.c` (full boxed, needs ~70 libc symbols) | 1,355 |

The interpreter is 1,900 lines. What blocks it is not size — it is that
`runtime.c` pulls in all of libm, stdio, `opendir`, `fork`. Hosting zl on zlOS
means writing the ~30 libc functions the interpreter actually touches, plus a
bump allocator. Call it **800–1,200 lines** and a real REPL on bare metal.
This is the rung that pays: zlOS becomes self-hosting for its own language.

**Rung 2 — a Forth.** ~1,500 lines, needs no heap in the malloc sense (a
dictionary is a bump pointer), no filesystem, no processes. If the goal is
"another language running on my OS" as a demonstration, this is the cheapest
honest version and it has a finish line.

**Rung 3 — a Lua or a WASM interpreter.** Lua's core is ~20k lines of very
portable C and expects `malloc`/`realloc`/`free` and `setjmp`. A minimal WASM
interpreter (wasm3-style) is ~8k lines and is the more interesting target
because it gives you *any* language that compiles to WASM — Rust, C, Go,
AssemblyScript — for one port. **This is the highest-leverage answer to "run
other languages", and it is bounded.**

**Rung 4 — POSIX ELF binaries.** This is where "run other languages" usually
means, and it is the whole rest of an operating system: an ELF loader, per-process
page tables, a syscall ABI, a real filesystem, `fork`/`exec`, signals, a libc.
Everything zlOS refuses by design. This is not a feature, it is a second project.

**Recommendation: rung 1, then rung 3.** Rung 1 makes the project self-hosting;
rung 3 gives you every language at once without becoming Linux.

---

## 2. The internet

### There is currently zero network code

```bash
grep -ril 'e1000|virtio_net|rtl8139|tcp|udp|ethernet|socket|dhcp' kernel/*.c kernel/*.zl
# → nothing
```

The feature catalogue lists TCP/IP, HTTP/TLS, email, WiFi and servers as ❌
across the board. That is accurate.

### The hardware situation is better than expected

Checked on the actual test laptop with `lspci -nn`:

```
00:1f.6 Ethernet controller: Intel 400 Series On-Package GbE (Consumer) [8086:0d4f]
00:14.3 Network controller: Intel 400 Series CNVi WiFi                  [8086:02f0]
```

**There is a real wired NIC and it enumerates on PCI.** `8086:0d4f` is the i219-V
class part Linux drives with `e1000e`. That matters a lot: it is a
descriptor-ring MAC with public documentation, the same shape as the e1000 QEMU
model, and `pci.c` already finds devices.

### Why WiFi is a different animal from Ethernet

Ethernet is a **wire**. The card is a mailbox: you write a descriptor that says
"here is a packet, send it", you ring a doorbell register, and the packet leaves.
Nobody else is on your wire. That is why `e1000e` is ~1,000 lines — there is
almost no *negotiation*, only plumbing.

WiFi is a **shared room where everyone is shouting**. Before a single byte of
your data moves you have to find out who is in the room, introduce yourself, get
permission to speak, prove who you are with real cryptography, and then keep
renegotiating as the signal changes. None of that is optional and none of it is
plumbing — it is protocol.

Then there is what Intel specifically did to this laptop:

**AX201 is CNVi, not a normal PCI card.** Intel split the chip in half. The radio
lives in a separate module and the digital half lives inside the chipset, so
there is no self-contained "WiFi card" sitting on the bus for a driver to own.

> **Superseded in part by [`wireless-plan.md`](wireless-plan.md).** Everything
> below about the AX201 is correct, but it is an argument about *one radio*, not
> about WiFi. With an **AR9271 USB dongle — 50 KB of open-source firmware** —
> WiFi is ~8,000 bounded lines. And the blob figure below should read **1.3 MB**
> (measured: 1,406,572 bytes), not 2–3 MB.

**And the chip boots up empty.** It has no firmware in it. Linux uploads a
**1.3 MB blob** (`iwlwifi-Qu-b0-hr-b0-77.ucode`) into the device at init,
and from then on the driver is not talking to hardware — it is talking to
*Intel's firmware*, over a command/response protocol Intel does not publish. The
only specification is the `iwlwifi` source code itself.

For scale, on a **1.07 MB** kernel:

| Piece | Size |
|---|---|
| The firmware blob you would have to ship | **2–3 MB** — bigger than zlOS |
| `iwlwifi` host↔firmware protocol (`mvm/`) | ~40k lines |
| `mac80211` — scan, associate, authenticate | ~50k lines |
| WPA2-PSK supplicant (minimal, reusing TLS crypto) | ~2k lines |

**And there is no partial win.** The display driver had one — you could read the
panel correctly long before you could light it. With WiFi you get *nothing at
all* until the whole stack works. You cannot half-associate.

### The escape hatch — and `xhci.c` already has the hard part

Do not drive the radio. Make the radio somebody else's problem and give zlOS
a **wire that happens to be virtual**:

- **USB tethering from your phone.** The phone presents itself as a USB ethernet
  device (CDC-ECM or RNDIS). To zlOS that is a mailbox with descriptors, exactly
  like `e1000e` — the phone deals with the radio, WPA, roaming, all of it.
- **A USB ethernet dongle** (ASIX AX88179, ~500 lines) — same shape.

The reason this is cheap here rather than theoretical: **`xhci.c` already
implements bulk IN/OUT endpoints** — `configure_bulk()` at `xhci.c:1709`,
`EPTYPE_BULK_IN`/`EPTYPE_BULK_OUT` at 1693–4, written and proven for USB mass
storage. Bulk endpoints are the one genuinely new piece a USB NIC needs, and
they already work.

**So: wired NIC on the desk, USB tethering on the move, and skip the AX201.**
That is not a downgrade — it is the same internet, reached by climbing one rung
instead of building a radio stack larger than the entire OS.

### The ladder, with honest sizes

| Layer | Lines | Bounded? | Notes |
|---|---|---|---|
| virtio-net (QEMU only) | ~400 | yes | **`virtio_gpu.c` already has virtqueue plumbing to reuse** — start here |
| e1000e / i219 (real laptop) | ~1,000 | yes | descriptor rings, documented; the DMA-outside-RAM bug class applies |
| ARP + IPv4 + ICMP | ~600 | yes | ping works at the end of this |
| UDP + DHCP + DNS | ~900 | yes | you have an address and can resolve names |
| TCP | ~2,500 | yes, but subtle | retransmit, RTO, windows, the state machine. This is the one that bites |
| TLS 1.3 | ~5,000–8,000 | yes | X25519, AES-GCM, SHA-256, HKDF, X.509 parsing, a trust store. **Needs bignum arithmetic and certificate parsing with no heap** |
| HTTP/1.1 | ~300 | yes | trivial next to TLS |

**Total to reach `https://` : roughly 10,000 lines — about the size of the
entire hand-written kernel again.** Every layer is bounded and specified, which
makes it a grind rather than a risk. TLS is where a no-heap kernel hurts most:
certificate chains are variable-length and recursive, and today there is nowhere
to put them.

**Cheap intermediate that is worth naming:** stop at UDP+DNS and you have a real
network stack, `ping`, and a clock synced over NTP — about 2,900 lines and a
genuine "zlOS is on the internet" moment, with no TLS and no TCP.

---

## 2b. The browser

`feature-catalogue.md` says don't. **That call was wrong as stated**, and this
section supersedes it.

The mistake was treating "a browser" as one thing. It is three products with
wildly different endings, and only the third is unbounded:

| | What it renders | Lines | Finish line? |
|---|---|---|---|
| **A. Document browser** — HTML + CSS, no JS | Wikipedia, docs, HN, most text sites, your own pages | **~13,000** | **Yes** |
| **B. Late-90s browser** — + a JS engine | A lot more, badly | +50k (port QuickJS) | Blurry |
| **C. Matches Chrome on the live web** | Everything | 300k+ | **No, genuinely** |

The unbounded argument is only true of **C**. **A has shipped repeatedly** —
Dillo, NetSurf, w3m, Lynx are real browsers people have used for decades, and
NetSurf runs on RISC OS and AmigaOS with less hardware than this. A renders a
readable Wikipedia page. That is a finish line and you can stand on it.

**So the answer is: build A, name it A, and decide about B later.**

### What zlOS already has that a browser needs

More than you would think, and it is the expensive-looking half:

- **Gamma-correct, linear-light alpha blending** and **subpixel LCD text**
  (`fb.c:83`, `font_sub.c`) — this is the part that makes text look like a real
  browser rather than a terminal, and it is done and benchmarked.
- **Glyph shapes from a real TrueType font** (DejaVu Sans Mono, rasterized by
  `gen_hd_font.py` at build time).
- A **software renderer measured at 4.88 ms** for a full desktop at 1920×1200.
- An **immediate-mode toolkit** already designed for a no-heap kernel
  (`desktop-toolkit.md`) — a browser's UI chrome is exactly that shape.

### What is genuinely missing — the honest bill of materials

| Piece | Lines | Note |
|---|---|---|
| **A heap** | ~300 | A DOM is a tree of variable-size nodes. There is no way around this one. It is the gate on everything below. |
| **Runtime font rasterizer** (stb_truetype) | ~2,500 | **Today's fonts are baked at build time into exactly three sizes — 8×16, 16×32, 24×48, monospace only.** Web pages need arbitrary sizes and proportional faces. |
| **inflate / zlib** | ~1,000 | Needed twice: HTTP `gzip` encoding *and* PNG. Do it once. |
| **PNG decode** | ~800 | Sits on inflate. |
| **JPEG decode** | ~2,000 | Optional for v1; a lot of the web degrades acceptably without it. |
| **HTML5 parser** | ~2,500 | Tokenizer + tree construction. The spec is a real algorithm and is *precisely* written — this is the well-specified part. |
| **CSS parse + cascade** | ~2,000 | Selectors, specificity, inheritance. Subset it deliberately. |
| **Layout — block + inline + text flow** | ~3,000 | The heart of it. Line breaking, margins, floats if you want them. |
| **Painting into `fb.c`** | ~800 | You already have the primitives. |
| **HTTP/1.1 client** | ~300 | Trivial once TLS exists. |
| | **~13,200** | plus the network stack below it |

**Plus the ~10,000-line network stack from §2**, because a browser with no TLS
reaches almost nothing today. Total to "zlOS opens Wikipedia over HTTPS":
**roughly 23,000 lines**, about double the current hand-written kernel.

### The order, and where it stops being optional

Every arrow is a hard dependency. Nothing below can start before the thing above:

```
heap  ->  NIC (USB tether or e1000e)  ->  ARP/IP/UDP/DNS  ->  TCP  ->  TLS  ->  HTTP
  |
  +->  stb_truetype  ->  inflate  ->  PNG
                 |
                 +->  HTML parser  ->  CSS  ->  layout  ->  paint
```

**The heap is the gate on both branches.** It is ~300 lines, it is the
cheapest item on this page, and it is currently a stated design constraint of
the project — so building a browser means *deciding to relax that constraint*,
not just writing code. That is the actual decision to make, and it is a design
call, not an engineering one.

Useful property of the right-hand branch: **HTML parser → CSS → layout → paint
needs no network at all.** You can build and test the entire renderer against
`.html` files in the NVMe store, on QEMU, with no NIC, no TLS, and no laptop.
That is the same trick as `hosttest/` — it is the half of a browser that can be
developed with a fast loop.

### What to explicitly not chase

Not because it is hard — because it has no ending: matching Chrome's quirks,
running site JavaScript frameworks, video, WebGL, web fonts, and the
compatibility long tail. Build A. If A renders Wikipedia and your own docs
cleanly, that is a real browser on an OS you wrote, and it is done.

---

## 3. Rewriting the C in zl — the real blocker

This is the interesting one, because the intuition is wrong. The blocker is not
that drivers are hard to express, or that 12,138 lines is a lot of typing. **The
blocker is that zl's own compiler cannot currently emit driver-grade code**, and
that is a fault in one file of the toolchain, not in the kernel.

Keeping the two apart, because the fix is different for each:

- **"the language can't do this"** → needs work in `compilel.c` / the runtime
- **"nobody has written it yet"** → needs work in `kernel/`

Almost everything here is the first kind.

### The kernel is built on the archived backend

`kernel/build.sh` line 15 calls `../compile` — the boxed C backend. The root
`README.md` marks that backend **"ARCHIVED — don't develop"** and names
`compilel` (LLVM) as the active speed path. The kernel has never moved.

What "boxed" costs, measured:

- `sizeof(Value)` is now **16 bytes** at `-m32` and `-m64`, enforced by the
  current runtime guard. Value16 removed substantial copying, but every number
  the kernel touches from zl is still a tagged box passed through generic
  helpers rather than a native integer register.
- Every builtin call goes through
  `zl_calln(const char *name, ...)` — a **linear chain of 644 string
  comparisons** in the current `freestanding/runtime_kernel.c`.
- Common drawing primitives are late in that chain: `fill_rgb` is entry 509,
  `rrect` 515 and `text_aa` 516. The exact positions change when builtins are
  added, but name search is still the structural cost.

Think of it as posting a letter to yourself to remember a number. The number
arrives, correctly, every time — but you are running a postal service inside a
loop that has to finish before the next scanline.

### Numbers are doubles, so 64-bit registers are not representable

`runtime.h:16` — a zl number is a `double`. Exact for every integer up to 2^53,
and inexact above it. `runtime_kernel.c:882` is explicit that `peek64`/`poke64`
are deliberately absent for this reason.

For this kernel that is not academic. 32-bit MMIO is fine — every 32-bit value
is exact in a double. But `intel.c` and `nvme.c` deal in 64-bit BARs and 64-bit
descriptor addresses, and **a 64-bit value in a zl variable silently loses its
low bits.** That is the same shape as the bug class `HANDOFF.md` already calls
out five times: *"a DMA buffer outside guest RAM, or an address truncated to
32 bits."* Moving those drivers to zl as it stands would industrialise the
project's most expensive recurring bug.

### No function pointers in the kernel subset

`runtime_kernel.c` makes `zl_callv` a hard fault:
`"calling a function value is not available in the kernel subset"`.

So no IDT vector table, no driver dispatch table, no callback of any kind can be
written in zl today. `idt.c`, `apic.c` and `input.c` are structurally
untranslatable until this changes — not because the logic is hard, but because
the language cannot name a function as a value.

### The fast path exists — and bitwise operators fall off it

This is the finding worth keeping. `compilel` is a **typed** backend with real
type inference: 199 `i64` references in its codegen against 88 `double`.

Same function, two versions, compiled with `./compilel`:

```zl
fn setbit(v, n) { return v + n * 2 }          # arithmetic
fn setbit(v, n) { return bor(band(v,255), shl(1,n)) }   # bitwise
```

```llvm
; arithmetic  →  fully native
define i64 @fn_setbit(i64 %a_p0, i64 %a_p1) {
  ...  add i64 / mul i64  ...

; bitwise  →  return type degrades to double, and every operand is boxed
define double @fn_setbit(i64 %a_p0, i64 %a_p1) {
  %t2 = sitofp i64 %t1 to double            ; i64 → double
  %t5 = alloca [128 x i8]                   ; box two Values on the stack
  call void @zlx_num(ptr %t6, double %t2)
  call void @zlx_call(ptr %t8, ptr @.bname.0, ptr %t5, i32 2)   ; dispatch by NAME
  %t9 = call double @zlx_as_num(ptr %t8)    ; unbox
```

The linker proves it independently. The documented recipe for the unboxed path
is `clang -O2 out.ll -o program` with **no runtime linked at all**:

```
arithmetic version → links clean, standalone binary
bitwise version    → undefined reference to `zlx_num'
                     undefined reference to `zlx_call'
                     undefined reference to `zlx_as_num'
```

And the cost, timed on this box — 10M iterations of `acc ^= ((i & 255) << 3)`,
best of three:

| | |
|---|---|
| C, `gcc -O2` | **7 ms** |
| zl via `compilel`, arithmetic only | **4–8 ms** |
| zl via `compilel`, bitwise | **999 ms** |

The arithmetic figure is a real loop, not a constant fold — 10× the iteration
count gives 48 ms, so it scales.

**~140× on the exact operations a driver is made of.** And the kernel's actual
backend is `compile`, which boxes *everything*, so the kernel path is worse than
the 999 ms — that figure is the LLVM backend, measured on the host, and is a
floor rather than the kernel's real number.

> Reproduce: the two `.zl` files, `./compilel`, `clang -O2 out.ll -o x`. The
> failing link *is* the result — the arithmetic version needs no runtime and the
> bitwise one does.

### So the order of work is the opposite of the obvious one

Do not start translating drivers. Start in `compilel.c`.

1. **Make `band`/`bor`/`bxor`/`bnot`/`shl`/`shr` native instructions** — LLVM
   `and`/`or`/`xor`/`shl`/`lshr` on `i64`, no boxing, no name dispatch. This is
   a codegen change in one file and it is the single highest-leverage item on
   this page: it turns 999 ms into single-digit ms and stops the return type
   collapsing to `double`.
2. **Add the memory and port primitives** as intrinsics, not builtins:
   `peek8/16/32` → `load volatile`, `poke8/16/32` → `store volatile`,
   `inb`/`outb` → `call ... asm sideeffect`. Roughly 150 lines.
3. **Add an exact 64-bit integer**, so a BAR or a DMA address survives a round
   trip. Without this, `intel.c` and `nvme.c` can never be zl — see the 2^53
   hazard above.
4. **Function values in the kernel subset**, so `idt.c`/`apic.c`/`input.c`
   become expressible at all.
5. **Prove it on the smallest driver that is not load-bearing**, in this order:
   `vga.c` (117) → `bga.c` (178) → `pci.c` (239) → `console.c` (391). Each has a
   working C version to diff behaviour against, which is the only cheap
   correctness check available.
6. `xhci.c` (1,920) and `intel.c` (4,357) **last, or never.** `intel.c` is the
   stated point of the project, it works, and it took a 13-conflict research plan
   and an 86-defect audit to get there. Rewriting it in a new backend to prove a
   language point would be trading the one thing that works for a slogan.
7. The **749 lines of assembly are permanent.** A multiboot header, GDT/IDT
   loading and an SMP trampoline are not high-level-language objects.

**Honest total, once steps 1–4 are done:** the ~9,000 lines of `vga`→`xhci`
driver code are ordinary mechanical translation, weeks of grind, low risk
because each file has a C reference to diff against. Steps 1–4 are the hard,
interesting part and are perhaps **1,500 lines of compiler work**.

### "Is it actually possible, or not really?"

**Possible. Not "in principle" — possible, with a finite list.** Nothing here is
a dead end; every blocker is a specific thing to write, and every one of them is
in the compiler rather than in the kernel.

The reason it *feels* impossible is a single design choice deep in the language:
**zl stores every number as a `double`.**

Think of a set of kitchen scales that reads out in decimals. Perfect for
weighing flour. Now try to use it to record a barcode. The scale isn't broken
and it isn't missing a feature — it stores numbers in the wrong *shape* for that
job, and every barcode you weigh comes back subtly wrong in the last digits.

A driver is a barcode. It is not arithmetic, it is exact bit patterns, 64 bits
wide, where the bottom bit matters as much as the top one. A `double` holds
every integer exactly up to 2^53 and then starts rounding — so 32-bit registers
survive the trip and **64-bit BARs and DMA addresses do not.**

Keeping the two kinds of problem apart, because they need different fixes:

**"The language genuinely cannot do this yet"** — needs work in `compilel.c`:

| | Why it blocks drivers |
|---|---|
| No exact 64-bit integer | 64-bit BARs and DMA addresses silently lose their low bits |
| Bitwise ops leave the fast path | 999 ms vs 7 ms, and the function's type collapses to `double` |
| No `peek`/`poke`/`inb`/`outb` in the typed backend | Cannot touch hardware at all from the fast path |
| No function values (`zl_callv` is a hard fault) | No IDT vectors, no dispatch tables — `idt.c`/`apic.c`/`input.c` are unwritable |

**Estimated ~1,500 lines total.** That figure is an estimate, not a measurement
— the 999 ms historical benchmark, the current 644-entry chain and the 2^53
ceiling are measured. The benchmark predates Value16 and must be rerun before
being used as a current end-to-end speed figure.

**"Nobody has written it yet"** — ordinary work in `kernel/`:

- ~9,000 lines of `vga.c`→`xhci.c` driver translation. Mechanical, weeks of
  grind, **low risk** because every file has a working C version to diff
  behaviour against.
- 749 lines of assembly that stay assembly, permanently.
- `intel.c` (4,357), which should go last or never.

So: **the ceiling is not the language.** It is about 1,500 lines of compiler
work, and until that is done, translating drivers would produce code 140× slower
that silently truncates addresses — the exact bug class this project has already
paid for five times.

---

## What this changes about the roadmap

Nothing on `display-roadmap.md` moves. The display driver is still the point and
is still the thing in flight.

But there is a genuine ordering insight here worth recording: **item 1 above
(native bitwise ops in `compilel`) is a prerequisite for the zl-native
ambition and is completely independent of the laptop, the panel, and the
desktop.** It needs no hardware access, cannot break a running driver, and is
testable entirely on the host with a diff against the interpreter. It is the
only major item on this page with that property.
