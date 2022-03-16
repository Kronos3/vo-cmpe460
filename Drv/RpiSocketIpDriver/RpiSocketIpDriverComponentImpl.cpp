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
    RpiSocketIpDriverComponentImpl::RpiSocketIpDriverComponentImpl(const char* const compName)
    : ByteStreamDriverModelComponentBase(compName),
        m_helper(),
        m_stop(false)
    {}

    void RpiSocketIpDriverComponentImpl::init(const NATIVE_INT_TYPE instance,
                                              const CIPAddress& hostname,
                                              U16 port,
                                              const bool send_udp)
    {
        ByteStreamDriverModelComponentBase::init(instance);
        m_server.hostname = hostname;
        m_server.port = port;
        m_server.is_udp = send_udp;
    }

    RpiSocketIpDriverComponentImpl::~RpiSocketIpDriverComponentImpl() = default;

    void RpiSocketIpDriverComponentImpl::task()
    {
        SocketIpStatus status = SOCK_SUCCESS;

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
                Fw::Buffer buffer = allocate_out(0, 1024);
                U8* data = buffer.getData();
                FW_ASSERT(data);
                I32 size = static_cast<I32>(buffer.getSize());
                status = m_helper.recv(data, size);
                if (status != SOCK_SUCCESS &&
                    status != SOCK_INTERRUPTED_TRY_AGAIN)
                {
                    m_helper.close();
                    recv_out(0, buffer, RECV_ERROR);
                }
                else
                {
                    buffer.setSize(size);
                    recv_out(0, buffer, RECV_OK);
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
                Fw::Logger::logMsg("Configuring WLAN\r\n");
                m_helper.configure(m_server.hostname, m_server.port, m_server.is_udp);

                Fw::Logger::logMsg("Opening socket to GDS\r\n");
                SocketIpStatus stat = m_helper.open();
                if (stat != SOCK_SUCCESS)
                {
                    Fw::Logger::logMsg("Unable to open socket: %d\r\n", stat);
                }
            }

            Fw::Logger::logMsg("Starting comm task\r\n");
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

    Drv::PollStatus RpiSocketIpDriverComponentImpl::poll_handler(NATIVE_INT_TYPE portNum, Fw::Buffer &pollBuffer)
    {
        FW_ASSERT(0); // It is an error to call this handler on IP drivers
        return Drv::POLL_ERROR;
    }

    Drv::SendStatus RpiSocketIpDriverComponentImpl::send_handler(NATIVE_INT_TYPE portNum, Fw::Buffer &fwBuffer)
    {
        m_helper.send(fwBuffer.getData(), fwBuffer.getSize());
        // Always return the buffer
        deallocate_out(0, fwBuffer);
        return SEND_OK;
    }
} // end namespace Svc
