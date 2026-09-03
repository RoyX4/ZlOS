/* user_process_service.c - reconcile process custody with scheduler state. */
#include "user_process_service.h"

#define SERVICE_U64_MAX 0xffffffffffffffffULL

static user_process_service_u64 add_one_saturated(
    user_process_service_u64 value)
{
    return value == SERVICE_U64_MAX ? value : value + 1;
}

static int service_shape(const struct user_process_service *service)
{
    return service && service->lifecycle && service->step &&
           service->scheduler.slots && service->scheduler.capacity &&
           service->scheduler.capacity <= service->lifecycle->capacity;
}

static int map_policy_status(int status)
{
    if (status == SCHEDULER_POLICY_E_CAPACITY)
        return USER_PROCESS_SERVICE_E_CAPACITY;
    if (status == SCHEDULER_POLICY_E_DUPLICATE)
        return USER_PROCESS_SERVICE_E_DUPLICATE;
    if (status == SCHEDULER_POLICY_E_NOT_FOUND)
        return USER_PROCESS_SERVICE_E_NOT_FOUND;
    if (status == SCHEDULER_POLICY_E_IDLE)
        return USER_PROCESS_SERVICE_E_IDLE;
    if (status == SCHEDULER_POLICY_E_PENDING)
        return USER_PROCESS_SERVICE_E_PENDING;
    if (status == SCHEDULER_POLICY_E_ARGUMENT)
        return USER_PROCESS_SERVICE_E_ARGUMENT;
    return USER_PROCESS_SERVICE_E_STATE;
}

static int fail_stop(struct user_process_service *service, int error)
{
    service->failed = 1;
    service->last_error = error;
    return error;
}

static int lifecycle_snapshot(
    const struct user_process_service *service,
    process_lifecycle_handle handle,
    struct process_lifecycle_snapshot *snapshot)
{
    int status = process_lifecycle_snapshot(service->lifecycle, handle,
                                            snapshot);
    if (status == PROCESS_LIFECYCLE_OK) return USER_PROCESS_SERVICE_OK;
    if (status == PROCESS_LIFECYCLE_E_ARGUMENT)
        return USER_PROCESS_SERVICE_E_ARGUMENT;
    if (status == PROCESS_LIFECYCLE_E_STALE)
        return USER_PROCESS_SERVICE_E_NOT_FOUND;
    return USER_PROCESS_SERVICE_E_STATE;
}

int user_process_service_check(const struct user_process_service *service)
{
    if (!service_shape(service)) return USER_PROCESS_SERVICE_E_ARGUMENT;
    if (service->failed || service->last_error)
        return USER_PROCESS_SERVICE_E_STATE;
    if (process_lifecycle_check(service->lifecycle) != PROCESS_LIFECYCLE_OK ||
        scheduler_policy_check(&service->scheduler) != SCHEDULER_POLICY_OK)
        return USER_PROCESS_SERVICE_E_STATE;

    for (unsigned int i = 0; i < service->scheduler.capacity; i++) {
        const struct scheduler_policy_slot *slot =
            &service->scheduler.slots[i];
        if (slot->state == SCHEDULER_POLICY_EMPTY) continue;
        struct process_lifecycle_snapshot snapshot;
        if (process_lifecycle_snapshot(service->lifecycle, slot->owner,
                                       &snapshot) != PROCESS_LIFECYCLE_OK)
            return USER_PROCESS_SERVICE_E_STATE;
        if (snapshot.state == PROCESS_LIFECYCLE_RUNNABLE) {
            if (slot->state != SCHEDULER_POLICY_RUNNABLE &&
                slot->state != SCHEDULER_POLICY_RUNNING)
                return USER_PROCESS_SERVICE_E_STATE;
        } else if (snapshot.state == PROCESS_LIFECYCLE_EXITED ||
                   snapshot.state == PROCESS_LIFECYCLE_FAULTED) {
            if (slot->state != SCHEDULER_POLICY_EXITED)
                return USER_PROCESS_SERVICE_E_STATE;
        } else {
            return USER_PROCESS_SERVICE_E_STATE;
        }
    }
    return USER_PROCESS_SERVICE_OK;
}

