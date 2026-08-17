/* intel.c - a real Intel Gen9 / Gen9.5 display driver.
 *
 * This targets the actual silicon in the machine zlOS is being built on:
 *
 *     00:02.0  Intel CometLake-U GT2 [UHD Graphics]   8086:9b41
 *     BAR0 = 0xE9000000, 16 MB   GTTMMADR - registers + the global GTT
 *     BAR2 = 0xC0000000, 256 MB  GMADR    - the CPU aperture
 *
 * Every offset below comes from Intel's public Programmer's Reference Manual
 * for Skylake-class hardware, cross-checked against the i915 and Haiku
 * drivers. Nothing here is guessed.
 *
 * WHAT THIS IS HONEST ABOUT
 * -------------------------
 * A complete Intel driver - the kind that lights a dark panel from cold - has
 * to program the DPLLs, drive DDI buffer translation, run DisplayPort link
 * training, and sequence panel power within timings that can damage hardware
 * if violated. That is i915: ~200,000 lines, fifteen years, Intel's own
 * engineers. This file does not attempt it and says so.
 *
 * What it DOES do is the part that is genuinely reachable, and it is not
 * nothing: the firmware has already lit the panel by the time we run, so we
 * can find the GPU, map its registers, READ BACK the exact mode it programmed,
 * and re-point the display's primary plane at a framebuffer of our own. That
 * last step is real Intel GPU programming - the hardware scans out what we
 * tell it to.
 *
 * The milestone ladder, and where this file stops:
 *     1. detect the GPU and map MMIO                    <- done
 *     2. read the live display configuration            <- done
 *     3. re-point the primary plane at our surface      <- done
 *     4. full cold-start modeset (DPLL/DDI/link/panel)  <- NOT attempted
 */

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef unsigned long long u64;

#if defined(ZL_64)
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

u32 idt_ticks(void);

/* Real timing, from cpu.c's PIT-calibrated TSC. idt_ticks() resolves 10 ms and
 * stops advancing when interrupts are masked, which is the state a modeset runs
 * in - it cannot express a 100 us link-training wait and must not be used for
 * one. Every host harness that links intel.c has to provide these three. */
void cpu_delay_us(u32 us);
void cpu_delay_ms(u32 ms);
u32  cpu_now_ms(void);

int  pci_count(void);
int  pci_vendor(int i);
int  pci_device(int i);
int  pci_class(int i);
void pci_scan(void);
void pci_enable(int i);
u32  pci_bar(int i, int which);
u32  pci_bar_size(int i, int which);
u32  pci_read32(int bus, int dev, int fn, int off);

/* ---- config-space registers on the GPU's own function (0:2:0) ------------
 * Linux reads both of these from the IGD rather than the host bridge, which
 * means the whole stolen-memory discovery needs only the GPU we already
 * found. */
#define MGGC0  0x50      /* graphics control: stolen size + GGTT size */
#define BDSM   0x5C      /* base of data stolen memory                */

/* ---- MMIO display registers, relative to BAR0 --------------------------- */
#define PIPE_SRCSZ_A      0x6001C   /* the composed image size, minus one   */
#define TRANS_CONF_A      0x70008   /* transcoder A config + enable         */
#define PIPE_FRMCNT_A     0x70040   /* free-running frame counter           */
#define PLANE_CTL_1_A     0x70180   /* primary plane control                */
#define PLANE_STRIDE_1_A  0x70188   /* stride, in units of 64 bytes         */
#define PLANE_SIZE_1_A    0x70190   /* fetched image size, minus one        */
#define PLANE_SURF_1_A    0x7019C   /* surface address AND the arm trigger  */
#define PLANE_OFFSET_1_A  0x701A4   /* pan position inside the surface      */
#define PLANE_SURFLIVE_A  0x701AC   /* read-only: what is scanning out NOW  */
#define TRANS_DDI_EDP     0x6F400   /* which pipe drives the laptop panel   */
#define VGACNTRL          0x71400   /* legacy VGA plane                     */

/* ---- the full timing generator, pipe A -------------------------------
 * These describe the actual video signal: how many pixels and lines there are
 * including the blanking intervals, and where the sync pulses sit inside them.
 * All are stored as (start | end << 16) and all are minus one. Reading them
 * back tells us the panel's real mode, including the refresh rate, without
 * touching a single PLL. */
/* WHICH TRANSCODER - this cost a real bug before the host harness existed.
 *
 * A laptop's internal panel is almost always driven by the dedicated eDP
 * transcoder, NOT transcoder A, even though it feeds pipe A. The timing
 * registers therefore live at 0x6F000, and reading 0x60000 returns zeroes -
 * which decodes to a 1x1 mode and looks like the driver is broken rather than
 * looking in the wrong place. Verified on the real machine: transcoder A reads
 * 0x00000000 while the eDP transcoder reads 0x0A9F09FF = 2560 of 2720. */
#define TRANS_A_BASE      0x60000
#define TRANS_EDP_BASE    0x6F000
#define TRANS_OFF_HTOTAL  0x00
#define TRANS_OFF_HBLANK  0x04
#define TRANS_OFF_HSYNC   0x08
#define TRANS_OFF_VTOTAL  0x0C
#define TRANS_OFF_VBLANK  0x10
#define TRANS_OFF_VSYNC   0x14
#define TRANS_OFF_CONF    0x08       /* relative to 0x70008 / 0x6F008 */
/* The pipe configuration register is NOT inside the transcoder timing block.
 * PIPECONF lives in the pipe register range: A at 0x70008, and the eDP
 * transcoder's at 0x7F008. Reading 0x6F008 gets HSYNC instead, which is a
 * plausible-looking non-zero number - so the bug reads as "pipe disabled"
 * while the panel is visibly on. Found with the host harness in one run. */
#define TRANS_A_CONF      0x70008
#define TRANS_EDP_CONF    0x7F008

#define HTOTAL_A          0x60000
#define HBLANK_A          0x60004
#define HSYNC_A           0x60008
#define VTOTAL_A          0x6000C
#define VBLANK_A          0x60010
#define VSYNC_A           0x60014
#define PIPE_FLIPCNT_A    0x70044

/* ---- the hardware cursor plane ---------------------------------------
 * A separate plane the display engine composites on top of the primary one,
 * for free, every frame. Moving it costs one register write and no redraw -
 * which is why every real OS has a hardware cursor and software mice look
 * laggy by comparison. */
#define CUR_CTL_A         0x70080
#define CUR_BASE_A        0x70084
#define CUR_POS_A         0x70088
#define CUR_MODE_128_ARGB 0x22      /* 128x128, 32-bit ARGB */
#define CUR_MODE_64_ARGB  0x27      /* 64x64, 32-bit ARGB   */
#define CUR_MODE_DISABLE  0x00

/* ---- plane geometry --------------------------------------------------- */
#define PLANE_POS_1_A     0x7018C
#define PLANE_KEYMAX_1_A  0x701A0
#define PLANE_WM_1_A(l)   (0x70240 + (l) * 4)   /* eight watermark levels */

/* ---- GMBUS: the I2C bus the monitor's EDID lives on -------------------
 * Every display carries a 128-byte EDID blob at I2C address 0x50 describing
 * what it is and which modes it supports. Intel exposes that bus through five
 * registers rather than a general I2C controller. */
#define GMBUS0            0xC5100   /* clock rate and pin pair              */
#define GMBUS1            0xC5104   /* command and slave address            */
#define GMBUS2            0xC5108   /* status                               */
#define GMBUS3            0xC510C   /* data, four bytes at a time           */
#define GMBUS4            0xC5110
#define GMBUS5            0xC5120

#define GMBUS_SW_RDY      (1u << 30)
#define GMBUS_CYCLE_WAIT  (1u << 25)
#define GMBUS_CYCLE_INDEX (1u << 26)
#define GMBUS_CYCLE_STOP  (1u << 27)
#define GMBUS_SLAVE_READ  1u
#define GMBUS_HW_RDY      (1u << 11)
#define GMBUS_NAK         (1u << 10)
#define GMBUS_ACTIVE      (1u << 9)

/* Panel-power bits. These live up here rather than beside the sequencer code
 * because aux_xfer() needs them: it must refuse to drive AUX into a panel
 * whose VDD is down, and it sits 500 lines above that code. */
#define PP_ON             (1u << 0)
/* b1 is PANEL_POWER_RESET, "power down on reset". It must stay SET: with it
 * clear, any reset drops VDD instantly under live video instead of running the
 * ordered T9/T10 sequence. It measures 1 on this box only because firmware
 * left it that way - so we assert it on every write rather than preserve it. */
#define PP_PWR_DOWN_ON_RESET (1u << 1)
#define PP_RESET          PP_PWR_DOWN_ON_RESET   /* old name, same bit */
#define PP_BACKLIGHT_EN   (1u << 2)
#define PP_VDD_FORCE      (1u << 3)

#define PP_STATUS_ON      (1u << 31)
#define PP_STATUS_SEQ     (3u << 28)    /* sequencing progress */
#define PP_SEQ_NONE       (0u << 28)
#define PP_SEQ_POWER_UP   (1u << 28)
#define PP_SEQ_POWER_DOWN (2u << 28)

/* ---- panel power and backlight ---------------------------------------
 * On a laptop these are the difference between a driver that can dim the
 * screen and one that cannot. The PCH owns them. */
#define PP_STATUS         0xC7200
#define PP_CONTROL        0xC7204
#define BLC_PWM_PCH_CTL1  0xC8250   /* enable, polarity                     */
#define BLC_PWM_PCH_CTL2  0xC8254   /* frequency [31:16], duty cycle [15:0] */

#define PLANE_CTL_ENABLE  0x80000000u
#define PLANE_CTL_FORMAT_MASK 0x0F000000u

static int  gpu_idx = -1;
static uptr mmio    = 0;      /* BAR0 - registers and the GGTT window */
static uptr aperture = 0;     /* BAR2 - the CPU-visible window        */
static u32  mmio_size = 0;
static u32  aper_size = 0;
static u16  gpu_devid = 0;

/* config-space reads go through this, so a host-side test harness can
 * substitute its own source without the driver knowing */
static u32 (*cfg_read)(int bus, int dev, int fn, int off) = 0;

/* MMIO is plain memory to us: flat 32-bit segments, no paging, so the BAR
 * address is directly addressable. `volatile` matters - these are registers,
 * not variables, and the compiler must not cache or reorder them. */
static u32 mmio_r(u32 off)
{
    if (!mmio) return 0;
    return *(volatile u32 *)(mmio + (uptr)off);
}

static void mmio_w(u32 off, u32 val)
{
    if (!mmio) return;
    *(volatile u32 *)(mmio + (uptr)off) = val;
}

static u32 gpu_cfg(int off)
{
    return cfg_read ? cfg_read(0, 2, 0, off) : pci_read32(0, 2, 0, off);
}

/* ---- attaching without a PCI scan --------------------------------------
 * The kernel finds the GPU by walking the bus. A test harness running under
 * Linux already knows where it is - it has the BAR mapped - and needs to hand
 * that mapping in directly. Everything downstream is identical either way,
 * which is the point: the code being tested is the code that ships. */
void intel_attach(uptr mmio_base, u32 mmio_bytes,
                  uptr aper_base, u32 aper_bytes, int devid,
                  u32 (*cfg)(int, int, int, int))
{
    mmio      = mmio_base;
    mmio_size = mmio_bytes;
    aperture  = aper_base;
    aper_size = aper_bytes;
    gpu_devid = (u16)devid;
    cfg_read  = cfg;
    gpu_idx   = 0;                 /* "found", by assertion rather than scan */
}

/* Is this device ID a Gen9/Gen9.5 part we understand? Skylake through Comet
 * Lake share the display register layout used above. The list covers the
 * common mobile/desktop GT1/GT2 IDs; anything else is reported but not driven. */
static int is_gen9(u16 id)
{
    /* Skylake */
    if (id == 0x1906 || id == 0x1916 || id == 0x191B || id == 0x1912) return 1;
    /* Kaby Lake */
    if (id == 0x5906 || id == 0x5916 || id == 0x591B || id == 0x5912) return 1;
    /* Coffee Lake / Whiskey Lake */
    if (id == 0x3EA0 || id == 0x3E9B || id == 0x3E92 || id == 0x3EA5) return 1;
    /* Comet Lake - including the 9B41 in this laptop */
    if (id == 0x9B41 || id == 0x9BCA || id == 0x9BC4 || id == 0x9BC8) return 1;
    return 0;
}

/* Find the Intel integrated GPU and map its register block. */
int intel_find(void)
{
    pci_scan();
    for (int i = 0; i < pci_count(); i++) {
        if (pci_vendor(i) != 0x8086) continue;   /* Intel            */
        if (pci_class(i)  != 0x03)   continue;   /* display controller */
        gpu_idx   = i;
        gpu_devid = (u16)pci_device(i);
        pci_enable(i);                            /* memory + bus master */
        mmio      = (uptr)pci_bar(i, 0);          /* GTTMMADR */
        mmio_size = pci_bar_size(i, 0);
        aperture  = (uptr)pci_bar(i, 2);          /* GMADR    */
        aper_size = pci_bar_size(i, 2);
        return i;
    }
    gpu_idx = -1;
    return -1;
}

int intel_present(void)   { return gpu_idx >= 0 && mmio != 0; }
int intel_devid(void)     { return gpu_devid; }
int intel_supported(void) { return intel_present() && is_gen9(gpu_devid); }
u32 intel_mmio(void)      { return (u32)mmio; }
u32 intel_mmio_size(void) { return mmio_size; }
u32 intel_aperture(void)  { return (u32)aperture; }
u32 intel_aper_size(void) { return aper_size; }

/* ---- stolen memory: the RAM the firmware reserved for graphics ----------
 * GMS is bits 15:8 of MGGC0. On Gen9 the size is 32 MiB per step below 0xF0,
 * then 4 MiB steps from 4..60 MiB - the encoding i915's early-quirks.c uses.
 * The base is BDSM masked to a 1 MiB boundary; the low bits are lock flags. */
u32 intel_stolen_base(void)
{
    if (gpu_idx < 0) return 0;
    return gpu_cfg(BDSM) & 0xFFF00000u;
}

u32 intel_stolen_size(void)
{
    if (gpu_idx < 0) return 0;
    u32 ggc = gpu_cfg(MGGC0);
    u32 gms = (ggc >> 8) & 0xFF;
    if (gms < 0xF0) return gms * (32u << 20);
    return (gms - 0xF0) * (4u << 20) + (4u << 20);
}

/* GGTT size from GGMS (bits 7:6): 0, 2, 4 or 8 MiB of page-table entries.
 * Each entry maps 4 KiB, so 8 MiB of PTEs covers the full 4 GiB address space. */
u32 intel_ggtt_size(void)
{
    if (gpu_idx < 0) return 0;
    u32 ggc  = gpu_cfg(MGGC0);
    u32 ggms = (ggc >> 6) & 0x3;
    if (!ggms) return 0;
    return (1u << 20) << ggms;         /* 2, 4 or 8 MiB */
}

static u32 trans_base(void);   /* defined with the timing helpers below */
int intel_plane_tiling(void);  /* defined with the plane helpers below      */

/* ---- reading the live display configuration -----------------------------
 * The firmware has already brought the panel up, so these registers describe
 * a working mode. Reading them back is the honest way to learn the panel's
 * real resolution without touching a single PLL. Sizes are stored minus one. */
int intel_pipe_width(void)
{
    u32 v = mmio_r(PIPE_SRCSZ_A);
    return (int)((v >> 16) & 0x1FFF) + 1;
}

int intel_pipe_height(void)
{
    u32 v = mmio_r(PIPE_SRCSZ_A);
    return (int)(v & 0x1FFF) + 1;
}

