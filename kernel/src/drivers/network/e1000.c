/* e1000.c - bounded Intel wired Ethernet for QEMU/legacy e1000 devices.
 *
 * This is deliberately a link driver only.  netdev.c selects it, net.c owns
 * Ethernet/IP, and dhcp.c owns address configuration.  The legacy descriptor
 * format and registers below are valid for 8254x-class parts.  I219 is detected
 * for inventory, but its PCH-specific reset/PHY sequence is deliberately
 * quarantined before MMIO until that sequence exists.  Every supported wait is
 * bounded, all DMA is explicit, and failure cannot become a boot dependency. */

#include "e1000.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
#if defined(ZL_64) || defined(E1000_HOSTTEST)
typedef unsigned long long uptr;
#else
typedef unsigned int uptr;
#endif

void pci_scan(void);
int pci_count(void);
int pci_vendor(int i);
int pci_device(int i);
int pci_class(int i);
int pci_subclass(int i);
u32 pci_bar(int i, int which);
u32 pci_bar_hi(int i, int which);
void pci_enable(int i);

#define REG_CTRL   0x0000
#define REG_STATUS 0x0008
#define REG_EERD   0x0014
#define REG_ICR    0x00c0
#define REG_IMC    0x00d8
#define REG_RCTL   0x0100
#define REG_TCTL   0x0400
#define REG_TIPG   0x0410
#define REG_RDBAL  0x2800
#define REG_RDBAH  0x2804
#define REG_RDLEN  0x2808
#define REG_RDH    0x2810
#define REG_RDT    0x2818
#define REG_TDBAL  0x3800
#define REG_TDBAH  0x3804
#define REG_TDLEN  0x3808
#define REG_TDH    0x3810
#define REG_TDT    0x3818
#define REG_RAL    0x5400
#define REG_RAH    0x5404

#define CTRL_RST   (1u << 26)
#define CTRL_ASDE  (1u << 5)
#define CTRL_SLU   (1u << 6)
#define STATUS_LU  (1u << 1)
#define RCTL_EN    (1u << 1)
#define RCTL_BAM   (1u << 15)
#define RCTL_SECRC (1u << 26)
#define TCTL_EN    (1u << 1)
#define TCTL_PSP   (1u << 3)

#define RX_N 64
#define TX_N 32
#define BUF_N 2048

struct rx_desc { u64 addr; u16 len, csum; u8 status, errors; u16 special; } __attribute__((packed));
struct tx_desc { u64 addr; u16 len; u8 cso, cmd, status, css; u16 special; } __attribute__((packed));

static struct rx_desc rx[RX_N] __attribute__((aligned(128)));
static struct tx_desc tx[TX_N] __attribute__((aligned(128)));
static u8 rxbuf[RX_N][BUF_N] __attribute__((aligned(128)));
static u8 txbuf[TX_N][BUF_N] __attribute__((aligned(128)));

static volatile u8 *mmio;
static u32 mmio_low, mmio_high;
static int dev_idx = -1, dev_id, ready, rx_at, tx_at;
static u8 mac[6];
static int n_tx, n_rx, n_drop, n_full, n_reset_timeout;

#ifdef E1000_HOSTTEST
void e1000_host_write(unsigned reg, unsigned value);
#endif

static void fence(void) { __asm__ volatile("" ::: "memory"); }
static void pause_cpu(void) { __asm__ volatile("pause"); }
static u32 rd(u32 reg) { return *(volatile u32 *)(mmio + reg); }
static void wr(u32 reg, u32 value)
{
    *(volatile u32 *)(mmio + reg) = value; fence();
#ifdef E1000_HOSTTEST
    e1000_host_write(reg, value);
#endif
}

static u64 phys(const void *p) { return (u64)(uptr)p; }

static int supported(int id)
{
    /* 82540EM (QEMU), 82574L, I217/I218, and the target I219-LM 0d4f. */
    return id == 0x100e || id == 0x10d3 || id == 0x153a ||
           id == 0x15b7 || id == 0x0d4f;
}

