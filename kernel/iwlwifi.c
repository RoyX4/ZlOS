/* iwlwifi.c - read-only Intel AX201 (8086:02f0) transport inventory.
 *
 * This is intentionally not yet a Wi-Fi driver.  The AX201 is an integrated
 * CNVi device whose scan/auth/association/data path runs through Intel API-77
 * firmware.  Before that large state machine is allowed near the physical
 * laptop, this layer proves the exact PCI identity, 64-bit BAR handling and
 * direct CSR register map without enabling bus mastering, resetting the NIC,
 * requesting MAC access or writing one device register.
 *
 * Intel's upstream iwl-csr.h states that these CSR registers are direct PCI
 * bus registers and may be read without waking the MAC.  Keep this file read
 * only until the physical `wifi` command has returned a valid QuZ snapshot. */

#include "iwlwifi.h"
#include "crypto.h"

typedef unsigned int u32;
typedef unsigned long long u64;
#if defined(ZL_64) || defined(IWLWIFI_HOSTTEST)
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

#define INTEL_VENDOR       0x8086
#define AX201_DEVICE       0x02f0
#define NETWORK_CLASS      0x02
#define NETWORK_OTHER      0x80

#define CSR_HW_IF_CONFIG   0x000
#define CSR_INT            0x008
#define CSR_INT_MASK       0x00c
#define CSR_GPIO_IN        0x018
#define CSR_RESET          0x020
#define CSR_GP_CNTRL       0x024
#define CSR_HW_REV         0x028
#define CSR_HW_RF_ID       0x09c
#define CSR_MAC_ADDR       0x380

#define HW_TYPE_MASK       0x000fff0u
#define HW_TYPE_QUZ        0x351u

static u64 mmio_addr;
static u32 bar_low, bar_high;
static int dev_idx = -1, ready;
static u32 csr_hw_if, csr_int, csr_int_mask, csr_gpio, csr_reset;
static u32 csr_gp, csr_hw_rev, csr_rf_id, csr_mac0, csr_mac1;

#ifdef IWLWIFI_HOSTTEST
u32 iwlwifi_host_read(u64 base, u32 reg);
static u32 rd(u32 reg) { return iwlwifi_host_read(mmio_addr, reg); }
#else
static void fence(void) { __asm__ volatile("" ::: "memory"); }
static u32 rd(u32 reg)
{
    u32 value = *(volatile u32 *)(uptr)(mmio_addr + reg);
    fence();
    return value;
}
#endif

static void clear_snapshot(void)
{
    ready = 0; dev_idx = -1; mmio_addr = 0; bar_low = bar_high = 0;
    csr_hw_if = csr_int = csr_int_mask = csr_gpio = csr_reset = 0;
    csr_gp = csr_hw_rev = csr_rf_id = csr_mac0 = csr_mac1 = 0;
}

int iwlwifi_probe(void)
{
    clear_snapshot();
    pci_scan();
    for (int i = 0; i < pci_count(); i++) {
        if (pci_vendor(i) != INTEL_VENDOR || pci_device(i) != AX201_DEVICE ||
            pci_class(i) != NETWORK_CLASS || pci_subclass(i) != NETWORK_OTHER)
            continue;

        bar_low = pci_bar(i, 0);
        bar_high = pci_bar_hi(i, 0);
        mmio_addr = ((u64)bar_high << 32) | bar_low;
        if (!mmio_addr) { clear_snapshot(); return 0; }

        /* Reads only.  In particular: no pci_enable(), CSR_RESET write,
         * interrupt acknowledgement, MAC_ACCESS_REQ or firmware doorbell. */
        csr_hw_if = rd(CSR_HW_IF_CONFIG);
        csr_int = rd(CSR_INT);
        csr_int_mask = rd(CSR_INT_MASK);
        csr_gpio = rd(CSR_GPIO_IN);
        csr_reset = rd(CSR_RESET);
        csr_gp = rd(CSR_GP_CNTRL);
        csr_hw_rev = rd(CSR_HW_REV);
        csr_rf_id = rd(CSR_HW_RF_ID);
        csr_mac0 = rd(CSR_MAC_ADDR);
        csr_mac1 = rd(CSR_MAC_ADDR + 4);

        /* A dead/unmapped BAR normally reads all ones.  Device 02f0 on this
         * ThinkPad is QuZ; accepting another MAC family would turn a useful
         * inventory result into permission for the wrong future init path. */
        if (csr_hw_if == 0xffffffffu || csr_hw_rev == 0xffffffffu ||
            csr_rf_id == 0xffffffffu ||
            ((csr_hw_rev & HW_TYPE_MASK) >> 4) != HW_TYPE_QUZ) {
            clear_snapshot();
            return 0;
        }
        dev_idx = i;
        ready = 1;
        return 1;
    }
    return 0;
}

int iwlwifi_present(void) { return ready; }
int iwlwifi_device(void) { return ready ? AX201_DEVICE : 0; }
u32 iwlwifi_bar_lo(void) { return bar_low; }
u32 iwlwifi_bar_hi(void) { return bar_high; }
u32 iwlwifi_hw_if_config(void) { return csr_hw_if; }
u32 iwlwifi_int_status(void) { return csr_int; }
u32 iwlwifi_int_mask(void) { return csr_int_mask; }
u32 iwlwifi_gpio(void) { return csr_gpio; }
u32 iwlwifi_reset(void) { return csr_reset; }
u32 iwlwifi_gp_cntrl(void) { return csr_gp; }
u32 iwlwifi_hw_rev(void) { return csr_hw_rev; }
u32 iwlwifi_hw_type(void) { return (csr_hw_rev & HW_TYPE_MASK) >> 4; }
u32 iwlwifi_rf_id(void) { return csr_rf_id; }
u32 iwlwifi_mac_csr0(void) { return csr_mac0; }
u32 iwlwifi_mac_csr1(void) { return csr_mac1; }

