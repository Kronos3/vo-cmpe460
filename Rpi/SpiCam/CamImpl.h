#ifndef VO_CMPE460_CAMIMPL_H
#define VO_CMPE460_CAMIMPL_H

#include <Rpi/Cam/CamComponentAc.hpp>
#include <Rpi/Cam/ardu_cam.h>

namespace Rpi
{
    class CamImpl : public CamComponentBase, public OV2640
    {
    public:
        explicit CamImpl(const char* comp_name);

        void init(
                NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );

        ~CamImpl();

    private:
        void cam_i2c_init();

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
        void captureFinished_internalInterfaceHandler(Drv::SpiStatus &spiStatus) override;

        enum CamState
        {
            CAM_STATE_NOT_INITIALIZED,      //!< Standard camera initialization has not been performed
            CAM_STATE_READY,                //!< Camera is ready for operation
            CAM_STATE_PANIC,                //!< Error during initialization or camera transaction
            CAM_STATE_CAPTURE,              //!< Currently capturing an image
        };

        struct Request
        {
            CamImpl* m_cam;
            U32 m_read_n;

            Fw::Buffer read_block;
            Fw::Buffer &m_image;

            explicit Request(CamImpl* cam, Fw::Buffer& image)
            : m_cam(cam), m_read_n(0), m_image(image)
            {
            }

            virtual ~Request() = default;

            Fw::Buffer& get_read_block();

            /**
             * Handle a reply from the DMA
             * This will handle DMA block finishing transfer from SPI
             * The next DMA request will be made if need be otherwise
             * the reply queued
             * @param status
             * @param writeBuffer
             * @param readBuffer
             */
            void dma_reply(Drv::SpiStatus status,
                           Fw::Buffer &writeBuffer,
                           Fw::Buffer &readBuffer);

            /**
             * Request specific reply to handle an image
             * This will be called asynchronously by an internal FPrime interface
             * @param status
             */
            virtual void reply(Drv::SpiStatus status) = 0;
        };

        class CommandRequest : public Request
        {
            U32 m_opCode;                   //!< opCode from dispatch
            U32 m_cmdSeq;                   //!< cmdSeq from dispatch
            Fw::String m_filename;          //!< Where to write the image to
            Fw::Buffer final_buffer;        //!< Command image request will allocate every time

            void reply(Drv::SpiStatus status) override;

        public:
            CommandRequest(
                    CamImpl* cam,
                    U32 opCode,
                    U32 cmdSeq,
                    const Fw::String& filename);

            ~CommandRequest() override;
        };

        friend Request;
        Request* m_request;

        volatile CamState m_state;
        Fw::Buffer m_write_buffer;
        U32 m_tlm_img_n;
        F32 m_tlm_hz;
    };
}

#endif //VO_CMPE460_CAMIMPL_H
