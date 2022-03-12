#ifndef VO_CMPE460_FRAMEBUFFER_H
#define VO_CMPE460_FRAMEBUFFER_H

#include <Fw/Buffer/Buffer.hpp>
#include <Os/Mutex.hpp>

namespace Fw
{
    class FrameBuffer : public Buffer
    {
        Os::Mutex mutex;

    public:
        // Inherit all constructors from Buffer
        FrameBuffer();
        explicit FrameBuffer(const Buffer& src);
        FrameBuffer(U8* data, U32 size, U32 context=NO_CONTEXT);

        void acquire();
        void release();
    };
}

#endif //VO_CMPE460_FRAMEBUFFER_H
