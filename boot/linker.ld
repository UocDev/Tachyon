ENTRY(kernel_start)

PHDRS
{
  text   PT_LOAD FLAGS(5); /* RX */
  rodata PT_LOAD FLAGS(4); /* R  */
  data   PT_LOAD FLAGS(6); /* RW */
}

SECTIONS
{
  /* Kernel load address */
  . = 1M;

  /* Multiboot2 header MUST be first */
  .multiboot2 : ALIGN(8)
  {
    KEEP(*(.multiboot2))
  } :text

  /* Code */
  .text : ALIGN(4K)
  {
    *(.text*)
  } :text

  /* Read-only data */
  .rodata : ALIGN(4K)
  {
    *(.rodata*)
  } :rodata

  /* Data */
  .data : ALIGN(4K)
  {
    *(.data*)
  } :data

  /* BSS */
  .bss : ALIGN(4K)
  {
    *(COMMON)
    *(.bss*)
  } :data
}
