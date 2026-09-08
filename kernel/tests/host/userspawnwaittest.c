/* Real architecture syscall/constructor/reap code. Only privileged CR3 reads,
 * devices, logging and entering Ring 3 are outside this host evidence lane. */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "../../src/arch/x86/usermode.c"

#define TEST_POOL_PAGES 32U
static u64 host_root[512] __attribute__((aligned(4096)));
static u64 host_stack;
static unsigned int checks, failures, reads;
static u64 tss_top;

u64 usermode_host_cr3(void) { return (u64)(uintptr_t)host_root; }
void *pmm_host_page_pointer(pmm_u64 address)
{
    return address >= PMM_DYNAMIC_FLOOR &&
           address < PMM_DYNAMIC_FLOOR + TEST_POOL_PAGES * PMM_PAGE_BYTES
        ? (void *)(uintptr_t)address : NULL;
}
void zl_putc_pub(char c) { (void)c; }
void gdt64_set_kernel_stack(u64 top) { tss_top = top; }
u64 gdt64_active_kernel_stack_top(void) { return tss_top; }
u32 idt_ticks(void) { return 17; }
void idt_timer_tick(void) {}
void yield(void) {}
void userwin_close_owner(int owner) { (void)owner; }
int fs_mounted(void) { return 1; }
int fs_find(const char *name) { return !strcmp(name,"child") ? 0 : !strcmp(name,"empty") ? 1 : !strcmp(name,"large") ? 2 : -1; }
u32 fs_size(int index) { return index == 0 ? 2 : index == 2 ? 4097 : 0; }
int fs_read(int index, void *out, u32 bytes)
{
    reads++;
    if (index || bytes != 2) return -1;
    unsigned char *p = out; p[0] = 0x0f; p[1] = 0x0b;
    return 2;
}
/* These routes are intentionally outside this process-ABI test. Accidentally
 * reaching one fails the harness instead of pretending the device worked. */
int fs_create(const char *name, u32 bytes) { abort(); }
int fs_write(int index, const void *data, u32 bytes) { abort(); }
int fs_maxfiles(void) { abort(); }
int fs_used(int index) { abort(); }
int fs_name_byte(int index, int offset) { abort(); }
int fs_delete(int index) { abort(); }
int fs_rename(int index, const char *name) { abort(); }
int fs_sync(void) { abort(); }
int userwin_open(int owner, const char *title) { abort(); }
int userwin_present(int owner, int handle, const char *text, u32 bytes) { abort(); }
int userwin_poll(int owner, int handle, struct userwin_event64 *event) { abort(); }
int userwin_close(int owner, int handle) { abort(); }

