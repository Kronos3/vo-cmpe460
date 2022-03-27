
#include "CamImpl.h"
#include <libcamera/libcamera.h>

namespace Rpi
{
    struct CamImpl_
    {
        libcamera::ControlList controls;
    };

    CamImpl::CamImpl()
    : impl_(new CamImpl_)
    {
//        libcamera::controls::MeteringMatrix
//        impl_->controls.set(libcamera::controls::ExposureTime, )
    }

    CamImpl::~CamImpl()
    {
        delete impl_;
    }
}
