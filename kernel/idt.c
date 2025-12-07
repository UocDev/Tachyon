// kernel/idt.c
#include <stdint.h>
#include "idt.h"
#include "gdt.h" /* not required but ok to include */

extern void serial_write(const char *s);
extern void serial_write_hex(uint64_t v);

/* Extern isr stubs */
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

/* helper to install single gate with IST */
void idt_set_gate_ist(int n, void (*handler)(), uint8_t ist)
{
    uint64_t addr = (uint64_t)handler;
    idt[n].offset_low  = (uint16_t)(addr & 0xFFFF);
    idt[n].selector    = 0x08;       // kernel code selector
    idt[n].ist         = ist & 0x7;
    idt[n].type_attr   = 0x8E;       // present, DPL=0, interrupt gate (0xE)
    idt[n].offset_mid  = (uint16_t)((addr >> 16) & 0xFFFF);
    idt[n].offset_high = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
    idt[n].zero        = 0;
}

/* convenience wrapper (ist=0) */
void idt_set_gate(int n, void (*handler)()) {
    idt_set_gate_ist(n, handler, 0);
}

/* load IDT */
static inline void lidt_load(void *base, uint16_t size)
{
    idtp.base = (uint64_t)base;
    idtp.limit = size - 1;
    __asm__ volatile ("lidt %0" : : "m"(idtp));
}

void idt_install(void)
{
    /* clear */
    for (int i = 0; i < IDT_ENTRIES; ++i) {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].ist = 0;
        idt[i].type_attr = 0;
        idt[i].offset_mid = 0;
        idt[i].offset_high = 0;
        idt[i].zero = 0;
    }

    /* Set gates for exceptions 0..31.
       For double-fault (vector 8) set IST=1 to use TSS.ist1 stack.
    */
    idt_set_gate(0,  isr0);
    idt_set_gate(1,  isr1);
    idt_set_gate(2,  isr2);
    idt_set_gate(3,  isr3);
    idt_set_gate(4,  isr4);
    idt_set_gate(5,  isr5);
    idt_set_gate(6,  isr6);
    idt_set_gate(7,  isr7);
    idt_set_gate_ist(8,  isr8, 1);   /* DOUBLE FAULT -> use IST1 */
    idt_set_gate(9,  isr9);
    idt_set_gate(10, isr10);
    idt_set_gate(11, isr11);
    idt_set_gate(12, isr12);
    idt_set_gate(13, isr13);
    idt_set_gate(14, isr14);
    idt_set_gate(15, isr15);
    idt_set_gate(16, isr16);
    idt_set_gate(17, isr17);
    idt_set_gate(18, isr18);
    idt_set_gate(19, isr19);
    idt_set_gate(20, isr20);
    idt_set_gate(21, isr21);
    idt_set_gate(22, isr22);
    idt_set_gate(23, isr23);
    idt_set_gate(24, isr24);
    idt_set_gate(25, isr25);
    idt_set_gate(26, isr26);
    idt_set_gate(27, isr27);
    idt_set_gate(28, isr28);
    idt_set_gate(29, isr29);
    idt_set_gate(30, isr30);
    idt_set_gate(31, isr31);

    /* load */
    lidt_load(idt, sizeof(idt));
}
