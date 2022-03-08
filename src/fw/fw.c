#include <fw/fw.h>
#include <stdarg.h>

// TODO(tumbar) Implement a Logging system
s32 uprintf(const char* fmt, ...)
{
    (void) fmt;
    return 0;
}

void fw_assertion_failure(const char* file, u32 line,
                          const char* expr_str,
                          u32 nargs, ...)
{
    // Mask all interrupts
    // Assertion reached, nothing else should run
    __asm__(  "cpsid if");

    uprintf("Assertion failed %s:%u : (%s)",
            file, line, expr_str);

    va_list args;
    va_start(args, nargs);
    for (u32 i = 0; i < nargs; i++)
    {
        uprintf(", %d", va_arg(args, int));
    }
    va_end(args);
    uprintf("\r\n");

    // Hang Mr. CPU please
//    BREAKPOINT(); // break point if we are in a debugger
    while (1)
    {
        __asm__("   wfi");
        // spoilers: it will never come because we masked all interrupts)
        // places the CPU in low power state until reset
    }
}

