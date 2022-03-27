#ifndef VO_CMPE460_RPITIMEIMPL_H
#define VO_CMPE460_RPITIMEIMPL_H

#include <Rpi/RpiTime/RpiTimeComponentAc.hpp>

namespace Rpi
{
    class RpiTimeImpl : public RpiTimeComponentBase
    {
    public:
        RpiTimeImpl();
        void init(
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );

    private:
        void timeGetPort_handler(NATIVE_INT_TYPE portNum, Fw::Time &time) override;
        static void isrHandler();
    };
}

#endif //VO_CMPE460_RPITIMEIMPL_H
