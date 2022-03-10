// ======================================================================
// \title  Queue.cpp
// \author mstarch, borrowed from @dinkel
// \brief  Queue implementation for Baremetal devices. No IPC nor thread
//         safety is implemented as this intended for baremetal devices.
//         Based on Os/Pthreads/Queue.cpp from @dinkel
// ======================================================================
#include <Fw/Types/BasicTypes.hpp>
#include <Fw/Types/Assert.hpp>
#include <Os/Queue.hpp>

#include <src/RPI/kernel.h>
#include <circle/sched/scheduler.h>

#include <Os/RPIQueue.h>

namespace Os
{
    Queue::Queue() :
            m_handle(static_cast<POINTER_CAST>(NULL))
    {}

    Queue::QueueStatus Queue::createInternal(const Fw::StringBase &name, NATIVE_INT_TYPE depth, NATIVE_INT_TYPE msgSize)
    {
        (void) name;

        auto* handle = reinterpret_cast<RPIQueue*>(this->m_handle);

        // Queue has already been created... remove it and try again:
        delete handle;

        //New queue handle, check for success or return error
        handle = new RPIQueue;
        if (!handle->create(depth, msgSize))
        {
            return QUEUE_UNINITIALIZED;
        }

        //Set handle member variable
        m_handle = reinterpret_cast<POINTER_CAST>(handle);

        //Register the queue
#if FW_QUEUE_REGISTRATION
        if (Os::Queue::s_queueRegistry)
        {
            Os::Queue::s_queueRegistry->regQueue(this);
        }
#endif
        return QUEUE_OK;
    }

    Queue::~Queue()
    {
        // Clean up the queue handle:
        auto* handle = reinterpret_cast<RPIQueue*>(this->m_handle);
        if (nullptr != handle)
        {
            CMemorySystem::HeapFree(handle);
        }
        this->m_handle = static_cast<POINTER_CAST>(NULL);
    }

    Queue::QueueStatus
    bareSendNonBlock(RPIQueue* queue, const U8* buffer, NATIVE_INT_TYPE size, NATIVE_INT_TYPE priority)
    {
        Queue::QueueStatus status = Queue::QUEUE_OK;
        // Push item onto queue:
        bool success = queue->push(buffer, size, priority);
        if (!success)
        {
            status = Queue::QUEUE_FULL;
        }
        return status;
    }

    Queue::QueueStatus
    bareSendBlock(RPIQueue* queue, const U8* buffer, NATIVE_INT_TYPE size, NATIVE_INT_TYPE priority)
    {
        // If the queue is full, wait until a message is taken off the queue.
        while (queue->isFull())
        {
            CScheduler::Get()->Yield();
        }

        // Push item onto queue:
        bool success = queue->push(buffer, size, priority);
        // The only reason push would not succeed is if the queue
        // was full. Since we waited for the queue to NOT be full
        // before sending on the queue, the push must have succeeded
        // unless there was a programming error or a bit flip.
        FW_ASSERT(success, success);
        return Queue::QUEUE_OK;
    }

    Queue::QueueStatus
    Queue::send(const U8* buffer, NATIVE_INT_TYPE size, NATIVE_INT_TYPE priority, QueueBlocking block)
    {
        //Check if the handle is null or check the underlying queue is null
        auto* queue = reinterpret_cast<RPIQueue*>(this->m_handle);
        if (nullptr == queue)
        {
            return QUEUE_UNINITIALIZED;
        }

        //Check that the buffer is non-null
        if (nullptr == buffer)
        {
            return QUEUE_EMPTY_BUFFER;
        }

        //Fail if there is a size miss-match
        if (size < 0 || (NATIVE_UINT_TYPE) size > queue->getMsgSize())
        {
            return QUEUE_SIZE_MISMATCH;
        }
        //Send to the queue
        if (QUEUE_NONBLOCKING == block)
        {
            return bareSendNonBlock(queue, buffer, size, priority);
        }

        return bareSendBlock(queue, buffer, size, priority);
    }

