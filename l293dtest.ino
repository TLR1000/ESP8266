//
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
//Library https://github.com/marcoschwartz/LiquidCrystal_I2C
//

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//L293D Interface test
//
// PIN bezetting:
// D8 - 13 - L293 Control pin 1
// D7 - 15 - L293 Control pin 2
//
// D5 - 14 - switch 1
// D6 - 12 - switch 2
//
const int motorPin1 =  13; // the number of the motor pin
const int motorPin2 =  15; // the number of the motor pin
const int switchPin1 =  14; // the number of the switch pin
const int switchPin2 =  12; // the number of the switch pin
int switch1State = 0; 
int switch2State = 0; 

void setup() {
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Init pinmodes");
  
  // initialize the motor pin as an output: 
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  //initialize switch pins as an output: 
  pinMode(switchPin1, INPUT);
  pinMode(switchPin2, INPUT);
  Serial.println("Init done");
  
  motorStop();
  motorPinStatus();
  Serial.println("setup() done");
}

void loop() {
  // put your main code here, to run repeatedly:

  switch1State = digitalRead(switchPin1);
  switch2State = digitalRead(switchPin2);
  Serial.print("Switches: ");
  Serial.print(switch1State);
  Serial.print(",");
  Serial.println(switch2State);

  if (switch1State = HIGH) {
    motorUp();
  } else {
    motorStop();
  }
  
  if (switch2State = HIGH) {
    motorDown();
  } else {
    motorStop();
  }

}

void motorPinStatus() {
  int x;
  Serial.println("Pinstatus:");
  Serial.print("motorPin1:");
  x=digitalRead(motorPin1);
  Serial.println(x);  
  Serial.print("motorPin2:");
  x=digitalRead(motorPin2);
  Serial.println(x);  
}

void motorUp() {
 
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH); 

  lcd.clear();
  lcd.print("Omhoog");
}

void motorDown() {
  lcd.clear();
 
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW); 

  lcd.clear();
  lcd.print("Omlaag");
}

void motorStop() {
  lcd.clear();
  lcd.print("stop");
  Serial.println("motorStop()");
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);

}

