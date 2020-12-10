/* Pin Name on the Board Function  Pin Number in Arduino IDE Alias Name in Arduino IDE
  D3  GPIO  0  0 D3
  TX  GPIO  1  1 D10
  D4  GPIO  2  2 D4
  RX  GPIO  3  3 D9
  D2  GPIO  4  4 D2  I2C
  D1  GPIO  5  5 D1  I2C
  D6  GPIO 12 12 D6
  D7  GPIO 13 13 D7
  D5  GPIO 14 14 D5 
  D8  GPIO 15 15 D8
  D0  GPIO 16 16 D0, LED_BUILTIN
  A0  ADC0 A0  analog_ip

  As built:
   display
   D1 -       I2C
   D2 -       I2C
   D3 -       Servo1
   Stappenmotor
   D5 - 14   Aa
   D6 - 12   Ab
   D7 - 13   Ba
   D8 - 15   Bb

*/

/*includes*/
//Servo
#include <Servo.h>
//Network
#include <ESP8266WiFi.h>
#include "networkcredentials.h"
//JSON
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include <ArduinoJson.h>

/*working storage objects*/
//Servo
  /*
  On standard servos a parameter value of 1000 is fully counter-clockwise, 2000 is fully clockwise, and 1500 is in the middle.
  Note that some manufactures do not follow this standard very closely so that servos often respond to values between 700 and 2300. 
  */
Servo azimuthServo; //midpoint 1550us, L 2500us ,R 550us
Servo elevationServo; //450 - 2500 :mid 1450
//credentials
const char* ssid     = mySSID;
const char* password = myPASSWORD;
//n2yo
const char* satID = "25544"; //Starlink-7 group
float pointerAzimuth = 0;
float pointerElevation = 0;

/*logic*/
void setup() {
  // put your setup code here, to run once:
    
  //attach servo
  azimuthServo.attach(D3);  // attaches the servo on pin D3 to the servo object
  elevationServo.attach(D5);  // attaches the servo on pin D3 to the servo object
  
  Serial.begin(9600);
  //testServos();

  //connect wifi
  //Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected: "); Serial.println(WiFi.localIP());
  
}

void setBothServos( float aziDegrees, float eleDegrees) {
  float usecondsA;
  float usecondsE;
  bool aziFlip;
  bool eleFlip;

  //correct for 360 deg
  if (aziDegrees > 180){
    aziDegrees = aziDegrees -180;
    aziFlip = true;
  } else{
    aziFlip = false;
  }
  
  Serial.print("aziFlip "); Serial.println(aziFlip);
  //azimuth
  usecondsA=((2500-450)/180*aziDegrees)+450;
  azimuthServo.writeMicroseconds(usecondsA);
  Serial.print("azimuth useconds ");Serial.println(usecondsA);
  delay(500);// niet tegelijk

  
  
  if (eleDegrees<180){
    eleDegrees = eleDegrees + 180;
    eleFlip = true;
  } else{
    eleFlip = false;
  }
  Serial.print("eleFlip ");Serial.println(eleFlip);
  //usecondsE=((2500-550)/180*eleDegrees)+550;
  usecondsE=((2500-450)/180*eleDegrees)+450;
  elevationServo.writeMicroseconds(usecondsE);    
  Serial.print("elevation useconds ");Serial.println(usecondsE);
}

void adjustPointer() {
 
  Serial.println(F("function adjustPointer():"));
  //Positie op dit moment opzoeken.

  //Send HTTP request
  WiFiClient client;
  client.setTimeout(10000);
  if (!client.connect("www.n2yo.com", 80)) {
    Serial.println(F("Connection failed"));
    return;
  }
  Serial.println(F("Connected!"));
  // Send HTTP request
  client.println(F("GET /rest/v1/satellite/positions/25544/51.833/4.133/0/1/&apiKey=NLNKL2-HDRWS8-249AXP-4GVA HTTP/1.0"));
  client.println(F("Host: n2yo.com"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  // It should be "HTTP/1.0 200 OK" or "HTTP/1.1 200 OK"
  if (strcmp(status + 9, "200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  // Allocate the JSON document
  // Use arduinojson.org/v6/assistant to compute the capacity.
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(9) + 170;
  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Extract values to vars
  Serial.println(F("Variables:"));
  pointerAzimuth = doc["positions"][0]["azimuth"]; // 253.71
  pointerElevation = doc["positions"][0]["elevation"]; // -52.2
  Serial.print(F("pointerAzimuth:")); Serial.println(pointerAzimuth);
  Serial.print(F("pointerElevation:")); Serial.println(pointerElevation);

  // Disconnect
  client.stop();

  /*
   * we hebben nu gegevens
  */

  //azimuth aanpassen
  //elevation aanpassen
  setBothServos(pointerAzimuth,pointerElevation);

  //even wachten ivm overload calls
  delay(500);
}

void loop() {
  adjustPointer();
  delay(30000);//30 seconden
  if (pointerElevation < 10) {
    delay(300000);//5 minuten
    } else {
      delay(30000);//30 seconden
      }
  }
