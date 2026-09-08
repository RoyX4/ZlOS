; Raw position-independent parent. IPC rendezvous makes pending-wait testing
; independent of host load or a guessed sleep interval. Parent PID is 1000;
; each child's PID is obtained from the existing authenticated FROM result.
bits 64
default rel
org 0
%ifndef EXPECT_FAULT
%define EXPECT_FAULT 1
%endif
%ifndef ORPHAN_ORDER
%define ORPHAN_ORDER 0
%endif
%macro syscall3 4
    mov eax, %1
    mov rbx, %2
    mov rcx, %3
    mov rdx, %4
    int 0x80
%endmacro
%macro require_zero 0
    test rax,rax
    jnz failed
%endmacro
start:
    mov r15,rsp
    sub rsp,128
    syscall3 2,0,0,0
    cmp eax,1000
    jne failed
    syscall3 22,0,1,0             ; parent-only anonymous page
    test rax,rax
    js failed
    mov r12,rax
    syscall3 23,0,1,0
    require_zero
    mov dword [r12],0x12decafe
    mov dword [r15-4],0x11223344
    lea rbx,[child_name]
    mov ecx,child_name_end-child_name
    lea rdx,[r15-4]               ; eight bytes would cross the stack boundary
    mov eax,26
    int 0x80
    cmp rax,-22
    jne failed
    cmp dword [r15-4],0x11223344
    jne failed
    xor r13d,r13d
cycle:
    lea rbx,[child_name]
    mov ecx,child_name_end-child_name
    lea rdx,[rsp+32]
    mov eax,26
    int 0x80
    require_zero
    mov r14,[rsp+32]
    test r13d,r13d
    jz first
    syscall3 27,[rsp+48],rsp,32   ; old generation must not name the replacement
    cmp rax,-2
    jne failed
first:
    lea rbx,[child_name]
    mov ecx,child_name_end-child_name
    lea rdx,[rsp+40]
    mov qword [rdx],0x1234567
    mov eax,26
    int 0x80
    cmp rax,-28
    jne failed
    cmp qword [rsp+40],0x1234567
    jne failed
receive_ready:
    lea rbx,[rsp+56]
    syscall3 16,rbx,1,0
    cmp rax,-11
    jne got_ready
    syscall3 6,0,0,0
    jmp receive_ready
got_ready:
    cmp rax,1
    jne failed
    cmp byte [rsp+56],'R'
    jne failed
    syscall3 17,0,0,0
    test rax,rax
    jz failed
    mov rbp,rax
    syscall3 27,r14,rsp,32
    cmp rax,-11                    ; child waits for our ACK, so it cannot have exited
    jne failed
%if ORPHAN_ORDER = 1
    syscall3 3,-19,0,0             ; child is still waiting; kernel adopts it
    ud2
%endif
    lea rcx,[ack]
    syscall3 15,rbp,rcx,1
    cmp rax,1
    jne failed
; SEND admits only a runnable exact PID. Nobody else can reap our child,
; and the child does not sleep, so ENOENT proves it is terminal before the
; invalid-output retry. A guessed delay would not establish that ordering.
await_terminal:
%if ORPHAN_ORDER = 2
    syscall3 3,-19,0,0             ; terminal child record transfers unchanged
    ud2
%endif
    lea rcx,[ack]
    syscall3 15,rbp,rcx,1
    cmp rax,-2
    je terminal
    cmp rax,1
    je still_running
    cmp rax,-28                    ; its bounded inbox may have filled
    jne failed
still_running:
    syscall3 6,0,0,0
    jmp await_terminal
terminal:
    lea rcx,[r15-16]
    syscall3 27,r14,rcx,32
    cmp rax,-22                    ; terminal custody survives invalid output
    jne failed
wait_child:
    syscall3 27,r14,rsp,32
    cmp rax,-11
    jne got_result
    syscall3 6,0,0,0
    jmp wait_child
got_result:
    require_zero
    cmp dword [rsp],1
    jne failed
%if EXPECT_FAULT
    cmp dword [rsp+4],2
    jne failed
    cmp dword [rsp+8],0
    jne failed
    cmp dword [rsp+12],14
    jne failed
    cmp dword [rsp+16],4
    jne failed
    cmp [rsp+24],r12
    jne failed
%else
    cmp dword [rsp+4],1
    jne failed
    cmp dword [rsp+8],-37
    jne failed
    cmp qword [rsp+12],0
    jne failed
    cmp qword [rsp+24],0
    jne failed
%endif
    cmp dword [rsp+20],0
    jne failed
    cmp dword [r12],0x12decafe      ; parent's private page survives
    jne failed
    mov [rsp+48],r14
    inc r13d
    cmp r13d,2
    jb cycle
    syscall3 24,0,1,0
    require_zero
    lea rsi,[success]
print_success:
    movzx ebx,byte [rsi]
    test bl,bl
    jz done
    mov eax,1
    int 0x80
    inc rsi
    jmp print_success
done:
    syscall3 3,37,0,0
failed:
    ud2
child_name: db '/system/child.bin'
child_name_end:
ack: db 'A'
success: db 'SPAWN-WAIT-OK',0
