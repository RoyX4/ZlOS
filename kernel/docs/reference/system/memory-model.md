# The zlOS address space, and what is actually enforcing it

Written 2026-08-20, during the work to turn zlOS's memory from a set of
hand-chosen constants into a memory *model*. This page is the layout as it
stands, **who enforces each boundary**, and which of the five stages is done.

`kernel/docs/reference/system/memory-map.md` is the older, longer account of the same territory and is
still worth reading for the history. This one exists to answer a different
question: *if I put a buffer at address X, what stops me being wrong?*

---

## The layout, measured

Every number below was read out of the file that owns it, or measured from a
build, on 2026-08-20. Sizes marked MEASURED come from `readelf`/`stat` output
in the same session, not from arithmetic.

| base | | what | owner | enforced by |
|---|---|---|---|---|
| `0x00000000` | 0 | PML4, PDPT, PD0..PD3 | `boot64.S:40-42` | — |
| `0x00008FE0` | | `CR3_PTR`, `ENTRY_PTR` | `smp.c:56-57` | — |
| `0x00009000` | | the SMP trampoline | `smp.c:55,124` | — |
| `0x000A0000` | | VGA hole and BIOS ROM | hardware | — |
| `0x00100000` | 1 MiB | the kernel image | `link*.ld` | 6 linker `ASSERT`s |
| `0x00700000` | 7 MiB | top of what the raw loader fills | `raw_boot.asm` `CHUNKS` | `mkdisk.sh` size guard |
| `0x00C00000` | 12 MiB | raw-boot stack TOP, grows **down** | `raw_entry.S`, `raw_boot.asm` | `arena.c` `_Static_assert` |
| `0x00E00000` | 14 MiB | the program arena, 16 MiB budget | `arena.c` | `check-memmap.sh` |
| `0x01E00000` | 30 MiB | arena end | | |
| `0x02000000` | 32 MiB | `SNAKE_X`, `FS_*`, `LINE_BUF`, `HIST_BUF` … | `kernel.zl` | `check-memmap.sh` |
| `0x04000000` | 64 MiB | virtio-net rings and frame buffers | `virtio_net.c` | 4 `_Static_assert`s |
| `0x08000000` | 128 MiB | `back`, the back buffer | `fb.c` | `memmap.h` chain |
| `0x0A800000` | 168 MiB | the AP stacks | `smp_trampoline{,64}.S` | `memmap.h` chain |
| `0x0B000000` | 176 MiB | task stacks + demo counters | `sched.c` | `memmap.h` chain |
| `0x0B800000` | 184 MiB | HID report + descriptor buffers | `i2c_hid.c` | `memmap.h` chain |
| `0x0BC00000` | 188 MiB | the GPU command ring | `gpuring.c` | `memmap.h` chain |
| `0x0C000000` | 192 MiB | the cached-blur arena | `fb.c` | `memmap.h` chain |
| `0x0D000000` | 208 MiB | NVMe admin + I/O queues | `nvme.c` | `memmap.h` chain |
| `0x0E000000` | 224 MiB | the USB DMA arena | `xhci.c` | `memmap.h` chain |
| `0x0F000000` | 240 MiB | virtio-gpu rings + framebuffer | `virtio_gpu.c` | `memmap.h` chain |
| `0x10000000` | 256 MiB | **the heap**, 64 MiB, hands out and takes back | `heap.c` | `memmap.h` chain |
| `0x14000000` | 320 MiB | — unclaimed — | | |
| `0x40000000` | 1 GiB | **`HI_TOP`** — the smallest guest we promise | `memmap.h` | `check-ram.sh` |

Note what the heap row does to this table: it is the first region whose
*contents* have no fixed addresses. Everything above it is one buffer at one
address chosen by a person. Nothing inside the heap appears in `memmap.h`,
because nothing inside it has an address until something asks for one. That is
the whole direction of travel — Stage 4 extends it to the rest of the table.

Measured build sizes, 2026-08-20:

```
kernel_raw.bin    1,614,532 bytes   what the raw loader must carry (no .bss)
__bss_start       0x0028A2D0        2.539 MiB
__bss_end         0x002E15C0        2.880 MiB   = __kernel_end
kernel.elf        __kernel_end 0x003215C0       3.130 MiB (multiboot build)
kernel64.elf      1,353,880 bytes
BOOTX64.EFI       1,291,776 bytes
```

