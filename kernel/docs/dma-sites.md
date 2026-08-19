# Every place zlOS hands an address to hardware

Written 2026-08-20 as the prerequisite for Stage 4 (paging for the kernel's own
use). Stage 4's whole hazard is one sentence:

> **DMA needs physical addresses. The device does not know about your page
> tables.**

Turn on virtual memory without finding every site below and a driver keeps
working right up until the allocator hands it an address whose virtual and
physical forms differ. Then the device writes to the wrong memory. Nothing
faults, nothing logs, and the symptom appears somewhere else entirely — which is
the failure mode `HANDOFF.md` already counts **six** times under "the recurring
bug class".

So this page is the inventory, and more importantly the argument for why the
inventory is **complete** rather than merely long.

---

## Why this list is closed

A device can only reach memory whose address we gave it. There are exactly
three mechanisms by which an address of ours reaches a device, and every DMA
site in this kernel is one of them:

**(a) Written to a register on the device.** A store to a BAR-derived address.
In this tree that is always one of five one-line helpers — `mmio_w` in
`virtio_gpu.c`, `virtio_net.c`, `gpuring.c` and `intel.c`, and `wr32` in
`xhci.c`, `nvme.c` and `i2c_hid.c`. There is no other way to write a device
register, because there is no other mapping of a BAR.

**(b) Written into a descriptor or ring that the device reads** — where that
ring's *own* address reached the device by (a). This is transitive and
terminates: rings do not create new rings.

**(c) Written into a device-side page table.** Exactly one exists: the GPU's
GGTT, in `intel.c`.

That closes the set. It also means the enumeration can be done from the *value*
side as a cross-check, which is the second half of the argument: **every DMA
buffer in this kernel is at a constant declared in `memmap.h`**, or at a
driver-local base derived from one. So grepping the `HI_*` constants and the
handful of local bases finds the same set from the other direction. Both
directions are listed below and they agree.

The one thing neither direction would catch is a driver that mapped a BAR and
wrote to it without using its file's helper. `check-dma.sh` is what makes that
a build failure rather than an assumption.

---

## The sites

Generated from the tree, not hand-typed — the first version of this table had
line numbers already stale by 1–8 and an entry (`VMEM_SGLIST`) for a region that
turned out to be dead. Regenerate with:

```
grep -n 'dma_addr(\|dma_kaddr(' kernel/{virtio_gpu,virtio_net,xhci,nvme}.c
```

**48 forward sites and 3 inverse sites**, as of the conversion commit.

| file | forward | inverse |
|---|---|---|
| `virtio_gpu.c` | 7 | — |
| `virtio_net.c` | 9 | — |
| `xhci.c` | 33 | 3 |
| `nvme.c` | 7 | — |

### The categories, and what is in each

**(a) Written to a device register.** `virtio_gpu.c` and `virtio_net.c` queue
bases (`CC_QUEUE_DESC`/`DRIVER`/`DEVICE`, both halves); `xhci.c` `XOP_DCBAAP`,
`XOP_CRCR`, `XRT_ERDP` (twice — init and every event-ring drain), `XRT_ERSTBA`;
`nvme.c` `NVME_ASQ`, `NVME_ACQ`.

**(b) Written into a structure the device reads.** virtqueue descriptor `addr`
fields (`desc_set`, 6 call sites across the two virtio drivers); the
RESOURCE_ATTACH_BACKING scatter entry pointing at the framebuffer; `xhci.c`'s
scratchpad pointer array, `dcbaa[0]`, `dcbaa[slot]`, the ERST segment base, six
Link TRBs, the control Data-stage TRB, the bulk Normal TRB, the keyboard and
pointer interrupt-IN TRBs, four Input Context pointers in commands, and four
endpoint-context TR Dequeue Pointers (both halves); `nvme.c`'s PRP1 in every
command it builds.

**(c) The device-side page table.** `intel.c`'s `ggtt_map()` — and it is
deliberately NOT wrapped. See below.

### The four that were missed, and how they were found

My own enumeration produced 44 sites and **missed four**, all in `xhci.c`, all
on live paths:

```
1008  cmd_submit((u64)(ring | 1u), ... Set TR Dequeue Pointer)   every stall recovery
1398  trb_write(ring, kbd_enq, (u64)KBD_REPORT, ...)             every keystroke
1422  trb_write(ring, ptr_enq, (u64)PTR_BUF(ptr_enq), ...)       every mouse report
1426  trb_write(ring, PTR_RING_USE, (u64)ring, 0, ...)           every 32 reports
```

They were found by an **independent read-only audit** run against the converted
tree, whose entire brief was to find a site the conversion had overlooked. Two
of them are the hottest DMA paths in the file. Both of their *Link TRB siblings*
in the same functions were already converted — the inconsistency was inside
single functions, which is exactly what a careful reading of one's own diff does
not catch.

