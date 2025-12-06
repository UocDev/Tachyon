// kernel/gdt.c
#include <stdint.h>
#include <stddef.h>   /* untuk size_t */
#include "gdt.h"

/* forward serial (already initialized by kernel_main) */
extern void serial_write(const char *s);

extern void *stack_end; /* provided by kernel.c */

struct __attribute__((packed)) gdtr {
    uint16_t limit;
    uint64_t base;
};

static uint64_t gdt_table[7]; /* null, code, data, usercode, userdata, tss low, tss high */

/* TSS struct (minimal fields we use) */
struct __attribute__((packed)) tss_struct {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((aligned(16)));

static struct tss_struct tss;
#define IST1_STACK_SIZE 8192
static uint8_t ist1_stack[IST1_STACK_SIZE] __attribute__((aligned(16)));

/* Build simple 64-bit code/data descriptor (base=0, limit=0) */
static inline uint64_t make_descriptor(uint32_t access, uint32_t flags)
{
    uint64_t desc = 0;
    desc |= ((uint64_t)access) << 40;
    desc |= ((uint64_t)flags)  << 52;
    return desc;
}

/* write 128-bit TSS descriptor at gdt_table[index] & gdt_table[index+1] */
static inline void write_tss_descriptor(int index, void *tss_addr, uint32_t tss_limit)
{
    uint64_t base = (uint64_t)tss_addr;
    uint64_t low = 0;
    uint64_t high = 0;

    /* low 64:
       limit[15:0] | base[15:0]<<16 | base[23:16]<<32 | access<<40 |
       limit[19:16]<<48 | base[31:24]<<56
    */
    low  = (uint64_t)(tss_limit & 0xFFFF);
    low |= ( (base & 0xFFFFULL) << 16 );
    low |= ( ((base >> 16) & 0xFFULL) << 32 );
    low |= ( (uint64_t)0x89ULL << 40 ); /* access: present + type 9 (available 64-bit TSS) */

    /* jangan lakukan shift >=32 pada 32-bit int; gunakan uint64_t */
    low |= ( ((uint64_t)((tss_limit >> 16) & 0xF)) << 48 );
    low |= ( ((base >> 24) & 0xFFULL) << 56 );

    /* high 64: base[63:32] */
    high = (base >> 32) & 0xFFFFFFFFULL;

    gdt_table[index]   = low;
    gdt_table[index+1] = high;
}

void gdt_install(void)
{
    serial_write("[gdt] start\n");

    /* zero out table */
    for (size_t i = 0; i < sizeof(gdt_table)/sizeof(gdt_table[0]); ++i) gdt_table[i] = 0ULL;

    /* Null descriptor (0) left as 0 */

    /* Kernel code selector (index 1 -> selector 0x08): access=0x9A, flags L=1 => 0x20 */
    gdt_table[1] = make_descriptor(0x9A, 0x20);

    /* Kernel data selector (index 2 -> selector 0x10): access=0x92 */
    gdt_table[2] = make_descriptor(0x92, 0x00);

    /* User code (index 3 -> selector 0x18): DPL=3, L=1 */
    gdt_table[3] = make_descriptor(0xFA, 0x20);

    /* User data (index 4 -> selector 0x20): DPL=3 */
    gdt_table[4] = make_descriptor(0xF2, 0x00);

    /* prepare TSS: zero it safely */
    {
        uint32_t *p = (uint32_t*)&tss;
        size_t words = sizeof(tss) / sizeof(uint32_t);
        for (size_t i = 0; i < words; ++i) p[i] = 0;
    }

    /* set RSP0 and IST1 */
    tss.rsp0 = (uint64_t)stack_end;
    tss.ist1 = (uint64_t)(ist1_stack + IST1_STACK_SIZE);

    /* write TSS descriptor entries at index 5 (occupies entries 5 and 6) */
    write_tss_descriptor(5, &tss, (uint32_t)(sizeof(tss) - 1));

    /* prepare GDTR */
    struct gdtr gdtr;
    gdtr.limit = (uint16_t)(sizeof(gdt_table) - 1);
    gdtr.base  = (uint64_t)&gdt_table;

    serial_write("[gdt] lgdt\n");
    asm volatile ("lgdt %0" : : "m"(gdtr));

    /* reload data segments (set DS/ES/FS/GS/SS to 0x10) */
    serial_write("[gdt] reload segments\n");
    asm volatile (
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        ::: "ax"
    );

    serial_write("[gdt] load tss\n");
    /* Load TR with TSS selector (index 5 -> selector = 5*8 = 0x28) */
    uint16_t tss_sel = 0x28;
    asm volatile ("ltr %0" : : "r"(tss_sel));

    serial_write("[gdt] done\n");
}
