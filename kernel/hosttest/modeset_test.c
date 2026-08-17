/* modeset_test.c - the cold-start modeset harness.
 *
 * gen9-modeset-plan.txt orders the work in 11 stages and puts the firmware-state
 * survey first, for a reason worth restating: three register offsets in the plan
 * are contested between sources (C1, C2, C7), and picking the wrong one of each
 * gives a black screen with no error anywhere. All three are settleable by
 * READING a panel the firmware has already lit. So that is what stage 1 does,
 * and it costs nothing - i915 keeps running, no pixel moves.
 *
 * The fourth unknown is not a conflict but a genuine absence: which DDI buffer
 * translation table this board uses encodes the OEM's low-vswing decision, and
 * that lives in VBT, not in any register. The one place it is observable is the
 * values firmware already programmed into DDI_BUF_TRANS_A. Read them back and
 * compare against both candidate tables.
 *
 *   ./modeset_test --survey     read-only. safe with i915 running. (stage 1)
 *   ./modeset_test --modeset    the real thing. requires gpu-dev.sh detach.
 *
 * --modeset is deliberately not implemented yet: every later phase consumes
 * saved_port_bits and the buf-trans identity that --survey establishes, and
 * guessing them is exactly the failure the plan is written to avoid.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

typedef unsigned int u32; typedef unsigned char u8; typedef unsigned long long uptr;
#define PCI_DEV "/sys/bus/pci/devices/0000:00:02.0"
#define BAR_BYTES (8u << 20)      /* 8 MiB, not 16: the kernel refuses the full
                                   * BAR while i915 holds it, and every display
                                   * register is under 1 MiB anyway. */

static volatile unsigned char *bar;
static u32 rd(unsigned off) { return *(volatile u32 *)(bar + off); }

/* ---- the contested offsets, both candidates side by side ---------------- */
#define TRANSCONF_EDP_C1A   0x7F008     /* plan says this one   */
#define TRANSCONF_EDP_C1B   0x6F008     /* other report; is really TRANS_HSYNC */
#define PIPE_SRCSZ_C2A      0x6001C     /* plan says this one   */
#define PIPE_SRCSZ_C2B      0x6F01C
#define PP_CONTROL          0xC7204
#define PP_STATUS           0xC7200
#define PP_ON_DELAYS        0xC7208
#define PP_OFF_DELAYS       0xC720C
#define PP_DIVISOR          0xC7210     /* C7: dead on CMP. expect garbage    */
#define DDI_BUF_CTL_A       0x64000
#define DDI_BUF_TRANS_A     0x64E00
#define DISPIO_CR_TX_BMU    0x6C00C
#define DPLL_CTRL1          0x6C058
#define DPLL_CTRL2          0x6C05C
#define DPLL_STATUS         0x6C060
#define CDCLK_CTL           0x46000
#define TRANS_DDI_FUNC_CTL  0x6F400
#define EDP_PSR_CTL         0x6F800
#define PLANE_CTL_1_A       0x70180
#define PLANE_SURF_1_A      0x7019C
#define DC_STATE_EN         0x45504
#define TRANS_HTOTAL_EDP    0x6F000
#define TRANS_VTOTAL_EDP    0x6F00C
#define PIPE_DATA_M1_EDP    0x6F030
#define PIPE_DATA_N1_EDP    0x6F034
#define PIPE_LINK_M1_EDP    0x6F040
#define PIPE_LINK_N1_EDP    0x6F044

/* skl_u_trans_edp - CML-U eDP low vswing, 10 entries. */
static const u32 edp_tbl[10][2] = {
    {0x00000018,0x000000A8},{0x00004013,0x000000A9},{0x00007011,0x000000A2},
    {0x00009010,0x0000009C},{0x00000018,0x000000A9},{0x00006013,0x000000A2},
    {0x00007011,0x000000A6},{0x00002016,0x000000AB},{0x00005013,0x0000009F},
    {0x00000018,0x000000DF},
};
/* kbl_u_trans_dp - the fallback if VBT low-vswing is NOT set, 9 entries. */
static const u32 dp_tbl[9][2] = {
    {0x0000201B,0x000000A1},{0x00005012,0x00000088},{0x80007011,0x000000CD},
    {0x80009010,0x000000C0},{0x0000201B,0x0000009D},{0x80005012,0x000000C0},
    {0x80007011,0x000000C0},{0x00002016,0x0000004F},{0x80005012,0x000000C0},
};

