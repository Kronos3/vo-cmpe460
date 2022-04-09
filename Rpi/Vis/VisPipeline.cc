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

    cv::Mat& VisPipelineStage::run(CamFrame* frame, cv::Mat &image, VisRecord* recording, bool &valid)
    {
        valid = true;
        cv::Mat& transformed = process(frame, image, recording, valid);
        if (!valid || !m_next)
        {
            // The pipeline failed or is the last in the chain
            return transformed;
        }

        return m_next->run(frame, transformed, recording, valid);
    }

    VisPipelineStage::~VisPipelineStage()
    {
        delete m_next;
        m_next = nullptr;
    }

    cv::Mat& VisDownscale::process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid)
    {
        cv::resize(image, m_downscaled,
                   cv::Size(image.cols / m_downscale,
                            image.rows / m_downscale),
                   0, 0, m_interpolation);
        return m_downscaled;
    }

    cv::Mat& VisFindChessBoard::process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid)
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
            valid = false;
            return image;
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
            auto* record = dynamic_cast<ChessBoardRecord*>(recording);
            if (record == nullptr)
            {
                valid = false;
                return image;
            }

            record->corners.push_back(std::move(corners));
        }

        return image;
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

    cv::Mat& VisPoseCalculation::process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid)
    {
        auto* record = dynamic_cast<CalibrationRecord*>(recording);
        if (record == nullptr)
        {
            // The recording being passed through the pipeline not correct
            // We can't continue
            valid = false;
            return image;
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

        return image;
    }

    VisWarpCalculation::VisWarpCalculation(cv::Size patternSize)
    : m_pattern_size(patternSize)
    {}

    cv::Mat& VisWarpCalculation::process(CamFrame* frame, cv::Mat &image, VisRecord* recording, bool& valid)
    {
        if (recording)
        {
            auto* record = dynamic_cast<WarpRecord*>(recording);
            if (record == nullptr)
            {
                // We are using the wrong record
                // Just kill the pipeline
                valid = false;
                return image;
            }

            if (!record->done())
            {
                // Wait until the recording is ready
                return image;
            }

            // We took N images and found the chessboard corners
            // There is a little noise in this. We can assume that
            // the calibration target did not move during calibration.
            // We can therefore average the corner positions
            std::vector<cv::Point2f> average_corners;
            for (U32 i = 0; i < record->corners.at(0).size(); i++)
            {
                // Start with zeroes
                cv::Point2f current = cv::Point2f(0, 0);

                // Add up all the corner points from all the images
                for (U32 j = 0; j < record->total_frames; j++)
                {
                    current += record->corners.at(j).at(i);
                }

                // Find the average position
                current.x /= (F32)record->total_frames;
                current.y /= (F32)record->total_frames;

                average_corners.push_back(current);
            }

            record->corner_pattern = m_pattern_size;
            record->image_size = cv::Size(image.cols, image.rows);

            for (U32 i = 0; i < 4; i++)
            {
                record->image_corners.emplace_back(0, 0);
            }

            // Copy the four corners into place
            record->image_corners[0] = average_corners[0];
            record->image_corners[1] = average_corners[m_pattern_size.width - 1];
            record->image_corners[2] = average_corners[(m_pattern_size.height - 1) * (m_pattern_size.width)];
            record->image_corners[3] = average_corners[(m_pattern_size.height - 1) * (m_pattern_size.width) +
                                                       (m_pattern_size.width - 1)];

            // Each square is of equal size
            record->square_size = (record->image_corners[3].x - record->image_corners[2].x) / (F32)(m_pattern_size.width - 1);
        }

        return image;
    }

    cv::Mat& VisGaussian::process(CamFrame* frame, cv::Mat &image, VisRecord* recording, bool& valid)
    {
        // Remove noise by blurring with a Gaussian filter ( kernel size = 3 )
        cv::GaussianBlur(image, m_out, cv::Size(3, 3), m_sigma, 0, cv::BORDER_DEFAULT);
        return m_out;
    }

    cv::Mat& VisGradiant::process(CamFrame* frame, cv::Mat &image, VisRecord* recording, bool& valid)
    {
        // Find the dx and dy gradients by applying simple Sobel filters
        // This is just a convolution of gradient kernels.
        I32 delta = 0;
        I32 ddepth = CV_16S;
        I32 scale = 1;
        cv::Sobel(image, m_grad_x, ddepth, 1, 0, 1, scale, delta, cv::BORDER_DEFAULT);
        cv::Sobel(image, m_grad_y, ddepth, 0, 1, 1, scale, delta, cv::BORDER_DEFAULT);

        // We don't care about negative gradients
        cv::convertScaleAbs(m_grad_x, m_grad_x);
        cv::convertScaleAbs(m_grad_y, m_grad_y);

        // Todo(tumbar) cv::add
        // Combine dx and dy into a single frame
        // We can reuse the m_smaller buffer
        cv::addWeighted(m_grad_x, 1, m_grad_y, 1, 0, m_out);

        return m_out;
    }

    cv::Mat& VisErode::process(CamFrame* frame, cv::Mat &image, VisRecord* recording, bool& valid)
    {
        // The erosion kernel will remove small dots seen where the localized
        // gradient is higher. The resultant image not include small dots that
        // are unlikely to be actual track edges.
        cv::morphologyEx(image, m_out, cv::MORPH_OPEN, m_erosion_kernel);
        return m_out;
    }

    cv::Mat& VisOtsuThreshold::process(CamFrame* frame, cv::Mat &image, VisRecord* recording, bool& valid)
    {
        // Now that we have the gradient image calculated, we need to threshold
        // these values to filter out the small deltas in color. To avoid needing
        // to mess with a thresholding value that depends on environment etc. We
        // can expose this pipeline to image and use the Otsu thresholding algorithm
        // to find the optimal threshold between foreground and background.
        if (m_ostu_index < m_ostu_frames)
        {
            // We only wants to perform Otsu thresholding for a certain number of frames
            // We can average out the running sum at the end
            m_ostu_running_sum += cv::threshold(image, m_out, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
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
            cv::threshold(image, m_out, m_ostu_running_sum, 255, cv::THRESH_BINARY);
        }

        return m_out;
    }

    VisWarp::VisWarp(F32 scale_x, F32 scale_y,
                     F32 translate_x, F32 translate_y)
    : m_scale_x(scale_x), m_scale_y(scale_y),
    m_translate(translate_x, translate_y),
    m_calibration(0)
    {
        for (U32 i = 0; i < 4; i++)
        {
            m_object_corners.emplace_back(0, 0);
        }
    }

    bool VisWarp::read(const char* calibration_file)
    {
        bool status = m_calibration.read(calibration_file);
        if (!status) return false;

        // The bottom edge should sit aligned with the origin. This may provide
        // a slight rotation and translation if needed
        // We don't need to mess with the Y position of the origin. Move the origin
        // to the center of the image
        m_object_corners[2] = m_calibration.image_corners[2] + m_translate;
        m_object_corners[3] = cv::Point2f(m_calibration.image_corners[3].x * m_scale_x,
                                          m_calibration.image_corners[2].y)
                                                  + m_translate;
        m_object_corners[0] = cv::Point2f(m_object_corners[2].x,
                                          m_object_corners[2].y -
                                          (F32)(m_calibration.corner_pattern.height - 1) *
                                          (m_calibration.square_size * m_scale_y))
                                                  + m_translate;
        m_object_corners[1] = cv::Point2f(m_object_corners[3].x,
                                          m_object_corners[3].y -
                                          (F32)(m_calibration.corner_pattern.height - 1) *
                                          (m_calibration.square_size * m_scale_y))
                                                  + m_translate;

        return true;
    }

    cv::Mat& VisWarp::process(CamFrame* frame, cv::Mat& image, VisRecord* recording, bool &valid)
    {
        if (m_current_size != cv::Size(image.cols, image.rows))
        {
            m_current_size = cv::Size(image.cols, image.rows);
            m_transform = m_calibration.get_transform(
                    m_calibration.image_size.width / m_current_size.width,
                    m_object_corners);
        }

        // Warp the image to the object (birds-eye) view
        cv::warpPerspective(image, m_warped,
                            m_transform,
                            m_current_size,
                            cv::INTER_NEAREST);

//        cv::drawMarker(m_smaller, m_calibration.image_corners[0] * (1.0 / m_downscale), cv::Scalar(0, 0, 0), cv::MARKER_CROSS, 50, 4);
//        cv::drawMarker(m_smaller, m_calibration.image_corners[1] * (1.0 / m_downscale), cv::Scalar(0, 0, 0), cv::MARKER_CROSS, 50, 4);
//        cv::drawMarker(m_smaller, m_calibration.image_corners[2] * (1.0 / m_downscale), cv::Scalar(0, 0, 0), cv::MARKER_CROSS, 50, 4);
//        cv::drawMarker(m_smaller, m_calibration.image_corners[3] * (1.0 / m_downscale), cv::Scalar(0, 0, 0), cv::MARKER_CROSS, 50, 4);

//        cv::drawMarker(m_smaller, m_calibration.object_corners[0] * (1.0 / m_downscale), cv::Scalar(150, 150, 150), cv::MARKER_CROSS, 50, 4);
//        cv::drawMarker(m_smaller, m_calibration.object_corners[1] * (1.0 / m_downscale), cv::Scalar(150, 150, 150), cv::MARKER_CROSS, 50, 4);
//        cv::drawMarker(m_smaller, m_calibration.object_corners[2] * (1.0 / m_downscale), cv::Scalar(150, 150, 150), cv::MARKER_CROSS, 50, 4);
//        cv::drawMarker(m_smaller, m_calibration.object_corners[3] * (1.0 / m_downscale), cv::Scalar(150, 150, 150), cv::MARKER_CROSS, 50, 4);

        return m_warped;
    }
}
