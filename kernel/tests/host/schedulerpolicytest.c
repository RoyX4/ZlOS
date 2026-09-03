#include <stdio.h>
#include <string.h>

#include "scheduler_policy.h"

#define TEST_SLOTS 4U

static struct scheduler_policy_slot slots[TEST_SLOTS];
static struct scheduler_policy_table table;
static int checks;
static int failures;

static void expect(int condition, const char *message)
{
    checks++;
    if (!condition) {
        failures++;
        printf("FAIL: %s\n", message);
    }
}

static void reset_table(void)
{
    memset(slots, 0xa5, sizeof slots);
    expect(scheduler_policy_init(&table, slots, TEST_SLOTS) ==
           SCHEDULER_POLICY_OK, "table initializes");
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_OK,
           "initialized table is valid");
}

static void test_arguments(void)
{
    struct scheduler_policy_table empty = {0};
    unsigned int slot = 0;
    scheduler_policy_owner owner = 0;
    struct scheduler_policy_snapshot snapshot;
    expect(scheduler_policy_check(NULL) == SCHEDULER_POLICY_E_ARGUMENT,
           "null table check refused");
    expect(scheduler_policy_check(&empty) == SCHEDULER_POLICY_E_ARGUMENT,
           "empty table check refused");
    expect(scheduler_policy_init(NULL, slots, TEST_SLOTS) ==
           SCHEDULER_POLICY_E_ARGUMENT, "null init table refused");
    expect(scheduler_policy_init(&table, NULL, TEST_SLOTS) ==
           SCHEDULER_POLICY_E_ARGUMENT, "null init slots refused");
    expect(scheduler_policy_init(&table, slots, 0) ==
           SCHEDULER_POLICY_E_ARGUMENT, "zero capacity refused");
    reset_table();
    expect(scheduler_policy_admit(&table, 0, &slot) ==
           SCHEDULER_POLICY_E_ARGUMENT, "zero owner admission refused");
    expect(scheduler_policy_admit(&table, 1, NULL) ==
           SCHEDULER_POLICY_E_ARGUMENT, "null admission result refused");
    expect(scheduler_policy_dispatch(&table, 0, NULL) ==
           SCHEDULER_POLICY_E_ARGUMENT, "null dispatch result refused");
    expect(scheduler_policy_yield(&table, 0, 0) ==
           SCHEDULER_POLICY_E_ARGUMENT, "zero yield owner refused");
    expect(scheduler_policy_sleep(&table, 0, 0, 1, 0) ==
           SCHEDULER_POLICY_E_ARGUMENT, "zero sleep owner refused");
    expect(scheduler_policy_exit(&table, 0, 0) ==
           SCHEDULER_POLICY_E_ARGUMENT, "zero exit owner refused");
    expect(scheduler_policy_reap(&table, 0) ==
           SCHEDULER_POLICY_E_ARGUMENT, "zero reap owner refused");
    expect(scheduler_policy_snapshot(&table, 0, &snapshot) ==
           SCHEDULER_POLICY_E_ARGUMENT, "zero snapshot owner refused");
    expect(scheduler_policy_dispatch(&table, 0, &owner) ==
           SCHEDULER_POLICY_E_IDLE, "empty table dispatches idle");
}

static void test_admission_and_fairness(void)
{
    unsigned int admitted = 99;
    scheduler_policy_owner owner = 0;
    struct scheduler_policy_snapshot snapshot;
    reset_table();
    for (scheduler_policy_owner id = 1; id <= TEST_SLOTS; id++) {
        expect(scheduler_policy_admit(&table, id, &admitted) ==
               SCHEDULER_POLICY_OK, "bounded owner admitted");
        expect(admitted == id - 1, "admission selects first empty slot");
    }
    expect(scheduler_policy_admit(&table, 1, &admitted) ==
           SCHEDULER_POLICY_E_DUPLICATE,
           "duplicate owner wins over full capacity");
    expect(scheduler_policy_admit(&table, 99, &admitted) ==
           SCHEDULER_POLICY_E_CAPACITY, "full table refuses extra owner");

    for (scheduler_policy_owner id = 1; id <= TEST_SLOTS; id++) {
        expect(scheduler_policy_dispatch(&table, 100, &owner) ==
               SCHEDULER_POLICY_OK && owner == id,
               "round robin dispatches exact next owner");
        expect(scheduler_policy_dispatch(&table, 100, &owner) ==
               SCHEDULER_POLICY_E_STATE,
               "double dispatch without release is refused");
        expect(scheduler_policy_yield(&table, owner, id) ==
               SCHEDULER_POLICY_OK, "running owner yields");
    }
    expect(table.dispatches == TEST_SLOTS &&
           table.switches == TEST_SLOTS - 1,
           "dispatch and switch totals remain distinct");
    for (scheduler_policy_owner id = 1; id <= TEST_SLOTS; id++) {
        expect(scheduler_policy_snapshot(&table, id, &snapshot) ==
               SCHEDULER_POLICY_OK, "owner snapshot available");
        expect(snapshot.state == SCHEDULER_POLICY_RUNNABLE &&
               snapshot.run_ticks == id && snapshot.dispatches == 1,
               "snapshot retains exact accounting");
    }
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_OK,
           "fairness cycle preserves invariants");
}

