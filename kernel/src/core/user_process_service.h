/* user_process_service.h - lifecycle-bound bounded process dispatch. */
#ifndef ZLOS_USER_PROCESS_SERVICE_H
#define ZLOS_USER_PROCESS_SERVICE_H

#include "process_lifecycle.h"
#include "scheduler_policy.h"

typedef unsigned long long user_process_service_u64;

#define USER_PROCESS_SERVICE_MAX_SLEEP_TICKS 0x7fffffffU

enum user_process_service_status {
    USER_PROCESS_SERVICE_OK = 0,
    USER_PROCESS_SERVICE_E_ARGUMENT = -1,
    USER_PROCESS_SERVICE_E_STATE = -2,
    USER_PROCESS_SERVICE_E_CAPACITY = -3,
    USER_PROCESS_SERVICE_E_DUPLICATE = -4,
    USER_PROCESS_SERVICE_E_NOT_FOUND = -5,
    USER_PROCESS_SERVICE_E_IDLE = -6,
    USER_PROCESS_SERVICE_E_PENDING = -7,
    USER_PROCESS_SERVICE_E_RUNNER = -8
};

typedef int (*user_process_service_step)(
    void *context, process_lifecycle_handle handle,
    user_process_service_u64 *elapsed_ticks);

struct user_process_service {
    struct process_lifecycle_table *lifecycle;
    struct scheduler_policy_table scheduler;
    user_process_service_step step;
    void *step_context;
    user_process_service_u64 work_calls;
    scheduler_policy_u32 sleep_started;
    scheduler_policy_u32 sleep_delay;
    int failed;
    int last_error;
};

int user_process_service_init(
    struct user_process_service *service,
    struct process_lifecycle_table *lifecycle,
    struct scheduler_policy_slot *scheduler_slots,
    unsigned int capacity,
    user_process_service_step step,
    void *step_context);
int user_process_service_check(const struct user_process_service *service);
int user_process_service_admit(struct user_process_service *service,
                               process_lifecycle_handle handle);
int user_process_service_work(struct user_process_service *service,
                              scheduler_policy_u32 now,
                              process_lifecycle_handle *dispatched);
/* Only the exact running owner may request sleep from its step callback.
 * Reconciliation commits the deadline and charges elapsed run time once. */
int user_process_service_request_sleep(struct user_process_service *service,
                                       process_lifecycle_handle handle,
                                       scheduler_policy_u32 now,
                                       scheduler_policy_u32 delay_ticks);
int user_process_service_detach_terminal(
    struct user_process_service *service,
    process_lifecycle_handle handle);

#endif
