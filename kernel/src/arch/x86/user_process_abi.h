/* Fixed-width, versioned userspace process results. x86-64 little-endian. */
#ifndef ZLOS_USER_PROCESS_ABI_H
#define ZLOS_USER_PROCESS_ABI_H

#define USER_PROCESS_WAIT_VERSION 1U
#define USER_PROCESS_WAIT_BYTES 32U
#define USER_PROCESS_HANDLE_BYTES 8U

struct user_process_wait_result {
    unsigned int version;
    unsigned int kind; /* 1: normal exit, 2: fault */
    int exit_status;
    unsigned int fault_vector;
    unsigned int fault_error;
    unsigned int reserved;
    unsigned long long fault_address;
};

_Static_assert(sizeof(unsigned int) == 4 && sizeof(int) == 4,
               "process result needs exact 32-bit fields");
_Static_assert(sizeof(unsigned long long) == USER_PROCESS_HANDLE_BYTES,
               "process handles retain all generation bits");
_Static_assert(sizeof(struct user_process_wait_result) == USER_PROCESS_WAIT_BYTES &&
               __builtin_offsetof(struct user_process_wait_result, fault_address) == 24,
               "process result wire layout must match on every build target");

#endif
