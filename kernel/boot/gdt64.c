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
/* ONE TSS PER CORE (2026-09-06). Until then this table held a single TSS
 * and only the BSP ever executed ltr: every AP ran on the trampoline's
 * three-entry GDT with TR null. The #DF gate names IST1, and IST1 lives in
 * the TSS that TR points at - so a kernel-stack overflow on a rendering core
 * was a triple fault (silent reboot), not the diagnosed panic the BSP gets.
 * A TSS descriptor is marked Busy by ltr, so cores cannot share one: slot 0
 * is the BSP, slots 1.. match smp.c's ap_slots (SMP_SLOTS, held equal by
 * gdt64test). Selector for a slot: 0x28 + 16 * slot. */
#define GDT64_TSS_SLOTS 8
#define GDT64_TSS_SEL(slot) ((unsigned short)(0x28 + 16 * (slot)))
static unsigned long long gdt[5 + 2 * GDT64_TSS_SLOTS] __attribute__((aligned(16))) = {
    0x0000000000000000ULL,      /* null                                  */
    0x00AF9A000000FFFFULL,      /* 0x08: code, L=1, present, ring 0      */
    0x00CF92000000FFFFULL,      /* 0x10: data, present, writable, L=0    */
    0x00CFF2000000FFFFULL,      /* 0x18: ring-3 data (selector 0x1b)     */
    0x00AFFA000000FFFFULL,      /* 0x20: ring-3 64-bit code (0x23)       */
    /* 0x28 + 16*slot: 16-byte long-mode TSS descriptors, built per slot */
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

static struct tss64 tss[GDT64_TSS_SLOTS];
/* RSP0 is consumed only on a CPL3 -> CPL0 transition, and only the BSP runs
 * ring 3, so one kernel stack. An AP's rsp0 stays 0: a ring-3 entry there
 * would fault onto RSP 0, and that double fault now lands on the AP's own
 * IST1 stack and gets diagnosed instead of triple-faulting. */
static unsigned char tss_stack[16384] __attribute__((aligned(16)));
#define DOUBLE_FAULT_STACK_BYTES 16384u
static unsigned char double_fault_stack[GDT64_TSS_SLOTS][DOUBLE_FAULT_STACK_BYTES]
    __attribute__((aligned(16)));

int gdt64_tss_slots(void) { return GDT64_TSS_SLOTS; }
unsigned short gdt64_tss_selector(int slot) { return GDT64_TSS_SEL(slot); }

/* Which slot THIS core loaded. STR is readable at CPL0 everywhere the kernel
 * runs; a TR that is not one of ours (the trampoline GDT before
 * gdt64_ap_init, or the firmware's before gdt_init) reports slot 0. */
static int gdt64_this_slot(void)
{
    unsigned short tr = 0;
    __asm__ volatile("str %0" : "=r"(tr));
    if (tr < 0x28 || ((tr - 0x28) & 15)) return 0;
    int slot = (tr - 0x28) / 16;
    return slot < GDT64_TSS_SLOTS ? slot : 0;
}

unsigned long long gdt64_double_fault_stack_low(void)
{
    return (unsigned long long)double_fault_stack[gdt64_this_slot()];
}

unsigned long long gdt64_double_fault_stack_top(void)
{
    return (unsigned long long)(double_fault_stack[gdt64_this_slot()] + DOUBLE_FAULT_STACK_BYTES);
}

unsigned long long gdt64_kernel_stack_top(void)
{
    return (unsigned long long)(tss_stack + sizeof tss_stack);
}

unsigned long long gdt64_active_kernel_stack_top(void)
{
    return tss[0].rsp0;
}

/* The two qwords of a long-mode TSS descriptor: type 0x9 (available 64-bit
 * TSS), present, DPL 0, byte granularity. Pure, so gdt64test can hold it
 * against an independent encoder. */
void gdt64_tss_descriptor(unsigned long long base, unsigned long long limit,
                          unsigned long long *lo, unsigned long long *hi)
{
    *lo = (limit & 0xffffULL) |
          ((base & 0xffffffULL) << 16) |
          (0x89ULL << 40) |
          ((limit & 0xf0000ULL) << 32) |
          ((base & 0xff000000ULL) << 32);
    *hi = base >> 32;
}

/* Build slot's TSS and its descriptor pair. Not static: gdt64test drives it
 * for every slot on the host, where nothing may execute ltr. */
void gdt64_prepare_slot(int slot)
{
    if (slot < 0 || slot >= GDT64_TSS_SLOTS) return;
    struct tss64 *t = &tss[slot];
    t->rsp0 = slot == 0 ? gdt64_kernel_stack_top() : 0;
    /* IDT vector 8 names IST1. A double fault commonly means the interrupted
     * stack is unusable, so reusing RSP0 here would preserve the exact failure
     * it exists to survive. */
    t->ist1 = (unsigned long long)(double_fault_stack[slot] + DOUBLE_FAULT_STACK_BYTES);
    t->iomap = sizeof *t;          /* no I/O bitmap: all ports denied at CPL3 */
    gdt64_tss_descriptor((unsigned long long)t, sizeof *t - 1,
                         &gdt[5 + 2 * slot], &gdt[6 + 2 * slot]);
}

unsigned long long gdt64_descriptor_word(int index)
{
    return (index >= 0 && (unsigned)index < sizeof gdt / sizeof gdt[0]) ? gdt[index] : 0;
}
unsigned long long gdt64_tss_base(int slot)  { return (unsigned long long)&tss[slot]; }
unsigned long long gdt64_tss_ist1(int slot)  { return tss[slot].ist1; }
unsigned long long gdt64_tss_rsp0(int slot)  { return tss[slot].rsp0; }
unsigned long long gdt64_slot_df_stack_low(int slot) { return (unsigned long long)double_fault_stack[slot]; }
unsigned int gdt64_table_bytes(void) { return (unsigned int)sizeof gdt; }

void gdt64_set_kernel_stack(unsigned long long top)
{
    /* RSP0 is consumed by hardware only on a CPL3 -> CPL0 transition. The
     * scheduler updates it before entering a process, so syscall/fault frames
     * cannot collide with another process's kernel stack. */
    if (top) tss[0].rsp0 = top;
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

static void gdt64_load(unsigned short tr)
{
    /* Loading the GDT does not change CS - the CPU keeps using the descriptor
     * it already cached. In 64-bit mode the way to reload CS is a far return
     * through the new selector; then the data selectors and, last, TR. */
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
        "ltr %w1\n\t"
        :
        : "m"(gp), "r"(tr)
        : "rax", "memory");
}

void gdt_init(void)
{
    /* EFER.NXE. boot64.S and smp_trampoline64.S set it with LME; the native
     * UEFI entry (efi.c) inherits whatever the firmware left, and every
     * process PTE sets bit 63 - which is RESERVED while NXE is clear, so the
     * first ring-3 syscall would #PF(RSVD) on the TSS stack and double-fault.
     * OVMF happens to set NXE itself; nothing says the ThinkPad's firmware
     * does. This runs on every 64-bit route (setup_gdt in kernel.zl), and
     * setting a bit that is already set is harmless. */
    {
        unsigned int lo, hi;
        __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0xC0000080u));
        if (!(lo & (1u << 11))) {
            lo |= (1u << 11);
            __asm__ volatile("wrmsr" : : "c"(0xC0000080u), "a"(lo), "d"(hi) : "memory");
        }
    }
    gdt64_prepare_slot(0);
    gp.limit = sizeof(gdt) - 1;
    /* The cast matters: `unsigned long` is 4 bytes on the Win64 lane, so a
     * narrower cast truncated the GDT base to 32 bits before widening it. */
    gp.base  = (unsigned long long)&gdt;
    gdt64_load(GDT64_TSS_SEL(0));
}

/* An application processor's half of gdt_init: it arrives on the
 * trampoline's GDT (code, data, no TSS) with TR null. Give it OUR GDT and its
 * own TSS so vector 8 has an IST1 stack to switch to. gdt_init must have run
 * on the BSP first (setup_gdt in kernel.zl, every 64-bit route); if it has
 * not, the AP keeps the trampoline tables, exactly as before 2026-09-06. */
void gdt64_ap_init(int slot)
{
    if (slot < 1 || slot >= GDT64_TSS_SLOTS || !gp.base) return;
    gdt64_prepare_slot(slot);
    gdt64_load(GDT64_TSS_SEL(slot));
}
