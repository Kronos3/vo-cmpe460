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
    : m_filename(filename), m_num_frames(num_frame)
    {
        m_frames.reserve(m_num_frames);
    }

    bool VisRecord::is_finished() const
    {
        return m_current_frame >= m_num_frames;
    }

    U32 VisRecord::get_count() const
    {
        return m_current_frame;
    }

    U32 VisRecord::operator++()
    {
        return m_current_frame++;
    }

    void VisRecord::append(VisRecord::Type type, std::vector<F32> record)
    {
        // We can't record anything is there is no more space in the record
        if (is_finished()) return;

        m_frames[m_current_frame][type] = std::move(record);
    }

    Os::File::Status VisRecord::write() const
    {
        Os::File fp;
        Os::File::Status status = fp.open(m_filename.toChar(), Os::File::OPEN_CREATE);
        if (status != Os::File::OP_OK)
            return status;

        // Write the file header describing the metadata
        NATIVE_INT_TYPE size = sizeof(m_current_frame);
        status = fp.write(&m_current_frame, size);
        if (status != Os::File::OP_OK)
            return status;

        // Write each frame
        for (const auto& frame : m_frames)
        {
            // Write the number of records for this frame
            // We can use U8 because of the assertion at the top of this file
            U8 size_raw = frame.size();
            size = sizeof(size_raw);
            status = fp.write(&size_raw, size);
            if (status != Os::File::OP_OK)
                return status;

            // Write each record

        }
    }
}
