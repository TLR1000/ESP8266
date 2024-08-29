// I2C Scanner
/*
The Wire.begin(); function initializes the I2C communication using the default SDA and SCL pins for your microcontroller. On the Wemos D1 mini (ESP8266), these are typically:
SDA (D2, GPIO4)
SCL (D1, GPIO5)
The for loop iterates through possible I2C addresses, from 8 to 119/0x08 to 0x77. These are the standard 7-bit I2C address range for most devices.
The address is printed in both decimal and hexadecimal formats.

*/

#include <Wire.h>

void setup() {
  Serial.begin (9600);
  while (!Serial) 
    {
    }

  Serial.println ();
  Serial.println ("Scanning for addresses ...");
  byte count = 0;
  
  Wire.begin();
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);  
      } 
  } 
  Serial.println ("Done scanning.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
}  

void loop() {
  //niks
  }
