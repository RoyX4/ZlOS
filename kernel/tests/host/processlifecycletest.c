#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "process_lifecycle.h"

#define TEST_SLOTS 4U

static struct process_lifecycle_slot slots[TEST_SLOTS];
static struct process_lifecycle_table table;
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
    expect(process_lifecycle_init(&table, slots, TEST_SLOTS) ==
           PROCESS_LIFECYCLE_OK, "table initializes");
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_OK,
           "initialized table is valid");
}

static void test_arguments(void)
{
    struct process_lifecycle_table empty = {0};
    process_lifecycle_handle handle = 0;
    struct process_lifecycle_snapshot snapshot;
    expect(process_lifecycle_check(NULL) == PROCESS_LIFECYCLE_E_ARGUMENT,
           "null table check refused");
    expect(process_lifecycle_check(&empty) == PROCESS_LIFECYCLE_E_ARGUMENT,
           "empty table check refused");
    expect(process_lifecycle_init(NULL, slots, TEST_SLOTS) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "null table init refused");
    expect(process_lifecycle_init(&table, NULL, TEST_SLOTS) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "null slots init refused");
    expect(process_lifecycle_init(&table, slots, 0) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "zero capacity refused");
    reset_table();
    expect(process_lifecycle_create(&table, 0, 0, &handle) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "zero process id refused");
    expect(process_lifecycle_create(&table, 1, 0, NULL) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "null create result refused");
    expect(process_lifecycle_create_at(&table, TEST_SLOTS, 1, 0, &handle) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "out-of-range slot refused");
    expect(process_lifecycle_exit(&table, 0, 0) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "zero exit handle refused");
    expect(process_lifecycle_fault(&table, 0, 0, 0, 0) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "zero fault handle refused");
    expect(process_lifecycle_snapshot(&table, 0, &snapshot) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "zero snapshot handle refused");
    expect(process_lifecycle_observe(&table, 0, 0, &snapshot) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "zero observe child refused");
    expect(process_lifecycle_reap(&table, 0, 0) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "zero reap child refused");
    expect(process_lifecycle_find_runnable(&table, 0, &handle) ==
           PROCESS_LIFECYCLE_E_ARGUMENT, "zero runnable id refused");
}

static void test_exit_and_reuse(void)
{
    process_lifecycle_handle root = 0, child = 0, observer = 0, found = 0;
    struct process_lifecycle_snapshot snapshot = {0};
    reset_table();
    expect(process_lifecycle_create_at(&table, 0, 1, 0, &root) ==
           PROCESS_LIFECYCLE_OK, "root created in exact slot");
    expect(root == (1ULL << 32 | 1ULL),
           "root handle binds generation one and slot zero");
    expect(process_lifecycle_create(&table, 2, root, &child) ==
           PROCESS_LIFECYCLE_OK, "child created under root");
    expect(child == (1ULL << 32 | 2ULL),
           "child handle binds its own slot and generation");
    expect(process_lifecycle_create(&table, 2, root, &observer) ==
           PROCESS_LIFECYCLE_E_DUPLICATE, "live process id cannot alias");
    expect(process_lifecycle_create_at(&table, 1, 3, root, &observer) ==
           PROCESS_LIFECYCLE_E_STATE, "occupied slot cannot be replaced");
    expect(process_lifecycle_find_runnable(&table, 2, &found) ==
           PROCESS_LIFECYCLE_OK && found == child,
           "runnable id resolves to exact handle");
    expect(process_lifecycle_observe(&table, root, child, &snapshot) ==
           PROCESS_LIFECYCLE_E_PENDING, "live child is not waitable");
    expect(process_lifecycle_exit(&table, child, -7) ==
           PROCESS_LIFECYCLE_OK, "child records signed exit status");
    expect(process_lifecycle_find_runnable(&table, 2, &found) ==
           PROCESS_LIFECYCLE_E_NOT_FOUND, "exited child is not runnable");
    expect(process_lifecycle_exit(&table, child, 0) ==
           PROCESS_LIFECYCLE_E_STATE, "double exit refused");
    expect(process_lifecycle_fault(&table, child, 13, 0, 0) ==
           PROCESS_LIFECYCLE_E_STATE, "exit cannot become a fault");
    expect(process_lifecycle_observe(&table, 0, child, &snapshot) ==
           PROCESS_LIFECYCLE_E_PERMISSION, "foreign waiter refused");
    expect(process_lifecycle_observe(&table, root, child, &snapshot) ==
           PROCESS_LIFECYCLE_OK, "parent observes exited child");
    expect(snapshot.handle == child && snapshot.id == 2 &&
           snapshot.parent == root &&
           snapshot.state == PROCESS_LIFECYCLE_EXITED,
           "exit snapshot preserves exact identities");
    expect(snapshot.termination.kind == PROCESS_LIFECYCLE_TERMINATION_EXIT &&
           snapshot.termination.exit_status == -7 &&
           snapshot.termination.fault_vector == 0 &&
           snapshot.termination.fault_error == 0 &&
           snapshot.termination.fault_address == 0,
           "exit snapshot cannot be confused with a fault");
    expect(process_lifecycle_reap(&table, 0, child) ==
           PROCESS_LIFECYCLE_E_PERMISSION, "foreign reaper refused");
    expect(process_lifecycle_reap(&table, root, child) ==
           PROCESS_LIFECYCLE_OK, "parent reaps exited child");
    expect(process_lifecycle_snapshot(&table, child, &snapshot) ==
           PROCESS_LIFECYCLE_E_STALE, "reaped handle becomes stale");
    expect(process_lifecycle_exit(&table, child, 0) ==
           PROCESS_LIFECYCLE_E_STALE, "stale handle cannot terminate reuse");
    expect(process_lifecycle_reap(&table, root, child) ==
           PROCESS_LIFECYCLE_E_STALE, "stale handle cannot reap twice");

    process_lifecycle_handle replacement = 0;
    expect(process_lifecycle_create_at(&table, 1, 2, root, &replacement) ==
           PROCESS_LIFECYCLE_OK, "reaped slot can reuse process id");
    expect(replacement == (2ULL << 32 | 2ULL) && replacement != child,
           "slot reuse advances generation");
    expect(process_lifecycle_snapshot(&table, child, &snapshot) ==
           PROCESS_LIFECYCLE_E_STALE,
           "old handle cannot observe replacement generation");
    expect(process_lifecycle_snapshot(&table, replacement, &snapshot) ==
           PROCESS_LIFECYCLE_OK && snapshot.id == 2,
           "replacement handle observes replacement only");
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_OK,
           "exit and reuse preserve table invariants");
}

