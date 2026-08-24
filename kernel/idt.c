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
/* Explicitly 64 bits in every build. `unsigned long` is NOT: it is 4 bytes on
 * the EFI build's clang target (x86_64-unknown-windows, LLP64) and 8 with gcc.
 * Anything holding an address must use this, never `long`. */
typedef unsigned long long u64;
#ifdef ZL_64
typedef u64 uptr;
#else
typedef u32 uptr;
#endif

void          zl_outb(u16 port, u8 val);
unsigned char zl_inb(u16 port);
int  crash_capture(u32 vector, u32 has_error, u64 error_code,
                   u64 ip, u64 cs, u64 flags, u64 sp, u64 ss,
                   u64 cr2, u32 word_bits);
void crash_report(void);

/* The live screen size, so the pointer can be clamped to pixels that exist.
 * CACHED here rather than fetched by calling console_pxw() from the handler.
 * This file is built -mgeneral-regs-only so an interrupt never touches SSE;
 * console.c is not, so calling into it from an ISR can clobber XMM registers
 * the handler never saved. Every zl number is a double, so the interpreter
 * lives in those registers - on 64-bit that corrupted it mid-boot and the
 * kernel died inside setup_idt(). The console pushes its size in instead. */
static volatile int ptr_lim_x = 2000;   /* until a framebuffer says otherwise */
static volatile int ptr_lim_y = 1500;

void idt_set_pointer_bounds(int w, int h)
{
    if (w > 0) ptr_lim_x = w;
    if (h > 0) ptr_lim_y = h;
}

/* ---- IDT ------------------------------------------------------------- */
#ifdef ZL_64
/* A long-mode gate is 16 bytes: the handler address is split across three
 * fields, and there is an IST index for a dedicated interrupt stack. The
 * 32-bit layout below simply does not fit a 64-bit address. */
struct idt_entry { u16 lo; u16 sel; u8 ist; u8 flags; u16 mid; u32 hi; u32 zero; } __attribute__((packed));
/* The LIDT operand is 2 bytes of limit and EIGHT of base. `unsigned long` is
 * 8 bytes with gcc (LP64) but only 4 with the EFI build's clang target
 * (x86_64-unknown-windows, LLP64), which made this struct 6 bytes there - so
 * lidt read a 10-byte operand from a 6-byte object and took the TOP HALF of
 * the IDT base from whatever happened to follow it in memory. It worked only
 * while those bytes were zero, which made it exquisitely sensitive to code
 * layout: an unrelated edit could move things and the 64-bit boot would die at
 * the lidt with no diagnostic at all. Use an explicitly-sized type. */
struct idt_ptr   { u16 limit; unsigned long long base; } __attribute__((packed));
_Static_assert(sizeof(struct idt_ptr) == 10, "LIDT operand must be 10 bytes");
#else
struct idt_entry { u16 lo; u16 sel; u8 zero; u8 flags; u16 hi; } __attribute__((packed));
struct idt_ptr   { u16 limit; u32 base; } __attribute__((packed));
#endif

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

/* Same as set_gate but with DPL 3, for the ONE vector ring 3 is allowed to
 * invoke. The two-bit difference (0x8E -> 0xEE) is the whole reason a user
 * program can make a syscall and cannot fake a page fault: the CPU refuses an
 * `int n` from ring 3 when gate n's DPL is lower than the caller's CPL, so
 * every other vector in this table is unreachable from user mode.
 *
 * 32-bit only. usermode.c explains why ring 3 is not on the 64-bit builds. */
#ifndef ZL_64
static void set_gate_user(int n, void *handler)
{
    u32 a = (u32)handler;
    idt[n].lo    = a & 0xFFFF;
    idt[n].hi    = (a >> 16) & 0xFFFF;
    idt[n].sel   = 0x08;
    idt[n].zero  = 0;
    idt[n].flags = 0xEE;    /* present, DPL 3, 32-bit interrupt gate */
}
#endif

