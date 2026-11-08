#include <stdint.h>
#include <stddef.h>
#include "libc.h"
#include "console.h"

extern unsigned char font_psf[];

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04
#define FONT_WIDTH  8

struct psf1_header {
    uint8_t magic[2];
    uint8_t mode;
    uint8_t charsize;
};

static struct limine_framebuffer *fb;
static const uint8_t *glyphs;
static uint32_t font_height;

static uint64_t columns, rows;
static uint64_t cursor_x, cursor_y;

static uint32_t fg_color = 0x00CCCCCC;
static uint32_t bg_color = 0x00000000;

static void put_pixel(uint64_t x, uint64_t y, uint32_t color) {
    volatile uint32_t *pixels = fb->address;
    pixels[y * (fb->pitch / 4) + x] = color;
}

static void draw_glyph(char c, uint64_t col, uint64_t row) {
    const uint8_t *glyph = glyphs + (uint8_t)c * font_height;
    uint64_t px = col * FONT_WIDTH;
    uint64_t py = row * font_height;

    for (uint32_t y = 0; y < font_height; y++) {
        uint8_t bits = glyph[y];
        for (uint32_t x = 0; x < FONT_WIDTH; x++) {
            uint32_t color = (bits & (0x80 >> x)) ? fg_color : bg_color;
            put_pixel(px + x, py + y, color);
        }
    }
}

static void clear_row(uint64_t row) {
    for (uint64_t col = 0; col < columns; col++) {
        draw_glyph(' ', col, row);
    }
}

static void scroll(void) {
    uint8_t *base = fb->address;
    uint64_t row_bytes = fb->pitch * font_height;

    memmove(base, base + row_bytes, row_bytes * (rows - 1));
    clear_row(rows - 1);
}

static void new_line(void) {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= rows) {
        scroll();
        cursor_y = rows - 1;
    }
}

bool console_init(struct limine_framebuffer *framebuffer) {
    struct psf1_header *header = (struct psf1_header *)font_psf;

    if (header->magic[0] != PSF1_MAGIC0 || header->magic[1] != PSF1_MAGIC1) {
        return false;
    }

    fb = framebuffer;
    font_height = header->charsize;
    glyphs = font_psf + sizeof(struct psf1_header);

    columns = fb->width / FONT_WIDTH;
    rows = fb->height / font_height;

    console_clear();
    return true;
}

void console_set_color(uint32_t foreground, uint32_t background) {
    fg_color = foreground;
    bg_color = background;
}

void console_clear(void) {
    for (uint64_t y = 0; y < fb->height; y++) {
        for (uint64_t x = 0; x < fb->width; x++) {
            put_pixel(x, y, bg_color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

void console_putc(char c) {
    switch (c) {
    case '\n':
        new_line();
        return;
    case '\r':
        cursor_x = 0;
        return;
    case '\t':
        do {
            console_putc(' ');
        } while (cursor_x % 4 != 0);
        return;
    }

    draw_glyph(c, cursor_x, cursor_y);
    cursor_x++;
    if (cursor_x >= columns) {
        new_line();
    }
}

void console_puts(const char *s) {
    while (*s) {
        console_putc(*s++);
    }
}
