
#ifndef VO_CMPE460_VIDEOSTREAMERIMPL_H
#define VO_CMPE460_VIDEOSTREAMERIMPL_H

#include <Rpi/VideoStreamer/VideoStreamerComponentAc.hpp>
#include <Rpi/VideoStreamer/PracticalSocket.h>
#include <opencv4/opencv2/core/mat.hpp>
#include <vector>

namespace Rpi
{
    class VideoStreamerImpl : public VideoStreamerComponentBase
    {
    public:
        VideoStreamerImpl(const char* compName);

        void init(
                NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );

    PRIVATE:
        void rawVideo_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context) override;
        void frame_handler(NATIVE_INT_TYPE portNum, Rpi::CameraFrame frame) override;

        void START_cmdHandler(U32 opCode, U32 cmdSeq) override;
        void STOP_cmdHandler(U32 opCode, U32 cmdSeq) override;

    PRIVATE:
        UDPSocket* m_socket;            //!< UDP socket client

        cv::Mat m_compressed_frame;     //!< Resized frame given video parameters
        std::vector<U8> m_encoded;      //!< Encoded JPEG

        bool m_raw_stream_running;      //!< Are we requesting captures from the camera

        Fw::Time m_last_frame;          //!< Last sent frame for calculate frame rate
    };
}

#endif //VO_CMPE460_VIDEOSTREAMERIMPL_H
