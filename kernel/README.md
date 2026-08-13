# A kernel, written in zl

`kernel.zl` runs on bare metal. No operating system underneath it, no libc,
no syscalls. `print()` talks to COM1 by polling the UART; `poke8` writes to
VGA text memory at `0xB8000` and the characters appear on screen.

```bash
./build.sh                 # kernel.zl -> kernel.elf
./run.sh                   # boot it in QEMU, serial on your terminal
./verify.sh                # headless boot, diff serial against golden.txt
```

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

The serial driver is **written in zl** - `inb`/`outb` are builtins, the
polling loop and the command dispatch are ordinary zl functions. That is
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