static int requires_pch_init(int id)
{
    return id == 0x153a || id == 0x15b7 || id == 0x0d4f;
}

int e1000_find(void)
{
    pci_scan(); dev_idx = -1;
    /* Prefer a device this driver can initialize. Keep PCH parts visible for
     * inventory only when no working 8254x-class adapter is also present. */
    for (int pass = 0; pass < 2; pass++)
        for (int i = 0; i < pci_count(); i++) {
            int id = pci_device(i);
            if (pci_vendor(i) != 0x8086 || pci_class(i) != 0x02 ||
                pci_subclass(i) != 0x00 || !supported(id)) continue;
            if (pass == 0 && requires_pch_init(id)) continue;
            if (pass == 1 && !requires_pch_init(id)) continue;
            dev_idx = i; dev_id = id;
            mmio_low = pci_bar(i, 0); mmio_high = pci_bar_hi(i, 0);
            if (!mmio_low && !mmio_high) { dev_idx = -1; continue; }
            mmio = (volatile u8 *)(uptr)(((u64)mmio_high << 32) | mmio_low);
            return i;
        }
    return -1;
}

static int eeprom_word(int word, u16 *out)
{
    /* e1000 uses START bit 0/DONE bit 4/address at 8. I219 uses the compact
     * START bit 0/DONE bit 1/address at 2 form. Try both, bounded. */
    wr(REG_EERD, 1u | ((u32)word << 8));
    for (int n = 0; n < 100000; n++) {
        u32 v = rd(REG_EERD);
        if (v & (1u << 4)) { *out = (u16)(v >> 16); return 1; }
        pause_cpu();
    }
    wr(REG_EERD, 1u | ((u32)word << 2));
    for (int n = 0; n < 100000; n++) {
        u32 v = rd(REG_EERD);
        if (v & (1u << 1)) { *out = (u16)(v >> 16); return 1; }
        pause_cpu();
    }
    return 0;
}

static int read_mac(void)
{
    u32 lo = rd(REG_RAL), hi = rd(REG_RAH);
    if (hi & 0x80000000u) {
        mac[0] = lo; mac[1] = lo >> 8; mac[2] = lo >> 16; mac[3] = lo >> 24;
        mac[4] = hi; mac[5] = hi >> 8;
    } else {
        for (int w = 0; w < 3; w++) {
            u16 v;
            if (!eeprom_word(w, &v)) return 0;
            mac[w * 2] = (u8)v; mac[w * 2 + 1] = (u8)(v >> 8);
        }
    }
    int any = 0, allff = 1;
    for (int i = 0; i < 6; i++) { if (mac[i]) any = 1; if (mac[i] != 0xff) allff = 0; }
    return any && !allff && !(mac[0] & 1);
}

