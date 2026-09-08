/* Inactive x86-64 process images. No CR3, TSS or current-process mutation. */
#ifndef ZLOS_USER_IMAGE64_H
#define ZLOS_USER_IMAGE64_H

#include "core/process_memory.h"

#define USER_IMAGE64_ENTRIES 512U
#define USER_IMAGE64_ANON_FIRST 6U
#define USER_IMAGE64_STACK_FILL 0xa5U
#define USER_IMAGE64_ADDRESS_MASK 0x000ffffffffff000ULL

enum user_image64_status {
    USER_IMAGE64_OK = 0,
    USER_IMAGE64_E_ARGUMENT = -1,
    USER_IMAGE64_E_STATE = -2,
    USER_IMAGE64_E_TEMPLATE = -3,
    USER_IMAGE64_E_NOMEM = -4,
    USER_IMAGE64_E_CORRUPT = -5
};

struct user_image64 {
    struct process_memory memory;
    pmm_u64 cr3;
    pmm_u64 user_base;
    pmm_u64 user_stack_top;
    pmm_u64 kernel_stack_top;
};

/* A present root entry must be supervisor-only. The caller captures this
 * template in kernel context, never from a running user's address space.
 * Empty output required. Ordinary failure leaves it unchanged. A corruption
 * error retains any unreleased frame custody in output for fail-stop handling.
 * The caller serializes construction/publication; this adds no SMP locking. */
int user_image64_build(struct user_image64 *output,
                       const pmm_u64 kernel_root[USER_IMAGE64_ENTRIES],
                       unsigned int owner, const unsigned char *code,
                       unsigned int bytes);
int user_image64_discard(struct user_image64 *image);

#endif
