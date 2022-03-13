#include <circle/types.h>
#include <circle/actled.h>
#include <circle/timer.h>

#include <circle/gpiomanager.h>

NATIVE_INT_TYPE Fw::SwAssert(unsigned char const*, unsigned int)
{
    return 0;
}

CInterruptSystem interruptSystem;
CTimer tim(&interruptSystem);
CActLED act;

s32 main()
{
    interruptSystem.Initialize();
    tim.Initialize();

    while(TRUE)
    {
        act.Blink(1, 500, 500);
    }
}