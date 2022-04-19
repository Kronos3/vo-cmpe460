#ifndef VO_CMPE460_VISPIPELINE_H
#define VO_CMPE460_VISPIPELINE_H

#include <Fw/Types/BasicTypes.hpp>
#include <opencv2/core/mat.hpp>
#include "VisRecord.h"
#include "opencv2/imgproc.hpp"
#include <Rpi/Cam/Ports/CamFrame.h>

namespace Rpi
{
    class VisPipelineStage
    {
    public:
        VisPipelineStage();
        virtual ~VisPipelineStage();

        cv::Mat& run(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid);
        void chain(VisPipelineStage* next);

    protected:

        /**
         * Apply an in place transformation
         * @param image the image to transform
         * @param recording currently running recording
         * @return transformed image
         */
        virtual cv::Mat& process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid) = 0;

    private:
        VisPipelineStage* m_next;
    };

    class VisDownscale : public VisPipelineStage
    {
    public:
        explicit VisDownscale(I32 downscale,
                              cv::InterpolationFlags interpolation)
        : m_downscale(downscale), m_interpolation(interpolation)
        {
        }

    private:
        cv::Mat& process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid) override;

        I32 m_downscale;
        cv::InterpolationFlags m_interpolation;
        cv::Mat m_downscaled;
    };

    class VisFindChessBoard : public VisPipelineStage
    {
    public:
        explicit VisFindChessBoard(cv::Size patternSize)
        : m_patternSize(patternSize)
        {
        }

    private:
        cv::Mat& process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid) override;
        cv::Mat resized;
        cv::Size m_patternSize;
    };

    class VisPoseCalculation : public VisPipelineStage
    {
        /**
         * Uses cv::calibrateCamera to calibrate a camera matrix and distortion
         * vector as well as R/T transforms for all images.
         * Averages out R/T for all images and dumps to recording
         */
    public:
        explicit VisPoseCalculation(cv::Size patternSize);

        cv::Mat& process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid) override;
    private:
        cv::Size m_pattern_size;
        std::vector<cv::Point3f> m_object_points;
    };

    class VisWarpCalculation : public VisPipelineStage
    {
        /**
         * Warp will generate a homography transform between the imaging plane
         * and the birds eye object plane. Do make this operation simple, we assume
         * the size bottom chessboard corners will be the size we want the transformed
         * corners to be.
         */
    public:
        explicit VisWarpCalculation(cv::Size patternSize);

        cv::Mat& process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid) override;

    private:
        cv::Size m_pattern_size;
    };

    class VisGaussian : public VisPipelineStage
    {
    public:
        explicit VisGaussian(F32 sigma)
        : m_sigma(sigma) {}

    private:
        cv::Mat& process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid) override;
        F32 m_sigma;
        cv::Mat m_out;
    };


    class VisGradiant : public VisPipelineStage
    {
    public:
        explicit VisGradiant() = default;

    private:
        cv::Mat& process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid) override;

        cv::Mat m_grad_x;
        cv::Mat m_grad_y;
        cv::Mat m_out;
    };

    class VisErode : public VisPipelineStage
    {
    public:
        explicit VisErode()
        : m_erosion_kernel(cv::Mat::ones(cv::Size(3, 3), CV_8U))
        {}
    private:
        cv::Mat& process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid) override;

        cv::Mat m_out;
        cv::Mat m_erosion_kernel;
    };

    class VisOtsuThreshold : public VisPipelineStage
    {
    public:
        explicit VisOtsuThreshold(U32 ostu_frames)
        : m_ostu_running_sum(0.0), m_ostu_index(0),
          m_ostu_frames(ostu_frames)
        {
        }

    private:
        cv::Mat& process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid) override;

        // For the first OTSU_FRAMES of capture, we will average the
        // computed OTSU threshold. Then we will switch to flat binary
        // thresholding. This allows us to avoid extra work when finding
        // thresholds in different light environments.
        F64 m_ostu_running_sum;
        U32 m_ostu_index;
        U32 m_ostu_frames;

        cv::Mat m_out;
    };

    class VisWarp : public VisPipelineStage
    {
    public:
        explicit VisWarp(F32 scale_x, F32 scale_y,
                         F32 translate_x, F32 translate_y);

        bool read(const char* calibration_file);

    private:
        cv::Mat& process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid) override;

        // Transform edit parameters
        F32 m_scale_x;
        F32 m_scale_y;
        cv::Point2f m_translate;

        WarpRecord m_calibration;   //!< Calibration target
        std::vector<cv::Point2f> m_object_corners;

        cv::Size m_current_size;    //!< Size of the current transform
        cv::Mat m_transform;        //!< Warp transform to perform
        cv::Mat m_warped;           //!< Output warped image
    };

    class VisMaskQuad : public VisPipelineStage
    {
    public:
        explicit VisMaskQuad(cv::Point2f c1, cv::Point2f c2,
                             cv::Point2f c3, cv::Point2f c4,
                             U8 color)
        : m_c1(c1), m_c2(c2), m_c3(c3), m_c4(c4), m_color(color)
        {}

    private:
        cv::Mat & process(CamFrame *frame, cv::Mat &image, VisRecord *recording, bool &valid) override;

        cv::Point2f m_c1;
        cv::Point2f m_c2;
        cv::Point2f m_c3;
        cv::Point2f m_c4;
        U8 m_color;
    };
}

#endif //VO_CMPE460_VISPIPELINE_H
