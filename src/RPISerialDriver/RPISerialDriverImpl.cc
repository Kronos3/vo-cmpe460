
#include "RPISerialDriverImpl.h"
#include <RPI/kernel.h>

namespace Drv
{

    RPISerialDriverImpl::RPISerialDriverImpl()
    : m_read_buf_ptr{}, m_read_buf(m_read_buf_ptr, sizeof(m_read_buf_ptr))
    {
    }

    void RPISerialDriverImpl::init(NATIVE_INT_TYPE instance)
    {
        RPISerialDriverComponentBase::init(instance);
//        kernel::serial.Initialize(UART_BAUDRATE);
    }

    void RPISerialDriverImpl::serialSend_handler(NATIVE_INT_TYPE portNum, Fw::Buffer &serBuffer)
    {
        // Send some data via Tx
        U32 written = 0;
        while(written < serBuffer.getSize())
        {
            written += kernel::serial.Write(serBuffer.getData() + written, serBuffer.getSize() - written);
        }
    }

    void RPISerialDriverImpl::schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context)
    {
        U32 n_read = kernel::serial.Read(m_read_buf.getData(), sizeof(m_read_buf_ptr));
        if (n_read)
        {
            m_read_buf.setSize(n_read);
            serialRecv_out(0, m_read_buf);
        }
    }
}