#include "Logger.h"
#include <circle/logger.h>

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
    CLogger::Get()->Write("fsw", LogNotice, fmt,
                          a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
