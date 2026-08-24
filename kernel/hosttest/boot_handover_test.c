/* boot_handover_test.c - hostile proof for the versioned pre-kernel record. */
#include <stdio.h>
#include <string.h>

#include "../boot_handover.h"

struct fake_mb1_info {
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

static int checks;
static int failures;

static void ok(int condition, const char *name)
{
    checks++;
    if (!condition) {
        failures++;
        printf("  FAIL  %s\n", name);
    }
}

static void reseal(struct zlos_boot_handover *record)
{
    record->checksum = 0;
    record->checksum = zlos_boot_checksum(record);
}

int main(void)
{
    printf("boot_handover_test - one sealed record across every route\n\n");

    ok(sizeof(struct zlos_boot_handover) == ZLOS_BOOT_RECORD_BYTES,
       "record is exactly 256 bytes");
    ok(zlos_boot_begin(ZLOS_BOOT_ORIGIN_NATIVE_UEFI, 32,
                       ZLOS_BOOT_GENERATION_CURRENT, 1, 3) ==
       ZLOS_BOOT_E_UNSUPPORTED,
       "native UEFI refuses a 32-bit architecture claim");
    ok(zlos_boot_begin(ZLOS_BOOT_ORIGIN_RAW_BIOS, 32,
                       ZLOS_BOOT_GENERATION_CURRENT, 4, 3) ==
       ZLOS_BOOT_E_RANGE,
       "attempt ordinal cannot exceed the recovery budget");

    ok(zlos_boot_from_raw(0xE0000000ULL, 4096, 1024, 768, 32) == ZLOS_BOOT_OK,
       "raw BIOS route constructs and seals a valid record");
    const struct zlos_boot_handover *raw = zlos_boot_record();
    ok(zlos_boot_validate(raw) == ZLOS_BOOT_OK, "raw record validates");
    ok(zlos_boot_is_valid(), "global validity gate accepts the sealed record");
    ok(raw->origin == ZLOS_BOOT_ORIGIN_RAW_BIOS,
       "raw record states its exact route kind");
    ok(raw->framebuffer_bytes == 4096ULL * 768ULL,
       "raw framebuffer span is derived from pitch and height");
    ok((raw->flags & ZLOS_BOOT_F_FIRMWARE_RETIRED) != 0,
       "raw BIOS services are explicitly retired before kernel entry");
    ok(zlos_boot_loader_code() == 0, "raw compatibility loader code is 0");
    ok(zlos_boot_set_source(7) == ZLOS_BOOT_E_STATE,
       "a sealed record is immutable");

    /* Every byte is covered. This intentionally accepts any non-OK result:
     * header mutations are rejected by the version check before the checksum,
     * while payload mutations reach the checksum check. */
    for (unsigned int byte = 0; byte < sizeof(*raw); byte++) {
        struct zlos_boot_handover changed = *raw;
        ((unsigned char *)&changed)[byte] ^= 1U;
        if (zlos_boot_validate(&changed) == ZLOS_BOOT_OK) {
            failures++;
            printf("  FAIL  byte %u is outside validation/checksum coverage\n",
                   byte);
        }
        checks++;
    }

    struct zlos_boot_handover hostile = *raw;
    hostile.flags |= 1U << 31;
    reseal(&hostile);
    ok(zlos_boot_validate(&hostile) == ZLOS_BOOT_E_UNSUPPORTED,
       "unknown mandatory flags are rejected even with a valid checksum");

    hostile = *raw;
    hostile.framebuffer_bytes--;
    reseal(&hostile);
    ok(zlos_boot_validate(&hostile) == ZLOS_BOOT_E_RANGE,
       "a framebuffer one byte shorter than pitch times height is rejected");

    hostile = *raw;
    hostile.reserved[55] = 1;
    reseal(&hostile);
    ok(zlos_boot_validate(&hostile) == ZLOS_BOOT_E_UNSUPPORTED,
       "non-zero reserved data is rejected, not silently reinterpreted");

    hostile = *raw;
    hostile.flags |= ZLOS_BOOT_F_EXACT_KERNEL;
    reseal(&hostile);
    ok(zlos_boot_validate(&hostile) == ZLOS_BOOT_E_RANGE,
       "an exact-kernel claim needs an actual identity");

    struct fake_mb1_info mb;
    memset(&mb, 0, sizeof(mb));
    mb.flags = (1U << 6) | (1U << 12);
    mb.mmap_addr = 0x5000;
    mb.mmap_length = 128;
    mb.framebuffer_addr = 0xD0000000ULL;
    mb.framebuffer_pitch = 3200;
    mb.framebuffer_width = 800;
    mb.framebuffer_height = 600;
    mb.framebuffer_bpp = 32;
    mb.framebuffer_type = 1;
    ok(zlos_boot_from_multiboot((zlos_boot_uptr)&mb, 64) == ZLOS_BOOT_OK,
       "Multiboot route translates its foreign structure into the record");
    const struct zlos_boot_handover *multi = zlos_boot_record();
    ok(multi->origin == ZLOS_BOOT_ORIGIN_MULTIBOOT1,
       "Multiboot origin retained");
    ok((multi->flags & ZLOS_BOOT_F_VARIABLE_MAP_ENTRIES) != 0,
       "Multiboot variable-length memory entries are explicit");
    ok(multi->memory_map_address == 0x5000 && multi->memory_map_bytes == 128,
       "Multiboot memory-map range retained");
    ok(zlos_boot_loader_code() == 1, "Multiboot compatibility code is 1");

    mb.framebuffer_width = 0xFFFFFFFFU;
    ok(zlos_boot_from_multiboot((zlos_boot_uptr)&mb, 64) == ZLOS_BOOT_E_RANGE,
       "hostile Multiboot framebuffer geometry fails closed");
    ok(!zlos_boot_is_valid(), "failed construction cannot leave a valid record");

    unsigned char digest[32];
    for (unsigned int i = 0; i < sizeof(digest); i++) digest[i] = (unsigned char)(i + 1);
    ok(zlos_boot_begin(ZLOS_BOOT_ORIGIN_NATIVE_UEFI, 64,
                       ZLOS_BOOT_GENERATION_RECOVERY, 2, 3) == ZLOS_BOOT_OK,
       "a bounded recovery-generation record can begin");
    ok(zlos_boot_set_source(0x100000) == ZLOS_BOOT_OK,
       "UEFI selected image base retained");
    ok(zlos_boot_set_firmware_root(0x7F000) == ZLOS_BOOT_OK,
       "UEFI ACPI root retained");
    ok(zlos_boot_set_memory_map(0x800000, 4096, 48, 1, 0) == ZLOS_BOOT_OK,
       "UEFI fixed-size memory map retained");
    ok(zlos_boot_set_framebuffer(0xC0000000ULL, 1920ULL * 1080ULL * 4ULL,
                                  1920 * 4, 1920, 1080, 32,
                                  ZLOS_BOOT_PIXEL_BGRX8888) == ZLOS_BOOT_OK,
       "UEFI GOP facts retained");
    ok(zlos_boot_set_origin_identity(digest) == ZLOS_BOOT_OK,
       "exact origin identity can be attached");
    digest[0] ^= 0xA5;
    ok(zlos_boot_set_kernel_identity(digest) == ZLOS_BOOT_OK,
       "exact selected-kernel identity can be attached");
    ok(zlos_boot_mark_firmware_retired() == ZLOS_BOOT_OK,
       "firmware retirement is an explicit fact");
    ok(zlos_boot_seal() == ZLOS_BOOT_OK,
       "complete native UEFI recovery record seals");
    ok(zlos_boot_loader_code() == 2, "native UEFI compatibility code is 2");

    printf("\n  %d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
