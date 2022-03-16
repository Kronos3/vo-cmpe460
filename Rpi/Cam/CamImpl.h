#ifndef VO_CMPE460_CAMIMPL_H
#define VO_CMPE460_CAMIMPL_H

#include <Rpi/Cam/CamComponentAc.hpp>
#include <Rpi/Cam/ardu_cam.h>

namespace Rpi
{
    class CamImpl : public CamComponentBase, public OV2640
    {
    public:
        CamImpl();

        void init(
                NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );

    private:
        void schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context) override;

        void read_i2c(U8 address, U8* dst, U32 len) override;
        void write_i2c(U8 address, U8* src, U32 len) override;

        void read_write_sync_spi(U8* write, U8* read, U32 size) override;

        void INIT_cmdHandler(U32 opCode, U32 cmdSeq) override;
        void SET_CALIBRATION_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::CameraCalibration Selection, I8 Value) override;
        void SET_JPEG_SIZE_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::CameraJpegSize Value) override;
        void SET_LIGHT_MODE_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::CameraLightMode Value) override;
        void SET_SPECIAL_EFFECT_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::CameraSpecialEffect Value) override;
        void CAPTURE_cmdHandler(U32 opCode, U32 cmdSeq, const Fw::CmdStringArg &destination) override;
        void RESET_cmdHandler(U32 opCode, U32 cmdSeq) override;

        void spiDmaReply_handler(NATIVE_INT_TYPE portNum, Drv::SpiStatus status, Fw::Buffer &writeBuffer, Fw::Buffer &readBuffer) override;

        enum CamState
        {
            CAM_STATE_NOT_INITIALIZED,      //!< Standard camera initialization has not been performed
            CAM_STATE_READY,                //!< Camera is ready for operation
            CAM_STATE_PANIC,                //!< Error during initialization or camera transaction
            CAM_STATE_CAPTURE,              //!< Currently capturing an image
        };

        struct CamRequest
        {
            bool is_command;
            U32 opCode;
            U32 cmdSeq;
            Fw::CmdStringArg filename;

            CamRequest()
            : is_command(false),
            opCode(0), cmdSeq(0)
            {}

            void reset()
            {
                is_command = false;
                opCode = 0;
                cmdSeq = 0;
            }
        } m_request;

        volatile CamState m_state;
        U32 m_tlm_img_n;
        F32 m_tlm_hz;
    };
}

#endif //VO_CMPE460_CAMIMPL_H
