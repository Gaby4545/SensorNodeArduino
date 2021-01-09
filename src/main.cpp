#include <WiFi.h>

#include <WebServer.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <WiFiUdp.h>
WiFiUDP Udp;

#include <EEPROM.h>

//EEPROM_Rotate EEPROM;

#define EEPROM_SIZE 100

// SHT3x
#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();

#include <Wire.h> // I2C
#include <SPI.h> 

/* SSID & Password for Acces Point Mode*/
const char* ssid_ap = "SensorNode";  // Enter SSID here
const char* password_ap = "";  // Enter Password here

const char ssid[] = "";
const char pass[] = "";

/* Put IP Address details */
// Server
IPAddress ip(192, 168, 0, 167); // Server Adress

// Local Server AccesPoint
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);

uint32_t uniqueId = 12;

struct retCred {String ssid, pass;};
retCred getCredentials() {
  String ssid = ""; 
  String pass = ""; 

  int end = 0; 
  for (int addr = 0; end < 2; addr++) {
    char c = (char)(EEPROM.read(addr));

    Serial.println("Char : " + c);

    if (c = (char)0xff) {end++;} else {
      if (end == 0) {
        ssid += c;
      }
      if (end == 1) {
        pass += c;
      }
    }
  }

  return retCred {ssid, pass};
}

/// Utilitary functions
/** Converts a float to an array of bytes */
void float2Bytes(byte bytes_temp[4], float float_variable){ 
  memcpy(bytes_temp, (unsigned char*) (&float_variable), 4);
}

/** Converts a uint to an array of bytes */
void uint2Bytes(byte bytes_temp[4], uint32_t variable){ 
  memcpy(bytes_temp, (unsigned char*) (&variable), 4);
}

/// EEPROM saving functions
void saveCredentials(String ssid, String pass) {
  String data = ssid + (char)0xff + pass + (char)0xff;

  Serial.println("Saving : " + data);

  for (int addr = 0; addr < data.length(); addr++) {

    Serial.print("Saving : ");
    Serial.print(data[addr]);
    Serial.print("@");
    Serial.println(addr);

    EEPROM.write(addr, data[addr]);
  }
  EEPROM.commit();

  retCred cred = getCredentials();
  Serial.println("Cred : " + cred.ssid + cred.pass);
}

/// Internal webserver functions
String SendHTML() {
  String ptr = "<form action='/'  method='GET'><label for='ssid'>SSID:</label><br><input type='text' id='ssid' name='ssid' value=''><br><label for='pass'>Password:</label><br><input type='password' id='pass' name='pass' value=''><br><br><input type='submit' value='Submit'></form>"; 
  return ptr;
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void createWiFiAP() {
  WiFi.softAP(ssid_ap, password_ap);
  delay(1000);
  WiFi.softAPConfig(local_ip, gateway, subnet);


  // MDNS to allow connecting with : http://sensornode.local/
  // TODO : MDNS is not working
  MDNS.begin("SensorNode");
}

void getRequest(AsyncWebServerRequest *request) {
  Serial.println("Get request");

  if (request->hasParam("ssid") && request->hasParam("pass")) {

    // Saving to EEPROM
    // TODO : EEPROM is not working
    saveCredentials(request->getParam("ssid")->value(), request->getParam("pass")->value());
  }

  request->send(200, "text/html", SendHTML());
}

void setupWepServer() {
  server.on("/", HTTP_GET, getRequest);
  server.onNotFound(notFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

/// Data transfer functions
bool connectToWiFi(const char* ssid, const char* pass) {
  Serial.print("Connecting to "); Serial.println(ssid);

  //Serial.print(WL_NO_SSID_AVAIL); //1
  //Serial.print(WL_NO_SHIELD); //2
  //Serial.print(WL_DISCONNECTED); //6
  
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print(' ');
    WiFi.begin(ssid, pass);
    
    Serial.print(WiFi.status());
    delay(1000);
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Connection Failed");
    }
 
    if ((++i % 32) == 0) {
      Serial.println();
    }
    
    Serial.print(F("Connected. My IP address is: "));
    Serial.println(WiFi.localIP());
  }
}

bool connectToWiFiFromSaved() {
  // TODO
}

/// Sensors functions
void sht3xSetup() {
  if (!sht31.begin(0x45)) {   // Set to 0x45 for alternate i2c addr // TODO : config for this
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
}

void readAndSendValues() {
  // ShtSensor
  byte shtTemp[4]; float2Bytes(shtTemp, sht31.readTemperature());
  byte shtHum[4]; float2Bytes(shtHum, sht31.readHumidity());

  byte id[4]; uint2Bytes(id, uniqueId);

  Udp.beginPacket(ip, 27401);
  Udp.write(id, 4);
  Udp.write(shtTemp, 4);
  Udp.write(shtHum, 4);

  Udp.endPacket();
}

/// Base Arduino functions
/** Point of entry */
void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  // TODO
  //createWiFiAP();
  //setupWepServer();

  //retCred cred = getCredentials();
  //Serial.println("Cred : " + cred.ssid + cred.pass);

  //connectToWiFiFromSaved();

  connectToWiFi(ssid, pass);
  sht3xSetup();
  Udp.begin(27401);
}

/** Is executed after Setup indefinitly */
void loop() {
  readAndSendValues();
  delay(10000); // TODO : Find non blockingway to do this
}

