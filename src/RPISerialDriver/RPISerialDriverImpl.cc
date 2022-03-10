
#include <RPICfg.hpp>
#include "RPISerialDriverImpl.h"
#include <RPI/kernel.h>

namespace Drv
{

    RPISerialDriverImpl::RPISerialDriverImpl()
    : m_read_buf(m_read_buf_ptr, sizeof(m_read_buf_ptr))
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
        kernel::serial.Write(serBuffer.getData(), serBuffer.getSize());
    }

    void RPISerialDriverImpl::schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context)
    {
        U32 n_read = kernel::serial.Read(m_read_buf.getData(), m_read_buf.getSize());
        if (n_read)
        {
            SerialReadStatus serReadStat = SER_OK;
            serialRecv_out(0, m_read_buf, serReadStat);
        }
    }
}