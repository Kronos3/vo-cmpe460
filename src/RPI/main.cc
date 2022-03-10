//#include <cfg/vo_car.h>

#include <circle/startup.h>
#include <circle/memory.h>
#include <RPICfg.hpp>
#include <circle/sched/scheduler.h>
#include "Components.hpp"

#include "kernel.h"

class RPIAssertHook : public Fw::AssertHook
{
    void printAssert(const I8* msg) override
    {
        EnterCritical();
        U32 len = strlen(reinterpret_cast<const char*>(msg));
        U32 written = 0;
        kernel::serial.Write("\r\n", 2);
        while (written < len)
        {
            written += kernel::serial.Write(msg + written, len - written);
        }
        LeaveCritical();
    }

    void doAssert() override
    {
        // Disables interrupts
        EnterCritical();

        while(TRUE)
        {
            kernel::led.Blink(2, 100, 100);
            kernel::tim.MsDelay(100);
        }

        LeaveCritical();
    }
};

namespace kernel
{
    CMemorySystem memorySystem;
    CInterruptSystem interruptSystem;
    CScheduler scheduler;
    CActLED led;
    CTimer tim(&interruptSystem);
    CSerialDevice serial(&interruptSystem, /* useFIQ */ TRUE);

    RPIAssertHook assertHook;
}

s32 main()
{
    kernel::interruptSystem.Initialize();
    kernel::tim.Initialize();
    kernel::serial.Initialize(UART_BAUDRATE);

    kernel::assertHook.registerHook();

    RPI::init();
    constructRPIArchitecture();

    kernel::led.On();

    RPI::start();

    RPI::prmDb.readParamFile();

    while (1)
    {
        kernel::scheduler.Yield();
    }
}