---

## The four ceilings, and why they are different numbers

People conflate these constantly. They are four separate constraints and only
one of them is about how much RAM the machine has.

1. **What the loader carries** — `raw_boot.asm`'s `CHUNKS` × 32 KiB. The loader
   reads exactly that many chunks *whatever the kernel's size*, so a kernel
   past it is silently truncated and jumped into. `.bss` is exempt: it is
   `NOBITS`, not on the disk, and `raw_entry.S` zeroes it. So the number that
   must fit is `__bss_start`, not `__kernel_end`. **Guard: `mkdisk.sh`**, on the
   objcopy'd binary.
2. **Where the image may end** — `__kernel_end`, including `.bss`, must clear
   the raw-boot stack and then the arena. **Guard: six `ASSERT`s across
   `link.ld`, `link-raw.ld`, `link64.ld`.**
3. **Who owns which address** — the region map. **Guards: `memmap.h`'s
   `_Static_assert` chain, `check-memmap.sh`, `check-himap.sh`.**
4. **How much RAM exists** — `HI_TOP`. Everything a *device* DMAs must be below
   it, because below it is the only memory we promised. **Guard:
   `check-ram.sh`.**

A change can satisfy three of these and violate the fourth. Stage 2 is exactly
that shape: raising (1) pushed the loaded region through the stack, which is
(2).

---

## Stage status

- **Stage 1 — raise the RAM ceiling. DONE** (`078ebf8`). `HI_TOP` 256 MiB →
  1 GiB, every QEMU in the tree at `-m 1G`, `check-ram.sh` added to keep the two
  in step. The old 256 MiB line was honoured by no gate in either direction:
  four passed no `-m` at all and booted QEMU's 128 MiB default, so the top half
  of the declared map was unbacked RAM on them. See `HANDOFF.md`, "The RAM
  floor". **The trade: zlOS no longer claims to boot on a 256 MB machine.**
- **Stage 2 — raise the kernel size ceiling. DONE.** `CHUNKS` 60 → 192 (1.875 →
  6 MiB), which forced the raw-boot stack 6 → 12 MiB and the program arena
  8 → 14 MiB. `mkdisk.sh`'s image size is now *derived* from `CHUNKS` instead of
  being a literal that could drift from it. See below.
- **Stage 3 — a real allocator with free/reuse. DONE.** `heap.c`, 64 MiB at
  256 MiB, segregated free lists over boundary tags. Free, reuse, coalesce,
  realloc-in-place, and a worst case **measured** at 3 steps for `alloc` and 4
  for `free`. It is added *alongside* `arena.c`, which is unchanged — see "two
  allocators, on purpose" below.
- **Stage 4 — paging for the kernel's own use. DONE, for one region.**
  `paging.c` maps the heap's 64 MiB at virtual 4 GiB on the 64-bit builds, with
  everything else identity-mapped underneath. `dma.h` is the seam every
  device-visible address now passes through — 48 forward sites and 3 inverse —
  and `check-dma.sh` fails the build if a new one skips it. See below.
- **Stage 5 — ring 3 and syscalls. PARTIALLY DONE.** Ring 3, a TSS, a DPL-3
  syscall gate and a working `int 0x80` round trip exist on the 32-bit build and
  are pinned by `verify.sh`'s golden transcript. **Per-process address spaces do
  NOT exist.** See "what Stage 5 does and does not buy" below — the distinction
  between privilege separation and memory isolation is the whole of it.

---

## Stage 2 in detail: why raising one number moved two others

`CHUNKS` was 60 — 1.875 MiB — against a measured payload of 1,614,532 bytes.
**343 KiB of headroom**, and the 53-app desktop suite already planned is about
660 KB. It was overrun before anyone wrote the apps.

The number had been set twice before to just past wherever the kernel happened
to be that day: 40 (1.25 MiB) against a 1.23 MiB kernel — 84 KiB spare, which
the v10 type scale walked straight through — then 60. Setting it a third time
the same way would buy one more release.

So it is 192 (6 MiB), 3.9× the current image, sized against where the kernel can
plausibly go. That has a consequence: the loader now fills `0x100000` ..
`0x700000`, and **the raw-boot stack was at 6 MiB — inside the region being
loaded.** A machine that overwrites its own stack while booting. So:

