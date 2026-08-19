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
#include "../runtime.h"

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
extern int  console_blur(int x, int y, int w, int h, int r);
extern void console_blur_paint(int slot, int x, int y);
extern void console_blur_free(void);
extern void ser_puts(const char *s);
extern unsigned long console_vram(void);
extern int  console_cols(void);
extern int  console_cell_w(void);
extern int  console_ui_scale(void);
extern int  console_cell_h(void);
extern void fb_set_subpixel(int on);
extern int  fb_get_subpixel(void);
/* the PCI bus driver and our own modesetting driver */
extern void pci_scan(void);
extern int  pci_count(void);
extern int  pci_vendor(int i);
extern int  pci_device(int i);
extern int  pci_class(int i);
extern int  pci_find_class(int cls, int sub);
extern unsigned int pci_bar(int i, int which);
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
extern int  xhci_bringup(void);
extern int  xhci_owned(void);
extern unsigned int xhci_portsc(int p);
extern int  xhci_scratchpads(void);
extern int  xhci_bar_high(void);
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
/* the input stack: events, modifiers, repeat */
/* ---- the compositor (wm.c / ui.c / wmglue.c) ---------------------------
 * Mechanism only. kernel.zl supplies the policy through the app_* functions
 * that wmglue.c binds to - see kernel/docs/desktop-wiring.md. */
/* ---- the terminal app (term.c) -----------------------------------------
 * A scrollback ring plus a typed-command matcher. The matcher is HERE, in C,
 * because comparing two runtime strings is the one thing the zl kernel subset
 * cannot do - it has string literals but no string values. */
extern void term_putc(char c);
extern int  term_key(int code);
extern int  term_cmd(void);
extern int  term_unknown(void);
extern int  term_arg(void);
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
extern int  wm_frame_us(void);
extern int  wm_peak_us(void);
extern void wm_peak_reset(void);
extern void wm_client(int win, int *x, int *y, int *w, int *h);
extern void wm_focus(int win);
extern void wm_raise(int win);
extern void wm_set_home(int win);
extern int  wm_count(void);
extern int  wm_zorder_at(int i);
extern int  wm_win_app(int win);
extern int  wm_add_tab(int win, int app, const char *title);
extern void wm_damage(int x, int y, int w, int h);
extern void wm_damage_win(int win);
extern void ui_theme_init(int scale);
extern void ui_begin(int x, int y, int w, int h, int mode, int px, int py, int click);
extern void ui_label(const char *s);
extern void ui_label_dim(const char *s);
extern void ui_bar(int pct);
extern int  ui_button(const char *s);
extern void ui_sep(void);
extern void ui_space(int n);
extern int  ui_toggle(const char *s, int *on);
extern int  ui_slider(int *v, int lo, int hi);
extern void ui_num(const char *s, int v);
extern int  ui_list_row(const char *s, int selected);
extern void ui_scroll_begin(int h, int *off);
extern void ui_scroll_end(int *off);
extern int  ui_scroll_content(void);
extern void ui_row(void);
extern void ui_endrow(void);
extern int  settings_load(void);
extern int  fs_try_boot(void);
extern void fs_seed_hello(void);

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
extern int  browser_click(int cx, int cy);
extern int  browser_tick(void);
extern int  browser_back(void);
extern int  browser_can_back(void);
extern int  browser_url_focus(void);
extern const char *browser_title(void);
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
    for (int i = 0; i < 200000; i++)
        if (zl_inb(COM1 + 5) & 0x20) break;
    zl_outb(COM1, (unsigned char)c);
}