**That is the answer to "how do you know you found them all": I did not, on my
own.** What closes it is three independent things agreeing — the transitive
argument above, a separate audit per driver that was told to break it, and
`check-dma.sh` now failing on the exact shapes that were missed.

### The inverse direction, which the outbound audit nearly missed too

A device does not only read addresses we give it — it **reports them back**. An
xHCI Command Completion Event carries the address of the Command TRB that
produced it; a Transfer Event carries the address of the TRB that completed.
Two places compare those device-reported addresses against kernel addresses:

```
 613  cmd_wait():  if (dma_kaddr(p) != trb_addr) continue;
1914  kbd_event(): u32 pk = (u32)dma_kaddr(param);  idx = (pk - ring)/TRB_BYTES;
```

Neither hands anything to a device, so neither appears in any outbound
enumeration — and both break on the identical commit. Without the inverse,
every command would time out because no completion ever matches, and every
pointer report would fail its bounds check and be dropped. A half-converted
driver is worse than an unconverted one, because the outbound half looks
finished.

### Deliberately not DMA

- **`i2c_hid.c`** — every byte arrives by CPU store through `IC_DATA_CMD`.
- **`intel.c`'s EDID path** — `GMBUS3` register reads, byte at a time. This is
  why `edid_buf` was *deleted* rather than given a region
  (`GUARDS-THAT-DID-NOT-GUARD.md` §2).
- **`fb.c`'s back buffer and blur arena** — CPU-rendered. The scanout address
  the display engine reads is a GGTT address, programmed at (c).
- **`sched.c` task stacks, the AP stacks** — CPU only.
- **`nvme.c`'s `sqe()` `base` argument and `cq_wait()`'s `cq_base`** — those are
  CPU pointers to queues whose addresses reached the controller separately at
  `NVME_ASQ`/`CREATE_SQ`. They must stay *virtual* once paging is on, so
  wrapping them would be the mirror-image bug.
- **`xhci.c`'s `trb_write()` first argument** — the ring's own CPU address.
  `check-dma.sh` strips it before matching for exactly this reason.

### Known gaps, named rather than left to be discovered

1. **`VMEM_SGLIST` is dead.** `virtio_gpu.c:128` defines it and `:157` asserts
   on it; nothing else references it. The single scatter entry is written inline
   at `c[8]`/`c[9]`. An earlier version of this document credited it as a live
   site. It is a reserved region, not a DMA site.
2. **NVMe PRP2 does not exist yet.** `sqe()` zeroes all 16 dwords and never
   writes `e[8]`/`e[9]`, so PRP2 is always 0. Whoever adds it — needed for the
   >4096 blocksize case `nvme.c:350` currently refuses outright — creates a
   device-visible address built by pointer arithmetic into a list buffer, which
   is the "computes a device address some new way" case `check-dma.sh` admits it
   cannot see.
3. **Three accessors hand raw kernel addresses out of the seam's reach:**
   `virtio_gpu_fb()`, `nvme_data()`, `virtio_net_arena()`. CPU pointers today.
   If zl ever passes one into a device-facing builtin, that becomes a DMA site
   in `kernel.zl` — outside all four files and outside `check-dma.sh`'s reach.

## What Stage 4 has to do with this list

**Not** "translate these seven files". The order that keeps it safe:

1. ~~**Introduce the seam first, as an identity function.**~~ **DONE.**
   `dma_addr()` was inserted at every site above while paging was still off, so
   the boot gates proved behaviour unchanged and the diff was mechanical.
2. ~~**Make the seam a build requirement.**~~ **DONE** — `check-dma.sh`, four
   rules, each validated against a defect planted on purpose.
3. ~~**Only then turn on a non-identity mapping, and only for the heap.**~~
   **DONE** — `paging.c`, 64 MiB at a free PML4 slot. Measured on the 64-bit
   EFI path:

   ```
   heap: 64 MiB at 130560 GiB [VIRTUAL, physical 256 MiB]
   vmm:  64 MiB mapped: virtual 130560 GiB -> physical 256 MiB  (everything else identity)
   ready.
   ```

4. The regions in `memmap.h` that devices *do* reach stay identity-mapped, and
   there is no deadline on that. A driver handed a physical address today and a
   physical address tomorrow does not need to change at all. `dma_addr()` now
   consults `paging.c` and returns the identity for every one of them, which is
   a checked fact rather than a structural one.

**Step 3 must not include `intel.c`'s `ggtt_map`.** The GGTT is a second
translation layer that the CPU's page tables do not affect; feeding it anything
but a true physical address is silent corruption with a hardware engine doing
the writing.
