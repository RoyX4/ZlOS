/* smp_trampoline.S - what the other CPU cores wake up into.
 *
 * An application processor does not start where the boot processor is. It
 * starts in 16-bit REAL MODE, at a physical address the startup IPI encodes as
 * a page number, with nothing set up: no GDT, no protected mode, no stack.
 * That is why this is assembly and why it has to live below 1 MiB.
 *
 * So this code is copied to 0x9000 at run time and every core that wakes runs
 * exactly these instructions: load a GDT, switch to 32-bit protected mode,
 * work out which core it is, stand on its own stack, and call into C.
 *
 * Two things are handled by convention rather than by patching the code:
 *
 *   0x8FF0   the C entry point, written by the BSP before the IPI goes out
 *   stacks   computed from the core's own APIC id, so no shared state is
 *            touched before a stack exists - which matters, because until
 *            there is a stack there is nowhere to put anything
 *
 * A20 is already enabled by the boot processor and stays enabled, so there is
 * nothing to do about it here.
 */

.equ TRAMP,      0x9000        /* where this code is copied to           */
.equ ENTRY_PTR,  0x8FF0        /* the BSP leaves the C entry address here */
.equ STACK_BASE, 0x0A800000    /* 168 MiB: 16 KiB of stack per core       */
.equ STACK_SIZE, 0x4000

/* absolute address of a label once this code sits at TRAMP */
#define AT(label) (TRAMP + ((label) - smp_tramp_start))

.section .rodata
.align 16
.globl smp_tramp_start
.globl smp_tramp_end

.code16
smp_tramp_start:
    cli
    cld
    xorw    %ax, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

    /* our own GDT - the BSP's is somewhere we cannot assume is reachable
     * with a 16-bit data segment, and this one is three entries long */
    lgdt    AT(gdt_desc)

    movl    %cr0, %eax
    orl     $1, %eax
    movl    %eax, %cr0

    /* far jump to flush the pipeline and load CS with the code selector */
    ljmpl   $0x08, $AT(pm_entry)

.code32
pm_entry:
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    %ax, %fs
    movw    %ax, %gs

    /* Which core am I? CPUID leaf 1 EBX[31:24] is the initial APIC id, and it
     * is readable before the local APIC has been touched - which is the point,
     * because we need it to choose a stack before we can do anything else. */
    movl    $1, %eax
    cpuid
    shrl    $24, %ebx
    andl    $0xFF, %ebx

    /* stack top = STACK_BASE + (apic_id + 1) * STACK_SIZE */
    movl    %ebx, %eax
    incl    %eax
    imull   $STACK_SIZE, %eax, %eax
    addl    $STACK_BASE, %eax
    movl    %eax, %esp

    /* into C, at the address the BSP left for us */
    movl    ENTRY_PTR, %eax
    testl   %eax, %eax
    jz      park
    call    *%eax

park:
    cli
    hlt
    jmp     park

.align 8
gdt:
    .quad   0x0000000000000000      /* null                              */
    .quad   0x00CF9A000000FFFF      /* code: base 0, limit 4G, ring 0    */
    .quad   0x00CF92000000FFFF      /* data: base 0, limit 4G, ring 0    */
gdt_desc:
    .word   gdt_desc - gdt - 1
    .long   AT(gdt)

smp_tramp_end:
