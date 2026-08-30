# The physical memory map, measured

There is no allocator in this kernel, so every multi-megabyte buffer lives at a
hardcoded physical address and the only thing keeping one out of the next is
arithmetic somebody has to actually do. This project has hit that collision
five times. `fb.c:94-119` holds the canonical map and tells you to re-grep it
rather than trust it. This file is that re-grep, done once, with the numbers
measured instead of inherited — plus the two things the map comment does not
say, both of which change what you are allowed to do.

## The thing nobody had written down: how much RAM is actually there

**Not one gate in this project passes `-m` to QEMU.**

```
kernel/verify.sh      qemu-system-i386   -kernel kernel.elf ...        no -m
kernel/tools/checks/verify-raw.sh  qemu-system-i386   -drive ...                    no -m
kernel/tools/checks/verify-iso.sh  qemu-system-i386   -cdrom zlOS.iso ...           no -m
                      qemu-system-x86_64 -drive if=pflash ... (UEFI)   no -m
kernel/run.sh         qemu-system-i386   -kernel kernel.elf            no -m
kernel/tools/run/run-vm.sh      qemu-system-i386   -cdrom zlOS.iso -m 256        256 MiB
```

QEMU's default is not folklore, it is queryable:

```bash
(echo '{"execute":"qmp_capabilities"}'; sleep 0.4;
 echo '{"execute":"query-memory-size-summary"}'; sleep 0.4) |
  qemu-system-i386 -machine pc -display none -S -qmp stdio
```

```
{"return": {"base-memory": 134217728, "plugged-memory": 0}}
```

134217728 bytes is **exactly 128 MiB**. So on every gate this project runs,
**`0x08000000` and everything above it is not backed by RAM** — which is the
entire high-RAM map. `run-vm.sh` with `-m 256` reaches 255 MiB, the top of the
map, and not one byte further.

This is why `fb.c` reports `back OFF` under the gates, why `xhci.c` has
`xhci_ram_ok()` at all (`xhci.c:406`), and why `kernel.zl:1284` can print "the
DMA arena at 224 MiB is not backed by RAM". None of that was news to those
files; what was missing was the consequence for anything *new*:

> **A new fixed buffer placed above 128 MiB is dead code on every gate.**
> It will pass review, link, boot, and never once be exercised.

## The map, end to end

Each entry read out of the file that owns it. `end` means base plus the largest
size that code can actually reach, not the size it usually uses.

| base | end | owner | what | evidence |
|---|---|---|---|---|
| `0x00000000` | `0x00006FFF` | boot64.S | PML4, PDPT, PD0..PD3 | `boot64.S:40-42` |
| `0x00008FE0` | `0x00008FEF` | smp.c | `CR3_PTR` | `smp.c:57` |
| `0x00008FF0` | `0x00008FFF` | smp.c | `ENTRY_PTR` | `smp.c:56` |
| `0x00009000` | `0x00009FF0` | smp.c | SMP trampoline | `smp.c:55,124` |
| `0x000A0000` | `0x000FFFFF` | — | VGA hole, BIOS ROM | — |
| `0x00100000` | `0x0028F044 (STALE - see the note below)` | link.ld | **the kernel image** | measured, below |
| ↓ | `0x00600000` | raw_entry.S | raw-boot stack **top**, grows **down** | `raw_entry.S:16`, `raw_boot.asm:196` |
| **`0x00800000`** | **`0x01800000`** | **arena.c** | **the program arena** | `arena.c` `ARENA_BASE` |
| *(free)* | `0x08000000` | — | 104 MiB unclaimed | — |
| `0x08000000` | `0x0A000000` | fb.c | `bg_buf`, drag snapshot | `fb.c:120` |
| `0x0A000000` | `0x0B000000` | fb.c | `sp_buf`, drag sprite | `fb.c:121` |
| `0x0A800000` | +16 KiB/core | **smp_trampoline.S** | **AP stacks — see below** | `smp_trampoline.S:25` |
| `0x0B000000` | `0x0B040000` | sched.c | 8 task stacks × 32 KiB | `sched.c:54-56` |
| `0x0C000000` | `0x0D000000` | fb.c | `back`, the back buffer | `fb.c:123` |
| `0x0C900000` | +64 B | **i2c_hid.c** | **HID reports — inside `back`** | `i2c_hid.c:104` |
| `0x0C980000` | +128 B | **intel.c** | **EDID buffer — inside `back`** | `intel.c:762` |
| `0x0D000000` | `0x0D011000` | nvme.c | admin/IO queues, one data page | `nvme.c:101-106` |
| `0x0E000000` | `0x0E440000` | xhci.c | DMA arena + 4 MiB scratchpad | `xhci.c:362-372` |
| `0x0F000000` | `0x0FF00000` | virtio_gpu.c | rings, then 14 MiB framebuffer | `virtio_gpu.c:120-131` |

The kernel image end is measured, because it is the one entry that moves every
time somebody adds a file:

```
$ readelf -S kernel.elf | grep '\.bss'
  [ 6] .bss  NOBITS  00241820 142810 04d824
```

`0x00241820 + 0x0004D824 = 0x0028F044` = **2.559 MiB**.

## The AP stacks are not in `fb.c`'s map, and they collide

`smp_trampoline.S:25` and `smp_trampoline64.S:34` both set
`STACK_BASE = 0x0A800000` — **168 MiB**, `STACK_SIZE 0x4000` per core. That
address is absent from `fb.c`'s map, which lists `sp_buf` at 160 MiB and the
next occupant at 176 MiB.

So `fb.c:127` computes `SP_LIMIT = HI_SCHED - HI_SP` = **16 MiB** and
`fb.c:1423` bounds the drag sprite against it. The nearest real occupant is
**8 MiB** away, not 16.

A drag sprite for a 1920×1200 window is `1920 × 1200 × 4` = 8.79 MiB.
`160 + 8.79 = 168.79 MiB` — past the AP stacks. The bounds check passes,
because it is measuring against a neighbour that is not the nearest one.

Found by `nm kernel.elf | grep ' a '` reporting an absolute `STACK_BASE` of
`0a800000` that did not match `sched.c:56`'s `0x0B000000`. Two symbols, one
name, two files. Tracked as T-EXEC-1 in `.ultra/TENSIONS.md`; not fixed there,
because `fb.c` and `smp.c` belong to other tracks.

## The back buffer has the same problem, and it is not latent

Two more buffers live inside a span the map hands to somebody else, and this
pair is reachable on the actual test laptop at its actual resolution.

`fb.c:123,128` gives `back` `0x0C000000`..`0x0D000000` and sizes it with
`BACK_LIMIT = HI_NVME - HI_BACK` = **16 MiB**. Inside that span:

| address | offset into `back` | owner |
|---|---|---|
| `0x0C900000` | **9.00 MiB** | `i2c_hid.c:104` `HID_BUF`, input reports |
| `0x0C980000` | **9.50 MiB** | `intel.c:762` `edid_buf` |

`back`'s real clear headroom is **9 MiB, not 16**:

```
1920x1200    8.79 MiB -> ends 0x0C8CA000   fits, by 216 KiB
2560x1440   14.06 MiB -> ends 0x0CE10000   OVERRUNS HID_BUF by 5.06 MiB
2560x1600   15.62 MiB -> ends 0x0CFA0000   OVERRUNS HID_BUF by 6.62 MiB
```

`back_on` is **true** at 2560×1440, because 14.06 < 16 and `BACK_LIMIT` is
measuring against a neighbour that is not the nearest one. Every frame the
compositor draws then writes through the touchpad's report buffer and the EDID
read off the panel.

`fb.c:111-118` was rewritten precisely because "the ThinkPad's 2560x1440 made
`back_on` 0". It now makes `back_on` 1 and lands on two other buffers instead.
QEMU's 1920×1200 clears `HID_BUF` by 216 KiB, which is why no gate has seen it —
and `i2c_hid.c` is real-hardware-only anyway (QEMU has no LPSS I2C), so no gate
can. Tracked as T-EXEC-5. Not fixed here: `fb.c` is off-limits to every track and
`intel.c`'s write paths can damage hardware.

