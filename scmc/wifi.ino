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
    if (!updateDNS()) { //likely unnecessary when set up by Brown IT, see function definition for details
      report = false;
    }
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

boolean updateDNS() {
  boolean report = true;
  /*
    This function can probably be removed once Brown IT does their setup.
    DNS-redirects name like google.com or brown.edu to IP addresses a computer can connect to
    For testing purposes this makes sure that a DNS service redirects a url to the ip address of the router the esp32 is connected to.
    https://www.duckdns.org is the service I'm using to redirect http://bruno-sun.duckdns.org/ to the ip address of my home router.
    Since the ip of my router isn't guaranteed to stay constant, this function makes sure that duckdns knows my IP.
  */
  HTTPClient http;
  http.begin(dns_url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    if (httpCode == 200 && payload == "OK") {
      report = true;
    } else {
      report = false;
      Serial.println(F("DNS update error:"));
      Serial.println(httpCode);
      Serial.println(payload);
    }
  }  else {
    report = false;
    Serial.println(F("Error with HTTP request for DNS"));
  }
  http.end();
  return report;
}
