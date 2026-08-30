/* usermode.c - ring 3, and the one door back into the kernel.
 *
 * Before this file, EVERYTHING in zlOS ran in ring 0. grep found no ring 3, no
 * syscall entry, and no user/kernel boundary of any kind: a zl program, the
 * compositor, the disk driver and the interrupt handlers all had identical
 * authority over the machine. arena.c's ceiling was the only thing standing
 * between a foreign program and the rest of memory, and a ceiling is a budget,
 * not a boundary - a program that ignores the arena and writes to 0x08000000
 * scribbles on the framebuffer, and nothing stops it.
 *
 * ============================================================================
 * WHAT THIS BUYS, EXACTLY, AND WHAT IT DOES NOT
 * ============================================================================
 * This is the sentence that matters, and it is the one people get wrong:
 *
 *   RING 3 IS PRIVILEGE SEPARATION. IT IS NOT MEMORY ISOLATION.
 *
 * What a ring-3 program on THIS build can no longer do - each of these is a
 * #GP fault instead of an instruction:
 *
 *     cli / sti                turn interrupts off and never give the CPU back
 *     hlt                      stop the machine
 *     in / out                 talk to any device directly (IOPL is 0 and the
 *                              TSS declares no I/O bitmap)
 *     lgdt / lidt / ltr        replace the descriptor tables
 *     mov to cr0/cr3/cr4       turn paging on or off, switch address spaces
 *     wrmsr / rdmsr            reprogram the CPU
 *     invlpg                   invalidate other people's TLB entries
 *
 * What it does NOT buy on the 32-bit build: memory isolation. Paging is OFF
 * here (CR0.PG is never set), the ring-3 segments are FLAT 4 GiB, and
 * segmentation with a flat segment isolates nothing. A ring-3 program can still
 * READ AND WRITE ANY ADDRESS. It simply cannot reprogram the machine.
 *
 * That is a real and useful boundary - it is the difference between a runaway
 * program that corrupts a buffer and one that disables interrupts and wedges
 * the box - but calling it "isolation" would be a lie, and this project has
 * been bitten enough times by a comment that claimed more than the code did.
 *
 * MEMORY isolation needs the page tables' U/S bit and a separate address space
 * per process. paging.c has the machinery; the 64-bit builds have the tables.
 * docs/reference/system/memory-model.md, Stage 5, is the account of what is left.
 *
 * ============================================================================
 * THE THREE THINGS THAT MUST ALL BE TRUE, OR RING 3 TRIPLE FAULTS
 * ============================================================================
 *   1. A TSS with ss0/esp0, LOADED with ltr. On the first interrupt taken in
 *      ring 3 the CPU reads the ring-0 stack out of it. gdt.c does this.
 *   2. An IDT gate with DPL 3 for the syscall vector. A ring-3 `int $0x80`
 *      through a DPL-0 gate is a #GP, not a syscall - the CPU refuses to let
 *      less-privileged code invoke a more-privileged gate it was not offered.
 *      idt.c does this for vector 0x80 and NOTHING ELSE, which is the point:
 *      every other vector stays DPL 0, so ring 3 cannot fake a page fault or a
 *      timer interrupt.
 *   3. An iret with a ring-3 frame. Below.
 *
 * Miss any one and the symptom is a silent reboot loop, because a fault while
 * handling a fault while handling a fault is a triple fault and the CPU just
 * resets. There is no message. That is why this file's self-test prints from
 * BOTH sides of the boundary - see user_selftest().
 */

typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

#include "telemetry.h"

void zl_putc_pub(char c);

/* THIS FILE IS 32-BIT ONLY, and that is a scope decision rather than a
 * limitation of the idea. The transition itself is nearly identical in long
 * mode, but the entry stub is hand-written assembly with a specific register
 * layout, and `syscall`/`sysret` (which is what 64-bit code should use rather
 * than `int`) needs three MSRs programmed and EFER.SCE set. Writing both and
 * being able to test only one is how a subsystem gets a version that has never
 * executed - this project already has several, and CLAUDE.md's rule is that
 * "the code exists" is not "the code works".
 *
 * The 32-bit build is the one three boot gates actually boot (verify.sh,
 * verify-raw.sh, and verify-iso.sh's BIOS case), so it is the build where a
 * privilege transition can be PROVEN by a transcript rather than asserted. The
 * 64-bit stub below says so out loud instead of pretending. */
static void up(const char *s) { while (*s) zl_putc_pub(*s++); }

__attribute__((unused)) static void upu(unsigned long v)
{
    char b[24];
    int i = 0;
    if (!v) { zl_putc_pub('0'); return; }
    while (v) { b[i++] = (char)('0' + (int)(v % 10UL)); v /= 10UL; }
    while (i) zl_putc_pub(b[--i]);
}

#if !defined(ZL_64)

u32  gdt_kernel_stack_top(void);

#define USER_CODE_SEL 0x1B     /* GDT index 3, RPL 3 */
#define USER_DATA_SEL 0x23     /* GDT index 4, RPL 3 */

/* ---- the syscall numbers --------------------------------------------------
 * Deliberately tiny. The point of this stage is the BOUNDARY, not the API: a
 * syscall table that grows before the boundary is proven is a table of
 * untested entry points. Three calls are enough to demonstrate that a ring-3
 * program can ask the kernel for something and get it. */
#define SYS_WRITE  1           /* ebx = char                                */
#define SYS_GETPID 2           /* -> eax                                     */
#define SYS_EXIT   3           /* leaves ring 3 for good                     */

static volatile int  user_exited = 0;
static volatile u32  user_calls  = 0;
static volatile u32  user_last   = 0;
/* NOT static: the entry stub below is hand-written assembly and refers to both
 * by name. A static would still have a symbol, but the compiler is free to
 * discard or rename one it thinks nothing reads, and "nothing reads it" is
 * exactly what it looks like from C. */
u32 user_return_esp = 0;   /* kernel ESP to resume on SYS_EXIT */
u32 user_return_eip = 0;   /* and where                         */
u32 user_saved_flags = 0;  /* EFLAGS from before the excursion   */

static u32 syscall_finish(u32 nr, unsigned operation_id, u32 result)
{
    int signed_result = (int)result;
    unsigned error = signed_result < 0 ? (unsigned)-signed_result : 0u;
    zlt_event(ZLLOG_SUB_SYSCALL, ZLLOG_EV_SYSCALL_EXIT,
              error ? ZLLOG_WARN : ZLLOG_INFO, nr, result, user_calls);
    zlt_operation_result(ZLLOG_SUB_SYSCALL, operation_id,
                         ZLLOG_OP_SYSCALL_BASE + nr, signed_result, error, 0u);
    return result;
}

/* ---- the syscall handler, called from the ISR stub in idt.c ---------------
 * Runs in RING 0, on the kernel stack the TSS named, with the user's registers
 * in the frame below. Everything a user program can influence arrives in
 * registers, and every one of them is untrusted.
 *
 * Returns the value for eax. */
u32 syscall_dispatch(u32 nr, u32 arg1, u32 arg2, u32 arg3)
{
    (void)arg2; (void)arg3;
    user_calls++;
    user_last = nr;
    zlt_count(ZLLOG_C_SYSCALL, 1);
    zlt_event(ZLLOG_SUB_SYSCALL, ZLLOG_EV_SYSCALL_ENTER, ZLLOG_INFO,
              nr, arg1, user_calls);
    unsigned operation_id = zlt_operation_begin(
        ZLLOG_SUB_SYSCALL, ZLLOG_OBJ_PROCESS, 1u,
        ZLLOG_OP_SYSCALL_BASE + nr, nr);

    switch (nr) {
    case SYS_WRITE:
        /* THE VALIDATION, and it is the whole reason a syscall is not just a
         * function call. arg1 arrives from ring 3 and is whatever the program
         * put in ebx. Masking to a byte is not politeness - zl_putc_pub takes
         * a char, and handing it a value a user chose is how a boundary
         * becomes a hole. There is no pointer in this ABI ON PURPOSE: a
         * pointer argument would need range-checking against an address space
         * this build does not have yet. */
        zl_putc_pub((char)(arg1 & 0x7F));
        return syscall_finish(nr, operation_id, 0u);

    case SYS_GETPID:
        return syscall_finish(nr, operation_id, 1u); /* one process */

    case SYS_EXIT:
        user_exited = 1;
        {
        u32 result = syscall_finish(nr, operation_id, 0u);
        zlt_lifecycle(ZLLOG_SUB_SCHED, ZLLOG_OBJ_PROCESS, 1u,
                      ZLLOG_LIFE_EXIT, 0u, user_calls);
        return result;
        }

    default:
        /* An unknown call is NOT a fault and NOT silent. Silence here is how a
         * program that thinks it wrote to a file gets no error and no file. */
        up("  syscall: ring 3 asked for unknown call ");
        upu(nr);
        up("\n");
        return syscall_finish(nr, operation_id, (u32)-1);
    }
}

int  user_has_exited(void)  { return user_exited; }
u32  user_call_count(void)  { return user_calls; }

/* ---- the ring-3 payload ---------------------------------------------------
 * Plain C, compiled into the kernel image like everything else, and then RUN AT
 * RING 3. It is here rather than loaded from disk because a loader is a
 * separate problem and this stage is about the privilege transition.
 *
 * It must not call anything. Every function in this kernel is linked at an
 * address ring 3 can reach - segments are flat - so a call would WORK, and
 * that is exactly the confusion to avoid: this routine talks to the kernel
 * ONLY through int $0x80, so what it demonstrates is the door, not the
 * absence of a wall.
 *
 * `volatile` on the asm and no static data: the compiler must not hoist
 * anything out, and .bss writes from here would work but would prove nothing.
 */
static void user_payload(void)
{
    u32 ret;

    /* EVERY syscall declares eax as an OUTPUT, and that is a bug fix, not
     * style. The first version wrote
     *
     *     __asm__ volatile("int $0x80" :: "a"(SYS_WRITE), "b"('u'));
     *
     * with eax as input only. The handler returns its value in eax - that is
     * the ABI - so after the first call eax held 0, and gcc, told only that
     * eax was an input it had already set up, did not reload it. The second
     * syscall therefore arrived as call number 0. The boot log said so exactly:
     *
     *     ring 3: u  syscall: ring 3 asked for unknown call 0
     *
     * which is the "unknown call" branch earning its place on its first outing.
     * Had that branch been silent - the tempting choice for an unused number -
     * the log would have read "u1" instead of "u31" and looked like a missing
     * character rather than a broken calling convention.
     *
     * "=a"(ret) tells gcc eax is written, so it reloads it for each call. */
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_WRITE), "b"('u') : "memory");
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_WRITE), "b"('3') : "memory");

    /* Ask for something and USE the answer, so the return path is exercised -
     * a syscall that only goes one way would pass with a broken iret. */
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_GETPID) : "memory");
    __asm__ volatile("int $0x80" : "=a"(ret)
                     : "a"(SYS_WRITE), "b"('0' + (ret & 7)) : "memory");

    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_EXIT) : "memory");

    /* SYS_EXIT does not return. If it ever does, spin here rather than falling
     * off the end of this function into whatever follows it in .text - at ring
     * 3 with flat segments that would simply execute the next function. */
    for (;;) __asm__ volatile("pause");
}