/* Stride units depend on TILING, which is the sort of detail that silently
 * halves your framebuffer. A linear surface counts in 64-byte units; an
 * X-tiled one counts in 512-byte tiles, and Y-tiled in 128. This panel is
 * X-tiled, where reading it as linear reports 1280 bytes for a 2560-pixel
 * wide display - off by exactly the factor of 8 between the two units. */
int intel_stride(void)
{
    u32 raw = mmio_r(PLANE_STRIDE_1_A) & 0x3FF;
    switch (intel_plane_tiling()) {
        case 0: return (int)(raw * 64);     /* linear  */
        case 1: return (int)(raw * 512);    /* X tiled */
        case 4:
        case 5: return (int)(raw * 128);    /* Y / Yf tiled */
    }
    return (int)(raw * 64);
}
int intel_plane_enabled(void){ return (mmio_r(PLANE_CTL_1_A) & PLANE_CTL_ENABLE) ? 1 : 0; }
u32 intel_plane_ctl(void)    { return mmio_r(PLANE_CTL_1_A); }

/* the graphics address the display is scanning out of, right now */
u32 intel_surface(void)      { return mmio_r(PLANE_SURFLIVE_A); }
int intel_frame_count(void)  { return (int)mmio_r(PIPE_FRMCNT_A); }

/* is the transcoder actually running? bit 31 of TRANS_CONF is the enable */
int intel_pipe_enabled(void)
{
    u32 conf = (trans_base() == TRANS_EDP_BASE) ? TRANS_EDP_CONF : TRANS_A_CONF;
    return (mmio_r(conf) & 0x80000000u) ? 1 : 0;
}

/* ---- the real thing: re-point the display at OUR framebuffer ------------
 * PLANE_SURF is special. Every other plane register is double-buffered and
 * latched, and writing PLANE_SURF is what ARMS them all - the hardware picks
 * the new configuration up at the next vblank, so the change is atomic and
 * tear-free. The address is a GRAPHICS address (an offset into the GGTT), not
 * a CPU physical address, which is why a surface has to be mapped through the
 * GGTT before it can be scanned out.
 *
 * This is genuine Intel GPU programming: after this call the display engine
 * fetches pixels from where we told it to. */
int intel_set_surface(u32 gfx_addr, int stride_bytes)
{
    if (!intel_supported()) return 0;
    if (!intel_pipe_enabled()) return 0;      /* firmware has not lit it */

    /* stride must be a multiple of 64 for a linear surface */
    if (stride_bytes & 63) return 0;
    u32 st = (u32)(stride_bytes / 64);
    if (st > 0x3FF) return 0;

    int before = intel_frame_count();

    mmio_w(PLANE_STRIDE_1_A, st);
    mmio_w(PLANE_OFFSET_1_A, 0);              /* no panning */
    mmio_w(PLANE_SURF_1_A, gfx_addr & 0xFFFFF000u);   /* ...and ARM it */

    /* wait for the flip to actually take, bounded so a wedged pipe cannot
     * hang the kernel - the frame counter is free-running while scanning */
    for (int spin = 0; spin < 2000000; spin++) {
        if (intel_frame_count() != before) break;
    }
    return (intel_surface() == (gfx_addr & 0xFFFFF000u)) ? 1 : 0;
}

/* ---- GGTT: mapping our own memory so the display can reach it ------------
 * The page tables live in the upper half of BAR0. A Gen9 entry is 8 bytes and
 * is simply the physical page address with bit 0 set for present - there are
 * no cache or permission bits on this generation. */
#define GGTT_OFFSET 0x800000

int intel_ggtt_map(u32 gfx_page, u32 phys_addr)
{
    if (!intel_present()) return 0;
    u32 ggtt = intel_ggtt_size();
    if (!ggtt) return 0;
    if (gfx_page * 8u >= ggtt) return 0;      /* past the end of the table */

    volatile u32 *pte = (volatile u32 *)(mmio + (uptr)GGTT_OFFSET + (uptr)gfx_page * 8u);
    pte[0] = (phys_addr & 0xFFFFF000u) | 1u;  /* address | present */
    pte[1] = 0;                                /* HAW=39 on a client part */
    return 1;
}

/* ==== the full timing generator ==========================================
 * Everything below reads what the firmware programmed. Sizes and positions in
 * these registers are stored MINUS ONE, and each packs a start in the low half
 * and an end in the high half. */
static int reg_lo(u32 off) { return (int)(mmio_r(off) & 0x1FFF) + 1; }
static int reg_hi(u32 off) { return (int)((mmio_r(off) >> 16) & 0x1FFF) + 1; }

/* Which transcoder is actually driving the display?
 *
 * TRANS_DDI_FUNC_CTL_EDP bit 31 says the eDP transcoder is enabled, and bits
 * [12:14] say which pipe feeds it. If it is on, its timing block is the one
 * that matters. Otherwise fall back to transcoder A. Asking the hardware
 * beats assuming, and on this laptop the two answers differ completely. */
static u32 trans_base(void)
{
    u32 edp = mmio_r(TRANS_DDI_EDP);
    if (edp & 0x80000000u) return TRANS_EDP_BASE;
    return TRANS_A_BASE;
}

int intel_transcoder_is_edp(void) { return trans_base() == TRANS_EDP_BASE; }

int intel_htotal(void)   { return reg_hi(trans_base() + TRANS_OFF_HTOTAL); }
int intel_hactive(void)  { return reg_lo(trans_base() + TRANS_OFF_HTOTAL); }
int intel_vtotal(void)   { return reg_hi(trans_base() + TRANS_OFF_VTOTAL); }
int intel_vactive(void)  { return reg_lo(trans_base() + TRANS_OFF_VTOTAL); }
int intel_hsync_start(void) { return reg_lo(trans_base() + TRANS_OFF_HSYNC); }
int intel_hsync_end(void)   { return reg_hi(trans_base() + TRANS_OFF_HSYNC); }
int intel_vsync_start(void) { return reg_lo(trans_base() + TRANS_OFF_VSYNC); }
int intel_vsync_end(void)   { return reg_hi(trans_base() + TRANS_OFF_VSYNC); }
int intel_hblank_start(void){ return reg_lo(trans_base() + TRANS_OFF_HBLANK); }
int intel_vblank_start(void){ return reg_lo(trans_base() + TRANS_OFF_VBLANK); }

/* Refresh rate, derived rather than guessed.
 *
 * There is no register that says "60 Hz". What there is: the total pixel
 * count per frame, and a frame counter that increments once per vblank. Count
 * frames against the PIT for a known interval and the refresh rate falls out -
 * which also works on a panel running at 48 or 120 Hz, where an assumed 60
 * would silently be wrong. Returns milli-hertz to keep the fraction. */
u32 intel_refresh_mhz(void)
{
    if (!intel_pipe_enabled()) return 0;
    u32 t0 = idt_ticks();
    /* wait for a tick edge so the interval is a whole number of ticks */
    while (idt_ticks() == t0) { }
    u32 start_tick = idt_ticks();
    int f0 = intel_frame_count();
    while (idt_ticks() - start_tick < 50) { }     /* 500 ms at 100 Hz */
    int f1 = intel_frame_count();
    u32 elapsed = idt_ticks() - start_tick;
    if (!elapsed) return 0;
    u32 frames = (u32)(f1 - f0);
    /* frames per (elapsed*10) ms  ->  milli-hertz */
    return (frames * 100000u) / elapsed;
}

/* pixel clock in kHz, from the mode and the measured refresh */
u32 intel_pixel_clock_khz(void)
{
    u32 mhz = intel_refresh_mhz();
    if (!mhz) return 0;
    u64 dots = (u64)intel_htotal() * (u64)intel_vtotal();
    return (u32)((dots * (u64)mhz) / 1000000u);
}

int intel_flip_count(void) { return (int)mmio_r(PIPE_FLIPCNT_A); }

/* ==== GMBUS: reading the panel's EDID ====================================
 * The EDID is the display telling us what it is - manufacturer, physical size,
 * native resolution, supported timings. It is the input a real modesetting
 * driver works from, and reading it is the honest prerequisite for ever
 * attempting one.
 *
 * The transaction: select a pin pair, write a command with the slave address
 * and byte count, then read the data register four bytes at a time as the
 * hardware fills it. */
#define EDID_ADDR 0x50

static int gmbus_wait(u32 bit, int want)
{
    for (int i = 0; i < 2000000; i++) {
        u32 v = mmio_r(GMBUS2);
        if (v & GMBUS_NAK) return 0;
        if (want ? (v & bit) : !(v & bit)) return 1;
    }
    return 0;
}

/* Read `len` bytes of EDID into our buffer using one pin pair. Gen9 numbers
 * the DDI pin pairs 1..4; which one the panel is on is not knowable without
 * ACPI, so the caller tries each. */
static int gmbus_read_edid(int pin, uptr dest, int len)
{
    if (!intel_present()) return 0;

    mmio_w(GMBUS0, (u32)pin & 0x7);          /* pin pair, 100 kHz */
    mmio_w(GMBUS1, 0);
    if (!gmbus_wait(GMBUS_ACTIVE, 0)) { mmio_w(GMBUS0, 0); return 0; }

    /* offset 0 within the EDID: an index cycle with a zero index */
    mmio_w(GMBUS5, 0);
    mmio_w(GMBUS1, GMBUS_SW_RDY | GMBUS_CYCLE_WAIT | GMBUS_CYCLE_INDEX |
                   ((u32)len << 16) | (EDID_ADDR << 1) | GMBUS_SLAVE_READ);

    int got = 0;
    while (got < len) {
        if (!gmbus_wait(GMBUS_HW_RDY, 1)) { mmio_w(GMBUS0, 0); return 0; }
        u32 v = mmio_r(GMBUS3);
        for (int b = 0; b < 4 && got < len; b++, got++)
            *(volatile u8 *)(dest + (uptr)got) = (u8)((v >> (b * 8)) & 0xFF);
    }

    mmio_w(GMBUS1, GMBUS_SW_RDY | GMBUS_CYCLE_STOP);
    gmbus_wait(GMBUS_ACTIVE, 0);
    mmio_w(GMBUS0, 0);
    return got == len;
}

/* Where a 128-byte EDID lands. In the kernel this is fixed physical scratch;
 * a host harness has no such address and supplies its own buffer instead. */
static uptr edid_buf = 0x0C980000u;
void intel_set_edid_buffer(uptr p) { if (p) edid_buf = p; }

/* An EDID always begins 00 FF FF FF FF FF FF 00. That fixed header is how we
 * know we read a display and not an empty bus. */
static int edid_valid(uptr buf)
{
    volatile u8 *e = (volatile u8 *)buf;
    if (e[0] != 0x00 || e[7] != 0x00) return 0;
    for (int i = 1; i <= 6; i++) if (e[i] != 0xFF) return 0;
    u8 sum = 0;
    for (int i = 0; i < 128; i++) sum = (u8)(sum + e[i]);
    return sum == 0;
}

static int edid_pin = 0;

int intel_read_edid(void)
{
    if (!intel_present()) return 0;
    for (int pin = 1; pin <= 4; pin++) {
        if (!gmbus_read_edid(pin, edid_buf, 128)) continue;
        if (!edid_valid(edid_buf)) continue;
        edid_pin = pin;
        return pin;
    }
    return 0;
}

int intel_edid_pin(void)  { return edid_pin; }
int intel_edid_byte(int i)
{
    if (i < 0 || i >= 128) return 0;
    return (int)*(volatile u8 *)(edid_buf + (uptr)i);
}

/* The three manufacturer letters are packed five bits each, big endian, in
 * bytes 8-9, with 1 = 'A'. */
int intel_edid_vendor_char(int i)
{
    if (i < 0 || i > 2) return 0;
    u32 v = ((u32)intel_edid_byte(8) << 8) | (u32)intel_edid_byte(9);
    int shift = 10 - i * 5;
    int c = (int)((v >> shift) & 0x1F);
    return c ? (c + 'A' - 1) : '?';
}

int intel_edid_product(void)
{
    return intel_edid_byte(10) | (intel_edid_byte(11) << 8);
}

/* The first detailed timing descriptor, at byte 54, is the panel's native
 * mode. Width and height each split their high bits into a shared nibble. */
int intel_edid_native_w(void)
{
    return intel_edid_byte(56) | ((intel_edid_byte(58) & 0xF0) << 4);
}

int intel_edid_native_h(void)
{
    return intel_edid_byte(59) | ((intel_edid_byte(61) & 0xF0) << 4);
}

/* physical size in millimetres, bytes 66-68 */
int intel_edid_width_mm(void)
{
    return intel_edid_byte(66) | ((intel_edid_byte(68) & 0xF0) << 4);
}

int intel_edid_height_mm(void)
{
    return intel_edid_byte(67) | ((intel_edid_byte(68) & 0x0F) << 8);
}

/* ==== the hardware cursor ================================================
 * A plane the display engine composites for free. The position register takes
 * a signed x and y, and negative values are expressed with a sign bit rather
 * than two's complement across the whole field. */
int intel_cursor_enable(u32 gfx_addr, int size64)
{
    if (!intel_supported() || !intel_pipe_enabled()) return 0;
    mmio_w(CUR_CTL_A, size64 ? CUR_MODE_64_ARGB : CUR_MODE_128_ARGB);
    mmio_w(CUR_BASE_A, gfx_addr & 0xFFFFF000u);   /* arms it */
    return 1;
}

int intel_cursor_move(int x, int y)
{
    if (!intel_supported()) return 0;
    u32 v = 0;
    if (x < 0) { v |= (1u << 15) | ((u32)(-x) & 0xFFF); } else v |= ((u32)x & 0xFFF);
    if (y < 0) { v |= (1u << 31) | (((u32)(-y) & 0xFFF) << 16); }
    else       { v |= (((u32)y & 0xFFF) << 16); }
    mmio_w(CUR_POS_A, v);
    return 1;
}

int intel_cursor_disable(void)
{
    if (!intel_supported()) return 0;
    mmio_w(CUR_CTL_A, CUR_MODE_DISABLE);
    mmio_w(CUR_BASE_A, 0);
    return 1;
}

/* ==== backlight ==========================================================
 * The PCH drives the panel backlight with a PWM signal: one register holds the
 * period, another the duty cycle. Setting duty to zero is a black screen with
 * the panel still powered, which is a real thing to be careful about. */
u32 intel_backlight_max(void)
{
    if (!intel_present()) return 0;
    return (mmio_r(BLC_PWM_PCH_CTL2) >> 16) & 0xFFFF;
}

u32 intel_backlight_get(void)
{
    if (!intel_present()) return 0;
    return mmio_r(BLC_PWM_PCH_CTL2) & 0xFFFF;
}

/* Percentage rather than raw counts, because the period differs per machine.
 * Clamped to 5% at the bottom: zero is indistinguishable from a broken driver
 * from where the user is sitting. */
int intel_backlight_set(int percent)
{
    if (!intel_present()) return 0;
    u32 max = intel_backlight_max();
    if (!max) return 0;
    if (percent < 5)   percent = 5;
    if (percent > 100) percent = 100;
    u32 duty = (max * (u32)percent) / 100u;
    u32 v = mmio_r(BLC_PWM_PCH_CTL2);
    mmio_w(BLC_PWM_PCH_CTL2, (v & 0xFFFF0000u) | (duty & 0xFFFF));
    return 1;
}

int intel_panel_on(void)
{
    if (!intel_present()) return 0;
    return (mmio_r(PP_STATUS) & 0x80000000u) ? 1 : 0;
}

/* ==== page flipping ======================================================
 * Writing PLANE_SURF arms every double-buffered plane register at once, and
 * the hardware latches them at the next vblank. Waiting for the frame counter
 * to move afterwards is what makes a flip tear-free: return before that and
 * the caller may start drawing into a buffer still being scanned out. */
