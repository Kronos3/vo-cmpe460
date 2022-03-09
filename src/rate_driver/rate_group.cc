#include <fw/fw.h>
#include <rate_driver/rate_group.h>


RateGroup::RateGroup(RateDriver* driver, u32 n_ticks)
: m_handlers{nullptr}, m_params{nullptr}, m_n(0), m_parent(driver),
  m_group_id(0xFF)
{
    FW_ASSERT(m_parent);
    m_group_id = m_parent->setup(n_ticks, schedIn, this);
}

void RateGroup::add(RateHandler handler, void* param)
{
    FW_ASSERT(handler);
    FW_ASSERT(m_n < RATE_DRIVER_GROUPS_HANDLERS_MAX, m_n);

    m_handlers[m_n] = handler;
    m_params[m_n] = param;

    m_n++;
}

void RateGroup::start() const
{
    m_parent->start(m_group_id);
}

void RateGroup::schedIn(void* param)
{
    auto* _this = reinterpret_cast<RateGroup*>(param);
    for (u32 i = 0; i < _this->m_n; i++)
    {
        if (_this->m_handlers[i])
        {
            _this->m_handlers[i](_this->m_params[i]);
        }
        else
        {
            // A nullptr handler will mean this is a scheduled handler
            // The param will hold the pointer to the scheduled
            reinterpret_cast<Scheduled*>(param)->schedIn();
        }
    }
}

void RateGroup::add(Scheduled* scheduled)
{
    FW_ASSERT(scheduled);
    FW_ASSERT(m_n < RATE_DRIVER_GROUPS_HANDLERS_MAX, m_n);

    m_handlers[m_n] = nullptr;
    m_params[m_n] = scheduled;

    m_n++;
}