/* The user stack. 8 KiB in .bss - which ring 3 can also write, because nothing
 * is isolated yet. It is separate from the kernel stack anyway, because the
 * CPU switches stacks on entry and the two must not be the same memory. */
#define U_STACK_BYTES 8192
static u8 ustack[U_STACK_BYTES] __attribute__((aligned(16)));

/* ---- SYS_EXIT's way back --------------------------------------------------
 * A syscall returns with iret to wherever it came from - which for SYS_EXIT is
 * ring 3, and we do not want to go back. So the ISR checks user_has_exited()
 * and, if set, restores the kernel stack captured on the way in and returns
 * there instead. This is a longjmp in everything but name, and it is the same
 * shape exec.c already uses to stop a runaway program (see
 * docs/architecture/system/exec-kill-path.md) - deliberately, because a second mechanism for
 * "abandon what you were doing" is a second thing to get wrong. */
u32 user_exit_esp(void) { return user_return_esp; }

/* ---- the entry stub -------------------------------------------------------
 * Written as top-level assembly rather than a C function, because a syscall
 * entry has to save the user's registers BEFORE the compiler's prologue runs
 * and restore them AFTER its epilogue would. GCC has no `naked` attribute on
 * x86 - it is an ARM/AVR feature - so `__attribute__((naked))` would compile on
 * clang, silently do nothing on gcc, and produce an entry point that corrupts
 * every register a user program owns. An asm block behaves identically on both.
 *
 * THE STACK, from ESP after the pushes, because every offset below depends on
 * it and getting one wrong reads a user register as a syscall number:
 *
 *     esp+0  gs    esp+16 edi   esp+32 ebx
 *     esp+4  fs    esp+20 esi   esp+36 edx
 *     esp+8  es    esp+24 ebp   esp+40 ecx
 *     esp+12 ds    esp+28 esp   esp+44 eax   <- the syscall number, and where
 *                                               the return value goes back
 *
 * pusha pushes EAX first and EDI last, so EAX is at the HIGH end of the block.
 * That is the opposite of the order it is written in, and it is the single
 * easiest thing to get backwards here.
 *
 * The segment registers are reloaded to the ring-0 data selector immediately.
 * On entry ds/es/fs/gs still hold whatever ring 3 had - the CPU switches SS and
 * CS on a privilege change and nothing else - so any kernel memory access
 * before this point would run through a user selector.
 */
__asm__(
    ".text\n"
    ".globl syscall_isr\n"
    ".type syscall_isr, @function\n"
    "syscall_isr:\n"
    "    pusha\n"
    "    push %ds\n"
    "    push %es\n"
    "    push %fs\n"
    "    push %gs\n"
    "    mov $0x10, %ax\n"
    "    mov %ax, %ds\n"
    "    mov %ax, %es\n"
    "    mov %ax, %fs\n"
    "    mov %ax, %gs\n"
    /* load the four arguments out of the saved frame, THEN push - pushing
     * directly from an esp-relative slot would move esp under the next read */
    "    mov 44(%esp), %eax\n"        /* nr   */
    "    mov 32(%esp), %ebx\n"        /* arg1 */
    "    mov 40(%esp), %ecx\n"        /* arg2 */
    "    mov 36(%esp), %edx\n"        /* arg3 */
    "    push %edx\n"
    "    push %ecx\n"
    "    push %ebx\n"
    "    push %eax\n"
    "    call syscall_dispatch\n"
    "    add $16, %esp\n"
    "    mov %eax, 44(%esp)\n"        /* the return value, into saved EAX */
    /* SYS_EXIT does not iret. Going back would return to ring 3 at the
     * instruction after the int, and there is nothing there we want to run. */
    "    call user_has_exited\n"
    "    test %eax, %eax\n"
    "    jz 1f\n"
    "    mov user_return_esp, %esp\n"
    "    jmp *user_return_eip\n"
    "1:\n"
    "    pop %gs\n"
    "    pop %fs\n"
    "    pop %es\n"
    "    pop %ds\n"
    "    popa\n"
    "    iret\n"
    ".size syscall_isr, .-syscall_isr\n"
);

/* ---- into ring 3 ----------------------------------------------------------
 * The transition is one iret with a five-word frame. There is no instruction
 * for "drop privilege"; iret is it, because iret's whole job is to restore a
 * saved CS:EIP and EFLAGS, and if the CS it restores has RPL 3 the CPU lowers
 * privilege on the way out. The stack it needs is in the frame too, because a
 * ring change means a stack change.
 *
 *      SS       user data selector, RPL 3
 *      ESP      top of the user stack
 *      EFLAGS   with IF set - ring 3 with interrupts DISABLED would be a
 *               machine that never takes a timer tick again, and cli is one of
 *               the instructions ring 3 cannot use to fix it
 *      CS       user code selector, RPL 3
 *      EIP      where to start
 */
void user_enter(void)
{
    user_exited = 0;
    user_calls  = 0;
    zlt_lifecycle(ZLLOG_SUB_SCHED, ZLLOG_OBJ_PROCESS, 1u,
                  ZLLOG_LIFE_START, 0u, U_STACK_BYTES);

    __asm__ volatile(
        /* SAVE EFLAGS, AND THIS IS NOT BOOKKEEPING - IT IS THE BUG THAT COST
         * THIS STAGE ITS FIRST THREE BOOT GATES.
         *
         * The syscall vector is an INTERRUPT gate (0xEE), and an interrupt gate
         * CLEARS IF on entry - that is the difference between it and a trap
         * gate, and it is the right choice, because a trap gate would let a
         * timer tick re-enter the syscall handler.
         *
         * But SYS_EXIT does not iret. It restores ESP and jumps, because we are
         * abandoning ring 3 rather than returning from an interrupt - and a
         * jump restores no EFLAGS. So interrupts stayed OFF for the entire rest
         * of the boot. The kernel came up, printed "ready.", and then ignored
         * every keystroke and every timer tick forever, because the one
         * instruction that could turn interrupts back on is `sti` and nothing
         * ran it.
         *
         * The symptom was a gate reporting "kernel never halted - it hung" with
         * a boot log that looked completely healthy right up to the prompt.
         *
         * `sti` alone would be wrong: it would ENABLE interrupts for a caller
         * that had deliberately disabled them. Saving and restoring is the
         * honest version, and it costs two instructions. */
        "pushf\n\t"
        "pop %%eax\n\t"
        "mov %%eax, %2\n\t"

        /* Where SYS_EXIT should put us back. BOTH are needed: the stub restores
         * ESP and then jumps - there is no frame to iret to. */
        "movl $2f, %0\n\t"
        "movl %%esp, %1\n\t"

        /* Ring 3's data segments. SS comes from the iret frame; ds/es/fs/gs do
         * not, and a ring-3 program running with a ring-0 selector in ds faults
         * on its first memory access. */
        "mov %4, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"

        "push %4\n\t"           /* SS     */
        "push %5\n\t"           /* ESP    */
        "pushf\n\t"             /* EFLAGS, IF still set here                */
        "push %6\n\t"           /* CS     */
        "push %7\n\t"           /* EIP    */
        "iret\n\t"

        /* SYS_EXIT jumps here, with ESP restored by the stub. */
        "2:\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "push %2\n\t"           /* ...and the flags the caller had, IF and all */
        "popf\n\t"
        : "=m"(user_return_eip), "=m"(user_return_esp), "=m"(user_saved_flags)
        : "m"(user_return_eip),
          "i"(USER_DATA_SEL), "r"((u32)ustack + U_STACK_BYTES),
          "i"(USER_CODE_SEL), "r"(&user_payload)
        : "eax", "memory");
}

/* ---- the self-test --------------------------------------------------------
 * Prints from BOTH sides. "ring 3:" comes from ring 0 before the transition;
 * the "u3" and the pid digit are produced by syscalls made FROM ring 3 and can
 * be produced no other way; the summary is ring 0 again, after coming back.
 *
 * A boot transcript containing all three is the only evidence that the
 * transition happened, the door works, and the return works - and it is
 * evidence verify.sh's golden file pins, so a regression is a red gate rather
 * than something nobody notices. */
void user_selftest(void)
{
    up("  ring 3: ");
    user_enter();

    up(" <- from ring 3 via int 0x80, ");
    upu(user_calls);
    up(" syscalls, returned to ring 0\n");
}

#else  /* ZL_64 */

typedef unsigned long long u64;
typedef signed long long s64;
typedef unsigned char u8_64;

#define U64_P  1ULL
#define U64_W  2ULL
#define U64_U  4ULL
#define U64_NX (1ULL << 63)
#define U64_ADDR 0x000ffffffffff000ULL
#define U64_CODE_SEL 0x23
#define U64_DATA_SEL 0x1b

#include "user_syscalls_generated.h"

#define U64_HANDLES 8
#define U64_PROCS 2
#define U64_SAVED_QWORDS 20
#define U64_IPC_SLOTS 4
#define U64_IPC_BYTES 64
#define U64_IO_MAX 4096
#define U64_NAME_MAX 24
#define U64_KSTACK_PAGES 2
#define U64_KSTACK_BYTES (U64_KSTACK_PAGES * 4096)
#define U64_KSTACK_FILL 0xa5u
#define U64_ANON_PTE_FIRST 6U
#define U64_PAGE_BYTES 4096ULL
#define U64_EINVAL ((u64)-22)
#define U64_ENOENT ((u64)-2)
#define U64_ENOMEM ((u64)-12)
#define U64_ENOSPC ((u64)-28)
#define U64_EBADF  ((u64)-9)
#define U64_EAGAIN ((u64)-11)
#define U64_EIO    ((u64)-5)
#define U64_ENOSYS ((u64)-38)

#include "core/process_memory.h"
#include "core/anon_memory.h"

#define U64_PROCESS_FRAME_LIMIT (2U * PROCESS_MEMORY_PAGE_COUNT)
#define U64_ANON_FRAME_LIMIT ANON_MEMORY_PAGE_COUNT

_Static_assert(U64_ANON_PTE_FIRST == 6U,
               "anonymous window must follow the guarded kernel stack");
_Static_assert(U64_ANON_PTE_FIRST + ANON_MEMORY_PAGE_COUNT <= 512U,
               "anonymous window must fit one process page table");

struct ipc64_message {
    u32 from, bytes;
    u8_64 data[U64_IPC_BYTES];
};

struct process64 {
    u32 pid;
    u32 state;                 /* 0 empty, 1 runnable/running, 2 exited, 3 fault */
    u64 cr3, user_base, user_stack_top, kernel_stack_top;
    u32 calls, fault_vector;
    u32 fault_error;
    u32 bad_pointer_refused;
    u64 fault_address;
    u32 started, has_frame;
    u64 saved_frame[U64_SAVED_QWORDS];
    int handles[U64_HANDLES];       /* zlfs index + 1; zero means closed */
    struct ipc64_message inbox[U64_IPC_SLOTS];
    u32 inbox_head, inbox_tail, inbox_count, ipc_last_from;
    struct process_memory memory;
    struct anon_memory anonymous;
};

