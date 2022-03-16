#include <Os/File.hpp>
#include "CamImpl.h"

namespace Rpi
{
    CamImpl::CamImpl()
    : OV2640(),
    m_state(CAM_STATE_NOT_INITIALIZED),
    m_tlm_img_n(0), m_tlm_hz(0.0)
    {
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
                OV2640::init();
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

        if (m_state == CAM_STATE_READY)
        {
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

        Fw::Buffer write_buff(new U8[m_fifo_size], m_fifo_size);
        Fw::Buffer read_buff(new U8[m_fifo_size], m_fifo_size);
        m_request.opCode = opCode;
        m_request.cmdSeq = cmdSeq;
        m_request.filename = destination;
        m_request.is_command = true;

        // Burst FIFO
        write_buff.getData()[0] = 0x3C;

        Drv::SpiStatus status = spiDma_out(0, write_buff, read_buff);

        switch(status.e)
        {
            case Drv::SpiStatus::SPI_OK:
                // Don't reply yet
                return;
            case Drv::SpiStatus::SPI_BUSY_ERR:
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_BUSY);
                break;
            case Drv::SpiStatus::SPI_SIZE_MISMATCH_ERR:
            case Drv::SpiStatus::SPI_TRANSACT_POLLING_ERR:
                FW_ASSERT(0);
                break;
            case Drv::SpiStatus::SPI_TRANSACT_DMA_ERR:
            case Drv::SpiStatus::SPI_OTHER_ERR:
                m_state = CAM_STATE_PANIC;
                log_WARNING_HI_CameraPanicSPITransfer();
                cmdResponse_out(opCode, cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                break;
        }

        // The DMA request failed
        // Un-mark the request as active
        m_request.reset();
        delete[] write_buff.getData();
        delete[] read_buff.getData();
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
        // Make sure there is an active request
        FW_ASSERT(m_state == CAM_STATE_CAPTURE, m_state);

        Fw::CommandResponse response;
        switch(status.e)
        {
            case Drv::SpiStatus::SPI_OK:
                response = Fw::COMMAND_OK;
                break;
            case Drv::SpiStatus::SPI_BUSY_ERR:
                response = Fw::COMMAND_BUSY;
                break;
            default:
            case Drv::SpiStatus::SPI_SIZE_MISMATCH_ERR:
            case Drv::SpiStatus::SPI_TRANSACT_POLLING_ERR:
            case Drv::SpiStatus::SPI_TRANSACT_DMA_ERR:
            case Drv::SpiStatus::SPI_OTHER_ERR:
                response = Fw::COMMAND_EXECUTION_ERROR;
                m_state = CAM_STATE_PANIC;
                log_WARNING_HI_CameraPanicSPITransfer();
                break;
        }

        // Mark the camera as ready again
        m_state = CAM_STATE_READY;

        // Respond to the requester
        if (m_request.is_command)
        {
            // The command will utilize malloc()ed
            delete[] writeBuffer.getData();

            if (response == Fw::COMMAND_OK)
            {
                // Write all FIFO data down to a file
                Os::File fp;
                Os::File::Status file_status;
                Fw::LogStringArg fn_arg(m_request.filename.toChar());

                // Attempt to open the file
                // Truncate/create
                file_status = fp.open(m_request.filename.toChar(), Os::File::Mode::OPEN_CREATE);
                if (file_status != Os::File::OP_OK)
                {
                    // Failed to open
                    // Log error and respond with failure
                    delete[] readBuffer.getData();
                    log_WARNING_LO_CameraFileOpenFailed(fn_arg);
                    cmdResponse_out(m_request.opCode, m_request.cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                    return;
                }

                U32 size_written = readBuffer.getSize();
                while(size_written < readBuffer.getSize())
                {
                    I32 block_size = (I32)(readBuffer.getSize() - size_written);
                    file_status = fp.write(readBuffer.getData() + size_written, block_size);
                    if (file_status != Os::File::Status::OP_OK)
                    {
                        log_WARNING_LO_CameraFileWriteFailed(fn_arg, size_written);
                        fp.close();
                        delete[] readBuffer.getData();
                        cmdResponse_out(m_request.opCode, m_request.cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                        return;
                    }

                    size_written += block_size;
                }

                log_ACTIVITY_LO_CameraFileSuccess(fn_arg, size_written);
                fp.close();
                delete[] readBuffer.getData();
                cmdResponse_out(m_request.opCode, m_request.cmdSeq, Fw::COMMAND_OK);
            }
            else
            {
                delete[] readBuffer.getData();
                cmdResponse_out(m_request.opCode, m_request.cmdSeq, response);
            }
        }
        else
        {
            FW_ASSERT(0 && "Not implemented yet");
        }
    }
}
