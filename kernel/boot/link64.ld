/* link64.ld - the 64-bit kernel layout.
 * Loaded at 1 MiB like the 32-bit one; the multiboot header must sit in the
 * first 8 KiB of the image or GRUB will not recognise it. */
ENTRY(_start)
SECTIONS
{
    . = 1M;
    .text : { *(.multiboot) *(.text*) }
    .rodata : { *(.rodata*) }
    .data   : { *(.data*) }
    . = ALIGN(16);
    __bss_start = .;
    .bss    : { *(COMMON) *(.bss*) }
    . = ALIGN(16);
    __bss_end = .;

    /* See link.ld for why. arena.c's ARENA_BASE is 14 MiB and the image must
     * not reach it; the raw-boot stack at 12 MiB comes first either way. */
    __kernel_end = .;
    ASSERT(__kernel_end <= 0x00C00000,
           "the kernel image has grown into the raw-boot stack at 12 MiB (raw_entry.S)")
    ASSERT(__kernel_end <= 0x00E00000,
           "the kernel image has grown into the program arena at 14 MiB (arena.c ARENA_BASE)")
}
