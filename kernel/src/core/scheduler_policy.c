/* scheduler_policy.c - fail-closed bounded round-robin scheduling policy. */
#include "scheduler_policy.h"

#define POLICY_U64_MAX 0xffffffffffffffffULL
#define POLICY_MAX_DELAY 0x7fffffffU

static int table_shape(const struct scheduler_policy_table *table)
{
    return table && table->slots && table->capacity &&
           table->capacity < SCHEDULER_POLICY_INVALID_SLOT;
}

static scheduler_policy_u64 add_saturated(scheduler_policy_u64 left,
                                           scheduler_policy_u64 right)
{
    return POLICY_U64_MAX - left < right ? POLICY_U64_MAX : left + right;
}

static int deadline_reached(scheduler_policy_u32 now,
                            scheduler_policy_u32 deadline)
{
    return (scheduler_policy_u32)(now - deadline) <= POLICY_MAX_DELAY;
}

static struct scheduler_policy_slot *find_slot(
    struct scheduler_policy_table *table, scheduler_policy_owner owner,
    unsigned int *index)
{
    if (!table_shape(table) || !owner) return 0;
    for (unsigned int i = 0; i < table->capacity; i++)
        if (table->slots[i].state != SCHEDULER_POLICY_EMPTY &&
            table->slots[i].owner == owner) {
            if (index) *index = i;
            return &table->slots[i];
        }
    return 0;
}

static const struct scheduler_policy_slot *find_slot_const(
    const struct scheduler_policy_table *table, scheduler_policy_owner owner,
    unsigned int *index)
{
    if (!table_shape(table) || !owner) return 0;
    for (unsigned int i = 0; i < table->capacity; i++)
        if (table->slots[i].state != SCHEDULER_POLICY_EMPTY &&
            table->slots[i].owner == owner) {
            if (index) *index = i;
            return &table->slots[i];
        }
    return 0;
}

static int release_running(struct scheduler_policy_table *table,
                           scheduler_policy_owner owner,
                           scheduler_policy_u64 elapsed_ticks,
                           unsigned int next_state,
                           scheduler_policy_u32 wake_at)
{
    if (!table_shape(table) || !owner) return SCHEDULER_POLICY_E_ARGUMENT;
    if (scheduler_policy_check(table) != SCHEDULER_POLICY_OK)
        return SCHEDULER_POLICY_E_STATE;
    unsigned int index = 0;
    struct scheduler_policy_slot *slot = find_slot(table, owner, &index);
    if (!slot) return SCHEDULER_POLICY_E_NOT_FOUND;
    if (table->running != index || slot->state != SCHEDULER_POLICY_RUNNING)
        return SCHEDULER_POLICY_E_STATE;
    slot->run_ticks = add_saturated(slot->run_ticks, elapsed_ticks);
    slot->state = next_state;
    slot->wake_at = wake_at;
    table->running = SCHEDULER_POLICY_INVALID_SLOT;
    return SCHEDULER_POLICY_OK;
}

