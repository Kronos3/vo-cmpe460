
#include "CamImpl.h"
#include <core/libcamera_app.h>

namespace Rpi
{

    CamImpl::CamImpl(const char* compName)
            : CamComponentBase(compName),
              m_streaming_to(-1),
              m_camera(new LibcameraApp()),
              tlm_dropped(0), tlm_dropped_internal(0),
              tlm_captured(0)
    {
    }

    void CamImpl::init(NATIVE_INT_TYPE instance,
                       I32 width, I32 height,
                       I32 rotation,
                       bool vflip, bool hflip)
    {
        CamComponentBase::init(instance);

        m_camera->OpenCamera(0);
        m_camera->ConfigureCameraStream(width, height, rotation,
                                        hflip, vflip);

        log_ACTIVITY_HI_CameraConfiguring();
        CameraConfig cfg;
        get_config(cfg);
        m_camera->ConfigureCamera(cfg);
    }

    CamImpl::~CamImpl()
    {
        delete m_camera;
    }

    void CamImpl::streaming_thread()
    {
        log_ACTIVITY_HI_CameraStarting();
        m_camera->StartCamera();

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
            if (m_streaming_to == -1)
            {
                // Nobody is listening
                // Drop the frame
                m_camera->queueRequest(msg.payload);
                continue;
            }

            // Get an internal frame buffer
            auto completed_request = msg.payload;
            CamFrame* buffer = get_buffer();
            if (!buffer)
            {
                // Ran out of frame buffers
                tlm_dropped_internal++;
                tlmWrite_FramesDroppedInternal(tlm_dropped_internal);
                return;
            }

            buffer->request = completed_request;

            // Get the DMA buffer
            buffer->buffer = completed_request->buffers[m_camera->RawStream()];
            FW_ASSERT(m_buffers->buffer);

            // Get the userland pointer
            buffer->span = m_camera->Mmap(m_buffers->buffer)[0];
            buffer->info = m_camera->GetStreamInfo(m_camera->RawStream());

            // Send the frame to the requester on the same port
            frame_out(m_streaming_to, buffer);
        }

        log_ACTIVITY_HI_CameraStopping();
        m_camera->StopCamera();
    }

    void CamImpl::deallocate_handler(NATIVE_INT_TYPE portNum, CamFrame* frame)
    {
        // Return the buffer back to the camera to request another frame
        m_camera->queueRequest(frame->request);
        frame->clear();
    }

    void CamImpl::CAPTURE_cmdHandler(U32 opCode, U32 cmdSeq, const Fw::CmdStringArg &destination)
    {
        // Capture an image and save it to a file
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
        return;

//        Frame frame;
//        bool valid = m_camera->read_frame(frame);

        // Make sure we have enough buffers
//        if (!valid)
        {
            log_WARNING_LO_CaptureFailed();
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
            return;
        }

        // Save it to a file
        Fw::LogStringArg dest = destination;
        log_ACTIVITY_LO_ImageSaving(dest);

//        cv::Mat mat(m_height, m_width, CV_8UC3, frame.data);
//        cv::imwrite(destination.toChar(), mat);

        // Deallocate the buffer
//        m_camera->return_buffer(frame.request);
    }

    void CamImpl::CONFIGURE_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        // Grab the configuration from the parameter database

        log_ACTIVITY_HI_CameraStopping();
        m_camera->StopCamera();

        CameraConfig config;
        get_config(config);
        log_ACTIVITY_HI_CameraConfiguring();
        m_camera->ConfigureCamera(config);

        log_ACTIVITY_HI_CameraStarting();
        m_camera->StartCamera();

        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    CamFrame* CamImpl::get_buffer()
    {
        for (auto &m_buffer : m_buffers)
        {
            if (!m_buffer.in_use)
            {
                m_buffers->in_use = true;
                return &m_buffer;
            }
        }

        return nullptr;
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

    void CamImpl::start_handler(NATIVE_INT_TYPE portNum,
                                NATIVE_UINT_TYPE context)
    {
        if (m_streaming_to != -1)
        {
            log_ACTIVITY_LO_CameraStreamStopping(portNum);
        }

        m_streaming_to = portNum;
        log_ACTIVITY_LO_CameraStreamStarting(portNum);
    }

    void CamImpl::stop_handler(NATIVE_INT_TYPE portNum,
                               NATIVE_UINT_TYPE context)
    {
        if (m_streaming_to != -1)
        {
            log_ACTIVITY_LO_CameraStreamStopping(portNum);
            m_streaming_to = -1;
        }
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
}
