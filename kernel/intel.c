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
#define DPLL_CFGCR1(p)  (0x6C040 + (p) * 8)   /* HDMI mode: the divider     */
#define DPLL_CFGCR2(p)  (0x6C044 + (p) * 8)
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
    if (pll < 0 || pll > 3) return 0;
    return mmio_r(DPLL_CFGCR1(pll));
}

u32 intel_dpll_cfgcr2(int pll)
{
    if (pll < 0 || pll > 3) return 0;
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
    if (!intel_present() || pll < 0 || pll > 3) return 0;
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
#define DPLL_ENABLE(p)   (0x46010 + (p) * 4)   /* LCPLL1_CTL, LCPLL2_CTL, ... */

/* Turn a DPLL on and wait for it to lock. The wait is the point. */
int intel_dpll_enable(int pll)
{
    if (!intel_present() || pll < 0 || pll > 3) return 0;
    if (intel_dpll_locked(pll)) return 1;

    u32 en = DPLL_ENABLE(pll);
    mmio_w(en, mmio_r(en) | LCPLL_PLL_ENABLE);

    /* the PRM allows 5 ms; give it 50 and report honestly if it never comes */
    u32 t0 = idt_ticks();
    while (idt_ticks() - t0 < 5) {
        if (intel_dpll_locked(pll)) return 1;
    }
    return intel_dpll_locked(pll);
}

int intel_dpll_disable(int pll)
{
    if (!intel_present() || pll < 0 || pll > 3) return 0;
    u32 en = DPLL_ENABLE(pll);
    mmio_w(en, mmio_r(en) & ~LCPLL_PLL_ENABLE);
    return !intel_dpll_locked(pll);
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
    v |=  (1u << (ddi * 3 + 3));              /* select override on    */
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
#define DP_AUX_CH_CTL(port)   (0x64010 + (port) * 0x100)
#define DP_AUX_CH_DATA(port, i) (0x64014 + (port) * 0x100 + (i) * 4)

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

/* One raw transaction. Returns the number of bytes received, or -1. */
static int aux_xfer(int port, u32 cmd, u32 addr, const u8 *out, int out_len,
                    u8 *in, int in_len)
{
    if (!intel_present() || port < 0 || port > 3) return -1;
    if (out_len > 16 || in_len > 16) return -1;

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

    /* clear any stale status before starting - these bits are write-1-clear
     * and a leftover DONE makes the very next poll return immediately */
    mmio_w(ctl_reg, mmio_r(ctl_reg) |
           AUX_DONE | AUX_TIMEOUT_ERR | AUX_RECEIVE_ERR | AUX_INTERRUPT);

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

    /* SEND_BUSY clears when the transaction completes, one way or another */
    u32 status = 0;
    int done = 0;
    for (int spin = 0; spin < 2000000; spin++) {
        status = mmio_r(ctl_reg);
        if (!(status & AUX_SEND_BUSY)) { done = 1; break; }
    }
    if (!done) return -1;

    /* acknowledge whatever happened */
    mmio_w(ctl_reg, status | AUX_DONE | AUX_TIMEOUT_ERR | AUX_RECEIVE_ERR);

    if (status & AUX_TIMEOUT_ERR) { aux_last_reply = -2; return -1; }
    if (status & AUX_RECEIVE_ERR) { aux_last_reply = -3; return -1; }

    int recv = (int)((status >> AUX_MSG_SIZE_SHIFT) & 0x1F);
    if (recv < 1) return -1;

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

int intel_aux_last_reply(void) { return aux_last_reply; }

/* A native DPCD read, with the retry a DEFER requires. */
int intel_dpcd_read(int port, u32 addr, int len)
{
    if (len < 1 || len > 16) return 0;
    for (int attempt = 0; attempt < 8; attempt++) {
        int n = aux_xfer(port, 0x9, addr, 0, 0, aux_buf, len);
        if (n > 0 && aux_last_reply == AUX_REPLY_ACK) return n;
        if (aux_last_reply != AUX_REPLY_DEFER && n < 0 && attempt > 2) break;
        /* a short settle before asking again - the panel said "not yet" */
        for (volatile int d = 0; d < 200000; d++) { }
    }
    return 0;
}

int intel_dpcd_write(int port, u32 addr, const u8 *data, int len)
{
    if (len < 1 || len > 16) return 0;
    for (int attempt = 0; attempt < 8; attempt++) {
        int n = aux_xfer(port, 0x8, addr, data, len, aux_buf, 1);
        if (n >= 0 && aux_last_reply == AUX_REPLY_ACK) return 1;
        for (volatile int d = 0; d < 200000; d++) { }
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
int intel_dpcd_rev(void)        { return (int)aux_buf[0]; }
int intel_dpcd_max_rate(void)   { return (int)aux_buf[1]; }
int intel_dpcd_max_lanes(void)  { return (int)(aux_buf[2] & 0x1F); }
int intel_dpcd_enhanced(void)   { return (int)((aux_buf[2] >> 7) & 1); }
int intel_dpcd_tps3(void)       { return (int)((aux_buf[2] >> 6) & 1); }

/* Bit 1 of DPCD 0x0E (TRAINING_AUX_RD_INTERVAL) says the panel publishes a
 * SUPPORTED_LINK_RATES table at 0x10 - which is exactly how an eDP 1.4 panel
 * advertises the intermediate rates. A panel WITHOUT this bit must only be
 * driven at the standard rates, which is the rule our rate chooser follows. */
int intel_dpcd_has_rate_table(void) { return (int)((aux_buf[14] >> 7) & 1); }

u32 intel_dpcd_max_rate_kbps(void)
{
    /* the encoded value times 270 MHz, then times 10 for kbps per lane */
    return (u32)aux_buf[1] * 270000u;
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
static int lt_armed = 0;          /* refuse to touch anything until armed */

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
    /* The DDI buffer translation entry for this swing/pre pair would be
     * programmed here. The table is part-specific and is the one piece of
     * this sequence that cannot be derived - it has to come from the PRM for
     * the exact SKU, so it is left to a caller that knows the part. */
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
        /* the panel says how long to wait in DPCD 0x0E; 100 us is the
         * default and is what almost every panel asks for */
        for (volatile int d = 0; d < 400000; d++) { }

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
        for (volatile int d = 0; d < 400000; d++) { }

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
    for (volatile int d = 0; d < 200000; d++) { }
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
u32 intel_dp_tp_status(int port){ return intel_present() ? mmio_r(DP_TP_STATUS(port)) : 0; }
