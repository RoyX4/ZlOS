/* gdt64.c - install OUR OWN 64-bit GDT.
 *
 * This has to be real work, not a stub, and the reason is subtle enough that
 * it cost a reboot loop to find:
 *
 * On the multiboot path, boot64.S installs a GDT before it can even reach
 * 64-bit code, so by the time C runs the descriptors are ours and selector
 * 0x08 is a valid 64-bit code segment. On the UEFI path there is no such
 * step - the firmware hands us a machine that is ALREADY in long mode, using
 * the FIRMWARE's GDT, in which 0x08 means whatever that vendor decided.
 *
 * Our IDT gates all name selector 0x08. So on UEFI the first interrupt after
 * `sti` jumped through a descriptor that was not a 64-bit code segment: a
 * general protection fault, then a double fault, then a triple fault and a
 * silent reboot - which is exactly what the machine did, over and over.
 *
 * Installing our own GDT here makes selector 0x08 mean what the IDT says it
 * means, on every boot path.
 */

/* Long mode ignores base and limit for code and data. What matters is bit 53
 * (L, "this is 64-bit code"), bit 44 (S, not a system descriptor), bit 47
 * (present) and the type bits. L is reserved and must be zero on the data
 * descriptor; setting it there is invalid even when a permissive VM accepts
 * the descriptor. */
static unsigned long long gdt[7] __attribute__((aligned(16))) = {
    0x0000000000000000ULL,      /* null                                  */
    0x00AF9A000000FFFFULL,      /* 0x08: code, L=1, present, ring 0      */
    0x00CF92000000FFFFULL,      /* 0x10: data, present, writable, L=0    */
    0x00CFF2000000FFFFULL,      /* 0x18: ring-3 data (selector 0x1b)     */
    0x00AFFA000000FFFFULL,      /* 0x20: ring-3 64-bit code (0x23)       */
    0, 0                       /* 0x28: 16-byte long-mode TSS descriptor */
};

struct tss64 {
    unsigned int reserved0;
    unsigned long long rsp0, rsp1, rsp2;
    unsigned long long reserved1;
    unsigned long long ist1, ist2, ist3, ist4, ist5, ist6, ist7;
    unsigned long long reserved2;
    unsigned short reserved3, iomap;
} __attribute__((packed));
_Static_assert(sizeof(struct tss64) == 104, "long-mode TSS must be 104 bytes");

static struct tss64 tss;
static unsigned char tss_stack[16384] __attribute__((aligned(16)));
#define DOUBLE_FAULT_STACK_BYTES 16384u
static unsigned char double_fault_stack[DOUBLE_FAULT_STACK_BYTES]
    __attribute__((aligned(16)));

unsigned long long gdt64_double_fault_stack_low(void)
{
    return (unsigned long long)double_fault_stack;
}

unsigned long long gdt64_double_fault_stack_top(void)
{
    return (unsigned long long)(double_fault_stack + sizeof double_fault_stack);
}

unsigned long long gdt64_kernel_stack_top(void)
{
    return (unsigned long long)(tss_stack + sizeof tss_stack);
}

unsigned long long gdt64_active_kernel_stack_top(void)
{
    return tss.rsp0;
}

void gdt64_set_kernel_stack(unsigned long long top)
{
    /* RSP0 is consumed by hardware only on a CPL3 -> CPL0 transition. The
     * scheduler updates it before entering a process, so syscall/fault frames
     * cannot collide with another process's kernel stack. */
    if (top) tss.rsp0 = top;
}

/* Same trap as idt.c: LGDT wants 2 bytes of limit and EIGHT of base, but
 * `unsigned long` is only 4 bytes on the EFI build's clang target, making this
 * 6 bytes and leaving the top half of the base to be read from adjacent
 * memory. Explicitly sized, and asserted so it cannot regress quietly. */
struct gdt_ptr {
    unsigned short     limit;
    unsigned long long base;
} __attribute__((packed));
_Static_assert(sizeof(struct gdt_ptr) == 10, "LGDT operand must be 10 bytes");

static struct gdt_ptr gp;

void gdt_init(void)
{
    unsigned long long base = (unsigned long long)&tss;
    unsigned long long limit = sizeof(tss) - 1;
    tss.rsp0 = gdt64_kernel_stack_top();
    /* IDT vector 8 names IST1. A double fault commonly means the interrupted
     * stack is unusable, so reusing RSP0 here would preserve the exact failure
     * mode IST exists to contain. */
    tss.ist1 = gdt64_double_fault_stack_top();
    tss.iomap = sizeof(tss);       /* no I/O bitmap: all ports denied at CPL3 */
    gdt[5] = (limit & 0xffffULL) |
             ((base & 0xffffffULL) << 16) |
             (0x89ULL << 40) |
             ((limit & 0xf0000ULL) << 32) |
             ((base & 0xff000000ULL) << 32);
    gdt[6] = base >> 32;
    gp.limit = sizeof(gdt) - 1;
    /* The cast matters as much as the struct field did: `unsigned long` is 4
     * bytes here, so this truncated the GDT base to 32 bits before widening it
     * again. Harmless only while the image loads below 4 GiB. */
    gp.base  = (unsigned long long)&gdt;

    /* Loading the GDT does not change CS - the CPU keeps using the descriptor
     * it already cached. In 32-bit mode you reload CS with a far jump; that
     * instruction does not exist in 64-bit mode, so the way to do it is to
     * fake a far return: push the selector and a return address, then lretq. */
    __asm__ volatile(
        "lgdt %0\n\t"
        "pushq $0x08\n\t"              /* the new CS                */
        "leaq 1f(%%rip), %%rax\n\t"    /* ...and where to continue  */
        "pushq %%rax\n\t"
        "lretq\n"
        "1:\n\t"
        "mov $0x10, %%ax\n\t"          /* every data segment to our descriptor */
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%ss\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov $0x28, %%ax\n\t"
        "ltr %%ax\n\t"
        :
        : "m"(gp)
        : "rax", "memory");
}
