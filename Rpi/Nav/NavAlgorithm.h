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

        virtual void draw_information(cv::Mat& image, std::string text);
    };

    class NavSimple : public NavAlgorithm
    {
    public:
        explicit NavSimple(CarController* car);

    private:
        enum EdgesFound
        {
            NONE_FOUND = 0,
            LEFT_FOUND = 1,
            RIGHT_FOUND = 2,
            BOTH_FOUND = LEFT_FOUND | RIGHT_FOUND,
        };

        EdgesFound find_edges(cv::Mat &image, F32 &left, F32 &right) const;

        bool process(CamFrame *frame, cv::Mat &image) override;
    };

    class NavComplexPid : public NavAlgorithm
    {
    public:
        explicit NavComplexPid(CarController* car);

    private:
        bool process(CamFrame *frame, cv::Mat &image) override;

        cv::Mat m_smaller;
    };
}

#endif //VO_CMPE460_NAVALGORITHM_H
