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

static struct gdt_entry gdt[3];
static struct gdt_ptr   gdtp;

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
}
