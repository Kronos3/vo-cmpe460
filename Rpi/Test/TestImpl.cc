#include "TestImpl.h"
#include <Rpi/Top/kernel.h>

namespace Svc
{
    TestImpl::TestImpl()
    : m_i(0), m_enabled(false)
    {
    }

    void TestImpl::init(NATIVE_INT_TYPE queueDepth, NATIVE_INT_TYPE instance)
    {
        TestComponentBase::init(queueDepth, instance);
    }

    void TestImpl::schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context)
    {
        if (!m_enabled) return;

        switch(m_i++ % 2)
        {
            case 0:
                kernel::led.On();
                break;
            case 1:
                kernel::led.Off();
                break;
        }

        log_DIAGNOSTIC_TestCycle(m_i);
        tlmWrite_NumCycle(m_i);
    }

    void TestImpl::TOGGLE_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        m_enabled = !m_enabled;
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }
}
