
#ifndef VO_CMPE460_RPILOGGER_H
#define VO_CMPE460_RPILOGGER_H

#include <Fw/Logger/Logger.hpp>

namespace Rpi
{
    class Logger : public Fw::Logger
    {
    public:
        void log(
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
                POINTER_CAST a9
        ) override;
    };
}

#endif //VO_CMPE460_RPILOGGER_H
