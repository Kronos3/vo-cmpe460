#ifndef VO_CMPE460_VISIMPL_H
#define VO_CMPE460_VISIMPL_H

#include <Rpi/Vis/VisComponentAc.hpp>
#include <Rpi/Vis/VisPipeline.h>
#include "VisRecord.h"

namespace Rpi
{
    class VisImpl : public VisComponentBase
    {
    public:
        explicit VisImpl(const char* compName);

        void init(NATIVE_INT_TYPE queue_depth, NATIVE_INT_TYPE instance);

    PRIVATE:

        void frame_handler(NATIVE_INT_TYPE portNum, CamFrame *frame) override;
        void STREAM_cmdHandler(U32 opCode, U32 cmdSeq, VisListener listener) override;
        void CLEAR_cmdHandler(U32 opCode, U32 cmdSeq) override;
        void PIPE_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::VisPipe stage) override;
        void RECORD_FILE_cmdHandler(U32 opCode, U32 cmdSeq, VisRecordType record_type, U32 frame_count, const Fw::CmdStringArg &filename) override;
        void RECORD_WAIT_cmdHandler(U32 opCode, U32 cmdSeq) override;
        void MASK_QUAD_cmdHandler(U32 opCode, U32 cmdSeq, U8 color, F32 c1x, F32 c1y, F32 c2x, F32 c2y, F32 c3x, F32 c3y, F32 c4x, F32 c4y) override;

    PRIVATE:
        void clear_recording();

    PRIVATE:
        VisListener m_listener;

        Os::Mutex m_state_mutex;

        bool m_is_file_recording;
        Fw::String m_recording_file;

        bool cmd_waiting;
        U32 waiting_opCode;
        U32 waiting_cmdSeq;

        std::shared_ptr<VisRecord> m_recording;
        std::shared_ptr<VisPipelineStage> m_pipeline;
        VisPipelineStage* m_pipeline_last;

        U32 tlm_frame_in;
        U32 tlm_frame_failed;
        U32 tlm_frame_processed;
    };
}

#endif //VO_CMPE460_VISIMPL_H
