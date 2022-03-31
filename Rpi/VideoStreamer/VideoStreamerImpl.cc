
#include "VideoStreamerImpl.h"
#include <preview/preview.hpp>

namespace Rpi
{
    VideoStreamerImpl* this_ = nullptr;

    VideoStreamerImpl::VideoStreamerImpl(const char* compName)
    : VideoStreamerComponentBase(compName),
      m_preview(make_drm_preview()),
      tlm_packets_sent(0)
    {
        FW_ASSERT(!this_ && "Only one VideoStreamer may exist");
        this_ = this;
        m_preview->SetDoneCallback(VideoStreamerImpl::handle_done_s);
    }

    VideoStreamerImpl::~VideoStreamerImpl()
    {
        delete m_preview;
//        delete m_socket;
    }

    void VideoStreamerImpl::init(NATIVE_INT_TYPE queueDepth, NATIVE_INT_TYPE instance)
    {
        VideoStreamerComponentBase::init(queueDepth, instance);
    }

    void VideoStreamerImpl::frame_handler(NATIVE_INT_TYPE portNum, CamFrame* frame)
    {
#if 0
        cv::Mat& cv_frame = frame.get_image();

        Fw::ParamValid valid;
        I32 compress_width = paramGet_VIDEO_WIDTH(valid);
        FW_ASSERT(valid == Fw::PARAM_VALID || valid == Fw::PARAM_DEFAULT, valid);

        I32 compress_height = paramGet_VIDEO_HEIGHT(valid);
        FW_ASSERT(valid == Fw::PARAM_VALID || valid == Fw::PARAM_DEFAULT, valid);

        I32 quality = paramGet_VIDEO_QUALITY(valid);
        FW_ASSERT(valid == Fw::PARAM_VALID || valid == Fw::PARAM_DEFAULT, valid);

        // Resize the image to requested size
//            cv::resize(cv_frame, m_compressed_frame, cv::Size(compress_width, compress_height), 0, 0, cv::INTER_LINEAR);

        // Send the frame back to the camera
        // We are done with the original data
        frameDeallocate_out(portNum, frame);
#endif

        // Calculate frame rate
        Fw::Time current_time = getTime();
        Fw::Time delta = Fw::Time::sub(current_time, m_last_frame);

        F64 period = delta.getSeconds() + 1e-6 * delta.getUSeconds();
        tlmWrite_FramesPerSecond(1.0 / period);

        I32 fd = frame->buffer->planes()[0].fd.get();
        m_showing_frames.emplace(fd, frame);
        m_preview->Show(fd, frame->span, frame->info);
    }

    void VideoStreamerImpl::OPEN_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
    }

    void VideoStreamerImpl::preamble()
    {
        ActiveComponentBase::preamble();

        m_last_frame = getTime();
    }

    void VideoStreamerImpl::handle_done(I32 fd)
    {
        auto f = m_showing_frames.find(fd);
        if (f == m_showing_frames.end())
        {
            // We already freed this?
            // TODO(tumbar) Warning
        }
        else
        {
            frameDeallocate_out(0, f->second);
        }
    }

    void VideoStreamerImpl::handle_done_s(I32 fd)
    {
        FW_ASSERT(this_);
        this_->handle_done(fd);
    }
}
