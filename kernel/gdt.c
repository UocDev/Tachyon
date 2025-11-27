// kernel/gdt.c
#include <stdint.h>
#include "gdt.h"

/* Basic GDT layout:
 * 0: null
 * 1: kernel code (0x08)
 * 2: kernel data (0x10)
 * 3: user code   (0x18)
 * 4: user data   (0x20)
 * 5-6: TSS descriptor (selector 0x28)
 */

struct __attribute__((packed)) gdtr {
    uint16_t limit;
    uint64_t base;
};

static uint64_t gdt_table[7]; /* 7 * 8 = 56 bytes, plus TSS uses two descriptors */

/* 64-bit TSS (structure fields we need) */
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
};

/* TSS instance (aligned) */
static struct tss_struct tss __attribute__((aligned(16)));

/* IST stack for double-fault etc. Place in BSS (kernel link will provide) */
#define IST_STACK_SIZE 8192
static uint8_t ist1_stack[IST_STACK_SIZE] __attribute__((aligned(16)));

/* helper: build 64-bit code/data descriptor
 * access and flags as bytes:
 *  - access: e.g. 0x9A for kernel code, 0x92 for kernel data
 *  - flags: high nibble: granularity & L bit; use 0x20 for L bit
 */
static inline uint64_t build_gdt_entry(uint32_t access, uint32_t flags)
{
    /* 64-bit code/data descriptors for flat model commonly use base=0, limit=0xFFFFF with L=1 or limit=0 */
    uint64_t entry = 0;
    entry  = ((uint64_t)(0 & 0xFFFF)) | (((uint64_t)0 & 0xFFFF) << 16);
    /* base_mid and access: we place access at bits 40..47 */
    entry |= ((uint64_t)(access & 0xFF) << 40);
    /* flags (including L bit) and limit high nibble at bits 52..55 */
    entry |= ((uint64_t)(flags & 0xFF) << 52);
    /* base_high (bits 56..63) zero */
    return entry;
}

/* helper: write TSS descriptor (two 64-bit entries) */
static inline void write_tss_descriptor(uint64_t *gdt, void *tss_addr, uint32_t tss_limit)
{
    uint64_t base = (uint64_t)tss_addr;
    uint64_t low = 0;
    uint64_t high = 0;

    /* low: limit[15:0] | base[15:0]<<16 | base[23:16]<<32 | access<<40 | flags<<52 | base[31:24]<<56 */
    low  = (tss_limit & 0xFFFF);
    low |= ( (base & 0xFFFF) << 16 );
    low |= ( ( (base >> 16) & 0xFF ) << 32 );
    low |= ( (uint64_t)0x89 << 40 ); /* access: present + type(9 = avail 64-bit TSS) */
    low |= ( (uint64_t)0x00 << 48 ); /* flags low nibble = 0 */
    low |= ( ( (uint64_t)((tss_limit >> 16) & 0xF) ) << 48 );
    low |= ( ( (base >> 24) & 0xFF ) << 56 );

    /* high: base[63:32] */
    high = (uint64_t)( (base >> 32) & 0xFFFFFFFF );
    /* rest zero */

    gdt[5] = low;
    gdt[6] = high;
}

void gdt_install(void)
{
    /* zero table first */
    for (int i = 0; i < 7; ++i) gdt_table[i] = 0;

    /* NULL descriptor at index 0 (already zero) */

    /* index 1: kernel code descriptor (present, ring0, executable, readable), L bit */
    /* access 0x9A, flags: 0x20 (L=1) << 8 in build above, but we pass as flags byte */
    gdt_table[1] = build_gdt_entry(0x9A, 0x20);

    /* index 2: kernel data descriptor (present, ring0, data, writable) */
    gdt_table[2] = build_gdt_entry(0x92, 0x00);

    /* index 3: user code (DPL=3) */
    gdt_table[3] = build_gdt_entry(0xFA, 0x20); /* 0xFA = present + DPL=3 + executable */

    /* index 4: user data */
    gdt_table[4] = build_gdt_entry(0xF2, 0x00);

    /* prepare TSS */
    for (int i = 0; i < (int)sizeof(tss)/sizeof(uint32_t); ++i) ((uint32_t*)&tss)[i] = 0;

    /* set RSP0 to kernel stack provided by bootloader (stack_end symbol) if exists */
    extern char stack_end; /* defined in bootloader.S */
    tss.rsp0 = (uint64_t)&stack_end;

    /* IST1: point to our IST stack top (grow-down) */
    tss.ist1 = (uint64_t)( (uint8_t*)ist1_stack + IST_STACK_SIZE );

    /* write TSS descriptor entries at gdt[5..6] */
    write_tss_descriptor(gdt_table, &tss, (uint32_t)(sizeof(tss)-1));

    /* prepare GDTR */
    struct gdtr {
        uint16_t limit;
        uint64_t base;
    } __attribute__((packed)) gdtr;

    gdtr.limit = (uint16_t)(sizeof(gdt_table) - 1);
    gdtr.base  = (uint64_t)&gdt_table;

    /* load GDT */
    asm volatile ("lgdt %0" : : "m"(gdtr));

    /* reload segment registers with new selectors */
    asm volatile (
        "mov $0x10, %%ax\n"  /* data selector = index 2 (0x10) */
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        : : : "ax"
    );

    /* far jump to flush CS: use asm to do ljmp to code selector 0x08 */
    asm volatile (
        "pushq $0x08\n"
        "lea 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        ::: "rax"
    );

    /* load TR with TSS selector (index 5 -> selector 0x28) */
    uint16_t tss_sel = 0x28;
    asm volatile ("ltr %0" : : "r"(tss_sel));
}
