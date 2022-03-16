#include "ardu_cam.h"
#include "ov2640_regs.h"
#include <Os/Task.hpp>

OV2640::OV2640()
: ArduCam(0x30)
{
}

void OV2640::init()
{
    ArduCam::init();

    // Initialize the OV2640
    // This camera only works with the JPEG imaging format
    ws_8_8(0xff, 0x01);
    ws_8_8(0x12, 0x80);

    Os::Task::delay(100);

    ws_8_8(OV2640_JPEG_INIT);
    ws_8_8(OV2640_YUV422);
    ws_8_8(OV2640_JPEG);
    ws_8_8(0xff, 0x01);
    ws_8_8(0x15, 0x00);

    set_jpeg_size(Rpi::CameraJpegSize::P_320x240);

    update_fifo_length();
}

void OV2640::set_jpeg_size(const Rpi::CameraJpegSize& size)
{
    switch (size.e)
    {
        case Rpi::CameraJpegSize::P_160x120:
            ws_8_8(OV2640_160x120_JPEG);
            break;
        case Rpi::CameraJpegSize::P_176x144:
            ws_8_8(OV2640_176x144_JPEG);
            break;
        case Rpi::CameraJpegSize::P_320x240:
            ws_8_8(OV2640_320x240_JPEG);
            break;
        case Rpi::CameraJpegSize::P_352x288:
            ws_8_8(OV2640_352x288_JPEG);
            break;
        case Rpi::CameraJpegSize::P_640x480:
            ws_8_8(OV2640_640x480_JPEG);
            break;
        case Rpi::CameraJpegSize::P_800x600:
            ws_8_8(OV2640_800x600_JPEG);
            break;
        case Rpi::CameraJpegSize::P_1024x768:
            ws_8_8(OV2640_1024x768_JPEG);
            break;
        case Rpi::CameraJpegSize::P_1280x1024:
            ws_8_8(OV2640_1280x1024_JPEG);
            break;
        case Rpi::CameraJpegSize::P_1600x1200:
            ws_8_8(OV2640_1600x1200_JPEG);
            break;
        default:
            FW_ASSERT(0, size.e);
    }

    update_fifo_length();
}

void OV2640::set_special_effects(const Rpi::CameraSpecialEffect& effect)
{
    switch (effect.e)
    {
        case Rpi::CameraSpecialEffect::ANTIQUE:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x18);
            ws_8_8(0x7c, 0x05);
            ws_8_8(0x7d, 0x40);
            ws_8_8(0x7d, 0xa6);
            break;
        case Rpi::CameraSpecialEffect::BLUEISH:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x18);
            ws_8_8(0x7c, 0x05);
            ws_8_8(0x7d, 0xa0);
            ws_8_8(0x7d, 0x40);
            break;
        case Rpi::CameraSpecialEffect::GREENISH:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x18);
            ws_8_8(0x7c, 0x05);
            ws_8_8(0x7d, 0x40);
            ws_8_8(0x7d, 0x40);
            break;
        case Rpi::CameraSpecialEffect::REDDISH:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x18);
            ws_8_8(0x7c, 0x05);
            ws_8_8(0x7d, 0x40);
            ws_8_8(0x7d, 0xc0);
            break;
        case Rpi::CameraSpecialEffect::BW:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x18);
            ws_8_8(0x7c, 0x05);
            ws_8_8(0x7d, 0x80);
            ws_8_8(0x7d, 0x80);
            break;
        case Rpi::CameraSpecialEffect::NEGATIVE:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x40);
            ws_8_8(0x7c, 0x05);
            ws_8_8(0x7d, 0x80);
            ws_8_8(0x7d, 0x80);
            break;
        case Rpi::CameraSpecialEffect::BW_NEGATIVE:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x58);
            ws_8_8(0x7c, 0x05);
            ws_8_8(0x7d, 0x80);
            ws_8_8(0x7d, 0x80);
            break;
        case Rpi::CameraSpecialEffect::NORMAL:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x00);
            ws_8_8(0x7c, 0x05);
            ws_8_8(0x7d, 0x80);
            ws_8_8(0x7d, 0x80);
            break;
    }

    update_fifo_length();
}

