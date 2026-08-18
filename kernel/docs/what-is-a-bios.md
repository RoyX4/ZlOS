# What "writing your own BIOS" actually means

Written 2026-08-18 in answer to "explain what making your own BIOS means".
Numbers marked *measured* were read off this machine; everything else is labelled.

## The analogy

A BIOS is the janitor who unlocks the building before anyone arrives.

At power-on there is no RAM, no disk, no screen, no keyboard — there is a CPU and
a flash chip soldered next to it, and that is the entire computer. Nothing is
initialised because initialising things is *software*, and no software has run
yet. The janitor's job is to wake the RAM, turn on the lights, unlock the doors,
write down where everything is, and then hand the keys to whoever shows up first.

An OS never sees the dark building. It is handed a lit one. **Writing your own
BIOS means being the janitor** — starting from the dark building.

## What the janitor actually does, in order

1. **Reset vector.** The CPU comes out of reset in 16-bit real mode fetching
   from physical `0xFFFFFFF0`, which the chipset maps to the top of the flash.
   No RAM, no stack, no interrupts. You get about 16 bytes before you must jump.
2. **Cache-as-RAM.** You need a stack, but DRAM does not work yet. So you put
   the CPU's own cache into no-eviction mode and use it as scratch memory. This
   is why the very first code cannot be ordinary C — nothing that spills to a
   stack you do not have yet.
3. **DRAM training.** Bring up the memory controller: read the SPD EEPROM off
   the DIMM, then calibrate DDR read/write timing per byte-lane by sweeping
   delays until the eye opens. This is the hard part. See the wall below.
4. **Chipset and PCIe.** Enumerate the bus, assign BARs, set up SMM, run option
   ROMs (the video ROM is what puts a picture on screen before an OS exists).
5. **Write down where everything is.** The E820 memory map, ACPI tables, SMBIOS
   tables. The OS believes these; nothing verifies them.
6. **Hand off.** Legacy: load sector 0 into `0x7C00` and jump, leaving `int 0x13`
   (disk) / `int 0x10` (video) / `int 0x15` (memory map) live as a service API.
   UEFI: mount the FAT ESP, load `BOOTX64.EFI`, and expose boot services until it
   calls `ExitBootServices`.

Steps 4 and 5 are things you have already written for zlOS. Step 3 is the wall.

## Two different walls — do not confuse them

**Wall 1 — the DRAM training blob.** On modern Intel the memory reference code is
not published. coreboot does not write it either; it calls into Intel's FSP-M
binary. AMD's equivalent is AGESA. This is *nobody outside Intel has the timing
tables* — not a skill problem, not a language problem. On older platforms
(Sandy/Ivy Bridge era) people have reverse-engineered it, and coreboot has native
raminit for those. On Comet Lake, no.

**Wall 2 — Boot Guard, and this one is fused into the silicon.** Before your
reset-vector code runs at all, CPU microcode loads a signed Authenticated Code
Module from flash, and the ACM checks the initial boot block against a key hash
burned into the chipset's field-programmable fuses. Wrong signature, the machine
powers off. You do not get to run.

*Measured on this box (ThinkPad X1 Carbon Gen 8, firmware N2WET51W 1.41):*

```
$ sudo modprobe msr
$ sudo dd if=/dev/cpu/0/msr bs=8 count=1 skip=$((0x13A)) iflag=skip_bytes | od -An -tx8
 000000030000007d
```

