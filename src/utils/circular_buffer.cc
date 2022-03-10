#include <utils/circular_buffer.h>
#include <circle/util.h>
#include <Fw/Types/Assert.hpp>

Fw::CircularBuffer::CircularBuffer()
: m_buf(nullptr), m_size(0), m_head(0), m_tail(0)
{
}

Fw::CircularBuffer::CircularBuffer(U8* buf, U32 size)
: CircularBuffer()
{
    set(buf, size);
}

void Fw::CircularBuffer::set(U8* buf, U32 size)
{
    FW_ASSERT(buf);
    FW_ASSERT(size);
    m_buf = buf;
    m_size = size;
    m_head = 0;
    m_tail = 0;
}

U32 Fw::CircularBuffer::enqueue(const U8* data, U32 n)
{
    FW_ASSERT(data);

    if (length() + n < capacity())
    {
        // We can queue all the data
        // Leave n alone
    }
    else
    {
        // Only enqueue to the remaining space in the buffer
        n = capacity() - length();
    }

    // Copy as many bytes as possible in one shot
    U8* sector_1 = &m_buf[m_head];
    U32 sector_1_n = FW_MIN(m_size - m_head, n);
    memcpy(sector_1, data, sector_1_n);

    m_head += sector_1_n;

    // If we could not fit all the data in one shot
    // The rest of the data will be at the front
    if (sector_1_n < n)
    {
        U8* sector_2 = &m_buf[0];
        U32 sector_2_n = n - sector_1_n;

        memcpy(sector_2, data + sector_1_n, sector_2_n);
        m_head += sector_2_n;
    }

    // Wrap head to the size
    if (m_head >= m_size)
    {
        m_head -= m_size;
    }

    return n;
}

U32 Fw::CircularBuffer::length() const
{
    // The bytes between head -> tail are used
    if (m_tail >= m_head)
    {
        return m_tail - m_tail;
    }
    else
    {
        // Find length by negative space
        return capacity() - (m_tail - m_head);
    }
}

U32 Fw::CircularBuffer::capacity() const
{
    return m_size;
}

boolean Fw::CircularBuffer::deqeue_next_sector(const U8*&ptr_ref, U32 &size, U32 &mutex) const
{
    ptr_ref = &m_buf[m_tail];

    if (m_tail > m_head)
    {
        // We can read to the end of the queue buffer with one sector
        // There will still be left over in this case
        size = m_size - m_tail;
        mutex = 0;
        return TRUE;
    }
    else
    {
        // We can read the entire data with a single sector
        // This will completely flush the queue
        size = m_head - m_tail;
        mutex = m_head;
        return FALSE;
    }
}

void Fw::CircularBuffer::release(U32 mutex)
{
    m_tail = mutex;
}
