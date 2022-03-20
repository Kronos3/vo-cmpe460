#include <Os/File.hpp>
#include "CamImpl.h"
#include "ov2640_regs.h"

#define OV2640_CHIPID_HIGH 	0x0A
#define OV2640_CHIPID_LOW 	0x0B

namespace Rpi
{
    CamImpl::CamImpl(const char* comp_name)
    : CamComponentBase(comp_name), OV2640(),
    m_request(nullptr),
    m_state(CAM_STATE_NOT_INITIALIZED),
    m_write_buffer(new U8[U16_MAX], U16_MAX),
    m_tlm_img_n(0),
    m_tlm_hz(0.0)
    {
        memset(m_write_buffer.getData(), 0, U16_MAX);
    }

    void CamImpl::init(NATIVE_INT_TYPE queueDepth, NATIVE_INT_TYPE instance)
    {
        CamComponentBase::init(queueDepth, instance);
    }

    void CamImpl::read_i2c(U8 address, U8* dst, U32 len)
    {
        // Don't send I2C requests when camera is in panic
        if (m_state == CAM_STATE_PANIC)
        {
            return;
        }

        Fw::Buffer buf(dst, len);
        Drv::I2cStatus i2cStatus = i2cRead_out(0, address, buf);

        if (i2cStatus != Drv::I2cStatus::I2C_OK)
        {
            m_state = CAM_STATE_PANIC;
            log_WARNING_HI_CameraPanicI2CRead(i2cStatus);
        }
    }

    void CamImpl::write_i2c(U8 address, U8* src, U32 len)
    {
        // Don't send I2C requests when camera is in panic
        if (m_state == CAM_STATE_PANIC)
        {
            return;
        }

        Fw::Buffer buf(src, len);
        Drv::I2cStatus i2cStatus = i2cWrite_out(0, address, buf);

        if (i2cStatus != Drv::I2cStatus::I2C_OK)
        {
            log_WARNING_HI_CameraPanicI2CWrite(i2cStatus);
            m_state = CAM_STATE_PANIC;
        }
    }

    void CamImpl::read_write_sync_spi(U8* write, U8* read, U32 size)
    {
        // Don't send SPI requests when camera is in panic
        if (m_state == CAM_STATE_PANIC)
        {
            return;
        }

        Fw::Buffer write_buf(write, size);
        Fw::Buffer read_buf(write, size);
        Drv::SpiStatus status = spiSync_out(0, write_buf, read_buf);
        if (status != Drv::SpiStatus::SPI_OK)
        {
            log_WARNING_HI_CameraPanicSPITransfer();
            m_state = CAM_STATE_PANIC;
        }
    }

