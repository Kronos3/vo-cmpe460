#ifndef VO_CMPE460_RTLINUXTIMEIMPL_H
#define VO_CMPE460_RTLINUXTIMEIMPL_H

#include <Drv/RTLinuxTime/RTLinuxTimeComponentAc.hpp>
#include <pthread.h>
#include <ctime>
#include <csignal>

namespace Rpi
{
    class RTLinuxTimeImpl : public RTLinuxTimeComponentBase
    {
    public:
        explicit RTLinuxTimeImpl(U32 us_delay);

        void init(NATIVE_INT_TYPE instance);
        void start();
        void exit();

    PRIVATE:
        static void handler(int signum);

        void timeGetPort_handler(NATIVE_INT_TYPE portNum, Fw::Time &time) override;

    PRIVATE:
        U32 m_us_period;
        pthread_t cycle_thread;
        pthread_attr_t m_attr;
        struct sched_param m_param;

        struct itimerval m_timer;
        struct sigaction sa;
    };
}

#endif //VO_CMPE460_RTLINUXTIMEIMPL_H