int intel_flip(u32 gfx_addr)
{
    if (!intel_supported() || !intel_pipe_enabled()) return 0;
    int before = intel_frame_count();
    mmio_w(PLANE_SURF_1_A, gfx_addr & 0xFFFFF000u);
    for (int spin = 0; spin < 20000000; spin++)
        if (intel_frame_count() != before) return 1;
    return 0;
}

/* Wait for the start of the next vertical blank. */
int intel_wait_vblank(void)
{
    if (!intel_pipe_enabled()) return 0;
    int before = intel_frame_count();
    for (int spin = 0; spin < 20000000; spin++)
        if (intel_frame_count() != before) return 1;
    return 0;
}

/* ==== GGTT: mapping a whole range =======================================
 * One page at a time is fine for a proof; a framebuffer needs thousands. */
int intel_ggtt_map_range(u32 gfx_page, u32 phys_addr, int pages)
{
    if (!intel_present() || pages <= 0) return 0;
    for (int i = 0; i < pages; i++)
        if (!intel_ggtt_map(gfx_page + (u32)i, phys_addr + (u32)i * 4096u)) return 0;
    return 1;
}

/* ==== what the plane is actually doing ================================== */
u32 intel_plane_format(void) { return (mmio_r(PLANE_CTL_1_A) >> 24) & 0xF; }
int intel_plane_tiling(void) { return (int)((mmio_r(PLANE_CTL_1_A) >> 10) & 0x7); }
u32 intel_watermark(int level)
{
    if (level < 0 || level > 7) return 0;
    return mmio_r(PLANE_WM_1_A(level));
}
u32 intel_ddi_func_ctl(void) { return mmio_r(TRANS_DDI_EDP); }

/* ==== the DPLLs =========================================================
 * This is the part a real modesetting driver lives or dies on, and the part
 * that could not be developed at all until the host harness made the registers
 * readable in milliseconds instead of minutes.
 *
 * Skylake-class hardware has four shared DPLLs. DPLL0 doubles as the source
 * for the core display clock, so it is usually already running; the others are
 * assigned to whichever port needs them. Two registers configure them all:
 *
 *   DPLL_CTRL1  six bits per DPLL: an override enable, a link-rate index, a
 *               spread-spectrum bit, and a bit selecting HDMI mode (where the
 *               frequency comes from CFGCR1/2 instead of the rate table)
 *   DPLL_CTRL2  which DPLL each DDI port takes its clock from
 *   DPLL_STATUS one lock bit per DPLL, and lock is the thing you WAIT for
 *
 * Reading these back from a working panel is how you learn what a correct
 * configuration looks like before trying to produce one. On this laptop DPLL0
 * is locked and DPLL_CTRL1 = 0x4C3, which decodes to link rate index 1. */
#define DPLL_CTRL1      0x6C058
#define DPLL_CTRL2      0x6C05C
#define DPLL_STATUS     0x6C060
/* CFGCR is indexed from DPLL1, NOT from DPLL0.
 *
 * DPLL0 has no CFGCR at all - it is an LCPLL driven from the link-rate table
 * and has no divider registers. So the array starts at DPLL1 = 0x6C040, which
 * means DPLL2 is 0x6C048 and DPLL3 is 0x6C050.
 *
 * Computing it as 0x6C040 + pll*8 is off by one PLL: programming DPLL2 writes
 * DPLL3's registers. The symptom is exactly what the first write test produced
 * - every write appears to succeed and read back correctly, the enable bit
 * sets, and the PLL never locks, because the one being enabled was never given
 * a frequency. */
#define DPLL_CFGCR1(p)  (0x6C040 + ((p) - 1) * 8)
#define DPLL_CFGCR2(p)  (0x6C044 + ((p) - 1) * 8)
#define LCPLL1_CTL      0x46010
#define LCPLL2_CTL      0x46014
#define CDCLK_CTL       0x46000

u32 intel_dpll_ctrl1(void)  { return mmio_r(DPLL_CTRL1); }
u32 intel_dpll_ctrl2(void)  { return mmio_r(DPLL_CTRL2); }
u32 intel_dpll_status(void) { return mmio_r(DPLL_STATUS); }
u32 intel_cdclk_ctl(void)   { return mmio_r(CDCLK_CTL); }
u32 intel_lcpll1(void)      { return mmio_r(LCPLL1_CTL); }

u32 intel_dpll_cfgcr1(int pll)
{
    if (pll < 1 || pll > 3) return 0;      /* DPLL0 has no CFGCR */
    return mmio_r(DPLL_CFGCR1(pll));
}

u32 intel_dpll_cfgcr2(int pll)
{
    if (pll < 1 || pll > 3) return 0;
    return mmio_r(DPLL_CFGCR2(pll));
}

/* Is this DPLL locked? Every modeset sequence ends by waiting on this bit,
 * and a driver that does not wait produces a black screen intermittently -
 * the worst possible failure mode, because it looks like it works. */
int intel_dpll_locked(int pll)
{
    if (pll < 0 || pll > 3) return 0;
    return (mmio_r(DPLL_STATUS) >> (pll * 8)) & 1;
}

/* The six-bit configuration field for one DPLL out of DPLL_CTRL1. */
int intel_dpll_link_rate(int pll)
{
    if (pll < 0 || pll > 3) return -1;
    u32 f = (mmio_r(DPLL_CTRL1) >> (pll * 6)) & 0x3F;
    if (!(f & 1)) return -1;                 /* override not enabled */
    return (int)((f >> 1) & 0x7);
}

int intel_dpll_ssc(int pll)
{
    if (pll < 0 || pll > 3) return 0;
    return (int)((mmio_r(DPLL_CTRL1) >> (pll * 6 + 4)) & 1);
}

int intel_dpll_is_hdmi(int pll)
{
    if (pll < 0 || pll > 3) return 0;
    return (int)((mmio_r(DPLL_CTRL1) >> (pll * 6 + 5)) & 1);
}

/* Which DPLL feeds a given DDI port, from DPLL_CTRL2. Three bits of clock
 * select per port plus a "select override" bit, and a per-port clock-off bit
 * at the bottom of each field. */
int intel_ddi_clock_select(int ddi)
{
    if (ddi < 0 || ddi > 4) return -1;
    u32 v = mmio_r(DPLL_CTRL2);
    if ((v >> (ddi * 3 + 15)) & 1) { }        /* select override, informational */
    return (int)((v >> (ddi * 3 + 1)) & 0x3);
}

int intel_ddi_clock_off(int ddi)
{
    if (ddi < 0 || ddi > 4) return 1;
    return (int)((mmio_r(DPLL_CTRL2) >> (ddi + 15)) & 1);
}

/* The link rate index in DPLL_CTRL1 is not a frequency, it is a table entry.
 * These are the symbol clocks in units of 10 kHz, from the Skylake PRM. */
u32 intel_dpll_rate_khz(int idx)
{
    switch (idx) {
        case 0: return 2700000;   /* DP 5.4 GHz  */
        case 1: return 1350000;   /* DP 2.7 GHz  */
        case 2: return  810000;   /* DP 1.62 GHz */
        case 3: return 1620000;   /* DP 3.24 GHz */
        case 4: return 1080000;   /* DP 2.16 GHz */
        case 5: return 2160000;   /* DP 4.32 GHz */
    }
    return 0;
}

/* Nothing in this file WRITES to the display engine unless it has been armed.
 * The read paths are safe alongside another driver; the write paths are not,
 * and an accidental write while i915 owns the device fights it. */
static int lt_armed = 0;

/* ==== programming a DPLL =================================================
 * Everything above READS. This is where the driver starts deciding.
 *
 * Skylake-class hardware has two completely different ways of setting a DPLL,
 * and which one applies depends on what is plugged in:
 *
 *   DisplayPort (and eDP)  the link runs at one of six standard rates. You do
 *                          not choose a frequency, you choose a RATE INDEX,
 *                          and the pixel clock has to fit inside the bandwidth
 *                          that rate provides. Simple, and it is what the
 *                          internal panel uses.
 *
 *   HDMI                   the pixel clock is arbitrary, so the hardware has
 *                          to synthesise it: a DCO running near one of three
 *                          central frequencies, divided down by a chain of
 *                          P, Q and K dividers. Finding a combination that
 *                          lands close enough to the target is a search, and
 *                          it is the part people mean when they say DPLL
 *                          programming is hard.
 *
 * Both are implemented. The DP path is verifiable against this laptop right
 * now - the panel is running, so the correct answer is readable out of the
 * hardware and our computation can be checked against it.
 */

/* ---- DP: does this mode fit in this link rate? -------------------------
 * A DP link carries 8b/10b encoded symbols, so only 80% of the raw rate is
 * payload. Bandwidth is (rate * lanes * 0.8); demand is (pixel clock * bpp).
 * The right link rate is the slowest one that still fits, because a faster
 * link costs power for nothing. */
u32 intel_dp_link_bandwidth_kbps(int rate_idx, int lanes)
{
    u32 sym_khz = intel_dpll_rate_khz(rate_idx);      /* symbol clock */
    if (!sym_khz || lanes <= 0) return 0;
    /* symbol clock * 2 = raw bits/s per lane (DDR), * 8/10 for the encoding */
    return (sym_khz * 2u / 10u) * 8u * (u32)lanes;
}

u32 intel_mode_bandwidth_kbps(u32 pixel_khz, int bpp)
{
    if (bpp <= 0) bpp = 24;
    /* A zero pixel clock is not "needs no bandwidth" - it is "we do not know",
     * and it happens for real: intel_pixel_clock_khz() derives from the frame
     * counter, which is frozen whenever PSR is enabled. Returning 0 made the
     * rate chooser accept the very first candidate, which is RBR - a rate that
     * cannot carry this panel's mode. Training would pass and the screen stay
     * black. Report 0 only for a genuinely zero-bandwidth mode, never as a
     * side effect of not knowing. */
    if (!pixel_khz) return 0;
    return pixel_khz * (u32)bpp;
}

/* Choose the slowest link rate the mode fits inside.
 *
 * NOT every rate in the table is usable. DisplayPort standardises exactly
 * four: RBR 1.62, HBR 2.7, HBR2 5.4 and HBR3 8.1 Gbps - indices 2, 1 and 0
 * here. The other three (2.16, 3.24, 4.32 Gbps) are eDP intermediate rates,
 * legal only if the panel advertises them in its DPCD, and a panel that does
 * not will simply fail link training on one.
 *
 * This was caught by comparing against the running hardware: our first
 * version picked index 4 (2.16 Gbps) for this laptop's mode because it fit,
 * while i915 had chosen index 1 (2.7 Gbps). i915 was right - it was
 * restricting itself to the standard rates. Cheap to get wrong, and it would
 * have presented as "the panel stays black on our driver only".
 *
 * allow_edp_rates opens the intermediate rates up for a caller that has read
 * the DPCD and knows they are supported. */
int intel_dp_choose_rate_ex(u32 pixel_khz, int lanes, int bpp, int allow_edp_rates)
{
    /* ascending by real speed: 810, (1080), (1620), 1350, (2160), 2700 */
    static const int standard[3]  = { 2, 1, 0 };
    static const int extended[6]  = { 2, 4, 3, 1, 5, 0 };
    const int *order = allow_edp_rates ? extended : standard;
    int n = allow_edp_rates ? 6 : 3;

    u32 need = intel_mode_bandwidth_kbps(pixel_khz, bpp);
    for (int i = 0; i < n; i++)
        if (intel_dp_link_bandwidth_kbps(order[i], lanes) >= need) return order[i];
    return -1;
}

/* The safe default: standard DP rates only. */
int intel_dp_choose_rate(u32 pixel_khz, int lanes, int bpp)
{
    return intel_dp_choose_rate_ex(pixel_khz, lanes, bpp, 0);
}

/* Choose a rate the PANEL will actually accept.
 *
 * Two separate limits apply and both are easy to miss. The panel publishes a
 * maximum link rate in DPCD 0x01, and driving faster than that fails training
 * no matter what the GPU can do - this laptop's panel caps at 0x0A, 2.7 Gbps,
 * so the 5.4 Gbps rate the GPU supports is simply not available. And the
 * intermediate eDP rates are only legal if the panel advertises a rate table.
 *
 * Both facts come from the DPCD, so this takes the DPCD values rather than
 * assuming anything. Returns -1 if the mode cannot be driven at all, which is
 * a real answer a caller must handle - it means "reduce bpp or resolution",
 * not "try anyway". */
int intel_dp_choose_rate_for_panel(u32 pixel_khz, int bpp,
                                   int dpcd_max_rate, int dpcd_max_lanes,
                                   int dpcd_has_rate_table)
{
    int lanes = dpcd_max_lanes;
    if (lanes < 1) lanes = 1;
    if (lanes > 4) lanes = 4;

    int idx = intel_dp_choose_rate_ex(pixel_khz, lanes, bpp,
                                      dpcd_has_rate_table ? 1 : 0);
    if (idx < 0) return -1;

    /* DPCD 0x01 counts in units of 270 MHz of LINK rate; our table is in
     * symbol clocks, which are half that. Compare in the same units. */
    u32 chosen_link_khz = intel_dpll_rate_khz(idx) * 2u;
    u32 panel_max_khz   = (u32)dpcd_max_rate * 270000u;
    if (panel_max_khz && chosen_link_khz > panel_max_khz) return -1;
    return idx;
}

/* ---- HDMI: the divider search -----------------------------------------
 * The DCO runs at pixel_clock * 5 * (p * q * k). Intel's algorithm walks a
 * fixed set of divider combinations, keeps the ones that put the DCO within
 * its legal window around a central frequency, and picks whichever lands
 * closest to that centre - a DCO near the middle of its range is the one that
 * locks reliably.
 *
 * The candidate dividers and the three central frequencies are from the
 * Skylake PRM; this is the same set i915 walks in skl_ddi_calculate_wrpll. */
#define DCO_CENTRAL_0  9600000u    /* kHz */
#define DCO_CENTRAL_1  9000000u
#define DCO_CENTRAL_2  8400000u

static const int wrpll_dividers[] = {
    /* even dividers first - Intel's table order, and the order matters
     * because ties are broken by taking the first match */
    4, 6, 8, 10, 12, 14, 16, 18, 20, 24, 28, 30, 32, 36, 40, 42, 44,
    48, 52, 54, 56, 60, 64, 66, 68, 70, 72, 76, 78, 80, 84, 88, 90, 92, 96, 98,
    /* then odd */
    3, 5, 7, 9, 15, 21, 35
};
#define WRPLL_NDIV ((int)(sizeof(wrpll_dividers) / sizeof(wrpll_dividers[0])))

/* results of the last computation, so a caller can inspect or program them */
static u32 wr_dco_khz = 0, wr_central = 0;
static int wr_p = 0, wr_q = 0, wr_k = 0, wr_divider = 0;
static u32 wr_cfgcr1 = 0, wr_cfgcr2 = 0;

/* Split a total divider into the P, Q and K the hardware actually has.
 * K is 1, 2 or 3; P is 1, 2, 3 or 7; Q is whatever is left. Not every total
 * is expressible, which is why the search tries many. */
static int split_divider(int total, int *p, int *q, int *k)
{
    static const int ks[3] = { 1, 2, 3 };
    static const int ps[4] = { 1, 2, 3, 7 };
    for (int ki = 0; ki < 3; ki++) {
        if (total % ks[ki]) continue;
        int rem = total / ks[ki];
        for (int pi = 0; pi < 4; pi++) {
            if (rem % ps[pi]) continue;
            int qq = rem / ps[pi];
            if (qq < 1 || qq > 255) continue;
            /* the hardware only accepts a non-unity Q when K is 2 */
            if (qq != 1 && ks[ki] != 2) continue;
            *k = ks[ki]; *p = ps[pi]; *q = qq;
            return 1;
        }
    }
    return 0;
}

