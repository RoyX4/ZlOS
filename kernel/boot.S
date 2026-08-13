/* boot.S - multiboot entry.
 *
 * design_kernel.md picks UEFI, and its reason is specific: pe_min.zl already
 * hand-builds a PE32+ file, so on the NATIVEGEN path zl writes the boot image
 * itself. On the C-backend path that argument does not apply - gcc emits ELF,
 * and QEMU loads a multiboot ELF directly with -kernel, no GRUB, no OVMF, no
 * image writer. Same trophy ("it boots"), far less scaffolding. The UEFI route
 * stays correct for kernelgen.c when that exists.
 */
/* bit 0 align modules, bit 1 memory map, bit 2 ASK FOR A VIDEO MODE.
 * Bit 2 is what makes this boot usefully under UEFI: firmware there has no
 * VGA text mode, so unless we request a framebuffer GRUB reports "no
 * suitable video mode found" and the screen stays black. */
.set MB_MAGIC,    0x1BADB002
.set MB_FLAGS,    0x00000007
.set MB_CHECKSUM, -(MB_MAGIC + MB_FLAGS)

.section .multiboot, "a"
.align 4
.long MB_MAGIC
.long MB_FLAGS
.long MB_CHECKSUM
.long 0, 0, 0, 0, 0                   /* a.out fields, unused for ELF */
.long 0                               /* mode_type 0 = linear framebuffer */
.long 0                               /* width  0 = no preference */
.long 0                               /* height 0 = no preference */
.long 0                               /* depth  0 = no preference
   Asking for an exact 1024x768x32 fails on firmware that offers something
   else - OVMF hands out 1280x800 - and GRUB then reports "no suitable
   video mode found" and leaves the screen black. 0 means "any linear
   framebuffer you have", which is what we actually want. */

.section .bss
.align 16
stack_bottom:
.skip 16384                            /* 16 KiB - design_kernel K2: keep frames small */
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp               /* nobody sets up a stack for you */
    call serial_init                   /* serial BEFORE anything else (§7) */
    push %ebx                          /* the multiboot info GRUB handed us */
    call console_init                  /* picks framebuffer or VGA text */
    add $4, %esp
    call main                          /* main() is the zl program */
    call kernel_done
.hang:
    cli
    hlt
    jmp .hang
.size _start, . - _start
