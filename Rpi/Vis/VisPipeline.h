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
        /**
         * Uses cv::calibrateCamera to calibrate a camera matrix and distortion
         * vector as well as R/T transforms for all images.
         * Averages out R/T for all images and dumps to recording
         */
    public:
        explicit VisPoseCalculation(cv::Size patternSize);

        bool process(CamFrame* frame, cv::Mat &image, VisRecord* recording) override;
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

        bool process(CamFrame *frame, cv::Mat &image, VisRecord *recording) override;

    private:
        cv::Size m_pattern_size;
        std::vector<cv::Point3f> m_object_points;
    };

    class VisGradiant : public VisPipelineStage
    {
    public:
        explicit VisGradiant(U32 ostu_frames, F32 sigma);

        bool process(CamFrame* frame, cv::Mat &image, VisRecord *recording) override;

    private:
        // For the first OTSU_FRAMES of capture, we will average the
        // computed OTSU threshold. Then we will switch to flat binary
        // thresholding. This allows us to avoid extra work when finding
        // thresholds in different light environments.
        F64 m_ostu_running_sum;
        U32 m_ostu_index;
        U32 m_ostu_frames;

        F32 m_sigma;
        cv::Mat m_grad_x;
        cv::Mat m_grad_y;
        cv::Mat m_smaller;
    };

    class VisMapping : public VisPipelineStage
    {
    public:
        explicit VisMapping(const char* calibration_file,
                            F32 threshold,
                            U32 x_cells,
                            U32 y_cells);

        bool is_valid() const;

        bool process(CamFrame *frame, cv::Mat &image, VisRecord *recording) override;

    private:
        bool valid;
        CalibrationRecord m_calibration;    //!< Camera calibration with pose information

        F32 m_threshold;                    //!< Threshold to apply to Sobel filter
        U32 m_x_cells;                      //!< Number of cells to divide columns into
        U32 m_y_cells;                      //!< Number of cells to divide rows into
    };
}

#endif //VO_CMPE460_VISPIPELINE_H
