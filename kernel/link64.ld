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
}
