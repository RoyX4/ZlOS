/* _start: the entry point when there is no CRT. With -nostdlib nobody
 * sets up argc/argv or calls main for us, so we do the one thing that
 * matters - call main and then never return. In a kernel this is
 * efi_main instead, and that is the only line that changes. */
int main(void);
__attribute__((naked)) void _start(void)
{
    __asm__ volatile("xor %rbp, %rbp\n\t"
                     "call main\n\t"
                     "mov %eax, %edi\n\t"
                     "mov $60, %eax\n\t"
                     "syscall");
}
