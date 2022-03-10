#include <Os/Mutex.hpp>
#include <circle/sched/scheduler.h>
#include <circle/sched/mutex.h>

namespace Os
{
    Mutex::Mutex()
    : m_handle(0)
    {
        m_handle = reinterpret_cast<POINTER_CAST>(new CMutex);
    }

    Mutex::~Mutex() = default;

    void Mutex::lock()
    {
        reinterpret_cast<CMutex*>(m_handle)->Acquire();
    }

    void Mutex::unLock()
    {
        reinterpret_cast<CMutex*>(m_handle)->Release();
    }
}
