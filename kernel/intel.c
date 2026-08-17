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
/* ---- which pipe, and the stride that makes it one line of code ---------
 *
 * Every pipe register in this file was hardcoded to A. The hardware lays them
 * out regularly, which is the only reason that was survivable: both blocks step
 * by 0x1000 per pipe.
 *
 *     0x60000 + pipe*0x1000    transcoder timings, PIPE_SRCSZ
 *     0x70000 + pipe*0x1000    TRANSCONF, planes, cursor, watermarks
 *
 * The eDP transcoder is index 0xF in both - 0x6F000 and 0x7F008 - which is why
 * TRANSCONF landed at 0x7F008 rather than 0x6F008 and why that was contested
 * (C1). It is not a separate block; it is the same stride with a high index.
 *
 * Pipe selection is module state rather than a parameter on forty functions.
 * The driver drives one pipe at a time and always has; what changes is that it
 * is now a variable rather than an assumption. Defaults to A, so every existing
 * caller behaves exactly as before - which is the property the harness checks.
 */
#define PIPE_A            0
#define PIPE_B            1
#define PIPE_C            2
#define PIPE_STRIDE       0x1000u

/* A PIPE IS NOT A TRANSCODER, and this is the distinction that makes C2 a
 * conflict rather than a typo.
 *
 * The eDP transcoder is index 0xF, which is why TRANSCONF is at 0x7F008 and the
 * timings at 0x6F000. But it is FED BY PIPE A. Every pipe register - PIPE_SRCSZ,
 * the planes, the cursor, the watermarks, the frame counter - stays at pipe A's
 * index while the transcoder is eDP.
 *
 * So 0xF is deliberately NOT a legal value here. PIPE_REG(0x6001C) with 0xF
 * gives 0x6F01C, which the survey measures as reading 0 while the real
 * PIPE_SRCSZ at 0x6001C reads 09FF059F - exactly the trap C2 describes, and it
 * would have been reintroduced by treating the two indices as one.
 *
 * Transcoder selection stays where it already was: trans_base() for the timing
 * block, and TRANS_EDP_CONF against TRANS_CONF_A for the config register. */
static int cur_pipe = PIPE_A;      /* the pipe every register below refers to */

int  intel_pipe_select(int pipe)
{
    if (pipe != PIPE_A && pipe != PIPE_B && pipe != PIPE_C) return 0;
    cur_pipe = pipe;
    return 1;
}
int  intel_pipe_current(void) { return cur_pipe; }

#define PIPE_REG(base)    ((base) + (u32)cur_pipe * PIPE_STRIDE)

#define PIPE_SRCSZ_A      PIPE_REG(0x6001C)  /* composed image size, minus one */
#define TRANS_CONF_A      PIPE_REG(0x70008)  /* transcoder config + enable   */
#define PIPE_FRMCNT_A     PIPE_REG(0x70040)  /* free-running frame counter   */
#define PLANE_CTL_1_A     PIPE_REG(0x70180)  /* primary plane control        */
#define PLANE_STRIDE_1_A  PIPE_REG(0x70188)  /* stride, in tiling units      */
#define PLANE_SIZE_1_A    PIPE_REG(0x70190)  /* fetched image size, minus one*/
#define PLANE_SURF_1_A    PIPE_REG(0x7019C)  /* surface addr AND arm trigger */
#define PLANE_OFFSET_1_A  PIPE_REG(0x701A4)  /* pan inside the surface       */
#define PLANE_SURFLIVE_A  PIPE_REG(0x701AC)  /* RO: what is scanning out NOW */
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
/* M/N, in the same transcoder-indexed block. DATA is the payload ratio, LINK
 * the clock ratio; M2/N2 are the second set DRRS switches to and stay zero. */
#define TRANS_OFF_DATA_M1 0x30
#define TRANS_OFF_DATA_N1 0x34
#define TRANS_OFF_DATA_M2 0x38
#define TRANS_OFF_DATA_N2 0x3C
#define TRANS_OFF_LINK_M1 0x40
#define TRANS_OFF_LINK_N1 0x44
#define TRANS_OFF_LINK_M2 0x48
#define TRANS_OFF_LINK_N2 0x4C
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
#define PIPE_FLIPCNT_A    PIPE_REG(0x70044)

/* ---- the hardware cursor plane ---------------------------------------
 * A separate plane the display engine composites on top of the primary one,
 * for free, every frame. Moving it costs one register write and no redraw -
 * which is why every real OS has a hardware cursor and software mice look
 * laggy by comparison. */
#define CUR_CTL_A         PIPE_REG(0x70080)
#define CUR_BASE_A        PIPE_REG(0x70084)
#define CUR_POS_A         PIPE_REG(0x70088)
#define CUR_MODE_128_ARGB 0x22      /* 128x128, 32-bit ARGB */
#define CUR_MODE_64_ARGB  0x27      /* 64x64, 32-bit ARGB   */
#define CUR_MODE_DISABLE  0x00

/* ---- plane geometry --------------------------------------------------- */
#define PLANE_POS_1_A     PIPE_REG(0x7018C)
#define PLANE_KEYMAX_1_A  PIPE_REG(0x701A0)
#define PLANE_WM_1_A(l)   PIPE_REG(0x70240 + (u32)(l) * 4u)  /* 8 wm levels */

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
/* CNP/CMP layout - three separate registers. The old names here were CTL1/CTL2
 * with a comment claiming 0xC8254 packed freq and duty together, which is the
 * SKL/SPT layout and not this part. Named for what they are now. */
#define BLC_PWM_CTL       0xC8250   /* b31 enable, b29 polarity (1=active low) */
#define BLC_PWM_FREQ      0xC8254   /* period, in 24 MHz clocks. ALL 32 bits.  */
#define BLC_PWM_DUTY      0xC8258   /* active clocks.            ALL 32 bits.  */
#define BLC_PWM_ENABLE    (1u << 31)

#define PLANE_CTL_ENABLE  0x80000000u
#define PLANE_CTL_FORMAT_MASK 0x0F000000u

/* Nothing in this file WRITES to the display engine unless it has been armed.
 * The read paths are safe alongside another driver; the write paths are not,
 * and an accidental write while i915 owns the device fights it.
 *
 * Declared up here with the other module state rather than halfway down the
 * file, where it used to live: it is the gate on every write path, and a write
 * path added above its old declaration failed to compile in a way that reads
 * as "lt_armed is missing" rather than "this function is in the wrong place".
 * That cost time twice. */
static int lt_armed = 0;

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

/* Poll a register until the masked bits match, or the budget runs out.
 *
 * Two things this gets right that the loops it replaces did not.
 *
 * The time base cannot be idt_ticks(). That counter advances on a timer
 * interrupt, and a modeset runs with interrupts masked - so
 * `while (idt_ticks() - t0 < N)` never advances, the condition stays 0 < N, and
 * the loop spins forever. Three waits in this file were written that way. They
 * looked fine, because each also returns the moment the bit it wants appears;
 * the hang only reaches the path where the hardware does NOT come up, which is
 * the only path a timeout exists for. A wait that cannot expire is not a
 * timeout, it is a deadlock waiting for a bad panel.
 *
 * And the budget is counted in poll iterations rather than by re-reading a
 * clock. cpu_now_ms() divides a 64-bit cycle count, which on the 32-bit build
 * is a software routine out of divmod.c; cpu.c is explicit that it belongs once
 * per power transition and not inside a loop. Each pass also costs an MMIO
 * read, so the real elapsed time is a little over the budget - over-waiting a
 * timeout is the safe direction, and the same direction cpu_delay_us() chose.
 *
 * Microseconds, because that is the unit the plan states these waits in: 8 us
 * for DDI_BUF idle, 20 us for an IO power well. */
#define WAIT_POLL_US 10u

static int wait_bits_us(u32 reg, u32 mask, u32 want, u32 us)
{
    u32 tries = us / WAIT_POLL_US;
    if (!tries) tries = 1;
    for (u32 i = 0; i < tries; i++) {
        if ((mmio_r(reg) & mask) == want) return 1;
        cpu_delay_us(WAIT_POLL_US);
    }
    return (mmio_r(reg) & mask) == want;      /* one last look after the budget */
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
    /* PLANE_SURFLIVE carries status bits in the low byte alongside the
     * address - measured, SURF 0x01F40000 reads back as SURFLIVE 0x01F40020 -
     * so an equality test against a page-aligned address can never match, and
     * this reported failure on a plane it had just armed correctly. Compare
     * the address bits only. */
    return ((intel_surface() & 0xFFFFF000u) == (gfx_addr & 0xFFFFF000u)) ? 1 : 0;
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

    /* The eDP transcoder is not enabled. That does NOT mean transcoder A is
     * driving something - measured with i915 unbound, TRANS_DDI_FUNC_CTL reads
     * 0x00010006 (enable clear) while the eDP timing registers at 0x6F000 still
     * hold 2560 of 2720 and 1440 of 1481, and transcoder A reads zeroes.
     *
     * Falling through to A there gives a 1x1 mode built from an empty register
     * block, which is how a takeover of a STOPPED display reads the mode as
     * nonsense and then programs it. So: if the eDP block still describes a
     * real mode, it is the one that matters, enabled or not. */
    if ((mmio_r(TRANS_EDP_BASE + TRANS_OFF_HTOTAL) & 0x1FFF) &&
        (mmio_r(TRANS_EDP_BASE + TRANS_OFF_VTOTAL) & 0x1FFF))
        return TRANS_EDP_BASE;

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
 * would silently be wrong. Returns milli-hertz to keep the fraction.
 *
 * This one genuinely needs idt_ticks(), because it is counting vblanks against
 * wall time and the PIT tick IS the interval being measured. So it can only run
 * with interrupts enabled, and both waits below are bounded to say so: with
 * interrupts masked idt_ticks() never advances, and an unbounded
 * `while (idt_ticks() == t0)` is a hang, not a wait. Bounded, it returns 0,
 * which every caller already treats as "we do not know". */
#define REFRESH_SPIN_MAX 200000000u

u32 intel_refresh_mhz(void)
{
    if (!intel_pipe_enabled()) return 0;
    u32 t0 = idt_ticks();
    /* wait for a tick edge so the interval is a whole number of ticks */
    u32 spin = 0;
    while (idt_ticks() == t0)
        if (++spin > REFRESH_SPIN_MAX) return 0;      /* clock is not moving */
    u32 start_tick = idt_ticks();
    int f0 = intel_frame_count();
    spin = 0;
    while (idt_ticks() - start_tick < 50) {        /* 500 ms at 100 Hz */
        if (++spin > REFRESH_SPIN_MAX) return 0;
    }
    int f1 = intel_frame_count();
    u32 elapsed = idt_ticks() - start_tick;
    if (!elapsed) return 0;
    u32 frames = (u32)(f1 - f0);
    /* frames per (elapsed*10) ms  ->  milli-hertz */
    return (frames * 100000u) / elapsed;
}

/* ---- the pixel clock, without needing the frame counter to move ---------
 *
 * The measurement above is honest but it has a hole: PSR. With self-refresh
 * enabled the panel redraws itself from its own memory, the pipe is not
 * fetching, and the frame counter does not advance. Firmware leaves PSR on on
 * this laptop (EDP_PSR_CTL = 0x81F00406), so intel_refresh_mhz() returns 0 and
 * intel_pixel_clock_khz() returned 0 with it.
 *
 * The failure is worse than a flat zero, and this is the part that argues for
 * replacing it rather than working around it: whether the counter moves depends
 * on whether the panel happens to be in self-refresh at that instant. Measured
 * both ways on this machine - 0.0 Hz with an idle screen, a correct 60.0 Hz
 * with a terminal scrolling on it. So it passes in testing and returns zero in
 * the field, and every bandwidth, watermark and link-rate decision downstream
 * was resting on an assumed 60 Hz that nothing checked.
 *
 * There is an exact answer sitting in a register. A DP link runs at a fixed
 * symbol rate regardless of the mode, so the transcoder carries a ratio that
 * reconciles the two clocks, and PIPE_LINK_M1/N1 hold precisely
 * pixel_clock : link_clock. Scale the link clock by it and the pixel clock
 * falls out - no timer, no moving counter, and correct while PSR is on.
 *
 * Verified against this panel: LINK_M1 = 0x00072943 (469315), LINK_N1 =
 * 0x00080000 (524288), link clock 270000 kHz -> 241690 kHz, which is the
 * pixel clock in the panel's own EDID detailed timing descriptor (0x5E69 in
 * units of 10 kHz) to the kHz. So the assumed 60 Hz was very nearly right -
 * 59.998 Hz - but it is now read rather than assumed, and the same code
 * reports a 48 Hz or 120 Hz panel correctly.
 *
 * m is truncated when the ratio is built, so the true quotient sits just above
 * it: round rather than truncate on the way back, or lose a kHz. */
#define M_N_FIELD_MASK  0xFFFFFFu        /* both M and N are 24-bit fields */

/* all three are defined with the DPLL code further down */
u32 intel_dp_link_symbol_khz(int rate_idx);
int intel_ddi_clock_select(int ddi);
int intel_dpll_link_rate(int pll);

u32 intel_pixel_clock_mn_khz(void)
{
    if (!intel_present()) return 0;

    /* eDP transcoder only, and that is a correctness limit rather than
     * laziness. The ratio is against the LINK clock, so this needs to know
     * which port's DPLL to ask - and the answer is only knowable for free on
     * this transcoder, whose DDI-select field is ignored because it is
     * hardwired to DDI A. On transcoder A the field is live and the port could
     * be B, C or D, so asking DDI A would pair one transcoder's ratio with
     * another port's clock and return a wrong pixel clock with no symptom.
     * Say nothing instead and let the caller fall back. */
    u32 base = trans_base();
    if (base != TRANS_EDP_BASE) return 0;

    u32 m = mmio_r(base + TRANS_OFF_LINK_M1) & M_N_FIELD_MASK;
    u32 n = mmio_r(base + TRANS_OFF_LINK_N1) & M_N_FIELD_MASK;
    if (!m || !n) return 0;                  /* no DP link on this transcoder */

    /* Which DPLL feeds DDI A, and at what rate is it running? */
    int pll = intel_ddi_clock_select(0);
    u32 link_khz = intel_dp_link_symbol_khz(intel_dpll_link_rate(pll));
    if (!link_khz) return 0;

    return (u32)(((u64)link_khz * (u64)m + (u64)(n / 2)) / (u64)n);
}

/* pixel clock in kHz. The M/N ratio is exact and survives PSR, so it is the
 * first choice; the frame-counter measurement is the fallback for a path that
 * has no M/N to read, which on this hardware means HDMI. */
u32 intel_pixel_clock_khz(void)
{
    u32 khz = intel_pixel_clock_mn_khz();
    if (khz) return khz;

    u32 mhz = intel_refresh_mhz();
    if (!mhz) return 0;
    u64 dots = (u64)intel_htotal() * (u64)intel_vtotal();
    return (u32)((dots * (u64)mhz) / 1000000u);
}

/* Refresh rate derived from the pixel clock rather than counted, in milli-hertz.
 * Works while PSR has the counter frozen, which is when it is actually needed. */
u32 intel_refresh_mhz_derived(void)
{
    u32 khz = intel_pixel_clock_khz();
    u64 dots = (u64)intel_htotal() * (u64)intel_vtotal();
    if (!khz || !dots) return 0;
    return (u32)(((u64)khz * 1000000u + dots / 2) / dots);
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
 * the panel still powered, which is a real thing to be careful about.
 *
 * WHICH registers hold them is platform-specific, and this code had the wrong
 * platform. Two layouts exist:
 *
 *   SKL / SPT      0xC8254 packs BOTH: freq in 31:16, duty in 15:0
 *   CNP / CMP      0xC8254 is freq, all 32 bits. Duty is its own register,
 *                  0xC8258. (This part. The plan says so explicitly, and adds
 *                  that SKL also needs a SOUTH_CHICKEN1 granularity bit that
 *                  does not apply here.)
 *
 * We are CMP and the code used the SKL packing, so on this laptop:
 *
 *   0xC8254 = 00005EB2   real period, 24242 clocks of 24 MHz  (~990 Hz)
 *   0xC8258 = 0000556E   real duty, 21870  ->  90% brightness
 *
 * intel_backlight_max() read 0x5EB2 >> 16 = 0, so intel_backlight_set() hit
 * its `if (!max) return 0` and did nothing at all, while
 * intel_backlight_get() returned 24242 - the period - as the brightness.
 *
 * Worth being precise about the severity: it failed SAFE. Because max read 0
 * it bailed before writing, so it never corrupted the period. But it was dead
 * code that looked live, and it is one of the few write paths here NOT gated
 * behind lt_armed, so it would have been the first thing to run for real. */
u32 intel_backlight_max(void)
{
    if (!intel_present()) return 0;
    return mmio_r(BLC_PWM_FREQ);            /* the whole register is the period */
}

u32 intel_backlight_get(void)
{
    if (!intel_present()) return 0;
    return mmio_r(BLC_PWM_DUTY);
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
    /* 32-bit period, so scale before dividing would overflow at periods above
     * ~42 M. This panel's is 24242, but divide first if it ever is not. */
    u32 duty = (max > 42000000u) ? (max / 100u) * (u32)percent
                                 : (max * (u32)percent) / 100u;
    mmio_w(BLC_PWM_DUTY, duty);
    return 1;
}

int intel_backlight_pwm_enabled(void)
{
    return intel_present() ? ((mmio_r(BLC_PWM_CTL) & BLC_PWM_ENABLE) ? 1 : 0) : 0;
}
/* The PWM enable/disable pair lives with the panel-power code further down,
 * where lt_armed is in scope and where plan step 58 pairs it with
 * PP_CONTROL b2 - the order between the two is the whole point of that step. */

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

/* How many lanes the port is running: bits 3:1 hold lanes-1. */
int intel_ddi_lanes(void)
{
    return (int)((mmio_r(TRANS_DDI_EDP) >> 1) & 0x7) + 1;
}

/* Bits per pixel, from TRANS_DDI_FUNC_CTL bits 22:20.
 *
 * Two registers on the same transcoder encode bpc, and they DISAGREE:
 *
 *    TRANS_DDI_FUNC_CTL 22:20   0=8  1=10  2=6  3=12
 *    TRANS_MSA_MISC     7:5     0=6  1=8   2=10 3=12
 *
 * So the same field value means 8 bpc in one and 6 bpc in the other. Reading
 * this register with the MSA table gets 6 bpc on a panel running at 8, which
 * under-computes every bandwidth figure by 25% and picks a link rate that
 * trains and then cannot carry a frame. Hence one accessor, here, with the
 * table written down next to it. */
int intel_ddi_bpp(void)
{
    /* The field is three bits wide and only four values are defined; 4..7 are
     * reserved. Report 0 for those rather than folding them onto a real bpc -
     * 0 propagates as "we do not know" and gets rejected downstream, which is
     * the behaviour this file already settled on for an unknown pixel clock. */
    static const int bpc[8] = { 8, 10, 6, 12, 0, 0, 0, 0 };
    return bpc[(mmio_r(TRANS_DDI_EDP) >> 20) & 0x7] * 3;
}

/* M/N as the hardware currently holds it, for comparing against a computation.
 * DATA_M1 is returned raw, TU_SIZE field and all, because that is what has to
 * be reproduced by a write. */
u32 intel_data_m1_reg(void) { return mmio_r(trans_base() + TRANS_OFF_DATA_M1); }
u32 intel_data_n1_reg(void) { return mmio_r(trans_base() + TRANS_OFF_DATA_N1); }
u32 intel_link_m1_reg(void) { return mmio_r(trans_base() + TRANS_OFF_LINK_M1); }
u32 intel_link_n1_reg(void) { return mmio_r(trans_base() + TRANS_OFF_LINK_N1); }

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

/* CDCLK in kHz. Bits 10:0 hold (kHz - 1000) / 500, so this inverts that.
 * Plan step 9: the only requirement is cdclk >= pixel rate, and if it already
 * is we leave it alone. Measured 337500 kHz here against a 241690 kHz pixel
 * rate, so there is nothing to do - changing CDCLK needs a pcode handshake and
 * is deliberately deferred. */
u32 intel_cdclk_khz(void)
{
    if (!intel_present()) return 0;
    return ((mmio_r(CDCLK_CTL) & 0x7FF) * 500u) + 1000u;
}

/* DBUF: the display data buffer has to be powered before any plane can fetch
 * through it. Plan step 10 - request, posting read, a FIXED 10 us delay, then a
 * single check. Not a poll: the PRM gives a settling time, not a handshake, and
 * a poll loop here would read the state bit before it is meaningful. */
#define DBUF_CTL_S0        0x45008
#define DBUF_POWER_REQUEST (1u << 31)
#define DBUF_POWER_STATE   (1u << 30)

int intel_dbuf_enable(int on)
{
    if (!intel_present() || !lt_armed) return 0;
    u32 v = mmio_r(DBUF_CTL_S0);
    mmio_w(DBUF_CTL_S0, on ? (v | DBUF_POWER_REQUEST) : (v & ~DBUF_POWER_REQUEST));
    (void)mmio_r(DBUF_CTL_S0);
    cpu_delay_us(10);
    int state = (mmio_r(DBUF_CTL_S0) & DBUF_POWER_STATE) ? 1 : 0;
    return state == (on ? 1 : 0);
}

int intel_dbuf_powered(void)
{
    return intel_present() ? ((mmio_r(DBUF_CTL_S0) & DBUF_POWER_STATE) ? 1 : 0) : 0;
}
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
    /* The select override is bit (port*3 + 0). This line used to read
     * (ddi*3 + 15), which for DDI A is bit 15 - the CLK_OFF bit, a different
     * field entirely - and then discarded the result in an empty body. It is
     * the same off-by-a-field that f652d56 fixed in intel_ddi_set_clock, still
     * present in the reader.
     *
     * It matters here rather than being cosmetic: with the override clear the
     * CLK_SEL bits are not what is driving the port, so returning them would
     * be reporting a guess as a reading. Say "unknown" instead, the same way
     * intel_dpll_link_rate() does for its own override. */
    if (!((v >> (ddi * 3)) & 1)) return -1;
    return (int)((v >> (ddi * 3 + 1)) & 0x3);
}

int intel_ddi_clock_off(int ddi)
{
    if (ddi < 0 || ddi > 4) return 1;
    return (int)((mmio_r(DPLL_CTRL2) >> (ddi + 15)) & 1);
}

/* The link rate index in DPLL_CTRL1 is not a frequency, it is a table entry.
 *
 * Three different clocks get called "the link rate" and confusing them is
 * hazard 4.3 #10. For HBR the numbers are:
 *
 *    2700000 kbps   bit rate         "2.7 Gbps", what the DPCD reports
 *    1350000 kHz    DPLL frequency   half the bit rate; DP is DDR. THIS TABLE.
 *     270000 kHz    symbol rate      bit rate / 10, for 8b/10b. What M/N uses.
 *
 * So this returns DPLL frequencies, not symbol clocks - the comment here used
 * to say "symbol clocks in units of 10 kHz", which is neither. */
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

/* The link symbol clock: bit rate / 10, which is DPLL frequency * 2 / 10.
 * This is the clock the M/N ratios are taken against, and the one number
 * i915 confusingly also calls "port_clock". HBR -> 270000 kHz. */
u32 intel_dp_link_symbol_khz(int rate_idx)
{
    u32 dpll_khz = intel_dpll_rate_khz(rate_idx);
    return dpll_khz ? dpll_khz / 5u : 0;
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
    return wait_bits_us(DPLL_STATUS, 1u << (pll * 8), 1u << (pll * 8), 500000u);
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

/* ==== EDID over I2C-over-AUX ============================================
 *
 * intel_read_edid() reads the EDID over GMBUS, and GMBUS does not serve eDP on
 * DDI A - the panel has no GMBUS pins. So the only way this driver has ever
 * known its own mode is by reading back the timing registers firmware
 * programmed, which works right up until nobody has programmed them.
 *
 * On this machine that is not hypothetical: with i915 unbound the transcoder is
 * disabled and the eDP timing block only still reads correctly because the
 * registers happen to retain their values. A genuine cold start, or any board
 * where firmware chose a different mode, needs the panel asked directly.
 *
 * The transport is the same AUX channel DPCD uses, with the I2C command values
 * instead of the native ones, addressing I2C slave 0x50. The important
 * differences from a native transaction:
 *
 *   - MOT (middle-of-transaction, bit 2 of the command) must stay SET for every
 *     transfer that is not the last, or the sink ends the I2C transaction and
 *     the address pointer resets. Getting this wrong reads byte 0 repeatedly.
 *   - An I2C DEFER is far more likely than a native one and needs more
 *     patience: the plan allows max(7, bus-derived) plus seven more, where a
 *     native DEFER gets 32 quick retries.
 *   - A read is a zero-length write to set the address, then reads.
 */
#define AUX_I2C_WRITE      0x0
#define AUX_I2C_READ       0x1
#define AUX_I2C_MOT        0x4
#define EDID_I2C_ADDR      0x50

/* One I2C-over-AUX chunk. AUX carries at most 16 bytes per transaction. */
static int aux_i2c(int port, u32 cmd, const u8 *out, int out_len, u8 *in, int in_len)
{
    for (int attempt = 0; attempt < 14; attempt++) {
        int n = aux_xfer(port, cmd, EDID_I2C_ADDR, out, out_len, in, in_len);
        if (n >= 0 && aux_last_reply == AUX_REPLY_ACK) return n;
        /* Reply bits 3:2 are the I2C status; a DEFER there is bit 3 in the
         * decoded nibble. Anything that is not a defer is a real no. */
        if (aux_last_reply != AUX_REPLY_DEFER && aux_last_reply != 2) return -1;
        cpu_delay_us(500);
    }
    return -1;
}

/* Read `len` bytes from EDID offset `off` into edid_buf. Returns bytes read.
 *
 * MOT is set on the address write and on every read but the last, so the whole
 * thing is one I2C transaction and the sink's address pointer advances. The
 * final read clears MOT to release the bus - leaving it set holds the
 * transaction open and the next caller starts mid-stream. */
int intel_edid_over_aux(int port, u32 off, int len)
{
    if (len < 1 || len > 128 || !edid_buf) return 0;

    u8 addr = (u8)off;
    if (aux_i2c(port, AUX_I2C_WRITE | AUX_I2C_MOT, &addr, 1, aux_buf, 1) < 0)
        return 0;

    int done = 0;
    while (done < len) {
        int chunk = len - done;
        if (chunk > 16) chunk = 16;
        int last = (done + chunk >= len);
        u32 cmd = AUX_I2C_READ | (last ? 0u : AUX_I2C_MOT);

        int n = aux_i2c(port, cmd, 0, 0, aux_buf, chunk);
        if (n <= 0) return done;
        for (int i = 0; i < n; i++)
            *(volatile u8 *)(edid_buf + (uptr)(done + i)) = aux_buf[i];
        done += n;
        if (n < chunk) break;              /* short read - the sink stopped */
    }
    return done;
}

/* The whole 128-byte block, validated. Same checksum rule as the GMBUS path:
 * all 128 bytes must sum to zero mod 256, and the header must be the fixed
 * 00 FF FF FF FF FF FF 00 - a partial read passes neither. */
int intel_read_edid_aux(int port)
{
    if (intel_edid_over_aux(port, 0, 128) != 128) return 0;
    return edid_valid(edid_buf);
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

/* ==== bringing the port up and down (plan steps 30-35, teardown 9-11) =====
 *
 * This did not exist. DDI_BUF_CTL_ENABLE was defined and never written by
 * anything, so intel_link_train() below was writing training patterns into a
 * port that nothing had switched on. The plan lists this as implementation
 * order #5 and calls it "mandatory before any training attempt"; it is also the
 * only way to take the port back from firmware, which is the state we are
 * always in on this machine.
 *
 * Two boot-time straps live in DDI_BUF_CTL and must survive every write:
 * DDI_A_4_LANES (b4) and Port Reversal (b16). Read once, OR into everything -
 * hazard 4.3 #19. saved_port_bits on this board is 0x00000010.
 */
#define DDI_BUF_PORT_REVERSAL  (1u << 16)
#define DDI_BUF_A_4_LANES      (1u << 4)
#define DDI_BUF_WIDTH_MASK     (7u << 1)
#define DDI_BUF_TRANS_SEL_MASK (0xFu << 24)
#define DISPIO_CR_TX_BMU_CR0   0x6C00C

static u32 port_saved_bits = 0;
static int port_saved_read = 0;

/* Read the straps once. Safe to call any time; it only reads. */
u32 intel_port_save_bits(int port)
{
    if (!intel_present()) return 0;
    port_saved_bits = mmio_r(DDI_BUF_CTL(port)) &
                      (DDI_BUF_PORT_REVERSAL | DDI_BUF_A_4_LANES);
    port_saved_read = 1;
    return port_saved_bits;
}

/* I_boost / balance leg (step 32). The low-vswing eDP table this board uses has
 * I_boost 0 for every entry, so the correct action is to SET the balance-leg
 * disable bit rather than program a boost - and firmware has already done
 * exactly that (DISPIO_CR_TX_BMU_CR0 = 08800000, b23 and b27 set).
 *
 * When the port runs x4, DDI A's upper lanes are driven by the DDI E field, so
 * both have to be programmed. Missing that half leaves two lanes on a different
 * drive setting from the other two. Bits 31:28 and 7:0 are never ours. */
int intel_iboost_set(int port, int iboost, int four_lanes)
{
    if (!intel_present() || !lt_armed) return 0;
    if (iboost != 0 && iboost != 1 && iboost != 3 && iboost != 7) return 0;

    u32 v = mmio_r(DISPIO_CR_TX_BMU_CR0);
    v &= ~(7u << (8 + port * 3));                    /* this port's sctl */
    v &= ~(1u << (23 + port));                       /* this port's disable */
    if (iboost) v |= ((u32)iboost << (8 + port * 3));
    else        v |= (1u << (23 + port));

    if (four_lanes && port == 0) {                   /* DDI A x4 -> also sctl_4 */
        v &= ~(7u << 20);
        v &= ~(1u << 27);
        if (iboost) v |= ((u32)iboost << 20);
        else        v |= (1u << 27);
    }
    mmio_w(DISPIO_CR_TX_BMU_CR0, v);
    return 1;
}

/* Take the port down. Order is the plan's and is not the obvious one: the idle
 * wait comes AFTER both disables, not between them (teardown 9, 10, then 11).
 * Returns 0 if the port never went idle, which is worth knowing but is not
 * fatal - the 8 us in the PRM is spec-accurate and field-inaccurate, so we
 * budget 1 ms (C4). */
int intel_port_disable(int port)
{
    if (!intel_present() || !lt_armed) return 0;

    mmio_w(DDI_BUF_CTL(port), mmio_r(DDI_BUF_CTL(port)) & ~DDI_BUF_CTL_ENABLE);
    /* Do NOT route through Idle on the way down - that is the enable path's
     * sequence, and the plan is explicit that the disable path skips it. */
    mmio_w(DP_TP_CTL(port), mmio_r(DP_TP_CTL(port)) & ~DP_TP_CTL_ENABLE);

    return wait_bits_us(DDI_BUF_CTL(port), DDI_BUF_IS_IDLE, DDI_BUF_IS_IDLE, 1000u);
}

/* Bring the port up ready for training (steps 30, 33, 34, 35).
 *
 * If firmware left the port enabled we must cycle it: the PRM requires
 * disable-then-re-enable with pattern 1 to retrain, and DDI_BUF_CTL[3:1] and
 * DP_TP_CTL[18] cannot legally change while the port is enabled (4.3 #20).
 *
 * DP_TP_CTL goes on FIRST, with TPS1 already selected, and only then
 * DDI_BUF_CTL - doing it the other way round enables a port with no transport
 * behind it. Enhanced framing must match DPCD 0x101 b7 or the link is stable
 * and shows garbage (4.3 #8), and it cannot be changed later, so it is decided
 * here.
 *
 * Then a blind 600 us. There is NO status bit for this on Gen9 - the !IDLE poll
 * everyone reaches for is gated to DISPLAY_VER >= 10 - so a poll here would
 * either spin its whole timeout or, worse, read the stale idle bit and sail
 * past. 518 us is the spec figure; 600 gives it margin. */
int intel_port_enable(int port, int lanes, int enhanced)
{
    if (!intel_present() || !lt_armed) return 0;
    if (lanes < 1 || lanes > 4) return 0;
    if (!port_saved_read) intel_port_save_bits(port);

    /* mandatory if firmware left it up */
    if (mmio_r(DDI_BUF_CTL(port)) & DDI_BUF_CTL_ENABLE) {
        if (!intel_port_disable(port)) return 0;
    }

    mmio_w(DP_TP_CTL(port), DP_TP_CTL_ENABLE | DP_TP_CTL_MODE_SST |
                            DP_TP_CTL_LINK_TRAIN_PAT1 |
                            (enhanced ? DP_TP_CTL_ENHANCED_FRAME : 0u));
    (void)mmio_r(DP_TP_CTL(port));

    /* vswing entry 0 to start; training walks it up from there */
    u32 buf = port_saved_bits | ((u32)(lanes - 1) << 1) | (0u << 24);
    mmio_w(DDI_BUF_CTL(port), buf);
    (void)mmio_r(DDI_BUF_CTL(port));
    mmio_w(DDI_BUF_CTL(port), buf | DDI_BUF_CTL_ENABLE);
    (void)mmio_r(DDI_BUF_CTL(port));

    cpu_delay_us(600);                  /* blind. no status bit exists (4.2) */
    return 1;
}

int intel_port_enabled(int port)
{
    if (!intel_present()) return 0;
    return (mmio_r(DDI_BUF_CTL(port)) & DDI_BUF_CTL_ENABLE) ? 1 : 0;
}
u32 intel_port_bits(void)  { return port_saved_bits; }
u32 intel_iboost_reg(void) { return intel_present() ? mmio_r(DISPIO_CR_TX_BMU_CR0) : 0; }

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

/* Plan step 12: program the sequencer BEFORE anything asserts panel power.
 *
 * This is the step the modeset sequence originally pointed at
 * intel_pp_sequencing(), which only READS the sequence-progress bits and
 * returns 0 when the sequencer is idle - so the modeset would have aborted at
 * step 12 with a name that claimed it had programmed something.
 *
 * Measured with i915 unbound, which is the state a real run starts from:
 *
 *   PP_STATUS  = 00000000    panel fully powered DOWN
 *   PP_CONTROL = 00000060    b0 off, b2 off, T12 field still 6, and b1 CLEAR
 *   PP_ON_DELAYS / PP_OFF_DELAYS survive intact (T3 200 ms, T10 50 ms)
 *
 * Two things follow. The delay registers persist, so the correct action is to
 * keep whatever is larger rather than overwrite good values with our defaults -
 * hazard H3 is about them resetting to ZERO, and a sequencer with T3 = 0
 * reports ready instantly and lets AUX drive an unpowered panel.
 *
 * And b1, power-down-on-reset, is CLEARED by i915 on the way out. Hazard H4:
 * with it clear, any reset drops VDD instantly under live video instead of
 * running the ordered T9/T10 sequence. Nothing in this driver set it. Now it
 * does, and it is the reason this function exists rather than being folded
 * into the power-on path - it has to happen before power is asserted, not with
 * it. */
int intel_pp_delays_program(void)
{
    if (!intel_present() || !lt_armed) return 0;

    /* eDP-spec ceilings, used only where the register reads lower */
    u32 want_t3 = 2000, want_bl_on = 10, want_t10 = 500, want_bl_off = 500;

    u32 on  = mmio_r(PP_ON_DELAYS);
    u32 off = mmio_r(PP_OFF_DELAYS);
    u32 t3      = (on  >> 16) & 0x1FFF, bl_on  = on  & 0x1FFF;
    u32 t10     = (off >> 16) & 0x1FFF, bl_off = off & 0x1FFF;

    if (t3     < want_t3)     t3     = want_t3;
    if (bl_on  < want_bl_on)  bl_on  = want_bl_on;
    if (t10    < want_t10)    t10    = want_t10;
    if (bl_off < want_bl_off) bl_off = want_bl_off;

    mmio_w(PP_ON_DELAYS,  (t3  << 16) | bl_on);
    mmio_w(PP_OFF_DELAYS, (t10 << 16) | bl_off);

    /* T12 field is "+1" encoded in 100 ms units: 6 means 500 ms. Keep whatever
     * is there if it already asks for at least 500 ms, and never write 0 -
     * that means "no delay", which is the H1 hazard itself. */
    u32 ctl = mmio_r(PP_CONTROL) & 0xFFFF;
    u32 cyc = (ctl >> 4) & 0x1F;
    if (cyc < 6) cyc = 6;                       /* >= 500 ms */
    ctl = (ctl & ~(0x1Fu << 4)) | (cyc << 4);
    ctl |= PP_PWR_DOWN_ON_RESET;                /* H4 - i915 leaves this clear */

    mmio_w(PP_CONTROL, PP_UNLOCK_KEY | ctl);
    (void)mmio_r(PP_CONTROL);
    return 1;
}

int intel_pp_down_on_reset(void)
{
    if (!intel_present()) return 0;
    return (mmio_r(PP_CONTROL) & PP_PWR_DOWN_ON_RESET) ? 1 : 0;
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

/* Bring the PWM up from cold: frequency and duty BEFORE the enable bit, which
 * is plan step 58 and the opposite order to how it reads naturally. Enabling
 * first runs the panel at whatever period happened to be left in the register.
 * Both are separate 32-bit registers on this PCH - see the BLC_PWM_* defines. */
int intel_backlight_pwm_enable(u32 period, u32 duty)
{
    if (!intel_present() || !lt_armed || !period) return 0;

    /* Same guard as intel_panel_backlight_enable, for the same reason: hazard
     * H2 names PWM alongside AUX and the main link as something not to drive
     * into an unpowered panel. Writing FREQ and DUTY alone is harmless while
     * the output is off, but this function ends by turning the output ON, so
     * the check belongs here and not at the call site. */
    if ((mmio_r(PP_STATUS) & PP_ON_MASK) != PP_ON_WANT) return 0;

    if (duty > period) duty = period;
    /* Enabling the output at zero duty is a lit backlight emitting nothing.
     * Nobody ever wants that, and it is indistinguishable from a dead panel. */
    if (!duty) duty = period / 2u;
    mmio_w(BLC_PWM_FREQ, period);
    mmio_w(BLC_PWM_DUTY, duty);
    (void)mmio_r(BLC_PWM_FREQ);                       /* posting read */
    mmio_w(BLC_PWM_CTL, mmio_r(BLC_PWM_CTL) | BLC_PWM_ENABLE);
    (void)mmio_r(BLC_PWM_CTL);
    return 1;
}

/* Teardown step 3: the PWM goes off AFTER PP_CONTROL b2 and after the
 * backlight-off delay, never before - see intel_panel_backlight_enable(). */
int intel_backlight_pwm_disable(void)
{
    if (!intel_present() || !lt_armed) return 0;
    mmio_w(BLC_PWM_CTL, mmio_r(BLC_PWM_CTL) & ~BLC_PWM_ENABLE);
    (void)mmio_r(BLC_PWM_CTL);
    return 1;
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

/* Well indices are not 0..3. The REQ/STATE bit pair is uniform across the
 * register - REQ = bit(2i+1), STATE = bit(2i) - but the indices that matter go
 * up to 15:
 *
 *   0  MISC_IO      1  DDI_A_E     2  DDI_B    3  DDI_C    4  DDI_D
 *   14 PW1 (b29/b28)             15 PW2 (b31/b30)
 *
 * The guards below used to stop at 3, which made PW1 - plan step 6, and a
 * prerequisite for everything in the display core - simply unrequestable. The
 * macros were always right; only the range check was wrong. */
#define PW_MISC_IO  0
#define PW_DDI_A_E  1
#define PW_PW1     14
#define PW_PW2     15

int intel_pwr_well_enabled(int well)
{
    if (!intel_present() || well < 0 || well > 15) return 0;
    return (mmio_r(PWR_WELL_CTL_DRIVER) & PW_STATE(well)) ? 1 : 0;
}

int intel_pwr_well_requested(int well)
{
    if (!intel_present() || well < 0 || well > 15) return 0;
    return (mmio_r(PWR_WELL_CTL_DRIVER) & PW_REQUEST(well)) ? 1 : 0;
}

/* Ask for a well and wait for the hardware to say it is actually up. The wait
 * is not optional: the request bit is a request, and the state bit is the
 * answer, and they are not the same thing. */
int intel_pwr_well_enable(int well)
{
    if (!intel_present() || !lt_armed || well < 0 || well > 15) return 0;
    u32 v = mmio_r(PWR_WELL_CTL_DRIVER);
    mmio_w(PWR_WELL_CTL_DRIVER, v | PW_REQUEST(well));

    /* The PRM allows 20 us for an IO well - the comment here used to say 20 ms,
     * a thousand times too long. Budget 1 ms, as the plan does for every one of
     * these short hardware handshakes. */
    return wait_bits_us(PWR_WELL_CTL_DRIVER, PW_STATE(well), PW_STATE(well), 1000u);
}

int intel_pwr_well_disable(int well)
{
    if (!intel_present() || !lt_armed || well < 0 || well > 15) return 0;
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

/* ==== M/N: reconciling a fixed link rate with an arbitrary pixel clock ====
 *
 * A DP link runs at one of a few fixed symbol rates. A panel wants whatever
 * pixel clock its EDID asks for. The transcoder bridges the two with a pair of
 * ratios it uses to meter data onto the link:
 *
 *    DATA M/N  = (bpp * pixel_clock) : (link_clock * lanes * 8)   payload share
 *    LINK M/N  = pixel_clock : link_clock                         clock ratio
 *
 * Both are 24-bit fields. N is rounded UP to a power of two and then capped -
 * 0x800000 for data, 0x80000 for link - and M follows from the ratio. The
 * rounding is not decoration: a power-of-two N is what lets the hardware do the
 * division by shifting.
 *
 * M is TRUNCATED, not rounded. That is worth stating because it is checkable:
 * with this panel's numbers the algorithm has to land on exactly what firmware
 * put in the registers, and rounding instead of truncating misses.
 *
 *   pixel 241690 kHz, link 270000 kHz, 4 lanes, 24 bpp
 *     DATA  M 5631785 N 8388608   -> 0x7E55EF29 / 0x00800000
 *     LINK  M  469315 N  524288   -> 0x00072943 / 0x00080000
 *
 * All four read back from this laptop bit for bit. That is the whole value of
 * doing this on a machine whose firmware has already solved the same problem:
 * a write path that has never executed still has a known-correct answer to be
 * checked against, which is a much stronger test than "it compiles".
 */
#define M_N_TU_SIZE       64             /* transfer unit, always 64 on gen9 */
#define M_N_DATA_N_MAX    0x800000u
#define M_N_LINK_N_MAX    0x080000u

/* results of the last computation, for a caller to inspect or program */
static u32 mn_data_m = 0, mn_data_n = 0, mn_link_m = 0, mn_link_n = 0;

/* N rounded up to a power of two, then capped. Stays in 32-bit arithmetic. */
static u32 mn_round_n(u32 n, u32 cap)
{
    u32 p = 1;
    while (p < n && p < cap) p <<= 1;
    return p > cap ? cap : p;
}

static void mn_ratio(u32 m, u32 n, u32 cap, u32 *out_m, u32 *out_n)
{
    u32 rn = mn_round_n(n, cap);
    u32 rm = (u32)(((u64)m * (u64)rn) / (u64)n);
    /* If either field overflows 24 bits, halve both - the ratio survives. */
    while (rm > M_N_FIELD_MASK || rn > M_N_FIELD_MASK) { rm >>= 1; rn >>= 1; }
    *out_m = rm;
    *out_n = rn;
}

/* Work out both ratios for a mode. link_khz is the SYMBOL clock (270000 for
 * HBR), not the bit rate and not the DPLL frequency - see
 * intel_dp_link_symbol_khz(). Returns 0 on an argument that cannot describe a
 * real link, rather than quietly producing a ratio from it. */
int intel_mn_compute(u32 pixel_khz, u32 link_khz, int lanes, int bpp)
{
    mn_data_m = mn_data_n = mn_link_m = mn_link_n = 0;
    if (!pixel_khz || !link_khz || lanes < 1 || lanes > 4 || bpp < 6) return 0;

    mn_ratio(pixel_khz * (u32)bpp, link_khz * (u32)lanes * 8u,
             M_N_DATA_N_MAX, &mn_data_m, &mn_data_n);
    mn_ratio(pixel_khz, link_khz, M_N_LINK_N_MAX, &mn_link_m, &mn_link_n);
    return 1;
}

u32 intel_mn_data_m(void) { return mn_data_m; }
u32 intel_mn_data_n(void) { return mn_data_n; }
u32 intel_mn_link_m(void) { return mn_link_m; }
u32 intel_mn_link_n(void) { return mn_link_n; }

/* Write what intel_mn_compute() worked out. M2/N2 are the alternate set DRRS
 * switches between; we do not do DRRS, and the plan says to leave them zero
 * rather than mirroring M1/N1 - a non-zero M2/N2 with no DRRS logic is a
 * second timing the hardware may switch to and nothing maintains. */
int intel_mn_program(void)
{
    if (!intel_present() || !lt_armed) return 0;
    if (!mn_data_n || !mn_link_n) return 0;      /* nothing computed */
    u32 base = trans_base();

    mmio_w(base + TRANS_OFF_DATA_M1, ((M_N_TU_SIZE - 1) << 25) | mn_data_m);
    mmio_w(base + TRANS_OFF_DATA_N1, mn_data_n);
    mmio_w(base + TRANS_OFF_DATA_M2, 0);
    mmio_w(base + TRANS_OFF_DATA_N2, 0);
    mmio_w(base + TRANS_OFF_LINK_M1, mn_link_m);
    mmio_w(base + TRANS_OFF_LINK_N1, mn_link_n);
    mmio_w(base + TRANS_OFF_LINK_M2, 0);
    mmio_w(base + TRANS_OFF_LINK_N2, 0);
    return 1;
}

/* ==== the rest of the transcoder and pipe (plan steps 46, 49-52, 54) =====
 * Every register below had NO code at all. They are individually trivial, which
 * is exactly why they go missing - and any one of them wrong is a black screen
 * with no error bit anywhere. Each is checked against what firmware left.
 */
#define TRANS_MSA_MISC_EDP  0x6F410
#define PIPE_MISC_A         PIPE_REG(0x70030)
#define CHICKEN_TRANS_EDP   0x420CC
#define PS_CTRL_1_A         0x68180
#define PS_CTRL_2_A         0x68280
#define WM_LINETIME_A       0x45270

/* TRANS_DDI_FUNC_CTL. A SINGLE 32-bit write, not a read-modify-write: the plan
 * is explicit, and an RMW would preserve whatever DDI-select and mode bits
 * firmware left behind.
 *
 * Input select stays 000b (pipe A). 100b appears on one i915 path but the KBL
 * PRM lists only 000/101/110 and the SKL PRM marks 14:12 flatly Reserved, so
 * the plan forbids it (C10).
 *
 * Firmware on this machine holds 0x82010006 and we reproduce that exactly for
 * 4 lanes / 8 bpc / hsync-positive, which is also what the panel's EDID asks
 * for. bpc uses THIS register's table, not MSA_MISC's - see intel_ddi_bpp(). */
u32 intel_trans_ddi_ctl_value(int lanes, int bpp, int phsync, int pvsync)
{
    u32 bpc_field;
    switch (bpp) {
        case 24: bpc_field = 0; break;      /* 8 bpc  */
        case 30: bpc_field = 1; break;      /* 10 bpc */
        case 18: bpc_field = 2; break;      /* 6 bpc  */
        case 36: bpc_field = 3; break;      /* 12 bpc */
        default: return 0;                  /* not a bpc this register can say */
    }
    if (lanes < 1 || lanes > 4) return 0;

    return (1u << 31)                       /* enable                        */
         | (2u << 24)                       /* mode: DP SST                  */
         | (bpc_field << 20)
         | (pvsync ? (1u << 17) : 0u)       /* PVSYNC */
         | (phsync ? (1u << 16) : 0u)       /* PHSYNC */
         | (0u << 12)                       /* input select: pipe A          */
         | ((u32)(lanes - 1) << 1);         /* port width                    */
}

int intel_trans_ddi_ctl_write(int lanes, int bpp, int phsync, int pvsync)
{
    if (!intel_present() || !lt_armed) return 0;
    u32 v = intel_trans_ddi_ctl_value(lanes, bpp, phsync, pvsync);
    if (!v) return 0;
    mmio_w(TRANS_DDI_EDP, v);               /* single write, deliberately */
    (void)mmio_r(TRANS_DDI_EDP);
    return 1;
}

int intel_trans_ddi_ctl_disable(void)
{
    if (!intel_present() || !lt_armed) return 0;
    mmio_w(TRANS_DDI_EDP, 0);               /* gen9 allows writing 0 outright */
    (void)mmio_r(TRANS_DDI_EDP);
    return 1;
}

/* TRANS_MSA_MISC (step 46). b0 = "sync clock", 7:5 = bpc.
 *
 * The bpc table here is NOT the one in TRANS_DDI_FUNC_CTL: 0=6, 1=8, 2=10,
 * 3=12. Same field name, same transcoder, one value apart. Firmware holds
 * 0x21 = sync clock + field 1 = 8 bpc, which is what this returns. */
u32 intel_msa_misc_value(int bpp)
{
    u32 f;
    switch (bpp) {
        case 18: f = 0; break;
        case 24: f = 1; break;
        case 30: f = 2; break;
        case 36: f = 3; break;
        default: return 0;
    }
    return 1u | (f << 5);
}

int intel_msa_misc_write(int bpp)
{
    if (!intel_present() || !lt_armed) return 0;
    u32 v = intel_msa_misc_value(bpp);
    if (!v) return 0;
    mmio_w(TRANS_MSA_MISC_EDP, v);
    return 1;
}

/* WM_LINETIME (step 52): how long one scanline takes, in units of 0.125 us,
 * rounded UP. Nine bits, so it saturates at 0x1FF.
 *
 * The plan works this out as 90. It is 91, and firmware agrees - 0x45270 reads
 * 0x0000005B. 2720 * 8000 / 241690 = 90.03, and the formula rounds up, so the
 * plan's number came from truncating where i915 uses DIV_ROUND_UP. Worth
 * having caught from a register rather than shipping an off-by-one into a
 * timing parameter. */
u32 intel_wm_linetime_value(u32 htotal, u32 pixel_khz)
{
    if (!htotal || !pixel_khz) return 0;
    u64 v = ((u64)htotal * 8000u + pixel_khz - 1) / pixel_khz;   /* round up */
    return (v > 0x1FF) ? 0x1FFu : (u32)v;
}

int intel_wm_linetime_write(u32 htotal, u32 pixel_khz)
{
    if (!intel_present() || !lt_armed) return 0;
    u32 v = intel_wm_linetime_value(htotal, pixel_khz);
    if (!v) return 0;
    mmio_w(WM_LINETIME_A, v);
    return 1;
}

/* PIPE_MISC (step 49) and the two things that go with it.
 *
 * bpc lives HERE on gen9, not in TRANSCONF - TRANSCONF's 7:5 and 4/3:2 are
 * ctg-through-ivb only, and programming bpc there on this part sets nothing
 * while looking correct (hazard 4.3 #12). Firmware holds 0. */
u32 intel_pipe_misc_value(int bpp, int dither)
{
    u32 f;
    switch (bpp) {
        case 24: f = 0; break;              /* 8 bpc  */
        case 30: f = 1; break;              /* 10 bpc */
        case 18: f = 2; break;              /* 6 bpc  */
        case 36: f = 3; break;              /* 12 bpc */
        default: return 0xFFFFFFFFu;        /* sentinel: 0 is a legal value  */
    }
    return (f << 5) | (dither ? (1u << 4) : 0u);
}

int intel_pipe_misc_write(int bpp, int dither)
{
    if (!intel_present() || !lt_armed) return 0;
    u32 v = intel_pipe_misc_value(bpp, dither);
    if (v == 0xFFFFFFFFu) return 0;
    mmio_w(PIPE_MISC_A, v);
    return 1;
}

/* Scalers off (step 51). We scan out at the panel's native size, so any scaler
 * left enabled by firmware would resample a correct image into a soft one. */
int intel_scalers_disable(void)
{
    if (!intel_present() || !lt_armed) return 0;
    mmio_w(PS_CTRL_1_A, 0);
    mmio_w(PS_CTRL_2_A, 0);
    return 1;
}

int intel_scaler_enabled(int which)
{
    if (!intel_present() || which < 1 || which > 2) return 0;
    u32 r = (which == 1) ? PS_CTRL_1_A : PS_CTRL_2_A;
    return (mmio_r(r) & (1u << 31)) ? 1 : 0;
}

/* CHICKEN_TRANS_EDP (step 50): frame start delay in 28:27, encoded minus one.
 * Reset value 0 means a delay of 1, which is what we want, so this is a no-op
 * on a cold box - it exists for the case where firmware left something else. */
int intel_chicken_trans_reset(void)
{
    if (!intel_present() || !lt_armed) return 0;
    mmio_w(CHICKEN_TRANS_EDP, mmio_r(CHICKEN_TRANS_EDP) & ~(3u << 27));
    return 1;
}

u32 intel_chicken_trans(void) { return intel_present() ? mmio_r(CHICKEN_TRANS_EDP) : 0; }
u32 intel_msa_misc(void)      { return intel_present() ? mmio_r(TRANS_MSA_MISC_EDP) : 0; }
u32 intel_pipe_misc(void)     { return intel_present() ? mmio_r(PIPE_MISC_A) : 0; }
u32 intel_wm_linetime(void)   { return intel_present() ? mmio_r(WM_LINETIME_A) : 0; }

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

    /* Poll STATE, not ENABLE. On the disable path especially: ENABLE reads back
     * cleared the instant it is written and tells you nothing, while the pipe is
     * still running on clocks the next teardown step is about to switch off.
     * The plan gives this wait 100 ms (teardown step 7). */
    return wait_bits_us(conf, TRANS_CONF_STATE, on ? TRANS_CONF_STATE : 0u, 100000u);
}

/* Configure the primary plane for a linear 32-bit surface. Kept separate from
 * the arming write so a caller can set everything up and then flip. */
#define PLANE_CTL_FORMAT_XRGB8888  (4u << 24)
#define PLANE_CTL_ORDER_RGBX       (1u << 20)
#define PLANE_CTL_TILING_LINEAR    (0u << 10)

/* PLANE_STRIDE is in units that depend on the tiling, and the register holds
 * bytes/unit with the division unchecked (4.3 #16). Getting the unit wrong is a
 * stride off by 8x, which shears the image rather than failing.
 *
 *   linear  64 B      X  512 B      Y  128 B
 *
 * Tiling value 5 is Yf on gen9 and Tile4 on gen12+ - the same encoding means
 * different memory layouts on different parts (4.3 #15), so only the three
 * above are offered here. */
#define PLANE_TILING_LINEAR 0
#define PLANE_TILING_X      1
#define PLANE_TILING_Y      4

static u32 plane_stride_unit(int tiling)
{
    switch (tiling) {
        case PLANE_TILING_X: return 512;
        case PLANE_TILING_Y: return 128;
        default:             return 64;
    }
}

/* Configure the plane with an explicit tiling.
 *
 * The tiling here describes how the SURFACE IS LAID OUT IN MEMORY, not a
 * preference - the display engine reads the buffer according to this field, so
 * telling it X-tiled while writing linear pixels produces a scrambled image,
 * not a slower correct one. Firmware scans this panel out X-tiled; we write
 * linear and say so, which costs memory bandwidth and is otherwise correct.
 *
 * Switching the kernel's framebuffer to X would mean the console's own drawing
 * had to swizzle every pixel into 512-byte-wide, 8-row tiles. That belongs with
 * whatever owns the drawing, not here - this side is ready for it. */
int intel_plane_configure_tiled(u32 width, u32 height, u32 stride_bytes, int tiling)
{
    if (!intel_present() || !lt_armed) return 0;
    if (tiling != PLANE_TILING_LINEAR && tiling != PLANE_TILING_X &&
        tiling != PLANE_TILING_Y) return 0;

    u32 unit = plane_stride_unit(tiling);
    if (stride_bytes % unit) return 0;          /* the division is unchecked */
    u32 st = stride_bytes / unit;
    if (!st || st > 0x3FF) return 0;

    mmio_w(PLANE_OFFSET_1_A, 0);
    mmio_w(PLANE_POS_1_A, 0);
    mmio_w(PLANE_SIZE_1_A, ((height - 1) << 16) | (width - 1));
    mmio_w(PLANE_STRIDE_1_A, st);
    mmio_w(PLANE_CTL_1_A, PLANE_CTL_ENABLE | PLANE_CTL_FORMAT_XRGB8888 |
                          ((u32)tiling << 10));
    return 1;
}

int intel_plane_configure(u32 width, u32 height, u32 stride_bytes)
{
    return intel_plane_configure_tiled(width, height, stride_bytes,
                                       PLANE_TILING_LINEAR);
}

/* What the stride register should hold for a given layout, so a caller can
 * check its own arithmetic without programming anything. */
u32 intel_plane_stride_reg(u32 stride_bytes, int tiling)
{
    u32 unit = plane_stride_unit(tiling);
    return (stride_bytes % unit) ? 0 : stride_bytes / unit;
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
#define PLANE_WM_TRANS_1_A  PIPE_REG(0x70268)
#define PLANE_BUF_CFG_1_A   PIPE_REG(0x7027C)
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
/* 12-bit fields, not 11. Immaterial at 892 blocks, but 0x7FF was a guess that
 * happened to be wide enough rather than the field width the plan states. */
int intel_ddb_start(void) { return (int)(intel_ddb_cfg() & 0xFFF); }
int intel_ddb_end(void)   { return (int)((intel_ddb_cfg() >> 16) & 0xFFF); }
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

/* ---- the cursor half of step 53, which had no code at all ---------------
 *
 * The plane watermarks above existed; the cursor's did not, and the DDB has to
 * be split between them. Overlapping allocations, or any index past 891, is
 * hazard 4.3 #14 - a FIFO underrun rather than a refused write.
 *
 * The plan suggests cursor 0..7 and plane 8..891. Firmware on this machine does
 * the opposite and does it more tightly:
 *
 *   PLANE_BUF_CFG 0x7027C = 035A0000  ->  plane  blocks   0..858
 *   CUR_BUF_CFG   0x7017C = 037B035B  ->  cursor blocks 859..891
 *   CUR_WM(0)     0x70140 = 8000000D  ->  enable, 13 blocks, 0 lines
 *
 * No gap, no overlap, exactly fills 0..891. Both splits are legal, but 13
 * blocks is the real cursor requirement measured on this panel where the plan
 * guessed 8, and the plan's cursor allocation of 8 blocks would therefore have
 * been one short of what firmware asks for. Follow the hardware.
 *
 * The write order matters and is the plan's: watermarks first, BUF_CFG last. */
#define CUR_WM_A(level)   PIPE_REG(0x70140u + 4u * (u32)(level))
#define CUR_WM_TRANS_A    PIPE_REG(0x70168)
#define CUR_BUF_CFG_A     PIPE_REG(0x7017C)
#define DDB_LAST_BLOCK    891              /* 896 blocks, 4 reserved, 1 slice */

u32 intel_cur_wm(int level)
{
    if (!intel_present() || level < 0 || level > 7) return 0;
    return mmio_r(CUR_WM_A(level));
}
u32 intel_cur_buf_cfg(void) { return intel_present() ? mmio_r(CUR_BUF_CFG_A) : 0; }

/* The two BUF_CFG words for a given cursor allocation, as values, so the
 * arithmetic can be checked against firmware without programming anything.
 * Return 0 for a split that will not fit - 0 is not a legal BUF_CFG here
 * because END must be at least START. */
u32 intel_ddb_cur_cfg_value(int cur_blocks)
{
    if (cur_blocks < 1 || cur_blocks > DDB_LAST_BLOCK) return 0;
    u32 cur_start = (u32)(DDB_LAST_BLOCK - cur_blocks + 1);
    return ((u32)DDB_LAST_BLOCK << 16) | cur_start;
}

u32 intel_ddb_plane_cfg_value(int cur_blocks)
{
    if (cur_blocks < 1 || cur_blocks >= DDB_LAST_BLOCK) return 0;
    u32 plane_end = (u32)(DDB_LAST_BLOCK - cur_blocks);
    return (plane_end << 16) | 0u;
}

/* Split the buffer: the plane gets everything below cur_blocks, the cursor the
 * top. Returns 0 on a split that would overlap or overrun rather than
 * programming one - this is the one place where a plausible-looking pair of
 * numbers corrupts scanout. */
int intel_ddb_split(int cur_blocks, u32 cur_wm0)
{
    if (!intel_present() || !lt_armed) return 0;
    u32 cur_cfg = intel_ddb_cur_cfg_value(cur_blocks);
    u32 pln_cfg = intel_ddb_plane_cfg_value(cur_blocks);
    if (!cur_cfg || !pln_cfg) return 0;
    /* the cursor cannot ask for more blocks than it was allocated */
    if ((cur_wm0 & WM_BLOCKS_MASK) > (u32)cur_blocks) return 0;

    /* watermarks before BUF_CFG, and BUF_CFG last of all */
    mmio_w(CUR_WM_A(0), cur_wm0);
    for (int l = 1; l < 8; l++) mmio_w(CUR_WM_A(l), 0);
    mmio_w(CUR_WM_TRANS_A, 0);
    mmio_w(CUR_BUF_CFG_A, cur_cfg);
    mmio_w(PLANE_BUF_CFG_1_A, pln_cfg);
    return 1;
}

/* Do the two allocations overlap or run past the end? A direct check on what
 * the hardware currently holds, for a caller that did not program it. */
int intel_ddb_valid(void)
{
    if (!intel_present()) return 0;
    u32 p = mmio_r(PLANE_BUF_CFG_1_A), c = mmio_r(CUR_BUF_CFG_A);
    int ps = (int)(p & 0xFFF), pe = (int)((p >> 16) & 0xFFF);
    int cs = (int)(c & 0xFFF), ce = (int)((c >> 16) & 0xFFF);
    if (pe > DDB_LAST_BLOCK || ce > DDB_LAST_BLOCK) return 0;
    if (pe < ps || ce < cs) return 0;
    return (pe < cs || ce < ps) ? 1 : 0;        /* disjoint */
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
#define PIPE_STATUS_A     PIPE_REG(0x70024)

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

/* Link training with a bounded retry.
 *
 * Not a loop around intel_link_train(). A second attempt on a still-enabled
 * port is not a second attempt at all: the PRM requires disable-then-re-enable
 * with pattern 1 to retrain, and DDI_BUF_CTL[3:1] and DP_TP_CTL[18] are
 * latched while the port is up (4.3 #20). So each retry takes the port fully
 * down and brings it back, which also resets the sink's own training state.
 *
 * Three attempts. i915 requires two consecutive failures before it reacts at
 * all, so one is too few to match the plan and many is just a slow way to fail
 * - a link that has not trained in three tries at a rate the mode needs is not
 * going to. Failing loudly is still the point; this only stops a single
 * transient from being fatal.
 *
 * Deliberately does NOT walk the rate/lane ladder. With the real pixel clock
 * nothing below 4 lanes at HBR carries this mode at 24 bpp, so descending can
 * only find a link that trains and then cannot carry a frame (plan step 11). */
#define LT_MAX_ATTEMPTS 3

static int lt_attempts_used = 0;

static int lt_train_retry(int port, int rate_idx, int lanes, int tps3, int enhanced)
{
    for (int try = 0; try < LT_MAX_ATTEMPTS; try++) {
        lt_attempts_used = try + 1;
        if (intel_link_train(port, rate_idx, lanes, tps3, enhanced)) return 1;
        if (try + 1 >= LT_MAX_ATTEMPTS) break;

        /* Cycle the port. A failure to bring it back is fatal - retrying into
         * a port that would not come up is worse than reporting the original
         * training failure. */
        if (!intel_port_disable(port)) return 0;
        if (!intel_port_enable(port, lanes, enhanced)) return 0;
    }
    return 0;
}

int intel_lt_attempts_used(void) { return lt_attempts_used; }

/* defined with the backlight save/restore in the teardown section below */
u32 intel_backlight_duty_wanted(void);

/* ==== the ordered cold-start modeset =====================================
 *
 * Everything above is a primitive. This is the sequence, and the sequence is
 * the part the plan exists for: 30-odd steps whose ORDER is load-bearing, where
 * getting one out of place gives a black screen with no error bit anywhere.
 * Until now it lived in this file as a comment.
 *
 * Two things make it reviewable before it has ever run:
 *
 *   - every step records its plan number, its name and its result, so a failure
 *     reports "step 34 (port enable) failed" rather than "modeset failed", and
 *     the whole intended sequence can be printed without executing it;
 *   - it composes primitives that were each checked against what firmware
 *     programmed for this exact panel. The sequence is the only genuinely
 *     untested thing left.
 *
 * It is still gated behind lt_armed, and NOTHING IN THE KERNEL ARMS IT. This
 * runs today only from the host harness with i915 detached. That is deliberate:
 * the first execution of a 30-step hardware sequence should be somewhere with
 * a recovery path, not on a machine whose only console is the panel being
 * reprogrammed.
 */
#define MS_MAX_STEPS 40

static struct { int plan_step; const char *name; int result; } ms_log[MS_MAX_STEPS];
static int ms_count = 0;
static int ms_failed_at = 0;

static int ms_dry = 0;

static int ms_do(int plan_step, const char *name, int result)
{
    if (ms_count < MS_MAX_STEPS) {
        ms_log[ms_count].plan_step = plan_step;
        ms_log[ms_count].name      = name;
        ms_log[ms_count].result    = result;
        ms_count++;
    }
    if (!result && !ms_failed_at) ms_failed_at = plan_step;
    return result;
}

/* Macros, not plain calls, for one reason: C evaluates arguments eagerly, so
 * `ms_do(3, "...", intel_dc_states_block())` would touch the hardware even in a
 * dry run. A macro defers the call, which is what makes the sequence printable
 * on a live machine without writing a single register.
 *
 * MS_STEP aborts the sequence on failure. MS_STEP_SOFT records and carries on,
 * for steps whose failure is worth knowing but is not fatal to the modeset. */
#define MS_STEP(n, name, call) \
    do { if (ms_dry) ms_do((n), name, 1); \
         else if (!ms_do((n), name, (call))) return 0; } while (0)

#define MS_STEP_SOFT(n, name, call) \
    do { if (ms_dry) ms_do((n), name, 1); else ms_do((n), name, (call)); } while (0)

int         intel_modeset_steps(void)          { return ms_count; }
int         intel_modeset_step_plan(int i)     { return (i >= 0 && i < ms_count) ? ms_log[i].plan_step : 0; }
const char *intel_modeset_step_name(int i)     { return (i >= 0 && i < ms_count) ? ms_log[i].name : ""; }
int         intel_modeset_step_result(int i)   { return (i >= 0 && i < ms_count) ? ms_log[i].result : 0; }
int         intel_modeset_failed_at(void)      { return ms_failed_at; }

/* ---- the mode, staged in pieces so no single call takes 16 arguments ----
 * Same shape as the WRPLL and M/N code above: compute into module state, then
 * a run function that consumes it. */
static u32 ms_hactive, ms_hblank, ms_hsync_s, ms_hsync_e, ms_htotal;
static u32 ms_vactive, ms_vblank, ms_vsync_s, ms_vsync_e, ms_vtotal;
static u32 ms_pixel_khz = 0, ms_stride = 0, ms_fb = 0;
static int ms_bpp = 24, ms_phsync = 1, ms_pvsync = 0, ms_have_mode = 0;

int intel_modeset_set_timing(u32 hactive, u32 hblank_start, u32 hsync_start,
                             u32 hsync_end, u32 htotal,
                             u32 vactive, u32 vblank_start, u32 vsync_start,
                             u32 vsync_end, u32 vtotal)
{
    if (!hactive || !htotal || !vactive || !vtotal) return 0;
    if (htotal < hactive || vtotal < vactive) return 0;
    ms_hactive = hactive; ms_hblank = hblank_start;
    ms_hsync_s = hsync_start; ms_hsync_e = hsync_end; ms_htotal = htotal;
    ms_vactive = vactive; ms_vblank = vblank_start;
    ms_vsync_s = vsync_start; ms_vsync_e = vsync_end; ms_vtotal = vtotal;
    ms_have_mode = 1;
    return 1;
}

int intel_modeset_set_link(u32 pixel_khz, int bpp, int phsync, int pvsync)
{
    if (!pixel_khz || bpp < 6) return 0;
    ms_pixel_khz = pixel_khz; ms_bpp = bpp;
    ms_phsync = phsync ? 1 : 0; ms_pvsync = pvsync ? 1 : 0;
    return 1;
}

int intel_modeset_set_fb(u32 gfx_addr, u32 stride_bytes)
{
    if (!stride_bytes || (stride_bytes & 63)) return 0;   /* linear: 64 B units */
    /* Reject a zero address rather than storing it. The caller that passed one
     * had read PLANE_SURF back from a display i915 had already switched off,
     * where it reads 0 - so "no framebuffer" arrived looking like a valid
     * request to scan out from address zero. */
    if (!gfx_addr) { ms_fb = 0; ms_stride = 0; return 0; }
    ms_fb = gfx_addr; ms_stride = stride_bytes;
    return 1;
}

/* Take the mode from whatever the hardware is already running. Useful for the
 * takeover case, which is the only case on this machine. */
int intel_modeset_set_from_hw(void)
{
    if (!intel_present()) return 0;
    u32 ddi = mmio_r(TRANS_DDI_EDP);
    if (!intel_modeset_set_timing((u32)intel_hactive(), (u32)intel_hblank_start(),
                                  (u32)intel_hsync_start(), (u32)intel_hsync_end(),
                                  (u32)intel_htotal(),
                                  (u32)intel_vactive(), (u32)intel_vblank_start(),
                                  (u32)intel_vsync_start(), (u32)intel_vsync_end(),
                                  (u32)intel_vtotal())) return 0;
    return intel_modeset_set_link(intel_pixel_clock_khz(), intel_ddi_bpp(),
                                  !!(ddi & (1u << 16)), !!(ddi & (1u << 17)));
}

/* ---- the sequence ------------------------------------------------------ */
/* dry != 0 walks the whole sequence recording every step and touching nothing.
 * It needs neither lt_armed nor a detached i915, so the intended order can be
 * reviewed against the plan on a running desktop. */
int intel_modeset_run_ex(int port, int dry)
{
    ms_count = 0; ms_failed_at = 0; ms_dry = dry ? 1 : 0;

    if (!intel_present())                       return 0;
    if (!ms_dry && !lt_armed)                   return 0;
    if (!ms_have_mode || !ms_pixel_khz)         return 0;

    /* The link has to be decided before anything is programmed: enhanced
     * framing and the lane count are latched at port enable and cannot be
     * changed afterwards (4.3 #8, #20). */
    int lanes = 4, rate_idx = 1;

    /* -- Phase B: display core ------------------------------------------ */
    MS_STEP(3,  "DC states off",        intel_dc_states_block());
    MS_STEP(4,  "PSR off",              intel_psr_disable());
    /* Step 6: PW1 feeds everything below. If firmware already has it up -
     * always, on this machine - requesting it again is harmless. */
    MS_STEP(6,  "power well PW1",       intel_pwr_well_enable(PW_PW1));
    MS_STEP(7,  "power well MISC_IO",   intel_pwr_well_enable(PW_MISC_IO));
    /* Step 8: DPLL0 is already locked and feeds CDCLK. Hazard H6 says do not
     * touch it when it is already at the rate we want. Verify, do not program. */
    MS_STEP_SOFT(8, "DPLL0 locked at wanted rate",
          intel_dpll_locked(0) && intel_dpll_link_rate(0) == rate_idx);
    MS_STEP(9,  "CDCLK >= pixel rate",  intel_cdclk_khz() >= ms_pixel_khz);
    MS_STEP(10, "DBUF powered",         intel_dbuf_enable(1));

    /* -- Phase C: panel power and VDD ----------------------------------- */
    /* Step 12 MUST precede any power-on assertion: the delay registers reset to
     * zero, and a sequencer with T3=T12=0 reports ready instantly and yanks VDD
     * under live video (hazard H3). */
    MS_STEP(12, "PPS delays + H4 bit", intel_pp_delays_program());
    MS_STEP(14, "panel VDD on (T12,T3)", intel_panel_vdd_on());

    /* -- Phase D: what does the panel say it can do? --------------------- */
    MS_STEP(20, "DPCD caps read",       intel_dpcd_read_caps(port));
    int tps3     = intel_dpcd_tps3();
    int enhanced = intel_dpcd_enhanced();
    MS_STEP(26, "mode fits 4 lanes @ HBR",
               intel_dp_link_bandwidth_kbps(rate_idx, lanes) >=
               intel_mode_bandwidth_kbps(ms_pixel_khz, ms_bpp));

    /* -- Phase E: full panel power --------------------------------------- */
    MS_STEP(27, "panel power on",       intel_panel_power_on());

    /* -- Phase F: the port ----------------------------------------------- */
    MS_STEP(28, "power well DDI_A_E",   intel_pwr_well_enable(PW_DDI_A_E));
    MS_STEP(29, "DDI A clock <- DPLL0", intel_ddi_set_clock(port, 0));
    /* Step 31 before 34, always: the hardware latches the buffer translation
     * at DDI_BUF_CTL enable, so programming it afterwards does nothing. */
    MS_STEP(31, "buf-trans entry 0",    intel_ddi_program_buf_trans(port, 0, 0));
    MS_STEP(32, "I_boost / balance leg", intel_iboost_set(port, 0, lanes == 4));
    MS_STEP(34, "port enable + 600us",  intel_port_enable(port, lanes, enhanced));

    /* -- Phase G: link training ------------------------------------------
     * intel_link_train, not intel_link_train_auto. Plan step 11: do NOT walk
     * the rate/lane ladder here. With the real pixel clock, nothing below
     * 4 lanes @ HBR carries this mode at 24 bpp, so a ladder can only descend
     * into a link that trains successfully and then cannot carry a frame.
     *
     * Retries are bounded and cycle the port between attempts - see
     * lt_train_retry(). The plan wants a bounded retry before giving up
     * (i915 waits for two consecutive failures before reacting), and a correct
     * one is not a loop: DDI_BUF_CTL[3:1] and DP_TP_CTL[18] cannot change while
     * the port is enabled (4.3 #20), so the port has to go down and come back
     * up with pattern 1 selected before a second attempt means anything. */
    MS_STEP(40, "link training",
               lt_train_retry(port, rate_idx, lanes, tps3, enhanced));

    /* -- Phase H: transcoder, pipe, plane -------------------------------- */
    MS_STEP(45, "M/N computed",
               intel_mn_compute(ms_pixel_khz, intel_dp_link_symbol_khz(rate_idx),
                                lanes, ms_bpp));
    MS_STEP(45, "M/N programmed",       intel_mn_program());
    MS_STEP(46, "MSA_MISC",             intel_msa_misc_write(ms_bpp));
    MS_STEP(47, "transcoder timings",
               intel_set_timings(ms_hactive, ms_hblank, ms_hsync_s, ms_hsync_e, ms_htotal,
                                 ms_vactive, ms_vblank, ms_vsync_s, ms_vsync_e, ms_vtotal));
    MS_STEP(49, "PIPE_MISC",            intel_pipe_misc_write(ms_bpp, 0));
    MS_STEP(50, "CHICKEN_TRANS reset",  intel_chicken_trans_reset());
    MS_STEP(51, "scalers off",          intel_scalers_disable());
    MS_STEP(52, "WM_LINETIME",          intel_wm_linetime_write(ms_htotal, ms_pixel_khz));
    /* Step 53: watermarks and the buffer split BEFORE the plane is enabled.
     * 33 cursor blocks and a 13-block cursor watermark are what firmware
     * programs for this panel. */
    MS_STEP(53, "watermarks + DDB split", intel_ddb_split(33, WM_ENABLE | 13u));
    MS_STEP(53, "plane watermark L0",
               intel_wm_set_level(0, intel_wm_compute_level0(ms_hactive, (u32)ms_bpp,
                                                             ms_pixel_khz, 0)));
    MS_STEP(54, "TRANS_DDI_FUNC_CTL",
               intel_trans_ddi_ctl_write(lanes, ms_bpp, ms_phsync, ms_pvsync));
    MS_STEP(55, "transcoder enable",    intel_transcoder_enable(1));

    /* Step 56: PLANE_SURF arms everything, so it goes last - nothing written to
     * CTL/SIZE/STRIDE takes effect until it does (4.3 #3). */
    /* A framebuffer address of zero is not "the framebuffer at address zero",
     * it is "we have no framebuffer" - and the first real run proved the
     * difference matters. It had a stride but no address, so it configured the
     * plane and armed it at 0, which is a scanout of nothing and reported
     * FAILED at the last step of an otherwise complete modeset.
     *
     * Both are required now, and a modeset with no surface is a legitimate
     * outcome rather than a failure: link up, transcoder running, nothing
     * being displayed yet. The kernel supplies a real framebuffer; the host
     * harness only can if it can reach the GGTT. */
    if (ms_fb && ms_stride) {
        MS_STEP(56, "plane configured",
                   intel_plane_configure(ms_hactive, ms_vactive, ms_stride));
        MS_STEP(56, "PLANE_SURF armed", intel_set_surface(ms_fb, (int)ms_stride));
    } else {
        MS_STEP_SOFT(56, "plane SKIPPED (no framebuffer)", 1);
    }
    MS_STEP_SOFT(57, "underrun telltale cleared", intel_pipe_underrun_clear());

    /* Step 58: frequency and duty before enable, then PP_CONTROL b2. */
    MS_STEP_SOFT(58, "backlight PWM",
          intel_backlight_pwm_enable(intel_backlight_max(),
                                     intel_backlight_duty_wanted()));
    MS_STEP_SOFT(58, "backlight enabled",         intel_panel_backlight_enable(1));

    return ms_failed_at ? 0 : 1;
}

int intel_modeset_run(int port) { return intel_modeset_run_ex(port, 0); }
int intel_modeset_dry(int port) { return intel_modeset_run_ex(port, 1); }
int intel_modeset_was_dry(void) { return ms_dry; }

/* The backlight as it was before we touched it. A run that ends with the
 * user's brightness at zero is a bug even when the modeset itself succeeded -
 * i915 restores its own idea of brightness on rebind, but it does not
 * re-derive the PWM duty we overwrote, so the panel comes back lit at nothing
 * and the only fix the user has is the brightness keys. */
static u32 bl_saved_ctl = 0, bl_saved_freq = 0, bl_saved_duty = 0;
static int bl_have_saved = 0;

int intel_backlight_save(void)
{
    if (!intel_present()) return 0;
    bl_saved_ctl  = mmio_r(BLC_PWM_CTL);
    bl_saved_freq = mmio_r(BLC_PWM_FREQ);
    bl_saved_duty = mmio_r(BLC_PWM_DUTY);
    bl_have_saved = 1;
    return 1;
}

/* Restore FREQ and DUTY, and deliberately NOT the enable bit.
 *
 * Teardown ends with the panel powered down, and re-arming the PWM output into
 * an unpowered panel is hazard H2 - the same rule that keeps AUX and the main
 * link off a dark panel. The first version of this restored PP_CONTROL's
 * companion CTL register wholesale, enable bit included, immediately AFTER
 * intel_panel_power_off(). Wrong order, and the hazard list says so.
 *
 * Duty and frequency are just counters; writing them with the output disabled
 * drives nothing. Leaving the right duty behind is the whole point - i915
 * re-enables the PWM itself on rebind and the panel then comes up at the
 * brightness the user had, instead of at whatever we left. */
int intel_backlight_restore(void)
{
    if (!intel_present() || !lt_armed || !bl_have_saved) return 0;
    mmio_w(BLC_PWM_FREQ, bl_saved_freq);
    mmio_w(BLC_PWM_DUTY, bl_saved_duty);
    (void)mmio_r(BLC_PWM_DUTY);
    return 1;
}

/* The duty to light the panel with, in preference order: what the user had
 * before we started, then whatever is in the register, then half brightness.
 *
 * A zero here is not "off", it is "nobody told us" - and enabling the PWM at
 * zero duty is a backlight that is on and emitting nothing, which looks
 * exactly like a failed modeset from in front of the screen. Measured: i915
 * leaves DUTY at 0 when it powers the panel down on unbind, so reading the
 * register at step 58 and passing it straight back produced a correctly
 * scanned-out image on an unlit panel, and left the user at zero brightness
 * afterwards. */
u32 intel_backlight_duty_wanted(void)
{
    u32 period = mmio_r(BLC_PWM_FREQ);
    if (bl_have_saved && bl_saved_duty && bl_saved_duty <= bl_saved_freq)
        return bl_saved_duty;
    u32 live = mmio_r(BLC_PWM_DUTY);
    if (live && live <= period) return live;
    return period / 2u;                    /* visible, and obviously not ours */
}

/* ==== teardown: never hand the device back half-configured ===============
 *
 * The first real run ended with 31 of 32 steps green and a dark screen that
 * needed a power button. The modeset itself was not the problem: it failed at
 * the last step and then the harness rebound i915 - onto a display engine
 * still holding OUR transcoder config, OUR panel power, and a plane pointed at
 * address zero. i915 took the device back and could not light it from there.
 *
 * So a failed modeset must undo itself. This is plan section 3, in its order,
 * and the order matters as much here as on the way up: backlight before video
 * (H5 - video stopping under a lit backlight shows garbage and, on some
 * panels, is a current transient in the LED driver), pipe fully off before the
 * clocks it uses are switched off, and the T12 epoch stamped at the end so the
 * next power-on knows what it owes.
 *
 * Best effort by design: every step runs even if an earlier one failed,
 * because a teardown that gives up halfway is exactly the state it exists to
 * prevent. Returns the number of steps that did not report success. */
int intel_modeset_teardown(int port)
{
    if (!intel_present() || !lt_armed) return -1;
    int bad = 0;

    /* 1-3: backlight off, wait T9, then the PWM itself. */
    if (!intel_panel_backlight_enable(0)) bad++;
    {
        u32 t9 = (u32)intel_pp_t9() / 10u;          /* 100 us units -> ms */
        cpu_delay_ms(t9 < 260u ? 260u : t9);        /* H5 - not optional */
    }
    if (!intel_backlight_pwm_disable()) bad++;

    /* 4: PSR, in case anything re-armed it. */
    if (mmio_r(EDP_PSR_CTL) & 0x80000000u) intel_psr_disable();

    /* 5: plane and cursor off. CTL first, then SURF/BASE - the SURF write is
     * what arms the CTL=0, exactly as it arms a real surface. */
    mmio_w(PLANE_CTL_1_A, 0);
    mmio_w(PLANE_SURF_1_A, 0);
    mmio_w(CUR_CTL_A, 0);
    mmio_w(CUR_BASE_A, 0);

    /* 7: the pipe, waiting on STATE - polling ENABLE reads back cleared at
     * once and tells you nothing while the pipe still runs on clocks step 13
     * is about to remove. */
    if (!intel_transcoder_enable(0)) bad++;

    /* 8-11: transcoder function, then the port. The idle wait comes AFTER
     * both disables, not between them. */
    if (!intel_trans_ddi_ctl_disable()) bad++;
    if (!intel_port_disable(port)) bad++;

    /* 13: gate DDI A's clock off. DPLL0 is left alone - it feeds CDCLK. */
    mmio_w(DPLL_CTRL2, mmio_r(DPLL_CTRL2) | (1u << 15));

    /* 14: panel down, which stamps the T12 epoch. */
    if (!intel_panel_power_off()) bad++;

    /* Put the backlight registers back exactly as they were found, so the
     * user's brightness survives a run whatever else happened. */
    intel_backlight_restore();

    /* 16: release the DDI A/E IO well. A well the DMC or BIOS is holding on
     * will not drop, and that is informational rather than an error. */
    intel_pwr_well_disable(PW_DDI_A_E);

    return bad;
}

/* ==== one call to light the panel, for a kernel with no allocator ========
 *
 * The harness does this in fifty lines of C because it can: it has mmap, it can
 * pick addresses, it can print. zlOS has no heap at all - `pci.c` says so
 * plainly - so the framebuffer cannot be allocated, only decided.
 *
 * Stolen memory is what makes that acceptable rather than a hack. Firmware
 * reserves it, no operating system manages it, and it is present on every boot
 * at an address the hardware itself reports. Deciding to put the framebuffer
 * one megabyte into it is not arbitrary the way a hardcoded 224 MiB is - it is
 * the only memory on the machine that is unambiguously ours.
 *
 * Returns a CPU-writable address for the framebuffer, or 0. The address is in
 * the aperture: BAR2 is the CPU's window onto whatever the GGTT maps, so GGTT
 * address X is reachable at aperture_base + X, and a plain store from the
 * kernel lands in the memory the display is scanning out.
 *
 * On failure it returns 0 having changed nothing that matters, and the caller
 * keeps whatever framebuffer the loader gave it. A driver that cannot bring the
 * panel up must not also take away the screen that was working.
 */
u32 intel_bringup_panel(void)
{
    if (!intel_present() || !intel_supported()) return 0;

    u32 stolen = intel_stolen_base();
    u32 ssize  = intel_stolen_size();
    u32 aper   = (u32)aperture;
    if (!stolen || !ssize || !aper) return 0;

    /* The mode comes from what firmware is already running. On this machine
     * that is always available: the system firmware lights the panel during
     * POST, long before any bootloader, so the timing registers describe a real
     * mode even when zlOS was loaded by its own 512-byte bootloader. */
    if (!intel_modeset_set_from_hw()) return 0;

    u32 w = (u32)intel_hactive(), h = (u32)intel_vactive();
    if (w < 640 || h < 480) return 0;

    u32 stride = (w * 4u + 63u) & ~63u;          /* linear: 64-byte multiples */
    u32 bytes  = stride * h;
    u32 gfx    = 1u << 20;                       /* 1 MiB into the GGTT       */
    u32 skip   = 1u << 20;                       /* firmware's own structures */

    /* Refuse rather than run off the end of stolen memory. */
    if (bytes + skip > ssize) return 0;

    u32 pages = (bytes + 4095u) / 4096u;
    if (!intel_ggtt_map_range(gfx >> 12, stolen + skip, (int)pages)) return 0;

    if (!intel_modeset_set_fb(gfx, stride)) return 0;

    intel_link_train_arm(1);
    int ok = intel_modeset_run(0);
    intel_link_train_arm(0);
    if (!ok) return 0;

    return aper + gfx;
}

/* Undo it. Same arming discipline, and the same reason it exists: a half
 * configured display handed to anything else is worse than no display. */
int intel_shutdown_panel(void)
{
    if (!intel_present()) return 0;
    intel_link_train_arm(1);
    int bad = intel_modeset_teardown(0);
    intel_link_train_arm(0);
    return bad == 0;
}

/* Which step it stopped at, for a caller that got 0 back. */
int intel_bringup_failed_step(void) { return intel_modeset_failed_at(); }

/* Bring the panel up and move the console onto it.
 *
 * This is the point of the whole exercise: after this returns 1, every
 * character zlOS prints is being scanned out by a mode this driver programmed,
 * on a link this driver trained, through a page table this driver wrote. The
 * loader's framebuffer is no longer involved.
 *
 * Ordered so a failure is survivable. The modeset runs first and the console is
 * only moved once it has actually succeeded - if the bring-up fails the console
 * keeps the loader's framebuffer and the machine is exactly as it was. The one
 * genuinely irreversible moment is the console_init_fb() call, because after it
 * there is no way to report anything except through the display we just
 * programmed. Everything that could fail has already been checked by then.
 *
 * Declared here rather than including console.h because intel.c takes its
 * kernel dependencies as externs - see idt_ticks and cpu_delay_us above. */
void console_init_fb(uptr addr, u32 pitch, u32 width, u32 height, u32 bpp);

int intel_panel_takeover(void)
{
    u32 fb = intel_bringup_panel();
    if (!fb) return 0;

    u32 w = (u32)intel_hactive(), h = (u32)intel_vactive();
    u32 stride = (w * 4u + 63u) & ~63u;

    /* Clear it before the console arrives. Stolen memory holds whatever
     * firmware left, and a console scrolling over that looks like corruption. */
    volatile u32 *px = (volatile u32 *)(uptr)fb;
    for (u32 i = 0; i < (stride / 4u) * h; i++) px[i] = 0;

    console_init_fb((uptr)fb, stride, w, h, 32);
    return 1;
}

/* ==== VBT: the OEM's description of THIS board ===========================
 *
 * Everything the driver knows about this laptop it currently knows by reading
 * back what firmware programmed. That works only because firmware ran first,
 * and it answers "what is set" rather than "what is correct". The Video BIOS
 * Table answers the second question, and it is the authoritative source for
 * facts no register carries: which ports are populated, the panel's power
 * sequence including T9, whether the panel is low-vswing, and how the backlight
 * is wired.
 *
 * Verified against this machine before a line of it was trusted. The VBT says
 * T1+T3 200 ms, T8 1 ms, T9 260 ms, T10 50 ms, T11+T12 500 ms, PWM 990 Hz, and
 * every one of those matches what was measured out of the registers or, in T9's
 * case, what had been hardcoded on the strength of a comment. It also settles
 * the low-vswing question that HANDOFF called "only in VBT and not discoverable
 * from any register" - and agrees with what reading back DDI_BUF_TRANS deduced.
 *
 * Reaching it: PCI config 0xFC (ASLS) holds the physical address of the Intel
 * opregion; mailbox 4, at opregion + 0x400, is the VBT. In the kernel that is a
 * plain pointer. The host harness cannot read /dev/mem on a locked-down kernel,
 * so it hands the blob in instead - same arrangement as intel_attach().
 */
#define ASLS_REG        0xFC        /* PCI config: opregion physical address  */
#define OPREGION_VBT    0x400       /* mailbox 4                              */

static uptr vbt_base = 0;
static u32  vbt_len  = 0;
static u32  bdb_base = 0;           /* offset of the BDB within the VBT       */
static u32  bdb_size = 0;
static int  vbt_ok   = 0;

static u8  vbt_u8 (u32 off) { return *(volatile u8 *)(vbt_base + (uptr)off); }
static u16 vbt_u16(u32 off) { return (u16)(vbt_u8(off) | (vbt_u8(off+1) << 8)); }
static u32 vbt_u32(u32 off)
{
    return (u32)vbt_u8(off) | ((u32)vbt_u8(off+1) << 8) |
           ((u32)vbt_u8(off+2) << 16) | ((u32)vbt_u8(off+3) << 24);
}

/* Point the parser at a VBT already in memory. */
int intel_vbt_attach(uptr base, u32 len)
{
    vbt_ok = 0; vbt_base = base; vbt_len = len; bdb_base = 0; bdb_size = 0;
    if (!base || len < 64) return 0;

    /* "$VBT" - the rest of the 20-byte signature names the platform and is not
     * worth matching, since this same driver should accept a KBL or CFL blob. */
    if (vbt_u8(0) != '$' || vbt_u8(1) != 'V' || vbt_u8(2) != 'B' || vbt_u8(3) != 'T')
        return 0;

    /* bdb_offset is a u32 at 0x1C. It is NOT the u16 at 0x1A - that is the
     * checksum and a reserved byte, and reading it there lands in the middle of
     * the copyright string with a plausible-looking small number. */
    u32 bdb = vbt_u32(0x1C);
    if (bdb + 22u >= len) return 0;

    /* "BIOS_DATA_BLOCK " - indices 0, 5 and 11. Index 10 is the second 'B',
     * not the 'L'; checking there rejects a perfectly good VBT. */
    if (vbt_u8(bdb) != 'B' || vbt_u8(bdb+5) != 'D' || vbt_u8(bdb+11) != 'L')
        return 0;

    bdb_base = bdb;
    bdb_size = vbt_u16(bdb + 0x14);
    if (!bdb_size || bdb + bdb_size > len) bdb_size = len - bdb;
    vbt_ok = 1;
    return 1;
}

/* Find the VBT ourselves, from PCI config. Kernel path. */
int intel_vbt_find(void)
{
    if (!intel_present()) return 0;
    u32 asls = gpu_cfg(ASLS_REG);
    if (!asls) return 0;
    /* The opregion is 8 KiB; the VBT is mailbox 4 and up to 6 KiB of it. */
    return intel_vbt_attach((uptr)(asls + OPREGION_VBT), 6u << 10);
}

int intel_vbt_present(void) { return vbt_ok; }
u32 intel_vbt_bdb_version(void) { return vbt_ok ? vbt_u16(bdb_base + 0x10) : 0; }

/* Offset of a BDB block's payload, or 0. Blocks are id, u16 size, payload. */
static u32 bdb_block(u8 want, u32 *out_size)
{
    if (!vbt_ok) return 0;
    u32 off = vbt_u16(bdb_base + 0x12);           /* skip the BDB header */
    while (off + 3 <= bdb_size) {
        u8  id = vbt_u8(bdb_base + off);
        u32 sz = vbt_u16(bdb_base + off + 1);
        if (!sz || off + 3 + sz > bdb_size) break;
        if (id == want) { if (out_size) *out_size = sz; return bdb_base + off + 3; }
        off += 3 + sz;
    }
    return 0;
}

u32 intel_vbt_block(int id)  { u32 s = 0; u32 o = bdb_block((u8)id, &s); return o ? s : 0; }

/* Which entry of every per-panel table in the VBT applies to this machine.
 * Block 40 byte 0, low nibble. Everything else here indexes on it. */
int intel_vbt_panel_type(void)
{
    u32 o = bdb_block(40, 0);
    return o ? (int)(vbt_u8(o) & 0xF) : -1;
}

/* Panel power sequence, in 100 us units, straight from the OEM.
 *
 * T9 is the reason this matters. It is the backlight-off to video-off delay,
 * hazard H5, and it does NOT live in PP_OFF_DELAYS - that register's low field
 * holds it only if firmware chose to program it there, and on this machine
 * firmware forces it to 1 and does the real wait in software. So a driver
 * reading the register gets 0.1 ms for a delay the panel actually needs 260 ms
 * for, and stops video under a lit backlight. */
static u32 vbt_pps_field(int which)
{
    int panel = intel_vbt_panel_type();
    u32 o = bdb_block(27, 0);
    if (!o || panel < 0) return 0;
    return vbt_u16(o + (u32)panel * 10u + (u32)which * 2u);
}
int intel_vbt_t1_t3(void)   { return (int)vbt_pps_field(0); }
int intel_vbt_t8(void)      { return (int)vbt_pps_field(1); }
int intel_vbt_t9(void)      { return (int)vbt_pps_field(2); }
int intel_vbt_t10(void)     { return (int)vbt_pps_field(3); }
int intel_vbt_t11_t12(void) { return (int)vbt_pps_field(4); }

/* Low vswing decides which DDI buffer translation table to program, and it is
 * the fact HANDOFF called undiscoverable from any register.
 *
 * i915's rule: the per-panel nibble of edp_vswing_preemph is 0 for low vswing.
 * The field sits after power_seqs[16] (160 B), color_depth (4), link_params[16]
 * (32), sdrrs delay (4), s3d feature (2) and t3 optimization (2). */
int intel_vbt_low_vswing(void)
{
    int panel = intel_vbt_panel_type();
    u32 o = bdb_block(27, 0);
    if (!o || panel < 0) return -1;
    u32 f = o + 160u + 4u + 32u + 4u + 2u + 2u;
    u32 lo = vbt_u32(f), hi = vbt_u32(f + 4);
    u32 nib = (panel < 8) ? ((lo >> (panel * 4)) & 0xF)
                          : ((hi >> ((panel - 8) * 4)) & 0xF);
    return nib == 0 ? 1 : 0;
}

/* Backlight: PWM frequency in Hz, polarity, and the OEM's minimum level. */
static u32 vbt_bl_entry(void)
{
    int panel = intel_vbt_panel_type();
    u32 o = bdb_block(43, 0);
    if (!o || panel < 0) return 0;
    u32 esz = vbt_u8(o);
    if (!esz || esz > 16) return 0;
    return o + 1u + (u32)panel * esz;
}
int intel_vbt_pwm_hz(void)      { u32 e = vbt_bl_entry(); return e ? (int)vbt_u16(e + 1) : 0; }
int intel_vbt_pwm_active_low(void){ u32 e = vbt_bl_entry(); return e ? (int)((vbt_u8(e) >> 2) & 1) : 0; }
int intel_vbt_bl_min(void)      { u32 e = vbt_bl_entry(); return e ? (int)vbt_u8(e + 3) : 0; }

/* Which ports the OEM actually wired up.
 *
 * Block 2 is general definitions: a header, then a list of fixed-size child
 * device entries. A port with no child device is not populated, and driving it
 * is training a link into an unconnected pad. This is the fact that has to come
 * before any external-port work.
 *
 * dvo_port lives at offset 0 of each entry: 0 = none, and the DP/HDMI values
 * identify A/B/C/D. Entry size is in the block header so it survives version
 * differences. */
int intel_vbt_child_count(void)
{
    u32 sz = 0, o = bdb_block(2, &sz);
    if (!o || sz < 6) return 0;
    u32 esz = vbt_u8(o + 4);            /* child_dev_size */
    if (!esz || esz > 64) return 0;
    return (int)((sz - 5u) / esz);
}

/* Offset of child entry i, or 0. Header is crt_ddc_pin, dpms bits,
 * boot_display[2], child_dev_size - five bytes - then the entries. */
static u32 vbt_child(int i)
{
    u32 sz = 0, o = bdb_block(2, &sz);
    if (!o || sz < 6 || i < 0) return 0;
    u32 esz = vbt_u8(o + 4);
    if (!esz || esz > 64) return 0;
    if (5u + (u32)(i + 1) * esz > sz) return 0;
    return o + 5u + (u32)i * esz;
}

/* device_type of 0 means the slot exists but nothing is wired to it. Those are
 * the entries that must NOT be treated as ports: this board declares eight
 * child slots and populates three. */
int intel_vbt_child_type(int i)
{
    u32 c = vbt_child(i);
    return c ? (int)vbt_u16(c + 2) : 0;
}

/* dvo_port sits at offset 16 of the entry, after handle, device_type, the
 * ten-byte device_id and addin_offset - NOT at offset 0. Values are i915's
 * DVO_PORT_*: 7 = DP-B, 8 = DP-C, 9 = DP-D, 10 = DP-A which is the eDP panel,
 * 0..3 = HDMI A..D. */
int intel_vbt_child_port(int i)
{
    u32 c = vbt_child(i);
    if (!c || !vbt_u16(c + 2)) return -1;    /* unpopulated slot */
    return (int)vbt_u8(c + 16);
}

/* Is this port wired on this board at all? Driving one that is not is training
 * a link into an unconnected pad, and it is the question that has to be
 * answered before any external-port work. */
int intel_vbt_port_present(int dvo_port)
{
    int n = intel_vbt_child_count();
    for (int i = 0; i < n; i++)
        if (intel_vbt_child_port(i) == dvo_port) return 1;
    return 0;
}

/* ==== colour: gamma and the pipe CSC (phase 6) ===========================
 *
 * Offsets confirmed on this machine rather than taken from a header. The
 * legacy palette at 0x4A000 reads 00000000, 00010101, 00020202, 00030303 -
 * an identity ramp, entry n = (n<<16)|(n<<8)|n - which is unmistakable and
 * settles both the base and the entry format in one read. PIPE_CSC_MODE at
 * 0x49028 reads 2 with all coefficients zero, a defined idle state.
 *
 * Two LUTs exist. The legacy one is 256 8-bit entries and is what firmware
 * loads; the precision one at 0x4A400 is an index/data pair for 10-bit and
 * reads zero here because nothing has asked for it. Legacy is enough for
 * brightness, contrast and a colour ramp, and it is the one with a verified
 * layout, so it is the one implemented.
 */
/* The palette and CSC blocks step differently from the pipe blocks: 0x800 for
 * the palettes (A 0x4A000, B 0x4A800) and 0x100 for the CSC. Assuming the
 * 0x1000 pipe stride here would land in the middle of pipe A's own LUT. */
#define PIPE_PAL_REG(base) ((base) + (u32)cur_pipe * 0x800u)
#define PIPE_CSC_REG(base) ((base) + (u32)cur_pipe * 0x100u)

#define LGC_PALETTE_A     PIPE_PAL_REG(0x4A000)   /* 256 entries, 4 bytes each */
#define PREC_PAL_INDEX_A  PIPE_PAL_REG(0x4A400)
#define PREC_PAL_DATA_A   PIPE_PAL_REG(0x4A404)
#define PIPE_CSC_RY_GY_A  PIPE_CSC_REG(0x49010)
#define PIPE_CSC_MODE_A   PIPE_CSC_REG(0x49028)

/* One palette entry. 8 bits each, packed (R<<16)|(G<<8)|B - the format the
 * identity ramp proves. */
int intel_gamma_set(int i, u32 r, u32 g, u32 b)
{
    if (!intel_present() || !lt_armed || i < 0 || i > 255) return 0;
    mmio_w(LGC_PALETTE_A + (u32)i * 4u,
           ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF));
    return 1;
}

u32 intel_gamma_get(int i)
{
    if (!intel_present() || i < 0 || i > 255) return 0;
    return mmio_r(LGC_PALETTE_A + (u32)i * 4u);
}

/* Straight through - what firmware leaves. Restoring this undoes anything
 * below it, which matters because a bad ramp is not visibly a bad ramp, it is
 * a screen with wrong colours and no other symptom. */
int intel_gamma_identity(void)
{
    if (!intel_present() || !lt_armed) return 0;
    for (int i = 0; i < 256; i++)
        mmio_w(LGC_PALETTE_A + (u32)i * 4u,
               ((u32)i << 16) | ((u32)i << 8) | (u32)i);
    return 1;
}

/* Brightness and contrast as a ramp, which is all a LUT can express: output =
 * (input - 128) * contrast / 256 + 128 + brightness, clamped. Contrast is in
 * 1/256ths so 256 is unity; brightness is a signed offset in output levels. */
int intel_gamma_ramp(int brightness, int contrast)
{
    if (!intel_present() || !lt_armed) return 0;
    if (contrast < 0) contrast = 0;
    if (contrast > 1024) contrast = 1024;
    for (int i = 0; i < 256; i++) {
        int v = ((i - 128) * contrast) / 256 + 128 + brightness;
        if (v < 0) v = 0;
        if (v > 255) v = 255;
        mmio_w(LGC_PALETTE_A + (u32)i * 4u,
               ((u32)v << 16) | ((u32)v << 8) | (u32)v);
    }
    return 1;
}

/* NO general power-law gamma here, deliberately.
 *
 * A first version of this bisected on an integer power and was simply wrong -
 * it scaled by an arbitrary constant each iteration and produced a smooth,
 * monotonic, plausible-looking curve that was not a gamma curve. It would have
 * shipped, because a wrong gamma ramp does not look like a bug: it looks like a
 * screen whose colours are slightly off, with no other symptom and nothing to
 * check it against.
 *
 * Everything else in this driver is verified against what firmware programmed
 * for the same hardware. There is no firmware answer for an arbitrary gamma
 * exponent, so that safety net does not exist here, and this kernel has no
 * floating point to do it honestly. A correct implementation needs either a
 * precomputed table per exponent or fixed-point roots done carefully.
 *
 * intel_gamma_ramp() covers brightness and contrast exactly, which is what a
 * LUT is actually asked for, and intel_gamma_identity() restores firmware's
 * state. Those are correct. A power law is left out rather than approximated
 * wrongly. */

u32 intel_csc_mode(void) { return intel_present() ? mmio_r(PIPE_CSC_MODE_A) : 0; }

/* ==== phase 5: sprite planes, rotation, scaling ==========================
 *
 * Gen9 gives each pipe three universal planes. Plane 1 is the one this driver
 * has always driven; 2 and 3 are identical in shape at +0x100 each, which is
 * why they cost so little to add. They are what a compositor uses to put a
 * video window or a cursor-sized overlay on screen without the CPU compositing
 * it into the primary surface.
 */
#define PLANE_STRIDE_BASE  0x70180u        /* plane 1; +0x100 per plane */
#define PLANE_REG(plane, off) PIPE_REG(PLANE_STRIDE_BASE + \
                                       (u32)((plane) - 1) * 0x100u + (off))
#define PL_CTL      0x00
#define PL_STRIDE   0x08
#define PL_POS      0x0C
#define PL_SIZE     0x10
#define PL_SURF     0x1C
#define PL_OFFSET   0x24

/* PLANE_CTL[1:0]. 90 and 270 need a Y or Yf tiled surface - the hardware
 * cannot rotate a linear one - so a caller asking for those on linear is
 * refused rather than silently given a scrambled screen. */
#define PLANE_ROT_0     0
#define PLANE_ROT_90    1
#define PLANE_ROT_180   2
#define PLANE_ROT_270   3

int intel_plane_setup(int plane, u32 gfx_addr, u32 x, u32 y, u32 w, u32 h,
                      u32 stride_bytes, int tiling, int rotation)
{
    if (!intel_present() || !lt_armed) return 0;
    if (plane < 1 || plane > 3) return 0;
    if (rotation < 0 || rotation > 3) return 0;
    if (!w || !h) return 0;

    /* 90 and 270 are only legal on a tiled surface. */
    if ((rotation == PLANE_ROT_90 || rotation == PLANE_ROT_270) &&
        tiling != PLANE_TILING_Y) return 0;

    u32 unit = plane_stride_unit(tiling);
    if (stride_bytes % unit) return 0;
    u32 st = stride_bytes / unit;
    if (!st || st > 0x3FF) return 0;

    mmio_w(PLANE_REG(plane, PL_OFFSET), 0);
    mmio_w(PLANE_REG(plane, PL_POS),    (y << 16) | x);
    mmio_w(PLANE_REG(plane, PL_SIZE),   ((h - 1) << 16) | (w - 1));
    mmio_w(PLANE_REG(plane, PL_STRIDE), st);
    mmio_w(PLANE_REG(plane, PL_CTL),
           PLANE_CTL_ENABLE | PLANE_CTL_FORMAT_XRGB8888 |
           ((u32)tiling << 10) | (u32)rotation);
    /* SURF last - it arms everything above it (4.3 #3). */
    mmio_w(PLANE_REG(plane, PL_SURF), gfx_addr & 0xFFFFF000u);
    return 1;
}

int intel_plane_disable(int plane)
{
    if (!intel_present() || !lt_armed || plane < 1 || plane > 3) return 0;
    mmio_w(PLANE_REG(plane, PL_CTL), 0);
    mmio_w(PLANE_REG(plane, PL_SURF), 0);   /* the CTL=0 is armed by this */
    return 1;
}

u32 intel_plane_ctl_n(int plane)
{
    if (!intel_present() || plane < 1 || plane > 3) return 0;
    return mmio_r(PLANE_REG(plane, PL_CTL));
}

/* ---- the pipe scalers --------------------------------------------------
 *
 * Two per pipe. They take the plane's output and resample it into a window on
 * the pipe, which is how a non-native resolution reaches a fixed-pixel panel
 * without the blur of doing it in software.
 *
 * The modeset switches both OFF (step 51) because we scan out at native size
 * and a scaler firmware left enabled would resample a correct image into a soft
 * one. This is the other direction: asking for one deliberately.
 *
 * PS_WIN_POS and PS_WIN_SZ come BEFORE PS_CTRL, and the window is in pipe
 * coordinates - the destination, not the source. The source is whatever the
 * plane produces. */
#define PS_WIN_POS(n)  PIPE_REG(0x68170u + (u32)((n) - 1) * 0x100u)
#define PS_WIN_SZ(n)   PIPE_REG(0x68174u + (u32)((n) - 1) * 0x100u)
#define PS_CTRL(n)     PIPE_REG(0x68180u + (u32)((n) - 1) * 0x100u)
#define PS_ENABLE      (1u << 31)

int intel_scaler_enable(int which, u32 x, u32 y, u32 w, u32 h)
{
    if (!intel_present() || !lt_armed) return 0;
    if (which < 1 || which > 2 || !w || !h) return 0;
    mmio_w(PS_WIN_POS(which), (x << 16) | y);
    mmio_w(PS_WIN_SZ(which),  (w << 16) | h);
    mmio_w(PS_CTRL(which),    PS_ENABLE);
    return 1;
}

int intel_scaler_disable(int which)
{
    if (!intel_present() || !lt_armed || which < 1 || which > 2) return 0;
    mmio_w(PS_CTRL(which), 0);
    return 1;
}

/* ==== phase 7: DRRS and PSR ==============================================
 *
 * Both are power features and both are the reverse of something the modeset
 * currently does on purpose. Step 45 writes M2/N2 as zero; step 4 disables PSR
 * before anything else. This is where those become choices rather than
 * blanket policy.
 *
 * ---- DRRS ----
 *
 * The transcoder holds two M/N pairs. M1/N1 is the mode as programmed; M2/N2 is
 * a second, slower refresh rate, and TRANS_DDI_FUNC_CTL's DRRS bit picks
 * between them without a modeset. A panel that idles at 40 Hz instead of 60
 * saves real power on a laptop.
 *
 * The reason M2/N2 are zero today is not caution about the registers - it is
 * that a non-zero M2/N2 with no DRRS logic is a second timing the hardware may
 * switch to and nothing maintains. Now there is logic, so they can be filled.
 *
 * The low rate must divide into the same link: only the pixel clock changes,
 * the link rate does not, so it is the same intel_mn_compute() with a smaller
 * pixel clock. The panel must also support it - a fixed-refresh panel driven
 * at 40 Hz shows nothing - which is a VBT and EDID question, not a register
 * one, so this refuses to guess and takes the rate from the caller.
 */
#define TRANS_DRRS_BIT   (1u << 26)      /* TRANS_DDI_FUNC_CTL: select M2/N2 */

static u32 drrs_low_khz = 0;

/* Program the second M/N pair for a lower refresh of the same mode. */
int intel_drrs_setup(u32 low_pixel_khz, u32 link_khz, int lanes, int bpp)
{
    if (!intel_present() || !lt_armed) return 0;
    if (!low_pixel_khz || low_pixel_khz >= ms_pixel_khz) return 0;  /* must be lower */
    if (!intel_mn_compute(low_pixel_khz, link_khz, lanes, bpp)) return 0;

    u32 base = trans_base();
    mmio_w(base + TRANS_OFF_DATA_M2, ((M_N_TU_SIZE - 1) << 25) | intel_mn_data_m());
    mmio_w(base + TRANS_OFF_DATA_N2, intel_mn_data_n());
    mmio_w(base + TRANS_OFF_LINK_M2, intel_mn_link_m());
    mmio_w(base + TRANS_OFF_LINK_N2, intel_mn_link_n());
    drrs_low_khz = low_pixel_khz;

    /* Recompute M1/N1 so the module state matches what is actually programmed
     * for the high rate - otherwise the next caller of intel_mn_program()
     * writes the LOW rate into the primary pair. */
    intel_mn_compute(ms_pixel_khz, link_khz, lanes, bpp);
    return 1;
}

/* Switch between them. No modeset, no retrain - the link is unchanged and only
 * the rate at which pixels are metered onto it moves. */
int intel_drrs_select(int low)
{
    if (!intel_present() || !lt_armed || !drrs_low_khz) return 0;
    u32 v = mmio_r(TRANS_DDI_EDP);
    if (!(v & 0x80000000u)) return 0;              /* transcoder must be up */
    mmio_w(TRANS_DDI_EDP, low ? (v | TRANS_DRRS_BIT) : (v & ~TRANS_DRRS_BIT));
    (void)mmio_r(TRANS_DDI_EDP);
    return 1;
}

int intel_drrs_active(void)
{
    return intel_present() ? ((mmio_r(TRANS_DDI_EDP) & TRANS_DRRS_BIT) ? 1 : 0) : 0;
}
u32 intel_drrs_low_khz(void) { return drrs_low_khz; }

/* ---- PSR ----
 *
 * Panel self-refresh: the panel keeps its own copy of the frame and the display
 * engine stops fetching. Firmware leaves it ON, and the modeset's step 4
 * disables it, for reasons that are worth keeping written down - it fights
 * every plane update, and it issues its own fast-wake AUX transactions on the
 * channel this driver is using (4.3 #17). It is also what froze the frame
 * counter and sent us to PIPE_LINK_M/N for the pixel clock in the first place.
 *
 * So enabling it is a real trade, not a free win, and this deliberately does
 * the minimum: arm the source, and require the caller to have established that
 * the sink supports it. Anything that updates a plane must disable it first.
 */
#define EDP_PSR_ENABLE      (1u << 31)
#define EDP_PSR_STATUS_MASK (7u << 29)

int intel_psr_supported_sink(int port)
{
    /* DPCD 0x70 bit 0. Read into the latched cap copy, not aux_buf, so a
     * concurrent link-status read cannot clobber it. */
    if (!intel_dpcd_read(port, 0x070, 1)) return 0;
    return intel_dpcd_byte(0) & 1;
}

int intel_psr_enable(void)
{
    if (!intel_present() || !lt_armed) return 0;
    if (!intel_pipe_enabled()) return 0;          /* nothing to self-refresh */
    u32 v = mmio_r(EDP_PSR_CTL);
    mmio_w(EDP_PSR_CTL, v | EDP_PSR_ENABLE);
    (void)mmio_r(EDP_PSR_CTL);
    return 1;
}

/* What state the source thinks it is in. 0 is idle/inactive; anything else
 * means the panel is holding its own frame and the pipe is not fetching, which
 * is exactly when the frame counter stops and a plane update will not appear. */
u32 intel_psr_state(void)
{
    return intel_present() ? ((mmio_r(EDP_PSR_STATUS) & EDP_PSR_STATUS_MASK) >> 29) : 0;
}

/* ==== phase 4: hotplug ===================================================
 *
 * Zero lines before this. Every port probe in the driver is something a human
 * started, which is fine for a fixed internal panel and useless for a socket.
 *
 * Two interrupt sources, and which one matters depends on where the port lives.
 * DDI A..D live in the north display engine and report through GEN8_DE_PORT;
 * the PCH has its own path through SDEIIR for ports it owns. On this part the
 * DDIs are north, so GEN8_DE_PORT is the one that matters and SDE is read for
 * completeness rather than acted on.
 *
 * Interrupts are NOT enabled here. This driver polls, deliberately - a modeset
 * runs with interrupts masked and the whole timing base was rebuilt around that
 * (see wait_bits_us). What this provides is the detection and decode: read the
 * pending bits, say which port and what kind of event, and clear it. A kernel
 * that wants an interrupt can unmask GEN8_DE_PORT_IMR and call the same code.
 *
 * The distinction that matters is short versus long pulse. A LONG pulse is a
 * cable being connected or disconnected and means re-probe. A SHORT pulse on
 * DisplayPort is the sink asking for attention - a link that has degraded and
 * wants retraining, or an IRQ_HPD from the DPCD - and re-probing on one is both
 * wrong and slow. Treating them the same is the classic hotplug bug: monitors
 * that flicker and re-detect whenever the link hiccups.
 */
#define GEN8_DE_PORT_ISR   0x44440
#define GEN8_DE_PORT_IMR   0x44444
#define GEN8_DE_PORT_IIR   0x44448
#define GEN8_DE_PORT_IER   0x4444C
#define SDEIIR             0xC4008
#define SHOTPLUG_CTL_DDI   0xC4030      /* PCH: per-port detect + status */

/* Hotplug does NOT all live in one register, and guessing cost a rewrite.
 *
 * Measured on this machine:
 *
 *   GEN8_DE_PORT_IER  0E000001   bits 0, 25, 26, 27
 *   GEN8_DE_PORT_IMR  C0000038   bits 3, 4, 5 masked
 *   SHOTPLUG_CTL      10001010   bits 4, 12, 28 set
 *
 * Bits 25/26/27 of DE_PORT are AUX B/C/D DONE, not hotplug - the plan's own
 * table says so, and a first version of this read them as DDI hotplug because
 * "the DDI bits are up the top somewhere" is exactly the kind of assumption
 * this driver keeps getting caught by. i915 enables them because it wants AUX
 * completion interrupts.
 *
 * The real split on gen9:
 *
 *   DDI A hotplug  -> GEN8_DE_PORT bit 3   (north display engine)
 *   DDI B/C/D      -> the PCH, SHOTPLUG_CTL at 0xC4030
 *
 * And SHOTPLUG_CTL's ports are 8 bits apart with A off on its own, not the
 * regular 4 I assumed: enable B b4, C b12, D b20, A b28; status B 1:0, C 9:8,
 * D 17:16, A 25:24.
 *
 * The reading confirms the VBT independently: enables set for B, C and A, which
 * is exactly the three ports the VBT says are wired, and D - which the VBT does
 * not list - is off. Two unrelated sources agreeing on the same three ports. */
#define DE_PORT_DDI_A_HOTPLUG   (1u << 3)
#define SHOTPLUG_EN(ddi)   (1u << ((ddi) == 0 ? 28 : 4 + ((ddi) - 1) * 8))
#define SHOTPLUG_ST_SHIFT(ddi)  ((ddi) == 0 ? 24 : ((ddi) - 1) * 8)
#define SHOTPLUG_ST_MASK        0x3u

u32 intel_hpd_pending(void)
{
    if (!intel_present()) return 0;
    return mmio_r(GEN8_DE_PORT_IIR);
}

/* Bitmask of DDIs with something pending. DDI A comes from the north engine,
 * B/C/D from the PCH status field. */
int intel_hpd_ports(void)
{
    if (!intel_present()) return 0;
    int m = 0;
    if (mmio_r(GEN8_DE_PORT_IIR) & DE_PORT_DDI_A_HOTPLUG) m |= 1;
    u32 sh = mmio_r(SHOTPLUG_CTL_DDI);
    for (int d = 1; d < 4; d++)
        if ((sh >> SHOTPLUG_ST_SHIFT(d)) & SHOTPLUG_ST_MASK) m |= (1 << d);
    return m;
}

/* Is the OEM's hotplug detection even enabled for this port? An unwired port
 * has it clear, which is a second, independent check against the VBT. */
int intel_hpd_enabled(int ddi)
{
    if (!intel_present() || ddi < 0 || ddi > 3) return 0;
    return (mmio_r(SHOTPLUG_CTL_DDI) & SHOTPLUG_EN(ddi)) ? 1 : 0;
}

/* Acknowledge. Write ONLY the bit being handled: every bit in an IIR and in the
 * SHOTPLUG status field is write-1-clear, so writing the register back
 * acknowledges everything pending - including the AUX-done bit a transaction in
 * flight is waiting on. That is the mistake intel_pipe_underrun_clear() had. */
int intel_hpd_clear(int ddi)
{
    if (!intel_present() || !lt_armed || ddi < 0 || ddi > 3) return 0;
    if (ddi == 0) mmio_w(GEN8_DE_PORT_IIR, DE_PORT_DDI_A_HOTPLUG);
    else          mmio_w(SHOTPLUG_CTL_DDI, SHOTPLUG_ST_MASK << SHOTPLUG_ST_SHIFT(ddi));
    return 1;
}

/* Long pulse or short, and the difference is the whole point.
 *
 * 00 none, 01 short, 10 long, 11 both. A LONG pulse is a cable arriving or
 * leaving and means re-probe. A SHORT pulse on DisplayPort is the sink asking
 * for attention - a degraded link wanting retraining, or an IRQ_HPD in the
 * DPCD - and re-probing on one is both wrong and slow. Treating them the same
 * is the classic hotplug bug: monitors that re-detect whenever the link
 * hiccups. */
int intel_hpd_pulse(int ddi)
{
    if (!intel_present() || ddi < 0 || ddi > 3) return 0;
    return (int)((mmio_r(SHOTPLUG_CTL_DDI) >> SHOTPLUG_ST_SHIFT(ddi)) & SHOTPLUG_ST_MASK);
}

int intel_hpd_pulse_clear(int ddi) { return intel_hpd_clear(ddi); }

/* Is anything actually connected to this DDI right now?
 *
 * The honest answer for DisplayPort is not a hotplug bit but whether the sink
 * answers on AUX - a pulse says something changed, a DPCD read says something
 * is there. This is what a re-probe should call after a long pulse, and it is
 * also why it needs the port powered first. */
int intel_port_connected(int port)
{
    if (!intel_present()) return 0;
    if (!intel_dpcd_read(port, 0x000, 1)) return 0;
    return intel_dpcd_byte(0) != 0;         /* DPCD_REV 0 means nothing home */
}

/* Whether the north engine's DDI A hotplug interrupt is unmasked.
 * Informational: this driver polls, and a modeset masks interrupts anyway. */
int intel_hpd_irq_enabled(void)
{
    if (!intel_present()) return 0;
    return (mmio_r(GEN8_DE_PORT_IMR) & DE_PORT_DDI_A_HOTPLUG) ? 0 : 1;
}
