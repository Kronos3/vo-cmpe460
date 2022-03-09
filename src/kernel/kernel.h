#ifndef VO_CMPE460_KERNEL_H
#define VO_CMPE460_KERNEL_H

#include <circle/timer.h>
#include <circle/actled.h>

#include <ardu_cam/ardu_cam.h>
#include <circle/i2cmaster.h>
#include <circle/spimasterdma.h>

#include <SDCard/emmc.h>
#include <circle/fs/fat/fatfs.h>

#include <rate_driver/rate_driver.h>
#include <rate_driver/rate_group.h>

#include <evr/evr.h>
#include <dp/dp.h>
#include <circle/memory.h>
#include <circle/devicenameservice.h>
#include <dbg/dbg_button.h>
#include <dbg/dbg_act.h>
#include <fatfs/ff.h>

namespace kernel
{
    extern CInterruptSystem interruptSystem;
    extern CTimer tim;
    extern RateDriver rgDriver;
    extern RateGroup rg1hz;
    extern RateGroup rg10hz;
    extern RateGroup rg50hz;

    extern CActLED actLed;
    extern DbgAct act;
    extern DbgButton dbgButton;

    // External communication
    extern CI2CMaster i2c;
    extern CSPIMasterDMA spi;
//    extern OV2640 cam0;
//    extern OV2640 cam1;

    extern CDeviceNameService deviceNameService;
    extern CEMMCDevice sdcard;
    extern FATFS root;
    extern DpEngine dpEngine;
    extern EvrEngine evrEngine;
}

#endif //VO_CMPE460_KERNEL_H
