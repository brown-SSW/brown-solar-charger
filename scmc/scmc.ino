/**
   Solar Charger Monitoring Code
   This program monitors a solar power system and runs a physical and online dashboard.
   microcontroller: esp32
   charge controller: Renogy ROVER ELITE 40A MPPT

  Using library WiFi at version 1.0 in folder: C:\Users\Joshua\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\WiFi
  Using library ESP_Mail_Client at version 1.6.4 in folder: C:\Users\Joshua\Documents\Arduino\libraries\ESP_Mail_Client
  Using library SD at version 1.0.5 in folder: C:\Users\Joshua\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\SD
  Using library FS at version 1.0 in folder: C:\Users\Joshua\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\FS
  Using library SPI at version 1.0 in folder: C:\Users\Joshua\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\SPI
  Using library SPIFFS at version 1.0 in folder: C:\Users\Joshua\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\SPIFFS
  Using library Dusk2Dawn at version 1.0.1 in folder: C:\Users\Joshua\Documents\Arduino\libraries\Dusk2Dawn
  Using library Firebase_Arduino_Client_Library_for_ESP8266_and_ESP32 at version 2.6.7 in folder: C:\Users\Joshua\Documents\Arduino\libraries\Firebase_Arduino_Client_Library_for_ESP8266_and_ESP32
  Using library WiFiClientSecure at version 1.0 in folder: C:\Users\Joshua\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\WiFiClientSecure
  Using library FastLED at version 3.5.0 in folder: C:\Users\Joshua\Documents\Arduino\libraries\FastLED
  Using library Wire at version 1.0.1 in folder: C:\Users\Joshua\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\Wire
  Using library Adafruit_GFX_Library at version 1.10.12 in folder: C:\Users\Joshua\Documents\Arduino\libraries\Adafruit_GFX_Library
  Using library Adafruit_LED_Backpack_Library at version 1.3.2 in folder: C:\Users\Joshua\Documents\Arduino\libraries\Adafruit_LED_Backpack_Library
  Using library Adafruit_BusIO at version 1.10.1 in folder: C:\Users\Joshua\Documents\Arduino\libraries\Adafruit_BusIO
  Using library ESPmDNS at version 1.0 in folder: C:\Users\Joshua\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\ESPmDNS
  Using library ArduinoOTA at version 1.0 in folder: C:\Users\Joshua\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\ArduinoOTA
  Using library Update at version 1.0 in folder: C:\Users\Joshua\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\libraries\Update


   Made by members of Brown University club "Scientists for a Sustainable World" (s4sw@brown.edu) 2021-  
   https://github.com/brown-SSW/brown-solar-charger
*/
#include <Arduino.h>
#include <WiFi.h> //used for sending stuff over the wifi radio within the esp32
#include <ESP_Mail_Client.h> //library for sending email https://github.com/mobizt/ESP-Mail-Client
#include <Dusk2Dawn.h> //sunrise sunset lookup https://github.com/dmkishi/Dusk2Dawn DOWNLOAD ZIP OF MASTER AND INSTALL MANUALLY SINCE THE RELEASE (1.0.1) IS OUTDATED AND DOESN'T COMPILE!
#include <Firebase_ESP_Client.h> //firebase library https://github.com/mobizt/Firebase-ESP-Client

#include "secret.h" //passwords and things we don't want public can be kept in this file since it doesn't upload to github (in gitignore) The file should be kept somewhat secure.
const byte LED_BUILTIN = 2; //esp32s have a blue light on pin 2, can be nice for status and debugging

boolean wifiAvailable = false;
boolean timeAvailable = false;
boolean firebaseAvailable = false;
boolean firebaseStarted = false;

boolean firebaseAvailSettings = true;
boolean firebaseAvailSendDebug = true;
boolean firebaseAvailRecvDebug = true;
boolean firebaseAvailSendLive = true;
boolean firebaseAvailDeleteDay = true;
boolean firebaseAvailDay = true;
boolean firebaseAvailDeleteMonth = true;
boolean firebaseAvailMonth = true;


boolean muteAllAlerts = true;
boolean firebaseAvailableAlerted = true;

boolean firebaseRanSomething = false;

boolean otaEnable = true;

