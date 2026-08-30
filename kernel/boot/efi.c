/* efi.c - zlOS as a UEFI application. No GRUB.
 *
 * Our own BIOS bootloader (raw_boot.asm) cannot boot a modern laptop, because
 * modern laptops are UEFI and there is no BIOS to call. That is the only
 * reason GRUB was ever in this project. This removes it.
 *
 * The trick is that there is no separate loader at all: the KERNEL ITSELF is
 * the UEFI application. The firmware loads EFI/BOOT/BOOTX64.EFI, calls
 * efi_main below, and we are already running - in 64-bit long mode, with
 * paging set up by the firmware, on a machine whose framebuffer we can simply
 * ask for. Going 64-bit is what made this easy: a UEFI application is x86-64
 * by definition, so there is no mode switch to perform and no page tables to
 * build. Compare boot64.S, which has to do both by hand.
 *
 * Two things must happen before we own the machine:
 *   1. ask the Graphics Output Protocol where the framebuffer is
 *   2. call ExitBootServices, after which the firmware is gone for good and
 *      every service pointer above becomes invalid
 *
 * ABI NOTE: UEFI uses the Microsoft x64 calling convention, while the rest of
 * this kernel is System V. Every firmware function pointer is therefore
 * declared __attribute__((ms_abi)) so the compiler emits the right call for
 * each side. Getting this wrong corrupts arguments in a way that looks random.
 */
#include "boot_handover.h"

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;
typedef u64                efi_status;
typedef void              *efi_handle;

#define EFI_SUCCESS 0
#define EFI_UNSUPPORTED      0x8000000000000003ULL
#define EFI_OUT_OF_RESOURCES 0x8000000000000009ULL
#define EFI_ABORTED          0x8000000000000015ULL
#define EFI_ALLOCATE_MAX_ADDRESS 1U
#define EFI_LOADER_DATA 2U
#define MS __attribute__((ms_abi))
#define EFI_FILE_MODE_READ   0x0000000000000001ULL
#define EFI_FILE_MODE_WRITE  0x0000000000000002ULL
#define EFI_FILE_MODE_CREATE 0x8000000000000000ULL
#define EFI_FILE_POSITION_END 0xffffffffffffffffULL
#define EFI_MEMORY_WC         0x0000000000000002ULL
#define WITNESS_SIZE_LIMIT (64ULL * 1024ULL)
#define FIXED_ARENA_START 0x00E00000ULL
#define FIXED_ARENA_END   0x01E00000ULL
#define FIXED_ZL_START    0x02000000ULL
#define FIXED_ZL_END      0x02100000ULL
#define FIXED_IMAGE_START 0x03000000ULL
#define FIXED_IMAGE_END   0x03401000ULL
#define FIXED_NET_START   0x04000000ULL
#define FIXED_NET_END     0x04100000ULL
#define FIXED_DOM_START   0x05000000ULL
#define FIXED_DOM_END     0x06000000ULL
#define FIXED_HIGH_START  0x08000000ULL
#define FIXED_HIGH_END    0x14000000ULL
#define HI_BACK_START     0x08000000ULL
#define HI_BACK_END       0x0A800000ULL

/* ---- the tiny slice of the UEFI spec we actually use -------------------- */
typedef struct {
    u32 type;
    u32 pad;
    u64 physical_start;
    u64 virtual_start;
    u64 number_of_pages;
    u64 attribute;
} efi_memory_descriptor;

typedef struct {
    u64 signature;
    u32 revision;
    u32 header_size;
    u32 crc32;
    u32 reserved;
} efi_table_header;

/* Runtime services remain callable after ExitBootServices. zlOS stays in the
 * flat physical addressing mode UEFI entered with and does not call
 * SetVirtualAddressMap, so these pointers keep their physical values. The
 * only post-exit service used is SetVariable: one bounded diagnostic record
 * when the USB driver itself is too broken to persist ZLLOG. */
typedef struct {
    efi_table_header hdr;
    void *get_time, *set_time, *get_wakeup_time, *set_wakeup_time;
    void *set_virtual_address_map, *convert_pointer;
    void *get_variable, *get_next_variable_name;
    efi_status (MS *set_variable)(u16 *name, void *vendor_guid,
                                  u32 attributes, u64 data_size, void *data);
    void *get_next_high_mono_count, *reset_system, *update_capsule;
    void *query_capsule_capabilities, *query_variable_info;
} efi_runtime_services;

/* Only the entries we call are named; the rest are padding of the right size,
 * because the offsets are fixed by the spec and a wrong one calls the wrong
 * function. */
typedef struct {
    efi_table_header hdr;
    void *raise_tpl, *restore_tpl;
    efi_status (MS *allocate_pages)(u32 type, u32 memory_type,
                                    u64 pages, u64 *memory);
    void *free_pages;
    efi_status (MS *get_memory_map)(u64 *size, efi_memory_descriptor *map,
                                    u64 *key, u64 *desc_size, u32 *desc_ver);
    void *allocate_pool, *free_pool;
    void *create_event, *set_timer, *wait_for_event, *signal_event,
         *close_event, *check_event;
    void *install_protocol_interface, *reinstall_protocol_interface,
         *uninstall_protocol_interface;
    efi_status (MS *handle_protocol)(efi_handle handle, void *protocol,
                                    void **interface);
    void *reserved2, *register_protocol_notify, *locate_handle,
         *locate_device_path, *install_configuration_table;
    void *load_image, *start_image, *exit;
    void *unload_image;
    efi_status (MS *exit_boot_services)(efi_handle image, u64 map_key);
} efi_boot_services;

typedef struct {
    efi_table_header hdr;
    u16 *firmware_vendor;
    u32 firmware_revision;
    efi_handle console_in_handle;
    void *con_in;
    efi_handle console_out_handle;
    void *con_out;
    efi_handle standard_error_handle;
    void *std_err;
    efi_runtime_services *runtime_services;
    efi_boot_services *boot_services;
    /* The configuration table is how UEFI hands over ACPI. There is no BIOS
     * region to scan on a UEFI machine, so without these two fields the ACPI
     * tables are simply unreachable - and with them, so is the MADT, which is
     * the only thing that knows where the I/O APIC is and how the IRQs are
     * really wired. */
    unsigned long long n_config_entries;
    void *config_table;
} efi_system_table;

