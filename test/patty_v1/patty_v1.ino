#include <Streaming.h>
#include <Metro.h>
#include <Bounce.h>
#include <EEPROM.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

/*  Belt Layout:
 *   
 *        n=23               n=23
 * ================== ==================
 *   B9  l8  l6  B5     B4  l3  l1  B0
 *         B7                 B2
 *                            
 * B = 7 led whorl
 * l = 1 led dot
 * 
 */

#define NUM_LEDS 2*(3*7+2)
#define PIN_DATA 12
#define MASTER_BRIGHTNESS 16
//CRGB *leds[NUM_LEDS];
CRGBArray<NUM_LEDS> leds;
// right side
CRGBSet rS = leds(0, 22); 
// left side
CRGBSet lS = leds(NUM_LEDS-1, 23);

#define BUTTON 14
#define PRESS_TIME 5UL
Bounce button = Bounce( BUTTON, PRESS_TIME );

void setup() {
  // put your setup code here, to run once:
  // put your setup code here, to run once:
  Serial.begin(115200);

  // set up the LEDs
  FastLED.addLeds<WS2811, PIN_DATA, GRB>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  // set master brightness control
  FastLED.setBrightness(MASTER_BRIGHTNESS);

  pinMode(BUTTON, INPUT_PULLUP);

  fill_solid(leds, NUM_LEDS, CRGB(255, 0, 0));
  FastLED.show();
  delay(1000);

  fill_solid(leds, NUM_LEDS, CRGB(0, 255, 0));
  FastLED.show();
  delay(1000);

  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 255));
  FastLED.show();
  delay(1000);

  fill_solid(leds, NUM_LEDS, CRGB(255, 255, 255));
  FastLED.show();
  delay(1000);

}

void loop() {
  static CRGB currentColor = CRGB::Black;

  // put your main code here, to run repeatedly:
  if ( button.update() ) {
    Serial << F("Button: ");
    if ( button.read() == LOW ) {
      // pressed
      Serial << F(" pressed.") << endl;
      currentColor = CRGB::Blue;
    } else {
      // released
      Serial << F(" released.") << endl;
      currentColor = CRGB::Black;
    }
  }

  fill_solid(leds, NUM_LEDS, currentColor);
  FastLED.show();

}
