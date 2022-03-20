#ifndef VO_CMPE460_VO_CAR_H
#define VO_CMPE460_VO_CAR_H

#include <circle/gpiopin.h>

enum
{
    I2C_DEVICE_NUMBER = 1,      //!< 0 on Raspberry Pi 1 Rev. 1 boards, 1 otherwise
    I2C_CONFIG_NUMBER = 0,      //!< 0 or 1 on Raspberry Pi 4, 0 otherwise
    I2C_FAST_MODE = TRUE,       //!< standard mode (100 Kbps) or fast mode (400 Kbps)

    I2C_MUX_A2_A1_A0 = 0x0,     //!< Pin connection to I2C mux

    // Camera pin configuration
    CAM_0_CS = 0x0,             //!< SPI chip select for Camera 0
    CAM_0_MUX = 0,              //!< Camera 0 on I2C mux 0
    CAM_1_CS = 0x1,             //!< SPI chip select for Camera 1
    CAM_1_MUX = 1,              //!< Camera 1 on I2C mux 1

    // SPI configuration for ArduCam
    SPI_SPEED = 8000000,        //!< 4Mhz, half of the maximum speed of the camera
    SPI_POL = 0,
    SPI_PHA = 1,
};

#define DRIVE               "SD:"
#define FIRMWARE_PATH       DRIVE "/firmware/"
#define CONFIG_FILE         DRIVE "/wpa_supplicant.conf"

#endif //VO_CMPE460_VO_CAR_H
