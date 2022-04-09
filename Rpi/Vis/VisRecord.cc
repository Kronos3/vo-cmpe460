//
// Created by tumbar on 3/31/22.
//

#include "VisRecord.h"
#include <opencv2/opencv.hpp>
#include "Logger.hpp"
#include "Assert.hpp"

namespace Rpi
{
    ChessBoardRecord::ChessBoardRecord(U32 num_frame)
    : total_frames(num_frame)
    {
    }

    bool ChessBoardRecord::done() const
    {
        return corners.size() >= total_frames;
    }

    bool CalibrationRecord::write(const char* filename) const
    {
        try
        {
            cv::FileStorage fs(filename, cv::FileStorage::WRITE);
            fs << "K" << k;
            fs << "D" << d;
            fs << "R" << r;
            fs << "T" << t;
            fs.release();

            return true;
        }
        catch (const std::exception& e)
        {
            Fw::Logger::logMsg("Failed to write DP to %s: %s",
                               (POINTER_CAST)filename,
                               (POINTER_CAST)e.what());
            return false;
        }
    }

    bool CalibrationRecord::read(const char* filename)
    {
        try
        {
            cv::FileStorage fs(filename, cv::FileStorage::READ);
            fs["K"] >> k;
            fs["D"] >> d;
            fs["R"] >> r;
            fs["T"] >> t;
            fs.release();

            return true;
        }
        catch (const std::exception& e)
        {
            Fw::Logger::logMsg("Failed to write DP to %s: %s",
                               (POINTER_CAST)filename,
                               (POINTER_CAST)e.what());

            return false;
        }
    }

    CalibrationRecord::CalibrationRecord(U32 num_frame) : ChessBoardRecord(num_frame)
    {

    }

    bool WarpRecord::write(const char* filename) const
    {
        try
        {
            cv::FileStorage fs(filename, cv::FileStorage::WRITE);
            fs << "I" << image_corners;
            fs << "SS" << square_size;
            fs << "IS" << image_size;
            fs << "P" << corner_pattern;
//            fs << "T" << transform;
            fs.release();

            return true;
        }
        catch (const std::exception& e)
        {
            Fw::Logger::logMsg("Failed to write DP to %s: %s",
                               (POINTER_CAST)filename,
                               (POINTER_CAST)e.what());
            return false;
        }
    }

    bool WarpRecord::read(const char* filename)
    {
        try
        {
            cv::FileStorage fs(filename, cv::FileStorage::READ);
            fs["I"] >> image_corners;
            fs["SS"] >> square_size;
            fs["IS"] >> image_size;
            fs["P"] >> corner_pattern;
            fs.release();

            return true;
        }
        catch (const std::exception& e)
        {
            Fw::Logger::logMsg("Failed to write DP to %s: %s",
                               (POINTER_CAST)filename,
                               (POINTER_CAST)e.what());

            return false;
        }
    }

    WarpRecord::WarpRecord(U32 num_frame)
    : ChessBoardRecord(num_frame), square_size(0.0)
    {
    }

    cv::Mat WarpRecord::get_transform(I32 downscale,
                                      const std::vector<cv::Point2f>& object_corners) const
    {
        if (!downscale)
        {
            return cv::getPerspectiveTransform(image_corners, object_corners);
        }

        cv::Point2f scaled_image[4] = {
                image_corners[0] * (1.0 / downscale),
                image_corners[1] * (1.0 / downscale),
                image_corners[2] * (1.0 / downscale),
                image_corners[3] * (1.0 / downscale),
        };

        cv::Point2f scaled_object[4] = {
                object_corners[0] * (1.0 / downscale),
                object_corners[1] * (1.0 / downscale),
                object_corners[2] * (1.0 / downscale),
                object_corners[3] * (1.0 / downscale),
        };
        return cv::getPerspectiveTransform(scaled_image, scaled_object);
    }
}
