/* bga.c - OUR OWN display driver. No BIOS, no VBE, no firmware.
 *
 * Until now every resolution zlOS ever ran at was chosen by somebody else:
 * raw_boot.asm asks the video BIOS through INT 0x10, and whatever the card's
 * option ROM offers is what we get, fixed for the life of the boot. That is
 * the difference the research kept pointing at - real operating systems do
 * MODESETTING: they program the display hardware directly, so resolution is
 * something the OS decides at run time, not something it inherits.
 *
 * This is that driver. The Bochs Graphics Adapter interface (which QEMU's
 * stdvga, Bochs and VirtualBox all implement) exposes the display controller
 * through two IO ports: write a register index to 0x01CE, then read or write
 * its 16-bit value at 0x01CF. Set width, height and depth, flip the enable
 * bit, and the card reprograms itself. No real-mode thunk, no BIOS call -
 * it works from protected mode because it is just port IO.
 *
 * The framebuffer address is not guessed either: it comes from the card's
 * PCI BAR0, which is why pci.c had to exist first.
 */

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

void zl_outw(u16 port, u16 val);
u16  zl_inw(u16 port);

int  pci_find(int vendor, int device);
int  pci_find_class(int cls, int sub);
u32  pci_bar(int i, int which);
u32  pci_bar_size(int i, int which);
void pci_enable(int i);
void pci_scan(void);

/* the index/data port pair */
#define VBE_INDEX 0x01CE
#define VBE_DATA  0x01CF

/* register indices */
#define VBE_ID          0
#define VBE_XRES        1
#define VBE_YRES        2
#define VBE_BPP         3
#define VBE_ENABLE      4
#define VBE_BANK        5
#define VBE_VIRT_WIDTH  6
#define VBE_VIRT_HEIGHT 7
#define VBE_X_OFFSET    8
#define VBE_Y_OFFSET    9

/* ENABLE register bits */
#define VBE_DISABLED    0x00
#define VBE_ENABLED     0x01
#define VBE_LFB_ENABLED 0x40
#define VBE_NOCLEARMEM  0x80

/* the adapter reports a version here; 0xB0C0..0xB0C5 are the known ones */
#define VBE_ID_MIN 0xB0C0
#define VBE_ID_MAX 0xB0C5

static void bga_write(u16 index, u16 value)
{
    zl_outw(VBE_INDEX, index);
    zl_outw(VBE_DATA,  value);
}

static u16 bga_read(u16 index)
{
    zl_outw(VBE_INDEX, index);
    return zl_inw(VBE_DATA);
}

/* Is a BGA-compatible controller actually here? Reading the ID register is
 * the documented probe - a card that does not implement this interface will
 * not answer with a version in range, and we must not start writing mode
 * registers into hardware that means something else by them. */
int bga_present(void)
{
    u16 id = bga_read(VBE_ID);
    return (id >= VBE_ID_MIN && id <= VBE_ID_MAX);
}

int bga_version(void) { return (int)bga_read(VBE_ID); }

/* Where the card's video memory is mapped, straight from PCI BAR0, and how
 * much of it there is. Nothing here is a constant we made up. */
static int   bga_pci_index = -1;
static u32   bga_lfb = 0;
static u32   bga_vram = 0;

int bga_find(void)
{
    pci_scan();
    /* QEMU stdvga and Bochs both present 1234:1111; VirtualBox is 80EE:BEEF.
     * Falling back to "any VGA-compatible display controller" (class 0x03,
     * subclass 0x00) covers the rest. */
    int i = pci_find(0x1234, 0x1111);
    if (i < 0) i = pci_find(0x80EE, 0xBEEF);
    if (i < 0) i = pci_find_class(0x03, 0x00);
    if (i < 0) return -1;

    pci_enable(i);
    bga_pci_index = i;
    bga_lfb  = pci_bar(i, 0);
    bga_vram = pci_bar_size(i, 0);
    return i;
}

u32 bga_framebuffer(void)
{
    /* probe the bus on first use - a caller asking where the framebuffer is
     * should not have to know that pci enumeration happens first */
    if (!bga_lfb) bga_find();
    return bga_lfb;
}
u32 bga_vram_bytes(void)  { return bga_vram; }

/* Would this mode fit in the card's video memory? Asking for more than the
 * hardware has is the classic way to get a corrupted or black screen, and
 * the card will not refuse on our behalf. */
int bga_mode_fits(int w, int h, int bpp)
{
    if (!bga_vram) return 1;                 /* size unknown - let it try */
    u32 need = (u32)w * (u32)h * (u32)(bpp / 8);
    return need <= bga_vram;
}

/* SET THE MODE. This is the whole point of the file: after this call the
 * display controller is running at a resolution the operating system chose.
 *
 * The sequence is fixed by the interface: the mode registers may only be
 * written while the extension is disabled, then enabling it with the linear
 * framebuffer bit makes the card reprogram its timings and expose the whole
 * screen as flat memory at BAR0. */
int bga_set_mode(int w, int h, int bpp)
{
    if (!bga_present()) return 0;
    if (bpp != 32 && bpp != 24 && bpp != 16) return 0;
    if (!bga_mode_fits(w, h, bpp)) return 0;

    bga_write(VBE_ENABLE, VBE_DISABLED);
    bga_write(VBE_XRES, (u16)w);
    bga_write(VBE_YRES, (u16)h);
    bga_write(VBE_BPP,  (u16)bpp);
    /* virtual size == visible size: no panning, so the pitch is exactly the
     * width and the framebuffer has no hidden padding */
    bga_write(VBE_VIRT_WIDTH,  (u16)w);
    bga_write(VBE_VIRT_HEIGHT, (u16)h);
    bga_write(VBE_X_OFFSET, 0);
    bga_write(VBE_Y_OFFSET, 0);
    bga_write(VBE_ENABLE, VBE_ENABLED | VBE_LFB_ENABLED);

    /* trust but verify - read the registers back and make sure the card
     * actually took the mode rather than silently clamping it */
    if (bga_read(VBE_XRES) != (u16)w) return 0;
    if (bga_read(VBE_YRES) != (u16)h) return 0;
    if (bga_read(VBE_BPP)  != (u16)bpp) return 0;
    return 1;
}

/* The scanline stride the CARD is actually using. Never assume it is
 * width*bpp/8: the controller is free to keep a wider virtual surface, and
 * guessing wrong shears or doubles the whole screen. VIRT_WIDTH is the
 * virtual width in pixels, so the pitch falls out of it. */
int bga_get_pitch(void)
{
    int vw = (int)bga_read(VBE_VIRT_WIDTH);
    int bpp = (int)bga_read(VBE_BPP);
    if (vw <= 0) vw = (int)bga_read(VBE_XRES);
    return vw * (bpp / 8);
}

int bga_get_width(void)  { return (int)bga_read(VBE_XRES); }
int bga_get_height(void) { return (int)bga_read(VBE_YRES); }
int bga_get_bpp(void)    { return (int)bga_read(VBE_BPP); }

/* raw register peek, for diagnosing what the card really answers */
int bga_reg(int idx) { return (int)bga_read((u16)idx); }
