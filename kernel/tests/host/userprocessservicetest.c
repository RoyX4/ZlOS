#include <stdio.h>
#include <string.h>

#include "process_lifecycle.h"
#include "scheduler_policy.h"
#include "user_process_service.h"

#define LIFE_SLOTS 3U
#define SCHED_SLOTS 2U

enum fake_action {
    FAKE_YIELD = 0,
    FAKE_EXIT,
    FAKE_FAULT,
    FAKE_REAP,
    FAKE_CORRUPT_POLICY,
    FAKE_SLEEP
};

struct fake_process {
    process_lifecycle_handle handle;
    unsigned int action;
    unsigned int runs;
    int fail;
};

struct fake_runner {
    struct process_lifecycle_table *lifecycle;
    struct user_process_service *service;
    struct fake_process processes[LIFE_SLOTS];
    unsigned int sleep_now;
    unsigned int sleep_delay;
};

static struct process_lifecycle_slot lifecycle_slots[LIFE_SLOTS];
static struct process_lifecycle_table lifecycle;
static struct scheduler_policy_slot scheduler_slots[SCHED_SLOTS];
static struct user_process_service service;
static struct fake_runner runner;
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

static struct fake_process *fake_find(struct fake_runner *fake,
                                      process_lifecycle_handle handle)
{
    for (unsigned int i = 0; i < LIFE_SLOTS; i++)
        if (fake->processes[i].handle == handle)
            return &fake->processes[i];
    return NULL;
}

static int fake_step(void *context, process_lifecycle_handle handle,
                     user_process_service_u64 *elapsed_ticks)
{
    struct fake_runner *fake = context;
    struct fake_process *process = fake_find(fake, handle);
    if (!process || process->fail) return -1;
    process->runs++;
    *elapsed_ticks = process->runs;
    if (process->action == FAKE_SLEEP) {
        process->action = FAKE_YIELD;
        expect(user_process_service_request_sleep(fake->service, handle,
                   fake->sleep_now, 0) == USER_PROCESS_SERVICE_E_ARGUMENT,
               "zero sleep is rejected without yielding");
        expect(user_process_service_request_sleep(fake->service, handle,
                   fake->sleep_now, 0x80000000U) == USER_PROCESS_SERVICE_E_ARGUMENT,
               "ambiguous half-wrap sleep is rejected");
        expect(user_process_service_request_sleep(fake->service,
                   handle ^ (1ULL << 32), fake->sleep_now, 1) ==
                   USER_PROCESS_SERVICE_E_NOT_FOUND,
               "foreign generation cannot request running owner's sleep");
        if (fake->processes[1].handle)
            expect(user_process_service_request_sleep(fake->service,
                       fake->processes[1].handle, fake->sleep_now, 1) ==
                       USER_PROCESS_SERVICE_E_STATE,
                   "runnable sibling cannot request current owner's sleep");
        int status = user_process_service_request_sleep(fake->service, handle,
            fake->sleep_now, fake->sleep_delay);
        expect(user_process_service_request_sleep(fake->service, handle,
                   fake->sleep_now + 7, 1) == USER_PROCESS_SERVICE_E_PENDING,
               "second request cannot overwrite the first deadline");
        return status;
    }
    if (process->action == FAKE_EXIT)
        return process_lifecycle_exit(fake->lifecycle, handle, 7);
    if (process->action == FAKE_FAULT)
        return process_lifecycle_fault(fake->lifecycle, handle,
                                       14, 6, 0x12345000ULL);
    if (process->action == FAKE_REAP) {
        if (process_lifecycle_exit(fake->lifecycle, handle, 8) !=
            PROCESS_LIFECYCLE_OK)
            return -1;
        return process_lifecycle_reap(fake->lifecycle, 0, handle);
    }
    if (process->action == FAKE_CORRUPT_POLICY)
        fake->service->scheduler.cursor =
            fake->service->scheduler.capacity;
    return 0;
}

static void reset_service(void)
{
    memset(&service, 0xa5, sizeof service);
    memset(&runner, 0, sizeof runner);
    expect(process_lifecycle_init(&lifecycle, lifecycle_slots, LIFE_SLOTS) ==
           PROCESS_LIFECYCLE_OK, "lifecycle initializes");
    runner.lifecycle = &lifecycle;
    expect(user_process_service_init(&service, &lifecycle, scheduler_slots,
                                     SCHED_SLOTS, fake_step, &runner) ==
           USER_PROCESS_SERVICE_OK, "service initializes");
    runner.service = &service;
    expect(user_process_service_check(&service) == USER_PROCESS_SERVICE_OK,
           "initialized service is valid");
}

