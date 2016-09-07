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

// analog reading must drop by this value from value at armed for firing to occur
const word delta = 50;
const unsigned long retriggerInterval = 100UL; // ms

#define ARM_PIN 3
#define BOUNCE_TIME 1UL
Bounce armedSelect(ARM_PIN, BOUNCE_TIME);

#define COUNT_PIN 5
Bounce countSelect(COUNT_PIN, BOUNCE_TIME);
#define N_CYCLE 4
byte nCycle = 0;
const byte cycle[N_CYCLE] = {1, 3, 5, 10};

#define DURATION_PIN 4
Bounce durationSelect(DURATION_PIN, BOUNCE_TIME);
#define N_DURATION 3
byte nDuration = 0;
const unsigned long onDuration[N_DURATION] = {100UL, 200UL, 500UL};
const unsigned long offDuration = 50UL;
unsigned long simulationEvery = 1000UL;

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
    // check for operational changes
    checkForCountSelect();
    checkForDurationSelect();
  
    // see if a new analog threshold has been given over serial
    checkForThresholdSet();

    // see if we need to run another trial loop
    static Metro simulationInterval(simulationEvery);
    if( !solenoid.running() ) {
      if( simulationInterval.check() ) {
        Serial << F("Simulation started.") << endl;
        solenoid.show();
        solenoid.start();
      }
    } else {
      simulationInterval.reset();
    }

  } else {
    // check for sensor activity
    if( !solenoid.running() ) checkForSensor();
  }

  // update the lights
  showSettings(armed, solenoid.isFiring());
  
  static Metro printInterval(1000UL);
  if ( printInterval.check() ) sensor.show();
 
}

void showSettings(boolean armed, boolean on) {
  // used colors
  const CRGB armedOn = CRGB(255,0,0);
  const CRGB armedOff = CRGB(64,0,0);
  const CRGB disarmedOn = CRGB(0,0,255);
  const CRGB disarmedOff = CRGB(0,0,64);

  // track color
  static CRGB color = disarmedOff;
  
  if(armed && on) color=blend(color, armedOn, 50);
  else if(armed && !on) color=blend(color, armedOff, 50);
  else if(!armed && on) color=blend(color, disarmedOn, 5);
  else if(!armed && !on) color=blend(color, disarmedOff, 5);

  fill_solid(leds, NUM_LEDS, color);
  
  FastLED.show();
}


void setSolenoidAction() {
  solenoid.set(onDuration[nDuration], offDuration, cycle[nCycle]);
  solenoid.stop();
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
    solenoid.stop(); // clear any running simulation
    solenoid.arm(); // arm the solneoid
    
    // set our threshold from the average of 10 current readings.
    unsigned long thresh = 0;
    for( byte i=0; i<10; i++ ) {
      thresh += sensor.analogValue();
    }
    thresh /= 10;
    sensor.setThreshold(thresh > delta ? thresh - delta : 5);
  }

  if( changed && !armed ) {
    // immediate disable of firing
    solenoid.stop(); // just in case
    solenoid.disarm();
  }
  
  return( armed );
}

void checkForSensor() {
//  word reading = sensor.analogValue();
//  static word lastReading = reading;

//  word thresh = sensor.getThreshold();
  
  static boolean canTrigger = true;
  static Metro retrigger(retriggerInterval);

  // read the sensor
  boolean sensorTrigger = sensor.analogTrue();

  // can we trigger and is there a trigger signal?
  if( canTrigger &&  sensorTrigger) {
    Serial << F("*** SENSOR TRIPPED ***") << endl;
    sensor.show();
    solenoid.show();
  
    Serial << F("******* FIRING *******") << endl;
    solenoid.start();

    canTrigger = false;
  }
  // can we not trigger and is there a trigger signal?
  else if( !canTrigger && sensorTrigger ) {
    // block a retrigger 
    retrigger.reset();
  }
  // can we not trigger and is there not a trigger signal?
  else if( !canTrigger && !sensorTrigger ) {
    // if the retrigger interval up, now we can trigger again
    if( retrigger.check() ) {
      canTrigger = true;
      Serial << F("*** will retrigger ***") << endl;
    }
  }

}

