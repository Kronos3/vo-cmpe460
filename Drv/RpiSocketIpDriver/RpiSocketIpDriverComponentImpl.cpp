// ======================================================================
// \title  SocketIpDriverComponentImpl.cpp
// \author mstarch
// \brief  cpp file for SocketIpDriver component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#include <Drv/RpiSocketIpDriver/RpiSocketIpDriverComponentImpl.hpp>
#include <SocketIpDriverCfg.hpp>
#include <Fw/Logger/Logger.hpp>
#include "Fw/Types/BasicTypes.hpp"
#include <Fw/Types/Assert.hpp>
#include <Os/TaskString.hpp>

namespace Drv
{
    //!< Storage for our keep-alive data
    const char KEEPALIVE_CONST[] = KEEPALIVE_DATA;

    RpiSocketIpDriverComponentImpl::RpiSocketIpDriverComponentImpl(const char* const compName)
    : RpiSocketIpDriverComponentBase(compName),
        m_helper(),
        m_buffer(m_backing_data, sizeof(m_buffer)),
        m_backing_data{},
        m_stop(false)
    {}

    void RpiSocketIpDriverComponentImpl::init(const NATIVE_INT_TYPE instance,
                                              const char* hostname,
                                              U16 port,
                                              const bool send_udp)
    {
        RpiSocketIpDriverComponentBase::init(instance);
        m_server.hostname = hostname;
        m_server.port = port;
        m_server.is_udp = send_udp;
    }

    RpiSocketIpDriverComponentImpl::~RpiSocketIpDriverComponentImpl() = default;

    void RpiSocketIpDriverComponentImpl::task()
    {
        SocketIpStatus status = SOCK_SUCCESS;
        m_helper.configure(m_server.hostname, m_server.port, m_server.is_udp);

        do
        {
            // Open a network connection if it has not already been open
            if (not m_helper.isOpened()
                and (m_helper.open() != SOCK_SUCCESS)
                and not m_stop)
            {
                Os::Task::delay(PRE_CONNECTION_RETRY_INTERVAL_MS);
            }

            // If the network connection is open, read from it
            if (m_helper.isOpened())
            {
                U8* data = m_buffer.getData();
                FW_ASSERT(data);
                I32 size = 0;
                status = m_helper.recv(data, size);
                if (status != SOCK_SUCCESS &&
                    status != SOCK_INTERRUPTED_TRY_AGAIN)
                {
                    m_helper.close();
                }
                else
                {
                    // Ignore KEEPALIVE data and send out any other data.
                    if (memcmp(data, KEEPALIVE_CONST,
                               (size > static_cast<I32>(sizeof(KEEPALIVE_CONST)) - 1) ? sizeof(KEEPALIVE_CONST) - 1
                                                                                      : size) != 0)
                    {
                        m_buffer.setSize(size);
                        recv_out(0, m_buffer);
                    }
                }
            }
        }
        while (not m_stop && (
                status == SOCK_SUCCESS ||
                status == SOCK_INTERRUPTED_TRY_AGAIN || RECONNECT_AUTOMATICALLY != 0));
    }

    void RpiSocketIpDriverComponentImpl::startSocketTask()
    {
        // Do not restart task
        Os::TaskString name("IpReadTask");
        if (not m_recvTask.isStarted())
        {
            // Start by opening the socket
            if (not m_helper.isOpened())
            {
                SocketIpStatus stat = m_helper.open();
                if (stat != SOCK_SUCCESS)
                {
                    Fw::Logger::logMsg("Unable to open socket: %d\n", stat);
                }
            }

            Os::Task::TaskStatus stat = m_recvTask.start(name, RpiSocketIpDriverComponentImpl::taskRoutine, this);
            FW_ASSERT(Os::Task::TASK_OK == stat, static_cast<NATIVE_INT_TYPE>(stat));
        }
    }

    void RpiSocketIpDriverComponentImpl::taskRoutine(void* self)
    {
        reinterpret_cast<RpiSocketIpDriverComponentImpl*>(self)->task();
    }

    Os::Task::TaskStatus RpiSocketIpDriverComponentImpl::joinSocketTask(void** value_ptr)
    {
        // provide return value of thread if value_ptr is not NULL
        return m_recvTask.join(value_ptr);
    }

    void RpiSocketIpDriverComponentImpl::exitSocketTask()
    {
        m_stop = true;
    }

    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    void RpiSocketIpDriverComponentImpl::send_handler(
            const NATIVE_INT_TYPE portNum,
            Fw::Buffer &fwBuffer)
    {
        U32 size = fwBuffer.getSize();
        U8* data = fwBuffer.getData();
        FW_ASSERT(data);
        m_helper.send(data, size);
    }
} // end namespace Svc
