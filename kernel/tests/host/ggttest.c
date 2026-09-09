/* ggttest.c - the GGTT entries the bring-up overwrites come back byte-for-byte.
 *
 * intel.c against a fake BAR0: a 16 MiB anonymous mapping stands in for the
 * register block and the GGTT window at GGTT_OFFSET, and the PCI config hook
 * answers MGGC0 with "2 MiB table". Nothing here touches hardware; that is
 * the point - the save/restore is arithmetic on a table, and arithmetic can
 * be held exactly.
 *
 * Until 2026-09-08 the teardown left our scanout mapping in the firmware's
 * table (receipt D2 relocated the window past the live scanout; this closes
 * the "only relocation landed" item).
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

typedef unsigned int u32; typedef unsigned short u16;
typedef unsigned char u8; typedef unsigned long long uptr;

void intel_attach(uptr, u32, uptr, u32, int, u32 (*)(int, int, int, int));
int  intel_ggtt_map_range(u32 gfx_page, u32 phys_addr, int pages);
int  intel_ggtt_map_range_saved(u32 gfx_page, u32 phys_addr, int pages);
int  intel_ggtt_restore(void);
int  intel_ggtt_saved_pages(void);
u32  intel_ggtt_size(void);

/* intel.c's world, minimally */
void cpu_delay_us(unsigned int us) { (void)us; }
void cpu_delay_ms(unsigned int ms) { (void)ms; }
unsigned int cpu_now_ms(void) { return 0; }
u32  idt_ticks(void) { return 0; }
int  pci_count(void) { return 0; } int pci_vendor(int i) { (void)i; return 0; }
int  pci_device(int i) { (void)i; return 0; } int pci_class(int i) { (void)i; return 0; }
void pci_scan(void) { } void pci_enable(int i) { (void)i; }
u32  pci_bar(int i, int w) { (void)i; (void)w; return 0; }
u32  pci_bar_size(int i, int w) { (void)i; (void)w; return 0; }
u32  pci_bar_hi(int i, int w) { (void)i; (void)w; return 0; }
u32  pci_read32(int b, int d, int f, int o) { (void)b; (void)d; (void)f; (void)o; return 0; }
void console_init_fb(unsigned long long a, u32 p, u32 w, u32 h, u32 b)
{ (void)a; (void)p; (void)w; (void)h; (void)b; }

static u32 cfg(int b, int d, int f, int off)
{
    (void)b; (void)d; (void)f;
    if (off == 0x50) return 1u << 6;         /* MGGC0: GGMS = 1 -> 2 MiB table */
    return 0;
}

#define BAR_BYTES   (16u << 20)
#define GGTT_OFFSET 0x800000u                /* same constant as intel.c */

static int fails;
static void ok(const char *what, int cond)
{
    printf("  %-66s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}

int main(void)
{
    printf("ggttest - GGTT entries survive a bring-up and teardown\n\n");
    u8 *bar = mmap(NULL, BAR_BYTES, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (bar == MAP_FAILED) { perror("mmap"); return 2; }
    intel_attach((uptr)bar, BAR_BYTES, 0, 0, 0x5916, cfg);
    ok("the fake config space reports a 2 MiB GGTT", intel_ggtt_size() == (2u << 20));

    /* the firmware's table: a recognisable pattern per entry, both words */
    u32 *table = (u32 *)(bar + GGTT_OFFSET);
    u32 entries = intel_ggtt_size() / 8u;
    for (u32 i = 0; i < entries; i++) {
        table[2 * i]     = 0xF1000000u | (i << 12) | 1u;
        table[2 * i + 1] = 0x000000A0u | (i & 0xF);
    }
    u8 *before = malloc(entries * 8u);
    memcpy(before, table, entries * 8u);

    ok("nothing is held before any mapping", intel_ggtt_saved_pages() == 0);
    ok("restore with nothing held is a refusal, not a write", intel_ggtt_restore() == 0);
    ok("...and the table is untouched", memcmp(before, table, entries * 8u) == 0);

    /* the bring-up's mapping: 1 MiB in, three pages */
    ok("mapping three pages at 1 MiB succeeds", intel_ggtt_map_range_saved(256u, 0x10000000u, 3) == 1);
    ok("...and holds exactly those three", intel_ggtt_saved_pages() == 3);
    ok("...and wrote them present, pointing at our frames",
       table[2 * 256] == (0x10000000u | 1u) && table[2 * 256 + 1] == 0 &&
       table[2 * 258] == (0x10002000u | 1u) && table[2 * 258 + 1] == 0);
    ok("...and touched nothing outside the range",
       memcmp(before, table, 256u * 8u) == 0 &&
       memcmp(before + 259u * 8u, (u8 *)table + 259u * 8u, (entries - 259u) * 8u) == 0);
    ok("a second range while one is held is refused", intel_ggtt_map_range_saved(600u, 0x20000000u, 1) == 0);
    ok("...and did not write", memcmp(before + 600u * 8u, (u8 *)table + 600u * 8u, 8u) == 0);

    ok("restore reports success", intel_ggtt_restore() == 1);
    ok("...and the whole table is byte-identical to the firmware's", memcmp(before, table, entries * 8u) == 0);
    ok("...and nothing is held afterwards", intel_ggtt_saved_pages() == 0);
    ok("a second restore is a refusal", intel_ggtt_restore() == 0);

    /* limits: never a partial save */
    ok("a range past the table is refused", intel_ggtt_map_range_saved(entries - 2u, 0x10000000u, 3) == 0);
    ok("...holding nothing", intel_ggtt_saved_pages() == 0);
    ok("a range longer than the save area is refused", intel_ggtt_map_range_saved(0u, 0x10000000u, 16385) == 0);
    ok("...holding nothing", intel_ggtt_saved_pages() == 0);
    ok("...and the table is still the firmware's", memcmp(before, table, entries * 8u) == 0);

    /* mapping again after a restore works - the hold was released */
    ok("mapping again after restore succeeds", intel_ggtt_map_range_saved(256u, 0x10000000u, 2) == 1);
    ok("...and restore returns the table once more",
       intel_ggtt_restore() == 1 && memcmp(before, table, entries * 8u) == 0);

    /* the PLAIN call holds nothing and may be called back to back - the
     * compositor maps its back and scan buffers that way (gpuring.c) */
    ok("the plain map_range maps a range", intel_ggtt_map_range(600u, 0x20000000u, 2) == 1);
    ok("...and a second one right after it", intel_ggtt_map_range(700u, 0x30000000u, 2) == 1);
    ok("...holding nothing either time", intel_ggtt_saved_pages() == 0);
    ok("...even while the bring-up's range is held", intel_ggtt_map_range_saved(256u, 0x10000000u, 2) == 1 &&
       intel_ggtt_map_range(800u, 0x40000000u, 1) == 1 && intel_ggtt_saved_pages() == 2);
    ok("a 4K panel's 8100 pages fit the save area", intel_ggtt_restore() == 1 &&
       intel_ggtt_map_range_saved(256u, 0x10000000u, 8100) == 1 && intel_ggtt_saved_pages() == 8100);
    ok("...and come back", intel_ggtt_restore() == 1 && memcmp(before, table, 256u * 8u) == 0 &&
       memcmp(before + 8356u * 8u, (u8 *)table + 8356u * 8u, (entries - 8356u) * 8u) == 0);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
