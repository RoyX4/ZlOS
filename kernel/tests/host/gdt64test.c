/* gdt64test.c - one TSS per core, held against an independent encoder.
 *
 * Until 2026-09-06 the 64-bit lanes had ONE TSS and only the BSP loaded it:
 * every AP ran on the trampoline's three-entry GDT with TR null, so the #DF
 * gate's IST1 had no TSS to be read from and a stack overflow on a rendering
 * core was a triple fault. gdt64.c now builds a TSS per slot and smp.c loads
 * it on each AP.
 *
 * What a host can prove: the descriptor encoding, one slot at a time, against
 * a second implementation of the layout; that the slot count equals smp.c's
 * SMP_SLOTS (read out of smp.c by build.sh, not restated here); that every
 * slot's selector fits inside the table; and that no two slots share a TSS or
 * an IST1 stack. What it cannot: execute ltr. That is verify-64 / verify-efi,
 * which boot with -smp 2 and count bands - an AP whose TR did not take now
 * parks instead of joining.
 */
#include <stdio.h>
#include <string.h>

int  gdt64_tss_slots(void);
unsigned short gdt64_tss_selector(int slot);
void gdt64_tss_descriptor(unsigned long long base, unsigned long long limit,
                          unsigned long long *lo, unsigned long long *hi);
void gdt64_prepare_slot(int slot);
unsigned long long gdt64_descriptor_word(int index);
unsigned long long gdt64_tss_base(int slot);
unsigned long long gdt64_tss_ist1(int slot);
unsigned long long gdt64_tss_rsp0(int slot);
unsigned long long gdt64_slot_df_stack_low(int slot);
unsigned int gdt64_table_bytes(void);
unsigned long long gdt64_kernel_stack_top(void);

static int fails = 0;
static void ok(const char *what, int cond)
{
    printf("  %-64s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}

/* The Intel SDM layout, written out field by field rather than shifted in one
 * expression, so a shared mistake cannot make both sides agree. */
static void decode(unsigned long long lo, unsigned long long hi,
                   unsigned long long *base, unsigned long long *limit,
                   int *type, int *present, int *dpl, int *gran)
{
    unsigned long long b = 0, l = 0;
    l |= lo & 0xFFFFULL;                           /* limit 15:0            */
    l |= ((lo >> 48) & 0xFULL) << 16;              /* limit 19:16           */
    b |= (lo >> 16) & 0xFFFFFFULL;                 /* base 23:0             */
    b |= ((lo >> 56) & 0xFFULL) << 24;             /* base 31:24            */
    b |= (hi & 0xFFFFFFFFULL) << 32;               /* base 63:32            */
    *type    = (int)((lo >> 40) & 0xF);
    *dpl     = (int)((lo >> 45) & 0x3);
    *present = (int)((lo >> 47) & 0x1);
    *gran    = (int)((lo >> 55) & 0x1);
    *base = b; *limit = l;
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    printf("gdt64test - one TSS per core\n\n");

    int slots = gdt64_tss_slots();
    ok("the slot count matches smp.c's SMP_SLOTS", slots == SMP_SLOTS_EXPECT);
    printf("       (gdt64 %d, smp.c %d)\n", slots, SMP_SLOTS_EXPECT);

    /* the encoder against the independent decoder, on an awkward base */
    {
        unsigned long long lo = 0, hi = 0, base = 0, limit = 0;
        int type, present, dpl, gran;
        gdt64_tss_descriptor(0x0000000123456789ULL, 103ULL, &lo, &hi);
        decode(lo, hi, &base, &limit, &type, &present, &dpl, &gran);
        ok("a TSS descriptor decodes to the base it was built from", base == 0x0000000123456789ULL);
        ok("...and to its limit", limit == 103ULL);
        ok("...as an available 64-bit TSS (type 9)", type == 9);
        ok("...present, DPL 0, byte granularity", present == 1 && dpl == 0 && gran == 0);
        gdt64_tss_descriptor(0xFFFFFFFFFFFFFFF0ULL, 0xFFFFFULL, &lo, &hi);
        decode(lo, hi, &base, &limit, &type, &present, &dpl, &gran);
        ok("the top of the address space and a 20-bit limit survive", base == 0xFFFFFFFFFFFFFFF0ULL && limit == 0xFFFFFULL);
    }

    /* every slot: selector inside the table, descriptor pointing at its own
     * TSS, its own IST1 inside its own stack, nothing shared */
    int sel_ok = 1, desc_ok = 1, ist_ok = 1, distinct = 1, rsp0_ok = 1;
    for (int s = 0; s < slots; s++) {
        gdt64_prepare_slot(s);
        unsigned sel = gdt64_tss_selector(s);
        if (sel != 0x28u + 16u * (unsigned)s) sel_ok = 0;
        if (sel + 16u > gdt64_table_bytes()) sel_ok = 0;
        unsigned long long base = 0, limit = 0;
        int type, present, dpl, gran;
        decode(gdt64_descriptor_word((int)(sel / 8)), gdt64_descriptor_word((int)(sel / 8) + 1),
               &base, &limit, &type, &present, &dpl, &gran);
        if (base != gdt64_tss_base(s) || limit != 103ULL || type != 9 || !present) desc_ok = 0;
        unsigned long long low = gdt64_slot_df_stack_low(s), ist = gdt64_tss_ist1(s);
        if (ist != low + 16384ULL || (ist & 15ULL)) ist_ok = 0;
        for (int t = 0; t < s; t++) {
            if (gdt64_tss_base(t) == gdt64_tss_base(s)) distinct = 0;
            if (gdt64_tss_ist1(t) == gdt64_tss_ist1(s)) distinct = 0;
            /* stacks must not overlap: [low, low+16K) pairwise disjoint */
            unsigned long long lt = gdt64_slot_df_stack_low(t);
            if (!(lt + 16384ULL <= low || low + 16384ULL <= lt)) distinct = 0;
        }
        if (s == 0 && gdt64_tss_rsp0(0) != gdt64_kernel_stack_top()) rsp0_ok = 0;
        if (s > 0 && gdt64_tss_rsp0(s) != 0) rsp0_ok = 0;
    }
    ok("every slot's selector is 0x28 + 16*slot and inside the GDT", sel_ok);
    ok("every slot's descriptor names its own 104-byte TSS", desc_ok);
    ok("every slot's IST1 is the 16-aligned top of its own 16 KiB stack", ist_ok);
    ok("no two slots share a TSS, an IST1, or a stack", distinct);
    ok("only the BSP has a ring-0 stack in RSP0; APs carry 0", rsp0_ok);

    /* a slot past the table is refused, not written */
    {
        unsigned long long before = gdt64_descriptor_word(5 + 2 * slots - 1);
        gdt64_prepare_slot(slots);
        gdt64_prepare_slot(-1);
        ok("preparing a slot past the table changes nothing", gdt64_descriptor_word(5 + 2 * slots - 1) == before);
    }

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
