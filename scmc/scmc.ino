/**
   Solar Charger Monitoring Code
   This program monitors a solar power system and runs a physical and online dashboard.
   microcontroller: esp32
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
boolean firebaseStarted = false;

boolean muteAllAlerts = false;
boolean firebaseAvailableAlerted = true;

boolean firebaseAvailableHelper = true;
boolean firebaseRanSomething = false;

boolean otaEnable = true;

const int8_t UTC_offset = -5;//EST (not daylight time)
Dusk2Dawn sunTime(41.82, -71.40, UTC_offset);//latitude,longitude of Brown
struct tm timeClock; //used for keeping track of what time it is (in EST not toggling daylight savings)
time_t timestampEpoch; //internet time

unsigned long wifiCheckUpdateMillis = 0;

//live data
float liveGenW = 0.0;
float liveUseW = 0.0;
float liveBatPer = 0.0;
boolean available = false;
int cumulativeWhGen = 0;
float cumuWhGenHelper = 0.0;
//day cumulative
float dayUseWh = 0.0;
float dayGenWh = 0.0;
float dayHoursUsed = 0;

unsigned long lastLiveUpdateMillis = 0;
long lastLiveUpdateMillisInterval = 30000;

unsigned long lastDayDataUpdateMillis = 0;
long lastDayDataUpdateMillisInterval = 30000;

unsigned long lastMonthDataUpdateMillis = 0;
long lastMonthDataUpdateMillisInterval = 100000;

unsigned long lastCalcUpdateMillis = 0;
long lastCalcUpdateMillisInterval = 1000;

unsigned long lastLoadSettingsMillis = 0;
long lastLoadSettingsMillisInterval = 10000;

long wifiCheckIntervalMillis = 5000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);

  //set up pin modes and hardware connections.
  setSafe();//everything should be in the safest state until the code has fully started
  setupWifi();
  setupOTA();
  Serial.println("STARTING Loop!");
  digitalWrite(LED_BUILTIN, LOW);  //one time setup finished
}

void loop() {
  //main loop code! runs continuously
  wifiAvailable = checkWifiConnection();
  timeAvailable = updateTimeClock();

  runCalc();

  if (!firebaseStarted && wifiAvailable) {
    connectFirebase();
    firebaseStarted = true;
  }
  firebaseAvailableHelper = (timeAvailable && firebaseStarted && wifiAvailable);
  runLiveUpdate();
  runDayDataUpdate();
  runMonthDataUpdate();
  runSettingsDebugUpdate();
  if (firebaseRanSomething) {
    firebaseAvailable = firebaseAvailableHelper;
  }

  if (firebaseAvailable) {
    firebaseAvailableAlerted = false;
  } else {
    if (!firebaseAvailableAlerted) {
      if (!muteAllAlerts) {
        sendEmail("[ALERT] solar charging station error", "Firebase not available");
      }
      Serial.println("[ALERT]: Firebase not available");
      firebaseAvailableAlerted = true;
    }
  }

  runOTA();
  vTaskDelay(20);
}

void setSafe() {
  //put everything in the safest possible state (when booting, in detected error state, or reprogramming)
  Serial.println("setting safe");
}

void runCalc() {
  if (millis() - lastCalcUpdateMillis > lastCalcUpdateMillisInterval) {
    cumuWhGenHelper += 1.0 * liveGenW * (millis() - lastCalcUpdateMillis) / (1000 * 60 * 60);
    cumulativeWhGen += long(cumuWhGenHelper);
    cumuWhGenHelper -= long(cumuWhGenHelper);
    lastCalcUpdateMillis = millis();
  }
}

void runLiveUpdate() {
  if (millis() - lastLiveUpdateMillis > lastLiveUpdateMillisInterval) {
    lastLiveUpdateMillis = millis();
    firebaseRanSomething = true;
    if (timeAvailable) { // don't want to give inaccurate timestamps
      if (!firebaseSendLiveData()) {
        firebaseAvailableHelper = false;
      }
    }
  }
}

void runSettingsDebugUpdate() {
  if (millis() - lastLoadSettingsMillis > lastLoadSettingsMillisInterval) {
    lastLoadSettingsMillis = millis();
    firebaseRanSomething = true;
    if (!firebaseGetSettings()) {
      firebaseAvailableHelper = false;
    }
    if (!firebaseRecvDebug()) {
      firebaseAvailableHelper = false;
    }
    if (!firebaseSendDebug()) {
      firebaseAvailableHelper = false;
    }
  }
}

void runDayDataUpdate() {

  if (millis() - lastDayDataUpdateMillis > lastDayDataUpdateMillisInterval) {
    lastDayDataUpdateMillis = millis();
    firebaseRanSomething = true;
    if (timeAvailable) {
      if (!firebaseSendDayData()) {
        firebaseAvailableHelper = false;
      }
      if (!firebaseDeleteOldData("/data/dayData/", 24 * 60 * 60, 2)) {
        firebaseAvailableHelper = false;
      }
    }

    liveGenW = 10000;
    liveUseW = random(5, 500);
    liveBatPer = random(50, 100);
  }
}

void runMonthDataUpdate() {

  if (millis() - lastMonthDataUpdateMillis > lastMonthDataUpdateMillisInterval) {
    lastMonthDataUpdateMillis = millis();
    firebaseRanSomething = true;
    if (timeAvailable) {
      if (!firebaseSendMonthData()) {
        firebaseAvailableHelper = false;
      }
      if (!firebaseDeleteOldData("/data/monthData/", 31 * 24 * 60 * 60, 2)) {
        firebaseAvailableHelper = false;
      }
    }
    //after testing, these variables should actually be reset to 0 for the next day
    dayGenWh = random(1400, 2000);
    dayUseWh = random(500, 3000);
    dayHoursUsed = random(0, 60) / 10.0;

  }
}

void rebootESP32() {
  setSafe();
  Serial.flush();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  ESP.restart();
}