static void set_gate(int n, void *handler)
{
#ifdef ZL_64
    /* MUST be 64 bits. As `unsigned long` this truncated the handler address to
     * 32 bits in the EFI build, and `a >> 32` then shifted a 32-bit value by 32
     * - undefined behaviour, which clang compiled to a bare `ret`, so the gate's
     * top 32 bits came from whatever was left in eax. Every vector was one
     * register's worth of luck away from pointing into nowhere. */
    u64 a = (u64)handler;
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
/* FOUR, not three. The wheel packet is 4 bytes, and mphase indexes this
 * directly - at u8[3] a wheel mouse wrote one byte past the end of it on every
 * single packet, into whatever the linker put next. Caught by -Warray-bounds,
 * which is the only reason it is not a corruption bug that shows up somewhere
 * else entirely. */
static volatile u8  mpkt[4];
static volatile int mphase = 0;
static volatile int mouse_irqs = 0;

int idt_mouse_x(void)   { return mouse_x; }
int idt_mouse_y(void)   { return mouse_y; }
int idt_mouse_btn(void) { return mouse_btn; }

/* ---- the scroll wheel ------------------------------------------------------
 * The PS/2 protocol has one and this driver never asked for it: a plain mouse
 * sends 3-byte packets, and only after the "IntelliMouse knock" does it send 4
 * with the wheel in the last one. mouse_init does the knock; mouse_pktlen says
 * whether it worked. It is NOT assumed - a device that declines stays at 3.
 *
 * A WHEEL IS A DELTA, NOT A POSITION. There is no "where the wheel is", so
 * this accumulates notches and hands them over on read, clearing as it goes.
 * Publishing a position would mean a poll that misses a notch loses it, and
 * two notches between polls would read as one. */
static volatile int mouse_wz = 0;
static int mouse_pktlen = 3;

int idt_mouse_wheel(void)
{
    int v = mouse_wz;
    mouse_wz = 0;               /* read-and-clear: notches accumulate, never
                                   pile up as a position nobody resets */
    return v;
}
int idt_mouse_haswheel(void) { return mouse_pktlen == 4; }

/* How many times IRQ12 has actually fired. The pointer being dead has two
 * very different causes and they need telling apart: zero here means no
 * interrupt is arriving at all (controller not enabled, or the line is not
 * routed), non-zero with a stuck position means the packets are arriving and
 * the decode is wrong. */
/* mouse_irqs itself is declared with the rest of the PS/2 state above; this
 * merge produced a second one, `unsigned` against the original `int`. */
unsigned idt_mouse_irqs(void) { return (unsigned)mouse_irqs; }

/* One byte of a mouse packet, wherever it was noticed.
 *
 * This has to be callable from BOTH interrupt handlers, and that is the whole
 * point. The keyboard and the mouse are one 8042 controller sharing ONE output
 * buffer at port 0x60; status bit 5 at 0x64 is the only thing that says which
 * device the pending byte belongs to. IRQ1 used to read 0x60 unconditionally,
 * so a key pressed while the mouse was moving swallowed a byte out of the
 * middle of a 3-byte packet. The stream then read dx/dy from the wrong offsets
 * and the pointer moved erratically - not dead, just wrong - until the framing
 * happened to realign on the bit-3 check below. Intermittent by construction:
 * it depends on the timing between a keystroke and a mouse packet. */
static void mouse_byte(u8 b)
{
    /* byte 0 of a packet always has bit 3 set; anything else means we are
     * mid-stream and out of sync, so drop until a real header shows up */
    if (mphase == 0 && !(b & 0x08)) return;

    mpkt[mphase++] = b;
    /* 3 without the IntelliMouse knock, 4 with it. This MUST track
     * mouse_pktlen rather than the literal 3: mouse_init does the knock,
     * and a reader still framing on 3 bytes against a device now sending 4
     * desyncs permanently and the pointer moves at random. */
    if (mphase < mouse_pktlen) return;
    mphase = 0;

    u8 flags = mpkt[0];
    if (flags & 0xC0) return;                  /* overflowed - ignore */

    int dx = mpkt[1], dy = mpkt[2];
    if (flags & 0x10) dx |= 0xFFFFFF00;        /* sign-extend */
    if (flags & 0x20) dy |= 0xFFFFFF00;
    mouse_x += dx;
    mouse_y -= dy;                             /* mouse Y is inverted */

    /* Clamp to the SCREEN, not to a guessed 2000x1500 - those constants sit
     * 80 px right of and 300 px below a 1920x1200 panel, so the pointer could
     * wander somewhere with no pixels and need 300 px of travel to come back.
     * console_pxw/pxh are 0 on the VGA text console, which has no pointer. */
    int lim_x = ptr_lim_x, lim_y = ptr_lim_y;
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x > lim_x - 1) mouse_x = lim_x - 1;
    if (mouse_y > lim_y - 1) mouse_y = lim_y - 1;
    mouse_btn = flags & 0x07;

    /* THE WHEEL FIELD IS 4-BIT SIGNED, not a byte. Bits 3:0 are the notch
     * count in two's complement; 4..7 are buttons 4 and 5 on the mice that
     * have them. Reading the whole byte makes one notch backwards (0x0F)
     * look like fifteen forwards - and it only shows when you scroll UP. */
    if (mouse_pktlen == 4) {
        int wz = mpkt[3] & 0x0F;
        if (wz & 0x08) wz -= 16;           /* sign-extend from 4 bits */
        mouse_wz += wz;
    }
}
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
    /* Read the status BEFORE the data: bit 5 says this byte came from the
     * mouse, not the keyboard. Taking it regardless is what desynced the
     * pointer whenever a key overlapped a mouse packet. */
    u8 status = zl_inb(0x64);
    u8 b = zl_inb(0x60);
    if (status & 0x20) {                 /* the mouse's byte, on our line */
        mouse_irqs++;
        mouse_byte(b);
        irq_done(1);
        return;
    }
    int next = (kbuf_head + 1) & (KBUF_SIZE - 1);
    if (next != kbuf_tail) {     /* drop it rather than overwrite unread input */
        kbuf[kbuf_head] = b;
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
    /* Drain every mouse byte the controller has, not just one. A single
     * interrupt can cover more than one byte when packets arrive faster than
     * we are scheduled, and leaving them queued lets the 16-byte buffer
     * overflow - which loses a byte and desyncs the packet framing exactly
     * like the IRQ1 theft did. Bounded so a stuck controller cannot wedge us. */
    for (int i = 0; i < 16; i++) {
        u8 status = zl_inb(0x64);
        if (!(status & 0x01)) break;           /* output buffer empty */
        if (!(status & 0x20)) break;           /* keyboard's byte - leave it */
        mouse_irqs++;
        mouse_byte(zl_inb(0x60));
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

/* A CPU exception must not iret to the faulting instruction. The old handler
 * stopped correctly but threw the frame away and used one no-error signature
 * for all 32 vectors, including the ten whose CPU frame starts with an error
 * code. The compiler needs the right signature so it can find IP/CS/FLAGS and
 * discard the architectural error slot correctly.
 *
 * General registers are intentionally NOT claimed here. GCC/clang have already
 * made an interrupt prologue before C sees the frame; truthful GPR capture needs
 * assembly entry stubs. What is exact now is the architectural control frame,
 * vector, error code and CR2 for #PF. crash.c commits a bounded checksum record
 * and emits one parseable serial line before this path halts. */
__attribute__((noreturn, noinline))
static void fault_stop(u32 vector, u32 has_error, uptr error,
                       struct interrupt_frame *f)
{
    uptr sp;
    uptr ss = 0;
    uptr cr2 = 0;

    __asm__ volatile("cli");
    if ((f->cs & 3u) != 0) {
        /* A privilege transition pushes the interrupted SP and SS. */
        sp = (uptr)f->sp;
        ss = (uptr)f->ss;
    } else {
        /* Same-ring exceptions push only IP, CS and FLAGS. The interrupted SP
         * is therefore the address immediately above those three words; f->sp
         * would read unrelated pre-fault stack contents. */
        sp = (uptr)f + 3u * (uptr)sizeof(uptr);
    }
    if (vector == 14u) __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

    (void)crash_capture(vector, has_error, (u64)error,
                        (u64)f->ip, (u64)f->cs, (u64)f->flags,
                        (u64)sp, (u64)ss, (u64)cr2,
                        (u32)(sizeof(uptr) * 8u));
    crash_report();
    for (;;) __asm__ volatile("hlt");
}

#define FAULT_NOERR(n) \
    __attribute__((interrupt)) static void fault_##n(struct interrupt_frame *f) \
    { fault_stop((n), 0, 0, f); }
#define FAULT_ERR(n) \
    __attribute__((interrupt)) static void fault_##n(struct interrupt_frame *f, uptr error) \
    { fault_stop((n), 1, error, f); }

FAULT_NOERR(0)   FAULT_NOERR(1)   FAULT_NOERR(2)   FAULT_NOERR(3)
FAULT_NOERR(4)   FAULT_NOERR(5)   FAULT_NOERR(6)   FAULT_NOERR(7)
FAULT_ERR(8)     FAULT_NOERR(9)   FAULT_ERR(10)    FAULT_ERR(11)
FAULT_ERR(12)    FAULT_ERR(13)    FAULT_ERR(14)    FAULT_NOERR(15)
FAULT_NOERR(16)  FAULT_ERR(17)    FAULT_NOERR(18)  FAULT_NOERR(19)
FAULT_NOERR(20)  FAULT_ERR(21)    FAULT_NOERR(22)  FAULT_NOERR(23)
FAULT_NOERR(24)  FAULT_NOERR(25)  FAULT_NOERR(26)  FAULT_NOERR(27)
FAULT_NOERR(28)  FAULT_ERR(29)    FAULT_ERR(30)    FAULT_NOERR(31)

/* Deliberate invalid-opcode trigger for the dedicated QEMU crash receipt.
 * It is reachable only through the explicit `crashtest` diagnostic command. */
void crash_test_ud2(void) { __asm__ volatile("ud2"); }

#ifndef ZL_64
/* usermode.c, in assembly. Declared as a function taking no arguments purely so
 * its address can be taken - it is never called from C and never returns
 * normally; it iret's, or it abandons ring 3 entirely on SYS_EXIT. */
void syscall_isr(void);
#endif

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

/* the same, but with an argument byte - 0xF3 (set sample rate) needs one */
static void mouse_cmd_arg(u8 cmd, u8 arg)
{
    mouse_cmd(cmd);
    ps2_wait_in();  zl_outb(0x64, 0xD4);
    ps2_wait_in();  zl_outb(0x60, arg);
    ps2_wait_out(); zl_inb(0x60);           /* ACK */
}

/* ...and one that returns the reply, for 0xF2 (get device id) */
static u8 mouse_cmd_reply(u8 cmd)
{
    mouse_cmd(cmd);
    ps2_wait_out();
    return zl_inb(0x60);
}

/* THE INTELLIMOUSE KNOCK. Setting the sample rate to 200, then 100, then 80 is
 * a magic sequence, not a configuration: a wheel mouse recognises it and
 * switches to 4-byte packets, reporting device id 3 afterwards. A plain mouse
 * ignores it, keeps reporting id 0, and stays at 3 bytes.
 *
 * So the id is CHECKED rather than assumed. Guessing wrong is not cosmetic -
 * reading 4 bytes from a device sending 3 desynchronises the stream
 * permanently and the pointer moves at random. */
static void mouse_enable_wheel(void)
{
    mouse_cmd_arg(0xF3, 200);
    mouse_cmd_arg(0xF3, 100);
    mouse_cmd_arg(0xF3, 80);
    if (mouse_cmd_reply(0xF2) == 3) mouse_pktlen = 4;
    mouse_cmd_arg(0xF3, 100);      /* a sane reporting rate either way */
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
    mouse_enable_wheel();                   /* ...and 4-byte packets, if it has one */
    mouse_cmd(0xF4);                        /* enable data reporting */
    ps2_flush();
}

void idt_init(void)
{
    set_gate(0, fault_0);    set_gate(1, fault_1);
    set_gate(2, fault_2);    set_gate(3, fault_3);
    set_gate(4, fault_4);    set_gate(5, fault_5);
    set_gate(6, fault_6);    set_gate(7, fault_7);
    set_gate(8, fault_8);    set_gate(9, fault_9);
    set_gate(10, fault_10);  set_gate(11, fault_11);
    set_gate(12, fault_12);  set_gate(13, fault_13);
    set_gate(14, fault_14);  set_gate(15, fault_15);
    set_gate(16, fault_16);  set_gate(17, fault_17);
    set_gate(18, fault_18);  set_gate(19, fault_19);
    set_gate(20, fault_20);  set_gate(21, fault_21);
    set_gate(22, fault_22);  set_gate(23, fault_23);
    set_gate(24, fault_24);  set_gate(25, fault_25);
    set_gate(26, fault_26);  set_gate(27, fault_27);
    set_gate(28, fault_28);  set_gate(29, fault_29);
    set_gate(30, fault_30);  set_gate(31, fault_31);
    for (int i = 32; i < 256; i++) set_gate(i, ignore_isr);   /* stray IRQs: ack     */
#ifndef ZL_64
    /* THE SYSCALL DOOR. Installed before the IRQs so the ordering is visible:
     * this is the only DPL-3 entry in the whole table, and it is the only way
     * back into the kernel that ring 3 has. syscall_isr is hand-written
     * assembly in usermode.c - it has to save the user's registers before any
     * compiler prologue could run. */
    set_gate_user(0x80, syscall_isr);
#endif

    set_gate(0x20, timer_isr);      /* IRQ0  timer            */
    set_gate(0x21, keyboard_isr);   /* IRQ1  keyboard         */
    set_gate(0x2C, mouse_isr);      /* IRQ12 mouse (on slave) */

    idtp.limit = sizeof(idt) - 1;
#ifdef ZL_64
    idtp.base  = (u64)&idt;      /* NOT `unsigned long` - see the typedef */
#else
    idtp.base  = (u32)&idt;
#endif
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
