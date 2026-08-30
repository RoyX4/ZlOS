# The platform run — 2026-08-18

What one session did against [`PLATFORM-PROMPT.md`](../archive/prompts/PLATFORM-PROMPT.md), what
it found that the brief did not predict, and what is left. Written because chat
is gone next session and the repo is not.

**Branch:** `desktop/apps-in-windows` · **10 commits**, one per queue item ·
every item gated before it was committed.

```
verify.sh          32-bit BIOS, transcript byte-identical to golden.txt
verify-raw.sh      our own bootloader - text leg AND framebuffer leg
verify-efi.sh      NEW - the native UEFI application. Nothing booted it before
verify-iso.sh      BIOS and UEFI through GRUB, both reach the compositor
verify-sources.sh  NEW - one source list, proved in both directions
wmtest 0 · inputtest 0 · tritest 0 · fbbench hashes unchanged
probe-term · probe-apps · probe-snake · probe-smp · probe-frame · probe-edit
```

---

## 1. The four things nobody predicted

These are the ones worth reading. Each is written up in
`../../.ultra/TENSIONS.md`.

### P-2 — the branch did not build

`kernel/build.sh` failed at `7a0f7ad` with **20 undefined references**, so
`verify.sh` reported "kernel did not build" and *every* gate in the project was
unrunnable. Not caused by anything in this run — confirmed by stashing and
rebuilding.

Commit `b19207d` ("wip(usb,input)") landed the **call sites** of a USB pointer
driver without the **definitions**. `git log --all -S"int xhci_ptr_ready"` finds
no commit on any branch; they exist, uncommitted, in the display session's
working tree.

Three of the seventeen missing symbols were trivially writable and one of those
was load-bearing (`console_vram`, which `golden.txt` asserts through the `m`
command). The other fourteen are **weak references** now — the `wmglue.c`
pattern — so the kernel links, falls back to the PS/2 mouse, and binds to the
real driver the moment that commit lands.

> **For whoever owns that work:** commit `xhci.c`, `idt.c`, `console.c`. Expect
> a small conflict in the latter two where the same three functions were
> written here; take yours, delete these, and the weak references bind on their
> own.

### P-6 — zlOS had been hanging at boot under UEFI

Making the compositor the boot state turned `verify-iso.sh`'s UEFI leg red. The
kernel was innocent: **HEAD hung in exactly the same place**, proved by
restoring `git show HEAD:kernel/kernel.zl` and booting it.

Under OVMF the RSDP is in the EFI configuration table, not the legacy
`0xE0000..0x100000` window `acpi_find_rsdp` scans — and only `efi.c` calls
`acpi_set_rsdp`, never the GRUB path. So `madt_found` was 0, there were no
interrupt source overrides, and `apic_init` charged on anyway: fell back to
`0xFEC00000`, called **`pic_disable()`**, and routed IRQ0 to pin 0. ISA IRQ0 is
commonly overridden to GSI 2. Wrong pin, and the 8259 that *had* been delivering
the timer was masked. No timer at all, and `wait_ticks` was an unbounded spin.

The boot log said so and nobody read it as a contradiction:

```
[  OK  ] APIC: IRQs via I/O APIC at 0xFEC00000, 0 CPU(s)
```

An I/O APIC with **zero CPUs to deliver to** is not a working configuration.
It is `[  OK  ]`-prefixed, which is most of why it read as fine.

**The gate could not have caught it.** `verify-iso.sh` waited for `ready.` and
slept one second before killing QEMU — and `ready.` is printed *before* the
chime that hung.

> Wait for the thing you are going to assert on. A gate that stops watching
> before the interesting part cannot fail, and a gate that cannot fail is not a
> gate.

### P-3 — the SMP stacks were inside a framebuffer buffer

`smp_trampoline{,64}.S` put each core's 16 KiB at `STACK_BASE 0x0A800000` —
168 MiB — and `fb.c`'s high-RAM map did not list it. `sp_buf`, the drag sprite
buffer, spanned 160..176 MiB. **Dragging a window with the other cores awake
wrote a window bitmap through their stacks.** It never fired only because
`smp_go()` is reached by typing `*` and nobody did both at once.

The five `_Static_assert`s could not have caught it: they compared the five
bases they knew about, and `STACK_BASE` was not one of them. A compile-time
check that proves the wrong thing is worse than none, because it reads as
coverage.

