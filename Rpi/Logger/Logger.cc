#include "Logger.h"
#include <cstdio>
#include <Rpi/Top/kernel.h>

void Rpi::Logger::log(
        const char* fmt,
        POINTER_CAST a0,
        POINTER_CAST a1,
        POINTER_CAST a2,
        POINTER_CAST a3,
        POINTER_CAST a4,
        POINTER_CAST a5,
        POINTER_CAST a6,
        POINTER_CAST a7,
        POINTER_CAST a8,
        POINTER_CAST a9)
{
    // Put the time stamp
    char timestamp[20];
    U32 s, us;
    kernel::tim.GetLocalTime(&s, &us);
    I32 len = snprintf(timestamp, sizeof timestamp, "%06d.%03d ", s, us);
    U32 written = 0;
    while (written < len)
    {
        written += kernel::serial.Write(timestamp + written, len - written);
    }

    char log_msg[160];
    len = snprintf(log_msg, sizeof(log_msg), fmt, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);

    // Add a null terminator just in case
    log_msg[sizeof (log_msg) - 1] = 0;

    // Put the message
    len = FW_MIN(sizeof(log_msg), len);
    written = 0;
    while (written < len)
    {
        written += kernel::serial.Write(log_msg + written, len - written);
    }
}