static process_lifecycle_handle create_process(unsigned int slot,
                                               unsigned int pid)
{
    process_lifecycle_handle handle = 0;
    expect(process_lifecycle_create_at(&lifecycle, slot, pid, 0, &handle) ==
           PROCESS_LIFECYCLE_OK, "fake process identity created");
    runner.processes[slot].handle = handle;
    runner.processes[slot].action = FAKE_YIELD;
    runner.processes[slot].runs = 0;
    runner.processes[slot].fail = 0;
    return handle;
}

static void test_arguments(void)
{
    struct user_process_service empty = {0};
    struct scheduler_policy_slot extra[LIFE_SLOTS + 1U];
    process_lifecycle_handle dispatched = 0;
    expect(user_process_service_check(NULL) == USER_PROCESS_SERVICE_E_ARGUMENT,
           "null service check refused");
    expect(user_process_service_check(&empty) == USER_PROCESS_SERVICE_E_ARGUMENT,
           "empty service check refused");
    expect(process_lifecycle_init(&lifecycle, lifecycle_slots, LIFE_SLOTS) ==
           PROCESS_LIFECYCLE_OK, "argument lifecycle initializes");
    expect(user_process_service_init(NULL, &lifecycle, scheduler_slots,
                                     SCHED_SLOTS, fake_step, &runner) ==
           USER_PROCESS_SERVICE_E_ARGUMENT, "null service init refused");
    expect(user_process_service_init(&service, &lifecycle, scheduler_slots,
                                     LIFE_SLOTS + 1U, fake_step, &runner) ==
           USER_PROCESS_SERVICE_E_ARGUMENT,
           "scheduler capacity above lifecycle capacity refused");
    expect(user_process_service_init(&service, &lifecycle, extra,
                                     LIFE_SLOTS, NULL, &runner) ==
           USER_PROCESS_SERVICE_E_ARGUMENT, "null runner refused");
    reset_service();
    expect(user_process_service_admit(&service, 0) ==
           USER_PROCESS_SERVICE_E_ARGUMENT, "zero handle admission refused");
    expect(user_process_service_work(&service, 0, NULL) ==
           USER_PROCESS_SERVICE_E_ARGUMENT, "null dispatch output refused");
    expect(user_process_service_detach_terminal(&service, 0) ==
           USER_PROCESS_SERVICE_E_ARGUMENT, "zero detach handle refused");
    dispatched = 0xfeedULL;
    expect(user_process_service_work(&service, 0, &dispatched) ==
           USER_PROCESS_SERVICE_E_IDLE && dispatched == 0xfeedULL,
           "idle service leaves dispatch output untouched");
}

