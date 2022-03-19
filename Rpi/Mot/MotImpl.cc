#include "MotImpl.h"


#define MSP_SLAVE_ADDRESS (0x48)

namespace Rpi
{
    // Make sure the pwm packet size is sane
    FW_STATIC_ASSERT(MSP432Pwm::SERIALIZED_SIZE == 3);

    MotImpl::MotImpl(const char* name)
    : MotComponentBase(name),
      m_steering(0.0), m_left(0.0), m_right(0.0)
    {
    }

    void MotImpl::init(NATIVE_INT_TYPE instance)
    {
        MotComponentBase::init(instance);
    }

    void MotImpl::parameterUpdated(FwPrmIdType id)
    {
        MotComponentBase::parameterUpdated(id);

        // Update the current request given the parameter
        send();
    }

    void MotImpl::get_handler(NATIVE_INT_TYPE portNum,
                              F64 & steering, F64 &left, F64 &right)
    {
        steering = m_steering;
        left = m_left;
        right = m_right;
    }

    void MotImpl::send()
    {
        // TODO(tumbar) Set the DC values

    }

    void MotImpl::STEER_cmdHandler(U32 opCode, U32 cmdSeq, F64 position)
    {
        // Scale -1.0 to 1.0
        bool status = set_steering(position);
        cmdResponse_out(opCode, cmdSeq, status ? Fw::COMMAND_OK : Fw::COMMAND_EXECUTION_ERROR);
    }

    bool MotImpl::set_steering(F64 position)
    {
        FW_ASSERT(position >= -1.0 && position <= 1.0, position * 100);

        // Translate position to 0 for left and 1 for right
        position = (position + 1) / 2.0;
        FW_ASSERT(position >= 0.0 && position <= 1.0, position * 100);

        // Grab the parameter that control the PWM limits
        Fw::ParamValid valid;
        F32 left_pwm = paramGet_STEERING_LEFT_PWM(valid);
        F32 right_pwm = paramGet_STEERING_RIGHT_PWM(valid);

        F64 duty_cycle;

        // Validate the duty cycle is between the limits
        if (left_pwm < right_pwm)
        {
            duty_cycle = position * (right_pwm - left_pwm) + left_pwm;
            FW_ASSERT(duty_cycle >= left_pwm && duty_cycle <= right_pwm,
                      duty_cycle * 100,
                      left_pwm * 100,
                      right_pwm * 100);
        }
        else
        {
            duty_cycle = position * (right_pwm - left_pwm) + right_pwm;
            FW_ASSERT(duty_cycle <= left_pwm && duty_cycle >= right_pwm,
                      duty_cycle * 100,
                      left_pwm * 100,
                      right_pwm * 100);
        }

        // Normalize duty_cycle to 0 to UINT16_MAX
        U16 normalized = static_cast<U16>(duty_cycle * __UINT16_MAX__);
        steering_pwm.set(normalized, true);

        U8 internal_buf[3];
        Fw::ExternalSerializeBuffer steering_ser(internal_buf,  sizeof(internal_buf));
        steering_pwm.serialize(steering_ser);
        Fw::Buffer steering_send(internal_buf, sizeof(internal_buf));

        Drv::I2cStatus status = writeSteering_out(0, MSP_SLAVE_ADDRESS, steering_send);
        return status == Drv::I2cStatus::I2C_OK;
    }

    void MotImpl::steer_handler(NATIVE_INT_TYPE portNum, F64 steer)
    {
        set_steering(steer);
    }

    void MotImpl::throttle_handler(NATIVE_INT_TYPE portNum, F64 left, F64 right)
    {
        // TODO(tumbar) TODO
    }
}
