/* kernel/idt.c */

// @UocDev — Type Notation
// @NurAzizah — Helper

#include "idt.h"

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

static struct idt_entry idt[256];
static struct idt_ptr idtr;

static inline void lidt(void* base, uint16_t size)
{
    struct idt_ptr idtr;
    idtr.base = (uint64_t) base;
    idtr.limit = size - 1;
    asm volatile ("lidt %0" : : "m"(idtr));
}

void idt_set_gate(int n, uint64_t handler)
{
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = 0x08;
    idt[n].ist         = 0;
    idt[n].type_attr   = 0x8E;
    idt[n].offset_mid  = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].zero        = 0;
}

void idt_init()
{
    idt_set_gate(0,  (uint64_t)isr0);
    idt_set_gate(1,  (uint64_t)isr1);
    idt_set_gate(2,  (uint64_t)isr2);
    idt_set_gate(3,  (uint64_t)isr3);
    idt_set_gate(4,  (uint64_t)isr4);
    idt_set_gate(5,  (uint64_t)isr5);
    idt_set_gate(6,  (uint64_t)isr6);
    idt_set_gate(7,  (uint64_t)isr7);
    idt_set_gate(8,  (uint64_t)isr8);
    idt_set_gate(9,  (uint64_t)isr9);
    idt_set_gate(10, (uint64_t)isr10);
    idt_set_gate(11, (uint64_t)isr11);
    idt_set_gate(12, (uint64_t)isr12);
    idt_set_gate(13, (uint64_t)isr13);
    idt_set_gate(14, (uint64_t)isr14);
    idt_set_gate(15, (uint64_t)isr15);
    idt_set_gate(16, (uint64_t)isr16);
    idt_set_gate(17, (uint64_t)isr17);
    idt_set_gate(18, (uint64_t)isr18);
    idt_set_gate(19, (uint64_t)isr19);
    idt_set_gate(20, (uint64_t)isr20);
    idt_set_gate(21, (uint64_t)isr21);
    idt_set_gate(22, (uint64_t)isr22);
    idt_set_gate(23, (uint64_t)isr23);
    idt_set_gate(24, (uint64_t)isr24);
    idt_set_gate(25, (uint64_t)isr25);
    idt_set_gate(26, (uint64_t)isr26);
    idt_set_gate(27, (uint64_t)isr27);
    idt_set_gate(28, (uint64_t)isr28);
    idt_set_gate(29, (uint64_t)isr29);
    idt_set_gate(30, (uint64_t)isr30);
    idt_set_gate(31, (uint64_t)isr31);

    lidt(idt, sizeof(idt));
}
