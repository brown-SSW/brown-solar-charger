/**
   Solar Charger Monitoring Code
   This program monitors a solar power system and runs a physical and online dashboard.
   Compatible with esp32 dev board with 38 pins
   charge controller: Renogy ROVER ELITE 40A MPPT

   Made by members of Brown University club "Scientists for a Sustainable World" (s4sw@brown.edu) 2021
*/

#include <WiFi.h> //used for sending stuff over the wifi radio within the esp32
#include <SPIFFS.h> //this library gives access to an internal file system where webpage (.html) files can be stored
//https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
#include <ESPAsyncWebServer.h> //library for being a web server
#include <HTTPClient.h> //used for making http GET requests to other servers
#include <ArduinoJson.h> //library for handling .json data https://arduinojson.org/v6/doc/installation/
#include <CircularBuffer.h> //used for having fixed size buffers that discard the oldest data https://github.com/rlogiacco/CircularBuffer

const char* ssid = "chicken"; //name of the wifi network you want to connect to (e.g. Brown-Guest)
const char* password = "bawkbawk"; //password to network, for network without password, remove this line and the password argument in WiFi.begin
//https://ithelp.brown.edu/kb/articles/connect-a-browserless-device-to-brown-s-wireless-network-printers-consoles-smart-tvs-etc

const byte LED_BUILTIN = 2; //esp32s have a blue light on pin 2, can be nice for status and debugging

AsyncWebServer server(80); //port 80 is the standard for serving web pages, though Brown IT may want us to use something else

CircularBuffer<float, 50> dataBuffer1; //declare a buffer that can hold 50 floats

StaticJsonDocument<1024> dataDoc; //must be large enough to hold all the data served as a .json. https://arduinojson.org/v6/assistant/

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  if (!SPIFFS.begin()) { //start the internal filesystem thing
    Serial.println("An Error has occurred while mounting SPIFFS");
    while (true); //this makes the code freeze if there's an error. TODO: better error handling
  }

  //set up pin modes and hardware connections.
  //everything should be in the safest state until the code has fully started

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //connect to wifi network

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { //wait until connected
    delay(1000);
    Serial.print(".");
    if (millis() > 10000) { //if after 10 seconds there's no connection, restart the esp32 since that often fixes the connection
      ESP.restart();   //TODO: instead of a hard restart, try restarting just wifi connection?
      //and handle error of wifi never connecting without station being completely non functional?
    }
  }
  Serial.println();
  Serial.print("local ip: ");  Serial.println(WiFi.localIP()); //display IP address of station on local network, for debugging and internet setup

  updateDNS(); //likely unnecessary when set up by Brown IT, see function definition for details

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) { //set up function for what to do when the webserver is asked for its home webpage
    digitalWrite(LED_BUILTIN, HIGH); //for debugging, blink led when requests are made
    request->send(SPIFFS, "/page.html", "text/html"); //send the file page.html that was stored in SPIFFS
    digitalWrite(LED_BUILTIN, LOW);
  });

  server.on("/data.json", HTTP_GET, [](AsyncWebServerRequest * request) { //set up function for what to do when the webpage asks for new data. [server's ip or web address]/data.json
    digitalWrite(LED_BUILTIN, HIGH); //for debugging, blink led when requests are made
    String response;
    serializeJson(dataDoc, response); //TODO: optimize, serialize data once in createJsonData(), not every time request made
    request->send(200, "application/json", response); //send requested .json file with data that was assembled
    digitalWrite(LED_BUILTIN, LOW);
  });

  server.onNotFound(notFound); //setup a page not found response

  server.begin();
  digitalWrite(LED_BUILTIN, LOW);
  //one time setup finished

}

void notFound(AsyncWebServerRequest * request)
{
  request->send(404, "text/html", "page not found :(");
}

void updateDNS() {
  /*
    This function can probably be removed once Brown IT does their setup.
    DNS-redirects name like google.com or brown.edu to IP addresses a computer can connect to
    For testing purposes this makes sure that a DNS service redirects a url to the ip address of the router the esp32 is connected to.
    https://www.duckdns.org is the service I'm using to redirect http://bruno-sun.duckdns.org/ to the ip address of my home router.
    Since the ip of my router isn't guaranteed to stay constant, this function makes sure that duckdns knows my IP.
  */
  HTTPClient http;
  Serial.println("updating DNS");
  http.begin("https://www.duckdns.org/update?domains=bruno-sun&token=14552b6f-450e-441e-94cc-c7841ccad875");
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    if (httpCode == 200 && payload == "OK") {
      Serial.println("DNS update successful");
    } else {
      Serial.println("DNS update error:");
      Serial.println(httpCode);
      Serial.println(payload);
    }
  }  else {
    Serial.println(F("Error with HTTP request for DNS"));
  }
  http.end();
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
void loop() {
  //main loop code! runs continuously
  float value = analogRead(36);
  dataBuffer1.push(value);
  createJsonData();
  delay(1000); //TODO: run createJsonData() on a timer and remove delay(). loop() should be fast!
}
