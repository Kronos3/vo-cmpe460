#ifndef VO_CMPE460_VISPIPELINE_H
#define VO_CMPE460_VISPIPELINE_H

#include <Fw/Types/BasicTypes.hpp>
#include <opencv2/core/mat.hpp>
#include "VisRecord.h"
#include <Rpi/Cam/Ports/CamFrame.h>

namespace Rpi
{
    class VisPipelineStage
    {
    public:
        VisPipelineStage();
        virtual ~VisPipelineStage();

        bool run(CamFrame* frame, cv::Mat& image, VisRecord* recording);
        void chain(VisPipelineStage* next);

    protected:

        /**
         * Apply an in place transformation
         * @param image the image to transform
         * @param recording currently running recording
         */
        virtual bool process(CamFrame* frame, cv::Mat& image, VisRecord* recording) = 0;

    private:
        VisPipelineStage* m_next;
    };

#if 0
    class VisGrayscale : public VisPipelineStage
    {
    private:
        void process(cv::Mat &image) override;
    };
#endif

    class VisFindChessBoard : public VisPipelineStage
    {
    public:
        explicit VisFindChessBoard(cv::Size patternSize);

    private:
        cv::Mat resized;
        bool process(CamFrame* frame, cv::Mat &image, VisRecord* recording) override;
        cv::Size m_patternSize;
    };

    class VisPoseCalculation : public VisPipelineStage
    {
    public:
        explicit VisPoseCalculation(cv::Size patternSize);

        bool process(CamFrame* frame, cv::Mat &image, VisRecord *recording) override;
    private:
        cv::Size m_pattern_size;
        std::vector<cv::Point3f> m_object_points;
    };

    class VisGradiant : public VisPipelineStage
    {
    public:
        explicit VisGradiant();

        bool process(CamFrame* frame, cv::Mat &image, VisRecord *recording) override;

    private:
        cv::Mat grad_x, grad_y;
        cv::Mat smaller;
    };
}

#endif //VO_CMPE460_VISPIPELINE_H
