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
 * Hardware IRQs use gcc's `interrupt` attribute. CPU exceptions use typed
 * assembly stubs so every general register is captured before a compiler
 * prologue and error-code and no-error frames cannot be confused. The file is
 * compiled with -mgeneral-regs-only so a handler never touches SSE.
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

#include "telemetry.h"
#include "crash.h"

void          zl_outb(u16 port, u8 val);
unsigned char zl_inb(u16 port);
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
 * every other vector in this table is unreachable from user mode. */
static void set_gate_user(int n, void *handler)
{
#ifdef ZL_64
    u64 a = (u64)handler;
    idt[n].lo = a & 0xffff;
    idt[n].mid = (a >> 16) & 0xffff;
    idt[n].hi = (u32)(a >> 32);
    idt[n].sel = 0x08;
    idt[n].ist = 0;
    idt[n].flags = 0xEE;
    idt[n].zero = 0;
#else
    u32 a = (u32)handler;
    idt[n].lo    = a & 0xFFFF;
    idt[n].hi    = (a >> 16) & 0xFFFF;
    idt[n].sel   = 0x08;
    idt[n].zero  = 0;
    idt[n].flags = 0xEE;    /* present, DPL 3, 32-bit interrupt gate */
#endif
}

static void set_gate_ist(int n, void *handler, u8 ist)
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
    idt[n].ist   = ist & 7u;
    idt[n].flags = 0x8E;        /* present, ring 0, 64-bit interrupt gate  */
    idt[n].zero  = 0;
#else
    (void)ist;
    u32 a = (u32)handler;
    idt[n].lo    = a & 0xFFFF;
    idt[n].hi    = (a >> 16) & 0xFFFF;
    idt[n].sel   = 0x08;        /* our GDT code selector */
    idt[n].zero  = 0;
    idt[n].flags = 0x8E;        /* present, ring 0, 32-bit interrupt gate */
#endif
}

static void set_gate(int n, void *handler)
{
    set_gate_ist(n, handler, 0);
}

/* ---- the state the ISRs publish, read by zl ------------------------- */
static volatile u32 tick_count = 0;

#define KBUF_SIZE 256
static volatile u8  kbuf[KBUF_SIZE];
static volatile u32 kbuf_tsc[KBUF_SIZE];
static volatile int kbuf_head = 0;   /* ISR writes here */
static volatile int kbuf_tail = 0;   /* zl reads here   */
static u32 last_scan_tsc;

static u32 irq_tsc_lo(void)
{
    u32 lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    (void)hi;
    return lo;
}

u32 idt_ticks(void) { return tick_count; }

/* pull one scancode from the ring, or 0 if empty (0 is never a real code) */
int idt_scan(void)
{
    if (kbuf_tail == kbuf_head) return 0;
    u8 c = kbuf[kbuf_tail];
    last_scan_tsc = kbuf_tsc[kbuf_tail];
    kbuf_tail = (kbuf_tail + 1) & (KBUF_SIZE - 1);
    return c;
}
u32 idt_scan_tsc(void) { return last_scan_tsc; }

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
static volatile u32 mouse_packet_tsc;

int idt_mouse_x(void)   { return mouse_x; }
int idt_mouse_y(void)   { return mouse_y; }
int idt_mouse_btn(void) { return mouse_btn; }
u32 idt_mouse_take_tsc(void)
{
    u32 tsc = mouse_packet_tsc;
    mouse_packet_tsc = 0;
    return tsc;
}

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
    mouse_packet_tsc = irq_tsc_lo();

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
/* EFI clang is LLP64: unsigned long is only 32 bits there. The CPU always
 * builds this long-mode frame from 64-bit words, so every field must be u64. */
struct interrupt_frame { u64 ip, cs, flags, sp, ss; };
#else
struct interrupt_frame { u32 ip, cs, flags, sp, ss; };
#endif

/* IRQ0: the PIT ticks ~100 times a second. Long mode uses a hand-written full
 * register stub in usermode.c so an interrupted Ring-3 frame can become a
 * resumable process context; 32-bit keeps the compiler interrupt wrapper. */
