/* runtime_kernel.c - the FREESTANDING runtime for the kernel subset.
 *
 * runtime.c is the full boxed runtime: lists, strings, 93 builtins, and
 * about 70 libc symbols under it (all of libm, stdio, opendir, fork...).
 * None of that can exist in a kernel, and design_kernel.md is explicit
 * about why: "No implicit allocation in the kernel subset - a zl_alloc
 * reaching a kernel binary is a fault with no diagnostic."
 *
 * So this is a SECOND implementation of the same runtime.h API, covering
 * only what a kernel may use:
 *
 *     numbers (i64 semantics), the arithmetic and comparison operators,
 *     print of a number or a string LITERAL, exit, and raw memory.
 *
 * Everything else - lists, string values, concatenation, the maths
 * builtins - is a hard error rather than a silent surprise, which is the
 * project's standing rule (MASTER_PLAN §8 risk 9: reject, never answer
 * differently).
 *
 * ZERO libc. The only outside contact is zl_putc, and that is deliberately
 * the single seam: on Linux it is a write syscall, in a kernel it is the
 * serial port (design_kernel.md §7 - "serial before interrupts, always").
 * Swapping targets is one function, which is what makes this backend
 * -agnostic in the way that matters.
 */
#include "../src/runtime/runtime.h"

/* Pointer-sized, for the raw-memory builtins below. NOT `unsigned long`: the
 * EFI target is LLP64, where it is 4 bytes, so peek/poke/fill_mem/copy_mem and
 * the fs read/write buffers all silently capped at 4 GiB there - and a zl
 * number carries 53 bits of address, which is the whole point of having them.
 * Same shape as fb.c's fb_uptr and xhci.c's uptr. */
#if defined(ZL_64) || defined(__x86_64__)
typedef unsigned long long zl_uptr;
#else
typedef unsigned int       zl_uptr;
#endif

/* Panic logging is opportunistic in every freestanding target. The real kernel
 * links zllog.c; the standalone libc-free proof does not, so these must remain
 * weak and checked before use. */
#ifndef ZL_KERNEL_SERIAL
extern void zllog_event(unsigned, unsigned, unsigned,
                        unsigned, unsigned, unsigned) __attribute__((weak));
extern void zllog_event_irq(unsigned, unsigned, unsigned,
                            unsigned, unsigned, unsigned) __attribute__((weak));
extern int zllog_ready(void) __attribute__((weak));
extern int zllog_io_active(void) __attribute__((weak));
extern int zllog_flush(void) __attribute__((weak));
#endif

/* ---------------------------------------------------------------- seam */
#ifdef ZL_KERNEL_SERIAL
/* In a kernel: COM1, polled. §7.1's registers. outb is the intrinsic the
 * kernel backend will provide; until then this path is compiled only when
 * the caller supplies one. */
extern void zl_outb(unsigned short port, unsigned char val);
extern unsigned char zl_inb(unsigned short port);
#define COM1 0x3F8
/* the VGA text console lives in the kernel dir - it is board support, not
 * language runtime, so it is only linked in on the kernel target */
/* console.c chooses VGA text or a UEFI framebuffer at run time, so the
 * runtime - and therefore kernel.zl - never learns which screen it is on */
extern void console_putc(char c);
extern void zllog_putc(char c);
extern void zllog_event(unsigned, unsigned, unsigned,
                        unsigned, unsigned, unsigned) __attribute__((weak));
extern void zllog_event_irq(unsigned, unsigned, unsigned,
                            unsigned, unsigned, unsigned) __attribute__((weak));
extern int zllog_ready(void) __attribute__((weak));
extern int zllog_io_active(void) __attribute__((weak));
extern int zllog_flush(void) __attribute__((weak));
extern void console_clear(void);
extern void console_setcolor(unsigned char attr);
extern void console_bar(int row, unsigned char attr);
extern void console_at(int row, int col, const char *s, unsigned char attr);
extern void console_set_row(int r);
extern int  console_get_row(void);
extern int  console_get_col(void);
extern int  console_status_row(void);
extern int  console_loader(void);
extern int  console_pxw(void);
extern int  console_pxh(void);
extern void console_fill(int x, int y, int w, int h, unsigned char attr);
extern void console_gradient(int x, int y, int w, int h, unsigned char at, unsigned char ab);
extern void console_logo(int px, int py, const char *s, int scale, unsigned char attr);
extern void console_cursor(int row, int col, int on, unsigned char attr);
extern void console_set_region(int top, int bot);
extern void gdt_init(void);
extern void idt_init(void);
extern unsigned int idt_ticks(void);
extern int  idt_scan(void);
extern void crash_test_ud2(void);
extern void console_at_num(int row, int col, long n, unsigned char attr);
extern void console_fill_rgb(int x, int y, int w, int h, unsigned int rgb);
extern void console_gradient_rgb(int x, int y, int w, int h, unsigned int top, unsigned int bot);
extern void console_text_rgb(int px, int py, const char *s, unsigned int rgb);
extern int  console_get_px(int x, int y);
extern void console_shade(int x, int y, int w, int h, int num, int den);
extern void console_shadow(int x, int y, int w, int h, int off, int soft);
extern void console_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
extern void console_text_aa(int px, int py, const char *s, unsigned int rgb);
extern void console_text_aa2x(int px, int py, const char *s, unsigned int rgb);
extern void console_num_aa(int px, int py, long n, unsigned int rgb);
extern void console_puthex(unsigned long v, int digits);
extern void console_char_aa(int px, int py, int code, unsigned int rgb);
extern void console_set_text_box(int c0, int c1);
extern void console_cube(int cx, int cy, int size, int angle, unsigned int rgb);
extern void console_pointer_show(int x, int y);
extern void console_pointer_hide(void);
extern void console_present(void);
extern void console_icon(int px, int py, int n, unsigned int rgb);
extern void console_cube_filled(int cx, int cy, int size, int angle, unsigned int rgb);
extern void console_cube_clip(int x0, int y0, int x1, int y1);
extern int  cpu_brand_byte(int i);
extern void speaker_on(unsigned freq);
extern void speaker_off(void);
extern void kreboot(void);
extern int  idt_mouse_x(void);
extern int  idt_mouse_y(void);
extern int  idt_mouse_btn(void);
extern unsigned idt_mouse_irqs(void);
/* ---- the USB pointer: absolute when it is a tablet, so it cannot drift -----
 *
 * THESE ARE WEAK ON PURPOSE, and it is the same trick wmglue.c uses for the
 * app callbacks. Commit b19207d ("wip(usb,input)") landed these CALL SITES
 * without the driver that defines them: `git log --all -S"int xhci_ptr_ready"`
 * finds no commit on any branch, so the tree as committed does not link, and
 * every gate in the project has been unrunnable since. The definitions exist,
 * uncommitted, in the display session's working tree.
 *
 * Writing a second USB pointer driver here to fix the link would be the wrong
 * repair twice over - it is someone else's work in flight, and xhci.c is the
 * file they have open. A weak reference is NULL when nothing defines it, so
 * the kernel links today, falls back to the PS/2 mouse that has always worked,
 * and BINDS TO THE REAL DRIVER the moment that commit lands, with no change
 * here. Nothing to remember, nothing to undo.
 *
 * The fallback ANNOUNCES ITSELF (see usb_ptr_ok below). A silent fallback is
 * this repo's most expensive recurring bug - `intel_backlight_set` computed a
 * max of 0 and quietly did nothing for months - and one that hides a missing
 * driver would be exactly that shape. */
#define ZL_WEAK __attribute__((weak))
extern int xhci_ptr_ready(void) ZL_WEAK;
extern int xhci_ptr_abs(void) ZL_WEAK;
extern int xhci_ptr_x(void) ZL_WEAK;
extern int xhci_ptr_y(void) ZL_WEAK;
extern int xhci_ptr_btn(void) ZL_WEAK;
extern int xhci_ptr_poll(void) ZL_WEAK;
extern unsigned xhci_ptr_reports(void) ZL_WEAK;
extern unsigned xhci_ptr_events(void) ZL_WEAK;
extern int      xhci_ptr_lastcc(void) ZL_WEAK;
extern unsigned xhci_kbd_events(void) ZL_WEAK;
extern unsigned xhci_kbd_requeues(void) ZL_WEAK;
extern int      xhci_kbd_lastcc(void) ZL_WEAK;
extern int xhci_ptr_slot(void) ZL_WEAK;
extern int xhci_ptr_ep(void) ZL_WEAK;

/* ---- arena.c: the memory a program the kernel was NOT built with may use --
 * Not weak. arena.c is in all four source lists and is pure arithmetic against
 * memory - if it is missing the build should fail, because unlike a USB
 * pointer there is no fallback that "has always worked" to degrade to. */
/* heap.c, the general allocator. Same argument as arena.c for not being weak:
 * it is in all four source lists and is arithmetic against memory. The two are
 * NOT alternatives - the arena hands memory to zl programs and reclaims it
 * wholesale on reset, the heap is for memory the kernel keeps and frees one
 * object at a time. Both exist on purpose. */
extern void user_selftest(void);
extern int  user_has_exited(void);
extern unsigned int user_call_count(void);
#if defined(ZL_64)
extern int user64_run_default_file(void);
#endif

extern void vmm_report(void);
extern int  vmm_active(void);
extern unsigned long long vmm_window_virt(void);

extern int heap_init(void);
extern int heap_ok(void);
extern unsigned long heap_capacity(void);
extern unsigned long heap_used(void);
extern unsigned long heap_available(void);
extern unsigned long heap_high_water(void);
extern unsigned long heap_refusals(void);
extern unsigned long heap_blocks(void);
extern unsigned long heap_check(void);

extern int arena_init(void);
extern int arena_ok(void);
extern void arena_reset(void);
extern unsigned long arena_resets(void);
extern unsigned long arena_capacity(void);
extern unsigned long arena_used(void);
extern unsigned long arena_available(void);
extern unsigned long arena_high_water(void);
extern unsigned long arena_refusals(void);
extern unsigned long arena_base_addr(void);

/* ---- exec.c: `run`, and every way it declines --------------------------- */
extern int  exec_run(void);
extern int  exec_state(void);
extern int  exec_wants_window(void);
extern const char *exec_title(void);
extern void exec_draw(int x, int y, int w, int h,
                      unsigned int fg, unsigned int dim, unsigned int accent);

/* Is there a USB pointer at all? Two questions in one, and both have to be
 * yes: is the driver linked in (weak symbol non-NULL), and did it find a
 * device. Every xhci_ptr_* call below is guarded by this, so a NULL weak
 * symbol can never be called. */
static int usb_ptr_ok(void)
{
    return xhci_ptr_ready != 0 && xhci_ptr_ready() != 0;
}

/* Zero when the driver is absent, so the diagnostic builtins report 0 rather
 * than jumping through a null pointer. */
#define ZL_WEAK_CALL(fn) ((fn) ? (fn)() : 0)
extern void console_box(int x, int y, int w, int h, unsigned char attr);
extern void console_line(int x0, int y0, int x1, int y1, unsigned char attr);
extern void console_mouse_cursor(int x, int y, unsigned char fill, unsigned char edge);
extern int  console_kind(void);
extern void console_mute(int on);
extern void console_unmute(void);
extern void console_text_role(int x, int y, const char *s, unsigned int rgb, int role, int weight);
extern int  console_text_role_w(const char *s, int role, int weight);
extern int  console_text_role_h(int role);
extern void console_num_role(int x, int y, long v, unsigned int rgb, int role, int weight);
extern void console_gradtop(int x, int y, int w, int h, int r, unsigned int t, unsigned int b);
extern void console_blend(int x, int y, int w, int h, unsigned int rgb, int a);
extern void console_rrblend(int x, int y, int w, int h, int r, unsigned int rgb, int a);
extern void console_glow(int cx, int cy, int rx, int ry, unsigned int rgb, int ai, int ao, int stop);
extern int  console_wall_save(void);
extern void console_wall_paint(int x, int y, int w, int h);
extern int  console_wall_ok(void);
extern void console_wedge(int cx, int cy, unsigned int rgb, int a0, int f, int m, int e);
void console_clip(int x, int y, int w, int h);
void console_clip_off(void);
extern int  console_blur(int x, int y, int w, int h, int r);
extern void console_blur_paint(int slot, int x, int y);
extern void console_blur_free(void);
extern void ser_puts(const char *s);
extern unsigned long long console_vram(void);
extern int  console_cols(void);
extern int  wm_thumb(int, int, int, int, int);
extern int  console_cell_w(void);
extern int  console_ui_scale(void);
/* uikit's table widgets. They have existed since the widget set was written and
 * nothing outside C could reach them, so every table-shaped app in the prototype
 * - the kernel log, hex, network, disk usage, registers - would have had to
 * hand-draw its own header and rows. */
extern int  ui_colhead(int, int, int, const char *, int, int);
extern void ui_grid(const char *);
extern void ui_grid_span(int, int, int, int *, int *);
extern void ui_grid_cell(int, int, int, int, int, const char *, int, unsigned, int, int);
extern int  ui_colhead_h(void);
extern int  ui_grid_row(int, int, int, int, int);
extern int  ui_grid_row_h(void);
extern int  console_ui_scale_q8(void);
extern int  console_cell_h(void);
extern void fb_set_subpixel(int on);
extern int  fb_get_subpixel(void);
extern int  fb_enable_write_combining(void);
/* the PCI bus driver and our own modesetting driver */
extern void pci_scan(void);
extern int  pci_count(void);
extern int  pci_vendor(int i);
extern int  pci_device(int i);
extern int  pci_class(int i);
extern int  pci_find_class(int cls, int sub);
extern unsigned int pci_bar(int i, int which);
/* AX201 transport probe: read-only PCI identity and direct CSR snapshot. */
extern int  iwlwifi_probe(void);
extern int  iwlwifi_present(void);
extern int  iwlwifi_device(void);
extern unsigned int iwlwifi_bar_lo(void);
extern unsigned int iwlwifi_bar_hi(void);
extern unsigned int iwlwifi_hw_if_config(void);
extern unsigned int iwlwifi_int_status(void);
extern unsigned int iwlwifi_int_mask(void);
extern unsigned int iwlwifi_gpio(void);
extern unsigned int iwlwifi_reset(void);
extern unsigned int iwlwifi_gp_cntrl(void);
extern unsigned int iwlwifi_hw_rev(void);
extern unsigned int iwlwifi_hw_type(void);
extern unsigned int iwlwifi_rf_id(void);
extern unsigned int iwlwifi_mac_csr0(void);
extern unsigned int iwlwifi_mac_csr1(void);
extern int  bga_present(void);
extern int  bga_version(void);
extern int  bga_find(void);
extern unsigned int bga_framebuffer(void);
extern unsigned int bga_vram_bytes(void);
extern int  bga_set_mode(int w, int h, int bpp);
extern int  bga_get_width(void);
extern int  bga_get_height(void);
extern int  console_set_res(int w, int h);
extern int  bga_get_pitch(void);
extern int  bga_reg(int idx);
/* the xHCI USB host controller driver */
extern int  xhci_find(void);
extern int  xhci_present(void);
extern int  xhci_version(void);
extern int  xhci_slots(void);
extern int  xhci_ports(void);
extern int  xhci_ctx_size(void);
extern unsigned int xhci_mmio(void);
extern int  xhci_reset(void);
extern int  xhci_halted(void);
extern unsigned int xhci_usbsts(void);
extern unsigned int xhci_usbcmd(void);
extern int  xhci_port_connected(int p);
extern int  xhci_port_speed(int p);
extern int  xhci_devices_attached(void);
extern int  xhci_init_rings(void);
extern int  xhci_running(void);
extern int  xhci_test_noop(void);
extern int  xhci_port_reset(int p);
extern int  xhci_port_enabled(int p);
extern int  xhci_enumerate(int p);
extern int  xhci_device_address(void);
extern int  xhci_desc_vendor(void);
extern int  xhci_desc_product(void);
extern int  xhci_desc_usbver(void);
extern int  xhci_desc_class(void);
extern int  xhci_desc_mps0(void);
extern int  xhci_desc_byte(int i);
extern int  xhci_kbd_init(void);
extern int  xhci_kbd_ready(void);
extern int  xhci_kbd_slot(void);
extern int  xhci_kbd_ep(void);
extern int  xhci_kbd_poll(void);
extern int  xhci_key(void);
extern int  xhci_kbd_report(int i);
extern int  xhci_ram_ok(void);
extern int  settings_load(void);
extern int  fs_try_boot(void);
extern void fs_seed_hello(void);
extern int  xhci_bringup(void);
extern int  xhci_owned(void);
extern unsigned int xhci_portsc(int p);
extern int  xhci_scratchpads(void);
extern int  xhci_bar_high(void);
extern int  xhci_ecm_init_stage(void);
extern int  xhci_ecm_config_index(void);
extern int  xhci_ecm_last_cc(void);
extern unsigned int xhci_ecm_parse_bits(void);
extern int  xhci_ecm_diag_len(void);
extern int  xhci_ecm_diag_byte(int);
/* the APIC: the interrupt controller that replaces the 1981 PIC */
extern int  apic_init(void);
extern int  apic_active(void);
extern int  apic_supported(void);
extern unsigned int apic_lapic_base(void);
extern unsigned int apic_ioapic_base(void);
extern int  apic_ioapic_pins(void);
extern int  apic_madt_ok(void);
extern int  apic_cpus(void);
extern unsigned int apic_rsdp(void);
extern int  apic_gsi(int irq);
extern int  apic_id(void);
extern unsigned int apic_redtbl(int irq);
/* virtio-gpu: a real GPU interface we can prove on every build */
extern int  virtio_gpu_find(void);
extern int  virtio_gpu_init(void);
extern int  virtio_gpu_present(void);
extern int  virtio_gpu_ready(void);
extern int  virtio_gpu_display_info(void);
extern int  virtio_gpu_width(void);
extern int  virtio_gpu_height(void);
extern int  virtio_gpu_scanouts(void);
extern unsigned int virtio_gpu_setup(void);
extern unsigned int virtio_gpu_fb(void);
extern int  virtio_gpu_flush(int x, int y, int w, int h);
extern int  virtio_gpu_create_2d(unsigned id, unsigned w, unsigned h);
extern int  virtio_gpu_attach_backing(unsigned id, unsigned bytes);
extern int  virtio_gpu_set_scanout(unsigned id, unsigned w, unsigned h);
extern unsigned int virtio_gpu_last_resp(void);
extern int  virtio_gpu_testpattern(void);
/* the processor itself */
extern int  cpu_vendor_byte(int i);
extern int  cpu_family(void);
extern int  cpu_model(void);
extern int  cpu_stepping(void);
extern int  cpu_cores(void);
extern int  cpu_threads(void);
extern int  cpu_threads_per_core(void);
extern int  cpu_apic_id(void);
extern unsigned int cpu_tsc_khz(void);
extern unsigned int cpu_mhz(void);
extern unsigned int cpu_tsc_lo(void);
extern unsigned long long cpu_tsc(void);
extern int  cpu_tsc_invariant(void);
extern int  cpu_cache_type(int i);
extern int  cpu_cache_level(int i);
extern int  cpu_cache_kb(int i);
extern int  cpu_temp_c(void);
extern int  cpu_hypervisor_byte(int i);
extern int  cpu_has_sse2(void);
extern int  cpu_has_sse42(void);
extern int  cpu_has_avx(void);
extern int  cpu_has_avx2(void);
extern int  cpu_has_aes(void);
extern int  cpu_has_rdrand(void);
extern int  cpu_has_hypervisor(void);
/* NVMe: real storage */
extern int  nvme_find(void);
extern int  nvme_present(void);
extern int  nvme_setup(void);
extern int  nvme_ready(void);
extern unsigned int nvme_version(void);
extern unsigned int nvme_mmio(void);
extern int  nvme_model_byte(int i);
extern int  nvme_serial_byte(int i);
extern unsigned int nvme_blocks_lo(void);
extern unsigned int nvme_blocksize(void);
extern unsigned int nvme_capacity_mb(void);
extern int  nvme_read_block(unsigned lo, unsigned hi);
extern int  nvme_write_block(unsigned lo, unsigned hi);
extern int  nvme_data_byte(int i);
extern void nvme_data_set(int i, int v);
/* the scheduler */
extern int  sched_init(void);
extern int  sched_start_demo(void);
extern int  sched_active(void);
extern int  sched_count(void);
extern int  sched_current(void);
extern unsigned int sched_switches(void);
extern int  sched_state(int i);
extern unsigned int sched_ticks(int i);
extern unsigned int sched_counter(int i);
extern void yield(void);
extern void task_sleep(unsigned ticks);
extern unsigned int smp_band_wakes(void);
/* SMP: the other cores */
extern int  smp_start(void);
extern int  smp_online(void);
extern int  smp_cpu_count(void);
extern int  smp_last_id(void);
extern unsigned int smp_mask(void);
extern int  smp_tramp_size(void);
extern int  apic_cpu_id(int i);
/* USB mass storage over Bulk-Only Transport */
extern int  xhci_msc_init(void);
extern int  xhci_msc_ready(void);
extern int  xhci_msc_slot(void);
extern int  xhci_msc_inquiry(void);
extern int  xhci_msc_read_capacity(void);
extern int  xhci_msc_read_block(unsigned lba);
extern int  xhci_msc_byte(int i);
extern unsigned int xhci_msc_blocks(void);
extern unsigned int xhci_msc_blocksize(void);
extern unsigned int xhci_msc_capacity_mb(void);
extern int  xhci_msc_init_stage(void);
extern int  xhci_msc_init_port(void);
extern int  xhci_msc_init_slot(void);
extern int  xhci_msc_init_cc(void);
extern int  xhci_msc_init_vid(void);
extern int  xhci_msc_init_pid(void);
/* Persistent boot observer. Its hot path is RAM-only; diag_up is the one
 * place that may discover and validate the dedicated ZLLOG partition. */
