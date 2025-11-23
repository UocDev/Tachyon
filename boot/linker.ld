ENTRY(kernel_entry_32)

PHDRS
{
  text   PT_LOAD FLAGS(5);
  rodata PT_LOAD FLAGS(4);
  data   PT_LOAD FLAGS(6);
}

SECTIONS
{
  . = 1M;

  .multiboot2 : ALIGN(8)
  {
    KEEP(*(.multiboot2))
  } :text

  .text : ALIGN(4K)
  {
    *(.text*)
    *(.rodata*)  /* include small literals in text to keep them in loadable seg */
  } :text

  .rodata : ALIGN(4K)
  {
    *(.rodata*)
  } :rodata

  .data : ALIGN(4K)
  {
    *(.data*)
  } :data

  .bss : ALIGN(4K)
  {
    *(COMMON)
    *(.bss*)
  } :data
}
