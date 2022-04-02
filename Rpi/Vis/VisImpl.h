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
        void RECORD_cmdHandler(U32 opCode, U32 cmdSeq, U32 frame_count, const Fw::CmdStringArg &filename) override;
        void RECORD_TIMEOUT_cmdHandler(U32 opCode, U32 cmdSeq) override;

    PRIVATE:
        VisListener m_listener;

        Os::Mutex m_state_mutex;

        VisRecord* m_recording;
        VisPipelineStage* m_pipeline;
        VisPipelineStage* m_pipeline_last;
    };
}

#endif //VO_CMPE460_VISIMPL_H