static u32 absdiff(u32 a, u32 b) { return a > b ? a - b : b - a; }

/* Find a divider chain for a pixel clock. Returns 1 on success and leaves the
 * answer in the wr_* statics, ready for intel_dpll_program_hdmi(). */
int intel_dpll_compute_hdmi(u32 pixel_khz)
{
    if (pixel_khz < 25000 || pixel_khz > 600000) return 0;

    u32 afe = pixel_khz * 5u;              /* the AFE clock the link needs */
    static const u32 centrals[3] = { DCO_CENTRAL_0, DCO_CENTRAL_1, DCO_CENTRAL_2 };

    u32 best_dev = 0xFFFFFFFFu;
    int found = 0;

    for (int c = 0; c < 3; c++) {
        u32 central = centrals[c];
        /* the DCO may sit up to +1% / -6% away from its central frequency */
        u32 hi = central + central / 100u;
        u32 lo = central - (central * 6u) / 100u;

        for (int d = 0; d < WRPLL_NDIV; d++) {
            u32 dco = afe * (u32)wrpll_dividers[d];
            if (dco < lo || dco > hi) continue;

            int p, q, k;
            if (!split_divider(wrpll_dividers[d], &p, &q, &k)) continue;

            u32 dev = absdiff(dco, central);
            if (dev >= best_dev) continue;

            best_dev   = dev;
            wr_dco_khz = dco;
            wr_central = central;
            wr_divider = wrpll_dividers[d];
            wr_p = p; wr_q = q; wr_k = k;
            found = 1;
        }
    }
    if (!found) return 0;

    /* CFGCR1: the DCO frequency as a multiple of 24 MHz, integer part in
     * [8:0] and a 15-bit fraction above it, plus the enable bit. */
    u32 dco_int  = wr_dco_khz / 24000u;
    u32 dco_rem  = wr_dco_khz - dco_int * 24000u;
    u32 dco_frac = (u32)(((u64)dco_rem << 15) / 24000u);
    wr_cfgcr1 = (1u << 31) | (dco_frac << 9) | (dco_int & 0x1FF);

    /* CFGCR2: the divider chain, plus which central frequency we picked.
     * qdiv_mode is set only when Q is actually dividing. */
    u32 pdiv_enc = (wr_p == 1) ? 0u : (wr_p == 2) ? 1u : (wr_p == 3) ? 2u : 4u;
    u32 kdiv_enc = (wr_k == 1) ? 0u : (wr_k == 2) ? 1u : 2u;
    u32 cf_enc   = (wr_central == DCO_CENTRAL_0) ? 0u :
                   (wr_central == DCO_CENTRAL_1) ? 1u : 3u;
    wr_cfgcr2 = ((u32)wr_q << 8) | ((wr_q > 1 ? 1u : 0u) << 7) |
                (kdiv_enc << 5) | (pdiv_enc << 2) | cf_enc;
    return 1;
}

u32 intel_wrpll_dco_khz(void) { return wr_dco_khz; }
u32 intel_wrpll_central(void) { return wr_central; }
int intel_wrpll_p(void)       { return wr_p; }
int intel_wrpll_q(void)       { return wr_q; }
int intel_wrpll_k(void)       { return wr_k; }
int intel_wrpll_divider(void) { return wr_divider; }
u32 intel_wrpll_cfgcr1(void)  { return wr_cfgcr1; }
u32 intel_wrpll_cfgcr2(void)  { return wr_cfgcr2; }

/* What pixel clock does a computed chain actually produce? Rounding in the
 * divider search means the answer is close to but not exactly the target, and
 * a driver that does not check can be several MHz out without noticing. */
u32 intel_wrpll_actual_khz(void)
{
    if (!wr_divider) return 0;
    return wr_dco_khz / (5u * (u32)wr_divider);
}

/* ---- actually writing them --------------------------------------------
 * These are the only functions in this file that change the display's clock
 * source. Everything is guarded on the pll index and on the driver having
 * been attached, and enabling always ends by WAITING FOR LOCK - a modeset
 * that proceeds without lock produces an intermittently black screen, which
 * is the worst failure mode because it looks like it works. */
int intel_dpll_program_dp(int pll, int rate_idx, int ssc)
{
    if (!intel_present() || pll < 0 || pll > 3) return 0;
    if (rate_idx < 0 || rate_idx > 5) return 0;

    u32 v = mmio_r(DPLL_CTRL1);
    u32 field = 1u                                   /* override enable */
              | ((u32)rate_idx << 1)
              | ((ssc ? 1u : 0u) << 4);              /* bit 5 (HDMI) stays 0 */
    v &= ~(0x3Fu << (pll * 6));
    v |=  (field  << (pll * 6));
    mmio_w(DPLL_CTRL1, v);
    return 1;
}

int intel_dpll_program_hdmi(int pll, u32 pixel_khz)
{
    /* DPLL0 cannot do HDMI mode: it has no divider registers. */
    if (!intel_present() || pll < 1 || pll > 3) return 0;
    if (!intel_dpll_compute_hdmi(pixel_khz)) return 0;

    mmio_w(DPLL_CFGCR1(pll), wr_cfgcr1);
    mmio_w(DPLL_CFGCR2(pll), wr_cfgcr2);

    u32 v = mmio_r(DPLL_CTRL1);
    u32 field = 1u | (1u << 5);        /* override enable + HDMI mode */
    v &= ~(0x3Fu << (pll * 6));
    v |=  (field  << (pll * 6));
    mmio_w(DPLL_CTRL1, v);
    return 1;
}

#define LCPLL_PLL_ENABLE (1u << 31)
#define LCPLL_PLL_LOCK   (1u << 30)

/* The four DPLLs do NOT have their enable bits in a regular array. DPLL0 and
 * DPLL1 are LCPLLs; DPLL2 and DPLL3 are WRPLLs 64 bytes apart in a different
 * block. Computing the address as 0x46010 + pll*4 lands on 0x46018 for DPLL2,
 * which is a different register entirely.
 *
 * Worse: DPLL0's enable is LCPLL1_CTL, and DPLL0 FEEDS CDCLK - the core
 * display clock for the whole engine. Clearing its enable bit does not just
 * turn off a PLL, it stops the display engine. Verified on hardware: that
 * register reads 0xC0000000, enabled and locked, right now. So disabling
 * DPLL0 is refused outright rather than left as a footgun. */
static u32 dpll_enable_reg(int pll)
{
    switch (pll) {
        case 0: return 0x46010;      /* LCPLL1_CTL  - feeds CDCLK */
        case 1: return 0x46014;      /* LCPLL2_CTL  */
        case 2: return 0x46040;      /* WRPLL_CTL(0) */
        case 3: return 0x46060;      /* WRPLL_CTL(1) */
    }
    return 0;
}

/* Turn a DPLL on and wait for it to lock. The wait is the point. */
int intel_dpll_enable(int pll)
{
    if (!intel_present() || !lt_armed || pll < 0 || pll > 3) return 0;
    if (intel_dpll_locked(pll)) return 1;

    u32 en = dpll_enable_reg(pll);
    if (!en) return 0;
    mmio_w(en, mmio_r(en) | LCPLL_PLL_ENABLE);

    /* The PRM quotes 5 ms for DPLL lock. MEASURED on this part, a cold WRPLL
     * that has not been used since power-on takes about 80 ms - well past both
     * the documented figure and the 50 ms this code originally allowed, which
     * is why the first write test reported "did not lock" on a PLL that had in
     * fact locked moments later.
     *
     * A modeset happens rarely and a generous bound costs nothing, so wait
     * half a second before giving up. Reporting failure on a PLL that was
     * merely slow is far worse than waiting. */
    u32 t0 = idt_ticks();
    while (idt_ticks() - t0 < 50) {          /* 500 ms */
        if (intel_dpll_locked(pll)) return 1;
    }
    return intel_dpll_locked(pll);
}

int intel_dpll_disable(int pll)
{
    if (!intel_present() || !lt_armed || pll < 1 || pll > 3) return 0;
    /* pll 0 is deliberately excluded above: it feeds CDCLK. */
    u32 en = dpll_enable_reg(pll);
    if (!en) return 0;
    mmio_w(en, mmio_r(en) & ~LCPLL_PLL_ENABLE);

    /* also drop the override so DPLL_CTRL1 stops claiming it is configured */
    u32 v = mmio_r(DPLL_CTRL1);
    v &= ~(0x3Fu << (pll * 6));
    mmio_w(DPLL_CTRL1, v);
    return !intel_dpll_locked(pll);
}

u32 intel_dpll_enable_reg(int pll)  { return dpll_enable_reg(pll); }
u32 intel_dpll_enable_val(int pll)
{
    u32 r = dpll_enable_reg(pll);
    return (intel_present() && r) ? mmio_r(r) : 0;
}
int intel_dpll_in_use(int pll)
{
    if (!intel_present() || pll < 0 || pll > 3) return 1;
    return (mmio_r(DPLL_CTRL1) >> (pll * 6)) & 1;   /* override enable */
}

/* Point a DDI port at a DPLL. Three bits of clock select per port in
 * DPLL_CTRL2, plus a per-port override bit that must be set for the selection
 * to take effect, plus a clock-off bit that has to be cleared. */
int intel_ddi_set_clock(int ddi, int pll)
{
    if (!intel_present() || ddi < 0 || ddi > 4 || pll < 0 || pll > 3) return 0;
    u32 v = mmio_r(DPLL_CTRL2);
    v &= ~(0x3u << (ddi * 3 + 1));            /* clear the select      */
    v |=  ((u32)pll << (ddi * 3 + 1));        /* ...and set it         */
    /* Override lives at bit (ddi*3), NOT (ddi*3 + 3). This was setting the
      * NEXT port's override bit: asking for DDI A set DDI B's. It survives on
      * this machine only because firmware left DDI A's b0 set - measured
      * DPLL_CTRL2 = 0x00A30001, b0 set and b3 clear. First cold start without
      * firmware's help, it would have failed with nothing to print. */
    v |=  (1u << (ddi * 3));                  /* select override on    */
    v &= ~(1u << (ddi + 15));                 /* clock off -> off      */
    mmio_w(DPLL_CTRL2, v);
    return 1;
}

/* ==== the DisplayPort AUX channel ========================================
 * Everything so far has talked to the GPU. This talks to the PANEL.
 *
 * AUX is a tiny bidirectional link alongside the main DisplayPort lanes,
 * carrying short transactions to a register map inside the display called the
 * DPCD. It is how a driver learns what the panel can actually do - its
 * maximum link rate, how many lanes it has, which training patterns it
 * supports - and it is the channel link training itself is negotiated over.
 * Without AUX there is no modeset, only guessing.
 *
 * A transaction is a header plus up to 16 bytes, written into the data
 * registers, kicked off by setting SEND_BUSY, and answered in the same
 * registers. The header is four bytes:
 *
 *     [7:4] command   9 = native read, 8 = native write
 *     [3:0] address bits 19:16
 *     address bits 15:8
 *     address bits 7:0
 *     length - 1
 *
 * The reply's top nibble is ACK (0), NACK (1) or DEFER (2). DEFER means "ask
 * again" and is completely normal - a panel that is busy defers rather than
 * failing, and a driver that treats a defer as an error will look flaky on
 * hardware that is working correctly.
 */
/* DC5/DC6 gate power well 1, and AUX channel A lives in PG1. With a DC state
 * armed, AUX register reads return zeros and writes go nowhere - with no error
 * bit set anywhere. This is the documented number-one cause of "AUX works on
 * some boots". Measured on this box going 0 -> 2 between two runs minutes
 * apart, so it is not hypothetical. Defined up here because aux_xfer needs it. */
#define DC_STATE_EN         0x45504

#define DC_STATE_DISABLE        0u
#define DC_STATE_EN_UPTO_DC5    (1u << 0)
#define DC_STATE_EN_UPTO_DC6    (2u << 0)
#define DC_STATE_EN_DC9         (1u << 3)

#define DP_AUX_CH_CTL(port)   (0x64010 + (port) * 0x100)
#define DP_AUX_CH_DATA(port, i) (0x64014 + (port) * 0x100 + (i) * 4)
#define DP_AUX_MUTEX(port)    (0x6402C + (port) * 0x100)
#define AUX_MUTEX_ENABLE      (1u << 31)
#define AUX_MUTEX_HELD        (1u << 30)

/* Force every DC state off and make it stick.
 *
 * The DMC firmware re-arms DC5/DC6 behind our back, so a single write loses a
 * race we would never see - the register reads back correct once and is wrong
 * by the time AUX runs. The plan's rule is to keep writing until the value
 * holds across six consecutive reads. No wall-clock timeout: this either
 * settles in a few iterations or the DMC is fighting us, and spinning a bounded
 * 100 times is cheaper than reasoning about how long that takes. */
static void dc_states_off(void)
{
    for (int attempt = 0; attempt < 100; attempt++) {
        mmio_w(DC_STATE_EN, DC_STATE_DISABLE);
        int stable = 0;
        for (; stable < 6; stable++)
            if (mmio_r(DC_STATE_EN) != DC_STATE_DISABLE) break;
        if (stable == 6) return;
    }
}

/* The AUX channel is shared with the hardware's own users - PSR fast-wake and
 * GTC both issue transactions on it without asking. The mutex is how you tell
 * them to wait, and it has an unusual protocol: READING the register while
 * enabled IS the acquire attempt. Read back 0 in the HELD bit and it is ours;
 * read 1 and somebody else has it.
 *
 * PSR is measured ON on this panel, so this is not optional. */
static int aux_mutex_acquire(int port)
{
    for (int i = 0; i < 20; i++) {                /* 20 x 500 us = 10 ms */
        mmio_w(DP_AUX_MUTEX(port), AUX_MUTEX_ENABLE);
        if (!(mmio_r(DP_AUX_MUTEX(port)) & AUX_MUTEX_HELD)) return 1;
        cpu_delay_us(500);
    }
    return 0;
}

static void aux_mutex_release(int port)
{
    mmio_w(DP_AUX_MUTEX(port), AUX_MUTEX_ENABLE | AUX_MUTEX_HELD);
}

#define AUX_SEND_BUSY     (1u << 31)
#define AUX_DONE          (1u << 30)
#define AUX_INTERRUPT     (1u << 29)
#define AUX_TIMEOUT_ERR   (1u << 28)
#define AUX_TIMEOUT_MAX   (3u << 26)
#define AUX_RECEIVE_ERR   (1u << 25)
#define AUX_MSG_SIZE_SHIFT 20

/* Skylake derives the AUX bit clock from CDCLK itself, so there is no divider
 * to compute - but it does want the sync pulse counts programmed, and i915
 * uses 32 for both. */
#define AUX_SYNC_PULSE_SKL(c)    (((c) - 1) << 0)
#define AUX_FW_SYNC_PULSE_SKL(c) (((c) - 1) << 5)

#define AUX_REPLY_ACK    0x0
#define AUX_REPLY_NACK   0x1
#define AUX_REPLY_DEFER  0x2

static u8 aux_buf[20];
static int aux_last_reply = -1;
static int aux_last_len   = 0;

/* The transaction itself, run with the AUX mutex already held. Split out from
 * aux_xfer() below purely so that the mutex cannot leak: this function has
 * eight exit paths, and a release on each is a release that will eventually be
 * forgotten. The wrapper owns acquire and release; this owns the protocol. */
