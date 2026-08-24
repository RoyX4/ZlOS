/* efi_stage0.c - the deliberately tiny, observable zlOS UEFI boundary.
 *
 * Firmware loads this file as EFI/BOOT/BOOTX64.EFI.  It proves entry before
 * the large kernel touches GOP, ACPI, the memory map, or ExitBootServices,
 * appends a durable witness to the ESP, then loads EFI/ZLOS/ZLOS.EFI from the
 * same USB and starts it.  There is no prompt and no manual command.
 *
 * This is diagnostic infrastructure, not a second bootloader policy layer:
 * every successful invocation immediately chains to the one real zlOS image.
 */

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;
typedef u64                efi_status;
typedef void              *efi_handle;

#define MS __attribute__((ms_abi))
#define EFI_SUCCESS 0ULL
#define EFI_ERROR_BIT 0x8000000000000000ULL
#define EFI_BAD_BUFFER_SIZE (EFI_ERROR_BIT | 4ULL)
#define EFI_FILE_MODE_READ   0x0000000000000001ULL
#define EFI_FILE_MODE_WRITE  0x0000000000000002ULL
#define EFI_FILE_MODE_CREATE 0x8000000000000000ULL
#define EFI_FILE_POSITION_END 0xffffffffffffffffULL
#define CHILD_SIZE_LIMIT (128ULL * 1024ULL * 1024ULL)
#define TRACE_SIZE_LIMIT (64ULL * 1024ULL)

typedef struct {
    u64 signature;
    u32 revision;
    u32 header_size;
    u32 crc32;
    u32 reserved;
} efi_table_header;

typedef struct { u32 d1; u16 d2; u16 d3; u8 d4[8]; } efi_guid;

typedef struct efi_text_output efi_text_output;
struct efi_text_output {
    void *reset;
    efi_status (MS *output_string)(efi_text_output *self, u16 *text);
    void *test_string;
    void *query_mode;
    void *set_mode;
    void *set_attribute;
    void *clear_screen;
    void *set_cursor_position;
    void *enable_cursor;
    void *mode;
};

typedef struct efi_boot_services {
    efi_table_header hdr;
    void *raise_tpl, *restore_tpl;
    void *allocate_pages, *free_pages, *get_memory_map;
    efi_status (MS *allocate_pool)(u32 pool_type, u64 size, void **buffer);
    efi_status (MS *free_pool)(void *buffer);
    void *create_event, *set_timer, *wait_for_event, *signal_event;
    void *close_event, *check_event;
    void *install_protocol_interface, *reinstall_protocol_interface;
    void *uninstall_protocol_interface;
    efi_status (MS *handle_protocol)(efi_handle handle, efi_guid *protocol,
                                    void **interface);
    void *reserved2, *register_protocol_notify, *locate_handle;
    void *locate_device_path, *install_configuration_table;
    efi_status (MS *load_image)(u8 boot_policy, efi_handle parent_image,
                                void *device_path, void *source_buffer,
                                u64 source_size, efi_handle *image_handle);
    efi_status (MS *start_image)(efi_handle image_handle, u64 *exit_data_size,
                                 u16 **exit_data);
    void *exit, *unload_image, *exit_boot_services;
    void *get_next_monotonic_count;
    efi_status (MS *stall)(u64 microseconds);
} efi_boot_services;

typedef struct {
    efi_table_header hdr;
    u16 *firmware_vendor;
    u32 firmware_revision;
    efi_handle console_in_handle;
    void *con_in;
    efi_handle console_out_handle;
    efi_text_output *con_out;
    efi_handle standard_error_handle;
    efi_text_output *std_err;
    void *runtime_services;
    efi_boot_services *boot_services;
    u64 n_config_entries;
    void *config_table;
} efi_system_table;

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
    efi_status (MS *delete_file)(efi_file *self);
    efi_status (MS *read)(efi_file *self, u64 *size, void *buffer);
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

