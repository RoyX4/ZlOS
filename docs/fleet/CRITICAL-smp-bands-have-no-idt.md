# SMP band rendering: the APs have no IDT, and the barrier has no timeout

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366` · verified by hand**

Found by fleet driver agent `smp-bands`. Re-derived here. **Confirmed.**

This is the most decision-relevant finding of the run, because
`.ultra/STATE.md` §4 in that 2026-08-19 worktree ranked turning SMP bands on as
**"the best win-to-risk on the board"** and `kernel/docs/gpu-next.md` calls its risk
*"real but bounded."*

The win is real — a measured **1.78×** on the whole desktop redraw, from code already
written. The risk is not bounded in the way those documents describe.

---

## 1. Application processors run with no interrupt descriptor table

```
$ grep -n "lidt\|idt_load\|setup_idt\|IDT" kernel/smp.c kernel/smp_trampoline64.S
  (no output)
```

Neither the AP trampoline nor `smp_ap_main` ever loads an IDT. `smp_ap_main`
(`smp.c:127`) does `cpuid`, claims a slot, and drops straight into its work loop:

```c
/* kernel/smp.c:147-155 */
u32 seen = 0;
for (;;) {
    while (ap_slots[slot].seq == seen) smp_pause();
    seen = ap_slots[slot].seq;
    ap_slots[slot].fn(ap_slots[slot].ctx, ap_slots[slot].y0, ap_slots[slot].y1);
    __sync_synchronize();
    ap_slots[slot].done = seen;
}
```

`ap_slots[slot].fn` is a framebuffer band function — `grad_band`, `shadow_band` and
friends, i.e. real pixel code operating on caller-supplied geometry.

**With no IDT loaded, any fault on an AP has no handler.** A page fault, #GP or
divide-by-zero inside a band job escalates fault → double fault → triple fault, which
on x86 is an immediate processor reset.

That is not "the desktop looks wrong" or "the machine hangs with a message." It is
**the laptop reboots instantly, with no output**, on the one test machine that
`kernel/docs/thinkpad-first-boot.md` records as having **no serial port — the screen is
the only diagnostic.** A triple fault clears the screen on the way out.

This repo has lost the 64-bit boot once before inside `setup_idt()` with all three gates
green. This is the same neighbourhood.

## 2. The barrier has no timeout

```c
/* kernel/smp.c:177 */
while (ap_slots[i].done != ap_slots[i].seq) smp_pause();
```

Unbounded. If an AP resets, wedges, or never claims its slot, the BSP spins here
forever. The comment above it is right that the barrier is *not optional* — `fb_present`
must not blit a row a core is still writing — but "not optional" and "unbounded" are
different choices, and only one of them is recoverable.

Note `smp.c:184-185` *does* bound a different loop (`long spins = 100000000L`), so the
bounded-wait idiom already exists in the file and just is not used here.

## 3. `kernel.zl:1874`'s safety note is doing more work than it can bear

The line quoted in `.ultra/STATE.md` — APs *"park immediately: nothing is lock-protected
yet"* — is describing why APs are **safe while parked**. It is not evidence that they
are safe once given work, and giving them `smp_band_dispatch` is exactly giving them
work. The document's own caveat says this needs *"to be true rather than assumed."*
It is currently assumed.

---

## Correction to the reachability claim, which cuts the other way

`.ultra/STATE.md` §4 and `gpu-next.md` both state that `smp_go()` is *"reachable solely
from the old text shell's `*` key, so `smp_start()` never runs on a desktop boot."*

**That is false.** Measured:

```
$ grep -n '"smp"\|"cores"' kernel/term.c
kernel/term.c:190:    { "smp",      42 }, { "cores",    42 },
kernel/term.c:204:    { "smp",      42 }, { "cores",    42 },

$ grep -n 'smp_go' kernel/kernel.zl freestanding/runtime_kernel.c
kernel/kernel.zl:1886:        smn = smp_go()
freestanding/runtime_kernel.c:1402:    if (streq(name,"smp_go")) return zl_num((double)smp_start());
```

`smp` and `cores` are typeable commands, registered **twice** in `term.c`'s table, and
`kernel.zl:1886` calls `smp_go()` directly. So the APs can be started today by typing a
word — which means finding 1 above is reachable *now*, not only after someone wires the
dispatcher into the boot path.

What remains true is the narrower claim: `fb_par_hook` is only installed when
`bands > 1` (`smp.c:265`), so the *band dispatcher* is not active on a default desktop
boot. Starting the APs and dispatching work to them are two different switches, and only
the second one is off.

---

## What this changes

**Do not re-rank SMP bands as "cheap."** The 1.78× is real and the code is written; the
honest cost is 1.78× *plus an IDT for the APs plus a bounded barrier*, not 1.78× for one
call in `kernel.zl`'s boot path.

Ordered, smallest first:

1. **Bound the barrier.** Copy the `spins` idiom already at `smp.c:184`. On expiry,
   `kfatal` with the slot number — a named failure beats a hang, and this is ~5 lines.
2. **Give the APs an IDT** before dispatching any band work. Even a minimal one that
   `kfatal`s with the vector and the faulting core converts an instant silent reboot
   into a message on the screen — which on this machine is the only diagnostic there is.
3. **Then** turn bands on, and measure.

Steps 1 and 2 are the difference between a bug in band code costing a debugging session
and costing a reboot with no evidence. Neither is large. Both should land before the
switch, not after the first mysterious reset.

## Related, from the same lens — not hand-verified

- `hosttest/fbbench.c:701` — `smp_band_dispatch` is executed by nothing any gate runs;
  `fbbench` measures a *different* dispatcher. So the 1.78× figure and the shipping code
  path are not the same code.
- `probe-smp.py:137` — the pixel-identity proof crops the shell client rect only, so
  `grad_band` and `shadow_band` pixels are never compared.

If both hold, the 1.78× is measured on something other than what would ship, and the
correctness proof does not cover the bands. **Check these two before trusting the
number** — this repo has already been burned once by benchmarking against a stand-in
rather than the shipping code, which `.ultra/STATE.md` records under "What this session
got wrong."
