# The CI gate for this repo's #1 bug class scans 6 of ~50 translation units

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366` · measured, not inferred**

Found by fleet bug-agent `llp64` during a whole-tree sweep for the LLP64 pointer
truncation class. Re-measured here. **Confirmed.**

`tools/hazard-scan.sh` is the check that feeds `.github/workflows/gates.yml` and reports
*"no new truncation sites."* It compiles a file list extracted from `kernel/buildefi.sh`
— **by reading that script as text.**

---

## The extraction

```sh
# tools/hazard-scan.sh:86
files=$(sed -n '/^for f in/,/do$/p' kernel/buildefi.sh | tr ' \\' '\n\n' | grep '\.c$')
```

and the line it reads:

```sh
# kernel/buildefi.sh:53,57
CORE=$(grep -vE '^[[:space:]]*(#|$)' SOURCES | tr '\n' ' ')
for f in efi.c _genefi.c ../freestanding/runtime_kernel.c gdt64.c $CORE; do
```

`sed` sees the literal text `$CORE`. It is never expanded, so `grep '\.c$'` keeps only
the four hardcoded filenames.

## What it actually scans, run just now

```
$ sed -n '/^for f in/,/do$/p' kernel/buildefi.sh | tr ' \\' '\n\n' | grep '\.c$'
efi.c
_genefi.c
../freestanding/runtime_kernel.c
gdt64.c
idt.c
apic.c
```

**Six.** And two of those six — `idt.c` and `apic.c` — are not from the `for` line at
all; they appear because the `EXTRA=()` case statement a few lines below mentions them
by name for `-mgeneral-regs-only`, and the `sed` range happens to reach it.

Against the real build:

```
$ grep -vcE '^[[:space:]]*(#|$)' kernel/SOURCES
46
```

46 files from `SOURCES` plus the 4 literals ≈ **50 translation units**. The gate looks
at **6**, and it looks at them partly by accident.

## What is therefore unscanned

Everything in `SOURCES`, which is to say the entire kernel: `fb.c`, `intel.c`, `xhci.c`,
`smp.c`, `console.c`, `http.c`, `nvme.c`, `virtio_gpu.c`, `browser.c`, `gpuring.c`,
`gpucursor.c`, and 37 others.

Add a truncating cast to any of them and `hazard-scan.sh` counts the same number as
before. `.github/workflows/gates.yml` compares `head_n == base_n`, finds no change, and
prints **"no new truncation sites."**

### Corrected by adversarial verification — the class is not unguarded, only uncounted

**An earlier version of this file implied the bug class itself is unprotected in those 44
files. That is too strong**, and a fleet verifier caught it. `kernel/buildefi.sh:40-44`
carries the four flags with no `-w`, and they apply to **every** translation unit in the
real build:

```sh
CF="-target x86_64-unknown-windows -ffreestanding -fno-stack-protector \
    -fshort-wchar -mno-red-zone -O2 -DZL_64 -DZL_EFI -I.. \
    -Wno-excessive-regsave \
    -Werror=shift-count-overflow -Werror=void-pointer-to-int-cast \
    -Werror=pointer-to-int-cast -Werror=int-to-pointer-cast"
```

So a new **pointer↔integer** cast in `fb.c` or `xhci.c` fails the build even though the
gate never counts it. The gate is blind; the compiler is not.

**What neither catches is the shape that actually bit.** `intel.c:435`'s
`return (u32)mmio` narrows an address already held as an *integer* (`uptr`) to another
integer. That is not a pointer-to-int cast, so none of the four `-Werror=` flags
diagnose it — and `hazard-scan.sh` never opens `intel.c` to count it either.

The honest statement is therefore: **the gate is uncounted across 44 files, the compiler
covers the pointer-cast half of the class in all of them, and the integer-narrowing half
is covered by nothing anywhere.** Fixing the extractor is still worth doing; adding
`-Wconversion` (or a targeted check for narrowing casts of `uptr`) is what would have
caught `intel.c:435`.

That is not a gate failing to catch something hard. It is a gate reporting a green
result about files it never opened — which is exactly the class
`docs/GUARDS-THAT-DID-NOT-GUARD.md` exists to enumerate, and it is currently not in it.

## Corroboration from the same sweep

The agent found two other guards in the same family, both worth re-deriving before
acting on:

- **`kernel/tools/checks/wguard.sh:45`** — the guard *for this bug class* reportedly never reads the
  flag line it claims to guard, so it cannot detect any single `-Werror=` flag being
  removed. The agent notes `tools/hazard-scan.sh` check 1 (lines 51-75) does this
  correctly for one flag, by compiling a probe with the real parsed `CF` and failing if
  it is accepted — so a correct pattern already exists in the tree to copy.
- **`kernel/src/arch/x86/idt.c:244`** — `struct interrupt_frame` is reportedly 20 bytes instead of 40
  in the EFI build, the **third** instance of the documented struct-field bug, in the
  same file as the two that were fixed. Latent only because every handler discards its
  argument (`(void)f;`).

**Neither has been re-measured by hand.** Treat as leads.

## The fix

Make the extractor expand the list rather than read it:

```sh
files=$( cd kernel && \
         printf '%s\n' efi.c _genefi.c ../freestanding/runtime_kernel.c gdt64.c \
                       $(grep -vE '^[[:space:]]*(#|$)' SOURCES) )
```

Better, and in keeping with how `SOURCES` is already described in `CLAUDE.md` as *"one
file, read by all four build"* paths: have `buildefi.sh` emit its own TU list under a
flag (`buildefi.sh --list`) and have `hazard-scan.sh` call it. Then the gate and the
build cannot drift, which is the actual defect — the gate re-implements the build's file
list by scraping it.

**Then watch it go red.** Plant a truncating cast in a `SOURCES` file, confirm the count
rises, remove it. Per this repo's own rule: *a gate that can only pass is worse than no
gate, and any new gate must be shown failing on a case it is supposed to fail on before
it is trusted.* That rule was written here after `land-gate.sh` piped builds through
`| tail`; this is the same lesson in a different shell idiom.
