/* process_lifecycle.c - bounded identities that cannot alias after slot reuse. */
#include "process_lifecycle.h"

#define HANDLE_INDEX_MASK 0xffffffffULL
#define GENERATION_MAX 0xffffffffU

static process_lifecycle_handle make_handle(unsigned int index,
                                             unsigned int generation)
{
    return ((process_lifecycle_handle)generation << 32) |
           (process_lifecycle_handle)(index + 1U);
}

static unsigned int handle_index(process_lifecycle_handle handle)
{
    return (unsigned int)(handle & HANDLE_INDEX_MASK) - 1U;
}

static unsigned int handle_generation(process_lifecycle_handle handle)
{
    return (unsigned int)(handle >> 32);
}

static int table_shape(const struct process_lifecycle_table *table)
{
    return table && table->slots && table->capacity &&
           (process_lifecycle_u64)table->capacity < HANDLE_INDEX_MASK;
}

static int handle_shape(const struct process_lifecycle_table *table,
                        process_lifecycle_handle handle)
{
    process_lifecycle_u64 encoded_index = handle & HANDLE_INDEX_MASK;
    return table_shape(table) && handle_generation(handle) && encoded_index &&
           encoded_index <= table->capacity;
}

static const struct process_lifecycle_slot *find_slot_const(
    const struct process_lifecycle_table *table,
    process_lifecycle_handle handle)
{
    if (!handle_shape(table, handle)) return 0;
    const struct process_lifecycle_slot *slot =
        &table->slots[handle_index(handle)];
    if (slot->state == PROCESS_LIFECYCLE_EMPTY ||
        slot->generation != handle_generation(handle))
        return 0;
    return slot;
}

static struct process_lifecycle_slot *find_slot(
    struct process_lifecycle_table *table, process_lifecycle_handle handle)
{
    return (struct process_lifecycle_slot *)find_slot_const(table, handle);
}

static int empty_termination(
    const struct process_lifecycle_termination *termination)
{
    return termination->kind == PROCESS_LIFECYCLE_TERMINATION_NONE &&
           termination->exit_status == 0 && termination->fault_vector == 0 &&
           termination->fault_error == 0 && termination->fault_address == 0;
}

static void clear_termination(
    struct process_lifecycle_termination *termination)
{
    termination->kind = PROCESS_LIFECYCLE_TERMINATION_NONE;
    termination->exit_status = 0;
    termination->fault_vector = 0;
    termination->fault_error = 0;
    termination->fault_address = 0;
}

static void copy_snapshot(const struct process_lifecycle_table *table,
                          const struct process_lifecycle_slot *slot,
                          struct process_lifecycle_snapshot *snapshot)
{
    unsigned int index = (unsigned int)(slot - table->slots);
    snapshot->handle = make_handle(index, slot->generation);
    snapshot->id = slot->id;
    snapshot->parent = slot->parent;
    snapshot->state = slot->state;
    snapshot->termination = slot->termination;
}

int process_lifecycle_check(const struct process_lifecycle_table *table)
{
    if (!table_shape(table)) return PROCESS_LIFECYCLE_E_ARGUMENT;
    for (unsigned int i = 0; i < table->capacity; i++) {
        const struct process_lifecycle_slot *slot = &table->slots[i];
        if (slot->state == PROCESS_LIFECYCLE_EMPTY) {
            if (slot->id || slot->parent || !empty_termination(&slot->termination))
                return PROCESS_LIFECYCLE_E_STATE;
            continue;
        }
        if (!slot->id || !slot->generation ||
            slot->state > PROCESS_LIFECYCLE_FAULTED)
            return PROCESS_LIFECYCLE_E_STATE;
        process_lifecycle_handle own = make_handle(i, slot->generation);
        if (slot->parent == own)
            return PROCESS_LIFECYCLE_E_STATE;
        if (slot->parent && !find_slot_const(table, slot->parent))
            return PROCESS_LIFECYCLE_E_STATE;
        process_lifecycle_handle ancestor = slot->parent;
        for (unsigned int depth = 0; ancestor && depth < table->capacity;
             depth++) {
            if (ancestor == own) return PROCESS_LIFECYCLE_E_STATE;
            const struct process_lifecycle_slot *ancestor_slot =
                find_slot_const(table, ancestor);
            if (!ancestor_slot) return PROCESS_LIFECYCLE_E_STATE;
            ancestor = ancestor_slot->parent;
        }
        if (ancestor) return PROCESS_LIFECYCLE_E_STATE;
        if (slot->state == PROCESS_LIFECYCLE_RUNNABLE &&
            !empty_termination(&slot->termination))
            return PROCESS_LIFECYCLE_E_STATE;
        if (slot->state == PROCESS_LIFECYCLE_EXITED &&
            (slot->termination.kind != PROCESS_LIFECYCLE_TERMINATION_EXIT ||
             slot->termination.fault_vector || slot->termination.fault_error ||
             slot->termination.fault_address))
            return PROCESS_LIFECYCLE_E_STATE;
        if (slot->state == PROCESS_LIFECYCLE_FAULTED &&
            (slot->termination.kind != PROCESS_LIFECYCLE_TERMINATION_FAULT ||
             slot->termination.exit_status))
            return PROCESS_LIFECYCLE_E_STATE;
        for (unsigned int j = 0; j < i; j++)
            if (table->slots[j].state != PROCESS_LIFECYCLE_EMPTY &&
                table->slots[j].id == slot->id)
                return PROCESS_LIFECYCLE_E_DUPLICATE;
    }
    return PROCESS_LIFECYCLE_OK;
}

