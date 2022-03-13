#ifndef CMPE460_ARDU_CAM_H
#define CMPE460_ARDU_CAM_H

#include <Assert.hpp>
#include "common.h"

#ifndef __cplusplus
#error "ArduCam needs C++ to be included"
#endif

class ArduCam
{
protected:

    /**
     * Create a new ArduCam given the hardware
     * selection made by the user
     * @param addr I2C address of the camera (set by which camera we are using)
     * @param mux_channels I2C multiplexer channel selection
     * @param chip_select SPI CS pin on the GPIO header (0-2)
     */
    explicit ArduCam(U8 addr);

    virtual void init();

    // Camera hardware configuration
    U8 m_addr;              //!< I2C slave address

    // Camera state
    U32 m_fifo_size;        //!< Image size in bytes

    /**
     * Write to a SPI register
     * @param addr register address
     * @param data data to write
     * @param read buffer to read into NULL for ignore
     */
//    void w_spi_reg(U8 addr, U8 data, U8* read = nullptr) const;

    /**
     * Read from a SPI register
     * @param addr address of register
     * @param read buffer to read into
     */
//    U8 r_spi_reg(U8 addr) const;

    /**
     * Update the internal FIFO length
     * Call this whenever camera settings are changes
     */
    void update_fifo_length();

    // Read/write 8 bit value to/from 8 bit register address
    void ws_8_8(const SensorReg* regs);
    void ws_8_8(U8 regID, U8 regDat);
    void rs_8_8(U8 regID, U8* regDat);

    // Read/write 16 bit value to/from 8 bit register address
    void ws_8_16(const SensorReg* regs);
    void ws_8_16(U8 regID, U16 regDat);
    void rs_8_16(U8 regID, U16* regDat);

    // Read/write 8 bit value to/from 16 bit register address
    void ws_16_8(const SensorReg* regs);
    void ws_16_8(U16 regID, U8 regDat);
    void rs_16_8(U16 regID, U8* regDat);

    // Read/write 16 bit value to/from 16 bit register address
    void ws_16_16(const SensorReg* regs);
    void ws_16_16(U16 regID, U16 regDat);
    void rs_16_16(U16 regID, U16* regDat);

    // Implemented by FPrime component

    virtual void read_i2c(U8 address, U8* dst, U32 len) = 0;
    virtual void write_i2c(U8 address, U8* src, U32 len) = 0;

public:
    /**
     * Trigger the camera to capture a single image
     * The data transfer will be done later
     * This way the cameras can be triggered simultaneously
     * and transferred serially along the SPI bus
     */
//    void capture_image() const;

    /**
     * Capture a single image
     * Note that this function will return immediately
     * @param image_buffer image buffer
     * @param reply reply from DMA that data has been transferred
     * @param param parameter to be sent to reply
     */
//    void transfer_image(U8* image_buffer, void (* reply)(bool, void*), void* param) const;
};

class OV2640 : public ArduCam
{
public:
    explicit OV2640();

    /**
     * Initialize the camera by setting up the
     * I2C, SPI and camera registers
     */
    void init() override;

    //Light Mode
    enum LightMode
    {
        AUTO,
        SUNNY,
        CLOUDY,
        OFFICE,
        HOME,
    };

    // Saturation, Brightness, Contrast
    enum Calibration
    {
        PLUS_2,
        PLUS_1,
        ZERO,
        MINUS_1,
        MINUS_2,
    };

    enum SpecialEffects
    {
        ANTIQUE,
        BLUEISH,
        GREENISH,
        REDDISH,
        BW,
        NEGATIVE,
        BW_NEGATIVE,
        NORMAL,
    };

    enum JpegSize
    {
        DIMENSION_160x120,
        DIMENSION_176x144,
        DIMENSION_320x240,
        DIMENSION_352x288,
        DIMENSION_640x480,
        DIMENSION_800x600,
        DIMENSION_1024x768,
        DIMENSION_1280x1024,
        DIMENSION_1600x1200,
    };

    void set_special_effects(SpecialEffects effect);
    void set_contrast(Calibration contrast);
    void set_brightness(Calibration brightness);
    void set_color_saturation(Calibration color_saturation);
    void set_light_mode(LightMode light_mode);
    void set_jpeg_size(JpegSize size);

private:
//    virtual void read_i2c(U8 address, U8* dst, U32 len) = 0;
//    virtual void write_i2c(U8 address, const U8* src, U32 len) = 0;
};

class OV5642 : public ArduCam
{
public:
    typedef enum
    {
        BMP,
        JPEG,
        RAW,
    } ImgType;

    enum LightMode
    {
        ADVANCED_AWB,
        SIMPLE_AWB,
        MANUAL_DAY,
        MANUAL_A,
        MANUAL_CWF,
        MANUAL_CLOUDY,
    };

    // Saturation, Brightness, Contrast
    enum Calibration
    {
        CAL_PLUS_4,
        CAL_PLUS_3,
        CAL_PLUS_2,
        CAL_PLUS_1,
        CAL_ZERO,
        CAL_MINUS_1,
        CAL_MINUS_2,
        CAL_MINUS_3,
        CAL_MINUS_4,
    };

    enum Hue
    {
        HUE_PLUS_180,
        HUE_PLUS_150,
        HUE_PLUS_120,
        HUE_PLUS_90,
        HUE_PLUS_60,
        HUE_PLUS_30,
        HUE_0,
        HUE_MINUS_30,
        HUE_MINUS_60,
        HUE_MINUS_90,
        HUE_MINUS_120,
        HUE_MINUS_150,
    };

    enum JpegSize
    {
        DIMENSION_320x240,
        DIMENSION_640x480,
        DIMENSION_1024x768,
        DIMENSION_1280x960,
        DIMENSION_1600x1200,
        DIMENSION_2048x1536,
        DIMENSION_2592x1944,
    };

    enum SpecialEffects
    {
//            ANTIQUE,
        BLUEISH,
        GREENISH,
        REDDISH,
        BW,
        NEGATIVE,
//            BW_NEGATIVE,
        NORMAL,
        SEPIA,
    };

    enum Sharpness
    {
        AUTO_SHARPNESS,
        AUTO_SHARPNESS_1,
        AUTO_SHARPNESS_2,
        MANUAL_SHARPNESS_OFF,
        MANUAL_SHARPNESS_1,
        MANUAL_SHARPNESS_2,
        MANUAL_SHARPNESS_3,
        MANUAL_SHARPNESS_4,
        MANUAL_SHARPNESS_5,
    };

    enum MirrorFlip
    {
        MIRROR,
        FLIP,
        MIRROR_FLIP,
        MIRROR_NORMAL,
    };

    enum Compress
    {
        HIGH_QUALITY,
        DEFAULT_QUALITY,
        LOW_QUALITY,
    };

    enum TestPattern
    {
        COLOR_BAR,
        COLOR_SQUARE,
        BW_SQUARE,
        DLI,
    };

    enum ExposureLevel
    {
        MINUS_17_EV,
        MINUS_13_EV,
        MINUS_10_EV,
        MINUS_07_EV,
        MINUS_03_EV,
        EXPOSURE_DEFAULT,
        PLUS_03_EV,
        PLUS_07_EV,
        PLUS_10_EV,
        PLUS_13_EV,
        PLUS_17_EV,
    };

    explicit OV5642(ImgType format = JPEG);
    void init() override;

    void set_special_effects(SpecialEffects effect);
    void set_contrast(Calibration contrast);
    void set_brightness(Calibration brightness);
    void set_color_saturation(Calibration color_saturation);
    void set_light_mode(LightMode light_mode);
    void set_hue(Hue hue);
    void set_jpeg_size(JpegSize size);
    void set_sharpness(Sharpness sharpness);
    void set_mirror_flip(MirrorFlip mirror_flip);
    void set_compress_quality(Compress quality);
    void set_exposure_level(ExposureLevel level);

    void test_pattern(TestPattern pattern);

private:
    /**
     * The OV5642 can operate with different image format
     */
    ImgType m_fmt;
};

#endif //CMPE460_ARDU_CAM_H
