
#ifndef VO_CMPE460_NAVPARAMS_H
#define VO_CMPE460_NAVPARAMS_H

namespace Rpi
{
    struct NavParams
    {
        struct
        {
            F32 center = 0.5;
            F32 turning = 2.0;
            F32 row = 0.5;
            F32 throttle = 0.35;
            F32 cutoff = 0.10;
        } simple;
    };
}

#endif //VO_CMPE460_NAVPARAMS_H
