#ifndef VO_CMPE460_VISPIPELINE_H
#define VO_CMPE460_VISPIPELINE_H

#include <Fw/Types/BasicTypes.hpp>
#include <opencv2/core/mat.hpp>
#include "VisRecord.h"

namespace Rpi
{
    class VisPipelineStage
    {
    public:
        VisPipelineStage();
        virtual ~VisPipelineStage();

        bool run(cv::Mat& image, VisRecord* recording);
        void chain(VisPipelineStage* next);

    protected:

        /**
         * Apply an in place transformation
         * @param image the image to transform
         * @param recording currently running recording
         */
        virtual bool process(cv::Mat& image, VisRecord* recording) = 0;

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
        bool process(cv::Mat &image, VisRecord* recording) override;
        cv::Size m_patternSize;
    };

    class VisPoseCalculation : public VisPipelineStage
    {
    public:
        explicit VisPoseCalculation(cv::Size patternSize);

        bool process(cv::Mat &image, VisRecord *recording) override;
    private:
        cv::Size m_pattern_size;
        std::vector<cv::Point3f> m_object_points;
    };
}

#endif //VO_CMPE460_VISPIPELINE_H
