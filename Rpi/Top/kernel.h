#ifndef VO_CMPE460_KERNEL_H
#define VO_CMPE460_KERNEL_H

#include <Assert.hpp>
#include <circle/timer.h>
#include <circle/actled.h>
#include <circle/sched/scheduler.h>
#include <circle/serial.h>
#include <circle/memory.h>
#include <circle/i2cmaster.h>
#include <circle/spimasterdma.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/net/netsubsystem.h>

namespace kernel
{
    extern CMemorySystem memorySystem;
    extern CInterruptSystem interruptSystem;
    extern CScheduler scheduler;
    extern CTimer tim;
    extern CActLED led;
    extern CSerialDevice serial;

    extern CNetSubSystem net;

    // External communication
    extern CI2CMaster i2c;
    extern CSPIMasterDMA spi;
}

#endif //VO_CMPE460_KERNEL_H
