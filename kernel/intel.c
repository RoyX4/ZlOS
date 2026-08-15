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

#define PLANE_CTL_ENABLE  0x80000000u
#define PLANE_CTL_FORMAT_MASK 0x0F000000u

static int  gpu_idx = -1;
static u32  mmio    = 0;      /* BAR0 - registers and the GGTT window */
static u32  aperture = 0;     /* BAR2 - the CPU-visible window        */
static u32  mmio_size = 0;
static u32  aper_size = 0;
static u16  gpu_devid = 0;

/* MMIO is plain memory to us: flat 32-bit segments, no paging, so the BAR
 * address is directly addressable. `volatile` matters - these are registers,
 * not variables, and the compiler must not cache or reorder them. */
static u32 mmio_r(u32 off)
{
    if (!mmio) return 0;
    return *(volatile u32 *)(mmio + off);
}

static void mmio_w(u32 off, u32 val)
{
    if (!mmio) return;
    *(volatile u32 *)(mmio + off) = val;
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
        mmio      = pci_bar(i, 0);                /* GTTMMADR */
        mmio_size = pci_bar_size(i, 0);
        aperture  = pci_bar(i, 2);                /* GMADR    */
        aper_size = pci_bar_size(i, 2);
        return i;
    }
    gpu_idx = -1;
    return -1;
}

int intel_present(void)   { return gpu_idx >= 0 && mmio != 0; }
int intel_devid(void)     { return gpu_devid; }
int intel_supported(void) { return intel_present() && is_gen9(gpu_devid); }
u32 intel_mmio(void)      { return mmio; }
u32 intel_mmio_size(void) { return mmio_size; }
u32 intel_aperture(void)  { return aperture; }
u32 intel_aper_size(void) { return aper_size; }

/* ---- stolen memory: the RAM the firmware reserved for graphics ----------
 * GMS is bits 15:8 of MGGC0. On Gen9 the size is 32 MiB per step below 0xF0,
 * then 4 MiB steps from 4..60 MiB - the encoding i915's early-quirks.c uses.
 * The base is BDSM masked to a 1 MiB boundary; the low bits are lock flags. */
u32 intel_stolen_base(void)
{
    if (gpu_idx < 0) return 0;
    return pci_read32(0, 2, 0, BDSM) & 0xFFF00000u;
}

u32 intel_stolen_size(void)
{
    if (gpu_idx < 0) return 0;
    u32 ggc = pci_read32(0, 2, 0, MGGC0);
    u32 gms = (ggc >> 8) & 0xFF;
    if (gms < 0xF0) return gms * (32u << 20);
    return (gms - 0xF0) * (4u << 20) + (4u << 20);
}

/* GGTT size from GGMS (bits 7:6): 0, 2, 4 or 8 MiB of page-table entries.
 * Each entry maps 4 KiB, so 8 MiB of PTEs covers the full 4 GiB address space. */
u32 intel_ggtt_size(void)
{
    if (gpu_idx < 0) return 0;
    u32 ggc  = pci_read32(0, 2, 0, MGGC0);
    u32 ggms = (ggc >> 6) & 0x3;
    if (!ggms) return 0;
    return (1u << 20) << ggms;         /* 2, 4 or 8 MiB */
}

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

/* stride is held in units of 64 bytes for a linear surface */
int intel_stride(void)       { return (int)(mmio_r(PLANE_STRIDE_1_A) & 0x3FF) * 64; }
int intel_plane_enabled(void){ return (mmio_r(PLANE_CTL_1_A) & PLANE_CTL_ENABLE) ? 1 : 0; }
u32 intel_plane_ctl(void)    { return mmio_r(PLANE_CTL_1_A); }

/* the graphics address the display is scanning out of, right now */
u32 intel_surface(void)      { return mmio_r(PLANE_SURFLIVE_A); }
int intel_frame_count(void)  { return (int)mmio_r(PIPE_FRMCNT_A); }

/* is the transcoder actually running? bit 31 of TRANS_CONF is the enable */
int intel_pipe_enabled(void) { return (mmio_r(TRANS_CONF_A) & 0x80000000u) ? 1 : 0; }

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

    volatile u32 *pte = (volatile u32 *)(mmio + GGTT_OFFSET + gfx_page * 8u);
    pte[0] = (phys_addr & 0xFFFFF000u) | 1u;  /* address | present */
    pte[1] = 0;                                /* HAW=39 on a client part */
    return 1;
}
