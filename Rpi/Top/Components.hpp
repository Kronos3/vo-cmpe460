#ifndef RPI_COMPONENTS_H
#define RPI_COMPONENTS_H

// FPrime Core
#include <Svc/ActiveRateGroup/ActiveRateGroupImpl.hpp>
#include <Svc/RateGroupDriver/RateGroupDriverImpl.hpp>

#include <Svc/CmdDispatcher/CommandDispatcherImpl.hpp>
#include <Svc/CmdSequencer/CmdSequencerImpl.hpp>
#include <Svc/Deframer/DeframerComponentImpl.hpp>
#include <Svc/Framer/FramerComponentImpl.hpp>

#include <Svc/ActiveLogger/ActiveLoggerImpl.hpp>
#include <Svc/TlmChan/TlmChanImpl.hpp>
#include <Svc/PrmDb/PrmDbImpl.hpp>

#include <Svc/FileManager/FileManager.hpp>
#include <Svc/StaticMemory/StaticMemoryComponentImpl.hpp>
//#include <Svc/FatalHandler/FatalHandlerComponentImpl.hpp>
#include <Svc/FileUplink/FileUplink.hpp>
#include <Svc/FileDownlink/FileDownlink.hpp>

#include <Drv/LinuxSerialDriver/LinuxSerialDriverComponentImpl.hpp>
#include <Drv/LinuxI2cDriver/LinuxI2cDriverComponentImpl.hpp>
//#include <Drv/LinuxSpiDriver/LinuxSpiDriverComponentImpl.hpp>
#include <Drv/TcpClient/TcpClientComponentImpl.hpp>
#include <Svc/LinuxTime/LinuxTimeImpl.hpp>
#include <Svc/LinuxTimer/LinuxTimerComponentImpl.hpp>

#include <Rpi/Cam/CamImpl.h>
#include <Rpi/VideoStreamer/VideoStreamerImpl.h>
#include <Rpi/Mot/MotImpl.h>
#include <Rpi/Vis/VisImpl.h>
#include <Rpi/FramePipe/FramePipeImpl.h>

#include <Svc/BufferManager/BufferManagerComponentImpl.hpp>
#include <FprimeProtocol.hpp>
#include <MallocAllocator.hpp>
#include <Log.hpp>

class Kernel
{
    Os::Log os_logger;

    Svc::RateGroupDriverImpl rgDriver;
    Svc::ActiveRateGroupImpl rg1hz;
    Svc::ActiveRateGroupImpl rg10hz;
    Svc::ActiveRateGroupImpl rg20hz;
    Svc::ActiveRateGroupImpl rg50hz;

    Fw::MallocAllocator mallocAllocator;
    Svc::CmdSequencerComponentImpl cmdSeq;
    Svc::CommandDispatcherImpl cmdDisp;

    Svc::ActiveLoggerImpl eventLogger;
    Svc::TlmChanImpl chanTlm;
    Svc::PrmDbImpl prmDb;

    Svc::DeframerComponentImpl uplink;
    Svc::FramerComponentImpl downlink;
    Svc::FileUplink fileUplink;
    Svc::BufferManagerComponentImpl fileUplinkBufferManager;
    Svc::FileDownlink fileDownlink;

    Svc::FileManager fileManager;
    Svc::StaticMemoryComponentImpl staticMemory;
//    Svc::FatalHandlerComponentImpl fatalHandler;

    Svc::LinuxTimerComponentImpl linuxTimer;
    Svc::LinuxTimeImpl systemTime;
    Drv::LinuxSerialDriverComponentImpl serialDriver;
    Drv::LinuxI2cDriverComponentImpl i2cDriver;
//    Drv::LinuxSpiDriverComponentImpl spiDriver;
    Drv::TcpClientComponentImpl comm;


//    Rpi::CamImpl camL;
//    Rpi::CamImpl camR;
    Rpi::CamImpl cam;
    Rpi::VideoStreamerImpl videoStreamer;
    Rpi::MotImpl mot;
    Rpi::VisImpl vis;
    Rpi::FramePipeImpl framePipe;

    Svc::FprimeDeframing deframing;
    Svc::FprimeFraming framing;
    Fw::MallocAllocator mallocator;

    // FSW entry points
    void prv_init();
    void prv_start();
    void prv_reg_commands();
    void prv_loadParameters();

    void setRpiRTLinuxIds();
    void constructRpiRTLinuxArchitecture();

public:
    Kernel();

    void start();
    void run();
    void exit();
};

#endif //RPI_COMPONENTS_H
