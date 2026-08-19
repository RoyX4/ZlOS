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

That is not a gate failing to catch something hard. It is a gate reporting a green
result about files it never opened — which is exactly the class
`docs/GUARDS-THAT-DID-NOT-GUARD.md` exists to enumerate, and it is currently not in it.

## Corroboration from the same sweep

The agent found two other guards in the same family, both worth re-deriving before
acting on:

- **`kernel/wguard.sh:45`** — the guard *for this bug class* reportedly never reads the
  flag line it claims to guard, so it cannot detect any single `-Werror=` flag being
  removed. The agent notes `tools/hazard-scan.sh` check 1 (lines 51-75) does this
  correctly for one flag, by compiling a probe with the real parsed `CF` and failing if
  it is accepted — so a correct pattern already exists in the tree to copy.
- **`kernel/idt.c:244`** — `struct interrupt_frame` is reportedly 20 bytes instead of 40
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
