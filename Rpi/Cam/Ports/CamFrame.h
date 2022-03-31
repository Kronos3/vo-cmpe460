#ifndef VO_CMPE460_CAMFRAME_H
#define VO_CMPE460_CAMFRAME_H

#include <Fw/Types/Serializable.hpp>
#include <opencv2/core/mat.hpp>
#include <libcamera/framebuffer.h>
#include <Rpi/Cam/core/stream_info.hpp>
#include <Rpi/Cam/core/completed_request.hpp>

namespace Rpi
{
    class CamFrame
    {
    public:
        CamFrame() = default;

        CamFrame(const CamFrame&) = delete;
        CamFrame(CamFrame&&) = delete;

        bool in_use = false;
        CompletedRequest* request = nullptr;
        StreamInfo info;
        libcamera::FrameBuffer* buffer = nullptr;
        libcamera::Span<U8> span;

        void clear()
        {
            in_use = false;
            request = nullptr;
            buffer = nullptr;
            size_t s = 0;
            span = libcamera::Span<U8>(nullptr, s);
        }
    };
}

#endif //VO_CMPE460_CAMFRAME_H
