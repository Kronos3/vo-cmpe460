// ======================================================================
// \title  SocketIpDriverComponentImpl.hpp
// \author mstarch
// \brief  hpp file for SocketIpDriver component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#ifndef SocketIpDriver_HPP
#define SocketIpDriver_HPP

#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Buffer/Buffer.hpp>
#include <Drv/RpiSocketIpDriver/RpiSocketIpDriverComponentAc.hpp>
#include <SocketIpDriverCfg.hpp>
#include <Drv/RpiSocketIpDriver/RpiSocketHelper.hpp>
#include <Drv/RpiSocketIpDriver/RpiSocketIpDriverTypes.hpp>
#include <Os/Task.hpp>

#include <circle/net/socket.h>

namespace Drv
{
    class RpiSocketIpDriverComponentImpl : public RpiSocketIpDriverComponentBase
    {
    public:

        // ----------------------------------------------------------------------
        // Construction, initialization, and destruction
        // ----------------------------------------------------------------------

        //! Construct object SocketIpDriver
        //!
        explicit RpiSocketIpDriverComponentImpl(
                const char* compName /*!< The component name*/
        );

        //! Initialize object SocketIpDriver
        //!
        void init(
                NATIVE_INT_TYPE instance, /*!< The instance number*/
                const char* hostname, /*!< Hostname of remote server */
                U16 port, /*!< Port of remote server */
                bool send_udp = SOCKET_SEND_UDP /*!< Send down using UDP. Default: read from configuration HPP*/
        );

        //! Destroy object SocketIpDriver
        //!
        ~RpiSocketIpDriverComponentImpl() override;

        //! Start the socket task
        //!
        void startSocketTask();

        //! Task to join nondetached pthreads
        //!
        Os::Task::TaskStatus joinSocketTask(void** value_ptr);

        //! Set the stop flag on the thread's loop such that it will shutdown promptly
        //!
        void exitSocketTask();

    PRIVATE:

        static void taskRoutine(void*);
        void task();

        // ----------------------------------------------------------------------
        // Handler implementations for user-defined typed input ports
        // ----------------------------------------------------------------------

        //! Handler implementation for downlink
        //!
        void send_handler(
                NATIVE_INT_TYPE portNum, /*!< The port number*/
                Fw::Buffer &fwBuffer
        ) override;

        struct ServerParams
        {
            const char* hostname;
            U16 port;
            bool is_udp;
            ServerParams()
            : hostname(nullptr), port(0), is_udp(false)
            {}
        } m_server;

        // socket helper instance
        RpiSocketHelper m_helper;

        Os::Task m_recvTask;                         //!< Os::Task to start for receiving data
        Fw::Buffer m_buffer;                         //!< Fw::Buffer used to pass data
        U8 m_backing_data[MAX_RECV_BUFFER_SIZE];     //!< Buffer used to store data
        bool m_stop;                                 //!< Stop the receiving port
    };

} // end namespace Svc

#endif
