bits 64
default rel
org 0
%ifndef EXPECT_FAULT
%define EXPECT_FAULT 1
%endif
%ifndef ORPHAN_ORDER
%define ORPHAN_ORDER 0
%endif
start:
    sub rsp,16
    mov eax,15
    mov ebx,1000
    lea rcx,[ready]
    mov edx,1
    int 0x80
    cmp rax,1
    jne failed
%if ORPHAN_ORDER = 1
wait_parent:
    mov eax,15
    mov ebx,1000
    lea rcx,[ready]
    mov edx,1
    int 0x80
    cmp rax,-2                    ; parent has exited, no guessed time delay
    je orphan_alive
    cmp rax,1
    je parent_running
    cmp rax,-28
    jne failed
parent_running:
    mov eax,6
    int 0x80
    jmp wait_parent
orphan_alive:
    lea rsi,[orphan_marker]
print_orphan:
    movzx ebx,byte [rsi]
    test bl,bl
    jz orphan_continue
    mov eax,1
    int 0x80
    inc rsi
    jmp print_orphan
orphan_continue:
%else
receive_ack:
    mov eax,16
    mov rbx,rsp
    mov ecx,1
    xor edx,edx
    int 0x80
    cmp rax,-11
    jne got_ack
    mov eax,6
    int 0x80
    jmp receive_ack
got_ack:
    cmp rax,1
    jne failed
    cmp byte [rsp],'A'
    jne failed
%endif
%if EXPECT_FAULT
    lea rbx,[start]
    mov rax,[rbx+0x6000]           ; only the parent committed this virtual page
    jmp failed                    ; access succeeding must fail the parent's oracle
%else
    mov eax,3
    mov ebx,-37
    int 0x80
%endif
failed:
    ud2
ready: db 'R'
orphan_marker: db 'ORPHAN-ALIVE',0
