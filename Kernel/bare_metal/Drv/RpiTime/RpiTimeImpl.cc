#include <circle/timer.h>
#include <Rpi/Top/kernel.h>
#include "RpiTimeImpl.h"

namespace Rpi
{
    static RpiTimeImpl* self = nullptr;
    RpiTimeImpl::RpiTimeImpl()
    {
        FW_ASSERT(!self, (POINTER_CAST)self);
        self = this;
    }

    void RpiTimeImpl::timeGetPort_handler(NATIVE_INT_TYPE portNum, Fw::Time &time)
    {
        unsigned seconds, useconds;
        CTimer::Get()->GetLocalTime(&seconds, &useconds);
        time.set(seconds, useconds);
    }

    void RpiTimeImpl::init(NATIVE_INT_TYPE instance)
    {
        RpiTimeComponentBase::init(instance);

        kernel::tim.RegisterPeriodicHandler(isrHandler);
    }

    void RpiTimeImpl::isrHandler()
    {
        FW_ASSERT(self);

        Svc::TimerVal start;
        if (self->isConnected_timerIsr_OutputPort(0))
        {
            self->timerIsr_out(0, start);
        }
    }
}
