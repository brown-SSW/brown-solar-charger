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
  boolean report = true;
  byte counter = 0;
  while (WiFi.status() != WL_CONNECTED && report) {
    WiFi.begin(wifi_ssid, wifi_password); //connect to wifi network, remove password for open networks
    delay(300);
    counter++;
    if (counter > 3) { //try to connect 3 times
      report = false;
    }
  }
  return report;
}
