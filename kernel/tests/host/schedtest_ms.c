/* schedtest_ms.c - the EFI build's switch_to, under the Microsoft x64 ABI.
 *
 * WHOLE FILE BUILT -mabi=ms, with sched.c compiled -DZL_64 -DZL_EFI -mabi=ms
 * beside it, so the switch_to arm under test is the one the ThinkPad runs.
 *
 * WHY. The Microsoft callee-saved set is bigger than System V's: rsi, rdi
 * and xmm6-xmm15 belong to the callee too. Until 2026-09-04 the ZL_EFI arm
 * of switch_to saved the SysV six only, so the first switch back handed a
 * task the OTHER task's rsi/rdi - and clang keeps the hidden struct-return
 * pointer of zl_builtin in rsi across `call yield`. Measured before the fix:
 *
 *     after one round trip through switch_to (MS ABI): rsi=0xbbbb rdi=0xcccc
 *
 * (set 0x1111/0x2222 before yield; task B held 0xBBBB/0xCCCC).
 *
 * libc is System V, so the two calls into it are declared sysv_abi. */
typedef unsigned long long uptr;
extern int printf(const char *, ...) __attribute__((sysv_abi));
extern void *mmap(void *, unsigned long, int, int, int, long) __attribute__((sysv_abi));

static unsigned ticks;
unsigned idt_ticks(void) { return ticks++; }
int sched_init(void); int task_create(uptr); void yield(void);

/* task B: put known values in rsi/rdi and xmm6/xmm15, then give up the CPU forever */
__asm__(".globl taskb\ntaskb:\n"
        "  movq $0xBBBB, %rsi\n  movq $0xCCCC, %rdi\n"
        "  movq $0xB6, %rax\n  movq %rax, %xmm6\n"
        "  movq $0xB15, %rax\n  movq %rax, %xmm15\n"
        "  subq $40, %rsp\n"
        "1: call yield\n  jmp 1b\n");

/* task 0: set rsi/rdi/xmm6/xmm15, yield once, return what came back:
 * rax = rsi, rdx = rdi, r8 = xmm6 low, r9 = xmm15 low */
__asm__(".globl probe\nprobe:\n"
        "  pushq %rsi\n  pushq %rdi\n"
        "  movq $0x1111, %rsi\n  movq $0x2222, %rdi\n"
        "  movq $0x66, %rax\n  movq %rax, %xmm6\n"
        "  movq $0xF15, %rax\n  movq %rax, %xmm15\n"
        "  subq $40, %rsp\n  call yield\n  addq $40, %rsp\n"
        "  movq %rsi, %rax\n  movq %rdi, %rdx\n"
        "  movq %xmm6, %r8\n  movq %xmm15, %r9\n"
        "  popq %rdi\n  popq %rsi\n  ret\n");
extern void taskb(void);

int main(void)
{
    if (mmap((void *)0x0B000000UL, 0x800000, 3, 0x22 | 0x100000, -1, 0) != (void *)0x0B000000UL) {
        printf("  FAIL mmap HI_SCHED\n"); return 2;
    }
    sched_init();
    task_create((uptr)taskb);
    unsigned long rsi, rdi, x6, x15;
    __asm__ volatile("call probe\n movq %%rax,%0\n movq %%rdx,%1\n movq %%r8,%2\n movq %%r9,%3"
                     : "=r"(rsi), "=r"(rdi), "=r"(x6), "=r"(x15)
                     : : "rax", "rdx", "rcx", "r8", "r9", "r10", "r11", "memory");
    int ok = rsi == 0x1111 && rdi == 0x2222 && x6 == 0x66 && x15 == 0xF15;
    printf("  %s rsi=0x%lx rdi=0x%lx xmm6=0x%lx xmm15=0x%lx after a round trip through switch_to (MS ABI)\n",
           ok ? "ok  " : "FAIL", rsi, rdi, x6, x15);
    printf("\n%s: %d failure(s)\n", ok ? "all good" : "FAILED", ok ? 0 : 1);
    return ok ? 0 : 1;
}
