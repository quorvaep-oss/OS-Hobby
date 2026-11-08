#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "serial.h"

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

static void put_pixel(struct limine_framebuffer *fb, uint64_t x, uint64_t y, uint32_t color) {
    volatile uint32_t *pixels = fb->address;
    pixels[y * (fb->pitch / 4) + x] = color;
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

    for (uint64_t y = 0; y < fb->height; y++) {
        for (uint64_t x = 0; x < fb->width; x++) {
            uint8_t r = (uint8_t)(x * 255 / fb->width);
            uint8_t g = (uint8_t)(y * 255 / fb->height);
            uint8_t b = 0x80;
            put_pixel(fb, x, y, ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
        }
    }

    serial_puts("[Quorv-OS] Tela desenhada. Parando a CPU.\n");
    halt();
}
