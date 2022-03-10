#ifndef VO_CMPE460_CIRCULAR_BUFFER_H
#define VO_CMPE460_CIRCULAR_BUFFER_H

#include <Fw/Types/BasicTypes.hpp>

namespace Fw
{
    class CircularBuffer
    {
        U8* m_buf;
        U32 m_size;

        // Circular buffer part
        U32 m_head;
        U32 m_tail;

    public:
        CircularBuffer();

        /**
         * Initialize a circular buffer with the
         * underlying straight buffer
         * @param buf normal memory
         * @param size length of linear memory
         */
        void set(U8* buf, U32 size);
        CircularBuffer(U8* buf, U32 size);

        /**
         * @return Number of bytes currently enqueued
         */
        U32 length() const;

        /**
         * @return Total number of bytes in underlying buffer
         */
        U32 capacity() const;

        /**
         * Get the next pointer and as many bytes as possible in
         * one straight shot. This can be used to flush the queue
         * as fast as possible.
         * @param ptr_ref pointer to start of output sector
         * @param size length of sector
         * @param mutex mutex to hold buffer to
         * @return TRUE if there are more sectors remaining in the buffer
         */
        bool deqeue_next_sector(const U8*& ptr_ref, U32& size, U32& mutex) const;

        /**
         * Release a mutex while data is being used from the buffer
         * @param mutex future value of tail from deqeue_next_sector
         */
       void release(U32 mutex);

        /**
         * Enqueue data to the circular buffer
         * @param data data to enqueue
         * @param n length data
         * @return number of bytes enqueued from data, =n if there is enough space
         */
        U32 enqueue(const U8* data, U32 n);
    };
}

#endif //VO_CMPE460_CIRCULAR_BUFFER_H
