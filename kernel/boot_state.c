/* boot_state.c - pure recovery policy, shared by stage zero and host tests. */
#include "boot_state.h"

#define ZLOS_BOOT_STATE_KNOWN_FLAGS                                      \
    (ZLOS_BOOT_STATE_PENDING | ZLOS_BOOT_STATE_READY |                   \
     ZLOS_BOOT_STATE_DEGRADED)
_Static_assert(sizeof(struct zlos_boot_state) == ZLOS_BOOT_STATE_BYTES,
               "boot state ABI must remain exactly 64 bytes");
_Static_assert(__builtin_offsetof(struct zlos_boot_state, checksum) == 12,
               "boot state checksum offset is part of the ABI");

static void state_zero(void *where, unsigned int bytes)
{
    unsigned char *p = (unsigned char *)where;
    while (bytes--) *p++ = 0;
}

static int reserved_nonzero(const unsigned char *p, unsigned int bytes)
{
    unsigned char any = 0;
    while (bytes--) any |= *p++;
    return any != 0;
}

static int generation_valid(unsigned int generation)
{
    return generation == ZLOS_BOOT_GENERATION_CURRENT ||
           generation == ZLOS_BOOT_GENERATION_PREVIOUS ||
           generation == ZLOS_BOOT_GENERATION_RECOVERY;
}

static unsigned int *failure_counter(struct zlos_boot_state *state,
                                     unsigned int generation)
{
    if (generation == ZLOS_BOOT_GENERATION_CURRENT)
        return &state->current_failures;
    if (generation == ZLOS_BOOT_GENERATION_PREVIOUS)
        return &state->previous_failures;
    if (generation == ZLOS_BOOT_GENERATION_RECOVERY)
        return &state->recovery_failures;
    return 0;
}

static unsigned int failure_count(const struct zlos_boot_state *state,
                                  unsigned int generation)
{
    if (generation == ZLOS_BOOT_GENERATION_CURRENT)
        return state->current_failures;
    if (generation == ZLOS_BOOT_GENERATION_PREVIOUS)
        return state->previous_failures;
    if (generation == ZLOS_BOOT_GENERATION_RECOVERY)
        return state->recovery_failures;
    return ~0U;
}

static unsigned int candidate_bit(unsigned int generation)
{
    if (generation == ZLOS_BOOT_GENERATION_CURRENT)
        return ZLOS_BOOT_HAVE_CURRENT;
    if (generation == ZLOS_BOOT_GENERATION_PREVIOUS)
        return ZLOS_BOOT_HAVE_PREVIOUS;
    if (generation == ZLOS_BOOT_GENERATION_RECOVERY)
        return ZLOS_BOOT_HAVE_RECOVERY;
    return 0;
}

unsigned int zlos_boot_state_checksum(const struct zlos_boot_state *state)
{
    if (!state || state->bytes != ZLOS_BOOT_STATE_BYTES) return 0;
    const unsigned char *p = (const unsigned char *)state;
    unsigned int hash = 2166136261U;
    for (unsigned int i = 0; i < state->bytes; i++) {
        unsigned char value = (i >= 12U && i < 16U) ? 0 : p[i];
        hash ^= value;
        hash *= 16777619U;
    }
    return hash;
}

void zlos_boot_state_default(struct zlos_boot_state *state)
{
    if (!state) return;
    state_zero(state, sizeof(*state));
    state->magic = ZLOS_BOOT_STATE_MAGIC;
    state->version = ZLOS_BOOT_STATE_VERSION;
    state->bytes = ZLOS_BOOT_STATE_BYTES;
    state->attempt_limit = ZLOS_BOOT_ATTEMPT_LIMIT;
    state->checksum = zlos_boot_state_checksum(state);
}

int zlos_boot_state_validate(const struct zlos_boot_state *state)
{
    if (!state) return ZLOS_BOOT_E_ARGUMENT;
    if (state->magic != ZLOS_BOOT_STATE_MAGIC ||
        state->version != ZLOS_BOOT_STATE_VERSION ||
        state->bytes != ZLOS_BOOT_STATE_BYTES)
        return ZLOS_BOOT_E_VERSION;
    if (state->flags & ~ZLOS_BOOT_STATE_KNOWN_FLAGS)
        return ZLOS_BOOT_E_UNSUPPORTED;
    if ((state->flags & ZLOS_BOOT_STATE_PENDING) &&
        (state->flags & ZLOS_BOOT_STATE_READY))
        return ZLOS_BOOT_E_STATE;
    if (reserved_nonzero(state->reserved, sizeof(state->reserved)))
        return ZLOS_BOOT_E_UNSUPPORTED;
    if (state->attempt_limit != ZLOS_BOOT_ATTEMPT_LIMIT)
        return ZLOS_BOOT_E_VERSION;
    if ((state->flags & (ZLOS_BOOT_STATE_PENDING | ZLOS_BOOT_STATE_READY)) &&
        !generation_valid(state->selected_generation))
        return ZLOS_BOOT_E_STATE;
    if (state->flags & ZLOS_BOOT_STATE_PENDING) {
        if (!state->attempt_ordinal ||
            state->attempt_ordinal > state->attempt_limit)
            return ZLOS_BOOT_E_RANGE;
    } else if (state->attempt_ordinal > state->attempt_limit) {
        return ZLOS_BOOT_E_RANGE;
    }
    if (state->checksum != zlos_boot_state_checksum(state))
        return ZLOS_BOOT_E_CHECKSUM;
    return ZLOS_BOOT_OK;
}

