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
        void RECORD_CANCEL_IF_RUNNING_cmdHandler(U32 opCode, U32 cmdSeq) override;

    PRIVATE:
        void clear_recording();

    PRIVATE:
        VisListener m_listener;

        Os::Mutex m_state_mutex;

        bool m_is_file_recording;
        Fw::String m_recording_file;

        std::shared_ptr<VisRecord> m_recording;
        std::shared_ptr<VisPipelineStage> m_pipeline;
        VisPipelineStage* m_pipeline_last;
    };
}

#endif //VO_CMPE460_VISIMPL_H
