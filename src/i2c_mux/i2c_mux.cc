#include <i2c_mux/i2c_mux.h>
#include <fw/fw.h>

#include <kernel/kernel.h>

static struct
{
    u8 mux_addr;
    u8 mask;

    // If the mux is not configured, bytes sent to
    // the mux device will not be sent and dropped
    // When not configured, this module will act exactly
    // like the I2C driver
    boolean configured;
} mux = {0};


void i2c_mux_init(u8 a2_a1_a0)
{
    FW_ASSERT(!(a2_a1_a0 & ~0x7) && "Invalid a2_a1_a0 mask", a2_a1_a0);
    mux.mux_addr = 0x70 | (a2_a1_a0 & 0x7);
    mux.mask = 0x0;
    mux.configured = TRUE;

    // Enable all channels by default
    i2c_mux_set(I2C_MUX_CH_ALL);
}

void i2c_mux_set(u8 i2c_channels)
{
    if (mux.configured && mux.mask != i2c_channels)
    {
        mux.mask = i2c_channels;
        kernel::i2c.Write(mux.mux_addr, &mux.mask, 1);
    }
}

void i2c_mux_write(u8 i2c_channels,
                   u8 addr,
                   const u8* data,
                   u32 n)
{
    FW_ASSERT(data);
    i2c_mux_set(i2c_channels);
    kernel::i2c.Write(addr, data, n);
}

void i2c_mux_read(u8 i2c_channels,
                  u8 addr,
                  u8* data,
                  u32 n)
{
    FW_ASSERT(data);
    FW_ASSERT(mux.mask == i2c_channels && "Expected i2c channels to already be selected: expected, got",
              i2c_channels, mux.mask);

    kernel::i2c.Read(addr, data, n);
}
