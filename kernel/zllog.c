/* zllog.c - persistent, power-loss-tolerant boot evidence.
 *
 * The hot path is deliberately only a RAM append.  USB writes happen from
 * zllog_mount/zllog_flush in normal kernel context and only after an exact GPT
 * type, label, partition bound, unique GUID and journal superblock all agree.
 * A random USB disk can therefore never become a logging target by accident.
 */
#include "zllog.h"

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

extern u64 cpu_tsc(void);
extern u32 cpu_tsc_khz(void);
extern int cpu_has_intel_pt(void) __attribute__((weak));
extern u32 idt_ticks(void);
extern int xhci_msc_init(void);
extern int xhci_msc_read_capacity(void);
extern u32 xhci_msc_blocks(void);
extern u32 xhci_msc_blocksize(void);
extern int xhci_msc_read_blocks(u32 lba, void *dst, u32 count);
extern int xhci_msc_write_blocks(u32 lba, const void *src, u32 count);
extern int xhci_msc_sync_cache(void);
extern int xhci_msc_last_result(void);
extern int xhci_msc_sense_key(void);
extern int xhci_msc_sense_asc(void);
extern int xhci_msc_sense_ascq(void);
extern int xhci_msc_init_stage(void);
extern int xhci_msc_init_port(void);
extern int xhci_msc_init_slot(void);
extern int xhci_msc_init_cc(void);
extern int xhci_msc_init_vid(void);
extern int xhci_msc_init_pid(void);
extern u32 xhci_portsc(int port);
extern u32 xhci_usbsts(void);
extern u32 xhci_usbcmd(void);
extern void efi_persist_storage_diag(u32 reason, u32 result, u32 stage,
                                     u32 port, u32 slot, u32 cc, u32 portsc,
                                     u32 usbsts, u32 usbcmd, u32 vid, u32 pid,
                                     u32 ticks) __attribute__((weak));
extern void efi_persist_storage_ready(u32 ticks) __attribute__((weak));

#define ZLLOG_RECORD_BYTES 64u
#define ZLLOG_SUPER_BYTES  4096u
#define ZLLOG_SLOT_BYTES   2097152u
#define ZLLOG_PART_MIN_BYTES 67108864u
#define ZLLOG_PART_MAX_BYTES 536870912u
#define ZLLOG_SLOT_HEADER  4096u
#define ZLLOG_RAM_RECORDS  4096u
#define ZLLOG_NONE_SLOT    0xFFFFFFFFu

/* bytes_le form of a2bc51d4-225d-4ad4-8db5-b0095953aa19 */
static const u8 zllog_type_guid[16] = {
    0xd4,0x51,0xbc,0xa2, 0x5d,0x22, 0xd4,0x4a,
    0x8d,0xb5,0xb0,0x09,0x59,0x53,0xaa,0x19
};
static const u8 super_magic[8] = {'Z','L','L','O','G','V','1',0};
static const u8 commit_magic[8] = {'Z','L','C','O','M','M','I','T'};
static const u8 slot_magic[8] = {'Z','L','B','O','O','T','1',0};
static const u8 slot_commit[8] = {'Z','L','S','L','O','T','O','K'};

/* Records are byte arrays, not packed C structs.  The EFI build is LLP64 and
 * all four targets must put every field at the exact same byte offset. */
static u8 ram_records[ZLLOG_RAM_RECORDS][ZLLOG_RECORD_BYTES];
static u32 ram_head, ram_count, ram_dropped;
static u64 next_sequence = 1;
static u8 text_pending[12];
static u32 text_len;
static u32 text_stop_match;
static int text_capture = 1;
static int boot_started;
static u32 input_batch_sample, input_mouse_sample, input_last_mouse_code;

#define ZLLOG_COUNTERS ((u32)ZLLOG_COUNTER_LIMIT)
#define ZLLOG_SPANS 32u
#define ZLLOG_FRAME_HISTORY 8u
#define ZLLOG_IRQ_RECORDS 64u
static volatile u32 counter_total[ZLLOG_COUNTERS];
static volatile u32 counter_total_high[ZLLOG_COUNTERS];
static volatile u32 counter_samples[ZLLOG_COUNTERS];
static volatile u32 counter_samples_high[ZLLOG_COUNTERS];
static volatile u32 counter_max[ZLLOG_COUNTERS];
static u64 counter_emitted_total[ZLLOG_COUNTERS];
static u64 counter_emitted_samples[ZLLOG_COUNTERS];
static u32 last_counter_tick, reported_ram_dropped;
static u32 span_next;
static u32 operation_next;
static struct { u32 id, tick; u64 tsc; } spans[ZLLOG_SPANS];
static u32 burst_until_tick;
static u32 frame_seen, late_seen, frame_peak_total, frame_burst_tick;
static int recorder_io_depth;
struct frame_history {
    u32 seq, total_us, present_us, compositor_us;
    u32 damage_area, input_latency_us, input_sequence, present_bytes, flags;
};
static struct frame_history frame_history[ZLLOG_FRAME_HISTORY];
struct irq_record {
    volatile u32 valid;
    u32 subsystem, event, severity, a, b, c;
    u64 tsc;
};
static struct irq_record irq_records[ZLLOG_IRQ_RECORDS];
static volatile u32 irq_write;
static volatile u32 irq_read;

/* One DMA-sized staging page. xhci.c copies between this and its own fixed DMA
 * window, so zllog never owns or retains a driver buffer. */
static u8 io_page[ZLLOG_SUPER_BYTES];

struct super_info {
    u64 generation;
    u64 next_sequence;
    u64 completed_boots;
    u64 disk_blocks;
    u64 part_start;
    u64 part_blocks;
    u32 active_slot;
    u32 slot_count;
    u32 block_bytes;
    u32 record_bytes;
    u32 slot_bytes;
    u32 flags;
    u8 unique_guid[16];
    u8 image_sha256[32];
};

static struct super_info super;
static u32 disk_blocks, block_bytes, part_start, part_blocks;
static u32 part_bytes;
static int mounted;
static int storage_ready_persisted;
static int completing;
static u32 last_error;
static u32 last_flush_tick;
static u32 flush_failure_count, next_flush_retry_tick;
static u32 mount_retry_count, next_mount_retry_tick, last_refuse_signature;
static int mount_permanent_refusal;
static u32 current_super_offset;
static int slot_active;
static u32 slot_index, slot_record_count, slot_dropped_at_start;
static u64 slot_boot_id, slot_first_sequence, slot_last_sequence, slot_started_tsc;
static u64 session_boot_id;

static void atomic_counter_add(volatile u32 *low, volatile u32 *high, u32 delta)
{
    u32 old = __atomic_fetch_add(low, delta, __ATOMIC_RELAXED);
    if ((u32)(old + delta) < old)
        __atomic_fetch_add(high, 1u, __ATOMIC_RELAXED);
}

static u64 atomic_counter_read(volatile u32 *low, volatile u32 *high)
{
    /* A carry is published just after the low-word addition. If a snapshot
     * lands in that tiny window it may see the old high word and new low
     * word; counter_snapshot clamps against the last emitted value and the
     * following snapshot observes the carry. Totals therefore never go
     * backwards and no hot path needs a 64-bit runtime helper or lock. */
    u32 hi1, hi2, lo;
    do {
        hi1 = __atomic_load_n(high, __ATOMIC_ACQUIRE);
        lo = __atomic_load_n(low, __ATOMIC_RELAXED);
        hi2 = __atomic_load_n(high, __ATOMIC_ACQUIRE);
    } while (hi1 != hi2);
    return ((u64)hi2 << 32) | lo;
}

