
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

    void VisPipelineStage::run(cv::Mat& image)
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
        bool patternFound = cv::findChessboardCorners(image, m_patternSize, corners);
        cv::drawChessboardCorners(image, m_patternSize, corners, patternFound);
    }
}
