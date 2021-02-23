//kippendeurNodemcu
/*
 * Versie in productie: V5.3.2
 * 
 * te fixen:
 * -pubsub same topic bug
 * -hourOpen en hourClose setten dmv. MQTT callback
 * -realtime luik bediening
 * -fuse: max tijd voor openen of sluiten luik 
 * 
 * Versions
 * V5.3.3 Timer logica en berekeningen aangepast
 * V5.3.2 Password e.a. include toegevoegd
 * V5.3.1 Bugfixes: MQTTSwitchLuik naming consistency
 * V5.3 MQTT uitgebreid met paramateriseren open/dicht en MQTT open/dicht
 * V5.2 MQTT toegevoegd
 * V5.1 RTC vervangen door NTP
 * V5   OTA toegevoegd (en dus ook WiFi)
 * V4   deurmotor, 2 reedswitches, RTC, Mode-schakelaar
 */
// Wiring:
// A0 Naar spanningsdeler circuit voor 3 standen schakelaar 
// D0 GPIO16 --> relais Mains       is  Node MCU LED_BUILTIN 
// D1 GPIO5  --> SCL                DS3231 
// D2 GPIO4  --> SDA                DS3231
// D3 GPIO0  --> 
// D4 GPIO2                         is ESP-12 Led
// D5 GPIO14 --> ReedPin1           Boven GRIJS  -- WIT   -- BLAUW  Feed is -rail BRUIN - ZWART
// D6 GPIO12 --> ReedPin2           Onder VIOLET -- GRIJS -- ROOD   Feed is -rail BRUIN - ZWART
// D7 GPIO13 --> relais 1
// D8 GPIO15 --> relais 2           heeft externe pullup nodig indien gebruikt moet low zijn voor boot
// RX GPIO3
// TX GPIO1 

//Voor passwords
#include "config.h"
//Voor Wifi
#include <ESP8266WiFi.h>
//voor OTA
#include <ArduinoOTA.h>
//Voor NTP
#include <EasyNTPClient.h>
#include <WiFiUdp.h>
//Voor interne Klok
#include <TimeLib.h>
//voor MQTT
#include <PubSubClient.h>

//reed schakelaar
const int reedBoven = D5;
const int reedOnder = D6;

//Relais
const int relais1 = D7;
const int relais2 = D8;
const int relaisM = D0;

//voor NTP
WiFiUDP udp;
EasyNTPClient ntpClient(udp, "pool.ntp.org", ((1*60*60))); // 2 = GMT + 2 zomertijdcorrectie, 1 = GMT + 1 

//voor MQTT
WiFiClient espClient;
PubSubClient client(espClient);
String luik;  //global met string die stand vh luik aangeeft
//globals tbv. conversie voor MQTT publish
int MQTTSwitchLuik = 2; //initaliseren naar automatisch
//int MQTThourOpen;
//int MQTThourClose;
String MQTTSwitchLuik_str;  
String MQTThourOpen_str;
String MQTThourClose_str;
char MQTTSwitchLuikCharArray[10];
char MQTThourOpenCharArray[10];
char MQTThourCloseCharArray[10];

//voor de stand van de schakelaar
String modus; 

//declaratie openings- en sluitingstijd
int hourOpen;
int hourClose;
int minuteOpen;
int minuteClose;

void setup() {
  //times in GMT+1
  hourOpen = 8;
  minuteOpen = 30;
  hourClose = 22;
  minuteClose = 00;
  
  //debug
  Serial.begin(57600);
  while (!Serial) ; // wait for Arduino Serial Monitor

  //setup wifi
  WiFi.begin(ssid, passphrase);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //klok initialiseren
  setClock();
  
  //reedcontacten
  pinMode(reedBoven, INPUT_PULLUP);
  pinMode(reedOnder, INPUT_PULLUP);  
  //relais
  pinMode(relais1, OUTPUT);
  pinMode(relais2, OUTPUT);
  pinMode(relaisM, OUTPUT);
  //relais uitzetten
  relais("uit");

  //MQTT
  client.setServer(mqtt_server, 16103);
  client.setCallback(callback);
  
  //OTA
  ArduinoOTA.setHostname(myhostname);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");  

}

void callback(char* topic, byte* payload, unsigned int length) {   
  //debug input
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //process input
  if (strcmp(topic,"kippenhok/MQTTSwitchLuik")==0){
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    MQTTSwitchLuik = atoi((char *)payload);
  }  
  if (strcmp(topic,"kippenhok/MQTThourOpen")==0){
    //copy message into variable
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    hourOpen = atoi((char *)payload);
  }
  if (strcmp(topic,"kippenhok/MQTThourClose")==0){
    //copy message into variable
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    hourClose = atoi((char *)payload); 
  }
  Serial.println("Callback done.");
}