int scheduler_policy_check(const struct scheduler_policy_table *table)
{
    if (!table_shape(table)) return SCHEDULER_POLICY_E_ARGUMENT;
    if (table->cursor >= table->capacity ||
        (table->running != SCHEDULER_POLICY_INVALID_SLOT &&
         table->running >= table->capacity))
        return SCHEDULER_POLICY_E_STATE;
    unsigned int running = SCHEDULER_POLICY_INVALID_SLOT;
    for (unsigned int i = 0; i < table->capacity; i++) {
        const struct scheduler_policy_slot *slot = &table->slots[i];
        if (slot->state == SCHEDULER_POLICY_EMPTY) {
            if (slot->owner || slot->wake_at || slot->run_ticks ||
                slot->dispatches)
                return SCHEDULER_POLICY_E_STATE;
            continue;
        }
        if (!slot->owner || slot->state > SCHEDULER_POLICY_EXITED)
            return SCHEDULER_POLICY_E_STATE;
        if (slot->state != SCHEDULER_POLICY_SLEEPING && slot->wake_at)
            return SCHEDULER_POLICY_E_STATE;
        if (slot->state == SCHEDULER_POLICY_RUNNING) {
            if (running != SCHEDULER_POLICY_INVALID_SLOT)
                return SCHEDULER_POLICY_E_STATE;
            running = i;
        }
        for (unsigned int j = 0; j < i; j++)
            if (table->slots[j].state != SCHEDULER_POLICY_EMPTY &&
                table->slots[j].owner == slot->owner)
                return SCHEDULER_POLICY_E_DUPLICATE;
    }
    if (running != table->running) return SCHEDULER_POLICY_E_STATE;
    if ((!table->dispatches && (table->switches || table->last_owner)) ||
        (table->dispatches && !table->last_owner))
        return SCHEDULER_POLICY_E_STATE;
    if (table->switches > table->dispatches)
        return SCHEDULER_POLICY_E_STATE;
    return SCHEDULER_POLICY_OK;
}

int scheduler_policy_init(struct scheduler_policy_table *table,
                          struct scheduler_policy_slot *slots,
                          unsigned int capacity)
{
    if (!table || !slots || !capacity ||
        capacity >= SCHEDULER_POLICY_INVALID_SLOT)
        return SCHEDULER_POLICY_E_ARGUMENT;
    table->slots = slots;
    table->capacity = capacity;
    table->cursor = capacity - 1U;
    table->running = SCHEDULER_POLICY_INVALID_SLOT;
    table->last_owner = SCHEDULER_POLICY_INVALID_OWNER;
    table->dispatches = 0;
    table->switches = 0;
    for (unsigned int i = 0; i < capacity; i++) {
        slots[i].owner = SCHEDULER_POLICY_INVALID_OWNER;
        slots[i].state = SCHEDULER_POLICY_EMPTY;
        slots[i].wake_at = 0;
        slots[i].run_ticks = 0;
        slots[i].dispatches = 0;
    }
    return SCHEDULER_POLICY_OK;
}

int scheduler_policy_admit(struct scheduler_policy_table *table,
                           scheduler_policy_owner owner,
                           unsigned int *slot_index)
{
    if (!table_shape(table) || !owner || !slot_index)
        return SCHEDULER_POLICY_E_ARGUMENT;
    if (scheduler_policy_check(table) != SCHEDULER_POLICY_OK)
        return SCHEDULER_POLICY_E_STATE;
    if (find_slot(table, owner, 0)) return SCHEDULER_POLICY_E_DUPLICATE;
    for (unsigned int i = 0; i < table->capacity; i++)
        if (table->slots[i].state == SCHEDULER_POLICY_EMPTY) {
            table->slots[i].owner = owner;
            table->slots[i].state = SCHEDULER_POLICY_RUNNABLE;
            table->slots[i].wake_at = 0;
            table->slots[i].run_ticks = 0;
            table->slots[i].dispatches = 0;
            *slot_index = i;
            return SCHEDULER_POLICY_OK;
        }
    return SCHEDULER_POLICY_E_CAPACITY;
}

int scheduler_policy_dispatch(struct scheduler_policy_table *table,
                              scheduler_policy_u32 now,
                              scheduler_policy_owner *owner)
{
    if (!table_shape(table) || !owner) return SCHEDULER_POLICY_E_ARGUMENT;
    if (scheduler_policy_check(table) != SCHEDULER_POLICY_OK)
        return SCHEDULER_POLICY_E_STATE;
    if (table->running != SCHEDULER_POLICY_INVALID_SLOT)
        return SCHEDULER_POLICY_E_STATE;
    for (unsigned int i = 0; i < table->capacity; i++)
        if (table->slots[i].state == SCHEDULER_POLICY_SLEEPING &&
            deadline_reached(now, table->slots[i].wake_at)) {
            table->slots[i].state = SCHEDULER_POLICY_RUNNABLE;
            table->slots[i].wake_at = 0;
        }
    unsigned int i = table->cursor;
    for (unsigned int visited = 0; visited < table->capacity; visited++) {
        i++;
        if (i == table->capacity) i = 0;
        if (table->slots[i].state != SCHEDULER_POLICY_RUNNABLE) continue;
        scheduler_policy_owner next = table->slots[i].owner;
        table->slots[i].state = SCHEDULER_POLICY_RUNNING;
        table->slots[i].dispatches =
            add_saturated(table->slots[i].dispatches, 1);
        table->running = i;
        table->cursor = i;
        table->dispatches = add_saturated(table->dispatches, 1);
        if (table->last_owner && table->last_owner != next)
            table->switches = add_saturated(table->switches, 1);
        table->last_owner = next;
        *owner = next;
        return SCHEDULER_POLICY_OK;
    }
    return SCHEDULER_POLICY_E_IDLE;
}

