
#ifndef VO_CMPE460_FRAME_H
#define VO_CMPE460_FRAME_H

#include <Rpi/Cam/Ports/CameraFrameSerializableAc.hpp>
#include <opencv4/opencv2/core/mat.hpp>

namespace Rpi
{
    inline cv::Mat frame_to_cv(const CameraFrame& frame)
    {
        return {frame.getheight(), frame.getwidth(),
                frame.getencoding(),
                (U8*)frame.getframe()};
    }
}

#endif //VO_CMPE460_FRAME_H