extern int zllog_mount(void);
extern int zllog_ready(void);
extern int zllog_flush(void);
extern int zllog_complete(void);
extern void zllog_flush_if_due(void);
extern void zllog_milestone(unsigned int id, unsigned int value);
extern unsigned int zllog_buffered(void);
extern unsigned int zllog_dropped(void);
extern unsigned int zllog_last_error(void);
/* I2C-HID: the touchpad */
extern int  i2c_find(int which);
extern int  i2c_present(void);
extern int  i2c_is_designware(void);
extern unsigned int i2c_mmio(void);
extern unsigned int i2c_comp_type(void);
extern int  i2c_hid_probe(void);
extern int  i2c_hid_ready(void);
extern int  i2c_hid_address(void);
extern int  i2c_hid_vid(void);
extern int  i2c_hid_pid(void);
extern int  i2c_hid_version(void);
extern int  i2c_hid_max_input(void);
extern int  i2c_hid_rdesc_len(void);
extern int  i2c_hid_read_report(void);
extern int  i2c_hid_byte(int i);
extern int  i2c_hid_device_id(void);
extern unsigned int i2c_hid_abort_source(void);
extern unsigned int i2c_hid_fs_hcnt(void);
extern unsigned int i2c_hid_fs_lcnt(void);
extern unsigned int i2c_hid_lpss_reset(void);
extern int  i2c_hid_service(void);
extern int  i2c_hid_pointer_ready(void);
extern unsigned int i2c_hid_ptr_reports(void);
extern unsigned int i2c_hid_ptr_malformed(void);
/* the input stack: events, modifiers, repeat */
/* ---- the compositor (wm.c / ui.c / wmglue.c) ---------------------------
 * Mechanism only. kernel.zl supplies the policy through the app_* functions
 * that wmglue.c binds to - see kernel/docs/archive/superseded/desktop-wiring.md. */
/* ---- the terminal app (term.c) -----------------------------------------
 * A scrollback ring plus a typed-command matcher. The matcher is HERE, in C,
 * because comparing two runtime strings is the one thing the zl kernel subset
 * cannot do - it has string literals but no string values. */
extern void term_putc(char c);
extern int  term_key(int code);
extern int  term_cmd(void);
extern int  term_unknown(void);
extern int  term_arg(void);
extern void term_submit(int command, int argument, int word_len);
extern void term_complete(int command, int result);
extern void term_clear(void);
extern void term_draw(int x, int y, int w, int h, unsigned int fg,
                      unsigned int dim, unsigned int accent, int cursor_on);

extern int  wm_available(void);
extern int  wm_bind_zl(void);
extern void wm_init(void);
extern int  wm_open(int app, const char *title, int x, int y, int w, int h);
extern void wm_close(int win);
extern void wm_frame(void);
extern int  wm_running(void);
extern void wm_stop(void);
extern int  wm_focused(void);
extern int  wm_is_open(int win);
extern void wm_set_modal(int win, int on);
extern int  wm_anim(int win, int kind);
extern int  wm_anim_at(int id, int kind, int x, int y, int w, int h);
extern int  wm_anim_scale(int id);
extern int  wm_pulse(int period_ms);
extern void wm_set_sweep(int on);
extern void wm_close_fx(int win);
extern int  wm_frame_us(void);
extern int  wm_peak_us(void);
extern void wm_peak_reset(void);
/* the miss counters - an average hides stutter and a peak is one sample */
extern int  wm_late(void);
extern int  wm_lost(void);
extern int  wm_painted(void);
extern int  wm_budget_us(void);
extern int  wm_sample_count(void);
extern int  wm_sample_frame(int);
extern int  wm_sample_input(int);
extern unsigned long wm_client_surface_bytes(void);
extern unsigned int wm_client_surface_refusals(void);
extern unsigned int wm_region_fallbacks(void);
extern unsigned long long wm_region_occluded_pixels(void);
extern void wm_client(int win, int *x, int *y, int *w, int *h);
extern void wm_focus(int win);
/* THE THREE POLICY CELLS. wm.c draws the register number, the mono
 * subtitle and the status band, but it must not INVENT their contents -
 * which app is 03, what its subtitle says, what its status line reads are
 * all shell decisions. These three carry them across. Without them the
 * chrome renders correctly with every cell blank, which is the shape of
 * bug this project keeps finding: complete-looking code that nothing
 * calls. */
extern void wm_set_label(int win, int reg, const char *sub);
extern void wm_set_status(int win, const char *status);
extern void wm_set_field(int x, int y, int w, int h);
extern void wm_raise(int win);
extern void wm_set_home(int win);
extern int  wm_count(void);
extern int  wm_zorder_at(int i);
extern int  wm_win_app(int win);
/* workspaces. The compositor owns which one is current because it owns the
 * window table; kernel.zl owns how many there are and what the pips look
 * like. cur_ws()/set_ws() in kernel.zl are these four. */
extern int  wm_ws(void);
extern int  wm_set_ws(int n);
extern int  wm_win_ws(int win);
extern int  wm_set_win_ws(int win, int n);
extern int  wm_set_ws_n(int n);
extern int  wm_add_tab(int win, int app, const char *title);
extern void wm_damage(int x, int y, int w, int h);
extern void wm_damage_win(int win);
extern void wm_invalidate_client(int win);
extern void wm_invalidate_client_rect(int win, int x, int y, int w, int h);
extern void ui_theme_init(int scale);
extern void ui_theme_init_q8(int scale_q8);
extern unsigned ui_color(int role);
extern int ui_metric(int role);
/* THE SETTINGS PANE'S OWN FIVE. ui_ratio/ui_ceil_* are the contrast engine -
 * WCAG in integer fixed point, x10^4 - and ui_knock/ui_fbar are the two live
 * controls the FOCUS tab drives. A native with no registration is dead code
 * and this tree has already paid for that once, so all five are registered
 * below and check-zlcalls.py resolves every zl call site against this list. */
extern unsigned ui_ratio_q4(unsigned a, unsigned b);
extern unsigned ui_ceil_dn_q4(unsigned rgb);
extern unsigned ui_ceil_up_q4(unsigned rgb);
extern int ui_knockout_get(void);
extern int ui_knockout_set(int on);
extern int ui_focus_bar_dp(void);
extern int ui_focus_bar_set(int n);
extern unsigned ui_ref_color(int which);
extern int ui_ref_num(int which);
/* THE LABEL AND DISPLAY STYLES, and the ATLAS TABLE - what the SYSTEM and TYPE
 * panes are built on. The first pair is PRESSWORK's tracked-caps style, which
 * uikit.c owns and which nothing in kernel.zl could reach until it was
 * registered below; the atlas seven are sizeof() over the linked font arrays,
 * so the TYPE pane reads its own facts instead of restating design.h's. */
extern int  ui_caps_w(const char *s, int size);
extern void ui_caps(int x, int y, const char *s, unsigned rgb, int size);
extern int  ui_display_w(const char *s, int size);
extern void ui_display(int x, int y, const char *s, unsigned rgb, int size);
extern int ui_atlas_n(void);
extern int ui_atlas_w(int i);
extern int ui_atlas_h(int i);
extern int ui_atlas_glyphs(int i);
extern int ui_atlas_face(int i);
extern int ui_atlas_in_image(int i);
extern int ui_atlas_for_role(int role, int weight);
extern void ui_begin(int x, int y, int w, int h, int mode, int px, int py, int click);
extern int  ui_fired(void);
extern void ui_label(const char *s);
extern void ui_label_dim(const char *s);
extern void ui_bar(int pct);
extern int  ui_button(const char *s);
extern void ui_sep(void);
extern void ui_space(int n);
extern int  ui_toggle_value(const char *s, int on);
extern int  ui_slider_value(int v, int lo, int hi);
extern void ui_num(const char *s, int v);
extern int  ui_list_row(const char *s, int selected);
extern void ui_scroll_begin_value(int h, int off);
extern int  ui_scroll_end_value(void);
extern int  ui_scroll_content(void);
extern void ui_row(void);
extern void ui_endrow(void);

/* ---- uikit.c, the shared widget catalogue ---------------------------------
 * uikit.c has been in kernel/SOURCES since 5f0c1fc, so every target already
 * LINKS it - and nothing could CALL it, because the whole catalogue stopped at
 * the C boundary and every app past the original 13 is written in zl. The
 * declarations and the bindings below are that boundary and nothing more: no
 * new widget, no new geometry, no colour. Two shapes need a wrapper and both
 * are noted at their binding.
 *
 * NAMING follows the ui_* bindings above (ui_toggle -> ui_toggle_value, not
 * the pointer form), so a zl app sees one prefix for the whole toolkit. */
extern int  ui_text_w(const char *s, int size, int flags);
extern int  ui_text_h(int size);
extern void ui_text(int x, int y, const char *s, unsigned rgb, int size, int flags);
/* TRACKED TEXT, and it is what the `label*` builtins below now draw through.
 * uikit.c had the tracking and it was file-local, so three C widgets could
 * draw PRESSWORK's label style and the entire zl shell could not - which is
 * every uppercase run the desktop actually shows: the register rail's section
 * heads, RASTER, ADVANCE, IDLE WAKEUPS/H, BUDGET, MEMORY, WORKSPACE. See
 * ui.h. `ui_text_tracked` with a track of 0 is a plain draw, which is why
 * `label` can go through it too and the whole zl side ends up on ONE text
 * engine instead of on fb.c's role ladder. */
extern int  ui_text_tracked_w(const char *s, int size, int flags, int track_x10);
extern void ui_text_tracked(int x, int y, const char *s, unsigned rgb,
                            int size, int flags, int track_x10);
/* ui_caps / ui_display are declared with the SYSTEM and TYPE pane's externs
 * further up; they are the same four functions and one declaration is enough.
 *
 * ui.h's UI_F_BOLD, mirrored. This file DECLARES the toolkit rather than
 * including it - every ui_* here is an extern - so the one flag the label
 * bindings need is spelled here, exactly as uikit.c mirrors fb.c's FBT_BOLD
 * for the same reason. If ui.h's value moves, the `ui_txt` binding below
 * breaks at the same moment, because it passes zl's own flag word straight
 * through without ever naming it. */
#define ZL_UI_F_BOLD 2
extern int  ui_pill_w(const char *s, int size, int flags);
extern int  ui_pill_h(int size);
extern int  ui_pill(int x, int y, int w, int h, const char *s,
                    int size, int kind, int flags);
extern int  ui_icon_button(int x, int y, int px, const char *glyph, int active);
extern int  ui_seg_h(int size);
extern int  ui_seg_w(const char *items, int size);
extern int  ui_segmented(int x, int y, int w, int h, const char *items,
                         int sel, int size);
extern int  ui_toolbar_h(void);
extern void ui_toolbar(int x, int y, int w, int h, int at_bottom);
extern int  ui_status_h(void);
extern void ui_statusbar(int x, int y, int w, int h);
extern void ui_stat_begin(int x, int y, int w, int minw);
extern void ui_stat_cell(const char *key, const char *val, unsigned val_rgb);
extern int  ui_stat_end(void);
extern void ui_mono_panel(int x, int y, int w, int h, int kind);
extern int  ui_mono_line_h(int kind);
extern void ui_mono_line(int x, int y, int w, const char *s, unsigned rgb,
                         int kind, int highlight);
extern void ui_card(int x, int y, int w, int h);
extern int  ui_chip_w(const char *s);
extern int  ui_chip_h(void);
extern int  ui_chip(int x, int y, const char *s, int active);
extern int  ui_badge_w(const char *s);
extern int  ui_badge_h(void);
extern void ui_badge(int x, int y, const char *s, unsigned rgb);
extern int  ui_dot_size(void);
extern void ui_dot(int x, int y, unsigned rgb, int glow);
extern int  ui_meter_h(void);
extern void ui_meter(int x, int y, int w, int pct, unsigned rgb);
extern unsigned ui_ink_on(unsigned bg);
extern int  ui_items_count(const char *items);

/* design.h is the ONE file a colour literal may appear in (see its header and
 * hosttest/palette.c). ui_color() publishes the ten theme ROLES; it does not
 * publish WARN, the two dimmest inks, or the hex-byte lime, and those are
 * named by reference-widgets.md S14.2/S14.3 for widgets a zl app draws. This
 * table is a window onto design.h, not a second palette - it adds no value of
 * its own, which is what keeps the single-source rule true. */
#include "../kernel/src/graphics/ui/design.h"
static unsigned zl_design_ink(int i)
{
    switch (i) {
    case 0:  return ZD_WARN;          /* amber: warning, wired to state     */
    case 1:  return ZD_OK;            /* green: healthy/pass                */
    case 2:  return ZD_BAD;           /* red: failure only                  */
    case 3:  return ZD_TEXT_0;        /* emphasis, above body               */
    case 4:  return ZD_TEXT_3;        /* the kernel-log info message ink    */
    case 5:  return ZD_TEXT_5;        /* tertiary: labels, column heads     */
    case 6:  return ZD_TEXT_6;        /* quaternary: hints, timestamps      */
    case 7:  return ZD_SURF_7;        /* the dimmest ink: hex offsets       */
    case 8:  return ZD_ACCENT_LINK;   /* hex bytes                          */
    case 9:  return ZD_ACCENT_BR;     /* live values                        */
    case 10: return ZD_SURF_0;        /* the canvas behind everything       */
    case 11: return ZD_SURF_1;        /* sunken wells                       */
    case 12: return ZD_SURF_2;        /* hairline / terminal ground         */
    case 13: return ZD_SURF_5;        /* menu + input borders               */
    default: return ZD_TEXT_1;        /* body */
    }
}

/* zl has no runtime strings, so a stat cell whose value is a live counter
 * cannot be built on the zl side at all - ui_stat_cell takes a const char *.
 * This is the ONE wrapper: format the integer here and hand the catalogue the
 * string it already wanted. Not a new widget; the same cell, reached with a
 * number. */

/* 32-BIT DIVISION, DELIBERATELY. The obvious `unsigned long long % 10` here
 * links against __udivmoddi4, which divmod.c does not supply (it supplies
 * __divdi3/__moddi3 only) - so the -m32 kernel failed to LINK, and it failed
 * behind a `| tail` that reported exit 0. A counter that needs more than 32
 * bits is saturated rather than given a libgcc dependency. */
static const char *zl_itoa(int v)
{
    static char buf[16];
    int i = (int)sizeof buf - 1;
    int neg = v < 0;
    unsigned u = neg ? (unsigned)(-v) : (unsigned)v;
    buf[i] = 0;
    do { buf[--i] = (char)('0' + (int)(u % 10u)); u /= 10u; } while (u && i > 1);
    if (neg && i > 0) buf[--i] = '-';
    return &buf[i];
}

/* THE SAME, BUT IT KEEPS THE WIDTH `label_num` ALWAYS HAD. That binding used
 * to call console_num_role, which takes a `long` and formats the digits
 * itself; it now formats here so the run can go through the toolkit's type
 * scale rather than fb.c's role ladder, and going through zl_itoa above would
 * have narrowed a 64-bit value to `int` on the ZL_64 build. kernel.zl feeds it
 * byte counts - `fs_free() * fs_bs()` - so 2 GiB is not a theoretical ceiling.
 *
 * `unsigned long % 10u` is safe on BOTH targets for the reason zl_itoa's own
 * note gives and nothing more: on the -m32 kernel `long` is 32 bits, so this
 * is a 32-bit divide and needs no __udivmoddi4; on ZL_64 the compiler emits a
 * native 64-bit div. console_num_role has done exactly this since it landed.
 * The separate buffer matters - one caller may be measuring while another
 * draws, and a shared static would hand the second the first's digits. */
static const char *zl_ltoa(long v)
{
    static char buf[24];
    int i = (int)sizeof buf - 1;
    int neg = v < 0;
    unsigned long u = neg ? (unsigned long)(-v) : (unsigned long)v;
    buf[i] = 0;
    do { buf[--i] = (char)('0' + (int)(u % 10ul)); u /= 10ul; } while (u && i > 1);
    if (neg && i > 0) buf[--i] = '-';
    return &buf[i];
}

/* ---- the browser (browser.c / html.c / layout.c) ------------------------
 * kernel.zl owns the browser app's policy - which window, which keys - and
 * nothing else. The parse, the box model and the paint are all C, because
 * the zl kernel subset has no runtime strings and a tokenizer is nothing
 * but string handling. Same split as term.c's command matcher. */
/* ---- virtio-net (virtio_net.c) -----------------------------------------
 * The network card. The browser's item 1: without it a browser is a file
 * viewer. Everything below is diagnostic or one frame in each direction. */
extern int  virtio_net_find(void);
extern int  virtio_net_init(void);
extern int  virtio_net_present(void);
extern int  virtio_net_ready(void);
extern int  virtio_net_ram_ok(void);
extern int  virtio_net_has_mac(void);
extern int  virtio_net_mac(int i);
extern int  virtio_net_link_up(void);
extern int  virtio_net_tx_count(void);
extern int  virtio_net_rx_count(void);
extern int  virtio_net_rx_drops(void);
extern int  virtio_net_tx_full(void);
extern int  virtio_net_runts(void);
extern int  virtio_net_unwritten(void);
extern int  virtio_net_arp_seen(void);
extern int  virtio_net_ip_seen(void);
extern int  virtio_net_id_reuse(void);
extern int  virtio_net_tr_n(void);
extern int  virtio_net_tr_id(int);
extern int  virtio_net_tr_len(int);
extern int  virtio_net_tr_et(int);
extern int  virtio_net_tx_avail(void);
extern int  virtio_net_tx_used(void);
extern int  virtio_net_rx_avail(void);
extern int  virtio_net_rx_used(void);
extern unsigned int virtio_net_arena(void);
extern int  virtio_net_arp_probe(unsigned int my_ip, unsigned int target_ip, int ms);
extern int  virtio_net_peer_known(void);
extern int  virtio_net_peer_mac(int i);
extern int  netdev_find(void);
extern int  netdev_init(void);
extern int  netdev_send(const unsigned char *, int);
extern int  netdev_poll(unsigned char *, int);
extern int  netdev_mac(int);
extern int  netdev_link_up(void);
extern int  netdev_kind(void);
extern int  netdev_device(void);
extern int  netdev_tx_count(void);
extern int  netdev_rx_count(void);
extern int  netdev_rx_drops(void);
extern int  netdev_tx_full(void);
extern int  netdev_ram_ok(void);

/* ---- the IP stack (net.c) ----------------------------------------------
 * net.c holds no link driver: the link is two function pointers, so the
 * whole stack links into a host harness with no machine. Joining it to
 * virtio_net.c is therefore something someone has to do, and this file is
 * already the place where C subsystems are joined for zl - the same role
 * wmglue.c plays for the compositor. */
extern void net_link(int (*send)(const unsigned char *, int),
                     int (*poll)(unsigned char *, int),
                     const unsigned char mac[6]);
