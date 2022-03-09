
#include <circle/timer.h>
#include "dbg_act.h"

DbgAct::DbgAct(CActLED* act)
: m_act(act)
{
}

void DbgAct::flash()
{
    m_act->On();
    CTimer::SimpleMsDelay(100);
    m_act->Off();
    CTimer::SimpleMsDelay(100);
}

void DbgAct::flash_fatal()
{
    m_act->On();
    CTimer::SimpleMsDelay(100);
    m_act->Off();
    CTimer::SimpleMsDelay(100);
    m_act->On();
    CTimer::SimpleMsDelay(100);
    m_act->Off();
    CTimer::SimpleMsDelay(1000);
}
