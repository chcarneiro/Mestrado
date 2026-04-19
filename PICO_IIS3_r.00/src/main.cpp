#include <Arduino.h>
#include "iis3dwb_driver.hpp"
#include "iis3dwb_regs.hpp"

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println("BOOT,IIS3DWB,Pico RP2040");

  if (!sensor.begin()) {
    Serial.println("ERROR,WHO_AM_I");
    while (true) {
      delay(1000);
    }
  }

  Serial.print("WHO_AM_I=0x");
  Serial.println(sensor.readRegister(REG_WHO_AM_I), HEX);

  Serial.print("CTRL1_XL=0x");
  Serial.println(sensor.readRegister(REG_CTRL1_XL), HEX);

  Serial.print("CTRL3_C=0x");
  Serial.println(sensor.readRegister(REG_CTRL3_C), HEX);

  Serial.print("CTRL6_C=0x");
  Serial.println(sensor.readRegister(REG_CTRL6_C), HEX);

  Serial.println("t_ms,x,y,z");
}

void loop() {
  int16_t x, y, z;

  sensor.readXYZraw(x, y, z);

  Serial.print(millis());
  Serial.print(',');
  Serial.print(x);
  Serial.print(',');
  Serial.print(y);
  Serial.print(',');
  Serial.println(z);

  delay(50);
}