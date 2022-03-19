#include "ardu_cam.h"

// Write 8 bit values to 8 bit register address
void ArduCam::ws_8_8(const SensorReg regs[])
{
    for(const SensorReg* iter = regs;
        (iter->reg != 0xff) | (iter->val != 0xff);
        iter++)
    {
        ws_8_8(iter->reg, iter->val);
    }
}

// Write 16 bit values to 8 bit register address
void ArduCam::ws_8_16(const SensorReg regs[])
{
    for(const SensorReg* iter = regs;
        (iter->reg != 0xff) | (iter->val != 0xffff);
        iter++)
    {
        ws_8_16(iter->reg, iter->val);
    }
}

// Write 8 bit values to 16 bit register address
void ArduCam::ws_16_8(const SensorReg regs[])
{
    for(const SensorReg* iter = regs;
        (iter->reg != 0xffff) | (iter->val != 0xff);
        iter++)
    {
        ws_16_8(iter->reg, iter->val);
    }
}

void ArduCam::ws_16_16(const SensorReg regs[])
{
    for(const SensorReg* iter = regs;
        (iter->reg != 0xffff) | (iter->val != 0xffff);
        iter++)
    {
        ws_16_16(iter->reg, iter->val);
    }
}

// Read/write 8 bit value to/from 16 bit register address
void ArduCam::ws_16_8(U16 regID, U8 regDat)
{
    U8 buf[3] = {0};
    buf[0] = (regID >> 8) & 0xff;
    buf[1] = (regID) & 0xff;
    buf[2] = regDat;
    write_i2c(m_addr, buf, 3);
}

// Read/write 8 bit value to/from 8 bit register address
void ArduCam::ws_8_8(U8 regID, U8 regDat)
{
    U8 buf[2] = {regID, regDat};
    write_i2c(m_addr, buf, 2);
}

void ArduCam::rs_8_8(U8 regID, U8* regDat)
{
    write_i2c(m_addr, &regID, 1);
    read_i2c(m_addr, regDat, 1);
}

void ArduCam::rs_16_8(U16 regID, U8* regDat)
{
    U8 buffer[2] = {0};
    buffer[0] = (regID >> 8) & 0xff;
    buffer[1] = regID & 0xff;
    write_i2c(m_addr, buffer, 2);
    read_i2c(m_addr, regDat, 1);
}

void ArduCam::ws_8_16(U8 regID, U16 regDat)
{
    U8 buf[3] = {0};
    buf[0] = regID;
    buf[1] = (regDat >> 8) & 0xff;
    buf[2] = (regDat) & 0xff;
    write_i2c(m_addr, buf, 3);
}

void ArduCam::rs_8_16(U8 regID, U16* regDat)
{
    write_i2c(m_addr, &regID, 1);
    read_i2c(m_addr, reinterpret_cast<U8*>(regDat), 2);
}

void ArduCam::ws_16_16(U16 regID, U16 regDat)
{
    U8 buf[4] = {0};
    buf[0] = (regID >> 8) & 0xff;
    buf[1] = regID & 0xff;
    buf[2] = (regDat >> 8) & 0xff;
    buf[3] = regDat & 0xff;
    write_i2c(m_addr, buf, 4);
}

void ArduCam::rs_16_16(U16 regID, U16* regDat)
{
    U8 buf[2] = {0};
    buf[0] = (regID >> 8) & 0xff;
    buf[1] = regID & 0xff;
    write_i2c(m_addr, buf, 2);
    read_i2c(m_addr, reinterpret_cast<U8*>(regDat), 2);
}

