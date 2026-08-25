# CRITICAL — every GPU ring register write truncates a 64-bit BAR to 32 bits

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366` · verified by hand, end to end**

Raised by fleet driver agent `gpu-ring`. Re-derived here from source. **Confirmed.**

This is bug class 2 from `CLAUDE.md` — *"a DMA buffer outside guest RAM, or an address
truncated to 32 bits"* — living in the newest subsystem in the tree, the one that just
drove real silicon.

---

## The defect in three lines

```c
/* kernel/src/drivers/display/intel.c:435 — the exported accessor */
u32 intel_mmio(void)      { return (u32)mmio; }
```

```c
/* kernel/src/drivers/display/gpuring.c:248-256 — every ring register access */
static gr_u32 mmio_r(gr_u32 off)
{
    return *(volatile gr_u32 *)((gr_uptr)intel_mmio() + (gr_uptr)off);
}
static void mmio_w(gr_u32 off, gr_u32 val)
{
    if (!ring_armed) return;
    *(volatile gr_u32 *)((gr_uptr)intel_mmio() + (gr_uptr)off) = val;
```

`mmio` is a `uptr`. `intel_mmio()` returns `u32`. `gpuring.c` then widens that
already-truncated value back to `gr_uptr`. **The top 32 bits are gone before the cast
that looks like it preserves them.**

## Why it is not theoretical

`intel.c` assembles the full 64-bit BAR correctly, and the code doing it is careful and
well-commented:

```c
/* kernel/src/drivers/display/intel.c:414-425 */
mmio      = (uptr)pci_bar(i, 0);          /* GTTMMADR */
...
/* Written as two 16-bit shifts, never one 32-bit shift. On a 32-bit
 * uptr `x << 32` is undefined, and CLAUDE.md records what this exact
 * toolchain did with it: clang compiled the expression to a bare `ret`. */
if (sizeof(uptr) >= 8) {
    mmio     |= ((uptr)mmio_hi << 16) << 16;
    aperture |= ((uptr)aper_hi << 16) << 16;
}
```

So the driver goes to real trouble to build a >4 GiB address — and then the accessor
throws the top half away. The 32-bit build is explicitly refused earlier
(`intel.c:403-410`, `bar_too_high`), which means **the only builds that reach this code
are exactly the ones where the high bits can be non-zero.**

If firmware places GTTMMADR above 4 GiB, every `FORCEWAKE`, `RING_CTL`, `RING_START`,
`RING_HEAD` and `RING_TAIL` access goes to a truncated physical address — into RAM or
another device's BAR — and **does not fault.** Per this repo's own rule: *an
out-of-bounds access that does not fault landed in the next mapping.*

## Blast radius: the ring only, but the whole ring

```
$ grep -rn "intel_mmio()" kernel/*.c freestanding/*.c | grep -v 'out.c\|_gen'
freestanding/runtime_kernel.c:1476:  if (streq(name,"intel_mmio")) return zl_num((double)intel_mmio());
kernel/src/drivers/display/gpuring.c:249:  return *(volatile gr_u32 *)((gr_uptr)intel_mmio() + (gr_uptr)off);
kernel/src/drivers/display/gpuring.c:255:  *(volatile gr_u32 *)((gr_uptr)intel_mmio() + (gr_uptr)off) = val;
```

`intel.c`'s own display path is **safe** — it uses the raw `uptr mmio` internally
(`intel.c:291` `mmio_w`, and three `(volatile u32 *)(mmio …)` sites), never the
truncating accessor. The damage is confined to `gpuring.c`, which is to say: to
100% of the GPU command-ring register traffic.

The zl builtin at `runtime_kernel.c:1476` also reports a truncated address, so the
diagnostic a human would use to check this **agrees with the bug.**

## Why 116 green checks never saw it

```c
/* kernel/tests/host/gputest.c:33 */
static unsigned intel_mmio(void)    { return 0; }
```

The harness stubs the accessor to a constant `0`. There is no address to truncate, so
no possible test in `gputest.c` can distinguish the correct accessor from the broken
one. This is the repo's documented "green gate over a hole" pattern, and here the stub
*is* the hole.

## The fix

Change the accessor's type, not its callers:

```c
/* kernel/src/drivers/display/intel.c:435 */
uptr intel_mmio(void)      { return mmio; }
```

and update the three declarations to match:

| file:line | current | should be |
|---|---|---|
| `kernel/src/drivers/display/gpuring.c:56` | `gr_u32 intel_mmio(void);` | `gr_uptr intel_mmio(void);` |
| `kernel/tests/host/intel_probe.c:57` | `u32  intel_mmio(void);` | `uptr intel_mmio(void);` |
| `freestanding/runtime_kernel.c:623` | `extern unsigned int intel_mmio(void);` | `extern unsigned long long intel_mmio(void);` |

Then the harness stub at `hosttest/gputest.c:33` must return a **>4 GiB sentinel**
rather than `0`, or the gate goes on proving nothing. That change is the actual test:
watch it go red against the current accessor before trusting it.

`intel_mmio_size()` (`intel.c:436`) returns a size, not an address, and is fine as `u32`.

## Do this before arming anything

`gpu_ring_arm(1)` and the `G` command must not run on hardware until this lands.
`.ultra/STATE.md`'s "Next action" is to flash the USB image and press `G` on the
ThinkPad — **that is the action this bug is waiting for.** The prior silicon run
succeeded, which tells us this firmware happened to place GTTMMADR below 4 GiB on that
boot; it does not tell us it always will.
