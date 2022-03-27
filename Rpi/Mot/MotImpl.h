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
//        void parameterUpdated(
//                FwPrmIdType id /*!< The parameter ID*/
//        ) override;

        void get_handler(NATIVE_INT_TYPE portNum, F64 &steering, F64 &left, F64 &right) override;
        void steer_handler(NATIVE_INT_TYPE portNum, F64 steer) override;
        void throttle_handler(NATIVE_INT_TYPE portNum, F64 left, F64 right) override;

        void STEER_cmdHandler(U32 opCode, U32 cmdSeq, F64 position) override;
        void THROTTLE_cmdHandler(U32 opCode, U32 cmdSeq, F64 left, F64 right) override;

        // Internal helper functions used across port handlers and commanding
        bool set_steering(F64 position);
        bool set_throttle(F64 left, F64 right);

        /**
         * Send a packet down to the MSP432 via I2C
         * @param packet packet to serialize
         * @return i2c send status
         */
        Drv::I2cStatus send_packet(MSP432Pwm& packet);

    PRIVATE:
        F64 m_steering;
        F64 m_left;
        F64 m_right;
    };
}

#endif //VO_CMPE460_MOTIMPL_H
