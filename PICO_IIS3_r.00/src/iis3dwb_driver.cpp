#include <Arduino.h>
#include <SPI.h>
#include "pinout.hpp"
#include "iis3dwb_regs.hpp"
#include "iis3dwb_driver.hpp"

static constexpr uint32_t SPI_HZ = 500000;
static constexpr uint8_t ODR_DEBUG = 0x80;

IIS3DWBDriver sensor;

static void csLow() {
  digitalWrite(hw::HW_PIN_SPI_CS, LOW);
}

static void csHigh() {
  digitalWrite(hw::HW_PIN_SPI_CS, HIGH);
}

uint8_t IIS3DWBDriver::readRegister(uint8_t reg) {
  uint8_t v;
  csLow();
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE3));
  SPI.transfer(reg | 0x80);
  v = SPI.transfer(0x00);
  SPI.endTransaction();
  csHigh();
  return v;
}

void IIS3DWBDriver::writeRegister(uint8_t reg, uint8_t value) {
  csLow();
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE3));
  SPI.transfer(reg & 0x7F);
  SPI.transfer(value);
  SPI.endTransaction();
  csHigh();
}

void IIS3DWBDriver::readBurst(uint8_t startReg, uint8_t *dst, size_t len) {
  csLow();
  SPI.beginTransaction(SPISettings(SPI_HZ, MSBFIRST, SPI_MODE3));
  SPI.transfer(startReg | 0xC0);
  for (size_t i = 0; i < len; i++) {
    dst[i] = SPI.transfer(0x00);
  }
  SPI.endTransaction();
  csHigh();
}

bool IIS3DWBDriver::begin() {
  pinMode(hw::HW_PIN_SPI_CS, OUTPUT);
  csHigh();

  SPI.setRX(hw::HW_PIN_SPI_MISO);
  SPI.setCS(hw::HW_PIN_SPI_CS);
  SPI.setSCK(hw::HW_PIN_SPI_SCK);
  SPI.setTX(hw::HW_PIN_SPI_MOSI);
  SPI.begin();

  delay(20);

  if (readRegister(REG_WHO_AM_I) != WHO_AM_I_VALUE) {
    return false;
  }

  writeRegister(REG_CTRL1_XL, 0x00);
  delay(10);

  writeRegister(REG_CTRL3_C, BDU | IF_INC);
  delay(10);

  writeRegister(REG_CTRL6_C, 0x00);
  delay(10);

  writeRegister(REG_CTRL1_XL, ODR_DEBUG);
  delay(50);

  int16_t x, y, z;
  readXYZraw(x, y, z);
  return true;
}

bool IIS3DWBDriver::dataReady() {
  return (readRegister(REG_STATUS_REG) & STATUS_XLDA) != 0;
}

void IIS3DWBDriver::readXYZraw(int16_t &x, int16_t &y, int16_t &z) {
  uint8_t buf[6];
  readBurst(REG_OUTX_L_XL, buf, 6);

  x = (int16_t)((buf[1] << 8) | buf[0]);
  y = (int16_t)((buf[3] << 8) | buf[2]);
  z = (int16_t)((buf[5] << 8) | buf[4]);
}