static void observe_previous(struct zlos_boot_state *state)
{
    if (state->flags & ZLOS_BOOT_STATE_PENDING) {
        unsigned int *failures = failure_counter(
            state, state->selected_generation);
        if (failures && *failures != ~0U) (*failures)++;
        state->last_failure = ZLOS_BOOT_FAILURE_NOT_READY;
    } else if (state->flags & ZLOS_BOOT_STATE_READY) {
        unsigned int *failures = failure_counter(
            state, state->selected_generation);
        if (failures) *failures = 0;
        state->last_failure = ZLOS_BOOT_FAILURE_NONE;
    }
    state->flags = 0;
    state->attempt_ordinal = 0;
}

static unsigned int select_generation(const struct zlos_boot_state *state,
                                      unsigned int available,
                                      int *degraded)
{
    static const unsigned int order[3] = {
        ZLOS_BOOT_GENERATION_CURRENT,
        ZLOS_BOOT_GENERATION_PREVIOUS,
        ZLOS_BOOT_GENERATION_RECOVERY
    };
    for (unsigned int i = 0; i < 3; i++) {
        unsigned int generation = order[i];
        if ((available & candidate_bit(generation)) &&
            failure_count(state, generation) < ZLOS_BOOT_ATTEMPT_LIMIT)
            return generation;
    }

    /* Exhausted counters must not turn an otherwise bootable device into a
     * brick. Prefer the recovery image, then previous, then current, and make
     * the degraded decision explicit in the durable state. */
    *degraded = 1;
    for (int i = 2; i >= 0; i--)
        if (available & candidate_bit(order[i])) return order[i];
    return 0;
}

int zlos_boot_state_prepare(struct zlos_boot_state *state,
                            unsigned int available_candidates)
{
    if (!state || !available_candidates ||
        (available_candidates & ~ZLOS_BOOT_HAVE_ALL))
        return ZLOS_BOOT_E_ARGUMENT;
    if (zlos_boot_state_validate(state) != ZLOS_BOOT_OK)
        zlos_boot_state_default(state);
    observe_previous(state);

    int degraded = 0;
    unsigned int selected = select_generation(
        state, available_candidates, &degraded);
    if (!selected) return ZLOS_BOOT_E_RANGE;
    unsigned int count = failure_count(state, selected);
    state->selected_generation = selected;
    state->attempt_ordinal = count < ZLOS_BOOT_ATTEMPT_LIMIT
        ? count + 1U : ZLOS_BOOT_ATTEMPT_LIMIT;
    state->attempt_limit = ZLOS_BOOT_ATTEMPT_LIMIT;
    state->flags = ZLOS_BOOT_STATE_PENDING |
        (degraded ? ZLOS_BOOT_STATE_DEGRADED : 0U);
    if (state->sequence != ~0ULL) state->sequence++;
    state->checksum = zlos_boot_state_checksum(state);
    return zlos_boot_state_validate(state);
}

int zlos_boot_state_mark_ready(struct zlos_boot_state *state)
{
    int status = zlos_boot_state_validate(state);
    if (status != ZLOS_BOOT_OK) return status;
    if (!(state->flags & ZLOS_BOOT_STATE_PENDING) ||
        !generation_valid(state->selected_generation))
        return ZLOS_BOOT_E_STATE;
    unsigned int *failures = failure_counter(
        state, state->selected_generation);
    if (failures) *failures = 0;
    state->flags &= ~(ZLOS_BOOT_STATE_PENDING | ZLOS_BOOT_STATE_DEGRADED);
    state->flags |= ZLOS_BOOT_STATE_READY;
    state->last_failure = ZLOS_BOOT_FAILURE_NONE;
    state->checksum = zlos_boot_state_checksum(state);
    return zlos_boot_state_validate(state);
}
