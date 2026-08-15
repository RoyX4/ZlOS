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

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;
typedef u64                efi_status;
typedef void              *efi_handle;

#define EFI_SUCCESS 0
#define MS __attribute__((ms_abi))

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

/* Only the entries we call are named; the rest are padding of the right size,
 * because the offsets are fixed by the spec and a wrong one calls the wrong
 * function. */
typedef struct {
    efi_table_header hdr;
    void *raise_tpl, *restore_tpl;
    void *allocate_pages, *free_pages;
    efi_status (MS *get_memory_map)(u64 *size, efi_memory_descriptor *map,
                                    u64 *key, u64 *desc_size, u32 *desc_ver);
    void *allocate_pool, *free_pool;
    void *create_event, *set_timer, *wait_for_event, *signal_event,
         *close_event, *check_event;
    void *install_protocol_interface, *reinstall_protocol_interface,
         *uninstall_protocol_interface, *handle_protocol, *reserved2,
         *register_protocol_notify, *locate_handle, *locate_device_path,
         *install_configuration_table;
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
    void *runtime_services;
    efi_boot_services *boot_services;
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

/* LocateProtocol lives further into boot services than the fields above, so
 * it is reached by offset rather than by growing the struct: it is entry 37
 * of the table (0-based), i.e. 37 pointers past the header. */
typedef efi_status (MS *locate_protocol_fn)(efi_guid *proto, void *reg, void **iface);

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

/* ---- what the kernel proper needs --------------------------------------- */
void serial_init(void);
void console_init_fb(unsigned long addr, unsigned int pitch, unsigned int width,
                     unsigned int height, unsigned int bpp);
void console_init_efi(unsigned long addr, unsigned int pitch, unsigned int width,
                      unsigned int height, unsigned int bpp);
void console_init(unsigned long mb_addr);
int  main(void);
void kernel_done(void);

/* Remember the mode across ExitBootServices. Everything the firmware owns
 * becomes invalid the moment that call returns, so anything we still need has
 * to be plain numbers in our own memory before then. */
static unsigned long fb_addr;
static unsigned int  fb_w, fb_h, fb_pitch_bytes;

MS efi_status efi_main(efi_handle image, efi_system_table *st)
{
    efi_boot_services *bs = st->boot_services;

    /* ---- 1. find the framebuffer ---------------------------------------
     * UEFI has no VGA text mode and no BIOS to ask, so this protocol IS the
     * display. It also hands back a real mode list, which is strictly better
     * information than the VBE scan our BIOS bootloader has to do. */
    locate_protocol_fn locate_protocol =
        *(locate_protocol_fn *)((u8 *)bs + sizeof(efi_table_header) + 37 * 8);

    efi_gop *gop = 0;
    if (locate_protocol(&GOP_GUID, 0, (void **)&gop) == EFI_SUCCESS && gop && gop->mode) {
        efi_gop_mode_info *mi = gop->mode->info;
        fb_addr        = (unsigned long)gop->mode->framebuffer_base;
        fb_w           = mi->horizontal_resolution;
        fb_h           = mi->vertical_resolution;
        /* the stride is given in pixels; our renderer wants bytes */
        fb_pitch_bytes = mi->pixels_per_scan_line * 4;
    }

    /* ---- 2. take the machine ------------------------------------------
     * ExitBootServices needs the CURRENT memory map key, and the map can
     * change underneath us (even asking for its size can allocate), so the
     * documented dance is: get the map, try to exit, and if the key went
     * stale, get it again and retry. */
    static u8 map[16384];
    u64 map_size, map_key = 0, desc_size;
    u32 desc_ver;
    for (int attempt = 0; attempt < 4; attempt++) {
        map_size = sizeof(map);
        if (bs->get_memory_map(&map_size, (efi_memory_descriptor *)map,
                               &map_key, &desc_size, &desc_ver) != EFI_SUCCESS)
            continue;
        if (bs->exit_boot_services(image, map_key) == EFI_SUCCESS)
            break;
    }

    /* From here the firmware no longer exists. We are the operating system:
     * still in long mode, still on the page tables UEFI built (identity
     * mapped, so our physical addresses stay valid), and nothing else is
     * running on this machine. */

    serial_init();
    if (fb_addr) {
        console_init_efi(fb_addr, fb_pitch_bytes, fb_w, fb_h, 32);
    } else {
        console_init(0);            /* no framebuffer - fall back to text */
    }

    main();
    kernel_done();
    for (;;) __asm__ volatile("hlt");
    return EFI_SUCCESS;
}
