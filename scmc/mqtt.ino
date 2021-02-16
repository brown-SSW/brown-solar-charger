void setupMQTT() {
  MQTTclient.setServer(MQTTserver, MQTTport);
  MQTTclient.setCallback(MQTTcallback);
}
boolean checkMQTTConnection() {
  if (MQTTclient.connect("station1")) {
    subscribeMQTT();
    return true;
  } else {
    Serial.print("mqtt connection failed: ");
    Serial.println(MQTTclient.state());
    return false;
  }
}

void mqttSendBy(byte msg) {  // add a byte to the tosendData array
  mqttBuffer[mqttArrayCounter] = msg;
  mqttArrayCounter++;
}

void mqttSendIn(int msg) {  // add an int to the tosendData array (four bytes)
  union {
    byte b[4];
    int v;
  } d;  // d is the union, d.b acceses the byte array, d.v acceses the int

  d.v = msg;  // put the value into the union as an int

  for (int i = 0; i < 4; i++) {
    mqttBuffer[mqttArrayCounter] = d.b[i];
    mqttArrayCounter++;
  }
}

void mqttSendFl(float msg) {  // add a float to the tosendData array (four bytes)
  union {  // this lets us translate between two variables (equal size, but one's 4 bytes in an array, and one's a 4 byte float Reference for unions: https:// www.mcgurrin.info/robots/127/
    byte b[4];
    float v;
  } d;  // d is the union, d.b acceses the byte array, d.v acceses the float

  d.v = msg;

  for (int i = 0; i < 4; i++) {
    mqttBuffer[mqttArrayCounter] = d.b[i];
    mqttArrayCounter++;
  }
}