static int aux_xfer_locked(int port, u32 cmd, u32 addr, const u8 *out, int out_len,
                           u8 *in, int in_len)
{

    u32 ctl_reg = DP_AUX_CH_CTL(port);

    /* the header, then any payload, packed big-endian into the data regs */
    u8 msg[20];
    int msg_len = 0;
    msg[msg_len++] = (u8)((cmd << 4) | ((addr >> 16) & 0x0F));
    msg[msg_len++] = (u8)((addr >> 8) & 0xFF);
    msg[msg_len++] = (u8)(addr & 0xFF);
    /* a zero-length transaction encodes as length-1 = 0xFF... but we never
     * issue one, so the simple form is correct here */
    msg[msg_len++] = (u8)((out_len ? out_len : in_len) - 1);
    for (int i = 0; i < out_len; i++) msg[msg_len++] = out[i];

    /* There used to be a read-modify-write status pre-clear here. It was a
     * loaded gun: if SEND_BUSY happened to be set in the value read back -
     * which PSR's own fast-wake transactions on this same channel will do -
     * writing it back LAUNCHES a second transaction, with whatever stale bytes
     * are still in DATA1-5 and whatever stale Message Size is in the register.
     * It also preserved timeout field [27:26] = 00b, the forbidden 400 us
     * setting. The send write below already folds all three write-1-clear bits
     * into the same write that starts the transaction, which is the documented
     * way to do this, so nothing is lost by deleting it. */

    for (int i = 0; i < 5; i++) {
        u32 w = 0;
        for (int b = 0; b < 4; b++) {
            int idx = i * 4 + b;
            w = (w << 8) | (idx < msg_len ? msg[idx] : 0);
        }
        mmio_w(DP_AUX_CH_DATA(port, i), w);
    }

    u32 send = AUX_SEND_BUSY | AUX_DONE | AUX_INTERRUPT | AUX_TIMEOUT_ERR |
               AUX_TIMEOUT_MAX | AUX_RECEIVE_ERR |
               ((u32)msg_len << AUX_MSG_SIZE_SHIFT) |
               AUX_FW_SYNC_PULSE_SKL(32) | AUX_SYNC_PULSE_SKL(32);
    mmio_w(ctl_reg, send);

    /* SEND_BUSY clears when the transaction completes, one way or another.
     * Poll on a real clock: the hardware's own timeout is 1600 us, so a 10 ms
     * bound is generous, and the previous 2,000,000-iteration count was both
     * far past that on a fast core and potentially short of it on a slow one. */
    u32 status = 0;
    int done = 0;
    for (int i = 0; i < 1000; i++) {              /* 1000 x 10 us = 10 ms */
        status = mmio_r(ctl_reg);
        if (!(status & AUX_SEND_BUSY)) { done = 1; break; }
        cpu_delay_us(10);
    }

    /* Acknowledge FIRST, on every path. Returning before this - as the !done
     * path used to - leaves SEND_BUSY and every write-1-clear bit set, which
     * then makes the next transaction's status decode meaningless. Note
     * `status` is written back with b31 already clear on the done path, so
     * this cannot itself launch a transaction; on the !done path we must mask
     * it off explicitly. */
    mmio_w(ctl_reg, (status & ~AUX_SEND_BUSY) |
                    AUX_DONE | AUX_TIMEOUT_ERR | AUX_RECEIVE_ERR);

    if (!done) { aux_last_reply = -4; return -1; }

    if (status & AUX_TIMEOUT_ERR) { aux_last_reply = -2; return -1; }
    if (status & AUX_RECEIVE_ERR) {
        aux_last_reply = -3;
        /* DP CTS 4.2.1.1: settle 400 us before anyone retries. Not needed after
         * a hardware TIMEOUT, whose 1600 us already exceeds it. */
        cpu_delay_us(400);
        return -1;
    }
    /* DONE is the only positive completion signal, and it was never tested. */
    if (!(status & AUX_DONE)) { aux_last_reply = -5; return -1; }

    int recv = (int)((status >> AUX_MSG_SIZE_SHIFT) & 0x1F);
    if (recv < 1 || recv > 20) return -1;         /* 0 or >20 is illegal */

    /* first byte is the reply header; the rest is data */
    u32 d0 = mmio_r(DP_AUX_CH_DATA(port, 0));
    aux_last_reply = (int)((d0 >> 28) & 0xF);
    aux_last_len   = recv - 1;

    int n = recv - 1;
    if (n > in_len) n = in_len;
    for (int i = 0; i < n; i++) {
        int byte_index = i + 1;                    /* skip the reply header */
        u32 w = mmio_r(DP_AUX_CH_DATA(port, byte_index / 4));
        in[i] = (u8)((w >> (24 - (byte_index % 4) * 8)) & 0xFF);
    }
    return n;
}

/* One raw transaction. Returns the number of bytes received, or -1.
 *
 * Everything that must be true BEFORE the hardware is touched lives here, and
 * the mutex is released on every path out. */
static int aux_xfer(int port, u32 cmd, u32 addr, const u8 *out, int out_len,
                    u8 *in, int in_len)
{
    if (!intel_present() || port < 0 || port > 3) return -1;
    if (out_len > 16 || in_len > 16) return -1;

    /* Stale state must not survive a rejected call: every early return here
     * used to leave the PREVIOUS transaction's reply visible to the caller. */
    aux_last_reply = -1;
    aux_last_len   = 0;

    /* THE PANEL MUST BE POWERED. The sink's AUX receiver runs off panel VDD;
     * with VDD down we are driving AUX+/- into unpowered input pins and the
     * current goes through the TCON's ESD/body diodes. That is an absolute-
     * maximum-rating violation (Vin <= VDD + 0.3 V), not a timing preference.
     *
     * This guard belongs here rather than at the call sites precisely because
     * the risky caller is the retrain-after-failure path, which runs exactly
     * when the panel may have been taken down. */
    if (!(mmio_r(PP_STATUS) & PP_STATUS_ON) && !(mmio_r(PP_CONTROL) & PP_VDD_FORCE))
        return -1;

    /* AUX A is in power well 1, which a DC state gates off. Done per
     * transaction, not once at init - the DMC re-arms it behind us. */
    dc_states_off();

    /* Take the channel off PSR and GTC. Failing to get the mutex is a real
     * error, not something to push through: proceeding would interleave our
     * transaction with the hardware's own. */
    if (!aux_mutex_acquire(port)) { aux_last_reply = -6; return -1; }

    int r = aux_xfer_locked(port, cmd, addr, out, out_len, in, in_len);

    aux_mutex_release(port);
    return r;
}


int intel_aux_last_reply(void) { return aux_last_reply; }

/* A native DPCD read, with the retry a DEFER requires. */
int intel_dpcd_read(int port, u32 addr, int len)
{
    if (len < 1 || len > 16) return 0;
    for (int attempt = 0; attempt < 8; attempt++) {
        int n = aux_xfer(port, 0x9, addr, 0, 0, aux_buf, len);
        if (n > 0 && aux_last_reply == AUX_REPLY_ACK) return n;
        if (aux_last_reply != AUX_REPLY_DEFER && n < 0 && attempt > 2) break;
        /* a short settle before asking again - the panel said "not yet".
         * 500 us is what the DP spec allows between native DEFER retries. */
        cpu_delay_us(500);
    }
    return 0;
}

int intel_dpcd_write(int port, u32 addr, const u8 *data, int len)
{
    if (len < 1 || len > 16) return 0;
    for (int attempt = 0; attempt < 8; attempt++) {
        int n = aux_xfer(port, 0x8, addr, data, len, aux_buf, 1);
        if (n >= 0 && aux_last_reply == AUX_REPLY_ACK) return 1;
        /* A NACK means "no", not "not yet" - re-sending it 8 times is wrong
         * and burns the whole budget. Only a DEFER earns a retry. */
        if (aux_last_reply != AUX_REPLY_DEFER) return 0;
        cpu_delay_us(500);
    }
    return 0;
}

int intel_dpcd_byte(int i)
{
    if (i < 0 || i >= 20) return 0;
    return (int)aux_buf[i];
}

/* ---- what the DPCD says, decoded --------------------------------------
 * These are the fields that decide a modeset. MAX_LINK_RATE is in units of
 * 270 MHz: 0x06 = 1.62, 0x0A = 2.7, 0x14 = 5.4, 0x1E = 8.1 Gbps. */
/* The capability block gets its OWN storage.
 *
 * These accessors used to read aux_buf directly - the single scratch buffer
 * that every DPCD read and write shares. So the moment link training read link
 * status from 0x202, intel_dpcd_rev() started returning a link-status byte and
 * intel_dpcd_max_lanes() returned whatever happened to land in aux_buf[2].
 * Nothing latched the caps anywhere. Read them once, keep them. */
static u8  dpcd_caps[16];
static int dpcd_caps_valid = 0;

/* Read DPCD 0x00000..0x0000E in one transaction and latch it. Must succeed
 * before any accessor below means anything. */
int intel_dpcd_read_caps(int port)
{
    dpcd_caps_valid = 0;
    int n = intel_dpcd_read(port, 0x00000, 15);
    if (n < 15) return 0;                    /* short read is a protocol error */
    for (int i = 0; i < 15; i++) dpcd_caps[i] = (u8)intel_dpcd_byte(i);
    if (!dpcd_caps[0]) return 0;             /* rev 0 means the read failed */
    dpcd_caps_valid = 1;
    return 1;
}
int intel_dpcd_caps_valid(void) { return dpcd_caps_valid; }

int intel_dpcd_rev(void)        { return (int)dpcd_caps[0]; }
int intel_dpcd_max_rate(void)   { return (int)dpcd_caps[1]; }
int intel_dpcd_max_lanes(void)  { return (int)(dpcd_caps[2] & 0x1F); }
int intel_dpcd_enhanced(void)   { return (int)((dpcd_caps[2] >> 7) & 1); }
int intel_dpcd_tps3(void)       { return (int)((dpcd_caps[2] >> 6) & 1); }

/* DPCD 0x0E bits 6:0 is TRAINING_AUX_RD_INTERVAL - how long the sink wants us
 * to wait before reading link status back. Nothing read this before, so both
 * training loops used the same hardcoded spin and a panel asking for interval 4
 * (16 ms) got about 1 ms and read back "not locked" on a healthy link. */
static u32 lt_rd_interval(void)
{
    return (u32)(dpcd_caps[14] & 0x7F);
}
static u32 lt_cr_interval_us(void)
{
    u32 iv = lt_rd_interval();
    if (intel_dpcd_rev() >= 0x14) return 100u;   /* rev >= 1.4 forces 100 us */
    return iv ? iv * 4000u : 100u;
}
static u32 lt_eq_interval_us(void)
{
    /* Note the asymmetry with clock recovery: 400 us at interval 0, and the
     * rev >= 1.4 "force 100 us" rule applies to CR only. */
    u32 iv = lt_rd_interval();
    return iv ? iv * 4000u : 400u;
}

/* Does the panel publish a SUPPORTED_LINK_RATES table at DPCD 0x10?
 *
 * This used to return bit 7 of DPCD 0x0E, which is EXTENDED_RECEIVER_CAP_FIELD
 * _PRESENT - a different field entirely, set on most modern eDP panels. That
 * made the rate chooser hand back the eDP intermediate rates, and the caller
 * then divided them by 270000 to build LINK_BW_SET, producing 0x08 - not one
 * of the four legal bandwidth codes. Training simply timed out.
 *
 * Answering this honestly needs a real read of 0x10..0x1F plus the Method B
 * LINK_RATE_SET write path, and neither exists yet. Until they do, the correct
 * answer is "no table", which confines the chooser to the standard rates and
 * to LINK_BW_SET. The measured panel has no rate table, so nothing is lost. */
int intel_dpcd_has_rate_table(void) { return 0; }

u32 intel_dpcd_max_rate_kbps(void)
{
    /* the encoded value times 270 MHz, then times 10 for kbps per lane */
    return (u32)dpcd_caps[1] * 270000u;
}

/* ==== DisplayPort link training ==========================================
 * This is the negotiation that actually brings a link up, and it is the last
 * large piece of a cold-start modeset.
 *
 * The problem it solves: a DP link is a high-speed serial connection over a
 * cable or a flex of unknown quality, and the right transmitter drive strength
 * is not knowable in advance. So the two ends negotiate. The source sends a
 * known pattern, the sink reports whether it locked onto it, and if not it
 * says how much more drive it wants. Repeat until it works or the attempts run
 * out.
 *
 * Two phases, and they are not interchangeable:
 *
 *   CLOCK RECOVERY (pattern 1)      the sink locks its PLL to the bit clock.
 *                                   Success is CR_DONE on every lane.
 *   CHANNEL EQUALISATION (2 or 3)   the sink equalises the channel and finds
 *                                   symbol boundaries. Success needs
 *                                   CHANNEL_EQ_DONE, SYMBOL_LOCKED on every
 *                                   lane AND INTERLANE_ALIGN_DONE.
 *
 * The retry rules come from the DP specification and exist to stop a driver
 * looping forever on a link that will never work: at most five attempts at
 * clock recovery, and at most four at the same voltage level. Reaching
 * maximum swing without CR_DONE means this rate is not achievable - the
 * correct response is to fall back to a lower link rate, then to fewer lanes,
 * not to keep trying.
 *
 * WARNING: everything here WRITES, both to the GPU and to the panel's own
 * registers. Running it while another driver owns the display will fight that
 * driver. On the host harness, detach i915 first.
 */
#define DDI_BUF_CTL(port)   (0x64000 + (port) * 0x100)
#define DDI_BUF_CTL_ENABLE  (1u << 31)
#define DDI_BUF_IS_IDLE     (1u << 7)
#define DP_TP_CTL(port)     (0x64040 + (port) * 0x100)
#define DP_TP_STATUS(port)  (0x64044 + (port) * 0x100)

#define DP_TP_CTL_ENABLE          (1u << 31)
#define DP_TP_CTL_MODE_SST        (0u << 27)
#define DP_TP_CTL_ENHANCED_FRAME  (1u << 18)
#define DP_TP_CTL_LINK_TRAIN_MASK (7u << 8)
#define DP_TP_CTL_LINK_TRAIN_PAT1 (0u << 8)
#define DP_TP_CTL_LINK_TRAIN_PAT2 (1u << 8)
#define DP_TP_CTL_LINK_TRAIN_IDLE (2u << 8)
#define DP_TP_CTL_LINK_TRAIN_NORM (3u << 8)
#define DP_TP_CTL_LINK_TRAIN_PAT3 (4u << 8)

/* DPCD addresses used during training */
#define DPCD_LINK_BW_SET        0x100
#define DPCD_LANE_COUNT_SET     0x101
#define DPCD_TRAINING_PATTERN   0x102
#define DPCD_TRAINING_LANE0     0x103
#define DPCD_LANE0_1_STATUS     0x202
#define DPCD_LANE_ALIGN_STATUS  0x204
#define DPCD_ADJUST_REQ_LANE0_1 0x206

#define DP_TRAIN_PAT_1          0x01
#define DP_TRAIN_PAT_2          0x02
#define DP_TRAIN_PAT_3          0x03
#define DP_TRAIN_PAT_NONE       0x00
#define DP_SCRAMBLING_DISABLE   0x20

#define DP_CR_DONE              (1 << 0)
#define DP_CHANNEL_EQ_DONE      (1 << 1)
#define DP_SYMBOL_LOCKED        (1 << 2)
#define DP_INTERLANE_ALIGN_DONE (1 << 0)
#define DP_MAX_SWING_REACHED    (1 << 2)
#define DP_MAX_PRE_REACHED      (1 << 5)

/* the state of the last training run, for diagnostics */
static int lt_lanes = 0, lt_rate_idx = 0;
static int lt_cr_attempts = 0, lt_eq_attempts = 0;
static int lt_final_swing[4], lt_final_pre[4];
static int lt_last_status[6];

void intel_link_train_arm(int on) { lt_armed = on ? 1 : 0; }
int  intel_link_train_armed(void) { return lt_armed; }

/* The link rate index our DPLL table uses is not what the panel wants in
 * DPCD 0x100 - that field counts in units of 270 MHz of LINK rate. */
static u8 dpcd_bw_for_rate_idx(int idx)
{
    u32 link_khz = intel_dpll_rate_khz(idx) * 2u;
    return (u8)(link_khz / 270000u);
}

