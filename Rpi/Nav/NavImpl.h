
#ifndef VO_CMPE460_NAVIMPL_H
#define VO_CMPE460_NAVIMPL_H

#include <Rpi/Nav/NavComponentAc.hpp>
#include <Rpi/Nav/NavAlgorithm.h>

namespace Rpi
{
    class NavImpl :
            public NavComponentBase,
            public CarController
    {
    public:
        explicit NavImpl(const char* compName);

        void init(NATIVE_INT_TYPE queue_depth, NATIVE_INT_TYPE instance);

    PRIVATE:
        const NavParams& params() const override;
        void parameterUpdated(FwPrmIdType id) override;
        void parametersLoaded() override;

        void frame_handler(NATIVE_INT_TYPE portNum, CamFrame *frame) override;

        void RUN_cmdHandler(U32 opCode, U32 cmdSeq, Rpi::NavAlgorithmType algorithm) override;
        void STOP_cmdHandler(U32 opCode, U32 cmdSeq) override;

        void steer(F32 steering) override;
        void throttle(F32 throttle_left, F32 throttle_right) override;

    PRIVATE:
        void clear();

        Rpi::NavAlgorithmType m_type;
        Rpi::NavAlgorithm* m_algorithm;
        NavParams m_params;
    };
}

#endif //VO_CMPE460_NAVIMPL_H
