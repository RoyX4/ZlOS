/* crash.c - bounded CPU-fault evidence, before recovery exists.
 *
 * The previous exception path threw away its frame and halted. That preserves
 * the screen but loses the only facts that identify the failure. This module
 * records the vector, architectural error code, instruction pointer, control
 * frame, stack identity and CR2 without allocating, locking or following a
 * pointer from the failed context. The magic is committed last, so a second
 * reader never mistakes a half-written record for a complete one.
 *
 * It does NOT claim a full register dump, stack trace, durable dump or
 * offender-only recovery. idt.c's compiler-generated interrupt prologue has
 * already changed the general registers before C can see them, and the record
 * remains in RAM. Those are later contracts; inventing them here would turn a
 * real first step into a false completion claim. */
#include "crash.h"

void ser_puts(const char *s);

static volatile struct crash_record last_record;
static volatile crash_u32 recording;

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
    h = mix64(h, r->error_code);
    h = mix64(h, r->ip);
    h = mix64(h, r->cs);
    h = mix64(h, r->flags);
    h = mix64(h, r->sp);
    h = mix64(h, r->ss);
    h = mix64(h, r->cr2);
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
    out->reserved = last_record.reserved;
    out->error_code = last_record.error_code;
    out->ip = last_record.ip;
    out->cs = last_record.cs;
    out->flags = last_record.flags;
    out->sp = last_record.sp;
    out->ss = last_record.ss;
    out->cr2 = last_record.cr2;
    out->checksum = last_record.checksum;
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
    if (r->vector != 14 && r->cr2 != 0) return 0;
    if (r->reserved != 0) return 0;
    return r->checksum == checksum_of(r);
}

int crash_capture(crash_u32 vector, crash_u32 has_error,
                  crash_u64 error_code, crash_u64 ip, crash_u64 cs,
                  crash_u64 flags, crash_u64 sp, crash_u64 ss,
                  crash_u64 cr2, crash_u32 word_bits)
{
    if (recording || last_record.magic == CRASH_RECORD_MAGIC) return 0;
    if (vector >= 32 || has_error > 1u) return 0;
    if ((int)has_error != crash_vector_has_error(vector)) return 0;
    if (word_bits != 32u && word_bits != 64u) return 0;
    if (!has_error && error_code != 0) return 0;
    if (vector != 14) cr2 = 0;

    recording = 1;
    last_record.magic = 0;              /* incomplete until the final store */
    last_record.version = CRASH_RECORD_VERSION;
    last_record.bytes = (crash_u16)sizeof(last_record);
    last_record.vector = vector;
    last_record.has_error = has_error;
    last_record.word_bits = word_bits;
    last_record.reserved = 0;
    last_record.error_code = error_code;
    last_record.ip = ip;
    last_record.cs = cs;
    last_record.flags = flags;
    last_record.sp = sp;
    last_record.ss = ss;
    last_record.cr2 = cr2;
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
    puts_b(&w, " vec="); dec_b(&w, r->vector);
    puts_b(&w, " haserr="); dec_b(&w, r->has_error);
    puts_b(&w, " err="); hex_b(&w, r->error_code);
    puts_b(&w, " ip="); hex_b(&w, r->ip);
    puts_b(&w, " cs="); hex_b(&w, r->cs);
    puts_b(&w, " flags="); hex_b(&w, r->flags);
    puts_b(&w, " sp="); hex_b(&w, r->sp);
    puts_b(&w, " ss="); hex_b(&w, r->ss);
    puts_b(&w, " cr2="); hex_b(&w, r->cr2);
    puts_b(&w, " checksum="); hex_b(&w, r->checksum);
    puts_b(&w, " END\n");
    if (w.len < capacity) out[w.len] = 0;
    else out[capacity - 1] = 0;
    return w.ok;
}

void crash_report(void)
{
    struct crash_record snapshot;
    char line[320];
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
