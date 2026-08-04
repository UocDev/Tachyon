/* Bootloader AArch64 - menulis "AArch64 Booting...\n" ke UART0 (PL011) lalu loop */
.globl _start

.equ UART0_BASE, 0x09000000
.equ UARTFR,   0x18
.equ UARTFR_TXFF, (1 << 5)
.equ UARTDR,   0x00

_start:
    ldr x0, =UART0_BASE
    ldr x1, =msg

1:  ldrb w2, [x1], #1
    cbz w2, halt
    bl putc
    b 1b

putc:
    /* tunggu TX FIFO tidak penuh */
    ldr w3, [x0, #UARTFR]
    tbnz w3, #5, putc
    str w2, [x0, #UARTDR]
    ret

halt:
    b halt

msg:
    .asciz "AArch64 Booting...\n"
