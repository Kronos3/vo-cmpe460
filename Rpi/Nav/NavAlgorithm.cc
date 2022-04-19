#include "NavAlgorithm.h"
#include "Assert.hpp"
#include "opencv2/imgproc.hpp"

namespace Rpi
{
    static bool theta_initialized = false;
    static F32 cos_lut[THETA_DIVISIONS];
    static F32 sin_lut[THETA_DIVISIONS];

    static void init_theta_divisions()
    {
        if (theta_initialized) return;
        theta_initialized = true;

        for (U32 i = 0; i < THETA_DIVISIONS; i++)
        {
            F32 percent = (F32)i / (F32)THETA_DIVISIONS;
            cos_lut[i] = std::cos((F32)percent * (F32)(2 * M_PI));
            sin_lut[i] = std::sin((F32)percent * (F32)(2 * M_PI));
        }
    }

    NavAlgorithm::NavAlgorithm(CarController* car)
            : m_car(car)
    {
        init_theta_divisions();
    }

    void NavAlgorithm::draw_information(cv::Mat &image, std::string text)
    {
        // Top Left Corner
        cv::Point p1(30, 30);

        // Bottom Right Corner
        cv::Point p2(255, 255);

        int thickness = -1;

        // Drawing the Rectangle
        cv::rectangle(
                image, p1, p2,
                cv::Scalar(255, 255, 255),
                thickness, cv::LINE_8);

        std::string token;
        I32 line = 1;
        while (token != text)
        {
            token = text.substr(0, text.find_first_of('\n'));
            text = text.substr(text.find_first_of('\n') + 1);

            cv::putText(image, token, p1 + cv::Point(30, 30 * line++),
                        cv::FONT_HERSHEY_PLAIN, 1,
                        cv::Scalar(0, 255, 0), 2, false);
        }

    }

    bool NavDiagContour::process(CamFrame* frame, cv::Mat &image)
    {
        // Run Hough on edge detected image
        // Output "lines" is an array containing endpoints of detected line segments
        std::vector<cv::Vec4i> lines;
        cv::HoughLinesP(
                image, lines,
                m_car->params().contour.rho,
                m_car->params().contour.theta,
                m_car->params().contour.threshold,
                m_car->params().contour.min_line_length,
                m_car->params().contour.max_line_gap);

        // Draw the contour lines
        for (const auto &l: lines)
        {
            cv::line(image,
                     cv::Point(l[0], l[1]),
                     cv::Point(l[2], l[3]),
                     cv::Scalar(150, 150, 150),
                     3, cv::LINE_8);
        }

        return true;
    }

    EdgesFound find_edges(cv::Mat &image,
                          const cv::Point& start,
                          I32 &left,
                          I32 &right)
    {
        I32 out = EdgesFound::NONE_FOUND;
        for (I32 index_from_center = 0;
             index_from_center < image.cols;
             index_from_center++)
        {
            cv::Point left_point = cv::Point(start.x - index_from_center, start.y);
            cv::Point right_point = cv::Point(start.x + index_from_center, start.y);
            U8 left_pixel = left_point.x > 0 ? image.at<U8>(left_point) : 0;
            U8 right_pixel = right_point.x < image.cols ? image.at<U8>(right_point) : 0;

            // Pixel values should be 0 or 255
            if (left_pixel > 180 && !(LEFT_FOUND & out))
            {
                out |= LEFT_FOUND;
                left = start.x - index_from_center;
            }

            if (right_pixel > 180 && !(RIGHT_FOUND & out))
            {
                out |= RIGHT_FOUND;
                right = start.x + index_from_center;
            }

            // Exit early for performance
            if (out == BOTH_FOUND)
                break;
        }

        return static_cast<EdgesFound>(out);
    }

    bool trace_line(cv::Mat& image,
                    const cv::Point& start,
                    cv::Point& next,
                    F32 norm_mag,
                    I32 &center_angle)
    {
        // Sweep 30 radial steps in each direction
        cv::Point left;
        cv::Point right;
        bool left_inbound = true;
        bool right_inbound = true;

        for (I32 i = 0; i < (THETA_DIVISIONS / 2); i++)
        {
            I32 left_angle = (center_angle - i) % THETA_DIVISIONS;
            left = start -
                    cv::Point(
                            (I32)(norm_mag * cos_lut[left_angle]),
                            (I32)(norm_mag * sin_lut[left_angle])
            );

            left_inbound = left_inbound && (
                    left.x >= 0 && left.x < image.cols &&
                    left.y >= 0 && left.y < image.rows
                                           );
            if (left_inbound && image.at<U8>(left) > 128)
            {
                // Found a solution
                next = left;
                center_angle = center_angle - i;
                return true;
            }

            I32 right_angle = (center_angle + i) % THETA_DIVISIONS;
            right = start - cv::Point(
                    (I32)(norm_mag * cos_lut[right_angle]),
                    (I32)(norm_mag * sin_lut[right_angle])
            );

            right_inbound = right_inbound && (
                    right.x >= 0 && right.x < image.cols &&
                            right.y >= 0 && right.y < image.rows
            );

            if (right_inbound && image.at<U8>(right) > 128)
            {
                // Found a solution
                next = right;
                center_angle = center_angle + i;
                return true;
            }

            if (!right_inbound && !left_inbound)
            {
                // We can't recover from this
                break;
            }
        }

        cv::line(image, start, left, cv::Scalar(150, 150, 150));
        cv::line(image, start, right, cv::Scalar(150, 150, 150));
        return false;
    }

    void trace_center(cv::Mat& image,
                      cv::Point center_point,
                      U32 step,
                      std::vector<cv::Point>& center,
                      std::vector<cv::Point>& left_points,
                      std::vector<cv::Point>& right_points)
    {
        center.clear();
        left_points.clear();
        right_points.clear();

        cv::Point gradient(1, 0);
        for (U32 iter = 0; iter < 50; iter++)
        {
            bool left_inbound = true, right_inbound = true;
            cv::Point left = center_point, right = center_point;
            for (I32 i = 0; left_inbound || right_inbound; i++)
            {
                left -= gradient;
                right += gradient;

                left_inbound = left_inbound && (
                        left.x >= 0 && left.x < image.cols &&
                        left.y >= 0 && left.y < image.rows
                );

                right_inbound = right_inbound && (
                        right.x >= 0 && right.x < image.cols &&
                        right.y >= 0 && right.y < image.rows
                );

                if (left_inbound && image.at<U8>(left) > 100)
                {
                    left_points.push_back(left);
                    left_inbound = false;
                }
            }
        }
    }
}
