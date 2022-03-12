#ifndef VO_CMPE460_CAMIMPL_H
#define VO_CMPE460_CAMIMPL_H

#include <src/Cam/CamComponentAc.hpp>
#include <src/Cam/ardu_cam.h>

namespace Rpi
{
    class CamImpl : public CamComponentBase, public OV2640
    {
    public:
        CamImpl();

        void init(
                NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
                NATIVE_INT_TYPE instance = 0 /*!< The instance number*/
        );

    private:
        void schedIn_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context) override;

        void read_i2c(U8 address, U8* dst, U32 len) override;
        void write_i2c(U8 address, U8* src, U32 len) override;

        enum CamState
        {
            CAM_STATE_NOT_INITIALIZED,      //!< Standard camera initialization has not been performed
            CAM_STATE_READY,                //!< Camera is ready for operation
            CAM_STATE_PANIC,                //!< Error during initialization or camera transaction
        };

        CamState m_state;
        U32 m_tlm_img_n;
        F32 m_tlm_hz;
    };
}

#endif //VO_CMPE460_CAMIMPL_H