static void bytes_zero(void *vp, u32 n)
{
    u8 *p = (u8 *)vp;
    while (n--) *p++ = 0;
}

static void bytes_copy(void *dstp, const void *srcp, u32 n)
{
    u8 *d = (u8 *)dstp;
    const u8 *s = (const u8 *)srcp;
    while (n--) *d++ = *s++;
}

static int bytes_equal(const void *ap, const void *bp, u32 n)
{
    const u8 *a = (const u8 *)ap, *b = (const u8 *)bp;
    while (n--) if (*a++ != *b++) return 0;
    return 1;
}

static u16 get16(const u8 *p) { return (u16)((u16)p[0] | ((u16)p[1] << 8)); }
static u32 get32(const u8 *p)
{
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24);
}
static u64 get64(const u8 *p) { return (u64)get32(p) | ((u64)get32(p + 4) << 32); }
static void put16(u8 *p, u16 v) { p[0]=(u8)v; p[1]=(u8)(v>>8); }
static void put32(u8 *p, u32 v)
{
    p[0]=(u8)v; p[1]=(u8)(v>>8); p[2]=(u8)(v>>16); p[3]=(u8)(v>>24);
}
static void put64(u8 *p, u64 v) { put32(p,(u32)v); put32(p+4,(u32)(v>>32)); }

