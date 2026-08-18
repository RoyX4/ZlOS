/* link-raw.ld - layout for the kernel when OUR bootloader loads it.
 *
 * The kernel is linked at 0x10000 and run there directly - low memory the
 * boot sector can reach with plain BIOS disk reads, no 1 MiB copy dance. The
 * raw entry (raw_entry.S) is placed FIRST so the loader can jump straight to
 * the load address. __bss_start/__bss_end let the entry zero the BSS by hand.
 */
ENTRY(_rawstart)
SECTIONS
{
    . = 0x100000;
    .text : {
        *(.text.rawentry)     /* _rawstart at exactly 0x10000 */
        *(.text*)
    }
    .rodata : { *(.rodata*) }
    .data   : { *(.data*)   }
    . = ALIGN(16);
    __bss_start = .;
    .bss    : { *(COMMON) *(.bss*) }
    . = ALIGN(16);
    __bss_end = .;

    /* See link.ld. This path matters MOST of the three: raw_entry.S:16 puts
     * the stack top at 6 MiB and it grows DOWN toward this symbol, so the
     * image and the stack are walking at each other. */
    __kernel_end = .;
    ASSERT(__kernel_end <= 0x00600000,
           "the kernel image has grown into the raw-boot stack at 6 MiB (raw_entry.S:16)")
    ASSERT(__kernel_end <= 0x00800000,
           "the kernel image has grown into the program arena at 8 MiB (arena.c ARENA_BASE)")
}
