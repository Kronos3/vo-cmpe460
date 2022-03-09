#include <fw/fw.h>
#include <stdarg.h>
#include <circle/synchronize.h>

static FwFatalHandler fatalHandler = nullptr;
void fw_register_fatal_handler(FwFatalHandler func)
{
    fatalHandler = func;
}

void fw_assertion_failure(const char* file, u32 line,
                          const char* expr_str,
                          u32 nargs, ...)
{
    // Mask all interrupts
    // Assertion reached, nothing else should run
    DisableIRQs();
    DisableFIQs();

    va_list args;
    va_start(args, nargs);

    if (fatalHandler)
    {
        fatalHandler(file, line, expr_str, nargs, args);
    }

    va_end(args);

    // Hang Mr. CPU please
    while (TRUE)
    {
        __asm ("  wfi");
    }
}