static struct process64 procs64[U64_PROCS];
static struct process64 *proc64;
static int proc64_index;
static u8_64 proc_io[U64_PROCS][U64_IO_MAX];
static u32 proc_kstack_last_used[U64_PROCS];
static u32 proc_kstack_high_water[U64_PROCS];

u64 user64_kernel_cr3, user64_process_cr3;
u64 user64_return_rsp, user64_return_rip, user64_return_rflags;
static volatile int user64_exited, user64_running, user64_faulted, user64_yielded;
static volatile int user64_preempt_on;
static u32 user64_preemptions;
static char user64_sched_trace[8];
static int user64_sched_trace_n, user64_sched_trace_on;

extern unsigned char user64_blob[], user64_blob_end[];
extern unsigned char user64_anon_probe[], user64_anon_probe_end[];
extern unsigned char user64_anon_reserved_fault[], user64_anon_reserved_fault_end[];
extern unsigned char user64_anon_released_fault[], user64_anon_released_fault_end[];
void __attribute__((sysv_abi)) user64_enter_asm(u64 rip, u64 rsp);
void __attribute__((sysv_abi)) user64_resume_asm(u64 *frame);
void user64_abort(void) __attribute__((noreturn));
extern void gdt64_set_kernel_stack(u64 top);
extern u64 gdt64_active_kernel_stack_top(void);
extern u32 idt_ticks(void);
extern void idt_timer_tick(void);
extern void yield(void);
extern int fs_mounted(void);
extern int fs_find(const char *name);
extern int fs_create(const char *name, u32 bytes);
extern int fs_read(int idx, void *dst, u32 max);
extern int fs_write(int idx, const void *src, u32 bytes);
extern int fs_delete(int idx);
extern int fs_rename(int idx, const char *name);
extern int fs_sync(void);
extern int fs_used(int idx);
extern u32 fs_size(int idx);
extern int fs_maxfiles(void);
extern int fs_name_byte(int idx, int i);
struct userwin_event64 { u32 type, code, x, y; };
extern int userwin_open(int owner, const char *title);
extern int userwin_present(int owner, int handle, const char *text, u32 bytes);
extern int userwin_poll(int owner, int handle, struct userwin_event64 *out);
extern int userwin_close(int owner, int handle);
extern void userwin_close_owner(int owner);
extern int userwin_test_inject(int owner, int handle, u32 type, u32 code, u32 x, u32 y);
extern int userwin_count(void);
extern int userwin_text_byte(int owner, int handle, int index);
extern int userwin_has_wm_window(int owner, int handle);

static u64 cr3_read64(void)
{
    u64 v; __asm__ volatile("mov %%cr3,%0" : "=r"(v)); return v;
}

static int process64_flush_anonymous(void *context)
{
    if (!context) return 0;
    u64 target = *(u64 *)context & U64_ADDR;
    if (!target) return 0;
    u64 current = cr3_read64();
    /* PCID is not enabled. An inactive process is flushed by its next CR3
     * load; the active process must discard translations before returning. */
    if ((current & U64_ADDR) == target)
        __asm__ volatile("mov %0,%%cr3" :: "r"(current) : "memory");
    return 1;
}

static void zero_page(u64 *p) { for (int i = 0; i < 512; i++) p[i] = 0; }

static void *process64_memory_pointer(const struct process_memory *memory,
                                      enum process_memory_page page)
{
    return (void *)(__UINTPTR_TYPE__)process_memory_page(memory, page);
}

static void *process64_page_pointer(int index, enum process_memory_page page)
{
    if (index < 0 || index >= U64_PROCS) return 0;
    return process64_memory_pointer(&procs64[index].memory, page);
}

static u8_64 *process64_kstack_byte(int index, u32 offset)
{
    enum process_memory_page page = offset < 4096U
        ? PROCESS_MEMORY_KERNEL_STACK_LOW
        : PROCESS_MEMORY_KERNEL_STACK_HIGH;
    u8_64 *base = (u8_64 *)process64_page_pointer(index, page);
    return base ? base + (offset & 4095U) : 0;
}

static int process64_anonymous_uninitialized(const struct anon_memory *memory)
{
    if (!memory || memory->entries || memory->pte_flags || memory->owner ||
        memory->reserved_count || memory->committed_count ||
        memory->broken_count || memory->fail_after_write || memory->flush ||
        memory->flush_context)
        return 0;
    for (unsigned int i = 0; i < ANON_MEMORY_PAGE_COUNT; i++)
        if (memory->pages[i] || memory->states[i]) return 0;
    return 1;
}

static int process64_release_anonymous(struct process64 *process)
{
    if (process64_anonymous_uninitialized(&process->anonymous)) return 1;
    return anon_memory_destroy(&process->anonymous) == ANON_MEMORY_OK;
}

static int process64_release_slot(int index)
{
    if (index < 0 || index >= U64_PROCS) return 0;
    struct process64 *process = &procs64[index];
    if (!process_memory_ready(&process->memory))
        return process->memory.acquired == 0 &&
               process64_anonymous_uninitialized(&process->anonymous);
    if (!process64_release_anonymous(process)) return 0;
    userwin_close_owner((int)process->pid);
    if (process_memory_release(&process->memory) != PROCESS_MEMORY_OK) return 0;
    process->pid = 0;
    process->state = 0;
    process->cr3 = 0;
    process->user_base = 0;
    process->user_stack_top = 0;
    process->kernel_stack_top = 0;
    return 1;
}

static int process64_prepare(int index, u32 pid)
{
    if (index < 0 || index >= U64_PROCS) return 0;
    u64 blob_bytes = (u64)(user64_blob_end - user64_blob);
    if (!blob_bytes || blob_bytes > PMM_PAGE_BYTES) return 0;
    u64 old = cr3_read64();
    u64 *live = (u64 *)(old & U64_ADDR);
    if (!live) return 0;
    int slot = -1;
    for (int i = 1; i < 255; i++) if (!(live[i] & U64_P)) { slot = i; break; }
    if (slot < 0) return 0;

    unsigned int process_owner = PROCESS_MEMORY_OWNER_BASE + (unsigned)index;
    unsigned int anonymous_owner = ANON_MEMORY_OWNER_BASE + (unsigned)index;
    /* Image replacement acquires the complete successor before releasing the
     * predecessor, so the fixed-frame account admits exactly two images. The
     * anonymous account is bounded to the one typed window it can publish. */
    if (pmm_set_owner_limit(process_owner, U64_PROCESS_FRAME_LIMIT) != PMM_OK ||
        pmm_set_owner_limit(anonymous_owner, U64_ANON_FRAME_LIMIT) != PMM_OK)
        return 0;

    struct process_memory next = {0};
    struct anon_memory next_anonymous = {0};
    if (process_memory_acquire(&next, process_owner) != PROCESS_MEMORY_OK)
        return 0;
    u64 *pml4 = (u64 *)process64_memory_pointer(&next, PROCESS_MEMORY_PML4);
    u64 *pdpt = (u64 *)process64_memory_pointer(&next, PROCESS_MEMORY_PDPT);
    u64 *pd = (u64 *)process64_memory_pointer(&next, PROCESS_MEMORY_PD);
    u64 *pt = (u64 *)process64_memory_pointer(&next, PROCESS_MEMORY_PT);
    u8_64 *code = (u8_64 *)process64_memory_pointer(&next, PROCESS_MEMORY_CODE);
    u8_64 *stack = (u8_64 *)process64_memory_pointer(&next,
                                                      PROCESS_MEMORY_USER_STACK);
    u8_64 *kstack_low = (u8_64 *)process64_memory_pointer(
        &next, PROCESS_MEMORY_KERNEL_STACK_LOW);
    u8_64 *kstack_high = (u8_64 *)process64_memory_pointer(
        &next, PROCESS_MEMORY_KERNEL_STACK_HIGH);
    if (!pml4 || !pdpt || !pd || !pt || !code || !stack ||
        !kstack_low || !kstack_high) {
        process_memory_release(&next);
        return 0;
    }

    for (int i = 0; i < 512; i++) pml4[i] = live[i];
    zero_page(pdpt); zero_page(pd); zero_page(pt);
    u64 base = (u64)(unsigned)slot << 39;
    pml4[slot] = ((u64)pdpt & U64_ADDR) | U64_P | U64_W | U64_U;
    pdpt[0] = ((u64)pd & U64_ADDR) | U64_P | U64_W | U64_U;
    pd[0] = ((u64)pt & U64_ADDR) | U64_P | U64_W | U64_U;
    pt[0] = ((u64)code & U64_ADDR) | U64_P | U64_U;
    /* PTE 1 is intentionally absent: the stack's lower guard page. */
    pt[2] = ((u64)stack & U64_ADDR) | U64_P | U64_W | U64_U | U64_NX;
    /* PTE 3 is intentionally absent: the TSS kernel stack's lower guard. */
    pt[4] = ((u64)kstack_low & U64_ADDR) | U64_P | U64_W | U64_NX;
    pt[5] = ((u64)kstack_high & U64_ADDR) | U64_P | U64_W | U64_NX;

    if (anon_memory_init(&next_anonymous, anonymous_owner,
                         &pt[U64_ANON_PTE_FIRST], U64_P | U64_W | U64_U | U64_NX,
                         process64_flush_anonymous, &procs64[index].cr3) !=
        ANON_MEMORY_OK) {
        process_memory_release(&next);
        return 0;
    }

    for (u64 i = 0; i < blob_bytes; i++) code[i] = user64_blob[i];
    for (u64 i = blob_bytes; i < PMM_PAGE_BYTES; i++) code[i] = 0xcc;
    for (u32 i = 0; i < PMM_PAGE_BYTES; i++) stack[i] = 0;
    for (u32 i = 0; i < PMM_PAGE_BYTES; i++) {
        kstack_low[i] = U64_KSTACK_FILL;
        kstack_high[i] = U64_KSTACK_FILL;
    }

    if (process_memory_ready(&procs64[index].memory) &&
        (!process64_release_anonymous(&procs64[index]) ||
         process_memory_release(&procs64[index].memory) != PROCESS_MEMORY_OK)) {
        process_memory_release(&next);
        return 0;
    }
    if (procs64[index].state && procs64[index].pid)
        userwin_close_owner((int)procs64[index].pid);
    proc_kstack_last_used[index] = 0;

    proc64_index = index;
    proc64 = &procs64[index];
    proc64->memory = next;
    proc64->anonymous = next_anonymous;
    proc64->pid = pid; proc64->state = 1;
    proc64->cr3 = (u64)pml4 & U64_ADDR;
    proc64->user_base = base;
    proc64->user_stack_top = base + 3 * 4096ULL;
    proc64->kernel_stack_top = base + 6 * 4096ULL;
    proc64->calls = 0; proc64->fault_vector = 0; proc64->fault_error = 0;
    proc64->fault_address = 0; proc64->bad_pointer_refused = 0;
    proc64->started = proc64->has_frame = 0;
    for (int i = 0; i < U64_HANDLES; i++) proc64->handles[i] = 0;
    proc64->inbox_head = proc64->inbox_tail = proc64->inbox_count = 0;
    proc64->ipc_last_from = 0;
    user64_kernel_cr3 = old;
    user64_process_cr3 = proc64->cr3;
    return 1;
}

