/* crash.c - bounded CPU-fault evidence, before recovery exists.
 *
 * The previous exception path threw away its frame and halted. That preserves
 * the screen but loses the only facts that identify the failure. This module
 * records the vector, architectural error code, instruction pointer, control
 * frame, stack identity, CR2 and any pre-prologue general registers supplied
 * by the architecture entry stub. It does not allocate or lock. The magic is
 * committed last, so a second reader never mistakes a half-written record for
 * a complete one.
 *
 * It does NOT claim a stack trace, durable dump or offender-only recovery. The
 * The 32-bit and 64-bit assembly entries supply all eight or sixteen
 * pre-prologue general registers. The record remains in RAM. */
#include "crash.h"

void ser_puts(const char *s);

static volatile struct crash_record last_record;
static volatile crash_u32 recording;

_Static_assert(sizeof(struct crash_record) == 240,
               "crash record must have one ILP32/LP64 wire size");

int crash_vector_has_error(crash_u32 vector)
{
    switch (vector) {
    case 8:                         /* #DF */
    case 10: case 11: case 12:      /* #TS, #NP, #SS */
    case 13: case 14:               /* #GP, #PF */
    case 17:                        /* #AC */
    case 21:                        /* #CP */
    case 29:                        /* #VC */
    case 30:                        /* #SX */
        return 1;
    default:
        return 0;
    }
}

static crash_u64 mix64(crash_u64 hash, crash_u64 value)
{
    for (int i = 0; i < 8; i++) {
        hash ^= (value >> (i * 8)) & 0xFFu;
        hash *= 1099511628211ULL;
    }
    return hash;
}

static crash_u64 checksum_of(const struct crash_record *r)
{
    crash_u64 h = 1469598103934665603ULL;
    h = mix64(h, r->version);
    h = mix64(h, r->bytes);
    h = mix64(h, r->vector);
    h = mix64(h, r->has_error);
    h = mix64(h, r->word_bits);
    h = mix64(h, r->register_mask);
    h = mix64(h, r->error_code);
    h = mix64(h, r->ip);
    h = mix64(h, r->cs);
    h = mix64(h, r->flags);
    h = mix64(h, r->sp);
    h = mix64(h, r->ss);
    h = mix64(h, r->cr2);
    h = mix64(h, r->handler_sp);
    h = mix64(h, r->emergency_stack_low);
    h = mix64(h, r->emergency_stack_high);
    h = mix64(h, r->ax); h = mix64(h, r->bx);
    h = mix64(h, r->cx); h = mix64(h, r->dx);
    h = mix64(h, r->si); h = mix64(h, r->di);
    h = mix64(h, r->bp); h = mix64(h, r->register_sp);
    h = mix64(h, r->r8); h = mix64(h, r->r9);
    h = mix64(h, r->r10); h = mix64(h, r->r11);
    h = mix64(h, r->r12); h = mix64(h, r->r13);
    h = mix64(h, r->r14); h = mix64(h, r->r15);
    return h;
}

static void copy_from_volatile(struct crash_record *out)
{
    out->magic = last_record.magic;
    out->version = last_record.version;
    out->bytes = last_record.bytes;
    out->vector = last_record.vector;
    out->has_error = last_record.has_error;
    out->word_bits = last_record.word_bits;
    out->register_mask = last_record.register_mask;
    out->error_code = last_record.error_code;
    out->ip = last_record.ip;
    out->cs = last_record.cs;
    out->flags = last_record.flags;
    out->sp = last_record.sp;
    out->ss = last_record.ss;
    out->cr2 = last_record.cr2;
    out->handler_sp = last_record.handler_sp;
    out->emergency_stack_low = last_record.emergency_stack_low;
    out->emergency_stack_high = last_record.emergency_stack_high;
    out->ax = last_record.ax; out->bx = last_record.bx;
    out->cx = last_record.cx; out->dx = last_record.dx;
    out->si = last_record.si; out->di = last_record.di;
    out->bp = last_record.bp; out->register_sp = last_record.register_sp;
    out->r8 = last_record.r8; out->r9 = last_record.r9;
    out->r10 = last_record.r10; out->r11 = last_record.r11;
    out->r12 = last_record.r12; out->r13 = last_record.r13;
    out->r14 = last_record.r14; out->r15 = last_record.r15;
    out->checksum = last_record.checksum;
}

