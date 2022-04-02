
#include "VisPipeline.h"
#include "Assert.hpp"
#include "VoCarCfg.h"
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

    bool VisPipelineStage::run(cv::Mat &image, VisRecord* recording)
    {
        bool status = process(image, recording);
        if (!status)
        {
            // The pipeline failed
            return false;
        }

        if (m_next)
        {
            return m_next->run(image, recording);
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

    bool VisFindChessBoard::process(cv::Mat &image, VisRecord* recording)
    {
        std::vector<cv::Point2f> corners;

        // Downscaling the image will GREATLY improve the frame rate
        // findChessboardCorners has a convolution step in which the entire
        // image is scanned meaning it cannot be split into sub-tasks run by
        // multiple threads
        cv::Mat resized;
        cv::Size down_scaled_size = cv::Size(image.cols / VIS_CHESSBOARD_DOWNSCALE, image.rows / VIS_CHESSBOARD_DOWNSCALE);
        cv::resize(image, resized, down_scaled_size);

        bool patternFound = cv::findChessboardCorners(resized, m_patternSize, corners);
        if (!patternFound)
        {
            return false;
        }

        F32 x_original_center = (F32)(resized.cols - 1) / (F32)2.0;
        F32 y_original_center = (F32)(resized.rows - 1) / (F32)2.0;

        F32 x_scaled_center = (F32)(image.cols - 1) / (F32)2.0;
        F32 y_scaled_center = (F32)(image.rows - 1) / (F32)2.0;

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
                p.x = ((F32)patternSize.width / (F32)2.0) - (F32)j;
                p.y = ((F32)patternSize.height / (F32)2.0) - (F32)i;
                p.z = 0.0;  // Calibration board is sitting on the floor
                m_object_points.push_back(p);
            }
        }
    }

    bool VisPoseCalculation::process(cv::Mat &image, VisRecord* recording)
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

        // Label offset from the corner point
        cv::Point2f c(0.1, 0.1);

        // Place labels on the corners from the most recent frame
        for (const auto& p : record->corners.back())
        {
            std::ostringstream ss;
            ss << "("
               << p.x
               << ","
               << p.y
               << ")";

            cv::putText(image, ss.str(), p + c, cv::FONT_HERSHEY_PLAIN, 1, CV_RGB(0, 255, 0), 3);
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
            for (const auto& r_i : r_all)
            {
                record->r += r_i;
            }
            record->r = (1.0 / (F64)r_all.size()) * record->r;

            record->t = cv::Mat::zeros(t_all.at(0).rows,
                                       t_all.at(0).cols,
                                       t_all.at(0).type());
            for (const auto& t_i : t_all)
            {
                record->t += t_i;
            }
            record->t = (1.0 / (F64)t_all.size()) * record->t;
        }

        return true;
    }

    VisUndistort::VisUndistort()
    : calibration(0)
    {
    }

    bool VisUndistort::read(const char* calibration_file)
    {
        bool read_status = calibration.read(calibration_file);
        if (!read_status) return false;

        cv::Size s(CAMERA_RAW_WIDTH, CAMERA_RAW_HEIGHT);
        new_k = cv::getOptimalNewCameraMatrix(
                calibration.k, calibration.d,
                s, 1, s, &roi);

        cv::Mat r;
        cv::initUndistortRectifyMap(calibration.k, calibration.d,
                                    r, new_k, s, CV_32FC1,
                                    map1, map2);

        return true;
    }

    bool VisUndistort::process(cv::Mat &image, VisRecord* recording)
    {
        // TODO(tumbar) Do we pull this out the constructor?

//        cv::remap(image, image, map1, map2, cv::INTER_NEAREST);
        cv::Laplacian(image, undistorted, CV_32F);
        std::memcpy(image.data, undistorted.data, image.total());

        return true;
    }
}
