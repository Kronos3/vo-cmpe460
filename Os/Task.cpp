#include <Os/Task.hpp>
#include <Fw/Types/Assert.hpp>

#include <circle/sched/task.h>

namespace Os
{
    class CircleTask : public CTask
    {
        Task::taskRoutine m_routine;
        void* m_arg;

    public:
        explicit CircleTask(U32 stack_size, Task::taskRoutine routine, void* arg)
        : CTask(stack_size == -1 ? 0x8000 : stack_size, TRUE), m_routine(routine), m_arg(arg)
//        : m_routine(routine), m_arg(arg)
        {
            FW_ASSERT(m_routine);
        }

        void Run() override
        {
            FW_ASSERT(m_routine);
            m_routine(m_arg);
        }
    };

    Task::Task() :
            m_handle(0),
            m_identifier(0),
            m_affinity(-1),
            m_started(false),
            m_suspendedOnPurpose(false)
    {}

    Task::TaskStatus Task::start(const Fw::StringBase &name, taskRoutine routine, void* arg, NATIVE_UINT_TYPE priority,
                                 NATIVE_UINT_TYPE stackSize, NATIVE_UINT_TYPE cpuAffinity, NATIVE_UINT_TYPE identifier)
    {
        //Get a task handle, and set it up
        auto* handle = new CircleTask(stackSize, routine, arg);

        //Register this task using our custom task handle
        m_handle = reinterpret_cast<POINTER_CAST>(handle);
        this->m_name = "BR_";
        this->m_name += name;
        this->m_identifier = identifier;
        // If a registry has been registered, register task
        if (Task::s_taskRegistry)
        {
            Task::s_taskRegistry->addTask(this);
        }

        handle->Start();

        return Task::TASK_OK;
    }

    Task::TaskStatus Task::delay(NATIVE_UINT_TYPE milliseconds)
    {
        //Task delays are a bad idea in baremetal tasks
        return Task::TASK_DELAY_ERROR;
    }

    Task::~Task()
    {
        if (this->m_handle)
        {
            delete reinterpret_cast<CircleTask*>(this->m_handle);
            m_handle = 0;
        }
        // If a registry has been registered, remove task
        if (Task::s_taskRegistry)
        {
            Task::s_taskRegistry->removeTask(this);
        }
    }

    void Task::suspend(bool onPurpose)
    {
        FW_ASSERT(reinterpret_cast<CircleTask*>(this->m_handle) != nullptr);
        reinterpret_cast<CircleTask*>(this->m_handle)->Suspend();
    }

    void Task::resume(void)
    {
        FW_ASSERT(reinterpret_cast<CircleTask*>(this->m_handle) != nullptr);
        reinterpret_cast<CircleTask*>(this->m_handle)->Resume();
    }

    bool Task::isSuspended(void)
    {
        FW_ASSERT(reinterpret_cast<CircleTask*>(this->m_handle) != nullptr);
        return reinterpret_cast<CircleTask*>(this->m_handle)->IsSuspended();
    }

    Task::TaskStatus Task::join(void** value_ptr)
    {
        return TASK_OK;
    }

}
