/* iwlwifi.h - read-only Intel AX201 transport inventory. */
#ifndef ZL_IWLWIFI_H
#define ZL_IWLWIFI_H

int iwlwifi_probe(void);
int iwlwifi_present(void);
int iwlwifi_device(void);
unsigned iwlwifi_bar_lo(void);
unsigned iwlwifi_bar_hi(void);
unsigned iwlwifi_hw_if_config(void);
unsigned iwlwifi_int_status(void);
unsigned iwlwifi_int_mask(void);
unsigned iwlwifi_gpio(void);
unsigned iwlwifi_reset(void);
unsigned iwlwifi_gp_cntrl(void);
unsigned iwlwifi_hw_rev(void);
unsigned iwlwifi_hw_type(void);
unsigned iwlwifi_rf_id(void);
unsigned iwlwifi_mac_csr0(void);
unsigned iwlwifi_mac_csr1(void);

/* API-77 TLV parsing is pure data work: no device access and no allocation. */
#define IWLFW_MAX_SECTIONS 64
struct iwlfw_section {
    const unsigned char *data;
    unsigned length;
    unsigned offset;
    unsigned type;
};
struct iwlfw_info {
    unsigned version;
    unsigned build;
    unsigned api;
    unsigned tlv_count;
    unsigned flags;
    unsigned max_probe_length;
    unsigned runtime_count;
    unsigned init_count;
    unsigned long long payload_bytes;
    char human[65];
    struct iwlfw_section runtime[IWLFW_MAX_SECTIONS];
    struct iwlfw_section init[IWLFW_MAX_SECTIONS];
};

int iwlfw_parse_api77(const unsigned char *image, unsigned long long bytes,
                      struct iwlfw_info *out);
int iwlfw_verify_target_api77(const unsigned char *image,
                              unsigned long long bytes,
                              struct iwlfw_info *out);

#endif
