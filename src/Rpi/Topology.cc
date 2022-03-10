#include <circle/timer.h>
#include <src/Rpi/Components.hpp>

#define QUEUE_DEPTH (20)

namespace Rpi
{

    static NATIVE_INT_TYPE rgDivs[Svc::RateGroupDriverImpl::DIVIDER_SIZE] = {
            HZ / 1,
            HZ / 10,
            HZ / 20,
            HZ / 50,
    };
    Svc::RateGroupDriverImpl rgDriver("RGDRV", rgDivs, FW_NUM_ARRAY_ELEMENTS(rgDivs));

    U32 contexts[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    Svc::ActiveRateGroupImpl rg1hz("RG1HZ", contexts, FW_NUM_ARRAY_ELEMENTS(contexts));
    Svc::ActiveRateGroupImpl rg10hz("RG10HZ", contexts, FW_NUM_ARRAY_ELEMENTS(contexts));
    Svc::ActiveRateGroupImpl rg20hz("RG20HZ", contexts, FW_NUM_ARRAY_ELEMENTS(contexts));
    Svc::ActiveRateGroupImpl rg50hz("RG50HZ", contexts, FW_NUM_ARRAY_ELEMENTS(contexts));
//
//    Svc::CmdSequencerComponentImpl cmdSeq("SEQ");
//    Svc::CommandDispatcherImpl cmdDisp("DISP");
//
    Svc::ActiveLoggerImpl eventLogger("LOG");
    Svc::TlmChanImpl tlmChan("TLM");
    Svc::PrmDbImpl prmDb("PRM", "/prm.dat");
    Svc::GroundInterfaceComponentImpl groundIf("GRD_IF");
//
//    Svc::FileManager fileManager("FILE_MGR");
//    Svc::StaticMemoryComponentImpl staticMemory("STATIC_MEM");
//    Svc::FatalHandlerComponentImpl fatalHandler("FATAL_HANDLER");
//
    SystemTimeImpl systemTime;
    Drv::RpiSerialDriverImpl serialDriver;
    Svc::TestImpl test;

    void init()
    {
        rgDriver.init();
        rg1hz.init(QUEUE_DEPTH, 0);
        rg10hz.init(QUEUE_DEPTH, 1);
        rg20hz.init(QUEUE_DEPTH, 1);
        rg50hz.init(QUEUE_DEPTH, 3);
//
//        cmdSeq.init(QUEUE_DEPTH, 0);
//        cmdDisp.init(QUEUE_DEPTH, 0);
//
        eventLogger.init(QUEUE_DEPTH, 0);
        tlmChan.init(QUEUE_DEPTH, 0);
        prmDb.init(QUEUE_DEPTH, 0);
        groundIf.init(0);
//
//        fileManager.init(QUEUE_DEPTH, 0);
//        staticMemory.init(0);
//        fatalHandler.init(0);
//
        systemTime.init(0);
        serialDriver.init(0);
        test.init(QUEUE_DEPTH, 0);
    }

    void start()
    {
        rg1hz.start();
        rg10hz.start();
        rg20hz.start();
        rg50hz.start();
        test.start();

        eventLogger.start();
        tlmChan.start();
        prmDb.start();
    }

    void reg_commands()
    {
//        cmdSeq.regCommands();
//        eventLogger.regCommands();
//        prmDb.regCommands();
//        fileManager.regCommands();
    }
}
