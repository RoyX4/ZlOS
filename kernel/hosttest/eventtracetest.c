/* Host proof for the real EV-018 bounded event core. */
#include <stdio.h>
#include <string.h>

#include "trace_event.h"

static int checks;
static int failures;

static void check(int condition, const char *message)
{
    checks++;
    if (condition) printf("  ok   %s\n", message);
    else { printf("  FAIL %s\n", message); failures++; }
}

static struct zlos_trace_input input_at(trace_u64 now, trace_u64 correlation)
{
    struct zlos_trace_input input;
    memset(&input, 0, sizeof(input));
    input.monotonic_ns = now;
    input.correlation_id = correlation;
    input.process_id = 7;
    input.process_generation = 3;
    input.authority_id = 11;
    input.authority_generation = 5;
    input.event_code = 42;
    input.outcome = 0;
    input.severity = ZLOS_TRACE_INFO;
    input.privacy = ZLOS_TRACE_INTERNAL;
    input.kind = ZLOS_TRACE_EFFECT;
    strcpy(input.feature_id, "EV-018");
    strcpy(input.component_id, "host-proof");
    input.payload[0] = 0x1122334455667788ULL;
    input.payload[1] = 0x8877665544332211ULL;
    return input;
}

int main(void)
{
    struct zlos_trace trace;
    struct zlos_trace_event event;
    struct zlos_trace_event decoded;
    struct zlos_trace_input input;
    trace_u8 wire[ZLOS_TRACE_WIRE_BYTES];

    puts("structured event trace\n");
    check(sizeof(struct zlos_trace_event) == ZLOS_TRACE_WIRE_BYTES,
          "the in-memory envelope is the exact 152-byte wire shape");
    check(zlos_trace_init(NULL, 1) == ZLOS_TRACE_INVALID &&
          zlos_trace_init(&trace, 0) == ZLOS_TRACE_INVALID,
          "a trace requires storage and a nonzero boot identity");
    check(zlos_trace_init(&trace, 0xAABBCCDDu) == ZLOS_TRACE_OK,
          "a valid boot identity initializes an empty trace");
    check(zlos_trace_pending(&trace) == 0 && zlos_trace_dropped(&trace) == 0,
          "initial counters are empty and explicit");

    input = input_at(100, 9);
    check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_OK,
          "a typed event is admitted");
    check(zlos_trace_take(&trace, &event) == ZLOS_TRACE_OK,
          "the admitted event can be consumed");
    check(event.sequence == 1 && event.monotonic_ns == 100 &&
          event.boot_id == 0xAABBCCDDu,
          "sequence, monotonic time and boot identity are exact");
    check(event.process_id == 7 && event.process_generation == 3 &&
          event.authority_id == 11 && event.authority_generation == 5,
          "process and authority generations survive byte-for-byte");
    check(event.correlation_id == 9 && event.parent_sequence == 0 &&
          strcmp(event.feature_id, "EV-018") == 0 &&
          strcmp(event.component_id, "host-proof") == 0,
          "correlation and stable component identities are preserved");
    check(zlos_trace_validate(&event),
          "the committed event validates its checksum and schema");
    check(zlos_trace_encode(wire, sizeof(wire), &event) == ZLOS_TRACE_OK &&
          zlos_trace_decode(&decoded, wire, sizeof(wire)) == ZLOS_TRACE_OK &&
          memcmp(&decoded, &event, sizeof(event)) == 0,
          "little-endian export/import round-trips exactly");
    wire[112] ^= 1;
    check(zlos_trace_decode(&decoded, wire, sizeof(wire)) == ZLOS_TRACE_CORRUPT,
          "wire payload corruption fails the checksum");
    check(zlos_trace_encode(wire, sizeof(wire) - 1, &event) == ZLOS_TRACE_INVALID &&
          zlos_trace_decode(&decoded, wire, sizeof(wire) - 1) == ZLOS_TRACE_INVALID,
          "short wire buffers fail closed");

    input = input_at(101, 10);
    input.privacy = ZLOS_TRACE_SENSITIVE;
    input.redaction_mask = 2;
    check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_OK &&
          zlos_trace_take(&trace, &event) == ZLOS_TRACE_OK,
          "a sensitive event with an explicit field mask is accepted");
    check(event.payload[0] == 0x1122334455667788ULL &&
          event.payload[1] == 0 && event.redaction_mask == 2,
          "redaction zeros only the classified payload before commit");
    input = input_at(102, 11);
    input.privacy = ZLOS_TRACE_SECRET;
    input.redaction_mask = 7;
    check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_INVALID,
          "a secret event cannot leave any payload word unredacted");
    input.redaction_mask = 15;
    check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_OK &&
          zlos_trace_take(&trace, &event) == ZLOS_TRACE_OK &&
          event.payload[0] == 0 && event.payload[1] == 0,
          "a fully redacted secret event commits only zero payloads");

    {
        trace_u32 before = zlos_trace_pending(&trace);
        input = input_at(101, 12);
        check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_INVALID,
              "monotonic time regression is rejected");
        input = input_at(103, 12);
        input.parent_sequence = 99;
        check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_INVALID,
              "a parent cannot name a future sequence");
        input = input_at(103, 12);
        input.process_generation = 0;
        check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_INVALID,
              "a process ID without its generation is rejected");
        input = input_at(103, 12);
        input.correlation_id = 0;
        check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_INVALID,
              "an uncorrelated event is rejected");
        input = input_at(103, 12);
        strcpy(input.feature_id, "bad id");
        check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_INVALID,
              "a noncanonical feature identity is rejected");
        check(zlos_trace_pending(&trace) == before,
              "all rejected inputs leave queue state unchanged");
    }

    check(zlos_trace_init(&trace, 77) == ZLOS_TRACE_OK,
          "the drop test begins from a fresh boot trace");
    for (trace_u32 i = 0; i < ZLOS_TRACE_CAPACITY; i++) {
        input = input_at(200 + i, 100 + i);
        check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_OK,
              "each bounded slot accepts exactly one event");
    }
    input = input_at(300, 200);
    check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_FULL &&
          zlos_trace_dropped(&trace) == 1 &&
          zlos_trace_pending(&trace) == ZLOS_TRACE_CAPACITY,
          "a full trace drops the new event without overwriting evidence");
    check(zlos_trace_take(&trace, &event) == ZLOS_TRACE_OK &&
          zlos_trace_take(&trace, &event) == ZLOS_TRACE_OK,
          "two consumed records make room for drop evidence and new work");
    input = input_at(301, 201);
    check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_OK,
          "the first post-pressure emit commits the drop marker first");
    check(zlos_trace_take(&trace, &event) == ZLOS_TRACE_OK && event.sequence == 3 &&
          zlos_trace_take(&trace, &event) == ZLOS_TRACE_OK && event.sequence == 4,
          "pre-pressure FIFO order remains intact");
    check(zlos_trace_take(&trace, &event) == ZLOS_TRACE_OK &&
          event.kind == ZLOS_TRACE_DROP && event.sequence == 5 &&
          event.payload[0] == 1 && strcmp(event.feature_id, "EV-018") == 0,
          "one synthetic record exposes the exact dropped-event count");
    check(zlos_trace_take(&trace, &event) == ZLOS_TRACE_OK &&
          event.sequence == 6 && event.correlation_id == 201,
          "the admitted post-drop event follows the marker");
    check(zlos_trace_take(&trace, &event) == ZLOS_TRACE_EMPTY,
          "the drained queue reports empty rather than stale data");

    input = input_at(302, 202);
    check(zlos_trace_emit(&trace, &input) == ZLOS_TRACE_OK &&
          zlos_trace_host_corrupt_head(&trace),
          "the mutation hook corrupts a queued field without its checksum");
    check(zlos_trace_take(&trace, &event) == ZLOS_TRACE_CORRUPT &&
          zlos_trace_pending(&trace) == 1,
          "corruption fails closed and preserves the original record for inspection");

    printf("\n%d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
