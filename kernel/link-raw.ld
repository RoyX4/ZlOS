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
}