int process_lifecycle_init(struct process_lifecycle_table *table,
                           struct process_lifecycle_slot *slots,
                           unsigned int capacity)
{
    if (!table || !slots || !capacity ||
        (process_lifecycle_u64)capacity >= HANDLE_INDEX_MASK)
        return PROCESS_LIFECYCLE_E_ARGUMENT;
    table->slots = slots;
    table->capacity = capacity;
    for (unsigned int i = 0; i < capacity; i++) {
        slots[i].id = 0;
        slots[i].generation = 0;
        slots[i].parent = PROCESS_LIFECYCLE_INVALID_HANDLE;
        slots[i].state = PROCESS_LIFECYCLE_EMPTY;
        clear_termination(&slots[i].termination);
    }
    return PROCESS_LIFECYCLE_OK;
}

int process_lifecycle_create_at(struct process_lifecycle_table *table,
                                unsigned int index, unsigned int id,
                                process_lifecycle_handle parent,
                                process_lifecycle_handle *handle)
{
    if (!table_shape(table) || !handle || !id || index >= table->capacity)
        return PROCESS_LIFECYCLE_E_ARGUMENT;
    if (process_lifecycle_check(table) != PROCESS_LIFECYCLE_OK)
        return PROCESS_LIFECYCLE_E_STATE;
    if (table->slots[index].state != PROCESS_LIFECYCLE_EMPTY)
        return PROCESS_LIFECYCLE_E_STATE;
    if (table->slots[index].generation == GENERATION_MAX)
        return PROCESS_LIFECYCLE_E_GENERATION;
    if (parent) {
        const struct process_lifecycle_slot *owner =
            find_slot_const(table, parent);
        if (!owner) return PROCESS_LIFECYCLE_E_STALE;
        if (owner->state != PROCESS_LIFECYCLE_RUNNABLE)
            return PROCESS_LIFECYCLE_E_STATE;
    }
    for (unsigned int i = 0; i < table->capacity; i++)
        if (table->slots[i].state != PROCESS_LIFECYCLE_EMPTY &&
            table->slots[i].id == id)
            return PROCESS_LIFECYCLE_E_DUPLICATE;

    struct process_lifecycle_slot *slot = &table->slots[index];
    slot->generation++;
    slot->id = id;
    slot->parent = parent;
    slot->state = PROCESS_LIFECYCLE_RUNNABLE;
    clear_termination(&slot->termination);
    *handle = make_handle(index, slot->generation);
    return PROCESS_LIFECYCLE_OK;
}

int process_lifecycle_create(struct process_lifecycle_table *table,
                             unsigned int id,
                             process_lifecycle_handle parent,
                             process_lifecycle_handle *handle)
{
    if (!table_shape(table) || !id || !handle)
        return PROCESS_LIFECYCLE_E_ARGUMENT;
    if (process_lifecycle_check(table) != PROCESS_LIFECYCLE_OK)
        return PROCESS_LIFECYCLE_E_STATE;
    for (unsigned int i = 0; i < table->capacity; i++)
        if (table->slots[i].state != PROCESS_LIFECYCLE_EMPTY &&
            table->slots[i].id == id)
            return PROCESS_LIFECYCLE_E_DUPLICATE;
    for (unsigned int i = 0; i < table->capacity; i++)
        if (table->slots[i].state == PROCESS_LIFECYCLE_EMPTY &&
            table->slots[i].generation != GENERATION_MAX)
            return process_lifecycle_create_at(table, i, id, parent, handle);
    return PROCESS_LIFECYCLE_E_CAPACITY;
}

int process_lifecycle_exit(struct process_lifecycle_table *table,
                           process_lifecycle_handle handle, int status)
{
    if (!handle_shape(table, handle)) return PROCESS_LIFECYCLE_E_ARGUMENT;
    if (process_lifecycle_check(table) != PROCESS_LIFECYCLE_OK)
        return PROCESS_LIFECYCLE_E_STATE;
    struct process_lifecycle_slot *slot = find_slot(table, handle);
    if (!slot) return PROCESS_LIFECYCLE_E_STALE;
    if (slot->state != PROCESS_LIFECYCLE_RUNNABLE)
        return PROCESS_LIFECYCLE_E_STATE;
    slot->state = PROCESS_LIFECYCLE_EXITED;
    clear_termination(&slot->termination);
    slot->termination.kind = PROCESS_LIFECYCLE_TERMINATION_EXIT;
    slot->termination.exit_status = status;
    return PROCESS_LIFECYCLE_OK;
}

