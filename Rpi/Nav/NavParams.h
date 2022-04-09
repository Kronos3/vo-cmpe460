
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
        struct
        {
            F32 p = 0.5;
            F32 i = 0.5;
            F32 d = 0.5;
            F32 t = 0.5;
        } pid;
        struct
        {
            std::string calibration_filename;
        } complex;
    };
}

#endif //VO_CMPE460_NAVPARAMS_H