    void CamImpl::schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context)
    {

    }

    void CamImpl::INIT_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        switch(m_state)
        {
            case CAM_STATE_NOT_INITIALIZED:
                log_ACTIVITY_HI_CameraInitializing();
                cam_i2c_init();
                break;
            case CAM_STATE_READY:
                log_DIAGNOSTIC_CameraAlreadyInitialized();
                // Don't respond with failure
                break;
            case CAM_STATE_PANIC:
                log_WARNING_HI_CameraPanicFailure();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_CAPTURE:
                log_WARNING_LO_CameraBusy();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_BUSY);
                return;
        }

        // Make sure the camera hasn't entered panic mode
        if (m_state == CAM_STATE_NOT_INITIALIZED)
        {
            m_state = CAM_STATE_READY;
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
        }
        else
        {
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
        }
    }

    void CamImpl::SET_CALIBRATION_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::CameraCalibration Selection, I8 Value)
    {
        // Validate the command
        switch(Selection.e)
        {
            case CameraCalibration::CONTRAST:
            case CameraCalibration::BRIGHTNESS:
            case CameraCalibration::COLOR_SATURATION:
                break;
            default:
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_VALIDATION_ERROR);
                return;
        }

        switch(Value)
        {
            case 2:
            case 1:
            case 0:
            case -1:
            case -2:
                break;
            default:
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_VALIDATION_ERROR);
                return;
        }

        switch(m_state)
        {
            case CAM_STATE_NOT_INITIALIZED:
                log_WARNING_HI_CameraNotInitialized();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_READY:
                break;
            case CAM_STATE_PANIC:
                log_WARNING_HI_CameraPanicFailure();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_CAPTURE:
                log_WARNING_LO_CameraBusy();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_BUSY);
                return;
        }

        switch(Selection.e)
        {
            case CameraCalibration::CONTRAST:
                set_contrast(Value);
                break;
            case CameraCalibration::BRIGHTNESS:
                set_brightness(Value);
                break;
            case CameraCalibration::COLOR_SATURATION:
                set_color_saturation(Value);
                break;
            default:
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_INVALID_OPCODE);
                return;
        }

        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void CamImpl::SET_JPEG_SIZE_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::CameraJpegSize Value)
    {
        switch(Value.e)
        {
            case CameraJpegSize::P_160x120:
            case CameraJpegSize::P_176x144:
            case CameraJpegSize::P_320x240:
            case CameraJpegSize::P_352x288:
            case CameraJpegSize::P_640x480:
            case CameraJpegSize::P_800x600:
            case CameraJpegSize::P_1024x768:
            case CameraJpegSize::P_1280x1024:
            case CameraJpegSize::P_1600x1200:
                break;
            default:
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_VALIDATION_ERROR);
                return;
        }

        switch(m_state)
        {
            case CAM_STATE_NOT_INITIALIZED:
                log_WARNING_HI_CameraNotInitialized();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_READY:
                break;
            case CAM_STATE_PANIC:
                log_WARNING_HI_CameraPanicFailure();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_CAPTURE:
                log_WARNING_LO_CameraBusy();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_BUSY);
                return;
        }

        set_jpeg_size(Value);

        if (m_state == CAM_STATE_PANIC)
        {
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
        }
        else
        {
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
        }
    }

    void CamImpl::SET_LIGHT_MODE_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::CameraLightMode Value)
    {
        switch(Value.e)
        {
            case CameraLightMode::AUTO:
            case CameraLightMode::SUNNY:
            case CameraLightMode::CLOUDY:
            case CameraLightMode::OFFICE:
            case CameraLightMode::HOME:
                break;
            default:
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_VALIDATION_ERROR);
                return;
        }

        switch(m_state)
        {
            case CAM_STATE_NOT_INITIALIZED:
                log_WARNING_HI_CameraNotInitialized();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_READY:
                break;
            case CAM_STATE_PANIC:
                log_WARNING_HI_CameraPanicFailure();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_CAPTURE:
                log_WARNING_LO_CameraBusy();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_BUSY);
                return;
        }

        set_light_mode(Value);

        if (m_state == CAM_STATE_PANIC)
        {
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
        }
        else
        {
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
        }
    }

    void CamImpl::SET_SPECIAL_EFFECT_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::CameraSpecialEffect Value)
    {
        switch(Value.e)
        {
            case CameraSpecialEffect::ANTIQUE:
            case CameraSpecialEffect::BLUEISH:
            case CameraSpecialEffect::GREENISH:
            case CameraSpecialEffect::REDDISH:
            case CameraSpecialEffect::BW:
            case CameraSpecialEffect::NEGATIVE:
            case CameraSpecialEffect::BW_NEGATIVE:
            case CameraSpecialEffect::NORMAL:
                break;
            default:
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_VALIDATION_ERROR);
                return;
        }

        switch(m_state)
        {
            case CAM_STATE_NOT_INITIALIZED:
                log_WARNING_HI_CameraNotInitialized();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_READY:
                break;
            case CAM_STATE_PANIC:
                log_WARNING_HI_CameraPanicFailure();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_CAPTURE:
                log_WARNING_LO_CameraBusy();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_BUSY);
                return;
        }

        set_special_effects(Value);

        if (m_state == CAM_STATE_PANIC)
        {
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
        }
        else
        {
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
        }
    }

    void CamImpl::CAPTURE_cmdHandler(U32 opCode, U32 cmdSeq, const Fw::CmdStringArg &destination)
    {
        switch(m_state)
        {
            case CAM_STATE_NOT_INITIALIZED:
                log_WARNING_HI_CameraNotInitialized();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_READY:
                break;
            case CAM_STATE_PANIC:
                log_WARNING_HI_CameraPanicFailure();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            case CAM_STATE_CAPTURE:
                log_WARNING_LO_CameraBusy();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_BUSY);
                return;
        }

        capture_image();

        if (m_state == CAM_STATE_PANIC)
        {
            log_WARNING_HI_CameraPanicFailure();
            cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
            return;
        }

        // Burst FIFO
        m_write_buffer.getData()[0] = 0x3C;

        FW_ASSERT(!m_request, (POINTER_CAST)m_request);
        m_state = CAM_STATE_CAPTURE;
        m_request = new CommandRequest(this, opCode, cmdSeq, destination);

        Fw::Buffer& next_block = m_request->get_read_block();
        m_write_buffer.setSize(next_block.getSize());
        Drv::SpiStatus status = spiDma_out(0, m_write_buffer, next_block);

        if (status != Drv::SpiStatus::SPI_OK)
        {
            // The initial DMA request failed
            // Reply to the request
            captureFinished_internalInterfaceInvoke(status);
        }
    }

    void CamImpl::RESET_cmdHandler(U32 opCode, U32 cmdSeq)
    {
        m_state = CAM_STATE_NOT_INITIALIZED;
        log_ACTIVITY_HI_CameraReset();
        cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_OK);
    }

    void CamImpl::spiDmaReply_handler(NATIVE_INT_TYPE portNum,
                                      Drv::SpiStatus status,
                                      Fw::Buffer &writeBuffer,
                                      Fw::Buffer &readBuffer)
    {
        FW_ASSERT(m_state == CAM_STATE_CAPTURE, m_state);
        FW_ASSERT(m_request);

        m_request->dma_reply(status, writeBuffer, readBuffer);
    }

    void CamImpl::cam_i2c_init()
    {
        // Reset the CPLD
        log_DIAGNOSTIC_CameraInitCPLD();
        w_spi_reg(0x07, 0x80);
        Os::Task::delay(100);
        w_spi_reg(0x07, 0x00);
        Os::Task::delay(100);

        // Enable FIFO mode
        log_DIAGNOSTIC_CameraInitEnableFifo();
        w_spi_reg(0x03, 1 << 4);
        Os::Task::delay(100);

        // TODO Check SPI interface

        //Check if the camera module type is OV2640
//        log_DIAGNOSTIC_CameraInitCheckModule();
//        ws_8_8(0xff, 0x01);
//        U8 vid, pid;
//        rs_8_8(OV2640_CHIPID_HIGH, &vid);
//        rs_8_8(OV2640_CHIPID_LOW, &pid);
        // TODO

        // Initialize the OV2640
        // This camera only works with the JPEG imaging format
        log_DIAGNOSTIC_CameraInitOV2640();
        ws_8_8(0xff, 0x01);
        ws_8_8(0x12, 0x80);

        Os::Task::delay(100);

        log_DIAGNOSTIC_CameraInitJpegInit();
        ws_8_8(OV2640_JPEG_INIT);

        log_DIAGNOSTIC_CameraInitYUV422();
        ws_8_8(OV2640_YUV422);

        log_DIAGNOSTIC_CameraInitJpeg();
        ws_8_8(OV2640_JPEG);
        ws_8_8(0xff, 0x01);
        ws_8_8(0x15, 0x00);

        set_jpeg_size(Rpi::CameraJpegSize::P_320x240);

        update_fifo_length();
    }

    void CamImpl::captureFinished_internalInterfaceHandler(Drv::SpiStatus &spiStatus)
    {
        // Make sure there is an active request
        FW_ASSERT(m_state == CAM_STATE_CAPTURE, m_state);
        FW_ASSERT(m_request);

        m_request->reply(spiStatus);

        if (spiStatus == Drv::SpiStatus::SPI_OK)
        {
            m_tlm_img_n++;
            tlmWrite_FramesCaptured(m_tlm_img_n);
        }

        // Clear the request
        delete m_request;
        m_request = nullptr;
    }

    CamImpl::~CamImpl()
    {
        delete m_request;
        m_request = nullptr;

        delete m_write_buffer.getData();
        m_write_buffer.set(nullptr, 0);
    }
}
