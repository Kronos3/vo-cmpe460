#ifndef VO_CMPE460_RPIGPIODRIVERIMPL_H
#define VO_CMPE460_RPIGPIODRIVERIMPL_H

#include <Drv/RpiGpioDriver/RpiGpioDriverComponentAc.hpp>

namespace Drv
{
    class RpiGpioDriverImpl : public RpiGpioDriverComponentBase
    {
    public:
        void init(
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );

        void gpioRead_handler(NATIVE_INT_TYPE portNum, bool &state) override;
        void gpioWrite_handler(NATIVE_INT_TYPE portNum, bool state) override;
    };
}

#endif //VO_CMPE460_RPIGPIODRIVERIMPL_H
