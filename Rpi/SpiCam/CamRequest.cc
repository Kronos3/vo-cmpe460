#include <Os/File.hpp>
#include "CamImpl.h"

namespace Rpi
{
    void CamImpl::Request::dma_reply(Drv::SpiStatus status,
                                     Fw::Buffer &writeBuffer,
                                     Fw::Buffer &readBuffer)
    {
        if (status != Drv::SpiStatus::SPI_OK)
        {
            // The DMA request failed
            // We should not continue transferring the image
            // End now
            // (let the status fallthrough)
        }
        else
        {
            m_read_n += readBuffer.getSize();
            if (m_read_n >= m_image.getSize())
            {
                // Image transfer finished
                // Reply asynchronously
                m_cam->captureFinished_internalInterfaceInvoke(status);
            }
            else
            {
                // We are ready to transfer the next DMA block from SPI
                // Make sure we are not writing the burst FIFO command on MOSI
                writeBuffer.getData()[0] = 0x0;

                Fw::Buffer& next_block = get_read_block();
                writeBuffer.setSize(next_block.getSize());
                status = m_cam->spiDma_out(0, writeBuffer, next_block);
            }
        }

        if (status != Drv::SpiStatus::SPI_OK)
        {
            m_cam->captureFinished_internalInterfaceInvoke(status);
        }
    }

    Fw::Buffer &CamImpl::Request::get_read_block()
    {
        // Write as many bytes over the DMA as possible
        // Raspberry Pi does not allow over 65535 byte to be
        // requested for transfer over the DMA
        U32 bytes_left = m_image.getSize() - m_read_n;
        read_block.set(m_image.getData() + m_read_n,
                       FW_MIN(U16_MAX, bytes_left));

        return read_block;
    }

    CamImpl::CommandRequest::CommandRequest(CamImpl* cam, U32 opCode, U32 cmdSeq, const Fw::String &filename)
    : Request(cam, final_buffer), m_opCode(opCode), m_cmdSeq(cmdSeq), m_filename(filename)
    {
        // Allocate a frame buffer for the entire image
        // This will be deleted when the image request is finished
        final_buffer.set(new U8[m_cam->m_fifo_size], m_cam->m_fifo_size);
    }

    CamImpl::CommandRequest::~CommandRequest()
    {
        // De-alloc the buffer requested at construction
        delete final_buffer.getData();
        final_buffer.set(nullptr, 0);
    }

    void CamImpl::CommandRequest::reply(Drv::SpiStatus status)
    {
        Fw::CommandResponse response;
        switch (status.e)
        {
            case Drv::SpiStatus::SPI_OK:
                // Mark the camera as ready again
                response = Fw::COMMAND_OK;
                m_cam->m_state = CAM_STATE_READY;
                break;
            case Drv::SpiStatus::SPI_BUSY_ERR:
                // Mark the camera as ready again
                response = Fw::COMMAND_BUSY;
                m_cam->m_state = CAM_STATE_READY;
                break;
            default:
            case Drv::SpiStatus::SPI_SIZE_MISMATCH_ERR:
            case Drv::SpiStatus::SPI_TRANSACT_POLLING_ERR:
            case Drv::SpiStatus::SPI_TRANSACT_DMA_ERR:
            case Drv::SpiStatus::SPI_OTHER_ERR:
                response = Fw::COMMAND_EXECUTION_ERROR;
                m_cam->m_state = CAM_STATE_PANIC;
                m_cam->log_WARNING_HI_CameraPanicSPITransfer();
                break;
        }

        // Only write the image down to a file if the SPI transfer was good
        if (response == Fw::COMMAND_OK)
        {
            // Write all FIFO data down to a file
            Os::File fp;
            Os::File::Status file_status;
            Fw::LogStringArg fn_arg(m_filename.toChar());

            // Attempt to open the file
            // Truncate/create
            file_status = fp.open(m_filename.toChar(), Os::File::OPEN_WRITE);
            if (file_status != Os::File::OP_OK)
            {
                // Failed to open
                // Log error and respond with failure
                m_cam->log_WARNING_LO_CameraFileOpenFailed(fn_arg);
                m_cam->cmdResponse_out(m_opCode, m_cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                return;
            }

            U32 size_written = 0;
            while (size_written < m_image.getSize())
            {
                U32 bytes_left = m_image.getSize() - size_written;
                I32 write_size = FW_MIN(1024 * 2, bytes_left);
                file_status = fp.write(m_image.getData() + size_written, write_size, true);
                if (file_status != Os::File::OP_OK || write_size <= 0)
                {
                    fp.close();
                    m_cam->log_WARNING_LO_CameraFileWriteFailed(fn_arg, size_written);
                    m_cam->cmdResponse_out(m_opCode, m_cmdSeq, Fw::COMMAND_EXECUTION_ERROR);
                    return;
                }

                size_written += write_size;
            }

            fp.close();
            m_cam->log_ACTIVITY_LO_CameraFileSuccess(size_written, fn_arg);
            m_cam->cmdResponse_out(m_opCode, m_cmdSeq, Fw::COMMAND_OK);
        }
        else
        {
            m_cam->cmdResponse_out(m_opCode, m_cmdSeq, response);
        }
    }
}
