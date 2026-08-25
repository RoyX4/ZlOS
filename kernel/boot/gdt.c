/* gdt.c - our own Global Descriptor Table.
 *
 * The GDT tells the CPU how memory is segmented. Both boot paths already
 * left us in protected mode with *some* GDT (GRUB's, or our boot sector's),
 * but their segment selector numbers differ. The IDT's gates have to name a
 * code selector, so the kernel loads its OWN GDT with selectors it controls:
 * 0x08 = flat 4 GiB code, 0x10 = flat 4 GiB data. After this, 0x08/0x10 mean
 * the same thing no matter who booted us. (design_kernel.md §4.)
 *
 * "Flat" means every segment covers all 4 GiB starting at 0, so a pointer is
 * just a physical address and segmentation effectively disappears - which is
 * what every modern kernel wants; the real memory protection comes later from
 * paging, not segments.
 */
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

struct gdt_entry {
    u16 limit_lo;
    u16 base_lo;
    u8  base_mid;
    u8  access;
    u8  gran;
    u8  base_hi;
} __attribute__((packed));

struct gdt_ptr {
    u16 limit;
    u32 base;
} __attribute__((packed));

/* ---- the task state segment, and why a kernel with no tasks needs one -----
 *
 * The TSS is a relic: 80386 hardware task switching, which nobody has used
 * since. Exactly ONE field in it still matters on x86, and it is the reason
 * this struct is here.
 *
 * When the CPU takes an interrupt while running in ring 3, it cannot keep using
 * the ring-3 stack - a user program could have pointed ESP at anything, or at
 * nothing. So it loads a fresh SS:ESP from the TSS's ss0/esp0 BEFORE pushing
 * the interrupt frame. Without a loaded TSS, the first ring-3 interrupt - the
 * first timer tick, the first syscall - faults while trying to handle a fault,
 * which is a double fault, and then a triple fault, which is a silent reboot.
 *
 * So: no TSS, no ring 3. Not "ring 3 is unreliable" - ring 3 does not survive
 * one timer tick. Every other field below is written as zero and stays zero.
 */
struct tss_entry {
    u32 prev;
    u32 esp0, ss0;          /* THE two fields. Everything else is ballast. */
    u32 esp1, ss1, esp2, ss2;
    u32 cr3, eip, eflags;
    u32 eax, ecx, edx, ebx, esp, ebp, esi, edi;
    u32 es, cs, ss, ds, fs, gs;
    u32 ldt;
    u16 trap, iomap_base;
} __attribute__((packed));

/* Six entries now, and the three new ones are ADDED after the three that were
 * here - 0x08 and 0x10 keep meaning exactly what they meant, which is what the
 * IDT's gates and both boot paths depend on.
 *
 *   0x00  null
 *   0x08  ring 0 code, flat        <- unchanged
 *   0x10  ring 0 data, flat        <- unchanged
 *   0x18  ring 3 code, flat        (used as 0x1B: selector | RPL 3)
 *   0x20  ring 3 data, flat        (used as 0x23)
 *   0x28  the TSS
 *
 * The ring-3 segments are FLAT, covering the same 4 GiB as the ring-0 ones.
 * That is deliberate and it is the honest limit of what this buys: on a build
 * with paging off, segmentation is the only mechanism available and a flat
 * segment isolates nothing. What ring 3 does buy here is PRIVILEGE separation -
 * no CLI/STI, no LGDT/LIDT, no MOV to CR0/CR3, no IN/OUT (IOPL is 0), no HLT.
 * A ring-3 program that tries any of them takes a #GP instead of doing it.
 *
 * MEMORY isolation needs paging's U/S bit and a separate address space per
 * process, and that is 64-bit-only work - see docs/reference/system/memory-model.md, Stage 5.
 * Saying "ring 3" and meaning "isolated" is the confusion this comment exists
 * to prevent. */
static struct gdt_entry gdt[6];
static struct gdt_ptr   gdtp;
static struct tss_entry tss;

/* The ring-0 stack the CPU switches to on entry from ring 3. It is a dedicated
 * 16 KiB, not the boot stack: the boot stack is whatever raw_entry.S or GRUB
 * left, and it is already deep in use by the compositor when a syscall
 * arrives. A separate stack also means a runaway user program cannot arrange
 * for the kernel to re-enter on a stack it has been scribbling on. */
