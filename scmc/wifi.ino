boolean setupWifi() {
  boolean report = true;
  WiFi.mode(WIFI_STA);
  byte counter = 0;
  do {
    delay(500);
    WiFi.begin(wifi_ssid, wifi_password); //connect to wifi network, remove password for open networks
    if (counter > 10) { //try to connect 10 times
      report = false;
    }
    counter++;
  } while (WiFi.status() != WL_CONNECTED && report);
  if (report) {
    Serial.println();
    Serial.print(F("local ip: "));  Serial.println(WiFi.localIP()); //display IP address of station on local network, for debugging and internet setup
  }
  return report;
}

boolean checkWifiConnection() {
  if (millis() - wifiCheckUpdateMillis > 1000) {
    wifiCheckUpdateMillis = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("reconnecting to wifi");
      WiFi.reconnect();
      if (WiFi.status() != WL_CONNECTED) {
        return false;
      }
    }
    return true;
  } else {
    return wifiAvailable;
  }
}
