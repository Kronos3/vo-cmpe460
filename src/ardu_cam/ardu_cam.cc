#include <fw/fw.h>

#include <ardu_cam/ardu_cam.h>
#include <kernel/kernel.h>

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

ArduCam::ArduCam(u8 addr, u8 mux_channels, u32 chip_select)
: m_addr(addr), m_mux(mux_channels), m_cs(chip_select),
m_fifo_size(0)
{
}

void ArduCam::init()
{
    // Clear all internal camera fifos
    w_spi_reg(FIFO,
              ACAM_FIFO_RST_WPTR |
              ACAM_FIFO_RST_RPTR |
              ACAM_FIFO_CLR_DONE
              );

    // We only need a single image per burst for our purposes
    w_spi_reg(CCR, 1);
}

void ArduCam::w_spi_reg(u8 addr, u8 data, u8* read) const
{
    u8 r_buf[2];    // dummy buffer
    u8 w_buf[2] = {
            static_cast<u8>(addr | ACAM_SPI_WRITE),
            data
    };

    kernel::spi.WriteReadSync(m_cs, w_buf, read ? read : r_buf, 2);
}

u8 ArduCam::r_spi_reg(u8 addr) const
{
    u8 w_buf[2] = {static_cast<u8>(addr & 0x7f), 0x0};
    u8 r_buf[2];

    kernel::spi.StartWriteRead(m_cs, &w_buf, &r_buf, 2);
    return r_buf[1];
}

void ArduCam::update_fifo_length()
{
    u32 len1, len2, len3;
    len1 = r_spi_reg(FIFO_SIZE_0);
    len2 = r_spi_reg(FIFO_SIZE_1);
    len3 = r_spi_reg(FIFO_SIZE_2) & 0x7f;
    m_fifo_size = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
}

void ArduCam::capture_image() const
{
    // Trigger the camera to capture an image
    w_spi_reg(FIFO, ACAM_FIFO_START);
}

void ArduCam::transfer_image(u8* image_buffer, void (* reply)(boolean, void*), void* param) const
{
    FW_ASSERT(image_buffer);
    FW_ASSERT(reply);

    // Command the SPI to burst the entire FIFO over SPI
    image_buffer[0] = BURST_FIFO;

    kernel::spi.SetCompletionRoutine(reply, param);

    // Queue the SPI data transfer with DMA
    kernel::spi.StartWriteRead(m_cs, image_buffer, image_buffer, m_fifo_size);
}

