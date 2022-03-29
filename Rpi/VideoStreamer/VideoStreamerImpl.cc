
#include "VideoStreamerImpl.h"
#include "VideoStreamerCfg.h"

#include <Rpi/Cam/Frame.h>
#include <opencv2/opencv.hpp>

namespace Rpi
{
    VideoStreamerImpl::VideoStreamerImpl(const char* compName)
    : VideoStreamerComponentBase(compName), m_socket(nullptr), m_raw_stream_running(false)
    {
    }

    void VideoStreamerImpl::init(NATIVE_INT_TYPE queueDepth, NATIVE_INT_TYPE instance)
    {
        VideoStreamerComponentBase::init(queueDepth, instance);
    }

    void VideoStreamerImpl::rawVideo_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context)
    {
        if (m_raw_stream_running)
        {
            // Request a raw image from the camera
            rawVideoCapture_out(0, context);
        }
    }

    void VideoStreamerImpl::frame_handler(NATIVE_INT_TYPE portNum, CameraFrame frame)
    {
        // Check if we have an active connection
        if (m_socket)
        {
            cv::Mat cv_frame = frame_to_cv(frame);

            Fw::ParamValid valid;
            I32 compress_width = paramGet_VIDEO_WIDTH(valid);
            FW_ASSERT(valid == Fw::PARAM_VALID || valid == Fw::PARAM_DEFAULT, valid);

            I32 compress_height = paramGet_VIDEO_HEIGHT(valid);
            FW_ASSERT(valid == Fw::PARAM_VALID || valid == Fw::PARAM_DEFAULT, valid);

            I32 quality = paramGet_VIDEO_QUALITY(valid);
            FW_ASSERT(valid == Fw::PARAM_VALID || valid == Fw::PARAM_DEFAULT, valid);

            // Resize the image to requested size
            cv::resize(cv_frame, m_compressed_frame, cv::Size(compress_width, compress_height), 0, 0, cv::INTER_LINEAR);

            // Compress and encode the image to JPEG
            std::vector<I32> compression_params;
            compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
            compression_params.push_back(quality);
            cv::imencode(".jpg", m_compressed_frame, m_encoded, compression_params);

            // Calculate the number of UDP frames to send for this image
            U32 total_pack = 1 + (m_encoded.size() - 1) / VIDEOSTREAMER_UDP_DGRAM_SIZE;

            Fw::ParamString hostname = paramGet_HOSTNAME(valid);
            FW_ASSERT(valid == Fw::PARAM_VALID || valid == Fw::PARAM_DEFAULT, valid);

            U16 port = paramGet_PORT(valid);
            FW_ASSERT(valid == Fw::PARAM_VALID || valid == Fw::PARAM_DEFAULT, valid);

            // Tell the client how many packets we are sending
            m_socket->sendTo(&total_pack, sizeof(I32), hostname.toChar(), port);

            // Send each packet of the image
            for (U32 i = 0; i < total_pack; i++)
            {
                m_socket->sendTo(&m_encoded[i * VIDEOSTREAMER_UDP_DGRAM_SIZE],
                                 VIDEOSTREAMER_UDP_DGRAM_SIZE,
                                 hostname.toChar(),
                                 port);
            }

            // Calculate frame rate
            Fw::Time current_time = getTime();
            Fw::Time delta = Fw::Time::sub(current_time, m_last_frame);

            F64 period = delta.getSeconds() + 1e-6 * delta.getUSeconds();
            tlmWrite_FramesPerSecond(1.0 / period);
        }

        // Send the frame back to the camera
        frameDeallocate_out(portNum, frame);
    }

    void VideoStreamerImpl::START_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        m_raw_stream_running = true;
    }

    void VideoStreamerImpl::STOP_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        m_raw_stream_running = false;
    }
}
