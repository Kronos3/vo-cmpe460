#ifndef VO_CMPE460_DBG_ACT_H
#define VO_CMPE460_DBG_ACT_H

#include <circle/types.h>
#include <circle/actled.h>

class DbgAct
{
    CActLED* m_act;

public:
    explicit DbgAct(CActLED* act);

    /**
     * Quick flashing
     */
    void flash();

    /**
     * Heart beat
     */
    void flash_fatal();
};

#endif //VO_CMPE460_DBG_ACT_H
