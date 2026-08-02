# Bootloader Multiboot sederhana - mode 32 bit protected mode
# Mengirim teks ke UART serial (COM1, 0x3F8), lalu keluar dengan kernel_exit(0)
# Syntax: AT&T (default GAS)

.code32

# Konstanta Multiboot
.set MULTIBOOT_MAGIC,        0x1BADB002
.set MULTIBOOT_FLAGS,        0x00000003
.set MULTIBOOT_CHECKSUM,     -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

# Konstanta UART
.set UART_BASE,              0x3F8

# Deklarasi eksternal (fungsi C++)
.extern kernel_exit

.section .multiboot, "a"
.align 4
multiboot_header:
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_FLAGS
    .long MULTIBOOT_CHECKSUM
    .long 0, 0, 0, 0, 0
    .long 0, 0, 0, 0

.section .text
.globl _start
.type _start, @function
_start:
    # Inisialisasi stack pointer
    movl $stack_top, %esp

    # Inisialisasi UART (115200 8N1)
    call uart_init

    # Bersihkan layar terminal (ANSI escape)
    call clear_screen

    # Cetak string ke UART
    movl $hello_msg, %esi
    call uart_puts

    # Panggil kernel_exit(0) – argumen status = 0
    pushl $0                # push status
    call kernel_exit        # tidak kembali jika berhasil
    # Jika ternyata kembali (fallback), hentikan CPU
    cli
hang:
    hlt
    jmp hang

# ------------------------------------------------------------
# Inisialisasi UART (115200 bps, 8N1, tanpa interrupt)
# ------------------------------------------------------------
uart_init:
    pushw %ax
    pushw %dx
    movw $UART_BASE+3, %dx
    movb $0x80, %al
    outb %al, %dx
    movw $UART_BASE, %dx
    movb $1, %al
    outb %al, %dx
    incw %dx
    movb $0, %al
    outb %al, %dx
    movw $UART_BASE+3, %dx
    movb $0x03, %al
    outb %al, %dx
    movw $UART_BASE+1, %dx
    movb $0, %al
    outb %al, %dx
    popw %dx
    popw %ax
    ret

# ------------------------------------------------------------
# Kirim satu karakter via UART
# Input: %al = karakter
# ------------------------------------------------------------
uart_putc:
    pushw %ax
    pushw %dx
    movw $UART_BASE+5, %dx   # Line Status Register
1:
    inb %dx, %al
    testb $0x20, %al         # THR empty?
    jz 1b
    popw %dx
    popw %ax
    pushw %ax
    pushw %dx
    movw $UART_BASE, %dx     # Data Register
    outb %al, %dx
    popw %dx
    popw %ax
    ret

# ------------------------------------------------------------
# Kirim string null-terminated via UART
# Input: %esi = pointer ke string
# ------------------------------------------------------------
uart_puts:
    pushl %esi
1:
    lodsb
    testb %al, %al
    jz 2f
    call uart_putc
    jmp 1b
2:
    popl %esi
    ret

# ------------------------------------------------------------
# Bersihkan layar terminal (ANSI escape)
# ------------------------------------------------------------
clear_screen:
    movl $ansi_clear, %esi
    call uart_puts
    ret

.section .data
hello_msg:
    .asciz "Hello, Multiboot World!\r\n"

ansi_clear:
    .asciz "\x1b[2J\x1b[H"

.section .bss
.align 16
stack_bottom:
    .skip 4096
stack_top:


.section .note.GNU-stack, "", @progbits
