/* raw_entry.S - the kernel entry point when OUR bootloader loaded it.
 *
 * boot.S is the entry GRUB uses: it carries a multiboot header and assumes
 * GRUB already put the CPU in 32-bit protected mode and passed a multiboot
 * info pointer. This entry is for raw_boot.asm - our own boot sector - which
 * enters protected mode itself and hands over with no multiboot info at all.
 *
 * It must be the very first thing in the flat binary, because the loader
 * jumps straight to the load address. The linker script link-raw.ld puts
 * this section first at 0x10000.
 */
.code32
.section .text.rawentry, "ax"
.global _rawstart
_rawstart:
    mov $0x90000, %esp          /* a stack below 640 KiB, above the kernel */

    /* Zero the .bss ourselves. GRUB-loaded kernels get a zeroed .bss from
     * the ELF loader; a flat binary loaded off disk does not, so the font
     * table, console state and the keyboard map would start as garbage. */
    xor %eax, %eax
    mov $__bss_start, %edi
    mov $__bss_end, %ecx
    sub %edi, %ecx
    cld
    rep stosb

    call serial_init            /* serial before anything else */

    xor %ebx, %ebx              /* no multiboot info -> console_init picks VGA text */
    push %ebx
    call console_init
    add $4, %esp

    call main                   /* main() is the zl program */
    call kernel_done
1:  hlt
    jmp 1b
