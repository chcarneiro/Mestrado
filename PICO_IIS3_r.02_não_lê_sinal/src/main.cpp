#include <Arduino.h>
#include <SPI.h>
#include "pinout.hpp"
#include "iis3dwb_driver.hpp"
#include "iis3dwb_regs.hpp"

extern IIS3DWBDriver sensor;

void setup()
{
  Serial.begin(115200);
  delay(3000);

  while (!Serial && millis() < 8000)
  {
    delay(10);
  }

  // Serial.println("A");
  Serial.println("BOOT,IIS3DWB,Pico RP2040");
  // Serial.println("B");

  if (!sensor.begin())
  {
    // Serial.println("C");
    Serial.println("ERROR,WHO_AM_I");
    while (true)
    {
      delay(1000);
    }
  }

  // Serial.println("D");
  Serial.print("WHO_AM_I=0x");
  Serial.println(sensor.readRegister(REG_WHO_AM_I), HEX);
  // Serial.println("E");
  // Serial.println("t_ms,x,y,z");

  Serial.println("Teleplot: t_ms_raw,x_raw,y_raw,z_raw"); // Msg. indicando que os dados são RAW em Teleplot
}

// Cabeçalho indicando que os dados são RAW
// Serial.println("t_ms,x_raw,y_raw,z_raw"); // plotado no teleplot, eliminar

void loop()
{
  int16_t x_raw, y_raw, z_raw;
  // sensor.readXYZraw(x, y, z);

  // Espera uma nova amostra
  if (!sensor.dataReady())
    return;

  sensor.readXYZraw(x_raw, y_raw, z_raw);
  uint32_t t_ms = millis();

  // --- Formato Teleplot: >nome:valor ---
  // Tempo em ms (curva 't_ms_raw')
  Serial.print(">t_ms_raw:");
  Serial.println(t_ms);

  // Eixo X raw
  Serial.print(">x_raw:");
  Serial.println(x_raw);

  // Eixo Y raw
  Serial.print(">y_raw:");
  Serial.println(y_raw);

  // Eixo Z raw
  Serial.print(">z_raw:");
  Serial.println(z_raw);

  // Saída CSV em modo RAW: tempo, x, y, z
  Serial.print(t_ms);
  Serial.print(',');
  Serial.print(x_raw);
  Serial.print(',');
  Serial.print(y_raw);
  Serial.print(',');
  Serial.println(z_raw);
  
delay(120);
}