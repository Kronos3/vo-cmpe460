#ifndef VO_CMPE460_VISPIPELINE_H
#define VO_CMPE460_VISPIPELINE_H

#include <Fw/Types/BasicTypes.hpp>
#include <opencv2/core/mat.hpp>

namespace Rpi
{
    class VisPipelineStage
    {
    public:
        VisPipelineStage();
        virtual ~VisPipelineStage();

        void run(cv::Mat& image);
        void chain(VisPipelineStage* next);

    protected:

        /**
         * Apply an in place transformation
         * @param image the image to transform
         */
        virtual void process(cv::Mat& image) = 0;

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

    class VisPoseCalibration : public VisPipelineStage
    {
    public:
        explicit VisPoseCalibration(cv::Size patternSize);

    private:
        void process(cv::Mat &image) override;
        cv::Size m_patternSize;
    };
}

#endif //VO_CMPE460_VISPIPELINE_H
