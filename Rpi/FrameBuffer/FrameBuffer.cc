
#include "FrameBuffer.h"

namespace Fw
{
    FrameBuffer::FrameBuffer()
    : Buffer()
    {
    }

    FrameBuffer::FrameBuffer(const Buffer &src) : Buffer(src)
    {
    }

    FrameBuffer::FrameBuffer(U8* data, U32 size, U32 context)
    : Buffer(data, size, context)
    {
    }

    void FrameBuffer::acquire()
    {
        mutex.lock();
    }

    void FrameBuffer::release()
    {
        mutex.unLock();
    }
}