extern void net_config(unsigned int ip, unsigned int mask, unsigned int gw);
extern int  net_poll_once(void);
extern int  net_live(void);
extern unsigned int net_ip(void);
extern int  net_ping(unsigned int ip, int ms);
extern int  net_ping_run(unsigned int ip, int n, int ms);
extern int  net_ping_sent(void);
extern int  net_ping_recv(void);
extern int  net_ping_lost(void);
extern int  net_ping_min(void);
extern int  net_ping_max(void);
extern int  net_ping_avg(void);
extern int  net_ping_jitter(void);
extern int  net_ping_mask(void);
extern int  net_rx_frames(void);
extern int  net_rx_arp(void);
extern int  net_rx_ip(void);
extern int  net_rx_icmp(void);
extern int  net_rx_bad_csum(void);
extern int  net_rx_not_ours(void);
extern int  net_tx_frames(void);
extern int  net_rx_stale_echo(void);
extern int  net_rx_short(void);
extern int  net_rx_badver(void);
extern int  net_rx_badihl(void);
extern int  net_rx_frag(void);
extern int  virtio_net_send(const unsigned char *frame, int len);
extern int  virtio_net_poll(unsigned char *out, int max);
extern int  dhcp_start(void);
extern int  dhcp_poll(void);
extern int  dhcp_state(void);
extern unsigned int dhcp_address(void);
extern unsigned int dhcp_mask(void);
extern unsigned int dhcp_gateway(void);
extern unsigned int dhcp_dns(void);
extern unsigned int dhcp_lease_seconds(void);
extern int  dhcp_retries(void);
extern int  dhcp_bad_packets(void);

/* ---- TCP and HTTP (tcp.c / http.c) -------------------------------------
 * Both take their transport by injection so both run in a host harness with
 * no machine; joining them to net.c is this file's job, as with the IP
 * stack above. */
extern void tcp_attach(int (*out)(unsigned int, int, const unsigned char *, int),
                       unsigned int local_ip);
extern int  tcp_connect(unsigned int ip, int port);
extern int  tcp_state(void);
extern void tcp_close(void);
extern void tcp_abort(void);
extern void tcp_tick(void);
extern void tcp_input(unsigned int src, int proto, const unsigned char *p, int len);
extern int  tcp_rx_segs(void);
extern int  tcp_tx_segs(void);
extern int  tcp_retransmits(void);
extern int  tcp_rx_dup(void);
extern int  tcp_rx_ooo(void);
extern int  tcp_dup_acks(void);
extern int  tcp_rx_bad_csum(void);
extern int  tcp_cwnd(void);
extern int  net_send_ip(unsigned int dst, int proto, const unsigned char *p, int len);
extern void net_set_ip_sink(void (*f)(unsigned int, int, const unsigned char *, int));
extern int  http_start(unsigned int ip, int port, const char *host, const char *path);
extern int  http_poll(void);
extern void http_reset(void);
extern int  http_status(void);
extern int  http_body_len(void);
extern int  http_body_byte(int i);
extern unsigned int http_body_addr(void);
extern int  http_total(void);
extern int  http_truncated(void);
extern int  http_refused(void);
extern int  http_redirects(void);

/* ---- DNS (dns.c) --------------------------------------------------------
 * Names into addresses. Registered as net.c's UDP sink alongside tcp.c's. */
extern void dns_server(unsigned int ip);
extern int  dns_start(const char *name, int len);
extern int  dns_poll(void);
extern int  dns_state(void);
extern unsigned int dns_result(void);
extern void dns_reset(void);
extern void dns_cache_clear(void);
extern int  dns_cache_count(void);
extern int  dns_queries(void);
extern int  dns_replies(void);
extern int  dns_rejected(void);
extern int  dns_cache_hits(void);
extern void dns_ip_sink(unsigned int src, int proto, const unsigned char *p, int len);
extern void net_set_proto_sink(int proto, void (*f)(unsigned int, int, const unsigned char *, int));

extern void browser_go(const char *u, int len);
extern void browser_home(void);
extern void browser_load_mem(unsigned int addr, int len);
extern void browser_draw(int x, int y, int w, int h, int focused);
extern int  browser_key(int code);
extern int  browser_click(int cx, int cy, int btn);
extern int  browser_tick(void);
extern int  browser_code(void);
extern int  browser_doc_len(void);
extern int  browser_back(void);
extern int  browser_can_back(void);
extern int  browser_url_focus(void);
extern const char *browser_title(void);
const char *browser_why(void);
extern int  browser_scroll_by(int d);
extern int  browser_height(void);
extern int  browser_lines(void);
extern int  browser_runs(void);
extern int  browser_status(void);
extern void wm_resize(int win, int w, int h);
extern void wm_geometry(int win, int *x, int *y, int *w, int *h);

extern void input_poll(void);
extern int  input_next(void);
extern int  input_type(void);
extern int  input_code(void);
extern int  input_mods(void);
extern int  input_shift(void);
extern int  input_ctrl(void);
extern int  input_alt(void);
extern int  input_caps(void);
extern int  input_char(void);
extern int  input_key(void);
extern int  input_queued(void);
extern int  input_key_held(int code);
/* the full Intel display driver */
extern int  intel_htotal(void);
extern int  intel_hactive(void);
extern int  intel_vtotal(void);
extern int  intel_vactive(void);
extern unsigned int intel_refresh_mhz(void);
extern unsigned int intel_pixel_clock_khz(void);
extern int  intel_read_edid(void);
extern int  intel_edid_pin(void);
extern int  intel_edid_vendor_char(int i);
extern int  intel_edid_product(void);
extern int  intel_edid_native_w(void);
extern int  intel_edid_native_h(void);
extern int  intel_edid_width_mm(void);
extern int  intel_edid_height_mm(void);
extern unsigned int intel_backlight_max(void);
extern unsigned int intel_backlight_get(void);
extern int  intel_backlight_set(int percent);
extern int  intel_panel_on(void);
/* gpuring.c - the GPU self-test. There is no serial port on the ThinkPad, so
 * every number this exposes exists so kernel.zl can put it on the SCREEN. */
extern int      gpu_selftest(void);
extern unsigned gpu_st_filled(void);
extern unsigned gpu_st_want(void);
extern unsigned gpu_st_poison(void);
extern unsigned gpu_st_ctl(void);
extern unsigned gpu_st_head(void);
extern unsigned gpu_st_tail(void);
extern int      gpu_compositor_enable(int);
extern int      gpu_compositor_live(void);
extern unsigned gpu_present_successes(void);
extern unsigned gpu_present_failures(void);
extern unsigned gpu_present_mismatches(void);

extern int  intel_cursor_enable(unsigned gfx, int size64);
extern int  intel_cursor_move(int x, int y);
extern int  intel_cursor_disable(void);
extern int  intel_flip(unsigned gfx);
extern int  intel_wait_vblank(void);
extern int  intel_flip_count(void);
extern unsigned int intel_plane_format(void);
extern int  intel_plane_tiling(void);
extern int  intel_ggtt_map_range(unsigned page, unsigned phys, int pages);
/* the Intel Gen9 display driver */
extern int  intel_find(void);
/* 1 when a display controller WAS found and refused because its 64-bit BAR is
 * above 4 GiB and this build has 32-bit pointers. intel_find() returns -1 in
 * both cases; this is what tells "there is no Intel GPU" from "there is one and
 * we cannot reach it", which are different things to print. */
extern int  intel_bar_too_high(void);
extern int  intel_present(void);
extern int  intel_supported(void);
extern int  intel_devid(void);
extern unsigned int intel_mmio(void);
extern unsigned int intel_aperture(void);
extern unsigned int intel_stolen_base(void);
extern unsigned int intel_stolen_size(void);
extern unsigned int intel_ggtt_size(void);
extern int  intel_pipe_width(void);
extern int  intel_pipe_height(void);
extern int  intel_stride(void);
/* The whole panel bring-up in one call: pick memory in stolen, map it through
 * the GGTT, run the 34-step modeset, and hand back a CPU-writable framebuffer
 * address - or 0, having left the loader's screen alone. */
extern unsigned int intel_bringup_panel(void);
extern int  intel_shutdown_panel(void);
extern int  intel_bringup_failed_step(void);
extern int  intel_panel_takeover(void);
extern int  intel_plane_enabled(void);
extern int  intel_pipe_enabled(void);
extern unsigned int intel_surface(void);
extern int  intel_frame_count(void);
extern int  console_rows(void);
    /* console_quiet was retired when desktop/exec-track landed: one flag,
     * one builtin, con_mute. See console.c. */

/* ONE byte to COM1, and nothing else. Extracted from zl_putc because there are
 * now two callers with different needs, and inlining it in one of them made the
 * other impossible: term.c generates its own text (the unknown-command line,
 * the echo of what you typed) which has to reach the serial log so a gate can
 * assert on it, but must NOT reach console_putc - during a compositor session
 * console_putc draws glyphs straight into the back buffer at the old text
 * region, which is nowhere near the shell window. Three sinks, chosen
 * per-caller, rather than one bundle nobody can take apart.
 *
 * Wait for the transmit holding register - but never forever. A laptop has no
 * UART at 0x3F8; an undecoded port floats high, so this reads 0xFF and falls
 * straight through, which is why it has always worked. If a machine ever read
 * back zero instead, the kernel would hang inside its FIRST printed character
 * with nothing on screen to say why. The bound is ~1000x one character time at
 * 115200, so a real UART is never cut short. */
void zl_serial_putc(char c)
{
    /* The persistent observer mirrors the serial transcript into RAM from the
     * first byte.  It never performs storage I/O here: this path is also used
     * while xHCI is down and while a storage command is in flight. */
    zllog_putc(c);
    for (int i = 0; i < 200000; i++)
        if (zl_inb(COM1 + 5) & 0x20) break;
    zl_outb(COM1, (unsigned char)c);
}

/* ---- the system track: nvme.c, fs.c, rtc.c, clip.c, notify.c ------------ */
extern int  nvme_read_to(unsigned dst, unsigned lba_lo, unsigned lba_hi);
extern int  nvme_write_from(unsigned src, unsigned lba_lo, unsigned lba_hi);
extern int  nvme_ram_ok(void);
extern int  nvme_fault(void);
extern int  block_service(void);
extern int  block_flush(void);
extern unsigned block_cache_hits(void);
extern unsigned block_cache_misses(void);
extern unsigned block_dirty_blocks(void);
extern unsigned block_dirty_peak(void);
extern unsigned block_refusals(void);
extern unsigned block_completion_us_max(void);
extern unsigned block_writeback_us(void);
extern unsigned block_forced_syncs(void);

extern int  fs_mkfs(void);
extern int  fs_mount(void);
extern int  fs_mounted(void);
extern int  fs_count(void);
extern int  fs_maxfiles(void);
extern int  fs_used(int idx);
extern unsigned fs_size(int idx);
extern unsigned fs_start(int idx);
extern unsigned fs_runlen(int idx);
extern unsigned fs_mtime(int idx);
extern unsigned fs_free_blocks(void);
extern unsigned fs_capacity(void);
extern unsigned fs_bsize(void);
extern int  fs_name_byte(int idx, int i);
extern void fs_name_clear(void);
extern int  fs_name_push(int ch);
extern int  fs_name_pop(void);
extern int  fs_name_stage_len(void);
extern int  fs_name_stage_byte(int i);
extern int  fs_create_named(unsigned bytes);
extern int  fs_find_named(void);
extern int  fs_rename_named(int idx);
extern int  fs_sync(void);
extern int  fs_delete(int idx);
extern int  fs_read(int idx, void *dst, unsigned max);
extern int  fs_write(int idx, const void *src, unsigned bytes);
extern void fs_set_time(unsigned secs);

extern int  rtc_read(void);
extern int  rtc_present(void);
extern int  rtc_valid(void);
extern int  rtc_fail(void);
extern int  rtc_year(void);
extern int  rtc_month(void);
extern int  rtc_day(void);
extern int  rtc_hour(void);
extern int  rtc_min(void);
extern int  rtc_sec(void);
extern unsigned rtc_unix(void);
extern int  rtc_hhmm_byte(int i);

extern unsigned clip_len(void);
extern int  clip_byte(int i);
extern unsigned clip_seq(void);
extern void clip_begin(void);
extern int  clip_push(int ch);
extern int  clip_commit(int type);
extern void clip_clear(void);

extern int  notify_post(const char *text, unsigned ticks);
extern int  notify_tick(unsigned now);
extern int  notify_active(void);
extern int  notify_byte(int i);
extern int  notify_dismiss(void);
extern int  notify_queued(void);

static void zl_putc(char c)
{
    /* screen for a human, serial for verify.sh - both, always, so a
     * headless test still sees everything the user would */
    console_putc(c);
    /* ...and into the terminal's scrollback, so the shell can be REDRAWN.
     * A tee, not a redirect: the console and the serial log below are
     * completely unaffected, which is what keeps verify.sh byte-identical.
     * Without this the compositor has nothing to repaint the shell FROM, and
     * dragging a window across it would erase it permanently. */
    term_putc(c);
    zl_serial_putc(c);
}
#else
/* On Linux, for testing the pipeline: write(1, &c, 1) by raw syscall.
 * No libc, no CRT - this is what proves the freestanding claim. */
static void zl_putc(char c)
{
    long ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(1L), "D"(1L), "S"(&c), "d"(1L)
                     : "rcx", "r11", "memory");
    (void)ret;
}
#endif

static void zl_puts(const char *s) { while (*s) zl_putc(*s++); }

/* the one character sink, exposed so console.c can print hex through it */
void zl_putc_pub(char c) { zl_putc(c); }

/* signed 64-bit to decimal, no snprintf */
static void zl_put_i64(long long v)
{
    char buf[24];
    int  i = 0;
    if (v == 0) { zl_putc('0'); return; }
    if (v < 0) { zl_putc('-'); }
    unsigned long long u = (v < 0) ? (unsigned long long)(-(v + 1)) + 1ULL
                                   : (unsigned long long)v;
    while (u) { buf[i++] = (char)('0' + (u % 10)); u /= 10; }
    while (i) zl_putc(buf[--i]);
}

/* A kernel has no way to report a fault except to say so and stop. */
static void kfatal(const char *msg)
{
    static int recording_panic;
    if (!recording_panic) {
        recording_panic = 1;
        unsigned hash = 2166136261u;
        for (const unsigned char *p = (const unsigned char *)msg; *p; p++)
            hash = (hash ^ *p) * 16777619u;
        /* Stable wire IDs: kernel subsystem 6, panic event 45, fatal 3.
         * Never recurse through USB if the panic came from the recorder. */
        if (zllog_io_active && zllog_io_active()) {
            if (zllog_event_irq) zllog_event_irq(6u, 45u, 3u, hash, 0u, 0u);
        } else if (zllog_event) {
            zllog_event(6u, 45u, 3u, hash, 0u, 0u);
            if (zllog_ready && zllog_ready() && zllog_flush)
                (void)zllog_flush();
        }
    }
    /* ...and it must actually reach a SCREEN. The compositor mutes the
     * console's pixels for the duration of a session (console_mute), so
     * without this the machine would halt having drawn nothing, with the
     * diagnostic in a scrollback that will never be repainted. Dropping the
     * mute costs nothing here: the next statement is an infinite halt. */
#ifdef ZL_KERNEL_SERIAL
    console_unmute();  /* drops the mute for good - the next statement halts */
#endif
    zl_puts("\nkernel runtime error: ");
    zl_puts(msg);
    zl_putc('\n');
    for (;;) {
#ifdef ZL_KERNEL_SERIAL
        __asm__ volatile("hlt");
#else
        __asm__ volatile("syscall" :: "a"(60L), "D"(1L) : "rcx", "r11");
#endif
    }
}

/* ------------------------------------------------------- making values */
/* ZERO THE WHOLE STRUCT, not field by field. This used to assign every member
 * in turn, which stopped working when Value's payload became a union
 * (runtime.h): num, str, items and fnptr now overlap, so nine assignments
 * leave only the last one's bytes behind, and cap/tip are not fields at all.
 *
 * Whole-struct zeroing is also what the rest of the system already depends
 * on. interp.c:44-50 records why: the C backend has always relied on zl_nil
 * clearing everything, because a partially initialised Value gave the
 * interpreter garbage where the backend saw 0, and the two engines
 * disagreed. `= {0}` needs no libc, which matters here. */
Value zl_nil(void)        { Value v = {0}; return v; }
Value zl_num(double n)    { Value v = zl_nil(); v.type = V_NUM;  v.num = n; return v; }
Value zl_bool(int b)      { Value v = zl_nil(); v.type = V_BOOL; v.num = b ? 1 : 0; return v; }

/* A string VALUE needs a heap. A string LITERAL handed straight to print
 * does not, so zl_str keeps the pointer and print is the only consumer. */
Value zl_str(const char *s) { Value v = zl_nil(); v.type = V_STR; v.str = (char *)s; return v; }

Value zl_list_n(int count, ...)
{
    (void)count;
    kfatal("lists are not available in the kernel subset");
    return zl_nil();
}

/* ------------------------------------------------------- using values */
int zl_truthy(Value v)
{
    switch (v.type) {
        case V_NIL:  return 0;
        case V_BOOL: return v.num != 0;
        case V_NUM:  return v.num != 0;
        case V_STR:  return v.str && v.str[0] != '\0';
        default:     return 1;
    }
}

/* Integer semantics on purpose. design_kernel.md §2 is built on
 * nativegen's exact-i64 arithmetic; a kernel doing floating-point div on a
 * descriptor is how you get a wrong GDT entry. Numbers here are whole. */
Value zl_binop(const char *op, Value a, Value b)
{
    if (a.type == V_STR || b.type == V_STR)
        kfatal("string operators are not available in the kernel subset");

    long long x = (long long)a.num, y = (long long)b.num;

    if (op[0] == '+' && !op[1]) return zl_num((double)(x + y));
    if (op[0] == '-' && !op[1]) return zl_num((double)(x - y));
    if (op[0] == '*' && !op[1]) return zl_num((double)(x * y));
    if (op[0] == '/' && !op[1]) { if (!y) kfatal("divide by zero"); return zl_num((double)(x / y)); }
    if (op[0] == '%' && !op[1]) { if (!y) kfatal("modulo by zero");  return zl_num((double)(x % y)); }

    if (op[0] == '=' && op[1] == '=') return zl_bool(x == y);
    if (op[0] == '!' && op[1] == '=') return zl_bool(x != y);
    if (op[0] == '<' && op[1] == '=') return zl_bool(x <= y);
    if (op[0] == '>' && op[1] == '=') return zl_bool(x >= y);
    if (op[0] == '<' && !op[1])       return zl_bool(x <  y);
    if (op[0] == '>' && !op[1])       return zl_bool(x >  y);

    kfatal("operator not available in the kernel subset");
    return zl_nil();
}

Value zl_unop(const char *op, Value a)
{
    if (op[0] == '-' && !op[1]) return zl_num(-(double)(long long)a.num);
    if (op[0] == 'n')           return zl_bool(!zl_truthy(a));   /* not */
    kfatal("unary operator not available in the kernel subset");
    return zl_nil();
}

Value zl_index(Value seq, Value idx)      { (void)seq; (void)idx;
    kfatal("indexing needs lists, which the kernel subset does not have"); return zl_nil(); }
void  zl_set(Value l, Value i, Value v)   { (void)l; (void)i; (void)v;
    kfatal("index-assign needs lists, which the kernel subset does not have"); }
int   zl_len_list(Value v)                { (void)v;
    kfatal("len needs lists, which the kernel subset does not have"); return 0; }
Value zl_item(Value v, int i)             { (void)v; (void)i;
    kfatal("lists are not available in the kernel subset"); return zl_nil(); }

Value zl_fn(void *fnptr, int nargs)       { Value v = zl_nil(); v.type = V_FN;
                                            v.fnptr = fnptr; v.fnargs = nargs; return v; }

Value zl_callv(Value f, int n, ...)       { (void)f; (void)n;
    kfatal("calling a function value is not available in the kernel subset"); return zl_nil(); }

/* --------------------------------------------------------- the builtins
 * Only the ones a kernel may legitimately use. Everything else is a fault
 * with a name, rather than a link error or silence. */
static int streq(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}

