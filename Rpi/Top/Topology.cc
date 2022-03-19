#include <circle/timer.h>
#include <Rpi/Top/Components.hpp>
#include <VoCarCfg.h>
#include <FprimeProtocol.hpp>
#include <MallocAllocator.hpp>

namespace Rpi
{
    enum
    {
        QUEUE_DEPTH = 32,
        FILE_UPLINK_QUEUE_DEPTH = 4096
    };

    enum {
        UPLINK_BUFFER_STORE_SIZE = 4096,
        UPLINK_BUFFER_QUEUE_SIZE = 4096,
        UPLINK_BUFFER_MGR_ID = 200
    };

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

//    Svc::CmdSequencerComponentImpl cmdSeq("SEQ");
    Svc::CommandDispatcherImpl cmdDisp("DISP");

    Svc::ActiveLoggerImpl eventLogger("LOG");
    Svc::TlmChanImpl chanTlm("TLM");
    Svc::PrmDbImpl prmDb("PRM", "/prm.dat");

    Svc::DeframerComponentImpl uplink("UPLINK");
    Svc::FramerComponentImpl downlink("DOWNLINK");
    Svc::FprimeDeframing deframing;
    Svc::FprimeFraming framing;
    Svc::FileUplink fileUplink("fileUplink");
    Fw::MallocAllocator mallocator;
    Svc::BufferManagerComponentImpl fileUplinkBufferManager("fileUplinkBufferManager");

    Svc::FileManager fileManager("FILE_MGR");
    Svc::StaticMemoryComponentImpl staticMemory("STATIC_MEM");
//    Svc::FatalHandlerComponentImpl fatalHandler("FATAL_HANDLER");

    SystemTimeImpl systemTime;
    Drv::RpiSerialDriverImpl serialDriver;
    Drv::RpiI2cDriverImpl i2cDriver;
    Drv::RpiSocketIpDriverComponentImpl comm("COMM");

    static const U32 cs_lines[] = {CAM_0_CS, CAM_1_CS};
    Drv::RpiSpiDriverImpl spiDriver(cs_lines, FW_NUM_ARRAY_ELEMENTS(cs_lines));
    Svc::TestImpl test;

    Rpi::CamImpl cam("CAM");
    Rpi::MotImpl mot("MOT");

    void init()
    {
        rgDriver.init();
        rg1hz.init(QUEUE_DEPTH, 0);
        rg10hz.init(QUEUE_DEPTH, 1);
        rg20hz.init(QUEUE_DEPTH, 1);
        rg50hz.init(QUEUE_DEPTH, 3);

        // Connect to GDS
        static U8 hostIp[4] = {192, 168, 1, 220};
        comm.init(0, CIPAddress(hostIp), 50000, SOCKET_SEND_UDP);

//        cmdSeq.init(QUEUE_DEPTH, 0);
        cmdDisp.init(QUEUE_DEPTH, 0);

        eventLogger.init(QUEUE_DEPTH, 0);
        chanTlm.init(QUEUE_DEPTH, 0);
        prmDb.init(QUEUE_DEPTH, 0);

        staticMemory.init(0);
        uplink.init(0);
        downlink.init(0);

        downlink.setup(framing);
        uplink.setup(deframing);
        fileUplink.init(FILE_UPLINK_QUEUE_DEPTH, 0);
        fileUplinkBufferManager.init(0);

        // set up BufferManager instances
        Svc::BufferManagerComponentImpl::BufferBins upBuffMgrBins;
        memset(&upBuffMgrBins, 0, sizeof(upBuffMgrBins));
        upBuffMgrBins.bins[0].bufferSize = UPLINK_BUFFER_STORE_SIZE;
        upBuffMgrBins.bins[0].numBuffers = UPLINK_BUFFER_QUEUE_SIZE;
        fileUplinkBufferManager.setup(UPLINK_BUFFER_MGR_ID, 0, mallocator, upBuffMgrBins);

        fileManager.init(QUEUE_DEPTH, 0);
//        fatalHandler.init(0);

        systemTime.init(0);
        serialDriver.init(0);
        i2cDriver.init(0);
        spiDriver.init(0);

        test.init(QUEUE_DEPTH, 0);
        cam.init(QUEUE_DEPTH, 0);
        mot.init(0);
    }

    void start()
    {
        rg1hz.start();
        rg10hz.start();
        rg20hz.start();
        rg50hz.start();
        test.start();

        eventLogger.start();
        chanTlm.start();
        prmDb.start();

        fileUplink.start();
        fileManager.start();

        cmdDisp.start();
        cam.start();

        // Always start this last (or first, but not in the middle)
        comm.startSocketTask();
    }

    void reg_commands()
    {
//        cmdSeq.regCommands();
        cmdDisp.regCommands();
        eventLogger.regCommands();
        prmDb.regCommands();
        test.regCommands();
        cam.regCommands();
        mot.regCommands();
        fileManager.regCommands();
    }

    void loadParameters()
    {
        mot.loadParameters();
    }
}
