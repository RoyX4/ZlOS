# The zlOS address space, and what is actually enforcing it

Written 2026-08-20, during the work to turn zlOS's memory from a set of
hand-chosen constants into a memory *model*. This page is the layout as it
stands, **who enforces each boundary**, and which of the five stages is done.

`docs/memory-map.md` is the older, longer account of the same territory and is
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
| `0x10000000` | 256 MiB | — unclaimed — | | |
| `0x40000000` | 1 GiB | **`HI_TOP`** — the smallest guest we promise | `memmap.h` | `check-ram.sh` |

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
- **Stage 3 — a real allocator with free/reuse.** NOT STARTED. `arena.c` is a
  bump allocator with a reset and no `free()`; `k_malloc` in `interp_kernel.c`
  forwards to it.
- **Stage 4 — paging for the kernel's own use.** NOT STARTED. Page tables exist
  on the 64-bit path (`boot64.S` identity-maps the first 4 GiB with 2 MiB pages)
  but the kernel runs identity-mapped, which is *why* every buffer above needs a
  hand-chosen physical address.
- **Stage 5 — ring 3 and syscalls.** NOT STARTED. There is no user mode at all:
  no ring 3, no syscall entry, no user/kernel boundary.

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
