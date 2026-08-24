/* boot_handover.c - construct, seal and validate the zlOS boot contract. */
#include "boot_handover.h"

#define ZLOS_BOOT_KNOWN_FLAGS                                             \
    (ZLOS_BOOT_F_SEALED | ZLOS_BOOT_F_FRAMEBUFFER |                       \
     ZLOS_BOOT_F_MEMORY_MAP | ZLOS_BOOT_F_VARIABLE_MAP_ENTRIES |          \
     ZLOS_BOOT_F_ACPI | ZLOS_BOOT_F_EXACT_ORIGIN |                        \
     ZLOS_BOOT_F_EXACT_KERNEL | ZLOS_BOOT_F_FIRMWARE_RETIRED)

#define ZLOS_MB_F_MEMORY_MAP  (1U << 6)
#define ZLOS_MB_F_FRAMEBUFFER (1U << 12)
#define ZLOS_MB_FB_RGB 1U

/* Multiboot 1 is a byte ABI. Keep this definition beside the only code which
 * converts it into the native zlOS record; the rest of the kernel never needs
 * to understand the loader's structure. */
struct zlos_mb1_info {
    unsigned int flags;
    unsigned int mem_lower, mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count, mods_addr;
    unsigned int syms[4];
    unsigned int mmap_length, mmap_addr;
    unsigned int drives_length, drives_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
    unsigned int apm_table;
    unsigned int vbe_control_info, vbe_mode_info;
    unsigned short vbe_mode, vbe_interface_seg, vbe_interface_off,
                   vbe_interface_len;
    unsigned long long framebuffer_addr;
    unsigned int framebuffer_pitch;
    unsigned int framebuffer_width;
    unsigned int framebuffer_height;
    unsigned char framebuffer_bpp;
    unsigned char framebuffer_type;
};

static struct zlos_boot_handover current;

_Static_assert(sizeof(struct zlos_boot_handover) == ZLOS_BOOT_RECORD_BYTES,
               "boot handover ABI must stay exactly 256 bytes");
_Static_assert(__builtin_offsetof(struct zlos_boot_handover, checksum) == 16,
               "boot checksum offset is part of the ABI");
_Static_assert(__builtin_offsetof(struct zlos_boot_handover,
                                  boot_source_address) ==
               ZLOS_BOOT_HEADER_BYTES,
               "boot header length must end at the first address");
_Static_assert(__builtin_offsetof(struct zlos_boot_handover,
                                  origin_identity) == 136,
               "boot identity offset drifted");

static void bytes_zero(void *where, unsigned int bytes)
{
    unsigned char *p = (unsigned char *)where;
    while (bytes--) *p++ = 0;
}

static int bytes_nonzero(const unsigned char *p, unsigned int bytes)
{
    unsigned char any = 0;
    while (bytes--) any |= *p++;
    return any != 0;
}

static int origin_valid(unsigned int origin)
{
    return origin == ZLOS_BOOT_ORIGIN_RAW_BIOS ||
           origin == ZLOS_BOOT_ORIGIN_MULTIBOOT1 ||
           origin == ZLOS_BOOT_ORIGIN_NATIVE_UEFI;
}

static int generation_valid(unsigned int generation)
{
    return generation >= ZLOS_BOOT_GENERATION_CURRENT &&
           generation <= ZLOS_BOOT_GENERATION_DEBUG;
}

static int mutable_record(void)
{
    return current.magic == ZLOS_BOOT_MAGIC &&
           !(current.flags & ZLOS_BOOT_F_SEALED);
}

unsigned int zlos_boot_checksum(const struct zlos_boot_handover *record)
{
    if (!record || record->total_bytes != ZLOS_BOOT_RECORD_BYTES) return 0;
    const unsigned char *p = (const unsigned char *)record;
    unsigned int hash = 2166136261U;
    for (unsigned int i = 0; i < record->total_bytes; i++) {
        /* The checksum authenticates every byte except itself. Treat that
         * four-byte field as zero without mutating a sealed record. */
        unsigned char value = (i >= 16U && i < 20U) ? 0 : p[i];
        hash ^= value;
        hash *= 16777619U;
    }
    return hash;
}

