#include "VisPipeline.h"
#include "VoCarCfg.h"
#include "CamFrame.h"
#include <opencv2/opencv.hpp>
#include <Fw/Logger/Logger.hpp>

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

    bool VisPipelineStage::run(CamFrame* frame, cv::Mat &image, VisRecord* recording)
    {
        bool status = process(frame, image, recording);
        if (!status)
        {
            // The pipeline failed
            return false;
        }

        if (m_next)
        {
            return m_next->run(frame, image, recording);
        }

        return true;
    }

    VisPipelineStage::~VisPipelineStage()
    {
        delete m_next;
        m_next = nullptr;
    }

    VisFindChessBoard::VisFindChessBoard(cv::Size patternSize)
            : m_patternSize(patternSize)
    {
    }

    bool VisFindChessBoard::process(CamFrame* frame, cv::Mat &image, VisRecord* recording)
    {
        std::vector<cv::Point2f> corners;

        // Downscaling the image will GREATLY improve the frame rate
        // findChessboardCorners has a convolution step in which the entire
        // image is scanned meaning it cannot be split into sub-tasks run by
        // multiple threads
        cv::Size down_scaled_size = cv::Size(image.cols / VIS_CHESSBOARD_DOWNSCALE,
                                             image.rows / VIS_CHESSBOARD_DOWNSCALE);
        cv::resize(image, resized, down_scaled_size);

        bool patternFound = cv::findChessboardCorners(resized, m_patternSize, corners,
                                                      cv::CALIB_CB_ADAPTIVE_THRESH +
                                                      cv::CALIB_CB_NORMALIZE_IMAGE
                                                      + cv::CALIB_CB_FAST_CHECK);
        if (!patternFound)
        {
            return false;
        }

        F32 x_original_center = (F32) (resized.cols - 1) / (F32) 2.0;
        F32 y_original_center = (F32) (resized.rows - 1) / (F32) 2.0;

        F32 x_scaled_center = (F32) (image.cols - 1) / (F32) 2.0;
        F32 y_scaled_center = (F32) (image.rows - 1) / (F32) 2.0;

        // Scale the corners back up the original size frame position
        for (auto &c: corners)
        {
            c.x = (c.x - x_original_center) * VIS_CHESSBOARD_DOWNSCALE + x_scaled_center;
            c.y = (c.y - y_original_center) * VIS_CHESSBOARD_DOWNSCALE + y_scaled_center;
        }

        // Run sub-pixel refinement on the corners found previously
        // This needs to be down on the original image to get as much quality as possible
        // This algorithm operates on the local areas around the rough point found findChessboardCorners()
        cornerSubPix(image, corners, m_patternSize, cv::Size(-1, -1),
                     cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.1));

        cv::drawChessboardCorners(image, m_patternSize, corners, patternFound);



        // Label offset from the corner point
        cv::Point2f c(0.1, 0.1);

        // Label the object coordinates on each frame
        auto iter = corners.begin();
        for (I32 i = 0; i < m_patternSize.height; i++)
        {
            for (I32 j = 0; j < m_patternSize.width; j++)
            {
                std::ostringstream ss;
                cv::Point3f object_point;
                object_point.x = ((F32) m_patternSize.width / (F32) 2.0) - (F32)j - (F32)0.5;
                object_point.y = ((F32) m_patternSize.height) - (F32) (i + 1);
                object_point.z = 0.0;  // Calibration board is sitting on the floor
                ss << "(" << object_point.x << "," << object_point.y << ")";
                cv::putText(image, ss.str(), *iter + c, cv::FONT_HERSHEY_PLAIN, 1, CV_RGB(0, 255, 0), 2);
                iter++;
            }
        }

        if (recording)
        {
            auto* record = dynamic_cast<CalibrationRecord*>(recording);
            if (record == nullptr)
            {
                return false;
            }

            record->corners.push_back(std::move(corners));
        }

        return true;
    }

    VisPoseCalculation::VisPoseCalculation(cv::Size patternSize)
            : m_pattern_size(patternSize)
    {
        // Object points vector needs to ordered with the top left pointer coming first
        // Points are row major

        for (I32 i = 0; i < patternSize.height; i++)
        {
            for (I32 j = 0; j < patternSize.width; j++)
            {
                cv::Point3f p;
                p.x = ((F32) patternSize.width / (F32) 2.0) - (F32)j - (F32)0.5;
                p.y = ((F32) patternSize.height) - (F32) (i + 1);
                p.z = 0.0;  // Calibration board is sitting on the floor
                m_object_points.push_back(p);
            }
        }
    }

    bool VisPoseCalculation::process(CamFrame* frame, cv::Mat &image, VisRecord* recording)
    {
        if (!recording)
        {
            // This stage does nothing is we don't get points from
            // the FindChessBoard pipe
            return false;
        }

        auto* record = dynamic_cast<CalibrationRecord*>(recording);
        if (record == nullptr)
        {
            // The recording being passed through the pipeline not correct
            // We can't continue
            return false;
        }

        // Camera calibration will be calculated on the final frame
        if (record->done())
        {
            // All the object points are the same
            // The calibration board should stay motionless during calibration
            std::vector<std::vector<cv::Point3f>> objpoints;
            for (U32 i = 0; i < record->corners.size(); i++)
            {
                objpoints.push_back(m_object_points);
            }

            // Calibrate the camera calibration into the record
            // We don't want to dump the r/t transform matrices directly
            // into the record yet. This is because this is actually a vector
            // of cv::Mat where each item corresponds to the pose transform for every
            // frame. The average of these should be passed to the record
            // The intrinsic and distortion matrices are over all the frames
            std::vector<cv::Mat> r_all;
            std::vector<cv::Mat> t_all;

            I32 flag = 0;
            flag |= cv::CALIB_FIX_K4;
            flag |= cv::CALIB_FIX_K5;

            cv::calibrateCamera(objpoints,
                                record->corners,
                                m_pattern_size,
                                record->k,
                                record->d,
                                r_all,
                                t_all,
                                flag);

            // Find the average of these transforms
            record->r = cv::Mat::zeros(r_all.at(0).rows,
                                       r_all.at(0).cols,
                                       r_all.at(0).type());
            for (const auto &r_i: r_all)
            {
                record->r += r_i;
            }
            record->r = (1.0 / (F64) r_all.size()) * record->r;

            record->t = cv::Mat::zeros(t_all.at(0).rows,
                                       t_all.at(0).cols,
                                       t_all.at(0).type());
            for (const auto &t_i: t_all)
            {
                record->t += t_i;
            }
            record->t = (1.0 / (F64) t_all.size()) * record->t;
        }

        return true;
    }

    VisGradiant::VisGradiant(U32 ostu_frames, F32 sigma)
    : m_ostu_running_sum(0.0), m_ostu_index(0),
    m_ostu_frames(ostu_frames),
    m_sigma(sigma)
    {
    }

    bool VisGradiant::process(CamFrame* frame, cv::Mat &image, VisRecord* recording)
    {
        U8 value = 128;
        U32 num = (frame->info.stride * frame->info.height) / 2;
        I32 scale = 1;
        I32 delta = 0;
        I32 ddepth = CV_16S;

        memset(frame->span.data() + frame->info.stride * frame->info.height, value, num);

        // Downscale the image so that we actually get good performance
        // We upscale the results later
        cv::resize(image, m_smaller,
                   cv::Size(image.cols / VIS_RACE_DOWNSCALE,
                            image.rows / VIS_RACE_DOWNSCALE),
                   0, 0, cv::INTER_NEAREST // faster than lerping
                   );

        // Remove noise by blurring with a Gaussian filter ( kernel size = 3 )
        cv::GaussianBlur(m_smaller, m_smaller, cv::Size(3, 3), m_sigma, 0, cv::BORDER_DEFAULT);

        // Find the dx and dy gradients by applying simple Sobel filters
        // This is just a convolution of gradient kernels.
        cv::Sobel(m_smaller, m_grad_x, ddepth, 1, 0, 1, scale, delta, cv::BORDER_DEFAULT);
        cv::Sobel(m_smaller, m_grad_y, ddepth, 0, 1, 1, scale, delta, cv::BORDER_DEFAULT);

        // We don't care about negative gradients
        cv::convertScaleAbs(m_grad_x, m_grad_x);
        cv::convertScaleAbs(m_grad_y, m_grad_y);

        // Todo(tumbar) cv::add
        // Combine dx and dy into a single frame
        // We can reuse the m_smaller buffer
        cv::addWeighted(m_grad_x, 1, m_grad_y, 1, 0, m_smaller);

        cv::Mat erosion_kernel = cv::Mat::ones(cv::Size(3, 3), CV_8U);
        cv::morphologyEx(m_smaller, m_smaller, cv::MORPH_OPEN, erosion_kernel);

        // Now that we have the gradient image calculated, we need to threshold
        // these values to filter out the small deltas in color. To avoid needing
        // to mess with a thresholding value that depends on environment etc. We
        // can expose this pipeline to image and use the Otsu thresholding algorithm
        // to find the optimal threshold between foreground and background.
        if (m_ostu_index < m_ostu_frames)
        {
            // We only wants to perform Otsu thresholding for a certain number of frames
            // We can average out the running sum at the end
            m_ostu_running_sum += cv::threshold(m_smaller, m_smaller, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
            m_ostu_index++;
        }
        else
        {
            if (m_ostu_index == m_ostu_frames)
            {
                // This is the first frame after Otsu thresholding running average has finished
                // We can calculate the average threshold now and use it for the rest of the life
                // of the pipeline.
                m_ostu_running_sum /= m_ostu_frames;
                m_ostu_index++;
            }

            // Standard binary thresholding is faster than Otsu
            cv::threshold(m_smaller, m_smaller, m_ostu_running_sum, 255, cv::THRESH_BINARY);
        }

        // Update the result image back to the output
        // The rest of the pipeline and the output does not handle a size change
        // We can use INTER_NEAREST to get sharp edges on the up-scaled image.
        // Linear interpolate hear would get rid of the thresholding work we already did
        cv::resize(m_smaller,
                   image,
                   cv::Size(image.cols, image.rows),
                   0, 0, cv::INTER_NEAREST);

        return true;
    }
}
