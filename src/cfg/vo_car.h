#ifndef VO_CMPE460_VO_CAR_H
#define VO_CMPE460_VO_CAR_H

enum
{
    RATE_DRIVER_GROUPS_MAX = 4,             //!< Total number of rate groups
    RATE_DRIVER_GROUPS_HANDLERS_MAX = 16,    //!< Number of handlers per rate group

    I2C_DEVICE_NUMBER = 0,      //!< See i2cmaster.h
    I2C_FAST_MODE = 1,          //!< 400 kHz

    I2C_MUX_A2_A1_A0 = 0x0,     //!< Pin connection to I2C mux

    // Camera pin configuration
    CAM_0_CS = 0x0,             //!< SPI chip select for Camera 0
    CAM_0_MUX = 0,              //!< Camera 0 on I2C mux 0
    CAM_1_CS = 0x1,             //!< SPI chip select for Camera 1
    CAM_1_MUX = 1,              //!< Camera 1 on I2C mux 1

    // SPI configuration for ArduCam
    SPI_SPEED = 4000000,        //!< 4Mhz, half of the maximum speed of the camera
    SPI_POL = 0,
    SPI_PHA = 1,
};

#endif //VO_CMPE460_VO_CAR_H
