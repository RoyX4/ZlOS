/* i2c_hid.c - the touchpad.
 *
 * The trackpad on this laptop is not USB and not PS/2. Linux reports it as
 * `SYNA8006:00 06CB:CD8B Touchpad` on `i2c-2`, which is the Synopsys
 * DesignWare I2C controller Intel puts at PCI 00:15.1 - so reaching it means
 * two drivers stacked: a controller driver that can move bytes on an I2C bus,
 * and the HID-over-I2C protocol on top.
 *
 * (The TrackPoint, by contrast, IS PS/2 - `isa0060/serio1` - and the existing
 * IRQ12 mouse driver already handles it. This is specifically the touchpad.)
 *
 * HOW THIS DIFFERS FROM EVERY OTHER DRIVER HERE - READ THIS FIRST
 * --------------------------------------------------------------
 * QEMU does not emulate Intel's LPSS I2C controller. There is no way to test a
 * single line of this on this machine short of booting the laptop, exactly like
 * intel.c. It is written from the DesignWare I2C databook and the HID-over-I2C
 * specification, and it is honest about being unproven.
 *
 * The other thing worth knowing: normally the I2C slave address and the HID
 * descriptor register come from an ACPI _DSM method, which needs an AML
 * interpreter - tens of thousands of lines, and not happening here. So instead
 * this PROBES: it walks the valid 7-bit address range asking each device for a
 * HID descriptor, and a device that answers with the right length and version
 * is the one. That is slower than reading a table, but it needs no interpreter
 * and it cannot be wrong about a machine it was not written for.
 */

#include "memmap.h"

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

#if defined(ZL_64)
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

int  pci_count(void);
int  pci_class(int i);
int  pci_subclass(int i);
int  pci_vendor(int i);
int  pci_device(int i);
void pci_scan(void);
void pci_enable(int i);
u32  pci_bar(int i, int which);
u32  pci_bar_hi(int i, int which);
u32  idt_ticks(void);

static u32  rd32(uptr a)         { return *(volatile u32 *)a; }
static void wr32(uptr a, u32 v)  { *(volatile u32 *)a = v; }

/* ---- DesignWare I2C registers, offsets from BAR0 ----------------------- */
#define IC_CON            0x00
#define IC_TAR            0x04
#define IC_DATA_CMD       0x10
#define IC_SS_SCL_HCNT    0x14
#define IC_SS_SCL_LCNT    0x18
#define IC_FS_SCL_HCNT    0x1C
#define IC_FS_SCL_LCNT    0x20
#define IC_INTR_MASK      0x30
#define IC_RAW_INTR_STAT  0x34
#define IC_RX_TL          0x38
#define IC_TX_TL          0x3C
#define IC_CLR_INTR       0x40
#define IC_CLR_TX_ABRT    0x54
#define IC_ENABLE         0x6C
#define IC_STATUS         0x70
#define IC_TXFLR          0x74
#define IC_RXFLR          0x78
#define IC_TX_ABRT_SOURCE 0x80
#define IC_ENABLE_STATUS  0x9C
#define IC_COMP_TYPE      0xFC

#define IC_COMP_TYPE_VALUE 0x44570140u   /* "DW" + version - the ID check */

#define CON_MASTER      (1u << 0)
#define CON_SPEED_FAST  (2u << 1)
#define CON_SLAVE_DIS   (1u << 6)
#define CON_RESTART_EN  (1u << 5)

#define STATUS_TFE      (1u << 2)    /* transmit FIFO empty  */
#define STATUS_RFNE     (1u << 3)    /* receive FIFO not empty */
#define STATUS_ACTIVITY (1u << 0)

#define CMD_READ        (1u << 8)
#define CMD_STOP        (1u << 9)
#define CMD_RESTART     (1u << 10)