| | was | now |
|---|---|---|
| `CHUNKS` | 60 (1.875 MiB) | **192 (6 MiB)** |
| raw-boot stack top | 6 MiB | **12 MiB** |
| `ARENA_BASE` | 8 MiB | **14 MiB** (16 MiB budget → ends 30 MiB) |
| `mkdisk.sh` image size | literal `2M` | **derived: `512 + CHUNKS*32 KiB`** |
| linker `ASSERT`s ×6 | 6 MiB / 8 MiB | **12 MiB / 14 MiB** |

The stack top is written in **three** places — `raw_entry.S`, `raw_boot.asm`'s
`pm_entry`, and `arena.c`'s `RAW_STACK_TOP` which asserts on it. All three or
none; `arena.c`'s `_Static_assert` is what catches two of the three.

`ARENA_BASE` is written in **three** places too — `arena.c`, and deliberately
duplicated in `hosttest/arenatest.c` and `hosttest/libctest.c` so a partial move
fails loudly in the harness instead of testing a different address than the
kernel uses. `arenatest` asserts `arena_base_addr() == ARENA_BASE`, which is the
check that fires.

### The derived image size, and the drift that was already there

`mkdisk.sh` had `truncate -s 2M` beside a comment claiming "12 * 32 KiB",
against a `CHUNKS` of 60. **All three disagreed** — 2 MiB, 384 KiB, 1.875 MiB —
and it worked only because 2 MiB happened to be the largest. Had anyone raised
`CHUNKS` without noticing the literal, the loader would read past the end of the
image, INT 13h returns carry, and `raw_boot.asm` prints `D` and halts: a dead
machine with no build error. It is now `512 + CHUNKS * 64 * 512`.

### The neighbour nobody was checking

`check-memmap.sh` swept `kernel.zl`'s fixed buffers from 32 MiB up.
`memmap.h`'s chain covered the map from 128 MiB up. **The 16 MiB program arena
sat between the two, in neither** — a region larger than everything
`check-memmap.sh` checked put together, invisible to it.

That cost nothing while the arena had 24 MiB of clearance. Moving it is exactly
when a gap gets spent, so the arena is now folded into `check-memmap.sh`'s
overlap sweep (read from `arena.c`, not restated), plus a `_Static_assert` in
`arena.c` against `kernel.zl`'s lowest fixed address. Validated by planting an
`ARENA_BASE` that overlaps `SNAKE_X` — six overlap FAILs — and by renaming the
constant away entirely, which fails loudly rather than sweeping a map with no
arena in it.

---

## Stage 3 in detail: two allocators, on purpose

`arena.c` is **not** replaced and **not** deprecated. The two answer different
questions and keeping both is a decision, not a migration that stalled:

| | `arena.c` | `heap.c` |
|---|---|---|
| whose memory | a zl program's | the kernel's own |
| freeing | all of it at once, `arena_reset()` | one object at a time |
| `free()` | there isn't one | `heap_free`, with coalescing |
| lifetime | one `run` | the uptime of the machine |
| what it defends against | use-after-free, by making every pointer die at the same instant | fragmentation and leaks |

`k_malloc`/`k_free`/`k_realloc` in `interp_kernel.c` **still forward to the
arena, deliberately.** Pointing them at the heap would look like progress and
would be a regression: zl programs currently get their memory reclaimed
wholesale between runs, and a program that leaks cannot hurt the next one. On
the heap they would leak for real. The migration candidates are kernel-side
allocations that outlive a program — window lists, the browser's DOM, anything
that today gets a fixed buffer sized for the worst case — not `k_malloc`.

### The design, and the two things that were hard

**Segregated free lists over boundary tags.** Free blocks are filed by size into
175 bins with a bitmap per level, so "find a block big enough" is a mask and a
count-trailing-zeros, never a walk. Every block records its own size and its
predecessor's, so coalescing on free is arithmetic on both neighbours.

**Hard thing 1 — a bin holds a RANGE, so its head may not fit.** One bin per
power of two is the obvious scheme and it is wrong under fragmentation: the
allocator must either walk the list (the unbounded pause a compositor cannot
afford) or skip the bin, which **refuses an allocation while a block that would
have fitted sits in the list it skipped**. The first draft did the second. The
fix is TLSF's: cut each power of two into 8 slices and round the request up to a
slice boundary, so every block in the chosen bin is at least the request. The
head always fits. Cost is ≤12.5% waste on the size class.

