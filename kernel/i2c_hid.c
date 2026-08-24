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
#include "i2c_touch.h"
#include "zllog.h"

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

/* Intel CML-LP LPSS wrapper.  The DesignWare block occupies BAR+0x000..1ff;
 * the wrapper private registers start at +0x200.  Linux performs this reset
 * and remap setup before it exposes the DesignWare child.  Reading COMP_TYPE
 * before it leaves reset only proves that an asleep wrapper returns rubbish. */
#define LPSS_PRIV_OFFSET       0x200u
#define LPSS_PRIV_RESETS       (LPSS_PRIV_OFFSET + 0x04u)
#define LPSS_PRIV_REMAP_LO     (LPSS_PRIV_OFFSET + 0x40u)
#define LPSS_PRIV_REMAP_HI     (LPSS_PRIV_OFFSET + 0x44u)
#define LPSS_RESETS_FUNC_IDMA  0x07u

/* Exact devices present in this ThinkPad.  00:1f.5 is also Intel class
 * 0c/80, but it is the 02a4 SPI flash controller.  Selecting by class and
 * calling pci_enable() on it was not a probe; it was programming the wrong
 * device by scan-order luck. */
#define CML_LPSS_I2C0 0x02e8u
#define CML_LPSS_I2C1 0x02e9u

/* CML-LP uses the 216 MHz CNL LPSS clock.  These conservative fast-mode
 * counts yield the firmware-declared 400 kHz bus; 0x3c/0x82 drove it at about
 * 1.09 MHz, outside the touchpad's declared timing. */
#define CML_FS_SCL_HCNT 191u
#define CML_FS_SCL_LCNT 345u

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
static u32  i2c_device = 0;
static u32  i2c_last_abort = 0;

/* HID descriptor fields we care about, as read off the device */
static u16 hid_input_reg = 0, hid_max_input = 0;
static u16 hid_cmd_reg = 0, hid_data_reg = 0;
static u16 hid_vid = 0, hid_pid = 0, hid_version = 0;
static u16 hid_report_desc_len = 0, hid_report_desc_reg = 0;
static u32 hid_report_logged = 0;
static struct zltouch_state touch;
static int touch_state = 0;       /* 0 idle, 1 power wait, 2 reset, 3 live,
                                   * 4 final power wait, 5 mode-set failed */
static int touch_auto_attempted = 0;
static u32 touch_deadline = 0;
static u32 touch_poll_at = 0;

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

/* ---- finding the controller ------------------------------------------- */
static int is_cml_i2c_device(int device)
{
    return device == (int)CML_LPSS_I2C0 || device == (int)CML_LPSS_I2C1;
}

static void lpss_prepare(void)
{
    /* Linux's intel-lpss core asserts both function/iDMA resets, releases
     * them, then publishes the DesignWare child's remap address.  The upper
     * write is required even on this laptop, where firmware placed BAR0 below
     * 4 GiB: leaving stale high bits is another scan-order dependency. */
    wr32(i2c_base + LPSS_PRIV_RESETS, 0);
    wr32(i2c_base + LPSS_PRIV_RESETS, LPSS_RESETS_FUNC_IDMA);
    wr32(i2c_base + LPSS_PRIV_REMAP_LO, (u32)i2c_base);
    wr32(i2c_base + LPSS_PRIV_REMAP_HI,
         sizeof(uptr) >= 8 ? (u32)(i2c_base >> 16 >> 16) : 0u);
}

/* There are two CML-LP I2C functions.  `which` counts only those exact PCI
 * IDs, never every 0c/80 function: the third class match is the SPI flash
 * controller and must not even reach pci_enable(). */
