#include <circle/types.h>
#include <circle/actled.h>
#include <circle/timer.h>

#include <rate_driver/rate_driver.h>
#include <dbg/dbg_button.h>
#include <dbg/dbg_act.h>
#include <circle/gpiomanager.h>

CInterruptSystem interruptSystem;
CTimer tim(&interruptSystem);
CActLED act;

RateDriver rgDriver(&tim);

CGPIOManager gpioManager(&interruptSystem);
DbgAct dbg_act(&act);
DbgButton dbg_button(4, &dbg_act, &gpioManager);
RebootButton reboot_button(17, &gpioManager);

s32 main()
{
    interruptSystem.Initialize();
    tim.Initialize();

    gpioManager.Initialize();
    dbg_button.Initialize();
    reboot_button.Initialize();
//    auto id = rgDriver.setup(50, interrupt, nullptr);
//    rgDriver.start(id);

    dbg_button.Breakpoint();

    while(TRUE)
    {
        act.Blink(1, 500, 500);
    }

    while(1);
}