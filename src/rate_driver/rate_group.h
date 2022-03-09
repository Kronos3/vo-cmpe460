#ifndef VO_CMPE460_RATE_GROUP_H
#define VO_CMPE460_RATE_GROUP_H

#include <circle/types.h>

#include <cfg/vo_car.h>
#include <rate_driver/rate_driver.h>
#include <rate_driver/scheduled.h>

typedef void (*RateHandler)(void*);

class RateGroup
{
    RateHandler m_handlers[RATE_DRIVER_GROUPS_HANDLERS_MAX];
    void* m_params[RATE_DRIVER_GROUPS_HANDLERS_MAX];
    u32 m_n;

    RateDriver* m_parent;
    RateGroupId m_group_id;

public:
    /**
     * Set up a rate group to create a one-to-many
     * relationship to
     * @param driver parent rate group driver
     * @param n_ticks Number of ticks to set up rate group scheduler with
     */
    RateGroup(RateDriver* driver, u32 n_ticks);

    /**
     * Set up a connection to the rate group
     * @param handler
     * @param param
     */
    void add(RateHandler handler, void* param);
    void add(Scheduled* scheduled);

    /**
     * Start the rate group
     */
    void start() const;

private:
    /**
     * Internal scheduler that will dispatch the handlers
     */
    static void schedIn(void*);
};

#endif //VO_CMPE460_RATE_GROUP_H