#define K_STACK_BYTES 16384
static u8 kstack[K_STACK_BYTES] __attribute__((aligned(16)));

u32 gdt_kernel_stack_top(void) { return (u32)kstack + K_STACK_BYTES; }

static void set_entry(int i, u32 base, u32 limit, u8 access, u8 gran)
{
    gdt[i].base_lo  = base & 0xFFFF;
    gdt[i].base_mid = (base >> 16) & 0xFF;
    gdt[i].base_hi  = (base >> 24) & 0xFF;
    gdt[i].limit_lo = limit & 0xFFFF;
    gdt[i].gran     = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].access   = access;
}

void gdt_init(void)
{
    /* access byte: present | ring 0 | code/data | exec | read/write
     * 0x9A = present, ring0, code, readable    (code segment)
     * 0x92 = present, ring0, data, writable     (data segment)
     * gran 0xCF = 4 KiB granularity, 32-bit, limit's top nibble = 0xF   */
    set_entry(0, 0, 0x00000000, 0x00, 0x00);   /* null - required first  */
    set_entry(1, 0, 0x000FFFFF, 0x9A, 0xCF);   /* 0x08 code, flat 4 GiB  */
    set_entry(2, 0, 0x000FFFFF, 0x92, 0xCF);   /* 0x10 data, flat 4 GiB  */
    /* 0xFA / 0xF2 are 0x9A / 0x92 with the DPL bits set to 3. That two-bit
     * difference is the entire privilege boundary. */
    set_entry(3, 0, 0x000FFFFF, 0xFA, 0xCF);   /* 0x18 ring-3 code       */
    set_entry(4, 0, 0x000FFFFF, 0xF2, 0xCF);   /* 0x20 ring-3 data       */

    /* The TSS descriptor. access 0x89 = present, ring 0, type 9 = available
     * 32-bit TSS. gran 0x00, NOT 0xCF: the limit is in BYTES here, and setting
     * 4 KiB granularity would describe a TSS 4096 times too large, which the
     * CPU accepts and then reads garbage out of. */
    set_entry(5, (u32)&tss, sizeof(tss) - 1, 0x89, 0x00);

    /* Zero it by hand - .bss is zeroed on both boot paths, but gdt_init() is
     * also callable twice and a stale esp0 is a fault that only happens after
     * the second call. */
    {
        u8 *p = (u8 *)&tss;
        unsigned i;
        for (i = 0; i < sizeof(tss); i++) p[i] = 0;
    }
    tss.ss0  = 0x10;                            /* ring-0 data selector   */
    tss.esp0 = gdt_kernel_stack_top();
    /* iomap_base past the end of the segment means "no I/O bitmap", which the
     * CPU reads as: ring 3 may not touch any port. Leaving it 0 would make the
     * CPU read the first bytes of the TSS itself as a permission bitmap. */
    tss.iomap_base = sizeof(tss);

    gdtp.limit = sizeof(gdt) - 1;
    gdtp.base  = (u32)&gdt;

    /* Load it, then RELOAD the segment registers so the new selectors take
     * effect. CS can only be reloaded by a far jump; the rest by plain movs.
     * The far jump to 1f lands at the same linear address (segments are flat)
     * but now running under selector 0x08. */
    __asm__ volatile(
        "lgdt %0\n\t"
        "ljmp $0x08, $1f\n\t"
        "1:\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%ss\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        :
        : "m"(gdtp)
        : "eax", "memory");

    /* LOAD THE TASK REGISTER. Without this the descriptor above is just six
     * bytes nobody reads, and the first interrupt taken from ring 3 triple
     * faults. It must come AFTER lgdt - ltr resolves the selector through the
     * GDT that is loaded at the time. */
    __asm__ volatile("ltr %%ax" :: "a"((u16)0x28));
}

/* Point the TSS at the stack an interrupt from ring 3 should land on. A
 * scheduler with more than one user task calls this on every switch; today
 * there is one, and it is set once in gdt_init(). Exposed rather than inlined
 * because the day a second one exists, forgetting this call is a bug whose
 * symptom is two tasks quietly sharing a kernel stack. */
void gdt_set_kernel_stack(u32 esp0) { tss.esp0 = esp0; }