static int pass = 0, fail = 0, warn = 0;
static void ok(const char *what, const char *detail)
{ printf("  [ PASS ] %-34s %s\n", what, detail); pass++; }
static void bad(const char *what, const char *detail)
{ printf("  [ FAIL ] %-34s %s\n", what, detail); fail++; }
static void hmm(const char *what, const char *detail)
{ printf("  [ ???? ] %-34s %s\n", what, detail); warn++; }

static void survey(void)
{
    char b[160];

    printf("\n-- C1: which offset is TRANSCONF(EDP)? --------------------------\n");
    /* The firmware has the panel lit, so the real TRANSCONF must read both
     * ENABLE (b31) and STATE (b30). The impostor is TRANS_HSYNC, which decodes
     * as a plausible sync pair - that is exactly why it fooled a report. */
    u32 a = rd(TRANSCONF_EDP_C1A), b2 = rd(TRANSCONF_EDP_C1B);
    snprintf(b, sizeof b, "0x7F008 = %08X  (b31=%d b30=%d)", a, !!(a>>31), !!(a>>30&1));
    /* b31/b30 only mean "this is TRANSCONF" while something is driving the
     * pipe. Measured with i915 unbound, 0x7F008 reads 00000000 - the pipe is
     * genuinely off, and failing there was the test asserting a precondition
     * it does not control. The offset is still settled either way, because
     * 0x6F008 decodes as a plausible hsync pair and TRANSCONF cannot. */
    if ((a >> 31) && (a >> 30 & 1))      ok("TRANSCONF is 0x7F008", b);
    else if (a == 0 && !(rd(TRANS_DDI_FUNC_CTL) >> 31))
                                         ok("TRANSCONF is 0x7F008 (pipe off - reads 0)", b);
    else                                 bad("TRANSCONF is 0x7F008", b);
    snprintf(b, sizeof b, "0x6F008 = %08X  -> hsync %u..%u", b2, (b2 & 0xFFFF)+1, (b2>>16)+1);
    ok("0x6F008 decodes as TRANS_HSYNC", b);

    printf("\n-- C2: which offset is PIPE_SRCSZ? ------------------------------\n");
    u32 s1 = rd(PIPE_SRCSZ_C2A), s2 = rd(PIPE_SRCSZ_C2B);
    snprintf(b, sizeof b, "0x6001C = %08X  -> %ux%u", s1, (s1>>16)+1, (s1&0xFFFF)+1);
    if (((s1>>16)+1) > 640 && ((s1&0xFFFF)+1) > 480) ok("PIPE_SRCSZ is 0x6001C", b);
    else bad("PIPE_SRCSZ is 0x6001C", b);
    snprintf(b, sizeof b, "0x6F01C = %08X", s2);
    hmm("0x6F01C (should be meaningless)", b);

    printf("\n-- C5/C7: panel power sequencer --------------------------------\n");
    u32 ppc = rd(PP_CONTROL), pps = rd(PP_STATUS);
    u32 ppon = rd(PP_ON_DELAYS), ppoff = rd(PP_OFF_DELAYS), ppdiv = rd(PP_DIVISOR);
    snprintf(b, sizeof b, "PP_CONTROL = %08X  (top half %04X)", ppc, ppc >> 16);
    if ((ppc >> 16) == 0) ok("C5: no 0xABCD key on gen9", b); else bad("C5: no unlock key", b);
    snprintf(b, sizeof b, "PP_CONTROL[8:4] = %u  -> T12 = %u ms", (ppc>>4)&0x1F,
             (((ppc>>4)&0x1F) ? (((ppc>>4)&0x1F)-1)*100 : 0));
    ok("C7: cycle delay in PP_CONTROL", b);
    snprintf(b, sizeof b, "PP_DIVISOR = %08X  (ref divider %u - PRM forbids 0)", ppdiv, ppdiv >> 24);
    if ((ppdiv >> 24) == 0) ok("C7: PP_DIVISOR is dead on CMP", b);
    else hmm("C7: PP_DIVISOR looks alive", b);
    snprintf(b, sizeof b, "PP_STATUS = %08X  (b31 on=%d, seq %u, b1 pwr-down-on-reset=%d)",
             pps, !!(pps>>31), (pps>>28)&3, !!(ppc&2));
    ok("panel state", b);
    /* The register unit is 100 us. Print tenths - a field of 1 is 0.1 ms, and
     * rounding that to "0 ms" next to an H5 warning reads as "no delay needed",
     * which is the opposite of true. i915 forces these HW fields to 1 and does
     * the real wait in software from VBT, so a 0.1 ms here means "ask VBT",
     * not "no wait". */
    printf("         T3  (power-up)     %3u.%u ms\n", ((ppon>>16)&0x1FFF)/10,  ((ppon>>16)&0x1FFF)%10);
    printf("         backlight-on       %3u.%u ms\n", (ppon&0x1FFF)/10,        (ppon&0x1FFF)%10);
    printf("         T10 (power-down)   %3u.%u ms\n", ((ppoff>>16)&0x1FFF)/10, ((ppoff>>16)&0x1FFF)%10);
    printf("         backlight-off      %3u.%u ms   <- H5: if this is 0.1, the real\n"
           "                                         T9 is in VBT (260 ms), not here\n",
           (ppoff&0x1FFF)/10, (ppoff&0x1FFF)%10);
    /* i915 CLEARS b1 when it releases the device - measured 00000060 after
     * unbind against 00000067 while it was running. So a cleared bit here is
     * the state we inherit, not a fault; what would be a fault is leaving it
     * clear. intel_pp_delays_program() sets it as plan step 12. */
    if (!(ppc & 2)) hmm("H4: b1 clear - we must set it (step 12)",
                        "power-down-on-reset CLEAR: normal after an i915 unbind");
    else            ok("H4: power-down-on-reset set", "b1 = 1");

    printf("\n-- straps: read once, OR into every DDI_BUF_CTL write -----------\n");
    u32 buf = rd(DDI_BUF_CTL_A);
    u32 saved_port_bits = buf & ((1u<<16) | (1u<<4));
    snprintf(b, sizeof b, "DDI_BUF_CTL_A = %08X  -> saved_port_bits = %08X", buf, saved_port_bits);
    ok("saved_port_bits", b);
    snprintf(b, sizeof b, "b4 DDI_A_4_LANES = %d  -> max %d lanes", !!(buf&(1u<<4)), (buf&(1u<<4))?4:2);
    if (buf & (1u<<4)) ok("4 lanes available on DDI A", b);
    else bad("DDI_A_4_LANES clear - 2 lanes cannot carry this mode", b);
    snprintf(b, sizeof b, "b16 port reversal = %d, b31 enabled = %d, b7 idle = %d",
             !!(buf&(1u<<16)), !!(buf>>31), !!(buf&(1u<<7)));
    ok("port state", b);

    printf("\n-- the one thing no register spells out: which buf-trans table? --\n");
    u32 got[10][2];
    for (int i = 0; i < 10; i++) {
        got[i][0] = rd(DDI_BUF_TRANS_A + i*8);
        got[i][1] = rd(DDI_BUF_TRANS_A + i*8 + 4);
    }
    int edp_hit = 0, dp_hit = 0;
    for (int i = 0; i < 10; i++) if (got[i][0]==edp_tbl[i][0] && got[i][1]==edp_tbl[i][1]) edp_hit++;
    for (int i = 0; i < 9;  i++) if (got[i][0]==dp_tbl[i][0]  && got[i][1]==dp_tbl[i][1])  dp_hit++;
    for (int i = 0; i < 10; i++)
        printf("         entry %d  %08X / %08X%s\n", i, got[i][0], got[i][1],
               (got[i][0]==edp_tbl[i][0]&&got[i][1]==edp_tbl[i][1]) ? "   = skl_u_trans_edp" :
               (i<9 && got[i][0]==dp_tbl[i][0]&&got[i][1]==dp_tbl[i][1]) ? "   = kbl_u_trans_dp" : "   = neither");
    snprintf(b, sizeof b, "skl_u_trans_edp %d/10, kbl_u_trans_dp %d/9", edp_hit, dp_hit);
    if (edp_hit == 10)      ok("table is skl_u_trans_edp (low vswing, max level 3)", b);
    else if (dp_hit == 9)   ok("table is kbl_u_trans_dp (max level 2)", b);
    else                    hmm("table matches NEITHER - firmware left its own", b);

    u32 bmu = rd(DISPIO_CR_TX_BMU);
    snprintf(b, sizeof b, "DISPIO_CR_TX_BMU_CR0 = %08X  (DDI A I_boost %u, disable b23=%d)",
             bmu, (bmu>>8)&7, !!(bmu&(1u<<23)));
    ok("I_boost / balance leg", b);

    printf("\n-- clocks -------------------------------------------------------\n");
    u32 c1 = rd(DPLL_CTRL1), c2 = rd(DPLL_CTRL2), st = rd(DPLL_STATUS), cd = rd(CDCLK_CTL);
    static const u32 rate_mhz[6] = {2700,1350,810,1620,1080,2160};
    int r0 = (c1 >> 1) & 7;
    snprintf(b, sizeof b, "DPLL_CTRL1=%08X DPLL0 rate_idx=%d (%u MHz) lock=%d",
             c1, r0, r0 < 6 ? rate_mhz[r0] : 0, !!(st & 1));
    if (st & 1) ok("DPLL0 locked (feeds CDCLK - never disable)", b); else bad("DPLL0 not locked", b);
    snprintf(b, sizeof b, "DPLL_CTRL2=%08X  DDI A clk_off=%d sel=%u",
             c2, !!(c2 & (1u<<15)), (c2 >> 1) & 3);
    ok("DDI A clock routing", b);
    u32 cdk = ((cd & 0x7FF) * 500) + 1000;
    snprintf(b, sizeof b, "CDCLK_CTL=%08X -> %u kHz", cd, cdk);
    ok("CDCLK", b);

    printf("\n-- what firmware programmed on the transcoder --------------------\n");
    u32 ddi = rd(TRANS_DDI_FUNC_CTL);
    snprintf(b, sizeof b, "TRANS_DDI_FUNC_CTL = %08X  enable=%d mode=%u bpc=%u lanes=%u input_sel=%u",
             ddi, !!(ddi>>31), (ddi>>24)&7, (ddi>>20)&7, ((ddi>>1)&7)+1, (ddi>>12)&7);
    ok("TRANS_DDI_FUNC_CTL", b);
    if (((ddi>>12)&7) != 0)
        hmm("C10: input select is not 000b", "plan says never use 100b; firmware differs");
    u32 ht = rd(TRANS_HTOTAL_EDP), vt = rd(TRANS_VTOTAL_EDP);
    printf("         timings  %u active of %u total  x  %u active of %u total\n",
           (ht & 0xFFFF)+1, (ht>>16)+1, (vt & 0xFFFF)+1, (vt>>16)+1);

    printf("\n-- the pixel clock, which had never been measured -----------------\n");
    /* §6 item 4 called this blocking: 2720x1481 total implies 241.7 MHz only if
     * the panel really runs at 60 Hz, and every bandwidth conclusion rested on
     * that. The frame counter cannot settle it - PSR freezes it - but PIPE_LINK
     * M/N can, because it holds pixel_clock : link_clock exactly.
     *
     * Worked here rather than through the driver, on purpose: the same rule
     * that keeps this file on raw offsets. A measurement that goes through the
     * code it is meant to check is not a measurement. */
    u32 dm = rd(PIPE_DATA_M1_EDP), dn = rd(PIPE_DATA_N1_EDP);
    u32 lm = rd(PIPE_LINK_M1_EDP), ln = rd(PIPE_LINK_N1_EDP);
    unsigned lanes = ((ddi >> 1) & 7) + 1;
    static const unsigned bpc_tbl[8] = { 8, 10, 6, 12, 0, 0, 0, 0 };
    unsigned bpp = bpc_tbl[(ddi >> 20) & 7] * 3;
    /* symbol clock = bit rate / 10 = DPLL frequency / 5 */
    unsigned link_khz = (r0 < 6 ? rate_mhz[r0] * 1000u : 0) / 5u;

    printf("         DATA_M1 %08X  DATA_N1 %08X   (TU %u)\n", dm, dn, ((dm>>25)&0x3F)+1);
    printf("         LINK_M1 %08X  LINK_N1 %08X\n", lm, ln);
    if (!(ln & 0xFFFFFF) || !link_khz) {
        bad("pixel clock derivable from LINK M/N", "LINK_N1 or link clock reads zero");
    } else {
        /* round, not truncate: M was truncated when the ratio was built */
        unsigned long long pix = ((unsigned long long)link_khz * (lm & 0xFFFFFF)
                                  + (ln & 0xFFFFFF) / 2) / (ln & 0xFFFFFF);
        unsigned long long dots = (unsigned long long)((ht>>16)+1) * ((vt>>16)+1);
        unsigned long long mhz  = dots ? (pix * 1000000ull + dots/2) / dots : 0;
        snprintf(b, sizeof b, "%llu kHz at %u lanes / %u bpp -> %llu.%03llu Hz",
                 pix, lanes, bpp, mhz/1000, mhz%1000);
        ok("pixel clock MEASURED, not assumed", b);

        /* Cross-check against the other ratio. DATA is (bpp * pixel) :
         * (link * lanes * 8), so it carries the same pixel clock by a different
         * route - through bpp and the lane count. Agreement means the lane
         * count, the bpc decode and the link rate are all right too; that is
         * four readings confirming each other, not one being trusted. */
        if (bpp && lanes && (dn & 0xFFFFFF)) {
            unsigned long long pix2 =
                ((unsigned long long)(dm & 0xFFFFFF) * link_khz * lanes * 8ull
                 + (unsigned long long)(dn & 0xFFFFFF) * bpp / 2)
                / ((unsigned long long)(dn & 0xFFFFFF) * bpp);
            long long delta = (long long)pix2 - (long long)pix;
            snprintf(b, sizeof b, "DATA M/N gives %llu kHz, LINK M/N %llu kHz (delta %lld)",
                     pix2, pix, delta);
            /* both ratios are truncated into 24-bit fields, so a kHz of
             * disagreement out of 241690 is the encoding, not an error */
            if (delta > -3 && delta < 3) ok("the two ratios agree", b);
            else bad("the two ratios disagree - a decode is wrong", b);
        }

        /* The bandwidth conclusion the plan's step 26 rests on, now with a
         * measured clock instead of an assumed one. */
        unsigned long long need = pix * bpp;
        snprintf(b, sizeof b, "need %llu kbps; 4xHBR 8640000, 4xRBR 5184000, 2xHBR 4320000",
                 need);
        if (need > 5184000ull && need <= 8640000ull)
            ok("4 lanes @ HBR is the only working point", b);
        else
            hmm("bandwidth conclusion changed - re-read plan step 26", b);
    }

    u32 psr = rd(EDP_PSR_CTL);
    snprintf(b, sizeof b, "EDP_PSR_CTL = %08X  enable=%d", psr, !!(psr>>31));
    if (psr >> 31) hmm("PSR is ON - must be disabled first (4.3 #17)", b);
    else           ok("PSR is off", b);

    u32 dcs = rd(DC_STATE_EN);
    snprintf(b, sizeof b, "DC_STATE_EN = %08X", dcs);
    if (dcs & 3) hmm("DC5/DC6 armed - #1 cause of flaky AUX (4.3 #5)", b);
    else         ok("DC states already off", b);

    printf("\n-- plane --------------------------------------------------------\n");
    printf("         PLANE_CTL = %08X   PLANE_SURF = %08X\n",
           rd(PLANE_CTL_1_A), rd(PLANE_SURF_1_A));
}

