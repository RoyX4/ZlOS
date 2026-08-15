/* idt.c - interrupts: the Interrupt Descriptor Table, the PIC, and the two
 * hardware IRQs a usable console needs - the timer and the keyboard.
 *
 * Until now the kernel POLLED: it sat in a loop asking the keyboard port
 * "anything yet?" forever. That is why input was flaky on real hardware -
 * a polled read can miss bytes the controller expected to be taken quickly.
 * Interrupts flip it around: the hardware TELLS the CPU when something
 * happened, the CPU jumps to a handler, the handler grabs the byte, done.
 *
 * design_kernel.md §6: serial before interrupts (so you can debug them), a
 * gate per vector, the PIC remapped off the CPU exception range, and IRQs
 * masked except the ones you handle. v1 handles IRQ0 (timer) and IRQ1
 * (keyboard); everything else is masked.
 *
 * The ISRs use gcc's `interrupt` attribute, which emits the register
 * save/restore and the `iret` for us - so no hand-written asm stubs. The
 * file is compiled with -mgeneral-regs-only so a handler never touches SSE.
 */
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

void          zl_outb(u16 port, u8 val);
unsigned char zl_inb(u16 port);

/* ---- IDT ------------------------------------------------------------- */
#ifdef ZL_64
/* A long-mode gate is 16 bytes: the handler address is split across three
 * fields, and there is an IST index for a dedicated interrupt stack. The
 * 32-bit layout below simply does not fit a 64-bit address. */
struct idt_entry { u16 lo; u16 sel; u8 ist; u8 flags; u16 mid; u32 hi; u32 zero; } __attribute__((packed));
struct idt_ptr   { u16 limit; unsigned long base; } __attribute__((packed));
#else
struct idt_entry { u16 lo; u16 sel; u8 zero; u8 flags; u16 hi; } __attribute__((packed));
struct idt_ptr   { u16 limit; u32 base; } __attribute__((packed));
#endif

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

static void set_gate(int n, void *handler)
{
#ifdef ZL_64
    unsigned long a = (unsigned long)handler;
    idt[n].lo    = a & 0xFFFF;
    idt[n].mid   = (a >> 16) & 0xFFFF;
    idt[n].hi    = (u32)(a >> 32);
    idt[n].sel   = 0x08;        /* the 64-bit code selector from boot64.S */
    idt[n].ist   = 0;           /* no separate interrupt stack for now     */
    idt[n].flags = 0x8E;        /* present, ring 0, 64-bit interrupt gate  */
    idt[n].zero  = 0;
#else
    u32 a = (u32)handler;
    idt[n].lo    = a & 0xFFFF;
    idt[n].hi    = (a >> 16) & 0xFFFF;
    idt[n].sel   = 0x08;        /* our GDT code selector */
    idt[n].zero  = 0;
    idt[n].flags = 0x8E;        /* present, ring 0, 32-bit interrupt gate */
#endif
}

/* ---- the state the ISRs publish, read by zl ------------------------- */
static volatile u32 tick_count = 0;

#define KBUF_SIZE 256
static volatile u8  kbuf[KBUF_SIZE];
static volatile int kbuf_head = 0;   /* ISR writes here */
static volatile int kbuf_tail = 0;   /* zl reads here   */

u32 idt_ticks(void) { return tick_count; }

/* pull one scancode from the ring, or 0 if empty (0 is never a real code) */
int idt_scan(void)
{
    if (kbuf_tail == kbuf_head) return 0;
    u8 c = kbuf[kbuf_tail];
    kbuf_tail = (kbuf_tail + 1) & (KBUF_SIZE - 1);
    return c;
}

/* End of interrupt.
 *
 * Which chip gets acknowledged depends on which one actually delivered the
 * interrupt. Once the I/O APIC is routing, the PIC is masked and completely
 * out of the path - sending it an EOI is harmless but pointless, and MISSING
 * the local APIC's EOI is fatal: that core never accepts another interrupt of
 * equal or lower priority again, so the machine goes quiet after exactly one
 * keypress. */
extern int  apic_active(void);
extern void apic_eoi(void);

static void irq_done(int irq)
{
    if (apic_active()) { apic_eoi(); return; }
    if (irq >= 8) zl_outb(0xA0, 0x20);      /* slave first, then the cascade */
    zl_outb(0x20, 0x20);
}

/* ---- PS/2 mouse state, published by the IRQ12 handler --------------- */
static volatile int mouse_x = 400, mouse_y = 300, mouse_btn = 0;
static volatile u8  mpkt[3];
static volatile int mphase = 0;

int idt_mouse_x(void)   { return mouse_x; }
int idt_mouse_y(void)   { return mouse_y; }
int idt_mouse_btn(void) { return mouse_btn; }