/* the Graphics Output Protocol - how a UEFI machine tells you where to draw */
typedef struct {
    u32 version;
    u32 horizontal_resolution;
    u32 vertical_resolution;
    u32 pixel_format;            /* 0 = RGBX8888, 1 = BGRX8888 */
    u32 pixel_information[4];
    u32 pixels_per_scan_line;    /* the stride, in PIXELS not bytes */
} efi_gop_mode_info;

typedef struct {
    u32 max_mode;
    u32 mode;
    efi_gop_mode_info *info;
    u64 size_of_info;
    u64 framebuffer_base;
    u64 framebuffer_size;
} efi_gop_mode;

typedef struct efi_gop {
    efi_status (MS *query_mode)(struct efi_gop *self, u32 n, u64 *size,
                                efi_gop_mode_info **info);
    efi_status (MS *set_mode)(struct efi_gop *self, u32 n);
    void *blt;
    efi_gop_mode *mode;
} efi_gop;

/* the firmware finds protocols by GUID */
typedef struct { u32 d1; u16 d2; u16 d3; u8 d4[8]; } efi_guid;
static efi_guid GOP_GUID =
    { 0x9042a9de, 0x23dc, 0x4a38, { 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } };
static efi_guid LOADED_IMAGE_GUID =
    { 0x5b1b31a1, 0x9562, 0x11d2,
      { 0x8e,0x3f,0x00,0xa0,0xc9,0x69,0x72,0x3b } };
static efi_guid SIMPLE_FS_GUID =
    { 0x964e5b22, 0x6459, 0x11d2,
      { 0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b } };
static efi_guid FILE_INFO_GUID =
    { 0x09576e92, 0x6d3f, 0x11d2,
      { 0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b } };
static efi_guid CPU_ARCH_GUID =
    { 0x26baccb1, 0x6f42, 0x11d4,
      { 0xbc,0xe7,0x00,0x80,0xc7,0x3c,0x88,0x81 } };
static efi_guid ZL_DIAG_GUID =
    { 0x8b1e7a9f, 0x5e2d, 0x4d39,
      { 0x9a,0x3c,0xcb,0x6f,0x92,0xe4,0xd5,0xa1 } };

/* LocateProtocol lives further into boot services than the fields above, so
 * it is reached by offset rather than by growing the struct: it is entry 37
 * of the table (0-based), i.e. 37 pointers past the header. */
typedef efi_status (MS *locate_protocol_fn)(efi_guid *proto, void *reg, void **iface);

/* PI 1.8, EFI_CPU_ARCH_PROTOCOL.  The first seven entries are deliberately
 * opaque: zlOS needs only SetMemoryAttributes, but its slot is fixed by the
 * protocol ABI and skipping an entry would call the wrong firmware function.
 * This call happens before ExitBootServices, while the protocol is alive. */
typedef struct efi_cpu_arch efi_cpu_arch;
struct efi_cpu_arch {
    void *flush_data_cache;
    void *enable_interrupt;
    void *disable_interrupt;
    void *get_interrupt_state;
    void *init;
    void *register_interrupt_handler;
    void *get_timer_value;
    efi_status (MS *set_memory_attributes)(efi_cpu_arch *self, u64 base,
                                            u64 length, u64 attributes);
    u32 number_of_timers;
    u32 dma_buffer_alignment;
};

/* The child kernel opens the same ESP that stage 0 came from and appends to
 * its bounded witness.  Every operation happens before ExitBootServices; no
 * firmware protocol is retained or called after a successful exit. */
typedef struct {
    u32 revision;
    efi_handle parent_handle;
    efi_system_table *system_table;
    efi_handle device_handle;
    void *file_path;
    void *reserved;
    u32 load_options_size;
    void *load_options;
    void *image_base;
    u64 image_size;
    u32 image_code_type;
    u32 image_data_type;
    void *unload;
} efi_loaded_image;

typedef struct efi_file efi_file;
struct efi_file {
    u64 revision;
    efi_status (MS *open)(efi_file *self, efi_file **new_handle,
                          u16 *file_name, u64 open_mode, u64 attributes);
    efi_status (MS *close)(efi_file *self);
    void *delete_file;
    void *read;
    efi_status (MS *write)(efi_file *self, u64 *size, void *buffer);
    void *get_position;
    efi_status (MS *set_position)(efi_file *self, u64 position);
    efi_status (MS *get_info)(efi_file *self, efi_guid *information_type,
                              u64 *buffer_size, void *buffer);
    void *set_info;
    efi_status (MS *flush)(efi_file *self);
};

typedef struct efi_simple_fs efi_simple_fs;
struct efi_simple_fs {
    u64 revision;
    efi_status (MS *open_volume)(efi_simple_fs *self, efi_file **root);
};

typedef struct {
    u64 size;
    u64 file_size;
    u64 physical_size;
} efi_file_info_prefix;

typedef struct {
    char data[192];
    unsigned length;
} witness_line;

static u16 WITNESS_PATH[] =
    { '\\','E','F','I','\\','Z','L','O','S','\\','W','I','T','N','E','S','S','.','L','O','G',0 };
static efi_file *witness_root;
static u64 boot_image_base;
static u64 boot_image_size;
static u64 witness_info_words[64];
static u16 ZL_DIAG_NAME[] =
    { 'Z','l','B','o','o','t','D','i','a','g',0 };
static efi_runtime_services *runtime_services;
#define DIAG_HEADER_BYTES 440U
#define DIAG_ENTRY_BYTES 104U
#define DIAG_MAX_PORTS 31U
#define DIAG_BUFFER_BYTES (DIAG_HEADER_BYTES + DIAG_MAX_PORTS * DIAG_ENTRY_BYTES + 4U)
_Static_assert(DIAG_BUFFER_BYTES <= 4096U,
               "ZlBootDiag must stay within the firmware variable budget");
