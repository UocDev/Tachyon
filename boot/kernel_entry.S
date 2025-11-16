; kernel_entry.S - NASM, 64-bit entry + minimal Multiboot2 header
BITS 64

; --- Multiboot2 header (magic, architecture, header_length, checksum) ---
section .multiboot2
align 8
    dd 0xE85250D6        ; multiboot2 magic
    dd 0x00000000        ; architecture (0 = i386 compatible)
    dd 16                ; header length (4 fields * 4 bytes)
    dd 0x17ADAF1A        ; checksum = -(magic + arch + length) mod 2^32

; --- Code ---
section .text
global kernel_start
extern kernel_main

kernel_start:
    cli
    xor rbp, rbp

    ; setup stack
    mov rsp, stack_top

    ; call C entry
    call kernel_main

.hang:
    hlt
    jmp .hang

; --- BSS for stack ---
section .bss
align 16
    resb 16384           ; 16 KiB stack
stack_top:
section .note.GNU-stack noalloc noexec nowrite progbits
