#ifndef VO_CMPE460_RPISPIDRIVERIMPL_H
#define VO_CMPE460_RPISPIDRIVERIMPL_H

#include <src/RpiSpiDriver/RpiSpiDriverComponentAc.hpp>

namespace Drv
{
    class RpiSpiDriverImpl : public RpiSpiDriverComponentBase
    {
    public:
        RpiSpiDriverImpl();
        void init(
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );
    private:
        struct SpiDmaRequest
        {
            U32 chipSelect;
            NATIVE_INT_TYPE portNum;
            Fw::Buffer* writeBuffer;
            Fw::Buffer* readBuffer;

            SpiDmaRequest()
            : chipSelect(0), portNum(0),
              writeBuffer(nullptr), readBuffer(nullptr)
            {}
        };

        Drv::SpiStatus transactCommon(U32 chipSelect,
                                      Fw::Buffer &writeBuffer,
                                      Fw::Buffer &readBuffer);

        Drv::SpiStatus transactDma_handler(NATIVE_INT_TYPE portNum,
                                           U32 chipSelect,
                                           Fw::Buffer &writeBuffer,
                                           Fw::Buffer &readBuffer) override;

        Drv::SpiStatus transactPolling_handler(NATIVE_INT_TYPE portNum,
                                               U32 chipSelect,
                                               Fw::Buffer &writeBuffer,
                                               Fw::Buffer &readBuffer) override;

        void dmaSendReply(bool status);

        static void dmaCompletion_routine(bool status, void* this_);

        volatile bool m_busy;       //!< A Spi transaction is running
        SpiDmaRequest m_dma;        //!< Running SPI DMA request
    };
}

#endif //VO_CMPE460_RPISPIDRIVERIMPL_H
