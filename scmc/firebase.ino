FirebaseData fbdo; //firebase data object
FirebaseAuth fauth;
FirebaseConfig fconfig;
FirebaseJson fjson;
boolean setupFirebase() {
  fconfig.host = FIREBASE_URL;
  fconfig.api_key = FIREBASE_API_KEY;
  fauth.user.email = FIREBASE_EMAIL;
  fauth.user.password = FIREBASE_PASSWORD;
  Firebase.begin(&fconfig, &fauth);
  fbdo.setResponseSize(1024);
  Firebase.RTDB.setReadTimeout(&fbdo, 1000 * 10);
  Firebase.RTDB.setwriteSizeLimit(&fbdo, "small");
  Firebase.setFloatDigits(3);
  //todo, load settings, confirm connection
  return true;
}
boolean firebaseSendLiveData() {
  fjson.clear();
  fjson.add("Available", available);
  fjson.add("WGen", liveGenW);
  fjson.add("WUse", liveUseW);
  fjson.add("bat%", liveBatPer);
  fjson.add("cumulativeWhGen", cumulativeWhGen);
  fjson.add("time", int(timestampEpoch));

  if (Firebase.RTDB.setJSON(&fbdo, "data/liveData", &fjson))
  {
    return true;
  }
  else
  {
    //    Serial.println("FIREBASE FAILED");
    //    Serial.println("REASON: " + fbdo.errorReason());
    //    Serial.println("------------------------------------");
    return false;
  }
}
