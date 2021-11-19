#include <FastLED.h>
const byte lct = 12;
const byte neopixelPin = 23;
CRGB leds[lct];
void setupDashboard() {
  setupDashboardLEDS();
}
void setupDashboardLEDS() {
  FastLED.addLeds<WS2812B, neopixelPin, GRB>(leds, lct);
}
void runDashboard() {
  displayDashboardBatIndicator(liveBatPer);
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
