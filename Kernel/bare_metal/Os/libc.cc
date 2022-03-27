#include <Fw/Types/Assert.hpp>
#include <cstdarg>

extern uint32_t strnlen(char const* str, unsigned long n);

uint32_t strnlen(char const* str, unsigned long n)
{
    const char* iter = str;
    for (; iter - str < n && *iter; iter++);
    return iter - str;
}

void uart_putchar(char c)
{
//    kernel::serial.Write(&c, 1);
}

static inline void printf_string(
        const char* string)
{
    for (; *string; string++)
    {
        uart_putchar(*string);
    }
}

static inline void uart_put_udec(
        unsigned i,
        char pad_char, unsigned pad_amt)
{
    char buf_[10];
    char* stack_position = buf_;
    if (!i)
    {
        *(stack_position++) = '0';
    }

    while (i)
    {
        *(stack_position++) = '0' + (i % 10);
        i /= 10;
    }

    if ((stack_position - buf_) < pad_amt)
    {
        for (U32 j = 0; j < (pad_amt - (stack_position - buf_)); j++)
        {
            uart_putchar(pad_char);
        }
    }

    while (stack_position != buf_)
    {
        uart_putchar(*(--stack_position));
    }
}

static inline void uart_put_dec(
        int i,
        char pad_char, unsigned pad_amt)
{
    if (i < 0)
    {
        uart_putchar('-');
        i *= -1;
    }

    uart_put_udec(i, pad_char, pad_amt);
}

static inline void uart_put_floating(double f)
{
    uart_put_dec((int) f, '0', 0);
    f -= (int) f;

    uart_putchar('.');
    for (int i = 0; i < 6; i++)
    {
        f *= 10;
        uart_putchar(((int) f) + '0');
        f -= (int) f;
    }
}

static inline void uart_put_hex(U32 i, bool lower_case)
{
    static const char hex_mappings_l[] = "0123456789abcdef";
    static const char hex_mappings_u[] = "0123456789ABCDEF";
    const char* hex_mappings = lower_case ? hex_mappings_l : hex_mappings_u;

    for (unsigned j = 0; j < 8; j++)
    {
        unsigned shift_amt = ((7 - j) * 4);
        uart_putchar(hex_mappings[(i >> shift_amt) & 0xF]);
    }
}

int uvprintf(
        const char* const format,
        va_list args)
{
    const char* iter = format;
    while (*iter)
    {
        if (*iter == '%')
        {
            char pad_char = 0;
            U8 pad_amt = 0;

            iter++;
            switch (*iter)
            {
                case 's':
                    printf_string(va_arg(args, const char*));
                    break;
                case 'c':
                    uart_putchar(va_arg(args, int));
                    break;
                case 'd':
                case 'i':
                    uart_put_dec(va_arg(args, int), pad_char, pad_amt);
                    break;
                case 'u':
                    uart_put_udec(va_arg(args, unsigned), pad_char, pad_amt);
                    break;
                case 'x':
                    uart_put_hex(va_arg(args, int), true);
                    break;
                case 'X':
                    uart_put_hex(va_arg(args, int), false);
                    break;
                case 'f':
                    uart_put_floating(va_arg(args, double));
                    break;
                default:
                    pad_char = *(iter++);
                    pad_amt = (*(iter++) - '0');
                    {
                        if (*iter == 'd' || *iter == 'i')
                        {
                            uart_put_dec(va_arg(args, int), pad_char, pad_amt);
                        }
                        else if (*iter == 'u')
                        {
                            uart_put_udec(va_arg(args, int), pad_char, pad_amt);
                        }
                    }
                    FW_ASSERT(pad_amt <= 9, pad_amt);
                    break;
            }
        }
        else
        {
            uart_putchar(*iter);
        }

        iter++;
    }

//    uart_flush(uart);
    return iter - format;
}

int uprintf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int out = uvprintf(fmt, args);
    va_end(args);
    return out;
}

