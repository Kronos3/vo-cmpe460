#include <cfg/vo_car.h>

#include <kernel/kernel.h>
#include <i2c_mux/i2c_mux.h>
#include <fw/fw.h>
#include <evrs.h>
#include <circle/gpiomanager.h>

namespace kernel
{
    CInterruptSystem interruptSystem;
    CTimer tim(&interruptSystem);

    CActLED actLed;
    DbgAct act(&actLed);
    CGPIOManager gpioManager(&interruptSystem);
    DbgButton dbgButton(DBG_BUTTON_1_PIN, &act, &gpioManager);

    RateDriver rgDriver(&tim);
    RateGroup rg1hz(&rgDriver, 100);
    RateGroup rg10hz(&rgDriver, 10);
    RateGroup rg50hz(&rgDriver, 2);

    CI2CMaster i2c(I2C_DEVICE_NUMBER, I2C_FAST_MODE);
    CSPIMasterDMA spi(&interruptSystem, SPI_SPEED, SPI_POL, SPI_PHA);

//    OV2640 cam0(CAM_0_CS, CAM_0_MUX);
//    OV2640 cam1(CAM_1_CS, CAM_1_MUX);

    CDeviceNameService deviceNameService;
    CEMMCDevice sdcard(&interruptSystem, &tim, nullptr);
    FATFS root;
    DpEngine dpEngine;
    EvrEngine evrEngine(&dpEngine);
}

class FlushButton : public Button
{

};

s32 main()
{
    boolean ok;
    ok = kernel::interruptSystem.Initialize();
    FW_ASSERT(ok && "Failed to initialize interrupt system");
    ok = kernel::tim.Initialize();
    FW_ASSERT(ok && "Failed to initialize tim");
    ok = kernel::i2c.Initialize();
    FW_ASSERT(ok && "Failed to initialize i2c");
    ok = kernel::spi.Initialize();
    FW_ASSERT(ok && "Failed to initialize spi");
    ok = kernel::gpioManager.Initialize();
    FW_ASSERT(ok && "Failed to initialize gpioManager");
    ok = kernel::sdcard.Initialize();
    FW_ASSERT(ok && "Failed to sdcard");

    kernel::dbgButton.Initialize();
    kernel::evrEngine.init();

    i2c_mux_init(&kernel::i2c, I2C_MUX_A2_A1_A0);

    // Mount root file system
    auto mount_status = f_mount(&kernel::root, SD_DRIVE, 1);
    FW_ASSERT(mount_status == FR_OK && "Failed to mount root filesystem");

    kernel::dpEngine.init();

    kernel::evrEngine.open(SD_DRIVE "/evr.dat");
    kernel::actLed.On();

    evr_RootMounted(SD_DRIVE);
    evr_CarBoot(__DATE__, __TIME__);

    // 1 Hz processes
    kernel::rg1hz.add(&kernel::dpEngine);

    // 10 Hz processes
    kernel::rg10hz.add(&kernel::evrEngine);

    kernel::rg1hz.start();
    kernel::rg10hz.start();

//    kernel::cam0.init();
//    kernel::cam1.init();

    kernel::actLed.On();

    while(TRUE)
    {
        __asm__ (" wfe");
    }
}
