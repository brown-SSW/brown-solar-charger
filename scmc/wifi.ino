void setupWifi() {
  WiFi.mode(WIFI_STA);
  if (0 == strcmp(wifi_password, "None")) {
    Serial.print("wifi setup with open network named: ");
    Serial.println(wifi_ssid);
    WiFi.begin(wifi_ssid); //connect to wifi network, remove password for open networks
  } else {//wifi requires password
    Serial.print("wifi setup with network named: ");
    Serial.print(wifi_ssid);
    Serial.print(" and password: ");
    Serial.println(wifi_password);
    WiFi.begin(wifi_ssid, wifi_password); //connect to wifi network, remove password for open networks
  }
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