static void process64_select(int index)
{
    proc64_index = index;
    proc64 = &procs64[index];
    user64_process_cr3 = proc64->cr3;
    gdt64_set_kernel_stack(proc64->kernel_stack_top);
}

static void process64_observe_kernel_stack(int index)
{
    u32 first_changed = U64_KSTACK_BYTES;
    for (u32 i = 0; i < U64_KSTACK_BYTES; i++) {
        u8_64 *byte = process64_kstack_byte(index, i);
        if (!byte || *byte != U64_KSTACK_FILL) {
            first_changed = i;
            break;
        }
    }
    proc_kstack_last_used[index] = first_changed == U64_KSTACK_BYTES
        ? 0 : U64_KSTACK_BYTES - first_changed;
    if (proc_kstack_last_used[index] > proc_kstack_high_water[index])
        proc_kstack_high_water[index] = proc_kstack_last_used[index];
}

static int process64_kernel_stack_contract(int index)
{
    const u64 mask = U64_ADDR | U64_P | U64_W | U64_U | U64_NX;
    u64 *pt = (u64 *)process64_page_pointer(index, PROCESS_MEMORY_PT);
    const u64 first = ((u64)process64_page_pointer(
                           index, PROCESS_MEMORY_KERNEL_STACK_LOW) & U64_ADDR) |
                      U64_P | U64_W | U64_NX;
    const u64 second = ((u64)process64_page_pointer(
                            index, PROCESS_MEMORY_KERNEL_STACK_HIGH) & U64_ADDR) |
                       U64_P | U64_W | U64_NX;
    return pt && pt[3] == 0 &&
           (pt[4] & mask) == first &&
           (pt[5] & mask) == second &&
           procs64[index].kernel_stack_top == procs64[index].user_base + 6 * 4096ULL;
}

/* Complete-range validation happens before the first byte is touched. */
static int user64_range(u64 addr, u64 bytes, int writing)
{
    if (!bytes || addr + bytes < addr) return 0;
    u64 b = proc64->user_base;
    if (!writing && addr >= b && addr + bytes <= b + 4096) return 1;
    if (addr >= b + 8192 && addr + bytes <= b + 12288) return 1;
    u64 anonymous_base = b + U64_ANON_PTE_FIRST * U64_PAGE_BYTES;
    u64 anonymous_end = anonymous_base +
                        ANON_MEMORY_PAGE_COUNT * U64_PAGE_BYTES;
    if (addr >= anonymous_base && addr + bytes <= anonymous_end) {
        unsigned int first = (unsigned int)((addr - anonymous_base) /
                                             U64_PAGE_BYTES);
        unsigned int last = (unsigned int)((addr + bytes - 1 - anonymous_base) /
                                            U64_PAGE_BYTES);
        for (unsigned int page = first; page <= last; page++)
            if (anon_memory_state(&proc64->anonymous, page) !=
                ANON_MEMORY_COMMITTED)
                return 0;
        return 1;
    }
    return 0;
}

int copy_from_user(void *dst, u64 src, u32 bytes)
{
    if (!dst || !user64_range(src, bytes, 0)) return 0;
    u8_64 *d = (u8_64 *)dst; const u8_64 *s = (const u8_64 *)src;
    for (u32 i = 0; i < bytes; i++) d[i] = s[i];
    return 1;
}

int copy_to_user(u64 dst, const void *src, u32 bytes)
{
    if (!src || !user64_range(dst, bytes, 1)) return 0;
    u8_64 *d = (u8_64 *)dst; const u8_64 *s = (const u8_64 *)src;
    for (u32 i = 0; i < bytes; i++) d[i] = s[i];
    return 1;
}

static int user64_name(char out[U64_NAME_MAX], u64 ptr, u64 len)
{
    if (!ptr || !len || len >= U64_NAME_MAX) return 0;
    if (!copy_from_user(out, ptr, (u32)len)) return 0;
    for (u64 i = 0; i < len; i++) if (!out[i]) return 0;
    out[len] = 0;
    return 1;
}

static int user64_handle(int h)
{
    if (h <= 0 || h > U64_HANDLES || !proc64->handles[h - 1]) return -1;
    return proc64->handles[h - 1] - 1;
}

static int user64_open_handle(int idx)
{
    for (int i = 0; i < U64_HANDLES; i++) if (!proc64->handles[i]) {
        proc64->handles[i] = idx + 1;
        return i + 1;
    }
    return -1;
}

static struct process64 *process64_pid(u32 pid)
{
    for (int i = 0; i < U64_PROCS; i++)
        if (procs64[i].state == 1 && procs64[i].pid == pid) return &procs64[i];
    return 0;
}

static u64 user64_finish(u64 nr, unsigned operation_id, u64 value)
{
    s64 signed_value = (s64)value;
    unsigned error = signed_value < 0 ? (unsigned)-signed_value : 0u;
    zlt_event(ZLLOG_SUB_SYSCALL, ZLLOG_EV_SYSCALL_EXIT,
              error ? ZLLOG_WARN : ZLLOG_INFO,
              (u32)nr, (u32)value, proc64->calls);
    zlt_operation_result(ZLLOG_SUB_SYSCALL, operation_id,
                         ZLLOG_OP_SYSCALL_BASE + (u32)nr,
                         (int)signed_value, error, 0u);
    return value;
}

static u64 user64_anonymous_status(int status)
{
    if (status == ANON_MEMORY_OK) return 0;
    if (status == ANON_MEMORY_E_NOMEM) return U64_ENOMEM;
    if (status == ANON_MEMORY_E_TRANSACTION ||
        status == ANON_MEMORY_E_CORRUPT)
        return U64_EIO;
    return U64_EINVAL;
}

