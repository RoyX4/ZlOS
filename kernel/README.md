# zlOS - a kernel written in zl

```
 zlOS   a kernel written in zl                        no OS  no libc
 [  OK  ] multiboot handoff, 32-bit protected mode
 [  OK  ] stack established, 16 KiB
 [  OK  ] COM1 initialised, 115200 8N1
 [  OK  ] VGA text console, 80x25
 [  OK  ] zl runtime, kernel subset
 [ INFO ] no interrupts - the shell polls
 [ INFO ] no heap, no filesystem, no scheduler

 ready.
 zl> 20f
 6765
 h help    q halt                    ready
```

`kernel.zl` runs on bare metal. No operating system underneath it, no libc,
no syscalls. `print()` talks to COM1 by polling the UART; `poke8` writes to
VGA text memory at `0xB8000` and the characters appear on screen.

```bash
./build.sh          # kernel.zl -> kernel.elf
./run.sh            # boot in QEMU: window = screen, this terminal = keyboard
./run.sh --term     # no window, everything in the terminal
./mkiso.sh          # -> zlOS.iso, bootable by GRUB on real hardware
./verify.sh         # headless boot, drive the shell, diff against golden.txt
```

## Running it on real hardware

```bash
./mkiso.sh
qemu-system-i386 -cdrom zlOS.iso              # test the real boot path first
sudo dd if=zlOS.iso of=/dev/sdX bs=4M status=progress && sync
```

Two things decide whether your machine will boot it:

- **It is a BIOS/legacy image.** Most machines made after ~2020 are UEFI-only
  with no CSM and will refuse it. A UEFI image needs the PE32+ route
  `design_kernel.md` §3 specifies, which belongs to `kernelgen.c`.
- **You need the PS/2 keyboard**, which is why it exists. There is no serial
  port on a modern laptop, so without it you could see the screen and never
  type. USB keyboards usually work through the firmware's PS/2 emulation,
  but that is the firmware's favour, not ours - a real USB stack is a much
  later floor.

Building the ISO needs `grub-pc-bin` (the i386-pc modules). With only
`grub-efi-*` installed, `grub-mkrescue` silently produces an image that
fails with "could not read the boot disk".

## The console

`vga.c` drives the 80x25 text grid at `0xB8000` directly - cursor via the
CRT controller at ports 0x3D4/0x3D5, and scrolling constrained to rows
1..23 so the title and status bars do not scroll away with the log.

zl drives it through builtins: `cls`, `color`, `bar`, `at`, `row`,
`goto_row`, and `put` (print with no trailing newline, which is what lets
`[ OK ] message` be assembled out of separately coloured pieces).

Everything printed also goes to COM1, so `verify.sh` can capture a
transcript headlessly while a human sees the screen.

## How it is built

```
kernel.zl ─► ../compile ─► out.c ─► gcc -m32 -ffreestanding -nostdlib ─► kernel.elf
                                  + runtime_kernel.c  (-DZL_KERNEL_SERIAL)
                                  + boot.S            (multiboot header, stack, entry)
                                  + support.c         (outb/inb, UART bring-up)
                                  + libgcc            (__divdi3/__moddi3 only)
```

`nm -u kernel.elf` reports **0 undefined symbols**: no libc, no OS.

## Two deviations from `docs/design/design_kernel.md`, and why

**1. Multiboot, not UEFI.** That doc picks UEFI, and its reason is specific:
`pe_min.zl` already hand-builds a PE32+ file, so on the *nativegen* path zl
writes the boot image itself. On the C-backend path that argument does not
apply — gcc emits ELF, and QEMU loads a multiboot ELF directly with
`-kernel`: no GRUB, no OVMF, no image writer. Same trophy, far less
scaffolding. UEFI stays correct for `kernelgen.c` when that exists.

**2. 32-bit, not long mode.** Long mode needs a GDT, page tables and a
trampoline before a single line of zl runs. 32-bit protected mode is where
multiboot already drops you. The 64-bit move is a known, separate step.

## The subset

`runtime_kernel.c` implements `runtime.h` for the kernel subset only:
numbers with **i64 semantics**, the operators, `print`, `exit`, and raw
memory. Lists, string values and concatenation are a **named fault**, not a
link error — `design_kernel.md` is explicit that "a `zl_alloc` reaching a
kernel binary is a fault with no diagnostic".

One deliberate divergence from the interpreter: `100 / 7` is **14** here and
`14.285714` there. The kernel subset is integer-only on purpose
(`design_kernel.md` §2 — a kernel doing floating-point division on a
descriptor is how you get a wrong GDT entry).

## The one seam

`zl_putc` in `runtime_kernel.c` is the only contact with the outside world:
a `write` syscall on Linux, COM1 under `-DZL_KERNEL_SERIAL`. Retargeting is
one function. That is what "swappable later" has to mean in practice.

## The shell

`kernel.zl` polls COM1 and runs commands you type:

```
zl> h        help
zl> 20f      fib(20)      -> 6765
zl> 10s      sum of squares to 10 -> 385
zl> m        write a byte to VGA and read it back
zl> c        clear the screen
zl> q        halt
```

The serial AND keyboard drivers are **written in zl** - `inb`/`outb` are
builtins, everything else is ordinary zl functions.

The PS/2 driver reads port 0x64 for "a byte is waiting" and 0x60 for the
scancode, ignores anything with bit 7 set (a key coming up), and maps
scancodes through a **table built in RAM with `poke8` and read with
`peek8`**. The kernel subset has no lists, and a 90-branch if/else chain
would be unreadable - so the raw memory primitives are the data structure.

Input is taken from whichever source has a byte, so the same build works
over serial (piped tests, a terminal) and on a real keyboard. That is
MASTER_PLAN Floor 6 level 2's definition: *"boot a VM, get a prompt, type a
command, my language runs it."*

### One quirk, and it is not the kernel's fault

QEMU can hand the guest the very first serial byte before it starts
executing, so a *piped* first character is lost - unrecoverable from inside
the kernel. A human at a terminal cannot type before the machine boots, so
this never shows up interactively. `verify.sh` sends a throwaway `.` first
for exactly this reason.

## What this is not

It owns the machine, prints, and takes input. It has no interrupts (the
shell polls), no timer, no memory manager, no scheduler, no filesystem, and
cannot load a program from disk. `design_kernel.md` draws the line:
**W6 owns the machine and can print; W7 does something with it.**