static efi_guid LOADED_IMAGE_GUID =
    { 0x5b1b31a1, 0x9562, 0x11d2,
      { 0x8e,0x3f,0x00,0xa0,0xc9,0x69,0x72,0x3b } };
static efi_guid SIMPLE_FS_GUID =
    { 0x964e5b22, 0x6459, 0x11d2,
      { 0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b } };
static efi_guid FILE_INFO_GUID =
    { 0x09576e92, 0x6d3f, 0x11d2,
      { 0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b } };
static efi_guid DEVICE_PATH_GUID =
    { 0x09576e91, 0x6d3f, 0x11d2,
      { 0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b } };

static u16 TRACE_PATH[] =
    { '\\','E','F','I','\\','Z','L','O','S','\\','W','I','T','N','E','S','S','.','L','O','G',0 };
static u16 CHILD_PATH[] =
    { '\\','E','F','I','\\','Z','L','O','S','\\','Z','L','O','S','.','E','F','I',0 };

static efi_system_table *g_st;
static efi_file *g_root;
static u64 info_words[64];

/* A PE image with ImageBase 0 must be loadable anywhere.  Most of this tiny
 * file is RIP-relative and would otherwise need no fixups at all, producing
 * an empty relocation directory that stricter firmware loaders reject. */
void *efi_stage0_reloc_anchor = &efi_stage0_reloc_anchor;

static void say(u16 *text)
{
    if (g_st && g_st->con_out && g_st->con_out->output_string)
        g_st->con_out->output_string(g_st->con_out, text);
}

static unsigned text_len(const char *text)
{
    unsigned n = 0;
    while (text[n]) n++;
    return n;
}