static u8 runtime_diag[DIAG_BUFFER_BYTES];

/* xHCI's class scan owns these facts. The firmware fallback only serialises a
 * bounded snapshot after storage has refused its normal on-stick journal. */
extern int xhci_ports(void);
extern int xhci_port_connected(int port);
extern int xhci_port_enabled(int port);
extern int xhci_port_speed(int port);
extern u32 xhci_portsc(int port);
extern int xhci_enum_stage(int port);
extern int xhci_enum_cc(int port);
extern int xhci_msc_port_stage(int port);
extern int xhci_msc_port_slot(int port);
extern int xhci_msc_port_cc(int port);
extern int xhci_msc_port_vid(int port);
extern int xhci_msc_port_pid(int port);
extern int xhci_msc_port_candidate(int port);
extern int xhci_msc_port_ep0_event(int port);
extern int xhci_msc_port_ep0_attempts(int port);
extern int xhci_msc_port_ep0_recovery(int port);
extern u32 xhci_msc_port_ep0_trace(int port, int word);
extern u32 xhci_ep0_first_device_probe(int which, int word);
extern u32 xhci_ep0_first_device_slot_context(int which);
extern u32 xhci_ep0_first_config_diag(int word);
extern u32 xhci_lifecycle_diag(int word);
extern int xhci_version(void);
extern int xhci_msc_last_xhci_cc(void);
extern int xhci_msc_last_csw_status(void);
extern u32 xhci_msc_last_residue(void);
extern int xhci_msc_last_recovery(void);
extern int xhci_msc_last_opcode(void);
extern int xhci_msc_sense_valid(void);
extern int xhci_msc_sense_key(void);
extern int xhci_msc_sense_asc(void);
extern int xhci_msc_sense_ascq(void);
extern int xhci_msc_init_port(void);
extern int xhci_msc_init_slot(void);
extern int xhci_msc_init_cc(void);
extern int xhci_msc_init_vid(void);
extern int xhci_msc_init_pid(void);
extern u32 xhci_usbsts(void);
extern u32 xhci_usbcmd(void);

#define EFI_VARIABLE_NON_VOLATILE       0x00000001U
#define EFI_VARIABLE_BOOTSERVICE_ACCESS 0x00000002U
#define EFI_VARIABLE_RUNTIME_ACCESS     0x00000004U

static void diag_put16(u8 *p, u16 v)
{
    p[0] = (u8)v; p[1] = (u8)(v >> 8);
}

static void diag_put32(u8 *p, u32 v)
{
    p[0] = (u8)v; p[1] = (u8)(v >> 8);
    p[2] = (u8)(v >> 16); p[3] = (u8)(v >> 24);
}

static u32 diag_fnv32(const u8 *p, unsigned n)
{
    u32 h = 2166136261U;
    while (n--) { h ^= *p++; h *= 16777619U; }
    return h;
}

