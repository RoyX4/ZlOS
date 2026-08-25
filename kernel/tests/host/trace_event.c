/* trace_event.c - bounded, pointer-free zlOS event evidence.
 *
 * This is the transport-independent core of EV-018, not yet a system service.
 * It owns the stable envelope, admission, sequencing, redaction, checksums,
 * drop evidence and wire encoding. It deliberately does not allocate, block,
 * overwrite, read a clock or invent process/authority identity. The future
 * audit service must serialize callers and supply those values. */
#include "trace_event.h"

typedef char zlos_trace_wire_size_must_be_152[
    sizeof(struct zlos_trace_event) == ZLOS_TRACE_WIRE_BYTES ? 1 : -1];
typedef char zlos_trace_signed_word_must_be_32[
    sizeof(trace_i32) == 4 ? 1 : -1];
typedef char zlos_trace_sequence_offset_must_be_8[
    __builtin_offsetof(struct zlos_trace_event, sequence) == 8 ? 1 : -1];
typedef char zlos_trace_feature_offset_must_be_80[
    __builtin_offsetof(struct zlos_trace_event, feature_id) == 80 ? 1 : -1];
typedef char zlos_trace_payload_offset_must_be_112[
    __builtin_offsetof(struct zlos_trace_event, payload) == 112 ? 1 : -1];
typedef char zlos_trace_checksum_offset_must_be_144[
    __builtin_offsetof(struct zlos_trace_event, checksum) == 144 ? 1 : -1];
typedef char zlos_trace_capacity_must_be_positive[
    ZLOS_TRACE_CAPACITY > 0 ? 1 : -1];

#define TRACE_PAYLOAD_MASK ((1u << ZLOS_TRACE_PAYLOAD_WORDS) - 1u)
#define TRACE_DROP_CODE 1u

static void zero_bytes(void *pointer, trace_u32 bytes)
{
    trace_u8 *out = (trace_u8 *)pointer;
    while (bytes--) *out++ = 0;
}

static int valid_token(const char *text, int feature)
{
    trace_u32 i;
    int hyphen = 0;
    if (!text || text[0] == 0) return 0;
    for (i = 0; i < ZLOS_TRACE_ID_BYTES; i++) {
        trace_u8 c = (trace_u8)text[i];
        if (c == 0) return !feature || hyphen;
        if (feature) {
            if (c == '-') hyphen = 1;
            else if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')))
                return 0;
        } else if (!((c >= 'A' && c <= 'Z') ||
                     (c >= 'a' && c <= 'z') ||
                     (c >= '0' && c <= '9') || c == '-' || c == '_' ||
                     c == '.' || c == '/')) return 0;
    }
    return 0; /* fixed array has no terminator */
}

static int identity_pair(trace_u32 id, trace_u32 generation)
{
    return (id == 0 && generation == 0) ||
           (id != 0 && generation != 0);
}

static int valid_state(const struct zlos_trace *trace)
{
    return trace && trace->boot_id != 0 && trace->next_sequence != 0 &&
           trace->head < ZLOS_TRACE_CAPACITY &&
           trace->count <= ZLOS_TRACE_CAPACITY;
}

static void record_drop(struct zlos_trace *trace)
{
    const trace_u64 maximum = ~(trace_u64)0;
    if (trace->dropped_total != maximum) trace->dropped_total++;
    if (trace->pending_drops != maximum) trace->pending_drops++;
}

static trace_u64 mix_byte(trace_u64 hash, trace_u8 value)
{
    hash ^= value;
    return hash * 1099511628211ULL;
}

static trace_u64 mix16(trace_u64 hash, trace_u16 value)
{
    for (int i = 0; i < 2; i++) hash = mix_byte(hash, (trace_u8)(value >> (i * 8)));
    return hash;
}

static trace_u64 mix32(trace_u64 hash, trace_u32 value)
{
    for (int i = 0; i < 4; i++) hash = mix_byte(hash, (trace_u8)(value >> (i * 8)));
    return hash;
}

static trace_u64 mix64(trace_u64 hash, trace_u64 value)
{
    for (int i = 0; i < 8; i++) hash = mix_byte(hash, (trace_u8)(value >> (i * 8)));
    return hash;
}

