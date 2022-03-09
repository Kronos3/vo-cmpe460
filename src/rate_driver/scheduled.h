#ifndef VO_CMPE460_SCHEDULED_H
#define VO_CMPE460_SCHEDULED_H

class Scheduled
{
public:
    /**
     * The scheduler callback run by the rate group
     */
    virtual void schedIn() = 0;
};

#endif //VO_CMPE460_SCHEDULED_H
