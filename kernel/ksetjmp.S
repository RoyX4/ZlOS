/* ksetjmp.S - setjmp and longjmp, for a kernel that has no libc.
 *
 * interp.c's error boundary is a longjmp: runtime_error() unwinds to whoever
 * armed the trap instead of calling exit(1), because exit(1) in a kernel is
 * the machine. That is Item 2's second non-negotiable, and it is the one piece
 * of it that cannot be written in C.
 *
 * WHY NOT __builtin_setjmp. GCC has one, it needs no assembly, and its manual
 * is a list of things that break it - the frame must not be optimised away,
 * the containing function has restrictions, and the failure mode is a jump
 * into a frame that no longer exists. Twenty lines of assembly whose contract
 * is written down beats a builtin whose contract is "usually".
 *
 * WHAT HAS TO BE SAVED, and it is less than people expect. The ABI splits
 * registers into caller-saved and callee-saved. Caller-saved registers are
 * already assumed dead across a call, and setjmp IS a call - so anything the
 * compiler cared about is either callee-saved or already spilled. Save the
 * callee-saved set, the stack pointer and the return address; everything else
 * is the compiler's problem and it has already solved it.
 *
 *   i386     ebx esi edi ebp, esp, and the return address
 *   x86-64   rbx rbp r12 r13 r14 r15, rsp, and the return address
 *
 * NOT the x87 or SSE state. idt.c is built -mgeneral-regs-only and the rule in
 * this kernel is that no SSE lives across a boundary like this; a longjmp that
 * restored a stale MXCSR would be a bug that only appears on the 64-bit build,
 * which is the build that boots the real laptop.
 *
 * THE CONTRACT, which is the whole reason this file has a comment this long:
 *
 *   - ksetjmp returns 0 when called, and returns `val` (never 0) when longjmp
 *     jumps back to it. Same as C's.
 *   - the frame that called ksetjmp MUST still be live when klongjmp runs.
 *     Jumping into a returned frame restores a stack pointer into dead space
 *     and the machine does not survive it. interp.c satisfies this by arming
 *     the trap in zl_run_program and disarming it on every exit path.
 *   - locals modified between the two are indeterminate unless volatile. That
 *     is not this file's rule, it is C's, and interp.c's return path reads
 *     only globals because of it.
 */

    .text

#if defined(ZL_EFI)

    /* Win64 ABI: first arg in rcx, second in rdx. rdi/rsi are callee-saved.
     * PE's assembler has no .type/.size. */
    .globl ksetjmp
ksetjmp:
    movq    %rbx,  0(%rcx)
    movq    %rbp,  8(%rcx)
    movq    %rdi, 16(%rcx)
    movq    %rsi, 24(%rcx)
    movq    %r12, 32(%rcx)
    movq    %r13, 40(%rcx)
    movq    %r14, 48(%rcx)
    movq    %r15, 56(%rcx)
    leaq    8(%rsp), %rax
    movq    %rax, 64(%rcx)
    movq    (%rsp), %rax
    movq    %rax, 72(%rcx)
    xorl    %eax, %eax
    ret

    .globl klongjmp
klongjmp:
    movl    %edx, %eax
    testl   %eax, %eax
    jnz     1f
    movl    $1, %eax
1:
    movq    0(%rcx), %rbx
    movq    8(%rcx), %rbp
    movq   16(%rcx), %rdi
    movq   24(%rcx), %rsi
    movq   32(%rcx), %r12
    movq   40(%rcx), %r13
    movq   48(%rcx), %r14
    movq   56(%rcx), %r15
    movq   72(%rcx), %r8
    movq   64(%rcx), %rsp
    jmp    *%r8

#elif defined(__x86_64__)

    .globl ksetjmp
    .type  ksetjmp, @function
ksetjmp:
    /* rdi = jmp_buf. The return address is at (%rsp) because `call` pushed it. */
    movq    %rbx,  0(%rdi)
    movq    %rbp,  8(%rdi)
    movq    %r12, 16(%rdi)
    movq    %r13, 24(%rdi)
    movq    %r14, 32(%rdi)
    movq    %r15, 40(%rdi)
    leaq    8(%rsp), %rax          /* rsp as it will be AFTER we return */
    movq    %rax, 48(%rdi)
    movq    (%rsp), %rax           /* the return address */
    movq    %rax, 56(%rdi)
    xorl    %eax, %eax             /* first time through: 0 */
    ret
    .size ksetjmp, .-ksetjmp

    .globl klongjmp
    .type  klongjmp, @function
klongjmp:
    /* rdi = jmp_buf, esi = value. */
    movl    %esi, %eax
    testl   %eax, %eax
    jnz     1f
    movl    $1, %eax               /* longjmp(buf, 0) must still return 1 */
1:
    movq   0(%rdi), %rbx
    movq   8(%rdi), %rbp
    movq  16(%rdi), %r12
    movq  24(%rdi), %r13
    movq  32(%rdi), %r14
    movq  40(%rdi), %r15
    movq  56(%rdi), %rdx           /* return address, read BEFORE rsp moves */
    movq  48(%rdi), %rsp
    jmp    *%rdx
    .size klongjmp, .-klongjmp

#else   /* i386 */

    .globl ksetjmp
    .type  ksetjmp, @function
ksetjmp:
    movl    4(%esp), %ecx          /* jmp_buf */
    movl    %ebx,  0(%ecx)
    movl    %esi,  4(%ecx)
    movl    %edi,  8(%ecx)
    movl    %ebp, 12(%ecx)
    leal    4(%esp), %eax          /* esp as it will be AFTER we return */
    movl    %eax, 16(%ecx)
    movl    (%esp), %eax           /* the return address */
    movl    %eax, 20(%ecx)
    xorl    %eax, %eax
    ret
    .size ksetjmp, .-ksetjmp

    .globl klongjmp
    .type  klongjmp, @function
klongjmp:
    movl    4(%esp), %ecx          /* jmp_buf */
    movl    8(%esp), %eax          /* value   */
    testl   %eax, %eax
    jnz     1f
    movl    $1, %eax
1:
    movl   0(%ecx), %ebx
    movl   4(%ecx), %esi
    movl   8(%ecx), %edi
    movl  12(%ecx), %ebp
    movl  20(%ecx), %edx           /* return address, BEFORE esp moves */
    movl  16(%ecx), %esp
    jmp    *%edx
    .size klongjmp, .-klongjmp

#endif

/* No executable stack, on ELF. PE has no GNU-stack note; emitting one
 * under the Windows EFI target is a useless (and sometimes fatal) section. */
#if !defined(ZL_EFI)
    .section .note.GNU-stack,"",@progbits
#endif
