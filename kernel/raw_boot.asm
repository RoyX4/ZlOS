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

; The kernel is loaded HIGH, at 1 MiB, not into low memory. It used to live at
; 0x10000, which put a hard ceiling on it: everything had to fit below the VGA
; hole at 0xA0000, and the desktop kernel hit that wall at 576 KiB and started
; being silently truncated. The BIOS can only read into the first megabyte, so
; each chunk is read into a low bounce buffer and then copied up - which needs
; 32-bit addressing from real mode, i.e. UNREAL MODE (see below).
KERNEL_SEG   equ 0x1000          ; bounce buffer at 0x10000, reused every chunk
KERNEL_DEST  equ 0x100000        ; where the kernel actually runs: 1 MiB
KERNEL_LBA   equ 1               ; kernel starts at the 2nd sector of the disk
CHUNK_SECS   equ 64              ; sectors per BIOS read (32 KiB)
; 60 * 32 KiB = 1.875 MiB, and the disk mkdisk.sh builds is 2 MiB, so this is
; the largest whole number of chunks that still fits behind the boot sector.
; It was 40 (1.25 MiB) against a 1.23 MiB kernel - 84 KiB of headroom, which
; the v10 type scale would have walked straight through. A kernel that outgrows
; this is not a build error: the loader reads exactly CHUNKS chunks whatever
; the kernel's size, so the tail is simply never loaded and the machine jumps
; into whatever happens to be at 1 MiB. mkdisk.sh now refuses to build an image
; that would do that.
CHUNKS       equ 60

; Scratch below the kernel (the boot sector ends at 0x7E00, the kernel starts
; at 0x10000), used only while we are still in real mode.
VBE_INFO     equ 0x8000          ; VbeInfoBlock   (512 bytes)
MODE_INFO    equ 0x8200          ; ModeInfoBlock  (256 bytes)
FB_INFO      equ 0x8300          ; what we hand the kernel (addr,pitch,w,h,bpp)
MAXW         equ 1920            ; widest mode we will take - 1080p. The back buffer
                                 ; + dirty-rect blitting is what pays for the pixels.

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00               ; stack just below us
    sti

    mov [boot_drive], dl         ; BIOS leaves the boot drive number in dl

    xor eax, eax                 ; "no framebuffer" until VBE says otherwise -
    mov [FB_INFO], eax           ; this memory is NOT zero on a cold boot

    ; ---- A20 first: without it, writes above 1 MiB wrap back to zero ----
    in  al, 0x92
    or  al, 2
    out 0x92, al

    ; ---- UNREAL MODE ----------------------------------------------------
    ; Real mode can only address 1 MiB, but the BIOS disk service only works
    ; in real mode. The way out is to enter protected mode just long enough to
    ; load FS with a segment whose limit is 4 GiB, then drop straight back.
    ; The CPU keeps the cached descriptor, so from ordinary real mode we can
    ; now write anywhere in memory through fs: with a 32-bit offset - while
    ; INT 13h still works. That is what makes loading a kernel to 1 MiB
    ; possible from a 512-byte boot sector.
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or  al, 1
    mov cr0, eax
    jmp $+2                      ; flush the prefetch queue
    mov bx, 0x10                 ; the flat 4 GiB data descriptor
    mov fs, bx
    mov eax, cr0
    and al, 0xFE
    mov cr0, eax                 ; back to real mode, FS keeps the big limit
    jmp $+2
    sti

    ; ---- load the kernel: read low, copy high ---------------------------
    mov edi, KERNEL_DEST
    mov cx, CHUNKS
.load:
    push cx
    mov ah, 0x42                 ; INT 13h extended read (LBA, no CHS maths)
    mov dl, [boot_drive]
    mov si, dap
    int 0x13
    jc  disk_error
    ; copy this chunk from the bounce buffer up to its real home
    mov esi, KERNEL_SEG << 4
    mov cx, (CHUNK_SECS * 512) / 4
.copy:
    mov eax, [fs:esi]
    mov [fs:edi], eax
    add esi, 4
    add edi, 4
    loop .copy
    add dword [dap_lba], CHUNK_SECS               ; advance source LBA
    pop cx
    loop .load

    ; ---- ask the card for a linear framebuffer, while we still have the BIOS
    ; VBE lives behind INT 0x10, which is real-mode only, so this is the last
    ; thing we do before leaving real mode for good. GRUB used to do exactly
    ; this on our behalf; now it is ours. If anything here fails we simply fall
    ; through with FB_INFO still zeroed and the kernel stays on VGA text.
    mov di, VBE_INFO
    mov dword [di], 'VBE2'       ; ask for VBE 2.0+ fields
    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x004F
    jne vbe_done                 ; no VBE at all

    mov si, [VBE_INFO + 0x0E]    ; VideoModePtr: offset...
    mov ax, [VBE_INFO + 0x10]    ; ...and segment
    mov fs, ax
    xor bx, bx                   ; bx = widest mode found so far (0 = none yet)
.scan:
    mov cx, [fs:si]              ; next mode number
    add si, 2
    cmp cx, 0xFFFF               ; the list ends with 0xFFFF
    je .chosen
    push si
    push bx
    mov ax, 0x4F01               ; query this mode's details
    mov di, MODE_INFO
    int 0x10
    pop bx
    pop si
    cmp ax, 0x004F
    jne .scan
    test byte [MODE_INFO], 0x80  ; attribute bit 7: has a linear framebuffer
    jz .scan
    cmp byte [MODE_INFO + 0x19], 32   ; 32 bits per pixel, like our drawing code
    jne .scan
    mov ax, [MODE_INFO + 0x12]   ; XResolution
    cmp ax, MAXW
    ja .scan                     ; wider than we want to push per frame
    cmp ax, bx
    jbe .scan                    ; no wider than the best so far
    mov bx, ax                   ; a new winner
    mov [best_mode], cx
    jmp .scan
.chosen:
    test bx, bx
    jz vbe_done                  ; nothing suitable - stay in text mode

    mov cx, [best_mode]          ; re-query the winner so MODE_INFO holds it
    mov ax, 0x4F01
    mov di, MODE_INFO
    int 0x10
    mov bx, [best_mode]
    or  bx, 0x4000               ; bit 14: give us the LINEAR framebuffer
    mov ax, 0x4F02
    int 0x10
    cmp ax, 0x004F
    jne vbe_done

    mov eax, [MODE_INFO + 0x28]  ; PhysBasePtr - where the pixels live
    mov [FB_INFO + 0], eax
    mov ax, [MODE_INFO + 0x10]   ; BytesPerScanLine (pitch)
    mov [FB_INFO + 4], ax
    mov ax, [MODE_INFO + 0x12]   ; width
    mov [FB_INFO + 6], ax
    mov ax, [MODE_INFO + 0x14]   ; height
    mov [FB_INFO + 8], ax
    mov al, [MODE_INFO + 0x19]   ; bits per pixel
    mov [FB_INFO + 10], al
vbe_done:

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
    mov esp, 0x600000            ; a stack in high memory (6 MiB). Low memory is
                                 ; too tight now: the kernel fills 0x10000..~0x92000
                                 ; and the framebuffer compositor nests deep.
    jmp KERNEL_DEST              ; into the kernel's raw entry, at 1 MiB

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
best_mode:  dw 0                 ; the widest 32bpp linear mode the card offers

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