static void expect(int ok, const char *label)
{
    checks++; if (!ok) { failures++; printf("FAIL: %s\n", label); }
}
static void *map_exact(u64 address, unsigned long bytes)
{
    void *p = mmap((void *)(uintptr_t)address, bytes, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p == MAP_FAILED) { perror("mmap test fixture"); exit(2); }
    return p;
}
static process_lifecycle_handle setup_parent(unsigned int extra_pages)
{
    if (host_stack) { munmap((void *)(uintptr_t)host_stack, PMM_PAGE_BYTES); host_stack = 0; }
    memset(procs64,0,sizeof procs64);
    memset(&proc_service,0,sizeof proc_service);
    proc_lifecycle_initialized = proc_service_initialized = 0;
    process64_kernel_template_ready = 0;
    user64_running = 0; proc64 = NULL;
    proc_service_next_pid = 1000;
    struct pmm_range range = {PMM_DYNAMIC_FLOOR,
        (PROCESS_MEMORY_PAGE_COUNT + extra_pages) * PMM_PAGE_BYTES, PMM_RANGE_USABLE};
    expect(pmm_init_ranges(&range,1) == PMM_OK && process64_service_ready(),
           "real allocator and process service initialize");
    const u8_64 code[] = {0x0f,0x0b};
    expect(process64_construct(0,500,0,code,sizeof code,0,1) == 1,
           "kernel admits parent before userspace runs");
    process64_select(0);
    process_lifecycle_handle selected = 0;
    expect(scheduler_policy_dispatch(&proc_service.scheduler,17,&selected) ==
           SCHEDULER_POLICY_OK && selected == procs64[0].lifecycle_handle,
           "parent owns current scheduler dispatch");
    host_stack = procs64[0].user_base + 8192;
    map_exact(host_stack, PMM_PAGE_BYTES);
    memset((void *)(uintptr_t)host_stack,0xa5,PMM_PAGE_BYTES);
    memcpy((void *)(uintptr_t)(host_stack+64),"child",5);
    user64_running = 1;
    return selected;
}
static void finish_child(process_lifecycle_handle parent, process_lifecycle_handle child, int fault)
{
    expect(scheduler_policy_yield(&proc_service.scheduler,parent,0) == SCHEDULER_POLICY_OK,
           "parent gives child a dispatch");
    process_lifecycle_handle selected = 0;
    expect(scheduler_policy_dispatch(&proc_service.scheduler,17,&selected) == SCHEDULER_POLICY_OK &&
           selected == child, "real scheduler selects admitted child");
    if (fault) expect(process_lifecycle_fault(&proc_lifecycle,child,14,4,0x8000006000ULL) == PROCESS_LIFECYCLE_OK,
                      "child records exact access fault");
    else expect(process_lifecycle_exit(&proc_lifecycle,child,-37) == PROCESS_LIFECYCLE_OK,
                "child records signed normal exit");
    struct process64 *p = &procs64[1];
    p->state = fault ? PROCESS_LIFECYCLE_FAULTED : PROCESS_LIFECYCLE_EXITED;
    p->exit_status = fault ? 0 : -37;
    p->fault_vector = fault ? 14 : 0; p->fault_error = fault ? 4 : 0;
    p->fault_address = fault ? 0x8000006000ULL : 0;
    expect(scheduler_policy_exit(&proc_service.scheduler,child,0) == SCHEDULER_POLICY_OK &&
           scheduler_policy_dispatch(&proc_service.scheduler,17,&selected) == SCHEDULER_POLICY_OK &&
           selected == parent, "parent resumes while terminal child remains owned");
}
/* Decode the actual Ring-3 fixture, so adding a syscall cannot silently
 * turn its first-gap negative case into an admitted call. */
static void test_unknown_syscall_fixture(void)
{
    const unsigned char *instruction = unknown_syscalls + 16;
    unsigned int prefix = instruction[0] == 0x48 ? 2 : 1;
    unsigned int width = prefix == 2 ? 8 : 4;
    expect(instruction[prefix - 1] == 0xb8, "target gap probe loads RAX");
    u64 number = 0;
    for (unsigned int i = 0; i < width; i++)
        number |= (u64)instruction[prefix + i] << (8 * i);
    expect(number == ZLOS_U64_SYSCALL_LAST + 1ULL,
           "actual target gap probe follows the generated ABI range");
    expect(!zlos_u64_syscall_known(number), "target gap stays outside admission");
    setup_parent(16);
    expect(user64_dispatch(number,0,0,0) == U64_ENOSYS,
           "target gap dispatch returns ENOSYS");
}

