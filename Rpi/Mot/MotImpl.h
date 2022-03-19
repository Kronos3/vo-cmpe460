#ifndef VO_CMPE460_MOTIMPL_H
#define VO_CMPE460_MOTIMPL_H

#include <Rpi/Mot/MotComponentAc.hpp>
#include <Rpi/Mot/Types/MSP432PwmSerializableAc.hpp>

namespace Rpi
{
    class MotImpl : public MotComponentBase
    {
    public:
        MotImpl(const char* comp_name);

        void init(
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );

    PRIVATE:
        void parameterUpdated(
                FwPrmIdType id /*!< The parameter ID*/
        ) override;

        void get_handler(NATIVE_INT_TYPE portNum, F64 &steering, F64 &left, F64 &right) override;
        void steer_handler(NATIVE_INT_TYPE portNum, F64 steer) override;
        void throttle_handler(NATIVE_INT_TYPE portNum, F64 left, F64 right) override;

        void STEER_cmdHandler(U32 opCode, U32 cmdSeq, F64 position) override;

        bool set_steering(F64 position);

        void send();

    PRIVATE:
        F64 m_steering;
        F64 m_left;
        F64 m_right;

        // Hardware
        MSP432Pwm steering_pwm;
    };
}

#endif //VO_CMPE460_MOTIMPL_H