/* ---- state ------------------------------------------------------------- */
static uptr i2c_base   = 0;
static int  i2c_idx    = -1;
static int  i2c_found  = 0;
static int  hid_addr   = -1;      /* the touchpad's 7-bit I2C address       */
static int  hid_desc_reg = -1;    /* which register held its HID descriptor */
static int  hid_ready  = 0;

/* HID descriptor fields we care about, as read off the device */
static u16 hid_input_reg = 0, hid_max_input = 0;
static u16 hid_cmd_reg = 0, hid_data_reg = 0;
static u16 hid_vid = 0, hid_pid = 0, hid_version = 0;
static u16 hid_report_desc_len = 0;

/* ---- where the buffers live -------------------------------------------
 * These were at 0x0C900000 and 0x0C900100, which is 9 MiB into fb.c's
 * 16 MiB cached-blur arena - and before the compositor moved things, inside
 * the span the framebuffer back buffer occupied at 2560x1440. Two buffers,
 * one address range, neither file aware of the other: the DMA-arena collision
 * HANDOFF.md had already counted five times before this one.
 *
 * It had never been seen because it could not be. QEMU has no Intel LPSS I2C
 * controller, so this driver only runs on the laptop, which is also the only
 * machine with the panel big enough to make the back buffer reach. The two
 * halves of the bug were never on the same machine as a working test.
 *
 * HI_HID is now this driver's own region in memmap.h - 8 MiB for the 320 bytes
 * below, which is what "clear of everything else" costs when there is no
 * allocator. The asserts are the point, not the address. */
#define HID_BUF     ((unsigned int)HI_HID)          /* input reports land here */
#define HID_BUF_MAX 64u

/* ---- finding the controller -------------------------------------------
 * Intel's LPSS I2C blocks appear as PCI class 0x0C (serial bus) subclass 0x80
 * (other). There are usually two; the right one is whichever has a DesignWare
 * core behind it AND a device that answers as HID. */
int i2c_find(int which)
{
    pci_scan();
    int seen = 0;
    for (int i = 0; i < pci_count(); i++) {
        if (pci_vendor(i)   != 0x8086) continue;
        if (pci_class(i)    != 0x0C)   continue;
        if (pci_subclass(i) != 0x80)   continue;
        if (seen++ != which) continue;

        pci_enable(i);
        u32 lo = pci_bar(i, 0);
        u32 hi = pci_bar_hi(i, 0);
        if (hi && sizeof(uptr) < 8) return -1;
        /* `<< 16 << 16`, not `<< 32` - see virtio_net.c's note. This was the
         * last of the four copies of this combine still shifting by the full
         * width; on the 32-bit build that is UB and gcc says so on every run. */
        uptr b = ((uptr)hi << 16 << 16) | (uptr)lo;
        if (!b) return -1;

        i2c_idx  = i;
        i2c_base = b;
        i2c_found = 1;
        return i;
    }
    return -1;
}

u32 i2c_mmio(void)      { return (u32)i2c_base; }
u32 i2c_comp_type(void) { return i2c_found ? rd32(i2c_base + IC_COMP_TYPE) : 0; }
int i2c_present(void)   { return i2c_found; }

/* Is there actually a DesignWare core here? The component type register is a
 * fixed magic value, and checking it is the difference between driving a real
 * controller and writing into whatever else happens to be mapped. */
int i2c_is_designware(void)
{
    if (!i2c_found) return 0;
    return rd32(i2c_base + IC_COMP_TYPE) == IC_COMP_TYPE_VALUE;
}

static void i2c_disable(void)
{
    wr32(i2c_base + IC_ENABLE, 0);
    for (int i = 0; i < 100000; i++)
        if (!(rd32(i2c_base + IC_ENABLE_STATUS) & 1)) return;
}

/* Set the controller up as a fast-mode master. The SCL counts decide the bus
 * clock; these are the values Linux uses for a 133 MHz LPSS source at 400 kHz,
 * and they are conservative enough to work if the source differs somewhat. */
