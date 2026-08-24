#ifndef ZLOS_TRACE_EVENT_H
#define ZLOS_TRACE_EVENT_H

typedef unsigned char trace_u8;
typedef unsigned short trace_u16;
typedef unsigned int trace_u32;
typedef unsigned long long trace_u64;
typedef int trace_i32;

#define ZLOS_TRACE_MAGIC          0x56454C5Au /* "ZLEV" in little endian */
#define ZLOS_TRACE_VERSION        1u
#define ZLOS_TRACE_WIRE_BYTES     152u
#define ZLOS_TRACE_ID_BYTES       16u
#define ZLOS_TRACE_PAYLOAD_WORDS  4u
#ifndef ZLOS_TRACE_CAPACITY
#define ZLOS_TRACE_CAPACITY       64u
#endif

enum zlos_trace_severity {
    ZLOS_TRACE_DEBUG = 0,
    ZLOS_TRACE_INFO = 1,
    ZLOS_TRACE_NOTICE = 2,
    ZLOS_TRACE_WARNING = 3,
    ZLOS_TRACE_ERROR = 4,
    ZLOS_TRACE_CRITICAL = 5
};

enum zlos_trace_privacy {
    ZLOS_TRACE_PUBLIC = 0,
    ZLOS_TRACE_INTERNAL = 1,
    ZLOS_TRACE_SENSITIVE = 2,
    ZLOS_TRACE_SECRET = 3
};

enum zlos_trace_kind {
    ZLOS_TRACE_STATE = 0,
    ZLOS_TRACE_EFFECT = 1,
    ZLOS_TRACE_SECURITY = 2,
    ZLOS_TRACE_PERFORMANCE = 3,
    ZLOS_TRACE_LIFECYCLE = 4,
    ZLOS_TRACE_DROP = 5
};

enum zlos_trace_result {
    ZLOS_TRACE_OK = 1,
    ZLOS_TRACE_EMPTY = 0,
    ZLOS_TRACE_INVALID = -1,
    ZLOS_TRACE_FULL = -2,
    ZLOS_TRACE_CORRUPT = -3
};

/* Stable logical and wire shape. Every field is scalar or a fixed byte array;
 * no kernel or user pointer can enter a trace. Identity pairs are generation
 * tagged so a reused PID/handle cannot impersonate the earlier subject. */
struct zlos_trace_event {
    trace_u32 magic;
    trace_u16 version;
    trace_u16 bytes;
    trace_u64 sequence;
    trace_u64 monotonic_ns;
    trace_u64 boot_id;
    trace_u64 correlation_id;
    trace_u64 parent_sequence;
    trace_u32 process_id;
    trace_u32 process_generation;
    trace_u32 authority_id;
    trace_u32 authority_generation;
    trace_u32 event_code;
    trace_i32 outcome;
    trace_u8 severity;
    trace_u8 privacy;
    trace_u8 kind;
    trace_u8 flags;
    trace_u8 redaction_mask;
    trace_u8 payload_words;
    trace_u16 reserved;
    char feature_id[ZLOS_TRACE_ID_BYTES];
    char component_id[ZLOS_TRACE_ID_BYTES];
    trace_u64 payload[ZLOS_TRACE_PAYLOAD_WORDS];
    trace_u64 checksum;
};

struct zlos_trace_input {
    trace_u64 monotonic_ns;
    trace_u64 correlation_id;
    trace_u64 parent_sequence;
    trace_u32 process_id;
    trace_u32 process_generation;
    trace_u32 authority_id;
    trace_u32 authority_generation;
    trace_u32 event_code;
    trace_i32 outcome;
    trace_u8 severity;
    trace_u8 privacy;
    trace_u8 kind;
    trace_u8 flags;
    trace_u8 redaction_mask;
    trace_u8 reserved8[3];
    char feature_id[ZLOS_TRACE_ID_BYTES];
    char component_id[ZLOS_TRACE_ID_BYTES];
    trace_u64 payload[ZLOS_TRACE_PAYLOAD_WORDS];
};

/* Single-owner bounded queue. Callers must serialize emit/take until the
 * future audit service owns it; this module does not pretend to provide an
 * interrupt-safe or SMP-safe transport. Full queues never overwrite records.
 * The next successful emit first commits a synthetic DROP record. */
struct zlos_trace {
    struct zlos_trace_event slots[ZLOS_TRACE_CAPACITY];
    trace_u64 boot_id;
    trace_u64 next_sequence;
    trace_u64 last_monotonic_ns;
    trace_u64 dropped_total;
    trace_u64 pending_drops;
    trace_u32 head;
    trace_u32 count;
};

int zlos_trace_init(struct zlos_trace *trace, trace_u64 boot_id);
int zlos_trace_emit(struct zlos_trace *trace,
                    const struct zlos_trace_input *input);
int zlos_trace_take(struct zlos_trace *trace,
                    struct zlos_trace_event *out);
int zlos_trace_validate(const struct zlos_trace_event *event);
int zlos_trace_encode(trace_u8 *out, trace_u32 capacity,
                      const struct zlos_trace_event *event);
int zlos_trace_decode(struct zlos_trace_event *out,
                      const trace_u8 *wire, trace_u32 bytes);
trace_u32 zlos_trace_pending(const struct zlos_trace *trace);
trace_u64 zlos_trace_dropped(const struct zlos_trace *trace);

#ifdef ZLOS_TRACE_HOSTTEST
int zlos_trace_host_corrupt_head(struct zlos_trace *trace);
#endif

#endif
