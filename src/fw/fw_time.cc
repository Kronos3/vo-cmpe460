#include <fw/fw_time.h>
#include <kernel/kernel.h>

namespace Fw
{
    SerializeStatus Time::serialize(SerializeBufferBase &buffer) const
    {
        SerializeStatus status;
        status = buffer.serialize(m_seconds);
        if (FW_SERIALIZE_OK != status) return status;

        status = buffer.serialize(m_useconds);
        if (FW_SERIALIZE_OK != status) return status;

        return FW_SERIALIZE_OK;
    }

    SerializeStatus Time::deserialize(SerializeBufferBase &buffer)
    {
        SerializeStatus status;
        status = buffer.deserialize(m_seconds);
        if (FW_SERIALIZE_OK != status) return status;

        status = buffer.deserialize(m_useconds);
        if (FW_SERIALIZE_OK != status) return status;

        return FW_SERIALIZE_OK;
    }

    Time::Time()
    : m_seconds(0), m_useconds(0)
    {
        // Get the current time
        kernel::tim.GetUniversalTime(&m_seconds, &m_useconds);
    }

    Time::Time(u32 m_seconds, u32 useconds)
    : m_seconds(m_seconds), m_useconds(useconds)
    {
    }

    u32 Time::get_seconds() const
    {
        return m_seconds;
    }

    u32 Time::get_useconds() const
    {
        return m_useconds;
    }
}

