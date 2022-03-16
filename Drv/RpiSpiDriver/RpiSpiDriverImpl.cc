#include "RpiSpiDriverImpl.h"
#include <Rpi/Top/kernel.h>

namespace Drv
{
    RpiSpiDriverImpl::RpiSpiDriverImpl(
            const U32 chipSelect[NUM_TRANSACTPOLLING_INPUT_PORTS],
            U32 n
            )
    : m_chipSelect(), m_tlm_RxTx(0), m_busy(false)
    {
        FW_ASSERT(n == NUM_TRANSACTPOLLING_INPUT_PORTS, n, NUM_TRANSACTPOLLING_INPUT_PORTS);
        for(U32 i = 0; i < n; i++)
        {
            m_chipSelect[i] = chipSelect[i];
        }
    }

    void RpiSpiDriverImpl::init(NATIVE_INT_TYPE instance)
    {
        RpiSpiDriverComponentBase::init(instance);
        kernel::spi.SetCompletionRoutine(dmaCompletion_routine, this);
    }

    Drv::SpiStatus RpiSpiDriverImpl::transactDma_handler(
            NATIVE_INT_TYPE portNum,
            Fw::Buffer &writeBuffer,
            Fw::Buffer &readBuffer)
    {
        FW_ASSERT(portNum < NUM_TRANSACTPOLLING_INPUT_PORTS);

        // Validate the SPI request
        Drv::SpiStatus status = transactCommon(writeBuffer, readBuffer);
        if (status != Drv::SpiStatus::SPI_OK)
        {
            return status;
        }

        // The Raspberry Pi DMA requires 4-byte aligned DMA buffers
        if (((POINTER_CAST)writeBuffer.getData() & 0x3) != 0 ||
            ((POINTER_CAST)writeBuffer.getData() & 0x3) != 0)
        {
            log_WARNING_HI_SPI_DmaInvalidBuffer((POINTER_CAST)readBuffer.getData(),
                                                (POINTER_CAST)writeBuffer.getData());
            return Drv::SpiStatus::SPI_TRANSACT_DMA_ERR;
        }

        // Set up the DMA request
        m_busy = true;
        m_dma.portNum = portNum;
        m_dma.readBuffer = &readBuffer;
        m_dma.writeBuffer = &writeBuffer;

        // Perform the polling on the Raspberry Pi SPI
        kernel::spi.StartWriteRead(m_chipSelect[portNum],
                                   writeBuffer.getData(),
                                   readBuffer.getData(),
                                   writeBuffer.getSize());

        return Drv::SpiStatus::SPI_OK;
    }

    Drv::SpiStatus
    RpiSpiDriverImpl::transactPolling_handler(
            NATIVE_INT_TYPE portNum,
            Fw::Buffer &writeBuffer,
            Fw::Buffer &readBuffer)
    {
        FW_ASSERT(portNum < NUM_TRANSACTPOLLING_INPUT_PORTS);

        // Validate the SPI request
        Drv::SpiStatus status = transactCommon(writeBuffer, readBuffer);
        if (status != Drv::SpiStatus::SPI_OK)
        {
            return status;
        }

        m_busy = true;

        I32 num_bytes;
        // Perform the polling on the Raspberry Pi SPI
        num_bytes = kernel::spi.WriteReadSync(m_chipSelect[portNum],
                                              writeBuffer.getData(),
                                              readBuffer.getData(),
                                              writeBuffer.getSize());

        m_busy = false;

        if (num_bytes < 0)
        {
            log_WARNING_HI_SPI_PollingFailed();
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
            Fw::Buffer &writeBuffer,
            Fw::Buffer &readBuffer)
    {
        if (m_busy)
        {
            log_WARNING_HI_SPI_Busy();
            return Drv::SpiStatus::SPI_BUSY_ERR;
        }

        if (writeBuffer.getSize() != readBuffer.getSize())
        {
            log_WARNING_HI_SPI_SizeMismatch(readBuffer.getSize(), writeBuffer.getSize());
            return Drv::SpiStatus::SPI_SIZE_MISMATCH_ERR;
        }

        return Drv::SpiStatus::SPI_OK;
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

        NATIVE_INT_TYPE portNum = m_dma.portNum;
        Fw::Buffer* readBuffer = m_dma.readBuffer;
        Fw::Buffer* writeBuffer = m_dma.writeBuffer;

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
            log_WARNING_HI_SPI_DmaFailed();
            spi_status = Drv::SpiStatus::SPI_TRANSACT_DMA_ERR;
        }

        replyDma_out(portNum,
                     spi_status,
                     *writeBuffer,
                     *readBuffer);
    }
}
