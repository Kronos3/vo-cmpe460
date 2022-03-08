#include <cfg/vo_car.h>

#include <kernel/kernel.h>
#include <i2c_mux/i2c_mux.h>

namespace kernel
{
    CInterruptSystem interrupt_system;
    CTimer tim(&interrupt_system);

    RateDriver rgDriver;
    RateGroup rg1hz(&rgDriver, 100);
    RateGroup rg10Hz(&rgDriver, 10);
    RateGroup rg50Hz(&rgDriver, 2);

    CI2CMaster i2c(I2C_DEVICE_NUMBER, I2C_FAST_MODE);
    CSPIMasterDMA spi(&interrupt_system, SPI_SPEED, SPI_POL, SPI_PHA);

    OV2640 cam0(CAM_0_CS, CAM_0_MUX);
    OV2640 cam1(CAM_1_CS, CAM_1_MUX);

    CActLED led;
}

void blinky_interrupt(void* param)
{
    static boolean i = FALSE;
    i = !i;
    auto* led = reinterpret_cast<CActLED*>(param);
    if (i)
    {
        led->On();
    }
    else
    {
        led->Off();
    }
}

s32 main()
{
    kernel::interrupt_system.Initialize();
    kernel::tim.Initialize();
    kernel::i2c.Initialize();
    kernel::spi.Initialize();

    i2c_mux_init(I2C_MUX_A2_A1_A0);

    kernel::led.On();
//    kernel::cam0.init();
//    kernel::cam1.init();
    kernel::led.Off();

    kernel::rg10Hz.add(blinky_interrupt, &kernel::led);
    kernel::rg10Hz.start();

    while(TRUE)
    {
        __asm__ (" wfe");
    }
}
