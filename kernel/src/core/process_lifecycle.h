/* process_lifecycle.h - generation-safe bounded process identity and teardown. */
#ifndef ZLOS_PROCESS_LIFECYCLE_H
#define ZLOS_PROCESS_LIFECYCLE_H

typedef unsigned long long process_lifecycle_u64;
typedef process_lifecycle_u64 process_lifecycle_handle;

#define PROCESS_LIFECYCLE_INVALID_HANDLE 0ULL

enum process_lifecycle_state {
    PROCESS_LIFECYCLE_EMPTY = 0,
    PROCESS_LIFECYCLE_RUNNABLE,
    PROCESS_LIFECYCLE_EXITED,
    PROCESS_LIFECYCLE_FAULTED
};

enum process_lifecycle_termination_kind {
    PROCESS_LIFECYCLE_TERMINATION_NONE = 0,
    PROCESS_LIFECYCLE_TERMINATION_EXIT,
    PROCESS_LIFECYCLE_TERMINATION_FAULT
};

enum process_lifecycle_status {
    PROCESS_LIFECYCLE_OK = 0,
    PROCESS_LIFECYCLE_E_ARGUMENT = -1,
    PROCESS_LIFECYCLE_E_STATE = -2,
    PROCESS_LIFECYCLE_E_CAPACITY = -3,
    PROCESS_LIFECYCLE_E_NOT_FOUND = -4,
    PROCESS_LIFECYCLE_E_PERMISSION = -5,
    PROCESS_LIFECYCLE_E_PENDING = -6,
    PROCESS_LIFECYCLE_E_STALE = -7,
    PROCESS_LIFECYCLE_E_DUPLICATE = -8,
    PROCESS_LIFECYCLE_E_GENERATION = -9
};

struct process_lifecycle_termination {
    unsigned int kind;
    int exit_status;
    unsigned int fault_vector;
    unsigned int fault_error;
    process_lifecycle_u64 fault_address;
};

struct process_lifecycle_slot {
    unsigned int id;
    unsigned int generation;
    process_lifecycle_handle parent;
    unsigned int state;
    struct process_lifecycle_termination termination;
};

struct process_lifecycle_table {
    struct process_lifecycle_slot *slots;
    unsigned int capacity;
};

struct process_lifecycle_snapshot {
    process_lifecycle_handle handle;
    unsigned int id;
    process_lifecycle_handle parent;
    unsigned int state;
    struct process_lifecycle_termination termination;
};

int process_lifecycle_init(struct process_lifecycle_table *table,
                           struct process_lifecycle_slot *slots,
                           unsigned int capacity);
int process_lifecycle_check(const struct process_lifecycle_table *table);
int process_lifecycle_create(struct process_lifecycle_table *table,
                             unsigned int id,
                             process_lifecycle_handle parent,
                             process_lifecycle_handle *handle);
int process_lifecycle_create_at(struct process_lifecycle_table *table,
                                unsigned int index, unsigned int id,
                                process_lifecycle_handle parent,
                                process_lifecycle_handle *handle);
int process_lifecycle_exit(struct process_lifecycle_table *table,
                           process_lifecycle_handle handle, int status);
int process_lifecycle_fault(struct process_lifecycle_table *table,
                            process_lifecycle_handle handle,
                            unsigned int vector, unsigned int error,
                            process_lifecycle_u64 address);
int process_lifecycle_observe(const struct process_lifecycle_table *table,
                              process_lifecycle_handle requester,
                              process_lifecycle_handle child,
                              struct process_lifecycle_snapshot *snapshot);
int process_lifecycle_snapshot(const struct process_lifecycle_table *table,
                               process_lifecycle_handle handle,
                               struct process_lifecycle_snapshot *snapshot);
int process_lifecycle_reap(struct process_lifecycle_table *table,
                           process_lifecycle_handle requester,
                           process_lifecycle_handle child);
int process_lifecycle_find_runnable(
    const struct process_lifecycle_table *table, unsigned int id,
    process_lifecycle_handle *handle);

#endif
