/* pci.c - finding the hardware.
 *
 * Everything up to now talked to devices at addresses somebody else told us
 * about: the BIOS handed over a framebuffer, the keyboard and timer live at
 * fixed legacy ports that have not moved since 1981. A graphics card is not
 * like that. It sits on the PCI bus, its registers and its video memory are
 * wherever the firmware decided to map them, and the only way to find any of
 * it is to ask the bus.
 *
 * So this is the first real bus driver in zlOS, and it is the prerequisite for
 * every display and GPU driver that follows: you cannot program a card you
 * cannot find.
 *
 * The mechanism is "configuration mechanism #1": write a target address to
 * port 0xCF8, then read or write the 32-bit register through port 0xCFC.
 * Every PCI function has 256 bytes of configuration space laid out by the
 * spec - vendor and device ID at 0x00, class code at 0x08, and the Base
 * Address Registers (where the device's memory actually lives) from 0x10.
 */

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

void zl_outl(u16 port, u32 val);
u32  zl_inl(u16 port);
void zl_outb(u16 port, u8 val);
u8   zl_inb(u16 port);

#define PCI_ADDR 0xCF8
#define PCI_DATA 0xCFC

/* config space offsets we care about */
#define PCI_VENDOR_ID   0x00
#define PCI_DEVICE_ID   0x02
#define PCI_COMMAND     0x04
#define PCI_CLASS_REV   0x08
#define PCI_HEADER_TYPE 0x0E
#define PCI_BAR0        0x10

/* The address is one 32-bit word: bit 31 enables the cycle, then bus,
 * device and function are packed in, and the register offset is aligned to
 * four bytes because the data port is always 32 bits wide. */
static u32 pci_addr(int bus, int dev, int fn, int off)
{
    return 0x80000000u
         | ((u32)bus << 16)
         | ((u32)(dev & 0x1F) << 11)
         | ((u32)(fn  & 0x07) << 8)
         | ((u32)off & 0xFC);
}

u32 pci_read32(int bus, int dev, int fn, int off)
{
    zl_outl(PCI_ADDR, pci_addr(bus, dev, fn, off));
    return zl_inl(PCI_DATA);
}

void pci_write32(int bus, int dev, int fn, int off, u32 val)
{
    zl_outl(PCI_ADDR, pci_addr(bus, dev, fn, off));
    zl_outl(PCI_DATA, val);
}

/* the data port is 32-bit, so a narrower read is a shift out of the word */
u16 pci_read16(int bus, int dev, int fn, int off)
{
    return (u16)((pci_read32(bus, dev, fn, off) >> ((off & 2) * 8)) & 0xFFFF);
}

u8 pci_read8(int bus, int dev, int fn, int off)
{
    return (u8)((pci_read32(bus, dev, fn, off) >> ((off & 3) * 8)) & 0xFF);
}

/* ---- what the scan found ------------------------------------------------
 * A kernel with no heap cannot build a list, so the results live in a fixed
 * table. 32 functions is far more than a virtual machine or a laptop
 * presents on bus 0, and overflowing it just stops recording rather than
 * corrupting anything. */
#define PCI_MAX 32
static struct {
    u8  bus, dev, fn;
    u16 vendor, device;
    u8  class_id, subclass, prog_if;
} found[PCI_MAX];
static int found_n = 0;

int pci_count(void)            { return found_n; }
int pci_vendor(int i)          { return (i < found_n) ? found[i].vendor : 0; }
int pci_device(int i)          { return (i < found_n) ? found[i].device : 0; }
int pci_class(int i)           { return (i < found_n) ? found[i].class_id : 0; }
int pci_subclass(int i)        { return (i < found_n) ? found[i].subclass : 0; }

static void record(int bus, int dev, int fn, u32 id, u32 cls)
{
    if (found_n >= PCI_MAX) return;
    found[found_n].bus      = (u8)bus;
    found[found_n].dev      = (u8)dev;
    found[found_n].fn       = (u8)fn;
    found[found_n].vendor   = (u16)(id & 0xFFFF);
    found[found_n].device   = (u16)(id >> 16);
    found[found_n].prog_if  = (u8)((cls >> 8)  & 0xFF);
    found[found_n].subclass = (u8)((cls >> 16) & 0xFF);
    found[found_n].class_id = (u8)((cls >> 24) & 0xFF);
    found_n++;
}

/* Brute-force scan. A recursive bridge walk is tidier, but every device we
 * care about - the display adapter and any virtio device - lives on bus 0 in
 * both QEMU and a laptop, and a flat scan of the first few buses cannot get
 * lost. Vendor 0xFFFF means "nothing is plugged in here". */
void pci_scan(void)
{
    found_n = 0;
    for (int bus = 0; bus < 4; bus++)
        for (int dev = 0; dev < 32; dev++) {
            u32 id = pci_read32(bus, dev, 0, PCI_VENDOR_ID);
            if ((id & 0xFFFF) == 0xFFFF) continue;          /* empty slot */
            record(bus, dev, 0, id, pci_read32(bus, dev, 0, PCI_CLASS_REV));

            /* bit 7 of the header type says the device has more functions */
            u8 hdr = pci_read8(bus, dev, 0, PCI_HEADER_TYPE);
            if (!(hdr & 0x80)) continue;
            for (int fn = 1; fn < 8; fn++) {
                u32 fid = pci_read32(bus, dev, fn, PCI_VENDOR_ID);
                if ((fid & 0xFFFF) == 0xFFFF) continue;
                record(bus, dev, fn, fid, pci_read32(bus, dev, fn, PCI_CLASS_REV));
            }
        }
}

