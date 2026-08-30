#ifndef ZLOS_PAGE_TABLE_TXN_H
#define ZLOS_PAGE_TABLE_TXN_H

typedef unsigned long long vmm_pt_word;
typedef int (*vmm_pt_flush_fn)(void *context);

enum vmm_pt_txn_status {
    VMM_PT_TXN_OK = 0,
    VMM_PT_TXN_ERR_ARGUMENT = -1,
    VMM_PT_TXN_ERR_STATE = -2,
    VMM_PT_TXN_ERR_CAPACITY = -3,
    VMM_PT_TXN_ERR_DUPLICATE = -4,
    VMM_PT_TXN_ERR_STALE = -5,
    VMM_PT_TXN_ERR_INJECTED = -6,
    VMM_PT_TXN_ERR_FLUSH = -7,
    VMM_PT_TXN_ERR_VERIFY = -8,
    VMM_PT_TXN_ERR_ROLLBACK = -9,
};

enum vmm_pt_txn_state {
    VMM_PT_TXN_EMPTY = 0,
    VMM_PT_TXN_RESERVING,
    VMM_PT_TXN_APPLIED,
    VMM_PT_TXN_COMMITTED,
    VMM_PT_TXN_ROLLED_BACK,
    VMM_PT_TXN_FAILED,
};

struct vmm_pt_change {
    volatile vmm_pt_word *entry;
    vmm_pt_word before;
    vmm_pt_word after;
};

struct vmm_pt_transaction {
    struct vmm_pt_change *changes;
    unsigned capacity;
    unsigned count;
    unsigned applied;
    unsigned fail_after_write;
    enum vmm_pt_txn_state state;
    vmm_pt_flush_fn flush;
    void *flush_context;
};

int vmm_pt_txn_begin(struct vmm_pt_transaction *transaction,
                     struct vmm_pt_change *changes, unsigned capacity,
                     vmm_pt_flush_fn flush, void *flush_context);
int vmm_pt_txn_reserve(struct vmm_pt_transaction *transaction,
                       volatile vmm_pt_word *entry, vmm_pt_word after);
void vmm_pt_txn_fail_after_write(struct vmm_pt_transaction *transaction,
                                 unsigned writes);
int vmm_pt_txn_apply(struct vmm_pt_transaction *transaction);
int vmm_pt_txn_commit(struct vmm_pt_transaction *transaction);
int vmm_pt_txn_rollback(struct vmm_pt_transaction *transaction);

#endif
