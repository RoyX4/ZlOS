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
 * docs/memory-model.md, Stage 5, is the account of what is left.
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
        zlt_event(ZLLOG_SUB_SYSCALL, ZLLOG_EV_SYSCALL_EXIT, ZLLOG_INFO,
                  nr, 0, user_calls);
        return 0;

    case SYS_GETPID:
        zlt_event(ZLLOG_SUB_SYSCALL, ZLLOG_EV_SYSCALL_EXIT, ZLLOG_INFO,
                  nr, 1, user_calls);
        return 1;                       /* one process, and it is honest */

    case SYS_EXIT:
        user_exited = 1;
        zlt_event(ZLLOG_SUB_SYSCALL, ZLLOG_EV_SYSCALL_EXIT, ZLLOG_INFO,
                  nr, 0, user_calls);
        return 0;

    default:
        /* An unknown call is NOT a fault and NOT silent. Silence here is how a
         * program that thinks it wrote to a file gets no error and no file. */
        up("  syscall: ring 3 asked for unknown call ");
        upu(nr);
        up("\n");
        zlt_event(ZLLOG_SUB_SYSCALL, ZLLOG_EV_SYSCALL_EXIT, ZLLOG_WARN,
                  nr, (u32)-1, user_calls);
        return (u32)-1;
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
 * docs/exec-kill-path.md) - deliberately, because a second mechanism for
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
typedef unsigned char u8_64;

#define U64_P  1ULL
#define U64_W  2ULL
#define U64_U  4ULL
#define U64_NX (1ULL << 63)
#define U64_ADDR 0x000ffffffffff000ULL
#define U64_CODE_SEL 0x23
#define U64_DATA_SEL 0x1b
#define U64_SYS_WRITE  1
#define U64_SYS_GETPID 2
#define U64_SYS_EXIT   3
#define U64_SYS_COPY   4
#define U64_SYS_TIME   5
#define U64_SYS_YIELD  6
#define U64_SYS_OPEN   7
#define U64_SYS_READ   8
#define U64_SYS_WRITEF 9
#define U64_SYS_CLOSE  10
#define U64_SYS_INFO   11
#define U64_SYS_REMOVE 12
#define U64_SYS_RENAME 13
#define U64_SYS_SYNC   14

#define U64_HANDLES 8
#define U64_IO_MAX 4096
#define U64_NAME_MAX 24
#define U64_EINVAL ((u64)-22)
#define U64_ENOENT ((u64)-2)
#define U64_ENOSPC ((u64)-28)
#define U64_EBADF  ((u64)-9)
#define U64_EIO    ((u64)-5)

struct process64 {
    u32 pid;
    u32 state;                 /* 0 empty, 1 runnable/running, 2 exited, 3 fault */
    u64 cr3, user_base, user_stack_top;
    u32 calls, fault_vector;
    u32 bad_pointer_refused;
    int handles[U64_HANDLES];       /* zlfs index + 1; zero means closed */
};

static struct process64 proc64;
static u64 proc_pml4[512] __attribute__((aligned(4096)));
static u64 proc_pdpt[512] __attribute__((aligned(4096)));
static u64 proc_pd[512]   __attribute__((aligned(4096)));
static u64 proc_pt[512]   __attribute__((aligned(4096)));
static u8_64 proc_code[4096]  __attribute__((aligned(4096)));
static u8_64 proc_stack[4096] __attribute__((aligned(4096)));
static u8_64 proc_io[U64_IO_MAX];

u64 user64_kernel_cr3, user64_process_cr3;
u64 user64_return_rsp, user64_return_rip, user64_return_rflags;
static volatile int user64_exited, user64_running, user64_faulted;

extern unsigned char user64_blob[], user64_blob_end[];
void __attribute__((sysv_abi)) user64_enter_asm(u64 rip, u64 rsp);
void user64_abort(void) __attribute__((noreturn));
extern u32 idt_ticks(void);
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

static u64 cr3_read64(void)
{
    u64 v; __asm__ volatile("mov %%cr3,%0" : "=r"(v)); return v;
}

static void zero_page(u64 *p) { for (int i = 0; i < 512; i++) p[i] = 0; }

