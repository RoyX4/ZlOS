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
            { 0x60000, 0x60040, "TRANS_A"   },
            { 0x6F000, 0x6F040, "TRANS_EDP" },
            { 0x6F400, 0x6F420, "DDI_EDP"   },
            { 0x64000, 0x64100, "DDI_BUF"   },
            { 0x6C000, 0x6C070, "DPLL"      },
            { 0x46000, 0x46020, "CDCLK"     },
            { 0x70000, 0x70200, "PIPE_A"    },
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
    printf("  frame counter   %d -> %d in 0.5 s  =  %.1f Hz\n", f0, f1, hz);
    if (intel_htotal() && intel_vtotal())
        printf("  pixel clock     %.1f MHz\n",
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
        double hz2 = (f1 - f0) * 2.0;
        u32 pixel_khz = (u32)((double)intel_htotal() * intel_vtotal() * hz2 / 1000.0);
        /* the frame counter under-reads when the panel self-refreshes, so also
         * compute from the nominal 60 Hz the mode implies */
        u32 nominal_khz = (u32)((double)intel_htotal() * intel_vtotal() * 60.0 / 1000.0);
        printf("  measured pixel clock  %u kHz   (frame counter may under-read: PSR)\n", pixel_khz);
        printf("  at a nominal 60 Hz    %u kHz\n", nominal_khz);

        for (int bpp = 18; bpp <= 30; bpp += 6) {
            int want = intel_dp_choose_rate(nominal_khz, 4, bpp);
            printf("    %d bpp, 4 lanes -> rate index %d", bpp, want);
            if (want >= 0)
                printf("  (%u kHz symbol, %u kbps link vs %u kbps needed)",
                       intel_dpll_rate_khz(want),
                       intel_dp_link_bandwidth_kbps(want, 4),
                       intel_mode_bandwidth_kbps(nominal_khz, bpp));
            printf("\n");
        }
        int live = intel_dpll_link_rate(0);
        printf("  i915 programmed DPLL0 at rate index %d\n", live);
        int ours = intel_dp_choose_rate(nominal_khz, 4, 24);
        printf("  our choice at 24 bpp      rate index %d   -> %s\n",
               ours, ours == live ? "MATCH" : "differs");
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

    printf("  -- backlight --\n");
    u32 blmax = intel_backlight_max();
    if (blmax)
        printf("  PWM             %u of %u  (%u%%)\n",
               intel_backlight_get(), blmax, intel_backlight_get() * 100 / blmax);
    else
        printf("  PWM             not reported here\n");
    printf("\n");

    printf("  -- watermarks (plane 1, pipe A) --\n  ");
    for (int l = 0; l < 8; l++) printf("L%d=%08X ", l, intel_watermark(l));
    printf("\n\n");

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
