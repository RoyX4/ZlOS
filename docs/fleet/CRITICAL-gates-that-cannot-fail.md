# Eight gates in this tree cannot fail, or skip silently

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366`**

`docs/GUARDS-THAT-DID-NOT-GUARD.md` documents five checks that reported green while
checking nothing. The fleet's `fake-gates` and `llp64` sweeps found **eight more**. Four are hand-verified
here; three remain leads.

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
| `kernel/tools/checks/check-himap.sh:107` | cannot see the address it was written to catch if it is spelled with one fewer leading zero |

A fifth lead, `kernel/tools/checks/wguard.sh:45`, **was** on this list and has since been verified —
see the section at the end of this file. It was the sharpest of them, because `CLAUDE.md`
presents `wguard.sh` as the reason the `-w` bug class is believed closed.

---

## Separately, and worse — a CRITICAL from the `dma-bounds` sweep

### `xhci_ram_ok()` zeroes a live DCBAA entry, and a zl builtin reaches it

```c
/* kernel/src/drivers/input/xhci.c:411-423 */
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

`kernel/src/core/arena.c:198` shows the tree knows the distinction: *"This version saves and
restores what was there first. `xhci_ram_ok()` does [not]"*.

The reachability is the problem:

```
kernel/src/drivers/input/xhci.c:450                 if (!xhci_ram_ok()) return 0;      ← during init, safe
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


---

## ✓ VERIFIED — `wguard.sh` cannot see the flag line it exists to guard

Promoted from a lead after wave 3's refutation stage re-derived it and I checked it by
hand. `CLAUDE.md:100-103` says *"`kernel/tools/checks/wguard.sh` is the check, and it runs all three
directions… Run it before touching that flag line."* **The flag line is the one input it
cannot observe.**

```
$ grep -n 'buildefi\|CF=' kernel/tools/checks/wguard.sh
2:# wguard.sh - prove buildefi.sh's four -Werror= flags actually bite.
4:# WHY THIS EXISTS. From 2026-08-18 to 2026-08-19 buildefi.sh carried
78:    echo "   buildefi.sh's -w could be restored; re-read this script's header."
87:    # buildefi.sh, on purpose.
```

**Four hits, all comments.** The script never greps, sources or parses `buildefi.sh`. It
defines its own:

```sh
# kernel/tools/checks/wguard.sh:45-48
GUARD="-Werror=shift-count-overflow -Werror=void-pointer-to-int-cast \
       -Werror=pointer-to-int-cast  -Werror=int-to-pointer-cast"
BASE="-target x86_64-unknown-windows -ffreestanding -fno-stack-protector \
      -fshort-wchar -mno-red-zone -O2"
```

So **restoring `-w` to `buildefi.sh` leaves all three directions green.** The script
proves that *clang* honours four flags, not that *the build* passes them.

### The threshold is the second half

```sh
# kernel/tools/checks/wguard.sh:58-60
n=$(grep -c 'error:' "$TMP/a.log" || true)
if [ "$n" -ge 4 ]; then
    say "A. guard catches a planted truncation" "ok ($n errors)"
```

A count over a *mixed* probe cannot say which flag is live. The planted file yields 5
errors under all four flags, and one line contributes two of them — so removing
`-Werror=shift-count-overflow` leaves 4 and still prints **ok**. That is the flag whose
absence let clang compile an IDT gate store to a bare `ret`.

*(The refuter's own correction, kept: this is not true of all four —
`-Werror=pointer-to-int-cast` is detected. "Cannot detect any single flag" overstates it.)*

### And it never runs

```
$ grep -rn "wguard" gates/ .github/ tools/
  (no output)
```

Not in `land-gate.sh`, not in the workflows. It is a manual check that four documents
point at.

### Fix — the right pattern is already in the tree

`tools/hazard-scan.sh:31-34` has `efi_cflags()`, which parses `CF=` straight out of
`buildefi.sh` *"so this test can never drift from the real build."* Have `wguard.sh` call
that instead of defining its own `GUARD`/`BASE`, and replace the `-ge 4` count with four
compiles, each carrying exactly one flag, each required to fail.

**The tree contains both the right pattern and the wrong one, and `CLAUDE.md` points
people at the wrong one.**
