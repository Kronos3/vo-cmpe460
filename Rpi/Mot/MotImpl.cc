#include "MotImpl.h"


#define MSP_SLAVE_ADDRESS (0x48)

namespace Rpi
{
    MotImpl::MotImpl(const char* name)
    : MotComponentBase(name),
      m_steering(0.0), m_left(0.0), m_right(0.0)
    {
    }

    void MotImpl::init(NATIVE_INT_TYPE instance)
    {
        MotComponentBase::init(instance);
    }

//    void MotImpl::parameterUpdated(FwPrmIdType id)
//    {
//        MotComponentBase::parameterUpdated(id);
//
//        // Update the current request given the parameter
//        send();
//    }

    void MotImpl::get_handler(NATIVE_INT_TYPE portNum,
                              F64 & steering, F64 &left, F64 &right)
    {
        steering = m_steering;
        left = m_left;
        right = m_right;
    }

    void MotImpl::STEER_cmdHandler(U32 opCode, U32 cmdSeq, F64 position)
    {
        bool status = set_steering(position);
        cmdResponse_out(opCode, cmdSeq, status ? Fw::COMMAND_OK : Fw::COMMAND_EXECUTION_ERROR);
    }

    bool MotImpl::set_steering(F64 position)
    {
        position = FW_MIN(1.0, position);
        position = FW_MAX(-1.0, position);

        // Translate position to 0 for left and 1 for right
        F64 normal = (position + 1) / 2.0;

        // Convert 0 - 1 (F64) position to 0 - 255 (U8)
        MSP432Pwm pwm(MSP432PwmOpcode::SERVO, static_cast<U8>(normal * UINT8_MAX));

        // Send the packet via i2c
        Drv::I2cStatus status = send_packet(pwm);

        if (status == Drv::I2cStatus::I2C_OK)
        {
            m_steering = position;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool MotImpl::set_throttle(F64 left, F64 right)
    {
        MSP432PwmOpcode left_opcode = left >= 0 ?
                                      MSP432PwmOpcode::DC0_FORWARD : MSP432PwmOpcode::DC0_BACKWARD;
        MSP432PwmOpcode right_opcode = right >= 0 ?
                                       MSP432PwmOpcode::DC1_FORWARD : MSP432PwmOpcode::DC1_BACKWARD;

        // Clamp motors to max pwm
        F64 left_norm = FW_MIN(FW_ABS(left), 1.0);
        F64 right_norm = FW_MIN(FW_ABS(right), 1.0);

        MSP432Pwm left_packet(left_opcode, static_cast<U8>(left_norm * UINT8_MAX));
        MSP432Pwm right_packet(right_opcode, static_cast<U8>(right_norm * UINT8_MAX));

        Drv::I2cStatus status;

        status = send_packet(left_packet);
        if (status == Drv::I2cStatus::I2C_OK)
        {
            m_left = left;
        }
        else
        {
            // Failed to set left motor
            // Quit early and exit with failure
            return false;
        }

        status = send_packet(right_packet);
        if (status == Drv::I2cStatus::I2C_OK)
        {
            m_right = right;
            return true;
        }
        else
        {
            return false;
        }
    }

    void MotImpl::steer_handler(NATIVE_INT_TYPE portNum, F64 steer)
    {
        set_steering(steer);
    }

    void MotImpl::throttle_handler(NATIVE_INT_TYPE portNum, F64 left, F64 right)
    {
        set_throttle(left, right);
    }

    Drv::I2cStatus MotImpl::send_packet(MSP432Pwm& packet)
    {
        BYTE serialized[2];
        Fw::Buffer buf(serialized, sizeof serialized);
        Fw::SerializeBufferBase& ser = buf.getSerializeRepr();

        Fw::SerializeStatus status;
        status = ser.serialize(static_cast<U8>(packet.getopcode().e));
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);

        status = ser.serialize(packet.getvalue());
        FW_ASSERT(status == Fw::FW_SERIALIZE_OK, status);

        Drv::I2cStatus i2c_status = command_out(0, MSP_SLAVE_ADDRESS, buf);
        if (i2c_status != Drv::I2cStatus::I2C_OK)
        {
            log_WARNING_HI_MotI2cFailure(packet.getopcode(), i2c_status);
        }

        return i2c_status;
    }

    void MotImpl::THROTTLE_cmdHandler(U32 opCode, U32 cmdSeq, F64 left, F64 right)
    {
        bool status = set_throttle(left, right);
        cmdResponse_out(opCode, cmdSeq, status ? Fw::COMMAND_OK : Fw::COMMAND_EXECUTION_ERROR);
    }
}
