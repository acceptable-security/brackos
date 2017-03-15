#include <stdarg.h>
#include <stdbool.h>

void kputch(char c) {
    // TODO - handle vga writes, new lines, line wrapping, etc.
}

void kputs(const char* str) {
    while ( *str ) {
        putch(*str);
        str++;
    }
}

void kprint_int(int n) {
    if ( n == 0 ) {
        putch('0');
        return;
    }

    bool negative = n < 0;
    n = (n < 0) ? -n : n; // abs w/o abs

    char tmp[13]; // sign + max int + null
    tmp[12] = 0;

    int index = 11;

    while ( n > 0 ) {
        tmp[index--] = '0' + (n % 10);
        n /= 10;
    }

    if ( negative ) {
        tmp[index--] = '-';
    }

    puts(&tmp[index]);
}

void kprintf(const char* format, ...) {
    va_list args;
    char curr;

    va_start(args, format);

    while ( *format ) {
        curr = *format++;

        if ( curr == '%' ) {
            switch ( *format ) {
                case '\0':
                    break;

                case '%':
                    format++;
                    break;

                case 'd': {
                    int read = va_arg(args, int);
                    kprint_int(read);
                    break;
                }
            }
        }
        else {
            putch(curr);
        }
    }
}
