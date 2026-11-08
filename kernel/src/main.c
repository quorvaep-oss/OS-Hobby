#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "serial.h"
#include "console.h"

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

static void halt(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

void kmain(void) {
    serial_init();
    serial_puts("\n[Quorv-OS] Kernel iniciado!\n");

    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        serial_puts("[Quorv-OS] ERRO: revisao do protocolo Limine nao suportada\n");
        halt();
    }

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        serial_puts("[Quorv-OS] ERRO: sem framebuffer\n");
        halt();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    serial_puts("[Quorv-OS] Framebuffer em ");
    serial_puthex((unsigned long)fb->address);
    serial_puts(", largura ");
    serial_puthex(fb->width);
    serial_puts(", altura ");
    serial_puthex(fb->height);
    serial_puts("\n");

    if (!console_init(fb)) {
        serial_puts("[Quorv-OS] ERRO: fonte invalida\n");
        halt();
    }

    console_set_color(0x0000AAFF, 0x00101820);
    console_clear();
    console_puts("Quorv-OS\n");
    console_set_color(0x00CCCCCC, 0x00101820);
    console_puts("========\n\n");
    console_puts("Kernel x86_64 iniciado pelo Limine.\n");
    console_puts("Texto desenhado pixel a pixel com uma fonte PSF.\n\n");
    console_puts("Tab:\tA\tB\tC\n");

    serial_puts("[Quorv-OS] Texto escrito. Parando a CPU.\n");
    halt();
}
