// kernel/kernel.c

// @UocDev — Type Notation
// @NurAzizha — Helper

#include <stdint.h>
#include "idt.h"
#include "gdt.h"    /* <-- added support for GDT/TSS */

/* --- kernel stack (Option A: static stack defined in kernel) --- */
/* 16 KiB aligned stack for kernel (RSP0), guaranteed 16-byte aligned */
__attribute__((aligned(16)))
static uint8_t kernel_stack[16 * 1024];

/* Export symbol expected by gdt.c (stack end = top of stack, stack grows down) */
volatile void *stack_end = (void*)(kernel_stack + sizeof(kernel_stack));

/* ---------- Serial (COM1) ---------- */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void serial_init(void) {
    /* disable interrupts on serial */
    outb(0x3F8 + 1, 0x00);    // Disable all interrupts
    outb(0x3F8 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(0x3F8 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(0x3F8 + 1, 0x00);    //                  (hi byte)
    outb(0x3F8 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(0x3F8 + 2, 0xC7);    // FIFO: enable, clear, 14-byte threshold
    outb(0x3F8 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static int serial_is_transmit_empty(void) {
    uint8_t status;
    __asm__ volatile ("inb %1, %0" : "=a"(status) : "Nd"(0x3F8 + 5));
    return status & 0x20;
}

void serial_write(const char *s) {
    if (!s) return;
    while (*s) {
        while (!serial_is_transmit_empty()) { /* spin */ }
        outb(0x3F8, (uint8_t)*s++);
    }
}

void serial_write_hex64(uint64_t v) {
    const char *hex = "0123456789ABCDEF";
    char buf[17];
    buf[16] = '\0';
    for (int i = 15; i >= 0; --i) {
        buf[i] = hex[v & 0xF];
        v >>= 4;
    }
    serial_write(buf);
}

void serial_write_dec(uint64_t v) {
    char buf[32];
    int i = 0;
    if (v == 0) { serial_write("0"); return; }
    while (v > 0 && i < (int)sizeof(buf)-1) {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    }
    for (int j = i-1; j >= 0; --j) serial_write((char[]){buf[j],0});
}

/* ---------- VGA text writer (simple) ---------- */
static volatile uint16_t *VGA = (volatile uint16_t *)0xB8000;
static int vga_pos = 0;

void vga_clear(void) {
    for (int i = 0; i < 80*25; ++i) VGA[i] = (uint16_t)(' ') | (uint16_t)(0x07 << 8);
    vga_pos = 0;
}

void vga_puts(const char *s) {
    while (*s) {
        if (*s == '\n') {
            int col = vga_pos % 80;
            vga_pos += (80 - col);
            ++s;
            continue;
        }
        VGA[vga_pos++] = (uint16_t)(*s) | (uint16_t)(0x07 << 8);
        ++s;
        if (vga_pos >= 80*25) vga_pos = 0;
    }
}

/* ---------- ISR handler called from isr.S ----------
   Signature: (vector in rdi, error in rsi) per our isr.S
   We'll print vector, optional error, CR2 for page-fault.
*/
void isr_handler(uint64_t vector, uint64_t error) {
    serial_write("\n[EXC] Vector=");
    serial_write_dec(vector);
    serial_write("  Err=0x");
    serial_write_hex64(error);
    serial_write("\n");

    vga_puts("Exception: ");
    // print 2-digit vector
    char tmp[3];
    tmp[2] = 0;
    tmp[0] = '0' + (vector / 10);
    tmp[1] = '0' + (vector % 10);
    vga_puts(tmp);
    vga_puts(" ");

    if (vector == 14) { // page fault
        uint64_t cr2;
        __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
        serial_write("CR2=0x");
        serial_write_hex64(cr2);
        serial_write("\n");
        vga_puts("PF ");
        vga_puts("0x");
        // quick hex: not pretty; use serial for full hex
    }

    // halt forever
    serial_write("Halting.\n");
    vga_puts(" HALT");
    while (1) __asm__ volatile ("cli; hlt");
}

/* ---------- helpers to trigger exceptions cleanly ---------- */
/* Trigger divide-by-zero via inline asm to avoid -Wdiv-by-zero warning */
static void trigger_divide_by_zero(void) {
    asm volatile (
        "xor %%rdx, %%rdx\n\t"   /* clear RDX for dividend hi */
        "mov $1, %%rax\n\t"      /* dividend low */
        "xor %%rbx, %%rbx\n\t"   /* divisor = 0 */
        "idiv %%rbx\n\t"         /* will generate #DE */
        :
        :
        : "rax","rbx","rdx"
    );
}

/* Trigger page-fault by dereferencing a known bad address */
static void trigger_page_fault(void) {
    volatile uint64_t *p = (volatile uint64_t*)0xDEADBEEF;
    (void)*p;
}

/* ---------- kernel_main ---------- */
void kernel_main(void) {
    serial_init();
    serial_write("\n[Tachyon] kernel_main starting...\n");

    vga_clear();
    vga_puts("Tachyon 64-bit kernel\n");
    vga_puts("Installing GDT/TSS...\n");

    /* install GDT + TSS first */
    gdt_install();
    serial_write("GDT+TSS installed.\n");
    vga_puts("GDT+TSS installed.\n");

    vga_puts("Initializing IDT...\n");
    idt_init();
    serial_write("IDT installed.\n");
    vga_puts("IDT installed.\n");

    serial_write("TEST: Triggering divide-by-zero...\n");
    /* use asm-trigger to avoid compile-time warning */
    trigger_divide_by_zero();

    /* won't reach here if exception occurs; otherwise test page-fault */
    serial_write("TEST: Triggering page fault by reading 0xDEADBEEF...\n");
    trigger_page_fault();

    serial_write("Kernel reached idle loop.\n");
    vga_puts("Kernel ready. Idle...");

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