static u32 crc_step(u32 crc, const u8 *p, u32 n)
{
    while (n--) {
        crc ^= *p++;
        for (int i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
    }
    return crc;
}

static u32 crc32(const u8 *p, u32 n)
{
    return crc_step(0xFFFFFFFFu, p, n) ^ 0xFFFFFFFFu;
}

static void make_record(u8 *r, unsigned subsystem, unsigned event,
                        unsigned severity, u64 v0, u64 v1, u64 v2,
                        const u8 *payload, u32 payload_len)
{
    bytes_zero(r, ZLLOG_RECORD_BYTES);
    put64(r + 0, next_sequence++);
    put64(r + 8, cpu_tsc());
    put16(r + 16, 0);                 /* CPU 0 until SMP exposes current CPU */
    r[18] = (u8)severity;
    r[19] = (u8)subsystem;
    put16(r + 20, (u16)event);
    if (payload_len > 12) payload_len = 12;
    put16(r + 22, (u16)payload_len);
    put64(r + 24, v0); put64(r + 32, v1); put64(r + 40, v2);
    if (payload && payload_len) bytes_copy(r + 48, payload, payload_len);
    put32(r + 60, crc32(r, 60));
}

static void ring_append(const u8 *r)
{
    u32 event = get16(r + 20);
    u32 severity = r[18];
    int sampled = event == ZLLOG_EV_TEXT || event == ZLLOG_EV_INPUT_BATCH ||
                  event == ZLLOG_EV_INPUT_EVENT || event == ZLLOG_EV_POINTER ||
                  event == ZLLOG_EV_FRAME ||
                  event == ZLLOG_EV_FRAME_EXT || event == ZLLOG_EV_FRAME_PAINT ||
                  event == ZLLOG_EV_COUNTER ||
                  event == ZLLOG_EV_SPAN_BEGIN || event == ZLLOG_EV_SPAN_END;
    int critical = severity >= ZLLOG_WARN || event == ZLLOG_EV_FAULT ||
                   event == ZLLOG_EV_PANIC || event == ZLLOG_EV_TIMEOUT ||
                   event == ZLLOG_EV_FS_MUTATION || event == ZLLOG_EV_DROP ||
                   event == ZLLOG_EV_INPUT_DROP || event == ZLLOG_EV_FLUSH_ERROR ||
                   event == ZLLOG_EV_RECORDER_DROP;
    /* Admission reserves half the RAM lane for exact transitions and the last
     * 512 cells for warnings/faults. Under a syscall/output storm, samples are
     * discarded before they can evict the failure boundary explaining that
     * storm. Every refusal still increments the durable recorder-drop total. */
    u32 ceiling = critical ? ZLLOG_RAM_RECORDS :
                  (sampled ? 2048u : ZLLOG_RAM_RECORDS - 512u);
    if (ram_count >= ceiling) {
        ram_dropped++;
        atomic_counter_add(&counter_total[ZLLOG_C_RECORDER_OVERWRITE],
                           &counter_total_high[ZLLOG_C_RECORDER_OVERWRITE], 1u);
        return;
    }
    u32 at;
    if (ram_count < ZLLOG_RAM_RECORDS) {
        at = (ram_head + ram_count) % ZLLOG_RAM_RECORDS;
        ram_count++;
    } else {
        at = ram_head;
        ram_head = (ram_head + 1) % ZLLOG_RAM_RECORDS;
        ram_dropped++;
        atomic_counter_add(&counter_total[ZLLOG_C_RECORDER_OVERWRITE],
                           &counter_total_high[ZLLOG_C_RECORDER_OVERWRITE], 1u);
    }
    bytes_copy(ram_records[at], r, ZLLOG_RECORD_BYTES);
}

static void flush_text_to_ram(void)
{
    if (!text_len) return;
    u8 r[ZLLOG_RECORD_BYTES];
    make_record(r, ZLLOG_SUB_KERNEL, ZLLOG_EV_TEXT, ZLLOG_INFO,
                text_len, 0, 0, text_pending, text_len);
    ring_append(r);
    text_len = 0;
}

static void ensure_boot_start(void)
{
    if (boot_started) return;
    boot_started = 1;
    u8 r[ZLLOG_RECORD_BYTES];
    make_record(r, ZLLOG_SUB_BOOT, ZLLOG_EV_BOOT_START, ZLLOG_INFO, 1, 0, 0, 0, 0);
    ring_append(r);
}

void zllog_putc(char c)
{
    static const char stop[] = "system ready\n";
    if (!text_capture) return;
    ensure_boot_start();
    text_pending[text_len++] = (u8)c;
    if (text_len == sizeof text_pending || c == '\n') flush_text_to_ram();
    if ((u8)c == (u8)stop[text_stop_match]) text_stop_match++;
    else text_stop_match = ((u8)c == (u8)stop[0]) ? 1u : 0u;
    if (text_stop_match == sizeof stop - 1u) {
        flush_text_to_ram();
        text_capture = 0;
    }
}

void zllog_event(unsigned subsystem, unsigned event, unsigned severity,
                 unsigned a, unsigned b, unsigned c)
{
    ensure_boot_start();
    flush_text_to_ram();
    u8 r[ZLLOG_RECORD_BYTES];
    make_record(r, subsystem, event, severity, a, b, c, 0, 0);
    ring_append(r);
}

void zllog_event_irq(unsigned subsystem, unsigned event, unsigned severity,
                     unsigned a, unsigned b, unsigned c)
{
    /* Never touch the normal record ring from an ISR: it can interrupt a
     * normal append halfway through. This reusable bounded lane is drained
     * in normal context. A fatal exception still cannot safely issue USB
     * commands, so its last boundary remains RAM-only until a later drain. */
    u32 at;
    for (;;) {
        u32 write = __atomic_load_n(&irq_write, __ATOMIC_RELAXED);
        u32 read = __atomic_load_n(&irq_read, __ATOMIC_ACQUIRE);
        if (write - read >= ZLLOG_IRQ_RECORDS) {
            atomic_counter_add(&counter_total[ZLLOG_C_RECORDER_OVERWRITE],
                               &counter_total_high[ZLLOG_C_RECORDER_OVERWRITE], 1u);
            return;
        }
        u32 next = write + 1u;
        if (__atomic_compare_exchange_n(&irq_write, &write, next, 1,
                                        __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)) {
            at = write;
            break;
        }
    }
    struct irq_record *q = &irq_records[at % ZLLOG_IRQ_RECORDS];
    q->subsystem = subsystem; q->event = event; q->severity = severity;
    q->a = a; q->b = b; q->c = c; q->tsc = cpu_tsc();
    __atomic_store_n(&q->valid, at + 1u, __ATOMIC_RELEASE);
}

static void drain_irq_records(void)
{
    u32 read = __atomic_load_n(&irq_read, __ATOMIC_RELAXED);
    u32 end = __atomic_load_n(&irq_write, __ATOMIC_ACQUIRE);
    while (read != end) {
        struct irq_record *q = &irq_records[read % ZLLOG_IRQ_RECORDS];
        if (__atomic_load_n(&q->valid, __ATOMIC_ACQUIRE) != read + 1u) break;
        ensure_boot_start();
        flush_text_to_ram();
        u8 r[ZLLOG_RECORD_BYTES];
        make_record(r, q->subsystem, q->event, q->severity,
                    q->a, q->b, q->c, 0, 0);
        put64(r + 8, q->tsc);
        put32(r + 60, crc32(r, 60));
        ring_append(r);
        __atomic_store_n(&q->valid, 0u, __ATOMIC_RELEASE);
        read++;
        __atomic_store_n(&irq_read, read, __ATOMIC_RELEASE);
    }
}

void zllog_counter_add(unsigned counter, unsigned delta)
{
    if (!counter || counter >= ZLLOG_COUNTERS || !delta) return;
    atomic_counter_add(&counter_total[counter], &counter_total_high[counter], delta);
}

void zllog_counter_observe(unsigned counter, unsigned value)
{
    if (!counter || counter >= ZLLOG_COUNTERS) return;
    atomic_counter_add(&counter_total[counter], &counter_total_high[counter], value);
    atomic_counter_add(&counter_samples[counter], &counter_samples_high[counter], 1u);
    u32 old = __atomic_load_n(&counter_max[counter], __ATOMIC_RELAXED);
    while (value > old &&
           !__atomic_compare_exchange_n(&counter_max[counter], &old, value, 1,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED)) { }
}

static void counter_snapshot(int force)
{
    drain_irq_records();
    u32 now = idt_ticks();
    if (!force && now - last_counter_tick < 100u) return; /* once/second */
    last_counter_tick = now;
    for (u32 id = 1; id < ZLLOG_COUNTERS; id++) {
        u64 total = atomic_counter_read(&counter_total[id], &counter_total_high[id]);
        u64 samples = atomic_counter_read(&counter_samples[id], &counter_samples_high[id]);
        if (total < counter_emitted_total[id]) total = counter_emitted_total[id];
        if (samples < counter_emitted_samples[id]) samples = counter_emitted_samples[id];
        if (!force && total == counter_emitted_total[id] &&
            samples == counter_emitted_samples[id]) continue;
        if (!total && !samples) continue;
        u8 r[ZLLOG_RECORD_BYTES];
        make_record(r, ZLLOG_SUB_PERF, ZLLOG_EV_COUNTER, ZLLOG_INFO,
                    id, total, __atomic_load_n(&counter_max[id], __ATOMIC_RELAXED),
                    0, 0);
        put32(r + 48, (u32)samples);
        put32(r + 52, (u32)(total - counter_emitted_total[id]));
        put32(r + 56, (u32)(samples - counter_emitted_samples[id]));
        put16(r + 22, 12);
        put32(r + 60, crc32(r, 60));
        ring_append(r);
        counter_emitted_total[id] = total;
        counter_emitted_samples[id] = samples;
    }
}

unsigned zllog_span_begin(unsigned subsystem, unsigned operation,
                          unsigned detail)
{
    u32 id = ++span_next;
    if (!id) id = ++span_next;
    u32 at = id % ZLLOG_SPANS;
    spans[at].id = id;
    spans[at].tick = idt_ticks();
    spans[at].tsc = cpu_tsc();
    zllog_event(subsystem, ZLLOG_EV_SPAN_BEGIN, ZLLOG_INFO,
                id, operation, detail);
    return id;
}

void zllog_span_end(unsigned subsystem, unsigned span, unsigned status)
{
    u32 at = span % ZLLOG_SPANS;
    u32 us = 0;
    if (span && spans[at].id == span) {
        u64 delta = cpu_tsc() - spans[at].tsc;
        u32 khz = cpu_tsc_khz();
        u32 cycles_us = khz / 1000u;
        if (cycles_us && delta <= 0xffffffffu) us = (u32)delta / cycles_us;
        else {
            u32 ticks = idt_ticks() - spans[at].tick;
            us = ticks > 429496u ? 0xffffffffu : ticks * 10000u;
        }
        spans[at].id = 0;
    }
    zllog_event(subsystem, ZLLOG_EV_SPAN_END,
                status ? ZLLOG_WARN : ZLLOG_INFO, span, status, us);
}

void zllog_trigger(unsigned subsystem, unsigned event, unsigned severity,
                   unsigned a, unsigned b, unsigned c)
{
    zllog_event(subsystem, event, severity, a, b, c);
    burst_until_tick = idt_ticks() + 100u; /* one second of detailed producers */
    zllog_event(subsystem, ZLLOG_EV_TRIGGER, severity, event, a, b);
}

void zllog_lifecycle(unsigned subsystem, unsigned object_kind,
                     unsigned object_id, unsigned action,
                     unsigned parent_id, unsigned detail)
{
    ensure_boot_start();
    flush_text_to_ram();
    u8 r[ZLLOG_RECORD_BYTES];
    make_record(r, subsystem, ZLLOG_EV_LIFECYCLE,
                action == ZLLOG_LIFE_FAULT || action == ZLLOG_LIFE_REFUSED
                    ? ZLLOG_WARN : ZLLOG_INFO,
                (u64)object_id | ((u64)object_kind << 32),
                (u64)parent_id | ((u64)action << 32), detail, 0, 0);
    ring_append(r);
}

unsigned zllog_operation_begin(unsigned subsystem, unsigned actor_kind,
                               unsigned actor_id, unsigned operation,
                               unsigned object_id)
{
    u32 id = ++operation_next;
    if (!id) id = ++operation_next;
    ensure_boot_start();
    flush_text_to_ram();
    u8 r[ZLLOG_RECORD_BYTES];
    make_record(r, subsystem, ZLLOG_EV_OPERATION_BEGIN, ZLLOG_INFO,
                id, (u64)actor_id | ((u64)actor_kind << 32),
                (u64)object_id | ((u64)operation << 32), 0, 0);
    ring_append(r);
    return id;
}

void zllog_operation_result(unsigned subsystem, unsigned operation_id,
                            unsigned operation, int result, unsigned error,
                            unsigned detail)
{
    if (!operation_id) return;
    ensure_boot_start();
    flush_text_to_ram();
    u8 r[ZLLOG_RECORD_BYTES];
    u8 payload[4];
    put32(payload, detail);
    make_record(r, subsystem, ZLLOG_EV_OPERATION_RESULT,
                error || result < 0 ? ZLLOG_WARN : ZLLOG_INFO,
                operation_id, (u64)(long long)result,
                (u64)operation | ((u64)error << 32), payload, 4);
    ring_append(r);
}

int zllog_burst_active(void)
{
    return (u32)(burst_until_tick - idt_ticks()) < 0x80000000u;
}

int zllog_io_active(void) { return recorder_io_depth != 0; }

void zllog_input_batch(unsigned processed, unsigned depth, unsigned drops)
{
    /* Continuous pointer motion can produce one active batch per frame. Keep
     * every drop, but only one ordinary batch in sixteen: enough to show queue
     * pressure/cadence for hours instead of filling a 2 MiB boot slot in a few
     * minutes and becoming the performance bug under investigation. */
    if (drops || (processed && (zllog_burst_active() ||
                                (++input_batch_sample & 15u) == 0)))
        zllog_event(ZLLOG_SUB_INPUT, drops ? ZLLOG_EV_INPUT_DROP : ZLLOG_EV_INPUT_BATCH,
                    drops ? ZLLOG_WARN : ZLLOG_INFO, processed, depth, drops);
}

void zllog_input_event(unsigned type, unsigned code, unsigned depth)
{
    /* Event kind/timing remain exact, but printable key identity is private:
     * the flight recorder must never become a command/key logger. Buttons
     * and wheels remain exact. Ordinary pointer movement is sampled unless a
     * trigger opened the one-second detailed window. */
    if (type == 3u || ((type == 1u || type == 2u) && code >= 32u && code <= 126u))
        code = 0;
    if (type == 4u) {
        int button_edge = code != input_last_mouse_code;
        input_last_mouse_code = code;
        if (!button_edge && !zllog_burst_active() &&
            (++input_mouse_sample & 15u) != 0) return;
    }
    zllog_event(ZLLOG_SUB_INPUT, ZLLOG_EV_INPUT_EVENT, ZLLOG_INFO, type, code, depth);
}

void zllog_pointer_event(unsigned x, unsigned y, unsigned buttons,
                         unsigned depth, unsigned source_tsc,
                         unsigned sequence)
{
    int edge = buttons != input_last_mouse_code;
    input_last_mouse_code = buttons;
    if (!edge && !zllog_burst_active() && (++input_mouse_sample & 15u) != 0) return;
    ensure_boot_start();
    flush_text_to_ram();
    u8 r[ZLLOG_RECORD_BYTES];
    make_record(r, ZLLOG_SUB_INPUT, ZLLOG_EV_POINTER, ZLLOG_INFO,
                (u64)x | ((u64)y << 32),
                (u64)buttons | ((u64)depth << 32),
                (u64)source_tsc | ((u64)sequence << 32), 0, 0);
    ring_append(r);
}

void zllog_frame(unsigned input_us, unsigned tick_us, unsigned compositor_us,
                 unsigned vblank_us, unsigned present_us, unsigned total_us,
                 unsigned flags, unsigned damage_count, unsigned damage_area)
{
    ensure_boot_start();
    flush_text_to_ram();
    u8 payload[12];
    put32(payload + 0, vblank_us);
    put32(payload + 4, present_us);
    put32(payload + 8, total_us);
    u8 r[ZLLOG_RECORD_BYTES];
    make_record(r, ZLLOG_SUB_FRAME,
                (flags & 1u) ? ZLLOG_EV_FRAME_LATE : ZLLOG_EV_FRAME,
                (flags & 1u) ? ZLLOG_WARN : ZLLOG_INFO,
                input_us, tick_us, compositor_us, payload, sizeof payload);
    /* The low 32 bits remain compositor_us for straightforward extraction.
     * The high word packs 3 stable flags, the 0..8 damage-rectangle count and
     * a 25-bit pixel area (enough for 8K). Existing readers which mask the
     * low flag bits remain compatible. */
    if (damage_count > 15u) damage_count = 15u;
    if (damage_area > 0x01FFFFFFu) damage_area = 0x01FFFFFFu;
    u32 meta = (flags & 7u) | (damage_count << 3) | (damage_area << 7);
    put64(r + 40, (u64)compositor_us | ((u64)meta << 32));
    put32(r + 60, crc32(r, 60));
    ring_append(r);
}

static void frame_burst_records(void)
{
    u32 have = frame_seen < ZLLOG_FRAME_HISTORY ? frame_seen : ZLLOG_FRAME_HISTORY;
    for (u32 n = have; n > 0; n--) {
        const struct frame_history *h =
            &frame_history[(frame_seen - n) % ZLLOG_FRAME_HISTORY];
        u8 payload[12];
        put32(payload + 0, h->damage_area);
        put32(payload + 4, h->present_bytes);
        put32(payload + 8, h->flags);
        u8 r[ZLLOG_RECORD_BYTES];
        make_record(r, ZLLOG_SUB_DISPLAY, ZLLOG_EV_FRAME_BURST, ZLLOG_WARN,
                    (u64)h->seq | ((u64)h->input_sequence << 32),
                    (u64)h->total_us | ((u64)h->present_us << 32),
                    (u64)h->compositor_us | ((u64)h->input_latency_us << 32),
                    payload, sizeof payload);
        ring_append(r);
    }
}

void zllog_frame_observe(unsigned input_us, unsigned tick_us,
                         unsigned compositor_us, unsigned vblank_us,
                         unsigned present_us, unsigned total_us,
                         unsigned flags, unsigned damage_count,
                         unsigned damage_area, unsigned input_to_present_us,
                         unsigned input_sequence, unsigned missed_deadlines,
                         unsigned queue_depth, unsigned present_bytes,
                         unsigned desk_us, unsigned chrome_us,
                         unsigned app_us, unsigned effects_us,
                         unsigned repaint_rects, unsigned repaint_pixels,
                         unsigned window_visits, unsigned app_calls)
{
    int cursor_only = (flags & (1u << 4)) != 0;
    u32 seq = frame_seen++;
    int late = (flags & 1u) != 0;
    int healthy_sample = !late && (seq % 60u) == 0;
    int burst_active = zllog_burst_active();
    int new_peak = total_us > frame_peak_total;
    int severe = total_us >= 100000u || present_us >= 100000u;
    if (new_peak) frame_peak_total = total_us;
    if (late) late_seen++;

    zllog_counter_add(ZLLOG_C_FRAME_PAINT, 1);
    if (late) zllog_counter_add(ZLLOG_C_FRAME_LATE, 1);
    if (missed_deadlines) zllog_counter_add(ZLLOG_C_FRAME_LOST, missed_deadlines);
    zllog_counter_add(ZLLOG_C_RENDERED_PIXELS, damage_area);
    zllog_counter_add(ZLLOG_C_PRESENT_BYTES, present_bytes);
    zllog_counter_observe(ZLLOG_C_INPUT_QUEUE, queue_depth);

    struct frame_history *h = &frame_history[seq % ZLLOG_FRAME_HISTORY];
    h->seq = seq; h->total_us = total_us; h->present_us = present_us;
    h->compositor_us = compositor_us; h->damage_area = damage_area;
    h->input_latency_us = input_to_present_us; h->input_sequence = input_sequence;
    h->present_bytes = present_bytes;
    h->flags = flags | (severe ? 8u : 0u);

    /* Sustained slowness is represented by an exact counter plus one detailed
     * frame in four. The first eight, new peaks, severe frames and active
     * triggered windows remain exact. This keeps hours of evidence instead of
     * filling one boot slot with the same 70 ms frame in minutes. */
    int keep = burst_active || healthy_sample || (late && (late_seen <= 8u ||
               (late_seen & 3u) == 0u || new_peak || severe ||
               burst_active));
    if (keep) {
        unsigned out_flags = flags | (healthy_sample ? 2u : 0u);
        zllog_frame(input_us, tick_us, compositor_us, vblank_us, present_us,
                    total_us, out_flags, damage_count, damage_area);
        u8 payload[12];
        put32(payload + 0, missed_deadlines);
        put32(payload + 4, queue_depth);
        put32(payload + 8, present_bytes |
              (cursor_only && present_bytes < 0x80000000u ? 0x80000000u : 0u));
        u8 r[ZLLOG_RECORD_BYTES];
        make_record(r, ZLLOG_SUB_DISPLAY, ZLLOG_EV_FRAME_EXT,
                    late ? ZLLOG_WARN : ZLLOG_INFO,
                    (u64)seq | ((u64)input_sequence << 32), input_to_present_us,
                    (u64)damage_area | ((u64)present_bytes << 32),
                    payload, sizeof payload);
        ring_append(r);

        put32(payload + 0, repaint_pixels);
        put32(payload + 4, window_visits);
        put32(payload + 8, app_calls);
        make_record(r, ZLLOG_SUB_DISPLAY, ZLLOG_EV_FRAME_PAINT,
                    late ? ZLLOG_WARN : ZLLOG_INFO,
                    (u64)seq | ((u64)repaint_rects << 32),
                    (u64)desk_us | ((u64)chrome_us << 32),
                    (u64)app_us | ((u64)effects_us << 32),
                    payload, sizeof payload);
        ring_append(r);
    }

    u32 now = idt_ticks();
    if (late && (!frame_burst_tick || now - frame_burst_tick >= 1000u)) {
        /* Persist the eight frames leading into the first late boundary, and
         * refresh that context at most once per ten seconds while it lasts. */
        frame_burst_tick = now;
        zllog_trigger(ZLLOG_SUB_DISPLAY, ZLLOG_EV_BURST, ZLLOG_WARN,
                      seq, total_us, present_us);
        frame_burst_records();
    }
}

void zllog_milestone(unsigned id, unsigned value)
{
    zllog_event(ZLLOG_SUB_BOOT, ZLLOG_EV_BOOT_MILESTONE, ZLLOG_INFO, id, value, 0);
}

static int exact_label(const u8 *entry)
{
    static const u8 name[10] = {'Z',0,'L',0,'L',0,'O',0,'G',0};
    if (!bytes_equal(entry + 56, name, sizeof name)) return 0;
    for (u32 i = 56 + sizeof name; i < 128; i++)
        if (entry[i]) return 0;
    return 1;
}

static int read_one(u32 lba)
{
    bytes_zero(io_page, sizeof io_page);
    return xhci_msc_read_blocks(lba, io_page, 1);
}

/* Validate the primary GPT and locate exactly one dedicated journal entry.
 * Its entry-array CRC is checked before the match is trusted. */
static int find_partition(u8 unique_guid[16])
{
    if (!read_one(1)) return 0;
    if (!bytes_equal(io_page, "EFI PART", 8)) return 0;
    u32 header_bytes = get32(io_page + 12);
    if (header_bytes < 92 || header_bytes > block_bytes) return 0;
    if (get64(io_page + 24) != 1) return 0;
    u64 alternate = get64(io_page + 32);
    if (!alternate || alternate >= disk_blocks) return 0;
    u32 saved_crc = get32(io_page + 16);
    put32(io_page + 16, 0);
    u32 actual_crc = crc32(io_page, header_bytes);
    put32(io_page + 16, saved_crc);
    if (actual_crc != saved_crc) return 0;

    u64 entries64 = get64(io_page + 72);
    u32 nentries = get32(io_page + 80);
    u32 entry_bytes = get32(io_page + 84);
    u32 entries_crc = get32(io_page + 88);
    if (!entries64 || entries64 > 0xFFFFFFFFu) return 0;
    if (!nentries || nentries > 256 || entry_bytes != 128) return 0;
    if (block_bytes < 512 || block_bytes > sizeof io_page || block_bytes % 128) return 0;

    u32 entries_lba = (u32)entries64;
    u32 total_bytes = nentries * entry_bytes;
    u32 total_blocks = (total_bytes + block_bytes - 1) / block_bytes;
    if (entries_lba >= disk_blocks || total_blocks > disk_blocks - entries_lba) return 0;

    u32 crc = 0xFFFFFFFFu, done = 0, seen = 0;
    u64 found_first = 0, found_last = 0;
    u8 found_guid[16];
    for (u32 b = 0; b < total_blocks; b++) {
        if (!read_one(entries_lba + b)) return 0;
        u32 take = total_bytes - done;
        if (take > block_bytes) take = block_bytes;
        crc = crc_step(crc, io_page, take);

        u32 entries_here = take / entry_bytes;
        for (u32 e = 0; e < entries_here; e++) {
            const u8 *p = io_page + e * entry_bytes;
            if (bytes_equal(p, zllog_type_guid, 16) && exact_label(p)) {
                seen++;
                found_first = get64(p + 32);
                found_last = get64(p + 40);
                bytes_copy(found_guid, p + 16, 16);
            }
        }
        done += take;
    }
    crc ^= 0xFFFFFFFFu;
    if (crc != entries_crc || seen != 1) return 0;
    if (!found_first || found_last < found_first) return 0;
    if (found_first > 0xFFFFFFFFu || found_last > 0xFFFFFFFFu) return 0;
    u32 first = (u32)found_first, last = (u32)found_last;
    if (last >= disk_blocks || last >= (u32)alternate) return 0;
    u32 want_blocks = last - first + 1u;
    u64 bytes = (u64)want_blocks * block_bytes;
    if (bytes < ZLLOG_PART_MIN_BYTES || bytes > ZLLOG_PART_MAX_BYTES ||
        bytes > 0xffffffffu) return 0;

    part_start = first;
    part_blocks = want_blocks;
    part_bytes = (u32)bytes;
    bytes_copy(unique_guid, found_guid, 16);
    return 1;
}

static int super_valid(u32 byte_offset, const u8 unique_guid[16], struct super_info *out)
{
    if (byte_offset % block_bytes) return 0;
    u32 blocks = ZLLOG_SUPER_BYTES / block_bytes;
    if (!blocks || blocks * block_bytes != ZLLOG_SUPER_BYTES) return 0;
    if (!xhci_msc_read_blocks(part_start + byte_offset / block_bytes, io_page, blocks)) return 0;
    if (!bytes_equal(io_page + 0, super_magic, 8) ||
        get16(io_page + 8) != 1 || get16(io_page + 10) != 164 ||
        get32(io_page + 12) != ZLLOG_SUPER_BYTES ||
        !bytes_equal(io_page + 156, commit_magic, 8)) return 0;
    u32 saved = get32(io_page + 152);
    put32(io_page + 152, 0);
    u32 actual = crc32(io_page, ZLLOG_SUPER_BYTES);
    put32(io_page + 152, saved);
    if (saved != actual) return 0;
    if (get64(io_page + 48) != disk_blocks ||
        get64(io_page + 56) != part_start || get64(io_page + 64) != part_blocks ||
        get32(io_page + 72) != block_bytes ||
        get32(io_page + 76) != ZLLOG_RECORD_BYTES ||
        get32(io_page + 80) != ZLLOG_SLOT_BYTES ||
        !bytes_equal(io_page + 88, zllog_type_guid, 16) ||
        !bytes_equal(io_page + 104, unique_guid, 16)) return 0;
    u32 slots = get32(io_page + 28);
    if (!slots || slots != (part_bytes - 8192u) / ZLLOG_SLOT_BYTES) return 0;
    u32 active = get32(io_page + 24);
    if (active != ZLLOG_NONE_SLOT && active >= slots) return 0;

    out->generation = get64(io_page + 16);
    out->active_slot = active;
    out->slot_count = slots;
    out->next_sequence = get64(io_page + 32);
    out->completed_boots = get64(io_page + 40);
    out->disk_blocks = get64(io_page + 48);
    out->part_start = get64(io_page + 56);
    out->part_blocks = get64(io_page + 64);
    out->block_bytes = get32(io_page + 72);
    out->record_bytes = get32(io_page + 76);
    out->slot_bytes = get32(io_page + 80);
    out->flags = get32(io_page + 84);
    bytes_copy(out->unique_guid, io_page + 104, 16);
    bytes_copy(out->image_sha256, io_page + 120, 32);
    return 1;
}

enum {
    ZLE_NONE = 0, ZLE_MSC_INIT = 1, ZLE_CAPACITY = 2, ZLE_BLOCK_SIZE = 3,
    ZLE_GPT = 4, ZLE_SUPER = 5, ZLE_WRITE_BOUNDS = 6, ZLE_WRITE = 7,
    ZLE_SYNC = 8, ZLE_SLOT_FULL = 9
};

static int refuse(u32 reason)
{
    last_error = reason;
    mounted = 0;
    u32 detail = ((u32)xhci_msc_init_stage() & 0xFFu) |
                 (((u32)xhci_msc_init_port() & 0xFFu) << 8) |
                 (((u32)xhci_msc_init_slot() & 0xFFu) << 16) |
                 (((u32)xhci_msc_init_cc() & 0xFFu) << 24);
    u32 signature = reason ^ (detail * 0x9E3779B1u) ^
                    ((u32)xhci_msc_last_result() << 16);
    /* Automatic retries must not become the recorder workload. Retain the
     * first refusal and every changed failure boundary, not one identical
     * record per retry forever. */
    if (!last_refuse_signature || signature != last_refuse_signature) {
        zllog_event(ZLLOG_SUB_STORAGE, ZLLOG_EV_STORAGE_REFUSED, ZLLOG_WARN,
                    reason, (unsigned)xhci_msc_last_result(), detail);
        int port = xhci_msc_init_port();
        zllog_event(ZLLOG_SUB_STORAGE, ZLLOG_EV_STORAGE_XHCI, ZLLOG_WARN,
                    port > 0 ? xhci_portsc(port) : 0u,
                    xhci_usbsts(), xhci_usbcmd());
        if (efi_persist_storage_diag)
            efi_persist_storage_diag(
                reason, (u32)xhci_msc_last_result(),
                (u32)xhci_msc_init_stage(), (u32)port,
                (u32)xhci_msc_init_slot(), (u32)xhci_msc_init_cc(),
                port > 0 ? xhci_portsc(port) : 0u,
                xhci_usbsts(), xhci_usbcmd(),
                (u32)xhci_msc_init_vid(), (u32)xhci_msc_init_pid(),
                idt_ticks());
        last_refuse_signature = signature ? signature : 1u;
    }
    /* Retry only failures that can change while the machine is running. A
     * wrong GPT identity, unsupported block size or invalid journal format is
     * permanent for this boot. Retrying those forever both wastes USB work
     * and can replace the useful identity error with a later transport error. */
    if (reason == ZLE_MSC_INIT || reason == ZLE_CAPACITY ||
        reason == ZLE_WRITE || reason == ZLE_SYNC) {
        mount_retry_count++;
        u32 delay = mount_retry_count == 1 ? 100u :
                    mount_retry_count == 2 ? 500u :
                    mount_retry_count == 3 ? 3000u : 6000u;
        next_mount_retry_tick = idt_ticks() + delay;
    } else {
        next_mount_retry_tick = 0;
        mount_permanent_refusal = 1;
    }
    return 0;
}

/* Early records exist before storage tells us the global next sequence. On a
 * second boot their provisional 1..N numbers would otherwise duplicate the
 * previous boot. Rebase the buffered prefix after selecting the durable
 * superblock, then repair each independent record CRC. */
static void rebase_ram_sequences(u64 first)
{
    if (!first) first = 1;
    for (u32 i = 0; i < ram_count; i++) {
        u32 at = (ram_head + i) % ZLLOG_RAM_RECORDS;
        put64(ram_records[at] + 0, first + i);
        put32(ram_records[at] + 60, crc32(ram_records[at], 60));
    }
    next_sequence = first + ram_count;
}

int zllog_mount(void)
{
    if (mounted) return 1;
    if (mount_permanent_refusal) return 0;
    if (!xhci_msc_init()) return refuse(ZLE_MSC_INIT);
    if (!xhci_msc_read_capacity()) return refuse(ZLE_CAPACITY);
    disk_blocks = xhci_msc_blocks();
    block_bytes = xhci_msc_blocksize();
    if (!disk_blocks || (block_bytes != 512 && block_bytes != 4096))
        return refuse(ZLE_BLOCK_SIZE);
    u8 unique_guid[16];
    if (!find_partition(unique_guid)) return refuse(ZLE_GPT);

    struct super_info a, b;
    int va = super_valid(0, unique_guid, &a);
    int vb = super_valid(ZLLOG_SUPER_BYTES, unique_guid, &b);
    if (!va && !vb) return refuse(ZLE_SUPER);
    int use_a = !vb || (va && a.generation >= b.generation);
    super = use_a ? a : b;
    current_super_offset = use_a ? 0u : ZLLOG_SUPER_BYTES;
    session_boot_id = super.generation + 1u;
    u64 durable_next = super.next_sequence ? super.next_sequence : 1;
    /* A forced-power-off boot leaves the superblock pointing at a WRITING
     * slot.  Its durable checkpoint is newer than super.next_sequence, so use
     * it to keep sequence numbers monotonic before selecting the next slot. */
    if (super.active_slot != ZLLOG_NONE_SLOT) {
        u32 off = 8192u + super.active_slot * ZLLOG_SLOT_BYTES;
        u32 nblocks = ZLLOG_SLOT_HEADER / block_bytes;
        if (nblocks && xhci_msc_read_blocks(part_start + off / block_bytes,
                                             io_page, nblocks) &&
            bytes_equal(io_page, slot_magic, 8) &&
            get16(io_page + 8) == 1 && get16(io_page + 10) == 132 &&
            get32(io_page + 20) == super.active_slot &&
            bytes_equal(io_page + 124, slot_commit, 8)) {
            u32 saved = get32(io_page + 120);
            put32(io_page + 120, 0);
            u32 actual = crc32(io_page, ZLLOG_SLOT_HEADER);
            put32(io_page + 120, saved);
            u64 last = get64(io_page + 40);
            if (saved == actual && last >= durable_next) durable_next = last + 1;
        }
    }
    flush_text_to_ram();
    rebase_ram_sequences(durable_next);
    mounted = 1;
    mount_retry_count = 0;
    next_mount_retry_tick = 0;
    last_refuse_signature = 0;
    mount_permanent_refusal = 0;
    last_error = ZLE_NONE;
    counter_snapshot(1);
    zllog_event(ZLLOG_SUB_STORAGE, ZLLOG_EV_STORAGE_READY, ZLLOG_INFO,
                part_start, part_blocks, block_bytes);
    zllog_event(ZLLOG_SUB_LAB, ZLLOG_EV_DRIVER_STATE, ZLLOG_INFO,
                1u /* Intel PT capability */, cpu_has_intel_pt ?
                (unsigned)cpu_has_intel_pt() : 0u, 0u /* disabled in normal boots */);
    /* Slot creation and the first drain happen in zllog_flush(). Do not
     * replace the firmware fallback with a success claim until that first
     * durable checkpoint has actually completed. The physical v6 run exposed
     * why this matters: the first scan refused, the bounded retry mounted a
     * few seconds later, but NVRAM kept reporting the obsolete refusal. */
    return zllog_flush();
}

int zllog_ready(void) { return mounted && last_error == ZLE_NONE; }
unsigned zllog_buffered(void)
{
    return ram_count + (text_len ? 1u : 0u) +
           (__atomic_load_n(&irq_write, __ATOMIC_ACQUIRE) -
            __atomic_load_n(&irq_read, __ATOMIC_ACQUIRE));
}
unsigned zllog_dropped(void) { return ram_dropped; }
unsigned zllog_last_error(void) { return last_error; }

static int range_ok(u32 byte_offset, u32 bytes)
{
    if (!bytes || byte_offset % block_bytes || bytes % block_bytes) return 0;
    if (byte_offset >= part_bytes || bytes > part_bytes - byte_offset) return 0;
    u32 lba = part_start + byte_offset / block_bytes;
    u32 blocks = bytes / block_bytes;
    if (lba < part_start || blocks > part_blocks || lba - part_start > part_blocks - blocks)
        return 0;
    return 1;
}

static int write_part(u32 byte_offset, const void *src, u32 bytes)
{
    if (bytes > sizeof io_page || !range_ok(byte_offset, bytes)) {
        last_error = ZLE_WRITE_BOUNDS;
        return 0;
    }
    if (!xhci_msc_write_blocks(part_start + byte_offset / block_bytes,
                               src, bytes / block_bytes)) {
        last_error = ZLE_WRITE;
        return 0;
    }
    zllog_counter_add(ZLLOG_C_USB_WRITE_BYTES, bytes);
    return 1;
}

static void build_super_page(void)
{
    bytes_zero(io_page, sizeof io_page);
    bytes_copy(io_page + 0, super_magic, 8);
    put16(io_page + 8, 1); put16(io_page + 10, 164);
    put32(io_page + 12, ZLLOG_SUPER_BYTES);
    put64(io_page + 16, super.generation);
    put32(io_page + 24, super.active_slot); put32(io_page + 28, super.slot_count);
    put64(io_page + 32, super.next_sequence);
    put64(io_page + 40, super.completed_boots);
    put64(io_page + 48, super.disk_blocks);
    put64(io_page + 56, super.part_start); put64(io_page + 64, super.part_blocks);
    put32(io_page + 72, super.block_bytes); put32(io_page + 76, ZLLOG_RECORD_BYTES);
    put32(io_page + 80, ZLLOG_SLOT_BYTES); put32(io_page + 84, super.flags);
    bytes_copy(io_page + 88, zllog_type_guid, 16);
    bytes_copy(io_page + 104, super.unique_guid, 16);
    bytes_copy(io_page + 120, super.image_sha256, 32);
    bytes_copy(io_page + 156, commit_magic, 8);
    put32(io_page + 152, crc32(io_page, ZLLOG_SUPER_BYTES));
}

static int publish_super(void)
{
    u32 target = current_super_offset ? 0u : ZLLOG_SUPER_BYTES;
    build_super_page();
    if (!write_part(target, io_page, ZLLOG_SUPER_BYTES)) return 0;
    if (!xhci_msc_sync_cache()) { last_error = ZLE_SYNC; return 0; }
    current_super_offset = target;
    return 1;
}

static void build_slot_page(u32 state, u64 ended_tsc)
{
    bytes_zero(io_page, sizeof io_page);
    bytes_copy(io_page + 0, slot_magic, 8);
    put16(io_page + 8, 1); put16(io_page + 10, 132);
    put32(io_page + 12, ZLLOG_SLOT_BYTES);
    put32(io_page + 16, state); put32(io_page + 20, slot_index);
    put64(io_page + 24, slot_boot_id);
    put64(io_page + 32, slot_first_sequence);
    put64(io_page + 40, slot_last_sequence);
    put32(io_page + 48, slot_record_count);
    put32(io_page + 52, (ZLLOG_SLOT_BYTES - ZLLOG_SLOT_HEADER) / ZLLOG_RECORD_BYTES);
    put32(io_page + 56, ram_dropped - slot_dropped_at_start);
    put32(io_page + 60, 0);             /* boot path is added by a milestone */
    put32(io_page + 64, 0); put32(io_page + 68, 0);
    put64(io_page + 72, slot_started_tsc); put64(io_page + 80, ended_tsc);
    bytes_copy(io_page + 88, super.image_sha256, 32);
    bytes_copy(io_page + 124, slot_commit, 8);
    put32(io_page + 120, crc32(io_page, ZLLOG_SLOT_HEADER));
}

static u32 slot_byte_offset(void)
{
    return 8192u + slot_index * ZLLOG_SLOT_BYTES;
}

static int write_slot_header(u32 state, u64 ended_tsc)
{
    build_slot_page(state, ended_tsc);
    if (!write_part(slot_byte_offset(), io_page, ZLLOG_SLOT_HEADER)) return 0;
    if (!xhci_msc_sync_cache()) { last_error = ZLE_SYNC; return 0; }
    return 1;
}

static int start_slot(void)
{
    u32 previous = super.active_slot;
    if (previous == ZLLOG_NONE_SLOT) {
        /* flags bits 0..7 store last_slot+1. completed_boots alone is not a
         * cursor: after an incomplete boot followed by a clean one it can be
         * smaller than the slot just completed and would overwrite that boot
         * immediately. A zero value is the freshly initialized journal. */
        u32 last_plus_one = super.flags & 0xFFu;
        slot_index = last_plus_one ? last_plus_one % super.slot_count : 0u;
    } else {
        slot_index = (previous + 1u) % super.slot_count;
    }
    if (!session_boot_id) session_boot_id = super.generation + 1u;
    slot_boot_id = session_boot_id;
    slot_record_count = 0;
    slot_first_sequence = next_sequence;
    slot_last_sequence = 0;
    slot_started_tsc = cpu_tsc();
    slot_dropped_at_start = ram_dropped;

    /* Header durable first, then publish the slot in the other superblock. */
    if (!write_slot_header(1, 0)) return 0;
    super.active_slot = slot_index;
    super.flags = (super.flags & ~0xFFu) | ((slot_index + 1u) & 0xFFu);
    super.next_sequence = next_sequence;
    super.generation++;
    if (!publish_super()) return 0;
    slot_active = 1;
    return 1;
}

/* A boot can run for hours. A fixed 2 MiB slot is a power-loss boundary, not
 * a reason for observability to stop. Seal a full segment, publish it, and
 * continue in the next slot. The continuation record is the first record in
 * the new segment, so host tools can join segments from one running session.
 * completed_boots remains a count of durable slot segments in format v1. */
static int rotate_full_slot(void)
{
    if (!slot_active) return start_slot();
    u32 old_slot = slot_index;
    if (!write_slot_header(2, cpu_tsc())) return 0;
    super.active_slot = ZLLOG_NONE_SLOT;
    super.completed_boots++;
    super.next_sequence = next_sequence;
    super.generation++;
    if (!publish_super()) return 0;
    slot_active = 0;
    zllog_event(ZLLOG_SUB_KERNEL, ZLLOG_EV_CHECKPOINT, ZLLOG_INFO,
                1u, old_slot, (unsigned)super.completed_boots);
    return start_slot();
}

static int zllog_flush_inner(void)
{
    if (!mounted) return 0;
    drain_irq_records();
    if (ram_dropped != reported_ram_dropped) {
        u32 prior = ram_dropped;
        zllog_event(ZLLOG_SUB_KERNEL, ZLLOG_EV_RECORDER_DROP, ZLLOG_ERROR,
                    prior - reported_ram_dropped, prior, ram_count);
        reported_ram_dropped = prior;
    }
    flush_text_to_ram();
    if (!slot_active && !start_slot()) return 0;
    if (!ram_count) return 1;

    const u32 capacity = (ZLLOG_SLOT_BYTES - ZLLOG_SLOT_HEADER) / ZLLOG_RECORD_BYTES;
    if (slot_record_count >= capacity && !rotate_full_slot()) {
        last_error = ZLE_SLOT_FULL;
        return 0;
    }
    u32 available = ram_count;
    /* One periodic checkpoint writes at most 32 KiB of records. This bounds
     * foreground USB latency even if RAM accumulated a full 256 KiB while the
     * target was unavailable. Shutdown completion loops until all chunks are
     * durable. */
    if (available > 512u) available = 512u;
    if (available > capacity - slot_record_count) available = capacity - slot_record_count;

    u32 written = 0;
    while (written < available) {
        u32 record_no = slot_record_count + written;
        u32 data_off = slot_byte_offset() + ZLLOG_SLOT_HEADER + record_no * ZLLOG_RECORD_BYTES;
        u32 aligned = data_off - data_off % block_bytes;
        u32 prefix = data_off - aligned;
        bytes_zero(io_page, sizeof io_page);
        if (prefix && !xhci_msc_read_blocks(part_start + aligned / block_bytes,
                                             io_page, 1)) {
            last_error = ZLE_WRITE;
            return 0;
        }
        u32 room_records = (sizeof io_page - prefix) / ZLLOG_RECORD_BYTES;
        u32 take = available - written;
        if (take > room_records) take = room_records;
        for (u32 i = 0; i < take; i++) {
            u32 src = (ram_head + written + i) % ZLLOG_RAM_RECORDS;
            bytes_copy(io_page + prefix + i * ZLLOG_RECORD_BYTES,
                       ram_records[src], ZLLOG_RECORD_BYTES);
        }
        u32 used = prefix + take * ZLLOG_RECORD_BYTES;
        u32 write_bytes = (used + block_bytes - 1u) / block_bytes * block_bytes;
        if (!write_part(aligned, io_page, write_bytes)) return 0;
        written += take;
    }
    if (!xhci_msc_sync_cache()) { last_error = ZLE_SYNC; return 0; }

    if (!slot_record_count && written)
        slot_first_sequence = get64(ram_records[ram_head] + 0);
    if (written) {
        u32 last = (ram_head + written - 1u) % ZLLOG_RAM_RECORDS;
        slot_last_sequence = get64(ram_records[last] + 0);
    }
    slot_record_count += written;
    if (!write_slot_header(1, 0)) return 0;

    /* Only forget RAM records after both their data and checkpoint are
     * durable. A failed write retries from the old header boundary. */
    ram_head = (ram_head + written) % ZLLOG_RAM_RECORDS;
    ram_count -= written;
    super.next_sequence = next_sequence;
    last_error = ZLE_NONE;
    if (ram_count && slot_record_count >= capacity) {
        if (!rotate_full_slot()) { last_error = ZLE_SLOT_FULL; return 0; }
    }
    return written == available;
}

int zllog_flush(void)
{
    u64 started = cpu_tsc();
    u32 started_tick = idt_ticks();
    recorder_io_depth++;
    int ok = zllog_flush_inner();
    recorder_io_depth--;
    drain_irq_records();
    u64 delta = cpu_tsc() - started;
    u32 khz = cpu_tsc_khz(), cycles_us = khz / 1000u;
    u32 us = 0;
    if (cycles_us && delta <= 0xffffffffu) us = (u32)delta / cycles_us;
    else {
        u32 ticks = idt_ticks() - started_tick;
        us = ticks > 429496u ? 0xffffffffu : ticks * 10000u;
    }
    zllog_counter_observe(ZLLOG_C_FLUSH_US, us);
    if (!ok) {
        u32 error = last_error;
        flush_failure_count++;
        u32 shift = flush_failure_count > 6u ? 6u : flush_failure_count;
        u32 delay = 100u << shift;       /* 2s, 4s, ... capped at 64s */
        next_flush_retry_tick = idt_ticks() + delay;
        zllog_event(ZLLOG_SUB_STORAGE, ZLLOG_EV_FLUSH_ERROR, ZLLOG_ERROR,
                    error, (unsigned)xhci_msc_last_result(), ram_count);
    } else {
        flush_failure_count = 0;
        next_flush_retry_tick = 0;
        if (!storage_ready_persisted && slot_active && efi_persist_storage_ready) {
            efi_persist_storage_ready(idt_ticks());
            storage_ready_persisted = 1;
        }
    }
    if (!completing && us >= 100000u)
        zllog_trigger(ZLLOG_SUB_STORAGE, ZLLOG_EV_CHECKPOINT, ZLLOG_WARN,
                      us, (unsigned)ok, ram_count);
    return ok;
}

int zllog_complete(void)
{
    if (!mounted) return 0;
    counter_snapshot(1);
    completing = 1;
    do {
        if (!zllog_flush()) { completing = 0; return 0; }
    } while (ram_count);
    completing = 0;
    if (!slot_active) return 1;
    recorder_io_depth++;
    int ok = write_slot_header(2, cpu_tsc());
    if (ok) {
        super.active_slot = ZLLOG_NONE_SLOT;
        super.completed_boots++;
        super.next_sequence = next_sequence;
        super.generation++;
        ok = publish_super();
    }
    recorder_io_depth--;
    drain_irq_records();
    if (!ok) {
        zllog_event(ZLLOG_SUB_STORAGE, ZLLOG_EV_FLUSH_ERROR, ZLLOG_ERROR,
                    last_error, (unsigned)xhci_msc_last_result(), ram_count);
        return 0;
    }
    slot_active = 0;
    mounted = 0;                       /* completion is a shutdown operation */
    return 1;
}

void zllog_flush_if_due(void)
{
    u32 now = idt_ticks();
    if (!mounted) {
        if (last_error && next_mount_retry_tick &&
            (u32)(now - next_mount_retry_tick) < 0x80000000u)
            (void)zllog_mount();
        return;
    }
    if (next_flush_retry_tick &&
        (u32)(now - next_flush_retry_tick) >= 0x80000000u) return;
    if (now - last_flush_tick < 1000u) return; /* at most once per ten seconds */
    last_flush_tick = now;
    counter_snapshot(0);
    (void)zllog_flush();
}
