/* link-raw.ld - layout for the kernel when OUR bootloader loads it.
 *
 * The kernel is linked at 1 MiB and run there. It used to be linked at
 * 0x10000 - low memory the boot sector can reach with plain BIOS disk reads,
 * no copy dance - but that capped the whole kernel at the 640 KiB line. So
 * raw_boot.asm now reads each chunk into a bounce buffer at 0x10000 and
 * copies it up to KERNEL_DEST (1 MiB); this address must match that one. The
 * raw entry (raw_entry.S) is placed FIRST so the loader can jump straight to
 * the load address. __bss_start/__bss_end let the entry zero the BSS by hand.
 */
ENTRY(_rawstart)
PHDRS
{
    text PT_LOAD FLAGS(5);   /* read + execute */
    ro   PT_LOAD FLAGS(4);   /* read only      */
    data PT_LOAD FLAGS(6);   /* read + write   */
}
SECTIONS
{
    . = 0x100000;
    .text : {
        *(.text.rawentry)     /* _rawstart at exactly 0x100000 */
        *(.text*)
    } :text
    .rodata : { *(.rodata*) } :ro
    .data   : { *(.data*)   } :data
    . = ALIGN(16);
    __bss_start = .;
    .bss    : { *(COMMON) *(.bss*) } :data
    . = ALIGN(16);
    __bss_end = .;

    /* See link.ld. This path matters MOST of the three: raw_entry.S puts the
     * stack top at 12 MiB and it grows DOWN toward this symbol, so the image
     * and the stack are walking at each other.
     *
     * There is a THIRD constraint on this path that these two asserts do not
     * express, and it is the tighter one: raw_boot.asm loads only CHUNKS x
     * 32 KiB (192 x 32 KiB = 6 MiB, so 1 MiB .. 7 MiB), and anything past that
     * is never read off the disk at all. .bss is exempt - it is NOBITS, it is
     * not in the image, and raw_entry.S zeroes it - so the number that must fit
     * under 7 MiB is __bss_start, not __kernel_end. mkdisk.sh checks it, on the
     * objcopy'd binary, where the size is a fact rather than a symbol. */
    __kernel_end = .;
    ASSERT(__kernel_end <= 0x00C00000,
           "the kernel image has grown into the raw-boot stack at 12 MiB (raw_entry.S)")
    ASSERT(__kernel_end <= 0x00E00000,
           "the kernel image has grown into the program arena at 14 MiB (arena.c ARENA_BASE)")
}