static trace_u64 checksum_of(const struct zlos_trace_event *e)
{
    trace_u64 h = 1469598103934665603ULL;
    h = mix32(h, e->magic);
    h = mix16(h, e->version);
    h = mix16(h, e->bytes);
    h = mix64(h, e->sequence);
    h = mix64(h, e->monotonic_ns);
    h = mix64(h, e->boot_id);
    h = mix64(h, e->correlation_id);
    h = mix64(h, e->parent_sequence);
    h = mix32(h, e->process_id);
    h = mix32(h, e->process_generation);
    h = mix32(h, e->authority_id);
    h = mix32(h, e->authority_generation);
    h = mix32(h, e->event_code);
    h = mix32(h, (trace_u32)e->outcome);
    h = mix_byte(h, e->severity);
    h = mix_byte(h, e->privacy);
    h = mix_byte(h, e->kind);
    h = mix_byte(h, e->flags);
    h = mix_byte(h, e->redaction_mask);
    h = mix_byte(h, e->payload_words);
    h = mix16(h, e->reserved);
    for (trace_u32 i = 0; i < ZLOS_TRACE_ID_BYTES; i++)
        h = mix_byte(h, (trace_u8)e->feature_id[i]);
    for (trace_u32 i = 0; i < ZLOS_TRACE_ID_BYTES; i++)
        h = mix_byte(h, (trace_u8)e->component_id[i]);
    for (trace_u32 i = 0; i < ZLOS_TRACE_PAYLOAD_WORDS; i++)
        h = mix64(h, e->payload[i]);
    return h;
}

static void copy_id(char *out, const char *input)
{
    trace_u32 i = 0;
    for (; i < ZLOS_TRACE_ID_BYTES && input[i]; i++) out[i] = input[i];
    for (; i < ZLOS_TRACE_ID_BYTES; i++) out[i] = 0;
}

static int valid_input(const struct zlos_trace *trace,
                       const struct zlos_trace_input *input)
{
    if (!valid_state(trace) || !input)
        return 0;
    if (!valid_token(input->feature_id, 1) ||
        !valid_token(input->component_id, 0)) return 0;
    if (input->severity > ZLOS_TRACE_CRITICAL ||
        input->privacy > ZLOS_TRACE_SECRET || input->kind > ZLOS_TRACE_DROP)
        return 0;
    if (input->flags != 0 || input->reserved8[0] != 0 ||
        input->reserved8[1] != 0 || input->reserved8[2] != 0) return 0;
    if ((input->redaction_mask & ~TRACE_PAYLOAD_MASK) != 0) return 0;
    if (input->privacy == ZLOS_TRACE_SECRET &&
        input->redaction_mask != TRACE_PAYLOAD_MASK) return 0;
    if (!identity_pair(input->process_id, input->process_generation) ||
        !identity_pair(input->authority_id, input->authority_generation))
        return 0;
    if (input->correlation_id == 0) return 0;
    if (input->parent_sequence >= trace->next_sequence &&
        input->parent_sequence != 0) return 0;
    if (input->monotonic_ns < trace->last_monotonic_ns) return 0;
    return 1;
}

static void commit(struct zlos_trace *trace,
                   const struct zlos_trace_input *input)
{
    trace_u32 index = (trace->head + trace->count) % ZLOS_TRACE_CAPACITY;
    struct zlos_trace_event *event = &trace->slots[index];
    zero_bytes(event, sizeof(*event));
    event->magic = ZLOS_TRACE_MAGIC;
    event->version = ZLOS_TRACE_VERSION;
    event->bytes = ZLOS_TRACE_WIRE_BYTES;
    event->sequence = trace->next_sequence++;
    event->monotonic_ns = input->monotonic_ns;
    event->boot_id = trace->boot_id;
    event->correlation_id = input->correlation_id;
    event->parent_sequence = input->parent_sequence;
    event->process_id = input->process_id;
    event->process_generation = input->process_generation;
    event->authority_id = input->authority_id;
    event->authority_generation = input->authority_generation;
    event->event_code = input->event_code;
    event->outcome = input->outcome;
    event->severity = input->severity;
    event->privacy = input->privacy;
    event->kind = input->kind;
    event->flags = input->flags;
    event->redaction_mask = input->redaction_mask;
    event->payload_words = ZLOS_TRACE_PAYLOAD_WORDS;
    copy_id(event->feature_id, input->feature_id);
    copy_id(event->component_id, input->component_id);
    for (trace_u32 i = 0; i < ZLOS_TRACE_PAYLOAD_WORDS; i++)
        event->payload[i] = (input->redaction_mask & (1u << i)) ? 0 : input->payload[i];
    event->checksum = checksum_of(event);
    trace->last_monotonic_ns = input->monotonic_ns;
    trace->count++;
}

