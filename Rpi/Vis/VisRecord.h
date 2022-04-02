//
// Created by tumbar on 3/31/22.
//

#ifndef VO_CMPE460_VISRECORD_H
#define VO_CMPE460_VISRECORD_H

#include <Fw/Types/String.hpp>
#include <Os/File.hpp>

#include <unordered_map>
#include <vector>
#include "Fw/Buffer/Buffer.hpp"
#include <memory>

namespace Rpi
{
    class VisRecord
    {
    public:
        using Label = U16;

        /**
         * Write constructor
         * @param num_frame number of frame to record over
         * @param filename to write record down to
         */
        VisRecord(U32 num_frame, const Fw::String& filename);

        /**
         * Read constructor
         * @param filename filename to read from
         */
        explicit VisRecord(const Fw::String& filename);

        Os::File::Status write() const;
        Os::File::Status read();

        enum Type
        {
            CHESSBOARD_CORNERS,     //!< 2F point
            FISHEYE_K,              //!< 3 x 3 matrix
            FISHEYE_D,              //!< 4 x 1 matrix
            VisRecord_Max,
        };

        bool is_finished() const;
        U32 get_current() const;
        U32 get_total() const;
        void retry();
        bool frame_failed() const;
        void increment();

        const Fw::StringBase& get_filename() const;

        void append(Label label, Type type, std::unique_ptr<F32[]> data, U32 length);
        F32* get(Label label, Type type, U32& length) const;

    private:
        std::unordered_map<U32, std::unique_ptr<F32[]>> m_records;
        std::unordered_map<U32, U32> m_length;
        Fw::String m_filename;
        U32 m_current_frame;
        U32 m_num_frames;

        bool m_retry_frame;
    };
}

#endif //VO_CMPE460_VISRECORD_H
