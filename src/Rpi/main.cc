//#include <cfg/vo_car.h>

#include <circle/startup.h>
#include <circle/memory.h>
#include <RPICfg.hpp>
#include <circle/sched/scheduler.h>
#include <VoCarCfg.h>
#include "Components.hpp"

#include "kernel.h"

class RpiAssertHook : public Fw::AssertHook
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
    CI2CMaster i2c(I2C_DEVICE_NUMBER, I2C_FAST_MODE);
    CSPIMasterDMA spi(&interruptSystem,
                      SPI_SPEED,
                      SPI_POL, SPI_PHA,
                      /* DMAChannelLite */ FALSE);

    RpiAssertHook assertHook;
}

s32 main()
{
    kernel::interruptSystem.Initialize();
    kernel::tim.Initialize();
    kernel::serial.Initialize(UART_BAUDRATE);

    kernel::assertHook.registerHook();

    Rpi::init();
    constructRpiArchitecture();

    kernel::led.On();

    Rpi::start();

    Rpi::prmDb.readParamFile();

    while (1)
    {
        kernel::scheduler.Yield();
    }
}
