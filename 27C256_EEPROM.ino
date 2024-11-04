/*
Two-byte Addressing:

The 27C256 EEPROM requires a two-byte address because it has more than 256 bytes of memory. The address is split into two parts:

The MSB (Most Significant Byte) handles the higher bits of the address.
The LSB (Least Significant Byte) handles the lower bits of the address.

Use both the MSB (Most Significant Byte) and LSB (Least Significant Byte) of the address when writing to and reading from the EEPROM. This allows you to access the full 32 KB of memory in the 27C256 EEPROM.
Wire.write((int)(addr >> 8)); sends the higher byte of the address (MSB).
Wire.write((int)(addr & 0xFF)); sends the lower byte of the address (LSB).
*/

#include <Wire.h>     // for I2C
#define i2caddr 0x50    // device address for left-hand chip on our breadboard
byte d=0; // data to store in or read from the EEPROM

void setup()
{
  Serial.begin(9600); // Initialize the serial line
  Wire.begin();         // wake up the I2C
 
  Serial.println("Writing data...");
  for (int i=0; i<20; i++)
  {
    Serial.print(i);
    Serial.print(" : ");
    Serial.println(i);
    writeData(i,i);
  }
  Serial.println("DONE");
  Serial.println("Reading data...");
  for (int i=0; i<20; i++)
  {
    Serial.print(i);
    Serial.print(" : ");
    d=readData(i);
    Serial.println(d, DEC);
  }
  Serial.println("DONE");

}

void writeData(unsigned int addr, byte data) {
  Wire.beginTransmission(i2caddr);
  Wire.write((int)(addr >> 8));  // Write the MSB of the address for larger EEPROMs
  Wire.write((int)(addr & 0xFF));  // Write the LSB of the address
  Wire.write(data);
  byte result = Wire.endTransmission(1);
  if (result != 0) {
    Serial.print("ERROR writing to address ");
    Serial.println(addr);
  }
  delay(10);  // Allow time for EEPROM to complete the write
}

byte readData(unsigned int addr) {
  byte result;
  Wire.beginTransmission(i2caddr);
  Wire.write((int)(addr >> 8));  // Write the MSB of the address for larger EEPROMs
  Wire.write((int)(addr & 0xFF));  // Write the LSB of the address
  Wire.endTransmission(1);
  Wire.requestFrom(i2caddr, 1);
  if (Wire.available()) {
    result = Wire.read();
  } else {
    Serial.print("ERROR reading from address ");
    Serial.println(addr);
    result = 255;  // Return 255 if the read fails
  }
  return result;
}


void loop()
{
}
