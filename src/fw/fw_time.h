#ifndef VO_CMPE460_FW_TIME_H
#define VO_CMPE460_FW_TIME_H

#include <circle/types.h>
#include <fw/serializable.h>

namespace Fw
{
    class Time : public Serializable
    {
        u32 m_seconds;
        u32 m_useconds;
    public:
        enum
        {
            SERIALIZED_SIZE = sizeof(u32) + sizeof(u32)
        };

        Time();
        Time(u32 m_seconds, u32 useconds);

        u32 get_seconds() const;
        u32 get_useconds() const;

        SerializeStatus serialize(SerializeBufferBase &buffer) const override;
        SerializeStatus deserialize(SerializeBufferBase &buffer) override;
    };
}

#endif //VO_CMPE460_FW_TIME_H
