#include <Arduino.h>
#include <LoRa.h>
#include <WiFi.h>
#include <time.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

//pin configuration
#define ss 5
#define rst 14
#define dio0 2

// function declaration
void setupLoRa();
void setupWiFi();
void printLocalTime();
void setupTime();
void setupServer();
void listAllFiles();
void formatSPIFFS();
void setupSPIFFS();
String processor(const String&);

//Object declaration
AsyncWebServer server(80);
StaticJsonDocument<200> node_address;
StaticJsonDocument<200> readings;
  StaticJsonDocument<200> doc;


// global Variables
const char* ssid = "OnePlus";
const char* pwd = "qwerty.1234";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
uint8_t rssi=0;
float snr=0.0;
uint8_t ewaste=0, dry=0, wet=0;
void setup(){
  pinMode(36, OUTPUT);
  digitalWrite(36, HIGH);
  Serial.begin(115200);
  // setupLoRa();
  setupSPIFFS();
  setupWiFi();
  setupTime();
  setupServer();
}

void loop(){
  int packetSize = LoRa.parsePacket();
  if(packetSize){
    // doc = NULL;
    Serial.print(F("Received packet: '"));
    String LoRaData;
    while (LoRa.available())
    {
      LoRaData = LoRa.readString();
    }
    rssi = LoRa.packetRssi();
    snr = LoRa.packetSnr();
    Serial.printf("Received packet with RSSI:%d SNR:%.2f\n", rssi,snr);
    DeserializationError error = deserializeJson(doc, LoRaData);
    if (error){
      Serial.println(error.c_str());
    }
    const char* id = doc["id"];
    dry = doc["bin"][0];
    wet = doc["bin"][1];
    ewaste = doc["bin"][2];
    Serial.printf("\tid:%s dry:%d wet:%d ewaste:%d\n",id, dry, wet, ewaste);
    
  }
}

void setupWiFi(){
  WiFi.enableSTA(true);
  WiFi.disconnect(true);
  delay(1000);
  unsigned long this_start = millis();
  WiFi.mode(WIFI_STA);
  Serial.printf("Connecting to %s\t", ssid);
  WiFi.begin(ssid, pwd);
  while(WiFi.status() != WL_CONNECTED && millis()-this_start < 20000){
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\nConnected"));
    Serial.print(F("IP address  :"));Serial.print(WiFi.localIP());
    Serial.print(F("\tMAC address :"));Serial.println(WiFi.softAPmacAddress());
  }else{
    WiFi.disconnect();
    Serial.println(F("WiFi disconnected"));
  }
}
void setupLoRa(){
  Serial.printf("Starting LoRa at CS=GPIO%d DIO0=GPIO%d RST=GPIO%d\n",ss, dio0, rst);
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(433E6))
  {
    Serial.println(F("Starting LoRa failed.!"));
    return;
  }
  Serial.println(F("LoRa started"));
  
}
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println(F("Failed to obtain time"));
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
void setupTime(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}
void setupServer(){
  server.begin();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/grid.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/grid.css", "text/css");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "text/js");
  });
  server.on("/nodes", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/json", String("[[\"pune\",8.5844986,73.7359567],[\"bangalore\",12.9410174,77.5633371]]"));
  });
  server.on("/read", HTTP_GET, [](AsyncWebServerRequest *request){
    char buff[100];
    sprintf(buff,"[[\"pune\",%d,%d,%d],[\"bangalore\",%d,%d,%d]]", (uint8_t)random(0,100), (uint8_t)random(0,100), (uint8_t)random(0,100), (uint8_t)random(0,100), (uint8_t)random(0,100), (uint8_t)random(0,100));
    request->send(200, "text/plain", String(buff));
  });
  // server.on("/dry", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send(200, "text/plain", String(dry));
  // });
  // server.on("/wet", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send(200, "text/plain", String(wet));
  // });
  
}
String processor(const String& var){
  return String();
}

void setupSPIFFS(){
  if(!SPIFFS.begin()){
    Serial.println(F("Error occurred while mounting SPIFFS!"));
    return;
  }
  Serial.println(F("SPIFFS mounted successfully"));
  listAllFiles();
}

void listAllFiles(){
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file){
    Serial.print(F("File: "));
    Serial.println(file.name());
    file = root.openNextFile();
  }
}
/* 

[["pune",47,68,21],["bangalore",40,75,16]]
[["pune",75,18,88],["bangalore",61,20,75]]
 */