static int registers_valid(const struct crash_record *r)
{
    crash_u32 full = r->word_bits == 32u ? CRASH_REGS_32_ALL : CRASH_REGS_64_ALL;
    if (r->register_mask != 0 && r->register_mask != full) return 0;
    if (!(r->register_mask & CRASH_REG_AX) && r->ax) return 0;
    if (!(r->register_mask & CRASH_REG_BX) && r->bx) return 0;
    if (!(r->register_mask & CRASH_REG_CX) && r->cx) return 0;
    if (!(r->register_mask & CRASH_REG_DX) && r->dx) return 0;
    if (!(r->register_mask & CRASH_REG_SI) && r->si) return 0;
    if (!(r->register_mask & CRASH_REG_DI) && r->di) return 0;
    if (!(r->register_mask & CRASH_REG_BP) && r->bp) return 0;
    if (!(r->register_mask & CRASH_REG_SP) && r->register_sp) return 0;
    if (!(r->register_mask & CRASH_REG_R8) && r->r8) return 0;
    if (!(r->register_mask & CRASH_REG_R9) && r->r9) return 0;
    if (!(r->register_mask & CRASH_REG_R10) && r->r10) return 0;
    if (!(r->register_mask & CRASH_REG_R11) && r->r11) return 0;
    if (!(r->register_mask & CRASH_REG_R12) && r->r12) return 0;
    if (!(r->register_mask & CRASH_REG_R13) && r->r13) return 0;
    if (!(r->register_mask & CRASH_REG_R14) && r->r14) return 0;
    if (!(r->register_mask & CRASH_REG_R15) && r->r15) return 0;
    return 1;
}

int crash_validate(const struct crash_record *r)
{
    if (!r || r->magic != CRASH_RECORD_MAGIC) return 0;
    if (r->version != CRASH_RECORD_VERSION) return 0;
    if (r->bytes != (crash_u16)sizeof(*r)) return 0;
    if (r->vector >= 32) return 0;
    if (r->has_error > 1u) return 0;
    if ((int)r->has_error != crash_vector_has_error(r->vector)) return 0;
    if (r->word_bits != 32u && r->word_bits != 64u) return 0;
    if (!r->has_error && r->error_code != 0) return 0;
    if (r->vector == 8u && r->error_code != 0) return 0;
    if (r->vector != 8u && r->vector != 14u && r->cr2 != 0) return 0;
    if (r->handler_sp == 0) return 0;
    if (r->vector == 8u && r->word_bits == 64u) {
        if (r->emergency_stack_low == 0 ||
                r->emergency_stack_high <= r->emergency_stack_low ||
                r->handler_sp < r->emergency_stack_low ||
                r->handler_sp >= r->emergency_stack_high) return 0;
    } else if (r->emergency_stack_low != 0 || r->emergency_stack_high != 0) {
        return 0;
    }
    if (!registers_valid(r)) return 0;
    return r->checksum == checksum_of(r);
}

int crash_capture(crash_u32 vector, crash_u32 has_error,
                  crash_u64 error_code, crash_u64 ip, crash_u64 cs,
                  crash_u64 flags, crash_u64 sp, crash_u64 ss,
                  crash_u64 cr2, crash_u64 handler_sp,
                  crash_u64 emergency_stack_low,
                  crash_u64 emergency_stack_high, crash_u32 word_bits,
                  const struct crash_registers *registers)
{
    if (recording || last_record.magic == CRASH_RECORD_MAGIC) return 0;
    if (vector >= 32 || has_error > 1u) return 0;
    if ((int)has_error != crash_vector_has_error(vector)) return 0;
    if (word_bits != 32u && word_bits != 64u) return 0;
    if (!has_error && error_code != 0) return 0;
    if (vector == 8u && error_code != 0) return 0;
    if (vector != 8u && vector != 14u) cr2 = 0;
    if (handler_sp == 0) return 0;
    if (vector == 8u && word_bits == 64u) {
        if (emergency_stack_low == 0 || emergency_stack_high <= emergency_stack_low ||
                handler_sp < emergency_stack_low ||
                handler_sp >= emergency_stack_high) return 0;
    } else if (emergency_stack_low != 0 || emergency_stack_high != 0) {
        return 0;
    }
    if (registers && (registers->reserved != 0 ||
            registers->mask != (word_bits == 32u ? CRASH_REGS_32_ALL :
                                                       CRASH_REGS_64_ALL))) return 0;

    recording = 1;
    last_record.magic = 0;              /* incomplete until the final store */
    last_record.version = CRASH_RECORD_VERSION;
    last_record.bytes = (crash_u16)sizeof(last_record);
    last_record.vector = vector;
    last_record.has_error = has_error;
    last_record.word_bits = word_bits;
    last_record.register_mask = registers ? registers->mask : 0;
    last_record.error_code = error_code;
    last_record.ip = ip;
    last_record.cs = cs;
    last_record.flags = flags;
    last_record.sp = sp;
    last_record.ss = ss;
    last_record.cr2 = cr2;
    last_record.handler_sp = handler_sp;
    last_record.emergency_stack_low = emergency_stack_low;
    last_record.emergency_stack_high = emergency_stack_high;
    last_record.ax = registers ? registers->ax : 0;
    last_record.bx = registers ? registers->bx : 0;
    last_record.cx = registers ? registers->cx : 0;
    last_record.dx = registers ? registers->dx : 0;
    last_record.si = registers ? registers->si : 0;
    last_record.di = registers ? registers->di : 0;
    last_record.bp = registers ? registers->bp : 0;
    last_record.register_sp = registers ? registers->sp : 0;
    last_record.r8 = registers ? registers->r8 : 0;
    last_record.r9 = registers ? registers->r9 : 0;
    last_record.r10 = registers ? registers->r10 : 0;
    last_record.r11 = registers ? registers->r11 : 0;
    last_record.r12 = registers ? registers->r12 : 0;
    last_record.r13 = registers ? registers->r13 : 0;
    last_record.r14 = registers ? registers->r14 : 0;
    last_record.r15 = registers ? registers->r15 : 0;
    {
        struct crash_record staged;
        copy_from_volatile(&staged);
        staged.magic = CRASH_RECORD_MAGIC;
        last_record.checksum = checksum_of(&staged);
    }
    __asm__ volatile("" ::: "memory");
    last_record.magic = CRASH_RECORD_MAGIC;  /* commit point */
    __asm__ volatile("" ::: "memory");
    return 1;
}

