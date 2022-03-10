#ifndef RPI_COMPONENTS_H
#define RPI_COMPONENTS_H

// FPrime Core
#include <Svc/ActiveRateGroup/ActiveRateGroupImpl.hpp>
#include <Svc/RateGroupDriver/RateGroupDriverImpl.hpp>

#include <Svc/CmdDispatcher/CommandDispatcherImpl.hpp>
#include <Svc/CmdSequencer/CmdSequencerImpl.hpp>

#include <Svc/ActiveLogger/ActiveLoggerImpl.hpp>
#include <Svc/TlmChan/TlmChanImpl.hpp>
#include <Svc/PrmDb/PrmDbImpl.hpp>

#include <Svc/FileManager/FileManager.hpp>
#include <Svc/StaticMemory/StaticMemoryComponentImpl.hpp>
#include <Svc/FatalHandler/FatalHandlerComponentImpl.hpp>

#include <src/SystemTime/SystemTimeImpl.h>
#include <src/RPISerialDriver/RPISerialDriverImpl.h>
#include <src/Test/TestImpl.h>

namespace RPI
{
    extern Svc::RateGroupDriverImpl rgDriver;
    extern Svc::ActiveRateGroupImpl rg1hz;
    extern Svc::ActiveRateGroupImpl rg10hz;
    extern Svc::ActiveRateGroupImpl rg20hz;
    extern Svc::ActiveRateGroupImpl rg50hz;

    extern Svc::CmdSequencerComponentImpl cmdSeq;
    extern Svc::CommandDispatcherImpl cmdDisp;

    extern Svc::ActiveLoggerImpl eventLogger;
    extern Svc::TlmChanImpl tlmChan;
    extern Svc::PrmDbImpl prmDb;

    extern Svc::FileManager fileManager;
    extern Svc::StaticMemoryComponentImpl staticMemory;
    extern Svc::FatalHandlerComponentImpl fatalHandler;

    extern RPI::SystemTimeImpl systemTime;
    extern Drv::RPISerialDriverImpl serialDriver;
    extern Svc::TestImpl test;

    // FSW entry points
    void init();
    void start();
    void reg_commands();
}

using namespace RPI;

// Autocoded definition
void constructRPIArchitecture();

#endif //RPI_COMPONENTS_H
