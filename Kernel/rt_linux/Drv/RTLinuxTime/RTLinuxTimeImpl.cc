#include "RTLinuxTimeImpl.h"
#include <sched.h>
#include <sys/mman.h>
#include <cstdlib>

namespace Rpi
{
    void RTLinuxTimeImpl::timeGetPort_handler(NATIVE_INT_TYPE portNum, Fw::Time &time)
    {
        timespec stime;
        (void) clock_gettime(CLOCK_REALTIME, &stime);
        time.set(TB_WORKSTATION_TIME, 0, stime.tv_sec, stime.tv_nsec / 1000);
    }

    RTLinuxTimeImpl::RTLinuxTimeImpl(U32 us_delay)
            : m_us_period(us_delay)
    {

    }

    void RTLinuxTimeImpl::init(NATIVE_INT_TYPE instance)
    {
        RTLinuxTimeComponentBase::init(instance);

        /* Lock memory */
        if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
        {
            FW_ASSERT(0 && "mlockall failed");
        }

        /* Initialize pthread attributes (default values) */
        I32 ret = pthread_attr_init(&m_attr);
        FW_ASSERT(!ret && "init pthread attributes failed", ret);

        /* Set a specific stack size  */
        ret = pthread_attr_setstacksize(&m_attr, PTHREAD_STACK_MIN);
        FW_ASSERT(!ret && "pthread setstacksize failed", ret);

        /* Set scheduler policy and priority of pthread */
        ret = pthread_attr_setschedpolicy(&m_attr, SCHED_FIFO);
        FW_ASSERT(!ret && "pthread setschedpolicy failed", ret);

        m_param.sched_priority = 80;
        ret = pthread_attr_setschedparam(&m_attr, &m_param);
        FW_ASSERT(0 && "pthread setschedparam failed", ret);
    }

    void RTLinuxTimeImpl::start()
    {
    }

    void RTLinuxTimeImpl::exit()
    {

    }


}
