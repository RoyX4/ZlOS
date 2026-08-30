# Ground-truth corrections — checked by hand, before the fleet synthesis ran

**2026-08-19 · tree `3f00366` · worktree `zl-linux-fleet`, branch `fleet/audit-2026-08-19`**

These were verified with direct greps against the tree *while* the 100-agent fleet
was in flight, specifically so the synthesis stage would not rank already-finished
work at the top of its board. Every claim below carries the command that establishes
it.

---

## The headline: `kernel/docs/driver-build-order.md` Wave 0 is **already landed**

That document is the output of a 34-agent hardware survey and it is excellent. It is
also **stale in the pessimistic direction** — it describes a pre-Wave-0 tree, and Wave 0
landed the same day it was written.

```
d6a0533  2026-08-18  docs: driver build order, from a 34-agent survey of the real hardware
06b1a0f  2026-08-18  feat(pci,apic,cpu): wave 0 of the driver build order - the unblock layer
```

Three specific claims in that document are **no longer true**:

### 1. "`acpi_find_table()` external linkage — 1 word"

The document calls deleting `static` from `apic.c:171` *"the single highest
leverage-per-line item in the entire survey."* It is already deleted.

```
$ grep -n "acpi_find_table" kernel/*.c kernel/*.h
kernel/apic.c:177:uptr acpi_find_table(const char *sig)
kernel/apic.c:219:    uptr madt = acpi_find_table("APIC");
```

No `static`. Line 177, not 171. **Done.**

### 2. "`write_msr()` … does not exist anywhere in the tree"

It exists.

```
$ grep -rn "write_msr" kernel/*.c kernel/*.h
kernel/cpu.c:65:void write_msr(u32 msr, u64 val)
kernel/cpu.c:67:    __asm__ volatile("wrmsr" : : "c"(msr), "a"((u32)val), "d"((u32)(val >> 32)));
```

(`apic.c:68` also has a private `static wrmsr`, which is a separate, older copy —
that duplication is real and is worth folding, but it is a tidy-up, not a blocker.)
**Done.**

### 3. "`pci.c`: capability-list walk + PMCSR D3→D0 — ~60 lines"

Landed, with the measurement in the commit message and the reasoning in the source.

```
kernel/pci.c:249:  int pci_find_cap(int bus, int dev, int fn, int id)
kernel/pci.c:283:  int pci_power_on(int bus, int dev, int fn)
kernel/pci.c:301:  int pci_power_state(int bus, int dev, int fn)
kernel/pci.c:313:  /* Power BEFORE command bits. Enabling memory decode on a device in D3 */
```

`pci_enable()` now wakes before it enables. **Done.**

---

## The blur/HID collision is fixed, and the fix is better than the one proposed

`driver-build-order.md` cites a live collision:

> `fb.c:135 HI_BLUR 0x0C000000` … `i2c_hid.c:104 HID_BUF 0x0C900000` — the touchpad's
> report buffer sits 9 MiB inside the blur arena.

Both line numbers are stale and the collision is gone. There is now a single
`kernel/memmap.h` holding every high-memory base:

```
kernel/memmap.h:74:#define HI_SCHED  0x0B000000UL   /* sched.c      - stacks, counters    */
kernel/memmap.h:75:#define HI_HID    0x0B800000UL   /* i2c_hid.c    - HID over I2C buffers */
kernel/memmap.h:76:#define HI_GPU    0x0BC00000UL   /* gpuring.c    - the GPU command ring  */
kernel/memmap.h:77:#define HI_BLUR   0x0C000000UL   /* fb.c         - the cached-blur arena */
kernel/memmap.h:78:#define HI_NVME   0x0D000000UL   /* nvme.c       - admin + I/O queues    */
```

`HI_HID` is `0x0B800000` — **below** the blur arena, which spans `0x0C000000`–`0x0D000000`.
And `i2c_hid.c:121` now derives `HID_BUF` from `HI_HID` rather than hardcoding an address.

The span asserts the document asked for also exist (`kernel/fb.c:203-215`), and one of
them catches a *different* collision the document did not find — the back buffer growing
into the AP stacks:

```c
#define BACK_LIMIT ((unsigned int)(HI_APSTK - HI_BACK))  /* 40 MiB, not 48 */
_Static_assert(3840UL * 2160UL * 4UL <= (unsigned long)BACK_LIMIT,
               "the back buffer no longer covers 3840x2160");
_Static_assert((unsigned long)HI_BLUR + BLUR_LIMIT <= HI_NVME,
               "the blur arena overruns nvme's queues");
#define AP_STACK_BASE 0x0A800000UL   /* == smp_trampoline{,64}.S STACK_BASE */
_Static_assert(AP_STACK_BASE == HI_APSTK,
               "HI_APSTK and smp_trampoline's STACK_BASE have drifted apart");
```