/* ---- API-77 firmware container -----------------------------------------
 * Pure bounds-checked parsing, shared by the host corpus test and the future
 * loader.  This does not make the firmware trusted and does not copy or upload
 * a byte.  It only turns the upstream TLV envelope into bounded section views.
 *
 * Layout from Linux iwlwifi/fw/file.h:
 *   88-byte header, then { le32 type, le32 length, data, 4-byte padding }.
 * Secure runtime/init section payloads begin with a le32 device offset. */
#define IWLFW_HEADER_BYTES       88u
#define IWLFW_MAGIC              0x0a4c5749u
#define IWLFW_API                77u
#define IWLFW_TLV_MAX_PROBE      6u
#define IWLFW_TLV_FLAGS          18u
#define IWLFW_TLV_SEC_RT         19u
#define IWLFW_TLV_SEC_INIT       20u
#define IWLFW_TLV_SECURE_RT      24u
#define IWLFW_TLV_SECURE_INIT    25u
#define IWLFW_TLV_SEC_RT_SNIFFER 34u

static u32 fw_le32(const unsigned char *p)
{
    return (u32)p[0] | (u32)p[1] << 8 | (u32)p[2] << 16 | (u32)p[3] << 24;
}

static void fw_zero(struct iwlfw_info *out)
{
    unsigned char *p = (unsigned char *)out;
    for (unsigned long long i = 0; i < sizeof *out; i++) p[i] = 0;
}

static int fw_section(struct iwlfw_section *sections, unsigned *count,
                      u32 type, const unsigned char *data, u32 length)
{
    if (length < 4u) return -8;
    if (*count >= IWLFW_MAX_SECTIONS) return -7;
    struct iwlfw_section *section = &sections[*count];
    section->offset = fw_le32(data);
    section->data = data + 4;
    section->length = length - 4;
    section->type = type;
    (*count)++;
    return 1;
}

int iwlfw_parse_api77(const unsigned char *image, u64 bytes,
                      struct iwlfw_info *out)
{
    if (!image || !out) return -1;
    fw_zero(out);
    if (bytes < IWLFW_HEADER_BYTES) return -2;
    if (fw_le32(image) != 0u || fw_le32(image + 4) != IWLFW_MAGIC) return -3;

    out->version = fw_le32(image + 72);
    out->build = fw_le32(image + 76);
    out->api = out->version <= 0xffu ? out->version :
               ((out->version >> 8) & 0xffu);
    if (out->api != IWLFW_API) return -4;
    for (int i = 0; i < 64; i++) {
        unsigned char c = image[8 + i];
        out->human[i] = (char)c;
        if (!c) break;
    }
    out->human[64] = 0;

    u64 at = IWLFW_HEADER_BYTES;
    while (at < bytes) {
        if (bytes - at < 8u) return -5;
        u32 type = fw_le32(image + at);
        u32 length = fw_le32(image + at + 4);
        at += 8u;
        u64 padded = ((u64)length + 3u) & ~3ull;
        if ((u64)length > bytes - at) return -5;
        if (padded > bytes - at) return -6;
        const unsigned char *data = image + at;
        int result = 1;

        if (type == IWLFW_TLV_FLAGS) {
            if (length < 4u || (length & 3u)) return -8;
            out->flags = fw_le32(data);
        } else if (type == IWLFW_TLV_MAX_PROBE) {
            if (length != 4u) return -8;
            out->max_probe_length = fw_le32(data);
        } else if (type == IWLFW_TLV_SEC_RT ||
                   type == IWLFW_TLV_SECURE_RT ||
                   type == IWLFW_TLV_SEC_RT_SNIFFER) {
            result = fw_section(out->runtime, &out->runtime_count,
                                type, data, length);
        } else if (type == IWLFW_TLV_SEC_INIT ||
                   type == IWLFW_TLV_SECURE_INIT) {
            result = fw_section(out->init, &out->init_count,
                                type, data, length);
        }
        if (result < 0) return result;
        out->tlv_count++;
        out->payload_bytes += length;
        at += padded;
    }

    /* API-77 without a runtime image is structurally valid TLV data but not a
     * firmware image this driver could ever boot. Refuse it here. */
    if (!out->runtime_count) return -9;
    return 1;
}

int iwlfw_verify_target_api77(const unsigned char *image, u64 bytes,
                              struct iwlfw_info *out)
{
    /* Exact artifact installed and used by Linux on the target on 2026-08-22.
     * A different API-77 file must be reviewed and deliberately promoted; a
     * valid TLV envelope alone is not permission to execute vendor firmware. */
    static const unsigned char expected[32] = {
        0x69,0xca,0x09,0x13,0xcc,0xca,0x60,0x9d,
        0xed,0xff,0x5e,0x30,0xb1,0xd4,0x78,0x48,
        0x24,0x87,0xfa,0x14,0xad,0x6b,0x7b,0xe0,
        0x79,0xf8,0xcc,0x85,0x6a,0xc2,0x6c,0xac
    };
    unsigned char digest[32];
    if (!image || !out || bytes != 1406716u) return 0;
    sha256(image, (u32)bytes, digest);
    if (!crypto_equal(digest, expected, 32)) return 0;
    return iwlfw_parse_api77(image, bytes, out) == 1;
}