static void test_sleep_wrap_exit_and_reap(void)
{
    unsigned int admitted = 0;
    scheduler_policy_owner owner = 0;
    struct scheduler_policy_snapshot snapshot;
    reset_table();
    expect(scheduler_policy_admit(&table, 11, &admitted) ==
           SCHEDULER_POLICY_OK, "sleeper admitted");
    expect(scheduler_policy_admit(&table, 22, &admitted) ==
           SCHEDULER_POLICY_OK, "sibling admitted");
    expect(scheduler_policy_dispatch(&table, 0xfffffffeU, &owner) ==
           SCHEDULER_POLICY_OK && owner == 11, "sleeper dispatched");
    expect(scheduler_policy_sleep(&table, 11, 0xfffffffeU, 4, 3) ==
           SCHEDULER_POLICY_OK, "sleep deadline crosses tick wrap");
    expect(scheduler_policy_snapshot(&table, 11, &snapshot) ==
           SCHEDULER_POLICY_OK &&
           snapshot.state == SCHEDULER_POLICY_SLEEPING &&
           snapshot.wake_at == 2 && snapshot.run_ticks == 3,
           "wrapped sleep snapshot is exact");
    expect(scheduler_policy_dispatch(&table, 0xffffffffU, &owner) ==
           SCHEDULER_POLICY_OK && owner == 22,
           "sibling runs before wrapped deadline");
    expect(scheduler_policy_yield(&table, 22, 5) ==
           SCHEDULER_POLICY_OK, "sibling yields before wake");
    expect(scheduler_policy_dispatch(&table, 1, &owner) ==
           SCHEDULER_POLICY_OK && owner == 22,
           "sleeper remains blocked one tick before deadline");
    expect(scheduler_policy_yield(&table, 22, 1) ==
           SCHEDULER_POLICY_OK, "sibling yields at pre-deadline");
    expect(scheduler_policy_dispatch(&table, 2, &owner) ==
           SCHEDULER_POLICY_OK && owner == 11,
           "sleeper wakes exactly at wrapped deadline");
    expect(scheduler_policy_sleep(&table, 11, 2, 0, 0) ==
           SCHEDULER_POLICY_E_ARGUMENT, "zero sleep delay refused");
    expect(scheduler_policy_exit(&table, 11, 7) ==
           SCHEDULER_POLICY_OK, "running owner exits with final accounting");
    expect(scheduler_policy_reap(&table, 22) ==
           SCHEDULER_POLICY_E_PENDING, "runnable owner cannot be reaped");
    expect(scheduler_policy_reap(&table, 11) ==
           SCHEDULER_POLICY_OK, "exited owner reaps");
    expect(scheduler_policy_snapshot(&table, 11, &snapshot) ==
           SCHEDULER_POLICY_E_NOT_FOUND, "reaped owner becomes absent");
    expect(scheduler_policy_admit(&table, 33, &admitted) ==
           SCHEDULER_POLICY_OK && admitted == 0,
           "reaped capacity admits a distinct exact owner");
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_OK,
           "sleep exit and reap preserve invariants");
}

static void test_saturation_and_corruption(void)
{
    unsigned int admitted = 0;
    scheduler_policy_owner owner = 0;
    struct scheduler_policy_snapshot snapshot;
    reset_table();
    expect(scheduler_policy_admit(&table, 7, &admitted) ==
           SCHEDULER_POLICY_OK, "saturation owner admitted");
    slots[0].run_ticks = 0xfffffffffffffffeULL;
    slots[0].dispatches = 0xffffffffffffffffULL;
    table.dispatches = 0xffffffffffffffffULL;
    table.last_owner = 7;
    expect(scheduler_policy_dispatch(&table, 0, &owner) ==
           SCHEDULER_POLICY_OK && owner == 7,
           "saturated counters do not block dispatch");
    expect(scheduler_policy_yield(&table, 7, 9) ==
           SCHEDULER_POLICY_OK, "saturated owner yields");
    expect(slots[0].run_ticks == 0xffffffffffffffffULL &&
           slots[0].dispatches == 0xffffffffffffffffULL &&
           table.dispatches == 0xffffffffffffffffULL,
           "all accounting saturates instead of wrapping");

    slots[0].state = SCHEDULER_POLICY_RUNNING;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_E_STATE,
           "running slot without table owner is detected");
    expect(scheduler_policy_snapshot(&table, 7, &snapshot) ==
           SCHEDULER_POLICY_E_STATE,
           "read operation fails closed on global corruption");
    slots[0].state = SCHEDULER_POLICY_RUNNABLE;
    slots[1].owner = 7;
    slots[1].state = SCHEDULER_POLICY_RUNNABLE;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_E_DUPLICATE,
           "duplicate owner corruption is detected");
    slots[1].owner = 0;
    slots[1].state = SCHEDULER_POLICY_EMPTY;
    slots[2].owner = 9;
    slots[2].state = SCHEDULER_POLICY_RUNNABLE;
    slots[2].wake_at = 1;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_E_STATE,
           "non-sleeper wake deadline corruption is detected");
    slots[2].owner = 0;
    slots[2].state = SCHEDULER_POLICY_EMPTY;
    slots[2].wake_at = 0;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_OK,
           "repaired corruption restores invariants");
}

