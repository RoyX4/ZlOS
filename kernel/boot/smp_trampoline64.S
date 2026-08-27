/* smp_trampoline64.S - waking a core all the way into 64-bit long mode.
 *
 * The 32-bit trampoline gets an application processor as far as protected
 * mode, which is enough when the kernel is 32-bit. The UEFI build is not: it
 * runs in long mode with 4-level paging, and a core sitting in protected mode
 * cannot call into it. So this one goes the whole way.
 *
 * An AP still starts in 16-bit REAL MODE with nothing set up. Getting from
 * there to long mode is a fixed sequence and the order is not negotiable:
 *
 *     1. load a GDT that has a 64-bit code descriptor (the L bit)
 *     2. CR4.PAE   - long mode requires PAE paging
 *     3. CR3       - the page tables. We reuse the ones the boot processor
 *                    already built: they identity map the low 4 GiB, which is
 *                    everything any core needs, and building a second set
 *                    would be a second thing to keep in step.
 *     4. EFER.LME|NXE - long mode plus NX page entries, via MSR 0xC0000080
 *     5. CR0.PG|PE - turning paging on is what actually ENTERS long mode
 *     6. a far jump to reload CS with the 64-bit selector
 *
 * SSE then has to be switched on before calling C, for the same reason the
 * boot processor does it: every zl number is a double, so the compiler emits
 * SSE instructions, and they fault with #UD until CR0/CR4 say otherwise.
 *
 * Two values are left in memory by the BSP rather than patched into the code:
 *
 *     0x8FE0   CR3, so we share the boot processor's page tables
 *     0x8FF0   the 64-bit C entry point
 */

.equ TRAMP,      0x9000
.equ IDT_PTR,    0x8FC0
.equ CR3_PTR,    0x8FE0
.equ ENTRY_PTR,  0x8FF0
.equ STACK_BASE, 0x0A800000
.equ STACK_SIZE, 0x4000

#define AT(label) (TRAMP + ((label) - smp_tramp64_start))

.section .rodata
.align 16
.globl smp_tramp64_start
.globl smp_tramp64_end

.code16
smp_tramp64_start:
    cli
    cld
    xorw    %ax, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

    lgdt    AT(gdt_desc)

    /* PAE - long mode will not engage without it */
    movl    %cr4, %eax
    orl     $(1 << 5), %eax
    movl    %eax, %cr4

    /* the boot processor's page tables */
    movl    CR3_PTR, %eax
    movl    %eax, %cr3

    /* EFER.LME|NXE: match the boot processor's page-table contract */
    movl    $0xC0000080, %ecx
    rdmsr
    orl     $((1 << 8) | (1 << 11)), %eax
    wrmsr

    /* paging + protection on: this is the instant we enter long mode */
    movl    %cr0, %eax
    orl     $0x80000001, %eax
    movl    %eax, %cr0

    /* reload CS with the 64-bit descriptor */
    ljmpl   $0x08, $AT(lm_entry)

.code64
lm_entry:
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    %ax, %fs
    movw    %ax, %gs
    lidt    IDT_PTR

    /* SSE on, or the first double the compiler touches is an invalid opcode */
    movq    %cr0, %rax
    andq    $~(1 << 2), %rax        /* EM = 0: do not emulate           */
    orq     $(1 << 1), %rax         /* MP = 1                            */
    movq    %rax, %cr0
    movq    %cr4, %rax
    orq     $(3 << 9), %rax         /* OSFXSR | OSXMMEXCPT               */
    movq    %rax, %cr4

    /* which core am I? the initial APIC id, readable before touching the APIC */
    movl    $1, %eax
    cpuid
    shrl    $24, %ebx
    andl    $0xFF, %ebx

    /* stack top = STACK_BASE + (apic_id + 1) * STACK_SIZE */
    movl    %ebx, %eax
    incl    %eax
    imull   $STACK_SIZE, %eax, %eax
    addl    $STACK_BASE, %eax
    movq    %rax, %rsp
    andq    $~15, %rsp              /* the ABI wants 16-byte alignment   */

    movq    ENTRY_PTR, %rax
    testq   %rax, %rax
    jz      park64
    callq   *%rax

park64:
    cli
    hlt
    jmp     park64

.align 8
gdt64:
    .quad   0x0000000000000000      /* null                               */
    .quad   0x00AF9A000000FFFF      /* 64-bit code: the L bit is what matters */
    .quad   0x00CF92000000FFFF      /* data                                */
gdt_desc:
    .word   gdt_desc - gdt64 - 1
    .long   AT(gdt64)

smp_tramp64_end:
