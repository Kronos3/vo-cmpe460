//#include <cfg/vo_car.h>

#include <circle/memory.h>
#include <RPICfg.hpp>
#include <circle/sched/scheduler.h>
#include <VoCarCfg.h>
#include <Logger.hpp>
#include <Logger.h>
#include <SDCard/emmc.h>
#include <fatfs/ff.h>
#include <wlan/bcm4343.h>
#include <wlan/hostap/wpa_supplicant/wpasupplicant.h>
#include <circle/devicenameservice.h>
#include <circle/debug.h>
#include "Components.hpp"

#include "kernel.h"

#include <unwind.h> // GCC's internal unwinder, part of libgcc



class RpiAssertHook : public Fw::AssertHook
{
public:
    static _Unwind_Reason_Code printStackTrace(
            struct _Unwind_Context* ctx, void* arg)
    {
        (void) arg;
        _Unwind_Ptr ip = _Unwind_GetIP(ctx);
        Fw::Logger::logMsg("ip: 0x%x\r\n", ip);
        return _URC_CONTINUE_UNWIND;
    }

    void printAssert(const I8* msg) override
    {
//        _Unwind_Backtrace(printStackTrace, nullptr);
        Fw::Logger::logMsg("%s\r\n", (POINTER_CAST)msg);
    }

    void doAssert() override
    {
        EnterCritical();
        // Hang this task
        while(TRUE)
        {
            for (unsigned i = 1; i <= 2; i++)
            {
                kernel::led.On();
                kernel::tim.MsDelay(100);

                kernel::led.Off();
                kernel::tim.MsDelay(100);
            }

            kernel::tim.MsDelay(300);
        }
    }
};


namespace kernel
{
    CMemorySystem memorySystem;
    CInterruptSystem interruptSystem;
    CScheduler scheduler;
    CActLED led;
    CTimer tim(&interruptSystem);
    Rpi::Logger logger;

    CSerialDevice serial(&interruptSystem, /* fiq */ TRUE);
    CI2CMaster i2c(I2C_DEVICE_NUMBER, I2C_FAST_MODE);
    CSPIMasterDMA spi(&interruptSystem,
                      SPI_SPEED,
                      SPI_POL, SPI_PHA,
                      /* DMAChannelLite */ FALSE);

    RpiAssertHook assertHook;

    CDeviceNameService deviceNameService;
    CUSBHCIDevice usbhci(&interruptSystem, &tim);
    CEMMCDevice emmc(&interruptSystem, &tim, &led);
    FATFS filesystem;
    CBcm4343Device wlan(FIRMWARE_PATH);
    CNetSubSystem net(
            /* IPAddress (use DHCP) */ nullptr,
            /* Netmask */ nullptr,
            /* DefaultGateway */ nullptr,
            /* DNSServer */ nullptr,
            DEFAULT_HOSTNAME,
            NetDeviceTypeEthernet);
    CWPASupplicant wpa_supplicant(CONFIG_FILE);
}

void assertion_failed (const char *pExpr, const char *pFile, unsigned nLine)
{
    Fw::Logger::logMsg("%s:%d %s\r\n", (POINTER_CAST)pFile, nLine, (POINTER_CAST)pExpr);
    kernel::assertHook.doAssert();
}

s32 main()
{
    boolean hardOk;
    Fw::Logger::registerLogger(&kernel::logger);
    hardOk = kernel::interruptSystem.Initialize();
    FW_ASSERT(hardOk);
    hardOk = kernel::tim.Initialize();
    FW_ASSERT(hardOk);
    hardOk = kernel::serial.Initialize(UART_BAUDRATE);
    FW_ASSERT(hardOk);

    kernel::led.On();
    kernel::assertHook.registerHook();

    Fw::Logger::logMsg("Booting up\r\n");

    Fw::Logger::logMsg("Initializing hardware\r\n");

    Fw::Logger::logMsg("Initializing I2C/SPI\r\n");
    kernel::i2c.Initialize();
    kernel::spi.Initialize();

    Fw::Logger::logMsg("Initializing USB\r\n");
    hardOk = kernel::usbhci.Initialize();
    FW_ASSERT(hardOk);

    Fw::Logger::logMsg("Initializing SDCard\r\n");
    hardOk = kernel::emmc.Initialize();
    FW_ASSERT(hardOk);

    if (f_mount(&kernel::filesystem, DRIVE, 1) != FR_OK)
    {
        Fw::Logger::logMsg("Failed to mount drive: %s\r\n", (POINTER_CAST)DRIVE);
        FW_ASSERT(0);
    }
    else
    {
        Fw::Logger::logMsg("Mounted %s to /\r\n", (POINTER_CAST)DRIVE);
    }

#ifdef USE_WLAN
    Fw::Logger::logMsg("Initializing WLAN\r\n");
    hardOk = kernel::wlan.Initialize();
    FW_ASSERT(hardOk);
#endif

    Fw::Logger::logMsg("Initializing Network\r\n");
    hardOk = kernel::net.Initialize(TRUE);
    FW_ASSERT(hardOk);

#ifdef USE_WLAN
    Fw::Logger::logMsg("Initializing WPA Supplicant\r\n");
    hardOk = kernel::wpa_supplicant.Initialize();
    FW_ASSERT(hardOk);
#endif

    CString ip_address;
    kernel::net.GetConfig()->GetIPAddress()->Format(&ip_address);
    Fw::Logger::logMsg("Raspberry Pi IP: %s\r\n", (POINTER_CAST) (const char*) ip_address);

    Fw::Logger::logMsg("Initializing components\r\n");
    Rpi::init();

    Fw::Logger::logMsg("Initializing port connections\r\n");
    constructRpiArchitecture();

    Fw::Logger::logMsg("Starting active tasks\r\n");
    Rpi::start();

//    Rpi::prmDb.readParamFile();

    Fw::Logger::logMsg("Registering commands\r\n");
    reg_commands();

    Fw::Logger::logMsg("Boot complete\r\n");

    while (TRUE)
    {
        kernel::scheduler.Yield();
    }
}
