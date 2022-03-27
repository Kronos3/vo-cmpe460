
#ifndef VO_CMPE460_CAMIMPL_H
#define VO_CMPE460_CAMIMPL_H

#include <Rpi/Cam/CamComponentAc.hpp>

namespace Rpi
{
    struct CamImpl_;
    class CamImpl : public CamComponentBase
    {
        CamImpl_* impl_;

    public:
        CamImpl();
        ~CamImpl() override;
    };
}

#endif //VO_CMPE460_CAMIMPL_H
