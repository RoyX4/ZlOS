/* The multiboot header must sit in the first 8 KiB of the file, so it is
 * placed first, before .text. 1 MiB is the conventional load address - below
 * it is BIOS and legacy device memory. */
ENTRY(_start)
SECTIONS
{
    . = 1M;
    .multiboot : { *(.multiboot) }
    .text      : { *(.text*)     }
    .rodata    : { *(.rodata*)   }
    .data      : { *(.data*)     }
    .bss       : { *(COMMON) *(.bss*) }
}