`0x7D` = `0111_1101`. Bit 0 set is no-eviction mode (cache-as-RAM, step 2 above,
already used by Lenovo's firmware this boot). **Bit 6 set is verified boot.**
The bit layout of `BOOTGUARD_SACM_INFO` is from memory, not from a datasheet on
this machine — treat the decode as high-confidence, not verified. The raw value
is measured.

Practical translation: you can write a BIOS for this laptop, but it will not boot
on this laptop. That is a fuse, not a bug, and no amount of code fixes it.

**Wall 3 does not exist.** "zl can't do it" is not one of the walls. zl emits
native x86-64 and already runs freestanding — the only real gap is that the
reset vector runs in *16-bit real mode*, and no zl backend emits 16-bit code.
That is a codegen feature nobody has written, roughly the same size of job as any
other backend mode, not a property of the language. And it only covers the first
few hundred instructions; everything after the jump to protected mode is ordinary
zl. Same distinction as always: *nobody wrote it* ≠ *it cannot be written*.

## Bounded versions, cheapest first

Real sizes for scale, *measured on this box*:

| Thing | Size |
|---|---|
| `raw_boot.bin` — our bootloader | **512 bytes** |
| `/usr/share/seabios/bios.bin` — the BIOS QEMU boots zlOS with today | **128 KiB** |
| `/usr/share/ovmf/OVMF.fd` — UEFI firmware | **4 MiB** |
| `BOOTX64.EFI` — our UEFI app | **978 KiB** |

**(a) A QEMU BIOS — genuinely doable, a weekend to a fortnight.** QEMU hands you
working RAM, so step 3 vanishes and there is no signing. You need the reset
vector, PAM shadowing, an E820 built from `fw_cfg`, `int 0x13` over ATA PIO,
`int 0x10` via the VGA option ROM, and the jump to `0x7C00`. *Estimate, not
measured: 1,500–3,000 lines of asm and C.* This is a real BIOS by any honest
definition; it is just standing on a simulated motherboard.

**(b) Bare-metal firmware on an unfused board.** Pick hardware without Boot Guard
(older ThinkPads, some AMD, most SBCs) and the same code runs on real silicon.
Still needs raminit — either native for a platform where it has been reversed, or
FSP if you accept the blob.

**(c) Firmware for this laptop.** Blocked at wall 2. Not a coding task.

**(d) A payload, not firmware.** coreboot's split is worth stealing: firmware does
raminit and hands off to a *payload* that does everything else. zlOS is already
shaped like a payload.

## What this has to do with zlOS right now

The interesting part is that **you have already been doing the janitor's job for
two months.** *Measured line counts:*

```
pci.c    239     apic.c   397     nvme.c   483
cpu.c    359     xhci.c  1720     intel.c 5084
```

PCI enumeration, interrupt controller setup, disk, USB, display — every one of
those is step 4 work. A BIOS does them so an OS does not have to. You do them
anyway, because zlOS does not want to be handed a lit building.

The clearest case is the display. `raw_boot.asm` still calls `int 0x10` for VBE
and `int 0x13` to read the kernel off disk (lines 88, 111, 128 …). Those are
BIOS services — the only two places zlOS still asks the firmware for anything.
Everything needed to delete both already exists in the tree: `nvme.c` reads
disks, `intel.c` sets modes.

And **the cold-start modeset is exactly this problem in miniature.** Every
"firmware already programmed this, we read it back and matched" line in
`HANDOFF.md` is a place where Lenovo's janitor did the work and `intel.c`
inherited it. Getting a mode up from a cold, unprogrammed pipe *is* writing the
video-init portion of a BIOS. That job is in scope, is bounded, and is already
underway.

So: making your own BIOS is not a new project to start. It is a name for the
part of this one already being done — minus DRAM training, which is a blob, and
minus running on this specific laptop, which is a fuse.

## "Could I write the whole BIOS in zl, years from now?"

**Yes.** Here is the short version.

A CPU is a machine that eats numbers. Instructions are numbers. There is no
field in an instruction saying which language produced it, and no step where the
chip asks. C is not blessed — it is just the language that happened to be in the
room first. zl already produces x86-64 machine code, so a zl BIOS is not a new
kind of thing; it is the same numbers arriving from a different direction.

Nothing stands between zl and firmware *as a language*. What stands in the way
is three separate things, and only one of them is even about zl:

**1. The first few instructions run in 16-bit mode.** The CPU wakes up in a
1978 costume and stays in it until you tell it to change. zl has no 16-bit
backend. That is a *thing nobody has written*, not a limit — and it is small,
because you only stay in that mode long enough to jump out of it. Until it
exists, ~20 lines of assembly cover it and everything after the jump is zl. This
is the only item on the list that a zl feature fixes.

**2. Waking the RAM needs numbers Intel does not publish.** Not a language
problem — C cannot do it either. coreboot, written in C by hundreds of people,
calls a closed Intel blob for exactly this step. On older chips the numbers were
reverse-engineered and are public, so on that hardware zl can do the whole thing.
On Comet Lake, nobody outside Intel can, in any language.

**3. This laptop checks a signature before it runs anything.** Measured above:
Boot Guard verified boot is fused on. It rejects unsigned firmware no matter what
wrote it. Assembly, C, Rust, zl — all rejected identically. A different machine
without the fuse has no such problem.

So, honestly:

| Target | All-zl BIOS? |
|---|---|
| QEMU | **Yes** — everything except ~20 lines of real-mode asm, and zero once zl has a 16-bit backend |
| Old real hardware, no Boot Guard, public raminit | **Yes**, same caveat |
| A modern Intel board, DRAM training included | **No — and neither can anyone else.** Not a zl limit |
| This X1 Carbon | **No.** A fuse, not a language |

The claim "you cannot write a BIOS in your own language" is false. The true
claim is much narrower: *one step of one kind of BIOS depends on a trade secret,
and one particular laptop refuses to run anything it did not sign.*

## If you want to check any of this yourself

```bash
sudo dd if=/dev/cpu/0/msr bs=8 count=1 skip=$((0x13A)) iflag=skip_bytes | od -An -tx8
```

```bash
sudo flashrom -p internal
```

The second one reports flash size and which regions the host is allowed to write.
I did not run it — it pokes the SPI controller, and other sessions were live on
this box. It is read-only, and it is the direct answer to "could I even flash it".
