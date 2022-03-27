/*
 * SocketHelper.cpp
 *
 *  Created on: May 28, 2020
 *      Author: tcanham
 */

#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/Assert.hpp>
#include <Drv/RpiSocketIpDriver/RpiSocketHelper.hpp>

#include <circle/net/netsubsystem.h>
#include <circle/net/in.h>
#include <Fw/Logger/Logger.hpp>

// This implementation has primarily implemented to isolate
// the socket interface from the F' Fw::Buffer class.
// There is a macro in VxWorks (m_data) that collides with
// the m_data member in Fw::Buffer.


namespace Drv
{

    RpiSocketHelper::RpiSocketHelper()
            : m_socketInFd(nullptr),
              m_socketOutFd(nullptr),
              m_sendUdp(false),
              m_hostname(),
              m_port(0)
    {
    }

    RpiSocketHelper::~RpiSocketHelper()
    {
        close();
    }

    SocketIpStatus RpiSocketHelper::configure(const CIPAddress& hostname,
                                              const U16 port,
                                              const bool send_udp)
    {
        m_sendUdp = send_udp;
        m_port = port;
        m_hostname = hostname;

        return SOCK_SUCCESS;
    }

    bool RpiSocketHelper::isOpened() const
    {
        return m_socketInFd;
    }

    void RpiSocketHelper::close()
    {
        // TCP uses the same socket for recv/send
        if (m_socketInFd != m_socketOutFd)
        {
            delete m_socketOutFd;
        }
        delete m_socketInFd;
        m_socketInFd = nullptr;
        m_socketOutFd = nullptr;
    }

    SocketIpStatus RpiSocketHelper::open()
    {
        // Only the input (TCP) socket needs closing
        delete m_socketInFd; // Close open sockets, to force a re-open

        CString hostname;
        m_hostname.Format(&hostname);

        // Open a TCP socket for incoming commands, and outgoing data if not using UDP
        m_socketInFd = new CSocket(CNetSubSystem::Get(), IPPROTO_TCP);
        if (m_socketInFd->Connect(m_hostname, m_port) < 0)
        {
            static bool shown_error = false;
            if (!shown_error)
            {
                Fw::Logger::logMsg("Failed to connect TCP: %s:%d\r\n", (POINTER_CAST)(const char*)hostname, m_port);
                shown_error = true;
            }

            return SOCK_FAILED_TO_CONNECT;
        }

        if (m_sendUdp && m_socketOutFd == nullptr)
        {
            // If we need UDP sending, attempt to open UDP
            m_socketOutFd = new CSocket(CNetSubSystem::Get(), IPPROTO_UDP);

            if (m_socketOutFd->Bind(m_port) < 0)
            {
                Fw::Logger::logMsg("Failed to bind UDP to port %d\r\n", m_port);
                return SOCK_FAILED_TO_GET_SOCKET;
            }

            if (m_socketOutFd->Connect(m_hostname, m_port) < 0)
            {
                Fw::Logger::logMsg("Failed to connect UDP %s:%d\r\n", (POINTER_CAST)(const char*)hostname, m_port);
                return SOCK_FAILED_TO_CONNECT;
            }
        }
        else if (!this->m_sendUdp)
        {
            // Not sending UDP, reuse our input TCP socket
            m_socketOutFd = m_socketInFd;
        }

        Fw::Logger::logMsg("Connected successfully to %s:%d\r\n", (POINTER_CAST)(const char*)hostname, m_port);
        return SOCK_SUCCESS;
    }

    void RpiSocketHelper::send(U8* data, const U32 size)
    {
        U32 total = 0;
        // Prevent transmission before connection, or after a disconnect
        if (m_socketOutFd == nullptr)
        {
            return;
        }
        // Attempt to send out data
        for (U32 i = 0; i < MAX_SEND_ITERATIONS && total < size; i++)
        {
            I32 sent;
            if (m_sendUdp)
            {
                // Output to send UDP
                sent = m_socketOutFd->SendTo(data + total, size - total,
                                             SOCKET_SEND_FLAGS,
                                             m_hostname, m_port);
            }
            else
            {
                // Output to send TCP
                sent = m_socketOutFd->Send(data + total, size - total, SOCKET_SEND_FLAGS);
            }

            if (sent == -1)
            {
                // Server disconnected
                close();
                break;
            }
            else
            {
                total += sent;
            }
        }

    }

    SocketIpStatus RpiSocketHelper::recv(U8* data, I32 &size)
    {
        SocketIpStatus status = SOCK_SUCCESS;
        // Attempt to recv out data
        size = m_socketInFd->Receive(data, size, SOCKET_RECV_FLAGS);

        FW_ASSERT(size != 0);

        // Error returned, and it wasn't an interrupt, nor a reset
        if (size < 0)
        {
            status = SOCK_READ_ERROR; // Stop recv task on error
        }

        return status;

    }

}