Value zl_calln(const char *name, int n, ...)
{
    __builtin_va_list ap;
    Value a[8];
    int i;
    __builtin_va_start(ap, n);
    for (i = 0; i < n && i < 8; i++) a[i] = __builtin_va_arg(ap, Value);
    __builtin_va_end(ap);

    if (streq(name, "print")) {
        for (i = 0; i < n; i++) {
            if (i) zl_putc(' ');
            if (a[i].type == V_STR)       zl_puts(a[i].str);
            else if (a[i].type == V_BOOL) zl_puts(a[i].num ? "true" : "false");
            else if (a[i].type == V_NIL)  zl_puts("nil");
            else                          zl_put_i64((long long)a[i].num);
        }
        zl_putc('\n');
        return zl_nil();
    }

    /* put(x) - print with NO trailing newline. print() always ends the
     * line, which makes "[ OK ] message" impossible to build out of
     * separately coloured pieces. */
    if (streq(name, "put")) {
        for (i = 0; i < n; i++) {
            if (a[i].type == V_STR)       zl_puts(a[i].str);
            else if (a[i].type == V_BOOL) zl_puts(a[i].num ? "true" : "false");
            else if (a[i].type == V_NIL)  zl_puts("nil");
            else                          zl_put_i64((long long)a[i].num);
        }
        return zl_nil();
    }

    if (streq(name, "exit")) {
#ifdef ZL_KERNEL_SERIAL
        for (;;) __asm__ volatile("hlt");
#else
        __asm__ volatile("syscall" :: "a"(60L), "D"((long)(long long)a[0].num) : "rcx", "r11");
#endif
        return zl_nil();
    }

    /* The graphical session is an event loop, not a benchmark. `sti; hlt` is
     * the race-free x86 idle pair: interrupts become visible immediately
     * before the halt and the next timer/input interrupt resumes the loop. */
    if (streq(name, "idle")) {
#ifdef ZL_KERNEL_SERIAL
        /* The observer checkpoints from normal context before sleeping. Its
         * own rate limiter keeps storage completely out of the frame/input
         * hot path and makes any resulting cadence miss visible next frame. */
        zllog_flush_if_due();
        __asm__ volatile("sti; hlt" ::: "memory");
#endif
        return zl_nil();
    }

    /* raw memory - the whole point of a kernel runtime */
    if (streq(name, "peek8"))  return zl_num((double)*(volatile unsigned char  *)(zl_uptr)a[0].num);
    if (streq(name, "peek16")) return zl_num((double)*(volatile unsigned short *)(zl_uptr)a[0].num);
    if (streq(name, "peek32")) return zl_num((double)*(volatile unsigned int   *)(zl_uptr)a[0].num);
    if (streq(name, "poke8"))  { *(volatile unsigned char  *)(zl_uptr)a[0].num = (unsigned char )(unsigned long long)a[1].num; return zl_nil(); }
    if (streq(name, "poke16")) { *(volatile unsigned short *)(zl_uptr)a[0].num = (unsigned short)(unsigned long long)a[1].num; return zl_nil(); }
    if (streq(name, "poke32")) { *(volatile unsigned int   *)(zl_uptr)a[0].num = (unsigned int  )(unsigned long long)a[1].num; return zl_nil(); }

    /* Port I/O - design_kernel.md §6.3 lists these among the intrinsics a
     * kernel backend must provide. Exposing them as builtins is what lets a
     * DRIVER be written in zl rather than in C: the serial and keyboard
     * drivers in kernel.zl are ordinary zl functions that call inb/outb. */
#ifdef ZL_KERNEL_SERIAL
    if (streq(name, "inb"))  return zl_num((double)zl_inb((unsigned short)(unsigned long long)a[0].num));
    if (streq(name, "outb")) { zl_outb((unsigned short)(unsigned long long)a[0].num,
                                       (unsigned char )(unsigned long long)a[1].num); return zl_nil(); }
#endif

#ifdef ZL_KERNEL_SERIAL
    /* the text console, driven from zl - colour, bars and cursor rows are
     * what turn a scrolling log into something that reads as an OS */
    if (streq(name, "cls"))      { console_clear(); return zl_nil(); }
    if (streq(name, "color"))    { console_setcolor((unsigned char)(unsigned long long)a[0].num); return zl_nil(); }
    if (streq(name, "bar"))      { console_bar((int)a[0].num, (unsigned char)(unsigned long long)a[1].num); return zl_nil(); }
    if (streq(name, "at"))       { if (a[2].type == V_STR)
                                       console_at((int)a[0].num, (int)a[1].num, a[2].str,
                                                  (unsigned char)(unsigned long long)a[3].num);
                                   return zl_nil(); }
    if (streq(name, "row"))      return zl_num((double)console_get_row());
    /* the status bar row differs between VGA (24) and a taller
     * framebuffer, so zl asks rather than hardcoding it */
    if (streq(name, "status_row")) return zl_num((double)console_status_row());
    if (streq(name, "con_kind"))   return zl_num((double)console_kind());
    /* Stop the console painting while the compositor owns the screen. The tee
     * into term.c's scrollback and the write to COM1 both continue - see
     * console.c for why that split is the point rather than a shortcut. */
    if (streq(name, "con_mute"))   { console_mute((int)a[0].num); return zl_nil(); }
    /* PROPORTIONAL text, by ROLE - caption / body / title, regular or bold.
     * Everything kernel.zl drew was monospace; only wm.c's window titles were
     * not. See console.c.
     *
     * THEY GO THROUGH THE TOOLKIT NOW, NOT THROUGH console_text_role. Same
     * six arguments, same T_CAPTION/T_BODY/T_TITLE numbers on the zl side,
     * one thing changed underneath: console_text_role is a passthrough to
     * fb_text_role, whose role ladder is floored at 12px, so T_CAPTION and
     * T_BODY BOTH RENDERED AT 12 on the 1920-wide panel. Measured:
     * fb_text_role_w() returns the identical width for the same string at
     * both. The desktop's type scale was two steps, not three, and the
     * prototype's is 11/13/21. ui_text_* resolve through design.h's ZD_T_*
     * instead, so the shell and the widget catalogue are now one scale
     * rather than two - see the note over ui_text_h in uikit.c.
     *
     * `label` is ui_text_tracked with a track of ZERO rather than ui_text,
     * because ui_text gates on ui_mode_get() - ui.c's cursor runs a widget
     * twice, once to hit-test and once to draw - and the shell draws OUTSIDE
     * any cursor pass, where L.mode still holds whatever the last ui_begin()
     * set. Going through the gated form would make the desktop's text vanish
     * after any settings hit-test. ui_text_tracked is the ungated primitive
     * and that is stated at its definition. */
    if (streq(name, "label"))      { if (a[2].type==V_STR) ui_text_tracked((int)a[0].num,(int)a[1].num,a[2].str,(unsigned int)(unsigned long long)a[3].num,(int)a[4].num,(int)a[5].num?ZL_UI_F_BOLD:0,0); return zl_nil(); }
    if (streq(name, "label_w"))    { if (a[0].type==V_STR) return zl_num((double)ui_text_tracked_w(a[0].str,(int)a[1].num,(int)a[2].num?ZL_UI_F_BOLD:0,0)); return zl_num(0.0); }
    if (streq(name, "label_h"))    return zl_num((double)ui_text_h((int)a[0].num));
    if (streq(name, "label_num"))  { ui_text_tracked((int)a[0].num,(int)a[1].num,zl_ltoa((long)a[2].num),(unsigned int)(unsigned long long)a[3].num,(int)a[4].num,(int)a[5].num?ZL_UI_F_BOLD:0,0); return zl_nil(); }
    /* THE TRACKED STYLES ARE NOT REGISTERED A SECOND TIME HERE. `ui_caps` /
     * `ui_capsw` / `ui_disp` / `ui_dispw` are bound further down with the
     * SYSTEM and TYPE panes' builtins and are the same four functions; a
     * second name for one thing is how a shell ends up with two label styles
     * that drift. kernel.zl's shell calls those. */
    if (streq(name, "gradtop"))    { console_gradtop((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num,(unsigned int)(unsigned long long)a[5].num,(unsigned int)(unsigned long long)a[6].num); return zl_nil(); }
    /* v10: translucency, the two gradient shapes, and the blur CACHE. The
     * blur is a cache and not a filter on purpose - one is 7.4 ms and the
     * other is 0.18 ms, and a panel that blurs while it moves pays the first
     * every frame. See fb.c. */
    if (streq(name, "blend"))      { console_blend((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned int)(unsigned long long)a[4].num,(int)a[5].num); return zl_nil(); }
    if (streq(name, "rrblend"))    { console_rrblend((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num,(unsigned int)(unsigned long long)a[5].num,(int)a[6].num); return zl_nil(); }
    if (streq(name, "glow"))       { console_glow((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned int)(unsigned long long)a[4].num,(int)a[5].num,(int)a[6].num,(int)a[7].num); return zl_nil(); }
    if (streq(name, "wall_save"))  return zl_num((double)console_wall_save());
    if (streq(name, "wall_ok"))    return zl_num((double)console_wall_ok());
    if (streq(name, "wall_paint")) { console_wall_paint((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "wedge"))      { console_wedge((int)a[0].num,(int)a[1].num,(unsigned int)(unsigned long long)a[2].num,(int)a[3].num,(int)a[4].num,(int)a[5].num,(int)a[6].num); return zl_nil(); }
    if (streq(name, "clip"))       { console_clip((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "clipoff"))    { console_clip_off(); return zl_nil(); }
    if (streq(name, "blur"))       return zl_num((double)console_blur((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num));
    if (streq(name, "blurdraw"))   { console_blur_paint((int)a[0].num,(int)a[1].num,(int)a[2].num); return zl_nil(); }
    if (streq(name, "blurfree"))   { console_blur_free(); return zl_nil(); }
    /* COM1 only - see support.c. The shell prompt has to reach the serial log
     * even when it is drawn as pixels at the bottom of a window, because that
     * is the string every gate in this repo waits for. */
    if (streq(name, "ser_out"))    { if (a[0].type==V_STR) ser_puts(a[0].str); return zl_nil(); }
    if (streq(name, "vram"))       return zl_num((double)console_vram());
    if (streq(name, "con_cols"))   return zl_num((double)console_cols());
    if (streq(name, "con_rows"))   return zl_num((double)console_rows());
    /* Mute the console's PIXELS while the compositor owns the screen. The
     * scrollback and the serial log are untouched - see console_mute(). */
    /* con_quiet was desktop/exec-track's name for con_mute. One flag,
     * one builtin - kernel.zl used to call both, so they could disagree
     * about who owns the screen. Call sites are rewritten to con_mute. */
    if (streq(name, "cell_w"))     return zl_num((double)console_cell_w());
    /* THE LAYOUT SCALE, which used to be cell_w()/8 in zl and therefore could
     * only ever be 1 or 2. See fb.c: the console cell has two atlases, the
     * layout does not have to. */
    if (streq(name, "ui_scale"))   return zl_num((double)console_ui_scale_q8() / 256.0);
    if (streq(name, "ui_scale_q8"))return zl_num((double)console_ui_scale_q8());
    if (streq(name, "cell_h"))     return zl_num((double)console_cell_h());
    if (streq(name, "bits"))       return zl_num((double)(sizeof(void *) * 8));
    if (streq(name, "hex"))        { console_puthex((unsigned long)(long long)a[0].num, (int)a[1].num); return zl_nil(); }
    if (streq(name, "subpix"))     { fb_set_subpixel((int)a[0].num); return zl_nil(); }
    if (streq(name, "subpix_on"))  return zl_num((double)fb_get_subpixel());
    if (streq(name, "fb_wc"))      return zl_num((double)fb_enable_write_combining());
    if (streq(name, "pci_scan"))   { pci_scan(); return zl_nil(); }
    if (streq(name, "pci_count"))  return zl_num((double)pci_count());
    if (streq(name, "pci_vendor")) return zl_num((double)pci_vendor((int)a[0].num));
    if (streq(name, "pci_device")) return zl_num((double)pci_device((int)a[0].num));
    if (streq(name, "pci_class"))  return zl_num((double)pci_class((int)a[0].num));
    if (streq(name, "wifi_probe")) return zl_num((double)iwlwifi_probe());
    if (streq(name, "wifi_ok"))    return zl_num((double)iwlwifi_present());
    if (streq(name, "wifi_dev"))   return zl_num((double)iwlwifi_device());
    if (streq(name, "wifi_bar_lo"))return zl_num((double)iwlwifi_bar_lo());
    if (streq(name, "wifi_bar_hi"))return zl_num((double)iwlwifi_bar_hi());
    if (streq(name, "wifi_hwif"))  return zl_num((double)iwlwifi_hw_if_config());
    if (streq(name, "wifi_int"))   return zl_num((double)iwlwifi_int_status());
    if (streq(name, "wifi_imask")) return zl_num((double)iwlwifi_int_mask());
    if (streq(name, "wifi_gpio"))  return zl_num((double)iwlwifi_gpio());
    if (streq(name, "wifi_reset")) return zl_num((double)iwlwifi_reset());
    if (streq(name, "wifi_gp"))    return zl_num((double)iwlwifi_gp_cntrl());
    if (streq(name, "wifi_hwrev")) return zl_num((double)iwlwifi_hw_rev());
    if (streq(name, "wifi_hwtype"))return zl_num((double)iwlwifi_hw_type());
    if (streq(name, "wifi_rfid"))  return zl_num((double)iwlwifi_rf_id());
    if (streq(name, "wifi_mac0"))  return zl_num((double)iwlwifi_mac_csr0());
    if (streq(name, "wifi_mac1"))  return zl_num((double)iwlwifi_mac_csr1());
    if (streq(name, "gpu_find"))   return zl_num((double)bga_find());
    if (streq(name, "gpu_present"))return zl_num((double)bga_present());
    if (streq(name, "gpu_version"))return zl_num((double)bga_version());
    if (streq(name, "gpu_fb"))     return zl_num((double)bga_framebuffer());
    if (streq(name, "gpu_vram"))   return zl_num((double)bga_vram_bytes());
    if (streq(name, "gpu_mode"))   return zl_num((double)bga_set_mode((int)a[0].num,(int)a[1].num,(int)a[2].num));
    if (streq(name, "gpu_w"))      return zl_num((double)bga_get_width());
    if (streq(name, "gpu_h"))      return zl_num((double)bga_get_height());
    if (streq(name, "set_res"))    return zl_num((double)console_set_res((int)a[0].num,(int)a[1].num));
    if (streq(name, "gpu_pitch"))  return zl_num((double)bga_get_pitch());
    if (streq(name, "gpu_reg"))    return zl_num((double)bga_reg((int)a[0].num));
    if (streq(name, "usb_find"))   return zl_num((double)xhci_find());
    if (streq(name, "usb_ok"))     return zl_num((double)xhci_present());
    if (streq(name, "usb_ver"))    return zl_num((double)xhci_version());
    if (streq(name, "usb_slots"))  return zl_num((double)xhci_slots());
    if (streq(name, "usb_ports"))  return zl_num((double)xhci_ports());
    if (streq(name, "usb_ctxsz"))  return zl_num((double)xhci_ctx_size());
    if (streq(name, "usb_mmio"))   return zl_num((double)xhci_mmio());
    if (streq(name, "usb_reset"))  return zl_num((double)xhci_reset());
    if (streq(name, "usb_sts"))    return zl_num((double)xhci_usbsts());
    if (streq(name, "usb_cmd"))    return zl_num((double)xhci_usbcmd());
    if (streq(name, "usb_conn"))   return zl_num((double)xhci_port_connected((int)a[0].num));
    if (streq(name, "usb_speed"))  return zl_num((double)xhci_port_speed((int)a[0].num));
    if (streq(name, "usb_count"))  return zl_num((double)xhci_devices_attached());
    if (streq(name, "usb_rings"))  return zl_num((double)xhci_init_rings());
    if (streq(name, "usb_run"))    return zl_num((double)xhci_running());
    if (streq(name, "usb_noop"))   return zl_num((double)xhci_test_noop());
    if (streq(name, "usb_prst"))   return zl_num((double)xhci_port_reset((int)a[0].num));
    if (streq(name, "usb_pen"))    return zl_num((double)xhci_port_enabled((int)a[0].num));
    if (streq(name, "usb_enum"))   return zl_num((double)xhci_enumerate((int)a[0].num));
    if (streq(name, "usb_addr"))   return zl_num((double)xhci_device_address());
    if (streq(name, "usb_vid"))    return zl_num((double)xhci_desc_vendor());
    if (streq(name, "usb_pid"))    return zl_num((double)xhci_desc_product());
    if (streq(name, "usb_uver"))   return zl_num((double)xhci_desc_usbver());
    if (streq(name, "usb_cls"))    return zl_num((double)xhci_desc_class());
    if (streq(name, "usb_mps"))    return zl_num((double)xhci_desc_mps0());
    if (streq(name, "usb_dbyte"))  return zl_num((double)xhci_desc_byte((int)a[0].num));
    if (streq(name, "usb_kbd"))    return zl_num((double)xhci_kbd_init());
    if (streq(name, "usb_kbd_ok")) return zl_num((double)xhci_kbd_ready());
    if (streq(name, "usb_kbd_slot"))return zl_num((double)xhci_kbd_slot());
    if (streq(name, "usb_kbd_ep")) return zl_num((double)xhci_kbd_ep());
    if (streq(name, "usb_poll"))   return zl_num((double)xhci_kbd_poll());
    if (streq(name, "usb_key"))    return zl_num((double)xhci_key());
    if (streq(name, "usb_rep"))    return zl_num((double)xhci_kbd_report((int)a[0].num));
    if (streq(name, "usb_ram"))    return zl_num((double)xhci_ram_ok());
    /* Read the saved settings back. settings_save() has had a caller since it
     * was written - one write per gesture, from settings_flush - and
     * settings_load() has never had one, so every setting was persisted to NVMe
     * and then ignored at boot. It applies on success and never writes. */
    if (streq(name, "set_load"))   return zl_num((double)settings_load());
    if (streq(name, "fs_try"))     return zl_num((double)fs_try_boot());
    if (streq(name, "fs_seed"))    { fs_seed_hello(); return zl_nil(); }
    if (streq(name, "usb_up"))     return zl_num((double)xhci_bringup());
    if (streq(name, "usb_ours"))   return zl_num((double)xhci_owned());
    if (streq(name, "usb_portsc")) return zl_num((double)xhci_portsc((int)a[0].num));
    if (streq(name, "usb_scratch"))return zl_num((double)xhci_scratchpads());
    if (streq(name, "usb_barhi"))  return zl_num((double)xhci_bar_high());
    if (streq(name, "usb_ecm_stage")) return zl_num((double)xhci_ecm_init_stage());
    if (streq(name, "usb_ecm_cfg")) return zl_num((double)xhci_ecm_config_index());
    if (streq(name, "usb_ecm_cc")) return zl_num((double)xhci_ecm_last_cc());
    if (streq(name, "usb_ecm_bits")) return zl_num((double)xhci_ecm_parse_bits());
    if (streq(name, "usb_ecm_len")) return zl_num((double)xhci_ecm_diag_len());
    if (streq(name, "usb_ecm_byte")) return zl_num((double)xhci_ecm_diag_byte((int)a[0].num));
    if (streq(name, "apic_up"))    return zl_num((double)apic_init());
    if (streq(name, "apic_on"))    return zl_num((double)apic_active());
    if (streq(name, "apic_cap"))   return zl_num((double)apic_supported());
    if (streq(name, "apic_lb"))    return zl_num((double)apic_lapic_base());
    if (streq(name, "apic_ib"))    return zl_num((double)apic_ioapic_base());
    if (streq(name, "apic_pins"))  return zl_num((double)apic_ioapic_pins());
    if (streq(name, "apic_madt"))  return zl_num((double)apic_madt_ok());
    if (streq(name, "apic_cpus"))  return zl_num((double)apic_cpus());
    if (streq(name, "apic_rsdp"))  return zl_num((double)apic_rsdp());
    if (streq(name, "apic_gsi"))   return zl_num((double)apic_gsi((int)a[0].num));
    if (streq(name, "apic_id"))    return zl_num((double)apic_id());
    if (streq(name, "apic_red"))   return zl_num((double)apic_redtbl((int)a[0].num));
    if (streq(name, "vg_find"))    return zl_num((double)virtio_gpu_find());
    if (streq(name, "vg_init"))    return zl_num((double)virtio_gpu_init());
    if (streq(name, "vg_ok"))      return zl_num((double)virtio_gpu_present());
    if (streq(name, "vg_ready"))   return zl_num((double)virtio_gpu_ready());
    if (streq(name, "vg_info"))    return zl_num((double)virtio_gpu_display_info());
    if (streq(name, "vg_w"))       return zl_num((double)virtio_gpu_width());
    if (streq(name, "vg_h"))       return zl_num((double)virtio_gpu_height());
    if (streq(name, "vg_scan"))    return zl_num((double)virtio_gpu_scanouts());
    if (streq(name, "vg_setup"))   return zl_num((double)virtio_gpu_setup());
    if (streq(name, "vg_fb"))      return zl_num((double)virtio_gpu_fb());
    if (streq(name, "vg_c2d"))     return zl_num((double)virtio_gpu_create_2d((unsigned)a[0].num,(unsigned)a[1].num,(unsigned)a[2].num));
    if (streq(name, "vg_attach"))  return zl_num((double)virtio_gpu_attach_backing((unsigned)a[0].num,(unsigned)a[1].num));
    if (streq(name, "vg_scanout")) return zl_num((double)virtio_gpu_set_scanout((unsigned)a[0].num,(unsigned)a[1].num,(unsigned)a[2].num));
    if (streq(name, "gpu_ht"))     return zl_num((double)intel_htotal());
    if (streq(name, "gpu_ha"))     return zl_num((double)intel_hactive());
    if (streq(name, "gpu_vt"))     return zl_num((double)intel_vtotal());
    if (streq(name, "gpu_va"))     return zl_num((double)intel_vactive());
    if (streq(name, "gpu_hz"))     return zl_num((double)intel_refresh_mhz());
    if (streq(name, "gpu_clk"))    return zl_num((double)intel_pixel_clock_khz());
    if (streq(name, "edid_read"))  return zl_num((double)intel_read_edid());
    if (streq(name, "edid_pin"))   return zl_num((double)intel_edid_pin());
    if (streq(name, "edid_vc"))    return zl_num((double)intel_edid_vendor_char((int)a[0].num));
    if (streq(name, "edid_prod"))  return zl_num((double)intel_edid_product());
    if (streq(name, "edid_w"))     return zl_num((double)intel_edid_native_w());
    if (streq(name, "edid_h"))     return zl_num((double)intel_edid_native_h());
    if (streq(name, "edid_mmw"))   return zl_num((double)intel_edid_width_mm());
    if (streq(name, "edid_mmh"))   return zl_num((double)intel_edid_height_mm());
    if (streq(name, "bl_max"))     return zl_num((double)intel_backlight_max());
    if (streq(name, "bl_get"))     return zl_num((double)intel_backlight_get());
    if (streq(name, "bl_set"))     return zl_num((double)intel_backlight_set((int)a[0].num));
    if (streq(name, "panel_on"))   return zl_num((double)intel_panel_on());
    if (streq(name, "gpu_test"))    return zl_num((double)gpu_selftest());
    if (streq(name, "gpu_filled"))  return zl_num((double)gpu_st_filled());
    if (streq(name, "gpu_want"))    return zl_num((double)gpu_st_want());
    if (streq(name, "gpu_poison"))  return zl_num((double)gpu_st_poison());
    if (streq(name, "gpu_ctl"))     return zl_num((double)gpu_st_ctl());
    if (streq(name, "gpu_head"))    return zl_num((double)gpu_st_head());
    if (streq(name, "gpu_tail"))    return zl_num((double)gpu_st_tail());
    if (streq(name, "gpu_comp"))    return zl_num((double)gpu_compositor_enable((int)a[0].num));
    if (streq(name, "gpu_comp_on")) return zl_num((double)gpu_compositor_live());
    if (streq(name, "gpu_copies"))  return zl_num((double)gpu_present_successes());
    if (streq(name, "gpu_cfails"))  return zl_num((double)gpu_present_failures());
    if (streq(name, "gpu_cbad"))    return zl_num((double)gpu_present_mismatches());
    if (streq(name, "cur_on"))     return zl_num((double)intel_cursor_enable((unsigned)a[0].num,(int)a[1].num));
    if (streq(name, "cur_move"))   return zl_num((double)intel_cursor_move((int)a[0].num,(int)a[1].num));
    if (streq(name, "cur_off"))    return zl_num((double)intel_cursor_disable());
    if (streq(name, "gpu_flip"))   return zl_num((double)intel_flip((unsigned)a[0].num));
    if (streq(name, "gpu_vbl"))    return zl_num((double)intel_wait_vblank());
    if (streq(name, "gpu_flips"))  return zl_num((double)intel_flip_count());
    /* PIPE_FRMCNT_A, straight. Declared at :636 for years with no binding, so
     * zl could not read the one register every vblank-paced thing starts from -
     * which is also why "it returns 0" was invisible from the desktop. It is 0
     * whenever mmio is 0 (mmio_r's own guard), i.e. whenever intel_find() has
     * not run, so a nonzero answer means BOTH that the driver found the part
     * and that the pipe is scanning out. */
    /* through `unsigned` first: intel_frame_count() hands back an `int`, and
     * PIPE_FRMCNT is a free-running 32-bit counter, so for half of every cycle
     * the top bit is set and a straight (double) cast reports a NEGATIVE frame
     * count. The register is unsigned; say so here rather than changing the
     * driver's signature, which intel.c's own f1 - f0 deltas rely on. */
    if (streq(name, "gpu_frames")) return zl_num((double)(unsigned)intel_frame_count());
    if (streq(name, "gpu_fmt"))    return zl_num((double)intel_plane_format());
    if (streq(name, "gpu_tile"))   return zl_num((double)intel_plane_tiling());
    if (streq(name, "ggtt_map"))   return zl_num((double)intel_ggtt_map_range((unsigned)a[0].num,(unsigned)a[1].num,(int)a[2].num));
    /* ---- the compositor. wm_avail() is the branch kernel.zl takes at boot:
     * it is TRUE only when there is a framebuffer AND kernel.zl defines
     * app_draw. On verify.sh's -kernel -display none there is no framebuffer,
     * so this is 0 and the plain text shell runs exactly as before. */
    if (streq(name, "term_key"))   return zl_num((double)term_key((int)a[0].num));
    if (streq(name, "term_cmd"))   return zl_num((double)term_cmd());
    if (streq(name, "term_bad"))   return zl_num((double)term_unknown());
    if (streq(name, "term_arg"))   return zl_num((double)term_arg());
    if (streq(name, "term_submit")) {
        term_submit((int)a[0].num, (int)a[1].num, (int)a[2].num); return zl_nil();
    }
    if (streq(name, "term_complete")) {
        term_complete((int)a[0].num, (int)a[1].num); return zl_nil();
    }
    if (streq(name, "term_clear")) { term_clear(); return zl_nil(); }
    if (streq(name, "term_draw"))  { term_draw((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,
                                               (unsigned int)(unsigned long long)a[4].num,
                                               (unsigned int)(unsigned long long)a[5].num,
                                               (unsigned int)(unsigned long long)a[6].num,
                                               (int)a[7].num); return zl_nil(); }
    if (streq(name, "wm_avail"))   return zl_num((double)wm_available());
    if (streq(name, "wm_bind"))    return zl_num((double)wm_bind_zl());
    if (streq(name, "wm_init"))    { wm_init(); return zl_nil(); }
    if (streq(name, "wm_open"))    { if (a[1].type==V_STR) return zl_num((double)wm_open((int)a[0].num, a[1].str, (int)a[2].num, (int)a[3].num, (int)a[4].num, (int)a[5].num)); return zl_num(-1.0); }
    if (streq(name, "wm_set_label"))  { if (a[2].type == V_STR) wm_set_label((int)a[0].num, (int)a[1].num, a[2].str); return zl_nil(); }
    if (streq(name, "wm_set_status")) { if (a[1].type == V_STR) wm_set_status((int)a[0].num, a[1].str); return zl_nil(); }
    if (streq(name, "wm_set_field"))  { wm_set_field((int)a[0].num, (int)a[1].num, (int)a[2].num, (int)a[3].num); return zl_nil(); }
    if (streq(name, "wm_tab"))     { if (a[2].type==V_STR) return zl_num((double)wm_add_tab((int)a[0].num, (int)a[1].num, a[2].str)); return zl_num(-1.0); }
    if (streq(name, "wm_close"))   { wm_close((int)a[0].num); return zl_nil(); }
    if (streq(name, "wm_frame"))   { wm_frame(); return zl_nil(); }
    if (streq(name, "wm_run"))     return zl_num((double)wm_running());
    if (streq(name, "wm_stop"))    { wm_stop(); return zl_nil(); }
    if (streq(name, "wm_focus"))   return zl_num((double)wm_focused());
    /* ...and the SETTER, which had no builtin at all. Without it the window
     * that owns the keyboard at boot is whichever one was opened last, and the
     * ordering that decides that is fixed for a different reason entirely -
     * the shell must exist before the boot log prints, because the log goes
     * inside it. So the shell was opened first and the About window got the
     * keys, and every character typed into a freshly booted desktop went to a
     * window with no app_event. Focus is policy; policy lives in zl. */
    if (streq(name, "wm_setfocus")) { wm_focus((int)a[0].num); return zl_nil(); }
    /* "is this window still open" - the question a dock has to ask before it
     * decides between raising an app and launching a second copy of it */
    if (streq(name, "wm_open_p")) return zl_num((double)wm_is_open((int)a[0].num));
    /* WF_MODAL had a setter in wm.c and no caller anywhere - the start menu is
     * the caller, and this is how policy reaches it. */
    if (streq(name, "wm_modal"))   { wm_set_modal((int)a[0].num, (int)a[1].num); return zl_nil(); }
    /* the animation timeline. wm.c had five kinds and no caller in zl at all -
     * built, asserted, and invisible. */
    if (streq(name, "wm_anim"))    return zl_num((double)wm_anim((int)a[0].num, (int)a[1].num));
    /* ...and the four calls the DOCK needs, which is where four of the
     * reference's seven animations live. A dock tile is not a window, so
     * wm_anim_at carries the rectangle to repaint; wm_anim_scale is the press
     * curve sampled per frame; wm_pulse is the infinite one and holds no slot;
     * wm_sweep is the wallpaper band, off in wm.c until policy asks. */
    if (streq(name, "wm_anim_at"))  return zl_num((double)wm_anim_at((int)a[0].num, (int)a[1].num, (int)a[2].num, (int)a[3].num, (int)a[4].num, (int)a[5].num));
    if (streq(name, "wm_anim_scale")) return zl_num((double)wm_anim_scale((int)a[0].num));
    if (streq(name, "wm_pulse"))   return zl_num((double)wm_pulse((int)a[0].num));
    if (streq(name, "wm_sweep"))   { wm_set_sweep((int)a[0].num); return zl_nil(); }
    /* Closing WITH the shrink. wm_close stays exactly as it was - teardown
     * loops and policy reshuffles must not animate - so the gesture form is a
     * separate name rather than a flag nobody would remember to pass. */
    if (streq(name, "wm_close_fx")) { wm_close_fx((int)a[0].num); return zl_nil(); }
    /* THE FRAME TIMER. desktop-TODO 0h, and it should have come first: nothing
     * in this kernel had ever measured a frame, so every performance claim
     * about the compositor was arithmetic rather than measurement. */
    if (streq(name, "wm_us"))      return zl_num((double)wm_frame_us());
    if (streq(name, "wm_peak"))    return zl_num((double)wm_peak_us());
    if (streq(name, "wm_peak0"))   { wm_peak_reset(); return zl_nil(); }
    /* HOW MANY frames missed, which is what a person perceives as stutter -
     * the peak is one sample and the average hides it. wm_late is frames over
     * the 16.67 ms budget; wm_lost is 100 Hz ticks no frame ran in at all. */
    if (streq(name, "wm_late"))    return zl_num((double)wm_late());
    if (streq(name, "wm_lost"))    return zl_num((double)wm_lost());
    if (streq(name, "wm_painted")) return zl_num((double)wm_painted());
    if (streq(name, "wm_budget"))  return zl_num((double)wm_budget_us());
    if (streq(name, "wm_sn"))      return zl_num((double)wm_sample_count());
    if (streq(name, "wm_sf"))      return zl_num((double)wm_sample_frame((int)a[0].num));
    if (streq(name, "wm_si"))      return zl_num((double)wm_sample_input((int)a[0].num));
    if (streq(name, "wm_sbytes"))  return zl_num((double)wm_client_surface_bytes());
    if (streq(name, "wm_srefuse")) return zl_num((double)wm_client_surface_refusals());
    if (streq(name, "wm_rfall"))   return zl_num((double)wm_region_fallbacks());
    if (streq(name, "wm_occpx"))   return zl_num((double)wm_region_occluded_pixels());
    /* the client rect, so an app can turn a screen-space pointer into a row */
    if (streq(name, "wm_cx"))      { int x,y,w,h; wm_client((int)a[0].num,&x,&y,&w,&h); return zl_num((double)x); }
    if (streq(name, "wm_cy"))      { int x,y,w,h; wm_client((int)a[0].num,&x,&y,&w,&h); return zl_num((double)y); }
    if (streq(name, "wm_raise"))   { wm_raise((int)a[0].num); return zl_nil(); }
    if (streq(name, "wm_n"))       return zl_num((double)wm_count());
    /* the window list, for a taskbar: which window is i-th from the back, and
     * which app is in it. A taskbar cannot exist without these. */
    if (streq(name, "wm_zat"))     return zl_num((double)wm_zorder_at((int)a[0].num));
    if (streq(name, "wm_app"))     return zl_num((double)wm_win_app((int)a[0].num));
    /* workspaces. wm_ws/wm_setws are the desktop's current one; wm_winws/
     * wm_setwinws are one window's. All four are the compositor's, so the
     * pips, the taskbar and the filter cannot disagree about which is which. */
    if (streq(name, "wm_ws"))      return zl_num((double)wm_ws());
    if (streq(name, "wm_setws"))   return zl_num((double)wm_set_ws((int)a[0].num));
    if (streq(name, "wm_winws"))   return zl_num((double)wm_win_ws((int)a[0].num));
    if (streq(name, "wm_setwinws"))return zl_num((double)wm_set_win_ws((int)a[0].num,(int)a[1].num));
    if (streq(name, "wm_wsn"))     return zl_num((double)wm_set_ws_n((int)a[0].num));
    /* ---- from desktop/exec-track ------------------------------------------
     * NAMING: exec-track had renamed wm_focus to be the SETTER and wm_focused
     * the getter. On this side wm_focus is the GETTER and wm_setfocus the
     * setter, which is what 17 call sites in kernel.zl already say against
     * exec's 5. Taking both would have left `wm_focus(win)` - a setter call
     * with an argument - resolving to a getter that ignores it, in a language
     * with no arity check. exec's call sites are rewritten instead. */
    if (streq(name, "wm_is_open")) return zl_num((double)wm_is_open((int)a[0].num));
    /* the other two components of the client rect. probe-term.py crops to the
     * rectangle the kernel REPORTS rather than recomputing the layout in
     * Python, which goes stale the first time a window moves. */
    if (streq(name, "wm_cw"))      { int x,y,w,h; wm_client((int)a[0].num,&x,&y,&w,&h); return zl_num((double)w); }
    if (streq(name, "wm_ch"))      { int x,y,w,h; wm_client((int)a[0].num,&x,&y,&w,&h); return zl_num((double)h); }
    if (streq(name, "wm_dmg"))     { wm_invalidate_client((int)a[0].num); return zl_nil(); }
    if (streq(name, "wm_thumb"))   { return zl_num((double)wm_thumb((int)a[0].num,
                                        (int)a[1].num, (int)a[2].num,
                                        (int)a[3].num, (int)a[4].num)); }
    if (streq(name, "wm_cdmg"))    { wm_invalidate_client_rect((int)a[0].num,
                                            (int)a[1].num,(int)a[2].num,
                                            (int)a[3].num,(int)a[4].num); return zl_nil(); }
    if (streq(name, "wm_damage"))  { wm_damage((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "ui_theme"))   { ui_theme_init_q8((int)(a[0].num * 256.0)); return zl_nil(); }
    if (streq(name, "ui_theme_q8")){ ui_theme_init_q8((int)a[0].num); return zl_nil(); }
    if (streq(name, "ui_color"))   return zl_num((double)ui_color((int)a[0].num));
    if (streq(name, "ui_metric"))  return zl_num((double)ui_metric((int)a[0].num));
    /* CONTRAST, x10^4, so 6.4796:1 arrives as 64796. zl has no formatting and
     * no float in the drawing path; the fixed point crosses the seam and
     * kernel.zl's set_dec() prints it. Order-independent - the ratio is
     * defined lighter over darker, so no caller has to know which is on top. */
    if (streq(name, "ui_ratio"))   return zl_num((double)ui_ratio_q4((unsigned)a[0].num,(unsigned)a[1].num));
    if (streq(name, "ui_ceil_dn")) return zl_num((double)ui_ceil_dn_q4((unsigned)a[0].num));
    if (streq(name, "ui_ceil_up")) return zl_num((double)ui_ceil_up_q4((unsigned)a[0].num));
    /* the two live controls. Both return the state they SETTLED on, not the
     * state asked for, because ui_focus_bar_set clamps - a caller that echoed
     * its own argument back into its slider would draw a thumb past the end. */
    if (streq(name, "ui_knock"))   return zl_num((double)ui_knockout_set((int)a[0].num));
    if (streq(name, "ui_knock_on"))return zl_num((double)ui_knockout_get());
    if (streq(name, "ui_fbar"))    return zl_num((double)ui_focus_bar_set((int)a[0].num));
    if (streq(name, "ui_fbar_dp")) return zl_num((double)ui_focus_bar_dp());
    /* the comparison ladder. design.h's ZD_REF_* block by index, so kernel.zl
     * can compute against the parent designs with no colour literal of its
     * own - see ui.h's enum for the numbering. */
    if (streq(name, "ui_ref"))     return zl_num((double)ui_ref_color((int)a[0].num));
    if (streq(name, "ui_refn"))    return zl_num((double)ui_ref_num((int)a[0].num));
    if (streq(name, "ui_begin"))   { ui_begin((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,
                                              (int)a[4].num,(int)a[5].num,(int)a[6].num,(int)a[7].num); return zl_nil(); }
    if (streq(name, "ui_fired"))   return zl_num((double)ui_fired());
    if (streq(name, "ui_label"))   { if (a[0].type==V_STR) ui_label(a[0].str); return zl_nil(); }
    if (streq(name, "ui_dim"))     { if (a[0].type==V_STR) ui_label_dim(a[0].str); return zl_nil(); }
    if (streq(name, "ui_bar"))     { ui_bar((int)a[0].num); return zl_nil(); }
    if (streq(name, "ui_button"))  { if (a[0].type==V_STR) return zl_num((double)ui_button(a[0].str)); return zl_num(0); }
    if (streq(name, "ui_sep"))     { ui_sep(); return zl_nil(); }
    if (streq(name, "ui_space"))   { ui_space((int)a[0].num); return zl_nil(); }
    if (streq(name, "ui_toggle"))  { if (a[0].type==V_STR) return zl_num((double)ui_toggle_value(a[0].str,(int)a[1].num)); return zl_num(a[1].num); }
    if (streq(name, "ui_slider"))  return zl_num((double)ui_slider_value((int)a[0].num,(int)a[1].num,(int)a[2].num));
    if (streq(name, "ui_num"))     { if (a[0].type==V_STR) ui_num(a[0].str,(int)a[1].num); return zl_nil(); }
    if (streq(name, "ui_list"))    { if (a[0].type==V_STR) return zl_num((double)ui_list_row(a[0].str,(int)a[1].num)); return zl_num(0); }
    if (streq(name, "ui_scroll"))  { ui_scroll_begin_value((int)a[0].num,(int)a[1].num); return zl_nil(); }
    if (streq(name, "ui_scroll_end")) return zl_num((double)ui_scroll_end_value());
    if (streq(name, "ui_scroll_content")) return zl_num((double)ui_scroll_content());
    if (streq(name, "ui_row"))     { ui_row(); return zl_nil(); }
    if (streq(name, "ui_endrow"))  { ui_endrow(); return zl_nil(); }
    /* ---- uikit.c. Every one of these is rect-in / fired-out, so an app runs
     * the SAME sequence twice: once under ui_begin(..., 0, ...) to draw and
     * once under ui_begin(..., 1, ex, ey, 1) to find out what was clicked.
     * That is the idiom menu_draw/menu_event in kernel.zl already use. */
    if (streq(name, "ui_ink"))     return zl_num((double)zl_design_ink((int)a[0].num));
    if (streq(name, "ui_ink_on"))  return zl_num((double)ui_ink_on((unsigned)(unsigned long long)a[0].num));
    if (streq(name, "ui_items"))   { if (a[0].type==V_STR) return zl_num((double)ui_items_count(a[0].str)); return zl_num(0.0); }
    if (streq(name, "ui_tw"))      { if (a[0].type==V_STR) return zl_num((double)ui_text_w(a[0].str,(int)a[1].num,(int)a[2].num)); return zl_num(0.0); }
    if (streq(name, "ui_th"))      return zl_num((double)ui_text_h((int)a[0].num));
    /* THE TABLE WIDGETS, which uikit has had all along and zl could not reach.
     * R.log, R.hex, R.net, R.disk and R.regs in the prototype are all TBL() -
     * a column header over grid rows - so every one of them needed these, and
     * without them each app would have hand-drawn its own header and rows. */
    if (streq(name, "ui_colhead")) { if (a[0].type == V_STR) return zl_num(0.0);
                                     if (a[3].type == V_STR)
                                         return zl_num((double)ui_colhead((int)a[0].num,
                                             (int)a[1].num, (int)a[2].num, a[3].str,
                                             (int)a[4].num, (int)a[5].num));
                                     return zl_num(0.0); }
    if (streq(name, "ui_colhead_h")) return zl_num((double)ui_colhead_h());
    /* THE COLUMN TRACKS, which ui_colhead and ui_grid_cell both read. Without
     * this ui_colhead iterates a column count of zero and draws no labels at
     * all - the header band appears and its text does not, which is exactly
     * what the kernel log did on its first render. */
    /* WHERE A COLUMN STARTS AND HOW WIDE IT IS. ui_grid_cell places text inside
     * a track, but the reference's level column is a DOT and then a word, and a
     * dot is not text - it needs the track's own left edge. Without these the
     * only way to place one is to re-derive the track arithmetic in zl, which
     * would be a second copy of ui_grid_span. */
    if (streq(name, "ui_gspanx"))  { int cx = 0, cw = 0;
                                     ui_grid_span((int)a[0].num, (int)a[1].num,
                                                  (int)a[2].num, &cx, &cw);
                                     return zl_num((double)cx); }
    if (streq(name, "ui_gspanw"))  { int cx = 0, cw = 0;
                                     ui_grid_span((int)a[0].num, (int)a[1].num,
                                                  (int)a[2].num, &cx, &cw);
                                     return zl_num((double)cw); }
    if (streq(name, "ui_grid"))    { if (a[0].type == V_STR) ui_grid(a[0].str);
                                     return zl_nil(); }
    /* THE CELL, IN EIGHT ARGUMENTS. A zl native gets Value[8] and
     * ui_grid_cell takes ten, so align, size and the mono flag are packed into
     * one `mode`: size in the low byte, bit 8 right-aligns, bit 9 selects the
     * mono face. That is not a shortcut around the limit - a table cell only
     * ever varies in those three ways, and passing them separately spent three
     * of the eight slots on booleans.
     *
     * zl has no way to turn a number into a string, so the numeric variant
     * carries the same zl_itoa bridge ui_txtn uses - and a table of figures is
     * most of what these apps are. */
#define ZL_CELL_ALIGN_R  0x100
#define ZL_CELL_MONO     0x200
    if (streq(name, "ui_gcelln"))  { int mode = (int)a[7].num;
                                     ui_grid_cell((int)a[0].num, (int)a[1].num,
                                         (int)a[2].num, (int)a[3].num, (int)a[4].num,
                                         zl_itoa((int)a[5].num),
                                         (mode & ZL_CELL_ALIGN_R) ? 1 : 0,
                                         (unsigned)(unsigned long long)a[6].num,
                                         mode & 0xFF,
                                         (mode & ZL_CELL_MONO) ? 1 : 0);
                                     return zl_nil(); }
    if (streq(name, "ui_gcell"))   { int mode = (int)a[7].num;
                                     if (a[5].type == V_STR)
                                         ui_grid_cell((int)a[0].num, (int)a[1].num,
                                             (int)a[2].num, (int)a[3].num, (int)a[4].num,
                                             a[5].str,
                                             (mode & ZL_CELL_ALIGN_R) ? 1 : 0,
                                             (unsigned)(unsigned long long)a[6].num,
                                             mode & 0xFF,
                                             (mode & ZL_CELL_MONO) ? 1 : 0);
                                     return zl_nil(); }
    if (streq(name, "ui_grid"))    { if (a[0].type == V_STR) ui_grid(a[0].str);
                                     return zl_nil(); }
    /* The numeric cell. zl has no way to turn a number into a string, so the
     * same zl_itoa bridge ui_txtn uses serves here - and a table of figures is
     * most of what these apps are. */
    if (streq(name, "ui_growh"))   return zl_num((double)ui_grid_row_h());
    if (streq(name, "ui_grow"))    return zl_num((double)ui_grid_row((int)a[0].num,
                                       (int)a[1].num, (int)a[2].num,
                                       (int)a[3].num, (int)a[4].num));
    if (streq(name, "ui_txt"))     { if (a[2].type==V_STR) ui_text((int)a[0].num,(int)a[1].num,a[2].str,(unsigned)(unsigned long long)a[3].num,(int)a[4].num,(int)a[5].num); return zl_nil(); }
    /* The number forms of ui_text/ui_text_w. zl cannot build "412" - there are
     * no runtime strings - and label_num/num_aa draw unconditionally, which is
     * wrong for an app that runs its widget sequence a second time to
     * hit-test: the hit-test pass would repaint. These go through ui_text, so
     * they honour ui_mode_get() like every other widget. */
    if (streq(name, "ui_txtn"))    { ui_text((int)a[0].num,(int)a[1].num, zl_itoa((int)a[2].num),(unsigned)(unsigned long long)a[3].num,(int)a[4].num,(int)a[5].num); return zl_nil(); }
    if (streq(name, "ui_tnw"))     return zl_num((double)ui_text_w(zl_itoa((int)a[0].num),(int)a[1].num,(int)a[2].num));
    if (streq(name, "ui_pill_w"))  { if (a[0].type==V_STR) return zl_num((double)ui_pill_w(a[0].str,(int)a[1].num,(int)a[2].num)); return zl_num(0.0); }
    if (streq(name, "ui_pill_h"))  return zl_num((double)ui_pill_h((int)a[0].num));
    if (streq(name, "ui_pill"))    { if (a[4].type==V_STR) return zl_num((double)ui_pill((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,a[4].str,(int)a[5].num,(int)a[6].num,(int)a[7].num)); return zl_num(0.0); }
    if (streq(name, "ui_iconbtn")) { if (a[3].type==V_STR) return zl_num((double)ui_icon_button((int)a[0].num,(int)a[1].num,(int)a[2].num,a[3].str,(int)a[4].num)); return zl_num(0.0); }
    if (streq(name, "ui_seg_h"))   return zl_num((double)ui_seg_h((int)a[0].num));
    if (streq(name, "ui_seg_w"))   { if (a[0].type==V_STR) return zl_num((double)ui_seg_w(a[0].str,(int)a[1].num)); return zl_num(0.0); }
    if (streq(name, "ui_seg"))     { if (a[4].type==V_STR) return zl_num((double)ui_segmented((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,a[4].str,(int)a[5].num,(int)a[6].num)); return zl_num(-1.0); }
    if (streq(name, "ui_tb_h"))    return zl_num((double)ui_toolbar_h());
    if (streq(name, "ui_tb"))      { ui_toolbar((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num); return zl_nil(); }
    if (streq(name, "ui_sb_h"))    return zl_num((double)ui_status_h());
    if (streq(name, "ui_sb"))      { ui_statusbar((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "ui_stat"))    { ui_stat_begin((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "ui_statc"))   { if (a[0].type==V_STR && a[1].type==V_STR) ui_stat_cell(a[0].str,a[1].str,(unsigned)(unsigned long long)a[2].num); return zl_nil(); }
    if (streq(name, "ui_statn"))   { if (a[0].type==V_STR) ui_stat_cell(a[0].str, zl_itoa((int)a[1].num),(unsigned)(unsigned long long)a[2].num); return zl_nil(); }
    if (streq(name, "ui_stat_end"))return zl_num((double)ui_stat_end());
    if (streq(name, "ui_mono"))    { ui_mono_panel((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num); return zl_nil(); }
    if (streq(name, "ui_mono_h"))  return zl_num((double)ui_mono_line_h((int)a[0].num));
    if (streq(name, "ui_monoln"))  { if (a[3].type==V_STR) ui_mono_line((int)a[0].num,(int)a[1].num,(int)a[2].num,a[3].str,(unsigned)(unsigned long long)a[4].num,(int)a[5].num,(int)a[6].num); return zl_nil(); }
    if (streq(name, "ui_card"))    { ui_card((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "ui_chip_w"))  { if (a[0].type==V_STR) return zl_num((double)ui_chip_w(a[0].str)); return zl_num(0.0); }
    if (streq(name, "ui_chip_h"))  return zl_num((double)ui_chip_h());
    if (streq(name, "ui_chip"))    { if (a[2].type==V_STR) return zl_num((double)ui_chip((int)a[0].num,(int)a[1].num,a[2].str,(int)a[3].num)); return zl_num(0.0); }
    if (streq(name, "ui_badge_w")) { if (a[0].type==V_STR) return zl_num((double)ui_badge_w(a[0].str)); return zl_num(0.0); }
    if (streq(name, "ui_badge_h")) return zl_num((double)ui_badge_h());
    if (streq(name, "ui_badge"))   { if (a[2].type==V_STR) ui_badge((int)a[0].num,(int)a[1].num,a[2].str,(unsigned)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "ui_dot_sz"))  return zl_num((double)ui_dot_size());
    if (streq(name, "ui_dot"))     { ui_dot((int)a[0].num,(int)a[1].num,(unsigned)(unsigned long long)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "ui_meter_h")) return zl_num((double)ui_meter_h());
    if (streq(name, "ui_meter"))   { ui_meter((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned)(unsigned long long)a[4].num); return zl_nil(); }
    /* ---- THE LABEL AND DISPLAY STYLES, REACHABLE FROM zl -------------------
     * uikit.c's own comment over text_run() says these are "registered as zl
     * builtins" and, when the SYSTEM and TYPE panes were written against them,
     * they were not - the three widgets inside uikit.c could draw the tracked
     * label style and nothing in kernel.zl could. That is the exact shape of
     * gap this file exists to close: a C function with no registration is
     * unreachable, and an unreachable style is a style the shell does not have.
     *
     * ui_caps bakes in ZD_TR_LAB and uppercases; ui_disp bakes in ZD_TR_BIG.
     * Each has a `w` twin because the measure and the draw MUST come from the
     * same loop or a tracked label clips at its right edge - which is what
     * design.h warns about in the same breath as the token, and what a
     * right-flushed table cell in these two panes would hit first. */
    if (streq(name, "ui_caps"))    { if (a[2].type==V_STR) ui_caps((int)a[0].num,(int)a[1].num,a[2].str,(unsigned)(unsigned long long)a[3].num,(int)a[4].num); return zl_nil(); }
    if (streq(name, "ui_capsw"))   { if (a[0].type==V_STR) return zl_num((double)ui_caps_w(a[0].str,(int)a[1].num)); return zl_num(0.0); }
    if (streq(name, "ui_disp"))    { if (a[2].type==V_STR) ui_display((int)a[0].num,(int)a[1].num,a[2].str,(unsigned)(unsigned long long)a[3].num,(int)a[4].num); return zl_nil(); }
    if (streq(name, "ui_dispw"))   { if (a[0].type==V_STR) return zl_num((double)ui_display_w(a[0].str,(int)a[1].num)); return zl_num(0.0); }
    /* ---- WHAT TYPE IS IN THIS IMAGE ---------------------------------------
     * sizeof() over the real font arrays, so a regenerated atlas moves the
     * TYPE pane and a font dropped from kernel/SOURCES stops the link. Contrast
     * is NOT here: ui_ratio above already is it.
     *
     * A native with no registration is dead code and this tree has paid for
     * that mistake, so every one of these seven has a call site in kernel.zl's
     * type_body(). */
    if (streq(name, "atlas_n"))    return zl_num((double)ui_atlas_n());
    if (streq(name, "atlas_w"))    return zl_num((double)ui_atlas_w((int)a[0].num));
    if (streq(name, "atlas_h"))    return zl_num((double)ui_atlas_h((int)a[0].num));
    if (streq(name, "atlas_g"))    return zl_num((double)ui_atlas_glyphs((int)a[0].num));
    if (streq(name, "atlas_face")) return zl_num((double)ui_atlas_face((int)a[0].num));
    if (streq(name, "atlas_in"))   return zl_num((double)ui_atlas_in_image((int)a[0].num));
    if (streq(name, "atlas_role")) return zl_num((double)ui_atlas_for_role((int)a[0].num,(int)a[1].num));
    /* ---- the browser. Everything below is one app's policy surface. */
    /* ---- virtio-net. net_up() is the one that does the work; everything
     * else reports what happened, because a driver that fails silently is
     * indistinguishable from one that is not there. */
    if (streq(name, "net_find"))   return zl_num((double)netdev_find());
    /* ip_up(ip, mask, gw): bring the card up, then hand net.c the link. Two
     * steps rather than one because a card that works and a stack that is
     * misconfigured are different failures and should report separately. */
    if (streq(name, "ip_up")) {
        if (!netdev_init()) return zl_num(0.0);
        unsigned char m[6];
        for (int k = 0; k < 6; k++) m[k] = (unsigned char)netdev_mac(k);
        net_link(netdev_send, netdev_poll, m);
        net_config((unsigned)a[0].num, (unsigned)a[1].num, (unsigned)a[2].num);
        return zl_num(1.0);
    }
    if (streq(name, "ip_auto")) {
        if (!netdev_init()) return zl_num(0.0);
        unsigned char m[6];
        for (int k = 0; k < 6; k++) m[k] = (unsigned char)netdev_mac(k);
        net_link(netdev_send, netdev_poll, m);
        return zl_num((double)dhcp_start());
    }
    if (streq(name, "dhcp_work")) {
        int before = dhcp_state();
        int changed = dhcp_poll();
        if (before != 3 && dhcp_state() == 3) {
            unsigned int ip = dhcp_address();
            tcp_attach(net_send_ip, ip);
            net_set_proto_sink(6, tcp_input);
            net_set_proto_sink(17, dns_ip_sink);
            dns_server(dhcp_dns());
            dns_cache_clear();
        }
        return zl_num((double)changed);
    }
    if (streq(name, "dhcp_state")) return zl_num((double)dhcp_state());
    if (streq(name, "dhcp_ip"))    return zl_num((double)dhcp_address());
    if (streq(name, "dhcp_mask"))  return zl_num((double)dhcp_mask());
    if (streq(name, "dhcp_gw"))    return zl_num((double)dhcp_gateway());
    if (streq(name, "dhcp_dns"))   return zl_num((double)dhcp_dns());
    if (streq(name, "dhcp_lease")) return zl_num((double)dhcp_lease_seconds());
    if (streq(name, "dhcp_retry")) return zl_num((double)dhcp_retries());
    if (streq(name, "dhcp_bad"))   return zl_num((double)dhcp_bad_packets());
    if (streq(name, "net_attached")) return zl_num((double)net_live());
    if (streq(name, "ip_live"))    return zl_num((double)(net_live() && net_ip() != 0));
    /* tcp_up(): hand tcp.c the IP layer and register it as net.c's sink for
     * everything that is not ICMP. Two calls, one place, once. */
    if (streq(name, "tcp_up")) {
        tcp_attach(net_send_ip, (unsigned)a[0].num);
        net_set_proto_sink(6, tcp_input);
        return zl_num(1.0);
    }
    /* dns_up(server): route UDP to the resolver and say who to ask. On QEMU's
     * user-mode network the server is 10.0.2.3; on a real link it is whatever
     * DHCP would have said, and there is no DHCP - so it is asserted, exactly
     * like the address is. */
    if (streq(name, "dns_up")) {
        net_set_proto_sink(17, dns_ip_sink);
        dns_server((unsigned)a[0].num);
        dns_cache_clear();
        return zl_num(1.0);
    }
    if (streq(name, "dns_ask")) {
        if (a[0].type != V_STR) return zl_num(0.0);
        const char *nm = a[0].str; int nl = 0; while (nm[nl]) nl++;
        dns_reset();
        return zl_num((double)dns_start(nm, nl));
    }
    if (streq(name, "dns_poll"))  return zl_num((double)dns_poll());
    if (streq(name, "dns_ip"))    return zl_num((double)dns_result());
    if (streq(name, "dns_n"))     return zl_num((double)dns_cache_count());
    if (streq(name, "dns_q"))     return zl_num((double)dns_queries());
    if (streq(name, "dns_r"))     return zl_num((double)dns_replies());
    if (streq(name, "dns_bad"))   return zl_num((double)dns_rejected());
    if (streq(name, "tcp_open"))   return zl_num((double)tcp_connect((unsigned)a[0].num,(int)a[1].num));
    if (streq(name, "tcp_st"))     return zl_num((double)tcp_state());
    if (streq(name, "tcp_shut"))   { tcp_close(); return zl_nil(); }
    if (streq(name, "tcp_kill"))   { tcp_abort(); return zl_nil(); }
    if (streq(name, "tcp_tick"))   { tcp_tick(); return zl_nil(); }
    if (streq(name, "tcp_rx"))     return zl_num((double)tcp_rx_segs());
    if (streq(name, "tcp_tx"))     return zl_num((double)tcp_tx_segs());
    if (streq(name, "tcp_rexmit")) return zl_num((double)tcp_retransmits());
    if (streq(name, "tcp_dup"))    return zl_num((double)tcp_rx_dup());
    if (streq(name, "tcp_ooo"))    return zl_num((double)tcp_rx_ooo());
    if (streq(name, "tcp_dupack")) return zl_num((double)tcp_dup_acks());
    if (streq(name, "tcp_badsum")) return zl_num((double)tcp_rx_bad_csum());
    if (streq(name, "tcp_cwnd"))   return zl_num((double)tcp_cwnd());
    if (streq(name, "http_get")) {
        if (a[2].type != V_STR || a[3].type != V_STR) return zl_num(0.0);
        return zl_num((double)http_start((unsigned)a[0].num,(int)a[1].num,a[2].str,a[3].str));
    }
    if (streq(name, "http_poll"))  return zl_num((double)http_poll());
    if (streq(name, "http_reset")) { http_reset(); return zl_nil(); }
    if (streq(name, "http_code"))  return zl_num((double)http_status());
    if (streq(name, "http_len"))   return zl_num((double)http_body_len());
    if (streq(name, "http_byte"))  return zl_num((double)http_body_byte((int)a[0].num));
    if (streq(name, "http_addr"))  return zl_num((double)http_body_addr());
    if (streq(name, "http_total")) return zl_num((double)http_total());
    if (streq(name, "http_trunc")) return zl_num((double)http_truncated());
    if (streq(name, "http_refused")) return zl_num((double)http_refused());
    if (streq(name, "http_redir")) return zl_num((double)http_redirects());
    if (streq(name, "ip_poll"))    return zl_num((double)net_poll_once());
    if (streq(name, "ip_ping"))    return zl_num((double)net_ping((unsigned)a[0].num,(int)a[1].num));
    if (streq(name, "ip_run"))     return zl_num((double)net_ping_run((unsigned)a[0].num,(int)a[1].num,(int)a[2].num));
    if (streq(name, "ip_sent"))    return zl_num((double)net_ping_sent());
    if (streq(name, "ip_recv"))    return zl_num((double)net_ping_recv());
    if (streq(name, "ip_lost"))    return zl_num((double)net_ping_lost());
    if (streq(name, "ip_min"))     return zl_num((double)net_ping_min());
    if (streq(name, "ip_max"))     return zl_num((double)net_ping_max());
    if (streq(name, "ip_avg"))     return zl_num((double)net_ping_avg());
    if (streq(name, "ip_jit"))     return zl_num((double)net_ping_jitter());
    if (streq(name, "ip_mask"))    return zl_num((double)net_ping_mask());
    if (streq(name, "ip_rxf"))     return zl_num((double)net_rx_frames());
    if (streq(name, "ip_rxarp"))   return zl_num((double)net_rx_arp());
    if (streq(name, "ip_rxip"))    return zl_num((double)net_rx_ip());
    if (streq(name, "ip_rxicmp"))  return zl_num((double)net_rx_icmp());
    if (streq(name, "ip_badsum"))  return zl_num((double)net_rx_bad_csum());
    if (streq(name, "ip_notours")) return zl_num((double)net_rx_not_ours());
    if (streq(name, "ip_txf"))     return zl_num((double)net_tx_frames());
    if (streq(name, "ip_stale"))   return zl_num((double)net_rx_stale_echo());
    if (streq(name, "ip_short"))   return zl_num((double)net_rx_short());
    if (streq(name, "ip_badver"))  return zl_num((double)net_rx_badver());
    if (streq(name, "ip_badihl"))  return zl_num((double)net_rx_badihl());
    if (streq(name, "ip_frag"))    return zl_num((double)net_rx_frag());
    if (streq(name, "net_up"))     return zl_num((double)netdev_init());
    if (streq(name, "net_there"))  return zl_num((double)virtio_net_present());
    if (streq(name, "net_ok"))     return zl_num((double)virtio_net_ready());
    if (streq(name, "net_ram"))    return zl_num((double)netdev_ram_ok());
    if (streq(name, "net_hasmac")) return zl_num((double)virtio_net_has_mac());
    if (streq(name, "net_mac"))    return zl_num((double)netdev_mac((int)a[0].num));
    if (streq(name, "net_link"))   return zl_num((double)netdev_link_up());
    if (streq(name, "net_kind"))   return zl_num((double)netdev_kind());
    if (streq(name, "net_devid"))  return zl_num((double)netdev_device());
    if (streq(name, "net_tx"))     return zl_num((double)netdev_tx_count());
    if (streq(name, "net_rx"))     return zl_num((double)netdev_rx_count());
    if (streq(name, "net_drop"))   return zl_num((double)netdev_rx_drops());
    if (streq(name, "net_txfull")) return zl_num((double)netdev_tx_full());
    if (streq(name, "net_runt"))   return zl_num((double)virtio_net_runts());
    if (streq(name, "net_unwrit")) return zl_num((double)virtio_net_unwritten());
    if (streq(name, "net_seenarp")) return zl_num((double)virtio_net_arp_seen());
    if (streq(name, "net_seenip"))  return zl_num((double)virtio_net_ip_seen());
    if (streq(name, "net_idreuse")) return zl_num((double)virtio_net_id_reuse());
    if (streq(name, "net_trn"))    return zl_num((double)virtio_net_tr_n());
    if (streq(name, "net_trid"))   return zl_num((double)virtio_net_tr_id((int)a[0].num));
    if (streq(name, "net_trlen"))  return zl_num((double)virtio_net_tr_len((int)a[0].num));
    if (streq(name, "net_tret"))   return zl_num((double)virtio_net_tr_et((int)a[0].num));
    if (streq(name, "net_txa"))    return zl_num((double)virtio_net_tx_avail());
    if (streq(name, "net_txu"))    return zl_num((double)virtio_net_tx_used());
    if (streq(name, "net_rxa"))    return zl_num((double)virtio_net_rx_avail());
    if (streq(name, "net_rxu"))    return zl_num((double)virtio_net_rx_used());
    if (streq(name, "net_arena"))  return zl_num((double)virtio_net_arena());
    if (streq(name, "net_arp"))    return zl_num((double)virtio_net_arp_probe((unsigned)a[0].num,(unsigned)a[1].num,(int)a[2].num));
    if (streq(name, "net_peerok")) return zl_num((double)virtio_net_peer_known());
    if (streq(name, "net_peer"))   return zl_num((double)virtio_net_peer_mac((int)a[0].num));
    if (streq(name, "br_home"))    { browser_home(); return zl_nil(); }
    /* br_go(url): exactly what the URL bar does, so the gate exercises the
     * path a person uses rather than a parallel one built for the test. */
    if (streq(name, "br_go")) {
        if (a[0].type != V_STR) return zl_num(0.0);
        const char *u = a[0].str; int ul = 0; while (u[ul]) ul++;
        browser_go(u, ul);
        return zl_num(1.0);
    }
    if (streq(name, "br_load"))    { browser_load_mem((unsigned)a[0].num, (int)a[1].num); return zl_nil(); }
    if (streq(name, "br_draw"))    { browser_draw((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num); return zl_nil(); }
    if (streq(name, "br_key"))     return zl_num((double)browser_key((int)a[0].num));
    if (streq(name, "br_click"))   return zl_num((double)browser_click((int)a[0].num,(int)a[1].num,(int)a[2].num));
    if (streq(name, "br_tick"))    return zl_num((double)browser_tick());
    if (streq(name, "br_back"))    return zl_num((double)browser_back());
    if (streq(name, "br_code"))    return zl_num((double)browser_code());
    if (streq(name, "br_doclen"))  return zl_num((double)browser_doc_len());
    if (streq(name, "br_why"))     return zl_str(browser_why());
    if (streq(name, "br_focus"))   return zl_num((double)browser_url_focus());
    if (streq(name, "br_state"))   return zl_num((double)browser_status());
    if (streq(name, "br_h"))       return zl_num((double)browser_height());
    if (streq(name, "br_title"))   return zl_str(browser_title());
    if (streq(name, "br_scroll"))  return zl_num((double)browser_scroll_by((int)a[0].num));
    if (streq(name, "br_h"))       return zl_num((double)browser_height());
    if (streq(name, "br_lines"))   return zl_num((double)browser_lines());
    if (streq(name, "br_runs"))    return zl_num((double)browser_runs());
    if (streq(name, "br_status"))  return zl_num((double)browser_status());
    /* wm_resize existed and had no caller and no way to reach it. The browser
     * is the first thing that needs it: reflow is only observable if the
     * window can change width while the machine is running. */
    if (streq(name, "wm_size"))    { wm_resize((int)a[0].num,(int)a[1].num,(int)a[2].num); return zl_nil(); }
    if (streq(name, "wm_w"))       { int gx,gy,gw,gh; wm_geometry((int)a[0].num,&gx,&gy,&gw,&gh); return zl_num((double)gw); }
    if (streq(name, "wm_hh"))      { int gx,gy,gw,gh; wm_geometry((int)a[0].num,&gx,&gy,&gw,&gh); return zl_num((double)gh); }
    if (streq(name, "in_poll"))    { input_poll(); return zl_nil(); }
    if (streq(name, "in_next"))    return zl_num((double)input_next());
    if (streq(name, "in_type"))    return zl_num((double)input_type());
    if (streq(name, "in_code"))    return zl_num((double)input_code());
    if (streq(name, "in_mods"))    return zl_num((double)input_mods());
    if (streq(name, "in_shift"))   return zl_num((double)input_shift());
    if (streq(name, "in_ctrl"))    return zl_num((double)input_ctrl());
    if (streq(name, "in_alt"))     return zl_num((double)input_alt());
    if (streq(name, "in_caps"))    return zl_num((double)input_caps());
    if (streq(name, "in_char"))    return zl_num((double)input_char());
    if (streq(name, "in_key"))     return zl_num((double)input_key());
    if (streq(name, "in_queued"))  return zl_num((double)input_queued());
    if (streq(name, "in_held"))    return zl_num((double)input_key_held((int)a[0].num));
    if (streq(name, "i2c_find"))   return zl_num((double)i2c_find((int)a[0].num));
    if (streq(name, "i2c_ok"))     return zl_num((double)i2c_present());
    if (streq(name, "i2c_dw"))     return zl_num((double)i2c_is_designware());
    if (streq(name, "i2c_mmio"))   return zl_num((double)i2c_mmio());
    if (streq(name, "i2c_ct"))     return zl_num((double)i2c_comp_type());
    if (streq(name, "tp_probe"))   return zl_num((double)i2c_hid_probe());
    if (streq(name, "tp_ok"))      return zl_num((double)i2c_hid_ready());
    if (streq(name, "tp_addr"))    return zl_num((double)i2c_hid_address());
    if (streq(name, "tp_vid"))     return zl_num((double)i2c_hid_vid());
    if (streq(name, "tp_pid"))     return zl_num((double)i2c_hid_pid());
    if (streq(name, "tp_ver"))     return zl_num((double)i2c_hid_version());
    if (streq(name, "tp_maxin"))   return zl_num((double)i2c_hid_max_input());
    if (streq(name, "tp_rdlen"))   return zl_num((double)i2c_hid_rdesc_len());
    if (streq(name, "tp_read"))    return zl_num((double)i2c_hid_read_report());
    if (streq(name, "tp_b"))       return zl_num((double)i2c_hid_byte((int)a[0].num));
    if (streq(name, "tp_devid"))   return zl_num((double)i2c_hid_device_id());
    if (streq(name, "tp_abort"))   return zl_num((double)i2c_hid_abort_source());
    if (streq(name, "tp_hcnt"))    return zl_num((double)i2c_hid_fs_hcnt());
    if (streq(name, "tp_lcnt"))    return zl_num((double)i2c_hid_fs_lcnt());
    if (streq(name, "tp_reset"))   return zl_num((double)i2c_hid_lpss_reset());
    if (streq(name, "tp_service")) return zl_num((double)i2c_hid_service());
    if (streq(name, "tp_ptr"))     return zl_num((double)i2c_hid_pointer_ready());
    if (streq(name, "tp_reports")) return zl_num((double)i2c_hid_ptr_reports());
    if (streq(name, "tp_bad"))     return zl_num((double)i2c_hid_ptr_malformed());
    if (streq(name, "msc_up"))     return zl_num((double)xhci_msc_init());
    if (streq(name, "msc_ok"))     return zl_num((double)xhci_msc_ready());
    if (streq(name, "msc_slot"))   return zl_num((double)xhci_msc_slot());
    if (streq(name, "msc_inq"))    return zl_num((double)xhci_msc_inquiry());
    if (streq(name, "msc_cap"))    return zl_num((double)xhci_msc_read_capacity());
    if (streq(name, "msc_read"))   return zl_num((double)xhci_msc_read_block((unsigned)a[0].num));
    if (streq(name, "msc_b"))      return zl_num((double)xhci_msc_byte((int)a[0].num));
    if (streq(name, "msc_blocks")) return zl_num((double)xhci_msc_blocks());
    if (streq(name, "msc_bsize"))  return zl_num((double)xhci_msc_blocksize());
    if (streq(name, "msc_mb"))     return zl_num((double)xhci_msc_capacity_mb());
    if (streq(name, "msc_istage")) return zl_num((double)xhci_msc_init_stage());
    if (streq(name, "msc_iport"))  return zl_num((double)xhci_msc_init_port());
    if (streq(name, "msc_islot"))  return zl_num((double)xhci_msc_init_slot());
    if (streq(name, "msc_icc"))    return zl_num((double)xhci_msc_init_cc());
    if (streq(name, "msc_ivid"))   return zl_num((double)xhci_msc_init_vid());
    if (streq(name, "msc_ipid"))   return zl_num((double)xhci_msc_init_pid());
    if (streq(name, "diag_up"))    return zl_num((double)zllog_mount());
    if (streq(name, "diag_on"))    return zl_num((double)zllog_ready());
    if (streq(name, "diag_save"))  return zl_num((double)zllog_flush());
    if (streq(name, "diag_done"))  return zl_num((double)zllog_complete());
    if (streq(name, "diag_mark")) {
        zllog_milestone((unsigned int)a[0].num, 0u);
        return zl_num((double)zllog_flush());
    }
    if (streq(name, "diag_buf"))   return zl_num((double)zllog_buffered());
    if (streq(name, "diag_drop"))  return zl_num((double)zllog_dropped());
    if (streq(name, "diag_err"))   return zl_num((double)zllog_last_error());
    if (streq(name, "smp_go"))     return zl_num((double)smp_start());
    if (streq(name, "smp_n"))      return zl_num((double)smp_online());
    if (streq(name, "smp_total"))  return zl_num((double)smp_cpu_count());
    if (streq(name, "smp_last"))   return zl_num((double)smp_last_id());
    if (streq(name, "smp_mask"))   return zl_num((double)smp_mask());
    if (streq(name, "smp_tsz"))    return zl_num((double)smp_tramp_size());
    if (streq(name, "smp_id"))     return zl_num((double)apic_cpu_id((int)a[0].num));
    if (streq(name, "sched_go"))   return zl_num((double)sched_start_demo());
    if (streq(name, "sched_on"))   return zl_num((double)sched_active());
    if (streq(name, "sched_n"))    return zl_num((double)sched_count());
    if (streq(name, "sched_cur"))  return zl_num((double)sched_current());
    if (streq(name, "sched_sw"))   return zl_num((double)sched_switches());
    if (streq(name, "smp_jobs"))   return zl_num((double)smp_band_wakes());
    if (streq(name, "sched_st"))   return zl_num((double)sched_state((int)a[0].num));
    if (streq(name, "sched_tk"))   return zl_num((double)sched_ticks((int)a[0].num));
    if (streq(name, "counter"))    return zl_num((double)sched_counter((int)a[0].num));
    if (streq(name, "yield"))      { yield(); return zl_nil(); }
    if (streq(name, "nv_find"))    return zl_num((double)nvme_find());
    if (streq(name, "nv_ok"))      return zl_num((double)nvme_present());
    if (streq(name, "nv_setup"))   return zl_num((double)nvme_setup());
    if (streq(name, "nv_ready"))   return zl_num((double)nvme_ready());
    if (streq(name, "nv_ver"))     return zl_num((double)nvme_version());
    if (streq(name, "nv_mmio"))    return zl_num((double)nvme_mmio());
    if (streq(name, "nv_mb"))      return zl_num((double)nvme_model_byte((int)a[0].num));
    if (streq(name, "nv_sb"))      return zl_num((double)nvme_serial_byte((int)a[0].num));
    if (streq(name, "nv_blocks"))  return zl_num((double)nvme_blocks_lo());
    if (streq(name, "nv_bsize"))   return zl_num((double)nvme_blocksize());
    if (streq(name, "nv_cap"))     return zl_num((double)nvme_capacity_mb());
    if (streq(name, "nv_read"))    return zl_num((double)nvme_read_block((unsigned)a[0].num,(unsigned)a[1].num));
    if (streq(name, "nv_write"))   return zl_num((double)nvme_write_block((unsigned)a[0].num,(unsigned)a[1].num));
    if (streq(name, "nv_get"))     return zl_num((double)nvme_data_byte((int)a[0].num));
    if (streq(name, "nv_set"))     { nvme_data_set((int)a[0].num,(int)a[1].num); return zl_nil(); }
    if (streq(name, "cpu_vb"))     return zl_num((double)cpu_vendor_byte((int)a[0].num));
    if (streq(name, "cpu_fam"))    return zl_num((double)cpu_family());
    if (streq(name, "cpu_mod"))    return zl_num((double)cpu_model());
    if (streq(name, "cpu_step"))   return zl_num((double)cpu_stepping());
    if (streq(name, "cpu_cores"))  return zl_num((double)cpu_cores());
    if (streq(name, "cpu_thr"))    return zl_num((double)cpu_threads());
    if (streq(name, "cpu_tpc"))    return zl_num((double)cpu_threads_per_core());
    if (streq(name, "cpu_apicid")) return zl_num((double)cpu_apic_id());
    if (streq(name, "cpu_khz"))    return zl_num((double)cpu_tsc_khz());
    if (streq(name, "cpu_mhz"))    return zl_num((double)cpu_mhz());
    /* THE FULL 64-BIT TSC, not its bottom half.
     *
     * This called cpu_tsc_lo(), which is a u32 - so zl saw a counter that
     * WRAPPED EVERY 1.8 SECONDS at 2.4 GHz. A frame timer built on it reads
     * correctly most of the time and returns a large negative number a few
     * times a minute, which is the kind of intermittent wrong answer that gets
     * blamed on the thing being measured rather than the clock.
     *
     * A zl number is a double, which holds an exact integer to 2^53 - about
     * 43 days at 2.4 GHz, against the 1.8 s it had. cpu_tsc_lo stays exposed
     * as cpu_tsc32 for anything that genuinely wants the low half. */
    if (streq(name, "cpu_tsc"))    return zl_num((double)cpu_tsc());
    if (streq(name, "cpu_tsc32"))  return zl_num((double)cpu_tsc_lo());
    if (streq(name, "cpu_inv"))    return zl_num((double)cpu_tsc_invariant());
    if (streq(name, "cpu_ctype"))  return zl_num((double)cpu_cache_type((int)a[0].num));
    if (streq(name, "cpu_clevel")) return zl_num((double)cpu_cache_level((int)a[0].num));
    if (streq(name, "cpu_ckb"))    return zl_num((double)cpu_cache_kb((int)a[0].num));
    if (streq(name, "cpu_temp"))   return zl_num((double)cpu_temp_c());
    if (streq(name, "cpu_hvb"))    return zl_num((double)cpu_hypervisor_byte((int)a[0].num));
    if (streq(name, "cpu_sse2"))   return zl_num((double)cpu_has_sse2());
    if (streq(name, "cpu_sse42"))  return zl_num((double)cpu_has_sse42());
    if (streq(name, "cpu_avx"))    return zl_num((double)cpu_has_avx());
    if (streq(name, "cpu_avx2"))   return zl_num((double)cpu_has_avx2());
    if (streq(name, "cpu_aes"))    return zl_num((double)cpu_has_aes());
    if (streq(name, "cpu_rdrand")) return zl_num((double)cpu_has_rdrand());
    if (streq(name, "cpu_hv"))     return zl_num((double)cpu_has_hypervisor());
    if (streq(name, "vg_test"))    return zl_num((double)virtio_gpu_testpattern());
    if (streq(name, "vg_resp"))    return zl_num((double)virtio_gpu_last_resp());
    if (streq(name, "vg_flush"))   return zl_num((double)virtio_gpu_flush((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num));
    if (streq(name, "intel_find"))  return zl_num((double)intel_find());
    if (streq(name, "intel_hibar")) return zl_num((double)intel_bar_too_high());
    if (streq(name, "intel_ok"))    return zl_num((double)intel_supported());
    if (streq(name, "intel_id"))    return zl_num((double)intel_devid());
    if (streq(name, "intel_mmio"))  return zl_num((double)intel_mmio());
    if (streq(name, "intel_aper"))  return zl_num((double)intel_aperture());
    if (streq(name, "intel_sbase")) return zl_num((double)intel_stolen_base());
    if (streq(name, "intel_ssize")) return zl_num((double)intel_stolen_size());
    if (streq(name, "intel_ggtt"))  return zl_num((double)intel_ggtt_size());
    if (streq(name, "intel_w"))     return zl_num((double)intel_pipe_width());
    if (streq(name, "intel_h"))     return zl_num((double)intel_pipe_height());
    if (streq(name, "intel_stride"))return zl_num((double)intel_stride());
    if (streq(name, "panel_up"))    return zl_num((double)intel_bringup_panel());
    if (streq(name, "panel_down"))  return zl_num((double)intel_shutdown_panel());
    if (streq(name, "panel_step"))  return zl_num((double)intel_bringup_failed_step());
    if (streq(name, "panel_console"))return zl_num((double)intel_panel_takeover());
    if (streq(name, "intel_plane")) return zl_num((double)intel_plane_enabled());
    if (streq(name, "intel_pipe"))  return zl_num((double)intel_pipe_enabled());
    if (streq(name, "intel_surf"))  return zl_num((double)intel_surface());
    if (streq(name, "loader"))     return zl_num((double)console_loader());
    if (streq(name, "px_w"))       return zl_num((double)console_pxw());
    if (streq(name, "px_h"))       return zl_num((double)console_pxh());
    if (streq(name, "fill_rect")) { console_fill((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned char)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "gradient"))  { console_gradient((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned char)(unsigned long long)a[4].num,(unsigned char)(unsigned long long)a[5].num); return zl_nil(); }
    if (streq(name, "logo"))      { if (a[2].type==V_STR) console_logo((int)a[0].num,(int)a[1].num,a[2].str,(int)a[3].num,(unsigned char)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "cursor"))    { console_cursor((int)a[0].num,(int)a[1].num,(int)a[2].num,(unsigned char)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "region"))    { console_set_region((int)a[0].num,(int)a[1].num); return zl_nil(); }
    if (streq(name, "setup_gdt")) { gdt_init(); return zl_nil(); }
    if (streq(name, "setup_idt")) { idt_init(); return zl_nil(); }
    if (streq(name, "ticks"))     return zl_num((double)idt_ticks());
    if (streq(name, "scan_get"))  return zl_num((double)idt_scan());
    if (streq(name, "at_num"))    { console_at_num((int)a[0].num,(int)a[1].num,(long)a[2].num,(unsigned char)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "fill_rgb"))  { console_fill_rgb((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned int)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "grad_rgb"))  { console_gradient_rgb((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned int)(unsigned long long)a[4].num,(unsigned int)(unsigned long long)a[5].num); return zl_nil(); }
    if (streq(name, "text_rgb"))  { if (a[2].type==V_STR) console_text_rgb((int)a[0].num,(int)a[1].num,a[2].str,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "get_px"))    return zl_num((double)console_get_px((int)a[0].num,(int)a[1].num));
    if (streq(name, "shade"))     { console_shade((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num,(int)a[5].num); return zl_nil(); }
    if (streq(name, "shadow"))    { console_shadow((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num,(int)a[5].num); return zl_nil(); }
    if (streq(name, "rrect"))     { console_rrect((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num,(unsigned int)(unsigned long long)a[5].num); return zl_nil(); }
    if (streq(name, "text_aa"))   { if (a[2].type==V_STR) console_text_aa((int)a[0].num,(int)a[1].num,a[2].str,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "text_big"))  { if (a[2].type==V_STR) console_text_aa2x((int)a[0].num,(int)a[1].num,a[2].str,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "num_aa"))    { console_num_aa((int)a[0].num,(int)a[1].num,(long)a[2].num,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "char_aa"))   { console_char_aa((int)a[0].num,(int)a[1].num,(int)a[2].num,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "text_box"))  { console_set_text_box((int)a[0].num,(int)a[1].num); return zl_nil(); }
    if (streq(name, "cube"))      { console_cube((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned int)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "mpoint"))    { console_pointer_show((int)a[0].num,(int)a[1].num); return zl_nil(); }
    if (streq(name, "mhide"))     { console_pointer_hide(); return zl_nil(); }
    if (streq(name, "present"))   { console_present(); return zl_nil(); }
    if (streq(name, "icon"))      { console_icon((int)a[0].num,(int)a[1].num,(int)a[2].num,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    /* bg_snap / bg_rest / grab / stamp are GONE, C4. They were the
     * snapshot-and-sticker drag; the compositor repaints from damage and needs
     * no snapshot. See the note where they lived in fb.c. */
    if (streq(name, "cube3d"))    { console_cube_filled((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned int)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "cube_clip")) { console_cube_clip((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "cpu_char"))  return zl_num((double)cpu_brand_byte((int)a[0].num));
    if (streq(name, "emit"))      { zl_putc((char)(int)a[0].num); return zl_nil(); }
    if (streq(name, "sc"))        { console_putc((char)(int)a[0].num); return zl_nil(); }
    if (streq(name, "col"))       return zl_num((double)console_get_col());
    if (streq(name, "beep_on"))   { speaker_on((unsigned)(long long)a[0].num); return zl_nil(); }
    if (streq(name, "beep_off"))  { speaker_off(); return zl_nil(); }
    if (streq(name, "reboot"))    { kreboot(); return zl_nil(); }
    /* Prefer the USB pointer. A tablet reports an ABSOLUTE position, so the
     * guest cursor sits exactly where the host one is with nothing to drift;
     * the PS/2 mouse is relative and stays as the fallback (and is what the
     * laptop's TrackPoint actually is). Polled here because the pointer shares
     * the keyboard's event ring and something has to turn the handle. */
    if (streq(name, "mouse_x"))   { if (usb_ptr_ok()) { xhci_ptr_poll(); return zl_num((double)xhci_ptr_x()); }
                                    return zl_num((double)idt_mouse_x()); }
    if (streq(name, "mouse_y"))   { if (usb_ptr_ok()) return zl_num((double)xhci_ptr_y());
                                    return zl_num((double)idt_mouse_y()); }
    if (streq(name, "mouse_btn")) { if (usb_ptr_ok()) return zl_num((double)xhci_ptr_btn());
                                    return zl_num((double)idt_mouse_btn()); }
    if (streq(name, "ptr_abs"))    return zl_num((double)(usb_ptr_ok() ? xhci_ptr_abs() : 0));
    /* 1 if the USB pointer driver is even linked in. Distinguishes "no driver"
     * from "driver present, no device" - which look identical from ptr_ready
     * alone, and cost a long hunt once already. */
    if (streq(name, "ptr_driver")) return zl_num(xhci_ptr_ready != 0 ? 1.0 : 0.0);
    if (streq(name, "ptr_reports"))return zl_num((double)ZL_WEAK_CALL(xhci_ptr_reports));
    if (streq(name, "ptr_events")) return zl_num((double)ZL_WEAK_CALL(xhci_ptr_events));
    if (streq(name, "ptr_lastcc")) return zl_num((double)ZL_WEAK_CALL(xhci_ptr_lastcc));
    if (streq(name, "kbd_events")) return zl_num((double)ZL_WEAK_CALL(xhci_kbd_events));
    if (streq(name, "kbd_requeues"))return zl_num((double)ZL_WEAK_CALL(xhci_kbd_requeues));
    if (streq(name, "kbd_lastcc")) return zl_num((double)ZL_WEAK_CALL(xhci_kbd_lastcc));
    if (streq(name, "ptr_slot"))   return zl_num((double)ZL_WEAK_CALL(xhci_ptr_slot));
    if (streq(name, "ptr_ep"))     return zl_num((double)ZL_WEAK_CALL(xhci_ptr_ep));
    if (streq(name, "mouse_irqs")) return zl_num((double)idt_mouse_irqs());
    /* ---- the program arena (arena.c) --------------------------------------
     * arena_up prints its own line, the way fb.c does, so the boot log states
     * an ADDRESS rather than a claim - a number somebody can check against the
     * map in fb.c and in arena.c's header comment. */
    /* ---- the general heap (heap.c) ----------------------------------------
     * heap_up prints its own line for the same reason arena_up does. heap_chk
     * is the one that matters operationally: it walks every block by boundary
     * tag and returns 0 if the heap still adds up, so "is the heap sound" is a
     * command somebody can type rather than a thing to hope about. It is O(the
     * number of blocks) and deliberately NOT on the alloc/free path. */
    /* vmm_up prints one line with ADDRESSES in it - "64 MiB mapped: virtual
     * 4096 MiB -> physical 256 MiB" is a fact somebody can check against
     * memmap.h; "virtual memory is on" would not be. */
    /* ring 3. user_up runs the self-test, which prints from BOTH sides of the
     * privilege boundary - the "u3" in the boot log is produced by syscalls
     * made from ring 3 and can be produced no other way. */
    if (streq(name, "user_up"))       { user_selftest(); return zl_num((double)user_call_count()); }
    if (streq(name, "user_calls"))    return zl_num((double)user_call_count());
#if defined(ZL_64)
    if (streq(name, "user_file"))     return zl_num((double)user64_run_default_file());
#else
    if (streq(name, "user_file"))     return zl_num(-64.0);
#endif
    /* Dedicated destructive diagnostic. The ordinary shell never invokes it;
     * term.c accepts only the exact word `crashtest`, and the QEMU gate uses
     * that route to prove vector 6 reaches the bounded crash recorder. */
    if (streq(name, "crash_test"))    { crash_test_ud2(); return zl_nil(); }

    if (streq(name, "vmm_up"))        { vmm_report(); return zl_num((double)vmm_active()); }
    if (streq(name, "vmm_on"))        return zl_num((double)vmm_active());

    if (streq(name, "heap_up"))       return zl_num((double)heap_init());
    if (streq(name, "heap_ok"))       return zl_num((double)heap_ok());
    if (streq(name, "heap_cap"))      return zl_num((double)heap_capacity());
    if (streq(name, "heap_used"))     return zl_num((double)heap_used());
    if (streq(name, "heap_free"))     return zl_num((double)heap_available());
    if (streq(name, "heap_hw"))       return zl_num((double)heap_high_water());
    if (streq(name, "heap_refused"))  return zl_num((double)heap_refusals());
    if (streq(name, "heap_blocks"))   return zl_num((double)heap_blocks());
    if (streq(name, "heap_chk"))      return zl_num((double)heap_check());

    if (streq(name, "arena_up"))      return zl_num((double)arena_init());
    if (streq(name, "arena_ok"))      return zl_num((double)arena_ok());
    if (streq(name, "arena_cap"))     return zl_num((double)arena_capacity());
    if (streq(name, "arena_used"))    return zl_num((double)arena_used());
    if (streq(name, "arena_free"))    return zl_num((double)arena_available());
    if (streq(name, "arena_hw"))      return zl_num((double)arena_high_water());
    if (streq(name, "arena_refused")) return zl_num((double)arena_refusals());
    if (streq(name, "arena_base"))    return zl_num((double)arena_base_addr());
    if (streq(name, "arena_resets"))  return zl_num((double)arena_resets());
    /* The one that makes the rest of the design work, and it was missed on the
     * first pass: without a reset exposed, a bump allocator is a one-shot.
     * `run` calls this BEFORE each program, never after - reclaiming on the way
     * in means a program that faulted still has its memory intact to look at,
     * and means nothing is holding a pointer into the arena at the moment it
     * is reclaimed except code that is about to be handed a new one. */
    if (streq(name, "arena_reset"))  { arena_reset(); return zl_nil(); }
    /* ---- exec.c -----------------------------------------------------------
     * exec_run takes NO arguments on purpose. The filename lives in term.c's
     * buffer and exec.c reads it there, in C. Passing it through here would
     * mean a V_STR in zl's hands, and zl_binop above hard-faults on any string
     * operand BEFORE it reaches the `==` arm - so `if n == "hello.zl"`
     * compiles clean, links clean, and halts the machine when it runs. */
    if (streq(name, "exec_run"))    return zl_num((double)exec_run());
    if (streq(name, "exec_state"))  return zl_num((double)exec_state());
    if (streq(name, "exec_window")) return zl_num((double)exec_wants_window());
    /* A pointer into exec.c's own buffer, handed straight to wm_open as a
     * title and never compared. Passing a V_STR is safe; operating on one is
     * not, and that is the entire distinction. */
    if (streq(name, "exec_title"))  return zl_str(exec_title());
    if (streq(name, "exec_draw"))   { exec_draw((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,
                                                (unsigned int)(unsigned long long)a[4].num,
                                                (unsigned int)(unsigned long long)a[5].num,
                                                (unsigned int)(unsigned long long)a[6].num);
                                      return zl_nil(); }
    if (streq(name, "box"))       { console_box((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned char)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "line"))      { console_line((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned char)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "mcursor"))   { console_mouse_cursor((int)a[0].num,(int)a[1].num,(unsigned char)(unsigned long long)a[2].num,(unsigned char)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "goto_row")) { console_set_row((int)a[0].num); return zl_nil(); }

    /* ---- the system track: storage, time, clipboard ---------------------
     * Appended, never interleaved, so this block is one contiguous diff and
     * three other sessions editing this file do not have to merge through it.
     *
     * The naming follows the file each one lives in rather than the zl verb,
     * because when a builtin misbehaves the first question is always which
     * driver it landed in. */

    /* the disk, at an ARBITRARY address rather than the one fixed page.
     * nv_read/nv_write above still exist and still use the bounce buffer;
     * these are what a filesystem can actually be built on. */
    if (streq(name, "nv_rd_to"))   return zl_num((double)nvme_read_to((unsigned)a[0].num,(unsigned)a[1].num,(unsigned)a[2].num));
    if (streq(name, "nv_wr_from")) return zl_num((double)nvme_write_from((unsigned)a[0].num,(unsigned)a[1].num,(unsigned)a[2].num));
    if (streq(name, "nv_ram"))     return zl_num((double)nvme_ram_ok());
    if (streq(name, "nv_fault"))   return zl_num((double)nvme_fault());

    /* zlfs. Names are pushed one character at a time and compared in C - the
     * zl kernel subset has string literals and no string values, so two
     * runtime strings can never meet in zl. Same seam term.c uses. */
    if (streq(name, "fs_format"))  return zl_num((double)fs_mkfs());
    if (streq(name, "fs_mount"))   return zl_num((double)fs_mount());
    if (streq(name, "fs_ok"))      return zl_num((double)fs_mounted());
    if (streq(name, "fs_n"))       return zl_num((double)fs_count());
    if (streq(name, "fs_max"))     return zl_num((double)fs_maxfiles());
    if (streq(name, "fs_inuse"))   return zl_num((double)fs_used((int)a[0].num));
    if (streq(name, "fs_bytes"))   return zl_num((double)fs_size((int)a[0].num));
    if (streq(name, "fs_lba"))     return zl_num((double)fs_start((int)a[0].num));
    if (streq(name, "fs_run"))     return zl_num((double)fs_runlen((int)a[0].num));
    if (streq(name, "fs_when"))    return zl_num((double)fs_mtime((int)a[0].num));
    if (streq(name, "fs_ch"))      return zl_num((double)fs_name_byte((int)a[0].num,(int)a[1].num));
    if (streq(name, "fs_free"))    return zl_num((double)fs_free_blocks());
    if (streq(name, "fs_cap"))     return zl_num((double)fs_capacity());
    if (streq(name, "fs_bs"))      return zl_num((double)fs_bsize());
    if (streq(name, "fs_nclear"))  { fs_name_clear(); return zl_nil(); }
    if (streq(name, "fs_npush"))   return zl_num((double)fs_name_push((int)a[0].num));
    if (streq(name, "fs_npop"))    return zl_num((double)fs_name_pop());
    if (streq(name, "fs_nlen"))    return zl_num((double)fs_name_stage_len());
    if (streq(name, "fs_nch"))     return zl_num((double)fs_name_stage_byte((int)a[0].num));
    if (streq(name, "fs_new"))     return zl_num((double)fs_create_named((unsigned)a[0].num));
    if (streq(name, "fs_get"))     return zl_num((double)fs_find_named());
    if (streq(name, "fs_rename"))  return zl_num((double)fs_rename_named((int)a[0].num));
    if (streq(name, "fs_rm"))      return zl_num((double)fs_delete((int)a[0].num));
    if (streq(name, "fs_rd"))      return zl_num((double)fs_read((int)a[0].num,(void *)(zl_uptr)a[1].num,(unsigned)a[2].num));
    if (streq(name, "fs_wr"))      return zl_num((double)fs_write((int)a[0].num,(const void *)(zl_uptr)a[1].num,(unsigned)a[2].num));
    if (streq(name, "fs_sync"))    return zl_num((double)fs_sync());
    if (streq(name, "blk_work"))   return zl_num((double)block_service());
    if (streq(name, "blk_flush"))  return zl_num((double)block_flush());
    if (streq(name, "blk_hit"))    return zl_num((double)block_cache_hits());
    if (streq(name, "blk_miss"))   return zl_num((double)block_cache_misses());
    if (streq(name, "blk_dirty"))  return zl_num((double)block_dirty_blocks());
    if (streq(name, "blk_dpeak"))  return zl_num((double)block_dirty_peak());
    if (streq(name, "blk_refuse")) return zl_num((double)block_refusals());
    if (streq(name, "blk_latmax")) return zl_num((double)block_completion_us_max());
    if (streq(name, "blk_wbus"))   return zl_num((double)block_writeback_us());
    if (streq(name, "blk_syncs"))  return zl_num((double)block_forced_syncs());
    if (streq(name, "fs_stamp"))   { fs_set_time((unsigned)a[0].num); return zl_nil(); }

    /* the clock */
    if (streq(name, "rtc_up"))     return zl_num((double)rtc_read());
    if (streq(name, "rtc_here"))   return zl_num((double)rtc_present());
    if (streq(name, "rtc_ok"))     return zl_num((double)rtc_valid());
    if (streq(name, "rtc_why"))    return zl_num((double)rtc_fail());
    if (streq(name, "rtc_y"))      return zl_num((double)rtc_year());
    if (streq(name, "rtc_mo"))     return zl_num((double)rtc_month());
    if (streq(name, "rtc_d"))      return zl_num((double)rtc_day());
    if (streq(name, "rtc_h"))      return zl_num((double)rtc_hour());
    if (streq(name, "rtc_mi"))     return zl_num((double)rtc_min());
    if (streq(name, "rtc_s"))      return zl_num((double)rtc_sec());
    if (streq(name, "rtc_epoch"))  return zl_num((double)rtc_unix());
    if (streq(name, "rtc_ch"))     return zl_num((double)rtc_hhmm_byte((int)a[0].num));

    /* the clipboard */
    if (streq(name, "clip_n"))     return zl_num((double)clip_len());
    if (streq(name, "clip_ch"))    return zl_num((double)clip_byte((int)a[0].num));
    if (streq(name, "clip_seq"))   return zl_num((double)clip_seq());
    if (streq(name, "clip_new"))   { clip_begin(); return zl_nil(); }
    if (streq(name, "clip_add"))   return zl_num((double)clip_push((int)a[0].num));
    if (streq(name, "clip_done"))  return zl_num((double)clip_commit(1));
    if (streq(name, "clip_wipe"))  { clip_clear(); return zl_nil(); }

    /* notifications */
    if (streq(name, "note_tick"))  return zl_num((double)notify_tick((unsigned)a[0].num));
    if (streq(name, "note_on"))    return zl_num((double)notify_active());
    if (streq(name, "note_ch"))    return zl_num((double)notify_byte((int)a[0].num));
    if (streq(name, "note_go"))    return zl_num((double)notify_dismiss());
    if (streq(name, "note_q"))     return zl_num((double)notify_queued());
    /* A string LITERAL is the one kind of string the zl kernel subset has, and
     * it is exactly what a notification is: fixed text chosen at compile time.
     * Same shape as the `at` builtin above - check the type, take the pointer,
     * never store it. */
    if (streq(name, "note_say")) {
        if (a[0].type != V_STR) return zl_num(0);
        return zl_num((double)notify_post(a[0].str, (unsigned)(n > 1 ? a[1].num : 0)));
    }
#endif

    /* Bitwise ops. A driver cannot be written without them - every status
     * register is read by masking a bit. Same names as runtime.c's. */
    if (streq(name, "band")) return zl_num((double)((long long)a[0].num & (long long)a[1].num));
    if (streq(name, "bor"))  return zl_num((double)((long long)a[0].num | (long long)a[1].num));
    if (streq(name, "bxor")) return zl_num((double)((long long)a[0].num ^ (long long)a[1].num));
    if (streq(name, "bnot")) return zl_num((double)(~(long long)a[0].num));
    if (streq(name, "shl"))  return zl_num((double)((long long)a[0].num << (long long)a[1].num));
    if (streq(name, "shr"))  return zl_num((double)((unsigned long long)a[0].num >> (long long)a[1].num));
    /* peek64/poke64 carry the 2^53 hazard (design_kernel.md §2); the two
     * -halves rule means a kernel should not need them for descriptors. */
    if (streq(name, "peek64")) {
        unsigned long long v = *(volatile unsigned long long *)(zl_uptr)a[0].num;
        if (v > 9007199254740992ULL) kfatal("peek64 above 2^53 - read two peek32 halves");
        return zl_num((double)v);
    }
    if (streq(name, "poke64")) {
        unsigned long long v = (unsigned long long)a[1].num;
        if (v > 9007199254740992ULL) kfatal("poke64 above 2^53 - write two poke32 halves");
        *(volatile unsigned long long *)(zl_uptr)a[0].num = v;
        return zl_nil();
    }
    if (streq(name, "fill_mem")) {
        volatile unsigned char *p = (volatile unsigned char *)(zl_uptr)a[0].num;
        unsigned long long cnt = (unsigned long long)a[2].num;
        unsigned char b = (unsigned char)(unsigned long long)a[1].num;
        while (cnt--) *p++ = b;
        return zl_nil();
    }
    if (streq(name, "copy_mem")) {
        volatile unsigned char *d = (volatile unsigned char *)(zl_uptr)a[0].num;
        volatile const unsigned char *s = (volatile const unsigned char *)(zl_uptr)a[1].num;
        unsigned long long cnt = (unsigned long long)a[2].num;
        if (d < s) { while (cnt--) *d++ = *s++; }
        else       { d += cnt; s += cnt; while (cnt--) *--d = *--s; }
        return zl_nil();
    }

    /* NAME THE BUILTIN. This said only "builtin not available in the kernel
     * subset" and halted, which costs a whole build-and-boot cycle to turn into
     * a symbol - and the boot is the expensive half. The message is assembled
     * here rather than passed through kfatal's hash, because the hash is a wire
     * ID for the flight recorder and two different missing builtins must not
     * look like one incident.
     *
     * It matters more than it looks: check-zlcalls.py reported "every call site
     * resolves" while this fired, so the checker and the kernel disagree about
     * what a resolved call is, and without the name there is nothing to take
     * back to the checker. */
    {
        static char miss[96];
        const char *pre = "builtin not available in the kernel subset: ";
        int i = 0;
        while (pre[i] && i < (int)sizeof miss - 1) { miss[i] = pre[i]; i++; }
        for (int j = 0; name[j] && i < (int)sizeof miss - 1; j++) miss[i++] = name[j];
        miss[i] = 0;
        kfatal(miss);
    }
    return zl_nil();
}
