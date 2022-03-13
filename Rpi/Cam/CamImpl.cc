#include "CamImpl.h"

namespace Rpi
{
    CamImpl::CamImpl()
    : OV2640(), m_state(CAM_STATE_NOT_INITIALIZED),
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
            log_WARNING_HI_CamPanicI2CRead();
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
            log_WARNING_HI_CamPanicI2CWrite();
            m_state = CAM_STATE_PANIC;
        }
    }

    void CamImpl::schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context)
    {

    }
}