int crash_snapshot(struct crash_record *out)
{
    if (!out) return 0;
    copy_from_volatile(out);
    return crash_validate(out);
}

struct writer {
    char *out;
    crash_u32 cap;
    crash_u32 len;
    int ok;
};

static void putc_b(struct writer *w, char c)
{
    if (w->cap > 0 && w->len + 1 < w->cap) w->out[w->len] = c;
    else w->ok = 0;
    w->len++;
}

static void puts_b(struct writer *w, const char *s)
{
    while (*s) putc_b(w, *s++);
}

static void dec_b(struct writer *w, crash_u32 value)
{
    char digits[10];
    int n = 0;
    if (!value) { putc_b(w, '0'); return; }
    while (value) { digits[n++] = (char)('0' + value % 10); value /= 10; }
    while (n) putc_b(w, digits[--n]);
}

static void hex_b(struct writer *w, crash_u64 value)
{
    static const char hex[] = "0123456789abcdef";
    puts_b(w, "0x");
    for (int i = 15; i >= 0; i--)
        putc_b(w, hex[(value >> (i * 4)) & 0xFu]);
}

int crash_format(char *out, crash_u32 capacity,
                 const struct crash_record *r)
{
    struct writer w = { out, capacity, 0, 1 };
    if (!out || capacity == 0 || !crash_validate(r)) return 0;
    puts_b(&w, "ZLCRASH v="); dec_b(&w, r->version);
    puts_b(&w, " bytes="); dec_b(&w, r->bytes);
    puts_b(&w, " bits="); dec_b(&w, r->word_bits);
    puts_b(&w, " regmask="); hex_b(&w, r->register_mask);
    puts_b(&w, " vec="); dec_b(&w, r->vector);
    puts_b(&w, " haserr="); dec_b(&w, r->has_error);
    puts_b(&w, " err="); hex_b(&w, r->error_code);
    puts_b(&w, " ip="); hex_b(&w, r->ip);
    puts_b(&w, " cs="); hex_b(&w, r->cs);
    puts_b(&w, " flags="); hex_b(&w, r->flags);
    puts_b(&w, " sp="); hex_b(&w, r->sp);
    puts_b(&w, " ss="); hex_b(&w, r->ss);
    puts_b(&w, " cr2="); hex_b(&w, r->cr2);
    puts_b(&w, " handlersp="); hex_b(&w, r->handler_sp);
    puts_b(&w, " stacklo="); hex_b(&w, r->emergency_stack_low);
    puts_b(&w, " stackhi="); hex_b(&w, r->emergency_stack_high);
    puts_b(&w, " ax="); hex_b(&w, r->ax);
    puts_b(&w, " bx="); hex_b(&w, r->bx);
    puts_b(&w, " cx="); hex_b(&w, r->cx);
    puts_b(&w, " dx="); hex_b(&w, r->dx);
    puts_b(&w, " si="); hex_b(&w, r->si);
    puts_b(&w, " di="); hex_b(&w, r->di);
    puts_b(&w, " bp="); hex_b(&w, r->bp);
    puts_b(&w, " rsp="); hex_b(&w, r->register_sp);
    puts_b(&w, " r8="); hex_b(&w, r->r8);
    puts_b(&w, " r9="); hex_b(&w, r->r9);
    puts_b(&w, " r10="); hex_b(&w, r->r10);
    puts_b(&w, " r11="); hex_b(&w, r->r11);
    puts_b(&w, " r12="); hex_b(&w, r->r12);
    puts_b(&w, " r13="); hex_b(&w, r->r13);
    puts_b(&w, " r14="); hex_b(&w, r->r14);
    puts_b(&w, " r15="); hex_b(&w, r->r15);
    puts_b(&w, " checksum="); hex_b(&w, r->checksum);
    puts_b(&w, " END\n");
    if (w.len < capacity) out[w.len] = 0;
    else out[capacity - 1] = 0;
    return w.ok;
}

void crash_report(void)
{
    struct crash_record snapshot;
    char line[896];
    if (!crash_snapshot(&snapshot)) {
        ser_puts("ZLCRASH INVALID END\n");
        return;
    }
    if (!crash_format(line, sizeof(line), &snapshot)) {
        ser_puts("ZLCRASH FORMAT_OVERFLOW END\n");
        return;
    }
    ser_puts(line);
}

#ifdef CRASH_HOSTTEST
void crash_host_reset(void)
{
    volatile unsigned char *p = (volatile unsigned char *)&last_record;
    for (unsigned int i = 0; i < sizeof(last_record); i++) p[i] = 0;
    recording = 0;
}
#endif
