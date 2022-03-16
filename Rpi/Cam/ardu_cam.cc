#include "ardu_cam.h"

enum ArduSPIRegister
{
    TEST = 0x00,                  //!< R/W Test register
    CCR = 0x01,                   //!< R/W Capture Control register
    TIMING = 0x03,                //!< R/W Sensor Interface Timing register
    FIFO = 0x04,                  //!< R/W FIFO Control register
    GPIO_DIR = 0x05,              //!< R/W GPIO Direction register
    GPIO_W = 0x06,                //!< R/W GPIO Write Register
    BURST_FIFO = 0x3C,            //!< RO Burst FIFO read operation
    SINGLE_FIFO = 0x3D,           //!< RO Single FIFO read operation
    CHIP_VERSION = 0x40,          //!< RO ArduChip version, constant value 0x40 for 2MP model
    STATUS = 0x41,                //!< Camera status flags
    FIFO_SIZE_0 = 0x42,           //!< Camera write FIFO size[7:0]
    FIFO_SIZE_1 = 0x43,           //!< Camera write FIFO size[15:8]
    FIFO_SIZE_2 = 0x44,           //!< Camera write FIFO size[18:16]
    GPIO_R = 0x45,                //!< GPIO Read Register
};

enum
{
    ACAM_SPI_WRITE = 0x80,                //!< Write to a SPI register
    ACAM_CCR_NUM_FRAMES_MASK = 0x7,       //!< Number of frames to be captured
    ACAM_FIFO_CLR_DONE = 1 << 0,          //!< Clear FIFO write done flag
    ACAM_FIFO_START = 1 << 1,             //!< Start capture
    ACAM_FIFO_RST_WPTR = 1 << 2,          //!< Reset write pointer
    ACAM_FIFO_RST_RPTR = 1 << 3,          //!< Reset read pointer
};

ArduCam::ArduCam(U8 addr)
: m_addr(addr), m_fifo_size(0)
{
}

void ArduCam::init()
{
//    // Clear all internal camera fifos
//    w_spi_reg(FIFO,
//              ACAM_FIFO_RST_WPTR |
//              ACAM_FIFO_RST_RPTR |
//              ACAM_FIFO_CLR_DONE
//              );
//
//    // We only need a single image per burst for our purposes
//    w_spi_reg(CCR, 1);
}

void ArduCam::w_spi_reg(U8 addr, U8 data, U8* read)
{
    U8 r_buf[2];    // dummy buffer
    U8 w_buf[2] = {
            static_cast<U8>(addr | ACAM_SPI_WRITE),
            data
    };

    read_write_sync_spi(w_buf, read ? read : r_buf, 2);
}

U8 ArduCam::r_spi_reg(U8 addr)
{
    U8 w_buf[2] = {static_cast<U8>(addr & 0x7f), 0x0};
    U8 r_buf[2];

    read_write_sync_spi(w_buf, r_buf, 2);
    return r_buf[1];
}

void ArduCam::capture_image()
{
    // Trigger the camera to capture an image
    w_spi_reg(FIFO, ACAM_FIFO_START);
}

void ArduCam::update_fifo_length()
{
    U32 len1, len2, len3;
    len1 = r_spi_reg(FIFO_SIZE_0);
    len2 = r_spi_reg(FIFO_SIZE_1);
    len3 = r_spi_reg(FIFO_SIZE_2) & 0x7f;
    m_fifo_size = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
}
