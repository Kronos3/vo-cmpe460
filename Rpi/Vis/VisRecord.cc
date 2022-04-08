//
// Created by tumbar on 3/31/22.
//

#include "VisRecord.h"
#include "opencv2/core/persistence.hpp"
#include "Logger.hpp"

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

    bool WarpRecord::write(const char* filename) const
    {
        try
        {
            cv::FileStorage fs(filename, cv::FileStorage::WRITE);
            fs << "I" << image_corners;
            fs << "O" << object_corners;
            fs << "T" << transform;
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
            fs["O"] >> object_corners;
            fs["T"] >> transform;
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
}