**Hard thing 2 — `split()` could leave two adjacent free blocks.** From
`heap_alloc` this is impossible: the block being split just came off a free
list, and a free block can never have a free neighbour. From `heap_realloc`'s
shrink path it happens routinely, because the block being split is *live* and
its successor is free as often as not. `heaptest.c` caught it — 1921 of 1923
checks passed and both failures were `realloc shrink`. It is invisible from
inside `alloc`/`free`: the metadata stays self-consistent and the heap merely
stops coalescing, which surfaces days later as an out-of-memory the byte counts
contradict.

### Offsets, not pointers

Free-list links are `u32` offsets from `HEAP_BASE`, never pointers, so the
block header is 16 bytes on the 32-bit kernel, the 64-bit kernel, the LLP64 EFI
target and the 64-bit Linux host alike — asserted, not hoped. `CLAUDE.md` opens
with the two times this project shipped a pointer-sized quantity that differed
between builds. An allocator's header is that hazard in its purest form: three
different heaps compiled from one file, corrupting on one target only.

### What proves it

`hosttest/heaptest.c` compiles the shipping `heap.c` unmodified as a Linux
program. **1923 checks, 0 failures**, no QEMU:

- `heap_check()` — a full boundary-tag walk — after **every** operation in the
  stress phases, not at the end. An allocator that is sound at the end and
  briefly corrupt in the middle is an allocator that corrupts memory.
- 32 MiB allocated, freed, and allocated again *at the same address*: the one
  behaviour `arena.c` cannot produce.
- 1024 blocks freed in scrambled order, then the whole heap allocated as one
  block — coalescing proved by its consequence, not by reading its metadata.
- 4000 random operations straddling the 512-byte small/large boundary, with
  every payload verified byte-for-byte before it is freed. That is what catches
  two live allocations that overlap, which `heap_check()` cannot see because the
  metadata stays consistent.
- the determinism claim **measured**: `alloc 3 steps, free 4 steps` worst case
  over the whole run, with thousands of free blocks outstanding. If the bound
  were secretly O(free blocks) those numbers would be in the thousands.

### The limit worth knowing

`ram_backed()` answers *"is there RAM here"*. It does **not** answer *"is this
RAM unclaimed"*, and on the EFI path that second question is open — firmware
chooses where to load the image and nothing here reads the UEFI memory map. That
is true of every region in `memmap.h` and is why each driver ships a
`*_ram_ok()`; the heap is no worse and no better. The probe restores what it
found so a probe alone is harmless, but `heap_init()` writes a block header
straight after. **Turning that convention into a check means reading the UEFI
memory map, which is Stage 4 work.** Until then the evidence is `verify-efi.sh`
booting green with it running.

---

## Stage 4 in detail: one window, and a seam everywhere else

### What actually got mapped, measured

`paging.c` takes a free **PML4** slot, hangs its own PDPT and page directory off
it, and maps the heap's 64 MiB physical at 256 MiB into it with 2 MiB pages. On
the 64-bit EFI path — the only 64-bit path with a boot gate:

```
  arena: 16 MiB at 14 MiB, ends at 30 MiB, ceiling 128 MiB (bg_buf)
  heap: 64 MiB at 130560 GiB [VIRTUAL, physical 256 MiB]
  vmm: 64 MiB mapped: virtual 130560 GiB -> physical 256 MiB  (everything else identity)
  ready.
```

130560 GiB is PML4 slot 255 (255 × 512 GiB). The kernel boots to `ready.` with
its heap living at an address that is not its physical address.

### It took two wrong answers, and the first one was silent

Both were the same mistake — reasoning about *firmware's* page tables from *our
bootloader's*:

1. hardcoded PDPT slot 4 (virtual 4 GiB), because `boot64.S` fills PDPT[0..3]
   and leaves the rest zero;
2. scanned PDPT slots 4..511 for a free one.

Both refused on every EFI boot. **The first version refused silently**, and the
boot line read `vmm: identity only - no window` — which is exactly what a build
with no paging prints, so it looked intentional. Making every refusal name its
reason produced the actual answer in one boot:

