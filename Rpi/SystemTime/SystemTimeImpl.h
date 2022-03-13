#ifndef VO_CMPE460_SYSTEMTIMEIMPL_H
#define VO_CMPE460_SYSTEMTIMEIMPL_H

#include <Rpi/SystemTime/SystemTimeComponentAc.hpp>

namespace Rpi
{
    class SystemTimeImpl : public SystemTimeComponentBase
    {
    public:
        SystemTimeImpl();
        void init(
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );

    private:
        void timeGetPort_handler(NATIVE_INT_TYPE portNum, Fw::Time &time) override;
        static void isrHandler();
    };
}

#endif //VO_CMPE460_SYSTEMTIMEIMPL_H