static int process64_prepare(void)
{
    u64 old = cr3_read64();
    u64 *live = (u64 *)(old & U64_ADDR);
    if (!live) return 0;
    int slot = -1;
    for (int i = 1; i < 255; i++) if (!(live[i] & U64_P)) { slot = i; break; }
    if (slot < 0) return 0;
    for (int i = 0; i < 512; i++) proc_pml4[i] = live[i];
    zero_page(proc_pdpt); zero_page(proc_pd); zero_page(proc_pt);
    u64 base = (u64)(unsigned)slot << 39;
    proc_pml4[slot] = ((u64)proc_pdpt & U64_ADDR) | U64_P | U64_W | U64_U;
    proc_pdpt[0] = ((u64)proc_pd & U64_ADDR) | U64_P | U64_W | U64_U;
    proc_pd[0] = ((u64)proc_pt & U64_ADDR) | U64_P | U64_W | U64_U;
    proc_pt[0] = ((u64)proc_code & U64_ADDR) | U64_P | U64_U;
    /* PTE 1 is intentionally absent: the stack's lower guard page. */
    proc_pt[2] = ((u64)proc_stack & U64_ADDR) | U64_P | U64_W | U64_U | U64_NX;

    u64 bytes = (u64)(user64_blob_end - user64_blob);
    if (!bytes || bytes > sizeof proc_code) return 0;
    for (u64 i = 0; i < bytes; i++) proc_code[i] = user64_blob[i];
    for (u64 i = bytes; i < sizeof proc_code; i++) proc_code[i] = 0xcc;
    for (int i = 0; i < (int)sizeof proc_stack; i++) proc_stack[i] = 0;

    proc64.pid = 1; proc64.state = 1;
    proc64.cr3 = (u64)proc_pml4 & U64_ADDR;
    proc64.user_base = base;
    proc64.user_stack_top = base + 3 * 4096ULL;
    proc64.calls = 0; proc64.fault_vector = 0; proc64.bad_pointer_refused = 0;
    for (int i = 0; i < U64_HANDLES; i++) proc64.handles[i] = 0;
    user64_kernel_cr3 = old;
    user64_process_cr3 = proc64.cr3;
    return 1;
}

/* Complete-range validation happens before the first byte is touched. */
static int user64_range(u64 addr, u64 bytes, int writing)
{
    if (!bytes || addr + bytes < addr) return 0;
    u64 b = proc64.user_base;
    if (!writing && addr >= b && addr + bytes <= b + 4096) return 1;
    if (addr >= b + 8192 && addr + bytes <= b + 12288) return 1;
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
    if (h <= 0 || h > U64_HANDLES || !proc64.handles[h - 1]) return -1;
    return proc64.handles[h - 1] - 1;
}

static int user64_open_handle(int idx)
{
    for (int i = 0; i < U64_HANDLES; i++) if (!proc64.handles[i]) {
        proc64.handles[i] = idx + 1;
        return i + 1;
    }
    return -1;
}

