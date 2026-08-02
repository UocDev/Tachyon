# Bootloader Multiboot sederhana - mode 32 bit protected mode
# Menulis teks ke layar VGA text mode (0xB8000)
# Syntax: AT&T (default GAS)

.code32

# Konstanta Multiboot
.set MULTIBOOT_MAGIC,        0x1BADB002
.set MULTIBOOT_FLAGS,        0x00000003   # bit 0: align modules, bit 1: memory info
.set MULTIBOOT_CHECKSUM,     -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

# Konstanta VGA
.set VGA_ADDR,               0xB8000
.set VGA_COLS,               80
.set VGA_ROWS,               25

.section .multiboot, "a"
.align 4
multiboot_header:
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHECKSUM
    # header address fields opsional (tidak diperlukan untuk ELF)
    .long 0, 0, 0, 0, 0
    # mode grafis opsional
    .long 0, 0, 0, 0

.section .text
.globl _start
.type _start, @function
_start:
    # Inisialisasi stack pointer
    movl $stack_top, %esp

    # Bersihkan layar (isi dengan spasi hitam background hitam)
    call clear_screen

    # Cetak string
    movl $hello_msg, %esi
    movl $VGA_ADDR, %edi    # baris 0, kolom 0
    movb $0x0F, %ah         # atribut: putih terang, latar hitam
    call print_string

    # Hentikan eksekusi (loop tak terhingga)
    cli
hang:
    hlt
    jmp hang

# Fungsi: print_string
# Input: %esi = pointer ke string null-terminated
#        %edi = alamat memori VGA untuk mulai menulis
#        %ah  = byte atribut warna
print_string:
    pushl %eax
    pushl %esi
    pushl %edi
1:
    lodsb                   # %al = (%esi), %esi++
    testb %al, %al
    jz 2f
    movw %ax, (%edi)        # tulis karakter + atribut ke VGA
    addl $2, %edi
    jmp 1b
2:
    popl %edi
    popl %esi
    popl %eax
    ret

# Fungsi: clear_screen
# Mengisi seluruh buffer VGA dengan spasi (0x20) dan atribut 0x0F
clear_screen:
    pushl %eax
    pushl %ecx
    pushl %edi
    movl $VGA_ADDR, %edi
    movl $(VGA_COLS * VGA_ROWS), %ecx
    movb $0x0F, %ah
    movb $' ', %al
1:
    movw %ax, (%edi)
    addl $2, %edi
    loop 1b
    popl %edi
    popl %ecx
    popl %eax
    ret

.section .data
hello_msg:
    .asciz "Hello, Multiboot World!"

.section .bss
.align 16
stack_bottom:
    .skip 4096
stack_top:
