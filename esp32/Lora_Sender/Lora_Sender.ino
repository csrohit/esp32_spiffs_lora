//Sender

#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

const char* id = "pune";
int counter = 0;
void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(200);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
//  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  Serial.print("Sending packet: ");
  short cm1=random(50,99), cm2=random(10,50), cm3=random(10,80);
  
char jsonData[100];
//serializeJson(doc, jsonData);
sprintf(jsonData, "{'id':'%s','bin':[%d,%d,%d]}",id,cm1,cm2,cm3);
  Serial.println(jsonData);
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(jsonData);
  LoRa.endPacket();

  counter++;

  delay(5000);
}
