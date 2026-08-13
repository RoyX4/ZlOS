; raw_boot.asm - OUR bootloader. 512 bytes, no GRUB.
;
; This is the first code that runs after the BIOS: the BIOS loads exactly one
; sector (this one) to 0x7C00 and jumps to it. From nothing, it:
;
;   1. loads the kernel off the disk into memory at 0x10000
;   2. turns on the A20 line (so addresses above 1 MiB are reachable)
;   3. loads a GDT and switches the CPU into 32-bit protected mode
;   4. jumps to the kernel's raw entry point
;
; That is exactly the job GRUB was doing. This does it in 512 bytes that are
; ours. It replaces GRUB *and* the Multiboot handshake on the BIOS path.
;
; Assemble: nasm -f bin raw_boot.asm -o raw_boot.bin

[BITS 16]
[ORG 0x7C00]

KERNEL_SEG   equ 0x1000          ; 0x1000:0000 = physical 0x10000
KERNEL_LBA   equ 1               ; kernel starts at the 2nd sector of the disk
CHUNK_SECS   equ 64              ; sectors per BIOS read (32 KiB)
CHUNKS       equ 12              ; 12 * 32 KiB = 384 KiB, more than the kernel

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00               ; stack just below us
    sti

    mov [boot_drive], dl         ; BIOS leaves the boot drive number in dl

    ; ---- load the kernel: CHUNKS reads of CHUNK_SECS sectors each ----
    mov cx, CHUNKS
.load:
    push cx
    mov ah, 0x42                 ; INT 13h extended read (LBA, no CHS maths)
    mov dl, [boot_drive]
    mov si, dap
    int 0x13
    jc  disk_error
    add word [dap_seg], (CHUNK_SECS * 512) >> 4   ; advance dest by 32 KiB
    add dword [dap_lba], CHUNK_SECS               ; advance source LBA
    pop cx
    loop .load

    ; ---- A20, so memory above 1 MiB is addressable (fast A20 via port 0x92)
    in  al, 0x92
    or  al, 2
    out 0x92, al

    ; ---- enter 32-bit protected mode ----
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or  eax, 1
    mov cr0, eax
    jmp 0x08:pm_entry            ; far jump loads CS with the code selector

disk_error:
    mov ah, 0x0E                 ; BIOS teletype: print 'D' for disk error
    mov al, 'D'
    int 0x10
    cli
    hlt
    jmp $

[BITS 32]
pm_entry:
    mov ax, 0x10                 ; data selector into every data segment
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov esp, 0x90000
    jmp 0x10000                  ; into the kernel's raw entry

; ---- GDT: null, flat 32-bit code, flat 32-bit data ----
align 8
gdt:
    dq 0x0000000000000000        ; null
    dq 0x00CF9A000000FFFF        ; code: base 0, limit 4G, exec/read, ring 0
    dq 0x00CF92000000FFFF        ; data: base 0, limit 4G, read/write, ring 0
gdt_desc:
    dw gdt_desc - gdt - 1
    dd gdt

boot_drive: db 0

; ---- Disk Address Packet for INT 13h AH=42h ----
align 4
dap:
    db 0x10                      ; packet size
    db 0                         ; reserved
    dw CHUNK_SECS                ; sectors to read
dap_off:  dw 0x0000              ; destination offset
dap_seg:  dw KERNEL_SEG          ; destination segment (advanced each chunk)
dap_lba:  dd KERNEL_LBA          ; starting LBA (advanced each chunk)
          dd 0                   ; LBA high 32 bits

times 510-($-$$) db 0
dw 0xAA55                        ; boot signature - the BIOS checks for this