int user_process_service_init(
    struct user_process_service *service,
    struct process_lifecycle_table *lifecycle,
    struct scheduler_policy_slot *scheduler_slots,
    unsigned int capacity,
    user_process_service_step step,
    void *step_context)
{
    if (!service || !lifecycle || !scheduler_slots || !capacity || !step ||
        capacity > lifecycle->capacity ||
        process_lifecycle_check(lifecycle) != PROCESS_LIFECYCLE_OK)
        return USER_PROCESS_SERVICE_E_ARGUMENT;
    if (scheduler_policy_init(&service->scheduler, scheduler_slots,
                              capacity) != SCHEDULER_POLICY_OK)
        return USER_PROCESS_SERVICE_E_ARGUMENT;
    service->lifecycle = lifecycle;
    service->step = step;
    service->step_context = step_context;
    service->work_calls = 0;
    service->failed = 0;
    service->last_error = 0;
    return USER_PROCESS_SERVICE_OK;
}

int user_process_service_admit(struct user_process_service *service,
                               process_lifecycle_handle handle)
{
    if (!service_shape(service) || !handle)
        return USER_PROCESS_SERVICE_E_ARGUMENT;
    if (user_process_service_check(service) != USER_PROCESS_SERVICE_OK)
        return USER_PROCESS_SERVICE_E_STATE;
    struct process_lifecycle_snapshot snapshot;
    int status = lifecycle_snapshot(service, handle, &snapshot);
    if (status != USER_PROCESS_SERVICE_OK) return status;
    if (snapshot.state != PROCESS_LIFECYCLE_RUNNABLE)
        return USER_PROCESS_SERVICE_E_STATE;
    unsigned int slot = 0;
    status = scheduler_policy_admit(&service->scheduler, handle, &slot);
    (void)slot;
    return status == SCHEDULER_POLICY_OK ? USER_PROCESS_SERVICE_OK
                                         : map_policy_status(status);
}

int user_process_service_work(struct user_process_service *service,
                              scheduler_policy_u32 now,
                              process_lifecycle_handle *dispatched)
{
    if (!service_shape(service) || !dispatched)
        return USER_PROCESS_SERVICE_E_ARGUMENT;
    if (user_process_service_check(service) != USER_PROCESS_SERVICE_OK)
        return fail_stop(service, USER_PROCESS_SERVICE_E_STATE);

    scheduler_policy_owner owner = 0;
    int status = scheduler_policy_dispatch(&service->scheduler, now, &owner);
    if (status != SCHEDULER_POLICY_OK) return map_policy_status(status);

    user_process_service_u64 elapsed = 0;
    if (service->step(service->step_context, owner, &elapsed) != 0)
        return fail_stop(service, USER_PROCESS_SERVICE_E_RUNNER);

    struct process_lifecycle_snapshot snapshot;
    if (lifecycle_snapshot(service, owner, &snapshot) !=
        USER_PROCESS_SERVICE_OK)
        return fail_stop(service, USER_PROCESS_SERVICE_E_STATE);
    if (snapshot.state == PROCESS_LIFECYCLE_RUNNABLE) {
        status = scheduler_policy_yield(&service->scheduler, owner, elapsed);
    } else if (snapshot.state == PROCESS_LIFECYCLE_EXITED ||
               snapshot.state == PROCESS_LIFECYCLE_FAULTED) {
        status = scheduler_policy_exit(&service->scheduler, owner, elapsed);
    } else {
        return fail_stop(service, USER_PROCESS_SERVICE_E_STATE);
    }
    if (status != SCHEDULER_POLICY_OK)
        return fail_stop(service, USER_PROCESS_SERVICE_E_STATE);

    service->work_calls = add_one_saturated(service->work_calls);
    *dispatched = owner;
    return USER_PROCESS_SERVICE_OK;
}

int user_process_service_detach_terminal(
    struct user_process_service *service,
    process_lifecycle_handle handle)
{
    if (!service_shape(service) || !handle)
        return USER_PROCESS_SERVICE_E_ARGUMENT;
    if (user_process_service_check(service) != USER_PROCESS_SERVICE_OK)
        return USER_PROCESS_SERVICE_E_STATE;
    struct process_lifecycle_snapshot lifecycle;
    int status = lifecycle_snapshot(service, handle, &lifecycle);
    if (status != USER_PROCESS_SERVICE_OK) return status;
    if (lifecycle.state == PROCESS_LIFECYCLE_RUNNABLE)
        return USER_PROCESS_SERVICE_E_PENDING;

    struct scheduler_policy_snapshot scheduler;
    status = scheduler_policy_snapshot(&service->scheduler, handle, &scheduler);
    if (status != SCHEDULER_POLICY_OK) return map_policy_status(status);
    if (scheduler.state != SCHEDULER_POLICY_EXITED)
        return USER_PROCESS_SERVICE_E_STATE;
    status = scheduler_policy_reap(&service->scheduler, handle);
    return status == SCHEDULER_POLICY_OK ? USER_PROCESS_SERVICE_OK
                                         : map_policy_status(status);
}
