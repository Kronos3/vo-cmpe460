/*
 * SocketHelper.hpp
 *
 *  Created on: May 28, 2020
 *      Author: tcanham
 */

#ifndef DRV_RPISOCKETIPDRIVER_SOCKETHELPER_HPP_
#define DRV_RPISOCKETIPDRIVER_SOCKETHELPER_HPP_

#include <Fw/Types/BasicTypes.hpp>
#include <Drv/RpiSocketIpDriver/RpiSocketIpDriverTypes.hpp>
#include <SocketIpDriverCfg.hpp>

#include <circle/net/ipaddress.h>
#include <circle/net/socket.h>

namespace Drv
{
    class RpiSocketHelper
    {
    public:

        RpiSocketHelper();

        virtual ~RpiSocketHelper();

        SocketIpStatus configure(const char* hostname,
                                 U16 port,
                                 bool send_udp);

        bool isOpened() const;

        SocketIpStatus open();

        void send(U8* data, U32 size);
        SocketIpStatus recv(U8* data, I32 &size);

        void close();

    PRIVATE:

        CSocket* m_socketInFd;  //!< Input file descriptor, always TCP
        CSocket* m_socketOutFd; //!< Output file descriptor, always UDP
        bool m_sendUdp;
        CIPAddress m_hostname;              //!< Hostname to supply
        U16 m_port;                         //!< IP address port used

    };

}

#endif /* DRV_RPISOCKETIPDRIVER_SOCKETHELPER_HPP_ */