int i2c_find(int which)
{
    if (which < 0) return -1;
    pci_scan();
    int seen = 0;
    for (int i = 0; i < pci_count(); i++) {
        if (pci_vendor(i)   != 0x8086) continue;
        if (!is_cml_i2c_device(pci_device(i))) continue;
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
        i2c_device = (u32)pci_device(i);
        i2c_found = 1;
        lpss_prepare();
        zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_INFO,
                    0x49324301u, i2c_device, rd32(i2c_base + IC_COMP_TYPE));
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

static int select_designware(void)
{
    if (i2c_found && i2c_is_designware()) return 1;

    /* The touchpad is on the second LPSS I2C function on this ThinkPad.  Fall
     * back to #0 rather than letting one asleep/absent function poison the
     * static `i2c_found` state forever. */
    if (i2c_find(1) >= 0 && i2c_is_designware()) return 1;
    if (i2c_find(0) >= 0 && i2c_is_designware()) return 1;
    return 0;
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
    if (!select_designware()) {
        zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_ERROR,
                    0x49324302u, i2c_device,
                    i2c_found ? rd32(i2c_base + IC_COMP_TYPE) : 0u);
        return 0;
    }

    i2c_disable();
    wr32(i2c_base + IC_CON, CON_MASTER | CON_SLAVE_DIS | CON_RESTART_EN | CON_SPEED_FAST);
    wr32(i2c_base + IC_FS_SCL_HCNT, CML_FS_SCL_HCNT);
    wr32(i2c_base + IC_FS_SCL_LCNT, CML_FS_SCL_LCNT);
    wr32(i2c_base + IC_TX_TL, 0);
    wr32(i2c_base + IC_RX_TL, 0);
    wr32(i2c_base + IC_INTR_MASK, 0);        /* we poll; no interrupts */
    zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_INFO,
                0x49324303u, i2c_device,
                (CML_FS_SCL_HCNT << 16) | CML_FS_SCL_LCNT);
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

static void clear_abort(int preserve)
{
    u32 source = rd32(i2c_base + IC_TX_ABRT_SOURCE);
    if (preserve && source) i2c_last_abort = source;
    (void)rd32(i2c_base + IC_CLR_TX_ABRT);
}

/* Write some bytes, then read some back with a repeated start - which is the
 * only transaction shape HID-over-I2C ever needs. */
static int i2c_write_read(int addr, const u8 *out, int nout, u32 inbuf, int nin)
{
    if (!i2c_found) return 0;
    i2c_last_abort = 0;
    clear_abort(0);
    i2c_set_target(addr);

    for (int i = 0; i < nout; i++) {
        if (!wait_tx_room()) { clear_abort(1); return 0; }
        u32 cmd = out[i];
        if (nin == 0 && i == nout - 1) cmd |= CMD_STOP;
        wr32(i2c_base + IC_DATA_CMD, cmd);
    }

    for (int i = 0; i < nin; i++) {
        if (!wait_tx_room()) { clear_abort(1); return 0; }
        u32 cmd = CMD_READ;
        /* A live HID input report is a direct i2c_master_recv(), with no
         * preceding register write. RESTART belongs only to the combined
         * write+read transactions used for descriptors. */
        if (i == 0 && nout > 0) cmd |= CMD_RESTART;
        if (i == nin - 1)  cmd |= CMD_STOP;
        wr32(i2c_base + IC_DATA_CMD, cmd);

        if (!wait_rx_byte()) { clear_abort(1); return 0; }
        *(volatile u8 *)(uptr)(inbuf + (u32)i) = (u8)(rd32(i2c_base + IC_DATA_CMD) & 0xFF);
    }

    if (rd32(i2c_base + IC_RAW_INTR_STAT) & (1u << 6)) {
        clear_abort(1);
        return 0;
    }
    return 1;
}

/* ---- HID over I2C ------------------------------------------------------
 * The HID descriptor is 30 bytes and self-describing: its first field is its
 * own length and its second is the protocol version, so a device that answers
 * with 30 and 0x0100 is a HID device and everything else is noise. That is
 * what makes probing safe. */
#define HID_DESC_BUF (HID_BUF + 0x100u)
#define HID_DESC_LEN 30u
#define HID_RDESC_BUF (HID_BUF + 0x200u)
#define HID_RDESC_MAX 2048u

/* Both buffers inside this driver's own region, and the report buffer clear of
 * the descriptor buffer that follows it. HID_BUF_MAX is 64 and the gap is 256,
 * but the gap is where a future max_input larger than 64 would land. */
_Static_assert(HID_BUF == (unsigned int)HI_HID,
               "i2c_hid: report buffer is not at the base of its region");
