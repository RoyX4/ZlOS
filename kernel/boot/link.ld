/* The multiboot header must sit in the first 8 KiB of the file, so it is
 * placed first, before .text. 1 MiB is the conventional load address - below
 * it is BIOS and legacy device memory. */
ENTRY(_start)
PHDRS
{
    text PT_LOAD FLAGS(5);   /* read + execute */
    ro   PT_LOAD FLAGS(4);   /* read only      */
    data PT_LOAD FLAGS(6);   /* read + write   */
}
SECTIONS
{
    . = 1M;
    __kernel_start = .;
    .multiboot : { *(.multiboot) } :text
    .text      : { *(.text*)     } :text
    .rodata    : { *(.rodata*)   } :ro
    .data      : { *(.data*)     } :data
    .bss       : { *(COMMON) *(.bss*) } :data

    /* The kernel image now has a NEIGHBOUR that is not the kernel's: arena.c
     * puts the program arena at a fixed 14 MiB. Nothing stopped the image from
     * growing into it except arithmetic nobody was doing, which is precisely
     * how this project has hit a fixed-address collision five times. The
     * linker knows the answer exactly, so it says so, at build time, instead
     * of leaving it to be discovered by a program that scribbles on .bss.
     *
     * Keep these three numbers in step with arena.c's ARENA_BASE. */
    . = ALIGN(16);
    __kernel_end = .;
    ASSERT(__kernel_end <= 0x00C00000,
           "the kernel image has grown into the raw-boot stack at 12 MiB (raw_entry.S)")
    ASSERT(__kernel_end <= 0x00E00000,
           "the kernel image has grown into the program arena at 14 MiB (arena.c ARENA_BASE)")
}