void OV2640::set_contrast(I8 contrast)
{
    switch (contrast)
    {
        case 2:

            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x04);
            ws_8_8(0x7c, 0x07);
            ws_8_8(0x7d, 0x20);
            ws_8_8(0x7d, 0x28);
            ws_8_8(0x7d, 0x0c);
            ws_8_8(0x7d, 0x06);
            break;
        case 1:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x04);
            ws_8_8(0x7c, 0x07);
            ws_8_8(0x7d, 0x20);
            ws_8_8(0x7d, 0x24);
            ws_8_8(0x7d, 0x16);
            ws_8_8(0x7d, 0x06);
            break;
        case 0:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x04);
            ws_8_8(0x7c, 0x07);
            ws_8_8(0x7d, 0x20);
            ws_8_8(0x7d, 0x20);
            ws_8_8(0x7d, 0x20);
            ws_8_8(0x7d, 0x06);
            break;
        case -1:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x04);
            ws_8_8(0x7c, 0x07);
            ws_8_8(0x7d, 0x20);
            ws_8_8(0x7d, 0x20);
            ws_8_8(0x7d, 0x2a);
            ws_8_8(0x7d, 0x06);
            break;
        case -2:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x04);
            ws_8_8(0x7c, 0x07);
            ws_8_8(0x7d, 0x20);
            ws_8_8(0x7d, 0x18);
            ws_8_8(0x7d, 0x34);
            ws_8_8(0x7d, 0x06);
            break;
        default:
            FW_ASSERT(0, contrast);
    }

    update_fifo_length();
}

void OV2640::set_brightness(I8 brightness)
{
    switch (brightness)
    {
        case 2:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x04);
            ws_8_8(0x7c, 0x09);
            ws_8_8(0x7d, 0x40);
            ws_8_8(0x7d, 0x00);
            break;
        case 1:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x04);
            ws_8_8(0x7c, 0x09);
            ws_8_8(0x7d, 0x30);
            ws_8_8(0x7d, 0x00);
            break;
        case 0:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x04);
            ws_8_8(0x7c, 0x09);
            ws_8_8(0x7d, 0x20);
            ws_8_8(0x7d, 0x00);
            break;
        case -1:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x04);
            ws_8_8(0x7c, 0x09);
            ws_8_8(0x7d, 0x10);
            ws_8_8(0x7d, 0x00);
            break;
        case -2:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x04);
            ws_8_8(0x7c, 0x09);
            ws_8_8(0x7d, 0x00);
            ws_8_8(0x7d, 0x00);
            break;
        default:
            FW_ASSERT(0, brightness);
    }

    update_fifo_length();
}

void OV2640::set_color_saturation(I8 color_saturation)
{
    switch (color_saturation)
    {
        case 2:

            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x02);
            ws_8_8(0x7c, 0x03);
            ws_8_8(0x7d, 0x68);
            ws_8_8(0x7d, 0x68);
            break;
        case 1:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x02);
            ws_8_8(0x7c, 0x03);
            ws_8_8(0x7d, 0x58);
            ws_8_8(0x7d, 0x58);
            break;
        case 0:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x02);
            ws_8_8(0x7c, 0x03);
            ws_8_8(0x7d, 0x48);
            ws_8_8(0x7d, 0x48);
            break;
        case -1:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x02);
            ws_8_8(0x7c, 0x03);
            ws_8_8(0x7d, 0x38);
            ws_8_8(0x7d, 0x38);
            break;
        case -2:
            ws_8_8(0xff, 0x00);
            ws_8_8(0x7c, 0x00);
            ws_8_8(0x7d, 0x02);
            ws_8_8(0x7c, 0x03);
            ws_8_8(0x7d, 0x28);
            ws_8_8(0x7d, 0x28);
            break;
        default:
            FW_ASSERT(0, color_saturation);
    }

    update_fifo_length();
}

void OV2640::set_light_mode(const Rpi::CameraLightMode& light_mode)
{
    switch (light_mode.e)
    {
        case Rpi::CameraLightMode::AUTO:
            ws_8_8(0xff, 0x00);
            ws_8_8(0xc7, 0x00); //AWB on
            break;
        case Rpi::CameraLightMode::SUNNY:
            ws_8_8(0xff, 0x00);
            ws_8_8(0xc7, 0x40); //AWB off
            ws_8_8(0xcc, 0x5e);
            ws_8_8(0xcd, 0x41);
            ws_8_8(0xce, 0x54);
            break;
        case Rpi::CameraLightMode::CLOUDY:
            ws_8_8(0xff, 0x00);
            ws_8_8(0xc7, 0x40); //AWB off
            ws_8_8(0xcc, 0x65);
            ws_8_8(0xcd, 0x41);
            ws_8_8(0xce, 0x4f);
            break;
        case Rpi::CameraLightMode::OFFICE:
            ws_8_8(0xff, 0x00);
            ws_8_8(0xc7, 0x40); //AWB off
            ws_8_8(0xcc, 0x52);
            ws_8_8(0xcd, 0x41);
            ws_8_8(0xce, 0x66);
            break;
        case Rpi::CameraLightMode::HOME:
            ws_8_8(0xff, 0x00);
            ws_8_8(0xc7, 0x40); //AWB off
            ws_8_8(0xcc, 0x42);
            ws_8_8(0xcd, 0x3f);
            ws_8_8(0xce, 0x71);
            break;
        default:
            FW_ASSERT(0, light_mode.e);
    }

    update_fifo_length();
}