/* ---- the handlers ---------------------------------------------------- */
#ifdef ZL_64
struct interrupt_frame { unsigned long ip, cs, flags, sp, ss; };
#else
struct interrupt_frame { u32 ip, cs, flags, sp, ss; };
#endif

/* IRQ0: the PIT ticks ~100 times a second. Just count. */
__attribute__((interrupt))
static void timer_isr(struct interrupt_frame *f)
{
    (void)f;
    tick_count++;
    irq_done(0);
}

/* IRQ1: a key changed. Grab the scancode before the controller moves on
 * and drop it in the ring for zl to translate at its leisure. */
__attribute__((interrupt))
static void keyboard_isr(struct interrupt_frame *f)
{
    (void)f;
    u8 sc = zl_inb(0x60);
    int next = (kbuf_head + 1) & (KBUF_SIZE - 1);
    if (next != kbuf_tail) {     /* drop it rather than overwrite unread input */
        kbuf[kbuf_head] = sc;
        kbuf_head = next;
    }
    irq_done(1);
}

/* IRQ12: the PS/2 mouse. Each move or click sends a 3-byte packet -
 * [flags, dx, dy] - and this reassembles them, updates the tracked position
 * (mouse Y is inverted, hence the minus), and clamps to a generous range;
 * zl clamps the rest to the real screen. This, drawn as a cursor, is the
 * TempleOS-style mouse-driven UI. */
__attribute__((interrupt))
static void mouse_isr(struct interrupt_frame *f)
{
    (void)f;
    u8 status = zl_inb(0x64);
    if (status & 0x20) {                       /* bit 5: byte is from the mouse */
        u8 b = zl_inb(0x60);
        if (mphase == 0 && !(b & 0x08)) {
            /* byte 0 always has bit 3 set; if not, we are out of sync - drop */
        } else {
            mpkt[mphase++] = b;
            if (mphase == 3) {
                mphase = 0;
                u8 flags = mpkt[0];
                if (!(flags & 0xC0)) {         /* ignore overflowed packets */
                    int dx = mpkt[1], dy = mpkt[2];
                    if (flags & 0x10) dx |= 0xFFFFFF00;   /* sign-extend */
                    if (flags & 0x20) dy |= 0xFFFFFF00;
                    mouse_x += dx;
                    mouse_y -= dy;
                    if (mouse_x < 0) mouse_x = 0;
                    if (mouse_y < 0) mouse_y = 0;
                    if (mouse_x > 2000) mouse_x = 2000;
                    if (mouse_y > 1500) mouse_y = 1500;
                    mouse_btn = flags & 0x07;
                }
            }
        }
    }
    irq_done(12);
}

/* a catch-all for hardware IRQs we do not handle - acknowledge and move on */
__attribute__((interrupt))
static void ignore_isr(struct interrupt_frame *f)
{
    (void)f;
    irq_done(8);                 /* acknowledge whichever controller is live */
}

/* a CPU exception (divide error, page fault, GP...) must NOT iret - that just
 * re-runs the faulting instruction and faults again forever, ending in a
 * triple fault and a reboot. Stop the machine instead, which at least leaves
 * the screen readable. (A real dump comes with design_kernel.md §6.2 later.) */
__attribute__((interrupt))
static void fault_isr(struct interrupt_frame *f)
{
    (void)f;
    __asm__ volatile("cli");
    for (;;) __asm__ volatile("hlt");
}

/* ---- PIC: move the 16 IRQs off the CPU exception vectors ------------- */
static void pic_remap(void)
{
    /* By default IRQ0-7 arrive as vectors 8-15, which collide with the CPU's
     * own exceptions (double fault is 8...). Remap the master to 0x20-0x27
     * and the slave to 0x28-0x2F. Standard 8259 init sequence. */
    u8 m1 = zl_inb(0x21), m2 = zl_inb(0xA1);   /* save masks */
    zl_outb(0x20, 0x11); zl_outb(0xA0, 0x11);  /* start init, cascade mode */
    zl_outb(0x21, 0x20); zl_outb(0xA1, 0x28);  /* vector offsets */
    zl_outb(0x21, 0x04); zl_outb(0xA1, 0x02);  /* master/slave wiring (IRQ2) */
    zl_outb(0x21, 0x01); zl_outb(0xA1, 0x01);  /* 8086 mode */
    zl_outb(0x21, m1);   zl_outb(0xA1, m2);    /* restore masks */
}

/* ---- the PIT: program channel 0 for ~100 Hz ------------------------- */
static void pit_init(void)
{
    u32 divisor = 1193182 / 100;               /* 100 ticks per second */
    zl_outb(0x43, 0x36);                        /* channel 0, lo/hi, mode 3 */
    zl_outb(0x40, (u8)(divisor & 0xFF));
    zl_outb(0x40, (u8)((divisor >> 8) & 0xFF));
}