static int validate_shape(const struct zlos_boot_handover *r,
                          int require_sealed)
{
    if (!r) return ZLOS_BOOT_E_ARGUMENT;
    if (r->magic != ZLOS_BOOT_MAGIC ||
        r->abi_version != ZLOS_BOOT_ABI_VERSION ||
        r->header_bytes != ZLOS_BOOT_HEADER_BYTES ||
        r->total_bytes != ZLOS_BOOT_RECORD_BYTES)
        return ZLOS_BOOT_E_VERSION;
    if (!origin_valid(r->origin) ||
        (r->architecture_bits != 32U && r->architecture_bits != 64U) ||
        !generation_valid(r->selected_generation))
        return ZLOS_BOOT_E_UNSUPPORTED;
    if (r->origin == ZLOS_BOOT_ORIGIN_NATIVE_UEFI &&
        r->architecture_bits != 64U)
        return ZLOS_BOOT_E_UNSUPPORTED;
    if (r->origin == ZLOS_BOOT_ORIGIN_RAW_BIOS &&
        r->architecture_bits != 32U)
        return ZLOS_BOOT_E_UNSUPPORTED;
    if (!r->attempt_limit || !r->attempt_ordinal ||
        r->attempt_ordinal > r->attempt_limit)
        return ZLOS_BOOT_E_RANGE;
    if (r->flags & ~ZLOS_BOOT_KNOWN_FLAGS) return ZLOS_BOOT_E_UNSUPPORTED;
    if (require_sealed && !(r->flags & ZLOS_BOOT_F_SEALED))
        return ZLOS_BOOT_E_STATE;
    if (!require_sealed && (r->flags & ZLOS_BOOT_F_SEALED))
        return ZLOS_BOOT_E_STATE;
    if (require_sealed && !(r->flags & ZLOS_BOOT_F_FIRMWARE_RETIRED))
        return ZLOS_BOOT_E_STATE;
    if (require_sealed && !r->boot_source_address)
        return ZLOS_BOOT_E_RANGE;
    if (r->memory_map_reserved0 || r->memory_map_reserved1 ||
        r->framebuffer_reserved ||
        bytes_nonzero(r->reserved, sizeof(r->reserved)))
        return ZLOS_BOOT_E_UNSUPPORTED;

    if (r->flags & ZLOS_BOOT_F_FRAMEBUFFER) {
        unsigned int bytes_per_pixel = r->framebuffer_bpp / 8U;
        if (!r->framebuffer_address || !r->framebuffer_width ||
            !r->framebuffer_height ||
            (r->framebuffer_bpp != 24U && r->framebuffer_bpp != 32U) ||
            !bytes_per_pixel ||
            r->framebuffer_width > (~0U / bytes_per_pixel) ||
            r->framebuffer_pitch < r->framebuffer_width * bytes_per_pixel ||
            r->framebuffer_height >
                (~0ULL / (unsigned long long)r->framebuffer_pitch) ||
            r->framebuffer_bytes <
                (unsigned long long)r->framebuffer_pitch *
                r->framebuffer_height)
            return ZLOS_BOOT_E_RANGE;
    } else if (r->framebuffer_address || r->framebuffer_bytes ||
               r->framebuffer_pitch || r->framebuffer_width ||
               r->framebuffer_height || r->framebuffer_bpp ||
               r->framebuffer_pixel_format) {
        return ZLOS_BOOT_E_STATE;
    }

    if (r->flags & ZLOS_BOOT_F_MEMORY_MAP) {
        if (!r->memory_map_address || !r->memory_map_bytes)
            return ZLOS_BOOT_E_RANGE;
        if (r->flags & ZLOS_BOOT_F_VARIABLE_MAP_ENTRIES) {
            if (r->memory_map_entry_bytes) return ZLOS_BOOT_E_STATE;
        } else if (r->memory_map_entry_bytes < 16U ||
                   r->memory_map_bytes < r->memory_map_entry_bytes) {
            return ZLOS_BOOT_E_RANGE;
        }
    } else if (r->memory_map_address || r->memory_map_bytes ||
               r->memory_map_entry_bytes || r->memory_map_version ||
               (r->flags & ZLOS_BOOT_F_VARIABLE_MAP_ENTRIES)) {
        return ZLOS_BOOT_E_STATE;
    }

    if ((r->flags & ZLOS_BOOT_F_ACPI) != 0U) {
        if (!r->firmware_root_address) return ZLOS_BOOT_E_RANGE;
    } else if (r->firmware_root_address) {
        return ZLOS_BOOT_E_STATE;
    }
    if ((r->flags & ZLOS_BOOT_F_EXACT_ORIGIN) &&
        !bytes_nonzero(r->origin_identity, sizeof(r->origin_identity)))
        return ZLOS_BOOT_E_RANGE;
    if ((r->flags & ZLOS_BOOT_F_EXACT_KERNEL) &&
        !bytes_nonzero(r->kernel_identity, sizeof(r->kernel_identity)))
        return ZLOS_BOOT_E_RANGE;
    return ZLOS_BOOT_OK;
}

