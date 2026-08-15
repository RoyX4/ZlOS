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
