void setupWifi() {
  boolean report = true;
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password); //connect to wifi network, remove password for open networks
}

boolean checkWifiConnection() {
  if (millis() - wifiCheckUpdateMillis > wifiCheckIntervalMillis) {
    wifiCheckUpdateMillis = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("reconnecting to wifi");
      WiFi.reconnect();
      if (WiFi.status() != WL_CONNECTED) {
        return false;
      }
    }
    if (wifiAvailable == false) {
      Serial.println("wifi reconnected");
    }
    return true;
  } else {
    return wifiAvailable;
  }
}
