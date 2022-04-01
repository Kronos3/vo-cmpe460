//
// Created by tumbar on 3/31/22.
//

#ifndef VO_CMPE460_VISRECORD_H
#define VO_CMPE460_VISRECORD_H

#include <Fw/Types/String.hpp>
#include <Os/File.hpp>

#include <unordered_map>
#include <vector>

namespace Rpi
{
    class VisRecord
    {
    public:
        VisRecord(U32 num_frame, const Fw::String& filename);
        Os::File::Status write() const;

        enum Type
        {
            CHESSBOARD_CORNER,      //!< 2F point
            FISHEYE_K,              //!< 3 x 3 matrix
            FISHEYE_D,              //!< 4 x 1 matrix
            VisRecord_Max,
        };

        bool is_finished() const;
        U32 get_count() const;
        U32 operator++();

        void append(Type type, std::vector<F32> record);

    private:
        using FrameRecord = std::unordered_map<Type, std::vector<F32>>;

        Fw::String m_filename;
        U32 m_current_frame;
        U32 m_num_frames;
        std::vector<FrameRecord> m_frames;
    };

}

#endif //VO_CMPE460_VISRECORD_H
