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

#include <Drv/RpiSerialDriver/RpiSerialDriverImpl.h>
#include <Drv/RpiI2cDriver/RpiI2cDriverImpl.h>
#include <Drv/RpiSpiDriver/RpiSpiDriverImpl.h>
#include <Drv/RpiSocketIpDriver/RpiSocketIpDriverComponentImpl.hpp>

#include <Rpi/SystemTime/SystemTimeImpl.h>
#include <Rpi/Cam/CamImpl.h>
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

    extern Rpi::SystemTimeImpl systemTime;
    extern Drv::RpiSerialDriverImpl serialDriver;
    extern Drv::RpiI2cDriverImpl i2cDriver;
    extern Drv::RpiSpiDriverImpl spiDriver;
    extern Drv::RpiSocketIpDriverComponentImpl comm;

    extern Svc::TestImpl test;

//    extern Rpi::CamImpl camL;
//    extern Rpi::CamImpl camR;
    extern Rpi::CamImpl cam;
    extern Rpi::MotImpl mot;

    // FSW entry points
    void init();
    void start();
    void reg_commands();
    void loadParameters();

    // Main entry point of FSW
    void fsw_start();
}

using namespace Rpi;

// Autocoded definition
void constructRpiArchitecture();

#endif //RPI_COMPONENTS_H
