#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <limine.h>

bool console_init(struct limine_framebuffer *framebuffer);
void console_set_color(uint32_t foreground, uint32_t background);
void console_clear(void);
void console_putc(char c);
void console_puts(const char *s);
