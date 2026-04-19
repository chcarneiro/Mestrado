/*
bits 7:4 = 1010 → ODR = 26.7 kHz (ok).

bits 3:2 = 00 → ±2 g (não ±4 g).

bits 1:0 = 0 → filtros default.

*/

#include <Arduino.h>
#include <SPI.h>
#include "pinout.hpp"
#include "iis3dwb_regs.hpp"
#include "iis3dwb_driver.hpp"

// SPI a 10 MHz conforme recomendação da AN5444 para IIS3DWB
static constexpr uint32_t SPI_HZ = 10000000;
static constexpr uint8_t ODR_DEBUG = 0x80;                   //***

IIS3DWBDriver sensor;

static inline void csHigh()
{
    digitalWrite(hw::HW_PIN_SPI_CS, HIGH);
}

static inline void csLow()
{
    digitalWrite(hw::HW_PIN_SPI_CS, LOW);
}

static void spiPinsInit()
{
    pinMode(hw::HW_PIN_SPI_CS, OUTPUT);
    csHigh();

    SPI.setRX(hw::HW_PIN_SPI_MISO);
    SPI.setCS(hw::HW_PIN_SPI_CS);
    SPI.setSCK(hw::HW_PIN_SPI_SCK);
    SPI.setTX(hw::HW_PIN_SPI_MOSI);
    SPI.begin();
}

uint8_t IIS3DWBDriver::readRegisterMode(uint8_t reg, uint8_t spiMode)
{
    uint8_t v = 0xFF;

    csLow();
    delayMicroseconds(2);

    //SPI.beginTransaction(SPISettings(500000, MSBFIRST, spiMode));
    SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, spiMode));
    SPI.transfer(reg | 0x80);    // bit 7 = 1 para leitura
    v = SPI.transfer(0x00);
    SPI.endTransaction();

    delayMicroseconds(2);
    csHigh();
    delayMicroseconds(2);

    return v;
}

void IIS3DWBDriver::writeRegisterMode(uint8_t reg, uint8_t value, uint8_t spiMode)
{
    csLow();
    delayMicroseconds(2);

    SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, spiMode));
    //SPI.beginTransaction(SPISettings(500000, MSBFIRST, spiMode));
    SPI.transfer(reg & 0x7F);    // bit 7 = 0 para escrita
    SPI.transfer(value);
    SPI.endTransaction();

    delayMicroseconds(2);
    csHigh();
    delayMicroseconds(2);
}

void IIS3DWBDriver::readBurstMode(uint8_t startReg, uint8_t *dst, size_t len, uint8_t spiMode)
{
    csLow();
    delayMicroseconds(2);

    //SPI.beginTransaction(SPISettings(500000, MSBFIRST, spiMode));
    SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, spiMode));
    SPI.transfer(startReg | 0xC0);    // bit7=1 (read) + bit6=1 (auto-increment)
    for (size_t i = 0; i < len; i++)
    {
        dst[i] = SPI.transfer(0x00);
    }
    SPI.endTransaction();

    delayMicroseconds(2);
    csHigh();
    delayMicroseconds(2);
}

uint8_t IIS3DWBDriver::readRegister(uint8_t reg)
{
    return readRegisterMode(reg, _spiMode);
}

void IIS3DWBDriver::writeRegister(uint8_t reg, uint8_t value)
{
    writeRegisterMode(reg, value, _spiMode);
}

void IIS3DWBDriver::readBurst(uint8_t startReg, uint8_t *dst, size_t len)
{
    readBurstMode(startReg, dst, len, _spiMode);
}

static bool tryWhoAmI(uint8_t mode, uint8_t &valueOut)
{
    valueOut = 0xFF;

    for (int i = 0; i < 5; i++)
    {
        csHigh();
        delay(2);

        csLow();
        delay(2);
        csHigh();
        delay(2);

        //SPI.beginTransaction(SPISettings(500000, MSBFIRST, mode));
        SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, mode));
        csLow();
        delayMicroseconds(2);
        SPI.transfer(REG_WHO_AM_I | 0x80);
        valueOut = SPI.transfer(0x00);
        delayMicroseconds(2);
        csHigh();
        SPI.endTransaction();

        Serial.print("WHO_AM_I tentativa");
        Serial.print(i + 1);
        Serial.print(" modo SPI");
        Serial.print(mode == SPI_MODE3 ? "3" : "0");
        Serial.print(" = 0x");
        Serial.println(valueOut, HEX);

        if (valueOut == WHO_AM_I_VALUE)
        {
            return true;
        }

        delay(20);
    }

    return false;
}

bool IIS3DWBDriver::begin()
{
    _spiMode = SPI_MODE3;
    spiPinsInit();

    Serial.println("IIS3DWB.begin(): init SPI");
    Serial.println("CS idle HIGH, teste de comunicacao iniciado");

    uint8_t who3 = 0xFF;
    uint8_t who0 = 0xFF;

    bool ok3 = tryWhoAmI(SPI_MODE3, who3);
    bool ok0 = false;

    if (!ok3)
    {
        Serial.println("Falha em SPI_MODE3, tentando SPI_MODE0");
        ok0 = tryWhoAmI(SPI_MODE0, who0);
    }

    if (ok3)
    {
        _spiMode = SPI_MODE3;
        Serial.println("WHO_AM_I ok em SPI_MODE3");
    }
    else if (ok0)
    {
        _spiMode = SPI_MODE0;
        Serial.println("WHO_AM_I ok em SPI_MODE0");
    }
    else
    {
        Serial.print("Falha final WHO_AM_I. Ultimo SPI_MODE3=0x");
        Serial.print(who3, HEX);
        Serial.print(" | Ultimo SPI_MODE0=0x");
        Serial.println(who0, HEX);
        return false;
    }

    // Garante power-down antes de reconfigurar
    // writeRegister(REG_CTRL1_XL, 0x00);
    writeRegister(REG_CTRL1_XL, 0xA0);               // define ODR mais o Full Scale (±2 g)
    delay(10);

    // BDU + auto-incremento (IF_INC obrigatório para leitura burst)
    writeRegister(REG_CTRL3_C, BDU | IF_INC);
    delay(10);

    // Passo 1 (AN5444): Habilita interrupção data-ready no pino INT1
    // INT1_CTRL (0x0D): INT1_DRDY_XL = 1
    writeRegister(0x0D, 0x01);
    delay(5);

    // Passo 2 (AN5444): Seleciona modo 3 eixos
    // CTRL6_C (0x15): XL_AXIS_SEL = 00 (3 eixos)
    writeRegister(REG_CTRL6_C, 0x00);
    delay(5);

    // Passo 3 (AN5444): Habilita acelerômetro
    // CTRL1_XL (0x10): XL_EN=101 (26.667 kHz), FS=00 (±2 g), LPF2_XL=0 (apenas LPF1)
    // Sensibilidade: 0,061 mg/LSB — máxima resolução para detecção de falhas incipientes
    writeRegister(REG_CTRL1_XL, 0xA0);
    delay(20);

    Serial.print("CTRL1_XL=0x");
    Serial.println(readRegister(REG_CTRL1_XL), HEX);

    Serial.print("CTRL3_C=0x");
    Serial.println(readRegister(REG_CTRL3_C), HEX);

    Serial.print("CTRL6_C=0x");
    Serial.println(readRegister(REG_CTRL6_C), HEX);

    return true;
}

