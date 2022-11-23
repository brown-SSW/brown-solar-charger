#include <FastLED.h>
#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix = Adafruit_7segment();
const byte lct = 12; //LedCounT
const byte neopixelPin = 23;
CRGB leds[lct];
void setupDashboard() {
  //  matrix.begin(0x70);
  setupDashboardLEDS();
}
void setupDashboardLEDS() {
  FastLED.addLeds<WS2812B, neopixelPin, GRB>(leds, lct);
}
void runDashboard() {
  displayDashboardBatIndicator(liveBatPer);
  //  printToDisplay(liveUseW);
}

void printToDisplay(int wattage) {
  matrix.print(wattage, DEC);
  matrix.writeDisplay();
}

void displayDashboardBatIndicator(int percent) {
  memset(leds, CRGB(0, 0, 0), sizeof(leds));
  percent = remapPercent(percent, BatteryDischargeFloor);
  int nb = round((float) percent / 100.0 * (float) lct);
  for (int i = 0; i < nb; i++) {
    if (percent <= LowBatteryThreshold) {
      leds[i] = CRGB(255, 0, 0);
    } else if (percent <= MidBatteryThreshold) {
      leds[i] = CRGB(255, 255, 0);
    } else {
      leds[i] = CRGB(0, 255, 0);
    }
  }
  FastLED.show();
}
int remapPercent(int input, int percentFloor) {
  return map(input, percentFloor, 100, 0, 100);
}
