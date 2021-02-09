/**
   Solar Charger Monitoring Code
   This program monitors a solar power system and runs a physical and online dashboard.
   Compatible with esp32 dev board with 38 pins
   charge controller: Renogy ROVER ELITE 40A MPPT

   Made by members of Brown University club "Scientists for a Sustainable World" (s4sw@brown.edu) 2021 https://github.com/brown-SSW/brown-solar-charger
*/

#include <WiFi.h> //used for sending stuff over the wifi radio within the esp32
//https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
#include <ESPAsyncWebServer.h> //library for being a web server
#include <HTTPClient.h> //used for making http GET requests to other servers
#include <ArduinoJson.h> //library for handling .json data https://arduinojson.org/v6/doc/installation/
#include <CircularBuffer.h> //used for having fixed size buffers that discard the oldest data https://github.com/rlogiacco/CircularBuffer
#include <SD.h>
#include <ESP_Mail_Client.h> //library for sending email https://github.com/mobizt/ESP-Mail-Client
#include <Dusk2Dawn.h> //sunrise sunset lookup https://github.com/dmkishi/Dusk2Dawn remove "static" from library as needed to fix compiling error

#include "secret.h" //passwords and things we don't want public can be kept in this file since it doesn't upload to github (in gitignore) The file should be kept somewhat secure.
const byte LED_BUILTIN = 2; //esp32s have a blue light on pin 2, can be nice for status and debugging

boolean wifiAvailable = false;
boolean timeAvailable = false;
boolean sdAvailable = false;

AsyncWebServer server(80); //port 80 is the standard for serving web pages, though Brown IT may want us to use something else
const int8_t UTC_offset = -5;//EST
Dusk2Dawn sunTime(41.82, -71.40, UTC_offset);

struct tm timeClock; //used for keeping track of what time it is (in EST not toggling daylight savings)

CircularBuffer<float, 50> dataBuffer1; //declare a buffer that can hold 50 floats

StaticJsonDocument<1024> dataDoc; //must be large enough to hold all the data served as a .json. https://arduinojson.org/v6/assistant/

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

  setupEmail();

  sdAvailable = MailClient.sdBegin(18, 19, 23, 5);//sck,miso,mosi,ss
  if (!sdAvailable) {
    Serial.println("WARNING: SD ERROR");
  }

  server.on("/data.json", HTTP_GET, [](AsyncWebServerRequest * request) { //set up function for what to do when the webpage asks for new data. [server's ip or web address]/data.json
    digitalWrite(LED_BUILTIN, HIGH); //for debugging, blink led when requests are made
    String response;
    serializeJson(dataDoc, response); //TODO: optimize, serialize data once in createJsonData(), not every time request made
    request->send(200, "application/json", response); //send requested .json file with data that was assembled
    digitalWrite(LED_BUILTIN, LOW);
  });

  server.begin();
  digitalWrite(LED_BUILTIN, LOW);
  //one time setup finished
}

void loop() {
  //main loop code! runs continuously
  wifiAvailable = checkWifiConnection();
  timeAvailable = updateTimeClock();

  float value = analogRead(36);
  dataBuffer1.push(value);
  createJsonData();
  delay(1000); //TODO: run createJsonData() on a timer and remove delay(). loop() should be fast!
  vTaskDelay(20);
}

void createJsonData() {
  /*
    Any variables that need to be sent to the webpage should be added here to a JSON document
    The webpage can ask for this JSON file to update itself
  */
  dataDoc.clear();
  dataDoc["time"] = millis() / 1000.0;
  JsonArray graph1 = dataDoc.createNestedArray("graph1"); //array of data within document
  for (int i = 0; i < dataBuffer1.size(); i++) {
    graph1.add(dataBuffer1[i]);
  }
}
