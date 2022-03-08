#include <circle/timer.h>
#include <circle/actled.h>
#include <rate_driver/rate_driver.h>

void blinky_interrupt(CActLED* led)
{
    static boolean i = FALSE;
    i = !i;
    if (i)
    {
        led->On();
    }
    else
    {
        led->Off();
    }
}

int main()
{
    CInterruptSystem interrupt_system;
    interrupt_system.Initialize();

    CTimer tim(&interrupt_system);
    tim.Initialize();

    RateDriver rgDriver(&tim);

    CActLED led;
    led.On();

    auto rg = rgDriver.setup(50, reinterpret_cast<RateGroupHandler>(blinky_interrupt), &led);
    rgDriver.start(rg);

    while(TRUE)
    {
        __asm__ (" wfe");
    }
}
