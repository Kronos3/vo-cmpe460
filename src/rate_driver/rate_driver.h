#ifndef VO_CMPE460_RATEDRIVER_H
#define VO_CMPE460_RATEDRIVER_H

#include <circle/types.h>
#include <cfg/vo_car.h>

typedef void (*RateDriverHandler)(void*);
typedef u32 RateGroupId;

class CTimer;
class RateDriver
{
    struct RateTimer
    {
        boolean m_configured;
        boolean m_started;
        u32 m_value;                    //!< Timer interrupt deadline
        u32 m_reload;                   //!< Timer period
        void* m_param;                  //!< Parameter to pass to handler
        RateDriverHandler m_handler;    //!< Handler to call when deadline runs out

        RateTimer()
        : m_configured(false), m_started(false),
        m_value(0), m_reload(0),
        m_param(nullptr), m_handler(nullptr)
        {}
    };

    u32 m_group_n;
    RateTimer m_groups[RATE_DRIVER_GROUPS_MAX];

public:
    explicit RateDriver(CTimer* timer);

    /**
     * Set up a rate group for periodic interrupts
     * @param n_ticks period in ticks (100 Hz standard)
     * @param handler callback to run periodically
     * @param param parameter to send to handler
     * @return rate group id of the group that was set up
     */
    RateGroupId setup(u32 n_ticks, RateDriverHandler handler, void* param);

    /**
     * Start a rate group
     * @param id id of rate group to start
     */
    void start(RateGroupId id);

    /**
     *
     * @param id
     */
    void stop(RateGroupId id);

    inline void interrupt();

    static void global_interrupt();
};

#endif //VO_CMPE460_RATEDRIVER_H