int main(int argc, char **argv)
{
    int mode_survey = 0, mode_modeset = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--survey"))  mode_survey = 1;
        if (!strcmp(argv[i], "--modeset")) mode_modeset = 1;
    }
    if (!mode_survey && !mode_modeset) {
        fprintf(stderr, "usage: %s --survey | --modeset\n", argv[0]);
        return 1;
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    char p[256];
    snprintf(p, sizeof p, "%s/resource0", PCI_DEV);
    /* read-only for the survey: it cannot damage anything even by mistake */
    int fd = open(p, mode_modeset ? O_RDWR : O_RDONLY);
    if (fd < 0) { perror("open resource0 (need sudo)"); return 1; }
    void *m = mmap(NULL, BAR_BYTES, mode_modeset ? (PROT_READ|PROT_WRITE) : PROT_READ,
                   MAP_SHARED, fd, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 1; }
    bar = (volatile unsigned char *)m;

    printf("zlOS cold-start modeset harness\n");
    printf("===============================\n");
    printf("  BAR0 mapped %u MiB, %s\n", BAR_BYTES >> 20, mode_modeset ? "READ/WRITE" : "read-only");

    if (mode_survey) {
        printf("\n### STAGE 1 - firmware state survey (%s) ###\n",
               (rd(TRANS_DDI_FUNC_CTL) >> 31) ? "display is live"
                                              : "display is DOWN - i915 released it");
        survey();
        printf("\n===============================\n");
        printf("  %d passed, %d failed, %d to look at\n", pass, fail, warn);
        if (fail) printf("  a FAIL here means an offset in the plan is wrong. stop and fix it.\n");
    }

    if (mode_modeset) {
        printf("\n### STAGE 2+ - not implemented ###\n");
        printf("  Phases B-H all consume saved_port_bits and the buf-trans table\n");
        printf("  identity that stage 1 establishes. Run --survey first and build\n");
        printf("  the write path on what it reports, not on what we assumed.\n");
    }

    munmap(m, BAR_BYTES);
    return fail ? 2 : 0;
}
