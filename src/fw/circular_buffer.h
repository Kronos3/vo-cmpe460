#ifndef VO_CMPE460_CIRCULAR_BUFFER_H
#define VO_CMPE460_CIRCULAR_BUFFER_H

#include <circle/types.h>

namespace Fw
{
    class CircularBuffer
    {
        u8* m_buf;
        u32 m_size;

        // Circular buffer part
        u32 m_head;
        u32 m_tail;

    public:
        CircularBuffer();

        /**
         * Initialize a circular buffer with the
         * underlying straight buffer
         * @param buf normal memory
         * @param size length of linear memory
         */
        void set(u8* buf, u32 size);
        CircularBuffer(u8* buf, u32 size);

        /**
         * @return Number of bytes currently enqueued
         */
        u32 length() const;

        /**
         * @return Total number of bytes in underlying buffer
         */
        u32 capacity() const;

        /**
         * Get the next pointer and as many bytes as possible in
         * one straight shot. This can be used to flush the queue
         * as fast as possible.
         * @param ptr_ref pointer to start of output sector
         * @param size length of sector
         * @param mutex mutex to hold buffer to
         * @return TRUE if there are more sectors remaining in the buffer
         */
        boolean deqeue_next_sector(const u8*& ptr_ref, u32& size, u32& mutex) const;

        /**
         * Release a mutex while data is being used from the buffer
         * @param mutex future value of tail from deqeue_next_sector
         */
       void release(u32 mutex);

        /**
         * Enqueue data to the circular buffer
         * @param data data to enqueue
         * @param n length data
         * @return number of bytes enqueued from data, =n if there is enough space
         */
        u32 enqueue(const u8* data, u32 n);
    };
}

#endif //VO_CMPE460_CIRCULAR_BUFFER_H
