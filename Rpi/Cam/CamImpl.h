
#ifndef VO_CMPE460_CAMIMPL_H
#define VO_CMPE460_CAMIMPL_H

#include <Rpi/Cam/CamComponentAc.hpp>

namespace Rpi
{
    class LibCamera;
    class CamImpl : public CamComponentBase
    {
        friend LibCamera;

    public:
        explicit CamImpl(const char* compName);
        ~CamImpl() override;

        void init(
                NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
                NATIVE_INT_TYPE instance, /*!< The instance number*/
                I32 width, I32 height,
                U32 buffer_count,
                I32 rotation
        );

    PRIVATE:
        void preamble() override;

        void capture_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context) override;
        void deallocate_handler(NATIVE_INT_TYPE portNum, CameraFrame &frame) override;

        void CAPTURE_cmdHandler(U32 opCode, U32 cmdSeq, const Fw::CmdStringArg &destination) override;

    PRIVATE:

        LibCamera* m_camera;

        I32 m_width;
        I32 m_height;

//        cv::Mat m_buffers[CAMERA_BUFFER_N];   //!< Frame buffers available to camera
//        bool m_in_use[CAMERA_BUFFER_N];       //!< Buffers currently in use
    };
}

#endif //VO_CMPE460_CAMIMPL_H
