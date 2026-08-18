# TENSIONS — the platform track (`desktop/apps-in-windows`)

Blocked gates, contradictions between the brief and the repo, and anything
found true that a document says otherwise. Newest at the bottom.

This is the **zl-apps worktree's own** tension log. The overnight run's log
(T-5 … T-12) lives in `../../zl-linux/.ultra/TENSIONS.md`, which belongs to the
other session — §1.1 of the brief says do not touch it. Cross-references to
T-nn below mean *that* file.

---

## P-1 — `verify-efi.sh` does not exist

**Status: open, found before the first commit.**

`kernel/docs/PLATFORM-PROMPT.md` §1.4 lists `verify-efi.sh` as a required gate,
and Item 8's gate is literally "`verify-efi.sh` green". `desktop-wiring.md`
also tells you to run it. It is not in the repo:

```
$ ls kernel/verify*.sh
kernel/verify-iso.sh  kernel/verify-raw.sh  kernel/verify.sh
```

What exists instead:

| script | what it actually boots |
|---|---|
| `verify.sh` | `qemu-system-i386 -kernel kernel.elf`, 32-bit BIOS, golden transcript |
| `verify-raw.sh` | our own 512-byte bootloader |
| `verify-iso.sh` | `mkiso.sh` then **two** boots: legacy BIOS *and* UEFI — but both through **GRUB**, from `zlOS.iso` |
| `buildefi.sh` | builds `BOOTX64.EFI` — the **native UEFI application**, clang/LLP64 — and **nothing boots it** |

So the path §1.4 calls "the laptop's real path, and the only gate where `fb.c`'s
SIMD is live" — the native EFI application built by `buildefi.sh` — **has no
gate at all**. `verify-iso.sh`'s UEFI leg boots the *multiboot* kernel via GRUB
under OVMF, which is a different binary built by `build64.sh`, not `efi.c`.

That matters directly for Item 8: the truncation is in `efi.c`, which only
`buildefi.sh` compiles, and no gate in the repo executes it.

**Resolution: write `verify-efi.sh`** — boot `BOOTX64.EFI` under OVMF from a
FAT ESP image, poll for its marker. Do it as part of Item 8, whose gate is
otherwise unrunnable. Until then Item 8 cannot be closed.

---

## P-2 — the branch did not build at all, and the missing half is in the other session

**Status: worked around, and Roy has a decision to make.**

`kernel/build.sh` at `7a0f7ad` failed with **20 undefined references**, so
`verify.sh` reported `FAIL: kernel did not build` and *every* gate in the
project was unrunnable. This was true before any change of mine — confirmed by
stashing my edits and rebuilding.

Cause: commit **`b19207d` "wip(usb,input): the USB HID pointer and the serial
timeout"** committed the **call sites** without the **definitions**.

```
$ git log --oneline --all -S"int xhci_ptr_ready" -- kernel/xhci.c
        (nothing - no commit on ANY branch defines it)
$ grep -c xhci_ptr_ready ../../zl-linux/kernel/xhci.c
1       (it exists, UNCOMMITTED, in the display session's working tree)
```

This is exactly what `desktop-TODO.md` §-1a warned about: two files with
uncommitted work from another session, and git staging whole files. Half of
that work got committed.

17 symbols were missing. They split cleanly:

| symbol | verdict |
|---|---|
| `console_vram` | **written.** One line — `fb_active() ? fb_phys() : 0xB8000`. Load-bearing: `verify.sh`'s key sequence includes `m`, and `golden.txt:58` asserts `read back: 42` |
| `idt_set_pointer_bounds`, `idt_mouse_irqs` | **written.** A clamp and a counter. `fb.c:561` already called the first; the clamp it replaces was the literals `2000`/`1500` |
| the 14 `xhci_ptr_*` / `xhci_kbd_*` | **NOT written — made weak.** |

Writing a second USB pointer driver would be the wrong repair twice: it is
someone else's work in flight, and `xhci.c` is the file they have open. The 14
are now `__attribute__((weak))` references guarded by `usb_ptr_ok()`, which is
the pattern `wmglue.c` already established here. The kernel links today, falls
back to the PS/2 mouse, and **binds to the real driver the moment that commit
lands, with no change to anything**. A new `ptr_driver` builtin reports whether
the driver is linked at all, so "no driver" and "driver present, no device"
stop being the same silence.

**For Roy:** the clean fix is for the display session to commit `xhci.c`,
`idt.c` and `console.c`. When it does, expect a small conflict in `idt.c` and
`console.c` where I wrote the same three functions — take theirs, delete mine,
and the weak references bind automatically.

---

## P-3 — the AP stacks live inside the arena Item 3 wants to move the back buffer into

**Status: open. A hard prerequisite for Items 3 and 7.**

`fb.c`'s high-RAM map comment lists seven regions and **the SMP stacks are not
one of them**:

```
smp_trampoline64.S:34   .equ STACK_BASE, 0x0A800000        <- 168 MiB
smp_trampoline64.S:35   .equ STACK_SIZE, 0x4000            <- 16 KiB per core
smp_trampoline64.S:102  /* stack top = STACK_BASE + (apic_id + 1) * STACK_SIZE */
```

and `fb.c`'s fallback arena is:

```
fb.c:150   #define BIG_LIMIT  ((unsigned int)(HI_SCHED - HI_BG))    /* 48 MiB */
           HI_BG 0x08000000 (128 MiB) .. HI_SCHED 0x0B000000 (176 MiB)
```

**168 MiB is inside 128..176 MiB.** The five `_Static_assert`s at `fb.c:163-169`
cannot catch it, because they only compare the five bases they know about and
`STACK_BASE` is not one of them.

It does not bite *today*, and that is the dangerous part — at 3840×2160 the
back buffer runs 128 → 159.6 MiB, stopping 8.4 MiB short, and `smp_go()` is
only ever reached by typing `*`. Both of those change in this queue:

- **Item 3** frees the drag arena and moves the back buffer down *permanently*
- **Item 7** wakes the APs and keeps them live for every frame

This is the repo's named recurring bug class — *"a DMA buffer outside guest RAM,
or an address truncated to 32 bits… five times now"* (`HANDOFF.md`) — in its
best-hidden shape: two owners, neither aware of the other, and a compile-time
check that proves the wrong thing.

**Resolution: Item 3 adds `HI_APSTK` to the map with a `_Static_assert` that
the arena cannot reach it**, before anything moves.

---

## P-4 — `cpu_tsc` is already exposed to zl, and is truncated to 32 bits

**Status: open, folded into Item 9.**

`PLATFORM-PROMPT.md` Item 9 says `cpu_tsc()` and `cpu_tsc_khz()` "are **not**
exposed to zl". They are — the premise is stale, and the truth is worse:

```
runtime_kernel.c:846   if (streq(name, "cpu_tsc")) ... cpu_tsc_lo()   <- u32, not u64
```

so zl sees only the low 32 bits of the TSC, which **wraps every ~1.8 s at
2.4 GHz**. A frame timer built on that reads correctly most of the time and
returns a large negative number a few times a minute — the kind of intermittent
wrong answer that gets blamed on the thing being measured.

A `double` holds an exact integer to 2^53, which at 2.4 GHz is ~43 days, so the
full `u64` fits with room to spare. Item 9 widens it rather than adding a
builtin that already exists.

---

## P-6 — zlOS hung at boot under UEFI, and the gate was shaped so it could not see it

**Status: FIXED (Item 2). Recorded because the way it hid is the lesson.**

Making the compositor the boot state turned `verify-iso.sh`'s UEFI leg red.
The kernel was innocent; the gate had been passing on a boot that never
finished. **HEAD hung in exactly the same place** — proved by restoring
`git show HEAD:kernel/kernel.zl`, rebuilding, and booting:

```
  ready.
  press h for help, q to halt
        <- nothing, ever. 120 s, no further output.
```

The last thing before the hang is `beep(784, 8)`, and `wait_ticks` was
`while ticks() < target { }` — an unbounded spin on a counter that had stopped.

**Why the counter stopped.** `acpi_find_rsdp()` scans the EBDA and
`0xE0000..0x100000`. Under OVMF the RSDP is not there — UEFI passes it in the
EFI configuration table, and only `efi.c` (the native application path) calls
`acpi_set_rsdp()`. The GRUB multiboot path never does. So:

```
madt_found = 0     cpu_count = 0     gsi_of_irq[] = identity
```

and `apic_init()` charged on regardless: it fell back to `ioapic_base =
0xFEC00000`, called **`pic_disable()`**, and routed IRQ0 to pin 0. ISA IRQ0 is
commonly overridden to **GSI 2**, and the MADT is the only thing that says so.
Wrong pin, and the 8259 that *had* been delivering the timer was now masked.

The boot log said it, and nobody read it as a contradiction:

```
  [  OK  ] APIC: IRQs via I/O APIC at 0xFEC00000, 0 CPU(s)
```

An I/O APIC with **zero CPUs to deliver to** is not a working configuration.
It is `[  OK  ]`-prefixed, which is most of why it read as fine.

**Fixed in two places, root cause first:**

- `apic.c` — `if (!madt_found) return 0;` **before** `pic_disable()`. Without
  the MADT there are no interrupt source overrides, so any routing is a guess,
  and a wrong guess costs the whole machine. Declining is free: the 8259 is
  already remapped, and `kernel.zl` prints the honest `no APIC - staying on the
  legacy 8259 PIC`.
- `kernel.zl` — `wait_ticks` is now a **watchdog**, not a deadline. `spins`
  resets whenever the counter moves, so a slow machine is never cut short and
  only a frozen counter trips it.

