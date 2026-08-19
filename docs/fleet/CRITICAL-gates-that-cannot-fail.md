# Seven gates in this tree cannot fail, or skip silently

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366`**

`docs/GUARDS-THAT-DID-NOT-GUARD.md` documents five checks that reported green while
checking nothing. The fleet's `fake-gates` bug-class sweep found **seven more**. Three
are hand-verified below; four are recorded as leads.

The repo's own rule, from `.ultra/TENSIONS.md` T-2: *"a gate that can only pass is worse
than no gate. Any new gate must be shown failing on a case it is supposed to fail on
before it is trusted."*

---

## VERIFIED — `gates/land-gate.sh:137` silently skips every boot gate it cannot execute

```sh
# gates/land-gate.sh:137-141
for g in mkiso.sh verify.sh verify-iso.sh verify-efi.sh verify-raw.sh verify-disk.sh verify-clock.sh; do
  [ -x "$WT/kernel/$g" ] || continue
  until guard; do sleep 30; done
  run "boot: $g" "$WT/kernel" "./$g"
done
```

`|| continue` with **no message and no counter**. If a gate script is missing, renamed,
or loses its executable bit, it is skipped in total silence and `FAIL` is never
incremented — so the run still prints `GATE GREEN`.

`verify-efi.sh` is in that list. It is the gate `CLAUDE.md` describes as the one that
closed the hole where *"three gates were green while the 64-bit build was dead"* — the
only gate that boots zlOS as its own UEFI application, i.e. the path the ThinkPad
actually takes. A `chmod -x` away from vanishing without a word.

**Fix:** count and report skips.

```sh
if [ ! -x "$WT/kernel/$g" ]; then
    SKIP=$((SKIP+1)); echo ">>> SKIP boot: $g (not executable)"; continue
fi
```
and print `SKIP` in the summary line. A skipped gate is not a passing gate, and the
summary currently cannot tell you which it was.

## VERIFIED — `run_tests.sh:192` drops the entire kernel-boot section with no output

```sh
# run_tests.sh:190-198
# The kernel is a separate gate (design_kernel.md §10) but a broken build
# should surface here too. Skipped cleanly if QEMU is not installed.
if command -v qemu-system-i386 >/dev/null; then
    echo "== kernel: boots in QEMU and matches its golden transcript =="
    ...
fi
```

There is no `else`. The comment says *"Skipped cleanly"*, and it is — so cleanly that
nothing is printed at all. On any CI runner or container without `qemu-system-i386`, the
whole kernel gate disappears and the suite reports success.

The intent is defensible; the implementation makes "QEMU absent" and "kernel fine"
indistinguishable in the output. **Fix:** add
`else echo "  SKIP  kernel gate (qemu-system-i386 not found)"`.

## VERIFIED — `tools/hazard-scan.sh:86` scans 6 of ~50 translation units

Full write-up: [`CRITICAL-ci-truncation-gate-is-blind.md`](CRITICAL-ci-truncation-gate-is-blind.md).
The file list is scraped out of `buildefi.sh` **as text**, so `$CORE` never expands and
46 of the kernel's source files are never examined by the gate that exists for this
repo's #1 bug class.

Two independent fleet agents found this one — the `llp64` class sweep and the
`fake-gates` class sweep — which is why it is ranked first among the three.

---

## Leads from the same sweep — reported, NOT hand-verified

| file:line | claim |
|---|---|
| `tools/hazard-scan.sh:62` | checks 1 and 2 report through `warn()`, which never sets `fail` — so the EFI guard checks are advisory while appearing to gate |
| `tools/engine-parity.sh:141` | treats a **total engine build failure** as not-a-failure |
| `kernel/check-himap.sh:107` | cannot see the address it was written to catch if it is spelled with one fewer leading zero |
| `kernel/wguard.sh:45` | never reads the flag line it claims to guard, so it cannot detect any single `-Werror=` flag being removed |

`wguard.sh` is the sharpest of the four if it holds, because `CLAUDE.md` presents it as
the check that *"runs all three directions"* and is the stated reason the `-w` class is
believed closed.

---

## Separately, and worse — a CRITICAL from the `dma-bounds` sweep

### `xhci_ram_ok()` zeroes a live DCBAA entry, and a zl builtin reaches it

```c
/* kernel/xhci.c:411-423 */
int xhci_ram_ok(void)
{
    volatile u32 *lo = (volatile u32 *)XMEM_DCBAA;
    volatile u32 *hi = (volatile u32 *)(XMEM_DATA + 0x800);
    *lo = 0xA5A5F00Du;
    *hi = 0x5A5A0FF0u;
    if (*lo != 0xA5A5F00Du) return 0;
    if (*hi != 0x5A5A0FF0u) return 0;
    *lo = 0; *hi = 0;                 /* ← destroys whatever was there */
    return 1;
}
```

It writes a probe pattern into `XMEM_DCBAA` — the Device Context Base Address Array —
and then **zeroes it**, restoring nothing. DCBAA entry 0 is where the xHCI spec puts the
**scratchpad buffer array pointer**.

`kernel/arena.c:198` shows the tree knows the distinction: *"This version saves and
restores what was there first. `xhci_ram_ok()` does [not]"*.

The reachability is the problem:

```
kernel/xhci.c:450                 if (!xhci_ram_ok()) return 0;      ← during init, safe
freestanding/runtime_kernel.c:1061  if (streq(name,"usb_ram")) …     ← zl builtin, ANY TIME
```

At `xhci.c:450` the DCBAA is not yet populated, so the probe is harmless — which is
presumably why it was written this way. But `usb_ram` is a **zl builtin**, callable from
the shell after the controller is up, and at that point entry 0 holds the live
scratchpad pointer. Zeroing it points the controller's scratchpad array at physical
address 0.

This lands in an area the project has already been burned in: scratchpad handling is one
of the four USB bugs recorded as *only appearing outside QEMU*.

**Fix:** save and restore, exactly as `arena.c:198` describes, or probe an address that
is not the DCBAA. The second is better — a liveness probe should not write to a
structure the hardware owns.
