#include <Streaming.h>
#include <Metro.h>
#include <Bounce.h>
#include <EEPROM.h>
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

#define UNUSED_PIN 5
Bounce unusedSelect(ARM_PIN, BOUNCE_TIME);

#define MODE_PIN 4
Bounce modeSelect(MODE_PIN, BOUNCE_TIME);
const byte nModes = 2;
const unsigned long onDuration[nModes] = {50, 100};
const unsigned long offDuration[nModes] = {100, 50};
const byte nCycles[nModes] = {1, 100};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // set up the solenoid
  solenoid.begin(SOLENOID_PIN, SOLENOID_OFF);
  setMode(0);

  // set up the sensor
  sensor.begin();
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

  // check for a mode change
  checkForModeSelect();

  // see if a new analog threshold has been given over serial
  checkForThresholdSet();

  // check for sensor activity
  if( armed ) checkForSensor();

  static Metro printInterval(1000UL);
  if ( printInterval.check() ) sensor.show();
}

void checkForModeSelect() {
  static byte currentMode = 0; // will set to zero at first run
  if ( modeSelect.update() && modeSelect.read() == HIGH ) {
    
    // button state changed
    currentMode++;
    if ( currentMode >= nModes ) currentMode = 0;
    // change timings
    setMode(currentMode);
  }
}

void setMode(byte i) {
  solenoid.set(onDuration[i], offDuration[i], nCycles[i]);
  solenoid.show();
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
  }
  if( changed && !armed ) {
    Serial << F("*** DISARMED ***") << endl;
  }
  return( armed );
}

void checkForSensor() {
  if( sensor.eitherTrue() ) {
    Serial << F("*** SENSOR TRIPPED ***") << endl;
    sensor.show();
    solenoid.show();
  
    Serial << F("******* FIRING *******") << endl;
    solenoid.start();
  }
}