int i2c_init(void)
{
    if (!i2c_found && i2c_find(1) < 0 && i2c_find(0) < 0) return 0;
    if (!i2c_is_designware()) return 0;

    i2c_disable();
    wr32(i2c_base + IC_CON, CON_MASTER | CON_SLAVE_DIS | CON_RESTART_EN | CON_SPEED_FAST);
    wr32(i2c_base + IC_FS_SCL_HCNT, 0x3C);
    wr32(i2c_base + IC_FS_SCL_LCNT, 0x82);
    wr32(i2c_base + IC_TX_TL, 0);
    wr32(i2c_base + IC_RX_TL, 0);
    wr32(i2c_base + IC_INTR_MASK, 0);        /* we poll; no interrupts */
    return 1;
}

static void i2c_set_target(int addr)
{
    i2c_disable();
    wr32(i2c_base + IC_TAR, (u32)addr & 0x7F);
    wr32(i2c_base + IC_ENABLE, 1);
}

static int wait_tx_room(void)
{
    for (int i = 0; i < 2000000; i++) {
        if (rd32(i2c_base + IC_RAW_INTR_STAT) & (1u << 6)) return 0;  /* TX_ABRT */
        if (rd32(i2c_base + IC_STATUS) & STATUS_TFE) return 1;
    }
    return 0;
}

static int wait_rx_byte(void)
{
    for (int i = 0; i < 2000000; i++) {
        if (rd32(i2c_base + IC_RAW_INTR_STAT) & (1u << 6)) return 0;  /* TX_ABRT */
        if (rd32(i2c_base + IC_STATUS) & STATUS_RFNE) return 1;
    }
    return 0;
}

static void clear_abort(void)
{
    (void)rd32(i2c_base + IC_CLR_TX_ABRT);
}

/* Write some bytes, then read some back with a repeated start - which is the
 * only transaction shape HID-over-I2C ever needs. */
static int i2c_write_read(int addr, const u8 *out, int nout, u32 inbuf, int nin)
{
    if (!i2c_found) return 0;
    clear_abort();
    i2c_set_target(addr);

    for (int i = 0; i < nout; i++) {
        if (!wait_tx_room()) { clear_abort(); return 0; }
        u32 cmd = out[i];
        if (nin == 0 && i == nout - 1) cmd |= CMD_STOP;
        wr32(i2c_base + IC_DATA_CMD, cmd);
    }

    for (int i = 0; i < nin; i++) {
        if (!wait_tx_room()) { clear_abort(); return 0; }
        u32 cmd = CMD_READ;
        if (i == 0)        cmd |= CMD_RESTART;
        if (i == nin - 1)  cmd |= CMD_STOP;
        wr32(i2c_base + IC_DATA_CMD, cmd);

        if (!wait_rx_byte()) { clear_abort(); return 0; }
        *(volatile u8 *)(uptr)(inbuf + (u32)i) = (u8)(rd32(i2c_base + IC_DATA_CMD) & 0xFF);
    }

    if (rd32(i2c_base + IC_RAW_INTR_STAT) & (1u << 6)) { clear_abort(); return 0; }
    return 1;
}

/* ---- HID over I2C ------------------------------------------------------
 * The HID descriptor is 30 bytes and self-describing: its first field is its
 * own length and its second is the protocol version, so a device that answers
 * with 30 and 0x0100 is a HID device and everything else is noise. That is
 * what makes probing safe. */
#define HID_DESC_BUF (HID_BUF + 0x100u)
#define HID_DESC_LEN 30u

/* Both buffers inside this driver's own region, and the report buffer clear of
 * the descriptor buffer that follows it. HID_BUF_MAX is 64 and the gap is 256,
 * but the gap is where a future max_input larger than 64 would land. */
_Static_assert(HID_BUF == (unsigned int)HI_HID,
               "i2c_hid: report buffer is not at the base of its region");
_Static_assert(HID_BUF + HID_BUF_MAX <= HID_DESC_BUF,
               "i2c_hid: report buffer overruns the descriptor buffer");