const int8_t UTC_offset = -5;//EST (not daylight time)
Dusk2Dawn sunTime(41.82, -71.40, UTC_offset);//latitude,longitude of Brown
struct tm timeClock; //used for keeping track of what time it is (in EST not toggling daylight savings)
time_t timestampEpoch; //internet time
boolean timeConfiged = false;

unsigned long wifiCheckUpdateMillis = 0;

//settings (from database)
int BatteryDischargeFloor = 50;
int LowBatteryThreshold = 20;
int MidBatteryThreshold = 40;

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
long liveDataUpdateMillisInterval = 30000;

unsigned long lastDayDataUpdateMillis = 0;
long dayDataUpdateMillisInterval = 30000;

unsigned long lastMonthDataUpdateMillis = 0;
long monthDataUpdateMillisInterval = 100000; //change to daily timer

unsigned long lastCalcUpdateMillis = 0;
long calcUpdateMillisInterval = 1000;

unsigned long lastLoadSettingsMillis = 0;
long loadSettingsMillisInterval = 60000;

long wifiCheckIntervalMillis = 5000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);

  //set up pin modes and hardware connections.
  setSafe();//everything should be in the safest state until the code has fully started
  setupWifi();
  setupOTA();
  setupDashboard();
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
  runLiveUpdate();
  runDayDataUpdate();
  runMonthDataUpdate();
  runSettingsDebugUpdate();
  if (firebaseRanSomething && firebaseStarted) {
    firebaseAvailable =
      firebaseAvailSettings &&
      firebaseAvailSendDebug &&
      firebaseAvailRecvDebug &&
      firebaseAvailSendLive &&
      firebaseAvailDeleteDay &&
      firebaseAvailDay &&
      firebaseAvailDeleteMonth &&
      firebaseAvailMonth;
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
  runDashboard();
  runOTA();
  vTaskDelay(20);
}

void setSafe() {
  //put everything in the safest possible state (when booting, in detected error state, or reprogramming)
  Serial.println("setting safe");
}

void runCalc() {
  if (millis() - lastCalcUpdateMillis > calcUpdateMillisInterval) {
    cumuWhGenHelper += 1.0 * liveGenW * (millis() - lastCalcUpdateMillis) / (1000 * 60 * 60);
    cumulativeWhGen += long(cumuWhGenHelper);
    cumuWhGenHelper -= long(cumuWhGenHelper);
    lastCalcUpdateMillis = millis();
  }
}

void runLiveUpdate() {
  if (millis() - lastLiveUpdateMillis > liveDataUpdateMillisInterval) {
    lastLiveUpdateMillis = millis();
    firebaseRanSomething = true;
    if (timeAvailable) { // don't want to give inaccurate timestamps
      firebaseAvailSendLive = firebaseSendLiveData();
    }
  }
}

void runSettingsDebugUpdate() {
  if (millis() - lastLoadSettingsMillis > loadSettingsMillisInterval) {
    lastLoadSettingsMillis = millis();
    firebaseRanSomething = true;
    firebaseAvailSettings = firebaseGetSettings();
    firebaseAvailRecvDebug = firebaseRecvDebug();
    firebaseAvailSendDebug = firebaseSendDebug();
  }
}

void runDayDataUpdate() {
  if (millis() - lastDayDataUpdateMillis > dayDataUpdateMillisInterval) {
    lastDayDataUpdateMillis = millis();
    firebaseRanSomething = true;
    if (timeAvailable) {
      firebaseAvailDay = firebaseSendDayData();
      firebaseAvailDeleteDay = firebaseDeleteOldData("/data/dayData/", 24 * 60 * 60, 2);
    }

    liveGenW = 10000;
    liveUseW = random(5, 500);
    liveBatPer = random(50, 100);
  }
}

void runMonthDataUpdate() {

  if (millis() - lastMonthDataUpdateMillis > monthDataUpdateMillisInterval) {
    lastMonthDataUpdateMillis = millis();
    firebaseRanSomething = true;
    if (timeAvailable) {
      firebaseAvailMonth = firebaseSendMonthData();
      firebaseAvailDeleteMonth = firebaseDeleteOldData("/data/monthData/", 31 * 24 * 60 * 60, 2);
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
