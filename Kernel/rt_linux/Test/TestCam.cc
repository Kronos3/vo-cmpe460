#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        std::cout << "usage: " << argv[0] << " outfile.jpg";
        return 1;
    }

    cv::Mat mat;

    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_VERBOSE);

    cv::VideoCapture cap(0);

    cap >> mat;
    cv::imwrite(argv[1], mat);
}
