#include "TestImpl.h"
#include <RPI/kernel.h>

namespace Svc
{
    TestImpl::TestImpl() = default;

    void TestImpl::init(NATIVE_INT_TYPE queueDepth, NATIVE_INT_TYPE instance)
    {
        TestComponentBase::init(queueDepth, instance);
    }

    void TestImpl::schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context)
    {
        static U32 i = 0;
        switch(i++ % 2)
        {
            case 0:
                kernel::led.On();
                break;
            case 1:
                kernel::led.Off();
                break;
        }

        log_DIAGNOSTIC_TestCycle(i);
    }
}
