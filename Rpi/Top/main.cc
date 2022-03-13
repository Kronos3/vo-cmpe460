//#include <cfg/vo_car.h>

#include <circle/memory.h>
#include <RPICfg.hpp>
#include <circle/sched/scheduler.h>
#include <VoCarCfg.h>
#include <Logger.hpp>
#include <Logger.h>
#include "Components.hpp"

#include "kernel.h"

class RpiAssertHook : public Fw::AssertHook
{
public:
    void printAssert(const I8* msg) override
    {
        Fw::Logger::logMsg("%s\r\n", (POINTER_CAST)msg);
    }

    void doAssert() override
    {
        EnterCritical();
        // Hang this task
        while(TRUE)
        {
            for (unsigned i = 1; i <= 2; i++)
            {
                kernel::led.On();
//                kernel::scheduler.MsSleep(100);
                kernel::tim.MsDelay(100);

                kernel::led.Off();
//                kernel::scheduler.MsSleep(100);
                kernel::tim.MsDelay(100);
            }

//            kernel::scheduler.MsSleep(100);
            kernel::tim.MsDelay(300);
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
    Rpi::Logger logger;

    CSerialDevice serial(&interruptSystem, /* fiq */ TRUE);
//    CI2CMaster i2c(I2C_DEVICE_NUMBER, I2C_FAST_MODE);
//    CSPIMasterDMA spi(&interruptSystem,
//                      SPI_SPEED,
//                      SPI_POL, SPI_PHA,
//                      /* DMAChannelLite */ FALSE);

    RpiAssertHook assertHook;
}

void assertion_failed (const char *pExpr, const char *pFile, unsigned nLine)
{
    Fw::Logger::logMsg("%s:%d %s\r\n", (POINTER_CAST)pFile, nLine, (POINTER_CAST)pExpr);

    while(TRUE)
    {
        for (unsigned i = 1; i <= 2; i++)
        {
            kernel::led.On();
            kernel::tim.MsDelay(100);

            kernel::led.Off();
            kernel::tim.MsDelay(100);
        }

        kernel::tim.MsDelay(1000);
    }
}

s32 main()
{
    Fw::Logger::registerLogger(&kernel::logger);
    kernel::interruptSystem.Initialize();
    kernel::tim.Initialize();
    kernel::serial.Initialize(UART_BAUDRATE);

    Fw::Logger::logMsg("Booting up\r\n");

    kernel::led.On();
    kernel::assertHook.registerHook();

    Rpi::init();
    constructRpiArchitecture();

    Rpi::start();

//    Rpi::prmDb.readParamFile();

    while (1)
    {
        kernel::scheduler.Yield();
    }
}
