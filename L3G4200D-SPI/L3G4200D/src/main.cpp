#include <Arduino.h>
#include <SPI.h>

#define CS_PIN 5

SPIClass *spi = NULL;

// ----- Thông số -----
#define SENSITIVITY 0.00875   // 250 dps mode
#define FILTER_ALPHA 0.9      // 0.8 → mượt vừa, 0.95 → rất mượt

float offsetX = 0, offsetY = 0, offsetZ = 0;
float x_filtered = 0, y_filtered = 0, z_filtered = 0;

// Ghi thanh ghi
void writeRegister(uint8_t reg, uint8_t value) {
    digitalWrite(CS_PIN, LOW);
    spi->transfer(reg & 0x7F);
    spi->transfer(value);
    digitalWrite(CS_PIN, HIGH);
}

// Đọc nhiều byte
void readRegisters(uint8_t reg, uint8_t *data, uint8_t len) {
    digitalWrite(CS_PIN, LOW);
    spi->transfer(reg | 0xC0);
    for (int i = 0; i < len; i++) {
        data[i] = spi->transfer(0x00);
    }
    digitalWrite(CS_PIN, HIGH);
}

// Đọc gyro raw
void readGyro(int16_t &x, int16_t &y, int16_t &z) {
    uint8_t rawData[6];
    readRegisters(0x28, rawData, 6);

    x = (int16_t)(rawData[1] << 8 | rawData[0]);
    y = (int16_t)(rawData[3] << 8 | rawData[2]);
    z = (int16_t)(rawData[5] << 8 | rawData[4]);
}

// Tính offset khi để yên
void calibrateOffset() {
    Serial.println("Calibrating... Keep sensor still!");
    delay(2000);

    long sumX = 0, sumY = 0, sumZ = 0;
    int samples = 1000;

    for (int i = 0; i < samples; i++) {
        int16_t x, y, z;
        readGyro(x, y, z);
        sumX += x;
        sumY += y;
        sumZ += z;
        delay(2);
    }

    offsetX = sumX / (float)samples;
    offsetY = sumY / (float)samples;
    offsetZ = sumZ / (float)samples;

    Serial.println("Calibration done!");
}

void setup() {
    Serial.begin(115200);

    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    spi = new SPIClass(VSPI);
    spi->begin(18, 19, 23, CS_PIN);

    delay(100);

    // Bật gyro
    writeRegister(0x20, 0x0F);

    calibrateOffset();
}

void loop() {
    int16_t x_raw, y_raw, z_raw;
    readGyro(x_raw, y_raw, z_raw);

    // Trừ offset
    float x = (x_raw - offsetX) * SENSITIVITY;
    float y = (y_raw - offsetY) * SENSITIVITY;
    float z = (z_raw - offsetZ) * SENSITIVITY;

    // Low-pass filter
    x_filtered = FILTER_ALPHA * x_filtered + (1 - FILTER_ALPHA) * x;
    y_filtered = FILTER_ALPHA * y_filtered + (1 - FILTER_ALPHA) * y;
    z_filtered = FILTER_ALPHA * z_filtered + (1 - FILTER_ALPHA) * z;

    Serial.print("X: "); Serial.print(x_filtered);
    Serial.print("  Y: "); Serial.print(y_filtered);
    Serial.print("  Z: "); Serial.println(z_filtered);

    delay(10);
}