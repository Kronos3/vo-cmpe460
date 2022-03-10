#ifndef VO_CMPE460_RPISERIALDRIVERIMPL_H
#define VO_CMPE460_RPISERIALDRIVERIMPL_H

#include <src/RpiSerialDriver/RpiSerialDriverComponentAc.hpp>

namespace Drv
{
    class RpiSerialDriverImpl : public RpiSerialDriverComponentBase
    {
    public:
        RpiSerialDriverImpl();
        void init(NATIVE_INT_TYPE instance);
    private:
        void schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context) override;
        void serialSend_handler(NATIVE_INT_TYPE portNum, Fw::Buffer &serBuffer) override;

        U8 m_read_buf_ptr[2048];
        Fw::Buffer m_read_buf;
    };
}

#endif //VO_CMPE460_RPISERIALDRIVERIMPL_H
