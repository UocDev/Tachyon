// Kernel exit untuk freestanding environment (C++)
// Gunakan compiler: i686-elf-g++ atau sejenisnya dengan flag -ffreestanding -fno-exceptions -fno-rtti

#ifdef __cplusplus
extern "C" {
#endif

// Menghentikan eksekusi dengan status keluar untuk QEMU / Bochs
void kernel_exit(int status) {
    // Untuk QEMU: tulis status ke port 0x501 (isa-debug-exit)
    // Untuk Bochs: tulis ke port 0x501 juga (atau 0xE9 untuk shutdown?)
    // Di hardware nyata: tidak didukung, jadi hanya HLT
#if defined(__i386__) || defined(__x86_64__)
unsigned short port = 0x501;
__asm__ volatile (
    "outb %0, %1"
    :
    : "a"((unsigned char)status), "d"(port)
);
#endif

    // Fallback: jika tidak keluar, hentikan CPU selamanya
    while (1) {
        __asm__ volatile ("hlt");
    }
}

#ifdef __cplusplus
}
#endif