void loop() {
  Serial.println("Begin loop()");

  //klokstatus naar MQTT publishen
  String tijdStringMsg;
  if (hour() >= 0 && hour() < 10) {
    tijdStringMsg.concat("0");
  }
  tijdStringMsg.concat(hour());
  tijdStringMsg.concat(":");
  if (minute() >= 0 && minute() < 10) {
    tijdStringMsg.concat("0");
  }  
  tijdStringMsg.concat(minute());
  tijdStringMsg.concat(":");
  if (second() >= 0 && second() < 10) {
    tijdStringMsg.concat("0");
  }  
  tijdStringMsg.concat(second());
  Serial.println("tijdStringMsg: "+tijdStringMsg);

  //deur
  luik = leesSensors();
    
  //Voor MQTT moeten we een actieve client hebben
  if (!client.connected()) {
    reconnect();
  }
  delay(100);
  client.publish("kippenhok/klok", (char*) tijdStringMsg.c_str());
  delay(100); //wait for data to be published.
  client.publish("kippenhok/luik", (char*) luik.c_str());
  delay(100); //wait for data to be published.

  MQTTSwitchLuik_str = String(MQTTSwitchLuik); //converting int variable to a string 
  MQTTSwitchLuik_str.toCharArray(MQTTSwitchLuikCharArray, MQTTSwitchLuik_str.length() + 1); //packaging up the data to publish to mqtt whoa...
  client.publish("kippenhok/MQTTSwitchLuik", MQTTSwitchLuikCharArray);  
  delay(100); //wait for data to be published.
  
  MQTThourOpen_str = String(hourOpen); //converting int variable to a string 
  MQTThourOpen_str.toCharArray(MQTThourOpenCharArray, MQTThourOpen_str.length() + 1); //packaging up the data to publish to mqtt whoa...  
  client.publish("kippenhok/MQTThourOpen", MQTThourOpenCharArray);
  delay(100); //wait for data to be published.

  MQTThourClose_str = String(hourClose); //converting int variable to a string 
  MQTThourClose_str.toCharArray(MQTThourCloseCharArray, MQTThourClose_str.length() + 1); //packaging up the data to publish to mqtt whoa... 
  client.publish("kippenhok/MQTThourClose", MQTThourCloseCharArray);  
  delay(100); //wait for data to be published.
  
  //welke mode runnen we?
  modus = leesSchakelaar();
  //spanningsdeler van de switch uitlezen
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  Serial.print("Modus: "); Serial.println(modus);
  if (modus == "auto"){
    autoBediening();
  } else if (modus == "omhoog"){
    deurOpenDoen();
  } else if (modus == "omlaag"){
    deurDichtDoen();
  }   
  //Voor MQTT moeten we een actieve client hebben
  if (!client.connected()) {
    reconnect();
  }
  client.publish("kippenhok/mode", (char*) modus.c_str());
  delay(100); //wait for data to be published.
  client.loop();
  
  //OTA
  ArduinoOTA.handle();

  //zorgen dat we callbacks zien
  delay(100);
  client.loop();
  
  delay(500);
  Serial.println("Einde loop()");
}

void autoBediening() {
  Serial.print("Now is: ");Serial.print(day());Serial.print("-");Serial.print(month());Serial.print(" ");Serial.print(hour());Serial.print(":");Serial.print(minute());Serial.print(" GMT+1 weekday:");Serial.println(weekday());
  
  //recalc for the weekend
  if (weekday() == 1 || weekday() == 7) {
      hourOpen = hourOpen + 1;
      Serial.print("weekend correctie: hourOpen is" + hourOpen);Serial.println(" GMT+1");
    }

  //recalc voor zomertijd (+1)
  //om het simpel te houden: uurtje optellen bij open en close tijden
  //2021: 28 maart - 31 oktober
  //2022: 27 maart - 30 oktober
  //2023: 26 maart - 29 oktober
  if (((month() > 3||month() == 3) && day() >= 28) && ((month() < 10|| month() == 10) && day() < 31)){
    hourOpen = hourOpen + 1;
    Serial.println ("zomertijd correctie: hourOpen is " + hourOpen);Serial.println(" GMT+1");
    }
  
  //open or close?
  if( (((hour() * 100) + minute()) >= ((hourOpen * 100) + minuteOpen)) && (((hour() * 100) + minute()) < ((hourClose * 100) + minuteClose)) ){
    //open     
    Serial.println("autoBediening: deurOpenDoen()");
    deurOpenDoen();
  } else {
    //dicht
    Serial.println("autoBediening: deurDichtDoen()");
    deurDichtDoen();
    }
}

String leesSchakelaar(){
  Serial.print("leesSchakelaar functie: ");
  //eerst kijken of de schakelaar is overruled door de instelling van de MQTTSwitches 
  //MQTTSwitchLuik waarde 0 = luik omhoog, waarde 1 = luik omlaag
  if (MQTTSwitchLuik > 1){
    Serial.println("MQTTSchakelaar: >1 dus fysieke schakelaar geldt"); 
  }  else if (MQTTSwitchLuik = 0){
    Serial.println("MQTTSchakelaar: MQTT omhoog");
    return "omhoog";  
  }  else if (MQTTSwitchLuik = 1){
    Serial.println("Schakelaar: MQTT omlaag");
    return "omlaag";
  }
  //Geen override door MQTT dus doorgaan:
  //spanningsdeler van de switch uitlezen
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  Serial.print("voltage: "); Serial.print(voltage); Serial.print(" ");
  if (voltage < 1){
    Serial.println("Schakelaar: automatisch");
    return "auto";
  } else if (voltage > 1 && voltage < 4 ){
    Serial.println("Schakelaar: handmatig omhoog");
    return "omhoog";
  } else if (voltage > 4 ){
    Serial.println("Schakelaar: handmatig omlaag");
    return "omlaag";
  }   
}