void idt_timer_tick(void)
{
    tick_count++;
    zlt_count(ZLLOG_C_IRQ_TIMER, 1);
    irq_done(0);
}

#ifdef ZL_64
void user64_timer_isr(void);
#else
__attribute__((interrupt))
static void timer_isr(struct interrupt_frame *f)
{
    (void)f;
    idt_timer_tick();
}
#endif

/* IRQ1: a key changed. Grab the scancode before the controller moves on
 * and drop it in the ring for zl to translate at its leisure. */
__attribute__((interrupt))
static void keyboard_isr(struct interrupt_frame *f)
{
    (void)f;
    zlt_count(ZLLOG_C_IRQ_KEYBOARD, 1);
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
        kbuf_tsc[kbuf_head] = irq_tsc_lo();
        kbuf_head = next;
    } else {
        zlt_count(ZLLOG_C_INPUT_DROP, 1);
        zlt_irq_event(ZLLOG_SUB_INPUT, ZLLOG_EV_DROP, ZLLOG_ERROR,
                      1u, b, KBUF_SIZE);
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
    zlt_count(ZLLOG_C_IRQ_MOUSE, 1);
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
    zlt_count(ZLLOG_C_IRQ_STRAY, 1);
    irq_done(8);                 /* acknowledge whichever controller is live */
}

/* Fixed IPI used only to wake an AP parked in smp.c. It carries no payload;
 * the per-core cache-line slot was published before the IPI. Keeping it out of
 * ignore_isr also keeps ordinary compositor work out of the stray-IRQ count. */
__attribute__((interrupt))
static void smp_wake_isr(struct interrupt_frame *f)
{
    (void)f;
    apic_eoi();
}

/* Exception stack shapes are not interchangeable. #PF/#GP and the other
 * error-code exceptions push one extra machine word; treating all 32 vectors
 * as a no-error handler shifted RIP/CS/RFLAGS and made the old "fault record"
 * fiction. Each gate below now uses the ABI-matching signature and supplies
 * its vector explicitly. */
#ifdef ZL_64
struct fault_frame64 {
    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 bp, di, si, dx, cx, bx, ax;
    u64 vector, error, ip, cs, flags, sp, ss;
};

__attribute__((noreturn, noinline, used))
void fault_stop64(const struct fault_frame64 *r)
{
    /* Intel SDM 3A 6.14.2: 64-bit mode pushes SS:RSP unconditionally, even
     * without a CPL change. The old same-CPL branch recorded the address of
     * the saved field instead of the saved RSP and the old receipt compared
     * that derived value only with itself. */
    u64 sp = r->sp;
    u64 ss = r->ss;
    u64 cr2 = 0;
    u64 handler_sp = (u64)(uptr)r;
    u64 stack_low = 0, stack_high = 0;
    extern unsigned long long gdt64_double_fault_stack_low(void);
    extern unsigned long long gdt64_double_fault_stack_top(void);
    if (r->vector == 8u) {
        stack_low = gdt64_double_fault_stack_low();
        stack_high = gdt64_double_fault_stack_top();
    }
    struct crash_registers registers = {
        CRASH_REGS_64_ALL, 0,
        r->ax, r->bx, r->cx, r->dx, r->si, r->di, r->bp, sp,
        r->r8, r->r9, r->r10, r->r11, r->r12, r->r13, r->r14, r->r15
    };

    __asm__ volatile("cli");
    if (r->vector == 8u || r->vector == 14u)
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    extern int user64_is_running(void);
    extern void user64_mark_fault(u32 vector, u32 error, u64 address);
    extern void user64_abort(void) __attribute__((noreturn));
    if ((r->cs & 3u) == 3u && user64_is_running()) {
        zlt_irq_event(ZLLOG_SUB_CPU, ZLLOG_EV_FAULT, ZLLOG_ERROR,
                      (u32)r->vector, (u32)r->error, (u32)cr2);
        user64_mark_fault((u32)r->vector, (u32)r->error, cr2);
        user64_abort();
    }
    zlt_irq_event(ZLLOG_SUB_CPU, ZLLOG_EV_FAULT, ZLLOG_FATAL,
                  (u32)r->vector, (u32)r->error, (u32)cr2);
    zlt_irq_event(ZLLOG_SUB_CPU, ZLLOG_EV_FAULT, ZLLOG_FATAL,
                  (u32)r->ip, (u32)(r->ip >> 32), (u32)r->flags);
    (void)crash_capture((u32)r->vector,
                        (u32)crash_vector_has_error((u32)r->vector), r->error,
                        r->ip, r->cs, r->flags, sp, ss, cr2, handler_sp,
                        stack_low, stack_high, 64u, &registers);
    crash_report();
    for (;;) __asm__ volatile("hlt");
}

