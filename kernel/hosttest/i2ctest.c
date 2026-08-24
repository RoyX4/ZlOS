/* i2ctest.c - the ThinkPad LPSS selection/reset/timing path without hardware.
 *
 * The real HID transaction still needs the laptop, but the dangerous part is
 * ordinary policy and register setup: never enable the class-identical SPI
 * flash controller, fall back when LPSS #1 is not a DesignWare core, release
 * the LPSS wrapper reset, publish the remap address, and program the measured
 * 216 MHz timing.  Those are deterministic and belong in a host gate. */
#define _GNU_SOURCE
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define MMIO0 0x30000000u
#define MMIO1 0x30001000u
#define IC_FS_SCL_HCNT 0x1cu
#define IC_FS_SCL_LCNT 0x20u
#define IC_COMP_TYPE   0xfcu
#define LPSS_RESETS    0x204u
#define LPSS_REMAP_LO  0x240u
#define LPSS_REMAP_HI  0x244u
#define DW_COMP_TYPE   0x44570140u

static const int vendors[] = { 0x8086, 0x8086, 0x8086 };
static const int devices[] = { 0x02a4, 0x02e8, 0x02e9 };
static const uint32_t bars[] = { 0x7d800000u, MMIO0, MMIO1 };
static int enabled[3];
static int failures;

static volatile uint32_t *reg32(uint32_t base, uint32_t off)
{
    return (volatile uint32_t *)(uintptr_t)(base + off);
}

static void ok(const char *name, int pass)
{
    printf("  %-66s %s\n", name, pass ? "ok" : "FAIL");
    if (!pass) failures++;
}

void pci_scan(void) { }
int pci_count(void) { return 3; }
int pci_vendor(int i) { return i >= 0 && i < 3 ? vendors[i] : 0; }
int pci_device(int i) { return i >= 0 && i < 3 ? devices[i] : 0; }
int pci_class(int i) { (void)i; return 0x0c; }
int pci_subclass(int i) { (void)i; return 0x80; }
void pci_enable(int i) { if (i >= 0 && i < 3) enabled[i]++; }
uint32_t pci_bar(int i, int which)
{
    return i >= 0 && i < 3 && which == 0 ? bars[i] : 0;
}
uint32_t pci_bar_hi(int i, int which) { (void)i; (void)which; return 0; }
uint32_t idt_ticks(void) { static uint32_t t; return ++t; }
void zllog_event(unsigned subsystem, unsigned event, unsigned severity,
                 unsigned a, unsigned b, unsigned c)
{
    (void)subsystem; (void)event; (void)severity;
    (void)a; (void)b; (void)c;
}

int i2c_init(void);
int i2c_hid_device_id(void);
uint32_t i2c_hid_lpss_reset(void);
uint32_t i2c_hid_fs_hcnt(void);
uint32_t i2c_hid_fs_lcnt(void);
int i2c_hid_touchpad_mode_packet(unsigned char *out, int cap,
                                  int command_reg, int data_reg);

int main(void)
{
    void *p = mmap((void *)(uintptr_t)MMIO0, 0x2000,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p == MAP_FAILED) {
        fprintf(stderr, "mmap fake LPSS MMIO: %s\n", strerror(errno));
        return 2;
    }

    /* Preferred #1 is deliberately not DesignWare.  #0 is valid, proving the
     * fallback rather than simply arranging the preferred path to succeed. */
    *reg32(MMIO1, IC_COMP_TYPE) = 0;
    *reg32(MMIO0, IC_COMP_TYPE) = DW_COMP_TYPE;

    ok("initialisation falls back from LPSS #1 to #0", i2c_init() == 1);
    ok("class-identical SPI flash controller is never enabled", enabled[0] == 0);
    ok("failed preferred I2C candidate was inspected once", enabled[2] == 1);
    ok("fallback I2C candidate was enabled once", enabled[1] == 1);
    ok("selected device is exact CML-LP I2C0 8086:02e8",
       i2c_hid_device_id() == 0x02e8);
    ok("LPSS function and iDMA resets are released",
       i2c_hid_lpss_reset() == 0x07u);
    ok("LPSS remap low dword points at DesignWare child",
       *reg32(MMIO0, LPSS_REMAP_LO) == MMIO0);
    ok("LPSS remap high dword is clear for the low BAR",
       *reg32(MMIO0, LPSS_REMAP_HI) == 0);
    ok("216 MHz fast-mode HCNT is 191", i2c_hid_fs_hcnt() == 191u);
    ok("216 MHz fast-mode LCNT is 345", i2c_hid_fs_lcnt() == 345u);

    {
        unsigned char got[10] = {0};
        const unsigned char want[10] = {
            0x22, 0x00, 0x34, 0x03, 0x23, 0x00, 0x04, 0x00, 0x04, 0x03
        };
        ok("precision-mode SET_REPORT packet is exact HID-over-I2C wire format",
           i2c_hid_touchpad_mode_packet(got, sizeof got, 0x22, 0x23) == 10 &&
           memcmp(got, want, sizeof got) == 0);
        ok("precision-mode packet refuses a short destination",
           i2c_hid_touchpad_mode_packet(got, 9, 0x22, 0x23) == 0);
    }

    printf("\n%d failure(s)\n", failures);
    return failures ? 1 : 0;
}
