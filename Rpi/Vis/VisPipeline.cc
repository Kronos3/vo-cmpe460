
#include "VisPipeline.h"
#include "Assert.hpp"
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

    void VisPipelineStage::run(cv::Mat &image, VisRecord* recording)
    {
        process(image, recording);

        if (m_next)
        {
            m_next->run(image, recording);
        }
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

    void VisFindChessBoard::process(cv::Mat &image, VisRecord* recording)
    {
        std::vector<cv::Point2f> corners;

        // Downscaling the image will GREATLY improve the frame rate
        // findChessboardCorners has a convolution step in which the entire
        // image is scanned meaning it cannot be split into sub-tasks run by
        // multiple threads
        cv::Mat resized;
        cv::Size down_scaled_size = cv::Size(image.cols / 6, image.rows / 6);
        cv::resize(image, resized, down_scaled_size);

        bool patternFound = cv::findChessboardCorners(resized, m_patternSize, corners);

        if (patternFound)
        {
            F32 x_original_center = (F32)(resized.cols - 1) / (F32)2.0;
            F32 y_original_center = (F32)(resized.rows - 1) / (F32)2.0;

            F32 x_scaled_center = (F32)(image.cols - 1) / (F32)2.0;
            F32 y_scaled_center = (F32)(image.rows - 1) / (F32)2.0;

            // Scale the corners back up the original size frame position
            for (auto &c: corners)
            {
                c.x = (c.x - x_original_center) * 6 + x_scaled_center;
                c.y = (c.y - y_original_center) * 6 + y_scaled_center;
            }

            // TODO(tumbar) Subpixel refinement of corners
            //    Much faster than the chessboard algorithm because its localized per point
            //    Refinement should be done on the full scale image for the best accuracy

            cv::drawChessboardCorners(image, m_patternSize, corners, patternFound);

            if (recording)
            {
                // Serialize the data into an opaque array
                std::unique_ptr<F32[]> f_ptr = std::unique_ptr<F32[]>(new F32[2 * corners.size()]);

                // Encode the points
                for (U32 i = 0; i < corners.size(); i++)
                {
                    f_ptr[i * 2] = corners.at(i).x;
                    f_ptr[i * 2 + 1] = corners.at(i).y;
                }

                // Place the corners into the recording
                recording->append(recording->get_current(),
                                  VisRecord::CHESSBOARD_CORNERS,
                                  std::move(f_ptr),
                                  2 * corners.size());
            }
        }
        else
        {
            if (recording)
            {
                // Make sure the frame doesn't get incremented
                // This is not considered a valid frame
                recording->retry();
            }
        }
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

    void VisPoseCalculation::process(cv::Mat &image, VisRecord* recording)
    {
        if (!recording)
        {
            // This stage does nothing is we don't get points from
            // the FindChessBoard pipe
            return;
        }

        if (recording->frame_failed())
        {
            // This is garbage frame
            // We couldn't find chessboard points on this frame
            return;
        }

        U32 length;
        F32* corners_raw = recording->get(recording->get_current(),
                                          VisRecord::CHESSBOARD_CORNERS,
                                          length);

        if (corners_raw == nullptr)
        {
            // We couldn't find the entry in the recording?
            // This means VisFindChessBoard somewhere before this
            // pipe. This is garbage.
            return;
        }

        // Every point has two entries
        FW_ASSERT(length > 0 && length % 2 == 0, length);

        // Label offset from the corner point
        cv::Point2f c(0.1, 0.1);

        // Load the points from the previous pipe
        std::vector<cv::Point2f> corners;
        for (U32 i = 0; i < length / 2; i++)
        {
            cv::Point2f p;
            p.x = corners_raw[i * 2];
            p.y = corners_raw[i * 2 + 1];
            corners.push_back(p);

            std::ostringstream ss;
            ss << "("
               << m_object_points[i].x
               << ","
               << m_object_points[i].y
               << ")";

            cv::putText(image, ss.str(), p + c, cv::FONT_HERSHEY_PLAIN, 3, CV_RGB(0, 255, 0), 3);
        }

        m_image_points.emplace_back(std::move(corners));

        // Camera calibration will be calculated on the final frame
        if (recording->get_current() + 1 == recording->get_total())
        {
            // All the object points are the same
            // The calibration board should stay motionless during calibration
            std::vector<std::vector<cv::Point3f>> objpoints;
            for (U32 i = 0; i < m_image_points.size(); i++)
                objpoints.push_back(m_object_points);

            cv::Mat K;              //!< Camera matrix
            cv::Mat D;              //!< Distortion matrix
            std::vector<F32> R;     //!< Rotation vector
            std::vector<F32> T;     //!< Translation vector
            cv::calibrateCamera(m_object_points, m_image_points,
                                m_pattern_size,
                                K, D, R, T);

            // Make sure our data makes sense

        }
    }
}
