"""Execute the shipping syscall/timer assembly with a synthetic CPL-3 frame.

The host supplies observation-only dispatchers and never takes the privileged
abort path. This checks the real assembly's FP/DF boundary and iret return; it
does not replace QEMU proof of privilege entry, preemption or address spaces.
"""
import ast
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

SOURCE = Path(__file__).resolve().parents[1] / 'kernel/src/arch/x86/usermode.c'

HARNESS = r'''
#include <stdint.h>
#include <stdio.h>
#include <string.h>
unsigned char user64_fx_kernel[512] __attribute__((aligned(16)));
unsigned char user64_fx_user[512] __attribute__((aligned(16)));
unsigned char host_fx[512] __attribute__((aligned(16)));
uintptr_t user64_return_rsp, user64_kernel_cr3, user64_return_rip;
uint32_t kernel_mxcsr = 0x1f80, user_mxcsr = 0, seen_mxcsr, returned_mxcsr;
uint16_t user_cw = 0x077f, seen_cw, returned_cw;
uint64_t seen_flags, returned_flags;
uint64_t kernel_xmm[2] = {0x1122334455667788, 0x8877665544332211};
uint64_t user_xmm[2] = {0x123456789abcdef0, 0xfedcba9876543210};
uint64_t seen_xmm[2], returned_xmm[2];
extern void invoke_syscall(void), invoke_timer(void);
int main(int argc, char **argv) {
    if (argc != 2) return 2;
    int timer = strcmp(argv[1], "timer") == 0;
    __asm__ volatile("fxsave host_fx(%%rip)\n"
                     "fninit\nldmxcsr kernel_mxcsr(%%rip)\n"
                     "movdqu kernel_xmm(%%rip),%%xmm6\n"
                     "fxsave user64_fx_kernel(%%rip)"
                     ::: "memory", "xmm6");
    if (timer) invoke_timer(); else invoke_syscall();
    int entry_ok = seen_mxcsr == kernel_mxcsr && seen_cw == 0x037f &&
                   !(seen_flags & 0x400) &&
                   memcmp(seen_xmm, kernel_xmm, sizeof seen_xmm) == 0;
    int return_ok = returned_mxcsr == user_mxcsr && returned_cw == user_cw &&
                    (returned_flags & 0x400) &&
                    memcmp(returned_xmm, user_xmm, sizeof returned_xmm) == 0;
    printf("%s: kernel entry=%s (MXCSR=%#x FCW=%#x DF=%llu), user return=%s\n",
           argv[1], entry_ok ? "PASS" : "FAIL", seen_mxcsr, seen_cw,
           (unsigned long long)((seen_flags >> 10) & 1),
           return_ok ? "PASS" : "FAIL");
    return entry_ok && return_ok ? 0 : 1;
}
'''

SHIM = r'''
.text
.globl user64_dispatch
.globl user64_timer_dispatch
user64_dispatch:
user64_timer_dispatch:
    stmxcsr seen_mxcsr(%rip)
    fnstcw seen_cw(%rip)
    pushfq
    popq seen_flags(%rip)
    movdqu %xmm6,seen_xmm(%rip)
    pxor %xmm6,%xmm6
    xor %eax,%eax
    ret
.globl user64_after_syscall
user64_after_syscall:
    xor %eax,%eax
    ret

.macro invoke name,target
.globl \name
\name:
    sub $8,%rsp
    ldmxcsr user_mxcsr(%rip)
    fldcw user_cw(%rip)
    movdqu user_xmm(%rip),%xmm6
    std
    mov %rsp,%rdx
    xor %eax,%eax
    mov %ss,%ax
    push %rax
    push %rdx
    pushfq
    mov %cs,%ax
    push %rax
    lea .Lreturn\@(%rip),%rax
    push %rax
    jmp \target
.Lreturn\@:
    stmxcsr returned_mxcsr(%rip)
    fnstcw returned_cw(%rip)
    pushfq
    popq returned_flags(%rip)
    movdqu %xmm6,returned_xmm(%rip)
    cld
    fxrstor host_fx(%rip)
    add $8,%rsp
    ret
.endm
invoke invoke_syscall,syscall_isr
invoke invoke_timer,user64_timer_isr
.section .note.GNU-stack,"",@progbits
'''


def shipping_assembly():
    source = SOURCE.read_text()
    block = source[source.rindex('__asm__('):]
    literals = [match[1] for line in block.splitlines()
                if (match := re.match(r'\s*("(?:[^"\\]|\\.)*")', line))]
    assembly = ''.join(ast.literal_eval(literal) for literal in literals)
    return '.text\n' + assembly[assembly.index('.globl syscall_isr\n'):]


class UserFpuBoundaryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix='user-fpu-boundary-')
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(cls.temp.name)
        (root / 'test.c').write_text(HARNESS)
        (root / 'entry.S').write_text(shipping_assembly() + SHIM)
        cls.binary = root / 'test'
        subprocess.run(['cc', '-m64', '-O2', '-Wall', '-Wextra', '-Werror',
                        '-fno-pie', '-no-pie', '-mno-red-zone',
                        str(root / 'test.c'), str(root / 'entry.S'),
                        '-o', str(cls.binary)], check=True, capture_output=True,
                       text=True, timeout=30)

    def check_route(self, route):
        result = subprocess.run([str(self.binary), route], capture_output=True,
                                text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_syscall_kernel_controls_and_user_state(self):
        self.check_route('syscall')

    def test_timer_kernel_controls_and_user_state(self):
        self.check_route('timer')


if __name__ == '__main__':
    unittest.main()