static void append_trace(const char *text)
{
    if (!g_root) return;
    efi_file *file = 0;
    efi_status status = g_root->open(g_root, &file, TRACE_PATH,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (status != EFI_SUCCESS || !file) return;

    /* Reset normally keeps one boot only. If firmware refuses deletion, this
     * independent cap still guarantees an always-on witness cannot fill ESP. */
    u64 info_size = sizeof(info_words);
    status = file->get_info(file, &FILE_INFO_GUID, &info_size, info_words);
    u64 existing = status == EFI_SUCCESS
        ? ((efi_file_info_prefix *)info_words)->file_size : TRACE_SIZE_LIMIT;
    u64 size = text_len(text);
    if (existing > TRACE_SIZE_LIMIT || size > TRACE_SIZE_LIMIT - existing) {
        file->close(file);
        return;
    }

    /* Never overwrite an older boot's evidence if append positioning fails. */
    status = file->set_position(file, EFI_FILE_POSITION_END);
    if (status == EFI_SUCCESS) {
        status = file->write(file, &size, (void *)text);
        if (status == EFI_SUCCESS) file->flush(file);
    }
    file->close(file);
}

static void reset_trace(void)
{
    if (!g_root) return;
    efi_file *file = 0;
    efi_status status = g_root->open(g_root, &file, TRACE_PATH,
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    if (status == EFI_SUCCESS && file) {
        /* Delete closes the handle. append_trace recreates it immediately. */
        file->delete_file(file);
    }
}

static void bytes_copy(u8 *dst, const u8 *src, u64 size)
{
    while (size--) *dst++ = *src++;
}

/* Append a MEDIA_FILEPATH node to the partition device path. Passing the full
 * path with SourceBuffer=NULL makes LoadImage use the firmware's own standard
 * filesystem loader and preserves the child's real DeviceHandle/FilePath. */
static void *build_child_device_path(efi_boot_services *bs,
                                     efi_handle device_handle,
                                     efi_status *result)
{
    u8 *base = 0;
    *result = bs->handle_protocol(device_handle, &DEVICE_PATH_GUID,
                                  (void **)&base);
    if (*result != EFI_SUCCESS || !base) return 0;

    u64 prefix = 0;
    int found_end = 0;
    while (prefix < 4096) {
        u8 *node = base + prefix;
        u64 length = (u64)node[2] | ((u64)node[3] << 8);
        if (length < 4 || prefix + length > 4096) break;
        if (node[0] == 0x7f && node[1] == 0xff) {
            found_end = 1;
            break;
        }
        prefix += length;
    }
    if (!found_end) {
        *result = EFI_BAD_BUFFER_SIZE;
        return 0;
    }

    u64 chars = 0;
    while (CHILD_PATH[chars]) chars++;
    chars++;                         /* include UTF-16 NUL */
    u64 file_node_size = 4 + chars * 2;
    u64 total = prefix + file_node_size + 4;
    u8 *path = 0;
    *result = bs->allocate_pool(2U, total, (void **)&path);
    if (*result != EFI_SUCCESS || !path) return 0;

    bytes_copy(path, base, prefix);
    u8 *file_node = path + prefix;
    file_node[0] = 0x04;             /* MEDIA_DEVICE_PATH */
    file_node[1] = 0x04;             /* MEDIA_FILEPATH_DP */
    file_node[2] = (u8)(file_node_size & 0xff);
    file_node[3] = (u8)(file_node_size >> 8);
    bytes_copy(file_node + 4, (u8 *)CHILD_PATH, chars * 2);
    u8 *end = file_node + file_node_size;
    end[0] = 0x7f;                   /* END_DEVICE_PATH_TYPE */
    end[1] = 0xff;                   /* END_ENTIRE_DEVICE_PATH_SUBTYPE */
    end[2] = 4;
    end[3] = 0;
    return path;
}

static void status_hex(u16 *wide, char *ascii, efi_status status)
{
    static const char digits[] = "0123456789ABCDEF";
    for (unsigned i = 0; i < 16; i++) {
        unsigned shift = (15U - i) * 4U;
        char c = digits[(status >> shift) & 15U];
        wide[i] = (u16)c;
        ascii[i] = c;
    }
    wide[16] = 0;
    ascii[16] = 0;
}

static void report(u16 *screen_label, const char *trace_label, efi_status status)
{
    u16 whex[17];
    char ahex[17];
    char line[96];
    unsigned n = 0;

    status_hex(whex, ahex, status);
    say(screen_label);
    say(whex);
    {
        static u16 newline[] = { '\r','\n',0 };
        say(newline);
    }

    while (*trace_label && n + 1 < sizeof(line)) line[n++] = *trace_label++;
    for (unsigned i = 0; i < 16 && n + 1 < sizeof(line); i++) line[n++] = ahex[i];
    if (n + 1 < sizeof(line)) line[n++] = '\n';
    line[n] = 0;
    append_trace(line);
}

static void visible_failure(void)
{
    static u16 failed[] = {
        '\r','\n','z','l','O','S',' ','s','t','a','g','e',' ','0',' ','s','t','o','p','p','e','d','.',
        ' ','T','r','a','c','e',':',' ','\\','E','F','I','\\','Z','L','O','S','\\',
        'W','I','T','N','E','S','S','.','L','O','G','\r','\n',0
    };
    say(failed);
    /* Keep the exact failure visible before returning to the boot manager. */
    if (g_st && g_st->boot_services && g_st->boot_services->stall)
        g_st->boot_services->stall(10000000ULL);
}

MS efi_status efi_stage0_main(efi_handle image, efi_system_table *st)
{
    static u16 entered[] = {
        '\r','\n','z','l','O','S',' ','E','F','I',' ','w','i','t','n','e','s','s',' ','s','t','a','g','e',' ','0',
        ' ','e','n','t','e','r','e','d','\r','\n',0
    };
    static u16 loaded_label[] = { 'L','o','a','d','e','d','I','m','a','g','e',':',' ','0','x',0 };
    static u16 fs_label[] = { 'S','i','m','p','l','e','F','S',':',' ','0','x',0 };
    static u16 volume_label[] = { 'O','p','e','n','V','o','l','u','m','e',':',' ','0','x',0 };
    static u16 open_label[] = { 'O','p','e','n',' ','Z','L','O','S','.','E','F','I',':',' ','0','x',0 };
    static u16 info_label[] = { 'G','e','t','I','n','f','o',':',' ','0','x',0 };
    static u16 path_label[] = { 'B','u','i','l','d','D','e','v','i','c','e','P','a','t','h',':',' ','0','x',0 };
    static u16 load_label[] = { 'L','o','a','d','I','m','a','g','e',':',' ','0','x',0 };
    static u16 start_label[] = { 'S','t','a','r','t','I','m','a','g','e',' ','r','e','t','u','r','n','e','d',':',' ','0','x',0 };
    efi_boot_services *bs;
    efi_loaded_image *loaded = 0;
    efi_simple_fs *fs = 0;
    efi_file *child = 0;
    efi_handle child_image = 0;
    void *child_device_path = 0;
    efi_status status;
    u64 info_size;
    u64 child_size;

    g_st = st;
    say(entered);
    if (!st || !st->boot_services) {
        visible_failure();
        return EFI_ERROR_BIT | 2ULL;
    }
    bs = st->boot_services;

    status = bs->handle_protocol(image, &LOADED_IMAGE_GUID, (void **)&loaded);
    report(loaded_label, "LOADED_IMAGE 0x", status);
    if (status != EFI_SUCCESS || !loaded) {
        visible_failure();
        return status;
    }

    status = bs->handle_protocol(loaded->device_handle, &SIMPLE_FS_GUID,
                                 (void **)&fs);
    report(fs_label, "SIMPLE_FS 0x", status);
    if (status != EFI_SUCCESS || !fs) {
        visible_failure();
        return status;
    }

    status = fs->open_volume(fs, &g_root);
    if (status != EFI_SUCCESS || !g_root) {
        report(volume_label, "OPEN_VOLUME 0x", status);
        visible_failure();
        return status;
    }
    reset_trace();
    append_trace("STAGE0 ENTER\n"
                 "LOADED_IMAGE 0x0000000000000000\n"
                 "SIMPLE_FS 0x0000000000000000\n");
    report(volume_label, "OPEN_VOLUME 0x", status);

    status = g_root->open(g_root, &child, CHILD_PATH, EFI_FILE_MODE_READ, 0);
    report(open_label, "OPEN_CHILD 0x", status);
    if (status != EFI_SUCCESS || !child) {
        visible_failure();
        return status;
    }

    info_size = sizeof(info_words);
    status = child->get_info(child, &FILE_INFO_GUID, &info_size, info_words);
    report(info_label, "GET_INFO 0x", status);
    if (status != EFI_SUCCESS) {
        child->close(child);
        visible_failure();
        return status;
    }
    child_size = ((efi_file_info_prefix *)info_words)->file_size;
    if (!child_size || child_size > CHILD_SIZE_LIMIT) {
        child->close(child);
        report(info_label, "CHILD_SIZE 0x", EFI_BAD_BUFFER_SIZE);
        visible_failure();
        return EFI_BAD_BUFFER_SIZE;
    }

    child->close(child);
    child_device_path = build_child_device_path(bs, loaded->device_handle,
                                                &status);
    report(path_label, "BUILD_DEVICE_PATH 0x", status);
    if (status != EFI_SUCCESS || !child_device_path) {
        visible_failure();
        return status;
    }

    status = bs->load_image(0, image, child_device_path, 0, 0, &child_image);
    report(load_label, "LOAD_IMAGE 0x", status);
    if (status != EFI_SUCCESS || !child_image) {
        bs->free_pool(child_device_path);
        visible_failure();
        return status;
    }

    append_trace("START_IMAGE CALL\n");
    status = bs->start_image(child_image, 0, 0);
    report(start_label, "START_IMAGE_RETURN 0x", status);
    bs->free_pool(child_device_path);
    visible_failure();
    return status;
}
