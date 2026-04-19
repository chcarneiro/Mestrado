#include <Arduino.h>


#ifndef LED_BUILTIN
#define LED_BUILTIN 25   // ajuste se necessário
#endif

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Pico 2W + PlatformIO OK");
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  Serial.println("Blink...");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
