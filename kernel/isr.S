/* kernel/isr.S */
    .intel_syntax noprefix
    .section .text
    .global isr_handler

/* Export all ISR stubs */
    .global isr0
    .global isr1
    .global isr2
    .global isr3
    .global isr4
    .global isr5
    .global isr6
    .global isr7
    .global isr8
    .global isr9
    .global isr10
    .global isr11
    .global isr12
    .global isr13
    .global isr14
    .global isr15
    .global isr16
    .global isr17
    .global isr18
    .global isr19
    .global isr20
    .global isr21
    .global isr22
    .global isr23
    .global isr24
    .global isr25
    .global isr26
    .global isr27
    .global isr28
    .global isr29
    .global isr30
    .global isr31

.macro ISR_NOERR num
    .global isr\num
isr\num:
    mov rdi, \num         /* arg1: vector */
    mov rsi, 0            /* arg2: error code = 0 */
    call isr_handler
    hlt
.endm

.macro ISR_ERR num
    .global isr\num
isr\num:
    mov rdi, \num         /* arg1: vector */
    mov rsi, [rsp]        /* arg2: error code (pushed by CPU) */
    call isr_handler
    add rsp, 8            /* clean error code */
    hlt
.endm


ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7

ISR_ERR 8      /* Double Fault */
ISR_NOERR 9
ISR_ERR 10     /* Invalid TSS */
ISR_ERR 11     /* Segment Not Present */
ISR_ERR 12     /* Stack Fault */
ISR_ERR 13     /* General Protection Fault */
ISR_ERR 14     /* Page Fault */

ISR_NOERR 15
ISR_NOERR 16
ISR_ERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

.section .note.GNU-stack,"",@progbits