/* find the index of the first device matching a vendor/device pair, or -1 */
int pci_find(int vendor, int device)
{
    for (int i = 0; i < found_n; i++)
        if (found[i].vendor == vendor && found[i].device == device) return i;
    return -1;
}

/* find the first device of a class/subclass - class 0x03 is "display
 * controller", subclass 0x00 is a VGA-compatible one */
int pci_find_class(int cls, int sub)
{
    for (int i = 0; i < found_n; i++)
        if (found[i].class_id == cls && found[i].subclass == sub) return i;
    return -1;
}

/* Read a Base Address Register and hand back the actual address. Bit 0 says
 * memory (0) or IO (1) space; for memory the low 4 bits are flags and the
 * address is the rest. This is how we learn where a card's framebuffer and
 * register block really are - the numbers are assigned by firmware at boot
 * and are not knowable any other way. */
u32 pci_bar(int i, int which)
{
    if (i < 0 || i >= found_n || which < 0 || which > 5) return 0;
    u32 v = pci_read32(found[i].bus, found[i].dev, found[i].fn, PCI_BAR0 + which * 4);
    if (v & 1) return v & 0xFFFFFFFC;        /* IO space  */
    return v & 0xFFFFFFF0;                    /* memory space */
}

/* ---- 64-bit BARs -------------------------------------------------------
 * Bits [2:1] of a memory BAR are the type: 0 = 32-bit, 2 = 64-bit. A 64-bit
 * BAR consumes the NEXT slot as well, and that slot holds the upper 32 bits of
 * the address.
 *
 * This is not a theoretical case. UEFI firmware routinely puts 64-bit BARs
 * high: OVMF maps QEMU's xHCI at 0xC000000000, and the Intel PCH xHCI on real
 * hardware is a 64-bit BAR too. Reading only the low dword there yields zero,
 * and the device looks like it does not exist - which is exactly the bug this
 * was written to fix. */
int pci_bar_is64(int i, int which)
{
    if (i < 0 || i >= found_n || which < 0 || which > 4) return 0;
    u32 v = pci_read32(found[i].bus, found[i].dev, found[i].fn, PCI_BAR0 + which * 4);
    if (v & 1) return 0;                      /* IO BARs are never 64-bit */
    return ((v >> 1) & 3) == 2;
}

u32 pci_bar_hi(int i, int which)
{
    if (!pci_bar_is64(i, which)) return 0;
    return pci_read32(found[i].bus, found[i].dev, found[i].fn,
                      PCI_BAR0 + (which + 1) * 4);
}

/* the programming interface byte - for class 0x0C subclass 0x03 this is what
 * separates xHCI (0x30) from EHCI (0x20), OHCI (0x10) and UHCI (0x00) */
int pci_prog_if(int i) { return (i < found_n) ? found[i].prog_if : 0; }

/* Where a device sits on the bus. Anything that has to reach config space
 * directly - walking a capability list, say - needs these rather than the
 * cached summary above. */
int pci_bus_of(int i) { return (i >= 0 && i < found_n) ? found[i].bus : -1; }
int pci_dev_of(int i) { return (i >= 0 && i < found_n) ? found[i].dev : -1; }
int pci_fn_of (int i) { return (i >= 0 && i < found_n) ? found[i].fn  : -1; }

/* Size a BAR the way the spec says: write all ones, read back, and the
 * hardware returns zeros in the bits it does not decode. The size is the
 * complement of the masked value, plus one. The original must be restored. */
u32 pci_bar_size(int i, int which)
{
    if (i < 0 || i >= found_n || which < 0 || which > 5) return 0;
    int b = found[i].bus, d = found[i].dev, f = found[i].fn;
    int off = PCI_BAR0 + which * 4;

    /* DECODING MUST BE OFF while the BAR holds all-ones. For that moment the
     * device claims an enormous window, and if it is still decoding it can
     * shadow real RAM or another device's registers - Linux does exactly this
     * dance in pci_read_bases() and warns not to even print while it is off. */
    u32 cmd = pci_read32(b, d, f, PCI_COMMAND);
    pci_write32(b, d, f, PCI_COMMAND, cmd & ~0x03u);   /* clear IO + memory */

    u32 orig = pci_read32(b, d, f, off);
    pci_write32(b, d, f, off, 0xFFFFFFFFu);
    u32 mask = pci_read32(b, d, f, off);
    pci_write32(b, d, f, off, orig);                    /* firmware's assignment back */

    pci_write32(b, d, f, PCI_COMMAND, cmd);             /* and decoding back on */

    if (!mask) return 0;
    if (orig & 1) mask &= 0xFFFFFFFC; else mask &= 0xFFFFFFF0;
    if (!mask) return 0;
    /* the size is the lowest set bit of the mask - devices may hardwire upper
     * bits to zero, which breaks the naive ~mask+1 form */
    return mask & (~mask + 1);
}

/* Let the device drive memory and act as a bus master. Nothing DMAs until
 * bus mastering is on, so any real GPU driver needs this. */
void pci_enable(int i)
{
    if (i < 0 || i >= found_n) return;
    int b = found[i].bus, d = found[i].dev, f = found[i].fn;
    u32 cmd = pci_read32(b, d, f, PCI_COMMAND);
    cmd |= 0x07;                    /* IO space | memory space | bus master */
    pci_write32(b, d, f, PCI_COMMAND, cmd);
}