```
  vmm: refused - every PDPT slot from 4 to 511 is already mapped
```

OVMF identity-maps its whole address space with 1 GiB pages, so under `PML4[0]`
there is no free PDPT slot at all. One level up there is room to spare.

**A third trap sat under that one:** the first measurement of the fix was taken
against a *stale* `zlOS-usb.img`. `buildefi.sh` builds `BOOTX64.EFI`; it does
**not** build the USB image — `mkusb.sh` does, and `verify-efi.sh` calls it. So
a rebuilt binary and an unchanged image gave a result that described the old
code. `HANDOFF.md` already says it: *if a diagnostic result is impossible, check
what you actually booted before you check anything else.*

The heap is the right first region and the choice is not arbitrary:

- nothing inside it has a fixed address, so no other file names one;
- **no device is ever given a pointer into it** — `heap.c` is deliberately
  outside `check-dma.sh`'s DMA set, and `kernel/docs/reference/system/dma-sites.md` enumerates every
  address that reaches hardware, none of which is a heap pointer;
- if the window fails, the physical address is a complete, already-tested
  system. There is no half state.

### How it refuses

Four layers, because this is the file where "it looked right" is worth least:

1. the walk is validated at every step (CR3 sane, `PML4[0]` present and not a
   huge page, **the PDPT slot currently absent**);
2. the entries are **read back** after writing and compared — fault-free, it
   dereferences no new address;
3. only then is the virtual address touched, and the probe writes through
   *virtual* and reads at *physical*, **both directions**. That is what catches
   a window that works but aliases the wrong memory. A merely-absent mapping
   faults instead, and `idt.c` installs `fault_isr` on all 32 exception vectors,
   so it halts with a message rather than triple-faulting;
4. any failure zeroes the entry, reloads CR3, and returns 0 — loudly.

`CR0.WP` is cleared around the table write and restored immediately, because on
the EFI path the PDPT is *firmware's* and firmware commonly marks its own page
tables read-only.

### What is still identity, deliberately

Every region a **device** can reach — `HI_XHCI`, `HI_NVME`, `HI_VGPU`,
virtio-net's arena — stays identity-mapped, and there is no plan to change it. A
driver handed a physical address today and a physical address tomorrow does not
have to change at all, and DMA is where this project's recurring bug class
lives. `dma_addr()` exists so that *if* it ever changes, it changes in one
place. It is not a promise that it will.

The 32-bit build has paging **off** (`CR0.PG` is never set). There is nothing to
extend, and `vmm_map_window()` says so and returns 0.

### The DMA seam, and the four sites I missed

`kernel/docs/reference/system/dma-sites.md` is the full account. The short version, because it is the
part worth remembering:

**My own enumeration found 44 sites and missed four**, all in `xhci.c`, all live
— every keystroke and every mouse report went through two of them. They were
found by an independent audit whose only brief was to break the conversion. A
second audit found that my *checker* was structurally blind to 11 of the 20
virtio sites, because those passed region bases with no cast at all.

Both holes are closed (`check-dma.sh` rules 3 and 4, each validated against the
exact shape that slipped through). **The honest answer to "how do you know you
found them all" is that a single careful pass did not — three independent
things agreeing is what closes it.**

The audit also found the half nobody looks for: **the seam needs an inverse.**
A device reports addresses *back* — an xHCI Command Completion Event carries the
address of the TRB it completed — and two places compare those against kernel
addresses. Neither is an outbound site, so no outbound enumeration finds them,
and both break on the identical commit. `dma_kaddr()` is that inverse.

---

## Stage 5 in detail: what ring 3 does and does not buy

`usermode.c` runs a program at ring 3 that talks to the kernel only through
`int 0x80`, and comes back. Measured, and pinned by `verify.sh`'s golden
transcript:

```
  ring 3: u31 <- from ring 3 via int 0x80, 5 syscalls, returned to ring 0
```

The `u` and the `3` were written by `SYS_WRITE` calls made **from ring 3**; the
`1` is what `SYS_GETPID` returned to ring 3 and ring 3 then handed back. Those
bytes can reach the serial port no other way.

### Two bugs it took to get there, both worth keeping

**1. The syscall ABI, caught by a branch nobody expected to need.** The payload
originally declared `eax` as an input only:

