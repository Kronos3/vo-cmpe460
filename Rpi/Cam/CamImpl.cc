
#include "CamImpl.h"
#include <core/libcamera_app.h>

namespace Rpi
{

    CamImpl::CamImpl(const char* compName)
            : CamComponentBase(compName),
              m_streaming_to(CamListener::NONE),
              m_camera(new LibcameraApp()),
              tlm_dropped(0),
              tlm_captured(0)
    {
    }

    void CamImpl::init(NATIVE_INT_TYPE instance,
                       I32 width, I32 height,
                       I32 rotation,
                       bool vflip, bool hflip)
    {
        CamComponentBase::init(instance);

        for (auto& buffer : m_buffers)
        {
            buffer.register_callback([this](CompletedRequest* cr) { m_camera->queueRequest(cr); });
        }

        m_camera->OpenCamera(0);
        m_camera->ConfigureCameraStream(width, height, rotation,
                                        hflip, vflip);
    }

    CamImpl::~CamImpl()
    {
        delete m_camera;
    }

    void CamImpl::streaming_thread()
    {
        while (true)
        {
            LibcameraApp::Msg msg = m_camera->Wait();
            if (msg.type == LibcameraApp::MsgType::Quit)
            {
                break;
            }

            FW_ASSERT(msg.type == LibcameraApp::MsgType::RequestComplete, (I32)msg.type);

            tlm_captured++;
            tlmWrite_FramesCapture(tlm_captured);

            // Check if anyone is listening to our stream
            if (m_streaming_to == CamListener::NONE)
            {
                // Nobody is listening
                // Drop the frame
                m_camera->queueRequest(msg.payload);
                continue;
            }

            // Get an internal frame buffer
            CamFrame* buffer = get_buffer();
            if (!buffer)
            {
                // Ran out of frame buffers
                tlm_dropped++;
                tlmWrite_FramesDropped(tlm_dropped);
                m_camera->queueRequest(msg.payload);
                continue;
            }

            buffer->request = msg.payload;

            // Get the DMA buffer
            buffer->buffer = msg.payload->buffers[m_camera->RawStream()];
            FW_ASSERT(buffer->buffer);

            // Get the userland pointer
            buffer->span = m_camera->Mmap(buffer->buffer)[0];
            buffer->info = m_camera->GetStreamInfo(m_camera->RawStream());

            // Send the frame to the requester on the same port
            frame_out(m_streaming_to.e, buffer);
        }

        log_ACTIVITY_LO_CameraStopping();
        m_camera->StopCamera();
    }

    CamFrame* CamImpl::get_buffer()
    {
        m_buffer_mutex.lock();
        for (auto &m_buffer : m_buffers)
        {
            if (!m_buffer.in_use())
            {
                m_buffer.incref();
                m_buffer_mutex.unlock();
                return &m_buffer;
            }
        }

        m_buffer_mutex.unlock();
        return nullptr;
    }

    void CamImpl::CAPTURE_cmdHandler(U32 opCode, U32 cmdSeq, const Fw::CmdStringArg &destination)
    {
        // Capture an image and save it to a file
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
    }

    void CamImpl::get_config(CameraConfig &config)
    {
        Fw::ParamValid valid;

#define GET_PARAM(type, pname, vname) \
        do { \
            auto temp = static_cast<type>(paramGet_##pname(valid)); \
            if (valid == Fw::PARAM_VALID) \
            { \
                config.vname = temp; \
            } \
        } while(0)

        GET_PARAM(U32, FRAME_RATE, frame_rate);
        GET_PARAM(U32, EXPOSURE_TIME, exposure_time);
        GET_PARAM(F32, GAIN, gain);
        GET_PARAM(CameraConfig::AeMeteringModeEnum, METERING_MODE, metering_mode);
        GET_PARAM(CameraConfig::AeExposureModeEnum, EXPOSURE_MODE, exposure_mode);
        GET_PARAM(CameraConfig::AwbModeEnum, AWB, awb);
        GET_PARAM(F32, EV, ev);
        GET_PARAM(F32, AWB_GAIN_R, awb_gain_r);
        GET_PARAM(F32, AWB_GAIN_B, awb_gain_b);
        GET_PARAM(F32, BRIGHTNESS, brightness);
        GET_PARAM(F32, CONTRAST, contrast);
        GET_PARAM(F32, SATURATION, saturation);
        GET_PARAM(F32, SHARPNESS, sharpness);
    }

    void
    CamImpl::startStreamThread(const Fw::StringBase &name)
    {
        m_task.start(name, streaming_thread_entry, this);
    }

    void CamImpl::streaming_thread_entry(void* this_)
    {
        reinterpret_cast<CamImpl*>(this_)->streaming_thread();
    }

    void CamImpl::quitStreamThread()
    {
        // Wait for the camera to exit
        m_camera->Quit();
        m_task.join(nullptr);

        m_camera->StopCamera();
    }

    void CamImpl::STOP_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        log_ACTIVITY_LO_CameraStopping();
        m_camera->StopCamera();
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void CamImpl::START_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        m_camera->StopCamera();

        CameraConfig config;
        get_config(config);
        log_ACTIVITY_LO_CameraConfiguring();
        m_camera->ConfigureCamera(config);

        log_ACTIVITY_LO_CameraStarting();
        m_camera->StartCamera();
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void CamImpl::STREAM_cmdHandler(U32 opCode, U32 cmdSeq, CamListener listener)
    {
        if (m_streaming_to != CamListener::NONE)
        {
            log_ACTIVITY_LO_CameraStreamStopping(m_streaming_to);
        }

        if (listener != CamListener::NONE)
        {
            log_ACTIVITY_LO_CameraStreamStarting(listener);
        }

        m_streaming_to = listener;
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void CamImpl::parametersLoaded()
    {
        CamComponentBase::parametersLoaded();

        CameraConfig config;
        get_config(config);
        log_ACTIVITY_LO_CameraConfiguring();
        m_camera->ConfigureCamera(config);

    }
}
