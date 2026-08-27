/* boot_state_test.c - deterministic reset/failure/recovery policy proof. */
#include <stdio.h>
#include "../../src/core/boot/boot_state.h"

static int checks, failures;
static void ok(int condition, const char *name)
{
    checks++;
    if (!condition) { failures++; printf("  FAIL  %s\n", name); }
}

static void reseal(struct zlos_boot_state *state)
{
    state->checksum = 0;
    state->checksum = zlos_boot_state_checksum(state);
}

int main(void)
{
    printf("boot_state_test - bounded current/previous/recovery policy\n\n");
    struct zlos_boot_state state;
    zlos_boot_state_default(&state);
    ok(sizeof(state) == 64, "state record is exactly 64 bytes");
    ok(zlos_boot_state_validate(&state) == ZLOS_BOOT_OK,
       "fresh state validates");

    ok(zlos_boot_state_prepare(&state, ZLOS_BOOT_HAVE_ALL) == ZLOS_BOOT_OK,
       "first selection succeeds");
    ok(state.selected_generation == ZLOS_BOOT_GENERATION_CURRENT,
       "current image is preferred");
    ok(state.attempt_ordinal == 1 &&
       (state.flags & ZLOS_BOOT_STATE_PENDING),
       "current attempt 1 is durable and pending");

    /* Simulate reset before ready twice: the next boot observes each pending
     * record and advances only then. */
    ok(zlos_boot_state_prepare(&state, ZLOS_BOOT_HAVE_ALL) == ZLOS_BOOT_OK,
       "second boot observes first failure");
    ok(state.selected_generation == ZLOS_BOOT_GENERATION_CURRENT &&
       state.attempt_ordinal == 2 && state.current_failures == 1,
       "current gets its second and final ordinary attempt");
    ok(zlos_boot_state_prepare(&state, ZLOS_BOOT_HAVE_ALL) == ZLOS_BOOT_OK,
       "third boot observes second current failure");
    ok(state.selected_generation == ZLOS_BOOT_GENERATION_PREVIOUS &&
       state.attempt_ordinal == 1 && state.current_failures == 2,
       "policy falls back to previous after two current failures");

    ok(zlos_boot_state_prepare(&state, ZLOS_BOOT_HAVE_ALL) == ZLOS_BOOT_OK,
       "previous second attempt prepared");
    ok(zlos_boot_state_prepare(&state, ZLOS_BOOT_HAVE_ALL) == ZLOS_BOOT_OK,
       "previous exhaustion observed");
    ok(state.selected_generation == ZLOS_BOOT_GENERATION_RECOVERY,
       "policy falls back to recovery after previous is exhausted");

    ok(zlos_boot_state_mark_ready(&state) == ZLOS_BOOT_OK,
       "recovery kernel can mark the ready boundary");
    ok((state.flags & ZLOS_BOOT_STATE_READY) &&
       !(state.flags & ZLOS_BOOT_STATE_PENDING),
       "ready and pending are mutually exclusive");
    ok(state.recovery_failures == 0, "ready clears that generation's failures");

    ok(zlos_boot_state_prepare(&state, ZLOS_BOOT_HAVE_ALL) == ZLOS_BOOT_OK,
       "boot after ready consumes the success state");
    ok(state.selected_generation == ZLOS_BOOT_GENERATION_RECOVERY,
       "exhausted current/previous stay quarantined after recovery succeeds");

    zlos_boot_state_default(&state);
    ok(zlos_boot_state_prepare(&state, ZLOS_BOOT_HAVE_CURRENT) == ZLOS_BOOT_OK,
       "an ordinary image with no optional backups still boots");
    ok(zlos_boot_state_prepare(&state, ZLOS_BOOT_HAVE_CURRENT) == ZLOS_BOOT_OK,
       "single-image second attempt still boots");
    ok(zlos_boot_state_prepare(&state, ZLOS_BOOT_HAVE_CURRENT) == ZLOS_BOOT_OK,
       "exhaustion cannot brick a single-image device");
    ok(state.selected_generation == ZLOS_BOOT_GENERATION_CURRENT &&
       (state.flags & ZLOS_BOOT_STATE_DEGRADED),
       "single-image fallback is explicit and degraded");

    zlos_boot_state_default(&state);
    ok(zlos_boot_state_prepare(&state, ZLOS_BOOT_HAVE_PREVIOUS) == ZLOS_BOOT_OK,
       "missing current selects an available previous image");
    ok(state.selected_generation == ZLOS_BOOT_GENERATION_PREVIOUS,
       "presence, not a filename assumption, controls selection");

    struct zlos_boot_state changed = state;
    changed.flags |= ZLOS_BOOT_STATE_READY;
    reseal(&changed);
    ok(zlos_boot_state_validate(&changed) == ZLOS_BOOT_E_STATE,
       "pending plus ready is rejected even with a valid checksum");
    changed = state;
    changed.reserved[7] = 1;
    reseal(&changed);
    ok(zlos_boot_state_validate(&changed) == ZLOS_BOOT_E_UNSUPPORTED,
       "non-zero reserved state is rejected");

    for (unsigned int byte = 0; byte < sizeof(state); byte++) {
        changed = state;
        ((unsigned char *)&changed)[byte] ^= 1U;
        if (zlos_boot_state_validate(&changed) == ZLOS_BOOT_OK) {
            failures++;
            printf("  FAIL  state byte %u escaped validation/checksum\n", byte);
        }
        checks++;
    }

    ok(zlos_boot_state_prepare(&state, 0) == ZLOS_BOOT_E_ARGUMENT,
       "no available image is rejected without mutating policy");
    ok(zlos_boot_state_prepare(&state, 1U << 31) == ZLOS_BOOT_E_ARGUMENT,
       "unknown candidate bits are rejected");

    printf("\n  %d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