u64 __attribute__((sysv_abi)) user64_dispatch(u64 nr, u64 arg1,
                                              u64 arg2, u64 arg3)
{
    proc64->calls++;
    zlt_count(ZLLOG_C_SYSCALL, 1);
    zlt_event(ZLLOG_SUB_SYSCALL, ZLLOG_EV_SYSCALL_ENTER, ZLLOG_INFO,
              (u32)nr, (u32)arg1, proc64->calls);
    unsigned operation_id = zlt_operation_begin(
        ZLLOG_SUB_SYSCALL, ZLLOG_OBJ_PROCESS, proc64->pid,
        ZLLOG_OP_SYSCALL_BASE + (u32)nr, (u32)nr);
#define U64_RETURN(value) return user64_finish(nr, operation_id, (value))
    if (!zlos_u64_syscall_known(nr)) U64_RETURN(U64_ENOSYS);
    if (nr == U64_SYS_WRITE) {
        char c = (char)(arg1 & 0x7f);
        if (user64_sched_trace_on && user64_sched_trace_n < (int)sizeof user64_sched_trace)
            user64_sched_trace[user64_sched_trace_n++] = c;
        zl_putc_pub(c);
        U64_RETURN(0);
    }
    if (nr == U64_SYS_GETPID) U64_RETURN(proc64->pid);
    if (nr == U64_SYS_EXIT) {
        userwin_close_owner((int)proc64->pid);
        user64_exited = 1; proc64->state = 2;
        u64 result = user64_finish(nr, operation_id, 0);
        zlt_lifecycle(ZLLOG_SUB_SCHED, ZLLOG_OBJ_PROCESS, proc64->pid,
                      ZLLOG_LIFE_EXIT, 0u, proc64->calls);
        return result;
    }
    if (nr == U64_SYS_COPY) {
        if (!user64_range(arg1, arg2, 0)) {
            proc64->bad_pointer_refused++;
            U64_RETURN((u64)-1);
        }
        U64_RETURN(0);
    }
    if (nr == U64_SYS_TIME) U64_RETURN(idt_ticks());
    if (nr == U64_SYS_YIELD) {
        user64_yielded = 1;
        proc64->state = 1;
        U64_RETURN(0);
    }

    /* The file ABI is intentionally whole-file and bounded. READ copies at
     * most 4 KiB from the beginning; WRITE atomically replaces the whole
     * file through zlfs's data-before-metadata path. This is a stable useful
     * contract without pretending the current flat filesystem is POSIX. */
    if (nr == U64_SYS_OPEN) {
        char name[U64_NAME_MAX];
        if (!fs_mounted() || !user64_name(name, arg1, arg2)) U64_RETURN(U64_EINVAL);
        int idx = fs_find(name);
        if (idx < 0 && (arg3 & 1ULL)) idx = fs_create(name, 0);
        if (idx < 0) U64_RETURN(U64_ENOENT);
        int h = user64_open_handle(idx);
        U64_RETURN(h < 0 ? U64_ENOSPC : (u64)h);
    }
    if (nr == U64_SYS_READ) {
        int idx = user64_handle((int)arg1);
        if (idx < 0) U64_RETURN(U64_EBADF);
        if (!arg3 || arg3 > U64_IO_MAX || fs_size(idx) > arg3) U64_RETURN(U64_EINVAL);
        int n = fs_read(idx, proc_io[proc64_index], (u32)arg3);
        if (n < 0 || !copy_to_user(arg2, proc_io[proc64_index], (u32)n)) U64_RETURN(U64_EIO);
        U64_RETURN((u64)n);
    }
    if (nr == U64_SYS_WRITEF) {
        int idx = user64_handle((int)arg1);
        if (idx < 0) U64_RETURN(U64_EBADF);
        if (arg3 > U64_IO_MAX ||
            (arg3 && !copy_from_user(proc_io[proc64_index], arg2, (u32)arg3)))
            U64_RETURN(U64_EINVAL);
        U64_RETURN(fs_write(idx, proc_io[proc64_index], (u32)arg3) ? arg3 : U64_EIO);
    }
    if (nr == U64_SYS_CLOSE) {
        if (arg1 == 0 || arg1 > U64_HANDLES || !proc64->handles[arg1 - 1])
            U64_RETURN(U64_EBADF);
        proc64->handles[arg1 - 1] = 0;
        U64_RETURN(0);
    }
    if (nr == U64_SYS_INFO) {
        if (arg1 >= (u64)fs_maxfiles() || !fs_used((int)arg1) ||
            !arg2 || !arg3 || arg3 > U64_NAME_MAX) U64_RETURN(U64_ENOENT);
        u32 n = 0;
        while (n + 1 < (u32)arg3) {
            int c = fs_name_byte((int)arg1, (int)n);
            proc_io[proc64_index][n++] = (u8_64)c;
            if (!c) break;
        }
        if (!n || proc_io[proc64_index][n - 1]) proc_io[proc64_index][n++] = 0;
        if (!copy_to_user(arg2, proc_io[proc64_index], n)) U64_RETURN(U64_EINVAL);
        U64_RETURN(fs_size((int)arg1));
    }
    if (nr == U64_SYS_REMOVE) {
        char name[U64_NAME_MAX];
        if (!user64_name(name, arg1, arg2)) U64_RETURN(U64_EINVAL);
        int idx = fs_find(name);
        U64_RETURN(idx >= 0 && fs_delete(idx) ? 0 : U64_ENOENT);
    }
    if (nr == U64_SYS_RENAME) {
        int idx = user64_handle((int)arg1);
        char name[U64_NAME_MAX];
        if (idx < 0) U64_RETURN(U64_EBADF);
        if (!user64_name(name, arg2, arg3)) U64_RETURN(U64_EINVAL);
        U64_RETURN(fs_rename(idx, name) ? 0 : U64_EIO);
    }
    if (nr == U64_SYS_SYNC) U64_RETURN(fs_sync() ? 0 : U64_EIO);
    if (nr == U64_SYS_SEND) {
        struct process64 *to = process64_pid((u32)arg1);
        if (!to) U64_RETURN(U64_ENOENT);
        if (!arg3 || arg3 > U64_IPC_BYTES || !user64_range(arg2, arg3, 0))
            U64_RETURN(U64_EINVAL);
        if (to->inbox_count >= U64_IPC_SLOTS) U64_RETURN(U64_ENOSPC);
        struct ipc64_message *m = &to->inbox[to->inbox_tail];
        if (!copy_from_user(m->data, arg2, (u32)arg3)) U64_RETURN(U64_EINVAL);
        m->from = proc64->pid; m->bytes = (u32)arg3;
        to->inbox_tail = (to->inbox_tail + 1u) % U64_IPC_SLOTS;
        to->inbox_count++;
        U64_RETURN(arg3);
    }
    if (nr == U64_SYS_RECV) {
        if (!proc64->inbox_count) U64_RETURN(U64_EAGAIN);
        struct ipc64_message *m = &proc64->inbox[proc64->inbox_head];
        if (!arg1 || arg2 < m->bytes || !copy_to_user(arg1, m->data, m->bytes))
            U64_RETURN(U64_EINVAL);
        proc64->ipc_last_from = m->from;
        u32 bytes = m->bytes;
        proc64->inbox_head = (proc64->inbox_head + 1u) % U64_IPC_SLOTS;
        proc64->inbox_count--;
        U64_RETURN(bytes);
    }
    if (nr == U64_SYS_FROM) U64_RETURN(proc64->ipc_last_from);
    if (nr == U64_SYS_WIN_OPEN) {
        char title[U64_NAME_MAX];
        if (!user64_name(title, arg1, arg2)) U64_RETURN(U64_EINVAL);
        int handle = userwin_open((int)proc64->pid, title);
        U64_RETURN(handle > 0 ? (u64)handle : U64_ENOSPC);
    }
    if (nr == U64_SYS_WIN_PRESENT) {
        if (!arg3 || arg3 >= 256 ||
            !copy_from_user(proc_io[proc64_index], arg2, (u32)arg3)) U64_RETURN(U64_EINVAL);
        proc_io[proc64_index][arg3] = 0;
        U64_RETURN(userwin_present((int)proc64->pid, (int)arg1,
                               (const char *)proc_io[proc64_index], (u32)arg3)
                   ? 0 : U64_EBADF);
    }
    if (nr == U64_SYS_WIN_POLL) {
        struct userwin_event64 event;
        if (arg3 < sizeof event) U64_RETURN(U64_EINVAL);
        if (!userwin_poll((int)proc64->pid, (int)arg1, &event)) U64_RETURN(U64_EAGAIN);
        U64_RETURN(copy_to_user(arg2, &event, sizeof event) ? sizeof event : U64_EINVAL);
    }
    if (nr == U64_SYS_WIN_CLOSE)
        U64_RETURN(userwin_close((int)proc64->pid, (int)arg1) ? 0 : U64_EBADF);
    if (nr == U64_SYS_ANON_RESERVE) {
        if (arg1 >= ANON_MEMORY_PAGE_COUNT || !arg2 ||
            arg2 > ANON_MEMORY_PAGE_COUNT - arg1)
            U64_RETURN(U64_EINVAL);
        int status = anon_memory_reserve(&proc64->anonymous,
                                         (unsigned int)arg1,
                                         (unsigned int)arg2);
        if (status != ANON_MEMORY_OK)
            U64_RETURN(user64_anonymous_status(status));
        U64_RETURN(proc64->user_base +
                   (U64_ANON_PTE_FIRST + arg1) * U64_PAGE_BYTES);
    }
    if (nr == U64_SYS_ANON_COMMIT) {
        if (arg1 >= ANON_MEMORY_PAGE_COUNT || !arg2 ||
            arg2 > ANON_MEMORY_PAGE_COUNT - arg1)
            U64_RETURN(U64_EINVAL);
        U64_RETURN(user64_anonymous_status(anon_memory_commit(
            &proc64->anonymous, (unsigned int)arg1, (unsigned int)arg2)));
    }
    if (nr == U64_SYS_ANON_RELEASE) {
        if (arg1 >= ANON_MEMORY_PAGE_COUNT || !arg2 ||
            arg2 > ANON_MEMORY_PAGE_COUNT - arg1)
            U64_RETURN(U64_EINVAL);
        U64_RETURN(user64_anonymous_status(anon_memory_release(
            &proc64->anonymous, (unsigned int)arg1, (unsigned int)arg2)));
    }
    U64_RETURN(U64_ENOSYS);
#undef U64_RETURN
}

int user64_is_running(void) { return user64_running; }
void user64_mark_fault(u32 vector, u32 error, u64 address)
{
    user64_faulted = 1; user64_running = 0;
    userwin_close_owner((int)proc64->pid);
    proc64->state = 3; proc64->fault_vector = vector;
    proc64->fault_error = error; proc64->fault_address = address;
}

int user_has_exited(void) { return user64_exited; }
u32 user_call_count(void) { return proc64 ? proc64->calls : 0; }
int user64_faulted_out(void) { return user64_faulted; }

/* Called with RSP at the first saved register in syscall_isr. A cooperative
 * yield snapshots every register plus the five-qword privilege-return frame;
 * the inactive process cannot borrow this stack because each process owns its
 * own saved frame and TSS ring-0 stack. */
int __attribute__((sysv_abi)) user64_after_syscall(u64 *frame)
{
    if (user64_yielded) {
        for (int i = 0; i < U64_SAVED_QWORDS; i++) proc64->saved_frame[i] = frame[i];
        proc64->has_frame = 1;
    }
    return user64_exited || user64_yielded;
}

int __attribute__((sysv_abi)) user64_timer_dispatch(u64 *frame)
{
    idt_timer_tick();
    /* saved[15..19] are RIP, CS, RFLAGS, RSP, SS after the 15 general
     * registers. Kernel-mode ticks have no process return stack to schedule. */
    if (!user64_preempt_on || !user64_running || !proc64 ||
        (frame[16] & 3u) != 3u) return 0;
    for (int i = 0; i < U64_SAVED_QWORDS; i++) proc64->saved_frame[i] = frame[i];
    proc64->has_frame = 1;
    proc64->state = 1;
    user64_preemptions++;
    return 1;
}

static int user64_load_process(int index, u32 pid, const u8_64 *code, u32 bytes)
{
    if (!code || !bytes || bytes > PMM_PAGE_BYTES ||
        !process64_prepare(index, pid)) return 0;
    u8_64 *page = (u8_64 *)process64_page_pointer(index, PROCESS_MEMORY_CODE);
    if (!page) return 0;
    for (u32 i = 0; i < bytes; i++) page[i] = code[i];
    for (u32 i = bytes; i < PMM_PAGE_BYTES; i++) page[i] = 0xcc;
    zlt_lifecycle(ZLLOG_SUB_SCHED, ZLLOG_OBJ_PROCESS, pid,
                  ZLLOG_LIFE_START, 0u, bytes);
    return 1;
}

/* Run until one syscall boundary, exit, or fault. Yielded register/iret state
 * lives in the process object and is resumed under that process's CR3. */
static int user64_step(int index)
{
    process64_select(index);
    if (proc64->state != 1) return 0;
    user64_exited = user64_faulted = user64_yielded = 0;
    user64_running = 1;
    if (!proc64->started) {
        proc64->started = 1;
        zlt_lifecycle(ZLLOG_SUB_SCHED, ZLLOG_OBJ_PROCESS, proc64->pid,
                      ZLLOG_LIFE_READY, 0u, proc64->calls);
        user64_enter_asm(proc64->user_base, proc64->user_stack_top);
    } else if (proc64->has_frame) {
        proc64->has_frame = 0;
        user64_resume_asm(proc64->saved_frame);
    }
    user64_running = 0;
    process64_observe_kernel_stack(index);
    /* user64_mark_fault runs inside an exception and therefore may not append
     * to the normal recorder ring. user64_abort returns here in ordinary
     * kernel context; publish the typed lifecycle boundary only now. */
    if (proc64->state == 3)
        zlt_lifecycle(ZLLOG_SUB_SCHED, ZLLOG_OBJ_PROCESS, proc64->pid,
                      ZLLOG_LIFE_FAULT, 0u, proc64->fault_vector);
    return proc64->state == 3 ? -(int)proc64->fault_vector : (int)proc64->state;
}

static int user64_run_probe(const u8_64 *code, u32 bytes)
{
    if (!user64_load_process(0, 1, code, bytes)) return -1;
    for (int turns = 0; turns < 16 && procs64[0].state == 1; turns++)
        user64_step(0);
    process64_select(0);
    return proc64->state == 3 ? (int)proc64->fault_vector : 0;
}

/* Load an ordinary raw x86-64 user image from zlfs. The format is deliberately
 * one RX page with entry at byte zero: relocations/shared libraries wait until
 * there is more than one real program asking for them. The important fact is
 * that these bytes were not linked into the kernel and still execute behind
 * the exact same page-table/syscall/fault boundary as the built-in probe. */
int user64_run_default_file(void)
{
    static const char name[] = "/system/user.bin";
    if (!fs_mounted()) return -1;
    int idx = fs_find(name);
    if (idx < 0) return -2;
    u32 n = fs_size(idx);
    if (!n || n > sizeof proc_io[0] || fs_read(idx, proc_io[0], n) != (int)n) return -3;
    return user64_run_probe(proc_io[0], n);
}