static void test_spawn_wait(void)
{
    process_lifecycle_handle parent = setup_parent(16);
    proc_lifecycle_slots[1].generation = 0x7fffffffU;
    const u64 out = host_stack+16, result_address = host_stack+128;
    struct process64 parent_before = procs64[0];
    u64 kernel_cr3 = user64_kernel_cr3, process_cr3 = user64_process_cr3, kernel_stack = tss_top;
    user64_return_rsp=0x11223344;user64_return_rip=0x55667788;user64_return_rflags=0x202;
    memset(user64_fx_user,0x3a,sizeof user64_fx_user);memset(user64_fx_kernel,0x7b,sizeof user64_fx_kernel);
    unsigned int before_reads = reads;
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,host_stack+PMM_PAGE_BYTES-4) == U64_EINVAL &&
           reads == before_reads && pmm_used_pages() == PROCESS_MEMORY_PAGE_COUNT,
           "cross-page handle output rejects before filesystem or allocation");
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,out) == 0,
           "actual spawn syscall admits a disk-reader child");
    process_lifecycle_handle child = *(process_lifecycle_handle *)(uintptr_t)out;
    expect(child == 0x8000000000000002ULL && proc_lifecycle_slots[1].parent == parent,
           "all eight handle bytes survive and parent comes from current kernel identity");
    parent_before.calls += 2;
    expect(!memcmp(&parent_before,&procs64[0],sizeof parent_before) && proc64 == &procs64[0] && proc64_index == 0,
           "only parent's syscall count changes while preparing child");
    expect(user64_kernel_cr3 == kernel_cr3 && user64_process_cr3 == process_cr3 && tss_top == kernel_stack &&
           user64_return_rsp == 0x11223344 && user64_return_rip == 0x55667788 && user64_return_rflags == 0x202,
           "active address-space, TSS and return context stay unchanged");
    for (unsigned int i=0;i<512;i++) expect(user64_fx_user[i]==0x3a && user64_fx_kernel[i]==0x7b,
                                           "both active FP scratch images stay unchanged");
    u64 sentinel = *(u64 *)(uintptr_t)(host_stack+24);
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,host_stack+24) == U64_ENOSPC &&
           *(u64 *)(uintptr_t)(host_stack+24)==sentinel, "full table fails without publishing another handle");
    unsigned char before[USER_PROCESS_WAIT_BYTES];memcpy(before,(void *)(uintptr_t)result_address,sizeof before);
    expect(user64_dispatch(U64_SYS_WAIT,child,result_address,USER_PROCESS_WAIT_BYTES)==U64_EAGAIN &&
           !memcmp(before,(void *)(uintptr_t)result_address,sizeof before), "pending wait preserves result buffer");
    expect(user64_dispatch(U64_SYS_WAIT,parent,result_address,USER_PROCESS_WAIT_BYTES)==(u64)-1,
           "foreign-parent wait is denied");
    finish_child(parent,child,1);
    expect(user64_dispatch(U64_SYS_WAIT,child,host_stack+PMM_PAGE_BYTES-16,USER_PROCESS_WAIT_BYTES)==U64_EINVAL &&
           procs64[1].lifecycle_handle==child && pmm_used_pages()==2*PROCESS_MEMORY_PAGE_COUNT,
           "invalid wait output cannot consume the sole termination record");
    expect(user64_dispatch(U64_SYS_WAIT,child,result_address,USER_PROCESS_WAIT_BYTES-1)==U64_EINVAL,
           "wrong result size cannot consume the child");
    u64 stolen=procs64[1].memory.pages[PROCESS_MEMORY_CODE];
    unsigned int owner=PROCESS_MEMORY_OWNER_BASE+1;
    expect(pmm_release(stolen,owner)==PMM_OK && pmm_alloc(owner+1)==stolen,
           "inject a foreign owner at the cleanup boundary");
    expect(user64_dispatch(U64_SYS_WAIT,child,result_address,USER_PROCESS_WAIT_BYTES)==U64_EIO &&
           procs64[1].lifecycle_handle==child &&
           !memcmp(before,(void *)(uintptr_t)result_address,sizeof before),
           "cleanup refusal preserves termination custody and output bytes");
    expect(pmm_release(stolen,owner+1)==PMM_OK && pmm_alloc(owner)==stolen,
           "restore the same physical ownership for cleanup retry");
    expect(user64_dispatch(U64_SYS_WAIT,child,result_address,USER_PROCESS_WAIT_BYTES)==0,
           "valid retry reaps the same terminal child");
    struct user_process_wait_result result;memcpy(&result,(void *)(uintptr_t)result_address,sizeof result);
    expect(result.version==1 && result.kind==2 && result.exit_status==0 && result.fault_vector==14 &&
           result.fault_error==4 && result.fault_address==0x8000006000ULL && result.reserved==0,
           "wire result preserves the complete typed fault record");
    expect(pmm_used_pages()==PROCESS_MEMORY_PAGE_COUNT && !procs64[1].state &&
           user_process_service_check(&proc_service)==USER_PROCESS_SERVICE_OK,
           "child reap restores exact allocator and scheduler ownership");
    expect(user64_dispatch(U64_SYS_WAIT,child,result_address,USER_PROCESS_WAIT_BYTES)==U64_ENOENT,
           "consumed handle cannot reap again");
    proc_lifecycle_slots[1].generation=0xfffffffeU;
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,out)==0 &&
           *(u64 *)(uintptr_t)out==0xffffffff00000002ULL, "maximum generation is not a signed-error return");
    process_lifecycle_handle next=*(u64 *)(uintptr_t)out;
    expect(user64_dispatch(U64_SYS_WAIT,child,result_address,USER_PROCESS_WAIT_BYTES)==U64_ENOENT,
           "stale generation cannot target the replacement");
    finish_child(parent,next,0);
    expect(user64_dispatch(U64_SYS_WAIT,next,result_address,USER_PROCESS_WAIT_BYTES)==0,
           "normal exit is reaped through the same boundary");
    memcpy(&result,(void *)(uintptr_t)result_address,sizeof result);
    expect(result.kind==1 && result.exit_status==-37 && !result.fault_vector && !result.fault_address,
           "signed exit is distinct from fault");
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,out)==U64_ENOSPC,
           "exhausted generation cannot silently wrap");
}
static void test_allocation_failures(void)
{
    for(unsigned int available=0;available<PROCESS_MEMORY_PAGE_COUNT;available++) {
        setup_parent(available);
        u64 before=*(u64 *)(uintptr_t)(host_stack+16);
        struct process64 parent_before=procs64[0];
        u64 kernel_cr3=user64_kernel_cr3, active_cr3=user64_process_cr3, stack=tss_top;
        u64 return_rsp=user64_return_rsp, return_rip=user64_return_rip, return_flags=user64_return_rflags;
        u8_64 user_fx[512],kernel_fx[512];
        memcpy(user_fx,user64_fx_user,sizeof user_fx);memcpy(kernel_fx,user64_fx_kernel,sizeof kernel_fx);
        struct process_lifecycle_slot slots_before[U64_PROCS];memcpy(slots_before,proc_lifecycle_slots,sizeof slots_before);
        struct scheduler_policy_slot schedule_before[U64_PROCS];memcpy(schedule_before,proc_scheduler_slots,sizeof schedule_before);
        expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,host_stack+16)==U64_ENOMEM,
               "spawn reports each physical allocation failure");
        expect(*(u64 *)(uintptr_t)(host_stack+16)==before && pmm_used_pages()==PROCESS_MEMORY_PAGE_COUNT &&
               pmm_free_pages()==available && !memcmp(slots_before,proc_lifecycle_slots,sizeof slots_before) &&
               !memcmp(schedule_before,proc_scheduler_slots,sizeof schedule_before) && proc_service_next_pid==1000,
               "failed spawn restores output, identities, scheduler, next PID and frames");
        parent_before.calls++;
        expect(!memcmp(&parent_before,&procs64[0],sizeof parent_before) && proc64==&procs64[0] &&
               proc64_index==0 && user64_kernel_cr3==kernel_cr3 && user64_process_cr3==active_cr3 &&
               tss_top==stack && user64_return_rsp==return_rsp && user64_return_rip==return_rip &&
               user64_return_rflags==return_flags && !memcmp(user_fx,user64_fx_user,sizeof user_fx) &&
               !memcmp(kernel_fx,user64_fx_kernel,sizeof kernel_fx),
               "every allocation refusal preserves the parent's active and saved execution state");
    }
}
static void test_admission_failures(void)
{
    setup_parent(16);
    u64 out=host_stack+16, sentinel=*(u64 *)(uintptr_t)out;
    expect(user64_dispatch(0,0,0,0)==U64_ENOSYS &&
           user64_dispatch(1ULL<<63,0,0,0)==U64_ENOSYS &&
           user64_dispatch(~0ULL,0,0,0)==U64_ENOSYS,
           "unknown and signed-looking syscall numbers remain rejected");
    memcpy((void *)(uintptr_t)(host_stack+64),"empty",5);
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,out)==U64_EINVAL,
           "zero-length executable rejected");
    memcpy((void *)(uintptr_t)(host_stack+64),"large",5);
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,out)==U64_EINVAL,
           "oversized raw executable rejected before reading it");
    memcpy((void *)(uintptr_t)(host_stack+64),"absent",6);
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,6,out)==U64_ENOENT,
           "missing executable returns a typed error");
    memcpy((void *)(uintptr_t)(host_stack+64),"chi\0d",5);
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,out)==U64_EINVAL,
           "embedded nul cannot shorten the admitted filename");
    memcpy((void *)(uintptr_t)(host_stack+64),"child",5);
    struct scheduler_policy_slot prior=proc_scheduler_slots[1];
    proc_scheduler_slots[1].state=SCHEDULER_POLICY_RUNNABLE;
    proc_scheduler_slots[1].owner=0xdead00000002ULL;
    unsigned int previous_reads=reads;
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,out)==U64_EIO && reads==previous_reads,
           "corrupt scheduler admission fails before reading or allocating the image");
    proc_scheduler_slots[1]=prior;
    procs64[1].anonymous.reserved_count=1;
    expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,out)==U64_EIO &&
           procs64[1].anonymous.reserved_count==1 && pmm_used_pages()==PROCESS_MEMORY_PAGE_COUNT,
           "nominally empty child slot cannot discard anonymous-memory custody");
    procs64[1].anonymous.reserved_count=0;
    expect(*(u64 *)(uintptr_t)out==sentinel && pmm_used_pages()==PROCESS_MEMORY_PAGE_COUNT &&
           proc_lifecycle_slots[1].state==PROCESS_LIFECYCLE_EMPTY &&
           user_process_service_check(&proc_service)==USER_PROCESS_SERVICE_OK,
           "all failed admissions preserve output and source ownership");
}
static void test_orphan_cleanup(void)
{
    for (int child_first=0;child_first<2;child_first++) {
        process_lifecycle_handle parent=setup_parent(16);
        expect(user64_dispatch(U64_SYS_SPAWN,host_stack+64,5,host_stack+16)==0,
               "spawn child for parent-death cleanup");
        process_lifecycle_handle child=*(u64 *)(uintptr_t)(host_stack+16);
        if (child_first) finish_child(parent,child,1);
        expect(user64_dispatch(U64_SYS_EXIT,(u64)-19,0,0)==0 &&
               scheduler_policy_exit(&proc_service.scheduler,parent,0)==SCHEDULER_POLICY_OK,
               "real exit dispatcher and scheduler retain terminal parent");
        user64_running=0;
        expect(user64_service_reap(0)==0 && !procs64[0].lifecycle_handle &&
               proc_lifecycle_slots[1].parent==0 && procs64[1].lifecycle_handle==child &&
               pmm_used_pages()==PROCESS_MEMORY_PAGE_COUNT &&
               user_process_service_check(&proc_service)==USER_PROCESS_SERVICE_OK,
               "architecture parent reap adopts both live and terminal children before identity reuse");
        struct process_lifecycle_snapshot snapshot;
        if (child_first) {
            expect(process_lifecycle_observe(&proc_lifecycle,0,child,&snapshot)==PROCESS_LIFECYCLE_OK &&
                   snapshot.termination.fault_vector==14 && snapshot.termination.fault_error==4 &&
                   snapshot.termination.fault_address==0x8000006000ULL,
                   "kernel custody retains the original child's complete fault record");
        } else {
            process_lifecycle_handle selected=0;
            expect(user64_service_reap(1)==-11 &&
                   scheduler_policy_dispatch(&proc_service.scheduler,17,&selected)==SCHEDULER_POLICY_OK &&
                   selected==child, "live orphan remains schedulable and cannot be reaped early");
            process64_select(1);user64_running=1;
            expect(user64_dispatch(U64_SYS_EXIT,(u64)-37,0,0)==0 &&
                   scheduler_policy_exit(&proc_service.scheduler,child,0)==SCHEDULER_POLICY_OK,
                   "orphan later exits through the real dispatcher");
            user64_running=0;
        }
        expect(user64_service_reap(1)==0 && !procs64[1].lifecycle_handle &&
               !pmm_used_pages() && user_process_service_check(&proc_service)==USER_PROCESS_SERVICE_OK,
               "kernel reaps orphan without leaked frames or scheduler ownership");
    }
}
int main(void)
{
    map_exact(PMM_DYNAMIC_FLOOR,TEST_POOL_PAGES*PMM_PAGE_BYTES);
    host_root[0]=0x200003;
    test_unknown_syscall_fixture();test_spawn_wait();test_allocation_failures();test_admission_failures();test_orphan_cleanup();
    printf("userspawnwaittest: %u checks, %u failures\n",checks,failures);
    return failures!=0;
}