static efi_status runtime_diag_store(u32 state, u32 reason, u32 result,
                                     u32 stage, u32 port, u32 slot, u32 cc,
                                     u32 portsc, u32 usbsts, u32 usbcmd,
                                     u32 vidpid, u32 ticks)
{
    if (!runtime_services || !runtime_services->set_variable)
        return EFI_UNSUPPORTED;
    (void)portsc; /* selected port status is retained in its v2 port entry */
    for (unsigned i = 0; i < sizeof(runtime_diag); i++) runtime_diag[i] = 0;
    runtime_diag[0] = 'Z'; runtime_diag[1] = 'L';
    runtime_diag[2] = 'D'; runtime_diag[3] = 'I';
    runtime_diag[4] = 'A'; runtime_diag[5] = 'G';
    runtime_diag[6] = '6';
    diag_put16(runtime_diag + 8, 6);
    diag_put16(runtime_diag + 10, DIAG_HEADER_BYTES);
    diag_put32(runtime_diag + 16, state);
    diag_put32(runtime_diag + 20, reason);
    diag_put32(runtime_diag + 24, result);
    diag_put32(runtime_diag + 28, stage);
    diag_put32(runtime_diag + 32, port);
    diag_put32(runtime_diag + 36, slot);
    diag_put32(runtime_diag + 40, cc);
    diag_put32(runtime_diag + 44, usbsts);
    diag_put32(runtime_diag + 48, usbcmd);
    diag_put32(runtime_diag + 52, ticks);
    diag_put32(runtime_diag + 56, vidpid);

    unsigned count = 0;
    int ports = xhci_ports();
    if (ports > (int)DIAG_MAX_PORTS) ports = (int)DIAG_MAX_PORTS;
    for (int p = 1; p <= ports; p++) {
        int mstage = xhci_msc_port_stage(p);
        int estage = xhci_enum_stage(p);
        if (!mstage && !estage) continue;
        u8 *entry = runtime_diag + DIAG_HEADER_BYTES + count * DIAG_ENTRY_BYTES;
        entry[0] = (u8)p;
        entry[1] = (u8)mstage;
        entry[2] = (u8)xhci_msc_port_slot(p);
        entry[3] = (u8)xhci_msc_port_cc(p);
        entry[4] = (u8)estage;
        entry[5] = (u8)xhci_enum_cc(p);
        /* Low nibble is the xHCI speed ID. Bits 5:4 retain EP0 recovery as
         * 0=not attempted, 1=failed, 2=completed without growing the stable
         * 16-byte v3 entry. */
        int ep0_recovery = xhci_msc_port_ep0_recovery(p);
        u32 recovery_code = ep0_recovery < 0 ? 0U : (ep0_recovery ? 2U : 1U);
        entry[6] = (u8)(((u32)xhci_port_speed(p) & 0x0FU) |
                        ((recovery_code & 3U) << 4));
        entry[7] = (u8)((xhci_port_connected(p) ? 1U : 0U) |
                        (xhci_port_enabled(p) ? 2U : 0U) |
                        (xhci_msc_port_candidate(p) ? 4U : 0U) |
                        (((u32)xhci_msc_port_ep0_event(p) & 3U) << 3) |
                        (((u32)xhci_msc_port_ep0_attempts(p) & 3U) << 5));
        diag_put16(entry + 8, (u16)xhci_msc_port_vid(p));
        diag_put16(entry + 10, (u16)xhci_msc_port_pid(p));
        diag_put32(entry + 12, xhci_portsc(p));
        for (int word = 0; word < 22; word++)
            diag_put32(entry + 16U + (u32)word * 4U,
                       xhci_msc_port_ep0_trace(p, word));
        count++;
    }
    diag_put16(runtime_diag + 60, (u16)count);
    diag_put16(runtime_diag + 62, DIAG_ENTRY_BYTES);
    diag_put32(runtime_diag + 64, (u32)xhci_msc_last_xhci_cc());
    diag_put32(runtime_diag + 68, (u32)xhci_msc_last_csw_status());
    diag_put32(runtime_diag + 72, xhci_msc_last_residue());
    diag_put32(runtime_diag + 76, (u32)xhci_msc_last_recovery());
    diag_put32(runtime_diag + 80, (u32)xhci_msc_last_opcode());
    diag_put32(runtime_diag + 84, (u32)xhci_msc_sense_valid());
    runtime_diag[88] = (u8)xhci_msc_sense_key();
    runtime_diag[89] = (u8)xhci_msc_sense_asc();
    runtime_diag[90] = (u8)xhci_msc_sense_ascq();
    diag_put32(runtime_diag + 92, (u32)xhci_version());
    for (int which = 0; which < 2; which++)
        for (int word = 0; word < 22; word++)
            diag_put32(runtime_diag + 96U + (u32)which * 88U + (u32)word * 4U,
                       xhci_ep0_first_device_probe(which, word));
    for (int word = 0; word < 8; word++)
        diag_put32(runtime_diag + 272U + (u32)word * 4U,
                   xhci_lifecycle_diag(word));
    for (int which = 0; which < 2; which++)
        diag_put32(runtime_diag + 304U + (u32)which * 4U,
                   xhci_ep0_first_device_slot_context(which));
    for (int word = 0; word < 32; word++)
        diag_put32(runtime_diag + 312U + (u32)word * 4U,
                   xhci_ep0_first_config_diag(word));
    unsigned total = DIAG_HEADER_BYTES + count * DIAG_ENTRY_BYTES + 4U;
    diag_put32(runtime_diag + 12, total);
    diag_put32(runtime_diag + total - 4U, diag_fnv32(runtime_diag, total - 4U));
    return runtime_services->set_variable(
        ZL_DIAG_NAME, &ZL_DIAG_GUID,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS |
        EFI_VARIABLE_RUNTIME_ACCESS,
        total, runtime_diag);
}

/* Called weakly by zllog.c only when a changed mount failure is retained.
 * One NVRAM update per distinct boundary is the fallback for exactly the case
 * where xHCI mass storage cannot write its own USB journal. */
void efi_persist_storage_diag(u32 reason, u32 result, u32 stage, u32 port,
                              u32 slot, u32 cc, u32 portsc, u32 usbsts,
                              u32 usbcmd, u32 vid, u32 pid, u32 ticks)
{
    (void)runtime_diag_store(2, reason, result, stage, port, slot, cc,
                             portsc, usbsts, usbcmd,
                             ((vid & 0xffffU) << 16) | (pid & 0xffffU), ticks);
}

/* A refusal is only a point-in-time diagnostic. Automatic recovery can mount
 * the exact journal later, so publish that final state as well or Linux sees a
 * stale failure beside a USB journal which demonstrably contains the boot. */
void efi_persist_storage_ready(u32 ticks)
{
    u32 port = (u32)xhci_msc_init_port();
    u32 slot = (u32)xhci_msc_init_slot();
    u32 cc = (u32)xhci_msc_init_cc();
    u32 vid = (u32)xhci_msc_init_vid();
    u32 pid = (u32)xhci_msc_init_pid();
    (void)runtime_diag_store(
        3, 0, 0, 9, port, slot, cc,
        port ? xhci_portsc((int)port) : 0U, xhci_usbsts(), xhci_usbcmd(),
        ((vid & 0xffffU) << 16) | (pid & 0xffffU), ticks);
}

static void witness_text(witness_line *line, const char *text)
{
    while (*text && line->length + 1 < sizeof(line->data))
        line->data[line->length++] = *text++;
}

static void witness_hex(witness_line *line, u64 value)
{
    static const char digits[] = "0123456789ABCDEF";
    witness_text(line, "0x");
    for (unsigned i = 0; i < 16 && line->length + 1 < sizeof(line->data); i++) {
        unsigned shift = (15U - i) * 4U;
        line->data[line->length++] = digits[(value >> shift) & 15U];
    }
}

static void witness_dec(witness_line *line, u64 value)
{
    char reverse[24];
    unsigned n = 0;
    if (!value) {
        witness_text(line, "0");
        return;
    }
    while (value) {
        reverse[n++] = (char)('0' + value % 10);
        value /= 10;
    }
    while (n && line->length + 1 < sizeof(line->data))
        line->data[line->length++] = reverse[--n];
}

static witness_line witness_begin(const char *marker)
{
    witness_line line;
    line.length = 0;
    witness_text(&line, marker);
    return line;
}

