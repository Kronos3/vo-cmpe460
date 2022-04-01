
#include "VisPipeline.h"
#include <opencv2/opencv.hpp>

namespace Rpi
{
    VisPipelineStage::VisPipelineStage()
            : m_next(nullptr)
    {
    }

    void VisPipelineStage::chain(VisPipelineStage* next)
    {
        m_next = next;
    }

    void VisPipelineStage::run(cv::Mat &image)
    {
        process(image);

        if (m_next)
        {
            m_next->run(image);
        }
    }

    VisPipelineStage::~VisPipelineStage()
    {
        delete m_next;
        m_next = nullptr;
    }

    VisPoseCalibration::VisPoseCalibration(cv::Size patternSize)
            : m_patternSize(patternSize)
    {
    }

    void VisPoseCalibration::process(cv::Mat &image)
    {
        std::vector<cv::Point2f> corners;

        // Downscaling the image will GREATLY improve the frame rate
        // findChessboardCorners has a convolution step in which the entire
        // image is scanned meaning it cannot be split into sub-tasks run by
        // multiple threads
        cv::Mat resized;
        cv::Size down_scaled_size = cv::Size(image.cols / 6, image.rows / 6);
        cv::resize(image, resized, down_scaled_size);

        bool patternFound = cv::findChessboardCorners(resized, m_patternSize, corners);

        if (patternFound)
        {
            F32 x_original_center = (F32)(resized.cols - 1) / (F32)2.0;
            F32 y_original_center = (F32)(resized.rows - 1) / (F32)2.0;

            F32 x_scaled_center = (F32)(image.cols - 1) / (F32)2.0;
            F32 y_scaled_center = (F32)(image.rows - 1) / (F32)2.0;

            // Scale the corners back up the original size frame position
            for (auto &c: corners)
            {
                c.x = (c.x - x_original_center) * 6 + x_scaled_center;
                c.y = (c.y - y_original_center) * 6 + y_scaled_center;
            }
            cv::drawChessboardCorners(image, m_patternSize, corners, patternFound);
        }
    }
}
