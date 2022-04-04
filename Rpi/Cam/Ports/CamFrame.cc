#include "CamFrame.h"

#include <utility>
#include "Assert.hpp"

namespace Rpi
{
    Rpi::CamFrame::CamFrame()
    : request(nullptr),
    buffer(nullptr),
    return_buffer(nullptr),
    ref_count(0)
    {
    }

    void CamFrame::incref()
    {
        ref_count++;
    }

    void CamFrame::decref()
    {
        if (ref_count == 1)
        {
            // Free the frame buffer
            clear();
        }
        else
        {
            ref_count--;
        }
    }

    void CamFrame::clear()
    {
        FW_ASSERT(ref_count == 1, ref_count);
        FW_ASSERT(request);

        // Returns the buffer back to the camera
        return_buffer(request);

        request = nullptr;
        buffer = nullptr;
        size_t s = 0;
        span = libcamera::Span<U8>(nullptr, s);
        ref_count = 0;
    }

    bool CamFrame::in_use() const
    {
        return ref_count > 0;
    }

    void CamFrame::register_callback(std::function<void(CompletedRequest*)> return_cb)
    {
        return_buffer = std::move(return_cb);
    }
}