static void test_fail_closed_corruption(void)
{
    unsigned int admitted = 0;
    scheduler_policy_owner owner = 0;

    reset_table();
    table.cursor = TEST_SLOTS;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_E_STATE,
           "out-of-range cursor is detected");

    reset_table();
    slots[0].owner = 1;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_E_STATE,
           "dirty empty slot is detected");

    reset_table();
    slots[0].state = SCHEDULER_POLICY_RUNNABLE;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_E_STATE,
           "occupied slot without owner is detected");

    reset_table();
    slots[0].owner = 1;
    slots[0].state = SCHEDULER_POLICY_RUNNING;
    slots[1].owner = 2;
    slots[1].state = SCHEDULER_POLICY_RUNNING;
    table.running = 0;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_E_STATE,
           "multiple running slots are detected");

    reset_table();
    table.dispatches = 1;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_E_STATE,
           "dispatch history without last owner is detected");

    reset_table();
    table.switches = 1;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_E_STATE,
           "switch history without dispatch is detected");

    reset_table();
    table.dispatches = 1;
    table.switches = 2;
    table.last_owner = 1;
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_E_STATE,
           "switch count above dispatch count is detected");

    reset_table();
    expect(scheduler_policy_admit(&table, 1, &admitted) ==
           SCHEDULER_POLICY_OK, "release corruption owner admitted");
    table.cursor = TEST_SLOTS;
    expect(scheduler_policy_yield(&table, 1, 0) ==
           SCHEDULER_POLICY_E_STATE,
           "release fails closed on global corruption");

    reset_table();
    expect(scheduler_policy_admit(&table, 1, &admitted) ==
           SCHEDULER_POLICY_OK, "non-running owner admitted");
    expect(scheduler_policy_yield(&table, 1, 0) ==
           SCHEDULER_POLICY_E_STATE,
           "release refuses a non-running owner");

    reset_table();
    table.cursor = TEST_SLOTS;
    expect(scheduler_policy_admit(&table, 1, &admitted) ==
           SCHEDULER_POLICY_E_STATE,
           "admission fails closed on global corruption");
    expect(scheduler_policy_dispatch(&table, 0, &owner) ==
           SCHEDULER_POLICY_E_STATE,
           "dispatch fails closed on global corruption");
    expect(scheduler_policy_reap(&table, 1) ==
           SCHEDULER_POLICY_E_STATE,
           "reap fails closed on global corruption");
}

static void test_idle_and_delay_clamp(void)
{
    unsigned int admitted = 0;
    scheduler_policy_owner owner = 0;
    struct scheduler_policy_snapshot snapshot;
    reset_table();
    expect(scheduler_policy_admit(&table, 44, &admitted) ==
           SCHEDULER_POLICY_OK, "single owner admitted");
    expect(scheduler_policy_dispatch(&table, 100, &owner) ==
           SCHEDULER_POLICY_OK && owner == 44,
           "single owner dispatched");
    expect(scheduler_policy_sleep(&table, 44, 100, 0xffffffffU, 2) ==
           SCHEDULER_POLICY_OK, "oversized delay is bounded");
    expect(scheduler_policy_snapshot(&table, 44, &snapshot) ==
           SCHEDULER_POLICY_OK && snapshot.wake_at == 0x80000063U,
           "bounded delay uses the maximum wrap-safe deadline");
    owner = 0xfeedULL;
    expect(scheduler_policy_dispatch(&table, 0x80000062U, &owner) ==
           SCHEDULER_POLICY_E_IDLE && owner == 0xfeedULL,
           "idle dispatch leaves output untouched before deadline");
    expect(scheduler_policy_dispatch(&table, 0x80000063U, &owner) ==
           SCHEDULER_POLICY_OK && owner == 44,
           "maximum delay wakes exactly at deadline");
    expect(scheduler_policy_yield(&table, 44, 1) ==
           SCHEDULER_POLICY_OK, "single owner yields");
    expect(scheduler_policy_dispatch(&table, 0x80000064U, &owner) ==
           SCHEDULER_POLICY_OK && owner == 44,
           "single owner redispatches");
    expect(table.dispatches == 3 && table.switches == 0,
           "redispatching one owner is not a context switch");
    expect(scheduler_policy_check(&table) == SCHEDULER_POLICY_OK,
           "idle and maximum delay preserve invariants");
}

int main(void)
{
    test_arguments();
    test_admission_and_fairness();
    test_sleep_wrap_exit_and_reap();
    test_saturation_and_corruption();
    test_fail_closed_corruption();
    test_idle_and_delay_clamp();
    printf("schedulerpolicytest: %d checks, %d failures\n", checks, failures);
    if (!failures)
        puts("bounded scheduler policy preserves fairness, deadlines and exact owner accounting");
    return failures != 0;
}
