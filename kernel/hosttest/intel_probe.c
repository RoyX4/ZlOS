/* intel_probe.c - run zlOS's Intel display driver against the REAL GPU,
 * from Linux, without booting anything.
 *
 * WHY THIS EXISTS
 * ---------------
 * The Intel driver is the whole point of the project, and until now it could
 * only be executed by writing a USB stick, rebooting the machine, and reading
 * a screen. That is a several-minute loop with no debugger, no printf, and no
 * way to diff a result against the previous run. Nobody develops a DPLL
 * sequence that way.
 *
 * The GPU's registers are just memory. Linux exposes the PCI BAR at
 * /sys/bus/pci/devices/0000:00:02.0/resource0, so a userspace program can mmap
 * it and reach exactly the same registers the kernel driver reaches. Compile
 * the SAME intel.c against that mapping and the driver runs here, at native
 * speed, with the full toolchain available - seconds per iteration instead of
 * minutes, and a real diff between runs.
 *
 * This is the code that ships. Not a model of it, not a rewrite: intel.c is
 * compiled directly into this binary. Anything it gets right here it gets
 * right in the kernel, and anything it gets wrong here is a real bug.
 *
 * SAFETY
 * ------
 * By default this only READS. i915 is driving the panel at the same time, and
 * reads are harmless - the GPU does not mind being looked at. Writes are a
 * different matter and are gated behind --unsafe, because two drivers
 * programming the same display engine will fight.
 *
 * For real DPLL work the right setup is to unbind i915 first (see
 * ./gpu-dev.sh detach) so nothing else is touching the hardware.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;
typedef unsigned long long uptr;

#define PCI_DEV "/sys/bus/pci/devices/0000:00:02.0"

/* ---- the driver under test -------------------------------------------- */
void intel_attach(uptr mmio, u32 mmio_bytes, uptr aper, u32 aper_bytes,
                  int devid, u32 (*cfg)(int, int, int, int));
