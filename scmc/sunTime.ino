int minutesAfterMidnight() {
  //note that daylight savings time is not going to be toggled
  return timeClock.tm_hour * 60 + timeClock.tm_min;
}
int sunriseTime() {
  //timeClock is zero indexed, sunTime needs standard month and day ranges
  return sunTime.sunrise(timeClock.tm_year + 1900, timeClock.tm_mon + 1, timeClock.tm_mday + 1, false);
}
int sunsetTime() {
  return sunTime.sunset(timeClock.tm_year + 1900, timeClock.tm_mon + 1, timeClock.tm_mday + 1, false);
}

boolean updateTimeClock() {
  boolean report = false;
  if (!timeConfiged && wifiAvailable) {
    Serial.println("#############CONFIGURING TIME##############");
    configTime(3600 * UTC_offset, 0, "time.google.com");
    getLocalTime(&timeClock);
    timeConfiged = true;
    delay(1000);
  }
  if (timeConfiged) {
    if (!getLocalTime(&timeClock)) {
      report = false;
      Serial.println("Failed to obtain time!");
    } else {
      time(&timestampEpoch);
      Serial.print("time= ");
      Serial.println(timestampEpoch);
      report = true;
    }
  }
  return report;
}
