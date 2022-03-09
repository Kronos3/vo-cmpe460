#include <evr/evr.h>
#include <fw/fw.h>
#include <dp/dp.h>
#include <circle/synchronize.h>
#include <evrs.h>
#include <kernel/kernel.h>

EvrEngine::EvrEngine(DpEngine* dp_engine)
: m_queue_n(0),
  m_queue_internal{0},
  m_queue(m_queue_internal, sizeof(m_queue_internal)),
  m_evrs_dropped_n(0),
  m_dp_engine(dp_engine),
  m_dp(nullptr) // data product is not opened until FS is ready
{
    FW_ASSERT(m_dp_engine);
}

static void fatal_handler(const char* file, u32 line, const char* expr_str, u32 nargs, va_list args)
{
    u32 fatal_args[3];
    for (u32 i = 0; i < nargs; i++)
    {
        switch(i)
        {
            case 0:
            case 1:
            case 2:
                fatal_args[i] = va_arg(args, u32);
                break;
            default:
                (void) va_arg(args, u32);
                break;
        }
    }

    switch(nargs)
    {
        case 0:
            evr_Fatal_0(file, line, expr_str);
            break;
        case 1:
            evr_Fatal_1(file, line, expr_str, fatal_args[0]);
            break;
        case 2:
            evr_Fatal_2(file, line, expr_str,
                        fatal_args[0], fatal_args[1]);
            break;
        default:
        case 3:
            evr_Fatal_3(file, line, expr_str,
                        fatal_args[0], fatal_args[1], fatal_args[2]);
            break;
    }

    // Flush all EVRs to disk
    kernel::evrEngine.flush();
    kernel::dpEngine.flush();

    while (TRUE)
    {
        kernel::act.flash_fatal();
    }
}

void EvrEngine::init()
{
    fw_register_fatal_handler(fatal_handler);
}

void EvrEngine::log(const Fw::SerializeBufferBase &evr)
{
    Fw::SerializeStatus status;

    // Don't run the scheduler while we are copying bytes into the queue
    lock.Acquire();

    // Actually serialize the EVR
    status = m_queue.serialize(evr);
    if (Fw::FW_SERIALIZE_NO_ROOM_LEFT == status)
    {
        // The queue is full!
        // We can't fit this EVR in here
        // Drop it
        m_evrs_dropped_n++;
    }
    else
    {
        m_queue_n++;
    }

    lock.Release();
}

void EvrEngine::open(const CString &evr_file)
{
    if (m_dp)
    {
        m_dp->close();
        m_dp = nullptr;
    }

    m_dp = &m_dp_engine->create(evr_file);
}

void EvrEngine::schedIn()
{
    flush();
}

void EvrEngine::flush()
{
    // Check if we are able to dump to a data product yet
    // If a data product is not opened soon, we will overflow!
    if (!m_dp)
    {
        return;
    }

    // Don't run the scheduler while we are copying bytes into the queue
    lock.Acquire();

    // Flush all the queued EVRs to DP
    m_dp->write(m_queue.getBuffAddr(), m_queue.getBuffLength());
    m_queue.resetSer();

    // Don't run the scheduler while we are copying bytes into the queue
    lock.Release();
}