int  intel_present(void);
int  intel_supported(void);
int  intel_devid(void);
u32  intel_mmio(void);
u32  intel_stolen_base(void);
u32  intel_stolen_size(void);
u32  intel_ggtt_size(void);
int  intel_pipe_width(void);
int  intel_pipe_height(void);
int  intel_stride(void);
int  intel_plane_enabled(void);
int  intel_pipe_enabled(void);
u32  intel_plane_ctl(void);
u32  intel_surface(void);
int  intel_frame_count(void);
int  intel_htotal(void);
int  intel_hactive(void);
int  intel_vtotal(void);
int  intel_vactive(void);
int  intel_hsync_start(void);
int  intel_hsync_end(void);
int  intel_vsync_start(void);
int  intel_vsync_end(void);
int  intel_read_edid(void);
int  intel_edid_pin(void);
int  intel_edid_vendor_char(int i);
int  intel_edid_product(void);
int  intel_edid_native_w(void);
int  intel_edid_native_h(void);
int  intel_edid_width_mm(void);
int  intel_edid_height_mm(void);
u32  intel_backlight_max(void);
u32  intel_backlight_get(void);
int  intel_panel_on(void);
u32  intel_plane_format(void);
int  intel_plane_tiling(void);
u32  intel_watermark(int level);
u32  intel_ddi_func_ctl(void);
int  intel_ddi_lanes(void);
int  intel_ddi_bpp(void);
u32  intel_pixel_clock_khz(void);
u32  intel_pixel_clock_mn_khz(void);
u32  intel_refresh_mhz_derived(void);
u32  intel_dp_link_symbol_khz(int rate_idx);
int  intel_mn_compute(u32 pixel_khz, u32 link_khz, int lanes, int bpp);
u32  intel_mn_data_m(void);
u32  intel_mn_data_n(void);
u32  intel_mn_link_m(void);
u32  intel_mn_link_n(void);
u32  intel_data_m1_reg(void);
u32  intel_data_n1_reg(void);
u32  intel_link_m1_reg(void);
u32  intel_link_n1_reg(void);
u32  intel_trans_ddi_ctl_value(int lanes, int bpp, int phsync, int pvsync);
u32  intel_msa_misc_value(int bpp);
u32  intel_msa_misc(void);
u32  intel_pipe_misc_value(int bpp, int dither);
u32  intel_pipe_misc(void);
u32  intel_wm_linetime_value(u32 htotal, u32 pixel_khz);
u32  intel_wm_linetime(void);
u32  intel_chicken_trans(void);
int  intel_scaler_enabled(int which);
u32  intel_cur_wm(int level);
u32  intel_cur_buf_cfg(void);
int  intel_ddb_valid(void);
u32  intel_ddb_cur_cfg_value(int cur_blocks);
u32  intel_ddb_plane_cfg_value(int cur_blocks);
int  intel_backlight_pwm_enabled(void);   /* max/get are declared further up */
void intel_set_edid_buffer(uptr p);
int  intel_transcoder_is_edp(void);
u32  intel_dpll_ctrl1(void);
u32  intel_dpll_ctrl2(void);
u32  intel_dpll_status(void);
u32  intel_cdclk_ctl(void);
u32  intel_dpll_cfgcr1(int pll);
u32  intel_dpll_cfgcr2(int pll);
int  intel_dpll_locked(int pll);
int  intel_dpll_link_rate(int pll);
int  intel_dpll_ssc(int pll);
int  intel_dpll_is_hdmi(int pll);
int  intel_ddi_clock_select(int ddi);
int  intel_ddi_clock_off(int ddi);
u32  intel_dpll_rate_khz(int idx);
u32  intel_dp_link_bandwidth_kbps(int rate_idx, int lanes);
u32  intel_mode_bandwidth_kbps(u32 pixel_khz, int bpp);
int  intel_dp_choose_rate(u32 pixel_khz, int lanes, int bpp);
int  intel_dpll_compute_hdmi(u32 pixel_khz);
u32  intel_wrpll_dco_khz(void);
u32  intel_wrpll_central(void);
int  intel_wrpll_p(void);
int  intel_wrpll_q(void);
int  intel_wrpll_k(void);
int  intel_wrpll_divider(void);
u32  intel_wrpll_cfgcr1(void);
u32  intel_wrpll_cfgcr2(void);
u32  intel_wrpll_actual_khz(void);
int  intel_dpcd_read(int port, u32 addr, int len);
int  intel_dpcd_byte(int i);
int  intel_dpcd_rev(void);
int  intel_dpcd_max_rate(void);
int  intel_dpcd_max_lanes(void);
int  intel_dpcd_enhanced(void);
int  intel_dpcd_tps3(void);
int  intel_dpcd_has_rate_table(void);
u32  intel_dpcd_max_rate_kbps(void);
int  intel_aux_last_reply(void);
int  intel_dp_choose_rate_ex(u32 pixel_khz, int lanes, int bpp, int allow_edp);
int  intel_dp_choose_rate_for_panel(u32 pixel_khz, int bpp, int maxrate, int maxlanes, int table);
u32  intel_dp_tp_ctl(int port);
u32  intel_dp_tp_status(int port);
void intel_link_train_arm(int on);
int  intel_link_train_armed(void);
int  intel_pp_status(void);
int  intel_pp_control(void);
int  intel_pp_t1_t3(void);
int  intel_pp_t8(void);
int  intel_pp_t9(void);
int  intel_pp_t10(void);
int  intel_pp_t11_t12(void);
int  intel_pp_sequencing(void);
u32  intel_pwr_well_driver(void);
u32  intel_pwr_well_bios(void);
u32  intel_dc_state(void);
int  intel_pwr_well_enabled(int w);
int  intel_pwr_well_requested(int w);
u32  intel_wm_trans(void);
u32  intel_ddb_cfg(void);
int  intel_wm_enabled(int level);
int  intel_wm_blocks(int level);
int  intel_wm_lines(int level);
int  intel_ddb_start(void);
int  intel_ddb_end(void);
int  intel_ddb_blocks(void);
u32  intel_wm_compute_level0(u32 w, u32 bpp, u32 khz, u32 lat);

static unsigned char edid_storage[256];

/* ---- what intel.c expects the kernel to provide ------------------------ */
static int cfg_fd = -1;

u32 host_cfg_read(int bus, int dev, int fn, int off)
{
    (void)bus; (void)dev; (void)fn;
    u32 v = 0;
    if (cfg_fd >= 0 && pread(cfg_fd, &v, 4, off) == 4) return v;
    return 0;
}

/* intel.c's timing helpers call this; on the host a monotonic 100 Hz stand-in
 * is enough, and it keeps the driver source identical. */
/* intel.c needs real timing now (cpu.c provides it in the kernel; here we are
 * a Linux process, so nanosleep is both simpler and more accurate than any
 * spin). Without these three the link fails at cpu_delay_us. */
void cpu_delay_us(unsigned int us)
{
    struct timespec ts = { (long)(us / 1000000u), (long)(us % 1000000u) * 1000L };
    while (nanosleep(&ts, &ts) == -1) { }
}
void cpu_delay_ms(unsigned int ms) { cpu_delay_us(ms * 1000u); }
unsigned int cpu_now_ms(void)
{
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return (unsigned int)(t.tv_sec * 1000ull + t.tv_nsec / 1000000ull);
}

u32 idt_ticks(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u32)(ts.tv_sec * 100 + ts.tv_nsec / 10000000);
}

/* the PCI functions intel.c declares but does not use on this path */
int  pci_count(void)                  { return 0; }
int  pci_vendor(int i)                { (void)i; return 0; }
int  pci_device(int i)                { (void)i; return 0; }
int  pci_class(int i)                 { (void)i; return 0; }
void pci_scan(void)                   { }
void pci_enable(int i)                { (void)i; }
u32  pci_bar(int i, int w)            { (void)i; (void)w; return 0; }
u32  pci_bar_size(int i, int w)       { (void)i; (void)w; return 0; }
u32  pci_read32(int b, int d, int f, int o) { return host_cfg_read(b, d, f, o); }

