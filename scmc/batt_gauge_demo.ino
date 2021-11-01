#include <FastLED.h>
#define NUM_LEDS 12
#define DATA_PIN 14
#define LOW_BATT 20
#define MID_BATT 50


CRGB leds[NUM_LEDS];
void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);//varies
  Serial.begin(115200);
  Serial.print("Lit fam :)");
}

void batt(int percent) {
  // 
  memset(leds, CRGB(0, 0, 0), sizeof(leds));
  int nb = round((float) percent / 100.0 * (float) NUM_LEDS);
  Serial.println(nb);
  for (int i = 0; i < nb; i++) {
    if (percent <= LOW_BATT) {
      leds[i] = CRGB(255, 0, 0);
    } else if (percent <= MID_BATT) {
      leds[i] = CRGB(255, 255, 0);
    } else {
      leds[i] = CRGB(0, 255, 0);
    }
  }
  FastLED.show();
}
void loop() {
  for (int j = 0; j < 100; j++) { // loops through animation (100ms steps)
    batt(j);
    delay(100);
  }
}