u64 __attribute__((sysv_abi)) user64_dispatch(u64 nr, u64 arg1,
                                              u64 arg2, u64 arg3)
{
    proc64.calls++;
    zlt_count(ZLLOG_C_SYSCALL, 1);
    zlt_event(ZLLOG_SUB_SYSCALL, ZLLOG_EV_SYSCALL_ENTER, ZLLOG_INFO,
              (u32)nr, (u32)arg1, proc64.calls);
    if (nr == U64_SYS_WRITE) { zl_putc_pub((char)(arg1 & 0x7f)); return 0; }
    if (nr == U64_SYS_GETPID) return proc64.pid;
    if (nr == U64_SYS_EXIT) {
        user64_exited = 1; proc64.state = 2;
        return 0;
    }
    if (nr == U64_SYS_COPY) {
        if (!user64_range(arg1, arg2, 0)) {
            proc64.bad_pointer_refused++;
            return (u64)-1;
        }
        return 0;
    }
    if (nr == U64_SYS_TIME) return idt_ticks();
    if (nr == U64_SYS_YIELD) { yield(); return 0; }

    /* The file ABI is intentionally whole-file and bounded. READ copies at
     * most 4 KiB from the beginning; WRITE atomically replaces the whole
     * file through zlfs's data-before-metadata path. This is a stable useful
     * contract without pretending the current flat filesystem is POSIX. */
    if (nr == U64_SYS_OPEN) {
        char name[U64_NAME_MAX];
        if (!fs_mounted() || !user64_name(name, arg1, arg2)) return U64_EINVAL;
        int idx = fs_find(name);
        if (idx < 0 && (arg3 & 1ULL)) idx = fs_create(name, 0);
        if (idx < 0) return U64_ENOENT;
        int h = user64_open_handle(idx);
        return h < 0 ? U64_ENOSPC : (u64)h;
    }
    if (nr == U64_SYS_READ) {
        int idx = user64_handle((int)arg1);
        if (idx < 0) return U64_EBADF;
        if (!arg3 || arg3 > U64_IO_MAX || fs_size(idx) > arg3) return U64_EINVAL;
        int n = fs_read(idx, proc_io, (u32)arg3);
        if (n < 0 || !copy_to_user(arg2, proc_io, (u32)n)) return U64_EIO;
        return (u64)n;
    }
    if (nr == U64_SYS_WRITEF) {
        int idx = user64_handle((int)arg1);
        if (idx < 0) return U64_EBADF;
        if (arg3 > U64_IO_MAX || (arg3 && !copy_from_user(proc_io, arg2, (u32)arg3)))
            return U64_EINVAL;
        return fs_write(idx, proc_io, (u32)arg3) ? arg3 : U64_EIO;
    }
    if (nr == U64_SYS_CLOSE) {
        if (arg1 == 0 || arg1 > U64_HANDLES || !proc64.handles[arg1 - 1])
            return U64_EBADF;
        proc64.handles[arg1 - 1] = 0;
        return 0;
    }
    if (nr == U64_SYS_INFO) {
        if (arg1 >= (u64)fs_maxfiles() || !fs_used((int)arg1) ||
            !arg2 || !arg3 || arg3 > U64_NAME_MAX) return U64_ENOENT;
        u32 n = 0;
        while (n + 1 < (u32)arg3) {
            int c = fs_name_byte((int)arg1, (int)n);
            proc_io[n++] = (u8_64)c;
            if (!c) break;
        }
        if (!n || proc_io[n - 1]) proc_io[n++] = 0;
        if (!copy_to_user(arg2, proc_io, n)) return U64_EINVAL;
        return fs_size((int)arg1);
    }
    if (nr == U64_SYS_REMOVE) {
        char name[U64_NAME_MAX];
        if (!user64_name(name, arg1, arg2)) return U64_EINVAL;
        int idx = fs_find(name);
        return idx >= 0 && fs_delete(idx) ? 0 : U64_ENOENT;
    }
    if (nr == U64_SYS_RENAME) {
        int idx = user64_handle((int)arg1);
        char name[U64_NAME_MAX];
        if (idx < 0) return U64_EBADF;
        if (!user64_name(name, arg2, arg3)) return U64_EINVAL;
        return fs_rename(idx, name) ? 0 : U64_EIO;
    }
    if (nr == U64_SYS_SYNC) return fs_sync() ? 0 : U64_EIO;
    return (u64)-1;
}

int user64_is_running(void) { return user64_running; }
void user64_mark_fault(u32 vector)
{
    user64_faulted = 1; user64_running = 0;
    proc64.state = 3; proc64.fault_vector = vector;
}

int user_has_exited(void) { return user64_exited; }
u32 user_call_count(void) { return proc64.calls; }
int user64_faulted_out(void) { return user64_faulted; }

static int user64_run_probe(const u8_64 *code, u32 bytes)
{
    if (!process64_prepare() || !code || !bytes || bytes > sizeof proc_code) return -1;
    for (u32 i = 0; i < bytes; i++) proc_code[i] = code[i];
    for (u32 i = bytes; i < sizeof proc_code; i++) proc_code[i] = 0xcc;
    user64_exited = user64_faulted = 0;
    user64_running = 1;
    user64_enter_asm(proc64.user_base, proc64.user_stack_top);
    user64_running = 0;
    return user64_faulted ? (int)proc64.fault_vector : 0;
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
    if (!n || n > sizeof proc_io || fs_read(idx, proc_io, n) != (int)n) return -3;
    return user64_run_probe(proc_io, n);
}