_Static_assert(HID_BUF + HID_BUF_MAX <= HID_DESC_BUF,
               "i2c_hid: report buffer overruns the descriptor buffer");
_Static_assert((unsigned long)HID_DESC_BUF + HID_DESC_LEN <= HI_GPU,
               "i2c_hid: buffers escape their region into the blur arena");
_Static_assert((unsigned long)HID_RDESC_BUF + HID_RDESC_MAX <= HI_GPU,
               "i2c_hid: report descriptor escapes its region");

static u32 pack4(volatile u8 *p)
{
    return (u32)p[0] | ((u32)p[1] << 8) |
           ((u32)p[2] << 16) | ((u32)p[3] << 24);
}

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
    hid_report_desc_reg = (u16)(d[6]  | (d[7]  << 8));
    hid_input_reg       = (u16)(d[8]  | (d[9]  << 8));
    hid_max_input       = (u16)(d[10] | (d[11] << 8));
    hid_cmd_reg         = (u16)(d[16] | (d[17] << 8));
    hid_data_reg        = (u16)(d[18] | (d[19] << 8));
    hid_vid             = (u16)(d[20] | (d[21] << 8));
    hid_pid             = (u16)(d[22] | (d[23] << 8));
    hid_version         = (u16)(d[24] | (d[25] << 8));
    return 1;
}

/* Capture the descriptor that defines the touchpad report layout.  This is
 * the missing physical evidence needed to write a decoder without guessing.
 * It is intentionally bounded and recorded in eight-byte chunks; the host
 * extractor reconstructs them after `diagsave`/`halt`. */
static int read_report_descriptor(int addr)
{
    u32 n = hid_report_desc_len;
    if (!n || n > HID_RDESC_MAX || !hid_report_desc_reg) return 0;
    u8 out[2] = {
        (u8)(hid_report_desc_reg & 0xff),
        (u8)(hid_report_desc_reg >> 8)
    };
    if (!i2c_write_read(addr, out, 2, HID_RDESC_BUF, (int)n)) return 0;

    zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_INFO,
                0x49324400u, n, hid_report_desc_reg);
    volatile u8 *d = (volatile u8 *)(uptr)HID_RDESC_BUF;
    for (u32 at = 0; at < n; at += 8u) {
        u8 tail[8] = {0,0,0,0,0,0,0,0};
        u32 take = n - at;
        if (take > 8u) take = 8u;
        for (u32 i = 0; i < take; i++) tail[i] = d[at + i];
        zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_INFO,
                    0x49324500u | (at / 8u),
                    (u32)tail[0] | ((u32)tail[1] << 8) |
                    ((u32)tail[2] << 16) | ((u32)tail[3] << 24),
                    (u32)tail[4] | ((u32)tail[5] << 8) |
                    ((u32)tail[6] << 16) | ((u32)tail[7] << 24));
    }
    return 1;
}

static int accept_hid(int addr, int reg)
{
    if (!read_hid_descriptor(addr, reg)) return 0;
    if (!read_report_descriptor(addr)) return 0;
    hid_addr     = addr;
    hid_desc_reg = reg;
    hid_ready    = 1;
    zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_INFO,
                0x49324304u, ((u32)addr << 16) | (u32)reg,
                ((u32)hid_vid << 16) | hid_pid);
    return 1;
}

/* This is the physical machine's ACPI-resolved route, independently observed
 * from Linux: PCI 8086:02e9, bus address 0x2c, descriptor register 0x20.
 * Trying it first turns automatic startup from a multi-second blind scan into
 * one self-validating transaction. A different machine can still use the
 * exhaustive diagnostic probe below. */
static int probe_x1c8(void)
{
    if (hid_ready) return hid_addr;
    if (!i2c_init()) return -1;
    return accept_hid(0x2c, 0x0020) ? hid_addr : -1;
}

/* Walk the bus. Addresses below 0x08 and above 0x77 are reserved by the I2C
 * specification and are never devices. The two candidate descriptor registers
 * cover essentially every shipping I2C-HID device. */