static void test_fairness_exit_fault_and_reuse(void)
{
    struct scheduler_policy_snapshot policy;
    struct process_lifecycle_snapshot custody;
    process_lifecycle_handle dispatched = 0;
    reset_service();
    process_lifecycle_handle first = create_process(0, 10);
    process_lifecycle_handle second = create_process(1, 20);
    process_lifecycle_handle third = create_process(2, 30);
    expect(user_process_service_admit(&service, first) ==
           USER_PROCESS_SERVICE_OK, "first exact handle admitted");
    expect(user_process_service_admit(&service, second) ==
           USER_PROCESS_SERVICE_OK, "second exact handle admitted");
    expect(user_process_service_admit(&service, first) ==
           USER_PROCESS_SERVICE_E_DUPLICATE, "duplicate handle refused");
    expect(user_process_service_admit(&service, third) ==
           USER_PROCESS_SERVICE_E_CAPACITY, "bounded scheduler refuses third");

    expect(user_process_service_work(&service, 100, &dispatched) ==
           USER_PROCESS_SERVICE_OK && dispatched == first,
           "first work call dispatches first owner");
    expect(user_process_service_work(&service, 101, &dispatched) ==
           USER_PROCESS_SERVICE_OK && dispatched == second,
           "second work call dispatches second owner");
    expect(scheduler_policy_snapshot(&service.scheduler, first, &policy) ==
           SCHEDULER_POLICY_OK && policy.run_ticks == 1 &&
           policy.dispatches == 1, "first yield accounting is exact");
    expect(scheduler_policy_snapshot(&service.scheduler, second, &policy) ==
           SCHEDULER_POLICY_OK && policy.run_ticks == 1 &&
           policy.dispatches == 1, "second yield accounting is exact");
    expect(user_process_service_detach_terminal(&service, first) ==
           USER_PROCESS_SERVICE_E_PENDING,
           "runnable process cannot detach as terminal");

    runner.processes[0].action = FAKE_EXIT;
    expect(user_process_service_work(&service, 102, &dispatched) ==
           USER_PROCESS_SERVICE_OK && dispatched == first,
           "exiting process receives its fair turn");
    expect(process_lifecycle_observe(&lifecycle, 0, first, &custody) ==
           PROCESS_LIFECYCLE_OK &&
           custody.termination.kind == PROCESS_LIFECYCLE_TERMINATION_EXIT &&
           custody.termination.exit_status == 7,
           "exit custody remains exact before detach");
    expect(scheduler_policy_snapshot(&service.scheduler, first, &policy) ==
           SCHEDULER_POLICY_OK && policy.state == SCHEDULER_POLICY_EXITED &&
           policy.run_ticks == 3, "exit turn is charged before terminal state");

    runner.processes[1].action = FAKE_FAULT;
    expect(user_process_service_work(&service, 103, &dispatched) ==
           USER_PROCESS_SERVICE_OK && dispatched == second,
           "faulting sibling remains independently schedulable");
    expect(process_lifecycle_observe(&lifecycle, 0, second, &custody) ==
           PROCESS_LIFECYCLE_OK &&
           custody.termination.kind == PROCESS_LIFECYCLE_TERMINATION_FAULT &&
           custody.termination.fault_vector == 14 &&
           custody.termination.fault_error == 6 &&
           custody.termination.fault_address == 0x12345000ULL,
           "fault custody remains exact before detach");
    expect(user_process_service_check(&service) == USER_PROCESS_SERVICE_OK,
           "terminal lifecycle and policy states agree");
    expect(service.work_calls == 4, "one work count is charged per exact turn");

    expect(user_process_service_detach_terminal(&service, first) ==
           USER_PROCESS_SERVICE_OK, "exited owner detaches from scheduler");
    expect(scheduler_policy_snapshot(&service.scheduler, first, &policy) ==
           SCHEDULER_POLICY_E_NOT_FOUND, "detached owner leaves policy table");
    expect(process_lifecycle_reap(&lifecycle, 0, first) ==
           PROCESS_LIFECYCLE_OK, "observed exited owner reaps from lifecycle");
    process_lifecycle_handle replacement = create_process(0, 40);
    expect(replacement != first, "slot reuse advances exact generation");
    expect(user_process_service_admit(&service, first) ==
           USER_PROCESS_SERVICE_E_NOT_FOUND, "stale generation cannot re-enter");
    expect(user_process_service_admit(&service, replacement) ==
           USER_PROCESS_SERVICE_OK, "replacement generation uses freed capacity");
    expect(user_process_service_detach_terminal(&service, second) ==
           USER_PROCESS_SERVICE_OK, "faulted owner detaches from scheduler");
    expect(process_lifecycle_reap(&lifecycle, 0, second) ==
           PROCESS_LIFECYCLE_OK, "observed faulted owner reaps from lifecycle");
    expect(user_process_service_check(&service) == USER_PROCESS_SERVICE_OK,
           "generation reuse preserves combined invariants");
}

static void test_fail_stop_boundaries(void)
{
    process_lifecycle_handle dispatched = 0xbeefULL;
    reset_service();
    process_lifecycle_handle handle = create_process(0, 50);
    expect(user_process_service_admit(&service, handle) ==
           USER_PROCESS_SERVICE_OK, "runner-failure owner admitted");
    runner.processes[0].fail = 1;
    expect(user_process_service_work(&service, 0, &dispatched) ==
           USER_PROCESS_SERVICE_E_RUNNER && dispatched == 0xbeefULL,
           "runner failure latches without publishing dispatch output");
    expect(service.failed && service.last_error ==
           USER_PROCESS_SERVICE_E_RUNNER,
           "runner failure records exact fail-stop reason");
    expect(user_process_service_work(&service, 1, &dispatched) ==
           USER_PROCESS_SERVICE_E_STATE,
           "failed service refuses subsequent work");

    reset_service();
    handle = create_process(0, 60);
    expect(user_process_service_admit(&service, handle) ==
           USER_PROCESS_SERVICE_OK, "drift owner admitted");
    expect(process_lifecycle_exit(&lifecycle, handle, 9) ==
           PROCESS_LIFECYCLE_OK, "external lifecycle drift injected");
    expect(user_process_service_check(&service) ==
           USER_PROCESS_SERVICE_E_STATE,
           "lifecycle-policy drift is detected before dispatch");
    expect(user_process_service_work(&service, 0, &dispatched) ==
           USER_PROCESS_SERVICE_E_STATE && service.failed,
           "detected drift fail-stops the service");

    reset_service();
    handle = create_process(0, 70);
    expect(user_process_service_admit(&service, handle) ==
           USER_PROCESS_SERVICE_OK, "corruption owner admitted");
    scheduler_slots[0].state = SCHEDULER_POLICY_EXITED;
    expect(user_process_service_check(&service) ==
           USER_PROCESS_SERVICE_E_STATE,
           "terminal scheduler state cannot masquerade as runnable");

    reset_service();
    handle = create_process(0, 71);
    expect(user_process_service_admit(&service, handle) ==
           USER_PROCESS_SERVICE_OK, "custody-destruction owner admitted");
    runner.processes[0].action = FAKE_REAP;
    expect(user_process_service_work(&service, 0, &dispatched) ==
           USER_PROCESS_SERVICE_E_STATE && service.failed,
           "mid-turn custody destruction fail-stops reconciliation");

    reset_service();
    handle = create_process(0, 72);
    expect(user_process_service_admit(&service, handle) ==
           USER_PROCESS_SERVICE_OK, "policy-corruption owner admitted");
    runner.processes[0].action = FAKE_CORRUPT_POLICY;
    expect(user_process_service_work(&service, 0, &dispatched) ==
           USER_PROCESS_SERVICE_E_STATE && service.failed,
           "mid-turn policy corruption fail-stops reconciliation");
}