### P-1 — `verify-efi.sh` did not exist

Three documents told you to run it. What existed was `verify.sh` (32-bit BIOS),
`verify-raw.sh` (our bootloader), `verify-iso.sh` (BIOS and UEFI, but both
through **GRUB**), and `buildefi.sh`, which builds `BOOTX64.EFI` and which
**nothing booted**.

That gap is why T-11 survived: `verify-iso.sh`'s UEFI leg boots the *multiboot*
kernel under OVMF, and `efi.c` is not in that binary. The file containing the
truncation — and the whole firmware-handoff path the ThinkPad actually takes —
was compiled by the build system and executed by nothing.

---

## 2. Three gates that passed for the wrong reason

All three were caught by tightening the test, not by the code changing.

**probe-snake** dragged a window LEFT across the snake and sampled the whole
snake window: 3.72% changed, green. The dragged window ended at x=942 against a
snake spanning 242..974 — it was measuring the monitor's own edge. It now drags
*away* and **asserts** the dragged window can never enter the snake's rectangle
rather than assuming it.

**probe-apps** measured 0.00% for three apps that were running perfectly,
because the windows cascaded and four of the five were invisible under the
fifth. They tile now.

**The frame timer** reported a confident, stable **9997 µs**. `wm_frame()`
returns immediately unless the 100 Hz tick has moved, so the gap between frames
is pinned at 10 ms *by construction*: it was measuring the PIT frequency
wearing a frame timer's clothes. Idle and busy differed by 4% and the gate
passed. It times the body of a frame now, over frames that actually painted.

> A test whose predicate is weaker than the property you care about will
> eventually pass while the property is false. This project already knew that
> — `desktop-overnight-run.md` §2 says it about `probe-mouse.py` — and it
> happened three more times in one run.

---

## 3. The numbers

**SMP band rendering is 1.64×, not 4×.** Full account with four runs tabulated,
including the one where four bands came out *slower* than two, in
[`desktop-smp-bands.md`](../desktop-smp-bands.md). The short version: the draw
list is zl and the zl runtime is not reentrant, so what is parallel is each
large *primitive* internally, which caps the whole scene at ~2.6× before any
overhead.

**The first attempt was 0.85× — slower** — because a thread per band per call
cost 27 `pthread_create`/`join` pairs per frame. Kept in the doc, per
`DECISIONS.md` #25.

**Pixels are unchanged everywhere it matters:**

```
fbbench scene hash, identical at 1/2/3/4 bands, all three modes
   1920x1200  8473499efb49abb1
   2560x1440  81c4be85c58763e7
   3840x2160  e735d8737eeff842
probe-smp.py  0 of 961608 pixels differ across 4 real cores
```

**4K stopped being a degraded mode.** Deleting the sticker drag freed 128..168
MiB, so `back` covers 3840×2160 (31.6 MiB) outright. There is no "back OFF"
path left for any mode this kernel can be handed.

---

## 4. What is left

**Blocked on the display session:** the USB pointer driver (P-2). Until it
lands, `mouse_x`/`mouse_y` are the PS/2 path, `probe-drag.py` needs
`--no-tablet`, and the Pointer window says `usb ptr absent` — which is the
correct answer, printed rather than guessed.

**Not attempted:** C5 (the start menu as a modal window) and F4 (fractional UI
scale). Neither is in the queue.

**The 4× that band rendering did not get** needs a display list — record the
C-level draw calls once, replay per band. That removes the zl reentrancy
blocker outright and makes the whole scene parallel, text included. It is the
single highest-leverage thing left in the renderer.

**A latent bug found and not fixed**, because it is outside this queue and
nothing currently trips it: `FS_DATA`'s ten 8 KiB slots (`0x02011000`) overlap
`LINE_BUF` (`0x02020000`) and `HIST_BUF` (`0x02021000`) — slot 7 contains one
and slot 8 the other. Harmless today only because `read_line` is dead code.
Same class as P-3.

---

Queue: [`PLATFORM-PROMPT.md`](../archive/prompts/PLATFORM-PROMPT.md) · Bands:
[`desktop-smp-bands.md`](../desktop-smp-bands.md) · Previous run:
[`desktop-overnight-run.md`](desktop-overnight-run.md) · Tasks:
[`desktop-TODO.md`](../desktop-TODO.md) · Blocks: `../../../.ultra/TENSIONS.md`