#define DECLARE_FAULT(n) void fault_##n(void);
DECLARE_FAULT(0)  DECLARE_FAULT(1)  DECLARE_FAULT(2)  DECLARE_FAULT(3)
DECLARE_FAULT(4)  DECLARE_FAULT(5)  DECLARE_FAULT(6)  DECLARE_FAULT(7)
DECLARE_FAULT(8)  DECLARE_FAULT(9)  DECLARE_FAULT(10) DECLARE_FAULT(11)
DECLARE_FAULT(12) DECLARE_FAULT(13) DECLARE_FAULT(14) DECLARE_FAULT(15)
DECLARE_FAULT(16) DECLARE_FAULT(17) DECLARE_FAULT(18) DECLARE_FAULT(19)
DECLARE_FAULT(20) DECLARE_FAULT(21) DECLARE_FAULT(22) DECLARE_FAULT(23)
DECLARE_FAULT(24) DECLARE_FAULT(25) DECLARE_FAULT(26) DECLARE_FAULT(27)
DECLARE_FAULT(28) DECLARE_FAULT(29) DECLARE_FAULT(30) DECLARE_FAULT(31)
#undef DECLARE_FAULT

#define FAULT_NOERR_ASM64(n) \
    ".globl fault_" #n "\nfault_" #n ":\n pushq $0\n pushq $" #n \
    "\n jmp fault_common64\n"
#define FAULT_ERR_ASM64(n) \
    ".globl fault_" #n "\nfault_" #n ":\n pushq $" #n \
    "\n jmp fault_common64\n"
#ifdef ZL_EFI
#define FAULT_CALL64 \
    "    mov %rsp, %rcx\n" \
    "    andq $-16, %rsp\n" \
    "    subq $32, %rsp\n" \
    "    call fault_stop64\n"
#else
#define FAULT_CALL64 \
    "    mov %rsp, %rdi\n" \
    "    andq $-16, %rsp\n" \
    "    call fault_stop64\n"
#endif

__asm__(
    ".text\n"
    "fault_common64:\n"
    "    pushq %rax\n    pushq %rbx\n    pushq %rcx\n    pushq %rdx\n"
    "    pushq %rsi\n    pushq %rdi\n    pushq %rbp\n"
    "    pushq %r8\n     pushq %r9\n     pushq %r10\n    pushq %r11\n"
    "    pushq %r12\n    pushq %r13\n    pushq %r14\n    pushq %r15\n"
    "    cld\n"
    FAULT_CALL64
    "    ud2\n"
    FAULT_NOERR_ASM64(0)  FAULT_NOERR_ASM64(1)
    FAULT_NOERR_ASM64(2)  FAULT_NOERR_ASM64(3)
    FAULT_NOERR_ASM64(4)  FAULT_NOERR_ASM64(5)
    FAULT_NOERR_ASM64(6)  FAULT_NOERR_ASM64(7)
    FAULT_ERR_ASM64(8)    FAULT_NOERR_ASM64(9)
    FAULT_ERR_ASM64(10)   FAULT_ERR_ASM64(11)
    FAULT_ERR_ASM64(12)   FAULT_ERR_ASM64(13)
    FAULT_ERR_ASM64(14)   FAULT_NOERR_ASM64(15)
    FAULT_NOERR_ASM64(16) FAULT_ERR_ASM64(17)
    FAULT_NOERR_ASM64(18) FAULT_NOERR_ASM64(19)
    FAULT_NOERR_ASM64(20) FAULT_ERR_ASM64(21)
    FAULT_NOERR_ASM64(22) FAULT_NOERR_ASM64(23)
    FAULT_NOERR_ASM64(24) FAULT_NOERR_ASM64(25)
    FAULT_NOERR_ASM64(26) FAULT_NOERR_ASM64(27)
    FAULT_NOERR_ASM64(28) FAULT_ERR_ASM64(29)
    FAULT_ERR_ASM64(30)   FAULT_NOERR_ASM64(31)
);

