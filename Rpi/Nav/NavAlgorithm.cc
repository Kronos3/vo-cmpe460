#include "NavAlgorithm.h"
#include "Assert.hpp"
#include "opencv2/imgproc.hpp"
#include "VoCarCfg.h"

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
}