void user_selftest(void)
{
    for (int i = 0; i < U64_PROCS; i++)
        if (!process64_release_slot(i)) {
            up("  process frame cleanup refused before selftest\n");
            return;
        }
    unsigned long process_frame_baseline = pmm_used_pages();
    for (int i = 0; i < U64_PROCS; i++) {
        proc_kstack_last_used[i] = 0;
        proc_kstack_high_water[i] = 0;
    }
    up("  ring 3 64: ");
    u32 blob_bytes = (u32)(user64_blob_end - user64_blob);
    if (user64_run_probe(user64_blob, blob_bytes) < 0) {
        up("page-table setup refused\n"); return;
    }
    if (proc64->state == 3) {
        up("process faulted alone, vector "); upu(proc64->fault_vector); up("\n");
        return;
    }
    up(" <- iretq/int80/iretq, "); upu(proc64->calls);
    up(" syscalls, process exited, kernel alive\n");

    /* Unknown syscall IDs must have one unsigned behavior. This Ring-3 image
     * probes zero, the first gap, the sign bit and all bits set; each result
     * is normalized by adding ENOSYS and ORed into RBX. A non-zero aggregate
     * reaches UD2 instead of SYS_EXIT, making the target gate fail. */
    static const u8_64 unknown_syscalls[] = {
        0x31,0xdb,
        0xb8,0,0,0,0, 0xcd,0x80, 0x48,0x83,0xc0,0x26, 0x48,0x09,0xc3,
        0xb8,25,0,0,0, 0xcd,0x80, 0x48,0x83,0xc0,0x26, 0x48,0x09,0xc3,
        0x48,0xb8, 0,0,0,0,0,0,0,0x80,
        0xcd,0x80, 0x48,0x83,0xc0,0x26, 0x48,0x09,0xc3,
        0x48,0xc7,0xc0, 0xff,0xff,0xff,0xff,
        0xcd,0x80, 0x48,0x83,0xc0,0x26, 0x48,0x09,0xc3,
        0x48,0x85,0xdb, 0x75,0x09,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b,
        0x0f,0x0b
    };
    int unknown_result = user64_run_probe(unknown_syscalls, sizeof unknown_syscalls);
    if (unknown_result == 0 && proc64->state == 2)
        up("  syscall ABI: zero/gap/sign-bit/max refused with ENOSYS\n");
    else
        up("  syscall ABI: unknown-number ENOSYS gate FAILED\n");

    /* Ring 3 reserves two pages without mappings, commits zero-filled PMM
     * frames, crosses the page boundary through copy validation, releases
     * each page, and confirms a released page is no longer a valid buffer. */
    up("  anonymous memory: ");
    int anonymous_result = user64_run_probe(
        user64_anon_probe,
        (u32)(user64_anon_probe_end - user64_anon_probe));
    if (anonymous_result == 0 && proc64->state == 2 &&
        anon_memory_ready(&proc64->anonymous) &&
        proc64->anonymous.reserved_count == 0 &&
        proc64->anonymous.committed_count == 0)
        up(" <- reserve/commit zero-fill, cross-page copy and release passed\n");
    else
        up(" <- anonymous reserve/commit/release FAILED\n");

    static const u8_64 anonymous_sibling_v[] = {
        0xb8,1,0,0,0, 0xbb,'V',0,0,0, 0xcd,0x80,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b
    };
    int arv = user64_load_process(
        0, 7, user64_anon_reserved_fault,
        (u32)(user64_anon_reserved_fault_end - user64_anon_reserved_fault));
    int arv_sibling = user64_load_process(
        1, 8, anonymous_sibling_v, sizeof anonymous_sibling_v);
    u64 reserved_address = arv ? procs64[0].user_base +
                               U64_ANON_PTE_FIRST * U64_PAGE_BYTES : 0;
    user64_sched_trace_n = 0; user64_sched_trace_on = 1;
    if (arv && arv_sibling) { user64_step(0); user64_step(1); }
    user64_sched_trace_on = 0;
    if (arv && arv_sibling && procs64[0].state == 3 &&
        procs64[0].fault_vector == 14 && procs64[0].fault_error == 0x4u &&
        procs64[0].fault_address == reserved_address &&
        anon_memory_state(&procs64[0].anonymous, 0) == ANON_MEMORY_RESERVED &&
        procs64[1].state == 2 && user64_sched_trace_n == 1 &&
        user64_sched_trace[0] == 'V')
        up(" <- reserved anonymous page stayed absent; sibling V exited\n");
    else
        up(" <- reserved anonymous page fault containment FAILED\n");

    static const u8_64 anonymous_sibling_r[] = {
        0xb8,1,0,0,0, 0xbb,'R',0,0,0, 0xcd,0x80,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b
    };
    int arr = user64_load_process(
        0, 9, user64_anon_released_fault,
        (u32)(user64_anon_released_fault_end - user64_anon_released_fault));
    int arr_sibling = user64_load_process(
        1, 10, anonymous_sibling_r, sizeof anonymous_sibling_r);
    u64 released_address = arr ? procs64[0].user_base +
                               U64_ANON_PTE_FIRST * U64_PAGE_BYTES : 0;
    user64_sched_trace_n = 0; user64_sched_trace_on = 1;
    if (arr && arr_sibling) { user64_step(0); user64_step(1); }
    user64_sched_trace_on = 0;
    if (arr && arr_sibling && procs64[0].state == 3 &&
        procs64[0].fault_vector == 14 && procs64[0].fault_error == 0x4u &&
        procs64[0].fault_address == released_address &&
        anon_memory_state(&procs64[0].anonymous, 0) == ANON_MEMORY_FREE &&
        procs64[1].state == 2 && user64_sched_trace_n == 1 &&
        user64_sched_trace[0] == 'R')
        up(" <- released anonymous page faulted exactly; sibling R exited\n");
    else
        up(" <- released anonymous page fault containment FAILED\n");

    /* Two independent process objects alternate at cooperative yield
     * boundaries. Each owns its PML4, code, user/kernel stack, saved frame and
     * handles; the trace proves resume continues after the syscall rather than
     * restarting either image. */
    static const u8_64 p1[] = {
        0xb8,1,0,0,0, 0xbb,'A',0,0,0, 0xcd,0x80,
        0xb8,6,0,0,0, 0xcd,0x80,
        0xb8,2,0,0,0, 0xcd,0x80, 0x83,0xc0,'0', 0x89,0xc3,
        0xb8,1,0,0,0, 0xcd,0x80,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b
    };
    static const u8_64 p2[] = {
        0xb8,1,0,0,0, 0xbb,'B',0,0,0, 0xcd,0x80,
        0xb8,6,0,0,0, 0xcd,0x80,
        0xb8,2,0,0,0, 0xcd,0x80, 0x83,0xc0,'0', 0x89,0xc3,
        0xb8,1,0,0,0, 0xcd,0x80,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b
    };
    int p1ok = user64_load_process(0, 1, p1, sizeof p1);
    int p2ok = user64_load_process(1, 2, p2, sizeof p2);
    user64_sched_trace_n = 0; user64_sched_trace_on = 1;
    if (p1ok && p2ok) {
        user64_step(0); user64_step(1);
        user64_step(0); user64_step(1);
    }
    user64_sched_trace_on = 0;
    int separate = procs64[0].cr3 != procs64[1].cr3 &&
                   process_memory_page(&procs64[0].memory, PROCESS_MEMORY_CODE) !=
                       process_memory_page(&procs64[1].memory, PROCESS_MEMORY_CODE) &&
                   process_memory_page(&procs64[0].memory,
                                       PROCESS_MEMORY_KERNEL_STACK_LOW) !=
                       process_memory_page(&procs64[1].memory,
                                           PROCESS_MEMORY_KERNEL_STACK_LOW);
    int interleaved = user64_sched_trace_n == 4 &&
                      user64_sched_trace[0] == 'A' && user64_sched_trace[1] == 'B' &&
                      user64_sched_trace[2] == '1' && user64_sched_trace[3] == '2';
    int kstack_syscall_paths = proc_kstack_last_used[0] > 0 &&
                               proc_kstack_last_used[0] < U64_KSTACK_BYTES &&
                               proc_kstack_last_used[1] > 0 &&
                               proc_kstack_last_used[1] < U64_KSTACK_BYTES;
    if (p1ok && p2ok && separate && interleaved &&
        procs64[0].state == 2 && procs64[1].state == 2)
        up(" <- two PML4 processes yielded/resumed AB12 and exited independently\n");
    else
        up(" <- multi-process yield/resume FAILED\n");

    /* Neither image yields or exits. The PIT must interrupt its busy loop,
     * save the complete user frame, return to this scheduler, and let the
     * sibling enter under another CR3/TSS stack. */
    static const u8_64 busy1[] = {
        0xb8,1,0,0,0, 0xbb,'P',0,0,0, 0xcd,0x80, 0xeb,0xfe
    };
    static const u8_64 busy2[] = {
        0xb8,1,0,0,0, 0xbb,'Q',0,0,0, 0xcd,0x80, 0xeb,0xfe
    };
    int b1 = user64_load_process(0, 1, busy1, sizeof busy1);
    int b2 = user64_load_process(1, 2, busy2, sizeof busy2);
    user64_sched_trace_n = 0; user64_sched_trace_on = 1;
    user64_preemptions = 0; user64_preempt_on = 1;
    if (b1 && b2) { user64_step(0); user64_step(1); }
    user64_preempt_on = 0; user64_sched_trace_on = 0;
    int preempt_trace = user64_sched_trace_n == 2 &&
                        user64_sched_trace[0] == 'P' && user64_sched_trace[1] == 'Q';
    int kstack_preempt_paths = proc_kstack_last_used[0] > 0 &&
                               proc_kstack_last_used[0] < U64_KSTACK_BYTES &&
                               proc_kstack_last_used[1] > 0 &&
                               proc_kstack_last_used[1] < U64_KSTACK_BYTES;
    if (b1 && b2 && preempt_trace && user64_preemptions >= 2 &&
        procs64[0].has_frame && procs64[1].has_frame)
        up(" <- PIT preempted two non-yielding Ring-3 loops PQ\n");
    else
        up(" <- timer process preemption FAILED\n");
    procs64[0].state = procs64[1].state = 2;

    static const u8_64 sibling_fault[] = { 0xfa, 0x0f, 0x0b };
    static const u8_64 sibling_alive[] = {
        0xb8,1,0,0,0, 0xbb,'K',0,0,0, 0xcd,0x80,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b
    };
    int f_ok = user64_load_process(0, 3, sibling_fault, sizeof sibling_fault);
    int s_ok = user64_load_process(1, 4, sibling_alive, sizeof sibling_alive);
    user64_sched_trace_n = 0; user64_sched_trace_on = 1;
    if (f_ok && s_ok) { user64_step(0); user64_step(1); }
    user64_sched_trace_on = 0;
    int kstack_fault_paths = proc_kstack_last_used[0] > 0 &&
                             proc_kstack_last_used[0] < U64_KSTACK_BYTES &&
                             proc_kstack_last_used[1] > 0 &&
                             proc_kstack_last_used[1] < U64_KSTACK_BYTES;
    if (f_ok && s_ok && procs64[0].state == 3 &&
        procs64[0].fault_vector == 13 && procs64[1].state == 2 &&
        user64_sched_trace_n == 1 && user64_sched_trace[0] == 'K')
        up(" <- one process GP-faulted; its sibling ran and exited\n");
    else
        up(" <- sibling fault isolation FAILED\n");

    /* Write into the intentionally absent page directly below the user stack.
     * A genuine guard hit is a non-present user write (#PF error 0x6), with CR2
     * equal to that exact address. The sibling must still run and exit. */
    u8_64 guard_fault[] = {
        0x48,0xb8, 0,0,0,0,0,0,0,0, 0xc6,0x00,0x01, 0x0f,0x0b
    };
    static const u8_64 guard_sibling[] = {
        0xb8,1,0,0,0, 0xbb,'G',0,0,0, 0xcd,0x80,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b
    };
    int gf_ok = user64_load_process(0, 5, guard_fault, sizeof guard_fault);
    int gs_ok = user64_load_process(1, 6, guard_sibling, sizeof guard_sibling);
    u64 guard_address = gf_ok ? procs64[0].user_base + 4096u + 2048u : 0;
    u8_64 *guard_code = (u8_64 *)process64_page_pointer(0, PROCESS_MEMORY_CODE);
    if (gf_ok && guard_code)
        for (int i = 0; i < 8; i++)
            guard_code[2 + i] = (u8_64)(guard_address >> (i * 8));
    user64_sched_trace_n = 0; user64_sched_trace_on = 1;
    if (gf_ok && gs_ok) { user64_step(0); user64_step(1); }
    user64_sched_trace_on = 0;
    if (gf_ok && gs_ok && procs64[0].state == 3 &&
        procs64[0].fault_vector == 14 && procs64[0].fault_error == 0x6u &&
        procs64[0].fault_address == guard_address && procs64[1].state == 2 &&
        user64_sched_trace_n == 1 && user64_sched_trace[0] == 'G')
        up(" <- lower stack guard PF error 6 at exact address; sibling G exited\n");
    else
        up(" <- lower stack guard fault containment FAILED\n");

    int kstack_guard_fault_paths = proc_kstack_last_used[0] > 0 &&
                                   proc_kstack_last_used[0] < U64_KSTACK_BYTES &&
                                   proc_kstack_last_used[1] > 0 &&
                                   proc_kstack_last_used[1] < U64_KSTACK_BYTES;
    process64_select(0);
    int tss0 = gdt64_active_kernel_stack_top() == procs64[0].kernel_stack_top;
    process64_select(1);
    int tss1 = gdt64_active_kernel_stack_top() == procs64[1].kernel_stack_top;
    if (kstack_syscall_paths && kstack_preempt_paths && kstack_fault_paths &&
        kstack_guard_fault_paths && process64_kernel_stack_contract(0) &&
        process64_kernel_stack_contract(1) && tss0 && tss1 &&
        process_memory_page(&procs64[0].memory,
                            PROCESS_MEMORY_KERNEL_STACK_LOW) !=
            process_memory_page(&procs64[1].memory,
                                PROCESS_MEMORY_KERNEL_STACK_LOW) &&
        proc_kstack_high_water[0] > 0 &&
        proc_kstack_high_water[0] < U64_KSTACK_BYTES &&
        proc_kstack_high_water[1] > 0 &&
        proc_kstack_high_water[1] < U64_KSTACK_BYTES) {
        up("  kernel stacks high-water: P0 "); upu(proc_kstack_high_water[0]);
        up(" P1 "); upu(proc_kstack_high_water[1]); up(" bytes\n");
        up(" <- two guarded supervisor TSS stacks bounded through syscall/preempt/fault paths\n");
    } else {
        up(" <- guarded supervisor TSS stack proof FAILED\n");
    }

    /* PID 1 sends "hi", yields, PID 2 receives it and reports sender 1, then
     * replies "ok". PID 1 resumes, receives it and reports sender 2. Buffers
     * live in separate user mappings; the kernel queue is the only bridge. */
    u8_64 ipc1[] = {
        0xb8,15,0,0,0, 0xbb,2,0,0,0, 0x48,0xb9, 0,0,0,0,0,0,0,0,
        0xba,2,0,0,0, 0xcd,0x80, 0xb8,6,0,0,0, 0xcd,0x80,
        0x48,0xbb, 0,0,0,0,0,0,0,0, 0xb9,8,0,0,0,
        0xb8,16,0,0,0, 0xcd,0x80, 0x0f,0xb6,0x1b,
        0xb8,1,0,0,0, 0xcd,0x80, 0xb8,17,0,0,0, 0xcd,0x80,
        0x83,0xc0,'0', 0x89,0xc3, 0xb8,1,0,0,0, 0xcd,0x80,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b, 'h','i'
    };
    u8_64 ipc2[] = {
        0x48,0xbb, 0,0,0,0,0,0,0,0, 0xb9,8,0,0,0,
        0xb8,16,0,0,0, 0xcd,0x80, 0x0f,0xb6,0x1b,
        0xb8,1,0,0,0, 0xcd,0x80, 0xb8,17,0,0,0, 0xcd,0x80,
        0x83,0xc0,'0', 0x89,0xc3, 0xb8,1,0,0,0, 0xcd,0x80,
        0xb8,15,0,0,0, 0xbb,1,0,0,0, 0x48,0xb9, 0,0,0,0,0,0,0,0,
        0xba,2,0,0,0, 0xcd,0x80, 0xb8,3,0,0,0, 0xcd,0x80,
        0x0f,0x0b, 'o','k'
    };
    int i1 = user64_load_process(0, 1, ipc1, sizeof ipc1);
    int i2 = user64_load_process(1, 2, ipc2, sizeof ipc2);
    if (i1 && i2) {
        u8_64 *code0 = (u8_64 *)process64_page_pointer(0, PROCESS_MEMORY_CODE);
        u8_64 *code1 = (u8_64 *)process64_page_pointer(1, PROCESS_MEMORY_CODE);
        u64 p1data = procs64[0].user_base + 94u;
        u64 p1stack = procs64[0].user_stack_top - 8u;
        u64 p2stack = procs64[1].user_stack_top - 8u;
        u64 p2data = procs64[1].user_base + 87u;
        if (code0 && code1)
            for (int i = 0; i < 8; i++) {
                code0[12 + i] = (u8_64)(p1data >> (i * 8));
                code0[36 + i] = (u8_64)(p1stack >> (i * 8));
                code1[2 + i] = (u8_64)(p2stack >> (i * 8));
                code1[63 + i] = (u8_64)(p2data >> (i * 8));
            }
    }
    user64_sched_trace_n = 0; user64_sched_trace_on = 1;
    if (i1 && i2) { user64_step(0); user64_step(1); user64_step(0); }
    user64_sched_trace_on = 0;
    int ipc_trace = user64_sched_trace_n == 4 &&
                    user64_sched_trace[0] == 'h' && user64_sched_trace[1] == '1' &&
                    user64_sched_trace[2] == 'o' && user64_sched_trace[3] == '2';
    if (i1 && i2 && ipc_trace && procs64[0].state == 2 &&
        procs64[1].state == 2 && !procs64[0].inbox_count &&
        !procs64[1].inbox_count)
        up(" <- bounded IPC crossed PML4s hi/ok with sender IDs h1o2\n");
    else
        up(" <- bounded IPC FAILED\n");

    /* Open a real WM-owned client, publish bounded text, yield to the kernel,
     * receive one queued input record into the guarded user stack, print its
     * key code, then close. Marker immediates are patched to this process's
     * virtual code/stack addresses after its PML4 exists. */
    u8_64 winprog[] = {
        0xb8,18,0,0,0, 0x48,0xbb, 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
        0xb9,11,0,0,0, 0xba,0,0,0,0, 0xcd,0x80, 0x41,0x89,0xc4,
        0xb8,19,0,0,0, 0x44,0x89,0xe3,
        0x48,0xb9, 0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,
        0xba,12,0,0,0, 0xcd,0x80, 0xb8,6,0,0,0, 0xcd,0x80,
        0xb8,20,0,0,0, 0x44,0x89,0xe3,
        0x48,0xb9, 0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
        0xba,16,0,0,0, 0xcd,0x80, 0x8b,0x59,0x04,
        0xb8,1,0,0,0, 0xcd,0x80, 0xb8,21,0,0,0, 0x44,0x89,0xe3, 0xcd,0x80,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b,
        'U','s','e','r',' ','w','i','n','d','o','w',
        'R','i','n','g','3',' ','w','i','n','d','o','w'
    };
    int wok = user64_load_process(0, 1, winprog, sizeof winprog);
    if (wok) {
        u8_64 *window_code = (u8_64 *)process64_page_pointer(
            0, PROCESS_MEMORY_CODE);
        u64 values[3] = {
            procs64[0].user_base + sizeof winprog - 23u,
            procs64[0].user_base + sizeof winprog - 12u,
            procs64[0].user_stack_top - 16u
        };
        for (int marker = 1; window_code && marker <= 3; marker++) {
            for (u32 at = 0; at + 8 <= sizeof winprog; at++) {
                int match = 1;
                for (int j = 0; j < 8; j++)
                    if (window_code[at + (u32)j] !=
                        (u8_64)(marker * 0x11)) match = 0;
                if (!match) continue;
                for (int j = 0; j < 8; j++)
                    window_code[at + (u32)j] =
                        (u8_64)(values[marker - 1] >> (j * 8));
                break;
            }
        }
    }
    user64_sched_trace_n = 0; user64_sched_trace_on = 1;
    if (wok) user64_step(0);
    int shown = userwin_count() == 1 && userwin_has_wm_window(1, 1) &&
                userwin_text_byte(1, 1, 0) == 'R' &&
                userwin_text_byte(1, 1, 4) == '3';
    int injected = userwin_test_inject(1, 1, 1, 'W', 23, 47);
    if (wok) user64_step(0);
    user64_sched_trace_on = 0;
    if (wok && shown && injected && procs64[0].state == 2 &&
        user64_sched_trace_n == 1 && user64_sched_trace[0] == 'W' &&
        userwin_count() == 0)
        up(" <- Ring-3 window presented text, polled input W, and closed\n");
    else
        up(" <- Ring-3 window/input ABI FAILED\n");

    /* Hostile-process gate: privileged instruction and supervisor mappings
     * fault only this process; a pointer crossing the stack guard is refused
     * before the kernel dereferences it. */
    static const u8_64 cli_probe[] = { 0xfa, 0x0f, 0x0b };
    u8_64 read_kernel[] = { 0x48,0xa1, 0,0,0,0,0,0,0,0, 0x0f,0x0b };
    u64 ka = (u64)proc64;
    for (int i = 0; i < 8; i++) read_kernel[2 + i] = (u8_64)(ka >> (i * 8));
    u8_64 write_device[] = {
        0x48,0xb8, 0x00,0x80,0x0b,0,0,0,0,0, 0xc6,0x00,0x01, 0x0f,0x0b
    };
    int gp = user64_run_probe(cli_probe, sizeof cli_probe);
    int kr = user64_run_probe(read_kernel, sizeof read_kernel);
    int dw = user64_run_probe(write_device, sizeof write_device);

    /* mov rbx, stack_top-4; mov rcx,8; SYS_COPY; SYS_EXIT */
    if (!process64_prepare(0, 1)) { up("  ring 3 hostile probes: setup refused\n"); return; }
    u8_64 cross[] = {
        0x48,0xbb, 0,0,0,0,0,0,0,0,
        0x48,0xc7,0xc1, 8,0,0,0,
        0xb8,4,0,0,0, 0xcd,0x80,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b
    };
    u64 cp = proc64->user_stack_top - 4;
    for (int i = 0; i < 8; i++) cross[2 + i] = (u8_64)(cp >> (i * 8));
    int cr = user64_run_probe(cross, sizeof cross);
    if (gp == 13 && kr == 14 && dw == 14 && cr == 0 &&
        proc64->bad_pointer_refused == 1)
        up("  ring 3 hostile: cli GP, kernel/device PF, crossing pointer refused; kernel alive\n");
    else {
        up("  ring 3 hostile: FAILED vectors "); upu(gp); up("/"); upu(kr);
        up("/"); upu(dw); up("/"); upu(cr); up(" ptr ");
        upu(proc64->bad_pointer_refused); up("\n");
    }

    int released = 1;
    for (int i = 0; i < U64_PROCS; i++)
        released &= process64_release_slot(i);
    int accounts = released;
    unsigned long anonymous_high_water = 0;
    for (int i = 0; i < U64_PROCS; i++) {
        struct pmm_owner_account fixed = {0};
        struct pmm_owner_account anonymous = {0};
        accounts &= pmm_owner_account(
            PROCESS_MEMORY_OWNER_BASE + (unsigned)i, &fixed) == PMM_OK;
        accounts &= pmm_owner_account(
            ANON_MEMORY_OWNER_BASE + (unsigned)i, &anonymous) == PMM_OK;
        accounts &= fixed.used_pages == 0 &&
                    fixed.high_water_pages >= PROCESS_MEMORY_PAGE_COUNT &&
                    fixed.high_water_pages <= U64_PROCESS_FRAME_LIMIT &&
                    fixed.limit_pages == U64_PROCESS_FRAME_LIMIT &&
                    fixed.available_pages == U64_PROCESS_FRAME_LIMIT &&
                    fixed.refusals == 0;
        accounts &= anonymous.used_pages == 0 &&
                    anonymous.high_water_pages <= U64_ANON_FRAME_LIMIT &&
                    anonymous.limit_pages == U64_ANON_FRAME_LIMIT &&
                    anonymous.available_pages == U64_ANON_FRAME_LIMIT &&
                    anonymous.refusals == 0;
        anonymous_high_water += anonymous.high_water_pages;
    }
    accounts &= anonymous_high_water >= 2;
    if (accounts && pmm_used_pages() == process_frame_baseline && !pmm_check()) {
        up(" <- process memory accounting: fixed/anonymous quotas and owner totals passed\n");
        up(" <- process-owned page tables/code/stacks reclaimed; PMM baseline restored\n");
    } else {
        up(" <- process memory accounting FAILED\n");
        up(" <- process frame reclamation FAILED\n");
    }
}

