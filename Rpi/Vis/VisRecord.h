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

    struct CalibrationRecord : public VisRecord
    {
        // Intermediate frames
        // Populated per frame
        std::vector<std::vector<cv::Point2f>> corners;
        U32 total_frames;

        // Final calibrated image data products
        cv::Mat k;                  //!< Camera intrinsic matrix
        cv::Mat d;                  //!< Distortion transform
        cv::Mat r;                  //!< Rotation Transform
        cv::Mat t;                  //!< Translation Transform

        explicit CalibrationRecord(U32 num_frame);
        bool write(const char* filename) const override;
        bool read(const char* filename) override;
        bool done() const override;
        ~CalibrationRecord() override = default;
    };
}

#endif //VO_CMPE460_VISRECORD_H
