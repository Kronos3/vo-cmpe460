#ifndef VO_CMPE460_KERNEL_H
#define VO_CMPE460_KERNEL_H

#include <circle/timer.h>
#include <circle/actled.h>
#include <circle/sched/scheduler.h>
#include <circle/serial.h>
#include <circle/memory.h>
#include <circle/i2cmaster.h>
#include <circle/spimasterdma.h>

namespace kernel
{
    extern CMemorySystem memorySystem;
    extern CInterruptSystem interruptSystem;
    extern CScheduler scheduler;
    extern CTimer tim;
    extern CActLED led;
    extern CSerialDevice serial;

    // External communication
//    extern CI2CMaster i2c;
//    extern CSPIMasterDMA spi;

//    extern CDeviceNameService deviceNameService;
//    extern CEMMCDevice sdcard;
//    extern FATFS root;
//    extern DpEngine dpEngine;
//    extern EvrEngine evrEngine;
}

#endif //VO_CMPE460_KERNEL_H
