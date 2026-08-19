/* raw_entry.S - the kernel entry point when OUR bootloader loaded it.
 *
 * boot.S is the entry GRUB uses: it carries a multiboot header and assumes
 * GRUB already put the CPU in 32-bit protected mode and passed a multiboot
 * info pointer. This entry is for raw_boot.asm - our own boot sector - which
 * enters protected mode itself and hands over with no multiboot info at all.
 *
 * It must be the very first thing in the flat binary, because the loader
 * jumps straight to the load address. The linker script link-raw.ld puts
 * this section first at 1 MiB, which is where raw_boot.asm copies us.
 */
.code32
.section .text.rawentry, "ax"
.global _rawstart
_rawstart:
    mov $0x00C00000, %esp       /* a stack in high memory (12 MiB), growing down. It
                                   was put at 6 MiB when the kernel still lived under
                                   the 640 KiB line and left only ~55 KiB for a stack
                                   - not enough for the framebuffer compositor, which
                                   nests deep and needs ~256 KiB.

                                   It moved to 12 MiB when raw_boot.asm's CHUNKS went
                                   to 192: the loader now fills 1 MiB .. 7 MiB, so
                                   6 MiB is INSIDE the region being loaded. The
                                   kernel image currently reaches __bss_end 0x2E15C0
                                   (2.88 MiB), so this leaves it about 9 MiB to grow
                                   down into. Check that gap before moving either.

                                   raw_boot.asm:pm_entry sets the same value before
                                   jumping here, and arena.c's RAW_STACK_TOP asserts
                                   against it. Three copies; all three or none. */

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

    /* Our bootloader sets a VBE mode itself and parks the details at 0x8300:
     * addr(4) pitch(2) width(2) height(2) bpp(1). A zero address means it could
     * not get one, and we fall back to the VGA text console. There is no
     * multiboot info on this path either way - this handoff is entirely ours. */
    movl 0x8300, %eax
    testl %eax, %eax
    jz 1f
    movzbl 0x830A, %ecx         /* bpp    */
    push %ecx
    movzwl 0x8308, %ecx         /* height */
    push %ecx
    movzwl 0x8306, %ecx         /* width  */
    push %ecx
    movzwl 0x8304, %ecx         /* pitch  */
    push %ecx
    /* THE ADDRESS IS EIGHT BYTES NOW (T-11): console_init_fb takes an
     * `unsigned long long` so that a UEFI framebuffer above 4 GiB survives the
     * trip. cdecl puts the low dword at the lower address, so push the high
     * half first. Our own bootloader only ever reports a 32-bit VBE base, so
     * the high half is always zero here - but the CALL has to match the
     * prototype or the four arguments after it land one slot too low, and
     * nothing in C would have caught that. 5 args -> 6 dwords -> add $24. */
    xor %edx, %edx
    push %edx                   /* addr, high dword */
    push %eax                   /* addr, low dword  */
    call console_init_fb
    add $24, %esp
    jmp 2f
1:  xor %ebx, %ebx              /* no framebuffer -> VGA text console */
    push %ebx
    call console_init
    add $4, %esp
2:

    call main                   /* main() is the zl program */
    call kernel_done
1:  hlt
    jmp 1b
