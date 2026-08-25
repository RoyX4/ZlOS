/* AX201 read-only transport probe against fake PCI/MMIO. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "../../src/drivers/network/iwlwifi.h"

static int checks, fails;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("  FAIL %d: ", __LINE__); printf(__VA_ARGS__); putchar('\n'); } } while (0)

static int fake_count, fake_vendor, fake_device, fake_class, fake_subclass;
static unsigned fake_bar_lo, fake_bar_hi;
static uint32_t regs[0x400 / 4];
static int mmio_reads, mmio_writes;
static uint64_t last_base;

static void put32(unsigned char *p, uint32_t v)
{
    p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24;
}

static size_t add_tlv(unsigned char *image, size_t at, uint32_t type,
                      const unsigned char *data, uint32_t length)
{
    put32(image + at, type); put32(image + at + 4, length); at += 8;
    if (length) memcpy(image + at, data, length);
    return at + ((length + 3u) & ~3u);
}

void pci_scan(void) {}
int pci_count(void) { return fake_count; }
int pci_vendor(int i) { return i == 0 ? fake_vendor : 0; }
int pci_device(int i) { return i == 0 ? fake_device : 0; }
int pci_class(int i) { return i == 0 ? fake_class : 0; }
int pci_subclass(int i) { return i == 0 ? fake_subclass : 0; }
unsigned pci_bar(int i, int which) { return i == 0 && which == 0 ? fake_bar_lo : 0; }
unsigned pci_bar_hi(int i, int which) { return i == 0 && which == 0 ? fake_bar_hi : 0; }

unsigned iwlwifi_host_read(uint64_t base, unsigned reg)
{
    mmio_reads++;
    last_base = base;
    if ((reg & 3) || reg >= sizeof regs) return 0xffffffffu;
    return regs[reg / 4];
}

static void prime(void)
{
    memset(regs, 0, sizeof regs);
    fake_count = 1; fake_vendor = 0x8086; fake_device = 0x02f0;
    fake_class = 0x02; fake_subclass = 0x80;
    fake_bar_lo = 0x12345000u; fake_bar_hi = 0x00000001u;
    regs[0x000 / 4] = 0x01000100u;
    regs[0x008 / 4] = 0x00000080u;
    regs[0x00c / 4] = 0x00000000u;
    regs[0x018 / 4] = 0x00000200u;
    regs[0x020 / 4] = 0x00000000u;
    regs[0x024 / 4] = 0x08000001u;
    regs[0x028 / 4] = 0x00003510u;
    regs[0x09c / 4] = 0x0010a100u;
    regs[0x380 / 4] = 0x44332202u;
    regs[0x384 / 4] = 0x00006655u;
    mmio_reads = mmio_writes = 0; last_base = 0;
}

static void rejected_identity(int *field, int wrong, const char *what)
{
    prime();
    *field = wrong;
    CHECK(!iwlwifi_probe(), "%s was accepted", what);
    CHECK(mmio_reads == 0, "%s touched MMIO", what);
    CHECK(!iwlwifi_present(), "%s left probe marked present", what);
}

int main(void)
{
    puts("iwlwifi.c AX201 read-only probe against fake PCI/MMIO\n");

    prime();
    CHECK(iwlwifi_probe(), "exact 8086:02f0 QuZ device rejected");
    CHECK(iwlwifi_present(), "valid probe not retained");
    CHECK(iwlwifi_device() == 0x02f0, "device %04x", iwlwifi_device());
    CHECK(iwlwifi_bar_lo() == 0x12345000u, "BAR low %08x", iwlwifi_bar_lo());
    CHECK(iwlwifi_bar_hi() == 1u, "BAR high %08x", iwlwifi_bar_hi());
    CHECK(last_base == 0x0000000112345000ull, "64-bit BAR became %llx", (unsigned long long)last_base);
    CHECK(iwlwifi_hw_rev() == 0x3510u && iwlwifi_hw_type() == 0x351u,
          "QuZ HW revision/type %08x/%03x", iwlwifi_hw_rev(), iwlwifi_hw_type());
    CHECK(iwlwifi_rf_id() == 0x0010a100u, "RF ID %08x", iwlwifi_rf_id());
    CHECK(iwlwifi_gp_cntrl() == 0x08000001u, "GP control %08x", iwlwifi_gp_cntrl());
    CHECK(iwlwifi_mac_csr0() == 0x44332202u && iwlwifi_mac_csr1() == 0x6655u,
          "MAC CSR snapshot differs");
    CHECK(mmio_reads == 10, "probe made %d reads, expected exact 10", mmio_reads);
    CHECK(mmio_writes == 0, "read-only probe made %d writes", mmio_writes);

    prime(); fake_count = 0;
    CHECK(!iwlwifi_probe() && mmio_reads == 0, "absent bus touched MMIO");
    rejected_identity(&fake_vendor, 0x1234, "wrong vendor");
    rejected_identity(&fake_device, 0x06f0, "wrong Intel device");
    rejected_identity(&fake_class, 0x03, "wrong class");
    rejected_identity(&fake_subclass, 0x00, "wired-network subclass");

    prime(); fake_bar_lo = fake_bar_hi = 0;
    CHECK(!iwlwifi_probe(), "zero BAR accepted");
    CHECK(mmio_reads == 0, "zero BAR touched MMIO");

    prime(); regs[0x000 / 4] = 0xffffffffu;
    CHECK(!iwlwifi_probe(), "all-ones interface CSR accepted");
    CHECK(!iwlwifi_present(), "failed CSR validation remained present");

    prime(); regs[0x028 / 4] = 0x00003310u;
    CHECK(!iwlwifi_probe(), "non-QuZ MAC family accepted for AX201 path");

    puts("\nAPI-77 firmware TLV parser\n");
    unsigned char fw[160]; memset(fw, 0, sizeof fw);
    put32(fw + 4, 0x0a4c5749u); memcpy(fw + 8, "release/test", 12);
    put32(fw + 72, 77); put32(fw + 76, 0x12345678u);
    unsigned char section[7] = { 0x00, 0x10, 0x00, 0x00, 1, 2, 3 };
    unsigned char flag[4] = { 0x44, 0x33, 0x22, 0x11 };
    size_t fwlen = add_tlv(fw, 88, 24, section, sizeof section);
    fwlen = add_tlv(fw, fwlen, 18, flag, sizeof flag);
    struct iwlfw_info info;
    CHECK(iwlfw_parse_api77(fw, fwlen, &info) == 1, "synthetic API-77 image rejected");
    CHECK(info.api == 77 && info.build == 0x12345678u, "version/build wrong");
    CHECK(!strcmp(info.human, "release/test"), "human version '%s'", info.human);
    CHECK(info.tlv_count == 2 && info.runtime_count == 1, "TLV/section count %u/%u", info.tlv_count, info.runtime_count);
    CHECK(info.runtime[0].offset == 0x1000 && info.runtime[0].length == 3,
          "runtime section offset/length %x/%u", info.runtime[0].offset, info.runtime[0].length);
    CHECK(info.runtime[0].data[0] == 1 && info.runtime[0].data[2] == 3,
          "runtime section payload wrong");
    CHECK(info.flags == 0x11223344u && info.payload_bytes == 11,
          "flags/payload %08x/%llu", info.flags, info.payload_bytes);

    CHECK(iwlfw_parse_api77(fw, 87, &info) == -2, "short header accepted");
    unsigned char saved = fw[4]; fw[4] = 0;
    CHECK(iwlfw_parse_api77(fw, fwlen, &info) == -3, "bad magic accepted"); fw[4] = saved;
    put32(fw + 72, 76);
    CHECK(iwlfw_parse_api77(fw, fwlen, &info) == -4, "wrong API accepted"); put32(fw + 72, 77);
    CHECK(iwlfw_parse_api77(fw, 88 + 8 + 7, &info) == -6,
          "truncated TLV padding accepted");
    put32(fw + 92, 0xffffffffu);
    CHECK(iwlfw_parse_api77(fw, fwlen, &info) == -5, "oversized TLV accepted");
    put32(fw + 92, sizeof section);

    const char *path = "/lib/firmware/intel/iwlwifi/iwlwifi-QuZ-a0-hr-b0-77.ucode";
    FILE *file = fopen(path, "rb");
    CHECK(file != NULL, "installed AX201 firmware missing");
    if (file) {
        CHECK(fseek(file, 0, SEEK_END) == 0, "firmware seek failed");
        long size = ftell(file);
        CHECK(size == 1406716, "installed firmware size %ld", size);
        CHECK(fseek(file, 0, SEEK_SET) == 0, "firmware rewind failed");
        unsigned char *bytes = size > 0 ? malloc((size_t)size) : NULL;
        CHECK(bytes != NULL, "firmware allocation failed");
        if (bytes) {
            CHECK(fread(bytes, 1, (size_t)size, file) == (size_t)size,
                  "firmware read incomplete");
            int parsed = iwlfw_parse_api77(bytes, (uint64_t)size, &info);
            CHECK(parsed == 1, "installed firmware parse returned %d", parsed);
            CHECK(info.api == 77 && info.build == 0xf39cc7f9u,
                  "installed API/build %u/%08x", info.api, info.build);
            CHECK(!strcmp(info.human, "release/core74::f39cc7f9"),
                  "installed human version '%s'", info.human);
            CHECK(info.runtime_count > 0, "installed image has no runtime sections");
            CHECK(iwlfw_verify_target_api77(bytes, (uint64_t)size, &info),
                  "installed firmware SHA-256 target allowlist failed");
            bytes[100] ^= 1;
            CHECK(!iwlfw_verify_target_api77(bytes, (uint64_t)size, &info),
                  "corrupted firmware passed SHA-256 target allowlist");
            bytes[100] ^= 1;
            printf("  installed: %u TLVs, %u runtime, %u init, %llu payload bytes\n",
                   info.tlv_count, info.runtime_count, info.init_count,
                   info.payload_bytes);
            free(bytes);
        }
        fclose(file);
    }

    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