int scheduler_policy_yield(struct scheduler_policy_table *table,
                           scheduler_policy_owner owner,
                           scheduler_policy_u64 elapsed_ticks)
{
    return release_running(table, owner, elapsed_ticks,
                           SCHEDULER_POLICY_RUNNABLE, 0);
}

int scheduler_policy_sleep(struct scheduler_policy_table *table,
                           scheduler_policy_owner owner,
                           scheduler_policy_u32 now,
                           scheduler_policy_u32 delay_ticks,
                           scheduler_policy_u64 elapsed_ticks)
{
    if (!delay_ticks) return SCHEDULER_POLICY_E_ARGUMENT;
    if (delay_ticks > POLICY_MAX_DELAY) delay_ticks = POLICY_MAX_DELAY;
    return release_running(table, owner, elapsed_ticks,
                           SCHEDULER_POLICY_SLEEPING, now + delay_ticks);
}

int scheduler_policy_exit(struct scheduler_policy_table *table,
                          scheduler_policy_owner owner,
                          scheduler_policy_u64 elapsed_ticks)
{
    return release_running(table, owner, elapsed_ticks,
                           SCHEDULER_POLICY_EXITED, 0);
}

int scheduler_policy_reap(struct scheduler_policy_table *table,
                          scheduler_policy_owner owner)
{
    if (!table_shape(table) || !owner) return SCHEDULER_POLICY_E_ARGUMENT;
    if (scheduler_policy_check(table) != SCHEDULER_POLICY_OK)
        return SCHEDULER_POLICY_E_STATE;
    struct scheduler_policy_slot *slot = find_slot(table, owner, 0);
    if (!slot) return SCHEDULER_POLICY_E_NOT_FOUND;
    if (slot->state != SCHEDULER_POLICY_EXITED)
        return SCHEDULER_POLICY_E_PENDING;
    slot->owner = SCHEDULER_POLICY_INVALID_OWNER;
    slot->state = SCHEDULER_POLICY_EMPTY;
    slot->wake_at = 0;
    slot->run_ticks = 0;
    slot->dispatches = 0;
    return SCHEDULER_POLICY_OK;
}

int scheduler_policy_snapshot(const struct scheduler_policy_table *table,
                              scheduler_policy_owner owner,
                              struct scheduler_policy_snapshot *snapshot)
{
    if (!table_shape(table) || !owner || !snapshot)
        return SCHEDULER_POLICY_E_ARGUMENT;
    if (scheduler_policy_check(table) != SCHEDULER_POLICY_OK)
        return SCHEDULER_POLICY_E_STATE;
    unsigned int index = 0;
    const struct scheduler_policy_slot *slot =
        find_slot_const(table, owner, &index);
    if (!slot) return SCHEDULER_POLICY_E_NOT_FOUND;
    snapshot->owner = slot->owner;
    snapshot->slot = index;
    snapshot->state = slot->state;
    snapshot->wake_at = slot->wake_at;
    snapshot->run_ticks = slot->run_ticks;
    snapshot->dispatches = slot->dispatches;
    return SCHEDULER_POLICY_OK;
}