_Static_assert((unsigned long)HID_DESC_BUF + HID_DESC_LEN <= HI_GPU,
               "i2c_hid: buffers escape their region into the blur arena");

static int read_hid_descriptor(int addr, int reg)
{
    u8 out[2] = { (u8)(reg & 0xFF), (u8)((reg >> 8) & 0xFF) };
    if (!i2c_write_read(addr, out, 2, HID_DESC_BUF, (int)HID_DESC_LEN)) return 0;

    volatile u8 *d = (volatile u8 *)(uptr)HID_DESC_BUF;
    u16 len = (u16)(d[0] | (d[1] << 8));
    u16 ver = (u16)(d[2] | (d[3] << 8));
    if (len != HID_DESC_LEN) return 0;
    if (ver != 0x0100) return 0;

    hid_report_desc_len = (u16)(d[4]  | (d[5]  << 8));
    hid_input_reg       = (u16)(d[8]  | (d[9]  << 8));
    hid_max_input       = (u16)(d[10] | (d[11] << 8));
    hid_cmd_reg         = (u16)(d[16] | (d[17] << 8));
    hid_data_reg        = (u16)(d[18] | (d[19] << 8));
    hid_vid             = (u16)(d[20] | (d[21] << 8));
    hid_pid             = (u16)(d[22] | (d[23] << 8));
    hid_version         = (u16)(d[24] | (d[25] << 8));
    return 1;
}

/* Walk the bus. Addresses below 0x08 and above 0x77 are reserved by the I2C
 * specification and are never devices. The two candidate descriptor registers
 * cover essentially every shipping I2C-HID device. */
int i2c_hid_probe(void)
{
    if (hid_ready) return hid_addr;
    if (!i2c_init()) return -1;

    static const int regs[2] = { 0x0020, 0x0001 };
    for (int a = 0x08; a <= 0x77; a++) {
        for (int r = 0; r < 2; r++) {
            if (!read_hid_descriptor(a, regs[r])) continue;
            hid_addr     = a;
            hid_desc_reg = regs[r];
            hid_ready    = 1;
            return a;
        }
    }
    return -1;
}

/* Read one input report. The first two bytes are the report's own length, so a
 * length of zero means "nothing happened" rather than an error - which is the
 * normal case when the pad is not being touched. */
int i2c_hid_read_report(void)
{
    if (!hid_ready) return 0;
    u8 out[2] = { (u8)(hid_input_reg & 0xFF), (u8)((hid_input_reg >> 8) & 0xFF) };
    int want = (int)hid_max_input;
    if (want <= 0 || want > (int)HID_BUF_MAX) want = (int)HID_BUF_MAX;

    for (u32 i = 0; i < HID_BUF_MAX; i++) *(volatile u8 *)(uptr)(HID_BUF + i) = 0;
    if (!i2c_write_read(hid_addr, out, 2, HID_BUF, want)) return 0;

    volatile u8 *b = (volatile u8 *)(uptr)HID_BUF;
    return (int)(b[0] | (b[1] << 8));
}

int i2c_hid_byte(int i)
{
    if (i < 0 || i >= (int)HID_BUF_MAX) return 0;
    return (int)*(volatile u8 *)(uptr)(HID_BUF + (u32)i);
}

int i2c_hid_ready(void)     { return hid_ready; }
int i2c_hid_address(void)   { return hid_addr; }
int i2c_hid_desc_reg(void)  { return hid_desc_reg; }
int i2c_hid_vid(void)       { return (int)hid_vid; }
int i2c_hid_pid(void)       { return (int)hid_pid; }
int i2c_hid_version(void)   { return (int)hid_version; }
int i2c_hid_input_reg(void) { return (int)hid_input_reg; }
int i2c_hid_max_input(void) { return (int)hid_max_input; }
int i2c_hid_rdesc_len(void) { return (int)hid_report_desc_len; }
