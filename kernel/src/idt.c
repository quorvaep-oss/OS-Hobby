#include <stdint.h>
#include "idt.h"
#include "gdt.h"
#include "kprintf.h"
#include "console.h"

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idt_pointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_pointer idtr;

extern uint64_t isr_stub_table[32];

static const char *exception_names[32] = {
    "Divisao por zero",            "Debug",
    "NMI",                         "Breakpoint",
    "Overflow",                    "Bound Range Exceeded",
    "Opcode invalido",             "Dispositivo indisponivel",
    "Double Fault",                "Coprocessor Segment Overrun",
    "TSS invalido",                "Segmento nao presente",
    "Falha no segmento de pilha",  "General Protection Fault",
    "Page Fault",                  "Reservada",
    "Erro de ponto flutuante x87", "Alignment Check",
    "Machine Check",               "Erro de ponto flutuante SIMD",
    "Virtualizacao",               "Control Protection",
    "Reservada",                   "Reservada",
    "Reservada",                   "Reservada",
    "Reservada",                   "Reservada",
    "Hypervisor Injection",        "VMM Communication",
    "Seguranca",                   "Reservada"
};

static void idt_set_gate(uint8_t vector, uint64_t handler, uint8_t type_attr) {
    idt[vector].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[vector].selector    = GDT_KERNEL_CODE;
    idt[vector].ist         = 0;
    idt[vector].type_attr   = type_attr;
    idt[vector].offset_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[vector].offset_high = (uint32_t)(handler >> 32);
    idt[vector].reserved    = 0;
}

void idt_init(void) {
    for (uint8_t i = 0; i < 32; i++) {
        idt_set_gate(i, isr_stub_table[i], 0x8E);
    }

    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint64_t)&idt;

    __asm__ volatile ("lidt %0" : : "m"(idtr) : "memory");
}

void exception_handler(struct interrupt_frame *frame) {
    console_set_color(0x00FFFFFF, 0x00AA0000);
    kprintf("\n !!! EXCECAO %lu: %s !!! \n",
            frame->vector, exception_names[frame->vector]);
    console_set_color(0x00CCCCCC, 0x00101820);

    kprintf("Codigo de erro: 0x%lx\n", frame->error_code);
    kprintf("RIP=%p  CS=0x%lx  RFLAGS=0x%lx\n", (void *)frame->rip, frame->cs, frame->rflags);
    kprintf("RSP=%p  SS=0x%lx\n", (void *)frame->rsp, frame->ss);

    if (frame->vector == 14) {
        uint64_t cr2;
        __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
        kprintf("Endereco que causou o Page Fault (CR2): %p\n", (void *)cr2);
    }

    kprintf("\nRAX=%016lx RBX=%016lx RCX=%016lx RDX=%016lx\n",
            frame->rax, frame->rbx, frame->rcx, frame->rdx);
    kprintf("RSI=%016lx RDI=%016lx RBP=%016lx\n",
            frame->rsi, frame->rdi, frame->rbp);
    kprintf("R8 =%016lx R9 =%016lx R10=%016lx R11=%016lx\n",
            frame->r8, frame->r9, frame->r10, frame->r11);
    kprintf("R12=%016lx R13=%016lx R14=%016lx R15=%016lx\n",
            frame->r12, frame->r13, frame->r14, frame->r15);

    kprintf("\nSistema parado.\n");
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}