/* ---- the system track: nvme.c, fs.c, rtc.c, clip.c, notify.c ------------ */
extern int  nvme_read_to(unsigned dst, unsigned lba_lo, unsigned lba_hi);
extern int  nvme_write_from(unsigned src, unsigned lba_lo, unsigned lba_hi);
extern int  nvme_ram_ok(void);
extern int  nvme_fault(void);

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
extern int  fs_create_named(unsigned bytes);
extern int  fs_find_named(void);
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

    /* raw memory - the whole point of a kernel runtime */
    if (streq(name, "peek8"))  return zl_num((double)*(volatile unsigned char  *)(unsigned long)a[0].num);
    if (streq(name, "peek16")) return zl_num((double)*(volatile unsigned short *)(unsigned long)a[0].num);
    if (streq(name, "peek32")) return zl_num((double)*(volatile unsigned int   *)(unsigned long)a[0].num);
    if (streq(name, "poke8"))  { *(volatile unsigned char  *)(unsigned long)a[0].num = (unsigned char )(unsigned long long)a[1].num; return zl_nil(); }
    if (streq(name, "poke16")) { *(volatile unsigned short *)(unsigned long)a[0].num = (unsigned short)(unsigned long long)a[1].num; return zl_nil(); }
    if (streq(name, "poke32")) { *(volatile unsigned int   *)(unsigned long)a[0].num = (unsigned int  )(unsigned long long)a[1].num; return zl_nil(); }

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
     * not. See console.c. */
    if (streq(name, "label"))      { if (a[2].type==V_STR) console_text_role((int)a[0].num,(int)a[1].num,a[2].str,(unsigned int)(unsigned long long)a[3].num,(int)a[4].num,(int)a[5].num); return zl_nil(); }
    if (streq(name, "label_w"))    { if (a[0].type==V_STR) return zl_num((double)console_text_role_w(a[0].str,(int)a[1].num,(int)a[2].num)); return zl_num(0.0); }
    if (streq(name, "label_h"))    return zl_num((double)console_text_role_h((int)a[0].num));
    if (streq(name, "label_num"))  { console_num_role((int)a[0].num,(int)a[1].num,(long)a[2].num,(unsigned int)(unsigned long long)a[3].num,(int)a[4].num,(int)a[5].num); return zl_nil(); }
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
    if (streq(name, "ui_scale"))   return zl_num((double)console_ui_scale());
    if (streq(name, "cell_h"))     return zl_num((double)console_cell_h());
    if (streq(name, "bits"))       return zl_num((double)(sizeof(void *) * 8));
    if (streq(name, "hex"))        { console_puthex((unsigned long)(long long)a[0].num, (int)a[1].num); return zl_nil(); }
    if (streq(name, "subpix"))     { fb_set_subpixel((int)a[0].num); return zl_nil(); }
    if (streq(name, "subpix_on"))  return zl_num((double)fb_get_subpixel());
    if (streq(name, "pci_scan"))   { pci_scan(); return zl_nil(); }
    if (streq(name, "pci_count"))  return zl_num((double)pci_count());
    if (streq(name, "pci_vendor")) return zl_num((double)pci_vendor((int)a[0].num));
    if (streq(name, "pci_device")) return zl_num((double)pci_device((int)a[0].num));
    if (streq(name, "pci_class"))  return zl_num((double)pci_class((int)a[0].num));
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
    if (streq(name, "usb_up"))     return zl_num((double)xhci_bringup());
    if (streq(name, "usb_ours"))   return zl_num((double)xhci_owned());
    if (streq(name, "usb_portsc")) return zl_num((double)xhci_portsc((int)a[0].num));
    if (streq(name, "usb_scratch"))return zl_num((double)xhci_scratchpads());
    if (streq(name, "usb_barhi"))  return zl_num((double)xhci_bar_high());
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
    if (streq(name, "cur_on"))     return zl_num((double)intel_cursor_enable((unsigned)a[0].num,(int)a[1].num));
    if (streq(name, "cur_move"))   return zl_num((double)intel_cursor_move((int)a[0].num,(int)a[1].num));
    if (streq(name, "cur_off"))    return zl_num((double)intel_cursor_disable());
    if (streq(name, "gpu_flip"))   return zl_num((double)intel_flip((unsigned)a[0].num));
    if (streq(name, "gpu_vbl"))    return zl_num((double)intel_wait_vblank());
    if (streq(name, "gpu_flips"))  return zl_num((double)intel_flip_count());
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
    /* THE FRAME TIMER. desktop-TODO 0h, and it should have come first: nothing
     * in this kernel had ever measured a frame, so every performance claim
     * about the compositor was arithmetic rather than measurement. */
    if (streq(name, "wm_us"))      return zl_num((double)wm_frame_us());
    if (streq(name, "wm_peak"))    return zl_num((double)wm_peak_us());
    if (streq(name, "wm_peak0"))   { wm_peak_reset(); return zl_nil(); }
    /* the client rect, so an app can turn a screen-space pointer into a row */
    if (streq(name, "wm_cx"))      { int x,y,w,h; wm_client((int)a[0].num,&x,&y,&w,&h); return zl_num((double)x); }
    if (streq(name, "wm_cy"))      { int x,y,w,h; wm_client((int)a[0].num,&x,&y,&w,&h); return zl_num((double)y); }
    if (streq(name, "wm_raise"))   { wm_raise((int)a[0].num); return zl_nil(); }
    if (streq(name, "wm_n"))       return zl_num((double)wm_count());
    /* the window list, for a taskbar: which window is i-th from the back, and
     * which app is in it. A taskbar cannot exist without these. */
    if (streq(name, "wm_zat"))     return zl_num((double)wm_zorder_at((int)a[0].num));
    if (streq(name, "wm_app"))     return zl_num((double)wm_win_app((int)a[0].num));
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
    if (streq(name, "wm_dmg"))     { wm_damage_win((int)a[0].num); return zl_nil(); }
    if (streq(name, "wm_damage"))  { wm_damage((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "ui_theme"))   { ui_theme_init((int)a[0].num); return zl_nil(); }
    /* ui.c widgets. zl has no pointers, so toggle/slider/scroll state lives
     * in a small slot table addressed by integer id. New UI uses these;
     * the existing 82 fill_rgb/label sites stay as they are. */
    {
        static int ui_slotv[16];
        #define UI_NSLOT 16
        #define UI_STR(i) ((a[i].type == V_STR && a[i].str) ? a[i].str : "")
        #define UI_SLOT(i) (((int)a[i].num >= 0 && (int)a[i].num < UI_NSLOT) \
                            ? (int)a[i].num : 0)
        if (streq(name, "ui_begin")) {
            ui_begin((int)a[0].num, (int)a[1].num, (int)a[2].num, (int)a[3].num,
                     (int)a[4].num, (int)a[5].num, (int)a[6].num, (int)a[7].num);
            return zl_nil();
        }
        if (streq(name, "ui_label"))     { ui_label(UI_STR(0)); return zl_nil(); }
        if (streq(name, "ui_label_dim")) { ui_label_dim(UI_STR(0)); return zl_nil(); }
        if (streq(name, "ui_bar"))       { ui_bar((int)a[0].num); return zl_nil(); }
        if (streq(name, "ui_button"))    return zl_num((double)ui_button(UI_STR(0)));
        if (streq(name, "ui_sep"))       { ui_sep(); return zl_nil(); }
        if (streq(name, "ui_space"))     { ui_space((int)a[0].num); return zl_nil(); }
        if (streq(name, "ui_toggle"))    return zl_num((double)ui_toggle(UI_STR(0), &ui_slotv[UI_SLOT(1)]));
        if (streq(name, "ui_slider"))    return zl_num((double)ui_slider(&ui_slotv[UI_SLOT(0)], (int)a[1].num, (int)a[2].num));
        if (streq(name, "ui_num"))       { ui_num(UI_STR(0), (int)a[1].num); return zl_nil(); }
        if (streq(name, "ui_list_row"))  return zl_num((double)ui_list_row(UI_STR(0), (int)a[1].num));
        if (streq(name, "ui_scroll_begin")) { ui_scroll_begin((int)a[0].num, &ui_slotv[UI_SLOT(1)]); return zl_nil(); }
        if (streq(name, "ui_scroll_end"))   { ui_scroll_end(&ui_slotv[UI_SLOT(0)]); return zl_nil(); }
        if (streq(name, "ui_scroll_content")) return zl_num((double)ui_scroll_content());
        if (streq(name, "ui_row"))       { ui_row(); return zl_nil(); }
        if (streq(name, "ui_endrow"))    { ui_endrow(); return zl_nil(); }
        if (streq(name, "ui_slot"))      return zl_num((double)ui_slotv[UI_SLOT(0)]);
        if (streq(name, "ui_set"))       { ui_slotv[UI_SLOT(0)] = (int)a[1].num; return zl_nil(); }
        #undef UI_NSLOT
        #undef UI_STR
        #undef UI_SLOT
    }
    if (streq(name, "settings_load")) return zl_num((double)settings_load());
    if (streq(name, "fs_try"))        return zl_num((double)fs_try_boot());
    if (streq(name, "fs_seed"))       { fs_seed_hello(); return zl_nil(); }
    /* ---- the browser. Everything below is one app's policy surface. */
    /* ---- virtio-net. net_up() is the one that does the work; everything
     * else reports what happened, because a driver that fails silently is
     * indistinguishable from one that is not there. */
    if (streq(name, "net_find"))   return zl_num((double)virtio_net_find());
    /* ip_up(ip, mask, gw): bring the card up, then hand net.c the link. Two
     * steps rather than one because a card that works and a stack that is
     * misconfigured are different failures and should report separately. */
    if (streq(name, "ip_up")) {
        if (!virtio_net_init()) return zl_num(0.0);
        unsigned char m[6];
        for (int k = 0; k < 6; k++) m[k] = (unsigned char)virtio_net_mac(k);
        net_link(virtio_net_send, virtio_net_poll, m);
        net_config((unsigned)a[0].num, (unsigned)a[1].num, (unsigned)a[2].num);
        return zl_num(1.0);
    }
    if (streq(name, "ip_live"))    return zl_num((double)net_live());
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
    if (streq(name, "net_up"))     return zl_num((double)virtio_net_init());
    if (streq(name, "net_there"))  return zl_num((double)virtio_net_present());
    if (streq(name, "net_ok"))     return zl_num((double)virtio_net_ready());
    if (streq(name, "net_ram"))    return zl_num((double)virtio_net_ram_ok());
    if (streq(name, "net_hasmac")) return zl_num((double)virtio_net_has_mac());
    if (streq(name, "net_mac"))    return zl_num((double)virtio_net_mac((int)a[0].num));
    if (streq(name, "net_link"))   return zl_num((double)virtio_net_link_up());
    if (streq(name, "net_tx"))     return zl_num((double)virtio_net_tx_count());
    if (streq(name, "net_rx"))     return zl_num((double)virtio_net_rx_count());
    if (streq(name, "net_drop"))   return zl_num((double)virtio_net_rx_drops());
    if (streq(name, "net_txfull")) return zl_num((double)virtio_net_tx_full());
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
    if (streq(name, "br_click"))   return zl_num((double)browser_click((int)a[0].num,(int)a[1].num));
    if (streq(name, "br_tick"))    return zl_num((double)browser_tick());
    if (streq(name, "br_back"))    return zl_num((double)browser_back());
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
    if (streq(name, "fs_new"))     return zl_num((double)fs_create_named((unsigned)a[0].num));
    if (streq(name, "fs_get"))     return zl_num((double)fs_find_named());
    if (streq(name, "fs_rm"))      return zl_num((double)fs_delete((int)a[0].num));
    if (streq(name, "fs_rd"))      return zl_num((double)fs_read((int)a[0].num,(void *)(unsigned long)a[1].num,(unsigned)a[2].num));
    if (streq(name, "fs_wr"))      return zl_num((double)fs_write((int)a[0].num,(const void *)(unsigned long)a[1].num,(unsigned)a[2].num));
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
        unsigned long long v = *(volatile unsigned long long *)(unsigned long)a[0].num;
        if (v > 9007199254740992ULL) kfatal("peek64 above 2^53 - read two peek32 halves");
        return zl_num((double)v);
    }
    if (streq(name, "poke64")) {
        unsigned long long v = (unsigned long long)a[1].num;
        if (v > 9007199254740992ULL) kfatal("poke64 above 2^53 - write two poke32 halves");
        *(volatile unsigned long long *)(unsigned long)a[0].num = v;
        return zl_nil();
    }
    if (streq(name, "fill_mem")) {
        volatile unsigned char *p = (volatile unsigned char *)(unsigned long)a[0].num;
        unsigned long long cnt = (unsigned long long)a[2].num;
        unsigned char b = (unsigned char)(unsigned long long)a[1].num;
        while (cnt--) *p++ = b;
        return zl_nil();
    }
    if (streq(name, "copy_mem")) {
        volatile unsigned char *d = (volatile unsigned char *)(unsigned long)a[0].num;
        volatile const unsigned char *s = (volatile const unsigned char *)(unsigned long)a[1].num;
        unsigned long long cnt = (unsigned long long)a[2].num;
        if (d < s) { while (cnt--) *d++ = *s++; }
        else       { d += cnt; s += cnt; while (cnt--) *--d = *--s; }
        return zl_nil();
    }

    kfatal("builtin not available in the kernel subset");
    return zl_nil();
}
