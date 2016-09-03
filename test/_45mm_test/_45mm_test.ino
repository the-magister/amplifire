// ArrayOfLedArrays - see https://github.com/FastLED/FastLED/wiki/Multiple-Controller-Examples for more info on
// using multiple controllers.  In this example, we're going to set up four NEOPIXEL strips on three
// different pins, each strip getting its own CRGB array to be played with, only this time they're going
// to be all parts of an array of arrays.

#include "FastLED.h"

#define NUM_STRIPS 1
#define NUM_LEDS_PER_STRIP 9
#define PIN_CLK 6 // shared by the strips
#define PIN_DATA 7
CRGB leds[NUM_LEDS_PER_STRIP];

#define MASTER_BRIGHTNESS 255
#define FRAMES_PER_SECOND 2

// For mirroring strips, all the "special" stuff happens just in setup.  We
// just addLeds multiple times, once for each strip
void setup() {
  Serial.begin(115200);

  Serial.println("Startup.");
  
  FastLED.addLeds<WS2801, PIN_DATA, PIN_CLK, BGR>(leds, NUM_LEDS_PER_STRIP);

  // set master brightness control
  FastLED.setBrightness(MASTER_BRIGHTNESS);

}

void loop() {
  /*
  // This inner loop will go over each led in the current strip, one at a time
  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[i] = CRGB::Green;
    FastLED.show();
  }
  delay(1000);
  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
  }
  delay(1000);
  */
  // update the lights
  static byte gHue = random8(0,255);
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS_PER_STRIP, gHue++, 360/(NUM_LEDS_PER_STRIP-1));
  FastLED.show();
  delay(5);
  
}