int zlos_trace_init(struct zlos_trace *trace, trace_u64 boot_id)
{
    if (!trace || boot_id == 0) return ZLOS_TRACE_INVALID;
    zero_bytes(trace, sizeof(*trace));
    trace->boot_id = boot_id;
    trace->next_sequence = 1;
    return ZLOS_TRACE_OK;
}

int zlos_trace_emit(struct zlos_trace *trace,
                    const struct zlos_trace_input *input)
{
    struct zlos_trace_input drop;
    trace_u64 dropped;
    if (!valid_input(trace, input)) return ZLOS_TRACE_INVALID;
    if (trace->count == ZLOS_TRACE_CAPACITY) {
        record_drop(trace);
        return ZLOS_TRACE_FULL;
    }
    if (trace->pending_drops) {
        dropped = trace->pending_drops;
        zero_bytes(&drop, sizeof(drop));
        drop.monotonic_ns = input->monotonic_ns;
        drop.correlation_id = input->correlation_id;
        drop.event_code = TRACE_DROP_CODE;
        drop.outcome = -1;
        drop.severity = ZLOS_TRACE_WARNING;
        drop.privacy = ZLOS_TRACE_PUBLIC;
        drop.kind = ZLOS_TRACE_DROP;
        copy_id(drop.feature_id, "EV-018");
        copy_id(drop.component_id, "trace-core");
        drop.payload[0] = dropped;
        commit(trace, &drop);
        trace->pending_drops = 0;
        if (trace->count == ZLOS_TRACE_CAPACITY) {
            record_drop(trace);
            return ZLOS_TRACE_FULL;
        }
    }
    commit(trace, input);
    return ZLOS_TRACE_OK;
}

int zlos_trace_validate(const struct zlos_trace_event *event)
{
    if (!event || event->magic != ZLOS_TRACE_MAGIC ||
        event->version != ZLOS_TRACE_VERSION ||
        event->bytes != ZLOS_TRACE_WIRE_BYTES || event->sequence == 0 ||
        event->boot_id == 0 || event->correlation_id == 0) return 0;
    if (!valid_token(event->feature_id, 1) ||
        !valid_token(event->component_id, 0)) return 0;
    if (event->severity > ZLOS_TRACE_CRITICAL ||
        event->privacy > ZLOS_TRACE_SECRET || event->kind > ZLOS_TRACE_DROP)
        return 0;
    if (event->flags != 0 || event->reserved != 0 ||
        event->payload_words != ZLOS_TRACE_PAYLOAD_WORDS ||
        (event->redaction_mask & ~TRACE_PAYLOAD_MASK) != 0) return 0;
    if (event->privacy == ZLOS_TRACE_SECRET &&
        event->redaction_mask != TRACE_PAYLOAD_MASK) return 0;
    if (!identity_pair(event->process_id, event->process_generation) ||
        !identity_pair(event->authority_id, event->authority_generation))
        return 0;
    if (event->parent_sequence >= event->sequence &&
        event->parent_sequence != 0) return 0;
    for (trace_u32 i = 0; i < ZLOS_TRACE_PAYLOAD_WORDS; i++)
        if ((event->redaction_mask & (1u << i)) && event->payload[i] != 0)
            return 0;
    return event->checksum == checksum_of(event);
}

int zlos_trace_take(struct zlos_trace *trace,
                    struct zlos_trace_event *out)
{
    struct zlos_trace_event *head;
    if (!valid_state(trace) || !out) return ZLOS_TRACE_INVALID;
    if (trace->count == 0) return ZLOS_TRACE_EMPTY;
    head = &trace->slots[trace->head];
    if (!zlos_trace_validate(head)) return ZLOS_TRACE_CORRUPT;
    *out = *head;
    zero_bytes(head, sizeof(*head));
    trace->head = (trace->head + 1) % ZLOS_TRACE_CAPACITY;
    trace->count--;
    return ZLOS_TRACE_OK;
}

trace_u32 zlos_trace_pending(const struct zlos_trace *trace)
{
    return trace ? trace->count : 0;
}

trace_u64 zlos_trace_dropped(const struct zlos_trace *trace)
{
    return trace ? trace->dropped_total : 0;
}

static void put16(trace_u8 *out, trace_u16 value)
{
    out[0] = (trace_u8)value;
    out[1] = (trace_u8)(value >> 8);
}

static void put32(trace_u8 *out, trace_u32 value)
{
    for (int i = 0; i < 4; i++) out[i] = (trace_u8)(value >> (i * 8));
}

