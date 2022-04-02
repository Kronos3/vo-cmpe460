//
// Created by tumbar on 3/31/22.
//

#include "VisRecord.h"
#include <Fw/Types/Assert.hpp>

namespace Rpi
{
    // Make sure we can fit ids in U8
    FW_STATIC_ASSERT(VisRecord::VisRecord_Max < (1 << (sizeof(U8) * 8)));

    VisRecord::VisRecord(U32 num_frame, const Fw::String &filename)
    : m_filename(filename), m_current_frame(0), m_num_frames(num_frame),
      m_retry_frame(false)
    {
    }

    bool VisRecord::is_finished() const
    {
        return m_current_frame >= m_num_frames;
    }

    U32 VisRecord::get_current() const
    {
        return m_current_frame;
    }

    U32 VisRecord::get_total() const
    {
        return m_num_frames;
    }

    void VisRecord::retry()
    {
        m_retry_frame = true;
    }

    bool VisRecord::frame_failed() const
    {
        return m_retry_frame;
    }

    void VisRecord::increment()
    {
        if (m_retry_frame)
        {
            m_retry_frame = false;
        }
        else
        {
            m_current_frame++;
        }
    }

    void VisRecord::append(Label label,
                           VisRecord::Type type,
                           std::unique_ptr<F32[]> data,
                           U32 length)
    {
        U32 key = (label << 16) & (type & 0xFFFF);
        m_records[key] = std::move(data);
        m_length[key] = length;
    }

    Os::File::Status VisRecord::write() const
    {
        Os::File fp;
        Os::File::Status status = fp.open(m_filename.toChar(), Os::File::OPEN_CREATE);
        if (status != Os::File::OP_OK)
            return status;

        // Write the file header describing the metadata
        U32 num_records = m_records.size();
        NATIVE_INT_TYPE size = sizeof(num_records);
        status = fp.write(&num_records, size);
        if (status != Os::File::OP_OK)
            return status;

        size = sizeof(m_num_frames);
        status = fp.write(&m_num_frames, size);
        if (status != Os::File::OP_OK)
            return status;

        for (const auto& record : m_records)
        {
            U32 id = record.first;
            size = sizeof(id);
            status = fp.write(&id, size);
            if (status != Os::File::OP_OK)
                return status;

            U32 length = m_length.at(id);
            size = sizeof(length);
            status = fp.write(&length, size);
            if (status != Os::File::OP_OK)
                return status;

            size = (NATIVE_INT_TYPE)(sizeof(F32) * length);
            status = fp.write(m_records.at(id).get(), size);
            if (status != Os::File::OP_OK)
                return status;
        }

        fp.close();
        return Os::File::OP_OK;
    }

    VisRecord::VisRecord(const Fw::String &filename)
    : m_filename(filename),
    m_current_frame(0),
    m_num_frames(0),
    m_retry_frame(false)
    {
    }

    Os::File::Status VisRecord::read()
    {
        Os::File fp;
        Os::File::Status status = fp.open(m_filename.toChar(), Os::File::OPEN_READ);
        if (status != Os::File::OP_OK)
            return status;

        // Read the file header describing the metadata
        U32 num_records = m_records.size();
        NATIVE_INT_TYPE size = sizeof(num_records);
        status = fp.read(&num_records, size);
        if (status != Os::File::OP_OK)
            return status;

        size = sizeof(m_num_frames);
        status = fp.read(&m_num_frames, size);
        if (status != Os::File::OP_OK)
            return status;

        for (U32 record_i = 0; record_i < num_records; record_i++)
        {
            U32 id;
            size = sizeof(id);
            status = fp.read(&id, size);
            if (status != Os::File::OP_OK)
                return status;

            U32 length;
            size = sizeof(length);
            status = fp.read(&length, size);
            if (status != Os::File::OP_OK)
                return status;

            std::unique_ptr<F32[]> ptr = std::unique_ptr<F32[]>(new F32[length]);
            size = (NATIVE_INT_TYPE)(sizeof(F32) * length);
            status = fp.read(ptr.get(), size);
            if (status != Os::File::OP_OK)
                return status;

            m_records[id] = std::move(ptr);
            m_length[id] = length;
        }

        fp.close();
        return Os::File::OP_OK;
    }

    F32* VisRecord::get(Label label, VisRecord::Type type, U32& length) const
    {
        U32 key = (label << 16) & (type & 0xFFFF);
        if (m_records.find(key) == m_records.end())
        {
            // Not found
            length = 0;
            return nullptr;
        }

        length = m_length.at(key);
        return m_records.at(key).get();
    }

    const Fw::StringBase &VisRecord::get_filename() const
    {
        return m_filename;
    }
}
