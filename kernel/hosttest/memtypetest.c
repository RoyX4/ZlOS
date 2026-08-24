/* Intel table 14-7 cache-type combination, without privileged MSR access. */
#include <stdio.h>

unsigned int cpu_combine_memory_type(unsigned int pat, unsigned int mtrr);

static int checks, failures;
static void eq(unsigned got, unsigned want, const char *what)
{
    checks++;
    if (got != want) {
        failures++;
        printf("FAIL  %s: got %u wanted %u\n", what, got, want);
    }
}

int main(void)
{
    /* stable encoding: 0 unknown/WT/WP, 1 UC, 2 WC, 3 WB.  Every valid
     * PAT/MTRR cell is pinned, not just the surprising framebuffer cell. */
    const unsigned pat[] = { 0, 1, 4, 5, 6, 7 };
    const unsigned mtrr[] = { 0, 1, 4, 5, 6 };
    const unsigned want[][6] = {
        { 1, 2, 1, 1, 1, 1 },
        { 1, 2, 1, 1, 2, 2 },
        { 1, 2, 0, 0, 0, 1 },
        { 1, 2, 0, 0, 0, 2 },
        { 1, 2, 0, 0, 3, 1 },
    };
    char label[64];
    for (unsigned r = 0; r < sizeof mtrr / sizeof mtrr[0]; r++)
        for (unsigned c = 0; c < sizeof pat / sizeof pat[0]; c++) {
            snprintf(label, sizeof label, "PAT %u + MTRR %u", pat[c], mtrr[r]);
            eq(cpu_combine_memory_type(pat[c], mtrr[r]), want[r][c], label);
        }
    printf("%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
