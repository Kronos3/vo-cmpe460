#ifndef VO_CMPE460_NAVALGORITHM_H
#define VO_CMPE460_NAVALGORITHM_H

#include <Utils/Types/CircularBuffer.hpp>

#include <Rpi/Nav/NavAlgorithmTypeEnumAc.hpp>
#include <Rpi/Cam/Ports/CamFrame.h>

#include <Rpi/Vis/VisRecord.h>

#include "NavParams.h"
#include "VoCarCfg.h"

#include "spline.h"

#define THETA_DIVISIONS (512)

namespace Rpi
{
    class CarController
    {
    public:
        virtual const NavParams &params() const = 0;

        virtual void steer(F32 steering) = 0;

        virtual void throttle(F32 throttle_left, F32 throttle_right) = 0;
    };

    enum EdgesFound
    {
        NONE_FOUND = 0,
        LEFT_FOUND = 1,
        RIGHT_FOUND = 2,
        BOTH_FOUND = LEFT_FOUND | RIGHT_FOUND,
    };

    EdgesFound find_edges(cv::Mat &image,
                          const cv::Point& start,
                          I32 &left,
                          I32 &right);

    bool trace_line(cv::Mat& image,
                    const cv::Point& start,
                    cv::Point& next,
                    F32 norm_mag,
                    I32 &center_angle);

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
        virtual bool process(CamFrame* frame, cv::Mat &image) = 0;

    protected:
        CarController* m_car;

        virtual void draw_information(cv::Mat &image, std::string text);
    };

    class NavSimple : public NavAlgorithm
    {
    public:
        explicit NavSimple(CarController* car);

    private:
        bool process(CamFrame* frame, cv::Mat &image) override;
    };

    class NavPid : public NavAlgorithm
    {
    public:
        explicit NavPid(CarController* car);

    protected:
        F64 pid(F64 e_t);

    private:

        bool process(CamFrame* frame, cv::Mat &image) override = 0;

        U32 m_history_i;
        F64 m_history[NAV_PID_HISTORY_N];
    };

    class NavSimplePid : public NavPid
    {
    public:
        explicit NavSimplePid(CarController* car);

        ~NavSimplePid() override = default;

    private:
        void find_track_edges(cv::Mat &image,
                              std::vector<cv::Point>& left_line,
                              std::vector<cv::Point>& right_line
                              ) const;

        bool process(CamFrame* frame, cv::Mat &image) override;

        cv::Mat m_resized;
    };

    class NavComplexPid : public NavPid
    {
    public:
        explicit NavComplexPid(CarController* car);

        ~NavComplexPid() override = default;

    private:
        /**
         * Attempt to converge on the racing line
         * @param image
         */
        void draw_spline(cv::Mat &image);

        bool process(CamFrame* frame, cv::Mat &image) override;

        // Drawn by draw spline
        std::vector<F64> sx;      //!< Parametric spline X component
        std::vector<F64> sy;      //!< Parametric spline Y component
    };

    // Diagnostic navigation (no car control)
    class NavDiagContour : public NavAlgorithm
    {
    public:
        explicit NavDiagContour(CarController* car)
                : NavAlgorithm(car)
        {}

    private:
        bool process(CamFrame* frame, cv::Mat &image) override;
    };
}

#endif //VO_CMPE460_NAVALGORITHM_H
