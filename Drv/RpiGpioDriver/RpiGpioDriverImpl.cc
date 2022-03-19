#include "RpiGpioDriverImpl.h"
#include <circle/gpiopin.h>
#include <circle/gpiomanager.h>
#include <circle/pwmoutput.h>

namespace Drv
{
    void RpiGpioDriverImpl::init(NATIVE_INT_TYPE instance)
    {
        RpiGpioDriverComponentBase::init(instance);
    }

    void RpiGpioDriverImpl::gpioRead_handler(NATIVE_INT_TYPE portNum, bool &state)
    {

    }

    void RpiGpioDriverImpl::gpioWrite_handler(NATIVE_INT_TYPE portNum, bool state)
    {

    }
}
