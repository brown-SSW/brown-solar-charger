/**
   Solar Charger Monitoring Code
   This program monitors a solar power system and runs a physical and online dashboard.
   Compatible with esp32 dev board with 38 pins
   charge controller: Renogy ROVER ELITE 40A MPPT

   Made by members of Brown University club "Scientists for a Sustainable World" (s4sw@brown.edu) 2021 https://github.com/brown-SSW/brown-solar-charger
*/

#include <WiFi.h> //used for sending stuff over the wifi radio within the esp32
#include <CircularBuffer.h> //used for having fixed size buffers that discard the oldest data https://github.com/rlogiacco/CircularBuffer
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <ESP_Mail_Client.h> //library for sending email https://github.com/mobizt/ESP-Mail-Client
#include <Dusk2Dawn.h> //sunrise sunset lookup https://github.com/dmkishi/Dusk2Dawn remove "static" from library as needed to fix compiling error

#include "secret.h" //passwords and things we don't want public can be kept in this file since it doesn't upload to github (in gitignore) The file should be kept somewhat secure.
const byte LED_BUILTIN = 2; //esp32s have a blue light on pin 2, can be nice for status and debugging

boolean wifiAvailable = false;
boolean timeAvailable = false;
boolean mqttAvailable = false;

const int8_t UTC_offset = -5;//EST
Dusk2Dawn sunTime(41.82, -71.40, UTC_offset);
struct tm timeClock; //used for keeping track of what time it is (in EST not toggling daylight savings)

IPAddress MQTTserver(10, 0, 0, 129);
int MQTTport = 1883;

WiFiClient wifiClient;
PubSubClient MQTTclient(wifiClient);

CircularBuffer<float, 50> dataBuffer1; //declare a buffer that can hold 50 floats


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);

  //set up pin modes and hardware connections.
  //everything should be in the safest state until the code has fully started
  wifiAvailable = setupWifi();
  if (!wifiAvailable) {
    Serial.println("WARNING: WIFI NOT CONNECTING");
  }

  setupMQTT();

  digitalWrite(LED_BUILTIN, LOW);  //one time setup finished
}

void loop() {
  //main loop code! runs continuously
  wifiAvailable = checkWifiConnection();
  timeAvailable = updateTimeClock();
  mqttAvailable = checkMQTTConnection();

  vTaskDelay(20);
}