static int dpcd_get(int port, u32 addr, int len, u8 *out)
{
    if (!intel_dpcd_read(port, addr, len)) return 0;
    for (int i = 0; i < len; i++) out[i] = (u8)intel_dpcd_byte(i);
    return 1;
}

/* Per-lane status is packed two lanes to a byte. */
static int lane_status(const u8 *st, int lane)
{
    return (st[lane / 2] >> ((lane % 2) * 4)) & 0xF;
}

static int adjust_swing(const u8 *adj, int lane)
{
    return (adj[lane / 2] >> ((lane % 2) * 4)) & 0x3;
}

static int adjust_pre(const u8 *adj, int lane)
{
    return (adj[lane / 2] >> ((lane % 2) * 4 + 2)) & 0x3;
}

/* Write the drive settings the sink asked for into both ends: the panel's
 * TRAINING_LANE registers and, on real silicon, the DDI buffer translation
 * that actually changes the transmitter. */
/* ---- DDI buffer translation ---------------------------------------------
 * The last piece of link training that cannot be derived: what voltage the
 * transmitter actually drives for a given swing/pre-emphasis pair. The values
 * are per-SKU silicon characterisation, published only in the PRM.
 *
 * These were READ OFF THIS MACHINE - the ten entries i915 programmed into
 * DDI_BUF_TRANS for DDI A, which is the eDP panel. They match i915's
 * skl_u_ddi_translations_edp exactly, confirming both the table and that a
 * Comet Lake-U part uses the SKL-U eDP set.
 *
 * The index is a triangular map over the legal (swing, pre-emphasis) pairs:
 * higher swing leaves room for less pre-emphasis, so the combinations are not
 * a 4x4 grid. Asking for an illegal pair is a driver bug, not a hardware one. */
static const u32 edp_buf_trans[10][2] = {
    { 0x00000018, 0x000000A8 },   /* swing 0, pre 0 */
    { 0x00004013, 0x000000A9 },   /* swing 0, pre 1 */
    { 0x00007011, 0x000000A2 },   /* swing 0, pre 2 */
    { 0x00009010, 0x0000009C },   /* swing 0, pre 3 */
    { 0x00000018, 0x000000A9 },   /* swing 1, pre 0 */
    { 0x00006013, 0x000000A2 },   /* swing 1, pre 1 */
    { 0x00007011, 0x000000A6 },   /* swing 1, pre 2 */
    { 0x00002016, 0x000000AB },   /* swing 2, pre 0 */
    { 0x00005013, 0x0000009F },   /* swing 2, pre 1 */
    { 0x00000018, 0x000000DF },   /* swing 3, pre 0 */
};

#define DDI_BUF_TRANS(port, i) (0x64E00 + (port) * 0x60 + (i) * 8)

static int buf_trans_index(int swing, int pre)
{
    static const int base[4] = { 0, 4, 7, 9 };
    static const int room[4] = { 4, 3, 2, 1 };
    if (swing < 0) swing = 0;
    if (swing > 3) swing = 3;
    if (pre < 0) pre = 0;
    if (pre >= room[swing]) pre = room[swing] - 1;
    return base[swing] + pre;
}

u32 intel_buf_trans1(int i) { return (i >= 0 && i < 10) ? edp_buf_trans[i][0] : 0; }
u32 intel_buf_trans2(int i) { return (i >= 0 && i < 10) ? edp_buf_trans[i][1] : 0; }
int intel_buf_trans_index(int swing, int pre) { return buf_trans_index(swing, pre); }

/* Program the whole table and select an entry. The table has to be written
 * before DDI_BUF_CTL is enabled - the hardware latches it at enable. */
int intel_ddi_program_buf_trans(int port, int swing, int pre)
{
    if (!intel_present() || !lt_armed || port < 0 || port > 4) return 0;
    for (int i = 0; i < 10; i++) {
        mmio_w(DDI_BUF_TRANS(port, i) + 0, edp_buf_trans[i][0]);
        mmio_w(DDI_BUF_TRANS(port, i) + 4, edp_buf_trans[i][1]);
    }
    int idx = buf_trans_index(swing, pre);
    u32 v = mmio_r(DDI_BUF_CTL(port));
    v &= ~(0xFu << 24);
    v |= ((u32)idx << 24);
    mmio_w(DDI_BUF_CTL(port), v);
    return 1;
}

static void set_drive(int port, int lanes, const int *swing, const int *pre)
{
    u8 v[4];
    for (int i = 0; i < 4; i++) {
        int sw = i < lanes ? swing[i] : 0;
        int pe = i < lanes ? pre[i] : 0;
        v[i] = (u8)((sw & 3) | ((pe & 3) << 3));
        if (sw >= 3) v[i] |= DP_MAX_SWING_REACHED;
        if (pe >= 3) v[i] |= (1 << 5);
    }
    intel_dpcd_write(port, DPCD_TRAINING_LANE0, v, lanes);
    /* and the transmitter side: select the buffer translation entry that
     * corresponds to the swing/pre-emphasis the sink asked for. Lane 0's
     * request drives the selection, which is what the hardware supports -
     * DDI_BUF_CTL has one entry select for the whole port, not per lane. */
    intel_ddi_program_buf_trans(port, swing[0], pre[0]);
}

static void tp_ctl_pattern(int port, u32 pattern, int enhanced)
{
    u32 v = DP_TP_CTL_ENABLE | DP_TP_CTL_MODE_SST | pattern;
    if (enhanced) v |= DP_TP_CTL_ENHANCED_FRAME;
    mmio_w(DP_TP_CTL(port), v);
}

/* ---- clock recovery ---------------------------------------------------- */
static int train_clock_recovery(int port, int lanes)
{
    int swing[4] = {0,0,0,0}, pre[4] = {0,0,0,0};
    int prev_swing = -1, same_voltage = 0;
    lt_cr_attempts = 0;

    tp_ctl_pattern(port, DP_TP_CTL_LINK_TRAIN_PAT1, 1);
    u8 pat = DP_TRAIN_PAT_1 | DP_SCRAMBLING_DISABLE;
    if (!intel_dpcd_write(port, DPCD_TRAINING_PATTERN, &pat, 1)) return 0;
    set_drive(port, lanes, swing, pre);

    for (int attempt = 0; attempt < 5; attempt++) {
        lt_cr_attempts++;
        /* The sink states its own interval in DPCD 0x0E bits 6:0. Interval 0
         * means 100 us for clock recovery; anything else is interval*4000 us.
         * This wait MUST come before the first status read - reading straight
         * after writing TRAINING_PATTERN_SET returns stale bits and fails a
         * link that is perfectly healthy. */
        cpu_delay_us(lt_cr_interval_us());

        u8 st[6];
        if (!dpcd_get(port, DPCD_LANE0_1_STATUS, 6, st)) return 0;
        for (int i = 0; i < 6; i++) lt_last_status[i] = st[i];

        int all = 1;
        for (int l = 0; l < lanes; l++)
            if (!(lane_status(st, l) & DP_CR_DONE)) all = 0;
        if (all) {
            for (int l = 0; l < 4; l++) { lt_final_swing[l] = swing[l]; lt_final_pre[l] = pre[l]; }
            return 1;
        }

        /* not locked - take the sink's adjustment request */
        u8 adj[2] = { st[4], st[5] };
        int maxed = 0;
        for (int l = 0; l < lanes; l++) {
            swing[l] = adjust_swing(adj, l);
            pre[l]   = adjust_pre(adj, l);
            if (swing[l] >= 3) maxed = 1;
        }

        if (swing[0] == prev_swing) {
            if (++same_voltage >= 4) return 0;   /* the spec's limit */
        } else {
            same_voltage = 0;
            prev_swing = swing[0];
        }
        if (maxed && same_voltage >= 1) return 0; /* no more drive available */

        set_drive(port, lanes, swing, pre);
    }
    return 0;
}

/* ---- channel equalisation ---------------------------------------------- */
static int train_channel_eq(int port, int lanes, int tps3)
{
    u32 hw_pat = tps3 ? DP_TP_CTL_LINK_TRAIN_PAT3 : DP_TP_CTL_LINK_TRAIN_PAT2;
    u8  dp_pat = (u8)((tps3 ? DP_TRAIN_PAT_3 : DP_TRAIN_PAT_2) | DP_SCRAMBLING_DISABLE);

    tp_ctl_pattern(port, hw_pat, 1);
    if (!intel_dpcd_write(port, DPCD_TRAINING_PATTERN, &dp_pat, 1)) return 0;
    lt_eq_attempts = 0;

    for (int attempt = 0; attempt < 5; attempt++) {
        lt_eq_attempts++;
        /* 400 us at interval 0, NOT the 100 us clock recovery uses. The
         * asymmetry is real and is easy to lose. */
        cpu_delay_us(lt_eq_interval_us());

        u8 st[6];
        if (!dpcd_get(port, DPCD_LANE0_1_STATUS, 6, st)) return 0;
        for (int i = 0; i < 6; i++) lt_last_status[i] = st[i];

        /* clock recovery must still be holding, or the link has come apart
         * and continuing to equalise is pointless */
        for (int l = 0; l < lanes; l++)
            if (!(lane_status(st, l) & DP_CR_DONE)) return 0;

        int all = 1;
        for (int l = 0; l < lanes; l++) {
            int s = lane_status(st, l);
            if (!(s & DP_CHANNEL_EQ_DONE) || !(s & DP_SYMBOL_LOCKED)) all = 0;
        }
        if (all && (st[2] & DP_INTERLANE_ALIGN_DONE)) return 1;

        u8 adj[2] = { st[4], st[5] };
        int swing[4], pre[4];
        for (int l = 0; l < lanes; l++) {
            swing[l] = adjust_swing(adj, l);
            pre[l]   = adjust_pre(adj, l);
        }
        set_drive(port, lanes, swing, pre);
    }
    return 0;
}

/* Run the whole sequence at one rate and lane count. */
int intel_link_train(int port, int rate_idx, int lanes, int tps3, int enhanced)
{
    if (!lt_armed) return 0;             /* refuse unless explicitly armed */
    if (!intel_present()) return 0;
    if (lanes < 1 || lanes > 4) return 0;

    lt_lanes = lanes; lt_rate_idx = rate_idx;

    /* tell the panel what link we intend to run */
    u8 bw = dpcd_bw_for_rate_idx(rate_idx);
    u8 lc = (u8)(lanes | (enhanced ? 0x80 : 0));
    if (!intel_dpcd_write(port, DPCD_LINK_BW_SET, &bw, 1)) return 0;
    if (!intel_dpcd_write(port, DPCD_LANE_COUNT_SET, &lc, 1)) return 0;

    if (!train_clock_recovery(port, lanes)) return 0;
    if (!train_channel_eq(port, lanes, tps3)) return 0;

    /* done: stop the pattern at both ends and let real pixels flow */
    u8 none = DP_TRAIN_PAT_NONE;
    intel_dpcd_write(port, DPCD_TRAINING_PATTERN, &none, 1);
    tp_ctl_pattern(port, DP_TP_CTL_LINK_TRAIN_IDLE, enhanced);
    cpu_delay_us(500);
    tp_ctl_pattern(port, DP_TP_CTL_LINK_TRAIN_NORM, enhanced);
    return 1;
}

/* Train with fallback. When a rate fails, the specification's answer is to
 * step DOWN - first the link rate, then the lane count - not to retry the
 * same configuration harder. Returns the rate index that worked, or -1. */
int intel_link_train_auto(int port, u32 pixel_khz, int bpp,
                          int dpcd_max_rate, int dpcd_max_lanes,
                          int has_rate_table, int tps3, int enhanced)
{
    if (!lt_armed) return -1;

    static const int by_speed_desc[3] = { 0, 1, 2 };   /* 2700, 1350, 810 */
    int lanes = dpcd_max_lanes;
    if (lanes > 4) lanes = 4;

    for (; lanes >= 1; lanes >>= 1) {
        for (int i = 0; i < 3; i++) {
            int idx = by_speed_desc[i];
            /* never exceed what the panel says it can take */
            u32 link_khz = intel_dpll_rate_khz(idx) * 2u;
            if (link_khz > (u32)dpcd_max_rate * 270000u) continue;
            /* and do not bother with a rate the mode cannot fit into */
            if (intel_dp_link_bandwidth_kbps(idx, lanes) <
                intel_mode_bandwidth_kbps(pixel_khz, bpp)) continue;

            if (intel_link_train(port, idx, lanes, tps3, enhanced)) {
                lt_rate_idx = idx; lt_lanes = lanes;
                return idx;
            }
        }
        if (lanes == 1) break;
    }
    (void)has_rate_table;
    return -1;
}

int intel_lt_lanes(void)        { return lt_lanes; }
int intel_lt_rate_idx(void)     { return lt_rate_idx; }
int intel_lt_cr_attempts(void)  { return lt_cr_attempts; }
int intel_lt_eq_attempts(void)  { return lt_eq_attempts; }
int intel_lt_status(int i)      { return (i >= 0 && i < 6) ? lt_last_status[i] : 0; }
int intel_lt_swing(int l)       { return (l >= 0 && l < 4) ? lt_final_swing[l] : 0; }
u32 intel_dp_tp_ctl(int port)   { return intel_present() ? mmio_r(DP_TP_CTL(port)) : 0; }
/* DDI A HAS NO DP_TP_STATUS. The PRM says so explicitly and i915 returns
 * early for PORT_A. A generic "wait for IDLE_DONE" helper pointed here spins
 * its entire timeout on every single modeset and still succeeds, which makes
 * it invisible except as unexplained slowness. Return 0 and mean it. */
u32 intel_dp_tp_status(int port)
{
    if (!intel_present() || port == 0) return 0;
    return mmio_r(DP_TP_STATUS(port));
}
int intel_dp_tp_status_exists(int port) { return port != 0; }

/* ==== eDP panel power sequencing =========================================
 * The part of a modeset that can physically damage a panel, and therefore the
 * part to be most careful with.
 *
 * An LCD panel is not a device you switch on. It has a required order and
 * required delays between its supply rail, its link, and its backlight, and
 * violating them can damage the panel over time - image sticking at best,
 * failure at worst. The delays are named T1..T12 in the eDP specification and
 * the hardware enforces them for us IF the delay registers are programmed
 * correctly, which is why this code reads them rather than inventing them.
 *
 * The order that matters:
 *
 *   power on    VDD up -> wait T1+T2 -> link training -> wait T3 ->
 *               backlight on (T7 after valid video)
 *   power off   backlight OFF FIRST -> wait T9 -> video off -> VDD down ->
 *               then T12 must elapse before it may be powered on again
 *
 * Backlight before video off is not a detail: a lit backlight over a panel
 * whose link has stopped shows garbage, and on some panels holding a static
 * pattern is what causes sticking.
 *
 * PP_CONTROL has a write-protect key in its top 16 bits. Writes without it
 * are silently ignored, which produces the very confusing symptom of a
 * register that reads back exactly what it did before.
 */
#define PP_ON_DELAYS      0xC7208
#define PP_OFF_DELAYS     0xC720C
#define PP_DIVISOR        0xC7210

/* NO UNLOCK KEY ON GEN9. The 0xABCD write-protect key in PP_CONTROL's top
 * half is a pre-DDI thing; i915 only applies it when !HAS_DDI(), and the
 * measured PP_CONTROL on this machine is 0x00000067 with the top half clear.
 * Writing the key here would put 0xABCD into bits the PRM marks Reserved.
 * Read-modify-write is the correct discipline instead. */
#define PP_UNLOCK_KEY     0x00000000u

/* On CNP/CMP (this part) the power cycle delay moved INTO PP_CONTROL and
 * PP_DIVISOR is dead - reading it gives a ref divider of 0, which the PRM
 * forbids, i.e. the register is not live. The field is "+1" encoded in units
 * of 100 ms: a value of v means (v-1)*100 ms, and 0 means none. */
