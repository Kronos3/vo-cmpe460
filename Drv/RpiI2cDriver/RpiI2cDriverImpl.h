#ifndef VO_CMPE460_RPII2CDRIVERIMPL_H
#define VO_CMPE460_RPII2CDRIVERIMPL_H

#include <Drv/RpiI2cDriver/RpiI2cDriverComponentAc.hpp>

namespace Drv
{
    class RpiI2cDriverImpl : public RpiI2cDriverComponentBase
    {

    public:
        RpiI2cDriverImpl();

        void init(
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );

    private:
        Drv::I2cStatus read_handler(NATIVE_INT_TYPE portNum, U32 addr, Fw::Buffer &serBuffer) override;

        Drv::I2cStatus writeRead_handler(NATIVE_INT_TYPE portNum, U32 addr, Fw::Buffer &writeBuffer, Fw::Buffer &readBuffer) override;

        Drv::I2cStatus write_handler(NATIVE_INT_TYPE portNum, U32 addr, Fw::Buffer &serBuffer) override;
    };
}

#endif //VO_CMPE460_RPII2CDRIVERIMPL_H
