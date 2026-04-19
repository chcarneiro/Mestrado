#include <Arduino.h>
#include <WiFi.h>

void setup() {
    WiFi.mode(WIFI_OFF);   // inicializa o chip Wi‑Fi
    Serial.println("WiFi OFF");
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH); // LED ON
    Serial.println("LED ON");
    delay(500);


    digitalWrite(LED_BUILTIN, LOW);  // LED OFF
    Serial.println("LED OFF");
    delay(500);
}


/*void setup() {
    
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);  // LED ON
    digitalWrite(LED_BUILTIN, LOW);   // LED OFF

    

    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("USB OK");
}

void loop() {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    Serial.println("LOOP OK");
    delay(1000);
}*/
