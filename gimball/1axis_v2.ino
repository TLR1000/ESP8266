#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050_tockn.h>
#include <Servo.h>
#include "I2CScanner.h"

#define SERVO_MIN_PULSE 1000
#define SERVO_MAX_PULSE 2000

Adafruit_SSD1306 display(128, 64, &Wire, D4);
MPU6050 gyro(Wire);
Servo servo;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, true);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("One-Axis Gimbal");
  display.display();
  delay(1000);

  scanI2CAddresses();

  gyro.begin();
  gyro.calcGyroOffsets(true);

  calibrateGyroOffset();

  servo.attach(D3);
}

void loop() {
  gyro.update();

  float gyroOffset = 82.8;
  float gyroAngle = gyro.getAngleY() + gyroOffset;

  int pulseWidth = map(gyroAngle, -45, 45, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  int servoAngle = map(gyroAngle, -45, 45, 0, 180);

  servo.writeMicroseconds(pulseWidth);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Gyro Angle (Y): ");
  display.print(gyroAngle);
  display.setCursor(0, 20);
  display.print("Servo Pulse: ");
  display.print(pulseWidth);
  display.setCursor(0, 30);
  display.print("Servo Angle: ");
  display.print(servoAngle);
  display.display();

  Serial.print("Gyro Angle: ");
  Serial.println(gyroAngle);
  Serial.print("Servo Pulse: ");
  Serial.println(pulseWidth);

  delay(2);
}

void calibrateGyroOffset() {
  float sum = 0.0;
  int numReadings = 100;  // Adjust the number of readings as needed

  // Perform multiple readings and calculate the sum
  for (int i = 0; i < numReadings; i++) {
    gyro.update();
    sum += gyro.getAngleY();
    delay(10);  // Delay between readings, adjust as needed
  }

  // Calculate the average gyro offset
  float gyroOffset = sum / numReadings;

  Serial.print("Calibrated Gyro Offset: ");
  Serial.println(gyroOffset);

  // Apply the gyro offset as the adjusted offset value
  float adjustedGyroOffset = -gyroOffset;

  Serial.print("Adjusted Gyro Offset: ");
  Serial.println(adjustedGyroOffset);

  // Apply the adjusted gyro offset to the angle calculation
  float gyroAngle = gyro.getAngleY() + adjustedGyroOffset;

  Serial.print("Gyro Angle: ");
  Serial.println(gyroAngle);
}