    Queue::QueueStatus
    bareReceiveNonBlock(RPIQueue* queue, U8* buffer, NATIVE_INT_TYPE capacity, NATIVE_INT_TYPE &actualSize,
                        NATIVE_INT_TYPE &priority)
    {
        FW_ASSERT(buffer);
        NATIVE_UINT_TYPE size = capacity;
        NATIVE_INT_TYPE pri = 0;
        Queue::QueueStatus status = Queue::QUEUE_OK;

        // Get an item off of the queue:
        bool success = queue->pop(buffer, size, pri);
        if (success)
        {
            // Pop worked - set the return size and priority:
            actualSize = (NATIVE_INT_TYPE) size;
            priority = pri;
        }
        else
        {
            actualSize = 0;
            if (size > (NATIVE_UINT_TYPE) capacity)
            {
                // The buffer capacity was too small!
                status = Queue::QUEUE_SIZE_MISMATCH;
            }
            else if (size == 0)
            {
                // The queue is empty:
                status = Queue::QUEUE_NO_MORE_MSGS;
            }
            else
            {
                // If this happens, a programming error or bit flip occurred:
                FW_ASSERT(0);
            }
        }
        return status;
    }

    Queue::QueueStatus
    bareReceiveBlock(RPIQueue* queue, U8* buffer, NATIVE_INT_TYPE capacity, NATIVE_INT_TYPE &actualSize,
                     NATIVE_INT_TYPE &priority)
    {
        FW_ASSERT(buffer);
        NATIVE_UINT_TYPE size = capacity;
        NATIVE_INT_TYPE pri = 0;
        Queue::QueueStatus status = Queue::QUEUE_OK;

        // If the queue is full, wait until a message is taken off the queue.
        while (queue->isEmpty())
        {
            // Yield to the OS
            CScheduler::Get()->Yield();
        }

        // Get an item off of the queue:
        bool success = queue->pop(buffer, size, pri);
        if (success)
        {
            // Pop worked - set the return size and priority:
            actualSize = (NATIVE_INT_TYPE) size;
            priority = pri;
        }
        else
        {
            actualSize = 0;
            if (size > (NATIVE_UINT_TYPE) capacity)
            {
                // The buffer capacity was too small!
                status = Queue::QUEUE_SIZE_MISMATCH;
            }
            else
            {
                // If this happens, a programming error or bit flip occurred:
                // The only reason a pop should fail is if the user's buffer
                // was too small.
                FW_ASSERT(0);
            }
        }
        return status;
    }

    Queue::QueueStatus
    Queue::receive(U8* buffer, NATIVE_INT_TYPE capacity, NATIVE_INT_TYPE &actualSize, NATIVE_INT_TYPE &priority,
                   QueueBlocking block)
    {
        //Check if the handle is null or check the underlying queue is null
        auto* handle = reinterpret_cast<RPIQueue*>(this->m_handle);
        if (nullptr == handle)
        {
            return QUEUE_UNINITIALIZED;
        }

        //Check the buffer can hold the out-going message
        if (capacity < this->getMsgSize())
        {
            return QUEUE_SIZE_MISMATCH;
        }

        //Receive either non-blocking or blocking
        if (QUEUE_NONBLOCKING == block)
        {
            return bareReceiveNonBlock(handle, buffer, capacity, actualSize, priority);
        }

        return bareReceiveBlock(handle, buffer, capacity, actualSize, priority);
    }

    NATIVE_INT_TYPE Queue::getNumMsgs(void) const
    {
        //Check if the handle is null or check the underlying queue is null
        if (NULL == reinterpret_cast<RPIQueue*>(this->m_handle))
        {
            return 0;
        }

        auto* queue = reinterpret_cast<RPIQueue*>(this->m_handle);
        return queue->getCount();
    }

    NATIVE_INT_TYPE Queue::getMaxMsgs(void) const
    {
        //Check if the handle is null or check the underlying queue is null
        if (nullptr == reinterpret_cast<RPIQueue*>(this->m_handle))
        {
            return 0;
        }

        auto* queue = reinterpret_cast<RPIQueue*>(this->m_handle);
        return queue->getMaxCount();
    }

    NATIVE_INT_TYPE Queue::getQueueSize(void) const
    {
        //Check if the handle is null or check the underlying queue is null
        if (nullptr == reinterpret_cast<RPIQueue*>(this->m_handle))
        {
            return 0;
        }
        auto* queue = reinterpret_cast<RPIQueue*>(this->m_handle);
        return queue->getDepth();
    }

    NATIVE_INT_TYPE Queue::getMsgSize(void) const
    {
        //Check if the handle is null or check the underlying queue is null
        if (nullptr == reinterpret_cast<RPIQueue*>(this->m_handle))
        {
            return 0;
        }

        auto* queue = reinterpret_cast<RPIQueue*>(this->m_handle);
        return queue->getMsgSize();
    }
}//Namespace Os
