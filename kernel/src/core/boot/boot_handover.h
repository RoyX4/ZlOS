/* boot_handover.h - the one pre-kernel contract for every zlOS boot route.
 *
 * The record contains values, never firmware-owned pointers to structures.
 * Addresses are always 64-bit physical addresses, even in the 32-bit build.
 * Its first version is deliberately fixed at 256 bytes: consumers check the
 * version, header length and total length before looking at any later field.
 */
#ifndef ZLOS_BOOT_HANDOVER_H
#define ZLOS_BOOT_HANDOVER_H

#if defined(ZL_64) || defined(__x86_64__)
typedef unsigned long long zlos_boot_uptr;
#else
typedef unsigned int zlos_boot_uptr;
#endif

#define ZLOS_BOOT_MAGIC 0x5A4C4F53424F4F54ULL /* "ZLOSBOOT" */
#define ZLOS_BOOT_ABI_VERSION 1U
#define ZLOS_BOOT_RECORD_BYTES 256U
#define ZLOS_BOOT_HEADER_BYTES 48U

enum zlos_boot_origin {
    ZLOS_BOOT_ORIGIN_NONE = 0,
    ZLOS_BOOT_ORIGIN_RAW_BIOS = 1,
    ZLOS_BOOT_ORIGIN_MULTIBOOT1 = 2,
    ZLOS_BOOT_ORIGIN_NATIVE_UEFI = 3
};

enum zlos_boot_generation {
    ZLOS_BOOT_GENERATION_CURRENT = 1,
    ZLOS_BOOT_GENERATION_PREVIOUS = 2,
    ZLOS_BOOT_GENERATION_RECOVERY = 3,
    ZLOS_BOOT_GENERATION_ASSERTION = 4,
    ZLOS_BOOT_GENERATION_DEBUG = 5
};

enum zlos_boot_pixel_format {
    ZLOS_BOOT_PIXEL_UNKNOWN = 0,
    ZLOS_BOOT_PIXEL_RGBX8888 = 1,
    ZLOS_BOOT_PIXEL_BGRX8888 = 2
};

enum zlos_boot_flags {
    ZLOS_BOOT_F_SEALED = 1U << 0,
    ZLOS_BOOT_F_FRAMEBUFFER = 1U << 1,
    ZLOS_BOOT_F_MEMORY_MAP = 1U << 2,
    ZLOS_BOOT_F_VARIABLE_MAP_ENTRIES = 1U << 3,
    ZLOS_BOOT_F_ACPI = 1U << 4,
    ZLOS_BOOT_F_EXACT_ORIGIN = 1U << 5,
    ZLOS_BOOT_F_EXACT_KERNEL = 1U << 6,
    ZLOS_BOOT_F_FIRMWARE_RETIRED = 1U << 7
};

enum zlos_boot_status {
    ZLOS_BOOT_OK = 0,
    ZLOS_BOOT_E_ARGUMENT = -1,
    ZLOS_BOOT_E_STATE = -2,
    ZLOS_BOOT_E_VERSION = -3,
    ZLOS_BOOT_E_RANGE = -4,
    ZLOS_BOOT_E_CHECKSUM = -5,
    ZLOS_BOOT_E_UNSUPPORTED = -6
};

struct zlos_boot_handover {
    unsigned long long magic;
    unsigned short abi_version;
    unsigned short header_bytes;
    unsigned int total_bytes;
    unsigned int checksum;
    unsigned int origin;
    unsigned int architecture_bits;
    unsigned int flags;
    unsigned int selected_generation;
    unsigned int attempt_ordinal;
    unsigned int attempt_limit;
    unsigned int previous_failure;

    unsigned long long boot_source_address;
    unsigned long long firmware_root_address;
    unsigned long long memory_map_address;
    unsigned long long memory_map_bytes;
    unsigned int memory_map_entry_bytes;
    unsigned int memory_map_version;
    unsigned int memory_map_reserved0;
    unsigned int memory_map_reserved1;

    unsigned long long framebuffer_address;
    unsigned long long framebuffer_bytes;
    unsigned int framebuffer_pitch;
    unsigned int framebuffer_width;
    unsigned int framebuffer_height;
    unsigned int framebuffer_bpp;
    unsigned int framebuffer_pixel_format;
    unsigned int framebuffer_reserved;

    /* These become mandatory only when their corresponding EXACT flag is set.
     * Keeping the slots in v1 lets later loaders add identity without changing
     * the kernel ABI: origin is disk+partition+filesystem+path, kernel is the
     * selected artifact bytes. */
    unsigned char origin_identity[32];
    unsigned char kernel_identity[32];
    unsigned char reserved[56];
};

int zlos_boot_begin(unsigned int origin, unsigned int architecture_bits,
                    unsigned int generation, unsigned int attempt_ordinal,
                    unsigned int attempt_limit);
int zlos_boot_set_source(unsigned long long address);
int zlos_boot_set_firmware_root(unsigned long long address);
int zlos_boot_set_memory_map(unsigned long long address,
                             unsigned long long bytes,
                             unsigned int entry_bytes,
                             unsigned int version,
                             int variable_entries);
int zlos_boot_set_framebuffer(unsigned long long address,
                              unsigned long long bytes,
                              unsigned int pitch, unsigned int width,
                              unsigned int height, unsigned int bpp,
                              unsigned int pixel_format);
int zlos_boot_mark_firmware_retired(void);
int zlos_boot_set_previous_failure(unsigned int reason);
int zlos_boot_set_origin_identity(const unsigned char digest[32]);
int zlos_boot_set_kernel_identity(const unsigned char digest[32]);
int zlos_boot_seal(void);
int zlos_boot_validate(const struct zlos_boot_handover *record);
unsigned int zlos_boot_checksum(const struct zlos_boot_handover *record);
const struct zlos_boot_handover *zlos_boot_record(void);
int zlos_boot_is_valid(void);

int zlos_boot_from_multiboot(zlos_boot_uptr address,
                             unsigned int architecture_bits);
int zlos_boot_from_raw(unsigned long long framebuffer_address,
                       unsigned int pitch, unsigned int width,
                       unsigned int height, unsigned int bpp);

/* Compatibility value consumed by kernel.zl's existing loader() builtin:
 * 0 raw BIOS, 1 Multiboot, 2 native UEFI, -1 invalid/uninitialised. */
int zlos_boot_loader_code(void);

#endif
