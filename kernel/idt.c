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
struct idt_entry { u16 lo; u16 sel; u8 zero; u8 flags; u16 hi; } __attribute__((packed));
struct idt_ptr   { u16 limit; u32 base; } __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

static void set_gate(int n, void *handler)
{
    u32 a = (u32)handler;
    idt[n].lo    = a & 0xFFFF;
    idt[n].hi    = (a >> 16) & 0xFFFF;
    idt[n].sel   = 0x08;        /* our GDT code selector */
    idt[n].zero  = 0;
    idt[n].flags = 0x8E;        /* present, ring 0, 32-bit interrupt gate */
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

/* ---- the handlers ---------------------------------------------------- */
struct interrupt_frame { u32 ip, cs, flags, sp, ss; };

/* IRQ0: the PIT ticks ~100 times a second. Just count. */
__attribute__((interrupt))
static void timer_isr(struct interrupt_frame *f)
{
    (void)f;
    tick_count++;
    zl_outb(0x20, 0x20);        /* EOI to the master PIC */
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
    zl_outb(0x20, 0x20);
}

/* a catch-all for hardware IRQs we do not handle - acknowledge and move on */
__attribute__((interrupt))
static void ignore_isr(struct interrupt_frame *f)
{
    (void)f;
    zl_outb(0x20, 0x20);
    zl_outb(0xA0, 0x20);         /* EOI to both PICs, in case it was IRQ8-15 */
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

void idt_init(void)
{
    for (int i = 0;  i < 32;  i++) set_gate(i, fault_isr);    /* CPU exceptions: halt */
    for (int i = 32; i < 256; i++) set_gate(i, ignore_isr);   /* stray IRQs: ack     */
    set_gate(0x20, timer_isr);      /* IRQ0 after remap */
    set_gate(0x21, keyboard_isr);   /* IRQ1 after remap */

    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (u32)&idt;
    __asm__ volatile("lidt %0" :: "m"(idtp));

    pic_remap();
    pit_init();

    /* unmask only IRQ0 (timer) and IRQ1 (keyboard) on the master; mask the
     * rest and the whole slave - nothing else is handled yet. */
    zl_outb(0x21, 0xFC);            /* 1111 1100 - bits 0,1 clear = enabled */
    zl_outb(0xA1, 0xFF);

    __asm__ volatile("sti");        /* interrupts on - the CPU will now be told */
}
