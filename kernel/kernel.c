// kernel.c - runs in x86_64 long mode
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void serial_write(const char* s) {
    while (*s) {
        outb(0x3F8, *s++);
    }
}

void kernel_main(void) {
    const char *msg = "Hello from Tachyon 64-bit kernel!\n";
    serial_write(msg);

    volatile uint16_t *vga = (uint16_t*)0xB8000;
    uint16_t attr = 0x0F00;

    for (int i = 0; i < 80; ++i) vga[i] = ' ' | attr;

    int i = 0;
    while (msg[i]) {
        vga[i] = (uint16_t)msg[i] | attr;
        ++i;
    }

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
