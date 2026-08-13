/* boot.S - multiboot entry.
 *
 * design_kernel.md picks UEFI, and its reason is specific: pe_min.zl already
 * hand-builds a PE32+ file, so on the NATIVEGEN path zl writes the boot image
 * itself. On the C-backend path that argument does not apply - gcc emits ELF,
 * and QEMU loads a multiboot ELF directly with -kernel, no GRUB, no OVMF, no
 * image writer. Same trophy ("it boots"), far less scaffolding. The UEFI route
 * stays correct for kernelgen.c when that exists.
 */
.set MB_MAGIC,    0x1BADB002
.set MB_FLAGS,    0x00000003          /* align modules + provide memory map */
.set MB_CHECKSUM, -(MB_MAGIC + MB_FLAGS)

.section .multiboot, "a"
.align 4
.long MB_MAGIC
.long MB_FLAGS
.long MB_CHECKSUM

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
    push %ebx                          /* multiboot info pointer, for later */
    call serial_init                   /* serial BEFORE anything else (§7) */
    call main                          /* main() is the zl program */
    call kernel_done
.hang:
    cli
    hlt
    jmp .hang
.size _start, . - _start