int i2c_hid_probe(void)
{
    if (hid_ready) return hid_addr;
    if (!i2c_init()) return -1;

    if (accept_hid(0x2c, 0x0020)) return hid_addr;

    static const int regs[2] = { 0x0020, 0x0001 };
    for (int a = 0x08; a <= 0x77; a++) {
        for (int r = 0; r < 2; r++) {
            if (a == 0x2c && regs[r] == 0x0020) continue;
            if (accept_hid(a, regs[r])) return a;
        }
    }
    zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_ERROR,
                0x49324305u, i2c_device, i2c_last_abort);
    return -1;
}

static int send_hid_command(int report_id, int opcode)
{
    u8 out[4] = {
        (u8)(hid_cmd_reg & 0xff), (u8)(hid_cmd_reg >> 8),
        (u8)(report_id & 0x0f), (u8)opcode
    };
    return hid_ready && hid_cmd_reg &&
           i2c_write_read(hid_addr, out, 4, 0, 0);
}

/* A Windows Precision Touchpad powers up in its compatibility mouse mode.
 * Report 2 is relative and has no scan counter; blindly polling it without
 * the ACPI GPIO interrupt can therefore replay one non-zero delta and make the
 * pointer coast after the finger stopped.  The physical SYNA8006 descriptor
 * declares Digitizer/Input Mode as feature report 4. Linux's multitouch driver
 * selects value 3 for a touchpad, which switches this device to the absolute
 * contact report 3 that zltouch_decode() already understands.
 *
 * Keep packet construction separate so the exact on-wire command is a host
 * test, not another physical-hardware guess. HID-over-I2C SET_REPORT is:
 * command register, feature/report-id nibble, opcode, data register, then a
 * length-prefixed numbered report. */
int i2c_hid_touchpad_mode_packet(u8 *out, int cap, int command_reg,
                                  int data_reg)
{
    if (!out || cap < 10) return 0;
    out[0] = (u8)command_reg;
    out[1] = (u8)(command_reg >> 8);
    out[2] = 0x34;                         /* feature report, ID 4 */
    out[3] = 0x03;                         /* SET_REPORT */
    out[4] = (u8)data_reg;
    out[5] = (u8)(data_reg >> 8);
    out[6] = 0x04;                         /* report bytes incl. size + ID */
    out[7] = 0x00;
    out[8] = 0x04;                         /* report ID */
    out[9] = 0x03;                         /* precision touchpad mode */
    return 10;
}

static int set_touchpad_mode(void)
{
    u8 out[10];
    int n = i2c_hid_touchpad_mode_packet(out, (int)sizeof out,
                                         hid_cmd_reg, hid_data_reg);
    return n && i2c_write_read(hid_addr, out, n, 0, 0);
}

static void log_input_report(volatile u8 *b, int length)
{
    if (length <= 2 || hid_report_logged >= 8u) return;
    u32 report = hid_report_logged++;
    zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_INFO,
                0x49325000u | (report << 4), pack4(b), pack4(b + 4));
    zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_INFO,
                0x49325001u | (report << 4), pack4(b + 8), pack4(b + 12));
}

/* Linux's upstream i2c-hid core services an input interrupt with
 * i2c_master_recv(): no input-register prefix. The old zlOS transaction wrote
 * hid_input_reg first, which is the descriptor-read shape and not the live
 * input shape. That could discover the pad and still never receive motion. */
static int read_input_direct(void)
{
    if (!hid_ready) return 0;
    int want = (int)hid_max_input;
    if (want <= 0 || want > (int)HID_BUF_MAX) want = (int)HID_BUF_MAX;

    for (u32 i = 0; i < HID_BUF_MAX; i++) *(volatile u8 *)(uptr)(HID_BUF + i) = 0;
    if (!i2c_write_read(hid_addr, 0, 0, HID_BUF, want)) return -1;

    volatile u8 *b = (volatile u8 *)(uptr)HID_BUF;
    int length = (int)(b[0] | (b[1] << 8));
    if (length > want || length < 0) return -1;
    log_input_report(b, length);
    return length;
}

/* Public diagnostic read. Automatic pointer operation uses service() below. */
int i2c_hid_read_report(void)
{
    int n = read_input_direct();
    return n < 0 ? 0 : n;
}