```c
__asm__ volatile("int $0x80" :: "a"(SYS_WRITE), "b"('u'));
```

The handler returns its value in `eax` — that is the ABI — so after the first
call `eax` held 0, and gcc, told only that `eax` was an input it had already set
up, did not reload it. The second syscall arrived as call number 0. The boot log
said so exactly:

```
  ring 3: u  syscall: ring 3 asked for unknown call 0
```

Had the unknown-call branch been silent — the tempting choice for an unused
number — the log would have read `u1` and looked like a dropped character rather
than a broken calling convention.

**2. Interrupts stayed off, and this one had no visible symptom at all.** The
syscall vector is an *interrupt* gate, which clears IF on entry — correct, since
a trap gate would let a timer tick re-enter the handler. But `SYS_EXIT` does not
`iret`; it restores ESP and jumps, because it is abandoning ring 3 rather than
returning from an interrupt, and **a jump restores no EFLAGS**. So interrupts
were off for the whole rest of the boot. The kernel came up, printed `ready.`,
and then ignored every keystroke and every timer tick forever.

The boot log looked completely healthy up to the prompt. The only evidence was
three boot gates reporting *"kernel never halted - it hung"*, and a bisect
showing that even `.q` — press q, halt — hung. `user_enter()` now saves EFLAGS
before the transition and restores them after. `sti` alone would have been
wrong: it would enable interrupts for a caller that had deliberately disabled
them.

**Three things must all be true or ring 3 triple-faults** (a silent reboot loop
with no message):

1. a TSS with `ss0`/`esp0`, loaded with `ltr` — the CPU reads the ring-0 stack
   out of it on the first interrupt taken in ring 3;
2. an IDT gate with **DPL 3** for the syscall vector, and only that vector — a
   ring-3 `int` through a DPL-0 gate is a `#GP`, which is exactly why ring 3
   cannot fake a page fault or a timer interrupt;
3. an `iret` with a ring-3 frame — there is no "drop privilege" instruction.

### The distinction that matters

> **Ring 3 is privilege separation. It is NOT memory isolation.**

What a ring-3 program can no longer do — each is a `#GP` instead of an
instruction: `cli`/`sti`, `hlt`, `in`/`out`, `lgdt`/`lidt`/`ltr`, writes to
`cr0`/`cr3`/`cr4`, `wrmsr`, `invlpg`. That is the difference between a runaway
program that corrupts a buffer and one that disables interrupts and wedges the
machine.

What it does **not** buy on the 32-bit build: paging is off, the ring-3 segments
are flat 4 GiB, and a flat segment isolates nothing. **A ring-3 program can
still read and write any address.** It simply cannot reprogram the machine.

### What is left, precisely

Per-process address spaces need, in order: the page-table `U/S` bit honoured
per region; a per-process CR3 and the tables behind it; a process abstraction
that owns one; a loader that places a program in it; and the syscall ABI
extended to take **pointers**, which today it deliberately does not — a pointer
argument has to be range-checked against an address space that does not exist
yet. `syscall_dispatch()`'s three calls pass values only, on purpose.

Ring 3 is 32-bit-only, and that is a scope decision: the entry stub is
hand-written assembly, and a 64-bit one wants `syscall`/`sysret` plus three MSRs
— writable, but with **no boot gate that could prove it works**, since the only
64-bit boot gate is `verify-efi.sh`. Shipping an untested second copy is how
this repo acquires subsystems that have never executed.

---

## Rules that survive all five stages

- **A region's ceiling is the next region's base.** "Does this fit" is that
  subtraction, never a compile-time pixel count — a pixel count silently stops
  being true when the panel gets bigger.
- **A literal equal to a region base is a deliberate cross-check; a literal
  strictly inside one is the bug.** That is `check-himap.sh`'s whole rule, and
  it needs no allowlist to rot.
- **DMA needs *physical* addresses.** The device does not know about your page
  tables. This is why Stage 4 is dangerous and why `virtio_gpu.c`, `xhci.c` and
  `nvme.c` all say "fixed physical addresses, identity mapped on every boot
  path" in their headers.
- **Every guard gets a planted defect.** `docs/GUARDS-THAT-DID-NOT-GUARD.md`
  lists five checks in this tree that reported green while checking nothing. A
  check whose failure mode is silence looks exactly like a check that passed.
