#include "NavAlgorithm.h"
#include "opencv2/imgproc.hpp"
#include "VoCarCfg.h"
#include "Assert.hpp"

// Pixels per time
#define pix_per_time (50.0)
#define dt (0.1)

namespace Rpi
{
    NavPid::NavPid(CarController* car)
    : NavAlgorithm(car),
      m_history_i(0),
      m_history{0}
    {}

    F64 NavPid::pid(F64 e_t)
    {
        // Push the new value into the history
        m_history[m_history_i++] = e_t;
        m_history_i %= NAV_PID_HISTORY_N;

        F64 integral = 0.0;
        for (const auto& i : m_history)
        {
            integral += i;
        }

        // dx = e(t) - e(t-1)
        F64 derivative =
                m_history[(m_history_i - 1) % NAV_PID_HISTORY_N]
                - m_history[(m_history_i - 2) % NAV_PID_HISTORY_N];

        F64 u =
                m_car->params().pid.p * e_t
                + m_car->params().pid.i * integral
                + m_car->params().pid.d * derivative;

        return u;
    }

    NavComplexPid::NavComplexPid(CarController* car)
    : NavPid(car)
    {
    }

    bool NavComplexPid::process(CamFrame* frame, cv::Mat &image)
    {
        // Clear the spline from the last run
        sx.clear();
        sy.clear();

        // Draw the spline for this navigation algorithm
        draw_spline(image);

        FW_ASSERT(sx.size() == sy.size(), sx.size(), sy.size());

        // Set up the time vector
        std::vector<F64> t;
        for (U32 i = 0; i < sx.size(); i++)
            t.push_back(i);

        // Set up the parametric splines
        tk::spline x(t, sx);
        tk::spline y(t, sy);

        // Calculate the turning from the current position.
        // Current position will look at steering time ahead of current
        // trajectory. Trajectory is static relative to the processed
        // camera from as it has been calibrated to the object frame

        // X: Middle of the camera
        // Y: Steering time away from the bottom
        cv::Point2f actual_position(
                (F32)image.cols / (F32)2.0,
                (F32)image.rows - ((F32)pix_per_time * m_car->params().pid.steering_t)
        );

        // Desired will be along the spline
        cv::Point2f desired_position(
                (F32)x(m_car->params().pid.steering_t),
                (F32)y(m_car->params().pid.steering_t)
        );

        // Find the error
        F64 error = cv::norm(desired_position - actual_position);

        // Calculate the steering via a PID controller
        F64 steering = pid(error);

        // Calculate the throttle response based on the second
        // derivative of the parametric spline
        //    d^2(y)
        //    ------  = d/dt ( (dy/dt) / (dx/dt) ) / (dx/dt)
        //     dx^2

        F64 t0 = m_car->params().pid.throttle_t;
        F64 t1 = t0 - dt;
        F64 dx_dt_0 = x.deriv(1, t0);
        F64 dy_dt_0 = y.deriv(1, t0);
        F64 dx_dt_1 = x.deriv(1, t1);
        F64 dy_dt_1 = y.deriv(1, t1);

        F64 d2y_dx2 = (((dy_dt_0 / dx_dt_0) - (dy_dt_1 / dx_dt_1)) / dt) / dx_dt_0;
        F64 throttle = 1 - m_car->params().pid.t * d2y_dx2;

        m_car->steer((F32)steering);
        m_car->throttle((F32)throttle, (F32) throttle);

        cv::drawMarker(image, actual_position,
                       cv::Scalar(150, 150, 150),
                       cv::MARKER_CROSS, 20, 3);
        cv::drawMarker(image, desired_position,
                       cv::Scalar(150, 150, 150),
                       cv::MARKER_DIAMOND, 20, 3);

        std::ostringstream os;
        os << "Steering: " << steering << "\n"
           << "d2y_dx2: " << d2y_dx2 << "\n"
           << "Throttle: " << throttle << "\n"
                ;

        draw_information(image, os.str());

        return true;
    }


    void NavComplexPid::draw_spline(cv::Mat &image)
    {
    }

    void draw_center_spline(
            cv::Mat& image,
            std::vector<F64>& sx,
            std::vector<F64>& sy)
    {
    }
}