int zlos_boot_validate(const struct zlos_boot_handover *record)
{
    int status = validate_shape(record, 1);
    if (status != ZLOS_BOOT_OK) return status;
    return record->checksum == zlos_boot_checksum(record)
        ? ZLOS_BOOT_OK : ZLOS_BOOT_E_CHECKSUM;
}

int zlos_boot_begin(unsigned int origin, unsigned int architecture_bits,
                    unsigned int generation, unsigned int attempt_ordinal,
                    unsigned int attempt_limit)
{
    bytes_zero(&current, sizeof(current));
    current.magic = ZLOS_BOOT_MAGIC;
    current.abi_version = ZLOS_BOOT_ABI_VERSION;
    current.header_bytes = ZLOS_BOOT_HEADER_BYTES;
    current.total_bytes = ZLOS_BOOT_RECORD_BYTES;
    current.origin = origin;
    current.architecture_bits = architecture_bits;
    current.selected_generation = generation;
    current.attempt_ordinal = attempt_ordinal;
    current.attempt_limit = attempt_limit;
    return validate_shape(&current, 0);
}

int zlos_boot_set_source(unsigned long long address)
{
    if (!mutable_record()) return ZLOS_BOOT_E_STATE;
    current.boot_source_address = address;
    return ZLOS_BOOT_OK;
}

int zlos_boot_set_firmware_root(unsigned long long address)
{
    if (!mutable_record()) return ZLOS_BOOT_E_STATE;
    current.firmware_root_address = address;
    if (address) current.flags |= ZLOS_BOOT_F_ACPI;
    else current.flags &= ~ZLOS_BOOT_F_ACPI;
    return ZLOS_BOOT_OK;
}

int zlos_boot_set_memory_map(unsigned long long address,
                             unsigned long long bytes,
                             unsigned int entry_bytes,
                             unsigned int version,
                             int variable_entries)
{
    if (!mutable_record()) return ZLOS_BOOT_E_STATE;
    if (!address || !bytes) return ZLOS_BOOT_E_ARGUMENT;
    current.memory_map_address = address;
    current.memory_map_bytes = bytes;
    current.memory_map_entry_bytes = entry_bytes;
    current.memory_map_version = version;
    current.flags |= ZLOS_BOOT_F_MEMORY_MAP;
    if (variable_entries) current.flags |= ZLOS_BOOT_F_VARIABLE_MAP_ENTRIES;
    else current.flags &= ~ZLOS_BOOT_F_VARIABLE_MAP_ENTRIES;
    return ZLOS_BOOT_OK;
}

int zlos_boot_set_framebuffer(unsigned long long address,
                              unsigned long long bytes,
                              unsigned int pitch, unsigned int width,
                              unsigned int height, unsigned int bpp,
                              unsigned int pixel_format)
{
    if (!mutable_record()) return ZLOS_BOOT_E_STATE;
    if (!address || !bytes) return ZLOS_BOOT_E_ARGUMENT;
    current.framebuffer_address = address;
    current.framebuffer_bytes = bytes;
    current.framebuffer_pitch = pitch;
    current.framebuffer_width = width;
    current.framebuffer_height = height;
    current.framebuffer_bpp = bpp;
    current.framebuffer_pixel_format = pixel_format;
    current.flags |= ZLOS_BOOT_F_FRAMEBUFFER;
    return ZLOS_BOOT_OK;
}

int zlos_boot_mark_firmware_retired(void)
{
    if (!mutable_record()) return ZLOS_BOOT_E_STATE;
    current.flags |= ZLOS_BOOT_F_FIRMWARE_RETIRED;
    return ZLOS_BOOT_OK;
}

int zlos_boot_set_previous_failure(unsigned int reason)
{
    if (!mutable_record()) return ZLOS_BOOT_E_STATE;
    current.previous_failure = reason;
    return ZLOS_BOOT_OK;
}

static int set_identity(unsigned char destination[32],
                        const unsigned char digest[32], unsigned int flag)
{
    if (!mutable_record() || !digest) return ZLOS_BOOT_E_STATE;
    if (!bytes_nonzero(digest, 32)) return ZLOS_BOOT_E_ARGUMENT;
    for (unsigned int i = 0; i < 32; i++) destination[i] = digest[i];
    current.flags |= flag;
    return ZLOS_BOOT_OK;
}