void user_selftest(void)
{
    up("  ring 3 64: ");
    if (!process64_prepare()) { up("page-table setup refused\n"); return; }
    user64_exited = user64_faulted = 0;
    user64_running = 1;
    user64_enter_asm(proc64.user_base, proc64.user_stack_top);
    user64_running = 0;
    if (user64_faulted) {
        up("process faulted alone, vector "); upu(proc64.fault_vector); up("\n");
        return;
    }
    up(" <- iretq/int80/iretq, "); upu(proc64.calls);
    up(" syscalls, process exited, kernel alive\n");

    /* Hostile-process gate: privileged instruction and supervisor mappings
     * fault only this process; a pointer crossing the stack guard is refused
     * before the kernel dereferences it. */
    static const u8_64 cli_probe[] = { 0xfa, 0x0f, 0x0b };
    u8_64 read_kernel[] = { 0x48,0xa1, 0,0,0,0,0,0,0,0, 0x0f,0x0b };
    u64 ka = (u64)&proc64;
    for (int i = 0; i < 8; i++) read_kernel[2 + i] = (u8_64)(ka >> (i * 8));
    u8_64 write_device[] = {
        0x48,0xb8, 0x00,0x80,0x0b,0,0,0,0,0, 0xc6,0x00,0x01, 0x0f,0x0b
    };
    int gp = user64_run_probe(cli_probe, sizeof cli_probe);
    int kr = user64_run_probe(read_kernel, sizeof read_kernel);
    int dw = user64_run_probe(write_device, sizeof write_device);

    /* mov rbx, stack_top-4; mov rcx,8; SYS_COPY; SYS_EXIT */
    if (!process64_prepare()) { up("  ring 3 hostile probes: setup refused\n"); return; }
    u8_64 cross[] = {
        0x48,0xbb, 0,0,0,0,0,0,0,0,
        0x48,0xc7,0xc1, 8,0,0,0,
        0xb8,4,0,0,0, 0xcd,0x80,
        0xb8,3,0,0,0, 0xcd,0x80, 0x0f,0x0b
    };
    u64 cp = proc64.user_stack_top - 4;
    for (int i = 0; i < 8; i++) cross[2 + i] = (u8_64)(cp >> (i * 8));
    int cr = user64_run_probe(cross, sizeof cross);
    if (gp == 13 && kr == 14 && dw == 14 && cr == 0 &&
        proc64.bad_pointer_refused == 1)
        up("  ring 3 hostile: cli GP, kernel/device PF, crossing pointer refused; kernel alive\n");
    else {
        up("  ring 3 hostile: FAILED vectors "); upu(gp); up("/"); upu(kr);
        up("/"); upu(dw); up("/"); upu(cr); up(" ptr ");
        upu(proc64.bad_pointer_refused); up("\n");
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
    ".globl syscall_isr\n"
    "syscall_isr:\n"
    "  push %rax\n  push %rbx\n  push %rcx\n  push %rdx\n  push %rsi\n"
    "  push %rdi\n  push %r8\n  push %r9\n  push %r10\n  push %r11\n"
    "  push %r12\n  push %r13\n  push %r14\n  push %r15\n  push %rbp\n"
    "  mov 112(%rsp),%rdi\n  mov 104(%rsp),%rsi\n"
    "  mov 96(%rsp),%rdx\n  mov 88(%rsp),%rcx\n  call user64_dispatch\n"
    "  mov %rax,112(%rsp)\n  call user_has_exited\n  test %eax,%eax\n  jnz 8f\n"
    "  pop %rbp\n  pop %r15\n  pop %r14\n  pop %r13\n  pop %r12\n"
    "  pop %r11\n  pop %r10\n  pop %r9\n  pop %r8\n  pop %rdi\n"
    "  pop %rsi\n  pop %rdx\n  pop %rcx\n  pop %rbx\n  pop %rax\n  iretq\n"
    "8:\n"
    ".globl user64_abort\n"
    "user64_abort:\n"
    "  mov user64_kernel_cr3(%rip),%rax\n  mov %rax,%cr3\n"
    "  mov user64_return_rsp(%rip),%rsp\n  jmp *user64_return_rip(%rip)\n"
);

#endif /* ZL_64 */
