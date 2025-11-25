/* kernel/kernel.c */
#include <stdint.h>
#include "idt.h"

void isr_handler(uint64_t vector, uint64_t error)
{
    /* Untuk sementara print ke VGA text mode */
    volatile char* vga = (char*)0xB8000;
    vga[0] = 'E';
    vga[1] = 0x4F;

    vga[2] = 'X';
    vga[3] = 0x4F;

    vga[4] = 'C';
    vga[5] = 0x4F;

    vga[6] = '0' + (vector / 10);
    vga[7] = 0x4F;
    vga[8] = '0' + (vector % 10);
    vga[9] = 0x4F;

    while (1) asm("hlt");
}

void kernel_main()
{
    idt_init();

    /* Trigger test exception: divide by zero */
    int a = 1 / 0;

    while (1) asm("hlt");
}
