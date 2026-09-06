/* schedtest.c - sched.c on the host: the one lane file that had no host test.
 *
 * sched.c is linked as shipped (with -DZL_HOSTTEST, which only turns the
 * trampoline's `sti` into `nop` - sti faults in ring 3). The task stacks
 * live at HI_SCHED, so that range is mmap'd first.
 *
 *   1. task_sleep(n) with every other task asleep really waits n ticks.
 *      Until 2026-09-04 yield() returned at once when pick_next() fell back
 *      to `current`, so a sleep with nobody else runnable was a hint:
 *      task_sleep(50) came back after two tick reads.
 *   2. a round trip through switch_to keeps the caller's callee-saved GPRs.
 *
 * The Win64-ABI variant of switch_to (the EFI build) is covered separately
 * by schedtest_ms, which is compiled -mabi=ms.
 */
#include <stdio.h>
#include <sys/mman.h>

typedef unsigned long long uptr;

static unsigned ticks;
unsigned idt_ticks(void) { return ticks++; }     /* every read advances one tick */

int  sched_init(void);
int  task_create(uptr entry);
void task_sleep(unsigned ticks);
void yield(void);

static int fails;
static void ok(const char *what, int cond)
{
    if (cond) printf("  ok    %s\n", what);
    else { fails++; printf("  FAIL  %s\n", what); }
}

static void bg(void) { for (;;) task_sleep(1000); }

int main(void)
{
    void *p = mmap((void *)0x0B000000UL, 0x800000, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p != (void *)0x0B000000UL) { perror("mmap HI_SCHED"); return 2; }

    ok("sched_init", sched_init() == 1);
    ok("a task is created", task_create((uptr)bg) >= 0);
    yield();                    /* bg runs once: sets wake_at and yields back */

    unsigned t0 = ticks;
    task_sleep(50);
    unsigned dt = ticks - t0;
    printf("        task_sleep(50) returned after %u tick reads\n", dt);
    ok("task_sleep(50) waits at least 50 ticks with nobody else runnable", dt >= 50);

    /* callee-saved registers survive a switch away and back */
    unsigned long rbx_out, r12_out;
    __asm__ volatile(
        "movq $0x1111, %%rbx\n\t"
        "movq $0x2222, %%r12\n\t"
        "call yield\n\t"
        "movq %%rbx, %0\n\t"
        "movq %%r12, %1\n\t"
        : "=r"(rbx_out), "=r"(r12_out) : : "rbx", "r12", "rax", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "memory");
    ok("rbx and r12 survive a round trip through switch_to (SysV)",
       rbx_out == 0x1111 && r12_out == 0x2222);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
