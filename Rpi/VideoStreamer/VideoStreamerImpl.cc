
#include "VideoStreamerImpl.h"
#include "encoder/h264_encoder.hpp"
#include "output/net_output.hpp"
#include "Logger.hpp"
#include <preview/preview.hpp>
#include <functional>

#include <opencv2/imgcodecs.hpp>

namespace Rpi
{
    VideoStreamerImpl::VideoStreamerImpl(const char* compName)
    : VideoStreamerComponentBase(compName),
      m_showing(nullptr),
      m_displaying(BOTH),
      m_preview(nullptr),
      m_encoder(nullptr),
      m_net(nullptr),
      m_capture{.requesting=false},
      tlm_packets_sent(0),
      tlm_total_frames(0)
    {
    }

    VideoStreamerImpl::~VideoStreamerImpl()
    {
        delete m_preview;
        delete m_encoder;
        delete m_net;
    }

    void VideoStreamerImpl::init(NATIVE_INT_TYPE queueDepth, NATIVE_INT_TYPE instance)
    {
        VideoStreamerComponentBase::init(queueDepth, instance);

        try
        {
            m_preview = make_drm_preview();
        }
        catch (const std::runtime_error& e)
        {
            Fw::Logger::logMsg("Failed initialize preview: %s\n", (POINTER_CAST)e.what());
        }
    }

    void VideoStreamerImpl::frame_handler(NATIVE_INT_TYPE portNum, CamFrame* frame)
    {
        FW_ASSERT(frame);
        FW_ASSERT(frame->in_use());

        tlm_total_frames++;
        tlmWrite_FramesTotal(tlm_total_frames);

        // Calculate frame rate
        Fw::Time current_time = getTime();
        Fw::Time delta = Fw::Time::sub(current_time, m_last_frame);

        F64 period = delta.getSeconds() + 1e-6 * delta.getUSeconds();
        tlmWrite_FramesPerSecond(1.0 / period);
        m_last_frame = current_time;

        I32 fd = frame->buffer->planes()[0].fd.get();
        if ((m_displaying == BOTH || m_displaying == UDP) && m_net)
        {
            if (!m_encoder)
            {
                m_encoder = new H264Encoder(frame->info);
                m_encoder->SetInputDoneCallback([this](void* mem)
                {
                    if (encoding_buffers.empty())
                    {
                        // The video stream was swapped during stream
                        return;
                    }

                    // Drop the reference to the oldest frame buffer we sent to the encoder
                    // This assumed that the H264 encoding will reply with in order frames...
                    encoding_buffers.front()->decref();
                    encoding_buffers.pop();
                });

                m_encoder->SetOutputReadyCallback(std::bind(&Output::OutputReady, m_net,
                                                            std::placeholders::_1,
                                                            std::placeholders::_2,
                                                            std::placeholders::_3,
                                                            std::placeholders::_4));
            }

            // Once the encoder is finished it will send the data to the network
            // This will get written out as a UDP stream
            frame->incref();
            encoding_buffers.push(frame);
            m_encoder->EncodeBuffer(fd, frame->span.size(), frame->span.data(), frame->info,
                                    (I64)frame->buffer->metadata().timestamp / 1000);
        }

        if ((m_displaying == BOTH || m_displaying == HDMI) && m_preview)
        {
            frame->incref();
            m_preview->Show(fd, frame->span, frame->info);

            if (m_showing)
            {
                m_showing->decref();
            }

            m_showing = frame;
        }

        if (m_capture.requesting)
        {
            cv::Mat image((I32)frame->info.height,
                          (I32)frame->info.width,
                          CV_8U,
                          frame->span.data(),
                          frame->info.stride);

            bool result;
            try
            {
                result = cv::imwrite(m_capture.destination.toChar(), image);
            }
            catch (const cv::Exception& ex)
            {
                Fw::LogStringArg error_str(ex.what());
                log_WARNING_HI_ImageCaptureEncodeFailed(error_str);
                result = false;
            }

            Fw::LogStringArg destination_log(m_capture.destination);
            if (result)
            {
                log_ACTIVITY_HI_ImageCaptured(destination_log);
            }
            else
            {
                log_WARNING_HI_ImageCaptureSaveFailed(destination_log);
            }

            // Reply to the command
            cmdResponse_out(m_capture.opcode, m_capture.cmdSeq,
                            result ? Fw::COMMAND_OK : Fw::COMMAND_EXECUTION_ERROR);

            // Clear the capture request
            m_capture.requesting = false;
        }

        frame->decref();
    }

    void VideoStreamerImpl::OPEN_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        // Don't delete this until nobody needs it anymore
        Output* old = m_net;

        Fw::ParamValid valid;
        Fw::ParamString hostname = paramGet_HOSTNAME(valid);
        FW_ASSERT(valid == Fw::PARAM_VALID || valid == Fw::PARAM_DEFAULT, valid);

        U16 port = paramGet_PORT(valid);
        FW_ASSERT(valid == Fw::PARAM_VALID || valid == Fw::PARAM_DEFAULT, valid);
        m_net = new NetOutput(hostname.toChar(), port);
        if (m_encoder)
        {
            // Set up an existing encoder to write to a different net
            m_encoder->SetOutputReadyCallback(std::bind(&Output::OutputReady, m_net,
                                                        std::placeholders::_1,
                                                        std::placeholders::_2,
                                                        std::placeholders::_3,
                                                        std::placeholders::_4));
        }
        else
        {
            // Even though we need the encoder for video, we can't create it
            // yet because we don't have the stream information, we can wait
            // for the first from the come in before we set it up
        }

        // The encoder is no longer referencing the old instance
        delete old;

        clean();
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void VideoStreamerImpl::preamble()
    {
        ActiveComponentBase::preamble();
        clean();
        m_last_frame = getTime();
    }

    void
    VideoStreamerImpl::DISPLAY_cmdHandler(U32 opCode, U32 cmdSeq, VideoStreamerComponentBase::DisplayLocation where)
    {
        clean();
        m_displaying = where;
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void VideoStreamerImpl::clean()
    {
        if (m_showing)
        {
            m_showing->decref();
            m_showing = nullptr;
        }

        while (!encoding_buffers.empty())
        {
            encoding_buffers.front()->decref();
            encoding_buffers.pop();
        }
    }

    void VideoStreamerImpl::CAPTURE_cmdHandler(U32 opCode, U32 cmdSeq, const Fw::CmdStringArg &destination)
    {
        // Check if there is a running image request
        if (m_capture.requesting)
        {
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_BUSY);
            return;
        }

        // Initial a capture request
        m_capture.requesting = true;
        m_capture.destination = destination;
        m_capture.opcode = opCode;
        m_capture.cmdSeq = cmdSeq;

        // Replies will be sent one the image has saved to disk or failed
    }
}
