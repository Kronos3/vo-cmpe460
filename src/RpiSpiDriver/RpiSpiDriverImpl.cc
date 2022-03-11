#include "RpiSpiDriverImpl.h"
#include <src/Rpi/kernel.h>

namespace Drv
{
    RpiSpiDriverImpl::RpiSpiDriverImpl()
    : m_tlm_RxTx(0), m_busy(false)
    {
    }

    Drv::SpiStatus RpiSpiDriverImpl::transactDma_handler(
            NATIVE_INT_TYPE portNum,
            U32 chipSelect,
            Fw::Buffer &writeBuffer,
            Fw::Buffer &readBuffer)
    {
        // Validate the SPI request
        Drv::SpiStatus status = transactCommon(chipSelect, writeBuffer, readBuffer);
        if (status != Drv::SpiStatus::SPI_OK)
        {
            return status;
        }

        // The Raspberry Pi DMA requires 4-byte aligned DMA buffers
        if (((POINTER_CAST)writeBuffer.getData() & 0x3) != 0 ||
            ((POINTER_CAST)writeBuffer.getData() & 0x3) != 0)
        {
            log_WARNING_HI_SPI_DmaInvalidBuffer(chipSelect,
                                                (U32)readBuffer.getData(),
                                                (U32)writeBuffer.getData());
            return Drv::SpiStatus::SPI_TRANSACT_DMA_ERR;
        }

        // Set up the DMA request
        m_busy = true;
        m_dma.chipSelect = chipSelect;
        m_dma.portNum = portNum;
        m_dma.readBuffer = &readBuffer;
        m_dma.writeBuffer = &writeBuffer;

        // Perform the polling on the Raspberry Pi SPI
        kernel::spi.StartWriteRead(chipSelect,
                                   writeBuffer.getData(),
                                   readBuffer.getData(),
                                   writeBuffer.getSize());

        return Drv::SpiStatus::SPI_OK;
    }

    Drv::SpiStatus
    RpiSpiDriverImpl::transactPolling_handler(
            NATIVE_INT_TYPE portNum,
            U32 chipSelect,
            Fw::Buffer &writeBuffer,
            Fw::Buffer &readBuffer)
    {
        // Validate the SPI request
        Drv::SpiStatus status = transactCommon(chipSelect, writeBuffer, readBuffer);
        if (status != Drv::SpiStatus::SPI_OK)
        {
            return status;
        }

        m_busy = true;

        // Perform the polling on the Raspberry Pi SPI
        I32 num_bytes = kernel::spi.WriteReadSync(chipSelect,
                                                  writeBuffer.getData(),
                                                  readBuffer.getData(),
                                                  writeBuffer.getSize());

        m_busy = false;

        if (num_bytes < 0)
        {
            log_WARNING_HI_SPI_PollingFailed(chipSelect);
            return Drv::SpiStatus::SPI_TRANSACT_POLLING_ERR;
        }
        else
        {
            m_tlm_RxTx += num_bytes;
            tlmWrite_SPI_Bytes(m_tlm_RxTx);
        }

        return Drv::SpiStatus::SPI_OK;
    }

    Drv::SpiStatus RpiSpiDriverImpl::transactCommon(
            U32 chipSelect,
            Fw::Buffer &writeBuffer,
            Fw::Buffer &readBuffer)
    {
        // There are only two CS lines on the Raspberry Pi
        // We could technically support more via GPIO and manually
        // setting more. For now, we don't need this feature
        if (!(chipSelect == 0 ||
              chipSelect == 1))
        {
            log_WARNING_HI_SPI_ChipSelectInvalid(chipSelect);
            return Drv::SpiStatus::SPI_CHIP_SELECT_ERR;
        }

        if (m_busy)
        {
            log_WARNING_HI_SPI_Busy(chipSelect);
            return Drv::SpiStatus::SPI_BUSY_ERR;
        }

        if (writeBuffer.getSize() != readBuffer.getSize())
        {
            log_WARNING_HI_SPI_SizeMismatch(chipSelect, readBuffer.getSize(), writeBuffer.getSize());
            return Drv::SpiStatus::SPI_SIZE_MISMATCH_ERR;
        }

        return Drv::SpiStatus::SPI_OK;
    }

    void RpiSpiDriverImpl::init(NATIVE_INT_TYPE instance)
    {
        RpiSpiDriverComponentBase::init(instance);

        kernel::spi.SetCompletionRoutine(dmaCompletion_routine, this);
    }

    void RpiSpiDriverImpl::dmaCompletion_routine(bool status, void* this_)
    {
        reinterpret_cast<RpiSpiDriverImpl*>(this_)->dmaSendReply(status);
    }

    void RpiSpiDriverImpl::dmaSendReply(bool status)
    {
        FW_ASSERT(m_busy);
        FW_ASSERT(m_dma.writeBuffer);
        FW_ASSERT(m_dma.readBuffer);

        m_busy = false;

        U32 chipSelect = m_dma.chipSelect;
        NATIVE_INT_TYPE portNum = m_dma.portNum;
        Fw::Buffer* readBuffer = m_dma.readBuffer;
        Fw::Buffer* writeBuffer = m_dma.writeBuffer;

        m_dma.chipSelect = 0;
        m_dma.portNum = 0;
        m_dma.readBuffer = nullptr;
        m_dma.writeBuffer = nullptr;

        Drv::SpiStatus spi_status;
        if (status)
        {
            m_tlm_RxTx += readBuffer->getSize();
            tlmWrite_SPI_Bytes(m_tlm_RxTx);
            spi_status = Drv::SpiStatus::SPI_OK;
        }
        else
        {
            log_WARNING_HI_SPI_DmaFailed(chipSelect);
            spi_status = Drv::SpiStatus::SPI_TRANSACT_DMA_ERR;
        }

        replyDma_out(portNum,
                     spi_status,
                     chipSelect,
                     *writeBuffer,
                     *readBuffer);
    }
}
