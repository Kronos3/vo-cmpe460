
#include "CamImpl.h"
#include <Rpi/Cam/LibCamera.h>

#include <opencv2/opencv.hpp>

namespace Rpi
{

    CamImpl::CamImpl(const char* compName)
    : CamComponentBase(compName),
    m_camera(new LibCamera(*this)),
    m_width(0), m_height(0)
    {
    }

    void CamImpl::init(NATIVE_INT_TYPE queueDepth, NATIVE_INT_TYPE instance,
                       I32 width, I32 height,
                       U32 buffer_count,
                       I32 rotation)
    {
        CamComponentBase::init(queueDepth, instance);

        m_width = width;
        m_height = height;

        I32 ret = m_camera->init(width, height,
                                 libcamera::formats::RGB888,
                                 buffer_count, rotation);
        FW_ASSERT(ret == 0 && "Failed to init camera", ret);
    }

    CamImpl::~CamImpl()
    {
        delete m_camera;
    }

    void CamImpl::preamble()
    {
        ActiveComponentBase::preamble();
        I32 ret = m_camera->start();

        FW_ASSERT(ret == 0 && "Failed to start camera", ret);
    }

    void CamImpl::capture_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context)
    {
        Frame frame;
        bool valid = m_camera->read_frame(frame);
        if (!valid)
        {
            // Drop the frame and don't send a capture reply
//            log_WARNING_LO_CaptureFailed();

            // TODO(tumbar) dropped frames telemetry

            return;
        }

        CameraFrame frameSer(
                (POINTER_CAST)frame.data,
                m_width,
                m_height,
                CV_8UC3,
                frame.request);

        // Send the frame to the requester on the same port
        frame_out(portNum, frameSer);
    }

    void CamImpl::deallocate_handler(NATIVE_INT_TYPE portNum, CameraFrame& frame)
    {
        m_camera->return_buffer(frame.getbuffer_id());
    }

    void CamImpl::CAPTURE_cmdHandler(U32 opCode, U32 cmdSeq, const Fw::CmdStringArg &destination)
    {
        // Capture an image and save it to a file

        Frame frame;
        bool valid = m_camera->read_frame(frame);

        // Make sure we have enough buffers
        if (!valid)
        {
            log_WARNING_LO_CaptureFailed();
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
            return;
        }

        // Save it to a file
        Fw::LogStringArg dest = destination;
        log_ACTIVITY_LO_ImageSaving(dest);

        cv::Mat mat(m_height, m_width, CV_8UC3, frame.data);
        cv::imwrite(destination.toChar(), mat);

        // Deallocate the buffer
        m_camera->return_buffer(frame.request);
    }
}
