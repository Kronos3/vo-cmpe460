#include <circle/timer.h>
#include <RPI/kernel.h>
#include "SystemTimeImpl.h"

static RPI::SystemTimeImpl* self = nullptr;
RPI::SystemTimeImpl::SystemTimeImpl()
{
    self = this;
}

void RPI::SystemTimeImpl::timeGetPort_handler(NATIVE_INT_TYPE portNum, Fw::Time &time)
{
    unsigned seconds, useconds;
    CTimer::Get()->GetLocalTime(&seconds, &useconds);
    time.set(seconds, useconds);
}

void RPI::SystemTimeImpl::init(NATIVE_INT_TYPE instance)
{
    SystemTimeComponentBase::init(instance);

    kernel::tim.RegisterPeriodicHandler(isrHandler);
}

void RPI::SystemTimeImpl::isrHandler()
{
    FW_ASSERT(self);

    Svc::TimerVal start;
    if (self->isConnected_timerIsr_OutputPort(0))
    {
        self->timerIsr_out(0, start);
    }
}