__asm__(
    ".section .rodata\n"
    ".globl user64_blob\n"
    "user64_blob:\n"
    "  .byte 0xb8,1,0,0,0, 0xbb,'u',0,0,0, 0xcd,0x80\n"
    "  .byte 0xb8,2,0,0,0, 0xcd,0x80, 0x83,0xc0,'0', 0x89,0xc3\n"
    "  .byte 0xb8,1,0,0,0, 0xcd,0x80\n"
    "  .byte 0xb8,5,0,0,0, 0xcd,0x80\n"
    "  .byte 0xb8,6,0,0,0, 0xcd,0x80\n"
    "  .byte 0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b\n"
    ".globl user64_blob_end\n"
    "user64_blob_end:\n"
    ".globl user64_anon_probe\n"
    "user64_anon_probe:\n"
    "  xor %r13d,%r13d\n"
    "  mov $22,%eax\n  xor %ebx,%ebx\n  mov $2,%ecx\n  int $0x80\n"
    "  mov %rax,%r12\n"
    "  mov $23,%eax\n  xor %ebx,%ebx\n  mov $2,%ecx\n  int $0x80\n"
    "  or %rax,%r13\n"
    "  mov (%r12),%rbx\n  or 4096(%r12),%rbx\n  or %rbx,%r13\n"
    "  movb $77,(%r12)\n  movb $78,4096(%r12)\n"
    "  movzbl (%r12),%ebx\n  xor $77,%ebx\n  or %rbx,%r13\n"
    "  movzbl 4096(%r12),%ebx\n  xor $78,%ebx\n  or %rbx,%r13\n"
    "  mov $4,%eax\n  lea 4094(%r12),%rbx\n  mov $4,%ecx\n  int $0x80\n"
    "  or %rax,%r13\n"
    "  mov $24,%eax\n  xor %ebx,%ebx\n  mov $1,%ecx\n  int $0x80\n"
    "  or %rax,%r13\n"
    "  mov $4,%eax\n  mov %r12,%rbx\n  mov $1,%ecx\n  int $0x80\n"
    "  inc %rax\n  or %rax,%r13\n"
    "  mov $24,%eax\n  mov $1,%ebx\n  mov $1,%ecx\n  int $0x80\n"
    "  or %rax,%r13\n  test %r13,%r13\n  jz .Lanon_success\n  ud2\n"
    ".Lanon_success:\n"
    "  mov $1,%eax\n  mov $77,%ebx\n  int $0x80\n"
    "  mov $3,%eax\n  int $0x80\n  ud2\n"
    ".globl user64_anon_probe_end\n"
    "user64_anon_probe_end:\n"
    ".globl user64_anon_reserved_fault\n"
    "user64_anon_reserved_fault:\n"
    "  mov $22,%eax\n  xor %ebx,%ebx\n  mov $1,%ecx\n  int $0x80\n"
    "  mov %rax,%r12\n  mov (%r12),%rax\n  ud2\n"
    ".globl user64_anon_reserved_fault_end\n"
    "user64_anon_reserved_fault_end:\n"
    ".globl user64_anon_released_fault\n"
    "user64_anon_released_fault:\n"
    "  mov $22,%eax\n  xor %ebx,%ebx\n  mov $1,%ecx\n  int $0x80\n"
    "  mov %rax,%r12\n"
    "  mov $23,%eax\n  xor %ebx,%ebx\n  mov $1,%ecx\n  int $0x80\n"
    "  mov $24,%eax\n  xor %ebx,%ebx\n  mov $1,%ecx\n  int $0x80\n"
    "  mov (%r12),%rax\n  ud2\n"
    ".globl user64_anon_released_fault_end\n"
    "user64_anon_released_fault_end:\n"
    ".text\n"
    ".globl user64_enter_asm\n"
    "user64_enter_asm:\n"
    "  push %rbx\n  push %rbp\n  push %r12\n  push %r13\n  push %r14\n  push %r15\n"
    "  mov %rsp,user64_return_rsp(%rip)\n"
    "  lea 9f(%rip),%rax\n  mov %rax,user64_return_rip(%rip)\n"
    "  pushfq\n  pop %rax\n  mov %rax,user64_return_rflags(%rip)\n"
    "  mov user64_process_cr3(%rip),%rax\n  mov %rax,%cr3\n"
    "  pushq $0x1b\n  push %rsi\n  pushfq\n  orq $0x200,(%rsp)\n"
    "  pushq $0x23\n  push %rdi\n  iretq\n"
    "9:\n"
    "  push user64_return_rflags(%rip)\n  popfq\n"
    "  pop %r15\n  pop %r14\n  pop %r13\n  pop %r12\n  pop %rbp\n  pop %rbx\n  ret\n"
    ".globl user64_resume_asm\n"
    "user64_resume_asm:\n"
    "  push %rbx\n  push %rbp\n  push %r12\n  push %r13\n  push %r14\n  push %r15\n"
    "  mov %rsp,user64_return_rsp(%rip)\n"
    "  lea 7f(%rip),%rax\n  mov %rax,user64_return_rip(%rip)\n"
    "  pushfq\n  pop %rax\n  mov %rax,user64_return_rflags(%rip)\n"
    "  mov user64_process_cr3(%rip),%rax\n  mov %rax,%cr3\n"
    "  mov %rdi,%rsp\n  jmp 6f\n"
    "7:\n"
    "  push user64_return_rflags(%rip)\n  popfq\n"
    "  pop %r15\n  pop %r14\n  pop %r13\n  pop %r12\n  pop %rbp\n  pop %rbx\n  ret\n"
    ".globl syscall_isr\n"
    "syscall_isr:\n"
    "  push %rax\n  push %rbx\n  push %rcx\n  push %rdx\n  push %rsi\n"
    "  push %rdi\n  push %r8\n  push %r9\n  push %r10\n  push %r11\n"
    "  push %r12\n  push %r13\n  push %r14\n  push %r15\n  push %rbp\n"
    "  mov 112(%rsp),%rdi\n  mov 104(%rsp),%rsi\n"
    "  mov 96(%rsp),%rdx\n  mov 88(%rsp),%rcx\n  call user64_dispatch\n"
    "  mov %rax,112(%rsp)\n  mov %rsp,%rdi\n  call user64_after_syscall\n"
    "  test %eax,%eax\n  jnz 8f\n"
    "6:\n"
    "  pop %rbp\n  pop %r15\n  pop %r14\n  pop %r13\n  pop %r12\n"
    "  pop %r11\n  pop %r10\n  pop %r9\n  pop %r8\n  pop %rdi\n"
    "  pop %rsi\n  pop %rdx\n  pop %rcx\n  pop %rbx\n  pop %rax\n  iretq\n"
    "8:\n"
    ".globl user64_abort\n"
    "user64_abort:\n"
    "  mov user64_return_rsp(%rip),%rsp\n"
    "  mov user64_kernel_cr3(%rip),%rax\n  mov %rax,%cr3\n"
    "  jmp *user64_return_rip(%rip)\n"
    ".globl user64_timer_isr\n"
    "user64_timer_isr:\n"
    "  push %rax\n  push %rbx\n  push %rcx\n  push %rdx\n  push %rsi\n"
    "  push %rdi\n  push %r8\n  push %r9\n  push %r10\n  push %r11\n"
    "  push %r12\n  push %r13\n  push %r14\n  push %r15\n  push %rbp\n"
    "  mov %rsp,%rdi\n  call user64_timer_dispatch\n"
    "  test %eax,%eax\n  jnz 8b\n"
    "  pop %rbp\n  pop %r15\n  pop %r14\n  pop %r13\n  pop %r12\n"
    "  pop %r11\n  pop %r10\n  pop %r9\n  pop %r8\n  pop %rdi\n"
    "  pop %rsi\n  pop %rdx\n  pop %rcx\n  pop %rbx\n  pop %rax\n  iretq\n"
);

#endif /* ZL_64 */
