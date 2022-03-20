#include "RpiSpiDriverImpl.h"
#include <Rpi/Top/kernel.h>

namespace Drv
{
    RpiSpiDriverImpl::RpiSpiDriverImpl()
    : m_tlm_RxTx(0),
    m_tlm_SyncRxTx(0),
    m_tlm_DmaRxTx(0),
    m_tlm_SyncN(0),
    m_tlm_DmaN(0),
    m_tlm_SyncFailN(0),
    m_tlm_DmaFailN(0),
    m_busy(false)
    {
    }

    void RpiSpiDriverImpl::init(NATIVE_INT_TYPE instance)
    {
        RpiSpiDriverComponentBase::init(instance);
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
            ((POINTER_CAST)readBuffer.getData() & 0x3) != 0)
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

        m_tlm_DmaN++;
        tlmWrite_DmaTransfers(m_tlm_DmaN);

        // A DMA reply will clear the completion routine
        // We need to set this every time
        kernel::spi.SetCompletionRoutine(dmaCompletion_routine, this);

        // Send the DMA request for SPI transfer
        kernel::spi.StartWriteRead(portNum,
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

        m_tlm_SyncN++;
        tlmWrite_SyncTransfers(m_tlm_SyncN);

        I32 num_bytes;
        // Perform the polling on the Raspberry Pi SPI
        num_bytes = kernel::spi.WriteReadSync(portNum,
                                              writeBuffer.getData(),
                                              readBuffer.getData(),
                                              writeBuffer.getSize());

        m_busy = false;

        if (num_bytes < 0)
        {
            m_tlm_SyncFailN++;
            tlmWrite_SyncFailures(m_tlm_SyncFailN);
            log_WARNING_HI_SPI_PollingFailed();
            return Drv::SpiStatus::SPI_TRANSACT_POLLING_ERR;
        }
        else
        {
            m_tlm_RxTx += num_bytes;
            m_tlm_SyncRxTx += num_bytes;
            tlmWrite_Bytes(m_tlm_RxTx);
            tlmWrite_SyncBytes(m_tlm_SyncRxTx);
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

        // Save the request on the stack
        NATIVE_INT_TYPE portNum = m_dma.portNum;
        Fw::Buffer* readBuffer = m_dma.readBuffer;
        Fw::Buffer* writeBuffer = m_dma.writeBuffer;

        // Clear the request from memory
        m_dma.portNum = 0;
        m_dma.readBuffer = nullptr;
        m_dma.writeBuffer = nullptr;

        // Process the reply status
        // Send out telemetry
        Drv::SpiStatus spi_status;
        if (status)
        {
            m_tlm_RxTx += readBuffer->getSize();
            m_tlm_DmaRxTx += readBuffer->getSize();
            tlmWrite_Bytes(m_tlm_RxTx);
            tlmWrite_DmaBytes(m_tlm_DmaRxTx);
            spi_status = Drv::SpiStatus::SPI_OK;
        }
        else
        {
            m_tlm_DmaFailN++;
            tlmWrite_SyncFailures(m_tlm_SyncFailN);
            log_WARNING_HI_SPI_DmaFailed();
            spi_status = Drv::SpiStatus::SPI_TRANSACT_DMA_ERR;
        }

        replyDma_out(portNum,
                     spi_status,
                     *writeBuffer,
                     *readBuffer);
    }
}