`intel.c:763` already exposes `intel_set_edid_buffer()`, so half the fix is one
call from whoever owns the map.

## Why the program arena is at 8 MiB and not 256

The obvious place for a new buffer is above the map, at 256 MiB. Per the
measurement above, that is dead code. So `arena.c` goes **below** it, in the
span between the kernel image and `bg_buf`:

- **above the raw-boot stack top at 6 MiB**, which grows *down*, with 2 MiB of
  clearance — and the kernel image has 3.4 MiB of room to grow *up* before it
  reaches that stack
- **2 MiB aligned**, because `boot64.S` maps the low address space with 2 MiB
  pages
- **ends 104 MiB below `bg_buf`, and 104 MiB below the RAM ceiling.** Those are
  the same number today for two entirely different reasons, and `arena.c`
  asserts them separately, because `-m 512` makes them different numbers and
  only one of the two checks still applies.

The size — 16 MiB out of a 120 MiB span — is a **budget, not the geometry**.
When a program hits the ceiling that means the program misbehaved. A ceiling set
to "whatever was left" tells you nothing when you hit it.

## The compiler is made to say so

`fb.c:152-169` argues that a map only makes sense if the bases are in ascending
order, and that a comment claiming the order would not have caught a reordering.
The same argument applies to the one buffer that is not the kernel's, in the one
direction a static assert cannot see: the kernel image is not a constant.

The linker knows the answer exactly, so all three scripts now say it:

```
__kernel_end = .;
ASSERT(__kernel_end <= 0x00600000, "... grown into the raw-boot stack at 6 MiB")
ASSERT(__kernel_end <= 0x00800000, "... grown into the program arena at 8 MiB")
```

Proven to fire rather than assumed to — relinking with the threshold moved to
2 MiB (below the measured 2.559 MiB image) produces exactly those two messages
and no object file.

## What is still an assumption

Under UEFI, `efi.c:253` calls `ExitBootServices` and never records the memory
map firmware returned, so nothing in this kernel knows which physical ranges
were firmware's. Every fixed buffer in the table above shares that assumption;
the arena is at a lower address than any of them, and low memory is where a
firmware loader is more likely to have put something.

`arena.c`'s `ram_backed()` probe therefore **saves and restores** the words it
tests, so a probe of an address that turned out to be occupied does not corrupt
it. `xhci_ram_ok()` does not bother and is right not to — it owns its arena
outright. Tracked as T-EXEC-3.


---

## STALE BY THREE MEGABYTES, and the argument above depends on the stale number

**Measured 2026-08-19 on `desktop/browser-next`, with the browser, HTTPS, the
JavaScript interpreter and the picture decoder all linked in:**

```
$ ./build.sh && nm kernel.elf | grep __kernel_end
005daac0 A __kernel_end

__kernel_end = 0x005DAAC0 = 5.854 MiB   link.ld ASSERT ceiling 6.000 MiB
HEADROOM     = 152,896 bytes = 149.3 KiB
text 1,750,279   data 332   bss 3,339,328
```

This file states the image ends at `0x0028F044` (2.559 MiB) and reasons from
"the kernel image has 3.4 MiB of room to grow up" when deciding the program
arena's base. **That figure is three megabytes out of date and the headroom is
now 149 KiB, not 3.4 MiB.** The placement conclusion still holds - the arena is
at 8 MiB and the image is nowhere near it - but the *margin* the reasoning
relied on is gone, so the next person to add a multi-megabyte array must not
take the old number as licence.

That is exactly what happened while writing `png.c`: a 2 MiB pixel arena as
BSS would not have linked, and the error would have read *"the kernel image has
grown into the raw-boot stack at 6 MiB"* - naming the stack rather than the
picture. The pixels went to `memmap.h`'s `HI_IMG` at 32 MiB instead.

This file's own header says to re-grep rather than trust it. This is that
re-grep, with the command that produced it, so the next one is cheaper:

```
./build.sh && nm kernel.elf | grep __kernel_end
```
