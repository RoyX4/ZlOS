/* sched.c - doing more than one thing at a time.
 *
 * Up to now zlOS has run exactly one thread of control. The shell loops
 * waiting for a key; while it waits, nothing else in the machine happens. That
 * is the last structural thing separating it from an operating system, and the
 * boot log has been honest about it: "no heap, no filesystem, no scheduler".
 *
 * WHAT A SCHEDULER ACTUALLY IS
 * ----------------------------
 * Less than people expect. A task is a stack plus a saved stack pointer, and
 * switching between tasks is:
 *
 *     push every register onto the current stack
 *     save the stack pointer into the old task
 *     load the stack pointer from the new task
 *     pop every register back off
 *     return
 *
 * That last `ret` is the whole trick. It does not return to the caller - it
 * returns to wherever the NEW task's stack says to go, which is wherever that
 * task was when it last gave up the CPU. Two tasks, two stacks, and one
 * instruction that steps between them.
 *
 * A brand-new task has never given up the CPU, so it has no such history. We
 * fabricate one: lay out a stack that looks exactly as if the task had been
 * suspended, with a trampoline as the return address.
 *
 * PREEMPTION
 * ----------
 * The timer interrupt already fires 100 times a second. It counts a tick and
 * sets a flag; the switch itself happens at a yield point rather than inside
 * the interrupt handler. Doing it inside the handler is possible but means
 * every task must be suspended inside an identical interrupt frame, and one
 * mistake there is an unbootable machine with no diagnostic. Cooperative
 * points that the timer *drives* get the same visible behaviour - a clock that
 * keeps time while you type - with a fraction of the ways to get it wrong.
 * That is a deliberate trade and worth naming rather than hiding.
 */

typedef unsigned int   u32;
typedef unsigned char  u8;

/* A saved stack pointer is pointer-width, not always 32 bits. The UEFI build -
 * the one that boots the real laptop - is 64-bit, and this file has to work
 * there too. */
#if defined(ZL_64)
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

extern u32 idt_ticks(void);

#define MAX_TASKS   8
#define STACK_BYTES 32768              /* 32 KiB per task */
#define STACK_BASE  0x0B000000u        /* 176 MiB: clear of everything else */

#define TASK_FREE    0
#define TASK_READY   1
#define TASK_RUNNING 2
#define TASK_DONE    3

struct task {
    uptr sp;           /* saved stack pointer - the whole of its context   */
    u32 state;
    u32 ticks;         /* how much CPU it has had, for the diagnostics     */
    u32 wake_at;       /* if sleeping, the tick to resume at               */
    u32 entry;
};

static struct task tasks[MAX_TASKS];
static int current   = 0;
static int ntasks    = 0;
static int sched_on  = 0;
static u32 switches  = 0;

/* The context switch. Naked, because a C prologue would corrupt the very
 * stack we are rearranging.
 *
 * pushal/popal move all eight general registers as a block. Segment registers
 * are not saved: every task runs in the same flat segments, so they never
 * differ. Nor is the FPU/SSE state - zl numbers are doubles and this WOULD
 * matter for a task doing floating point across a switch, which is a real
 * limitation and the first thing to fix if a task ever computes wrong. */
#if defined(ZL_64)
/* x86-64. There is no pushal, so the callee-saved set is pushed by hand -
 * which is all the ABI requires a function to preserve anyway. The argument
 * registers differ between the System V and Microsoft conventions, and the
 * UEFI build uses the Microsoft one, so both are handled. */
__asm__(
    ".globl switch_to\n"
    "switch_to:\n"
#if defined(ZL_EFI)
    "    movq %rcx, %rax\n"        /* MS x64: rcx = &old->sp, rdx = new sp */
    "    movq %rdx, %r10\n"
#else
    "    movq %rdi, %rax\n"        /* System V: rdi, rsi                   */
    "    movq %rsi, %r10\n"
#endif
    "    pushq %rbx\n"
    "    pushq %rbp\n"
    "    pushq %r12\n"
    "    pushq %r13\n"
    "    pushq %r14\n"
    "    pushq %r15\n"
    "    movq %rsp, (%rax)\n"
    "    movq %r10, %rsp\n"
    "    popq %r15\n"
    "    popq %r14\n"
    "    popq %r13\n"
    "    popq %r12\n"
    "    popq %rbp\n"
    "    popq %rbx\n"
    "    ret\n");

__asm__(
    ".globl task_trampoline\n"
    "task_trampoline:\n"
    "    sti\n"
    "    ret\n");

#define SAVED_REGS 6

#else
/* 32-bit: pushal/popal move all eight general registers as a block. */
__asm__(
    ".globl switch_to\n"
    "switch_to:\n"
    "    movl 4(%esp), %eax\n"     /* &old->sp                        */
    "    movl 8(%esp), %edx\n"     /* new->sp                         */
    "    pushal\n"
    "    movl %esp, (%eax)\n"      /* remember where we left off      */
    "    movl %edx, %esp\n"        /* ...and stand on the other stack */
    "    popal\n"
    "    ret\n");                  /* returns into the OTHER task     */

__asm__(
    ".globl task_trampoline\n"
    "task_trampoline:\n"
    "    sti\n"                    /* a new task starts interruptible */
    "    ret\n");                  /* ret pops the entry point        */

