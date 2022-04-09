//
// Created by tumbar on 3/31/22.
//

#ifndef VO_CMPE460_VISRECORD_H
#define VO_CMPE460_VISRECORD_H

#include <Fw/Types/String.hpp>
#include <Os/File.hpp>

#include <unordered_map>
#include <vector>
#include "Fw/Buffer/Buffer.hpp"
#include "opencv2/core/types.hpp"
#include "opencv2/core/mat.hpp"
#include <memory>

namespace Rpi
{
    class VisRecord
    {
    public:
        virtual bool write(const char* filename) const = 0;
        virtual bool read(const char* filename) = 0;
        virtual bool done() const = 0;
        virtual ~VisRecord() = default;
    };

    struct ChessBoardRecord : public VisRecord
    {
        // Intermediate frames
        // Populated per frame
        std::vector<std::vector<cv::Point2f>> corners;
        U32 total_frames;

        explicit ChessBoardRecord(U32 num_frame);
        bool write(const char* filename) const override = 0;
        bool read(const char* filename) override = 0;
        bool done() const override;
    };

    struct CalibrationRecord : public ChessBoardRecord
    {
        explicit CalibrationRecord(U32 num_frame);

        // Final calibrated image data products
        cv::Mat k;                  //!< Camera intrinsic matrix
        cv::Mat d;                  //!< Distortion transform
        cv::Mat r;                  //!< Rotation Transform
        cv::Mat t;                  //!< Translation Transform

        bool write(const char* filename) const override;
        bool read(const char* filename) override;
    };

    struct WarpRecord : public ChessBoardRecord
    {
        explicit WarpRecord(U32 num_frame);

        std::vector<cv::Point2f> image_corners;    //!< Actual image position of the chess board corners
        cv::Size image_size;                       //!< Size of the original image
        cv::Size corner_pattern;                   //!< Chessboard pattern (usually 9x6)
        F32 square_size;                           //!< Pixel size of the chessboard square

        bool write(const char* filename) const override;
        bool read(const char* filename) override;

        cv::Mat get_transform(I32 downscale, const std::vector<cv::Point2f>& object_corners) const;
    };
}

#endif //VO_CMPE460_VISRECORD_H