static void test_fault_and_parent_order(void)
{
    process_lifecycle_handle root = 0, child = 0;
    struct process_lifecycle_snapshot snapshot = {0};
    reset_table();
    expect(process_lifecycle_create(&table, 10, 0, &root) ==
           PROCESS_LIFECYCLE_OK, "fault-test root created");
    expect(process_lifecycle_create(&table, 11, root, &child) ==
           PROCESS_LIFECYCLE_OK, "fault-test child created");
    expect(process_lifecycle_fault(&table, child, 0, 0x4,
                                   0x123456789abcdef0ULL) ==
           PROCESS_LIFECYCLE_OK, "vector zero fault is representable");
    expect(process_lifecycle_observe(&table, root, child, &snapshot) ==
           PROCESS_LIFECYCLE_OK, "parent observes faulted child");
    expect(snapshot.state == PROCESS_LIFECYCLE_FAULTED &&
           snapshot.termination.kind == PROCESS_LIFECYCLE_TERMINATION_FAULT &&
           snapshot.termination.exit_status == 0 &&
           snapshot.termination.fault_vector == 0 &&
           snapshot.termination.fault_error == 0x4 &&
           snapshot.termination.fault_address == 0x123456789abcdef0ULL,
           "fault snapshot preserves vector, error and address exactly");
    expect(process_lifecycle_exit(&table, root, 3) ==
           PROCESS_LIFECYCLE_OK, "parent may exit before child is reaped");
    expect(process_lifecycle_reap(&table, 0, root) ==
           PROCESS_LIFECYCLE_E_STATE,
           "parent cannot be reaped while child identity refers to it");
    expect(process_lifecycle_reap(&table, root, child) ==
           PROCESS_LIFECYCLE_OK,
           "exited parent retains authority to reap its child");
    expect(process_lifecycle_reap(&table, 0, root) ==
           PROCESS_LIFECYCLE_OK, "root reaps after child relationship closes");
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_OK,
           "ordered parent teardown restores valid table");
}

