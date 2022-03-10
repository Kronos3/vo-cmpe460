
#include "RpiSerialDriverImpl.h"
#include <src/Rpi/kernel.h>

namespace Drv
{

    RpiSerialDriverImpl::RpiSerialDriverImpl()
    : m_read_buf_ptr{}, m_read_buf(m_read_buf_ptr, sizeof(m_read_buf_ptr))
    {
    }

    void RpiSerialDriverImpl::init(NATIVE_INT_TYPE instance)
    {
        RpiSerialDriverComponentBase::init(instance);
//        kernel::serial.Initialize(UART_BAUDRATE);
    }

    void RpiSerialDriverImpl::serialSend_handler(NATIVE_INT_TYPE portNum, Fw::Buffer &serBuffer)
    {
        // Send some data via Tx
        U32 written = 0;
        while(written < serBuffer.getSize())
        {
            written += kernel::serial.Write(serBuffer.getData() + written, serBuffer.getSize() - written);
        }
    }

    void RpiSerialDriverImpl::schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context)
    {
        U32 n_read = kernel::serial.Read(m_read_buf.getData(), sizeof(m_read_buf_ptr));
        if (n_read)
        {
            m_read_buf.setSize(n_read);
            serialRecv_out(0, m_read_buf);
        }
    }
}