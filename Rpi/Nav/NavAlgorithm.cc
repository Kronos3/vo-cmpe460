#include "NavAlgorithm.h"
#include "Assert.hpp"
#include "opencv2/imgproc.hpp"

namespace Rpi
{
    NavAlgorithm::NavAlgorithm(CarController* car)
            : m_car(car)
    {
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

    NavSimple::NavSimple(CarController* car)
            : NavAlgorithm(car)
    {
    }

    bool NavSimple::process(CamFrame* frame, cv::Mat &image)
    {
        F32 left, right;
        EdgesFound edges = find_edges(image, left, right);

        F32 turning = 0;
        char edge_mnemonic = '?';

        switch (edges)
        {
            case NONE_FOUND:
                // Stop the car
                return false;
            case LEFT_FOUND:
                turning = 1 * (1 - left);
                edge_mnemonic = 'L';
                break;
            case RIGHT_FOUND:
                turning = -1 * (1 - right);
                edge_mnemonic = 'R';
                break;
            case BOTH_FOUND:
                edge_mnemonic = 'B';
                if (left < right)
                {
                    turning = 1 * (1 - left);
                }
                else
                {
                    turning = -1 * (1 - right);
                }
                break;
            default:
                FW_ASSERT(0 && "Invalid edge case found", edges);
        }


        F32 turning_factor = m_car->params().simple.turning * turning;

        std::ostringstream ss;
        ss << "Edges: " << edge_mnemonic << "\n"
           << "Left: " << left << "\n"
           << "Right: " << right << "\n"
           << "Turn: " << turning_factor;

        draw_information(image, ss.str());

        // Send the commands to the car
        m_car->steer(turning_factor);
        m_car->throttle(m_car->params().simple.throttle,
                        m_car->params().simple.throttle);

        return true;
    }

    NavSimple::EdgesFound NavSimple::find_edges(cv::Mat &image, F32 &left, F32 &right) const
    {
        I32 row = (I32) (m_car->params().simple.center * (F32) image.rows);

        I32 out = EdgesFound::NONE_FOUND;
        cv::Point left_p, right_p;
        U8 left_v, right_v;

        for (I32 index_from_center = 0;
             index_from_center < ((image.cols / 2) - image.cols * m_car->params().simple.cutoff);
             index_from_center++)
        {
            cv::Point left_point = cv::Point((image.cols / 2) - index_from_center, row);
            cv::Point right_point = cv::Point((image.cols / 2) + index_from_center, row);
            U8 left_pixel = image.at<U8>(left_point);
            U8 right_pixel = image.at<U8>(right_point);

            // Pixel values should be 0 or 255
            if (left_pixel > 180 && !(LEFT_FOUND & out))
            {
                out |= LEFT_FOUND;
                left = (F32) index_from_center / ((F32) image.cols / 2);
                left_p = left_point;
                left_v = left_pixel;
            }

            if (right_pixel > 180 && !(RIGHT_FOUND & out))
            {
                out |= RIGHT_FOUND;
                right = (F32) index_from_center / ((F32) image.cols / 2);
                right_p = right_point;
                right_v = right_pixel;
            }
        }

        // Draw a gray line where we are scanning
        cv::line(image, cv::Point(0, row), cv::Point(image.cols, row),
                 cv::Scalar(120, 120, 120));

        if (out & LEFT_FOUND)
        {
            cv::drawMarker(image, left_p, cv::Scalar(120, 120, 120));
            cv::putText(image, std::to_string(left_v), left_p + cv::Point(0, 10), cv::FONT_HERSHEY_PLAIN, 1,
                        cv::Scalar(255, 255, 255), 1);
        }

        if (out & RIGHT_FOUND)
        {
            cv::drawMarker(image, right_p, cv::Scalar(120, 120, 120));
            cv::putText(image, std::to_string(right_v),
                        right_p - cv::Point(0, 10), cv::FONT_HERSHEY_PLAIN, 1,
                        cv::Scalar(255, 255, 255), 1);
        }

        return static_cast<EdgesFound>(out);
    }
}
