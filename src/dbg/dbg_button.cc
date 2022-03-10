#include <dbg/dbg_button.h>
#include <circle/memio.h>
#include <circle/timer.h>


Button::Button(u32 pin, CGPIOManager* gpio_mgr)
        : m_pin(pin, GPIOModeInputPullUp, gpio_mgr)
{
}

void Button::InterruptHandler(void* param)
{
    reinterpret_cast<Button*>(param)->Interrupt();
}

void Button::Initialize()
{
    m_pin.ConnectInterrupt(InterruptHandler, this);

    // Active LOW!!
    // We want button press to fire interrupt
    m_pin.EnableInterrupt(GPIOInterruptOnFallingEdge);
}

DbgButton::DbgButton(unsigned pin, DbgAct* dbg_act, CGPIOManager* gpio_mgr)
: Button(pin, gpio_mgr), m_dbg_act(dbg_act), m_blocked(FALSE)
{
}

void DbgButton::Interrupt()
{
    m_blocked = FALSE;
}

void DbgButton::Breakpoint(u32 n)
{
    m_blocked = TRUE;
    while(m_blocked)
    {
        if (n)
        {
            for(u32 i = 0; m_blocked && i < n; i++)
            {
                m_dbg_act->flash();
            }

            if (m_blocked) CTimer::SimpleMsDelay(500);
        }
        else
        {
            m_dbg_act->flash();
        }
    }
}

ExecuteButton::ExecuteButton(u32 pin, CGPIOManager* gpio_mgr, void (* function)())
: Button(pin, gpio_mgr), m_function(function)
{
}

void ExecuteButton::Interrupt()
{
    m_function();
}