int e1000_init(void)
{
    /* net_boot first validates the selected device, then ip_auto attaches the
     * link. Both cross the generic netdev_init boundary. Re-entry must not
     * reset a live I219 descriptor ring a second time. */
    if (dev_idx < 0 && e1000_find() < 0) return 0;
    /* I217/I218/I219 are integrated PCH devices, not 8254x-compatible parts.
     * A physical 8086:0d4f ThinkPad journal proved that the generic CTRL.RST
     * sequence never returned safely. Detect them, report them, but do not
     * touch MMIO until the PCH MAC/PHY/ULP/reset arbitration path is built. */
    if (requires_pch_init(dev_id)) return 0;
    if (ready) return 1;
    ready = 0; n_tx = n_rx = n_drop = n_full = 0; rx_at = tx_at = 0;
    pci_enable(dev_idx);
    wr(REG_IMC, 0xffffffffu); (void)rd(REG_ICR);
    wr(REG_CTRL, rd(REG_CTRL) | CTRL_RST);
    int reset_ok = 0;
    for (int n = 0; n < 1000000; n++) {
        if (!(rd(REG_CTRL) & CTRL_RST)) { reset_ok = 1; break; }
        pause_cpu();
    }
    if (!reset_ok) { n_reset_timeout++; return 0; }
    wr(REG_IMC, 0xffffffffu); (void)rd(REG_ICR);
    wr(REG_CTRL, rd(REG_CTRL) | CTRL_ASDE | CTRL_SLU);
    if (!read_mac()) return 0;

    for (int i = 0; i < RX_N; i++) {
        rx[i].addr = phys(rxbuf[i]); rx[i].len = rx[i].csum = 0;
        rx[i].status = rx[i].errors = 0; rx[i].special = 0;
    }
    for (int i = 0; i < TX_N; i++) {
        tx[i].addr = phys(txbuf[i]); tx[i].len = 0; tx[i].cmd = 0;
        tx[i].status = 1; tx[i].cso = tx[i].css = 0; tx[i].special = 0;
    }
    fence();
    u64 rp = phys(rx), tp = phys(tx);
    wr(REG_RDBAL, (u32)rp); wr(REG_RDBAH, (u32)(rp >> 32));
    wr(REG_RDLEN, (u32)sizeof rx); wr(REG_RDH, 0); wr(REG_RDT, RX_N - 1);
    wr(REG_TDBAL, (u32)tp); wr(REG_TDBAH, (u32)(tp >> 32));
    wr(REG_TDLEN, (u32)sizeof tx); wr(REG_TDH, 0); wr(REG_TDT, 0);
    wr(REG_TIPG, 10u | (8u << 10) | (6u << 20));
    wr(REG_TCTL, TCTL_EN | TCTL_PSP | (15u << 4) | (64u << 12));
    wr(REG_RCTL, RCTL_EN | RCTL_BAM | RCTL_SECRC);
    ready = 1;
    return 1;
}

int e1000_send(const u8 *frame, int len)
{
    if (!ready || !frame || len <= 0 || len > BUF_N) return 0;
    struct tx_desc *d = &tx[tx_at];
    if (!(d->status & 1)) { n_full++; return 0; }
    for (int i = 0; i < len; i++) txbuf[tx_at][i] = frame[i];
    d->len = (u16)len; d->cso = d->css = 0; d->special = 0;
    d->cmd = 0x0b; d->status = 0; fence();
    tx_at = (tx_at + 1) % TX_N; wr(REG_TDT, (u32)tx_at); n_tx++;
    return 1;
}

int e1000_poll(u8 *out, int max)
{
    if (!ready || !out || max <= 0) return 0;
    struct rx_desc *d = &rx[rx_at];
    if (!(d->status & 1)) return 0;
    int n = d->len;
    if (d->errors || n <= 0 || n > BUF_N) { n_drop++; n = 0; }
    else {
        int copy = n < max ? n : max;
        for (int i = 0; i < copy; i++) out[i] = rxbuf[rx_at][i];
        if (copy < n) n_drop++;
        n = copy; n_rx++;
    }
    d->status = 0; d->errors = 0; d->len = 0; fence();
    wr(REG_RDT, (u32)rx_at); rx_at = (rx_at + 1) % RX_N;
    return n;
}

int e1000_mac(int i) { return (i >= 0 && i < 6) ? mac[i] : 0; }
int e1000_link_up(void) { return ready && (rd(REG_STATUS) & STATUS_LU) != 0; }
int e1000_device(void) { return dev_id; }
u32 e1000_mmio_lo(void) { return mmio_low; }
u32 e1000_mmio_hi(void) { return mmio_high; }
int e1000_tx_count(void) { return n_tx; }
int e1000_rx_count(void) { return n_rx; }
int e1000_rx_drops(void) { return n_drop; }
int e1000_tx_full(void) { return n_full; }
int e1000_reset_timeouts(void) { return n_reset_timeout; }