/* ---- enable the PS/2 mouse in the 8042 controller ------------------- */
static void ps2_wait_in(void)  { int t = 100000; while (t-- && (zl_inb(0x64) & 2)); }
static void ps2_wait_out(void) { int t = 100000; while (t-- && !(zl_inb(0x64) & 1)); }

static void mouse_cmd(u8 cmd)
{
    ps2_wait_in();  zl_outb(0x64, 0xD4);    /* next byte goes to the mouse */
    ps2_wait_in();  zl_outb(0x60, cmd);
    ps2_wait_out(); zl_inb(0x60);           /* read+discard the ACK */
}

/* ---- bring the 8042 controller up properly ------------------------------
 * This is the bug that made zlOS look dead on real hardware while working
 * perfectly in QEMU: we never initialised the keyboard controller at all. We
 * unmasked IRQ1 on the PIC and started reading port 0x60, which is fine in an
 * emulator because QEMU hands the 8042 over already enabled and scanning.
 *
 * Real firmware does not. UEFI drives the keyboard through its own drivers and
 * hands control over with the keyboard's INTERRUPT DISABLED in the controller
 * config byte, and often with scanning switched off entirely. The PIC then
 * dutifully delivers an interrupt that the controller never raises, so every
 * key press vanishes and the machine looks frozen.
 *
 * So: flush whatever is stale, take explicit control of the config byte, and
 * tell the keyboard to actually start scanning. */
static void ps2_flush(void)
{
    int t = 1000;
    while (t-- && (zl_inb(0x64) & 1)) zl_inb(0x60);
}

static void kbd_ctrl_init(void)
{
    /* quiet both ports while we reconfigure */
    ps2_wait_in(); zl_outb(0x64, 0xAD);     /* disable the keyboard port  */
    ps2_wait_in(); zl_outb(0x64, 0xA7);     /* disable the mouse port     */
    ps2_flush();                            /* drop anything left over    */

    /* Take the config byte and set it to what WE need, rather than trusting
     * whatever state the firmware left behind. */
    ps2_wait_in();  zl_outb(0x64, 0x20);    /* read config byte */
    ps2_wait_out(); u8 cfg = zl_inb(0x60);
    cfg |=  0x01;      /* bit 0: keyboard interrupt (IRQ1) ON - the fix      */
    cfg |=  0x02;      /* bit 1: mouse interrupt (IRQ12) ON                  */
    cfg &= ~0x10;      /* bit 4: clear = keyboard clock ENABLED              */
    cfg &= ~0x20;      /* bit 5: clear = mouse clock ENABLED                 */
    cfg |=  0x40;      /* bit 6: translate to scancode set 1, which our map
                          expects - UEFI often leaves the keyboard in set 2  */
    ps2_wait_in(); zl_outb(0x64, 0x60);     /* write config byte back */
    ps2_wait_in(); zl_outb(0x60, cfg);

    /* ports back on */
    ps2_wait_in(); zl_outb(0x64, 0xAE);     /* enable the keyboard port */
    ps2_wait_in(); zl_outb(0x64, 0xA8);     /* enable the mouse port    */

    /* and finally tell the KEYBOARD ITSELF to scan. Without this the
     * controller is listening to a device that is not talking. */
    ps2_wait_in();  zl_outb(0x60, 0xF4);    /* enable scanning */
    ps2_wait_out(); zl_inb(0x60);           /* eat the ACK */
    ps2_flush();
}

static void mouse_init(void)
{
    kbd_ctrl_init();                        /* the controller, then the mouse */

    mouse_cmd(0xF6);                        /* set defaults */
    mouse_cmd(0xF4);                        /* enable data reporting */
    ps2_flush();
}

void idt_init(void)
{
    for (int i = 0;  i < 32;  i++) set_gate(i, fault_isr);    /* CPU exceptions: halt */
    for (int i = 32; i < 256; i++) set_gate(i, ignore_isr);   /* stray IRQs: ack     */
    set_gate(0x20, timer_isr);      /* IRQ0  timer            */
    set_gate(0x21, keyboard_isr);   /* IRQ1  keyboard         */
    set_gate(0x2C, mouse_isr);      /* IRQ12 mouse (on slave) */

    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (unsigned long)&idt;
    __asm__ volatile("lidt %0" :: "m"(idtp));

    pic_remap();
    pit_init();
    mouse_init();

    /* Master: unmask IRQ0 (timer), IRQ1 (keyboard) and IRQ2 (the cascade to
     * the slave PIC, without which IRQ12 never arrives).  0xF8 = 1111 1000.
     * Slave: unmask IRQ12 only.  0xEF = 1110 1111. */
    zl_outb(0x21, 0xF8);
    zl_outb(0xA1, 0xEF);

    __asm__ volatile("sti");        /* interrupts on */
}