int process_lifecycle_fault(struct process_lifecycle_table *table,
                            process_lifecycle_handle handle,
                            unsigned int vector, unsigned int error,
                            process_lifecycle_u64 address)
{
    if (!handle_shape(table, handle)) return PROCESS_LIFECYCLE_E_ARGUMENT;
    if (process_lifecycle_check(table) != PROCESS_LIFECYCLE_OK)
        return PROCESS_LIFECYCLE_E_STATE;
    struct process_lifecycle_slot *slot = find_slot(table, handle);
    if (!slot) return PROCESS_LIFECYCLE_E_STALE;
    if (slot->state != PROCESS_LIFECYCLE_RUNNABLE)
        return PROCESS_LIFECYCLE_E_STATE;
    slot->state = PROCESS_LIFECYCLE_FAULTED;
    clear_termination(&slot->termination);
    slot->termination.kind = PROCESS_LIFECYCLE_TERMINATION_FAULT;
    slot->termination.fault_vector = vector;
    slot->termination.fault_error = error;
    slot->termination.fault_address = address;
    return PROCESS_LIFECYCLE_OK;
}

int process_lifecycle_snapshot(const struct process_lifecycle_table *table,
                               process_lifecycle_handle handle,
                               struct process_lifecycle_snapshot *snapshot)
{
    if (!handle_shape(table, handle) || !snapshot)
        return PROCESS_LIFECYCLE_E_ARGUMENT;
    if (process_lifecycle_check(table) != PROCESS_LIFECYCLE_OK)
        return PROCESS_LIFECYCLE_E_STATE;
    const struct process_lifecycle_slot *slot = find_slot_const(table, handle);
    if (!slot) return PROCESS_LIFECYCLE_E_STALE;
    copy_snapshot(table, slot, snapshot);
    return PROCESS_LIFECYCLE_OK;
}

int process_lifecycle_observe(const struct process_lifecycle_table *table,
                              process_lifecycle_handle requester,
                              process_lifecycle_handle child,
                              struct process_lifecycle_snapshot *snapshot)
{
    if (!handle_shape(table, child) || !snapshot)
        return PROCESS_LIFECYCLE_E_ARGUMENT;
    if (process_lifecycle_check(table) != PROCESS_LIFECYCLE_OK)
        return PROCESS_LIFECYCLE_E_STATE;
    const struct process_lifecycle_slot *slot = find_slot_const(table, child);
    if (!slot) return PROCESS_LIFECYCLE_E_STALE;
    if (slot->parent != requester)
        return PROCESS_LIFECYCLE_E_PERMISSION;
    if (slot->state == PROCESS_LIFECYCLE_RUNNABLE)
        return PROCESS_LIFECYCLE_E_PENDING;
    copy_snapshot(table, slot, snapshot);
    return PROCESS_LIFECYCLE_OK;
}

int process_lifecycle_reap(struct process_lifecycle_table *table,
                           process_lifecycle_handle requester,
                           process_lifecycle_handle child)
{
    if (!handle_shape(table, child)) return PROCESS_LIFECYCLE_E_ARGUMENT;
    if (process_lifecycle_check(table) != PROCESS_LIFECYCLE_OK)
        return PROCESS_LIFECYCLE_E_STATE;
    struct process_lifecycle_slot *slot = find_slot(table, child);
    if (!slot) return PROCESS_LIFECYCLE_E_STALE;
    if (slot->parent != requester)
        return PROCESS_LIFECYCLE_E_PERMISSION;
    if (slot->state == PROCESS_LIFECYCLE_RUNNABLE)
        return PROCESS_LIFECYCLE_E_PENDING;
    for (unsigned int i = 0; i < table->capacity; i++)
        if (table->slots[i].state != PROCESS_LIFECYCLE_EMPTY &&
            table->slots[i].parent == child)
            return PROCESS_LIFECYCLE_E_STATE;
    slot->id = 0;
    slot->parent = PROCESS_LIFECYCLE_INVALID_HANDLE;
    slot->state = PROCESS_LIFECYCLE_EMPTY;
    clear_termination(&slot->termination);
    return PROCESS_LIFECYCLE_OK;
}

int process_lifecycle_find_runnable(
    const struct process_lifecycle_table *table, unsigned int id,
    process_lifecycle_handle *handle)
{
    if (!table_shape(table) || !id || !handle)
        return PROCESS_LIFECYCLE_E_ARGUMENT;
    if (process_lifecycle_check(table) != PROCESS_LIFECYCLE_OK)
        return PROCESS_LIFECYCLE_E_STATE;
    for (unsigned int i = 0; i < table->capacity; i++)
        if (table->slots[i].state == PROCESS_LIFECYCLE_RUNNABLE &&
            table->slots[i].id == id) {
            *handle = make_handle(i, table->slots[i].generation);
            return PROCESS_LIFECYCLE_OK;
        }
    return PROCESS_LIFECYCLE_E_NOT_FOUND;
}