int zlos_boot_set_origin_identity(const unsigned char digest[32])
{
    return set_identity(current.origin_identity, digest,
                        ZLOS_BOOT_F_EXACT_ORIGIN);
}

int zlos_boot_set_kernel_identity(const unsigned char digest[32])
{
    return set_identity(current.kernel_identity, digest,
                        ZLOS_BOOT_F_EXACT_KERNEL);
}

int zlos_boot_seal(void)
{
    int status = validate_shape(&current, 0);
    if (status != ZLOS_BOOT_OK) return status;
    current.flags |= ZLOS_BOOT_F_SEALED;
    current.checksum = zlos_boot_checksum(&current);
    return zlos_boot_validate(&current);
}

const struct zlos_boot_handover *zlos_boot_record(void) { return &current; }
int zlos_boot_is_valid(void)
{
    return zlos_boot_validate(&current) == ZLOS_BOOT_OK;
}

int zlos_boot_from_multiboot(zlos_boot_uptr address,
                             unsigned int architecture_bits)
{
    if (!address) return ZLOS_BOOT_E_ARGUMENT;
    int status = zlos_boot_begin(ZLOS_BOOT_ORIGIN_MULTIBOOT1,
                                 architecture_bits,
                                 ZLOS_BOOT_GENERATION_CURRENT, 1, 3);
    if (status != ZLOS_BOOT_OK) return status;
    zlos_boot_set_source((unsigned long long)address);
    struct zlos_mb1_info *mb = (struct zlos_mb1_info *)address;
    if ((mb->flags & ZLOS_MB_F_MEMORY_MAP) &&
        mb->mmap_addr && mb->mmap_length) {
        status = zlos_boot_set_memory_map(mb->mmap_addr, mb->mmap_length,
                                          0, 1, 1);
        if (status != ZLOS_BOOT_OK) return status;
    }
    if ((mb->flags & ZLOS_MB_F_FRAMEBUFFER) &&
        mb->framebuffer_type == ZLOS_MB_FB_RGB &&
        mb->framebuffer_addr && mb->framebuffer_pitch &&
        mb->framebuffer_width && mb->framebuffer_height) {
        unsigned long long bytes =
            (unsigned long long)mb->framebuffer_pitch *
            mb->framebuffer_height;
        status = zlos_boot_set_framebuffer(
            mb->framebuffer_addr, bytes, mb->framebuffer_pitch,
            mb->framebuffer_width, mb->framebuffer_height,
            mb->framebuffer_bpp, ZLOS_BOOT_PIXEL_UNKNOWN);
        if (status != ZLOS_BOOT_OK) return status;
    }
    status = zlos_boot_mark_firmware_retired();
    if (status != ZLOS_BOOT_OK) return status;
    return zlos_boot_seal();
}

int zlos_boot_from_raw(unsigned long long framebuffer_address,
                       unsigned int pitch, unsigned int width,
                       unsigned int height, unsigned int bpp)
{
    int status = zlos_boot_begin(ZLOS_BOOT_ORIGIN_RAW_BIOS, 32,
                                 ZLOS_BOOT_GENERATION_CURRENT, 1, 3);
    if (status != ZLOS_BOOT_OK) return status;
    /* 0x8300 is the bounded VBE fact block written by raw_boot.asm. */
    zlos_boot_set_source(0x8300ULL);
    if (framebuffer_address && pitch && width && height) {
        unsigned long long bytes = (unsigned long long)pitch * height;
        status = zlos_boot_set_framebuffer(
            framebuffer_address, bytes, pitch, width, height, bpp,
            ZLOS_BOOT_PIXEL_UNKNOWN);
        if (status != ZLOS_BOOT_OK) return status;
    }
    status = zlos_boot_mark_firmware_retired();
    if (status != ZLOS_BOOT_OK) return status;
    return zlos_boot_seal();
}

int zlos_boot_loader_code(void)
{
    if (zlos_boot_validate(&current) != ZLOS_BOOT_OK) return -1;
    if (current.origin == ZLOS_BOOT_ORIGIN_RAW_BIOS) return 0;
    if (current.origin == ZLOS_BOOT_ORIGIN_MULTIBOOT1) return 1;
    if (current.origin == ZLOS_BOOT_ORIGIN_NATIVE_UEFI) return 2;
    return -1;
}
