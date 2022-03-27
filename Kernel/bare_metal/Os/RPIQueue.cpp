// ======================================================================
// \title  BufferQueueCommon.hpp
// \author dinkel
// \brief  This file implements some of the methods for the generic
//         buffer queue data structure declared in BufferQueue.hpp that
//         are common amongst different queue implementations.
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#include "Os/RPIQueue.h"
#include <Fw/Types/Assert.hpp>
#include <string.h>
#include <circle/memory.h>
#include <circle/sched/synchronizationevent.h>

namespace Os {

    struct FIFOQueue {
        U8* data;
        NATIVE_UINT_TYPE head;
        NATIVE_UINT_TYPE tail;
        CSynchronizationEvent enqueue_event;
        CSynchronizationEvent dequeue_event;
    };

    /////////////////////////////////////////////////////
    // Class functions:
    /////////////////////////////////////////////////////

    RPIQueue::RPIQueue() {
        // Set member variables:
        this->queue = NULL;
        this->msgSize = 0;
        this->depth = 0;
        this->count = 0;
        this->maxCount = 0;
    }

    RPIQueue::~RPIQueue() {
        this->finalize();
    }

    bool RPIQueue::create(NATIVE_UINT_TYPE depth, NATIVE_UINT_TYPE msgSize) {
        // Queue is already set up. destroy it and try again:
        if (NULL != this->queue) {
            this->finalize();
        }
        FW_ASSERT(NULL == this->queue, (POINTER_CAST) this->queue);

        // Set member variables:
        this->msgSize = msgSize;
        this->depth = depth;
        return this->initialize(depth, msgSize);
    }

    bool RPIQueue::push(const U8* buffer, NATIVE_UINT_TYPE size, NATIVE_INT_TYPE priority) {

        FW_ASSERT(size <= this->msgSize);
        if( this->isFull() ) {
            return false;
        }

        // Enqueue the data:
        bool ret = enqueue(buffer, size, priority);
        if( !ret ) {
            return false;
        }

        // Increment count:
        ++this->count;
        if( this->count > this->maxCount ) {
            this->maxCount = this->count;
        }

        // Notify any waiting tasks
        reinterpret_cast<FIFOQueue*>(this->queue)->enqueue_event.Set();
        return true;
    }

    bool RPIQueue::pop(U8* buffer, NATIVE_UINT_TYPE& size, NATIVE_INT_TYPE &priority) {

        if( this->isEmpty() ) {
            size = 0;
            return false;
        }

        // Dequeue the data:
        bool ret = dequeue(buffer, size, priority);
        if( !ret ) {
            return false;
        }

        // Decrement count:
        --this->count;

        // Notify any waiting tasks
        reinterpret_cast<FIFOQueue*>(this->queue)->dequeue_event.Set();
        return true;
    }

    bool RPIQueue::isFull() {
        return (this->count == this->depth);
    }

    bool RPIQueue::isEmpty() {
        return (this->count == 0);
    }

    NATIVE_UINT_TYPE RPIQueue::getCount() {
        return this->count;
    }

    NATIVE_UINT_TYPE RPIQueue::getMaxCount() {
        return this->maxCount;
    }


    NATIVE_UINT_TYPE RPIQueue::getMsgSize() {
        return this->msgSize;
    }

    NATIVE_UINT_TYPE RPIQueue::getDepth() {
        return this->depth;
    }

    NATIVE_UINT_TYPE RPIQueue::getBufferIndex(NATIVE_INT_TYPE index) {
        return (index % this->depth) * (sizeof(NATIVE_INT_TYPE) + this->msgSize);
    }

    void RPIQueue::enqueueBuffer(const U8* buffer, NATIVE_UINT_TYPE size, U8* data, NATIVE_UINT_TYPE index) {
        // Copy size of buffer onto queue:
        void* dest = &data[index];
        void* ptr = memcpy(dest, &size, sizeof(size));
        FW_ASSERT(ptr == dest);

        // Copy buffer onto queue:
        index += sizeof(size);
        dest = &data[index];
        ptr = memcpy(dest, buffer, size);
        FW_ASSERT(ptr == dest);
    }

    bool RPIQueue::dequeueBuffer(U8* buffer, NATIVE_UINT_TYPE& size, U8* data, NATIVE_UINT_TYPE index) {
        // Copy size of buffer from queue:
        NATIVE_UINT_TYPE storedSize;
        void* source = &data[index];
        void* ptr = memcpy(&storedSize, source, sizeof(size));
        FW_ASSERT(ptr == &storedSize);

        // If the buffer passed in is not big
        // enough, return false, and pass out
        // the size of the message:
        if(storedSize > size){
            size = storedSize;
            return false;
        }
        size = storedSize;

        // Copy buffer from queue:
        index += sizeof(size);
        source = &data[index];
        ptr = memcpy(buffer, source, storedSize);
        FW_ASSERT(ptr == buffer);
        return true;
    }

    /////////////////////////////////////////////////////
    // Class functions:
    /////////////////////////////////////////////////////

    bool RPIQueue::initialize(NATIVE_UINT_TYPE depth, NATIVE_UINT_TYPE msgSize) {
        U8* data = new U8[depth * (sizeof(msgSize) + msgSize)];
        auto* fifoQueue = new FIFOQueue;

        fifoQueue->data = data;
        fifoQueue->head = 0;
        fifoQueue->tail = 0;
        this->queue = fifoQueue;
        return true;
    }

    void RPIQueue::finalize() {
        FIFOQueue* fQueue = static_cast<FIFOQueue*>(this->queue);
        if (NULL != fQueue)
        {
            U8* data = fQueue->data;
            if (NULL != data) {
                CMemorySystem::HeapFree(data);
            }
            CMemorySystem::HeapFree(fQueue);
        }
        this->queue = NULL;
    }

    bool RPIQueue::enqueue(const U8* buffer, NATIVE_UINT_TYPE size, NATIVE_INT_TYPE priority) {
        (void) priority;

        FIFOQueue* fQueue = static_cast<FIFOQueue*>(this->queue);
        U8* data = fQueue->data;

        // Store the buffer to the queue:
        NATIVE_UINT_TYPE index = getBufferIndex(fQueue->tail);
        this->enqueueBuffer(buffer, size, data, index);

        // Increment tail of fifo:
        ++fQueue->tail;
        return true;
    }

    bool RPIQueue::dequeue(U8* buffer, NATIVE_UINT_TYPE& size, NATIVE_INT_TYPE &priority) {
        (void) priority;

        FIFOQueue* fQueue = static_cast<FIFOQueue*>(this->queue);
        U8* data = fQueue->data;

        // Get the buffer from the queue:
        NATIVE_UINT_TYPE index = getBufferIndex(fQueue->head);
        bool ret = this->dequeueBuffer(buffer, size, data, index);
        if(!ret) {
            return false;
        }

        // Increment head of fifo:
        ++fQueue->head;
        return true;
    }

    void RPIQueue::waitOnFull()
    {
        while(isFull())
        {
            // Wait for someone to dequeue
            reinterpret_cast<FIFOQueue*>(this->queue)->dequeue_event.Wait();
            reinterpret_cast<FIFOQueue*>(this->queue)->dequeue_event.Clear();
        }
    }

    void RPIQueue::waitOnEmpty()
    {
        while(isEmpty())
        {
            // Wait for someone to enqueue
            reinterpret_cast<FIFOQueue*>(this->queue)->enqueue_event.Wait();
            reinterpret_cast<FIFOQueue*>(this->queue)->enqueue_event.Clear();
        }
    }
}

