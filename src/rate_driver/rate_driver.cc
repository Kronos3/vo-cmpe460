#include <fw/fw.h>
#include <rate_driver/rate_driver.h>
#include <circle/timer.h>

static RateDriver* rgDriver = nullptr;

RateDriver::RateDriver(CTimer* timer)
: m_group_n(0)
{
    FW_ASSERT(!rgDriver && "Only one RateDriver may be initialized");
    rgDriver = this;
    timer->RegisterPeriodicHandler(RateDriver::global_interrupt);
}

RateGroupId RateDriver::setup(u32 n_ticks, RateDriverHandler handler, void* param)
{
    FW_ASSERT(handler);
    FW_ASSERT(m_group_n < RATE_DRIVER_GROUPS_MAX, RATE_DRIVER_GROUPS_MAX);

    m_groups[m_group_n].m_handler = handler;
    m_groups[m_group_n].m_param = param;
    m_groups[m_group_n].m_reload = n_ticks;
    m_groups[m_group_n].m_value = n_ticks;
    m_groups[m_group_n].m_configured = TRUE;
    m_groups[m_group_n].m_started = FALSE;

    return m_group_n++;
}

void RateDriver::start(RateGroupId id)
{
    FW_ASSERT(id < RATE_DRIVER_GROUPS_MAX);
    FW_ASSERT(m_groups[id].m_configured);

    m_groups[id].m_value = m_groups[id].m_reload;
    m_groups[id].m_started = TRUE;
}

void RateDriver::stop(RateGroupId id)
{
    FW_ASSERT(id < RATE_DRIVER_GROUPS_MAX);
    FW_ASSERT(m_groups[id].m_configured);

    m_groups[id].m_started = FALSE;
}

void RateDriver::interrupt()
{
    for (auto& group : m_groups)
    {
        if (group.m_configured && group.m_started)
        {
            if (--group.m_value == 0)
            {
                group.m_handler(group.m_param);
                group.m_value = group.m_reload;
            }
        }
    }
}

void RateDriver::global_interrupt()
{
    FW_ASSERT(rgDriver && "Rate group driver has not been initialized");
    rgDriver->interrupt();
}
