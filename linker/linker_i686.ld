/* Linker script untuk kernel i686 ELF yang dimuat oleh bootloader Multiboot */
ENTRY(_start)

SECTIONS
{
    /* Kernel dimuat pada alamat fisik 1 MiB */
    . = 1M;

    /* Pastikan header Multiboot berada di awal */
    .multiboot : {
        *(.multiboot)
    }

    .text ALIGN(4K) : {
        *(.text)
    }

    /* Data read-only */
    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    /* Data initialized read-write */
    .data ALIGN(4K) : {
        *(.data)
    }

    /* Data uninitialized (BSS) */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
    }

    /* Akhir dari binary */
    _kernel_end = .;
}
