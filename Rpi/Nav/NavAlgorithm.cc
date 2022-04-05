#include "NavAlgorithm.h"

namespace Rpi
{
    NavAlgorithm::NavAlgorithm(CarController* car)
    : m_car(car)
    {
    }

    NavSimple::NavSimple(CarController* car)
    : NavAlgorithm(car)
    {
    }

    bool NavSimple::process(CamFrame* frame, cv::Mat &image)
    {

        return false;
    }

    NavSimple::EdgesFound NavSimple::find_edges(cv::Mat &image, F32 &left, F32 &right) const
    {
        return NavSimple::LEFT_FOUND;
    }
}