static void test_admission_and_detach_boundaries(void)
{
    process_lifecycle_handle malformed = 1ULL << 32;
    reset_service();
    expect(user_process_service_admit(&service, malformed) ==
           USER_PROCESS_SERVICE_E_ARGUMENT,
           "malformed nonzero lifecycle handle is refused");

    process_lifecycle_handle handle = create_process(0, 73);
    expect(process_lifecycle_exit(&lifecycle, handle, 1) ==
           PROCESS_LIFECYCLE_OK, "unadmitted process exits");
    expect(user_process_service_admit(&service, handle) ==
           USER_PROCESS_SERVICE_E_STATE,
           "terminal lifecycle handle cannot be admitted");
    expect(user_process_service_detach_terminal(&service, handle) ==
           USER_PROCESS_SERVICE_E_NOT_FOUND,
           "terminal lifecycle without policy ownership cannot detach");

    reset_service();
    handle = create_process(0, 74);
    service.failed = 1;
    service.last_error = USER_PROCESS_SERVICE_E_RUNNER;
    expect(user_process_service_admit(&service, handle) ==
           USER_PROCESS_SERVICE_E_STATE,
           "failed service refuses admission");
    expect(user_process_service_detach_terminal(&service, handle) ==
           USER_PROCESS_SERVICE_E_STATE,
           "failed service refuses terminal detach");

    reset_service();
    handle = create_process(0, 75);
    lifecycle_slots[0].id = 0;
    expect(user_process_service_check(&service) ==
           USER_PROCESS_SERVICE_E_STATE,
           "corrupt lifecycle table fails the combined check");
}

static void test_saturating_work_count(void)
{
    process_lifecycle_handle dispatched = 0;
    reset_service();
    process_lifecycle_handle handle = create_process(0, 80);
    expect(user_process_service_admit(&service, handle) ==
           USER_PROCESS_SERVICE_OK, "saturation owner admitted");
    service.work_calls = 0xffffffffffffffffULL;
    expect(user_process_service_work(&service, 0, &dispatched) ==
           USER_PROCESS_SERVICE_OK && dispatched == handle,
           "saturated service still executes work");
    expect(service.work_calls == 0xffffffffffffffffULL,
           "service work count saturates instead of wrapping");
}

