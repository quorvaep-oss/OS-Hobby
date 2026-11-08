#pragma once

void serial_init(void);
void serial_putc(char c);
void serial_puts(const char *s);
void serial_puthex(unsigned long value);