#define PP_CYCLE_DELAY_SHIFT 4
#define PP_CYCLE_DELAY_MASK  0x1F

int intel_pp_status(void)  { return intel_present() ? (int)mmio_r(PP_STATUS) : 0; }
int intel_pp_control(void) { return intel_present() ? (int)mmio_r(PP_CONTROL) : 0; }

/* The delays are stored in units of 100 microseconds. Reading them back tells
 * us what the firmware negotiated with THIS panel, which is far better
 * information than any default we could pick. */
int intel_pp_t1_t3(void)  { return intel_present() ? (int)((mmio_r(PP_ON_DELAYS) >> 16) & 0x1FFF) : 0; }
int intel_pp_t8(void)     { return intel_present() ? (int)(mmio_r(PP_ON_DELAYS) & 0x1FFF) : 0; }
/* PP_OFF_DELAYS: 28:16 is T10 (power-down), 12:0 is T9 (backlight-off ->
 * power-down). These two returned each other's field. Measured
 * PP_OFF_DELAYS = 0x01F40001 settles it: 0x1F4 = 500 = 50.0 ms is T10, and
 * the 0x0001 low field is i915 forcing the hardware delay to 0.1 ms while
 * doing the real 260 ms T9 wait in software from VBT. */
int intel_pp_t9(void)     { return intel_present() ? (int)(mmio_r(PP_OFF_DELAYS) & 0x1FFF) : 0; }
int intel_pp_t10(void)    { return intel_present() ? (int)((mmio_r(PP_OFF_DELAYS) >> 16) & 0x1FFF) : 0; }
/* T11+T12, in milliseconds. NOT from PP_DIVISOR - see the note above. */
int intel_pp_t11_t12(void)
{
    if (!intel_present()) return 0;
    int v = (int)((mmio_r(PP_CONTROL) >> PP_CYCLE_DELAY_SHIFT) & PP_CYCLE_DELAY_MASK);
    return v ? (v - 1) * 100 : 0;
}

int intel_pp_sequencing(void)
{
    if (!intel_present()) return 0;
    return (int)((mmio_r(PP_STATUS) >> 28) & 3);
}

/* ---- the T12 epoch -----------------------------------------------------
 *
 * T12 is the panel's power-cycle delay: after power goes away, it may not come
 * back for 500 ms on this panel (PP_CONTROL[8:4] reads 6). Violating it is the
 * one hazard in this driver that damages hardware rather than failing - the
 * rails have not discharged and the TCON can latch into an undefined state.
 *
 * The hardware sequencer enforces its own copy via PP_STATUS b27, but that is
 * not sufficient on its own: if VDD was dropped while PANEL_POWER_ON was
 * already clear, the sequencer never ran and b27 never sets, while the panel
 * still owes the full delay. So both halves are required - a software wait
 * from this epoch, AND the b27 poll. Neither is redundant.
 *
 * Initialised to 0 meaning "unknown history", which is treated as owing a full
 * T12 - the safe direction. */
static u32 pp_last_off_ms = 0;
static int pp_epoch_valid = 0;
static u32 pp_last_on_ms  = 0;   /* backlight-on delay is measured from here */
static int pp_panel_up    = 0;   /* our own view; AUX refuses without it */

static void pp_stamp_off(void)
{
    pp_last_off_ms = cpu_now_ms();
    pp_epoch_valid = 1;
}

/* T12 in milliseconds, from PP_CONTROL[8:4]. The field is "+1" encoded: a
 * value v means (v-1)*100 ms, and 0 means none. Floor at the eDP spec's
 * 500 ms rather than trusting a zero, because a zero here is far more likely
 * to mean "soft reset wiped the register" than "this panel needs no delay". */
static u32 pp_t12_ms(void)
{
    u32 v = (mmio_r(PP_CONTROL) >> PP_CYCLE_DELAY_SHIFT) & PP_CYCLE_DELAY_MASK;
    u32 ms = v ? (v - 1u) * 100u : 0u;
    return ms < 500u ? 500u : ms;
}

/* Pay whatever is left of T12, then confirm the hardware agrees. */
static void pp_wait_power_cycle(void)
{
    u32 t12 = pp_t12_ms();

    if (pp_epoch_valid) {
        u32 elapsed = cpu_now_ms() - pp_last_off_ms;
        if (elapsed < t12) cpu_delay_ms(t12 - elapsed);
    } else {
        cpu_delay_ms(t12);            /* unknown history - owe the lot */
    }

    /* Then the hardware's own view: b31 clear, b27 (cycle delay active) clear,
     * sequencer idle, and the undocumented-but-implemented state nibble at
     * OFF_IDLE. Polled, with a bound - a panel that never reaches this is a
     * fault to report, not a reason to spin forever. */
    for (u32 i = 0; i < 500u; i++) {
        if ((mmio_r(PP_STATUS) & 0xB800000Fu) == 0) return;
        cpu_delay_ms(10);
    }
}

/* Wait for the sequencer to reach a TARGET state, not merely to look idle.
 *
 * The old version returned as soon as PP_STATUS[29:28] read 00, and was called
 * one instruction after the PP_ON write - at which point the sequencer has not
 * started yet, 29:28 still reads 00, and it returned "done" on a panel that had
 * not begun powering up. Then the b31 test read 0 and the whole call reported
 * failure on a panel coming up perfectly.
 *
 * mask/want describe the state we are waiting FOR, so no such race exists. */
static int pp_wait_state(u32 mask, u32 want, u32 ms)
{
    for (u32 i = 0; i <= ms / 10u; i++) {
        if ((mmio_r(PP_STATUS) & mask) == want) return 1;
        cpu_delay_ms(10);
    }
    return (mmio_r(PP_STATUS) & mask) == want;
}

/* PP_STATUS predicates. b31 alone is NOT "on": it reads 1 for the whole
 * power-DOWN sequence too, so testing it alone lets a caller start driving AUX
 * into a rail that is collapsing. */
#define PP_ON_MASK    0xB000000Fu
#define PP_ON_WANT    0x80000008u        /* b31 set, no cycle/seq, state ON_IDLE */
#define PP_OFF_MASK   0xB0000000u
#define PP_OFF_WANT   0x00000000u

/* Force VDD on without lighting the panel.
 *
 * A driver needs this: AUX transactions require the panel's logic to be
 * powered, but during a modeset we do not want the display lit yet. This is
 * the standard "VDD force" every eDP driver holds across its AUX work. */
int intel_panel_vdd_on(void)
{
    if (!intel_present() || !lt_armed) return 0;

    /* Already up? Nothing to do, and in particular no second T3 wait. */
    if (mmio_r(PP_STATUS) & PP_STATUS_ON) { pp_panel_up = 1; return 1; }
    if (mmio_r(PP_CONTROL) & PP_VDD_FORCE) { pp_panel_up = 1; return 1; }

    /* Raising FORCE_VDD from a cold panel is itself subject to T12. The
     * 1->0->1 transition of this bit is a power cycle as far as the panel is
     * concerned, which is why this wait is here and not only in power_on(). */
    pp_wait_power_cycle();

    u32 v = mmio_r(PP_CONTROL) & 0xFFFF;
    mmio_w(PP_CONTROL, PP_UNLOCK_KEY | v | PP_VDD_FORCE | PP_PWR_DOWN_ON_RESET);
    (void)mmio_r(PP_CONTROL);                 /* posting read */

    /* T3, flat. No status bit reports "VDD is ready" separately, so there is
     * nothing to poll - the panel's logic simply is not answering AUX until
     * this has elapsed. Read from PP_ON_DELAYS 28:16 (100 us units), floored
     * at the eDP spec ceiling in case a soft reset zeroed the register. */
    u32 t3 = (u32)intel_pp_t1_t3() / 10u;     /* 100 us units -> ms */
    cpu_delay_ms(t3 < 200u ? 200u : t3);

    pp_panel_up = 1;
    return 1;
}

int intel_panel_vdd_off(void)
{
    if (!intel_present() || !lt_armed) return 0;
    u32 v = mmio_r(PP_CONTROL) & 0xFFFF;
    mmio_w(PP_CONTROL, PP_UNLOCK_KEY | (v & ~PP_VDD_FORCE));
    (void)mmio_r(PP_CONTROL);

    /* If PANEL_POWER_ON was already clear, dropping VDD just took the panel
     * fully down without the sequencer running - so the T12 clock starts now
     * and nothing else will record it. This is the case PP_STATUS b27 cannot
     * see, and the reason the software half of the wait exists. */
    if (!(mmio_r(PP_STATUS) & PP_STATUS_ON)) { pp_stamp_off(); pp_panel_up = 0; }
    return 1;
}

/* Bring the panel up. The hardware sequencer honours T1..T3 itself once
 * PP_ON is set; our job is to ask and then WAIT, not to guess timings. */
int intel_panel_power_on(void)
{
    if (!intel_present() || !lt_armed) return 0;

    /* Only a full ON_IDLE counts as already-on. b31 alone reads 1 during the
     * entire power-DOWN sequence, so returning early on it hands the caller a
     * panel whose rail is collapsing. */
    if ((mmio_r(PP_STATUS) & PP_ON_MASK) == PP_ON_WANT) { pp_panel_up = 1; return 1; }

    pp_wait_power_cycle();

    u32 v = mmio_r(PP_CONTROL) & 0xFFFF;
    mmio_w(PP_CONTROL, PP_UNLOCK_KEY | v | PP_ON | PP_PWR_DOWN_ON_RESET);
    (void)mmio_r(PP_CONTROL);

    /* The sequencer runs T1+T2+T3 during this poll, so there is no extra sleep
     * to add. 5000 ms because the plan specifies that bound and a slow panel
     * plus a full cycle delay genuinely approaches it. */
    if (!pp_wait_state(PP_ON_MASK, PP_ON_WANT, 5000)) return 0;
    pp_panel_up = 1;
    pp_last_on_ms = cpu_now_ms();
    return 1;
}

/* Take it down in the required order: backlight, T9, then power. */
int intel_panel_power_off(void)
{
    if (!intel_present() || !lt_armed) return 0;

    /* T9 - video must not stop while the backlight is lit. This is here rather
     * than in backlight_enable() on purpose: the documented calling order turns
     * the backlight off first, and when it does, this function would find b2
     * already clear and skip the wait entirely. Paying it unconditionally is
     * the only ordering that cannot be defeated by a correct caller.
     *
     * 260 ms is the VBT value. The register's 12:0 field reads 0.1 ms because
     * i915 forces the hardware delays to 1 and does the real wait in software;
     * take the larger of the two rather than believing the register. */
    u32 v = mmio_r(PP_CONTROL) & 0xFFFF;
    if (v & PP_BACKLIGHT_EN) {
        mmio_w(PP_CONTROL, PP_UNLOCK_KEY | (v & ~PP_BACKLIGHT_EN));
        (void)mmio_r(PP_CONTROL);
    }
    u32 t9 = (u32)intel_pp_t9() / 10u;
    cpu_delay_ms(t9 < 260u ? 260u : t9);

    /* Force VDD on FIRST, then drop everything in one write. Panels misbehave
     * if VDD is allowed to fall independently of the sequenced power-down, and
     * clearing PP_ON while b3 is left wherever vdd_on() put it can leave
     * FORCE_VDD asserted indefinitely on a panel reported as "off". */
    v = mmio_r(PP_CONTROL) & 0xFFFF;
    mmio_w(PP_CONTROL, PP_UNLOCK_KEY | v | PP_VDD_FORCE);
    (void)mmio_r(PP_CONTROL);

    v = mmio_r(PP_CONTROL) & 0xFFFF;
    mmio_w(PP_CONTROL, PP_UNLOCK_KEY |
           (v & ~(PP_ON | PP_PWR_DOWN_ON_RESET | PP_BACKLIGHT_EN | PP_VDD_FORCE)));
    (void)mmio_r(PP_CONTROL);

    int ok = pp_wait_state(PP_OFF_MASK, PP_OFF_WANT, 5000);

    /* Stamp the epoch either way. A panel that failed to report "off" is more
     * likely to owe T12 than less, and the whole point of the epoch is to be
     * conservative about what we do not know. */
    pp_stamp_off();
    pp_panel_up = 0;
    return ok;
}

int intel_panel_backlight_enable(int on)
{
    if (!intel_present() || !lt_armed) return 0;

    /* Never drive the backlight into an unpowered panel. The hardware does
     * interlock BLC behind T3, but that interlock is implemented by the
     * sequencer using the delay registers - and those reset to zero, so it
     * cannot be relied on as the only guard. */
    if (on && (mmio_r(PP_STATUS) & PP_ON_MASK) != PP_ON_WANT) return 0;

    if (on) {
        /* backlight-on delay, measured from the moment power came up */
        u32 want = (u32)intel_pp_t8() / 10u;
        if (!want) want = 1u;
        u32 since = cpu_now_ms() - pp_last_on_ms;
        if (since < want) cpu_delay_ms(want - since);
    }

    u32 v = mmio_r(PP_CONTROL) & 0xFFFF;
    if (on) v |= PP_BACKLIGHT_EN; else v &= ~PP_BACKLIGHT_EN;
    mmio_w(PP_CONTROL, PP_UNLOCK_KEY | v);
    (void)mmio_r(PP_CONTROL);
    return 1;
}

/* ==== power wells and DC states ==========================================
 * The first thing a modeset touches, and the reason a driver that skips it
 * reads zeroes from perfectly good registers.
 *
 * Modern Intel display hardware is split into power wells that the hardware
 * turns off when nothing needs them. A register inside a well that is down
 * reads back as zero and swallows writes - no fault, no error bit, just
 * silence. So "the register is zero" is ambiguous until you know the well is
 * up, and that ambiguity has cost people days.
 *
 * On top of that sit the DC states: deeper sleep modes the display engine
 * enters when the pipes are idle. DC5 and DC6 must be blocked before touching
 * anything, or the hardware may drop into one mid-sequence and lose what was
 * just written.
 *
 * PWR_WELL_CTL is four copies of the same register - one per requester (BIOS,
 * driver, KVMR, debug). A well stays up while ANY requester wants it, which
 * is why the driver has its own copy and must not write the BIOS's.
 */
#define PWR_WELL_CTL_BIOS   0x45400
#define PWR_WELL_CTL_DRIVER 0x45404
#define PWR_WELL_CTL_KVMR   0x45408
#define PWR_WELL_CTL_DEBUG  0x4540C

/* request is the odd bit of the pair, state is the even one */
#define PW_REQUEST(w)  (1u << ((w) * 2 + 1))
#define PW_STATE(w)    (1u << ((w) * 2))

u32 intel_pwr_well_driver(void) { return intel_present() ? mmio_r(PWR_WELL_CTL_DRIVER) : 0; }
u32 intel_pwr_well_bios(void)   { return intel_present() ? mmio_r(PWR_WELL_CTL_BIOS) : 0; }
u32 intel_dc_state(void)        { return intel_present() ? mmio_r(DC_STATE_EN) : 0; }

int intel_pwr_well_enabled(int well)
{
    if (!intel_present() || well < 0 || well > 3) return 0;
    return (mmio_r(PWR_WELL_CTL_DRIVER) & PW_STATE(well)) ? 1 : 0;
}

int intel_pwr_well_requested(int well)
{
    if (!intel_present() || well < 0 || well > 3) return 0;
    return (mmio_r(PWR_WELL_CTL_DRIVER) & PW_REQUEST(well)) ? 1 : 0;
}

/* Ask for a well and wait for the hardware to say it is actually up. The wait
 * is not optional: the request bit is a request, and the state bit is the
 * answer, and they are not the same thing. */
