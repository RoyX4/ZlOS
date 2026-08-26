/* boot_state.h - durable, bounded current/previous/recovery selection state. */
#ifndef ZLOS_BOOT_STATE_H
#define ZLOS_BOOT_STATE_H

#include "boot_handover.h"

#define ZLOS_BOOT_STATE_MAGIC 0x5A4C425354415445ULL /* "ZLBSTATE" */
#define ZLOS_BOOT_STATE_VERSION 1U
#define ZLOS_BOOT_STATE_BYTES 64U
#define ZLOS_BOOT_ATTEMPT_LIMIT 2U

enum zlos_boot_state_flags {
    ZLOS_BOOT_STATE_PENDING = 1U << 0,
    ZLOS_BOOT_STATE_READY = 1U << 1,
    ZLOS_BOOT_STATE_DEGRADED = 1U << 2
};

enum zlos_boot_candidate_mask {
    ZLOS_BOOT_HAVE_CURRENT = 1U << 0,
    ZLOS_BOOT_HAVE_PREVIOUS = 1U << 1,
    ZLOS_BOOT_HAVE_RECOVERY = 1U << 2
};
#define ZLOS_BOOT_HAVE_ALL                                                \
    (ZLOS_BOOT_HAVE_CURRENT | ZLOS_BOOT_HAVE_PREVIOUS |                  \
     ZLOS_BOOT_HAVE_RECOVERY)

enum zlos_boot_failure_reason {
    ZLOS_BOOT_FAILURE_NONE = 0,
    ZLOS_BOOT_FAILURE_NOT_READY = 1,
    ZLOS_BOOT_FAILURE_START_RETURNED = 2,
    ZLOS_BOOT_FAILURE_IMAGE_MISSING = 3,
    ZLOS_BOOT_FAILURE_IMAGE_INVALID = 4
};

struct zlos_boot_state {
    unsigned long long magic;
    unsigned short version;
    unsigned short bytes;
    unsigned int checksum;
    unsigned long long sequence;
    unsigned int flags;
    unsigned int selected_generation;
    unsigned int attempt_ordinal;
    unsigned int attempt_limit;
    unsigned int current_failures;
    unsigned int previous_failures;
    unsigned int recovery_failures;
    unsigned int last_failure;
    unsigned char reserved[8];
};

void zlos_boot_state_default(struct zlos_boot_state *state);
unsigned int zlos_boot_state_checksum(const struct zlos_boot_state *state);
int zlos_boot_state_validate(const struct zlos_boot_state *state);

/* Observe the previous pending/ready result, choose from the images which are
 * actually present, mark the new choice pending, and reseal the state. */
int zlos_boot_state_prepare(struct zlos_boot_state *state,
                            unsigned int available_candidates);

/* Called by the selected kernel only after the system reaches its ready gate. */
int zlos_boot_state_mark_ready(struct zlos_boot_state *state);

#endif
