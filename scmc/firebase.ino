FirebaseData fbdo; //firebase data object
FirebaseAuth fauth;
FirebaseConfig fconfig;
FirebaseJson fjson;
FirebaseJsonData jsonData;

void connectFirebase() {
  fconfig.host = FIREBASE_URL;
  fconfig.api_key = FIREBASE_API_KEY;
  fauth.user.email = FIREBASE_EMAIL;
  fauth.user.password = FIREBASE_PASSWORD;
  Firebase.begin(&fconfig, &fauth);
  fbdo.setResponseSize(4096);
  Firebase.RTDB.setReadTimeout(&fbdo, 1000 * 4);
  Firebase.RTDB.setwriteSizeLimit(&fbdo, "medium");
  Firebase.RTDB.setMaxRetry(&fbdo, 1);
  Firebase.setFloatDigits(3);
}
boolean firebaseGetSettings() {
  if (Firebase.RTDB.getJSON(&fbdo, "/settings")) {
    fjson = fbdo.jsonObject();


    fjson.get(jsonData, "/BatteryDischargeFloor");
    if (jsonData.type == "int") {
      BatteryDischargeFloor = jsonData.intValue;
    } else {
      Serial.println("ERROR! Firebase (settings) BatteryDischargeFloor type wrong");
    }

    fjson.get(jsonData, "/LowBatteryThreshold");
    if (jsonData.type == "int") {
      LowBatteryThreshold = jsonData.intValue;
    } else {
      Serial.println("ERROR! Firebase (settings) LowBatteryThreshold type wrong");
    }

    fjson.get(jsonData, "/MidBatteryThreshold");
    if (jsonData.type == "int") {
      MidBatteryThreshold = jsonData.intValue;
    } else {
      Serial.println("ERROR! Firebase (settings) MidBatteryThreshold type wrong");
    }

    fjson.get(jsonData, "/liveDataUpdateIntervalMillis");
    if (jsonData.type == "int") {
      liveDataUpdateMillisInterval = jsonData.intValue;
    } else {
      Serial.println("ERROR! Firebase (settings) liveDataUpdateIntervalMillis type wrong");
    }

    fjson.clear();
    return true;
  } else {
    Serial.println("ERROR! Firebase (settings)");
    Serial.println(fbdo.errorReason());
    return false;
  }
}
boolean firebaseSendDebug() {
  fjson.clear();
  fjson.set("local ip", WiFi.localIP().toString());
  fjson.set("mac address", WiFi.macAddress());
  fjson.set("OTAEnabled (read)", otaEnable);
  fjson.set("time", int(timestampEpoch));
  fjson.set("minAfterMidnight", minutesAfterMidnight());
  fjson.set("sunrise", sunriseTime());
  fjson.set("sunset", sunsetTime());
  fjson.set("liveDataUpdateMillisInterval (read)", int(liveDataUpdateMillisInterval));
  fjson.set("seconds since boot", int(millis() / 1000));

  if (Firebase.RTDB.updateNodeSilent(&fbdo, "/debug", &fjson))  {
    return true;
  }  else  {
    Serial.println("ERROR! Firebase (sendDebug)");
    Serial.println(fbdo.errorReason());
    return false;
  }
}
boolean firebaseRecvDebug() {
  if (Firebase.RTDB.getJSON(&fbdo, "/debug")) {
    fjson = fbdo.jsonObject();

    fjson.get(jsonData, "/EnableOTA");
    if (jsonData.typeNum == FirebaseJson::JSON_BOOL) {
      otaEnable = jsonData.boolValue;
    } else {
      Serial.println("ERROR! Firebase (RecvDebug) EnableOTA type wrong");
    }

    //add debug data to recieve

    fjson.get(jsonData, "/REBOOT");
    if (jsonData.typeNum == FirebaseJson::JSON_BOOL) {
      if (jsonData.boolValue) {
        fjson.set("/REBOOT", false);
        Firebase.RTDB.updateNodeSilent(&fbdo, "/debug", &fjson);
        Serial.println("REBOOTING! (settings/REBOOT equaled true in firebase)");
        rebootESP32();
      }
    } else {
      Serial.println("ERROR! Firebase (RecvDebug) REBOOT type wrong");
    }

  } else {
    Serial.println("ERROR! Firebase (get debug)");
    return false;
  }
  return true;
}
boolean firebaseSendLiveData() {
  fjson.clear();
  fjson.set("Available", available);
  fjson.set("WGen", liveGenW);
  fjson.set("WUse", liveUseW);
  fjson.set("bat%", liveBatPer);
  fjson.set("cumulativeWhGen", cumulativeWhGen);
  fjson.set("time", int(timestampEpoch));

  if (Firebase.RTDB.setJSON(&fbdo, "/data/liveData", &fjson))  {
    return true;
  }
  else  {
    Serial.println("ERROR! Firebase (live)");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

boolean firebaseDeleteOldData(String path, unsigned long interval, byte num) {
  boolean report = true;
  for (byte i_i = 0; i_i < num; i_i++) {//run multiple times so the number of entries can be reduced not just mantained
    if (timeAvailable) { //if there's an error with getting the time, don't do anything
      QueryFilter fquery;
      fquery.orderBy("$key");
      fquery.limitToFirst(1);

      if (Firebase.RTDB.getJSON(&fbdo, path.c_str(), &fquery)) {
        fjson = fbdo.jsonObject();

        fjson.iteratorBegin();
        String key;      String value;      int type;
        fjson.iteratorGet(0, type, key, value);

        FirebaseJsonData timeJsonData;
        fjson.get(timeJsonData, key + "/time");

        if (!timeJsonData.success || timeJsonData.intValue < timestampEpoch - interval) { //old data
          String nodeToDelete = path + key;
          if (!Firebase.RTDB.deleteNode(&fbdo, nodeToDelete.c_str())) {
            Serial.println("ERROR! Firebase (delete delete)");
            Serial.println(fbdo.errorReason());
            report = false;
          }
        }
        fjson.iteratorEnd();
        fjson.clear();

      }
      else {
        //Failed to get JSON data at defined node, print out the error reason
        Serial.println("ERROR! Firebase (delete)");
        Serial.println(fbdo.errorReason());
        report = false;
      }
      fquery.clear();
    }
  }
  return report;
}

boolean firebaseSendDayData() {
  fjson.clear();
  fjson.set("WGen", liveGenW);
  fjson.set("WUse", liveUseW);
  fjson.set("bat%", liveBatPer);
  fjson.set("time", int(timestampEpoch));

  if (Firebase.RTDB.pushJSON(&fbdo, "/data/dayData", &fjson)) {
    return true;
  }
  else {
    Serial.println("ERROR! Firebase (day)");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

boolean firebaseSendMonthData() {
  fjson.clear();
  fjson.set("WhGen", dayGenWh);
  fjson.set("WhUse", dayUseWh);
  fjson.set("hUsed", dayHoursUsed);
  fjson.set("time", int(timestampEpoch));

  if (Firebase.RTDB.pushJSON(&fbdo, "/data/monthData", &fjson)) {
    return true;
  }
  else {
    Serial.println("ERROR! Firebase (month)");
    Serial.println(fbdo.errorReason());
    return false;
  }
}
