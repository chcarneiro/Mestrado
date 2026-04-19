#pragma once
#include <Arduino.h>
#include "iis3dwb_regs.hpp"

class IIS3DWBDriver
{
public:
    bool begin();

    // Verifica se há nova amostra disponível no STATUS_REG (bit XLDA)
    inline bool dataReady()
    {
        return (readRegister(REG_STATUS_REG) & STATUS_XLDA) != 0;
    }

    // Leitura burst dos 3 eixos em contagens raw (int16_t)
    // Sensibilidade: 0,061 mg/LSB para FS = ±2 g
    inline void readXYZraw(int16_t &x, int16_t &y, int16_t &z)
    {
        uint8_t buf[6] = {0};
        readBurst(REG_OUTX_L_XL, buf, 6);
        x = (int16_t)((buf[1] << 8) | buf[0]);
        y = (int16_t)((buf[3] << 8) | buf[2]);
        z = (int16_t)((buf[5] << 8) | buf[4]);
    }

    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);
    void readBurst(uint8_t startReg, uint8_t *dst, size_t len);

private:
    uint8_t _spiMode = 3;

    uint8_t readRegisterMode(uint8_t reg, uint8_t spiMode);
    void writeRegisterMode(uint8_t reg, uint8_t value, uint8_t spiMode);
    void readBurstMode(uint8_t startReg, uint8_t *dst, size_t len, uint8_t spiMode);
};

extern IIS3DWBDriver sensor;