The source comment is explicit that `HI_SCHED` would have given a 48 MiB back buffer
spanning 128–176 MiB with `STACK_BASE` at 168 MiB *inside it*, and that only the
`desktop/apps-in-windows` track noticed.

**What is still open here** is narrower than the document implies, and worth keeping:
`memmap.h` is a table of bases, not a reservation table with runtime spans, and
`kernel/check-memmap.sh` still iterates a hardcoded list rather than discovering
constants from the C. A new base added tomorrow is still uncovered.

---

## Why this matters more than the individual corrections

This is the same failure mode `docs/STATE-OF-THE-PROJECT.md` was written to fix,
running in the opposite direction. That document warns that reading a planning doc
"tells you what somebody wanted, never what is true." The usual damage is a doc that
claims something works when it does not. This is the mirror image: **a doc that claims
something is broken when it has been fixed.**

It is the more expensive direction, because a wrong "not supported" never gets tested —
nobody tries what they are told is absent. Wave 0 is three landed commits that a reader
of `driver-build-order.md` would spend a day re-implementing.

**`kernel/docs/driver-build-order.md` needs the same audit banner the other twenty-one
planning documents already carry**, pointing at `docs/STATE-OF-THE-PROJECT.md`. It is
currently the most actionable-looking document in the repo and the top of its queue is
done.

---

## `intel.c` orphans: the number is **101**, not 63 — and there is no encapsulation at all

`kernel/docs/display-roadmap.md`'s audit banner says *"63 of 301 exported `intel_*` functions
have no call site."* Re-measured by hand on the current tree, the figure is **101 of 303**.

Method, so it can be re-run:

```bash
# every non-static intel_* definition
grep -nP '^(?!static)[A-Za-z_][\w \*]*\bintel_\w+\s*\(' kernel/intel.c \
  | grep -oP '\bintel_\w+(?=\s*\()' | sort -u > exported.txt

# those with no reference anywhere outside intel.c and outside generated files
while read f; do
  n=$(grep -rlw -- "$f" kernel/ freestanding/ 2>/dev/null \
      | grep -vE 'kernel/intel\.c$|out\.c$|_gen.*\.c$|\.o$' | wc -l)
  [ "$n" -eq 0 ] && echo "$f"
done < exported.txt
```

A first, cruder version of this counted 253 — it did not exclude functions whose only
callers are *inside* `intel.c`, which for a driver this size is normal and correct. The
253 figure is wrong and is recorded here only so nobody re-derives it and believes it.

### The finding underneath the number

```
$ grep -cP '^static\s+[\w \*]*\bintel_\w+\s*\(' kernel/intel.c
0
$ grep -cP '^(?!static)[A-Za-z_][\w \*]*\bintel_\w+\s*\(' kernel/intel.c
308
```

**Not one function in `intel.c` is `static`.** All 308 definitions are exported from a
5,182-line file. So the file has no interface/implementation boundary at all, and the
orphan analysis above is the *only* way to tell which functions are meant to be called
from outside and which are internal steps. That is why the number is soft: some of the
101 are genuinely unfinished display phases, and some are internal helpers that were
never marked static.

Marking the internal ones `static` is a mechanical change that would make the orphan
count mean something — and would let the compiler start reporting genuinely dead code,
which it currently cannot.

### What the 101 actually are

Grouped by prefix, they map almost exactly onto display-roadmap phases 3–7:

| group | n | group | n | group | n |
|---|---|---|---|---|---|
| `psr_*` | 9 | `plane_*` | 5 | `gamma_*` | 4 |
| `lt_*` (link training) | 7 | `panel_*` | 5 | `drrs_*` | 4 |
| `hpd_*` (hotplug) | 7 | `wm_*` (watermarks) | 4 | `backlight_*` | 4 |
| `port_*` | 6 | `pipe_*` | 4 | `dpcd_*` | 3 |
| | | `modeset_*` | 4 | `ddi_*` | 3 |

This is consistent with the roadmap's own honest summary — *"one port, one pipe, one
panel, one mode, polled"* — and it quantifies it: hotplug, PSR, DRRS, backlight and
multi-port are all **written and unreachable**, not missing.

---

## Standing method note

Every fleet agent in this run was briefed with these claims *stated as fact*, taken from
the repo's own documentation, and explicitly instructed to verify rather than accept
them. That instruction is why the briefs are usable at all. Where an agent's finding and
this file disagree, **this file was checked by hand against the tree and the agent's was
not** — re-run the grep before believing either.
