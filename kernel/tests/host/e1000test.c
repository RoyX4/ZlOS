/* Intel NIC descriptor rings against fake PCI/MMIO, no hardware. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../../src/drivers/network/e1000.h"

static int checks, fails;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("  FAIL %d: ", __LINE__); printf(__VA_ARGS__); putchar('\n'); } } while (0)

#define CTRL 0x0000
#define STATUS 0x0008
#define RDBAL 0x2800
#define RDBAH 0x2804
#define RDT 0x2818
#define TDBAL 0x3800
#define TDBAH 0x3804
#define TDT 0x3818
static unsigned char regs[0x6000] __attribute__((aligned(4096)));
static int clear_reset = 1, complete_tx = 1, fake_count = 1;
static int fake_devices[2] = { 0x0d4f, 0x100e };
static int mmio_writes;

struct rxd { uint64_t addr; uint16_t len, csum; uint8_t status, errors; uint16_t special; } __attribute__((packed));
struct txd { uint64_t addr; uint16_t len; uint8_t cso, cmd, status, css; uint16_t special; } __attribute__((packed));
static unsigned reg32(unsigned off) { return *(unsigned *)(regs + off); }
static void set32(unsigned off, unsigned v) { *(unsigned *)(regs + off) = v; }

void e1000_host_write(unsigned reg, unsigned value)
{
    mmio_writes++;
    if (reg == CTRL && (value & (1u << 26)) && clear_reset)
        set32(CTRL, value & ~(1u << 26));
    if (reg == TDT && complete_tx) {
        uint64_t a = (uint64_t)reg32(TDBAL) | (uint64_t)reg32(TDBAH) << 32;
        struct txd *d = (struct txd *)(uintptr_t)a;
        unsigned done = (value + 31) % 32;
        d[done].status = 1;
    }
}

void pci_scan(void) {}
int pci_count(void) { return fake_count; }
int pci_vendor(int i) { return i >= 0 && i < fake_count ? 0x8086 : 0; }
int pci_device(int i) { return i >= 0 && i < fake_count ? fake_devices[i] : 0; }
int pci_class(int i) { return i >= 0 && i < fake_count ? 2 : 0; }
int pci_subclass(int i) { return i >= 0 && i < fake_count ? 0 : -1; }
unsigned pci_bar(int i, int which) { return (i >= 0 && i < fake_count && which == 0) ? (unsigned)(uintptr_t)regs : 0; }
unsigned pci_bar_hi(int i, int which) { (void)i; (void)which; return 0; }
void pci_enable(int i) { (void)i; }

static void prime(void)
{
    memset(regs, 0, sizeof regs);
    set32(STATUS, 1u << 1);
    set32(0x5400, 0x44332202u);
    set32(0x5404, 0x80006655u);
    clear_reset = complete_tx = 1;
    mmio_writes = 0;
}

int main(void)
{
    puts("e1000.c against fake PCI/MMIO, no machine\n");
    const int pch_devices[] = { 0x153a, 0x15b7, 0x0d4f };
    for (unsigned p = 0; p < sizeof pch_devices / sizeof pch_devices[0]; p++) {
        fake_count = 1;
        fake_devices[0] = pch_devices[p];
        prime();
        CHECK(e1000_find() >= 0, "PCH device %04x not selected", fake_devices[0]);
        CHECK(e1000_device() == fake_devices[0], "device %04x", e1000_device());
        CHECK(!e1000_init(), "PCH device %04x entered legacy reset", fake_devices[0]);
        CHECK(mmio_writes == 0, "PCH device %04x wrote %d MMIO registers",
              fake_devices[0], mmio_writes);
    }

    fake_count = 2;
    fake_devices[0] = 0x0d4f;
    fake_devices[1] = 0x100e;
    prime();
    CHECK(e1000_find() == 1, "working 82540EM was not preferred over PCH inventory");
    CHECK(e1000_device() == 0x100e, "device %04x", e1000_device());
    clear_reset = 0;
    CHECK(!e1000_init(), "stuck reset was accepted");
    CHECK(e1000_reset_timeouts() > 0, "reset timeout not counted");

    prime();
    CHECK(e1000_init(), "initialisation failed");
    CHECK(e1000_link_up(), "carrier not reported");
    int reset_timeouts = e1000_reset_timeouts();
    clear_reset = 0;
    CHECK(e1000_init(), "live device was reset instead of reused");
    CHECK(e1000_reset_timeouts() == reset_timeouts,
          "idempotent init touched reset path");
    clear_reset = 1;
    unsigned char wantmac[6] = { 2, 0x22, 0x33, 0x44, 0x55, 0x66 };
    for (int i = 0; i < 6; i++) CHECK(e1000_mac(i) == wantmac[i], "MAC byte %d", i);

    unsigned char frame[64]; for (int i = 0; i < 64; i++) frame[i] = i ^ 0x5a;
    CHECK(e1000_send(frame, 64), "transmit refused");
    uint64_t ta = (uint64_t)reg32(TDBAL) | (uint64_t)reg32(TDBAH) << 32;
    struct txd *td = (struct txd *)(uintptr_t)ta;
    CHECK(td[0].len == 64 && td[0].cmd == 0x0b, "TX descriptor wrong");
    CHECK(!memcmp((void *)(uintptr_t)td[0].addr, frame, 64), "TX bytes differ");
    CHECK(e1000_tx_count() == 1, "TX count %d", e1000_tx_count());

    uint64_t ra = (uint64_t)reg32(RDBAL) | (uint64_t)reg32(RDBAH) << 32;
    struct rxd *rd = (struct rxd *)(uintptr_t)ra;
    memcpy((void *)(uintptr_t)rd[0].addr, frame, 64); rd[0].len = 64; rd[0].status = 1;
    unsigned char got[64];
    CHECK(e1000_poll(got, sizeof got) == 64, "receive length wrong");
    CHECK(!memcmp(got, frame, 64), "RX bytes differ");
    CHECK(reg32(RDT) == 0, "RX tail did not return descriptor");
    CHECK(e1000_rx_count() == 1 && e1000_rx_drops() == 0, "RX counters wrong");

    complete_tx = 0;
    for (int i = 0; i < 32; i++) CHECK(e1000_send(frame, 64), "ring filled early at %d", i);
    CHECK(!e1000_send(frame, 64), "full TX ring was accepted");
    CHECK(e1000_tx_full() == 1, "TX-full count %d", e1000_tx_full());
    CHECK(!e1000_send(frame, 2049), "oversized frame accepted");

    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
