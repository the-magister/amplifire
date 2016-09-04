#include <Streaming.h>
#include <Metro.h>
#include <Bounce.h>
#include <EEPROM.h>
#include <FastLED.h>
#include "Solenoid.h"
#include "Sensor.h"

// A: on/off, orange, D5
// B: selector, yellow, D4
// C: arm, green, D3
// D: fire, blue, D2

Solenoid solenoid;
#define SOLENOID_PIN 2
#define SOLENOID_OFF LOW
#define FIRE_AGAIN_LOCKOUT_DURATION 1000UL // after firing, don't fire again until this duration elapses.

Sensor sensor;

#define ARM_PIN 3
#define BOUNCE_TIME 1UL
Bounce armedSelect(ARM_PIN, BOUNCE_TIME);

#define COUNT_PIN 5
Bounce countSelect(COUNT_PIN, BOUNCE_TIME);
#define N_CYCLE 3
byte nCycle = 0;
const byte cycle[N_CYCLE] = {1, 3, 5};

#define DURATION_PIN 4
Bounce durationSelect(DURATION_PIN, BOUNCE_TIME);
#define N_DURATION 3
byte nDuration = 0;
const unsigned long onDuration[N_DURATION] = {100, 200, 500};
const unsigned long offDuration = 50;

#define NUM_LEDS 9
#define PIN_CLK 6 // yellow wire on LED strip
#define PIN_DATA 7 // green wire on LED strip
CRGB leds[NUM_LEDS];
#define MASTER_BRIGHTNESS 255

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // set up the solenoid
  solenoid.begin(SOLENOID_PIN, SOLENOID_OFF);
  setSolenoidAction();

  // set up the sensor
  sensor.begin();

  // set up the LEDs
  FastLED.addLeds<WS2801, PIN_DATA, PIN_CLK, BGR>(leds, NUM_LEDS);

  // set master brightness control
  FastLED.setBrightness(MASTER_BRIGHTNESS);

}

void loop() {

  // see if arming state has changed
  boolean armed = checkForArmed();
  if( ! armed ) {
    solenoid.stop(); // just in case
  }
  
  // if we're firing, just pay attention to that
  static boolean haveJustFired = false;
  static Metro fireAgainLockout(FIRE_AGAIN_LOCKOUT_DURATION);
  if ( solenoid.running() ) {
    haveJustFired = true;
    fireAgainLockout.reset();
    return;
  }

  // if we've just fired, check lockout timer to prevent refiring
  if ( haveJustFired ) {
    if ( fireAgainLockout.check() ) {
      // ok to fire again
      haveJustFired = false;
    } else {
      return;
    }
  }

  // check for operational changes
  checkForCountSelect();
  checkForDurationSelect();

  // see if a new analog threshold has been given over serial
  checkForThresholdSet();

  // check for sensor activity
  if( armed ) checkForSensor();

  static Metro printInterval(1000UL);
  if ( printInterval.check() ) sensor.show();

  // update the lights
  static byte currentHue = 0;
  if( armed ) currentHue = HUE_RED;
  else currentHue ++;
  showSettings(currentHue);
  
}

void showSettings(byte hue) {
  fill_rainbow(leds, NUM_LEDS, hue, 360/(NUM_LEDS-1));
  FastLED.show();
}


void setSolenoidAction() {
  solenoid.set(onDuration[nDuration], offDuration, cycle[nCycle]);
  solenoid.show();
}

void checkForDurationSelect() {
  if ( durationSelect.update() && durationSelect.read() == HIGH ) {
    
    // button state changed
    nDuration++;
    if ( nDuration >= N_DURATION ) nDuration = 0;
    // change timings
    setSolenoidAction();
  }
}
void checkForCountSelect() {
  if ( countSelect.update() && countSelect.read() == HIGH ) {
    
    // button state changed
    nCycle++;
    if ( nCycle >= N_CYCLE ) nCycle = 0;
    // change timings
    setSolenoidAction();
  }
}


void checkForThresholdSet() {
  if ( Serial.available() ) {
    delay(100UL); // wait for the rest of the input
    int set = Serial.parseInt();
    word analogThreshold = constrain(set, 0, 1023);
    sensor.setThreshold(analogThreshold);
  }
}

boolean checkForArmed() {
  boolean changed = armedSelect.update();
  boolean armed = armedSelect.read() == HIGH;

  if( changed && armed ) {
    Serial << F("*** ARMED ***") << endl;
    // set our threshold from the average of 10 current readings.
    unsigned long thresh = 0;
    for( byte i=0; i<10; i++ ) {
      thresh += sensor.analogValue();
    }
    thresh /= 10;
    sensor.setThreshold(thresh - 50);
  }

  if( changed && !armed ) {
    Serial << F("*** DISARMED ***") << endl;
  }
  
  return( armed );
}

void checkForSensor() {
  if( sensor.analogTrue() ) {
    Serial << F("*** SENSOR TRIPPED ***") << endl;
    sensor.show();
    solenoid.show();
  
    Serial << F("******* FIRING *******") << endl;
    solenoid.start();
  }
}