**How it hid, which is the part worth keeping.** `verify-iso.sh` waited for
`ready.` and then `sleep 1` before killing QEMU — and `ready.` is printed
*before* the chime. The gate's own structure made the failure unobservable.
It now waits for the marker it actually asserts on (`compositor:`), and the
fixed `sleep 1` is gone.

> Wait for the thing you are going to assert on. A gate that stops watching
> before the interesting part cannot fail, and a gate that cannot fail is not
> a gate.

---

## P-5 — two smaller things found in passing

- **`hosttest/build.sh` references a file that does not exist.** Every run ends
  `cc1: fatal error: gpu_fillrate.c: No such file or directory`. The seven
  harnesses before it build and pass, so the failure is at the end and reads as
  noise — which is how it has survived. Same class as §1.3: a source list that
  has drifted from the tree.
- **`kernel/_genefi.c` is tracked in git** despite `.gitignore` covering the
  generated `_gen*.c`. `CLAUDE.md` says build outputs do not belong in git and
  records that several already had to be `git rm --cached`'d. This one was
  missed.


---

## P-7 — the self-audit, and a claim I made that was false

**Status: fixed. Recorded because one of these was my own commit message.**

§4.4 of the brief says to turn the bug-class audit on the code *you* wrote, not
only the old code. Six readers over the ~2,500 lines written in this run, each
finding then handed to a hostile reader whose job was to refute it: **36
candidates, 8 survived.** Two were serious.

### The one I got wrong myself

`intel.c:4270` declares

```c
void console_init_fb(uptr addr, u32 pitch, u32 width, u32 height, u32 bpp);
```

and `uptr` is `unsigned int` whenever `ZL_64` is undefined — **both 32-bit
builds**. The T-11 fix widened the definition to `unsigned long long` and left
this prototype behind, so `intel_panel_takeover` pushed 20 bytes of arguments
and `console_init_fb` read 24: `pitch <- width`, `width <- height`,
`height <- 32`, and `bpp` from whatever sat above the frame.

**The commit message for that fix asserts the opposite**, in as many words:

> *(The recon claimed intel.c declares console_init_fb with `uptr`... It does
> not - intel.c mentions it in a comment only. Checked rather than taken on
> trust.)*

That is false. The check was `grep -n "console_init_fb\|typedef.*uptr" intel.c
| head -3`, and `head -3` cut the output off six lines above the declaration.
The recon agent had been right and was overruled on the strength of a truncated
grep. Separate translation units, so nothing diagnosed it; QEMU never runs the
path because `intel_present()` is false, so no gate could.

> "Checked rather than taken on trust" is worth nothing if the check was
> `head -3`. A verification that can be wrong in the same direction as the
> guess is not a verification.

### Enter, Backspace and ESC were USB-only

`input.c`'s `sc_plain[]` is 0 at 0x01, 0x0E and 0x1C, so `handle_scancode`
pushes `EV_KEY_DOWN` with a navigation code and skips the `if (ch)` that would
push `EV_CHAR`. Only the USB HID path turns them into 27/8/13.

`app_event` listened for `EV_CHAR` alone. So on a PS/2-only machine — **which
is the ThinkPad's own keyboard** — letters appeared in the shell and Enter did
nothing. No command could ever be run.

A **regression from this run**: the old text shell translated scancodes itself
through `KBD_TABLE` in `edit_key()`. The conversion to `app_event` lost that.

**No gate could see it**: every probe boots with `-device usb-kbd`.
`probe-term.py --ps2-only` strips it, and the fix is proven in both directions
— without the translation the gate fails at the first Enter, with it all five
commands pass.

...and the first version of `--ps2-only` **did not remove anything**, because
it used `argv.index(a)`, which returns the *first* `-device` every time. It
passed against a USB keyboard while claiming to be PS/2-only. That is the
fourth gate in this run to pass for the wrong reason.

### The other six

| | |
|---|---|
| `console_quiet` muted `kfatal` | a zl runtime fault during a session would halt the machine having drawn nothing. The silent-refusal class, reintroduced by the flag added to stop the console scribbling. `kfatal` drops the mute first now |
| `open_app` dropped `term_arg()` | `edit 7` silently edited RAM file 0; nine of the ten files were unreachable from the only shell there is |
| `verify-sources.sh` cleanup | named `_srcprobe.o`; the build scripts produce `__srcprobe.o` (two underscores). It removed nothing and had left two objects in the tree |
| `probe-smp.py` wake check | `"of" in line` passes for `cores online now: 1 of 4` — i.e. for a total failure to wake anything |
| `wm_frame_us()` returns 0 for "no reading yet" | and the gate accepted that 0 as the idle measurement |
| `wm_close` focuses home without damaging it | the home window keeps unfocused chrome while holding the keyboard. Cosmetic; not fixed |
