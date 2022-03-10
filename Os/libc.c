#include <StandardTypes.hpp>

extern uint32_t strnlen(char const* str, unsigned long n);

uint32_t strnlen(char const* str, unsigned long n)
{
    const char* iter = str;
    for (; iter - str < n && *iter; iter++);
    return iter - str;
}