static int time_reached(u32 now, u32 at)
{
    return (int)(now - at) >= 0;
}

/* Bounded polling stands in for the ACPI GPIO interrupt zlOS does not yet
 * route. It starts only on the exact self-validating X1C8 address and runs at
 * at most 100 Hz. Power/reset are a state machine so input_poll never sleeps. */
int i2c_hid_service(void)
{
    u32 now = idt_ticks();
    if (touch_state == 0) {
        if (touch_auto_attempted || now < 50u) return 0;
        touch_auto_attempted = 1;
        if (probe_x1c8() < 0) return 0;
        zltouch_init(&touch);
        if (!send_hid_command(0, 0x08)) return 0;       /* SET_POWER(ON) */
        touch_state = 1;
        touch_deadline = now + 6u;                     /* upstream waits 60 ms */
        return 0;
    }
    if (touch_state == 1) {
        if (!time_reached(now, touch_deadline)) return 0;
        if (!send_hid_command(0, 0x01)) { touch_state = 0; return 0; } /* RESET */
        touch_state = 2;
        touch_deadline = now + 100u;
        touch_poll_at = now;
        return 0;
    }
    if (touch_state == 2) {
        if (!time_reached(now, touch_poll_at)) return 0;
        touch_poll_at = now + 1u;
        int n = read_input_direct();
        if (n == 0 || time_reached(now, touch_deadline)) {
            if (!send_hid_command(0, 0x08)) { touch_state = 0; return 0; }
            touch_state = 4;
            touch_deadline = now + 6u;
        }
        return 0;
    }
    if (touch_state == 4) {
        if (!time_reached(now, touch_deadline)) return 0;
        if (!set_touchpad_mode()) {
            /* Never expose legacy relative report 2 as a live pointer. With
             * no routed GPIO interrupt, replaying it is exactly the physical
             * multi-second glide recorded on the first X1C8 run. */
            touch_state = 5;
            zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_ERROR,
                        0x49324307u, (u32)hid_addr, i2c_last_abort);
            return 0;
        }
        zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_INFO,
                    0x49324308u, 4u, 3u);
        touch_state = 3;
        /* A write-only DesignWare transaction is queued before it is fully
         * on the wire. Give SET_REPORT one 100 Hz service interval before a
         * direct read disables/re-targets the controller. */
        touch_poll_at = now + 1u;
        zllog_event(ZLLOG_SUB_DRIVER, ZLLOG_EV_DRIVER_STATE, ZLLOG_INFO,
                    0x49324306u, (u32)hid_addr, hid_max_input);
        return 0;
    }
    if (touch_state != 3) return 0;
    if (!time_reached(now, touch_poll_at)) return 0;
    touch_poll_at = now + 1u;
    int n = read_input_direct();
    if (n <= 2) return 0;
    return zltouch_decode(&touch, (const u8 *)(uptr)HID_BUF, n, now);
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
int i2c_hid_device_id(void) { return (int)i2c_device; }
u32 i2c_hid_abort_source(void) { return i2c_last_abort; }
u32 i2c_hid_fs_hcnt(void) { return i2c_found ? rd32(i2c_base + IC_FS_SCL_HCNT) : 0u; }
u32 i2c_hid_fs_lcnt(void) { return i2c_found ? rd32(i2c_base + IC_FS_SCL_LCNT) : 0u; }
u32 i2c_hid_lpss_reset(void) { return i2c_found ? rd32(i2c_base + LPSS_PRIV_RESETS) : 0u; }
int i2c_hid_pointer_ready(void) { return touch_state == 3; }
int i2c_hid_ptr_take_dx(void) { return zltouch_take_dx(&touch); }
int i2c_hid_ptr_take_dy(void) { return zltouch_take_dy(&touch); }
int i2c_hid_ptr_take_wheel(void) { return zltouch_take_wheel(&touch); }
int i2c_hid_ptr_buttons(void) { return touch.buttons; }
int i2c_hid_ptr_take_button(void) { return zltouch_take_button(&touch); }
u32 i2c_hid_ptr_reports(void) { return touch.reports; }
u32 i2c_hid_ptr_malformed(void) { return touch.malformed; }
