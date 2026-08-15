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
 * (present) and the type bits. */
static unsigned long long gdt[3] __attribute__((aligned(16))) = {
    0x0000000000000000ULL,      /* null                                  */
    0x00AF9A000000FFFFULL,      /* 0x08: code, L=1, present, ring 0      */
    0x00AF92000000FFFFULL       /* 0x10: data, present, writable, ring 0 */
};

struct gdt_ptr {
    unsigned short limit;
    unsigned long  base;
} __attribute__((packed));

static struct gdt_ptr gp;

void gdt_init(void)
{
    gp.limit = sizeof(gdt) - 1;
    gp.base  = (unsigned long)&gdt;

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
        :
        : "m"(gp)
        : "rax", "memory");
}
