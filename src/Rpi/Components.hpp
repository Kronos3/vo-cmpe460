#ifndef RPI_COMPONENTS_H
#define RPI_COMPONENTS_H

// FPrime Core
#include <Svc/ActiveRateGroup/ActiveRateGroupImpl.hpp>
#include <Svc/RateGroupDriver/RateGroupDriverImpl.hpp>

//#include <Svc/CmdDispatcher/CommandDispatcherImpl.hpp>
//#include <Svc/CmdSequencer/CmdSequencerImpl.hpp>

#include <Svc/ActiveLogger/ActiveLoggerImpl.hpp>
#include <Svc/TlmChan/TlmChanImpl.hpp>
#include <Svc/PrmDb/PrmDbImpl.hpp>
#include <Svc/GroundInterface/GroundInterface.hpp>

//#include <Svc/FileManager/FileManager.hpp>
//#include <Svc/StaticMemory/StaticMemoryComponentImpl.hpp>
//#include <Svc/FatalHandler/FatalHandlerComponentImpl.hpp>

#include <src/SystemTime/SystemTimeImpl.h>
#include <src/RpiSerialDriver/RpiSerialDriverImpl.h>
#include <src/RpiI2cDriver/RpiI2cDriverImpl.h>
#include <src/RpiSpiDriver/RpiSpiDriverImpl.h>

#include <src/Test/TestImpl.h>
#include <src/Cam/CamImpl.h>

namespace Rpi
{
    extern Svc::RateGroupDriverImpl rgDriver;
    extern Svc::ActiveRateGroupImpl rg1hz;
    extern Svc::ActiveRateGroupImpl rg10hz;
    extern Svc::ActiveRateGroupImpl rg20hz;
    extern Svc::ActiveRateGroupImpl rg50hz;

//    extern Svc::CmdSequencerComponentImpl cmdSeq;
//    extern Svc::CommandDispatcherImpl cmdDisp;

    extern Svc::ActiveLoggerImpl eventLogger;
    extern Svc::TlmChanImpl tlmChan;
    extern Svc::PrmDbImpl prmDb;
    extern Svc::GroundInterfaceComponentImpl groundIf;

//    extern Svc::FileManager fileManager;
//    extern Svc::StaticMemoryComponentImpl staticMemory;
//    extern Svc::FatalHandlerComponentImpl fatalHandler;

    extern Rpi::SystemTimeImpl systemTime;
    extern Drv::RpiSerialDriverImpl serialDriver;
    extern Drv::RpiI2cDriverImpl i2cDriver;
    extern Drv::RpiSpiDriverImpl spiDriver;
    extern Svc::TestImpl test;

    extern Rpi::CamImpl camL;
    extern Rpi::CamImpl camR;

    // FSW entry points
    void init();
    void start();
    void reg_commands();
}

using namespace Rpi;

// Autocoded definition
void constructRpiArchitecture();

#endif //RPI_COMPONENTS_H
