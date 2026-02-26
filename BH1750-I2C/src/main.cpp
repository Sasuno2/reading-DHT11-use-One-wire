#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

void setup() {
    Serial.begin(115200);

    // Khởi tạo I2C (SDA, SCL)
    Wire.begin(21, 22);

    // Khởi tạo BH1750
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23)) {
        Serial.println("BH1750 ready!");
    } else {
        Serial.println("BH1750 init failed!");
    }
}

void loop() {
    float lux = lightMeter.readLightLevel();

    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");

    delay(1000);
}