static void put64(trace_u8 *out, trace_u64 value)
{
    for (int i = 0; i < 8; i++) out[i] = (trace_u8)(value >> (i * 8));
}

static trace_u16 get16(const trace_u8 *in)
{
    return (trace_u16)((trace_u16)in[0] | ((trace_u16)in[1] << 8));
}

static trace_u32 get32(const trace_u8 *in)
{
    trace_u32 value = 0;
    for (int i = 0; i < 4; i++) value |= (trace_u32)in[i] << (i * 8);
    return value;
}

static trace_u64 get64(const trace_u8 *in)
{
    trace_u64 value = 0;
    for (int i = 0; i < 8; i++) value |= (trace_u64)in[i] << (i * 8);
    return value;
}

int zlos_trace_encode(trace_u8 *out, trace_u32 capacity,
                      const struct zlos_trace_event *event)
{
    if (!out || capacity < ZLOS_TRACE_WIRE_BYTES ||
        !zlos_trace_validate(event)) return ZLOS_TRACE_INVALID;
    put32(out + 0, event->magic);
    put16(out + 4, event->version);
    put16(out + 6, event->bytes);
    put64(out + 8, event->sequence);
    put64(out + 16, event->monotonic_ns);
    put64(out + 24, event->boot_id);
    put64(out + 32, event->correlation_id);
    put64(out + 40, event->parent_sequence);
    put32(out + 48, event->process_id);
    put32(out + 52, event->process_generation);
    put32(out + 56, event->authority_id);
    put32(out + 60, event->authority_generation);
    put32(out + 64, event->event_code);
    put32(out + 68, (trace_u32)event->outcome);
    out[72] = event->severity;
    out[73] = event->privacy;
    out[74] = event->kind;
    out[75] = event->flags;
    out[76] = event->redaction_mask;
    out[77] = event->payload_words;
    put16(out + 78, event->reserved);
    for (trace_u32 i = 0; i < ZLOS_TRACE_ID_BYTES; i++) {
        out[80 + i] = (trace_u8)event->feature_id[i];
        out[96 + i] = (trace_u8)event->component_id[i];
    }
    for (trace_u32 i = 0; i < ZLOS_TRACE_PAYLOAD_WORDS; i++)
        put64(out + 112 + i * 8, event->payload[i]);
    put64(out + 144, event->checksum);
    return ZLOS_TRACE_OK;
}

int zlos_trace_decode(struct zlos_trace_event *out,
                      const trace_u8 *wire, trace_u32 bytes)
{
    if (!out || !wire || bytes != ZLOS_TRACE_WIRE_BYTES)
        return ZLOS_TRACE_INVALID;
    zero_bytes(out, sizeof(*out));
    out->magic = get32(wire + 0);
    out->version = get16(wire + 4);
    out->bytes = get16(wire + 6);
    out->sequence = get64(wire + 8);
    out->monotonic_ns = get64(wire + 16);
    out->boot_id = get64(wire + 24);
    out->correlation_id = get64(wire + 32);
    out->parent_sequence = get64(wire + 40);
    out->process_id = get32(wire + 48);
    out->process_generation = get32(wire + 52);
    out->authority_id = get32(wire + 56);
    out->authority_generation = get32(wire + 60);
    out->event_code = get32(wire + 64);
    out->outcome = (trace_i32)get32(wire + 68);
    out->severity = wire[72];
    out->privacy = wire[73];
    out->kind = wire[74];
    out->flags = wire[75];
    out->redaction_mask = wire[76];
    out->payload_words = wire[77];
    out->reserved = get16(wire + 78);
    for (trace_u32 i = 0; i < ZLOS_TRACE_ID_BYTES; i++) {
        out->feature_id[i] = (char)wire[80 + i];
        out->component_id[i] = (char)wire[96 + i];
    }
    for (trace_u32 i = 0; i < ZLOS_TRACE_PAYLOAD_WORDS; i++)
        out->payload[i] = get64(wire + 112 + i * 8);
    out->checksum = get64(wire + 144);
    return zlos_trace_validate(out) ? ZLOS_TRACE_OK : ZLOS_TRACE_CORRUPT;
}

#ifdef ZLOS_TRACE_HOSTTEST
int zlos_trace_host_corrupt_head(struct zlos_trace *trace)
{
    if (!trace || trace->count == 0) return 0;
    trace->slots[trace->head].payload[0] ^= 1;
    return 1;
}
#endif
