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
#include <Firebase_ESP_Client.h> //firebase library https://github.com/mobizt/Firebase-ESP-Client

#include "secret.h" //passwords and things we don't want public can be kept in this file since it doesn't upload to github (in gitignore) The file should be kept somewhat secure.
const byte LED_BUILTIN = 2; //esp32s have a blue light on pin 2, can be nice for status and debugging

boolean wifiAvailable = false;
boolean timeAvailable = false;
boolean firebaseAvailable = false;

const int8_t UTC_offset = -5;//EST
Dusk2Dawn sunTime(41.82, -71.40, UTC_offset);
struct tm timeClock; //used for keeping track of what time it is (in EST not toggling daylight savings)
time_t timestampEpoch; //internet time

//live data
float liveGenW = 0.0;
float liveUseW = 0.0;
float liveBatPer = 0.0;
boolean available = false;
int cumulativeWhGen = 1;

float dayUseWh = 0.0;
float dayGenWh = 0.0;

unsigned long lastLiveUpdateMillis = 0;
long lastLiveUpdateMillisInterval = 30000;

unsigned long lastDayDataUpdateMillis = 0;
long lastDayDataUpdateMillisInterval = 30000;

unsigned long lastMonthDataUpdateMillis = 0;
long lastMonthDataUpdateMillisInterval = 10000;

//CircularBuffer<float, 50> dataBuffer1; //declare a buffer that can hold 50 floats

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
  firebaseAvailable = setupFirebase();

  digitalWrite(LED_BUILTIN, LOW);  //one time setup finished
}

void loop() {
  //main loop code! runs continuously
  wifiAvailable = checkWifiConnection();
  timeAvailable = updateTimeClock();

  runLiveUpdate();
  runDayDataUpdate();
  runMonthDataUpdate();

  vTaskDelay(20);
}



void runLiveUpdate() {
  if (millis() - lastLiveUpdateMillis > lastLiveUpdateMillisInterval) {
    lastLiveUpdateMillis = millis();
    firebaseSendLiveData();
  }
}
void runDayDataUpdate() {

  if (millis() - lastDayDataUpdateMillis > lastDayDataUpdateMillisInterval) {
    lastDayDataUpdateMillis = millis();
    firebaseSendDayData();
    firebaseDeleteOldData("/data/dayData/", 24 * 60 * 60, 2);

    liveGenW = random(0, 300);
    liveUseW = random(5, 500);
    liveBatPer = random(50, 100);
  }
}

void runMonthDataUpdate() {

  if (millis() - lastMonthDataUpdateMillis > lastMonthDataUpdateMillisInterval) {
    lastMonthDataUpdateMillis = millis();
    firebaseSendMonthData();
    firebaseDeleteOldData("/data/monthData/", 31 * 24 * 60 * 60, 2);

    dayGenWh = random(1400, 2000);
    dayUseWh = random(500, 3000);

  }
}
