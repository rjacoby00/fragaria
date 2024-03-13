/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/printk.c
 *
 * printk() and helper functions
 * 
 */

#include <stdarg.h>
#include <stdint.h>

#include "printk.h"
#include "serial.h"
#include "string.h"
#include "vga.h"

static void output(char c)
{
        VGA_display_char(c);
        SER_write(&c, 1);
        return;
}

static int print_hex(unsigned int x)
{
        int i = 1, ret = 0;

        /* Calculate how many digits we're printing (log_16(x)) */
        for (int j = x / 16; j > 0; j = j / 16)
                i = i * 16;

        /* Print them in reverse order */
        for (; i > 0; i = i / 16) {
                if((x / i) % 16 < 10)
                        output('0' + ((x / i) % 16));
                else
                        output('a' - 10 + ((x / i) % 16));
                ret++;
        }

        return ret;
}

static int print_unsigned(unsigned int u)
{
        int i = 1, ret = 0;

        /* Calculate how many digits we're printing (log_10(u)) */
        for (int j = u / 10; j > 0; j = j / 10)
                i = i * 10;
        
        /* Print them in reverse order */
        for (; i > 0; i = i / 10) {
                output('0' + ((u / i) % 10));
                ret++;
        }

        return ret;
}

static int print_decimal(int d)
{
        int ret = 0;

        /* If we have a negative number, print negative and treat invert */
        if (d < 0) {
                d = d * -1;
                output('-');
                ret++;
        }

        /* Let unsigned int printer handle the now positive number */
        return ret + print_unsigned((unsigned int)d);
}

__attribute__((format (printf, 1, 2)))
int printk(const char * fmt, ...)
{
        int ret = 0;

        /* Set up va_list */
        va_list current;
        va_start(current, fmt);

        /* Parse the format string */
        while (*fmt) {
                if (*fmt == '%') {
                        if (!(*fmt++)) 
                                break;

                        switch (*fmt) {
                        case '%':
                                        output('%');
                                        ret++;
                                        fmt++;
                                        break;
                        case 'd':
                        case 'i':
                                        ret += print_decimal(va_arg(current,
                                                        int));
                                        fmt++;
                                        break;
                        case 'u':
                                        ret += print_unsigned(va_arg(current,
                                                        unsigned int));
                                        fmt++;
                                        break;
                        case 'x':
                        case 'X':       /* Handle X the same as x */
                                        ret += print_hex(va_arg(current,
                                                        unsigned int));
                                        fmt++;
                                        break;
                        case 'c':
                                        output(0xFF & va_arg(current,
                                                        unsigned int));
                                        ret++;
                                        fmt++;
                                        break;
                        case 'p':
                                        /* Print 0x */
                                        output('0');
                                        output('x');
                                        /* Then do the same as lx */
                                        ret += 2 + print_hex(va_arg(current,
                                                        unsigned long));
                                        fmt++;
                                        break;
                        case 'h':
                                        /* half/short length modier %h[dux] */
                                        /* if %h at end of string, this should
                                         * cleanly exit printk */ 
                                        if (!(*fmt++))
                                                continue;

                                        switch (*fmt) {
                                case 'd':
                                case 'i':
                                        ret += print_decimal(va_arg(current,
                                                        int));
                                        fmt++;
                                        break;
                                case 'u':
                                        ret += print_unsigned(va_arg(current,
                                                        unsigned int));
                                        fmt++;
                                        break;
                                case 'x':
                                case 'X':
                                        ret += print_hex(va_arg(current,
                                                        unsigned int));
                                        fmt++;
                                        break;
                                default:
                                        }
                                        break;
                        case 'l':
                                        /* long length modifier %l[dux] */
                                        if (!(*fmt++))
                                                continue;

                                        switch (*fmt) {
                                case 'd':
                                case 'i':
                                        ret += print_decimal(va_arg(current,
                                                        long));
                                        fmt++;
                                        break;
                                case 'u':
                                        ret += print_unsigned(va_arg(current,
                                                        unsigned long));
                                        fmt++;
                                        break;
                                case 'x':
                                case 'X':
                                        ret += print_hex(va_arg(current,
                                                        unsigned long));
                                        fmt++;
                                        break;
                                default:
                                        }

                                        break;
                        case 'q':
                                        /* quad/long long modifier %q[dux] */
                                        if (!(*fmt++))
                                                continue;

                                        switch (*fmt) {
                                case 'd':
                                case 'i':
                                        ret += print_decimal(va_arg(current,
                                                        long long));
                                        fmt++;
                                        break;
                                case 'u':
                                        ret += print_unsigned(va_arg(current,
                                                        unsigned long long));
                                        fmt++;
                                        break;
                                case 'x':
                                case 'X':
                                        ret += print_hex(va_arg(current,
                                                        unsigned long long));
                                        fmt++;
                                        break;
                                default:
                                        }

                                        break;
                        case 's':
                                        char * cs = va_arg(current, char *);
                                        ret += VGA_display_str(cs);
                                        SER_write(cs, strlen(cs));
                                        fmt++;
                                        break;
                        default:
                        }
                } else {
                        output(*fmt);
                        ret++;
                        fmt++;
                }
        }

        /* Clean up va_list */
        va_end(current);

        return ret;
}
