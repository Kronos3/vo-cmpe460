
#ifndef VO_CMPE460_VIDEOSTREAMERIMPL_H
#define VO_CMPE460_VIDEOSTREAMERIMPL_H

#include <Rpi/VideoStreamer/VideoStreamerComponentAc.hpp>
//#include <Rpi/VideoStreamer/PracticalSocket.h>
#include <opencv2/core/mat.hpp>
#include <vector>
#include <preview/preview.hpp>
#include <queue>
#include "encoder/encoder.hpp"
#include "output/output.hpp"

//#include "bcm_host.h"
//#include "EGL/egl.h"

namespace Rpi
{
    class VideoStreamerImpl : public VideoStreamerComponentBase
    {
    public:
        explicit VideoStreamerImpl(const char* compName);
        ~VideoStreamerImpl() override;

        void init(
                NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );

    PRIVATE:
        void preamble() override;

        void frame_handler(NATIVE_INT_TYPE portNum, Rpi::CamFrame* frame) override;
        void OPEN_cmdHandler(U32 opCode, U32 cmdSeq) override;
        void DISPLAY_cmdHandler(U32 opCode, U32 cmdSeq, DisplayLocation where) override;

    PRIVATE:
        void clean();

        CamFrame* m_showing;
        DisplayLocation m_displaying;

        Preview* m_preview;

        std::queue<CamFrame*> encoding_buffers;
        Encoder* m_encoder;
        Output* m_net;
        cv::Mat m_compressed_frame;     //!< Resized frame given video parameters
        std::vector<U8> m_encoded;      //!< Encoded JPEG

        Fw::Time m_last_frame;          //!< Last sent frame for calculate frame rate
        U32 tlm_packets_sent;
        U32 tlm_total_frames;
    };
}

#endif //VO_CMPE460_VIDEOSTREAMERIMPL_H
