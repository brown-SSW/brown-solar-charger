#include <ESPmDNS.h>
#include <ArduinoOTA.h> //This makes it possible to upload new code over wifi
void runOTA() {
  if (otaEnable) {
    ArduinoOTA.handle();
  }
}
void setupOTA() {
  // ArduinoOTA.setPort(3232);
  ArduinoOTA.setHostname(OTA_hostname);
  ArduinoOTA.setPassword(OTA_password);

  ArduinoOTA
  .onStart([]() {
    Serial.println("OTA starting");
    setSafe();
  })
  .onEnd([]() {
    rebootESP32();
    //Serial.println("\n OTA ENDED!");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    //    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
