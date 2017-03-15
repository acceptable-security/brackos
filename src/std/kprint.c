#include <kprint.h>
#include <drivers/vga.h>

// Print out a character
void kputch(char c) {
    // TODO - handle new lines, line wrapping, etc.
    // TODO - have an actual tty system ;)
    vga_putchar(c);
}

// Print out a hexidecimal encoded character. Options specify formatting.
void kputch_hex(char c, bool capital, bool pad) {
    const int high = (int) ((c & 0xF0) >> 4);
    const int low = (int) c & 0x0F;
    const char* chars = capital ? "0123456789ABCDEF" : "0123456789abcdef";


    if ( high != 0 || pad ) {
        kputch(chars[high]);
    }

    kputch(chars[low]);
}

// Print out a string.
void kputs(const char* str) {
    while ( *str ) {
        kputch(*str);
        str++;
    }
}

// Print out a number.
void kprint_int(long n) {
    if ( n == 0 ) {
        kputch('0');
        return;
    }

    bool negative = n < 0;
    n = (n < 0) ? -n : n; // abs w/o abs

    // TODO - Adjust this to calculate the size, rather than just using the arbitrary 11 for 32 bit ints.
    char tmp[1 + 11 + 1]; // sign + max int + null
    tmp[sizeof(tmp) - 1] = 0; // add null byte

    int index = sizeof(tmp) - 2;

    // add it into the temporary buffer backwards
    while ( n > 0 ) {
        tmp[index--] = '0' + (n % 10);
        n /= 10;
    }

    // add sign
    if ( negative ) {
        tmp[index--] = '-';
    }

    // print :)
    kputs(&tmp[index + 1]);
}

// Print out an unsigned number.
void kprint_uint(unsigned long n) {
    if ( n == 0 ) {
        kputch('0');
        return;
    }

    // TODO - Adjust this to calculate the size, rather than just using the arbitrary 11 for 32 bit ints.
    char tmp[11 + 1]; // max int + null
    tmp[sizeof(tmp) - 1] = 0; // add null byte

    int index = sizeof(tmp) - 2;

    // add it into the temporary buffer backwards
    while ( n > 0 ) {
        tmp[index--] = '0' + (n % 10);
        n /= 10;
    }

    // print :)
    kputs(&tmp[index + 1]);
}

// Print out an unsinged number in hexidecimal.
void kprint_hex_uint(unsigned long n, bool capital, bool pad) {
    if ( n == 0 ) {
        kputch('0');
        return;
    }

    bool first = true;

    for ( int i = sizeof(n) - 1; i >= 0; i-- ) {
        const char byte = (n >> (8 * i)) & 0xFF;

        if ( first && byte == 0 ) {
            continue;
        }

        kputch_hex(byte, capital, !first ? true : pad);
        first = false;
    }
}

// Print out a formatted string.
// Currently WIP, and only supports basic functionality.
void kprintf(const char* format, ...) {
    va_list args;
    char curr;

    va_start(args, format);

    while ( *format ) {
        curr = *format++;

        if ( curr == '%' ) {
            // TODO - read flags
            // TODO - read width
            // TODO - read precision
            // TODO - read length
            // SEE: http://www.cplusplus.com/reference/cstdio/printf/

            switch ( *format++ ) {
                case '\0':
                case '%':
                    break;

                case 'i':
                case 'd': {
                    const int read = va_arg(args, int);
                    kprint_int(read);
                    break;
                }

                case 'u': {
                    const unsigned int read = va_arg(args, unsigned int);
                    kprint_uint(read);
                    break;
                }

                case 's': {
                    const char* read = va_arg(args, const char*);
                    kputs(read);
                    break;
                }

                case 'c': {
                    const char read = (char) va_arg(args, int);
                    kputch(read);
                    break;
                }

                case 'p':
                case 'x': {
                    const unsigned int read = va_arg(args, unsigned int);
                    kprint_hex_uint(read, false, false);
                    break;
                }

                case 'X': {
                    const unsigned int read = va_arg(args, unsigned int);
                    kprint_hex_uint(read, true, false);
                    break;
                }
            }
        }
        else {
            kputch(curr);
        }
    }
}