static void witness_append_line(witness_line *line)
{
    if (!witness_root || !line || !line->length) return;
    if (line->length + 1 < sizeof(line->data)) line->data[line->length++] = '\n';
    line->data[line->length] = 0;

    efi_file *file = 0;
    efi_status status = witness_root->open(
        witness_root, &file, WITNESS_PATH,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (status != EFI_SUCCESS || !file) return;

    u64 info_size = sizeof(witness_info_words);
    status = file->get_info(file, &FILE_INFO_GUID, &info_size,
                            witness_info_words);
    u64 existing = status == EFI_SUCCESS &&
                   info_size >= sizeof(efi_file_info_prefix)
        ? ((efi_file_info_prefix *)witness_info_words)->file_size
        : WITNESS_SIZE_LIMIT;
    if (existing > WITNESS_SIZE_LIMIT ||
        line->length > WITNESS_SIZE_LIMIT - existing) {
        file->close(file);
        return;
    }

    status = file->set_position(file, EFI_FILE_POSITION_END);
    if (status == EFI_SUCCESS) {
        u64 size = line->length;
        status = file->write(file, &size, line->data);
        if (status == EFI_SUCCESS && size == line->length) file->flush(file);
    }
    file->close(file);
}

static void witness_marker(const char *marker)
{
    witness_line line = witness_begin(marker);
    witness_append_line(&line);
}

static void witness_init(efi_handle image, efi_system_table *st)
{
    if (!st || !st->boot_services || !st->boot_services->handle_protocol)
        return;
    efi_boot_services *bs = st->boot_services;
    efi_loaded_image *loaded = 0;
    efi_simple_fs *fs = 0;
    if (bs->handle_protocol(image, &LOADED_IMAGE_GUID, (void **)&loaded) !=
            EFI_SUCCESS || !loaded)
        return;
    boot_image_base = (u64)loaded->image_base;
    boot_image_size = loaded->image_size;
    if (bs->handle_protocol(loaded->device_handle, &SIMPLE_FS_GUID,
                            (void **)&fs) != EFI_SUCCESS || !fs)
        return;
    if (fs->open_volume(fs, &witness_root) != EFI_SUCCESS || !witness_root)
        return;
    witness_marker("KERNEL_ENTRY");
    {
        witness_line line = witness_begin("KERNEL_IMAGE base=");
        witness_hex(&line, (u64)loaded->image_base);
        witness_text(&line, " size=");
        witness_dec(&line, loaded->image_size);
        witness_text(&line, " end=");
        u64 base = (u64)loaded->image_base;
        u64 end = loaded->image_size > ~base ? ~0ULL : base + loaded->image_size;
        witness_hex(&line, end);
        witness_append_line(&line);
    }
}

/* The native runtime cannot use ELF linker-script symbols in PE/COFF. Publish
 * the exact relocated range supplied by EFI_LOADED_IMAGE_PROTOCOL instead. */
u64 efi_kernel_image_start(void) { return boot_image_base; }
u64 efi_kernel_image_end(void)
{
    return boot_image_size > ~boot_image_base ? ~0ULL
                                               : boot_image_base + boot_image_size;
}

static int fixed_arena_overlap(u64 start, u64 end)
{
    return (end > FIXED_ARENA_START && start < FIXED_ARENA_END) ||
           (end > FIXED_ZL_START && start < FIXED_ZL_END) ||
           (end > FIXED_IMAGE_START && start < FIXED_IMAGE_END) ||
           (end > FIXED_NET_START && start < FIXED_NET_END) ||
           (end > FIXED_DOM_START && start < FIXED_DOM_END) ||
           (end > FIXED_HIGH_START && start < FIXED_HIGH_END);
}

/* EfiBootServicesCode/Data and EfiConventionalMemory are released to the OS
 * after a successful ExitBootServices. Loader memory is not accepted here: it
 * can contain this image or the allocated compositor buffer. Runtime, ACPI,
 * reserved, unusable and MMIO descriptors are also hard collisions. */
static int fixed_type_reclaimable(u32 type)
{
    return type == 3 || type == 4 || type == 7;
}

/* Enforce firmware ownership of each fixed physical range zlOS actually uses.
 * The low map has deliberate gaps; the high map is reserved from the legacy
 * backbuffer through the 64 MiB heap ending at 320 MiB. The explicit HI_BACK
 * flag remains useful on non-UEFI paths, even though UEFI allocates its
 * compositor backbuffer through firmware. */
static int witness_fixed_memory(efi_memory_descriptor *map, u64 map_size,
                                u64 descriptor_size)
{
    if (!map || descriptor_size < sizeof(efi_memory_descriptor) ||
        map_size < descriptor_size) {
        witness_marker("FIXED_MEMORY unavailable");
        return 0;
    }

    unsigned emitted = 0;
    int safe = 1;
    for (u64 offset = 0; offset <= map_size - descriptor_size;
         offset += descriptor_size) {
        efi_memory_descriptor *d =
            (efi_memory_descriptor *)((u8 *)map + offset);
        u64 start = d->physical_start;
        u64 bytes = d->number_of_pages > (~0ULL >> 12)
            ? ~0ULL : d->number_of_pages << 12;
        u64 end = bytes > ~start ? ~0ULL : start + bytes;
        if (!fixed_arena_overlap(start, end)) continue;

        if (!fixed_type_reclaimable(d->type)) safe = 0;
        if (emitted < 32) {
            witness_line line = witness_begin("FIXED_MEMORY type=");
            witness_dec(&line, d->type);
            witness_text(&line, " start=");
            witness_hex(&line, start);
            witness_text(&line, " end=");
            witness_hex(&line, end);
            witness_text(&line, " reclaimable=");
            witness_text(&line, fixed_type_reclaimable(d->type) ? "yes" : "NO");
            witness_text(&line, " hi_back_overlap=");
            witness_text(&line,
                end > HI_BACK_START && start < HI_BACK_END ? "yes" : "no");
            witness_append_line(&line);
            emitted++;
        }
    }
    if (!emitted) witness_marker("FIXED_MEMORY no_fixed_range_descriptors");
    witness_marker(safe ? "FIXED_MEMORY safe" : "FIXED_MEMORY REFUSED");
    return safe;
}

/* The Microsoft toolchain emits a reference to this whenever a translation
 * unit uses floating point, and every zl number is a double. It is a marker,
 * not code - the linker only wants it to exist. */
int _fltused = 0;

/* The Microsoft x64 toolchain calls __chkstk before any function with a stack
 * frame larger than a page, so it can touch each page and let the OS grow the
 * stack. There is no OS beneath us and our stack is one fully-mapped block, so
 * there is nothing to probe - the contract is only that it returns with RAX
 * intact and lets the caller do its own `sub rsp`. The zl compiler emits very
 * large frames (main's is ~13 KiB), which is why this is needed at all. */
__asm__(".globl __chkstk\n"
        "__chkstk:\n"
        "    ret\n");

/* clang lowers struct copies and array initialisation to these regardless of
 * -ffreestanding, and there is no libc to supply them. */
void *memset(void *d, int c, unsigned long n)
{
    unsigned char *p = (unsigned char *)d;
    while (n--) *p++ = (unsigned char)c;
    return d;
}
void *memcpy(void *d, const void *s, unsigned long n)
{
    unsigned char *a = (unsigned char *)d;
    const unsigned char *b = (const unsigned char *)s;
    while (n--) *a++ = *b++;
    return d;
}

/* An entry is a 16-byte GUID followed by a pointer. */
typedef struct { efi_guid guid; void *table; } efi_config_entry;

extern void acpi_set_rsdp(unsigned long long addr);

static int guid_eq(efi_guid *g, u32 d1, u16 d2, u16 d3,
                   u8 a, u8 b, u8 c, u8 d, u8 e, u8 f, u8 gg, u8 h)
{
    return g->d1 == d1 && g->d2 == d2 && g->d3 == d3 &&
           g->d4[0] == a && g->d4[1] == b && g->d4[2] == c && g->d4[3] == d &&
           g->d4[4] == e && g->d4[5] == f && g->d4[6] == gg && g->d4[7] == h;
}

/* Find the ACPI root pointer and hand it to the APIC driver. ACPI 2.0 is
 * preferred because only its RSDP carries the XSDT; the 1.0 GUID is accepted
 * as a fallback for firmware that offers nothing else. */
static u64 capture_acpi(efi_system_table *st)
{
    if (!st->config_table) return 0;
    efi_config_entry *e = (efi_config_entry *)st->config_table;
    unsigned long long found_1 = 0;

    for (unsigned long long i = 0; i < st->n_config_entries; i++) {
        if (guid_eq(&e[i].guid, 0x8868e871, 0xe4f1, 0x11d3,
                    0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81)) {
            /* No intermediate `unsigned long`: it is 4 bytes in this build, and
             * firmware is free to place the RSDP above 4 GiB. Truncating it
             * would hand ACPI/APIC/SMP discovery a wild pointer. */
            acpi_set_rsdp((u64)e[i].table);
            return (u64)e[i].table;                    /* ACPI 2.0+ - best */
        }
        if (guid_eq(&e[i].guid, 0xeb9d2d30, 0x2d88, 0x11d3,
                    0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d))
            found_1 = (u64)e[i].table;                    /* same truncation */
    }
    if (found_1) acpi_set_rsdp(found_1);
    return found_1;
}

/* ---- what the kernel proper needs --------------------------------------- */
void serial_init(void);
void console_init_fb(unsigned long long addr, unsigned int pitch, unsigned int width,
                     unsigned int height, unsigned int bpp);
void console_init_efi(unsigned long long addr, unsigned int pitch, unsigned int width,
                      unsigned int height, unsigned int bpp);
void fb_set_back_buffer(unsigned long long addr, unsigned int bytes);
void console_init(unsigned long mb_addr);
int  main(void);
void kernel_done(void);

/* The only way to say anything from here. It runs after console_init_efi(),
 * so it reaches the screen and the serial line exactly like every other line
 * of the boot log. */
void zl_putc_pub(char c);
static void efi_say(const char *s) { while (*s) zl_putc_pub(*s++); }

/* Remember the mode across ExitBootServices. Everything the firmware owns
 * becomes invalid the moment that call returns, so anything we still need has
 * to be plain numbers in our own memory before then. */
/* THE GOP BASE IS 64 BITS AND SO IS THIS. It used to be `unsigned long`, and
 * buildefi.sh targets x86_64-unknown-windows, which is LLP64: `unsigned long`
 * is FOUR bytes there while a pointer is eight. The explicit cast below then
 * threw away the top half of a UINT64 silently, and the four -Werror flags in
 * buildefi.sh do not catch it - they catch pointer<->int, and this is a UINT64
 * narrowed by a cast the programmer wrote on purpose. T-11. */
static unsigned long long fb_addr;
static unsigned int  fb_w, fb_h, fb_pitch_bytes;
static unsigned long long fb_back_addr;
static unsigned int fb_back_bytes;

MS efi_status efi_main(efi_handle image, efi_system_table *st)
{
    /* This must be the first observable action in the real kernel image. */
    witness_init(image, st);
    if (!st || !st->boot_services) return 2;
    int boot_status = zlos_boot_begin(
        ZLOS_BOOT_ORIGIN_NATIVE_UEFI, 64,
        ZLOS_BOOT_GENERATION_CURRENT, 1, 3);
    if (boot_status != ZLOS_BOOT_OK) return EFI_ABORTED;
    if (boot_image_base)
        boot_status = zlos_boot_set_source(boot_image_base);
    efi_boot_services *bs = st->boot_services;
    runtime_services = st->runtime_services;
    {
        efi_status diag_status = runtime_diag_store(
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        witness_line line = witness_begin("EFI_DIAG_ARM status=");
        witness_hex(&line, diag_status);
        witness_append_line(&line);
    }

    /* Do this BEFORE ExitBootServices: the configuration table is the
     * firmware's memory and everything about it stops being valid after. */
    {
        witness_line line = witness_begin("ACPI_SCAN entries=");
        witness_dec(&line, st->n_config_entries);
        witness_text(&line, " table=");
        witness_hex(&line, (u64)st->config_table);
        witness_append_line(&line);
    }
    u64 rsdp = capture_acpi(st);
    if (boot_status == ZLOS_BOOT_OK && rsdp)
        boot_status = zlos_boot_set_firmware_root(rsdp);
    {
        witness_line line = witness_begin("ACPI_RESULT rsdp=");
        witness_hex(&line, rsdp);
        witness_append_line(&line);
    }

    /* ---- 1. find the framebuffer ---------------------------------------
     * UEFI has no VGA text mode and no BIOS to ask, so this protocol IS the
     * display. It also hands back a real mode list, which is strictly better
     * information than the VBE scan our BIOS bootloader has to do. */
    locate_protocol_fn locate_protocol =
        *(locate_protocol_fn *)((u8 *)bs + sizeof(efi_table_header) + 37 * 8);

    efi_gop *gop = 0;
    efi_status gop_status = locate_protocol(&GOP_GUID, 0, (void **)&gop);
    {
        witness_line line = witness_begin("GOP_RESULT status=");
        witness_hex(&line, gop_status);
        witness_text(&line, " interface=");
        witness_hex(&line, (u64)gop);
        witness_append_line(&line);
    }
    if (gop_status == EFI_SUCCESS && gop && gop->mode && gop->mode->info) {
        efi_gop_mode_info *mi = gop->mode->info;
        u64 pitch_pixels = mi->pixels_per_scan_line;
        u64 framebuffer_need = pitch_pixels * 4ULL * mi->vertical_resolution;
        u64 back_need = (u64)mi->horizontal_resolution *
                        mi->vertical_resolution * 4ULL;
        if (!mi->horizontal_resolution || !mi->vertical_resolution ||
            pitch_pixels < mi->horizontal_resolution ||
            pitch_pixels > 0x3fffffffULL ||
            framebuffer_need > gop->mode->framebuffer_size ||
            back_need > 40ULL * 1024ULL * 1024ULL) {
            witness_marker("GOP_DETAILS invalid_geometry_or_size");
            return EFI_UNSUPPORTED;
        }
        fb_addr        = gop->mode->framebuffer_base;
        fb_w           = mi->horizontal_resolution;
        fb_h           = mi->vertical_resolution;
        /* the stride is given in pixels; our renderer wants bytes */
        fb_pitch_bytes = mi->pixels_per_scan_line * 4;
        if (boot_status == ZLOS_BOOT_OK) {
            unsigned int pixel_format = mi->pixel_format == 0
                ? ZLOS_BOOT_PIXEL_RGBX8888
                : (mi->pixel_format == 1
                    ? ZLOS_BOOT_PIXEL_BGRX8888
                    : ZLOS_BOOT_PIXEL_UNKNOWN);
            boot_status = zlos_boot_set_framebuffer(
                fb_addr, framebuffer_need, fb_pitch_bytes, fb_w, fb_h, 32,
                pixel_format);
        }
        witness_line line = witness_begin("GOP_DETAILS width=");
        witness_dec(&line, fb_w);
        witness_text(&line, " height=");
        witness_dec(&line, fb_h);
        witness_text(&line, " pitch_bytes=");
        witness_dec(&line, fb_pitch_bytes);
        witness_text(&line, " framebuffer=");
        witness_hex(&line, fb_addr);
        witness_text(&line, " size=");
        witness_dec(&line, gop->mode->framebuffer_size);
        witness_append_line(&line);

        /* GOP framebuffers are PCI apertures.  Firmware left the ThinkPad's
         * 2560x1440 mapping uncacheable, and the physical recorder measured
         * the consequence: 67--137 ms in the final copy at about 112 MB/s.
         * Ask the platform's own CPU-architecture driver to make the exact
         * visible span write-combining before ExitBootServices.  This is the
         * PI-defined owner of PAT/MTRR/page-table coordination; open-coding a
         * PAT rewrite here would silently change every mapping that shares a
         * PAT index.  Failure is non-fatal: the already-correct UC path stays
         * live and the witness records the exact status for the next boot. */
        efi_cpu_arch *cpu_arch = 0;
        efi_status cpu_arch_status = locate_protocol(
            &CPU_ARCH_GUID, 0, (void **)&cpu_arch);
        efi_status wc_status = cpu_arch_status;
        if (cpu_arch_status == EFI_SUCCESS && cpu_arch &&
            cpu_arch->set_memory_attributes) {
            wc_status = cpu_arch->set_memory_attributes(
                cpu_arch, fb_addr, framebuffer_need, EFI_MEMORY_WC);
        }
        {
            witness_line wc_line = witness_begin("FRAMEBUFFER_WC locate=");
            witness_hex(&wc_line, cpu_arch_status);
            witness_text(&wc_line, " set=");
            witness_hex(&wc_line, wc_status);
            witness_text(&wc_line, " bytes=");
            witness_dec(&wc_line, framebuffer_need);
            witness_append_line(&wc_line);
        }

        /* The old EFI path drew into an unreserved physical 128..168 MiB
         * span.  A firmware is free to place this image, its stack or its page
         * tables there, so the first clear could erase the machine before our
         * IDT existed.  Ask UEFI for real pages below 4 GiB and keep them after
         * ExitBootServices.  Other boot paths retain the fixed-memory fallback
         * in fb.c until their loaders grow the same handoff. */
        u64 pages = (back_need + 4095ULL) >> 12;
        u64 memory = 0xffffffffULL;
        efi_status back_status = bs->allocate_pages(
            EFI_ALLOCATE_MAX_ADDRESS, EFI_LOADER_DATA, pages, &memory);
        {
            witness_line back_line = witness_begin("BACK_BUFFER status=");
            witness_hex(&back_line, back_status);
            witness_text(&back_line, " address=");
            witness_hex(&back_line, memory);
            witness_text(&back_line, " pages=");
            witness_dec(&back_line, pages);
            witness_append_line(&back_line);
        }
        if (back_status != EFI_SUCCESS || memory > 0xffffffffULL)
            return back_status != EFI_SUCCESS
                ? back_status : EFI_OUT_OF_RESOURCES;
        fb_back_addr = memory;
        fb_back_bytes = (unsigned int)(pages << 12);
        fb_set_back_buffer(fb_back_addr, fb_back_bytes);
    } else {
        witness_marker("GOP_DETAILS unavailable");
        return gop_status != EFI_SUCCESS ? gop_status : EFI_UNSUPPORTED;
    }

    /* ---- 2. take the machine ------------------------------------------
     * ExitBootServices needs the CURRENT memory map key, and the map can
     * change underneath us (even asking for its size can allocate), so the
     * documented dance is: get the map, try to exit, and if the key went
     * stale, get it again and retry. */
    /* 32 KiB, not 16: a laptop's map is bigger than a VM's, and if it does not
     * fit there is no way to take the machine at all. At 48 bytes a descriptor
     * this holds ~680 entries, against the 60-150 real firmware reports. */
    static u8 map[32768];
    u64 map_size, map_key = 0, desc_size = 0;
    u32 desc_ver;
    int exited = 0;
    int fixed_memory_recorded = 0;
    for (int attempt = 0; attempt < 4 && !exited; attempt++) {
        {
            witness_line line = witness_begin("MEMORY_MAP_ATTEMPT attempt=");
            witness_dec(&line, (u64)attempt + 1);
            witness_append_line(&line);
        }
        map_size = sizeof(map);
        efi_status map_status = bs->get_memory_map(
            &map_size, (efi_memory_descriptor *)map,
            &map_key, &desc_size, &desc_ver);
        {
            witness_line line = witness_begin("MEMORY_MAP_RESULT status=");
            witness_hex(&line, map_status);
            witness_text(&line, " bytes=");
            witness_dec(&line, map_size);
            witness_text(&line, " descriptor_size=");
            witness_dec(&line, desc_size);
            witness_append_line(&line);
        }
        if (map_status != EFI_SUCCESS) {
            continue;
        }
        if (!fixed_memory_recorded) {
            if (!witness_fixed_memory((efi_memory_descriptor *)map, map_size,
                                      desc_size))
                return EFI_OUT_OF_RESOURCES;
            fixed_memory_recorded = 1;
        }

        /* Appending the result may itself change the memory map. Publish the
         * last durable marker, then refresh the map/key without doing any more
         * filesystem work before ExitBootServices. */
        {
            witness_line line = witness_begin("BEFORE_EXIT_BOOT_SERVICES attempt=");
            witness_dec(&line, (u64)attempt + 1);
            witness_append_line(&line);
        }
        map_size = sizeof(map);
        map_status = bs->get_memory_map(
            &map_size, (efi_memory_descriptor *)map,
            &map_key, &desc_size, &desc_ver);
        if (map_status != EFI_SUCCESS) {
            witness_line line = witness_begin("MEMORY_MAP_REFRESH_FAILED status=");
            witness_hex(&line, map_status);
            witness_text(&line, " bytes=");
            witness_dec(&line, map_size);
            witness_append_line(&line);
            continue;
        }

        efi_status exit_status = bs->exit_boot_services(image, map_key);
        if (exit_status == EFI_SUCCESS) {
            /* Do not depend on the firmware's final interrupt-flag state.
             * Some UEFI implementations disable interrupts while exiting,
             * but after this success their IDT and handlers are no longer a
             * service zlOS may call.  Keep interrupts off through the gap
             * before setup_idt() installs our table.  idt_init() deliberately
             * executes sti only after our GDT, IDT and PIC routes are ready. */
            __asm__ volatile("cli" ::: "memory");
            exited = 1;
        } else {
            /* A failed exit leaves boot services valid, so recording it is
             * legal. A successful exit takes the other branch and performs
             * no firmware call or ESP write from that point onward. */
            witness_line line = witness_begin("EXIT_BOOT_SERVICES_FAILED status=");
            witness_hex(&line, exit_status);
            witness_append_line(&line);
        }
    }

    /* A failed exit leaves the firmware alive. Do not initialize serial, draw
     * into a framebuffer or install our platform tables on top of it. Returning
     * lets stage 0 persist START_IMAGE RETURN with an exact EFI status. */
    if (!exited) return EFI_ABORTED;

    /* From here the firmware no longer exists. We are the operating system:
     * still in long mode, still on the page tables UEFI built (identity
     * mapped, so our physical addresses stay valid), and nothing else is
     * running on this machine. */

    if (boot_status == ZLOS_BOOT_OK && desc_size > 0xffffffffULL)
        boot_status = ZLOS_BOOT_E_RANGE;
    if (boot_status == ZLOS_BOOT_OK)
        boot_status = zlos_boot_set_memory_map(
            (u64)map, map_size, (unsigned int)desc_size, desc_ver, 0);
    if (boot_status == ZLOS_BOOT_OK)
        boot_status = zlos_boot_mark_firmware_retired();
    if (boot_status == ZLOS_BOOT_OK)
        boot_status = zlos_boot_seal();

    serial_init();
    if (fb_addr) {
        console_init_efi(fb_addr, fb_pitch_bytes, fb_w, fb_h, 32);
    } else {
        /* There is no VGA text mode under UEFI, so this "fallback" reaches
         * nothing at all - the panel stays black while the serial log below
         * looks perfectly healthy, which is the most misleading failure this
         * kernel can produce. Say it plainly instead. */
        console_init(0);
        efi_say("\n  zlOS: firmware gave us NO GOP framebuffer.\n"
                "  UEFI has no text mode, so the screen stays black - "
                "everything below is serial only.\n");
    }

    if (boot_status != ZLOS_BOOT_OK ||
        zlos_boot_validate(zlos_boot_record()) != ZLOS_BOOT_OK) {
        efi_say("\n  zlOS: typed boot handover REFUSED; kernel not started.\n");
        kernel_done();
        for (;;) __asm__ volatile("hlt");
    }

    main();
    kernel_done();
    for (;;) __asm__ volatile("hlt");
    return EFI_SUCCESS;
}
