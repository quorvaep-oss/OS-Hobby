#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "serial.h"
#include "console.h"
#include "kprintf.h"

static void out_char(char c) {
    serial_putc(c);
    console_putc(c);
}

static void out_str(const char *s) {
    while (*s) {
        out_char(*s++);
    }
}

static void print_unsigned(uint64_t value, unsigned base, bool upper, int width, char pad) {
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char buffer[24];
    int i = 0;

    do {
        buffer[i++] = digits[value % base];
        value /= base;
    } while (value != 0);

    while (i < width && i < (int)sizeof(buffer)) {
        buffer[i++] = pad;
    }

    while (i > 0) {
        out_char(buffer[--i]);
    }
}

static void print_signed(int64_t value, int width, char pad) {
    if (value < 0) {
        out_char('-');
        print_unsigned((uint64_t)0 - (uint64_t)value, 10, false, width - 1, pad);
    } else {
        print_unsigned((uint64_t)value, 10, false, width, pad);
    }
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char *p = fmt; *p != '\0'; p++) {
        if (*p != '%') {
            out_char(*p);
            continue;
        }
        p++;

        char pad = ' ';
        if (*p == '0') {
            pad = '0';
            p++;
        }

        int width = 0;
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            p++;
        }

        bool is_long = false;
        while (*p == 'l') {
            is_long = true;
            p++;
        }

        switch (*p) {
        case 'd':
        case 'i': {
            int64_t value = is_long ? va_arg(args, long) : va_arg(args, int);
            print_signed(value, width, pad);
            break;
        }
        case 'u': {
            uint64_t value = is_long ? va_arg(args, unsigned long) : va_arg(args, unsigned int);
            print_unsigned(value, 10, false, width, pad);
            break;
        }
        case 'x':
        case 'X': {
            uint64_t value = is_long ? va_arg(args, unsigned long) : va_arg(args, unsigned int);
            print_unsigned(value, 16, *p == 'X', width, pad);
            break;
        }
        case 'p':
            out_str("0x");
            print_unsigned((uintptr_t)va_arg(args, void *), 16, false, 16, '0');
            break;
        case 's': {
            const char *s = va_arg(args, const char *);
            out_str(s != NULL ? s : "(null)");
            break;
        }
        case 'c':
            out_char((char)va_arg(args, int));
            break;
        case '%':
            out_char('%');
            break;
        case '\0':
            va_end(args);
            return;
        default:
            out_char('%');
            out_char(*p);
            break;
        }
    }

    va_end(args);
}
