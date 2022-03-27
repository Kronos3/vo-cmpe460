#include "RpiSerialDriverImpl.h"
#include <Rpi/Top/kernel.h>

namespace Drv
{

    RpiSerialDriverImpl::RpiSerialDriverImpl()
    : m_read_buf_ptr{}, m_read_buf(m_read_buf_ptr, sizeof(m_read_buf_ptr))
    {
    }

    void RpiSerialDriverImpl::init(NATIVE_INT_TYPE instance)
    {
        RpiSerialDriverComponentBase::init(instance);
    }

    void RpiSerialDriverImpl::serialSend_handler(NATIVE_INT_TYPE portNum, Fw::Buffer &serBuffer)
    {
//        kernel::led.On();
        // Send some data via Tx
//        U32 written = 0;
//        while(written < serBuffer.getSize())
//        {
//            written += kernel::serial.Write(serBuffer.getData() + written, serBuffer.getSize() - written);
//        }
//        kernel::led.Off();
    }

    void RpiSerialDriverImpl::schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context)
    {
//        kernel::led.On();
//        U32 n_read = kernel::serial.Read(m_read_buf.getData(), sizeof(m_read_buf_ptr));
//        if (n_read > 0)
//        {
//            m_read_buf.setSize(n_read);
//            serialRecv_out(0, m_read_buf);
//        }
//        else if (n_read < 0)
//        {
//            FW_ASSERT(0, n_read);
//        }
//        kernel::led.Off();
    }
}