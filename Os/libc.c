#include <StandardTypes.hpp>

uint32_t strnlen(char const* str, uint32_t n)
{
    const char* iter = str;
    for (; iter - str < n && *iter; iter++);
    return iter - str;
}
