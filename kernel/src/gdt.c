#include <stdint.h>
#include "gdt.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  flags_limit;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_pointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct gdt_entry gdt[5];
static struct gdt_pointer gdtr;

static void gdt_set(int index, uint8_t access, uint8_t flags) {
    gdt[index].limit_low   = 0xFFFF;
    gdt[index].base_low    = 0;
    gdt[index].base_mid    = 0;
    gdt[index].access      = access;
    gdt[index].flags_limit = (uint8_t)((flags << 4) | 0x0F);
    gdt[index].base_high   = 0;
}


static void gdt_load(void) {
    __asm__ volatile ("lgdt %0" : : "m"(gdtr) : "memory");

    __asm__ volatile (
        "pushq %0\n"
        "leaq 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        "movw %1, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%ss\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        :
        : "i"(GDT_KERNEL_CODE), "i"(GDT_KERNEL_DATA)
        : "rax", "memory"
    );
}

void gdt_init(void) {
    gdt[0] = (struct gdt_entry){0};
    gdt_set(1, 0x9A, 0xA);
    gdt_set(2, 0x92, 0xC);
    gdt_set(3, 0xF2, 0xC);
    gdt_set(4, 0xFA, 0xA);

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base  = (uint64_t)&gdt;

    gdt_load();
}
