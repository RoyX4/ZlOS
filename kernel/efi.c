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
static void capture_acpi(efi_system_table *st)
{
    if (!st->config_table) return;
    efi_config_entry *e = (efi_config_entry *)st->config_table;
    unsigned long long found_1 = 0;

    for (unsigned long long i = 0; i < st->n_config_entries; i++) {
        if (guid_eq(&e[i].guid, 0x8868e871, 0xe4f1, 0x11d3,
                    0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81)) {
            /* No intermediate `unsigned long`: it is 4 bytes in this build, and
             * firmware is free to place the RSDP above 4 GiB. Truncating it
             * would hand ACPI/APIC/SMP discovery a wild pointer. */
            acpi_set_rsdp((u64)e[i].table);
            return;                                   /* ACPI 2.0+ - best */
        }
        if (guid_eq(&e[i].guid, 0xeb9d2d30, 0x2d88, 0x11d3,
                    0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d))
            found_1 = (u64)e[i].table;                    /* same truncation */
    }
    if (found_1) acpi_set_rsdp(found_1);
}

/* ---- what the kernel proper needs --------------------------------------- */
void serial_init(void);
void console_init_fb(unsigned long long addr, unsigned int pitch, unsigned int width,
                     unsigned int height, unsigned int bpp);
void console_init_efi(unsigned long long addr, unsigned int pitch, unsigned int width,
                      unsigned int height, unsigned int bpp);
void console_init(unsigned long mb_addr);
int  main(void);
void kernel_done(void);

/* The only way to say anything from here. It runs after console_init_efi(),
 * so it reaches the screen and the serial line exactly like every other line
 * of the boot log. */
void zl_putc_pub(char c);
static void efi_say(const char *s) { while (*s) zl_putc_pub(*s++); }
static void efi_say_u64(unsigned long long v)
{
    char b[24];
    int n = 0;
    if (!v) { zl_putc_pub('0'); return; }
    while (v) { b[n++] = (char)('0' + (v % 10)); v /= 10; }
    while (n) zl_putc_pub(b[--n]);
}

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

MS efi_status efi_main(efi_handle image, efi_system_table *st)
{
    efi_boot_services *bs = st->boot_services;

    /* Do this BEFORE ExitBootServices: the configuration table is the
     * firmware's memory and everything about it stops being valid after. */
    capture_acpi(st);

    /* ---- 1. find the framebuffer ---------------------------------------
     * UEFI has no VGA text mode and no BIOS to ask, so this protocol IS the
     * display. It also hands back a real mode list, which is strictly better
     * information than the VBE scan our BIOS bootloader has to do. */
    locate_protocol_fn locate_protocol =
        *(locate_protocol_fn *)((u8 *)bs + sizeof(efi_table_header) + 37 * 8);

    efi_gop *gop = 0;
    if (locate_protocol(&GOP_GUID, 0, (void **)&gop) == EFI_SUCCESS && gop && gop->mode) {
        efi_gop_mode_info *mi = gop->mode->info;
        fb_addr        = gop->mode->framebuffer_base;
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
    /* 32 KiB, not 16: a laptop's map is bigger than a VM's, and if it does not
     * fit there is no way to take the machine at all. At 48 bytes a descriptor
     * this holds ~680 entries, against the 60-150 real firmware reports. */
    static u8 map[32768];
    u64 map_size, map_key = 0, desc_size, want = 0;
    u32 desc_ver;
    int exited = 0;
    for (int attempt = 0; attempt < 4 && !exited; attempt++) {
        map_size = sizeof(map);
        if (bs->get_memory_map(&map_size, (efi_memory_descriptor *)map,
                               &map_key, &desc_size, &desc_ver) != EFI_SUCCESS) {
            want = map_size;         /* the firmware reports what it needs */
            continue;
        }
        if (bs->exit_boot_services(image, map_key) == EFI_SUCCESS)
            exited = 1;
    }

    /* From here the firmware no longer exists. We are the operating system:
     * still in long mode, still on the page tables UEFI built (identity
     * mapped, so our physical addresses stay valid), and nothing else is
     * running on this machine. */

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

    /* If that loop never exited, the firmware is STILL RUNNING: its timers, its
     * interrupt handlers, its watchdog. Installing our own GDT and IDT on top
     * of a live UEFI is not a degraded mode, it is a crash - and it used to be
     * a silent one, because this fell straight through into main() and the
     * screen simply stayed black. Say which of the two things went wrong. */
    if (!exited) {
        efi_say("\n  zlOS: ExitBootServices FAILED - the firmware still owns"
                " this machine.\n  Halted rather than fight it.\n");
        if (want) {
            efi_say("  cause: the memory map needs ");
            efi_say_u64(want);
            efi_say(" bytes; the buffer in efi.c is ");
            efi_say_u64(sizeof(map));
            efi_say(".\n");
        } else {
            efi_say("  the map fitted, so the map key went stale 4 times"
                    " running.\n");
        }
        for (;;) __asm__ volatile("hlt");
    }

    main();
    kernel_done();
    for (;;) __asm__ volatile("hlt");
    return EFI_SUCCESS;
}
