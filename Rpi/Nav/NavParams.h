
#ifndef VO_CMPE460_NAVPARAMS_H
#define VO_CMPE460_NAVPARAMS_H

#include <Fw/Types/BasicTypes.hpp>

namespace Rpi
{
    struct NavParams
    {
        struct
        {
            F32 row = 0.5;
            F32 step = 0.05;
            F32 discontinuity_thresh_1 = 10;
            F32 discontinuity_thresh_2 = 5;
            F32 normal_t = 20;
        } scan;
        struct
        {
            F32 p = 0.5;            //!< P
            F32 i = 0.5;            //!< I
            F32 d = 0.5;            //!< D
            F32 t = 0.5;            //!< Throttle factor
            F32 steering_t = 0.5;   //!< Time to lookahead and calculate steering error
            F32 throttle_t = 2.0;   //!< Time to lookahead and calculate throttle
        } pid;
        struct
        {
            F32 rho = 1;                //!< Distance resolution in pixels of the Hough grid
            F32 theta = 180;            //!< angular resolution in degrees of the Hough grid
            I32 threshold = 15;         //!< Minimum number of votes (intersections in Hough grid cell)
            F32 min_line_length = 50;   //!< Minimum number of pixels making up a line
            F32 max_line_gap = 20;      //!< Maximum gap in pixels between connectable line segments
        } contour;
    };
}

#endif //VO_CMPE460_NAVPARAMS_H
