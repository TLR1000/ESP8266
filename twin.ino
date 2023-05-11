//twin.ino
//https://create.arduino.cc/projecthub/LithiumION/mpu6050-gyroscope-with-arduino-64b931
//https://create.arduino.cc/projecthub/eEdizon/arduino-self-balancing-robot-482cd7?ref=similar&ref_id=459912&offset=5
//https://create.arduino.cc/projecthub/hammadiqbal12/controlling-of-servo-motor-with-arduino-and-mpu6050-6375b9?ref=similar&ref_id=459912&offset=1
//
//Standaard MPU address A0 to ground is 0b1101000 = 104
//alternatief address A0 to Vcc is 0b1101001 = 105
  
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <MPU6050_light.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);


MPU6050 mpu(Wire);
MPU6050 mpu1(Wire);
unsigned long timer = 0;


void setup() {
  Serial.begin(115200);                           // Ensure serial monitor set to this value also    
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))  // Address 0x3C for most of these displays, if doesn't work try 0x3D 
  { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);                                      // Don't proceed, loop forever
  } 
  display.setTextSize(1);             
  display.setTextColor(SSD1306_WHITE);            // Draw white text
  display.clearDisplay();     

  Wire.begin();
  mpu.setAddress(0x68);
  mpu.begin();
  mpu1.setAddress(0x69);
  mpu1.begin();

  display.println(F("Calculating gyro offset, do not move MPU6050"));
  display.display();        
  mpu.calcGyroOffsets();                          // This does the calibration
  mpu1.calcGyroOffsets();                          // This does the calibration
  display.setTextSize(1);

}

void loop() {
  mpu.update();  
  mpu1.update();  
  if((millis()-timer)>10)                         // print data every 10ms
  {                                           
    int angleX = mpu.getAngleX() + 0.5;
    int angleX1 = mpu1.getAngleX() + 0.5;
    int angleY = mpu.getAngleY() + 0.5;
    int angleY1 = mpu1.getAngleY() + 0.5;
    int angleZ = mpu.getAngleZ() + 0.5;
    int angleZ1 = mpu1.getAngleZ() + 0.5;
    display.clearDisplay();                       // clear screen
    display.setCursor(0,0);                  
    display.println("MPU Status : Running");
    display.print("MPU temperature:");
    display.println(mpu.getTemp());
    display.print("P/X: "); display.print(angleX); display.print(" "); display.println(angleX1);
    display.print("R/Y: "); display.print(angleY); display.print(" "); display.println(angleY1);
    display.print("Y/Z: "); display.print(angleZ); display.print(" "); display.println(angleZ1);
    display.display();                            // display data
    timer = millis();  
  }
}