static void test_capacity_and_generation_exhaustion(void)
{
    process_lifecycle_handle handles[TEST_SLOTS] = {0};
    struct process_lifecycle_snapshot snapshot = {0};
    reset_table();
    for (unsigned int i = 0; i < TEST_SLOTS; i++)
        expect(process_lifecycle_create(&table, 100 + i, 0, &handles[i]) ==
               PROCESS_LIFECYCLE_OK, "each bounded slot admits one root");
    process_lifecycle_handle extra = 0;
    expect(process_lifecycle_create(&table, 999, 0, &extra) ==
           PROCESS_LIFECYCLE_E_CAPACITY, "full table refuses extra process");
    expect(process_lifecycle_create(&table, 100, 0, &extra) ==
           PROCESS_LIFECYCLE_E_DUPLICATE,
           "full table still reports a duplicate live process id");
    expect(process_lifecycle_create(&table, 999, 0, NULL) ==
           PROCESS_LIFECYCLE_E_ARGUMENT,
           "full table still validates the result pointer");
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_OK,
           "full table remains valid");
    slots[1].id = slots[0].id;
    expect(process_lifecycle_create(&table, 999, 0, &extra) ==
           PROCESS_LIFECYCLE_E_STATE,
           "full corrupted table is refused before capacity reporting");
    slots[1].id = 101;
    expect(process_lifecycle_exit(&table, handles[2], 0) ==
           PROCESS_LIFECYCLE_OK, "capacity test process exits");
    expect(process_lifecycle_reap(&table, 0, handles[2]) ==
           PROCESS_LIFECYCLE_OK, "capacity test process reaps");
    slots[2].generation = UINT_MAX;
    expect(process_lifecycle_create_at(&table, 2, 999, 0, &extra) ==
           PROCESS_LIFECYCLE_E_GENERATION,
           "exhausted generation cannot alias an ancient handle");
    expect(process_lifecycle_create(&table, 999, 0, &extra) ==
           PROCESS_LIFECYCLE_E_CAPACITY,
           "table with no reusable slot reports bounded capacity");
    expect(process_lifecycle_snapshot(&table, handles[2], &snapshot) ==
           PROCESS_LIFECYCLE_E_STALE,
           "exhausted slot leaves its old handle stale");
    expect(process_lifecycle_exit(&table, handles[3], 0) ==
           PROCESS_LIFECYCLE_OK, "neighbor process exits for skip test");
    expect(process_lifecycle_reap(&table, 0, handles[3]) ==
           PROCESS_LIFECYCLE_OK, "neighbor slot reaps for skip test");
    expect(process_lifecycle_create(&table, 999, 0, &extra) ==
           PROCESS_LIFECYCLE_OK && extra == (2ULL << 32 | 4ULL),
           "allocator skips exhausted slot and advances a reusable one");
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_OK,
           "generation exhaustion preserves invariants");
}

static void test_corruption_detection(void)
{
    process_lifecycle_handle root = 0, child = 0;
    struct process_lifecycle_snapshot snapshot;
    reset_table();
    expect(process_lifecycle_create(&table, 1, 0, &root) ==
           PROCESS_LIFECYCLE_OK, "corruption root created");
    expect(process_lifecycle_create(&table, 2, root, &child) ==
           PROCESS_LIFECYCLE_OK, "corruption child created");

    slots[1].parent = child;
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_E_STATE,
           "self-parent corruption detected");
    slots[1].parent = root;
    slots[1].id = 1;
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_E_DUPLICATE,
           "duplicate live id corruption detected");
    slots[1].id = 2;
    slots[1].termination.kind = PROCESS_LIFECYCLE_TERMINATION_EXIT;
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_E_STATE,
           "runnable process cannot carry termination");
    slots[1].termination.kind = PROCESS_LIFECYCLE_TERMINATION_NONE;
    slots[1].parent = 0x0000000200000001ULL;
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_E_STATE,
           "stale parent generation detected");
    slots[1].parent = root;
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_OK,
           "repaired corruption restores valid table");
    slots[0].parent = child;
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_E_STATE,
           "multi-process parent cycle detected");
    slots[0].parent = 0;
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_OK,
           "repaired parent cycle restores valid table");
    slots[0].parent = child;
    slots[1].parent = 0x0000000200000001ULL;
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_E_STATE,
           "stale ancestor chain is refused without dereference");
    expect(process_lifecycle_snapshot(&table, root, &snapshot) ==
           PROCESS_LIFECYCLE_E_STATE,
           "read operation fails closed on global corruption");
    expect(process_lifecycle_exit(&table, root, 0) ==
           PROCESS_LIFECYCLE_E_STATE,
           "mutation fails closed on global corruption");
    slots[0].parent = 0;
    slots[1].parent = root;
    expect(process_lifecycle_check(&table) == PROCESS_LIFECYCLE_OK,
           "repaired ancestor chain restores valid table");
}

int main(void)
{
    test_arguments();
    test_exit_and_reuse();
    test_fault_and_parent_order();
    test_capacity_and_generation_exhaustion();
    test_corruption_detection();
    printf("processlifecycletest: %d checks, %d failures\n", checks, failures);
    if (!failures)
        puts("process handles reject stale generations and preserve exact exit/fault custody");
    return failures != 0;
}
