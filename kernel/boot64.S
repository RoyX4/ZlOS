/* boot64.S - the multiboot entry that takes zlOS into 64-bit long mode.
 *
 * GRUB hands us the CPU in 32-bit protected mode with paging OFF, which is as
 * far as multiboot 1 goes. Long mode is not a mode switch you can just ask
 * for: the CPU refuses to enter it without paging, and paging needs real page
 * tables, so the order is forced and unforgiving:
 *
 *     1. build 4-level page tables and identity-map memory
 *     2. point CR3 at them
 *     3. enable PAE            (CR4.PAE)      - 64-bit paging is PAE-based
 *     4. set the long mode bit (EFER.LME)     - via an MSR
 *     5. enable paging         (CR0.PG)       - THIS is what enters long mode
 *     6. far jump into a 64-bit code segment  - the CPU is only really in
 *                                               64-bit code after CS is reloaded
 *
 * The identity map covers the first 4 GiB with 2 MiB pages. That is deliberate
 * and not just laziness: the framebuffer and the GPU's registers live high in
 * the physical address space (0xE9000000 on this laptop, 0xFD000000 in QEMU),
 * and if they are not mapped the first pixel we draw is a page fault. 4 GiB of
 * 2 MiB pages is 2048 entries - four page directories - and costs 24 KiB.
 */

.set MB_MAGIC,    0x1BADB002
.set MB_FLAGS,    0x00000007
.set MB_CHECKSUM, -(MB_MAGIC + MB_FLAGS)

.section .multiboot, "a"
.align 4
.long MB_MAGIC
.long MB_FLAGS
.long MB_CHECKSUM
.long 0, 0, 0, 0, 0           /* a.out fields, unused for ELF */
.long 0                       /* mode_type 0 = linear framebuffer */
.long 0                       /* width  - no preference */
.long 0                       /* height - no preference */
.long 0                       /* depth  - no preference */

/* The page tables live in low memory below the kernel, which loads at 1 MiB.
 * They must be 4 KiB aligned; the CPU ignores the low bits of CR3 otherwise. */
.set PML4, 0x1000
.set PDPT, 0x2000
.set PD0,  0x3000             /* four page directories, 0x3000..0x6FFF */

.section .text
.code32
.global _start
.type _start, @function
_start:
    cli
    mov $stack_top, %esp
    mov %ebx, mb_ptr                /* park the multiboot pointer in memory -
                                       the stack does not survive the mode
                                       switch cleanly, and popping after
                                       resetting RSP reads off the end of it */

    /* ---- zero the page tables (5 pages) ---------------------------------- */
    mov $PML4, %edi
    xor %eax, %eax
    mov $(5 * 4096 / 4), %ecx
    rep stosl

    /* ---- PML4[0] -> PDPT ------------------------------------------------- */
    mov $PML4, %edi
    movl $(PDPT + 3), (%edi)        /* present + writable */

    /* ---- PDPT[0..3] -> the four page directories ------------------------- */
    mov $PDPT, %edi
    movl $(PD0 + 0x0000 + 3), 0(%edi)
    movl $(PD0 + 0x1000 + 3), 8(%edi)
    movl $(PD0 + 0x2000 + 3), 16(%edi)
    movl $(PD0 + 0x3000 + 3), 24(%edi)

    /* ---- fill 2048 entries, each a 2 MiB page, covering 0..4 GiB ---------
     * bit 7 (PS) is what makes the entry a 2 MiB page rather than a pointer
     * to a page table, which is how 4 GiB fits in four directories. */
    mov $PD0, %edi
    xor %eax, %eax                  /* physical address, low half  */
    xor %ecx, %ecx                  /* entry counter               */
1:
    mov %eax, %edx
    or  $0x83, %edx                 /* present | writable | 2 MiB page */
    mov %edx, (%edi)
    movl $0, 4(%edi)                /* high half of the entry is zero  */
    add $0x200000, %eax             /* next 2 MiB                      */
    add $8, %edi
    inc %ecx
    cmp $2048, %ecx
    jne 1b

    /* ---- CR3 -> our tables ----------------------------------------------- */
    mov $PML4, %eax
    mov %eax, %cr3

    /* ---- CR4.PAE: 64-bit paging is PAE paging ---------------------------- */
    mov %cr4, %eax
    or  $(1 << 5), %eax
    mov %eax, %cr4

    /* ---- EFER.LME: arm long mode (MSR 0xC0000080, bit 8) ------------------ */
    mov $0xC0000080, %ecx
    rdmsr
    or  $(1 << 8), %eax
    wrmsr

    /* ---- CR0.PG: enabling paging is what actually enters long mode -------- */
    mov %cr0, %eax
    or  $(1 << 31), %eax
    mov %eax, %cr0

    /* The CPU is in long mode now, but still executing through a 32-bit code
     * segment (compatibility mode). Only reloading CS with a descriptor whose
     * L bit is set makes it run real 64-bit code. */
    lgdt gdt64_ptr
    ljmp $0x08, $long_entry

.code64
long_entry:
    mov $0x10, %ax                  /* the flat data selector */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs

    /* ---- enable SSE ------------------------------------------------------
     * This is not optional on x86-64. The System V AMD64 ABI passes and
     * returns floating point in SSE registers, and every zl number is a
     * double - so the very first arithmetic the kernel does is a `movsd` into
     * %xmm0. SSE starts DISABLED after reset, and executing an SSE
     * instruction while it is disabled raises #UD, which is exactly how this
     * failed the first time: invalid opcode, then a double fault, then a
     * triple fault and a silent reboot with nothing on the serial line.
     *
     * 32-bit never needed this because the i386 build used the x87 stack. */
    mov %cr0, %rax
    and $~(1 << 2), %rax            /* EM = 0: do not trap FPU instructions  */
    or  $(1 << 1), %rax             /* MP = 1                                */
    mov %rax, %cr0
    mov %cr4, %rax
    or  $(1 << 9), %rax             /* OSFXSR:    fxsave/fxrstor + SSE on    */
    or  $(1 << 10), %rax            /* OSXMMEXCPT: SSE exceptions are ours   */
    mov %rax, %cr4

    mov $stack_top, %rsp            /* 16-byte aligned, as the ABI requires */
    movl mb_ptr(%rip), %edi         /* multiboot pointer -> first argument;
                                       a 32-bit move zero-extends into RDI   */

    call serial_init
    call console_init               /* takes the multiboot pointer in RDI   */
    call main
    call kernel_done
1:  hlt
    jmp 1b

/* ---- a 64-bit GDT ---------------------------------------------------------
 * In long mode the segment base and limit are ignored for code and data; what
 * matters is bit 53 (L) marking the segment as 64-bit, and bit 44 (S) plus the
 * type bits. Two entries are enough for a flat kernel. */
.section .rodata
.align 16
gdt64:
    .quad 0                                     /* null                     */
    .quad 0x00AF9A000000FFFF                    /* code: L=1, present, ring0 */
    .quad 0x00AF92000000FFFF                    /* data: present, writable   */
gdt64_end:
gdt64_ptr:
    .word gdt64_end - gdt64 - 1
    .quad gdt64

.section .bss
.align 8
mb_ptr:
.skip 8                             /* the multiboot pointer, across the switch */
.align 16
stack_bottom:
.skip 262144                        /* 256 KiB - the compositor nests deep */
stack_top:
