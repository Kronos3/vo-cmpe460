
#ifndef VO_CMPE460_CAMIMPL_H
#define VO_CMPE460_CAMIMPL_H

#include <Rpi/Cam/CamComponentAc.hpp>
#include "CamCfg.h"
#include "CameraConfig.h"
#include <core/completed_request.hpp>
#include <queue>
#include <mutex>

namespace Rpi
{
    class LibcameraApp;
    class CamImpl : public CamComponentBase
    {
//        friend LibcameraApp;

    public:
        explicit CamImpl(const char* compName);
        ~CamImpl() override;

        void init(
                NATIVE_INT_TYPE instance, /*!< The instance number*/
                I32 width, I32 height,
                I32 rotation,
                bool vflip, bool hflip
        );

        void startStreamThread(const Fw::StringBase &name);
        void quitStreamThread();

    PRIVATE:
        void get_config(CameraConfig& config);
        void deallocate_handler(NATIVE_INT_TYPE portNum, CamFrame* frame) override;

        void CAPTURE_cmdHandler(U32 opCode, U32 cmdSeq, const Fw::CmdStringArg &destination) override;
        void CONFIGURE_cmdHandler(U32 opCode, U32 cmdSeq) override;
        void STOP_cmdHandler(U32 opCode, U32 cmdSeq) override;
        void START_cmdHandler(U32 opCode, U32 cmdSeq) override;
        void STREAM_cmdHandler(U32 opCode, U32 cmdSeq, CamListener listener) override;

        CamFrame* get_buffer();

        static void streaming_thread_entry(void* this_);
        void streaming_thread();

    PRIVATE:
        std::mutex m_buffer_mutex;
        CamFrame m_buffers[CAMERA_BUFFER_N];

        CamListener m_streaming_to;

        LibcameraApp* m_camera;
        Os::Task m_task;

        U32 tlm_dropped;
        U32 tlm_captured;
    };
}

#endif //VO_CMPE460_CAMIMPL_H
