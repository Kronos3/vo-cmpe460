#ifndef VO_CMPE460_RPISPIDRIVERIMPL_H
#define VO_CMPE460_RPISPIDRIVERIMPL_H

#include <Drv/RpiSpiDriver/RpiSpiDriverComponentAc.hpp>

namespace Drv
{
    class RpiSpiDriverImpl : public RpiSpiDriverComponentBase
    {
    public:
        RpiSpiDriverImpl();

        void init(
                NATIVE_INT_TYPE instance = 0
        );
    private:
        struct SpiDmaRequest
        {
            NATIVE_INT_TYPE portNum;
            Fw::Buffer* writeBuffer;
            Fw::Buffer* readBuffer;

            SpiDmaRequest()
            : portNum(0),
              writeBuffer(nullptr), readBuffer(nullptr)
            {}
        };

        Drv::SpiStatus transactCommon(Fw::Buffer &writeBuffer,
                                      Fw::Buffer &readBuffer);

        Drv::SpiStatus transactDma_handler(NATIVE_INT_TYPE portNum,
                                           Fw::Buffer &writeBuffer,
                                           Fw::Buffer &readBuffer) override;

        Drv::SpiStatus transactPolling_handler(NATIVE_INT_TYPE portNum,
                                               Fw::Buffer &writeBuffer,
                                               Fw::Buffer &readBuffer) override;

        void dmaSendReply(bool status);

        static void dmaCompletion_routine(bool status, void* this_);

        U32 m_tlm_RxTx;             //!< Total number of send and received bytes
        U32 m_tlm_SyncRxTx;         //!< Number of synchronous bytes sent/received
        U32 m_tlm_DmaRxTx;          //!< Number of DMA bytes sent/received
        U32 m_tlm_SyncN;            //!< Number of sync transfers
        U32 m_tlm_DmaN;             //!< Number of DMA transfers
        U32 m_tlm_SyncFailN;        //!< Number of failed sync transfers
        U32 m_tlm_DmaFailN;         //!< Number of failed DMA transfers
        volatile bool m_busy;       //!< A Spi transaction is running
        SpiDmaRequest m_dma;        //!< Running SPI DMA request
    };
}

#endif //VO_CMPE460_RPISPIDRIVERIMPL_H
