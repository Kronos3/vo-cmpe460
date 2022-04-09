#include "NavAlgorithm.h"
#include "opencv2/imgproc.hpp"
#include "VoCarCfg.h"

namespace Rpi
{
    NavComplexPid::NavComplexPid(CarController* car)
    : NavAlgorithm(car)
    {
    }

    bool NavComplexPid::process(CamFrame* frame, cv::Mat &image)
    {

        // Resize the image to the downscaled value
        // We don't actually lose any quality here because
        // Vis already downscaled this image to perform the edge detection
        cv::resize(image, m_smaller,
                   cv::Size(image.cols / VIS_RACE_DOWNSCALE,
                            image.rows / VIS_RACE_DOWNSCALE),
                   0, 0, cv::INTER_NEAREST // faster than lerping
        );

        return true;
    }
}