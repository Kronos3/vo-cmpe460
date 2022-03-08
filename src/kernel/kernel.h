#ifndef VO_CMPE460_KERNEL_H
#define VO_CMPE460_KERNEL_H

#include <circle/timer.h>
#include <circle/actled.h>

#include <circle/i2cmaster.h>
#include <circle/spimasterdma.h>

#include <rate_driver/rate_driver.h>
#include <rate_driver/rate_group.h>

#include <ardu_cam/ardu_cam.h>

namespace kernel
{
    extern CInterruptSystem interrupt_system;
    extern CTimer tim;
    extern RateDriver rgDriver;
    extern RateGroup rg1hz;
    extern RateGroup rg10Hz;
    extern RateGroup rg50Hz;

    // External communication
    extern CI2CMaster i2c;
    extern CSPIMasterDMA spi;

    extern CActLED led;
    extern OV2640 cam0;
    extern OV2640 cam1;
}

#endif //VO_CMPE460_KERNEL_H