#undef FAULT_CALL64
#undef FAULT_NOERR_ASM64
#undef FAULT_ERR_ASM64
#else
/* The compiler interrupt attribute saves registers before C runs, which makes
 * a C-level register dump post-fault evidence rather than fault-time evidence.
 * These stubs normalize the error-code shape and run PUSHA before any compiler
 * prologue. The frame layout is asserted by the field order below and exercised
 * with exact sentinels by verify-crash.py. */
struct fault_frame32 {
    u32 di, si, bp, saved_sp, bx, dx, cx, ax;
    u32 vector, error, ip, cs, flags, sp, ss;
};

__attribute__((noreturn, noinline, used))
void fault_stop32(const struct fault_frame32 *r)
{
    u32 sp = (r->cs & 3u) ? r->sp : r->saved_sp + 5u * sizeof(u32);
    u32 ss = (r->cs & 3u) ? r->ss : 0;
    u32 cr2 = 0;
    struct crash_registers registers = {
        CRASH_REGS_32_ALL, 0,
        r->ax, r->bx, r->cx, r->dx, r->si, r->di, r->bp, sp,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    __asm__ volatile("cli");
    if (r->vector == 8u || r->vector == 14u)
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    zlt_irq_event(ZLLOG_SUB_CPU, ZLLOG_EV_FAULT, ZLLOG_FATAL,
                  r->vector, r->error, cr2);
    zlt_irq_event(ZLLOG_SUB_CPU, ZLLOG_EV_FAULT, ZLLOG_FATAL,
                  r->ip, r->cs, r->flags);
    (void)crash_capture(r->vector, crash_vector_has_error(r->vector), r->error,
                        r->ip, r->cs, r->flags, sp, ss, cr2, (u32)(uptr)r,
                        0, 0, 32u, &registers);
    crash_report();
    for (;;) __asm__ volatile("hlt");
}

#define DECLARE_FAULT(n) void fault_##n(void);
DECLARE_FAULT(0)  DECLARE_FAULT(1)  DECLARE_FAULT(2)  DECLARE_FAULT(3)
DECLARE_FAULT(4)  DECLARE_FAULT(5)  DECLARE_FAULT(6)  DECLARE_FAULT(7)
DECLARE_FAULT(8)  DECLARE_FAULT(9)  DECLARE_FAULT(10) DECLARE_FAULT(11)
DECLARE_FAULT(12) DECLARE_FAULT(13) DECLARE_FAULT(14) DECLARE_FAULT(15)
DECLARE_FAULT(16) DECLARE_FAULT(17) DECLARE_FAULT(18) DECLARE_FAULT(19)
DECLARE_FAULT(20) DECLARE_FAULT(21) DECLARE_FAULT(22) DECLARE_FAULT(23)
DECLARE_FAULT(24) DECLARE_FAULT(25) DECLARE_FAULT(26) DECLARE_FAULT(27)
DECLARE_FAULT(28) DECLARE_FAULT(29) DECLARE_FAULT(30) DECLARE_FAULT(31)
#undef DECLARE_FAULT

#define FAULT_NOERR_ASM(n) \
    ".globl fault_" #n "\n.type fault_" #n ", @function\n" \
    "fault_" #n ":\n pushl $0\n pushl $" #n "\n jmp fault_common32\n" \
    ".size fault_" #n ", .-fault_" #n "\n"
#define FAULT_ERR_ASM(n) \
    ".globl fault_" #n "\n.type fault_" #n ", @function\n" \
    "fault_" #n ":\n pushl $" #n "\n jmp fault_common32\n" \
    ".size fault_" #n ", .-fault_" #n "\n"

__asm__(
    ".text\n"
    "fault_common32:\n"
    "    pusha\n"
    "    cld\n"
    "    mov $0x10, %ax\n"
    "    mov %ax, %ds\n"
    "    mov %ax, %es\n"
    "    pushl %esp\n"
    "    call fault_stop32\n"
    "    ud2\n"
    FAULT_NOERR_ASM(0)  FAULT_NOERR_ASM(1)
    FAULT_NOERR_ASM(2)  FAULT_NOERR_ASM(3)
    FAULT_NOERR_ASM(4)  FAULT_NOERR_ASM(5)
    FAULT_NOERR_ASM(6)  FAULT_NOERR_ASM(7)
    FAULT_ERR_ASM(8)    FAULT_NOERR_ASM(9)
    FAULT_ERR_ASM(10)   FAULT_ERR_ASM(11)
    FAULT_ERR_ASM(12)   FAULT_ERR_ASM(13)
    FAULT_ERR_ASM(14)   FAULT_NOERR_ASM(15)
    FAULT_NOERR_ASM(16) FAULT_ERR_ASM(17)
    FAULT_NOERR_ASM(18) FAULT_NOERR_ASM(19)
    FAULT_NOERR_ASM(20) FAULT_ERR_ASM(21)
    FAULT_NOERR_ASM(22) FAULT_NOERR_ASM(23)
    FAULT_NOERR_ASM(24) FAULT_NOERR_ASM(25)
    FAULT_NOERR_ASM(26) FAULT_NOERR_ASM(27)
    FAULT_NOERR_ASM(28) FAULT_ERR_ASM(29)
    FAULT_ERR_ASM(30)   FAULT_NOERR_ASM(31)
);

#undef FAULT_NOERR_ASM
#undef FAULT_ERR_ASM
#endif

static void *const fault_handlers[32] = {
    fault_0, fault_1, fault_2, fault_3, fault_4, fault_5, fault_6, fault_7,
    fault_8, fault_9, fault_10, fault_11, fault_12, fault_13, fault_14, fault_15,
    fault_16, fault_17, fault_18, fault_19, fault_20, fault_21, fault_22, fault_23,
    fault_24, fault_25, fault_26, fault_27, fault_28, fault_29, fault_30, fault_31
};

/* Deliberate invalid-opcode trigger for the dedicated QEMU crash receipt.
 * It is reachable only through the explicit `crashtest` diagnostic command. */
#ifdef ZL_64
__asm__(
    ".text\n"
    ".globl crash_test_ud2\n"
    "crash_test_ud2:\n"
    "    movabs $0x0102030405060708, %rax\n"
    "    movabs $0x1112131415161718, %rbx\n"
    "    movabs $0x2122232425262728, %rcx\n"
    "    movabs $0x3132333435363738, %rdx\n"
    "    movabs $0x4142434445464748, %rsi\n"
    "    movabs $0x5152535455565758, %rdi\n"
    "    movabs $0x6162636465666768, %rbp\n"
    "    movabs $0x8182838485868788, %r8\n"
    "    movabs $0x9192939495969798, %r9\n"
    "    movabs $0xa1a2a3a4a5a6a7a8, %r10\n"
    "    movabs $0xb1b2b3b4b5b6b7b8, %r11\n"
    "    movabs $0xc1c2c3c4c5c6c7c8, %r12\n"
    "    movabs $0xd1d2d3d4d5d6d7d8, %r13\n"
    "    movabs $0xe1e2e3e4e5e6e7e8, %r14\n"
    /* A relocated PE image has no fixed link-time IP. Carry the exact runtime
     * fault-label address in one saved register so an external receipt can
     * prove symbol identity without guessing the firmware load base. */
    "    lea crash_test_ud2_fault(%rip), %r15\n"
    ".globl crash_test_ud2_fault\n"
    "crash_test_ud2_fault:\n"
    "    ud2\n"
);

/* An out-of-range GDT selector gives a deterministic #GP with selector 0x38
 * as its architectural error code. It proves the CPU-pushed error-code frame,
 * unlike INT 13 which would not push one. */
__asm__(
    ".text\n"
    ".globl crash_test_gp\n"
    "crash_test_gp:\n"
    "    movabs $0x1112131415161718, %rbx\n"
    "    movabs $0x2122232425262728, %rcx\n"
    "    movabs $0x3132333435363738, %rdx\n"
    "    movabs $0x4142434445464748, %rsi\n"
    "    movabs $0x5152535455565758, %rdi\n"
    "    movabs $0x6162636465666768, %rbp\n"
    "    movabs $0x8182838485868788, %r8\n"
    "    movabs $0x9192939495969798, %r9\n"
    "    movabs $0xa1a2a3a4a5a6a7a8, %r10\n"
    "    movabs $0xb1b2b3b4b5b6b7b8, %r11\n"
    "    movabs $0xc1c2c3c4c5c6c7c8, %r12\n"
    "    movabs $0xd1d2d3d4d5d6d7d8, %r13\n"
    "    movabs $0xe1e2e3e4e5e6e7e8, %r14\n"
    "    lea crash_test_gp_fault(%rip), %r15\n"
    "    mov $0x38, %eax\n"
    ".globl crash_test_gp_fault\n"
    "crash_test_gp_fault:\n"
    "    mov %ax, %ds\n"
    "    ud2\n"
);

/* A page fault while RSP names an unmapped page cannot deliver on that same
 * stack. The second page fault becomes #DF, whose IDT gate must switch to IST1
 * before any push. This route is destructive and QEMU-only. */
__asm__(
    ".text\n"
    ".globl crash_test_df\n"
    "crash_test_df:\n"
    "    cli\n"
    "    xor %rsp, %rsp\n"
    ".globl crash_test_df_fault\n"
    "crash_test_df_fault:\n"
    "    mov (%rsp), %rax\n"
    "    ud2\n"
);
#else
/* Distinct nonzero values make register swaps and post-prologue captures
 * observable. crash_test_ud2_fault names the exact instruction that must be
 * reported as IP. The function never returns, so clobbering callee-saved
 * registers is intentional. */
__asm__(
    ".text\n"
    ".globl crash_test_ud2\n"
    ".type crash_test_ud2, @function\n"
    "crash_test_ud2:\n"
    "    mov $0xa1b2c3d4, %eax\n"
    "    mov $0xb1c2d3e4, %ebx\n"
    "    mov $0xc1d2e3f4, %ecx\n"
    "    mov $0xd1e2f304, %edx\n"
    "    mov $0x51627384, %esi\n"
    "    mov $0x61728394, %edi\n"
    "    mov $0x718293a4, %ebp\n"
    ".globl crash_test_ud2_fault\n"
    "crash_test_ud2_fault:\n"
    "    ud2\n"
    ".size crash_test_ud2, .-crash_test_ud2\n"
);
#endif

/* usermode.c, in assembly. Declared as a function taking no arguments purely so
 * its address can be taken - it is never called from C and never returns
 * normally; it iret's, or it abandons ring 3 entirely on SYS_EXIT. */
void syscall_isr(void);

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
    for (int i = 0;  i < 32;  i++) set_gate(i, fault_handlers[i]);
#ifdef ZL_64
    /* #DF must not depend on the stack whose failure may have caused it. */
    set_gate_ist(8, fault_handlers[8], 1);
#endif
    for (int i = 32; i < 256; i++) set_gate(i, ignore_isr);   /* stray IRQs: ack     */
    /* THE SYSCALL DOOR. Installed before the IRQs so the ordering is visible:
     * this is the only DPL-3 entry in the whole table, and it is the only way
     * back into the kernel that ring 3 has. syscall_isr is hand-written
     * assembly in usermode.c - it has to save the user's registers before any
     * compiler prologue could run. */
    set_gate_user(0x80, syscall_isr);

#ifdef ZL_64
    set_gate(0x20, user64_timer_isr); /* IRQ0 full context/preemption */
#else
    set_gate(0x20, timer_isr);      /* IRQ0  timer            */
#endif
    set_gate(0x21, keyboard_isr);   /* IRQ1  keyboard         */
    set_gate(0x2C, mouse_isr);      /* IRQ12 mouse (on slave) */
    set_gate(0xF1, smp_wake_isr);   /* bounded AP band-work wake */

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