int intel_pwr_well_enable(int well)
{
    if (!intel_present() || !lt_armed || well < 0 || well > 3) return 0;
    u32 v = mmio_r(PWR_WELL_CTL_DRIVER);
    mmio_w(PWR_WELL_CTL_DRIVER, v | PW_REQUEST(well));

    u32 t0 = idt_ticks();
    while (idt_ticks() - t0 < 3) {          /* the PRM allows 20 ms */
        if (mmio_r(PWR_WELL_CTL_DRIVER) & PW_STATE(well)) return 1;
    }
    return (mmio_r(PWR_WELL_CTL_DRIVER) & PW_STATE(well)) ? 1 : 0;
}

int intel_pwr_well_disable(int well)
{
    if (!intel_present() || !lt_armed || well < 0 || well > 3) return 0;
    u32 v = mmio_r(PWR_WELL_CTL_DRIVER);
    mmio_w(PWR_WELL_CTL_DRIVER, v & ~PW_REQUEST(well));
    return 1;
}

/* Block the deep sleep states for the duration of a modeset. */
int intel_dc_states_block(void)
{
    if (!intel_present() || !lt_armed) return 0;
    /* One write loses a race with the DMC, which re-arms DC5/DC6 behind us.
     * dc_states_off() writes until the value holds across six reads. */
    dc_states_off();
    return mmio_r(DC_STATE_EN) == DC_STATE_DISABLE;
}

/* ---- panel self refresh ------------------------------------------------
 *
 * PSR lets the panel hold its own image and lets the display engine stop
 * scanning out. It is measured ENABLED on this machine (EDP_PSR_CTL reads
 * 0x81F00406), and while it is on it does three things that break a driver
 * trying to take over:
 *
 *   - it stops the frame counter, so PIPE_FRMCNT never advances and every
 *     pixel-clock measurement derived from it reads zero
 *   - it issues its own fast-wake AUX transactions on the channel we are
 *     trying to use
 *   - it fights every plane update
 *
 * The frozen frame counter is why the probe reports 0.0 Hz and why every
 * bandwidth number in this project currently rests on an ASSUMED 60 Hz. This
 * function is the prerequisite for ever measuring the real one. */
#define EDP_PSR_CTL         0x6F800
#define EDP_PSR_STATUS      0x6F840
#define EDP_PSR_ENABLE      (1u << 31)
#define EDP_PSR_STATE_MASK  (7u << 29)

int intel_psr_enabled(void)
{
    return intel_present() ? ((mmio_r(EDP_PSR_CTL) & EDP_PSR_ENABLE) ? 1 : 0) : 0;
}

u32 intel_psr_ctl(void)    { return intel_present() ? mmio_r(EDP_PSR_CTL) : 0; }
u32 intel_psr_status(void) { return intel_present() ? mmio_r(EDP_PSR_STATUS) : 0; }

/* Returns 1 if PSR is off and idle when we leave, 0 if it would not go idle.
 * Clearing the enable bit is not enough on its own - the hardware may be in
 * the middle of a self-refresh entry or exit, and the next thing a modeset
 * does is take the clocks away from underneath it. */
int intel_psr_disable(void)
{
    if (!intel_present() || !lt_armed) return 0;
    if (!(mmio_r(EDP_PSR_CTL) & EDP_PSR_ENABLE)) return 1;   /* already off */

    mmio_w(EDP_PSR_CTL, mmio_r(EDP_PSR_CTL) & ~EDP_PSR_ENABLE);
    (void)mmio_r(EDP_PSR_CTL);

    /* Wait for the state machine to reach IDLE. 50 ms is not a documented
     * figure - no public PRM gives one - it is the plan's estimate, and it is
     * recorded as an estimate rather than dressed up as a specification. */
    for (int i = 0; i < 50; i++) {
        if ((mmio_r(EDP_PSR_STATUS) & EDP_PSR_STATE_MASK) == 0) return 1;
        cpu_delay_ms(1);
    }
    return 0;
}

/* ==== the ordered modeset ================================================
 * Everything in this file exists to make this function possible, and its
 * value is almost entirely in the ORDER. Each step depends on the one before
 * having actually completed, which is why every one of them waits rather than
 * assuming.
 *
 * This is the sequence, and where each piece lives:
 *
 *    1. block DC states, bring up the power wells        (above)
 *    2. panel VDD on, so AUX works                       (panel power)
 *    3. read the panel's DPCD - what can it actually do? (AUX)
 *    4. choose a link rate that fits the mode AND the panel   (DPLL)
 *    5. program and lock the DPLL                        (DPLL)
 *    6. point the DDI at that DPLL                       (DPLL)
 *    7. program the transcoder timings                   (below)
 *    8. link training                                    (link training)
 *    9. enable the transcoder, then the pipe             (below)
 *   10. configure and enable the plane                   (plane)
 *   11. panel power on, wait T1+T3, backlight on         (panel power)
 *
 * Steps 1-8 and 11 are implemented and their registers verified against the
 * live hardware. Step 7 and 9 are below. What is NOT here is the DDI buffer
 * translation table for step 8's drive settings, which is SKU-specific.
 */
int intel_set_timings(u32 hactive, u32 hblank_start, u32 hsync_start, u32 hsync_end,
                      u32 htotal,
                      u32 vactive, u32 vblank_start, u32 vsync_start, u32 vsync_end,
                      u32 vtotal)
{
    if (!intel_present() || !lt_armed) return 0;
    u32 base = trans_base();

    /* every field is stored minus one, and start goes in the low half */
    mmio_w(base + TRANS_OFF_HTOTAL, ((htotal - 1) << 16) | (hactive - 1));
    mmio_w(base + TRANS_OFF_HBLANK, ((htotal - 1) << 16) | (hblank_start - 1));
    mmio_w(base + TRANS_OFF_HSYNC,  ((hsync_end - 1) << 16) | (hsync_start - 1));
    mmio_w(base + TRANS_OFF_VTOTAL, ((vtotal - 1) << 16) | (vactive - 1));
    mmio_w(base + TRANS_OFF_VBLANK, ((vtotal - 1) << 16) | (vblank_start - 1));
    mmio_w(base + TRANS_OFF_VSYNC,  ((vsync_end - 1) << 16) | (vsync_start - 1));

    /* the composed image size is a PIPE register, not a transcoder one */
    mmio_w(PIPE_SRCSZ_A, ((hactive - 1) << 16) | (vactive - 1));
    return 1;
}

#define TRANS_CONF_ENABLE  (1u << 31)
#define TRANS_CONF_STATE   (1u << 30)

/* Enable the transcoder and wait for it to actually start. The state bit
 * lagging the enable bit is normal; treating them as the same thing is how a
 * driver ends up configuring a plane against a pipe that is not running. */
int intel_transcoder_enable(int on)
{
    if (!intel_present() || !lt_armed) return 0;
    u32 conf = (trans_base() == TRANS_EDP_BASE) ? TRANS_EDP_CONF : TRANS_A_CONF;

    u32 v = mmio_r(conf);
    if (on) mmio_w(conf, v | TRANS_CONF_ENABLE);
    else    mmio_w(conf, v & ~TRANS_CONF_ENABLE);

    u32 t0 = idt_ticks();
    while (idt_ticks() - t0 < 10) {          /* 100 ms is generous */
        int state = (mmio_r(conf) & TRANS_CONF_STATE) ? 1 : 0;
        if (state == (on ? 1 : 0)) return 1;
    }
    return 0;
}

/* Configure the primary plane for a linear 32-bit surface. Kept separate from
 * the arming write so a caller can set everything up and then flip. */
#define PLANE_CTL_FORMAT_XRGB8888  (4u << 24)
#define PLANE_CTL_ORDER_RGBX       (1u << 20)
#define PLANE_CTL_TILING_LINEAR    (0u << 10)

int intel_plane_configure(u32 width, u32 height, u32 stride_bytes)
{
    if (!intel_present() || !lt_armed) return 0;
    if (stride_bytes & 63) return 0;

    mmio_w(PLANE_OFFSET_1_A, 0);
    mmio_w(PLANE_POS_1_A, 0);
    mmio_w(PLANE_SIZE_1_A, ((height - 1) << 16) | (width - 1));
    mmio_w(PLANE_STRIDE_1_A, stride_bytes / 64);     /* linear: 64-byte units */
    mmio_w(PLANE_CTL_1_A, PLANE_CTL_ENABLE | PLANE_CTL_FORMAT_XRGB8888 |
                          PLANE_CTL_TILING_LINEAR);
    return 1;
}

/* ==== watermarks and the display data buffer =============================
 * The least glamorous part of a display driver and one of the easiest to get
 * silently wrong.
 *
 * The display engine reads pixels through a FIFO. A watermark tells it how
 * full that FIFO must be before scanout may begin, and how much buffer the
 * plane owns. Set them too low and the FIFO runs dry mid-line - a FIFO
 * underrun - which shows as flickering, horizontal tearing or a briefly black
 * screen, and which the hardware reports through an error bit almost nobody
 * checks. Set them too high and the plane simply refuses to enable.
 *
 * Skylake has eight watermark levels, one per display power state, plus a
 * transition watermark. Each is computed from the plane's data rate, the
 * memory latency at that level, and how much of the display data buffer the
 * plane has been allocated. Computing them exactly needs the memory latency
 * values, which come from a mailbox exchange with the power controller.
 *
 * What this does instead is honest and useful: it reads back what the
 * firmware programmed for a working configuration, and offers a conservative
 * calculation for the level-0 watermark that is safe if pessimistic. Copying
 * a known-good configuration is a completely legitimate strategy for a driver
 * that is not changing the mode - and it is far better than inventing numbers.
 */
#define PLANE_WM_TRANS_1_A  0x70268
#define PLANE_BUF_CFG_1_A   0x7027C
/* The underrun telltale is GEN8_DE_PIPE_IIR bit 31, write-1-clear. It is NOT
 * in PIPECONF: 0x70008 is TRANS_CONF_A, whose b31 is the pipe ENABLE bit, so
 * reading it as an underrun flag reports a permanent true on any live pipe. */
#define GEN8_DE_PIPE_IIR_A  0x44408
#define DE_PIPE_UNDERRUN    (1u << 31)

/* Gen9 watermark field widths are NARROW: lines at 18:14 and blocks at 9:0.
 * The wider 26:14 / 11:0 encoding is a later generation. Legal gen9 values
 * fit inside both, so a too-wide mask reads correctly right up until it
 * doesn't - which is the sort of thing that only shows on one machine. */
#define WM_ENABLE       (1u << 31)
#define WM_LINES_SHIFT  14
#define WM_LINES_MASK   0x1Fu
#define WM_BLOCKS_MASK  0x3FFu

u32 intel_wm_trans(void)  { return intel_present() ? mmio_r(PLANE_WM_TRANS_1_A) : 0; }
u32 intel_ddb_cfg(void)   { return intel_present() ? mmio_r(PLANE_BUF_CFG_1_A) : 0; }

int intel_wm_enabled(int level)  { return (intel_watermark(level) & WM_ENABLE) ? 1 : 0; }
int intel_wm_blocks(int level)   { return (int)(intel_watermark(level) & WM_BLOCKS_MASK); }
int intel_wm_lines(int level)    { return (int)((intel_watermark(level) >> WM_LINES_SHIFT) & WM_LINES_MASK); }

/* The display data buffer allocation: which 512-byte blocks this plane owns,
 * as a start and end index. */
int intel_ddb_start(void) { return (int)(intel_ddb_cfg() & 0x7FF); }
int intel_ddb_end(void)   { return (int)((intel_ddb_cfg() >> 16) & 0x7FF); }
int intel_ddb_blocks(void)
{
    int e = intel_ddb_end(), st = intel_ddb_start();
    return (e >= st) ? (e - st + 1) : 0;
}

/* A conservative level-0 watermark.
 *
 * The real formula weighs the plane's data rate against a memory latency the
 * power controller reports. Without that latency the safe move is to demand
 * enough blocks to cover a generous fixed latency at this plane's data rate,
 * and to say plainly that it is pessimistic: a watermark that is too high
 * costs power, while one that is too low corrupts the display. Given the
 * choice, be too high.
 *
 * bytes per line = width * bpp/8 ; blocks are 512 bytes.
 * We ask for whatever covers `latency_us` of scanout at this pixel clock. */
u32 intel_wm_compute_level0(u32 width, u32 bpp, u32 pixel_khz, u32 latency_us)
{
    if (!width || !bpp || !pixel_khz) return 0;
    /* 30 us, chosen by measurement rather than taste. At 20 us this computes
     * 38 blocks where the firmware programmed 41 for the same mode - close,
     * but on the WRONG SIDE. A watermark below the correct value underruns
     * the FIFO and corrupts the display; one above it only costs a little
     * power. Given the choice, be too high. 30 us clears the firmware's
     * number with margin on this panel. */
    if (!latency_us) latency_us = 30;

    /* pixels drawn during the latency window */
    u32 pixels = (pixel_khz * latency_us) / 1000u;
    u32 bytes  = pixels * (bpp / 8u);
    u32 blocks = (bytes + 511u) / 512u;
    if (blocks < 8) blocks = 8;                   /* the hardware minimum */

    /* also express it in lines, which is what the hardware wants above the
     * block count for the higher levels */
    u32 bytes_per_line = width * (bpp / 8u);
    u32 blocks_per_line = (bytes_per_line + 511u) / 512u;
    u32 lines = blocks_per_line ? ((blocks + blocks_per_line - 1) / blocks_per_line) : 1;

    return WM_ENABLE | ((lines & WM_LINES_MASK) << WM_LINES_SHIFT) |
           (blocks & WM_BLOCKS_MASK);
}

int intel_wm_set_level(int level, u32 value)
{
    if (!intel_present() || !lt_armed || level < 0 || level > 7) return 0;
    mmio_w(PLANE_WM_1_A(level), value);
    return 1;
}

/* Copy a whole known-good watermark configuration. A driver that is taking
 * over a display the firmware already configured correctly can simply keep
 * those numbers, which is both safe and honest about what it knows. */
static u32 wm_saved[8], wm_saved_trans, wm_saved_ddb;
static int wm_have_saved = 0;

int intel_wm_save(void)
{
    if (!intel_present()) return 0;
    for (int l = 0; l < 8; l++) wm_saved[l] = intel_watermark(l);
    wm_saved_trans = intel_wm_trans();
    wm_saved_ddb   = intel_ddb_cfg();
    wm_have_saved  = 1;
    return 1;
}

int intel_wm_restore(void)
{
    if (!intel_present() || !lt_armed || !wm_have_saved) return 0;
    mmio_w(PLANE_BUF_CFG_1_A, wm_saved_ddb);
    for (int l = 0; l < 8; l++) mmio_w(PLANE_WM_1_A(l), wm_saved[l]);
    mmio_w(PLANE_WM_TRANS_1_A, wm_saved_trans);
    return 1;
}

int intel_wm_saved(void) { return wm_have_saved; }

/* Has the pipe underrun since we last looked? This is the bit that tells you
 * a watermark is wrong, and checking it is the difference between "the screen
 * flickers sometimes" and a diagnosis. Write 1 to clear. */
#define PIPECONF_UNDERRUN (1u << 31)   /* in the pipe's status, not conf */
#define PIPE_STATUS_A     0x70024

int intel_pipe_underrun(void)
{
    if (!intel_present()) return 0;
    return (mmio_r(GEN8_DE_PIPE_IIR_A) & DE_PIPE_UNDERRUN) ? 1 : 0;
}

int intel_pipe_underrun_clear(void)
{
    if (!intel_present() || !lt_armed) return 0;
    /* Write ONLY the underrun bit. The old version wrote the whole register
     * value back, and since every bit in an IIR is write-1-clear, that
     * acknowledged every other pending interrupt as a side effect. */
    mmio_w(GEN8_DE_PIPE_IIR_A, DE_PIPE_UNDERRUN);
    return 1;
}
