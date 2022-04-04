#ifndef VO_CMPE460_CAMFRAME_H
#define VO_CMPE460_CAMFRAME_H

#include <Fw/Types/Serializable.hpp>
#include <opencv2/core/mat.hpp>
#include <libcamera/framebuffer.h>
#include <Rpi/Cam/core/stream_info.hpp>
#include <Rpi/Cam/core/completed_request.hpp>

#include <functional>
#include <atomic>

namespace Rpi
{
    class CamFrame
    {
    public:
        CamFrame();

        CamFrame(const CamFrame&) = delete;
        CamFrame(CamFrame&&) = delete;

        CompletedRequest* request;
        StreamInfo info;
        libcamera::FrameBuffer* buffer;
        libcamera::Span<U8> span;

        void incref();
        void decref();
        bool in_use() const;

        void register_callback(std::function<void(CompletedRequest*)> return_cb);

    private:
        std::function<void(CompletedRequest*)> return_buffer;
        std::atomic<I32> ref_count;

        void clear();
    };
}

#endif //VO_CMPE460_CAMFRAME_H