String leesSensors(){
  Serial.print("leesSensors functie: ");
  //reed switches
  // 0 close - 1 open switch
  // dus: boven = 0 --> deur is open/omhoog
  //      beneden = 0 --> deur is gesloten/omlaag
  int boven;
  int onder;
  boven = digitalRead(reedBoven);
  onder = digitalRead(reedOnder);
  if (boven == 0){
     Serial.println("Deur is open");
     return "open";
  } else if (onder == 0){
     Serial.println("Deur is gesloten");
     return "gesloten";
  } else {
     Serial.println("Deur is onbekend");
     return "onbekend";
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void deurDichtDoen() {
  String oldModus = modus;
  String newModus;
  String stand;
  stand = leesSensors();
  Serial.print("deurDicht functie: ");
  Serial.print("sensors:");
  Serial.println(stand);
  if (stand != "gesloten"){
    
    relais("sluiten");
    while (stand != "gesloten"){    
      //zorgen dat we MQTT callbacks blijven ontvangen.
      if (!client.connected()) {
          reconnect();
      }   
      client.loop();
      
      delay(100); //0,1 seconde
      stand = leesSensors();
      Serial.print("loopen tot gesloten: ");
      Serial.print("sensors:");
      Serial.println(stand);
      newModus = leesSchakelaar();
      if (newModus != oldModus){
        break;      
      }      
      }
    relais("uit");
  } else {
    //niks 
    }
}    

void deurOpenDoen() {
  String oldModus = modus;
  String newModus;  
  Serial.print("deurOpen functie: ");
  String stand;
  stand = leesSensors();
  Serial.print("sensors:");
  Serial.println(stand);    
  if (stand != "open"){
    relais("openen");
    while (stand != "open"){
      delay(100); //0,1 seconde      
      Serial.print("loopen tot open: ");
      stand = leesSensors();
      Serial.print("sensors:");
      Serial.println(stand);   
      newModus = leesSchakelaar();
      if (newModus != oldModus){
        break;      
      } 
      }
    relais("uit");
  } else {
    //niks 
    }
} 

void relais(String functie){
  if (functie == "openen"){
    Serial.println("relais openen");
    digitalWrite(relaisM, LOW);
    delay(1500);
    digitalWrite(relais1, HIGH); 
    digitalWrite(relais2, LOW);       
    }
  if (functie == "sluiten"){
    Serial.println("relais sluiten");
    digitalWrite(relaisM, LOW);
    delay(1500);
    digitalWrite(relais1, LOW); 
    digitalWrite(relais2, HIGH);       
    }
  if (functie == "uit"){
    Serial.println("relais uit");
    digitalWrite(relais1, LOW); 
    digitalWrite(relais2, LOW);  
    digitalWrite(relaisM, HIGH);
    delay(500);     
    }
}

void setClock(){
  Serial.println("setClock()");
  int testSync = 1;
  while (testSync > 0) {
    Serial.println("setClock: syncClock() not ok - needs sync");
    testSync = syncClock();
  } 
  Serial.println("setClock: syncClock() ok - synced");      
}

int syncClock(){ 
  /*
 * Zomertijd 2019 (Nederland). 
 * Van: 02:00 zondag 31 maart
 * Tot: 03:00 zondag 27 oktober
 * Nog iets mee doen....
 */
  Serial.println("syncClock()");
  unsigned long prevTime = now();
  unsigned long UnixTime = ntpClient.getUnixTime();
  Serial.println("prevTime : "+prevTime);
  Serial.println("UnixTIme : "+UnixTime);
  if (UnixTime < 1) { //client geeft soms 0 terug
    Serial.println("UnixTIme invalid: "+UnixTime);
    return 1;
  }  
  if (UnixTime > prevTime) {{
    //klok instellen
    setTime(UnixTime);
  }
    Serial.println("Clock bijgesteld");
  } else {
    Serial.println("Clock niet bijgesteld");
  }  
  return 0;
}

boolean reconnect() {
  Serial.println("reconnect()");

  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client",mqtt_user,mqtt_password)) {  
      Serial.println("Connected to MQTT server");
      // Once connected, publish an announcement...
      client.publish("outTopic","reconnected");
      delay(100); //wait for data to be published.
      // ... and resubscribe
      client.subscribe("inTopic");
      delay(100); //wait for data to be published.
      client.subscribe("kippenhok/MQTTSwitchLuik");
      delay(100); //wait for data to be published.
      client.subscribe("kippenhok/MQTThourOpen");
      delay(100); //wait for data to be published.
      client.subscribe("kippenhok/MQTThourClose");
      delay(100); //wait for data to be published.
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);  
    }
  return client.connected();
  }
}
