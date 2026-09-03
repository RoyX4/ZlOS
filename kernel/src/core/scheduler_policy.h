/* scheduler_policy.h - bounded owner scheduling without architecture context. */
#ifndef ZLOS_SCHEDULER_POLICY_H
#define ZLOS_SCHEDULER_POLICY_H

typedef unsigned int scheduler_policy_u32;
typedef unsigned long long scheduler_policy_u64;
typedef scheduler_policy_u64 scheduler_policy_owner;

#define SCHEDULER_POLICY_INVALID_OWNER 0ULL
#define SCHEDULER_POLICY_INVALID_SLOT 0xffffffffU

enum scheduler_policy_state {
    SCHEDULER_POLICY_EMPTY = 0,
    SCHEDULER_POLICY_RUNNABLE,
    SCHEDULER_POLICY_RUNNING,
    SCHEDULER_POLICY_SLEEPING,
    SCHEDULER_POLICY_EXITED
};

enum scheduler_policy_status {
    SCHEDULER_POLICY_OK = 0,
    SCHEDULER_POLICY_E_ARGUMENT = -1,
    SCHEDULER_POLICY_E_STATE = -2,
    SCHEDULER_POLICY_E_CAPACITY = -3,
    SCHEDULER_POLICY_E_DUPLICATE = -4,
    SCHEDULER_POLICY_E_NOT_FOUND = -5,
    SCHEDULER_POLICY_E_IDLE = -6,
    SCHEDULER_POLICY_E_PENDING = -7
};

struct scheduler_policy_slot {
    scheduler_policy_owner owner;
    unsigned int state;
    scheduler_policy_u32 wake_at;
    scheduler_policy_u64 run_ticks;
    scheduler_policy_u64 dispatches;
};

struct scheduler_policy_table {
    struct scheduler_policy_slot *slots;
    unsigned int capacity;
    unsigned int cursor;
    unsigned int running;
    scheduler_policy_owner last_owner;
    scheduler_policy_u64 dispatches;
    scheduler_policy_u64 switches;
};

struct scheduler_policy_snapshot {
    scheduler_policy_owner owner;
    unsigned int slot;
    unsigned int state;
    scheduler_policy_u32 wake_at;
    scheduler_policy_u64 run_ticks;
    scheduler_policy_u64 dispatches;
};

int scheduler_policy_init(struct scheduler_policy_table *table,
                          struct scheduler_policy_slot *slots,
                          unsigned int capacity);
int scheduler_policy_check(const struct scheduler_policy_table *table);
int scheduler_policy_admit(struct scheduler_policy_table *table,
                           scheduler_policy_owner owner,
                           unsigned int *slot);
int scheduler_policy_dispatch(struct scheduler_policy_table *table,
                              scheduler_policy_u32 now,
                              scheduler_policy_owner *owner);
int scheduler_policy_yield(struct scheduler_policy_table *table,
                           scheduler_policy_owner owner,
                           scheduler_policy_u64 elapsed_ticks);
int scheduler_policy_sleep(struct scheduler_policy_table *table,
                           scheduler_policy_owner owner,
                           scheduler_policy_u32 now,
                           scheduler_policy_u32 delay_ticks,
                           scheduler_policy_u64 elapsed_ticks);
int scheduler_policy_exit(struct scheduler_policy_table *table,
                          scheduler_policy_owner owner,
                          scheduler_policy_u64 elapsed_ticks);
int scheduler_policy_reap(struct scheduler_policy_table *table,
                          scheduler_policy_owner owner);
int scheduler_policy_snapshot(const struct scheduler_policy_table *table,
                              scheduler_policy_owner owner,
                              struct scheduler_policy_snapshot *snapshot);

#endif
