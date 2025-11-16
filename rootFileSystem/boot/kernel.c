/* kernel.c - minimal kernel_main
   Compiled freestanding (-ffreestanding -nostdlib).
   Writes a short message to VGA text buffer and halts.
*/

typedef unsigned long uint64_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

void kernel_main(void) {
    const char *msg = "Hello from 64-bit kernel (GRUB -> QEMU)!";

    volatile uint16_t *vga = (uint16_t*)0xB8000;
    uint16_t attr = 0x0F00; /* white on black */

    /* Clear first 80x25 chars row and write message */
    for (int i = 0; i < 80; ++i) {
        vga[i] = ' ' | attr;
    }

    int i = 0;
    while (msg[i]) {
        vga[i] = (uint16_t)msg[i] | attr;
        ++i;
    }

    /* hang */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
