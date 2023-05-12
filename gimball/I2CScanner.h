#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include <Wire.h>

void scanI2CAddresses();

void scanI2CAddresses() {
  byte error, address;
  int devicesCount = 0;

  Serial.println("Scanning I2C addresses...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);

      // Identify devices based on default addresses
      if (address == 0x3C) {
        Serial.println(" (OLED Display)");
      } else if (address == 0x68) {
        Serial.println(" (Gyro)");
      } else {
        Serial.println();
      }

      devicesCount++;
    }
  }

  if (devicesCount == 0) {
    Serial.println("No I2C devices found.");
  }
}

#endif