static void test_sleep_deadline_and_sibling(void)
{
    struct scheduler_policy_snapshot policy;
    process_lifecycle_handle dispatched = 0;
    reset_service();
    process_lifecycle_handle sleeper = create_process(0, 90);
    process_lifecycle_handle sibling = create_process(1, 91);
    expect(user_process_service_admit(&service, sleeper) ==
           USER_PROCESS_SERVICE_OK, "sleeping owner admitted");
    expect(user_process_service_admit(&service, sibling) ==
           USER_PROCESS_SERVICE_OK, "sleeping owner's sibling admitted");
    runner.sleep_now = 0xfffffffeU;
    runner.sleep_delay = 5;
    runner.processes[0].action = FAKE_SLEEP;
    runner.processes[1].action = FAKE_EXIT;
    expect(user_process_service_work(&service, 0xfffffffdU, &dispatched) ==
           USER_PROCESS_SERVICE_OK && dispatched == sleeper,
           "running owner requests a sleep crossing tick wrap");
    expect(scheduler_policy_snapshot(&service.scheduler, sleeper, &policy) ==
           SCHEDULER_POLICY_OK && policy.state == SCHEDULER_POLICY_SLEEPING &&
           policy.wake_at == 3 && policy.run_ticks == 1 && policy.dispatches == 1,
           "sleep begins at request time and charges the completed turn once");
    expect(user_process_service_check(&service) == USER_PROCESS_SERVICE_OK,
           "sleeping scheduler owner retains valid runnable lifecycle custody");
    expect(user_process_service_detach_terminal(&service, sleeper) ==
           USER_PROCESS_SERVICE_E_PENDING, "sleep cannot be reaped as exit");
    expect(user_process_service_work(&service, 0xffffffffU, &dispatched) ==
           USER_PROCESS_SERVICE_OK && dispatched == sibling,
           "sibling completes while the first process sleeps");
    dispatched = 0xfeedULL;
    expect(user_process_service_work(&service, 2, &dispatched) ==
           USER_PROCESS_SERVICE_E_IDLE && dispatched == 0xfeedULL &&
           runner.processes[0].runs == 1 && service.work_calls == 2,
           "before deadline no process runs or consumes a dispatch turn");
    runner.processes[0].action = FAKE_EXIT;
    expect(user_process_service_work(&service, 3, &dispatched) ==
           USER_PROCESS_SERVICE_OK && dispatched == sleeper,
           "exact wrapped deadline resumes the sleeping process");
    expect(scheduler_policy_snapshot(&service.scheduler, sleeper, &policy) ==
           SCHEDULER_POLICY_OK && policy.state == SCHEDULER_POLICY_EXITED &&
           policy.run_ticks == 3 && policy.dispatches == 2,
           "sleeping time is not charged as running time");
    expect(user_process_service_detach_terminal(&service, sleeper) ==
           USER_PROCESS_SERVICE_OK &&
           process_lifecycle_reap(&lifecycle, 0, sleeper) == PROCESS_LIFECYCLE_OK,
           "woken process can exit, detach and release exact identity");
    expect(user_process_service_request_sleep(&service, sleeper, 4, 1) ==
           USER_PROCESS_SERVICE_E_NOT_FOUND,
           "stale sleeping handle cannot affect another generation");
}

static void test_sleep_request_boundaries(void)
{
    process_lifecycle_handle dispatched = 0;
    reset_service();
    process_lifecycle_handle handle = create_process(0, 92);
    expect(user_process_service_request_sleep(NULL, handle, 0, 1) ==
           USER_PROCESS_SERVICE_E_ARGUMENT, "null sleep service rejected");
    expect(user_process_service_request_sleep(&service, 0, 0, 1) ==
           USER_PROCESS_SERVICE_E_ARGUMENT, "zero sleep identity rejected");
    expect(user_process_service_admit(&service, handle) ==
           USER_PROCESS_SERVICE_OK, "boundary sleeper admitted");
    expect(user_process_service_request_sleep(&service, handle, 0, 1) ==
           USER_PROCESS_SERVICE_E_STATE, "sleep outside runner rejected");
    runner.sleep_now = 0x80000001U;
    runner.sleep_delay = USER_PROCESS_SERVICE_MAX_SLEEP_TICKS;
    runner.processes[0].action = FAKE_SLEEP;
    expect(user_process_service_work(&service, runner.sleep_now, &dispatched) ==
           USER_PROCESS_SERVICE_OK, "maximum unambiguous sleep accepted");
    expect(user_process_service_work(&service, 0xffffffffU, &dispatched) ==
           USER_PROCESS_SERVICE_E_IDLE, "deadline zero does not mean ready early");
    expect(user_process_service_work(&service, 0, &dispatched) ==
           USER_PROCESS_SERVICE_OK, "deadline zero resumes at tick wrap");
    service.sleep_delay = 1;
    expect(user_process_service_check(&service) == USER_PROCESS_SERVICE_E_STATE,
           "pending sleep without a running owner is corruption");
    service.sleep_delay = 0;
    service.sleep_started = 1;
    expect(user_process_service_check(&service) == USER_PROCESS_SERVICE_E_STATE,
           "stray request timestamp is corruption");
}

int main(void)
{
    test_arguments();
    test_fairness_exit_fault_and_reuse();
    test_fail_stop_boundaries();
    test_admission_and_detach_boundaries();
    test_saturating_work_count();
    test_sleep_deadline_and_sibling();
    test_sleep_request_boundaries();
    printf("userprocessservicetest: %d checks, %d failures\n", checks, failures);
    if (!failures)
        puts("lifecycle custody and bounded dispatch remain generation-exact across every turn");
    return failures != 0;
}