#define SAVED_REGS 8
#endif

extern void switch_to(uptr *old_sp, uptr new_sp);

/* Where a task begins life: reached by the fabricated `ret` above, with the
 * entry point sitting on the stack right behind it. */
extern void task_trampoline(void);
void task_exit(void);

static uptr stack_top_of(int i)
{
    return (uptr)STACK_BASE + (uptr)(i + 1) * STACK_BYTES - 64;
}

/* Build a stack that looks like a task suspended inside switch_to.
 *
 * Layout, from the stack pointer upward: eight saved registers, then the
 * address switch_to's `ret` will jump to, then what THAT returns into.
 * pushal's order puts EDI lowest, so eight zeroes is all we need. */
int task_create(uptr entry)
{
    if (ntasks >= MAX_TASKS) return -1;
    int i = ntasks++;

    uptr top = stack_top_of(i);
    volatile uptr *sp = (volatile uptr *)top;

    *(--sp) = (uptr)task_exit;                  /* if the task ever returns */
    *(--sp) = entry;                            /* trampoline rets here     */
    *(--sp) = (uptr)task_trampoline;            /* switch_to rets here      */
    for (int r = 0; r < SAVED_REGS; r++) *(--sp) = 0;   /* the saved block  */

    tasks[i].sp      = (uptr)sp;
    tasks[i].state   = TASK_READY;
    tasks[i].ticks   = 0;
    tasks[i].wake_at = 0;
    tasks[i].entry   = entry;
    return i;
}

/* Task 0 is whatever was already running when the scheduler started - the
 * shell. It needs an entry in the table but not a stack: it is standing on
 * one already, and its sp gets filled in the first time it switches away. */
int sched_init(void)
{
    if (sched_on) return 1;
    ntasks = 0;
    int i = ntasks++;
    tasks[i].state = TASK_RUNNING;
    tasks[i].ticks = 0;
    tasks[i].entry = 0;
    current = 0;
    sched_on = 1;
    return 1;
}

/* Round robin over everything runnable. A sleeping task becomes runnable
 * again once the tick it asked for has arrived. */
static int pick_next(void)
{
    u32 now = idt_ticks();
    for (int n = 1; n <= ntasks; n++) {
        int i = (current + n) % ntasks;
        if (tasks[i].state == TASK_DONE || tasks[i].state == TASK_FREE) continue;
        if (tasks[i].wake_at && now < tasks[i].wake_at) continue;
        tasks[i].wake_at = 0;
        return i;
    }
    return current;
}

/* Give up the CPU. Everything above exists to make this one call safe. */
void yield(void)
{
    if (!sched_on || ntasks < 2) return;

    int prev = current;
    int next = pick_next();
    if (next == prev) return;

    if (tasks[prev].state == TASK_RUNNING) tasks[prev].state = TASK_READY;
    tasks[next].state = TASK_RUNNING;
    tasks[next].ticks++;
    current = next;
    switches++;

    switch_to(&tasks[prev].sp, tasks[next].sp);
    /* control returns here whenever this task is scheduled again */
}

/* Sleep for a number of 100 Hz ticks, letting everything else run. */
void task_sleep(u32 ticks)
{
    if (!sched_on) { u32 t = idt_ticks() + ticks; while (idt_ticks() < t) { } return; }
    tasks[current].wake_at = idt_ticks() + ticks;
    yield();
}

/* A task that runs off the end of its function lands here. There is no
 * reaping and no stack reuse yet - it simply stops being scheduled. */
void task_exit(void)
{
    tasks[current].state = TASK_DONE;
    for (;;) yield();
}

int sched_active(void)   { return sched_on; }
int sched_count(void)    { return ntasks; }
int sched_current(void)  { return current; }
u32 sched_switches(void) { return switches; }
int sched_state(int i)   { return (i >= 0 && i < ntasks) ? (int)tasks[i].state : 0; }
u32 sched_ticks(int i)   { return (i >= 0 && i < ntasks) ? tasks[i].ticks : 0; }

/* ---- something for the tasks to actually do ----------------------------
 * Three counters in memory, each incremented by its own task at its own rate.
 * The shell reads them. If they all advance while the shell is also running,
 * more than one thing is genuinely happening. */
#define COUNTER_BASE 0x0B0F0000u

static void counter_task(int slot, u32 delay)
{
    volatile u32 *c = (volatile u32 *)(uptr)(COUNTER_BASE + (u32)slot * 4);
    for (;;) {
        (*c)++;
        task_sleep(delay);
    }
}

static void task_a(void) { counter_task(0, 1);  }   /* every 10 ms */
static void task_b(void) { counter_task(1, 5);  }   /* every 50 ms */
static void task_c(void) { counter_task(2, 25); }   /* every 250 ms */

u32 sched_counter(int i)
{
    if (i < 0 || i > 2) return 0;
    return *(volatile u32 *)(uptr)(COUNTER_BASE + (u32)i * 4);
}

/* Start the demo tasks. Returns how many are now runnable. */
int sched_start_demo(void)
{
    sched_init();
    for (int i = 0; i < 3; i++)
        *(volatile u32 *)(uptr)(COUNTER_BASE + (u32)i * 4) = 0;
    task_create((uptr)task_a);
    task_create((uptr)task_b);
    task_create((uptr)task_c);
    return ntasks;
}