/* ---- helpers ----------------------------------------------------------- */
static u32 read_sysfs_hex(const char *path, int field)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    unsigned long long a = 0, b = 0, c = 0;
    if (fscanf(f, "%llx %llx %llx", &a, &b, &c) != 3) { fclose(f); return 0; }
    fclose(f);
    return (u32)(field == 0 ? a : (field == 1 ? b : c));
}

static const char *tiling_name(int t)
{
    switch (t) {
        case 0: return "linear";
        case 1: return "X tiled";
        case 4: return "Y tiled";
        case 5: return "Yf tiled";
    }
    return "?";
}

int main(int argc, char **argv)
{
    int unsafe = 0, dump = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--unsafe")) unsafe = 1;
        if (!strcmp(argv[i], "--dump"))   dump = 1;
    }

    if (!dump) {
        printf("zlOS Intel display driver - host harness\n");
        printf("========================================\n\n");
    }

    char path[256];
    snprintf(path, sizeof path, "%s/config", PCI_DEV);
    cfg_fd = open(path, O_RDONLY);
    if (cfg_fd < 0) { perror("open config"); return 1; }

    u32 devid = host_cfg_read(0, 2, 0, 0) >> 16;

    snprintf(path, sizeof path, "%s/resource0", PCI_DEV);
    int fd = open(path, unsafe ? O_RDWR : O_RDONLY);
    if (fd < 0) { perror("open resource0 (run me with sudo)"); return 1; }

    snprintf(path, sizeof path, "%s/resource", PCI_DEV);
    u32 bar0 = read_sysfs_hex(path, 0);
    /* 8 MiB, not the full 16. The kernel refuses to map the whole BAR while
     * i915 holds it, but every display register we care about lives in the
     * first megabyte. The GGTT at offset 0x800000 is the exception and is out
     * of reach here - it is reachable from the kernel, where nothing else
     * owns the device. */
    size_t bar0_len = 8u << 20;

    void *map = mmap(NULL, bar0_len, unsafe ? (PROT_READ | PROT_WRITE) : PROT_READ,
                     MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) { perror("mmap"); return 1; }

    /* --dump: every display register, one per line, for diffing. A modeset
     * touches a few dozen out of hundreds, and seeing exactly which ones is
     * how you learn a sequence you cannot otherwise observe. */
    if (dump) {
        volatile unsigned char *b = (volatile unsigned char *)map;
        struct { unsigned lo, hi; const char *name; } ranges[] = {
            /* to 0x50, not 0x40: PIPE_LINK_M1/N1 live at +0x40/+0x44 and the
             * pixel clock is derivable from them. Stopping at 0x40 hid the one
             * pair of registers that says what the link is actually carrying. */
            { 0x60000, 0x60050, "TRANS_A"   },
            { 0x6F000, 0x6F050, "TRANS_EDP" },
            { 0x6F400, 0x6F420, "DDI_EDP"   },
            { 0x64000, 0x64100, "DDI_BUF"   },
            { 0x6C000, 0x6C070, "DPLL"      },
            { 0x46000, 0x46020, "CDCLK"     },
            { 0x70000, 0x70200, "PIPE_A"    },
            /* to 0x70300: PLANE_WM(0..7) at 0x70240, PLANE_WM_TRANS at 0x70268
             * and PLANE_BUF_CFG at 0x7027C all sit past the old 0x70200 edge,
             * so the entire watermark block was invisible. */
            { 0x70200, 0x70300, "PIPE_A_WM" },
            { 0x68180, 0x68300, "SCALER"    },
            { 0x45260, 0x45280, "WM_LINET"  },
            { 0x420C0, 0x420D0, "CHICKEN"   },
            { 0x7F000, 0x7F020, "PIPE_EDP"  },
            { 0xC5100, 0xC5130, "GMBUS"     },
            { 0xC7200, 0xC7220, "PANEL"     },
            { 0xC8250, 0xC8260, "BACKLIGHT" },
            { 0, 0, 0 }
        };
        for (int r = 0; ranges[r].name; r++)
            for (unsigned o = ranges[r].lo; o < ranges[r].hi; o += 4)
                printf("%-10s %06X %08X\n", ranges[r].name, o,
                       *(volatile unsigned *)(b + o));
        munmap(map, bar0_len);
        return 0;
    }

    printf("  BAR0 physical   0x%08X, mapped at %p, %zu MiB\n",
           bar0, map, bar0_len >> 20);
    printf("  device id       0x%04X\n", devid);
    printf("  mode            %s\n\n", unsafe ? "READ/WRITE (--unsafe)" : "read-only");

    /* hand the mapping to the driver - from here on it is zlOS's code */
    intel_attach((uptr)map, (u32)bar0_len, 0, 0, (int)devid, host_cfg_read);
    intel_set_edid_buffer((uptr)edid_storage);

    printf("  supported       %s\n", intel_supported() ? "yes - Gen9/9.5" : "NO");
    printf("  stolen memory   %u MiB at 0x%08X\n",
           intel_stolen_size() >> 20, intel_stolen_base());
    printf("  GGTT size       %u MiB\n\n", intel_ggtt_size() >> 20);

    printf("  -- live display state --\n");
    printf("  pipe A enabled  %d\n", intel_pipe_enabled());
    printf("  plane enabled   %d\n", intel_plane_enabled());
    printf("  panel powered   %d\n", intel_panel_on());
    printf("  resolution      %d x %d\n", intel_pipe_width(), intel_pipe_height());
    printf("  stride          %d bytes\n", intel_stride());
    printf("  plane format    0x%X, %s\n", intel_plane_format(),
           tiling_name(intel_plane_tiling()));
    printf("  scanout address 0x%08X\n", intel_surface());
    printf("  PLANE_CTL       0x%08X\n", intel_plane_ctl());
    printf("  TRANS_DDI_EDP   0x%08X\n\n", intel_ddi_func_ctl());

    printf("  -- timing generator --\n");
    printf("  horizontal      %d active of %d total\n", intel_hactive(), intel_htotal());
    printf("  vertical        %d active of %d total\n", intel_vactive(), intel_vtotal());
    printf("  hsync           %d .. %d\n", intel_hsync_start(), intel_hsync_end());
    printf("  vsync           %d .. %d\n", intel_vsync_start(), intel_vsync_end());

    /* measure refresh the same way the kernel does: count frames over time */
    int f0 = intel_frame_count();
    usleep(500000);
    int f1 = intel_frame_count();
    double hz = (f1 - f0) * 2.0;
    printf("  frame counter   %d -> %d in 0.5 s  =  %.1f Hz%s\n", f0, f1, hz,
           hz == 0.0 ? "   <- frozen: PSR is on" : "");

    /* The frame counter is the honest measurement and it reads zero here,
     * because firmware leaves PSR enabled and a self-refreshing panel does not
     * advance it. PIPE_LINK_M1/N1 give the answer exactly and without a timer -
     * they hold pixel_clock : link_clock, which is what a fixed-rate link needs
     * in order to carry an arbitrary mode. */
    u32 mn_khz = intel_pixel_clock_mn_khz();
    printf("  pixel clock     %u kHz   (from PIPE_LINK_M1/N1 - exact, PSR-immune)\n", mn_khz);
    printf("  refresh         %u.%03u Hz (derived, not counted)\n",
           intel_refresh_mhz_derived() / 1000u, intel_refresh_mhz_derived() % 1000u);
    if (intel_htotal() && intel_vtotal() && hz > 0.0)
        printf("  cross-check     %.1f MHz from the frame counter\n",
               (double)intel_htotal() * intel_vtotal() * hz / 1e6);
    printf("\n");

    printf("  -- DPLLs: the part a modeset lives or dies on --\n");
    printf("  transcoder      %s\n", intel_transcoder_is_edp() ? "eDP (internal panel)" : "A");
    printf("  DPLL_CTRL1      0x%08X\n", intel_dpll_ctrl1());
    printf("  DPLL_CTRL2      0x%08X\n", intel_dpll_ctrl2());
    printf("  DPLL_STATUS     0x%08X\n", intel_dpll_status());
    printf("  CDCLK_CTL       0x%08X\n", intel_cdclk_ctl());
    for (int p = 0; p < 4; p++) {
        int rate = intel_dpll_link_rate(p);
        printf("    DPLL%d  lock=%d", p, intel_dpll_locked(p));
        if (rate >= 0) {
            printf("  rate_idx=%d (%u kHz)  ssc=%d  hdmi=%d",
                   rate, intel_dpll_rate_khz(rate), intel_dpll_ssc(p), intel_dpll_is_hdmi(p));
            if (intel_dpll_is_hdmi(p))
                printf("  cfgcr1=%08X cfgcr2=%08X",
                       intel_dpll_cfgcr1(p), intel_dpll_cfgcr2(p));
        } else printf("  (not configured)");
        printf("\n");
    }
    static const char *ddi_name[5] = { "A", "B", "C", "D", "E" };
    for (int d = 0; d < 5; d++)
        printf("    DDI %s  clock_off=%d  dpll=%d\n",
               ddi_name[d], intel_ddi_clock_off(d), intel_ddi_clock_select(d));
    printf("\n");

    /* ---- the real test: does our maths agree with the running hardware? --
     * The panel is on, so the correct answer is readable. If the driver
     * computes the same link rate i915 chose, the DP path is right. */
    printf("  -- DPLL computation vs. what i915 actually programmed --\n");
    {
        /* No more "nominal 60 Hz". The pixel clock is read out of the link
         * ratio now, so every number below rests on a measurement. */
        u32 pixel_khz = intel_pixel_clock_khz();
        int live = intel_dpll_link_rate(0);
        printf("  pixel clock           %u kHz  (measured, was assumed 60 Hz)\n", pixel_khz);

        for (int bpp = 18; bpp <= 30; bpp += 6) {
            int want = intel_dp_choose_rate(pixel_khz, 4, bpp);
            printf("    %d bpp, 4 lanes -> rate index %d", bpp, want);
            if (want >= 0)
                printf("  (%u kHz DPLL, %u kbps link vs %u kbps needed)",
                       intel_dpll_rate_khz(want),
                       intel_dp_link_bandwidth_kbps(want, 4),
                       intel_mode_bandwidth_kbps(pixel_khz, bpp));
            printf("\n");
        }
        printf("  i915 programmed DPLL0 at rate index %d\n", live);
        int ours = intel_dp_choose_rate(pixel_khz, 4, 24);
        printf("  our choice at 24 bpp      rate index %d   -> %s\n",
               ours, ours == live ? "MATCH" : "differs");

        /* Is there anything slower that still carries this mode? The plan says
         * no, and says it matters: a driver that walks the rate ladder on a
         * training failure will "succeed" onto a link too slow for a frame. */
        u32 need = intel_mode_bandwidth_kbps(pixel_khz, 24);
        printf("  need %u kbps at 24 bpp;  2xHBR %u, 4xRBR %u  -> %s\n",
               need, intel_dp_link_bandwidth_kbps(1, 2), intel_dp_link_bandwidth_kbps(2, 4),
               (intel_dp_link_bandwidth_kbps(1, 2) < need &&
                intel_dp_link_bandwidth_kbps(2, 4) < need)
                   ? "4xHBR is the only working point, as the plan says"
                   : "a fallback exists - the plan's step 26 needs revisiting");
    }
    printf("\n");

    /* ---- M/N against firmware: a write path with a known-correct answer ----
     * PIPE_DATA_M1/N1 and PIPE_LINK_M1/N1 have never been written by this
     * driver - the code did not exist until now. But firmware has already
     * solved the identical problem for the identical mode, and left its answer
     * in the registers. So the computation can be checked exactly, which is a
     * far stronger claim than "it compiles and looks plausible". */
    printf("  -- M/N computation vs. what firmware left in the registers --\n");
    {
        u32 pixel_khz = intel_pixel_clock_khz();
        int lanes = intel_ddi_lanes();
        int bpp   = intel_ddi_bpp();
        u32 link_khz = intel_dp_link_symbol_khz(intel_dpll_link_rate(intel_ddi_clock_select(0)));

        printf("  inputs: pixel %u kHz, link %u kHz symbol, %d lanes, %d bpp\n",
               pixel_khz, link_khz, lanes, bpp);

        if (!intel_mn_compute(pixel_khz, link_khz, lanes, bpp)) {
            printf("  [ FAIL ] intel_mn_compute() rejected its inputs\n");
        } else {
            struct { const char *name; u32 got, want; } chk[] = {
                { "DATA_M1 (with TU)", intel_data_m1_reg(), (63u << 25) | intel_mn_data_m() },
                { "DATA_N1",           intel_data_n1_reg(), intel_mn_data_n() },
                { "LINK_M1",           intel_link_m1_reg(), intel_mn_link_m() },
                { "LINK_N1",           intel_link_n1_reg(), intel_mn_link_n() },
                { 0, 0, 0 }
            };
            int bad = 0;
            for (int i = 0; chk[i].name; i++) {
                int match = chk[i].got == chk[i].want;
                if (!match) bad++;
                printf("    %-18s firmware %08X   ours %08X   %s\n",
                       chk[i].name, chk[i].got, chk[i].want, match ? "MATCH" : "DIFFERS");
            }
            printf("  %s\n", bad == 0
                   ? "  all four exact - the M/N algorithm is right on this hardware"
                   : "  a mismatch here means the ratio maths is wrong, not the panel");
        }
    }
    printf("\n");

    /* ---- the rest of the pipe path, same method: compute, compare ---------
     * Every register here had no code at all until now. Firmware has all of
     * them set for a working 2560x1440, so each computation has a real answer
     * to be wrong against. */
    printf("  -- transcoder/pipe registers vs. firmware --\n");
    {
        u32 pixel_khz = intel_pixel_clock_khz();
        int lanes = intel_ddi_lanes(), bpp = intel_ddi_bpp();
        /* sync polarity as firmware has it, so we reproduce the same word */
        u32 live_ddi = intel_ddi_func_ctl();
        int phsync = !!(live_ddi & (1u << 16)), pvsync = !!(live_ddi & (1u << 17));

        struct { const char *name; u32 got, want; } chk[] = {
            { "TRANS_DDI_FUNC_CTL", live_ddi,
              intel_trans_ddi_ctl_value(lanes, bpp, phsync, pvsync) },
            { "TRANS_MSA_MISC",     intel_msa_misc(),    intel_msa_misc_value(bpp) },
            { "PIPE_MISC",          intel_pipe_misc(),   intel_pipe_misc_value(bpp, 0) },
            { "WM_LINETIME",        intel_wm_linetime(),
              intel_wm_linetime_value((u32)intel_htotal(), pixel_khz) },
            { 0, 0, 0 }
        };
        int bad = 0;
        for (int i = 0; chk[i].name; i++) {
            int match = chk[i].got == chk[i].want;
            if (!match) bad++;
            printf("    %-19s firmware %08X   ours %08X   %s\n",
                   chk[i].name, chk[i].got, chk[i].want, match ? "MATCH" : "DIFFERS");
        }
        printf("    %-19s %08X   %s\n", "CHICKEN_TRANS_EDP", intel_chicken_trans(),
               (intel_chicken_trans() & (3u << 27)) ? "frame start delay NOT reset"
                                                   : "frame start delay 1 (reset)");
        printf("    %-19s scaler1 %s  scaler2 %s\n", "PS_CTRL",
               intel_scaler_enabled(1) ? "ON" : "off",
               intel_scaler_enabled(2) ? "ON" : "off");
        printf("  %s\n", bad == 0 ? "  all four exact"
                                  : "  a DIFFERS above is our maths, not the panel");
    }
    printf("\n");

    /* ---- the DDB split, where the plan and the hardware disagree ---------- */
    printf("  -- display data buffer split (cursor watermarks were absent) --\n");
    {
        u32 p = intel_ddb_cfg(), c = intel_cur_buf_cfg();
        printf("    plane  blocks %4u..%-4u  (PLANE_BUF_CFG %08X)\n",
               p & 0xFFF, (p >> 16) & 0xFFF, p);
        printf("    cursor blocks %4u..%-4u  (CUR_BUF_CFG   %08X)\n",
               c & 0xFFF, (c >> 16) & 0xFFF, c);
        printf("    CUR_WM(0) %08X -> %u blocks, %u lines%s\n", intel_cur_wm(0),
               intel_cur_wm(0) & 0x3FF, (intel_cur_wm(0) >> 14) & 0x1F,
               (intel_cur_wm(0) & 0x3FF) > 8 ? "   <- plan guessed 8" : "");
        printf("  %s\n", intel_ddb_valid()
               ? "  disjoint and within 0..891 - valid"
               : "  OVERLAPPING or past block 891 (hazard 4.3 #14)");

        /* Does our split arithmetic reproduce firmware's, given the same
         * cursor allocation? Values only - nothing is written. */
        int cur_blocks = (int)(((c >> 16) & 0xFFF) - (c & 0xFFF) + 1);
        u32 ours_c = intel_ddb_cur_cfg_value(cur_blocks);
        u32 ours_p = intel_ddb_plane_cfg_value(cur_blocks);
        printf("    for %d cursor blocks, ours: CUR %08X %s   PLANE %08X %s\n",
               cur_blocks, ours_c, ours_c == c ? "MATCH" : "DIFFERS",
               ours_p, ours_p == p ? "MATCH" : "DIFFERS");
    }
    printf("\n");

    /* ---- backlight: the layout this code had wrong ------------------------ */
    printf("  -- backlight (CNP/CMP: freq and duty are SEPARATE registers) --\n");
    {
        u32 period = intel_backlight_max(), duty = intel_backlight_get();
        printf("    PWM enabled     %s\n", intel_backlight_pwm_enabled() ? "yes" : "no");
        printf("    period          %u clocks of 24 MHz  = %u Hz\n",
               period, period ? 24000000u / period : 0);
        printf("    duty            %u  = %u%% brightness\n",
               duty, period ? (100u * duty) / period : 0);
        if (!period)
            printf("    [ FAIL ] period reads 0 - the SKL packed layout is back\n");
        else if (duty > period)
            printf("    [ FAIL ] duty exceeds period\n");
        else
            printf("    [ PASS ] a sane period and a duty inside it\n");
    }
    printf("\n");

    /* the HDMI divider search, checked against clocks whose answers are known */
    printf("  -- HDMI divider search (a few standard modes) --\n");
    {
        struct { u32 khz; const char *name; } modes[] = {
            { 25175,  "640x480@60"    },
            { 74250,  "1280x720@60"   },
            { 148500, "1920x1080@60"  },
            { 241500, "2560x1440@60"  },
            { 297000, "3840x2160@30"  },
            { 594000, "3840x2160@60"  },
            { 0, 0 }
        };
        for (int i = 0; modes[i].name; i++) {
            if (!intel_dpll_compute_hdmi(modes[i].khz)) {
                printf("    %-16s %6u kHz  NO SOLUTION\n", modes[i].name, modes[i].khz);
                continue;
            }
            u32 got = intel_wrpll_actual_khz();
            long err = (long)got - (long)modes[i].khz;
            printf("    %-16s %6u kHz  dco=%u div=%d (p=%d q=%d k=%d)  cfgcr1=%08X cfgcr2=%08X  err=%+ld kHz\n",
                   modes[i].name, modes[i].khz, intel_wrpll_dco_khz(),
                   intel_wrpll_divider(), intel_wrpll_p(), intel_wrpll_q(),
                   intel_wrpll_k(), intel_wrpll_cfgcr1(), intel_wrpll_cfgcr2(), err);
        }
    }
    printf("\n");

    if (unsafe) {
        printf("  -- talking to the PANEL over the AUX channel --\n");
        int n = intel_dpcd_read(0, 0x00000, 16);   /* DDI A = port 0 */
        if (n > 0) {
            printf("  DPCD read OK    %d bytes, reply=%d\n", n, intel_aux_last_reply());
            printf("  DPCD revision   %d.%d\n",
                   intel_dpcd_rev() >> 4, intel_dpcd_rev() & 0xF);
            int mr = intel_dpcd_max_rate();
            const char *rn = mr == 0x06 ? "1.62 Gbps (RBR)" :
                             mr == 0x0A ? "2.70 Gbps (HBR)" :
                             mr == 0x14 ? "5.40 Gbps (HBR2)" :
                             mr == 0x1E ? "8.10 Gbps (HBR3)" : "?";
            printf("  MAX_LINK_RATE   0x%02X = %s\n", mr, rn);
            printf("  MAX_LANE_COUNT  %d%s\n", intel_dpcd_max_lanes(),
                   intel_dpcd_enhanced() ? "  (enhanced framing)" : "");
            printf("  TPS3 supported  %d\n", intel_dpcd_tps3());
            printf("  rate table      %s\n",
                   intel_dpcd_has_rate_table()
                     ? "YES - eDP 1.4, intermediate rates are legal here"
                     : "no - STANDARD RATES ONLY");
            printf("  raw: ");
            for (int i = 0; i < 16; i++) printf("%02X ", intel_dpcd_byte(i));
            printf("\n");

            /* now the question that actually matters for the modeset */
            u32 nominal = (u32)((double)intel_htotal() * intel_vtotal() * 60.0 / 1000.0);
            int allow = intel_dpcd_has_rate_table();
            (void)allow;
            int pick  = intel_dp_choose_rate_for_panel(nominal, 24,
                            intel_dpcd_max_rate(), intel_dpcd_max_lanes(),
                            intel_dpcd_has_rate_table());
            printf("\n  with the panel's REAL capabilities:\n");
            printf("    %u kHz, %d lanes, 24 bpp -> rate index %d\n",
                   nominal, intel_dpcd_max_lanes(), pick);
            printf("    (clamped to the panel's 0x%02X max and its rate-table policy)\n",
                   intel_dpcd_max_rate());
            printf("    i915 chose index %d  ->  %s\n",
                   intel_dpll_link_rate(0),
                   pick == intel_dpll_link_rate(0) ? "MATCH" : "differs");
        } else {
            printf("  DPCD read FAILED (reply=%d)\n", intel_aux_last_reply());
            printf("  i915 is probably holding the AUX channel - try\n");
            printf("    sudo ./gpu-dev.sh detach\n");
        }
        /* Read back the link's own training state. If these offsets are
         * wrong the values are garbage; a trained link has a very specific
         * signature, which is what makes this a real check. */
        u32 tp = intel_dp_tp_ctl(0);
        printf("\n  DP_TP_CTL(A)    0x%08X\n", tp);
        printf("    enable=%u  mode=%s  enhanced_frame=%u  pattern=%u (%s)\n",
               (tp >> 31) & 1, ((tp >> 27) & 1) ? "MST" : "SST",
               (tp >> 18) & 1, (tp >> 8) & 7,
               ((tp >> 8) & 7) == 0 ? "TPS1" : ((tp >> 8) & 7) == 1 ? "TPS2" :
               ((tp >> 8) & 7) == 2 ? "idle" : ((tp >> 8) & 7) == 3 ? "NORMAL - link is up" :
               ((tp >> 8) & 7) == 4 ? "TPS3" : "?");
        printf("  DP_TP_STATUS(A) 0x%08X\n", intel_dp_tp_status(0));

        /* and the link configuration the panel currently has */
        if (intel_dpcd_read(0, 0x100, 8)) {
            printf("  panel link cfg  BW=0x%02X (%u kHz link)  lanes=%d%s  pattern=0x%02X\n",
                   intel_dpcd_byte(0), intel_dpcd_byte(0) * 270000,
                   intel_dpcd_byte(1) & 0x1F,
                   (intel_dpcd_byte(1) & 0x80) ? " enhanced" : "",
                   intel_dpcd_byte(2));
        }
        if (intel_dpcd_read(0, 0x202, 6)) {
            printf("  lane status     %02X %02X  align=%02X  adjust=%02X %02X\n",
                   intel_dpcd_byte(0), intel_dpcd_byte(1), intel_dpcd_byte(2),
                   intel_dpcd_byte(4), intel_dpcd_byte(5));
            int l0 = intel_dpcd_byte(0) & 0xF;
            printf("    lane0: CR=%d EQ=%d SYM=%d   interlane align=%d\n",
                   l0 & 1, (l0 >> 1) & 1, (l0 >> 2) & 1, intel_dpcd_byte(2) & 1);
        }
        printf("\n");
    }

    printf("  -- power wells (a register in a down well reads ZERO) --\n");
    printf("  PWR_WELL_CTL_DRIVER 0x%08X\n", intel_pwr_well_driver());
    printf("  PWR_WELL_CTL_BIOS   0x%08X\n", intel_pwr_well_bios());
    printf("  DC_STATE_EN         0x%08X\n", intel_dc_state());
    for (int w = 0; w < 3; w++)
        printf("    well %d  requested=%d  actually up=%d\n",
               w, intel_pwr_well_requested(w), intel_pwr_well_enabled(w));
    printf("\n");

    printf("  -- panel power sequencing (delays in 100us units) --\n");
    printf("  PP_STATUS       0x%08X   on=%d  sequencing=%d\n",
           intel_pp_status(), (intel_pp_status() >> 31) & 1, intel_pp_sequencing());
    printf("  PP_CONTROL      0x%08X\n", intel_pp_control());
    printf("  T1+T3 (power up to video)   %5d = %d.%d ms\n",
           intel_pp_t1_t3(), intel_pp_t1_t3()/10, intel_pp_t1_t3()%10);
    printf("  T8    (video to backlight)  %5d = %d.%d ms\n",
           intel_pp_t8(), intel_pp_t8()/10, intel_pp_t8()%10);
    printf("  T9    (backlight off first) %5d = %d.%d ms\n",
           intel_pp_t9(), intel_pp_t9()/10, intel_pp_t9()%10);
    printf("  T10   (video off to vdd)    %5d = %d.%d ms\n",
           intel_pp_t10(), intel_pp_t10()/10, intel_pp_t10()%10);
    printf("  T11+T12 (cycle delay)        %d ms minimum before re-power\n",
           intel_pp_t11_t12());
    printf("\n");

    printf("  -- backlight --\n");
    u32 blmax = intel_backlight_max();
    if (blmax)
        printf("  PWM             %u of %u  (%u%%)\n",
               intel_backlight_get(), blmax, intel_backlight_get() * 100 / blmax);
    else
        printf("  PWM             not reported here\n");
    printf("\n");

    printf("  -- watermarks and the display data buffer --\n");
    printf("  DDB allocation  blocks %d..%d  (%d blocks of 512 bytes = %d KiB)\n",
           intel_ddb_start(), intel_ddb_end(), intel_ddb_blocks(),
           intel_ddb_blocks() / 2);
    for (int l = 0; l < 8; l++)
        printf("    level %d  %s  blocks=%-4d lines=%d\n", l,
               intel_wm_enabled(l) ? "ON " : "off",
               intel_wm_blocks(l), intel_wm_lines(l));
    printf("    transition   0x%08X\n", intel_wm_trans());
    {
        u32 nominal = (u32)((double)intel_htotal() * intel_vtotal() * 60.0 / 1000.0);
        u32 ours = intel_wm_compute_level0((u32)intel_pipe_width(), 32, nominal, 0);
        printf("  our conservative level 0: blocks=%u lines=%u (firmware: %d / %d)\n",
               ours & 0x3FF, (ours >> 14) & 0x1F,
               intel_wm_blocks(0), intel_wm_lines(0));
    }
    printf("\n");

    if (unsafe) {
        printf("  -- EDID over GMBUS (needs writes, hence --unsafe) --\n");
        int pin = intel_read_edid();
        if (pin > 0) {
            printf("  found on pin    %d\n", pin);
            printf("  manufacturer    %c%c%c  product 0x%04X\n",
                   intel_edid_vendor_char(0), intel_edid_vendor_char(1),
                   intel_edid_vendor_char(2), intel_edid_product());
            printf("  native mode     %d x %d\n",
                   intel_edid_native_w(), intel_edid_native_h());
            printf("  physical size   %d x %d mm\n",
                   intel_edid_width_mm(), intel_edid_height_mm());
        } else {
            printf("  no EDID on any pin pair (i915 may be holding the bus)\n");
        }
    } else {
        printf("  (EDID needs GMBUS writes - re-run with --unsafe)\n");
    }

    munmap(map, bar0_len);
    close(fd);
    close(cfg_fd);
    return 0;
}
