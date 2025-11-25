; kernel_entry.S - NASM
; 32-bit entry (GRUB -> protected mode) then trampoline to 64-bit long mode
BITS 32
default rel

section .multiboot2
align 8
header_start:
    dd 0xE85250D6
    dd 0x0
    dd header_end - header_start
    dd -(0xE85250D6 + 0 + (header_end - header_start))
    dd 0
    dd 8
header_end:

section .text
global kernel_entry_32
extern kernel_main

kernel_entry_32:
    cli
    cld

    ; setup stack (use symbol stack_end in .bss)
    mov esp, stack_end

    ; load GDT
    lgdt [gdt_descriptor]

    ; set flat data selectors (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; ----------------------
    ; Fill page-directory entries for 2MiB pages (map low memory)
    ; We'll fill first 32 entries -> map 64 MiB (enough for VGA & kernel)
    ; ----------------------
    mov edi, pd_table      ; destination pointer
    xor eax, eax
    mov ecx, 32            ; number of PD entries to fill (32 * 2MiB = 64MiB)
    xor ebx, ebx           ; physical base (starts at 0)
.fill_pd:
    mov eax, ebx
    or eax, 0x83           ; flags: present(1) | rw(2) | PS(0x80)  => 0x83
    mov [edi], eax         ; low dword
    mov dword [edi+4], 0   ; high dword = 0 (physical <4GiB)
    add ebx, 0x200000      ; +2MiB
    add edi, 8
    loop .fill_pd

    ; ----------------------
    ; Enable PAE in CR4 (bit 5 = 0x20)
    ; ----------------------
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    ; load CR3 with physical address of pml4_table
    mov eax, pml4_table
    mov cr3, eax

    ; Enable long-mode in EFER: set LME (bit 8 = 0x100)
    mov ecx, 0xC0000080    ; IA32_EFER MSR
    rdmsr
    or eax, 0x100
    wrmsr

    ; Enable paging (CR0.PG = 1 << 31 = 0x80000000)
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; Far jump to 64-bit code selector (CS = 0x08)
    jmp 0x08:kernel_entry_64

; ---------------- 64-bit entry ----------------
BITS 64
kernel_entry_64:
    ; set up 64-bit stack
    lea rsp, [rel stack_end]
    xor rbp, rbp

    ; clear general regs (optional)
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rsi, rsi
    xor rdi, rdi
    xor r8, r8
    xor r9, r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15

    ; call C kernel (x86_64 calling convention)
    extern kernel_main
    call kernel_main

.hang64:
    hlt
    jmp .hang64

; ---------------- Data: GDT ----------------
BITS 32
section .data
align 8
gdt_table:
    dq 0x0000000000000000

    ; 64-bit code descriptor (access=0x9A, flags L=1 -> 0x20)
    dw 0x0000
    dw 0x0000
    db 0x00
    db 0x9A
    db 0x20
    db 0x00

    ; data descriptor
    dw 0x0000
    dw 0x0000
    db 0x00
    db 0x92
    db 0x00
    db 0x00

gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_table - 1
    dd gdt_table

; ---------------- Page tables ----------------
align 4096
pml4_table:
    dq pdpt_table + 3    ; present|rw = +3

align 4096
pdpt_table:
    dq pd_table + 3

align 4096
pd_table:
    times 512 dq 0

; ---------------- Stacks and BSS ----------------
section .bss
align 16
    resb 65536           ; 64 KiB stack + workspace
stack_start:
stack_end:

; done

section .note.GNU-stack noalloc noexec nowrite progbits
