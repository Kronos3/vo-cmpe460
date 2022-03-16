#include <Drv/RpiI2cDriver/RpiI2cDriverImpl.h>
#include <Rpi/Top/kernel.h>

namespace Drv
{
    RpiI2cDriverImpl::RpiI2cDriverImpl() = default;

    void RpiI2cDriverImpl::init(NATIVE_INT_TYPE instance)
    {
        RpiI2cDriverComponentBase::init(instance);
    }

    Drv::I2cStatus RpiI2cDriverImpl::read_handler(NATIVE_INT_TYPE portNum, U32 addr, Fw::Buffer &serBuffer)
    {
        // Validate the I2C address
        // Circle only supported 7bit address
        if ((addr & 0x7F) != addr)
        {
            return Drv::I2cStatus::I2C_ADDRESS_ERR;
        }

        NATIVE_INT_TYPE status;
        U32 total_read = 0;
        do
        {
            status = kernel::i2c.Read(addr,
                                       serBuffer.getData() + total_read,
                                       serBuffer.getSize() - total_read);
            total_read += status;
        } while(status > 0 && total_read < serBuffer.getSize());

        if (status < 0)
        {
            log_WARNING_HI_I2cReadError(static_cast<const RpiI2cStatus::t>(-status));
            return Drv::I2cStatus::I2C_READ_ERR;
        }

        return Drv::I2cStatus::I2C_OK;
    }

    Drv::I2cStatus RpiI2cDriverImpl::writeRead_handler(NATIVE_INT_TYPE portNum, U32 addr,
                                                       Fw::Buffer &writeBuffer,
                                                       Fw::Buffer &readBuffer)
    {
        Drv::I2cStatus status;

        status = write_handler(portNum, addr, writeBuffer);

        // Don't continue if we failed to write
        if (status != Drv::I2cStatus::I2C_OK) return status;

        status = read_handler(portNum, addr, readBuffer);
        return status;
    }

    Drv::I2cStatus RpiI2cDriverImpl::write_handler(NATIVE_INT_TYPE portNum, U32 addr, Fw::Buffer &serBuffer)
    {
        // Validate the I2C address
        // Circle only supported 7bit address
        if ((addr & 0x7F) != addr)
        {
            return Drv::I2cStatus::I2C_ADDRESS_ERR;
        }

        NATIVE_INT_TYPE status;
        U32 total_write = 0;
        do
        {
            status = kernel::i2c.Write(addr,
                                      serBuffer.getData() + total_write,
                                      serBuffer.getSize() - total_write);
            total_write += status;
        } while(status > 0 && total_write < serBuffer.getSize());

        if (status < 0)
        {
            log_WARNING_HI_I2cWriteError(static_cast<const RpiI2cStatus::t>(-status));
            return Drv::I2cStatus::I2C_WRITE_ERR;
        }

        return Drv::I2cStatus::I2C_OK;
    }
}
