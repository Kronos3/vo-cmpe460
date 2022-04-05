#ifndef VO_CMPE460_NAVALGORITHM_H
#define VO_CMPE460_NAVALGORITHM_H

#include <Rpi/Nav/NavAlgorithmTypeEnumAc.hpp>
#include <Rpi/Cam/Ports/CamFrame.h>
#include "NavParams.h"

namespace Rpi
{
    class CarController
    {
    public:
        virtual const NavParams& params() const = 0;

        virtual void steer(F32 steering) = 0;
        virtual void throttle(F32 throttle_left, F32 throttle_right) = 0;
    };

    class NavAlgorithm
    {
    public:
        explicit NavAlgorithm(CarController* car);
        virtual ~NavAlgorithm() = default;

        /**
         * Process a new frame and update the state of the car
         * @param image new image processed from Vis
         * @return keep_going, false to stop the car, true to keep moving the car
         */
        virtual bool process(CamFrame* frame, cv::Mat& image) = 0;

    protected:
        CarController* m_car;
    };

    class NavSimple : public NavAlgorithm
    {
    public:
        explicit NavSimple(CarController* car);

    private:
        enum EdgesFound
        {
            LEFT_FOUND = 1,
            RIGHT_FOUND = 2,
            BOTH_FOUND = LEFT_FOUND | RIGHT_FOUND,
        };

        EdgesFound find_edges(cv::Mat &image, F32 &left, F32 &right) const;

        bool process(CamFrame *frame, cv::Mat &image) override;
    };
}

#endif //VO_CMPE460_NAVALGORITHM_H
