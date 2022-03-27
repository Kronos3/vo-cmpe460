#ifndef RPI_COMPONENTS_H
#define RPI_COMPONENTS_H

// FPrime Core
#include <Svc/ActiveRateGroup/ActiveRateGroupImpl.hpp>
#include <Svc/RateGroupDriver/RateGroupDriverImpl.hpp>

#include <Svc/CmdDispatcher/CommandDispatcherImpl.hpp>
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
#include <Drv/LinuxSpiDriver/LinuxSpiDriverComponentImpl.hpp>
#include <Drv/TcpClient/TcpClientComponentImpl.hpp>
#include <Svc/LinuxTime/LinuxTimeImpl.hpp>
#include <Svc/LinuxTimer/LinuxTimerComponentImpl.hpp>

//#include <Rpi/Cam/CamImpl.h>
#include <Rpi/Mot/MotImpl.h>

#include <Rpi/Test/TestImpl.h>
#include <Svc/BufferManager/BufferManagerComponentImpl.hpp>

namespace Rpi
{
    extern Svc::RateGroupDriverImpl rgDriver;
    extern Svc::ActiveRateGroupImpl rg1hz;
    extern Svc::ActiveRateGroupImpl rg10hz;
    extern Svc::ActiveRateGroupImpl rg20hz;
    extern Svc::ActiveRateGroupImpl rg50hz;

//    extern Svc::CmdSequencerComponentImpl cmdSeq;
    extern Svc::CommandDispatcherImpl cmdDisp;

    extern Svc::ActiveLoggerImpl eventLogger;
    extern Svc::TlmChanImpl chanTlm;
    extern Svc::PrmDbImpl prmDb;

    extern Svc::DeframerComponentImpl uplink;
    extern Svc::FramerComponentImpl downlink;
    extern Svc::FileUplink fileUplink;
    extern Svc::BufferManagerComponentImpl fileUplinkBufferManager;
    extern Svc::FileDownlink fileDownlink;

    extern Svc::FileManager fileManager;
    extern Svc::StaticMemoryComponentImpl staticMemory;
//    extern Svc::FatalHandlerComponentImpl fatalHandler;

    extern Svc::LinuxTimerComponentImpl linuxTimer;
    extern Svc::LinuxTimeImpl systemTime;
    extern Drv::LinuxSerialDriverComponentImpl serialDriver;
    extern Drv::LinuxI2cDriverComponentImpl i2cDriver;
//    extern Drv::LinuxSpiDriverComponentImpl spiDriver;
    extern Drv::TcpClientComponentImpl comm;

    extern Svc::TestImpl test;

//    extern Rpi::CamImpl camL;
//    extern Rpi::CamImpl camR;
//    extern Rpi::CamImpl cam;
    extern Rpi::MotImpl mot;

    // FSW entry points
    void init();
    void start();
    void reg_commands();
    void loadParameters();

    // Main entry point of FSW
    void fsw_start();
    void fsw_run();
    void fsw_exit();
}

using namespace Rpi;

// Autocoded definition
void constructRpiRTLinuxArchitecture();

#endif //RPI_COMPONENTS_H
