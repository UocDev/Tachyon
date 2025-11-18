; kernel_entry.S - NASM, 64-bit entry + minimal Multiboot2 header
BITS 64

section .multiboot2
align 8
header_start:
    ; Multiboot2 header (magic, architecture, header_length, checksum)
    dd 0xE85250D6            ; magic
    dd 0x0                   ; architecture (0 = i386)
    dd header_end - header_start    ; header length (must be multiple of 8)
    dd -(0xE85250D6 + 0 + (header_end - header_start)) ; checksum

    ; minimal tags: end tag
    dd 0x0                   ; tag type = 0 (end)
    dd 8                     ; tag size = 8
header_end:

section .text
global kernel_start
extern kernel_main

kernel_start:
    cli
    xor rbp, rbp

    mov rsp, stack_top

    call kernel_main

.hang:
    hlt
    jmp .hang

section .bss
align 16
    resb 16384
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits
